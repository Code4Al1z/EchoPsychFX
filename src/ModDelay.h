#pragma once
#include <juce_core/juce_core.h>
#include <juce_dsp/juce_dsp.h>

class ModDelay {
public:

    enum class ModulationType
    {
        Sine = 1,
        Triangle,
        Square,
        SawtoothUp,
        SawtoothDown
        // Add more waveforms as needed
    };


    ModDelay() = default;
    ~ModDelay() = default;

    void prepare(const juce::dsp::ProcessSpec& spec);
    void setParams(float delayMs, float depth, float rateHzOrNoteDiv, float feedbackL, float feedbackR, float mix);
    void process(juce::dsp::AudioBlock<float>& block);

    void setModulationType(ModulationType newType);
    ModulationType getModulationType() const { return currentModulationType; }

    void setSyncEnabled(bool shouldSync);
    void setTempo(float newBpm);

private:
    juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Lagrange3rd> delayL;
    juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Lagrange3rd> delayR;

    float sampleRate = 44100.0f;
    float phase = 0.0f;
    ModulationType currentModulationType = ModulationType::Sine;
    ModulationType targetModulationType = ModulationType::Sine;
    juce::LinearSmoothedValue<float> modulationTypeCrossfade; // For smoother waveform transitions

    float bpm = 120.0f;
    bool syncEnabled = false;
    float rawRate = 1.0f; // rate param as Hz or note division (depending on sync)

    juce::LinearSmoothedValue<float> targetDelayMs;
    juce::LinearSmoothedValue<float> currentDelayMs;
    juce::LinearSmoothedValue<float> targetModDepth;
    juce::LinearSmoothedValue<float> currentModDepth;
    juce::LinearSmoothedValue<float> targetModRateHz;
    juce::LinearSmoothedValue<float> currentModRateHz;
    juce::LinearSmoothedValue<float> targetFeedbackL;
    juce::LinearSmoothedValue<float> currentFeedbackL;
    juce::LinearSmoothedValue<float> targetFeedbackR;
    juce::LinearSmoothedValue<float> currentFeedbackR;
    juce::LinearSmoothedValue<float> targetMix;
    juce::LinearSmoothedValue<float> currentMix;

    float calculateModulation(float phaseNorm, float depth, ModulationType type);
    float getEffectiveRateHz() const;
    void updateEffectiveRate();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ModDelay)
};