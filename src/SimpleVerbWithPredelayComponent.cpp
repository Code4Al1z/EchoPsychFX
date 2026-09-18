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
    group.setBounds(area);
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

void SimpleVerbWithPredelayComponent::setPredelay(float v) { PluginLookAndFeel::setKnobValue(knobs, 0, v); }
void SimpleVerbWithPredelayComponent::setSize(float v) { PluginLookAndFeel::setKnobValue(knobs, 1, v); }
void SimpleVerbWithPredelayComponent::setDamping(float v) { PluginLookAndFeel::setKnobValue(knobs, 2, v); }
void SimpleVerbWithPredelayComponent::setWet(float v) { PluginLookAndFeel::setKnobValue(knobs, 3, v); }