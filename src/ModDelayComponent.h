#ifndef ECHOPSYCHFX_MODDELAYCOMPONENT_H_INCLUDED
#define ECHOPSYCHFX_MODDELAYCOMPONENT_H_INCLUDED

#include <juce_gui_extra/juce_gui_extra.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include "UIHelpers.h"
#include "ModDelay.h"
#include <memory>
#include <vector>

class ModDelayComponent : public juce::Component
{
public:
    ModDelayComponent(juce::AudioProcessorValueTreeState& state);
    ~ModDelayComponent() override = default;

    void paint(juce::Graphics& g) override;
    void resized() override;

    void setModulationType(ModDelay::ModulationType type);
    void setDelayTime(float value);
    void setFeedbackLeft(float value);
    void setFeedbackRight(float value);
    void setMix(float value);
    void setModDepth(float value);
    void setModRate(float value);

    int getMinimumWidth() const;
    int getMinimumHeight() const;
    int getRequiredHeight(int width) const;

private:
    juce::GroupComponent group{ "modDelayGroup", "Motion Shifter" };

    std::vector<std::unique_ptr<UIHelpers::KnobWithLabel>> knobs;

    std::unique_ptr<UIHelpers::KnobWithLabel> feedbackLKnob;
    std::unique_ptr<UIHelpers::KnobWithLabel> feedbackRKnob;

    juce::OwnedArray<juce::TextButton> waveformButtons;
    std::unique_ptr<juce::ComboBox> hiddenCombo;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> modulationTypeAttachment;
    int selectedWaveform = 0;

    juce::ToggleButton syncToggle;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> syncAttachment;
    juce::Component advancedSection;

    void updateWaveformSelection(int index);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ModDelayComponent)
};

#endif