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

    midSideSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    midSideSlider.setColour(juce::Slider::thumbColourId, PluginLookAndFeel::knobThumb);
    midSideSlider.setColour(juce::Slider::trackColourId, PluginLookAndFeel::track);
    midSideSlider.setColour(juce::Slider::backgroundColourId, PluginLookAndFeel::knobBackground);
    addAndMakeVisible(midSideSlider);

    PluginLookAndFeel::configureLabel(widthLabel, "Width");
    PluginLookAndFeel::configureLabel(intensityLabel, "Intensity");
    PluginLookAndFeel::configureLabel(midSideLabel, "Mid/Side");
    addAndMakeVisible(widthLabel);
    addAndMakeVisible(intensityLabel);
    addAndMakeVisible(midSideLabel);

    // Value label for Mid/Side — right-aligned, updates with slider
    midSideValueLabel.setJustificationType(juce::Justification::centredRight);
    midSideValueLabel.setColour(juce::Label::textColourId, PluginLookAndFeel::labelText);
    midSideValueLabel.setText("0.00", juce::dontSendNotification);
    addAndMakeVisible(midSideValueLabel);

    // 2 decimal places on all sliders
    widthSlider.setNumDecimalPlacesToDisplay(2);
    intensitySlider.setNumDecimalPlacesToDisplay(2);
    midSideSlider.setNumDecimalPlacesToDisplay(2);

    // Keep value label in sync — no separate listener needed
    midSideSlider.onValueChange = [this]()
        {
            midSideValueLabel.setText(
                juce::String(midSideSlider.getValue(), 2),
                juce::dontSendNotification);
        };

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
    const int gap = PluginLookAndFeel::spacing;

    const int labelH = 16;  // fixed label/value row height
    const int trackH = juce::jmax(18, static_cast<int>(h * 0.16f));
    const int monoW = juce::jlimit(40, 60, static_cast<int>(w * 0.20f));
    const int sliderW = w - monoW - gap;

    int x = area.getX();
    int y = area.getY();

    // ── Mid/Side: label top-left, value top-right, track below ───────────────
    const int valueW = 44; // fixed width for the value label on the right
    midSideLabel.setBounds(x, y, sliderW - valueW, labelH);
    midSideValueLabel.setBounds(x + sliderW - valueW, y, valueW, labelH);
    y += labelH;

    // Slider track with no built-in text box (label and value handled above)
    midSideSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    midSideSlider.setBounds(x, y, sliderW, trackH);

    // Mono toggle — centred vertically alongside the label+track block
    const int monoBlockH = labelH + trackH;
    monoToggle.setBounds(x + sliderW + gap,
        area.getY() + (monoBlockH - 22) / 2,
        monoW, 22);

    y += trackH + gap;

    // ── Width and Intensity rotary knobs ──────────────────────────────────────
    const int knobAreaH = area.getBottom() - y;
    const int knobW = (w - gap) / 2;
    const int knobLabelH = labelH;
    const int rotaryH = knobAreaH - knobLabelH;
    const int tbW = juce::jlimit(28, 56, static_cast<int>(knobW * 0.8f));
    const int tbH = juce::jlimit(12, 18, static_cast<int>(rotaryH * 0.2f));

    widthLabel.setBounds(area.getX(), y, knobW, knobLabelH);
    widthLabel.setJustificationType(juce::Justification::centred);
    widthSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, tbW, tbH);
    widthSlider.setBounds(area.getX(), y + knobLabelH, knobW, rotaryH);

    intensityLabel.setBounds(area.getX() + knobW + gap, y, knobW, knobLabelH);
    intensityLabel.setJustificationType(juce::Justification::centred);
    intensitySlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, tbW, tbH);
    intensitySlider.setBounds(area.getX() + knobW + gap, y + knobLabelH, knobW, rotaryH);

    widthSlider.setVisible(true);
    intensitySlider.setVisible(true);
    midSideSlider.setVisible(true);
    widthLabel.setVisible(true);
    intensityLabel.setVisible(true);
    midSideLabel.setVisible(true);
    midSideValueLabel.setVisible(true);
    monoToggle.setVisible(true);
}

void WidthBalancerComponent::setWidth(float v) { widthSlider.setValue(v); }
void WidthBalancerComponent::setMidSideBalance(float v) { midSideSlider.setValue(v); }
void WidthBalancerComponent::setMono(bool v) { monoToggle.setToggleState(v, juce::sendNotification); }
void WidthBalancerComponent::setIntensity(float v) { intensitySlider.setValue(v); }