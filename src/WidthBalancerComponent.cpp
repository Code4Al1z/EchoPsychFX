#include "WidthBalancerComponent.h"

WidthBalancerComponent::WidthBalancerComponent(juce::AudioProcessorValueTreeState& state)
{
    using namespace UIHelpers;

    addAndMakeVisible(group);
    configureGroup(group);

    // Width knob
    widthSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    widthSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
    configureKnob(widthSlider);
    addAndMakeVisible(widthSlider);

    // Mid-side balance (vertical slider for better space usage)
    midSideSlider.setSliderStyle(juce::Slider::LinearVertical);
    midSideSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
    midSideSlider.setColour(juce::Slider::thumbColourId, Colors::knobThumb);
    midSideSlider.setColour(juce::Slider::trackColourId, Colors::track);
    midSideSlider.setColour(juce::Slider::backgroundColourId, Colors::knobBackground);
    addAndMakeVisible(midSideSlider);

    // Intensity knob
    intensitySlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    intensitySlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
    intensitySlider.setSkewFactorFromMidPoint(0.2f);
    configureKnob(intensitySlider);
    addAndMakeVisible(intensitySlider);

    // Labels
    configureLabel(widthLabel, "Width");
    addAndMakeVisible(widthLabel);

    configureLabel(midSideLabel, "Mid/Side");
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
    const int availableWidth = area.getWidth();
    const int availableHeight = area.getHeight();

    // Adaptive layout based on available space
    const bool useCompactLayout = availableWidth < 400 || availableHeight < 110;

    if (useCompactLayout)
    {
        // Horizontal compact layout
        const int knobSize = juce::jmin(80, availableHeight - 30);
        const int sliderWidth = juce::jmax(40, (availableWidth - knobSize * 2 - margin * 4) / 2);

        int x = area.getX() + margin;
        const int y = area.getY() + labelHeight + 5;

        // Width knob
        widthLabel.setBounds(x, area.getY(), knobSize, labelHeight);
        widthSlider.setBounds(x, y, knobSize, knobSize);
        x += knobSize + margin * 2;

        // Mid/Side slider (vertical)
        midSideLabel.setBounds(x, area.getY(), sliderWidth, labelHeight);
        midSideSlider.setBounds(x + sliderWidth / 2 - 15, y, 30, knobSize);
        x += sliderWidth + margin * 2;

        // Intensity knob
        intensityLabel.setBounds(x, area.getY(), knobSize, labelHeight);
        intensitySlider.setBounds(x, y, knobSize, knobSize);
        x += knobSize + margin * 2;

        // Mono toggle
        monoToggle.setBounds(x, y + knobSize / 2 - 15, juce::jmin(80, area.getRight() - x - margin), 30);
    }
    else
    {
        // Standard layout with more space
        const int knobSize = juce::jmin(100, availableHeight - 35);
        int x = area.getX() + margin;
        const int y = area.getY() + labelHeight;

        // Width knob
        widthLabel.setBounds(x, area.getY(), knobSize, labelHeight);
        widthSlider.setBounds(x, y, knobSize, knobSize);
        x += knobSize + margin * 3;

        // Intensity knob
        intensityLabel.setBounds(x, area.getY(), knobSize, labelHeight);
        intensitySlider.setBounds(x, y, knobSize, knobSize);
        x += knobSize + margin * 3;

        // Mid-side slider and mono toggle in remaining space
        const int remainingWidth = area.getRight() - x - margin;
        const int sliderWidth = juce::jmax(60, remainingWidth / 2);

        midSideLabel.setBounds(x, y, sliderWidth, labelHeight);
        midSideSlider.setBounds(x + sliderWidth / 2 - 15, y + labelHeight + 5, 30, knobSize - labelHeight - 10);

        monoToggle.setBounds(x, y + knobSize - 35, remainingWidth, 30);
    }
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