#include "TiltEQComponent.h"

TiltEQComponent::TiltEQComponent(juce::AudioProcessorValueTreeState& state)
{
    addAndMakeVisible(group);
    PluginLookAndFeel::configureGroup(group);

    knobs.emplace_back(std::make_unique<PluginLookAndFeel::KnobWithLabel>(state, "tiltEQ", "Tilt EQ", *this));

    // A bipolar tilt reads best as a wide slider that fills outward from the centre detent.
    if (!knobs.empty() && knobs[0]->slider)
    {
        knobs[0]->slider->setSliderStyle(juce::Slider::LinearHorizontal);
        knobs[0]->slider->setTextBoxStyle(juce::Slider::TextBoxBelow, false, 90, 20);
        knobs[0]->label->setText("Dark  <  Tilt  >  Bright", juce::dontSendNotification);
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
    area.removeFromTop(PluginLookAndFeel::groupLabelHeight);
    if (!knobs.empty())
        knobs[0]->setBounds(area.getX() + 6, area.getY(), area.getWidth() - 12, area.getHeight());
}

void TiltEQComponent::setTilt(float v)
{
    PluginLookAndFeel::setKnobValue(knobs, 0, v);
}