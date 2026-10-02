#include "ExciterSaturation.h"
#include <cmath>

ExciterSaturation::ExciterSaturation()
{
}

void ExciterSaturation::prepare(const juce::dsp::ProcessSpec& spec)
{
    sampleRate = static_cast<float>(spec.sampleRate);

    // Construct oversampling here, now that the real channel count is known,
    // instead of hardcoding 2 (which broke on mono instances)
    oversampling = std::make_unique<juce::dsp::Oversampling<float>>(
        static_cast<size_t>(spec.numChannels), 1,
        juce::dsp::Oversampling<float>::filterHalfBandPolyphaseIIR, true, true);   // integer latency
    oversampling->initProcessing(spec.maximumBlockSize);
    latencySamples = juce::roundToInt(oversampling->getLatencyInSamples());

    dryDelay.setMaximumDelayInSamples(juce::jmax(16, latencySamples + 4));
    dryDelay.prepare(spec);
    dryDelay.setDelay(static_cast<float>(latencySamples));
    juce::dsp::ProcessSpec oversampledSpec = spec;
    oversampledSpec.sampleRate *= 2.0;
    oversampledSpec.maximumBlockSize *= 2;

    // Prepare all filters
    highpass.prepare(oversampledSpec);
    preEmphasis.prepare(oversampledSpec);
    deEmphasis.prepare(oversampledSpec);
    dcBlocker.prepare(oversampledSpec);
    toneFilter.prepare(spec);  // Tone filter at normal rate

    // Initialize filter coefficients
    updateHighpass();
    updatePreEmphasis();
    updateDeEmphasis();
    updateToneFilter();

    // DC blocker at 5Hz
    *dcBlocker.state = *juce::dsp::IIR::Coefficients<float>::makeHighPass(
        oversampledSpec.sampleRate, 5.0);

    // Prepare smoothed parameters
    smoothedDrive.reset(sampleRate, 0.02);      // 20ms ramp
    smoothedMix.reset(sampleRate, 0.02);
    smoothedDrive.setCurrentAndTargetValue(drive);
    smoothedMix.setCurrentAndTargetValue(mix);

    // Allocate buffers
    dryBuffer.setSize(static_cast<int>(spec.numChannels),
        static_cast<int>(spec.maximumBlockSize));
    oversampledBuffer.setSize(static_cast<int>(spec.numChannels),
        static_cast<int>(oversampledSpec.maximumBlockSize));

    reset();
}

void ExciterSaturation::reset()
{
    if (oversampling != nullptr)
        oversampling->reset();
    highpass.reset();
    preEmphasis.reset();
    deEmphasis.reset();
    dcBlocker.reset();
    toneFilter.reset();
    dryDelay.reset();
    autoGainValue = 1.0f;
    autoGainPrimed = false;
}

void ExciterSaturation::setDrive(float newDrive)
{
    // The exciterDrive APVTS parameter (and the Drive knob) spans 0-10, but this used to
    // clamp to 0-1 - so anything past the bottom 10% of the knob's travel, and most of the
    // factory presets' drive values, all collapsed to the same maxed-out internal drive.
    // That's why turning Drive further (or picking a preset with a "higher" drive value)
    // barely changed anything.
    drive = juce::jlimit(0.0f, 10.0f, newDrive);
    smoothedDrive.setTargetValue(drive);
}

void ExciterSaturation::setMix(float newMix)
{
    mix = juce::jlimit(0.0f, 1.0f, newMix);
    smoothedMix.setTargetValue(mix);
}

void ExciterSaturation::setHighpass(float freqHz)
{
    const float newFreq = juce::jlimit(20.0f, 20000.0f, freqHz);
    if (newFreq == highpassFreq)
        return;

    highpassFreq = newFreq;
    updateHighpass();
}

void ExciterSaturation::setToneBrightness(float brightness)
{
    const float newBrightness = juce::jlimit(0.0f, 1.0f, brightness);
    if (newBrightness == toneBrightness)
        return;

    toneBrightness = newBrightness;
    updatePreEmphasis();
    updateDeEmphasis();
}

void ExciterSaturation::setHarmonicBalance(float balance)
{
    const float newBalance = juce::jlimit(0.0f, 1.0f, balance);
    if (newBalance == harmonicBalance)
        return;

    harmonicBalance = newBalance;
    updateToneFilter();
}

void ExciterSaturation::setSaturationType(SaturationType type)
{
    saturationType = type;
}

void ExciterSaturation::setHarmonicMode(HarmonicMode mode)
{
    harmonicMode = mode;
}

