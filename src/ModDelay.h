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
    ModulationType getModulationType() const { return modulationType; }

    void setSyncEnabled(bool shouldSync);
    void setTempo(float newBpm);

private:
    juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Lagrange3rd> delayL;
    juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Lagrange3rd> delayR;

    float sampleRate = 44100.0f;
    float phase = 0.0f;

    float bpm = 120.0f;
    bool syncEnabled = false;
    float rawRate = 1.0f; // rate param as Hz or note division (depending on sync)

    juce::LinearSmoothedValue<float> delayMs;
    juce::LinearSmoothedValue<float> modDepth;
    juce::LinearSmoothedValue<float> modRateHz;
    juce::LinearSmoothedValue<float> feedbackL;
    juce::LinearSmoothedValue<float> feedbackR;
    juce::LinearSmoothedValue<float> mix;

    ModulationType modulationType = ModulationType::Sine;

    float calculateModulation(float phaseNorm, float depth);
    float getEffectiveRateHz() const;

    void updateEffectiveRate();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ModDelay)
};