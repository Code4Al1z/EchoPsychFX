#include "PerceptionModeComponent.h"
#include "PluginLookAndFeel.h"

PerceptionModeComponent::PerceptionModeComponent(PerceptionPresetManager& presetManager,
    juce::AudioProcessorValueTreeState& state)
    : presetManagerRef(presetManager)
{
    // Factory preset names come from the preset library. User presets are appended dynamically.
    factoryPresetNames = presetManager.getFactoryPresetNames();

    PluginLookAndFeel::configureComboBox(presetSelector);
    presetSelector.onChange = [this]() { comboBoxChanged(&presetSelector); };
    addAndMakeVisible(presetSelector);

    sortButton.setTooltip("Sort the presets: factory order, A to Z, or by what they do to perception");
    sortButton.onClick = [this] { showSortMenu(); };
    addAndMakeVisible(sortButton);

    for (auto* button : { &saveAsButton, &renameButton, &deleteButton, &prevButton, &nextButton, &insightButton })
    {
        button->setColour(juce::TextButton::buttonColourId, PluginLookAndFeel::panelRaised);
        button->setColour(juce::TextButton::textColourOffId, PluginLookAndFeel::labelText);
        button->setColour(juce::TextButton::textColourOnId, juce::Colours::white);
        addAndMakeVisible(button);
    }
    // Output trim: a bipolar slider that fills outward from 0 dB. Double-click resets it.
    PluginLookAndFeel::configureLabel(outputLabel, "OUTPUT");
    outputLabel.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(outputLabel);

    outputSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    outputSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 74, 22);
    outputSlider.setColour(juce::Slider::trackColourId, PluginLookAndFeel::labelText.withAlpha(0.75f));
    outputSlider.setColour(juce::Slider::textBoxTextColourId, PluginLookAndFeel::labelText);
    outputSlider.setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    outputSlider.setTooltip("Output trim: the final level after every effect. Double-click to reset to 0 dB.");
    addAndMakeVisible(outputSlider);
    outputAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(state, "outputGain", outputSlider);

    if (auto* trimParam = state.getParameter("outputGain"))
    {
        outputSlider.textFromValueFunction = [trimParam](double v)
            {
                return trimParam->getText(trimParam->convertTo0to1(static_cast<float>(v)), 0) + " dB";
            };
        outputSlider.updateText();
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
    sortButton.setBounds(row.removeFromRight(32));
    row.removeFromRight(6);
    presetSelector.setBounds(row);

    area.removeFromTop(10);
    chipsArea = area.removeFromTop(26);

    auto outputArea = chipsArea.removeFromRight(260);
    chipsArea.removeFromRight(12);
    outputLabel.setBounds(outputArea.removeFromLeft(58));
    outputSlider.setBounds(outputArea);

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

    const auto sorted = presetManagerRef.getSortedPresetNames();
    const bool defaultOrder = presetManagerRef.getSortMode() == PresetSort::FactoryOrder && !presetManagerRef.getSortReverse();
    sortButton.setSortActive(!defaultOrder);

    int id = 1;

    if (!defaultOrder)
        presetSelector.addSectionHeading("Sorted: " + PerceptionPresetManager::describeSort(
            presetManagerRef.getSortMode(), presetManagerRef.getSortReverse()));

    for (auto& name : sorted.factory)
        presetSelector.addItem(name, id++);

    if (!sorted.user.isEmpty())
    {
        presetSelector.addSeparator();
        presetSelector.addSectionHeading("User Presets");
        for (auto& name : sorted.user)
            presetSelector.addItem(name, id++);
    }

    presetSelector.setText(previousSelection, juce::dontSendNotification);
    updateButtonStates();
}

void PerceptionModeComponent::showSortMenu()
{
    using Sort = PresetSort;
    const auto mode = presetManagerRef.getSortMode();
    const bool reverse = presetManagerRef.getSortReverse();

    juce::PopupMenu menu;
    menu.setLookAndFeel(&getLookAndFeel());   // a popup doesn't inherit the editor's theme by itself
    menu.addSectionHeader("Order");
    menu.addItem(1, "Factory order", true, mode == Sort::FactoryOrder);
    menu.addItem(2, "A to Z", true, mode == Sort::Alphabetical);
    menu.addSeparator();
    menu.addSectionHeader("By what they do to perception");

    for (int axis = 0; axis < kNumPerceptionAxes; ++axis)
    {
        const auto a = static_cast<PerceptionAxis>(axis);
        const auto thisMode = static_cast<Sort>(static_cast<int>(Sort::Brightness) + axis);
        menu.addItem(10 + axis, juce::String(perceptionAxisName(a)) + "  -  "
            + (reverse ? perceptionAxisLowWord(a) : perceptionAxisHighWord(a)) + " first", true, mode == thisMode);
    }

    menu.addSeparator();
    menu.addItem(100, "Reverse order", true, reverse);

    menu.showMenuAsync(juce::PopupMenu::Options().withTargetComponent(&sortButton).withMinimumWidth(250),
        [this](int result)
        {
            if (result == 0)
                return;

            auto newMode = presetManagerRef.getSortMode();
            auto newReverse = presetManagerRef.getSortReverse();

            if (result == 1)        newMode = Sort::FactoryOrder;
            else if (result == 2)   newMode = Sort::Alphabetical;
            else if (result >= 10 && result < 10 + kNumPerceptionAxes)
                newMode = static_cast<Sort>(static_cast<int>(Sort::Brightness) + (result - 10));
            else if (result == 100) newReverse = !newReverse;

            presetManagerRef.setSort(newMode, newReverse);

            // Keep the same preset selected; "Custom" has no list entry, so hold on to the last real one
            refreshPresetList(presetSelector.getSelectedId() == kCustomItemId ? lastSelectedPresetName : presetSelector.getText());
        });
}

void PerceptionModeComponent::SortButton::paintButton(juce::Graphics& g, bool highlighted, bool down)
{
    using L = PluginLookAndFeel;
    auto r = getLocalBounds().toFloat().reduced(0.5f);
    g.setColour(L::panelRaised.brighter(down ? 0.2f : highlighted ? 0.1f : 0.0f));
    g.fillRoundedRectangle(r, 5.0f);
    g.setColour(juce::Colours::white.withAlpha(0.08f));
    g.drawRoundedRectangle(r, 5.0f, 1.0f);

    // Three bars of decreasing length: the usual "sort" glyph
    const float cx = r.getCentreX(), cy = r.getCentreY();
    g.setColour(active ? L::accentSpatial.brighter(0.2f) : L::labelText.withAlpha(0.85f));
    const float widths[] = { 14.0f, 10.0f, 6.0f };
    for (int i = 0; i < 3; ++i)
        g.fillRoundedRectangle(cx - 7.0f, cy - 6.0f + i * 5.0f, widths[i], 2.0f, 1.0f);

    if (active)
    {
        g.setColour(L::accentSpatial);
        g.fillEllipse(r.getRight() - 8.0f, r.getY() + 3.0f, 5.0f, 5.0f);
    }
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