void ExciterSaturation::setAutoGainEnabled(bool enabled)
{
    autoGainEnabled = enabled;
}

void ExciterSaturation::updateHighpass()
{
    float oversampledRate = sampleRate * 2.0f;
    *highpass.state = *juce::dsp::IIR::Coefficients<float>::makeHighPass(
        oversampledRate, highpassFreq);
}

void ExciterSaturation::updatePreEmphasis()
{
    // Pre-emphasis: boost highs before saturation for air/presence
    float oversampledRate = sampleRate * 2.0f;
    float emphasisFreq = juce::jmap(toneBrightness, 3000.0f, 8000.0f);
    float emphasisQ = 0.7f;
    float emphasisGain = juce::jmap(toneBrightness, 0.0f, 6.0f);  // Up to +6dB

    *preEmphasis.state = *juce::dsp::IIR::Coefficients<float>::makePeakFilter(
        oversampledRate, emphasisFreq, emphasisQ,
        juce::Decibels::decibelsToGain(emphasisGain));
}

void ExciterSaturation::updateDeEmphasis()
{
    // De-emphasis: compensate for pre-emphasis boost
    float oversampledRate = sampleRate * 2.0f;
    float emphasisFreq = juce::jmap(toneBrightness, 3000.0f, 8000.0f);
    float emphasisQ = 0.7f;
    float emphasisGain = juce::jmap(toneBrightness, 0.0f, 6.0f);

    *deEmphasis.state = *juce::dsp::IIR::Coefficients<float>::makePeakFilter(
        oversampledRate, emphasisFreq, emphasisQ,
        juce::Decibels::decibelsToGain(-emphasisGain * 0.5f));  // Partial compensation
}

void ExciterSaturation::updateToneFilter()
{
    // Post-saturation tone shaping based on harmonic balance
    float toneFreq = juce::jmap(harmonicBalance, 2000.0f, 8000.0f);
    float toneGain = juce::jmap(harmonicBalance, -3.0f, 3.0f);

    *toneFilter.state = *juce::dsp::IIR::Coefficients<float>::makeHighShelf(
        sampleRate, toneFreq, 0.7f, juce::Decibels::decibelsToGain(toneGain));
}

float ExciterSaturation::softSaturation(float x)
{
    // Smooth tanh saturation
    return std::tanh(x);
}

float ExciterSaturation::hardClip(float x)
{
    // A true hard clip. (This used to be a polynomial that fell back to zero at |x| = 2 and then
    // jumped to +/-1.5, which put a 1.5-unit cliff in the curve and glitched on every crossing.)
    return juce::jlimit(-1.0f, 1.0f, x);
}

float ExciterSaturation::tubeSaturation(float x)
{
    // Asymmetric saturation (emphasizes even harmonics)
    float bias = 0.1f;
    float biased = x + bias;
    return std::tanh(biased) - std::tanh(bias);
}

float ExciterSaturation::tapeSaturation(float x)
{
    // Tape-style saturation with compression
    float compressed = x / (1.0f + std::abs(x) * 0.3f);
    return std::tanh(compressed * 1.5f);
}

float ExciterSaturation::transformerSaturation(float x)
{
    // Transformer-style saturation (subtle, musical). x*(1-0.15x^2) only approximates a
    // saturation curve for |x| up to ~1.49 (where it peaks) - past that it curves back
    // down, crosses zero at |x| ~= 2.58, and then shoots off to +/-infinity for a cubic
    // in the wrong direction. driveAmount can multiply the input up to 20x, so almost any
    // real signal pushed it into that unbounded region, which is the "brutal, breaks
    // everything" behaviour. Clamp into the valid range first so it saturates smoothly
    // like the other types instead of inverting and exploding.
    float clamped = juce::jlimit(-1.49f, 1.49f, x);
    float x2 = clamped * clamped;
    return clamped * (1.0f - 0.15f * x2);
}

float ExciterSaturation::digitalSaturation(float x)
{
    // Bit reduction style. Clip to unity before quantizing - otherwise the driven
    // input can be far larger than the quantization step, so rounding barely moves
    // it and the bit-crush effect quietly disappears as Drive increases.
    float clipped = juce::jlimit(-1.0f, 1.0f, x);
    float bits = 8.0f;
    float levels = std::pow(2.0f, bits);
    return std::round(clipped * levels) / levels;
}

float ExciterSaturation::curve(float u, SaturationType type)
{
    switch (type)
    {
    case SaturationType::Soft:        return softSaturation(u);
    case SaturationType::Hard:        return hardClip(u);
    case SaturationType::Tube:        return tubeSaturation(u);
    case SaturationType::Tape:        return tapeSaturation(u);
    case SaturationType::Transformer: return transformerSaturation(u);
    case SaturationType::Digital:     return digitalSaturation(u);
    }

    return u;
}

