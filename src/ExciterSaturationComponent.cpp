#include "ExciterSaturationComponent.h"

ExciterSaturationComponent::ExciterSaturationComponent(juce::AudioProcessorValueTreeState& state)
{
    addAndMakeVisible(group);
    PluginLookAndFeel::configureGroup(group);

    knobs.emplace_back(std::make_unique<PluginLookAndFeel::KnobWithLabel>(state, "exciterDrive", "Drive", *this));
    knobs.emplace_back(std::make_unique<PluginLookAndFeel::KnobWithLabel>(state, "exciterMix", "Mix", *this));
    knobs.emplace_back(std::make_unique<PluginLookAndFeel::KnobWithLabel>(state, "exciterHighpass", "Highpass", *this));
}

void ExciterSaturationComponent::paint(juce::Graphics& g)
{
    g.fillAll(PluginLookAndFeel::background);
}

void ExciterSaturationComponent::resized()
{
    group.setBounds(getLocalBounds());

    auto area = getLocalBounds().reduced(PluginLookAndFeel::margin);
    const int numKnobs = static_cast<int>(knobs.size());

    auto layout = PluginLookAndFeel::calculateKnobLayout(numKnobs, area.getWidth(), area.getHeight(), false);

    for (int i = 0; i < numKnobs; ++i)
    {
        auto bounds = layout.knobBounds[i];
        bounds.translate(area.getX(), area.getY());
        knobs[i]->setBounds(bounds.getX(), bounds.getY(), bounds.getWidth(), bounds.getHeight());
    }
}

void ExciterSaturationComponent::setDrive(float newValue)
{
    if (!knobs.empty()) knobs[0]->slider->setValue(newValue);
}

void ExciterSaturationComponent::setMix(float newValue)
{
    if (knobs.size() > 1) knobs[1]->slider->setValue(newValue);
}

void ExciterSaturationComponent::setHighpass(float newValue)
{
    if (knobs.size() > 2) knobs[2]->slider->setValue(newValue);
}