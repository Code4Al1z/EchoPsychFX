#pragma once

#include "juce_dsp/juce_dsp.h"

class SimpleVerbWithPredelay
{
public:
    SimpleVerbWithPredelay();
    void prepare(const juce::dsp::ProcessSpec& spec);
    void reset();

    void setParams(float predelayMs, float size, float damping, float wet);

    float interpolate(float fraction, float y0, float y1);

    void process(juce::dsp::AudioBlock<float>& block);

private:
    juce::dsp::Reverb reverb;
    juce::AudioBuffer<float> predelayBuffer;
    int predelayWritePos = 0;
    int maxPredelaySamples = 0;
    float targetPredelaySamples = 0.0f;
    float currentPredelaySamples = 0.0f;

    float targetWetLevel = 0.3f;
    float currentWetLevel = 0.3f;
    float sampleRate = 44100.0f;
    static constexpr float predelaySmoothingCoeff = 0.005f; // Smoothing factor for predelay
    static constexpr float wetSmoothingCoeff = 0.02f;       // Smoothing factor for wet level
};