float ExciterSaturation::waveshape(float x, SaturationType type, HarmonicMode mode, float driveAmount)
{
    const float u = x * driveAmount;
    float y = curve(u, type);

    // The harmonic mode shapes the saturator output while it is still at full (about +/-1) level.
    // It used to run after the output had been divided down by the drive, where it was nearly linear
    // and did next to nothing (Even mode even made the 2nd harmonic quieter).
    switch (mode)
    {
    case HarmonicMode::Balanced:
        break;

    case HarmonicMode::OddOnly:
        // Keep only the odd-symmetric part of the curve: asymmetric types (Tube) lose their even harmonics
        y = 0.5f * (y - curve(-u, type));
        break;

    case HarmonicMode::EvenOnly:
        // Add a squared term, which generates 2nd (and higher even) harmonics; the DC blocker removes its offset
        y += 0.35f * y * y;
        break;
    }

    return y;
}

void ExciterSaturation::process(juce::dsp::AudioBlock<float>& block)
{
    if (block.getNumSamples() == 0 || oversampling == nullptr)
        return;

    const int numSamples = static_cast<int>(block.getNumSamples());
    const int numChannels = static_cast<int>(block.getNumChannels());

    // Keep the dry signal for the parallel mix, delayed by the same amount as the oversampled wet path
    // so the two line up (otherwise they comb-filter against each other).
    for (int ch = 0; ch < numChannels; ++ch)
    {
        const float* in = block.getChannelPointer(static_cast<size_t>(ch));
        float* dry = dryBuffer.getWritePointer(ch);
        for (int i = 0; i < numSamples; ++i)
        {
            dryDelay.pushSample(ch, in[i]);
            dry[i] = dryDelay.popSample(ch);
        }
    }

    // Upsample. `block` itself is untouched by this call until processSamplesDown() overwrites it
    // with the excited band only.
    juce::dsp::AudioBlock<float> oversampledBlock = oversampling->processSamplesUp(block);
    const int oversampledNumSamples = static_cast<int>(oversampledBlock.getNumSamples());

    highpass.process(juce::dsp::ProcessContextReplacing<float>(oversampledBlock));
    preEmphasis.process(juce::dsp::ProcessContextReplacing<float>(oversampledBlock));

    // Saturation. The drive smoother advances once per sample, not per channel. The band level going
    // in is measured so Auto Gain can match what comes out to it.
    double sumSquaresIn = 0.0;
    for (int i = 0; i < oversampledNumSamples; ++i)
    {
        const float driveAmount = juce::jmap(smoothedDrive.getNextValue(), 0.0f, 10.0f, 1.0f, 20.0f);

        for (int ch = 0; ch < numChannels; ++ch)
        {
            float* samples = oversampledBlock.getChannelPointer(static_cast<size_t>(ch));
            const float x = samples[i];
            sumSquaresIn += static_cast<double>(x) * x;

            float y = waveshape(x, saturationType, harmonicMode, driveAmount);

            // With Auto Gain off, keep the classic "unity gain for small signals" behaviour
            if (!autoGainEnabled)
                y /= driveAmount;

            samples[i] = y;
        }
    }

    dcBlocker.process(juce::dsp::ProcessContextReplacing<float>(oversampledBlock));

    if (autoGainEnabled)
    {
        // Level-match the saturated band to the band that went in, so Drive changes the harmonic
        // content rather than the loudness. Previously this compared the full-band input with the
        // high-passed output, which pinned the gain at its limit, and it jumped once per block.
        double sumSquaresOut = 0.0;
        for (int ch = 0; ch < numChannels; ++ch)
        {
            const float* samples = oversampledBlock.getChannelPointer(static_cast<size_t>(ch));
            for (int i = 0; i < oversampledNumSamples; ++i)
                sumSquaresOut += static_cast<double>(samples[i]) * samples[i];
        }

        const double count = static_cast<double>(oversampledNumSamples) * numChannels;
        const float rmsIn = static_cast<float>(std::sqrt(sumSquaresIn / count));
        const float rmsOut = static_cast<float>(std::sqrt(sumSquaresOut / count));

        float gainStart = autoGainValue;
        float gainEnd = autoGainValue;

        if (rmsIn > 1.0e-4f && rmsOut > 1.0e-6f)   // hold the last gain through silence
        {
            const float target = juce::jlimit(0.04f, 1.5f, rmsIn / rmsOut);

            if (!autoGainPrimed)
            {
                gainStart = gainEnd = target;
                autoGainPrimed = true;
            }
            else
            {
                // Come down fast when the band gets louder (no blasts on onsets), recover slowly
                const float blockSeconds = static_cast<float>(numSamples) / sampleRate;
                const float timeConstant = target < autoGainValue ? 0.005f : 0.040f;
                const float alpha = 1.0f - std::exp(-blockSeconds / timeConstant);
                gainEnd = autoGainValue + alpha * (target - autoGainValue);
            }
        }

        // Ramp across the block so the change is never a step
        for (int i = 0; i < oversampledNumSamples; ++i)
        {
            const float g = gainStart + (gainEnd - gainStart) * static_cast<float>(i + 1) / static_cast<float>(oversampledNumSamples);
            for (int ch = 0; ch < numChannels; ++ch)
                oversampledBlock.getChannelPointer(static_cast<size_t>(ch))[i] *= g;
        }

        autoGainValue = gainEnd;
    }
    else
    {
        autoGainValue = 1.0f;
        autoGainPrimed = false;
    }

    deEmphasis.process(juce::dsp::ProcessContextReplacing<float>(oversampledBlock));

    // Downsample - block now holds the excited band only
    oversampling->processSamplesDown(block);

    // Tone shaping of the excited band (at the normal rate)
    juce::dsp::ProcessContextReplacing<float> context(block);
    toneFilter.process(context);

    // Parallel mix: layer the excited band on top of the untouched (latency-matched) dry signal,
    // rather than crossfading the dry signal away - a crossfade would thin out everything below the
    // highpass as Mix increases. The mix smoother advances once per sample across all channels.
    for (int i = 0; i < numSamples; ++i)
    {
        const float currentMix = smoothedMix.getNextValue();

        for (int ch = 0; ch < numChannels; ++ch)
        {
            float* wet = block.getChannelPointer(static_cast<size_t>(ch));
            const float* dry = dryBuffer.getReadPointer(ch);
            wet[i] = dry[i] + wet[i] * currentMix;
        }
    }
}

