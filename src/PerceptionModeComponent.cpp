#include "PerceptionModeComponent.h"
#include "PluginLookAndFeel.h"

PerceptionModeComponent::PerceptionModeComponent(PerceptionPresetManager& presetManager)
    : presetManagerRef(presetManager)
{
    PluginLookAndFeel::configureLabel(titleLabel, "Perception Mode");
    titleLabel.setFont(juce::Font(20.0f, juce::Font::bold));
    addAndMakeVisible(titleLabel);

    // Factory preset names - fixed, read-only. User presets are appended dynamically.
    factoryPresetNames = {
        "Init",
        "Head Trip", "Panic Room", "Intimacy", "Blade Runner", "Alien Abduction",
        "Glass Tunnel", "Dream Logic", "Womb Space", "Bipolar Bloom", "Quiet Confidence",
        "Falling Upwards", "Molten Light", "Ethereal Echo", "Lush Dreamscape", "Skin Contact",
        "Sonic Embrace", "Strobe Heaven", "Glass Flame", "Celestial Vault", "Deep Illusion",
        "Ego Dissolve", "Memory Dust", "Gentle Slap", "Moon Dance", "Biting Lips",
        "Stormy Day", "Summer Sunset", "Ocean Waves", "Crystal Clear", "Sweetest Memory"
    };

    PluginLookAndFeel::configureComboBox(presetSelector);
    presetSelector.onChange = [this]() { comboBoxChanged(&presetSelector); };
    addAndMakeVisible(presetSelector);

    for (auto* button : { &saveAsButton, &renameButton, &deleteButton })
    {
        button->setColour(juce::TextButton::buttonColourId, PluginLookAndFeel::knobBackground);
        button->setColour(juce::TextButton::textColourOffId, PluginLookAndFeel::labelText);
        addAndMakeVisible(button);
    }

    saveAsButton.onClick = [this] { showSaveAsDialog(); };
    renameButton.onClick = [this] { showRenameDialog(); };
    deleteButton.onClick = [this] { showDeleteConfirmation(); };

    refreshPresetList(factoryPresetNames.isEmpty() ? juce::String() : factoryPresetNames[0]);
    lastSelectedPresetName = presetSelector.getText();

    startTimerHz(10);
}

PerceptionModeComponent::~PerceptionModeComponent()
{
    stopTimer();
}

void PerceptionModeComponent::resized()
{
    auto area = getLocalBounds().reduced(10);

    titleLabel.setBounds(area.removeFromTop(30));
    area.removeFromTop(10); // spacing

    presetSelector.setBounds(area.removeFromTop(30));
    area.removeFromTop(8);

    auto buttonRow = area.removeFromTop(26);
    const int buttonW = (buttonRow.getWidth() - 2 * 8) / 3;
    saveAsButton.setBounds(buttonRow.removeFromLeft(buttonW));
    buttonRow.removeFromLeft(8);
    renameButton.setBounds(buttonRow.removeFromLeft(buttonW));
    buttonRow.removeFromLeft(8);
    deleteButton.setBounds(buttonRow);
}

void PerceptionModeComponent::comboBoxChanged(juce::ComboBox* comboBoxThatHasChanged)
{
    if (comboBoxThatHasChanged == &presetSelector)
    {
        const auto selectedName = presetSelector.getText();
        DBG("Selected preset: " + selectedName);
        presetManagerRef.applyPreset(selectedName);
        lastSelectedPresetName = selectedName;

        // A real preset was picked, so drop the synthetic "Custom" entry - refreshPresetList
        // rebuilds the list from scratch, which naturally omits it.
        refreshPresetList(selectedName);
    }
}

void PerceptionModeComponent::timerCallback()
{
    const bool isShowingCustom = presetSelector.getSelectedId() == kCustomItemId;
    const bool matches = presetManagerRef.matchesLastAppliedPreset();

    if (!matches && !isShowingCustom)
    {
        if (presetSelector.indexOfItemId(kCustomItemId) < 0)
        {
            presetSelector.addSeparator();
            presetSelector.addItem("Custom", kCustomItemId);
        }
        presetSelector.setSelectedId(kCustomItemId, juce::dontSendNotification);
        updateButtonStates();
    }
    else if (matches && isShowingCustom)
    {
        // Parameters drifted back into matching the last applied preset - drop "Custom"
        // and restore its name rather than leaving a stale label showing.
        refreshPresetList(lastSelectedPresetName);
    }
}

