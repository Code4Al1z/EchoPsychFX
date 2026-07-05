#include "MicroPitchDetuneComponent.h"

MicroPitchDetuneComponent::MicroPitchDetuneComponent(juce::AudioProcessorValueTreeState& state)
    : CollapsibleComponent("Micro-Pitch Detune")
{
    addAndMakeVisible(group);
    PluginLookAndFeel::configureGroup(group);

    knobs.emplace_back(std::make_unique<PluginLookAndFeel::KnobWithLabel>(state, "detuneAmount", "Detune", *this));
    knobs.emplace_back(std::make_unique<PluginLookAndFeel::KnobWithLabel>(state, "lfoRate", "LFO Rate", *this));
    knobs.emplace_back(std::make_unique<PluginLookAndFeel::KnobWithLabel>(state, "lfoDepth", "LFO Depth", *this));
    knobs.emplace_back(std::make_unique<PluginLookAndFeel::KnobWithLabel>(state, "delayCentre", "Delay", *this));
    knobs.emplace_back(std::make_unique<PluginLookAndFeel::KnobWithLabel>(state, "stereoSeparation", "Stereo", *this));
    knobs.emplace_back(std::make_unique<PluginLookAndFeel::KnobWithLabel>(state, "mix", "Mix", *this));
}

void MicroPitchDetuneComponent::paintContent(juce::Graphics& g)
{
    g.fillAll(PluginLookAndFeel::background);
}

void MicroPitchDetuneComponent::layoutContent(juce::Rectangle<int> area)
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

void MicroPitchDetuneComponent::setDetuneAmount(float v) { if (!knobs.empty()) knobs[0]->slider->setValue(v); }
void MicroPitchDetuneComponent::setLfoRate(float v) { if (knobs.size() > 1) knobs[1]->slider->setValue(v); }
void MicroPitchDetuneComponent::setLfoDepth(float v) { if (knobs.size() > 2) knobs[2]->slider->setValue(v); }
void MicroPitchDetuneComponent::setDelayCentre(float v) { if (knobs.size() > 3) knobs[3]->slider->setValue(v); }
void MicroPitchDetuneComponent::setStereoSeparation(float v) { if (knobs.size() > 4) knobs[4]->slider->setValue(v); }
void MicroPitchDetuneComponent::setMix(float v) { if (knobs.size() > 5) knobs[5]->slider->setValue(v); }