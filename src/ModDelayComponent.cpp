#include "ModDelayComponent.h"

ModDelayComponent::ModDelayComponent(juce::AudioProcessorValueTreeState& state)
{
    using namespace UIHelpers;

    addAndMakeVisible(group);
    configureGroup(group);

    // Main knobs
    knobs.emplace_back(std::make_unique<KnobWithLabel>(state, "delayTime", "Delay", *this));
    knobs.emplace_back(std::make_unique<KnobWithLabel>(state, "modDepth", "Depth", *this));
    knobs.emplace_back(std::make_unique<KnobWithLabel>(state, "modRate", "Rate", *this));
    knobs.emplace_back(std::make_unique<KnobWithLabel>(state, "modMix", "Mix", *this));

    // Feedback knobs
    feedbackLKnob = std::make_unique<KnobWithLabel>(state, "feedbackL", "FB L", *this);
    feedbackRKnob = std::make_unique<KnobWithLabel>(state, "feedbackR", "FB R", *this);

    // Waveform buttons with better labels
    const std::vector<std::pair<juce::String, ModDelay::ModulationType>> waveformData = {
        { "Sin", ModDelay::ModulationType::Sine },
        { "Tri", ModDelay::ModulationType::Triangle },
        { "Sqr", ModDelay::ModulationType::Square },
        { "Sw▲", ModDelay::ModulationType::SawtoothUp },
        { "Sw▼", ModDelay::ModulationType::SawtoothDown }
    };

    int idx = 0;
    for (auto& [label, type] : waveformData)
    {
        auto* btn = waveformButtons.add(new juce::TextButton(label));
        btn->setColour(juce::TextButton::buttonColourId, juce::Colours::darkgrey);
        btn->setColour(juce::TextButton::textColourOnId, Colors::labelText);
        btn->setColour(juce::TextButton::textColourOffId, Colors::labelText.withAlpha(0.7f));
        btn->onClick = [this, idx] { updateWaveformSelection(idx); };
        addAndMakeVisible(btn);
        ++idx;
    }

    // Hidden combo for state management
    hiddenCombo = std::make_unique<juce::ComboBox>();
    modulationTypeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        state, "modulationType", *hiddenCombo);
    hiddenCombo->setVisible(false);
    addAndMakeVisible(*hiddenCombo);

    // Sync toggle
    syncToggle.setButtonText("Sync");
    syncToggle.setColour(juce::ToggleButton::textColourId, Colors::labelText);
    syncToggle.setColour(juce::ToggleButton::tickColourId, Colors::track);
    syncAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        state, "sync", syncToggle);
    addAndMakeVisible(syncToggle);

    updateWaveformSelection(0);
}

void ModDelayComponent::paint(juce::Graphics& g)
{
    g.fillAll(UIHelpers::Colors::background);
}

void ModDelayComponent::resized()
{
    using namespace UIHelpers::Dimensions;

    group.setBounds(getLocalBounds());

    auto area = getLocalBounds().reduced(margin);
    const int availableWidth = area.getWidth();
    const int availableHeight = area.getHeight();

    // Adaptive knob size
    const int adaptiveKnobSize = juce::jmin(knobSize, availableHeight - 50, availableWidth / 7);
    const bool useCompactLayout = availableWidth < 700;

    int y = area.getY() + 5;
    int x = area.getX() + margin;

    // Waveform buttons at top
    const int btnWidth = useCompactLayout ? 45 : 55;
    const int btnHeight = 26;

    for (auto* btn : waveformButtons)
    {
        btn->setBounds(x, y, btnWidth, btnHeight);
        x += btnWidth + 4;
    }

    // Sync toggle next to waveforms
    syncToggle.setBounds(x + margin, y, 70, btnHeight);

    y += btnHeight + margin + 5;
    x = area.getX() + margin;

    // Main knobs - adapt to available space
    const int numMainKnobs = static_cast<int>(knobs.size());
    const int mainKnobsWidth = (adaptiveKnobSize + spacing) * numMainKnobs - spacing;

    if (useCompactLayout || mainKnobsWidth > availableWidth * 0.7f)
    {
        // Compact: all knobs in one row
        const int compactSize = juce::jmin(adaptiveKnobSize, (availableWidth - margin * 2) / (numMainKnobs + 2) - spacing);

        for (int i = 0; i < numMainKnobs; ++i)
        {
            knobs[i]->setBounds(x, y, compactSize, compactSize + labelHeight);
            x += compactSize + spacing;
        }

        // Feedback knobs at end
        if (feedbackLKnob)
        {
            feedbackLKnob->setBounds(x, y, compactSize, compactSize + labelHeight);
            x += compactSize + spacing;
        }
        if (feedbackRKnob)
        {
            feedbackRKnob->setBounds(x, y, compactSize, compactSize + labelHeight);
        }
    }
    else
    {
        // Standard: main knobs, then feedback knobs in a separate section
        for (int i = 0; i < numMainKnobs; ++i)
        {
            knobs[i]->setBounds(x, y, adaptiveKnobSize, adaptiveKnobSize + labelHeight);
            x += adaptiveKnobSize + spacing;
        }

        // Add extra space before feedback knobs
        x += spacing * 2;

        if (feedbackLKnob)
        {
            feedbackLKnob->setBounds(x, y, adaptiveKnobSize, adaptiveKnobSize + labelHeight);
            x += adaptiveKnobSize + spacing;
        }
        if (feedbackRKnob && x + adaptiveKnobSize <= area.getRight())
        {
            feedbackRKnob->setBounds(x, y, adaptiveKnobSize, adaptiveKnobSize + labelHeight);
        }
    }
}

void ModDelayComponent::updateWaveformSelection(int index)
{
    if (index < 0 || index >= waveformButtons.size())
        return;

    selectedWaveform = index;

    for (int i = 0; i < waveformButtons.size(); ++i)
    {
        auto* btn = waveformButtons[i];
        const bool isSelected = (i == index);
        btn->setColour(juce::TextButton::buttonColourId,
            isSelected ? UIHelpers::Colors::track : juce::Colours::darkgrey);
        btn->setAlpha(isSelected ? 1.0f : 0.7f);
    }

    if (hiddenCombo)
        hiddenCombo->setSelectedId(index + 1, juce::NotificationType::sendNotification);
}

void ModDelayComponent::setModulationType(ModDelay::ModulationType type)
{
    int index = static_cast<int>(type) - 1;
    if (index != selectedWaveform)
        updateWaveformSelection(index);
}

void ModDelayComponent::setDelayTime(float value)
{
    if (!knobs.empty())
        knobs[0]->slider->setValue(value);
}

void ModDelayComponent::setFeedbackLeft(float value)
{
    if (feedbackLKnob)
        feedbackLKnob->slider->setValue(value);
}

void ModDelayComponent::setFeedbackRight(float value)
{
    if (feedbackRKnob)
        feedbackRKnob->slider->setValue(value);
}

void ModDelayComponent::setMix(float value)
{
    if (knobs.size() > 3)
        knobs[3]->slider->setValue(value);
}

void ModDelayComponent::setModDepth(float value)
{
    if (knobs.size() > 1)
        knobs[1]->slider->setValue(value);
}

void ModDelayComponent::setModRate(float value)
{
    if (knobs.size() > 2)
        knobs[2]->slider->setValue(value);
}