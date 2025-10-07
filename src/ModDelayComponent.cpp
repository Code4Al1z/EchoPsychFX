#include "ModDelayComponent.h"

ModDelayComponent::ModDelayComponent(juce::AudioProcessorValueTreeState& state)
{
    addAndMakeVisible(group);
    PluginLookAndFeel::configureGroup(group);

    knobs.emplace_back(std::make_unique<PluginLookAndFeel::KnobWithLabel>(state, "delayTime", "Delay", *this));
    knobs.emplace_back(std::make_unique<PluginLookAndFeel::KnobWithLabel>(state, "modDepth", "Depth", *this));
    knobs.emplace_back(std::make_unique<PluginLookAndFeel::KnobWithLabel>(state, "modRate", "Rate", *this));
    knobs.emplace_back(std::make_unique<PluginLookAndFeel::KnobWithLabel>(state, "modMix", "Mix", *this));

    feedbackLKnob = std::make_unique<PluginLookAndFeel::KnobWithLabel>(state, "feedbackL", "FB L", *this);
    feedbackRKnob = std::make_unique<PluginLookAndFeel::KnobWithLabel>(state, "feedbackR", "FB R", *this);

    const std::vector<std::pair<juce::String, ModDelay::ModulationType>> waveformData = {
        { "Sin", ModDelay::ModulationType::Sine },
        { "Tri", ModDelay::ModulationType::Triangle },
        { "Sqr", ModDelay::ModulationType::Square },
        { "Sw^", ModDelay::ModulationType::SawtoothUp },
        { "Sw_", ModDelay::ModulationType::SawtoothDown }
    };

    int idx = 0;
    for (auto& [label, type] : waveformData)
    {
        auto* btn = waveformButtons.add(new juce::TextButton(label));
        btn->setColour(juce::TextButton::buttonColourId, juce::Colours::darkgrey);
        btn->setColour(juce::TextButton::textColourOnId, PluginLookAndFeel::labelText);
        btn->setColour(juce::TextButton::textColourOffId, PluginLookAndFeel::labelText.withAlpha(0.7f));
        btn->onClick = [this, idx] { updateWaveformSelection(idx); };
        addAndMakeVisible(btn);
        ++idx;
    }

    hiddenCombo = std::make_unique<juce::ComboBox>();
    modulationTypeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        state, "modulationType", *hiddenCombo);
    hiddenCombo->setVisible(false);
    addAndMakeVisible(*hiddenCombo);

    syncToggle.setButtonText("Sync");
    syncToggle.setColour(juce::ToggleButton::textColourId, PluginLookAndFeel::labelText);
    syncToggle.setColour(juce::ToggleButton::tickColourId, PluginLookAndFeel::track);
    syncAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        state, "sync", syncToggle);
    addAndMakeVisible(syncToggle);

    updateWaveformSelection(0);
}

void ModDelayComponent::paint(juce::Graphics& g)
{
    g.fillAll(PluginLookAndFeel::background);
}

void ModDelayComponent::resized()
{
    group.setBounds(getLocalBounds());

    auto area = getLocalBounds().reduced(PluginLookAndFeel::margin);

    int y = area.getY();
    int x = area.getX();

    const int btnWidth = 50;
    const int btnHeight = 26;

    for (auto* btn : waveformButtons)
    {
        btn->setBounds(x, y, btnWidth, btnHeight);
        x += btnWidth + 4;
    }

    syncToggle.setBounds(x + PluginLookAndFeel::margin, y, 70, btnHeight);

    y += btnHeight + PluginLookAndFeel::margin + 5;

    area.removeFromTop(btnHeight + PluginLookAndFeel::margin + 5);

    const int numTotalKnobs = static_cast<int>(knobs.size()) + 2;
    auto layout = PluginLookAndFeel::calculateKnobLayout(numTotalKnobs, area.getWidth(), area.getHeight(), true);

    const int numMainKnobs = static_cast<int>(knobs.size());
    for (int i = 0; i < numTotalKnobs; ++i)
    {
        if (i >= static_cast<int>(layout.knobBounds.size()))
            break;

        auto bounds = layout.knobBounds[i];
        bounds.translate(area.getX(), area.getY());

        if (i < numMainKnobs)
        {
            knobs[i]->setBounds(bounds.getX(), bounds.getY(), bounds.getWidth(), bounds.getHeight());
        }
        else if (i == numMainKnobs && feedbackLKnob)
        {
            feedbackLKnob->setBounds(bounds.getX(), bounds.getY(), bounds.getWidth(), bounds.getHeight());
        }
        else if (i == numMainKnobs + 1 && feedbackRKnob)
        {
            feedbackRKnob->setBounds(bounds.getX(), bounds.getY(), bounds.getWidth(), bounds.getHeight());
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
            isSelected ? PluginLookAndFeel::track : juce::Colours::darkgrey);
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