#include "MicroPitchDetune.h"
#include <cmath>
#include <algorithm>

MicroPitchDetune::MicroPitchDetune()
    : randomEngine(std::random_device{}()),
    randomDistribution(0.0f, 1.0f)
{
}

void MicroPitchDetune::prepare(const juce::dsp::ProcessSpec& spec)
{
    sampleRate = static_cast<float>(spec.sampleRate);

    juce::dsp::ProcessSpec monoSpec = spec;
    monoSpec.numChannels = 1;

    auto dcCoeffs = juce::dsp::IIR::Coefficients<float>::makeHighPass(sampleRate, 5.0f);
    const int maxDelaySamples = static_cast<int>(sampleRate * maxDelayTime) + 1;

    // Prepare all delay taps, each with its own dc blocker and modulation smoother
    // so the 3 taps x 2 channels don't share (and corrupt) each other's filter state
    for (auto& tap : tapsL)
    {
        tap.delay.prepare(spec);
        tap.delay.setMaximumDelayInSamples(maxDelaySamples);
        tap.dcBlocker.coefficients = dcCoeffs;
        tap.dcBlocker.prepare(monoSpec);
        tap.modulationSmoother.reset(sampleRate, 0.002);  // 2ms for smooth modulation
        tap.phaseOffset = randomDistribution(randomEngine) * juce::MathConstants<float>::twoPi;
    }

    for (auto& tap : tapsR)
    {
        tap.delay.prepare(spec);
        tap.delay.setMaximumDelayInSamples(maxDelaySamples);
        tap.dcBlocker.coefficients = dcCoeffs;
        tap.dcBlocker.prepare(monoSpec);
        tap.modulationSmoother.reset(sampleRate, 0.002);
        tap.phaseOffset = randomDistribution(randomEngine) * juce::MathConstants<float>::twoPi;
    }

    updateTapOffsets();
    reset();
}

void MicroPitchDetune::reset()
{
    modPhase = 0.0f;

    for (auto& tap : tapsL)
    {
        tap.delay.reset();
        tap.dcBlocker.reset();
        tap.feedbackState = 0.0f;
        tap.pitchPhase = 0.0f;
        tap.modulationSmoother.setCurrentAndTargetValue(0.0f);
    }

    for (auto& tap : tapsR)
    {
        tap.delay.reset();
        tap.dcBlocker.reset();
        tap.feedbackState = 0.0f;
        tap.pitchPhase = 0.0f;
        tap.modulationSmoother.setCurrentAndTargetValue(0.0f);
    }
}

void MicroPitchDetune::setParams(float detuneCentsIn, float lfoRateIn, float lfoDepthIn,
    float delayCentreIn, float stereoSeparationIn, float mixIn,
    float feedbackIn, float diffusionIn)
{
    detuneCents = juce::jlimit(-50.0f, 50.0f, detuneCentsIn);

    // Calculate depth from detune if lfoDepthIn is not explicitly set
    if (lfoDepthIn <= 0.0f)
    {
        float calculatedDepth = std::abs(detuneCents) / 1000.0f * 0.005f;
        lfoDepth = juce::jlimit(0.0f, 0.01f, calculatedDepth);
    }
    else
    {
        lfoDepth = juce::jlimit(0.0f, 0.01f, lfoDepthIn);
    }

    lfoRate = juce::jlimit(0.01f, 10.0f, lfoRateIn);

    float oldDelayCentre = delayCentre;
    delayCentre = juce::jlimit(0.001f, maxDelayTime * 0.8f, delayCentreIn);

    stereoSeparation = juce::jlimit(0.0f, 1.0f, stereoSeparationIn);
    mix = juce::jlimit(0.0f, 1.0f, mixIn);
    feedback = juce::jlimit(0.0f, 0.7f, feedbackIn);

    float oldDiffusion = diffusion;
    diffusion = juce::jlimit(0.0f, 1.0f, diffusionIn);

    // Update tap offsets if diffusion or delay centre changed
    if (std::abs(oldDiffusion - diffusion) > 0.001f ||
        std::abs(oldDelayCentre - delayCentre) > 0.0001f)
    {
        updateTapOffsets();
    }
}

