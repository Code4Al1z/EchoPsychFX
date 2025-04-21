#pragma once

#include <juce_gui_extra/juce_gui_extra.h>
#include <juce_dsp/juce_dsp.h>

class SpatialFX {
public:
    SpatialFX() = default;
    ~SpatialFX() = default;

    void prepare(const juce::dsp::ProcessSpec& spec);
    void process(juce::dsp::AudioBlock<float>& block);

    // Parameters to control the spatial effects
    void setPhaseOffset(float leftOffset, float rightOffset);
    void setModulationRate(float rateHz);
    void setModulationDepth(float depth);

private:
    double sampleRate = 44100.0;

    // Example: For a simple static phase offset
    float currentPhaseOffsetLeft = 0.0f;
    float currentPhaseOffsetRight = 0.0f;

    // Example: For a modulated phase effect (using a sine oscillator)
    juce::dsp::Oscillator<float> lfoLeft;
    juce::dsp::Oscillator<float> lfoRight;
    float modulationRate = 0.1f;
    float modulationDepth = 0.0f;
    float phaseLeft = 0.0f;
    float phaseRight = juce::MathConstants<float>::pi / 4.0f; // Initial phase difference

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SpatialFX)
};