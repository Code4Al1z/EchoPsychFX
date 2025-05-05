#pragma once
#include <juce_dsp/juce_dsp.h>
#include <random>

class MicroPitchDetune
{
public:
    MicroPitchDetune();
    ~MicroPitchDetune() = default;

    void prepare(const juce::dsp::ProcessSpec& spec);

    void setParams(float detuneCentsIn, float lfoRateIn, float lfoDepthIn,
        float delayCentreIn, float stereoSeparationIn, float mixIn);

    void setSyncEnabled(bool shouldSync);
    void setBpm(float newBpm);

    void process(juce::dsp::AudioBlock<float>& block);

private:
    juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Lagrange3rd> delayL;
    juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Lagrange3rd> delayR;

    float sampleRate = 44100.0f;

    float detuneCents = 5.0f;
    float lfoRate = 0.1f;
    float lfoDepth = 0.002f;
    float delayCentre = 0.005f;
    float stereoSeparation = 0.5f;
    float mix = 0.5f;

    float modPhase = 0.0f;
    float maxDelayTime = 0.02f;

    bool syncEnabled = false;
    float bpm = 120.0f;

    std::mt19937 randomEngine;
    std::uniform_real_distribution<float> randomDistribution;

    float lfoPhaseOffset = 0.0f;

    float lfo(float phase);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MicroPitchDetune)
};
