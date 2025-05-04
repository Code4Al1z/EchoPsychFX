#ifndef ECHOPSYCHFX_PERCEPTIONCOMPONENT_H_INCLUDED
#define ECHOPSYCHFX_PERCEPTIONCOMPONENT_H_INCLUDED

#include <juce_gui_extra/juce_gui_extra.h>
#include "PerceptionPresetManager.h"

class PerceptionModeComponent : public juce::Component,
    private juce::ComboBox::Listener
{
public:
    PerceptionModeComponent(PerceptionPresetManager& presetManager);
    ~PerceptionModeComponent() override;

    //void paint(juce::Graphics& g) override;
    void resized() override;

private:
    juce::Label titleLabel;
    juce::ComboBox presetSelector;

    PerceptionPresetManager& presetManagerRef;

    void comboBoxChanged(juce::ComboBox* comboBoxThatHasChanged) override;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PerceptionModeComponent)
};

#endif // ECHOPSYCHFX_PERCEPTIONCOMPONENT_H_INCLUDED