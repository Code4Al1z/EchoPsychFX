#include "PluginProcessor.h"
#include "TiltEQComponent.h"

TiltEQComponent::TiltEQComponent(juce::AudioProcessorValueTreeState& state)
{
    addAndMakeVisible(group);
    group.setColour(juce::GroupComponent::outlineColourId, juce::Colours::white.withAlpha(0.4f));
    group.setColour(juce::GroupComponent::textColourId, juce::Colours::white);

    tiltSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    tiltSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 20);
    tiltSlider.setRange(-1.0, 1.0, 0.01);
    tiltSlider.setSkewFactorFromMidPoint(0.0); // "Flat" in the center

    // Colour the knob (thumb), filled track, and background
    tiltSlider.setColour(juce::Slider::thumbColourId, juce::Colour(255, 111, 41));                   // knob
    tiltSlider.setColour(juce::Slider::trackColourId, juce::Colours::deeppink);                 // filled portion
    tiltSlider.setColour(juce::Slider::backgroundColourId, juce::Colour(123, 0, 70));       // background track
    tiltSlider.setColour(juce::Slider::rotarySliderFillColourId, juce::Colours::deeppink);      // for rotary fill
    tiltSlider.setColour(juce::Slider::rotarySliderOutlineColourId, juce::Colour(90, 0, 50));      // rotary outline

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

void TiltEQComponent::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(31, 31, 31));
}

void TiltEQComponent::resized()
{
    group.setBounds(getLocalBounds());

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
	tiltSlider.setValue(newValue);
}

