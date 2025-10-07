#include "SimpleVerbWithPredelayComponent.h"
#include "PluginLookAndFeel.h"

SimpleVerbWithPredelayComponent::SimpleVerbWithPredelayComponent(juce::AudioProcessorValueTreeState& state)
{
    addAndMakeVisible(group);
    PluginLookAndFeel::configureGroup(group);

    knobs.emplace_back(std::make_unique<UIHelpers::KnobWithLabel>(state, "predelayMs", "Predelay", *this));
    knobs.emplace_back(std::make_unique<UIHelpers::KnobWithLabel>(state, "size", "Size", *this));
    knobs.emplace_back(std::make_unique<UIHelpers::KnobWithLabel>(state, "damping", "Damping", *this));
    knobs.emplace_back(std::make_unique<UIHelpers::KnobWithLabel>(state, "wet", "Wet", *this));
}

void SimpleVerbWithPredelayComponent::paint(juce::Graphics& g)
{
    g.fillAll(PluginLookAndFeel::background);
}

void SimpleVerbWithPredelayComponent::resized()
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

void SimpleVerbWithPredelayComponent::setPredelay(float newValue)
{
    if (!knobs.empty())
        knobs[0]->slider->setValue(newValue);
}

void SimpleVerbWithPredelayComponent::setSize(float newValue)
{
    if (knobs.size() > 1)
        knobs[1]->slider->setValue(newValue);
}

void SimpleVerbWithPredelayComponent::setDamping(float newValue)
{
    if (knobs.size() > 2)
        knobs[2]->slider->setValue(newValue);
}

void SimpleVerbWithPredelayComponent::setWet(float newValue)
{
    if (knobs.size() > 3)
        knobs[3]->slider->setValue(newValue);
}