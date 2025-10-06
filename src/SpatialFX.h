#pragma once
#include <juce_gui_extra/juce_gui_extra.h>
#include <juce_dsp/juce_dsp.h>

class SpatialFX {
public:
    enum class LfoWaveform { Sine = 1, Triangle, Square, Random };

    SpatialFX();
    void prepare(const juce::dsp::ProcessSpec& spec);
    void reset();

    // Phase manipulation (in radians, pi range)
    void setPhaseAmount(float leftPhase, float rightPhase);

    // LFO controls
    void setLfoDepth(float depthL, float depthR); // 0 to pi radians
    void setLfoRate(float rateL, float rateR); // 0 to 20 Hz
    void setLfoWaveform(LfoWaveform waveform);
    void setLfoPhaseOffset(float offset); // 0 to 2pi, phase relationship between L/R
    void setRandomUpdateRate(float hz); // For random waveform, 1-50 Hz

    // Mix and filtering
    void setWetDry(float newWetDry); // 0 to 1
    void setAllpassFrequency(float frequency); // Hz
    void setHaasDelayMs(float leftMs, float rightMs); // 0 to 30ms

    // Processing
    void process(juce::dsp::AudioBlock<float>& block);

    // Getters for UI feedback
    float getCurrentLfoValueL() const { return lastLfoValueL; }
    float getCurrentLfoValueR() const { return lastLfoValueR; }
    LfoWaveform getCurrentWaveform() const { return waveform; }

private:
    struct SpatialParameters {
        juce::LinearSmoothedValue<float> phaseL;
        juce::LinearSmoothedValue<float> phaseR;
        juce::LinearSmoothedValue<float> wetDry;
        juce::LinearSmoothedValue<float> lfoDepthL;
        juce::LinearSmoothedValue<float> lfoDepthR;
        juce::LinearSmoothedValue<float> lfoRateL;
        juce::LinearSmoothedValue<float> lfoRateR;
        juce::LinearSmoothedValue<float> allpassFreq;
        juce::LinearSmoothedValue<float> haasDelayL;
        juce::LinearSmoothedValue<float> haasDelayR;

        void reset(double sampleRate, double smoothingTime) {
            for (auto* p : { &phaseL, &phaseR, &wetDry, &lfoDepthL, &lfoDepthR,
                            &lfoRateL, &lfoRateR, &allpassFreq, &haasDelayL, &haasDelayR }) {
                p->reset(sampleRate, smoothingTime);
                p->setCurrentAndTargetValue(0.0f);
            }
        }
    };

    float sampleRate = 44100.0f;
    SpatialParameters params;

    // LFO state
    float lfoPhaseL = 0.0f;
    float lfoPhaseR = 0.0f;
    float lfoPhaseOffset = 0.0f;
    LfoWaveform waveform = LfoWaveform::Sine;

    // For UI feedback
    float lastLfoValueL = 0.0f;
    float lastLfoValueR = 0.0f;

    // Random LFO sample-and-hold
    float randomValueL = 0.0f;
    float randomValueR = 0.0f;
    float randomSampleCounterL = 0.0f;
    float randomSampleCounterR = 0.0f;
    float randomUpdateRateHz = 10.0f;
    juce::Random random;

    // DSP components
    juce::dsp::IIR::Filter<float> allpassL;
    juce::dsp::IIR::Filter<float> allpassR;
    juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Lagrange3rd> haasDelayL{ 4410 };
    juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Lagrange3rd> haasDelayR{ 4410 };

    // Filter management
    float lastAllpassFreq = -1.0f;
    bool needsFilterUpdate = true;
    static constexpr float filterUpdateThreshold = 0.5f;

    // DC blocking for safety
    juce::dsp::IIR::Filter<float> dcBlockerL;
    juce::dsp::IIR::Filter<float> dcBlockerR;

    // Helper methods
    void updateFilters();
    void updateRandomLfo(bool isLeftChannel, float& counter, float& value);
    float getLfoValue(float phase, bool isLeftChannel);
    float calculateTriangleWave(float phase) const;
    bool isValidWaveform(LfoWaveform wf) const;
    void initializeDCBlockers();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SpatialFX)
};