void MicroPitchDetune::setSyncEnabled(bool shouldSync)
{
    syncEnabled = shouldSync;
}

void MicroPitchDetune::setBpm(float newBpm)
{
    bpm = juce::jlimit(20.0f, 300.0f, newBpm);
}

float MicroPitchDetune::lfo(float phase)
{
    // Smooth sine wave
    return std::sin(phase);
}

float MicroPitchDetune::lfoTriangle(float phase)
{
    // Triangle wave for alternative modulation
    float normalizedPhase = phase / juce::MathConstants<float>::twoPi;
    normalizedPhase = std::fmod(normalizedPhase, 1.0f);
    if (normalizedPhase < 0.0f) normalizedPhase += 1.0f;

    return (normalizedPhase < 0.5f) ? (4.0f * normalizedPhase - 1.0f) : (3.0f - 4.0f * normalizedPhase);
}

void MicroPitchDetune::updateTapOffsets()
{
    // Configure multi-tap delays with diffusion
    // Tap 0: Center (main delay)
    // Tap 1: Slightly earlier with different phase
    // Tap 2: Slightly later with different phase

    for (int i = 0; i < NUM_TAPS; ++i)
    {
        float tapOffset = (i - 1.0f) * 0.002f * diffusion;  // �2ms max spread
        tapsL[i].timeOffset = tapOffset;
        tapsR[i].timeOffset = tapOffset * 1.1f;  // Slightly different for L/R
    }
}

void MicroPitchDetune::process(juce::dsp::AudioBlock<float>& block)
{
    auto numSamples = block.getNumSamples();
    auto numChannels = block.getNumChannels();

    if (numChannels == 0 || numSamples == 0)
        return;

    // Calculate modulation frequency
    float rate = lfoRate;
    if (syncEnabled && bpm > 0.0f)
    {
        float beatsPerSecond = bpm / 60.0f;
        rate = beatsPerSecond * lfoRate;
    }

    for (size_t i = 0; i < numSamples; ++i)
    {
        // Process each channel
        for (size_t ch = 0; ch < numChannels; ++ch)
        {
            float inSample = block.getSample(static_cast<int>(ch), static_cast<int>(i));
            float wetSample = 0.0f;

            // Determine if left or right processing
            bool isLeft = (ch % 2 == 0);
            auto& taps = isLeft ? tapsL : tapsR;

            // Detune L and R in opposite directions so the two channels drift apart in pitch
            const float channelDetuneCents = isLeft ? detuneCents : -detuneCents;
            const float pitchRatio = std::pow(2.0f, channelDetuneCents / 1200.0f);
            float channelStereoPhase = isLeft ? 0.0f : stereoSeparation;

            // Process each tap and sum the results
            for (int tapIdx = 0; tapIdx < NUM_TAPS; ++tapIdx)
            {
                auto& tap = taps[tapIdx];

                // Calculate LFO modulation for this tap (vibrato/chorus character, independent of the pitch shift below)
                float tapPhase = modPhase + channelStereoPhase + (tapIdx * 0.333f);  // Spread taps in phase
                float lfoValue = lfo(tapPhase * juce::MathConstants<float>::twoPi + tap.phaseOffset);

                // Anti-alias the modulation
                tap.modulationSmoother.setTargetValue(lfoValue);
                float smoothedLfo = tap.modulationSmoother.getNextValue();

                float baseDelayTime = delayCentre + tap.timeOffset + smoothedLfo * lfoDepth;
                baseDelayTime = juce::jlimit(0.0005f, maxDelayTime, baseDelayTime);

                // Real pitch shifting: continuously ramp the read-head position (a fixed offset
                // does nothing to steady-state pitch) using two crossfaded grains so the ramp can
                // wrap without a click, classic delay-line pitch shifter.
                const float headroom = maxDelayTime - baseDelayTime - 0.0005f;
                const float windowTime = juce::jlimit(0.002f, 0.010f, headroom);
                const float windowSamples = windowTime * sampleRate;

                tap.pitchPhase += (1.0f - pitchRatio) / windowSamples;
                tap.pitchPhase -= std::floor(tap.pitchPhase);  // wrap to [0, 1)

                float grain1Phase = tap.pitchPhase;
                float grain2Phase = grain1Phase + 0.5f;
                if (grain2Phase >= 1.0f)
                    grain2Phase -= 1.0f;

                // Equal-power crossfade window: sin^2 + cos^2 == 1
                const float gain1 = std::sin(juce::MathConstants<float>::pi * grain1Phase) *
                    std::sin(juce::MathConstants<float>::pi * grain1Phase);
                const float gain2 = 1.0f - gain1;

                const float baseDelaySamples = baseDelayTime * sampleRate;
                const float maxSamples = maxDelayTime * sampleRate;
                const float delay1Samples = juce::jlimit(1.0f, maxSamples, baseDelaySamples + grain1Phase * windowSamples);
                const float delay2Samples = juce::jlimit(1.0f, maxSamples, baseDelaySamples + grain2Phase * windowSamples);

                tap.delay.setDelay(delay1Samples);
                const float tapSample1 = tap.delay.popSample(0);
                tap.delay.setDelay(delay2Samples);
                const float tapSample2 = tap.delay.popSample(0);

                const float tapSample = tapSample1 * gain1 + tapSample2 * gain2;

                // Apply DC blocking to feedback path
                tap.feedbackState = (feedback > 0.0f) ? tap.dcBlocker.processSample(tapSample) : 0.0f;

                // Write to delay line with feedback
                tap.delay.pushSample(0, inSample + tap.feedbackState * feedback);

                // Accumulate tap output (with gain compensation for multiple taps)
                float tapGain = 1.0f / static_cast<float>(NUM_TAPS);
                wetSample += tapSample * tapGain;
            }

            // Mix dry and wet signals with equal-power crossfade
            float wetGain = std::sin(mix * juce::MathConstants<float>::halfPi);
            float dryGain = std::cos(mix * juce::MathConstants<float>::halfPi);
            float finalSample = inSample * dryGain + wetSample * wetGain;

            block.setSample(static_cast<int>(ch), static_cast<int>(i), finalSample);
        }

        // Advance LFO phase
        modPhase += rate / sampleRate;
        modPhase = std::fmod(modPhase, 1.0f);
    }
}

