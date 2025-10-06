#include "WidthBalancerComponent.h"

WidthBalancerComponent::WidthBalancerComponent(juce::AudioProcessorValueTreeState& state)
{
    using namespace UIHelpers;

    addAndMakeVisible(group);
    configureGroup(group);

    // Width knob
    widthSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    widthSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
    widthSlider.setColour(juce::Slider::thumbColourId, Colors::knobThumb);
    widthSlider.setColour(juce::Slider::trackColourId, Colors::track);
    widthSlider.setColour(juce::Slider::backgroundColourId, Colors::knobBackground);
    widthSlider.setColour(juce::Slider::rotarySliderFillColourId, Colors::knobFill);
    widthSlider.setColour(juce::Slider::rotarySliderOutlineColourId, Colors::knobOutline);
    addAndMakeVisible(widthSlider);

    // Mid-side slider (linear bar)
    midSideSlider.setSliderStyle(juce::Slider::LinearBar);
    midSideSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
    midSideSlider.setColour(juce::Slider::thumbColourId, Colors::knobThumb);
    midSideSlider.setColour(juce::Slider::trackColourId, Colors::track);
    midSideSlider.setColour(juce::Slider::backgroundColourId, Colors::knobBackground);
    addAndMakeVisible(midSideSlider);

    // Intensity knob
    intensitySlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    intensitySlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
    intensitySlider.setSkewFactorFromMidPoint(0.2f);
    intensitySlider.setColour(juce::Slider::thumbColourId, Colors::knobThumb);
    intensitySlider.setColour(juce::Slider::trackColourId, Colors::track);
    intensitySlider.setColour(juce::Slider::backgroundColourId, Colors::knobBackground);
    intensitySlider.setColour(juce::Slider::rotarySliderFillColourId, Colors::knobFill);
    intensitySlider.setColour(juce::Slider::rotarySliderOutlineColourId, Colors::knobOutline);
    addAndMakeVisible(intensitySlider);

    // Labels
    configureLabel(widthLabel, "Width");
    addAndMakeVisible(widthLabel);

    configureLabel(midSideLabel, "Mid-Side");
    addAndMakeVisible(midSideLabel);

    configureLabel(intensityLabel, "Intensity");
    addAndMakeVisible(intensityLabel);

    // Mono toggle
    monoToggle.setButtonText("Mono");
    monoToggle.setColour(juce::ToggleButton::textColourId, Colors::labelText);
    monoToggle.setColour(juce::ToggleButton::tickColourId, Colors::track);
    addAndMakeVisible(monoToggle);

    // Attachments
    widthAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        state, "width", widthSlider);
    midSideAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        state, "midSideBalance", midSideSlider);
    intensityAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        state, "intensity", intensitySlider);
    monoAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        state, "mono", monoToggle);
}

void WidthBalancerComponent::paint(juce::Graphics& g)
{
    g.fillAll(UIHelpers::Colors::background);
}

void WidthBalancerComponent::resized()
{
    using namespace UIHelpers::Dimensions;

    group.setBounds(getLocalBounds());

    auto area = getLocalBounds().reduced(margin);

    // Width knob
    widthLabel.setBounds(margin, 0, knobSize, labelHeight);
    widthSlider.setBounds(margin, labelHeight, knobSize, knobSize);

    // Intensity knob
    intensityLabel.setBounds(knobSize + margin * 3, 0, knobSize, labelHeight);
    intensitySlider.setBounds(knobSize + margin * 3, labelHeight, knobSize, knobSize);

    // Mid-side slider and mono toggle
    const int rightSectionX = 210;
    midSideLabel.setBounds(rightSectionX, labelHeight, 80, labelHeight);
    midSideSlider.setBounds(rightSectionX, labelHeight + labelHeight, getWidth() - rightSectionX - margin, labelHeight);
    monoToggle.setBounds(rightSectionX, labelHeight + labelHeight * 3, getWidth() - rightSectionX - margin, 40);
}

void WidthBalancerComponent::setWidth(float newValue)
{
    widthSlider.setValue(newValue);
}

void WidthBalancerComponent::setMidSideBalance(float newValue)
{
    midSideSlider.setValue(newValue);
}

void WidthBalancerComponent::setMono(bool newValue)
{
    monoToggle.setToggleState(newValue, juce::sendNotification);
}

void WidthBalancerComponent::setIntensity(float newValue)
{
    intensitySlider.setValue(newValue);
}