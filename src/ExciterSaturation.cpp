#include "ExciterSaturation.h"
#include <cmath>

ExciterSaturation::ExciterSaturation()
    : oversampling(2, 1, juce::dsp::Oversampling<float>::filterHalfBandPolyphaseIIR, true, false)
{
}

void ExciterSaturation::prepare(const juce::dsp::ProcessSpec& spec)
{
    sampleRate = static_cast<float>(spec.sampleRate);

    // Prepare oversampling
    oversampling.initProcessing(spec.maximumBlockSize);
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
    oversampling.reset();
    highpass.reset();
    preEmphasis.reset();
    deEmphasis.reset();
    dcBlocker.reset();
    toneFilter.reset();

    inputRMS.fill(0.0f);
    outputRMS.fill(0.0f);
}

void ExciterSaturation::setDrive(float newDrive)
{
    drive = juce::jlimit(0.0f, 1.0f, newDrive);
    smoothedDrive.setTargetValue(drive);
}

void ExciterSaturation::setMix(float newMix)
{
    mix = juce::jlimit(0.0f, 1.0f, newMix);
    smoothedMix.setTargetValue(mix);
}

void ExciterSaturation::setHighpass(float freqHz)
{
    highpassFreq = juce::jlimit(20.0f, 20000.0f, freqHz);
    updateHighpass();
}

void ExciterSaturation::setToneBrightness(float brightness)
{
    toneBrightness = juce::jlimit(0.0f, 1.0f, brightness);
    updatePreEmphasis();
    updateDeEmphasis();
}

