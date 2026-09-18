#include "WidthBalancerComponent.h"

WidthBalancerComponent::WidthBalancerComponent(juce::AudioProcessorValueTreeState& state)
{
    addAndMakeVisible(group);
    PluginLookAndFeel::configureGroup(group);

    widthKnob = std::make_unique<PluginLookAndFeel::KnobWithLabel>(state, "width", "Width", *this);
    intensityKnob = std::make_unique<PluginLookAndFeel::KnobWithLabel>(state, "intensity", "Intensity", *this);

    if (intensityKnob->slider)
        intensityKnob->slider->setSkewFactorFromMidPoint(0.2f);

    if (widthKnob->slider)
        widthKnob->slider->setNumDecimalPlacesToDisplay(2);
    if (intensityKnob->slider)
        intensityKnob->slider->setNumDecimalPlacesToDisplay(2);

    midSideSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    midSideSlider.setColour(juce::Slider::thumbColourId, PluginLookAndFeel::knobThumb);
    midSideSlider.setColour(juce::Slider::trackColourId, PluginLookAndFeel::track);
    midSideSlider.setColour(juce::Slider::backgroundColourId, PluginLookAndFeel::knobBackground);
    addAndMakeVisible(midSideSlider);

    PluginLookAndFeel::configureLabel(midSideLabel, "Mid/Side");
    addAndMakeVisible(midSideLabel);

    midSideValueLabel.setJustificationType(juce::Justification::centredRight);
    midSideValueLabel.setColour(juce::Label::textColourId, PluginLookAndFeel::labelText);
    midSideValueLabel.setText("0.00", juce::dontSendNotification);
    addAndMakeVisible(midSideValueLabel);

    midSideSlider.setNumDecimalPlacesToDisplay(2);

    midSideSlider.onValueChange = [this]()
        {
            midSideValueLabel.setText(juce::String(midSideSlider.getValue(), 2),
                juce::dontSendNotification);
        };

    monoToggle.setButtonText("Mono");
    monoToggle.setColour(juce::ToggleButton::textColourId, PluginLookAndFeel::labelText);
    monoToggle.setColour(juce::ToggleButton::tickColourId, PluginLookAndFeel::track);
    addAndMakeVisible(monoToggle);

    midSideAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(state, "midSideBalance", midSideSlider);
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
    area.removeFromTop(PluginLookAndFeel::groupLabelHeight);
    const int w = area.getWidth();
    const int h = area.getHeight();
    const int gap = PluginLookAndFeel::spacing;

    const int labelH = 16;
    const int trackH = juce::jmax(18, static_cast<int>(h * 0.16f));
    const int monoW = juce::jlimit(40, 60, static_cast<int>(w * 0.20f));
    const int sliderW = w - monoW - gap;
    const int valueW = 44;

    int x = area.getX();
    int y = area.getY();

    midSideLabel.setBounds(x, y, sliderW - valueW, labelH);
    midSideValueLabel.setBounds(x + sliderW - valueW, y, valueW, labelH);
    y += labelH;

    midSideSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    midSideSlider.setBounds(x, y, sliderW, trackH);

    const int monoBlockH = labelH + trackH;
    monoToggle.setBounds(x + sliderW + gap, area.getY() + (monoBlockH - 22) / 2, monoW, 22);
    y += trackH + gap;

    const int knobAreaH = area.getBottom() - y;
    const int knobW = (w - gap) / 2;

    widthKnob->setBounds(area.getX(), y, knobW, knobAreaH);
    intensityKnob->setBounds(area.getX() + knobW + gap, y, knobW, knobAreaH);

    midSideSlider.setVisible(true);
    midSideLabel.setVisible(true);
    midSideValueLabel.setVisible(true);
    monoToggle.setVisible(true);
}

void WidthBalancerComponent::setWidth(float v) { if (widthKnob && widthKnob->slider) widthKnob->slider->setValue(v); }
void WidthBalancerComponent::setMidSideBalance(float v) { midSideSlider.setValue(v); }
void WidthBalancerComponent::setMono(bool v) { monoToggle.setToggleState(v, juce::sendNotification); }
void WidthBalancerComponent::setIntensity(float v) { if (intensityKnob && intensityKnob->slider) intensityKnob->slider->setValue(v); }