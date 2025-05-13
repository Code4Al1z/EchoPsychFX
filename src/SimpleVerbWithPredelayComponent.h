#ifndef ECHOPSYCHFX_SIMPLEVERBWITHPREDELAYCOMPONENT_H_INCLUDED
#define ECHOPSYCHFX_SIMPLEVERBWITHPREDELAYCOMPONENT_H_INCLUDED

#include <juce_gui_extra/juce_gui_extra.h>
#include <juce_audio_processors/juce_audio_processors.h>

class SimpleVerbWithPredelayComponent : public juce::Component
{
public:
    SimpleVerbWithPredelayComponent(juce::AudioProcessorValueTreeState& state);
    ~SimpleVerbWithPredelayComponent() override;

    void paint(juce::Graphics& g) override;
    void resized() override;

    void setPredelay(float newValue);
    void setSize(float newValue);
    void setDamping(float newValue);
    void setWet(float newValue);

private:
    juce::GroupComponent group{ "simpleVerbWithPredelayGroup", "Simple Verb With Predelay" };

    static constexpr int knobSize = 100;
    static constexpr int margin = 10;
    static constexpr int labelHeight = 20;

    struct KnobWithLabel {
        juce::Slider slider;
        juce::TextEditor label;
        std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attachment;
    };

    KnobWithLabel predelay, size, damping, wet;

    void configureKnob(KnobWithLabel& kwl, const juce::String& paramID, const juce::String& labelText, juce::AudioProcessorValueTreeState& state);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SimpleVerbWithPredelayComponent)
};

#endif // ECHOPSYCHFX_SIMPLEVERBWITHPREDELAYCOMPONENT_H_INCLUDED
