#ifndef ECHOPSYCHFX_PERCEPTIONMODECOMPONENT_H_INCLUDED
#define ECHOPSYCHFX_PERCEPTIONMODECOMPONENT_H_INCLUDED

#include <juce_gui_extra/juce_gui_extra.h>
#include <functional>
#include "PerceptionPresetManager.h"

/**
 * @brief UI component for selecting psychoacoustic perception presets
 *
 * Provides a dropdown menu for choosing from various pre-configured
 * psychoacoustic effect combinations
 */
class PerceptionModeComponent : public juce::Component,
    private juce::ComboBox::Listener,
    private juce::Timer
{
public:
    PerceptionModeComponent(PerceptionPresetManager& presetManager, juce::AudioProcessorValueTreeState& state);
    ~PerceptionModeComponent() override;

    void resized() override;
    void paint(juce::Graphics& g) override;

    /** Height the bar wants right now: the compact strip, plus the insight drawer when open. */
    int getPreferredHeight() const;

    /** Called when the insight drawer opens or closes so the editor can resize the window. */
    std::function<void()> onHeightChanged;

private:
    // Reserved ComboBox item ID for the synthetic "Custom" entry - picked well above any
    // real preset's ID so it never collides with the factory/user preset list.
    static constexpr int kCustomItemId = 1 << 20;

    /** The small "sort" icon beside the preset name: three bars of falling length, with a dot when the
        list isn't in its default order. */
    class SortButton : public juce::TextButton
    {
    public:
        SortButton() : juce::TextButton("") {}
        void setSortActive(bool shouldShowDot) { if (active != shouldShowDot) { active = shouldShowDot; repaint(); } }
        void paintButton(juce::Graphics& g, bool highlighted, bool down) override;

    private:
        bool active = false;
    };

    juce::ComboBox presetSelector;
    SortButton sortButton;
    juce::TextButton prevButton{ "<" };
    juce::TextButton nextButton{ ">" };
    juce::TextButton saveAsButton{ "Save" };
    juce::TextButton renameButton{ "Rename" };
    juce::TextButton deleteButton{ "Delete" };
    juce::TextButton insightButton{ "Insight" };

    // Output trim lives here too, so the final level is always within reach
    juce::Label outputLabel;
    juce::Slider outputSlider;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> outputAttachment;

    juce::StringArray currentTags;
    juce::Rectangle<int> chipsArea;
    juce::Rectangle<int> profileArea;      // the six perception bars, shown in the Insight drawer
    PerceptionProfile currentProfile;
    juce::Label breakdownLabel;

    void stepPreset(int direction);
    void showSortMenu();

    juce::StringArray factoryPresetNames;

    juce::String lastSelectedPresetName;

    PerceptionPresetManager& presetManagerRef;

    void comboBoxChanged(juce::ComboBox* comboBoxThatHasChanged) override;
    void timerCallback() override;
    void refreshBreakdown();

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