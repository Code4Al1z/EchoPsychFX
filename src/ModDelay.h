#pragma once
#include <juce_dsp/juce_dsp.h>

class ModDelay {
public:
    ModDelay() = default;
    ~ModDelay() = default;

    void prepare(const juce::dsp::ProcessSpec& spec);
    void setParams(float delayMs, float modDepth, float modRateHz, float feedback, float mix);
    void process(juce::dsp::AudioBlock<float>& block);

private:
    juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Linear> delayL;
    juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Linear> delayR;

    float sampleRate = 44100.0f;
    float phase = 0.0f;

    float smoothedModRateHz = 0.0f; // <-- for gentle rate smoothing

    juce::LinearSmoothedValue<float> delayMs;
    juce::LinearSmoothedValue<float> modDepth;
    juce::LinearSmoothedValue<float> modRateHz;
    juce::LinearSmoothedValue<float> feedback;
    juce::LinearSmoothedValue<float> mix;


    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ModDelay)
};
