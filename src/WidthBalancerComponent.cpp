#include "WidthBalancerComponent.h"

WidthBalancerComponent::WidthBalancerComponent(juce::AudioProcessorValueTreeState& state)
{
    using namespace UIHelpers;

    addAndMakeVisible(group);
    configureGroup(group);

    widthSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    widthSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
    configureKnob(widthSlider);
    addAndMakeVisible(widthSlider);

    midSideSlider.setSliderStyle(juce::Slider::LinearVertical);
    midSideSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
    midSideSlider.setColour(juce::Slider::thumbColourId, Colors::knobThumb);
    midSideSlider.setColour(juce::Slider::trackColourId, Colors::track);
    midSideSlider.setColour(juce::Slider::backgroundColourId, Colors::knobBackground);
    addAndMakeVisible(midSideSlider);

    intensitySlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    intensitySlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
    intensitySlider.setSkewFactorFromMidPoint(0.2f);
    configureKnob(intensitySlider);
    addAndMakeVisible(intensitySlider);

    configureLabel(widthLabel, "Width");
    addAndMakeVisible(widthLabel);

    configureLabel(midSideLabel, "Mid/Side");
    addAndMakeVisible(midSideLabel);

    configureLabel(intensityLabel, "Intensity");
    addAndMakeVisible(intensityLabel);

    monoToggle.setButtonText("Mono");
    monoToggle.setColour(juce::ToggleButton::textColourId, Colors::labelText);
    monoToggle.setColour(juce::ToggleButton::tickColourId, Colors::track);
    addAndMakeVisible(monoToggle);

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
    const int totalKnobHeight = knobSize + labelHeight;

    int x = area.getX();
    const int y = area.getY();

    widthLabel.setBounds(x, y, knobSize, labelHeight);
    widthSlider.setBounds(x, y + labelHeight, knobSize, knobSize);
    x += knobSize + spacing * 2;

    intensityLabel.setBounds(x, y, knobSize, labelHeight);
    intensitySlider.setBounds(x, y + labelHeight, knobSize, knobSize);
    x += knobSize + spacing * 2;

    const int sliderWidth = 60;
    midSideLabel.setBounds(x, y, sliderWidth, labelHeight);
    midSideSlider.setBounds(x + sliderWidth / 2 - 15, y + labelHeight + 5, 30, knobSize - 10);
    x += sliderWidth + spacing * 2;

    monoToggle.setBounds(x, y + labelHeight + knobSize / 2 - 15, 80, 30);
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