void ExciterSaturation::loadPreset(const Preset& preset)
{
    setDrive(preset.drive);
    setMix(preset.mix);
    setHighpass(preset.highpassFreq);
    setToneBrightness(preset.toneBrightness);
    setHarmonicBalance(preset.harmonicBalance);
    setSaturationType(preset.satType);
    setHarmonicMode(preset.harmonicMode);
    setAutoGainEnabled(preset.autoGainEnabled);
}

ExciterSaturation::Preset ExciterSaturation::getCurrentPreset() const
{
    return Preset("Current", drive, mix, highpassFreq, toneBrightness,
        harmonicBalance, saturationType, harmonicMode, autoGainEnabled);
}

std::vector<ExciterSaturation::Preset> ExciterSaturation::getFactoryPresets()
{
    using ST = SaturationType;
    using HM = HarmonicMode;

    return {
        Preset("Subtle Air", 0.3f, 0.25f, 8000.0f, 0.7f, 0.6f, ST::Soft, HM::Balanced, true),
        Preset("Vocal Presence", 0.5f, 0.4f, 3000.0f, 0.6f, 0.7f, ST::Tube, HM::EvenOnly, true),
        Preset("Bright Shine", 0.6f, 0.5f, 5000.0f, 0.8f, 0.8f, ST::Soft, HM::Balanced, true),
        Preset("Warm Tape", 0.4f, 0.45f, 2000.0f, 0.4f, 0.4f, ST::Tape, HM::EvenOnly, true),
        Preset("Aggressive Edge", 0.7f, 0.6f, 4000.0f, 0.5f, 0.9f, ST::Hard, HM::OddOnly, true),
        Preset("Transformer Glue", 0.5f, 0.35f, 1000.0f, 0.5f, 0.5f, ST::Transformer, HM::Balanced, true),
        Preset("Digital Crunch", 0.6f, 0.5f, 3500.0f, 0.3f, 0.7f, ST::Digital, HM::OddOnly, false),
        Preset("Tube Warmth", 0.45f, 0.4f, 2500.0f, 0.55f, 0.45f, ST::Tube, HM::EvenOnly, true),
        Preset("Crystal Highs", 0.35f, 0.3f, 10000.0f, 0.9f, 0.85f, ST::Soft, HM::Balanced, true),
        Preset("Radio Voice", 0.65f, 0.55f, 500.0f, 0.4f, 0.6f, ST::Hard, HM::OddOnly, true)
    };
}