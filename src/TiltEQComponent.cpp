#include "TiltEQComponent.h"

TiltEQComponent::TiltEQComponent(juce::AudioProcessorValueTreeState& state)
{
    addAndMakeVisible(group);
    PluginLookAndFeel::configureGroup(group);

    knobs.emplace_back(std::make_unique<PluginLookAndFeel::KnobWithLabel>(state, "tiltEQ", "Tilt EQ", *this));

    if (!knobs.empty() && knobs[0]->slider)
    {
        knobs[0]->slider->setRange(-1.0, 1.0, 0.01);
        knobs[0]->slider->setSkewFactorFromMidPoint(0.0);
    }
}

void TiltEQComponent::paint(juce::Graphics& g)
{
    g.fillAll(PluginLookAndFeel::background);
}

void TiltEQComponent::resized()
{
    group.setBounds(getLocalBounds());

    auto area = getLocalBounds().reduced(PluginLookAndFeel::margin);
    const int numKnobs = static_cast<int>(knobs.size());

    auto layout = PluginLookAndFeel::calculateKnobLayout(numKnobs, area.getWidth(), area.getHeight(), false);

    for (int i = 0; i < numKnobs; ++i)
    {
        if (i >= static_cast<int>(layout.knobBounds.size()))
            break;

        auto bounds = layout.knobBounds[i];
        bounds.translate(area.getX(), area.getY());
        knobs[i]->setBounds(bounds.getX(), bounds.getY(), bounds.getWidth(), bounds.getHeight());
    }
}

void TiltEQComponent::setTilt(float newValue)
{
    if (!knobs.empty())
        knobs[0]->slider->setValue(newValue);
}