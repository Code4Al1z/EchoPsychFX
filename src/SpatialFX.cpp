#include "SpatialFX.h"

SpatialFX::SpatialFX()
{
    lfoLeft.initialise([](float x) { return std::sin(x); });
    lfoRight.initialise([](float x) { return std::sin(x); });
}

void SpatialFX::prepare(const juce::dsp::ProcessSpec& spec)
{
    sampleRate = spec.sampleRate;

    delayLineLeft.reset();
    delayLineRight.reset();

    delayLineLeft.prepare(spec);
    delayLineRight.prepare(spec);

    delayLineLeft.setMaximumDelayInSamples(maxDelaySamples);
    delayLineRight.setMaximumDelayInSamples(maxDelaySamples);

    lfoLeft.prepare(spec);
    lfoRight.prepare(spec);

    phaseOffsetLeft.reset(sampleRate, smoothingTime);
    phaseOffsetRight.reset(sampleRate, smoothingTime);
    modulationDepth.reset(sampleRate, smoothingTime);
    wetDryMix.reset(sampleRate, smoothingTime);

    currentBpm.store(120.0f); // default initial BPM
}

void SpatialFX::reset()
{
    delayLineLeft.reset();
    delayLineRight.reset();
    lfoLeft.reset();
    lfoRight.reset();
}

void SpatialFX::setPhaseOffsetLeft(float seconds)
{
    phaseOffsetLeft.setTargetValue(seconds);
}

void SpatialFX::setPhaseOffsetRight(float seconds)
{
    phaseOffsetRight.setTargetValue(seconds);
}

void SpatialFX::setModulationRate(float rateOrNote)
{
    targetRateOrNote = rateOrNote;

    updateLfoFrequencies();
}

void SpatialFX::updateLfoFrequencies()
{
    const bool sync = (syncParameter != nullptr) ? (*syncParameter > 0.5f) : false;

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

void SpatialFX::setModulationDepth(float seconds)
{
    modulationDepth.setTargetValue(seconds);
}

void SpatialFX::setWetDryMix(float mix)
{
    wetDryMix.setTargetValue(juce::jlimit(0.0f, 1.0f, mix));
}

void SpatialFX::process(juce::dsp::AudioBlock<float>& block, juce::AudioPlayHead* playHead)
{
    if (playHead != nullptr)
    {
        juce::AudioPlayHead::CurrentPositionInfo info;
        if (playHead->getCurrentPosition(info) && info.isPlaying)
        {
            if (info.bpm > 0.0f)
            {
                if (std::abs(currentBpm.load() - info.bpm) > 0.01f)
                {
                    currentBpm.store(info.bpm);
                    updateLfoFrequencies();
                }
            }
        }
    }

    const auto numSamples = block.getNumSamples();
    const auto numChannels = block.getNumChannels();

    for (size_t i = 0; i < numSamples; ++i)
    {
        // Update smoothed parameters once per sample
        const float phaseL = phaseOffsetLeft.getNextValue();
        const float phaseR = phaseOffsetRight.getNextValue();
        const float depth = modulationDepth.getNextValue();
        const float mix = wetDryMix.getNextValue();

        const float lfoValueLeft = lfoLeft.processSample(0.0f);
        const float lfoValueRight = lfoRight.processSample(0.0f);

        const float modulatedDelayL = juce::jlimit(0.0f, maxDelaySeconds, phaseL + depth * lfoValueLeft) * sampleRate;
        const float modulatedDelayR = juce::jlimit(0.0f, maxDelaySeconds, phaseR + depth * lfoValueRight) * sampleRate;

        for (size_t ch = 0; ch < numChannels; ++ch)
        {
            auto* data = block.getChannelPointer(ch);

            const float drySample = data[i];

            if (ch == 0)
            {
                delayLineLeft.pushSample(0, drySample);
                const float delayedSample = delayLineLeft.popSample(0, modulatedDelayL);
                data[i] = drySample * (1.0f - mix) + delayedSample * mix;
            }
            else
            {
                delayLineRight.pushSample(0, drySample);
                const float delayedSample = delayLineRight.popSample(0, modulatedDelayR);
                data[i] = drySample * (1.0f - mix) + delayedSample * mix;
            }
        }
    }
}
