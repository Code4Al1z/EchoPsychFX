#pragma once
#include <juce_core/juce_core.h>
#include <juce_dsp/juce_dsp.h>

class ModDelay {
public:
    enum class ModulationType {
        Sine = 1,
        Triangle,
        Square,
        SawtoothUp,
        SawtoothDown
    };

    ModDelay() = default;
    ~ModDelay() = default;

    void prepare(const juce::dsp::ProcessSpec& spec);
    void resetState();
    void setParams(float delayMs, float depth, float rateHzOrNoteDiv, float feedbackL, float feedbackR, float mix);
    void process(juce::dsp::AudioBlock<float>& block);
    void setModulationType(ModulationType newType);
    ModulationType getModulationType() const { return currentModulationType; }
    void setSyncEnabled(bool shouldSync);
    void setTempo(float newBpm);

private:
    struct ModDelayParameters {
        juce::LinearSmoothedValue<float> delayMs;
        juce::LinearSmoothedValue<float> modDepth;
        juce::LinearSmoothedValue<float> modRateHz;
        juce::LinearSmoothedValue<float> feedbackL;
        juce::LinearSmoothedValue<float> feedbackR;
        juce::LinearSmoothedValue<float> mix;

        void reset(double sampleRate, double smoothingTime) {
            for (auto* p : { &delayMs, &modDepth, &modRateHz, &feedbackL, &feedbackR, &mix }) {
                p->reset(sampleRate, smoothingTime);
                p->setCurrentAndTargetValue(0.0f);
            }
        }
    };

    juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Lagrange3rd> delayL;
    juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Lagrange3rd> delayR;

    float sampleRate = 44100.0f;
    float phase = 0.0f;
    ModulationType currentModulationType = ModulationType::Sine;
    ModulationType targetModulationType = ModulationType::Sine;
    juce::LinearSmoothedValue<float> modulationTypeCrossfade;

    float bpm = 120.0f;
    bool syncEnabled = false;
    float rawRate = 1.0f;

    ModDelayParameters params;

    float calculateModulation(float phaseNorm, float depth, ModulationType type);
    float getEffectiveRateHz() const;
    void updateEffectiveRate();
    bool isValidModulationType(ModulationType type) const;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ModDelay)
};