#include "SpatialFX.h"

SpatialFX::SpatialFX()
{
    lfoLeft.initialise([](float x) { return std::sin(x); });
    lfoRight.initialise([](float x) { return std::sin(x); });
}

void SpatialFX::prepare(const juce::dsp::ProcessSpec& spec)
{
    sampleRate = spec.sampleRate;

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

    currentBpm.store(120.0f); // default BPM
}

void SpatialFX::reset()
{
    delayLineLeft.reset();
    delayLineRight.reset();
    lfoLeft.reset();
    lfoRight.reset();
}

void SpatialFX::setParams(float leftOffset, float rightOffset, float rate, float depth, float mix)
{
	phaseOffsetLeft.setTargetValue(leftOffset);
	phaseOffsetRight.setTargetValue(rightOffset);
	targetRateOrNote = rate;
	modulationDepth.setTargetValue(depth);
	wetDryMix.setTargetValue(juce::jlimit(0.0f, 1.0f, mix));
	updateLfoFrequencies();
}

void SpatialFX::updateLfoFrequencies()
{
    const bool sync = syncEnabled || ((syncParameter != nullptr) && (*syncParameter > 0.5f));

    if (sync)
    {
        const float bpm = currentBpm.load();
        const float beatsPerSecond = bpm / 60.0f;
        const float syncedFrequency = beatsPerSecond / targetRateOrNote;

        lfoLeft.setFrequency(syncedFrequency);
        lfoRight.setFrequency(syncedFrequency);
    }
    else
    {
        lfoLeft.setFrequency(targetRateOrNote);
        lfoRight.setFrequency(targetRateOrNote);
    }
}

void SpatialFX::setSyncEnabled(bool shouldSync)
{
    syncEnabled = shouldSync;
    updateLfoFrequencies();
}

void SpatialFX::process(juce::dsp::AudioBlock<float>& block, juce::AudioPlayHead* playHead)
{
    if (playHead != nullptr)
    {
        juce::AudioPlayHead::CurrentPositionInfo info;
        if (playHead->getCurrentPosition(info) && info.isPlaying && info.bpm > 0.0f)
        {
            if (std::abs(currentBpm.load() - info.bpm) > 0.01f)
            {
                currentBpm.store(info.bpm);
                updateLfoFrequencies();
            }
        }
    }

    const auto numSamples = block.getNumSamples();
    const auto numChannels = block.getNumChannels();

    float* leftChannel = block.getChannelPointer(0);
    float* rightChannel = (numChannels > 1) ? block.getChannelPointer(1) : nullptr;

    for (size_t i = 0; i < numSamples; ++i)
    {
        const float phaseL = phaseOffsetLeft.getNextValue();
        const float phaseR = phaseOffsetRight.getNextValue();
        const float depth = modulationDepth.getNextValue();
        const float mix = wetDryMix.getNextValue();

        const float lfoValueLeft = lfoLeft.processSample(0.0f);
        const float lfoValueRight = lfoRight.processSample(0.0f);

        const float modulatedDelayL = juce::jlimit(0.0f, maxDelaySeconds, phaseL + depth * lfoValueLeft) * sampleRate;
        const float modulatedDelayR = juce::jlimit(0.0f, maxDelaySeconds, phaseR + depth * lfoValueRight) * sampleRate;

        const float dryL = leftChannel[i];
        const float delayedL = delayLineLeft.popSample(0, modulatedDelayL);
        delayLineLeft.pushSample(0, dryL);
        leftChannel[i] = dryL * (1.0f - mix) + delayedL * mix;

        if (rightChannel != nullptr)
        {
            const float dryR = rightChannel[i];
            const float delayedR = delayLineRight.popSample(0, modulatedDelayR);
            delayLineRight.pushSample(0, dryR);
            rightChannel[i] = dryR * (1.0f - mix) + delayedR * mix;
        }
    }
}