void PerceptionModeComponent::refreshPresetList(const juce::String& presetToSelect)
{
    const auto previousSelection = presetToSelect.isNotEmpty() ? presetToSelect : presetSelector.getText();

    presetSelector.clear(juce::dontSendNotification);

    int id = 1;
    for (auto& name : factoryPresetNames)
        presetSelector.addItem(name, id++);

    auto userNames = presetManagerRef.getUserPresetNames();
    if (!userNames.isEmpty())
    {
        presetSelector.addSeparator();
        presetSelector.addSectionHeading("User Presets");
        for (auto& name : userNames)
            presetSelector.addItem(name, id++);
    }

    presetSelector.setText(previousSelection, juce::dontSendNotification);
    updateButtonStates();
}

void PerceptionModeComponent::updateButtonStates()
{
    const bool isUserPreset = presetManagerRef.isUserPreset(presetSelector.getText());
    renameButton.setEnabled(isUserPreset);
    deleteButton.setEnabled(isUserPreset);
}

void PerceptionModeComponent::showSaveAsDialog()
{
    auto* aw = new juce::AlertWindow("Save Preset", "Save the current settings as a new preset:",
        juce::AlertWindow::NoIcon);
    aw->addTextEditor("name", presetSelector.getText(), "Name:");
    aw->addButton("Save", 1, juce::KeyPress(juce::KeyPress::returnKey));
    aw->addButton("Cancel", 0, juce::KeyPress(juce::KeyPress::escapeKey));

    aw->enterModalState(true, juce::ModalCallbackFunction::create([this, aw](int result)
        {
            if (result == 1)
            {
                const auto name = aw->getTextEditorContents("name").trim();
                if (name.isEmpty())
                    return;

                if (presetManagerRef.isFactoryPreset(name))
                {
                    juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::WarningIcon,
                        "Can't Overwrite Factory Preset",
                        "\"" + name + "\" is a factory preset and can't be overwritten. Choose a different name.");
                    return;
                }

                presetManagerRef.saveCurrentAsUserPreset(name);
                refreshPresetList(name);
            }
        }), true);
}

void PerceptionModeComponent::showRenameDialog()
{
    const auto oldName = presetSelector.getText();
    if (presetManagerRef.isFactoryPreset(oldName) || oldName.isEmpty())
        return;

    auto* aw = new juce::AlertWindow("Rename Preset", "Enter a new name for \"" + oldName + "\":",
        juce::AlertWindow::NoIcon);
    aw->addTextEditor("name", oldName, "Name:");
    aw->addButton("Rename", 1, juce::KeyPress(juce::KeyPress::returnKey));
    aw->addButton("Cancel", 0, juce::KeyPress(juce::KeyPress::escapeKey));

    aw->enterModalState(true, juce::ModalCallbackFunction::create([this, aw, oldName](int result)
        {
            if (result == 1)
            {
                const auto newName = aw->getTextEditorContents("name").trim();
                if (newName.isEmpty() || newName == oldName)
                    return;

                if (presetManagerRef.isFactoryPreset(newName))
                {
                    juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::WarningIcon,
                        "Can't Use Factory Preset Name",
                        "\"" + newName + "\" is a factory preset name. Choose a different name.");
                    return;
                }

                if (presetManagerRef.renameUserPreset(oldName, newName))
                    refreshPresetList(newName);
            }
        }), true);
}

void PerceptionModeComponent::showDeleteConfirmation()
{
    const auto name = presetSelector.getText();
    if (presetManagerRef.isFactoryPreset(name) || name.isEmpty())
        return;

    juce::AlertWindow::showOkCancelBox(juce::AlertWindow::WarningIcon, "Delete Preset",
        "Delete the preset \"" + name + "\"? This can't be undone.",
        "Delete", "Cancel", this,
        juce::ModalCallbackFunction::create([this, name](int result)
            {
                if (result != 1)
                    return;
                presetManagerRef.deleteUserPreset(name);
                refreshPresetList(factoryPresetNames.isEmpty() ? juce::String() : factoryPresetNames[0]);
                presetManagerRef.applyPreset(presetSelector.getText());
            }));
}