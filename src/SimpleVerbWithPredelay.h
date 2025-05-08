#pragma once

#include "juce_dsp/juce_dsp.h"

class SimpleVerbWithPredelay
{
public:
    SimpleVerbWithPredelay();
    void prepare(const juce::dsp::ProcessSpec& spec);
    void reset();

	void setParams(float predelayMs, float size, float damping, float wet);

    void process(juce::dsp::AudioBlock<float>& block);

private:
    juce::dsp::Reverb reverb;
    juce::AudioBuffer<float> predelayBuffer;
    int predelayWritePos = 0;
    int maxPredelaySamples = 0;
    float currentPredelayMs = 0.0f;

    float wetLevel = 0.3f;
    float sampleRate = 44100.0f;
};

