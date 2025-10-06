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
    knobs.emplace_back(std::make_unique<KnobWithLabel>(state, "modMix", "Mod Mix", *this));

    // Advanced section knobs
    feedbackLKnob = std::make_unique<KnobWithLabel>(state, "feedbackL", "FB Left", advancedSection);
    feedbackRKnob = std::make_unique<KnobWithLabel>(state, "feedbackR", "FB Right", advancedSection);

    // Waveform buttons
    const std::vector<std::pair<juce::String, ModDelay::ModulationType>> waveformData = {
        { "Sine", ModDelay::ModulationType::Sine },
        { "Tri", ModDelay::ModulationType::Triangle },
        { "Sq", ModDelay::ModulationType::Square },
        { "Saw+", ModDelay::ModulationType::SawtoothUp },
        { "Saw-", ModDelay::ModulationType::SawtoothDown }
    };

    int idx = 0;
    for (auto& [label, type] : waveformData)
    {
        auto* btn = waveformButtons.add(new juce::TextButton(label));
        btn->setColour(juce::TextButton::buttonColourId, juce::Colours::darkgrey);
        btn->setColour(juce::TextButton::textColourOnId, Colors::labelText);
        btn->setColour(juce::TextButton::textColourOffId, Colors::labelText);
        btn->onClick = [this, idx] { updateWaveformSelection(idx); };
        addAndMakeVisible(btn);
        ++idx;
    }

    // Hidden combo for state management
    hiddenCombo = std::make_unique<juce::ComboBox>();
    modulationTypeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        state, "modulationType", *hiddenCombo);
    hiddenCombo->setComponentID("modTypeCombo");
    hiddenCombo->setVisible(false);
    addAndMakeVisible(*hiddenCombo);

    // Sync toggle
    syncToggle.setButtonText("Sync");
    syncToggle.setColour(juce::ToggleButton::textColourId, Colors::labelText);
    syncToggle.setColour(juce::ToggleButton::tickColourId, Colors::track);
    syncAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        state, "sync", syncToggle);
    advancedSection.addAndMakeVisible(syncToggle);

    addAndMakeVisible(advancedSection);

    // Select first waveform by default
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
    int y = area.getY() + 5;
    int x = area.getX();

    // Waveform buttons
    int waveX = x;
    for (auto* btn : waveformButtons)
    {
        btn->setBounds(waveX, y, 60, 24);
        waveX += 65;
    }
    y += 35;

    // Main knobs
    const int numKnobs = static_cast<int>(knobs.size());
    for (int i = 0; i < numKnobs; ++i)
    {
        knobs[i]->setBounds(x + i * (knobSize + spacing), y, knobSize, knobSize + labelHeight);
    }

    // Advanced section
    int advX = x + numKnobs * (knobSize + spacing) + spacing;
    advancedSection.setBounds(advX, y, getWidth() - advX - margin, knobSize + labelHeight);

    syncToggle.setBounds(0, 0, 60, 30);

    if (feedbackLKnob)
        feedbackLKnob->setBounds(80, 0, knobSize, knobSize + labelHeight);

    if (feedbackRKnob)
        feedbackRKnob->setBounds(200, 0, knobSize, knobSize + labelHeight);
}

void ModDelayComponent::updateWaveformSelection(int index)
{
    if (index < 0 || index >= waveformButtons.size())
        return;

    selectedWaveform = index;

    for (int i = 0; i < waveformButtons.size(); ++i)
    {
        waveformButtons[i]->setColour(juce::TextButton::buttonColourId,
            i == index ? UIHelpers::Colors::track : juce::Colours::darkgrey);
    }

    // Update hidden combo for state management
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