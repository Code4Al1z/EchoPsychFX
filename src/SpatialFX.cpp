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

void SpatialFX::setModulationRate(float hz)
{
    lfoLeft.setFrequency(hz);
    lfoRight.setFrequency(hz);
}

void SpatialFX::setModulationDepth(float seconds)
{
    modulationDepth.setTargetValue(seconds);
}

void SpatialFX::setWetDryMix(float mix)
{
    wetDryMix.setTargetValue(juce::jlimit(0.0f, 1.0f, mix));
}

void SpatialFX::process(juce::dsp::AudioBlock<float>& block)
{
    auto numSamples = block.getNumSamples();

    for (size_t ch = 0; ch < block.getNumChannels(); ++ch)
    {
        auto* data = block.getChannelPointer(ch);

        for (size_t i = 0; i < numSamples; ++i)
        {
            float delaySeconds = (ch == 0 ? phaseOffsetLeft.getNextValue() : phaseOffsetRight.getNextValue())
                + modulationDepth.getNextValue() * (ch == 0 ? lfoLeft.processSample(0.0f) : lfoRight.processSample(0.0f));

            float delaySamples = juce::jlimit(0.0f, maxDelaySeconds, delaySeconds) * sampleRate;

            float delayedSample = (ch == 0)
                ? delayLineLeft.popSample(0, delaySamples)
                : delayLineRight.popSample(0, delaySamples);

            float drySample = data[i];
            float wet = delayedSample;
            float dry = drySample;

            float mixed = dry * (1.0f - wetDryMix.getNextValue()) + wet * wetDryMix.getNextValue();

            if (ch == 0)
                delayLineLeft.pushSample(0, drySample);
            else
                delayLineRight.pushSample(0, drySample);

            data[i] = mixed;
        }
    }
}
