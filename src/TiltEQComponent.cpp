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
    if (getWidth() <= 0 || getHeight() <= 0) return;
    group.setBounds(getLocalBounds());

    auto area = getLocalBounds().reduced(PluginLookAndFeel::margin);
    const int numKnobs = static_cast<int>(knobs.size());

    auto layout = PluginLookAndFeel::calculateKnobLayout(numKnobs, area.getWidth(), area.getHeight(), false);
    if ((int)layout.knobBounds.size() < numKnobs) return;

    for (int i = 0; i < numKnobs; ++i)
    {
        auto b = layout.knobBounds[i];
        knobs[i]->setBounds(area.getX() + b.getX(), area.getY() + b.getY(), b.getWidth(), b.getHeight());
    }
}

void TiltEQComponent::setTilt(float v)
{
    if (!knobs.empty()) knobs[0]->slider->setValue(v);
}