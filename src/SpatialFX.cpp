#include "SpatialFX.h"

SpatialFX::SpatialFX()
{
    updateLfoWaveforms(); // Ensures LFOs have an initial waveform.
}

void SpatialFX::prepare(const juce::dsp::ProcessSpec& spec)
{
    sampleRate = static_cast<float>(spec.sampleRate);
    maxDelaySamples = static_cast<int>(maxDelaySeconds * sampleRate);

    delayLineLeft.setMaximumDelayInSamples(maxDelaySamples);
    delayLineRight.setMaximumDelayInSamples(maxDelaySamples);

    delayLineLeft.prepare(spec);
    delayLineRight.prepare(spec);
    lfoLeft.prepare(spec);
    lfoRight.prepare(spec);

    phaseOffsetLeft.reset(sampleRate, smoothingTime);
    phaseOffsetRight.reset(sampleRate, smoothingTime);
    modulationDepth.reset(sampleRate, smoothingTime);
    wetDryMix.reset(sampleRate, smoothingTime);

    updateLfoWaveforms();
    updateLfoFrequencies();

    // Initialize smoothed noise LFO if needed
    smoothedNoiseLeft.reset(sampleRate, 0.05);
    smoothedNoiseRight.reset(sampleRate, 0.05);
}

void SpatialFX::reset()
{
    delayLineLeft.reset();
    delayLineRight.reset();
    lfoLeft.reset();
    lfoRight.reset();
    smoothedNoiseLeft.reset(sampleRate, 0.05);
    smoothedNoiseRight.reset(sampleRate, 0.05);
}

void SpatialFX::setParams(float leftOffset, float rightOffset, float rate, float depth, float mix, ModShape modShape)
{
    phaseOffsetLeft.setTargetValue(leftOffset);
    phaseOffsetRight.setTargetValue(rightOffset);
    modulationDepth.setTargetValue(depth);
    wetDryMix.setTargetValue(juce::jlimit(0.0f, 1.0f, mix));

    targetRateOrNote = rate;
    modulationShape = modShape;

    updateLfoWaveforms();
    updateLfoFrequencies();
}

void SpatialFX::setSyncEnabled(bool shouldSync)
{
    syncEnabled = shouldSync;
    updateLfoFrequencies();
}

void SpatialFX::setFeedback(float fb)
{
    feedbackAmount.store(juce::jlimit(0.0f, 0.95f, fb));
}

void SpatialFX::setLfoPhaseOffset(float radians)
{
    lfoPhaseOffsetRadians = radians;
    updateLfoWaveforms();
}

void SpatialFX::updateLfoFrequencies()
{
    float rateHz = getEffectiveRateHz();
    lfoLeft.setFrequency(rateHz);
    lfoRight.setFrequency(rateHz);
}

void SpatialFX::updateLfoWaveforms()
{
    constexpr auto twoPi = juce::MathConstants<float>::twoPi;

    switch (modulationShape)
    {
    case ModShape::Sine:
        lfoLeft.initialise([twoPi](float x) { return std::sin(std::fmod(x, twoPi)); });
        lfoRight.initialise([this, twoPi](float x) { return std::sin(std::fmod(x + lfoPhaseOffsetRadians, twoPi)); });
        break;

    case ModShape::Triangle:
        lfoLeft.initialise([twoPi](float x) {
            float wrappedX = std::fmod(x, twoPi);
            return 2.0f * std::abs(2.0f * (wrappedX / twoPi - std::floor(wrappedX / twoPi + 0.5f))) - 1.0f;
            });
        lfoRight.initialise([this, twoPi](float x) {
            float shifted = std::fmod(x + lfoPhaseOffsetRadians, twoPi);
            return 2.0f * std::abs(2.0f * (shifted / twoPi - std::floor(shifted / twoPi + 0.5f))) - 1.0f;
            });
        break;

    case ModShape::Noise:
        // Nothing to initialize, handled in process loop.
        break;
    }
}

void SpatialFX::process(juce::dsp::AudioBlock<float>& block, juce::AudioPlayHead* playHead)
{
    if (playHead != nullptr)
    {
        juce::AudioPlayHead::CurrentPositionInfo info;
        if (playHead->getCurrentPosition(info) && info.isPlaying && info.bpm > 0.0f)
        {
            constexpr float bpmChangeThreshold = 0.1f;
            if (std::abs(currentBpm.load() - info.bpm) > bpmChangeThreshold)
            {
                currentBpm.store(info.bpm);
                updateLfoFrequencies();
            }
        }
    }

    const size_t numSamples = block.getNumSamples();
    const int numChannels = static_cast<int>(block.getNumChannels());

    float* leftChannel = block.getChannelPointer(0);
    float* rightChannel = (numChannels > 1) ? block.getChannelPointer(1) : nullptr;

    float phaseL = phaseOffsetLeft.getNextValue();
    float phaseR = phaseOffsetRight.getNextValue();
    float depth = modulationDepth.getNextValue();
    float mix = wetDryMix.getNextValue();

    const float minDelaySec = minDelaySamples / sampleRate;

    for (size_t i = 0; i < numSamples; ++i)
    {
        float lfoLeftVal = 0.0f;
        float lfoRightVal = 0.0f;

        if (modulationShape == ModShape::Noise)
        {
            if (i % 32 == 0) // Update noise every ~32 samples (~1 ms @ 32 kHz)
            {
                juce::Random random;
                smoothedNoiseLeft.setTargetValue(random.nextFloat() * 2.0f - 1.0f);
                smoothedNoiseRight.setTargetValue(random.nextFloat() * 2.0f - 1.0f);
            }

            lfoLeftVal = smoothedNoiseLeft.getNextValue();
            lfoRightVal = smoothedNoiseRight.getNextValue();
        }
        else
        {
            lfoLeftVal = lfoLeft.processSample(0.0f);
            lfoRightVal = lfoRight.processSample(0.0f);
        }

        const float modDelayL = juce::jlimit(
            minDelaySec,
            maxDelaySeconds,
            phaseL + lfoLeftVal * depth * maxModDepthSeconds
        ) * sampleRate;

        const float modDelayR = juce::jlimit(
            minDelaySec,
            maxDelaySeconds,
            phaseR + lfoRightVal * depth * maxModDepthSeconds
        ) * sampleRate;

        const float dryL = leftChannel[i];
        float wetL = delayLineLeft.popSample(0, modDelayL);
        float fbL = dryL + wetL * feedbackAmount.load();
        delayLineLeft.pushSample(0, fbL);
        leftChannel[i] = dryL * (1.0f - mix) + wetL * mix;

        if (rightChannel != nullptr)
        {
            const float dryR = rightChannel[i];
            float wetR = delayLineRight.popSample(0, modDelayR);
            float fbR = dryR + wetR * feedbackAmount.load();
            delayLineRight.pushSample(0, fbR);
            rightChannel[i] = dryR * (1.0f - mix) + wetR * mix;
        }

        // Update smoothed params each sample
        phaseL = phaseOffsetLeft.getNextValue();
        phaseR = phaseOffsetRight.getNextValue();
        depth = modulationDepth.getNextValue();
        mix = wetDryMix.getNextValue();
    }
}

float SpatialFX::getEffectiveRateHz() const
{
    if ((syncEnabled || (syncParameter && *syncParameter > 0.5f)) && targetRateOrNote > 0.0f)
    {
        const float bpm = currentBpm.load();
        return (bpm / 60.0f) / targetRateOrNote;
    }
    return targetRateOrNote;
}
