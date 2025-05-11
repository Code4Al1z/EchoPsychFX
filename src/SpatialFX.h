#pragma once

#include <juce_gui_extra/juce_gui_extra.h>
#include <juce_dsp/juce_dsp.h>

class SpatialFX {
public:
public:
    enum class LfoWaveform { Sine = 1, Triangle, Square, Random };

    SpatialFX();
    void prepare(const juce::dsp::ProcessSpec& spec);
    void reset();

    void setPhaseAmount(float leftPhase, float rightPhase); // in radians
    void setLfoDepth(float depthL, float depthR);
    void setLfoRate(float rateL, float rateR);
    void setLfoWaveform(LfoWaveform waveform);
    void setLfoPhaseOffset(float offset); // 0 to 2*pi
    void setWetDry(float newWetDry);
    void setAllpassFrequency(float frequency);
    void setHaasDelayMs(float leftMs, float rightMs);

    void process(juce::dsp::AudioBlock<float>& block);

private:
    float sampleRate = 44100.0f;

    juce::SmoothedValue<float> targetPhaseL;
    juce::SmoothedValue<float> targetPhaseR;

    juce::SmoothedValue<float> wetDry;
    juce::SmoothedValue<float> lfoDepthL;
    juce::SmoothedValue<float> lfoDepthR;
    juce::SmoothedValue<float> lfoRateL;
    juce::SmoothedValue<float> lfoRateR;
    juce::SmoothedValue<float> allpassFrequency;
    juce::SmoothedValue<float> haasMsL;
    juce::SmoothedValue<float> haasMsR;

    float lfoPhaseL = 0.0f;
    float lfoPhaseR = 0.0f;
    float lfoPhaseOffset = 0.0f;

    LfoWaveform waveform = LfoWaveform::Sine;

    juce::dsp::IIR::Filter<float> allpassL;
    juce::dsp::IIR::Filter<float> allpassR;

    juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Lagrange3rd> haasDelayL{ 4410 };
    juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Lagrange3rd> haasDelayR{ 4410 };

    void updateFilters();
    void updateLfo();
    float getLfoValue(float phase);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SpatialFX)
};