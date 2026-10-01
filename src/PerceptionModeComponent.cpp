#include "PerceptionModeComponent.h"
#include "PluginLookAndFeel.h"

PerceptionModeComponent::PerceptionModeComponent(PerceptionPresetManager& presetManager)
    : presetManagerRef(presetManager)
{
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

    for (auto* button : { &saveAsButton, &renameButton, &deleteButton, &prevButton, &nextButton, &insightButton })
    {
        button->setColour(juce::TextButton::buttonColourId, PluginLookAndFeel::panelRaised);
        button->setColour(juce::TextButton::textColourOffId, PluginLookAndFeel::labelText);
        button->setColour(juce::TextButton::textColourOnId, juce::Colours::white);
        addAndMakeVisible(button);
    }
    insightButton.setClickingTogglesState(true);
    insightButton.setColour(juce::TextButton::buttonOnColourId, PluginLookAndFeel::accentSpatial.withAlpha(0.8f));
    insightButton.setTooltip("Explain what this sound does to perception");

    saveAsButton.onClick = [this] { showSaveAsDialog(); };
    renameButton.onClick = [this] { showRenameDialog(); };
    deleteButton.onClick = [this] { showDeleteConfirmation(); };
    prevButton.onClick = [this] { stepPreset(-1); };
    nextButton.onClick = [this] { stepPreset(+1); };
    insightButton.onClick = [this]
    {
        breakdownLabel.setVisible(insightButton.getToggleState());
        if (onHeightChanged)
            onHeightChanged();
    };

    breakdownLabel.setFont(juce::Font(14.0f));
    breakdownLabel.setColour(juce::Label::textColourId, PluginLookAndFeel::labelText.withAlpha(0.85f));
    breakdownLabel.setJustificationType(juce::Justification::topLeft);
    breakdownLabel.setMinimumHorizontalScale(1.0f);
    addChildComponent(breakdownLabel);

    refreshPresetList(factoryPresetNames.isEmpty() ? juce::String() : factoryPresetNames[0]);
    lastSelectedPresetName = presetSelector.getText();
    refreshBreakdown();

    startTimerHz(10);
}

PerceptionModeComponent::~PerceptionModeComponent()
{
    stopTimer();
}

int PerceptionModeComponent::getPreferredHeight() const
{
    return PluginLookAndFeel::kPresetBarH
        + (insightButton.getToggleState() ? PluginLookAndFeel::kInsightDrawerH : 0);
}

void PerceptionModeComponent::resized()
{
    auto area = getLocalBounds().reduced(12, 10);
    auto row = area.removeFromTop(32);

    row.removeFromLeft(196);   // brand wordmark is painted here

    insightButton.setBounds(row.removeFromRight(78));
    row.removeFromRight(14);
    deleteButton.setBounds(row.removeFromRight(64));
    row.removeFromRight(6);
    renameButton.setBounds(row.removeFromRight(70));
    row.removeFromRight(6);
    saveAsButton.setBounds(row.removeFromRight(60));
    row.removeFromRight(14);

    prevButton.setBounds(row.removeFromLeft(32));
    row.removeFromLeft(6);
    nextButton.setBounds(row.removeFromRight(32));
    row.removeFromRight(6);
    presetSelector.setBounds(row);

    area.removeFromTop(10);
    chipsArea = area.removeFromTop(26);

    if (breakdownLabel.isVisible())
    {
        area.removeFromTop(8);
        breakdownLabel.setBounds(area);
    }
}

void PerceptionModeComponent::paint(juce::Graphics& g)
{
    using L = PluginLookAndFeel;
    L::drawPanel(g, getLocalBounds().withHeight(L::kPresetBarH), L::panel);

    // Wordmark
    g.setFont(juce::Font(22.0f, juce::Font::bold));
    const int y = 10, h = 32;
    int x = 18;
    for (auto [text, colour] : { std::pair<const char*, juce::Colour>{ "Echo", L::labelText },
                                  { "Psych", L::accentSpatial }, { "FX", L::accentMotion } })
    {
        const int w = juce::roundToInt(juce::GlyphArrangement::getStringWidthInt(g.getCurrentFont(), text)) + 1;
        g.setColour(colour);
        g.drawText(text, x, y, w, h, juce::Justification::centredLeft, false);
        x += w;
    }

    // Live "feeling" chips
    static const juce::Colour palette[] = { L::accentInput, L::accentMotion, L::accentSpatial,
                                            L::accentMicroPitch, L::accentExciter, L::accentReverb };
    g.setFont(juce::Font(13.0f, juce::Font::bold));
    int cx = chipsArea.getX();
    for (int i = 0; i < currentTags.size(); ++i)
    {
        const auto& tag = currentTags[i];
        const int w = juce::roundToInt(juce::GlyphArrangement::getStringWidthInt(g.getCurrentFont(), tag)) + 22;
        if (cx + w > chipsArea.getRight())
        {
            g.setColour(L::mutedText);
            g.drawText("+" + juce::String(currentTags.size() - i), cx, chipsArea.getY(), 40, chipsArea.getHeight(),
                juce::Justification::centredLeft, false);
            break;
        }

        const auto colour = palette[static_cast<size_t>(tag.hashCode() & 0x7fffffff) % 6];
        const auto chip = juce::Rectangle<float>((float)cx, (float)chipsArea.getY(), (float)w, (float)chipsArea.getHeight());
        g.setColour(colour.withAlpha(0.16f));
        g.fillRoundedRectangle(chip, chip.getHeight() * 0.5f);
        g.setColour(colour.withAlpha(0.7f));
        g.drawRoundedRectangle(chip.reduced(0.5f), chip.getHeight() * 0.5f, 1.0f);
        g.setColour(juce::Colours::white.withAlpha(0.92f));
        g.drawText(tag, chip.toNearestInt(), juce::Justification::centred, false);
        cx += w + 8;
    }
}

void PerceptionModeComponent::stepPreset(int direction)
{
    const int total = presetSelector.getNumItems();
    if (total == 0)
        return;

    // Step from the last real preset so stepping works while "Custom" is showing.
    int index = 0;
    for (int i = 0; i < total; ++i)
        if (presetSelector.getItemText(i) == lastSelectedPresetName) { index = i; break; }

    for (int tries = 0; tries < total; ++tries)
    {
        index = (index + direction + total) % total;
        if (presetSelector.getItemId(index) != kCustomItemId)
            break;
    }
    presetSelector.setSelectedItemIndex(index, juce::sendNotification);
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
        refreshBreakdown();
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

    // Always kept live so it describes whatever's actually playing, factory preset, user
    // preset, or Custom alike - not just recomputed on preset changes.
    refreshBreakdown();
}

void PerceptionModeComponent::refreshBreakdown()
{
    const auto newTags = presetManagerRef.generateFeelingTags();
    if (newTags != currentTags)
    {
        currentTags = newTags;
        repaint();
    }

    const auto newText = presetManagerRef.generateBreakdown();
    if (breakdownLabel.getText() != newText)
        breakdownLabel.setText(newText, juce::dontSendNotification);
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