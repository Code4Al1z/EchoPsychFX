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

    // Mix and filtering
    void setWetDry(float newWetDry); // 0 to 1
    void setAllpassFrequency(float frequency); // Hz
    void setHaasDelayMs(float leftMs, float rightMs); // 0 to 40ms

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

    // Random LFO: a new random value once per LFO cycle (so the Rate controls set its speed), reached
    // through a short glide rather than a step, so the phase rotation never jumps and clicks.
    struct RandomLfoState
    {
        float current = 0.0f;       // value being output
        float target = 0.0f;        // value being glided to
        float step = 0.0f;          // change per sample while gliding
        int glideSamplesLeft = 0;
        float lastPhase = 0.0f;     // to spot the phase wrapping round, which starts a new cycle
        bool initialised = false;
    };
    RandomLfoState randomL, randomR;
    float currentRateL = 1.0f;
    float currentRateR = 1.0f;
    juce::Random random;

    // DSP components
    juce::dsp::IIR::Filter<float> allpassL;
    juce::dsp::IIR::Filter<float> allpassR;
    juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Lagrange3rd> haasDelayL;
    juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Lagrange3rd> haasDelayR;

    // Filter management
    float lastAllpassFreq = -1.0f;
    bool needsFilterUpdate = true;
    static constexpr float filterUpdateThreshold = 0.5f;

    // DC blocking for safety
    juce::dsp::IIR::Filter<float> dcBlockerL;
    juce::dsp::IIR::Filter<float> dcBlockerR;

    // Helper methods
    void updateFilters();
    float nextRandomLfoValue(RandomLfoState& state, float phase, float rateHz);
    float getLfoValue(float phase, bool isLeftChannel);
    float calculateTriangleWave(float phase) const;
    bool isValidWaveform(LfoWaveform wf) const;
    void initializeDCBlockers();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SpatialFX)
};