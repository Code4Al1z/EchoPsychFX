#include "WidthBalancerComponent.h"

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
    const int availableWidth = area.getWidth();
    const int availableHeight = area.getHeight();

    // --- Fixed sizes for right-side elements ---
    const int midSideSliderWidth = 60;
    const int monoToggleWidth = 80;
    const int fixedRightWidth = midSideSliderWidth + PluginLookAndFeel::spacing * 2 + monoToggleWidth;

    // --- Remaining space for the two main knobs ---
    const int knobCount = 2;
    const int totalSpacingBetweenKnobs = PluginLookAndFeel::spacing * (knobCount + 1);
    const int dynamicWidthAvailable = availableWidth - fixedRightWidth - totalSpacingBetweenKnobs;

    // --- Dynamically calculate knob size ---
    // Each knob gets half of the dynamic area
    const int knobWidth = juce::jmax(40, dynamicWidthAvailable / knobCount);
    const int knobHeight = knobWidth; // keep them square

    const int labelHeight = PluginLookAndFeel::labelHeight;
    const int elementHeight = knobHeight + labelHeight;
    const int y = area.getY() + (availableHeight - elementHeight) / 2;

    int x = area.getX() + PluginLookAndFeel::spacing;

    // --- Width knob ---
    widthLabel.setBounds(x, y, knobWidth, labelHeight);
    widthSlider.setBounds(x, y + labelHeight, knobWidth, knobHeight);
    x += knobWidth + PluginLookAndFeel::spacing;

    // --- Intensity knob ---
    intensityLabel.setBounds(x, y, knobWidth, labelHeight);
    intensitySlider.setBounds(x, y + labelHeight, knobWidth, knobHeight);
    x += knobWidth + PluginLookAndFeel::spacing;

    // --- Mid/Side slider + label ---
    midSideLabel.setBounds(x, y, midSideSliderWidth, labelHeight);
    midSideSlider.setBounds(
        x + midSideSliderWidth / 2 - 15,
        y + labelHeight + 5,
        30,
        knobHeight - 10
    );
    x += midSideSliderWidth + PluginLookAndFeel::spacing;

    // --- Mono toggle ---
    monoToggle.setBounds(
        x,
        y + labelHeight + knobHeight / 2 - 15,
        monoToggleWidth,
        30
    );
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