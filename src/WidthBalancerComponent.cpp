#include "WidthBalancerComponent.h"

WidthBalancerComponent::WidthBalancerComponent(juce::AudioProcessorValueTreeState& state)
{
    addAndMakeVisible(group);
    PluginLookAndFeel::configureGroup(group);

    PluginLookAndFeel::configureKnob(widthSlider);
    widthSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    addAndMakeVisible(widthSlider);

    PluginLookAndFeel::configureKnob(intensitySlider);
    intensitySlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    intensitySlider.setSkewFactorFromMidPoint(0.2f);
    addAndMakeVisible(intensitySlider);

    midSideSlider.setSliderStyle(juce::Slider::LinearVertical);
    midSideSlider.setColour(juce::Slider::thumbColourId, PluginLookAndFeel::knobThumb);
    midSideSlider.setColour(juce::Slider::trackColourId, PluginLookAndFeel::track);
    midSideSlider.setColour(juce::Slider::backgroundColourId, PluginLookAndFeel::knobBackground);
    addAndMakeVisible(midSideSlider);

    PluginLookAndFeel::configureLabel(widthLabel, "Width");
    PluginLookAndFeel::configureLabel(intensityLabel, "Inte...");
    PluginLookAndFeel::configureLabel(midSideLabel, "Mid/Side");
    addAndMakeVisible(widthLabel);
    addAndMakeVisible(intensityLabel);
    addAndMakeVisible(midSideLabel);

    monoToggle.setButtonText("Mono");
    monoToggle.setColour(juce::ToggleButton::textColourId, PluginLookAndFeel::labelText);
    monoToggle.setColour(juce::ToggleButton::tickColourId, PluginLookAndFeel::track);
    addAndMakeVisible(monoToggle);

    widthAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(state, "width", widthSlider);
    midSideAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(state, "midSideBalance", midSideSlider);
    intensityAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(state, "intensity", intensitySlider);
    monoAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(state, "mono", monoToggle);
}

void WidthBalancerComponent::paint(juce::Graphics& g)
{
    g.fillAll(PluginLookAndFeel::background);
}

void WidthBalancerComponent::resized()
{
    if (getWidth() <= 0 || getHeight() <= 0) return;
    group.setBounds(getLocalBounds());

    auto area = getLocalBounds().reduced(PluginLookAndFeel::margin);
    const int w = area.getWidth();
    const int h = area.getHeight();

    // Proportional layout:
    // We have 4 elements: width knob, intensity knob, mid/side slider, mono toggle.
    // The two knobs share roughly half the width each (40% each),
    // mid/side gets ~12%, mono toggle ~8%.
    // All scale with whatever space the editor gives this block.

    const int labelH = juce::jlimit(12, 20, static_cast<int>(h * 0.18f));
    const int knobH = h - labelH - PluginLookAndFeel::margin;
    const int knobW = juce::jlimit(30, static_cast<int>(w * 0.38f), static_cast<int>(w * 0.38f));
    const int midSideW = juce::jlimit(20, static_cast<int>(w * 0.14f), static_cast<int>(w * 0.14f));
    const int monoW = w - knobW * 2 - midSideW - PluginLookAndFeel::spacing * 3;

    const int y = area.getY();
    int x = area.getX();

    // Width knob
    widthLabel.setBounds(x, y, knobW, labelH);
    widthSlider.setBounds(x, y + labelH, knobW, knobH);
    widthSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false,
        juce::jlimit(30, 60, knobW - 4), juce::jlimit(12, 18, static_cast<int>(knobH * 0.2f)));
    x += knobW + PluginLookAndFeel::spacing;

    // Intensity knob
    intensityLabel.setBounds(x, y, knobW, labelH);
    intensitySlider.setBounds(x, y + labelH, knobW, knobH);
    intensitySlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false,
        juce::jlimit(30, 60, knobW - 4), juce::jlimit(12, 18, static_cast<int>(knobH * 0.2f)));
    x += knobW + PluginLookAndFeel::spacing;

    // Mid/Side vertical slider
    midSideLabel.setBounds(x, y, midSideW, labelH);
    midSideSlider.setBounds(x, y + labelH, midSideW, knobH);
    midSideSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false,
        midSideW, juce::jlimit(12, 18, static_cast<int>(knobH * 0.2f)));
    x += midSideW + PluginLookAndFeel::spacing;

    // Mono toggle — vertically centred in remaining space
    if (monoW > 20)
    {
        const int toggleH = juce::jlimit(18, 28, static_cast<int>(h * 0.25f));
        monoToggle.setBounds(x, y + (h - toggleH) / 2, monoW, toggleH);
        monoToggle.setVisible(true);
    }
    else
    {
        monoToggle.setVisible(false);
    }
}

void WidthBalancerComponent::setWidth(float v) { widthSlider.setValue(v); }
void WidthBalancerComponent::setMidSideBalance(float v) { midSideSlider.setValue(v); }
void WidthBalancerComponent::setMono(bool v) { monoToggle.setToggleState(v, juce::sendNotification); }
void WidthBalancerComponent::setIntensity(float v) { intensitySlider.setValue(v); }