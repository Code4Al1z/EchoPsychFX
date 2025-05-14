#include "SimpleVerbWithPredelay.h"
#include "SimpleVerbWithPredelay.h"

SimpleVerbWithPredelay::SimpleVerbWithPredelay()
{
}

void SimpleVerbWithPredelay::prepare(const juce::dsp::ProcessSpec& spec)
{
    sampleRate = spec.sampleRate;
    maxPredelaySamples = (int)(sampleRate * 0.2f); // Increased max predelay to 200 ms for smoother transitions
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
    currentPredelaySamples = 0.0f;
    targetPredelaySamples = 0.0f;
    currentWetLevel = targetWetLevel;
}

void SimpleVerbWithPredelay::setParams(float predelayMs, float size, float damping, float wet)
{
    targetPredelaySamples = juce::jlimit(0.0f, (float)maxPredelaySamples, sampleRate * (predelayMs / 1000.0f));
    targetWetLevel = juce::jlimit(0.0f, 1.0f, wet);

    auto p = reverb.getParameters();
    p.roomSize = juce::jlimit(0.0f, 1.0f, size);
    p.damping = juce::jlimit(0.0f, 1.0f, damping);
    reverb.setParameters(p);
}

float SimpleVerbWithPredelay::interpolate(float fraction, float y0, float y1)
{
    return y0 + fraction * (y1 - y0);
}

void SimpleVerbWithPredelay::process(juce::dsp::AudioBlock<float>& block)
{
    const int numChannels = block.getNumChannels();
    const int numSamples = block.getNumSamples();

    // Smooth the predelay and wet level changes
    currentPredelaySamples = currentPredelaySamples + (targetPredelaySamples - currentPredelaySamples) * predelaySmoothingCoeff;
    currentWetLevel = currentWetLevel + (targetWetLevel - currentWetLevel) * wetSmoothingCoeff;
    int predelaySamplesInt = static_cast<int>(currentPredelaySamples);
    float predelayFraction = currentPredelaySamples - predelaySamplesInt;

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

    // Prepare delayed audio block using linear interpolation for smoother delay changes
    juce::AudioBuffer<float> delayedBlock(numChannels, numSamples);
    for (int ch = 0; ch < numChannels; ++ch)
    {
        const float* delayData = predelayBuffer.getReadPointer(ch);
        float* dest = delayedBlock.getWritePointer(ch);

        for (int i = 0; i < numSamples; ++i)
        {
            float readPosFloat = static_cast<float>(predelayWritePos + i - predelaySamplesInt + maxPredelaySamples);
            readPosFloat = std::fmod(readPosFloat, static_cast<float>(maxPredelaySamples));
            int readPosInt = static_cast<int>(readPosFloat);
            int nextReadPosInt = (readPosInt + 1) % maxPredelaySamples;

            float sample1 = delayData[readPosInt];
            float sample2 = delayData[nextReadPosInt];
            dest[i] = interpolate(predelayFraction, sample1, sample2);
        }
    }

    // Advance write head
    predelayWritePos = (predelayWritePos + numSamples) % maxPredelaySamples;

    // Apply reverb to delayed signal
    juce::dsp::AudioBlock<float> reverbInput(delayedBlock);
    reverb.process(juce::dsp::ProcessContextReplacing<float>(reverbInput));

    // Mix reverb result with dry signal with proper gain control
    for (int ch = 0; ch < numChannels; ++ch)
    {
        float* wetBuffer = reverbInput.getChannelPointer(ch);
        float* dryBuffer = block.getChannelPointer(ch);

        for (int i = 0; i < numSamples; ++i)
        {
            dryBuffer[i] = dryBuffer[i] * (1.0f - currentWetLevel);
            wetBuffer[i] = wetBuffer[i] * currentWetLevel;
            dryBuffer[i] += wetBuffer[i]; // Add the wet signal to the dry signal
        }
    }
}