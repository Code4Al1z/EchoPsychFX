#include "PluginProcessor.h"
#include "WidthBalancerComponent.h"

WidthBalancerComponent::WidthBalancerComponent(juce::AudioProcessorValueTreeState& state)
{
    configureSlider(widthSlider, juce::Slider::RotaryHorizontalVerticalDrag);
    configureSlider(midSideSlider, juce::Slider::LinearBar);
    configureSlider(intensitySlider, juce::Slider::RotaryHorizontalVerticalDrag);
    intensitySlider.setSkewFactorFromMidPoint(0.2f);

    configureLabel(widthLabel, "Width");
    configureLabel(midSideLabel, "Mid-Side");
    configureLabel(intensityLabel, "Intensity");

    monoToggle.setButtonText("Mono");
    addAndMakeVisible(monoToggle);

    widthAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(state, "width", widthSlider);
    midSideAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(state, "midSideBalance", midSideSlider);
    intensityAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(state, "intensity", intensitySlider);
    monoAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(state, "mono", monoToggle);
}

void WidthBalancerComponent::configureSlider(juce::Slider& slider, juce::Slider::SliderStyle style)
{
    slider.setSliderStyle(style);
    slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
    addAndMakeVisible(slider);
}

void WidthBalancerComponent::configureLabel(juce::Label& label, const juce::String& text)
{
    label.setText(text, juce::dontSendNotification);
    label.setJustificationType(juce::Justification::centred);
    label.setColour(juce::Label::textColourId, juce::Colours::white);
    addAndMakeVisible(label);
}

void WidthBalancerComponent::resized()
{
    const int knobSize = 100;
    const int margin = 10;

    widthSlider.setBounds(0, 20, knobSize, knobSize);
    widthLabel.setBounds(widthSlider.getX(), 0, knobSize, 20);

    intensitySlider.setBounds(knobSize + margin * 2, 20, knobSize, knobSize);
    intensityLabel.setBounds(intensitySlider.getX(), 0, knobSize, 20);

    midSideLabel.setBounds(210, 20, 80, 20);
    midSideSlider.setBounds(210, 40, getWidth() - 220, 20);

    monoToggle.setBounds(210, 70, getWidth() - 220, 40);
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
