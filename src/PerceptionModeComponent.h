#ifndef ECHOPSYCHFX_PERCEPTIONMODECOMPONENT_H_INCLUDED
#define ECHOPSYCHFX_PERCEPTIONMODECOMPONENT_H_INCLUDED

#include <juce_gui_extra/juce_gui_extra.h>
#include "PerceptionPresetManager.h"

/**
 * @brief UI component for selecting psychoacoustic perception presets
 *
 * Provides a dropdown menu for choosing from various pre-configured
 * psychoacoustic effect combinations
 */
class PerceptionModeComponent : public juce::Component,
    private juce::ComboBox::Listener
{
public:
    explicit PerceptionModeComponent(PerceptionPresetManager& presetManager);
    ~PerceptionModeComponent() override;

    void resized() override;

private:
    juce::Label titleLabel;
    juce::ComboBox presetSelector;
    juce::TextButton saveAsButton{ "Save As..." };
    juce::TextButton renameButton{ "Rename..." };
    juce::TextButton deleteButton{ "Delete" };

    juce::StringArray factoryPresetNames;

    PerceptionPresetManager& presetManagerRef;

    void comboBoxChanged(juce::ComboBox* comboBoxThatHasChanged) override;

    /** Rebuilds the dropdown from the factory list plus the manager's current user
        presets, and selects presetToSelect if given (otherwise keeps the current text). */
    void refreshPresetList(const juce::String& presetToSelect = {});
    void updateButtonStates();
    void showSaveAsDialog();
    void showRenameDialog();
    void showDeleteConfirmation();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PerceptionModeComponent)
};

#endif // ECHOPSYCHFX_PERCEPTIONMODECOMPONENT_H_INCLUDED