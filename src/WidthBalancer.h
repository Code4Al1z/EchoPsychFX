#pragma once
#include <juce_dsp/juce_dsp.h>

class WidthBalancer {
public:
    WidthBalancer() = default;
    ~WidthBalancer() = default;

    void prepare(const juce::dsp::ProcessSpec& spec);

    void setWidth(float width);           // 0 = mono, 1 = normal, 2 = wide
    void setMidSideBalance(float balance); // -1 (only side) to 1 (only mid)
    void setMono(bool shouldMono);        // collapse to mono

    void process(juce::dsp::AudioBlock<float>& block);

    void setIntensity(float newIntensity);

    float getWidth() const { return width; }
    float getMidSideBalance() const { return midSideBalance; }
    bool isMono() const { return mono; }

private:
    float width = 1.0f;               // 0 to 2
    float midSideBalance = 0.0f;      // -1 to 1
	float intensity = 1.0f;           // 0 to 1, non-linearly scaled    
    bool mono = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(WidthBalancer)
};
