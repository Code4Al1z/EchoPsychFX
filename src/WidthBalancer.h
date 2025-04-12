#pragma once
#include <juce_dsp/juce_dsp.h>

class WidthBalancer {
public:
	WidthBalancer() = default;
	~WidthBalancer() = default;
    void prepare(const juce::dsp::ProcessSpec& spec);

    void setWidth(float width);       // 0 = mono, 1 = normal, 2 = extra wide
    void setMidGain(float gainDb);    // -12 to +12 dB
    void setSideGain(float gainDb);   // -12 to +12 dB

    void process(juce::dsp::AudioBlock<float>& block);

    float getWidth() const { return width; }

private:
    float width = 1.0f;
    float midGain = 0.0f;
    float sideGain = 0.0f;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(WidthBalancer)
};