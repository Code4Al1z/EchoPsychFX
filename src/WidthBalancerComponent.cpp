#include "WidthBalancerComponent.h"
#include "PluginLookAndFeel.h"

WidthBalancerComponent::WidthBalancerComponent(juce::AudioProcessorValueTreeState& state)
{
    addAndMakeVisible(group);
    PluginLookAndFeel::configureGroup(group);

    widthSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    widthSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
    PluginLookAndFeel::configureKnob(widthSlider);
    addAndMakeVisible(widthSlider);

    midSideSlider.setSliderStyle(juce::Slider::LinearVertical);
    midSideSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
    midSideSlider.setColour(juce::Slider::thumbColourId, PluginLookAndFeel::knobThumb);
    midSideSlider.setColour(juce::Slider::trackColourId, PluginLookAndFeel::track);
    midSideSlider.setColour(juce::Slider::backgroundColourId, PluginLookAndFeel::knobBackground);
    addAndMakeVisible(midSideSlider);

    intensitySlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    intensitySlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
    intensitySlider.setSkewFactorFromMidPoint(0.2f);
    PluginLookAndFeel::configureKnob(intensitySlider);
    addAndMakeVisible(intensitySlider);

    PluginLookAndFeel::configureLabel(widthLabel, "Width");
    addAndMakeVisible(widthLabel);

    PluginLookAndFeel::configureLabel(midSideLabel, "Mid/Side");
    addAndMakeVisible(midSideLabel);

    PluginLookAndFeel::configureLabel(intensityLabel, "Intensity");
    addAndMakeVisible(intensityLabel);

    monoToggle.setButtonText("Mono");
    monoToggle.setColour(juce::ToggleButton::textColourId, PluginLookAndFeel::labelText);
    monoToggle.setColour(juce::ToggleButton::tickColourId, PluginLookAndFeel::track);
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
    g.fillAll(PluginLookAndFeel::background);
}

void WidthBalancerComponent::resized()
{
    group.setBounds(getLocalBounds());

    auto area = getLocalBounds().reduced(PluginLookAndFeel::margin);
    const int totalKnobHeight = PluginLookAndFeel::knobSize + PluginLookAndFeel::labelHeight;

    int x = area.getX();
    const int y = area.getY();

    widthLabel.setBounds(x, y, PluginLookAndFeel::knobSize, PluginLookAndFeel::labelHeight);
    widthSlider.setBounds(x, y + PluginLookAndFeel::labelHeight, PluginLookAndFeel::knobSize, PluginLookAndFeel::knobSize);
    x += PluginLookAndFeel::knobSize + PluginLookAndFeel::spacing * 2;

    intensityLabel.setBounds(x, y, PluginLookAndFeel::knobSize, PluginLookAndFeel::labelHeight);
    intensitySlider.setBounds(x, y + PluginLookAndFeel::labelHeight, PluginLookAndFeel::knobSize, PluginLookAndFeel::knobSize);
    x += PluginLookAndFeel::knobSize + PluginLookAndFeel::spacing * 2;

    const int sliderWidth = 60;
    midSideLabel.setBounds(x, y, sliderWidth, PluginLookAndFeel::labelHeight);
    midSideSlider.setBounds(x + sliderWidth / 2 - 15, y + PluginLookAndFeel::labelHeight + 5, 30, PluginLookAndFeel::knobSize - 10);
    x += sliderWidth + PluginLookAndFeel::spacing * 2;

    monoToggle.setBounds(x, y + PluginLookAndFeel::labelHeight + PluginLookAndFeel::knobSize / 2 - 15, 80, 30);
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