void ExciterSaturation::setHarmonicBalance(float balance)
{
    harmonicBalance = juce::jlimit(0.0f, 1.0f, balance);
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

float ExciterSaturation::calculateGainCompensation()
{
    if (!autoGainEnabled)
        return 1.0f;

    // Calculate average RMS across channels
    float avgInputRMS = 0.0f;
    float avgOutputRMS = 0.0f;
    int numChannels = 0;

    for (size_t i = 0; i < inputRMS.size(); ++i)
    {
        if (inputRMS[i] > 0.0f)
        {
            avgInputRMS += inputRMS[i];
            avgOutputRMS += outputRMS[i];
            ++numChannels;
        }
    }

    if (numChannels == 0)
        return 1.0f;

    avgInputRMS /= static_cast<float>(numChannels);
    avgOutputRMS /= static_cast<float>(numChannels);

    // Prevent division by zero
    if (avgOutputRMS < 0.0001f)
        return 1.0f;

    // Calculate gain compensation with limiting
    float compensation = avgInputRMS / avgOutputRMS;
    return juce::jlimit(0.5f, 2.0f, compensation);  // Limit to ±6dB
}

float ExciterSaturation::softSaturation(float x)
{
    // Smooth tanh saturation
    return std::tanh(x);
}

float ExciterSaturation::hardClip(float x)
{
    // Soft clipping with polynomial
    float abs_x = std::abs(x);
    if (abs_x < 1.0f)
        return x;
    else if (abs_x < 2.0f)
        return juce::jlimit(-1.5f, 1.5f, x * (2.0f - abs_x) / 1.0f);
    else
        return (x > 0.0f) ? 1.5f : -1.5f;
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
    // Transformer-style saturation (subtle, musical)
    float x2 = x * x;
    return x * (1.0f - 0.15f * x2);
}

float ExciterSaturation::digitalSaturation(float x)
{
    // Bit reduction style
    float bits = 8.0f;
    float levels = std::pow(2.0f, bits);
    return std::round(x * levels) / levels;
}

float ExciterSaturation::waveshape(float x, SaturationType type)
{
    // Map drive (0-1) to useful range (1-20)
    float driveAmount = juce::jmap(smoothedDrive.getNextValue(), 1.0f, 20.0f);
    float driven = x * driveAmount;

    float output = 0.0f;

    switch (type)
    {
    case SaturationType::Soft:
        output = softSaturation(driven);
        break;
    case SaturationType::Hard:
        output = hardClip(driven);
        break;
    case SaturationType::Tube:
        output = tubeSaturation(driven);
        break;
    case SaturationType::Tape:
        output = tapeSaturation(driven);
        break;
    case SaturationType::Transformer:
        output = transformerSaturation(driven);
        break;
    case SaturationType::Digital:
        output = digitalSaturation(driven);
        break;
    }

    // Normalize output
    return output / driveAmount;
}

float ExciterSaturation::applyHarmonicMode(float x, HarmonicMode mode)
{
    switch (mode)
    {
    case HarmonicMode::Balanced:
        return x;

    case HarmonicMode::OddOnly:
    {
        // Symmetric saturation emphasizes odd harmonics
        return std::tanh(x * 2.0f) * 0.5f;
    }

    case HarmonicMode::EvenOnly:
    {
        // Asymmetric saturation emphasizes even harmonics
        float rectified = std::abs(x);
        float shaped = std::tanh(rectified * 1.5f);
        return (x >= 0.0f) ? shaped : -shaped * 0.8f;
    }
    }

    return x;
}

void ExciterSaturation::process(juce::dsp::AudioBlock<float>& block)
{
    if (block.getNumSamples() == 0)
        return;

    auto numSamples = static_cast<int>(block.getNumSamples());
    auto numChannels = static_cast<int>(block.getNumChannels());

    // Store dry signal
    dryBuffer.clear();
    for (int ch = 0; ch < numChannels; ++ch)
        dryBuffer.copyFrom(ch, 0, block.getChannelPointer(static_cast<size_t>(ch)), numSamples);

    // Calculate input RMS for auto-gain
    if (autoGainEnabled)
    {
        for (int ch = 0; ch < juce::jmin(numChannels, 2); ++ch)
        {
            float rms = 0.0f;
            auto* samples = block.getChannelPointer(static_cast<size_t>(ch));
            for (int i = 0; i < numSamples; ++i)
                rms += samples[i] * samples[i];
            inputRMS[static_cast<size_t>(ch)] = std::sqrt(rms / static_cast<float>(numSamples));
        }
    }

    // Upsample
    juce::dsp::AudioBlock<float> oversampledBlock = oversampling.processSamplesUp(block);

    // Apply highpass filter
    highpass.process(juce::dsp::ProcessContextReplacing<float>(oversampledBlock));

    // Apply pre-emphasis
    preEmphasis.process(juce::dsp::ProcessContextReplacing<float>(oversampledBlock));

    // Apply saturation
    for (int ch = 0; ch < numChannels; ++ch)
    {
        auto* samples = oversampledBlock.getChannelPointer(static_cast<size_t>(ch));
        auto oversampledNumSamples = static_cast<int>(oversampledBlock.getNumSamples());

        for (int i = 0; i < oversampledNumSamples; ++i)
        {
            float input = samples[i];
            float shaped = waveshape(input, saturationType);
            shaped = applyHarmonicMode(shaped, harmonicMode);
            samples[i] = shaped;
        }
    }

    // Apply de-emphasis
    deEmphasis.process(juce::dsp::ProcessContextReplacing<float>(oversampledBlock));

    // DC blocking
    dcBlocker.process(juce::dsp::ProcessContextReplacing<float>(oversampledBlock));

    // Downsample
    oversampling.processSamplesDown(block);

    // Apply tone filter (at normal sample rate)
    juce::dsp::ProcessContextReplacing<float> context(block);
    toneFilter.process(context);

    // Calculate output RMS and apply auto-gain
    float gainComp = 1.0f;
    if (autoGainEnabled)
    {
        for (int ch = 0; ch < juce::jmin(numChannels, 2); ++ch)
        {
            float rms = 0.0f;
            auto* samples = block.getChannelPointer(static_cast<size_t>(ch));
            for (int i = 0; i < numSamples; ++i)
                rms += samples[i] * samples[i];
            outputRMS[static_cast<size_t>(ch)] = std::sqrt(rms / static_cast<float>(numSamples));
        }

        gainComp = calculateGainCompensation();
    }

    // Apply gain compensation and mix with dry signal (equal-power crossfade)
    float currentMix = smoothedMix.getCurrentValue();
    float wetGain = std::sin(currentMix * juce::MathConstants<float>::halfPi) * gainComp;
    float dryGain = std::cos(currentMix * juce::MathConstants<float>::halfPi);

    for (int ch = 0; ch < numChannels; ++ch)
    {
        auto* wet = block.getChannelPointer(static_cast<size_t>(ch));
        auto* dry = dryBuffer.getReadPointer(ch);

        for (int i = 0; i < numSamples; ++i)
        {
            wet[i] = wet[i] * wetGain + dry[i] * dryGain;
        }

        // Advance smoothed mix for next sample
        smoothedMix.skip(1);
    }

    // Reset smoothed mix position for next block
    smoothedMix.skip(-numSamples);
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