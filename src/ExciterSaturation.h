#pragma once

#include <juce_dsp/juce_dsp.h>
#include <juce_gui_extra/juce_gui_extra.h>

class ExciterSaturation
{
public:
    ExciterSaturation();

    void prepare(const juce::dsp::ProcessSpec& spec);
    void reset();

    void setDrive(float newDrive);       // 0.0 to 1.0
    void setMix(float newMix);           // 0.0 (dry) to 1.0 (wet)
    void setHighpass(float freqHz);      // Apply saturation above this freq

    void process(juce::dsp::AudioBlock<float>& block);

private:
    float drive = 0.5f;
    float mix = 0.5f;
    float highpassFreq = 1000.0f;

    juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>,
        juce::dsp::IIR::Coefficients<float>> highpass;

    float waveshape(float x);  // The saturation curve

    juce::AudioBuffer<float> dryBuffer;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ExciterSaturation)
};
