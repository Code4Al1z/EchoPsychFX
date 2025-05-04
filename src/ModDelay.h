#pragma once
#include <juce_core/juce_core.h>
#include <juce_dsp/juce_dsp.h>

class ModDelay {
public:

    enum class ModulationType
    {
        Sine,
        Triangle,
        Square,
        SawtoothUp,
        SawtoothDown
        // Add more waveforms as needed
    };


    ModDelay() : modulationType(ModulationType::Sine) {}
    ~ModDelay() = default;

    void prepare(const juce::dsp::ProcessSpec& spec);
    void setParams(float delayMs, float depth, float rateHz, float feedbackL, float feedbackR, float mix);
    void process(juce::dsp::AudioBlock<float>& block);
    void setModulationType(ModulationType newType);
    ModulationType getModulationType() const { return modulationType; }

	ModulationType modulationType = ModulationType::Sine;

private:
    juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Linear> delayL;
    juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Linear> delayR;

    float sampleRate = 44100.0f;
    float phase = 0.0f;

    float smoothedModRateHz = 0.0f; // <-- for gentle rate smoothing

    juce::LinearSmoothedValue<float> delayMs;
    juce::LinearSmoothedValue<float> modDepth;
    juce::LinearSmoothedValue<float> modRateHz;
    juce::LinearSmoothedValue<float> feedbackL;
    juce::LinearSmoothedValue<float> feedbackR;
    juce::LinearSmoothedValue<float> mix;

    float calculateModulation(float rateHz, float depth, float currentPhase);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ModDelay)
};