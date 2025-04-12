#pragma once
#include <juce_dsp/juce_dsp.h>

class ModDelay {
public:
	ModDelay() = default;
	~ModDelay() = default;

    void prepare(const juce::dsp::ProcessSpec& spec);
    void setParams(float delayMs, float modDepth, float modRateHz);

    void process(juce::dsp::AudioBlock<float>& block);

    float getDelayTimeMs() const { return delayMs; }

private:
    juce::dsp::DelayLine<float> delayL{ 44100 }, delayR{ 44100 };
    float sampleRate = 44100.0f;
    float delayMs = 10.0f;
    float modDepth = 0.5f;
    float modRateHz = 0.3f;
    float phase = 0.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ModDelay)
};