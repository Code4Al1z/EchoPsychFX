#ifndef ECHOPSYCHFX_EXCITERSATURATIONCOMPONENT_H_INCLUDED
#define ECHOPSYCHFX_EXCITERSATURATIONCOMPONENT_H_INCLUDED

#include <juce_gui_extra/juce_gui_extra.h>
#include <juce_audio_processors/juce_audio_processors.h>

class ExciterSaturationComponent : public juce::Component
{
public:
    ExciterSaturationComponent(juce::AudioProcessorValueTreeState& state);
    ~ExciterSaturationComponent() override;

    void paint(juce::Graphics& g) override;
    void resized() override;

    void setDrive(float newValue);
    void setMix(float newValue);
    void setHighpass(float newValue);

private:
    juce::GroupComponent group{ "exciterSaturationGroup", "Exciter Saturation" };

    static constexpr int knobSize = 100;
    static constexpr int margin = 10;
    static constexpr int labelHeight = 20;

    struct KnobWithLabel {
        juce::Slider slider;
        juce::TextEditor label;
        std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attachment;
    };

    KnobWithLabel drive, mix, highpass;

    void configureKnob(KnobWithLabel& kwl, const juce::String& paramID, const juce::String& labelText, juce::AudioProcessorValueTreeState& state);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ExciterSaturationComponent)
};

#endif // ECHOPSYCHFX_EXCITERSATURATIONCOMPONENT_H_INCLUDED
