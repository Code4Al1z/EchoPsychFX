#include "TiltEQComponent.h"

TiltEQComponent::TiltEQComponent(juce::AudioProcessorValueTreeState& state)
{
    using namespace UIHelpers;

    addAndMakeVisible(group);
    configureGroup(group);

    tiltSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    tiltSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
    tiltSlider.setRange(-1.0, 1.0, 0.01);
    tiltSlider.setSkewFactorFromMidPoint(0.0);
    configureKnob(tiltSlider);
    addAndMakeVisible(tiltSlider);

    configureLabel(tiltLabel, "Tilt EQ");
    addAndMakeVisible(tiltLabel);

    tiltAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        state, "tiltEQ", tiltSlider);
}

void TiltEQComponent::paint(juce::Graphics& g)
{
    g.fillAll(UIHelpers::Colors::background);
}

void TiltEQComponent::resized()
{
    using namespace UIHelpers::Dimensions;

    group.setBounds(getLocalBounds());

    auto area = getLocalBounds().reduced(margin);
    const int x = area.getX() + (area.getWidth() - knobSize) / 2;
    const int y = area.getY();

    tiltLabel.setBounds(x, y, knobSize, labelHeight);
    tiltSlider.setBounds(x, y + labelHeight, knobSize, knobSize);
}

void TiltEQComponent::setTilt(float newValue)
{
    tiltSlider.setValue(newValue);
}
