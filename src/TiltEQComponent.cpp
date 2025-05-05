#include "PluginProcessor.h"
#include "TiltEQComponent.h"

TiltEQComponent::TiltEQComponent(juce::AudioProcessorValueTreeState& state)
{
    tiltSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    tiltSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 20);
    tiltSlider.setRange(-1.0, 1.0, 0.01);
    tiltSlider.setSkewFactorFromMidPoint(0.0); // "Flat" in the center
    addAndMakeVisible(tiltSlider);

    tiltLabel.setText("Tilt", juce::dontSendNotification);
    tiltLabel.setJustificationType(juce::Justification::centred);
    tiltLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    addAndMakeVisible(tiltLabel);

    tiltAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(state, "tiltEQ", tiltSlider);
}

TiltEQComponent::~TiltEQComponent()
{
}

void TiltEQComponent::resized()
{
    auto area = getLocalBounds().reduced(margin);

    // Left-aligned knob
    auto knobArea = area.removeFromLeft(knobSize);

    // Position the slider
    tiltSlider.setBounds(knobArea.removeFromTop(knobSize));

    // Text label right below the slider
    auto textBoxHeight = 24;
    tiltLabel.setBounds(tiltSlider.getX(), -5, knobSize, textBoxHeight);
}

void TiltEQComponent::setTilt(float newValue)
{
	tiltSlider.setValue(newValue, juce::dontSendNotification);
}

