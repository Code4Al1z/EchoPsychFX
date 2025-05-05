#ifndef ECHOPSYCHFX_MODDELAYCOMPONENT_H_INCLUDED
#define ECHOPSYCHFX_MODDELAYCOMPONENT_H_INCLUDED

#include <juce_gui_extra/juce_gui_extra.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include "ModDelay.h"

class ModDelayComponent : public juce::Component
{
public:
    ModDelayComponent(juce::AudioProcessorValueTreeState& state);
    ~ModDelayComponent() override = default;

    void resized() override;

    void setModulationType(ModDelay::ModulationType type);
    void setDelayTime(float);
    void setFeedbackLeft(float);
    void setFeedbackRight(float);
    void setMix(float);
    void setModDepth(float);
    void setModRate(float);

private:
    static constexpr int knobSize = 100;
    static constexpr int margin = 10;
    static constexpr int labelHeight = 20;
    static constexpr int spacing = 20;

    struct KnobWithLabel {
        juce::Slider slider;
        juce::TextEditor label;
        std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attachment;
    };

    KnobWithLabel delayTime, feedbackL, feedbackR, mix, modDepth, modRate;

    juce::ComboBox modulationTypeComboBox;
    juce::TextEditor modTypeLabel;
    juce::ToggleButton syncToggle;

    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> modulationTypeAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> syncAttachment;

    void configureKnob(KnobWithLabel& kwl, const juce::String& paramID, const juce::String& labelText, juce::AudioProcessorValueTreeState& state);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ModDelayComponent)
};

#endif // ECHOPSYCHFX_MODDELAYCOMPONENT_H_INCLUDED
