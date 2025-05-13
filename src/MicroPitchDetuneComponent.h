#ifndef ECHOPSYCHFX_MICROPITCHDETUNECOMPONENT_H_INCLUDED
#define ECHOPSYCHFX_MICROPITCHDETUNECOMPONENT_H_INCLUDED

#include <juce_gui_extra/juce_gui_extra.h>
#include <juce_audio_processors/juce_audio_processors.h>

class MicroPitchDetuneComponent : public juce::Component
{
public:
    MicroPitchDetuneComponent(juce::AudioProcessorValueTreeState& state);
    ~MicroPitchDetuneComponent() override;

    void paint(juce::Graphics& g) override;
    void resized() override;

    void setMix(float newValue);
    void setLfoRate(float newValue);
    void setLfoDepth(float newValue);
    void setDelayCentre(float newValue);
    void setDetuneAmount(float newValue);
    void setStereoSeparation(float newValue);

private:
    juce::GroupComponent group{ "microPitchDetuneGroup", "Micro-Pitch Detune" };

    static constexpr int knobSize = 100;
    static constexpr int margin = 10;
    static constexpr int labelHeight = 20;

    struct KnobWithLabel {
        juce::Slider slider;
        juce::TextEditor label;
        std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attachment;
    };

    KnobWithLabel mix, lfoRate, lfoDepth, delayCentre, detuneAmount, stereoSeparation;

    void configureKnob(KnobWithLabel& kwl, const juce::String& paramID, const juce::String& labelText, juce::AudioProcessorValueTreeState& state);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MicroPitchDetuneComponent)
};

#endif // ECHOPSYCHFX_MICROPITCHDETUNECOMPONENT_H_INCLUDED
