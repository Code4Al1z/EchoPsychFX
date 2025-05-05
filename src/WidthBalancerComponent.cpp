#include "PluginProcessor.h"
#include "WidthBalancerComponent.h"

WidthBalancerComponent::WidthBalancerComponent(juce::AudioProcessorValueTreeState& state)
{
    widthSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    widthSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
    addAndMakeVisible(widthSlider);

    midSideSlider.setSliderStyle(juce::Slider::LinearBar);
    midSideSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
    addAndMakeVisible(midSideSlider);

    monoToggle.setButtonText("Mono");
    addAndMakeVisible(monoToggle);

    intensitySlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    intensitySlider.setSkewFactorFromMidPoint(0.2f); // Feel: more control in lower values
    intensitySlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
    addAndMakeVisible(intensitySlider);

    widthTextBox.setText("Width");
    widthTextBox.setReadOnly(true); // Prevent the user from editing the text
    widthTextBox.setColour(juce::TextEditor::backgroundColourId, juce::Colours::transparentWhite);
    widthTextBox.setColour(juce::TextEditor::outlineColourId, juce::Colours::transparentWhite); // Remove the outline
    widthTextBox.setColour(juce::TextEditor::textColourId, juce::Colours::black); // Set the text color
    widthTextBox.setJustification(juce::Justification::centred); // Center text
    addAndMakeVisible(widthTextBox);

    midSideTextBox.setText("Mid-Side");
    midSideTextBox.setReadOnly(true);
    midSideTextBox.setColour(juce::TextEditor::backgroundColourId, juce::Colours::transparentWhite);
    midSideTextBox.setColour(juce::TextEditor::outlineColourId, juce::Colours::transparentWhite); // Remove the outline
    midSideTextBox.setColour(juce::TextEditor::textColourId, juce::Colours::black);
    addAndMakeVisible(midSideTextBox);

    intensityTextBox.setText("Intensity");
    intensityTextBox.setReadOnly(true);
    intensityTextBox.setColour(juce::TextEditor::backgroundColourId, juce::Colours::transparentWhite);
    intensityTextBox.setColour(juce::TextEditor::outlineColourId, juce::Colours::transparentWhite);
    intensityTextBox.setColour(juce::TextEditor::textColourId, juce::Colours::black);
    intensityTextBox.setJustification(juce::Justification::centred);
    addAndMakeVisible(intensityTextBox);

    widthAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(state, "width", widthSlider);
    midSideAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(state, "midSideBalance", midSideSlider);
    monoAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(state, "mono", monoToggle);
    intensityAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(state, "intensity", intensitySlider);
}

WidthBalancerComponent::~WidthBalancerComponent()
{
}

void WidthBalancerComponent::resized()
{
    widthSlider.setBounds(0, 20, knobSize, knobSize);
    widthTextBox.setBounds(widthSlider.getX() + widthSlider.getWidth() / 3, widthSlider.getY() + 80, 60, 20);

    midSideSlider.setBounds(210, 40, getWidth() - 220, 20);
    midSideTextBox.setBounds(midSideSlider.getX(), midSideSlider.getY() - 20, 80, 20);

    monoToggle.setBounds(210, 70, getWidth() - 220, 50);

    intensitySlider.setBounds(knobSize + margin * 2, 20, knobSize, knobSize);
    intensityTextBox.setBounds(intensitySlider.getX() + intensitySlider.getWidth() / 3,
        intensitySlider.getY() + 80, 60, 20);

    // Make sure the text boxes are in front of the sliders.
    widthTextBox.toFront(false);
    midSideTextBox.toFront(false);
    intensityTextBox.toFront(false);
}

void WidthBalancerComponent::setWidth(float newValue)
{
    widthSlider.setValue(newValue, juce::dontSendNotification);
}

void WidthBalancerComponent::setMidSideBalance(float newValue)
{
    midSideSlider.setValue(newValue, juce::dontSendNotification);
}

void WidthBalancerComponent::setMono(bool newValue)
{
    monoToggle.setToggleState(newValue, juce::dontSendNotification);
}

void WidthBalancerComponent::setIntensity(float newValue)
{
	intensitySlider.setValue(newValue, juce::dontSendNotification);
}

