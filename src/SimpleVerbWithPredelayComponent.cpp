#include "SimpleVerbWithPredelayComponent.h"

SimpleVerbWithPredelayComponent::SimpleVerbWithPredelayComponent(juce::AudioProcessorValueTreeState& state)
    : CollapsibleComponent("Simple Verb With Predelay")
{
    addAndMakeVisible(group);
    PluginLookAndFeel::configureGroup(group);

    knobs.emplace_back(std::make_unique<PluginLookAndFeel::KnobWithLabel>(state, "predelayMs", "Predelay", *this));
    knobs.emplace_back(std::make_unique<PluginLookAndFeel::KnobWithLabel>(state, "size", "Size", *this));
    knobs.emplace_back(std::make_unique<PluginLookAndFeel::KnobWithLabel>(state, "damping", "Damping", *this));
    knobs.emplace_back(std::make_unique<PluginLookAndFeel::KnobWithLabel>(state, "wet", "Wet", *this));
}

void SimpleVerbWithPredelayComponent::paintContent(juce::Graphics& g)
{
    g.fillAll(PluginLookAndFeel::background);
}

void SimpleVerbWithPredelayComponent::layoutContent(juce::Rectangle<int> area)
{
    group.setBounds(getLocalBounds());
    auto inner = area.reduced(PluginLookAndFeel::margin);
    const int numKnobs = static_cast<int>(knobs.size());

    auto layout = PluginLookAndFeel::calculateKnobLayout(numKnobs, inner.getWidth(), inner.getHeight(), false);
    if ((int)layout.knobBounds.size() < numKnobs) return;

    for (int i = 0; i < numKnobs; ++i)
    {
        auto b = layout.knobBounds[i];
        knobs[i]->setBounds(inner.getX() + b.getX(), inner.getY() + b.getY(), b.getWidth(), b.getHeight());
    }
}

void SimpleVerbWithPredelayComponent::setPredelay(float v) { if (!knobs.empty()) knobs[0]->slider->setValue(v); }
void SimpleVerbWithPredelayComponent::setSize(float v) { if (knobs.size() > 1) knobs[1]->slider->setValue(v); }
void SimpleVerbWithPredelayComponent::setDamping(float v) { if (knobs.size() > 2) knobs[2]->slider->setValue(v); }
void SimpleVerbWithPredelayComponent::setWet(float v) { if (knobs.size() > 3) knobs[3]->slider->setValue(v); }