void MicroPitchDetune::loadPreset(const Preset& preset)
{
    setParams(preset.detuneCents, preset.lfoRate, preset.lfoDepth,
        preset.delayCentre, preset.stereoSeparation, preset.mix,
        preset.feedback, preset.diffusion);
    setSyncEnabled(preset.syncEnabled);
}

MicroPitchDetune::Preset MicroPitchDetune::getCurrentPreset() const
{
    return Preset("Current", detuneCents, lfoRate, lfoDepth, delayCentre,
        stereoSeparation, mix, feedback, diffusion, syncEnabled);
}

std::vector<MicroPitchDetune::Preset> MicroPitchDetune::getFactoryPresets()
{
    return {
        Preset("Subtle Detune", 5.0f, 0.1f, 0.0f, 0.005f, 0.3f, 0.3f, 0.0f, 0.0f, false),
        Preset("Wide Chorus", 12.0f, 0.5f, 0.003f, 0.008f, 0.7f, 0.5f, 0.2f, 0.3f, false),
        Preset("Shimmer", 8.0f, 0.2f, 0.004f, 0.010f, 0.5f, 0.4f, 0.3f, 0.6f, false),
        Preset("Flanger-ish", 15.0f, 0.3f, 0.006f, 0.003f, 0.4f, 0.5f, 0.4f, 0.2f, false),
        Preset("Deep Space", 20.0f, 0.08f, 0.005f, 0.012f, 0.9f, 0.6f, 0.5f, 0.8f, false),
        Preset("Synced Vibrato", 10.0f, 1.0f, 0.004f, 0.006f, 0.5f, 0.7f, 0.1f, 0.0f, true),
        Preset("Lush Ensemble", 7.0f, 0.15f, 0.003f, 0.007f, 0.6f, 0.45f, 0.25f, 0.5f, false),
        Preset("Micro Shift", 3.0f, 0.05f, 0.001f, 0.004f, 0.2f, 0.25f, 0.0f, 0.0f, false)
    };
}