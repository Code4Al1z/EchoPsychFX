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

    widthAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(state, "width", widthSlider);

    midSideAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(state, "midSideBalance", midSideSlider);

    monoAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(state, "mono", monoToggle);
}

WidthBalancerComponent::~WidthBalancerComponent()
{
}

void WidthBalancerComponent::resized()
{
    // Place the widthSlider in the left corner, 200x200 pixels
    widthSlider.setBounds(0, 0, knobSize, knobSize);

    // Place the midSideSlider next to it on the right
    midSideSlider.setBounds(210, 40, getWidth() - 220, 20); // 10 pixels gap, 20 pixels high

    // Place the monoToggle below the midSideSlider
    monoToggle.setBounds(210, 70, getWidth() - 220, 50); // 10 pixels gap, 50 pixels high

    // Position the text boxes
    widthTextBox.setBounds(widthSlider.getX() + widthSlider.getWidth() / 3, widthSlider.getY() + 80, 60, 20);
    midSideTextBox.setBounds(midSideSlider.getX(), midSideSlider.getY() - 20, 80, 20);

    // Make sure the text boxes are in front of the sliders.
    widthTextBox.toFront(false);
    midSideTextBox.toFront(false);
}

