#pragma once
#include "juce_dsp/juce_dsp.h"

class TiltEQ {
public:
    TiltEQ() = default;
    ~TiltEQ() = default;

    void prepare(const juce::dsp::ProcessSpec& spec);
    void setTilt(float tiltAmount); // -1 = warm, +1 = bright
    void process(juce::dsp::AudioBlock<float>& block);
    float getTilt() const { return tilt; }

private:
    using Filter = juce::dsp::IIR::Filter<float>;
    using Coefficients = juce::dsp::IIR::Coefficients<float>;
    using FilterChain = juce::dsp::ProcessorDuplicator<Filter, Coefficients>;

    FilterChain lowShelf, highShelf;
    float tilt = 0.0f;
    double sampleRate = 44100.0;
    bool needsUpdate = true;

    void updateFilters();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TiltEQ)
};
