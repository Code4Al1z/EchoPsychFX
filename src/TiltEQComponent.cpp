#include "TiltEQComponent.h"

TiltEQComponent::TiltEQComponent(juce::AudioProcessorValueTreeState& state)
{
    using namespace UIHelpers;

    addAndMakeVisible(group);
    configureGroup(group);

    tiltSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    tiltSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 20);
    tiltSlider.setRange(-1.0, 1.0, 0.01);
    tiltSlider.setSkewFactorFromMidPoint(0.0);

    tiltSlider.setColour(juce::Slider::thumbColourId, Colors::knobThumb);
    tiltSlider.setColour(juce::Slider::trackColourId, Colors::track);
    tiltSlider.setColour(juce::Slider::backgroundColourId, Colors::knobBackground);
    tiltSlider.setColour(juce::Slider::rotarySliderFillColourId, Colors::knobFill);
    tiltSlider.setColour(juce::Slider::rotarySliderOutlineColourId, Colors::knobOutline);
    addAndMakeVisible(tiltSlider);

    configureLabel(tiltLabel, "Tilt");
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
    auto knobArea = area.removeFromLeft(120);

    tiltSlider.setBounds(knobArea.removeFromTop(120));
    tiltLabel.setBounds(tiltSlider.getX(), 0, 120, labelHeight);
}

void TiltEQComponent::setTilt(float newValue)
{
    tiltSlider.setValue(newValue);
}