#ifndef ECHOPSYCHFX_MODDELAYCOMPONENT_H_INCLUDED
#define ECHOPSYCHFX_MODDELAYCOMPONENT_H_INCLUDED

#include <memory>
#include <juce_gui_extra/juce_gui_extra.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include "ModDelay.h"

class ModDelayComponent: public juce::Component,
                         private juce::ComboBox::Listener
{
public:
    ModDelayComponent(juce::AudioProcessorValueTreeState& state);
    ~ModDelayComponent() override;

    void resized() override;

    void setModulationType(ModDelay::ModulationType type);

    void setDelayTime(float newValue);
    void setFeedbackLeft(float newValue);
    void setFeedbackRight(float newValue);
    void setMix(float newValue);
    void setModDepth(float newValue);
    void setModRate(float newValue);

private:

    int knobSize = 120;
    int margin = 10;

    juce::Slider delayTimeSlider, feedbackLSlider, feedbackRSlider, mixSlider, modDepthSlider, modRateSlider;
    juce::ComboBox modulationTypeComboBox; // New combo box for modulation type

    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> delayAttachment, feedbackLAttachment, feedbackRAttachment, mixAttachment, depthAttachment, rateAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> modulationTypeAttachment; // Attachment for combo box

    juce::TextEditor delayTimeTextBox, feedbackLTextBox, feedbackRTextBox, mixTextBox, modDepthTextBox, modRateTextBox;

    juce::TextEditor modulationTypeTextBox;
    juce::ToggleButton syncToggle;

    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> syncAttachment;

    void comboBoxChanged(juce::ComboBox* comboBoxThatHasChanged);
    ModDelay modDelay;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ModDelayComponent)
};
#endif // ECHOPSYCHFX_MODDELAYCOMPONENT_H_INCLUDED