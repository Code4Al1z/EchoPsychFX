#include "SimpleVerbWithPredelay.h"

SimpleVerbWithPredelay::SimpleVerbWithPredelay()
{
}

void SimpleVerbWithPredelay::prepare(const juce::dsp::ProcessSpec& spec)
{
    sampleRate = spec.sampleRate;
    maxPredelaySamples = (int)(sampleRate * 0.1f); // 100 ms max
    predelayBuffer.setSize((int)spec.numChannels, maxPredelaySamples + 1);
    predelayBuffer.clear();

    juce::dsp::Reverb::Parameters params;
    params.roomSize = 0.5f;
    params.damping = 0.3f;
    params.wetLevel = 1.0f;
    params.dryLevel = 0.0f;
    params.width = 1.0f;
    params.freezeMode = 0.0f;
    reverb.setParameters(params);

    reverb.prepare(spec);
    reset();
}

void SimpleVerbWithPredelay::reset()
{
    predelayBuffer.clear();
    reverb.reset();
    predelayWritePos = 0;
}

void SimpleVerbWithPredelay::setParams(float predelayMs, float size, float damping, float wet)
{
    currentPredelayMs = juce::jlimit(0.0f, 100.0f, predelayMs);
    wetLevel = juce::jlimit(0.0f, 1.0f, wet);

    auto p = reverb.getParameters();
    p.roomSize = juce::jlimit(0.0f, 1.0f, size);
    p.damping = juce::jlimit(0.0f, 1.0f, damping);
    reverb.setParameters(p);
}

void SimpleVerbWithPredelay::process(juce::dsp::AudioBlock<float>& block)
{
    const int numChannels = block.getNumChannels();
    const int numSamples = block.getNumSamples();
    int predelaySamples = (int)(sampleRate * (currentPredelayMs / 1000.0f));

    // Copy input to predelay buffer
    for (int ch = 0; ch < numChannels; ++ch)
    {
        float* channelData = block.getChannelPointer(ch);
        float* delayData = predelayBuffer.getWritePointer(ch);

        for (int i = 0; i < numSamples; ++i)
        {
            int delayPos = (predelayWritePos + i) % maxPredelaySamples;
            delayData[delayPos] = channelData[i];
        }
    }

    // Prepare delayed audio block
    juce::AudioBuffer<float> delayedBlock(numChannels, numSamples);
    for (int ch = 0; ch < numChannels; ++ch)
    {
        const float* delayData = predelayBuffer.getReadPointer(ch);
        float* dest = delayedBlock.getWritePointer(ch);

        for (int i = 0; i < numSamples; ++i)
        {
            int readPos = (predelayWritePos + i - predelaySamples + maxPredelaySamples) % maxPredelaySamples;
            dest[i] = delayData[readPos];
        }
    }

    // Advance write head
    predelayWritePos = (predelayWritePos + numSamples) % maxPredelaySamples;

    // Apply reverb to delayed signal
    juce::dsp::AudioBlock<float> reverbInput(delayedBlock);
    reverb.process(juce::dsp::ProcessContextReplacing<float>(reverbInput));

    // Mix reverb result with dry signal
    for (int ch = 0; ch < numChannels; ++ch)
    {
        float* wet = reverbInput.getChannelPointer(ch);
        float* dry = block.getChannelPointer(ch);

        for (int i = 0; i < numSamples; ++i)
            dry[i] = dry[i] * (1.0f - wetLevel) + wet[i] * wetLevel;
    }
}
