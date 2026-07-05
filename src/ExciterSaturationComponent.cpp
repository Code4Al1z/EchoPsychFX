#include "ExciterSaturationComponent.h"

ExciterSaturationComponent::ExciterSaturationComponent(juce::AudioProcessorValueTreeState& state)
    : CollapsibleComponent("Exciter Saturation")
{
    addAndMakeVisible(group);
    PluginLookAndFeel::configureGroup(group);

    knobs.emplace_back(std::make_unique<PluginLookAndFeel::KnobWithLabel>(state, "exciterDrive", "Drive", *this));
    knobs.emplace_back(std::make_unique<PluginLookAndFeel::KnobWithLabel>(state, "exciterMix", "Mix", *this));
    knobs.emplace_back(std::make_unique<PluginLookAndFeel::KnobWithLabel>(state, "exciterHighpass", "Highpass", *this));
}

void ExciterSaturationComponent::paintContent(juce::Graphics& g)
{
    g.fillAll(PluginLookAndFeel::background);
}

void ExciterSaturationComponent::layoutContent(juce::Rectangle<int> area)
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

void ExciterSaturationComponent::setDrive(float v) { if (!knobs.empty()) knobs[0]->slider->setValue(v); }
void ExciterSaturationComponent::setMix(float v) { if (knobs.size() > 1) knobs[1]->slider->setValue(v); }
void ExciterSaturationComponent::setHighpass(float v) { if (knobs.size() > 2) knobs[2]->slider->setValue(v); }