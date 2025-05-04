#include "PluginProcessor.h"
#include "SpatialFXComponent.h"

SpatialFXComponent::SpatialFXComponent(juce::AudioProcessorValueTreeState& state)
{
    // Phase Offset Left
    phaseOffsetLeftSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    phaseOffsetLeftSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
    addAndMakeVisible(phaseOffsetLeftSlider);
    phaseOffsetLeftTextBox.setText("Phase L Offset");
    phaseOffsetLeftTextBox.setReadOnly(true);
    phaseOffsetLeftTextBox.setColour(juce::TextEditor::backgroundColourId, juce::Colours::transparentWhite);
    phaseOffsetLeftTextBox.setColour(juce::TextEditor::outlineColourId, juce::Colours::transparentWhite);
    phaseOffsetLeftTextBox.setColour(juce::TextEditor::textColourId, juce::Colours::black);
    phaseOffsetLeftTextBox.setJustification(juce::Justification::centred);
    addAndMakeVisible(phaseOffsetLeftTextBox);
    phaseOffsetLeftAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(state, "phaseOffsetL", phaseOffsetLeftSlider);

    // Phase Offset Right
    phaseOffsetRightSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    phaseOffsetRightSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
    addAndMakeVisible(phaseOffsetRightSlider);
    phaseOffsetRightTextBox.setText("Phase R Offset");
    phaseOffsetRightTextBox.setReadOnly(true);
    phaseOffsetRightTextBox.setColour(juce::TextEditor::backgroundColourId, juce::Colours::transparentWhite);
    phaseOffsetRightTextBox.setColour(juce::TextEditor::outlineColourId, juce::Colours::transparentWhite);
    phaseOffsetRightTextBox.setColour(juce::TextEditor::textColourId, juce::Colours::black);
    phaseOffsetRightTextBox.setJustification(juce::Justification::centred);
    addAndMakeVisible(phaseOffsetRightTextBox);
    phaseOffsetRightAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(state, "phaseOffsetR", phaseOffsetRightSlider);

    // Modulation Rate (SFX)
    modulationRateSliderSFX.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    modulationRateSliderSFX.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
    modulationRateSliderSFX.setRange(0.0f, 10.0f, 0.01f); // Example range
    addAndMakeVisible(modulationRateSliderSFX);
    modulationRateTextBoxSFX.setText("SFX Rate");
    modulationRateTextBoxSFX.setReadOnly(true);
    modulationRateTextBoxSFX.setColour(juce::TextEditor::backgroundColourId, juce::Colours::transparentWhite);
    modulationRateTextBoxSFX.setColour(juce::TextEditor::outlineColourId, juce::Colours::transparentWhite);
    modulationRateTextBoxSFX.setColour(juce::TextEditor::textColourId, juce::Colours::black);
    modulationRateTextBoxSFX.setJustification(juce::Justification::centred);
    addAndMakeVisible(modulationRateTextBoxSFX);
    modulationRateAttachmentSFX = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(state, "sfxModRate", modulationRateSliderSFX);

    // Modulation Depth (SFX)
    modulationDepthSliderSFX.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    modulationDepthSliderSFX.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
    modulationDepthSliderSFX.setRange(0.0f, 1.0f, 0.01f); // Example range
    addAndMakeVisible(modulationDepthSliderSFX);
    modulationDepthTextBoxSFX.setText("SFX Depth");
    modulationDepthTextBoxSFX.setReadOnly(true);
    modulationDepthTextBoxSFX.setColour(juce::TextEditor::backgroundColourId, juce::Colours::transparentWhite);
    modulationDepthTextBoxSFX.setColour(juce::TextEditor::outlineColourId, juce::Colours::transparentWhite);
    modulationDepthTextBoxSFX.setColour(juce::TextEditor::textColourId, juce::Colours::black);
    modulationDepthTextBoxSFX.setJustification(juce::Justification::centred);
    addAndMakeVisible(modulationDepthTextBoxSFX);
    modulationDepthAttachmentSFX = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(state, "sfxModDepth", modulationDepthSliderSFX);

    // Wet/Dry Mix (SFX)
    wetDrySliderSFX.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    wetDrySliderSFX.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
    wetDrySliderSFX.setRange(0.0f, 1.0f, 0.01f);
    addAndMakeVisible(wetDrySliderSFX);

    wetDryTextBoxSFX.setText("SFX Wet/Dry");
    wetDryTextBoxSFX.setReadOnly(true);
    wetDryTextBoxSFX.setColour(juce::TextEditor::backgroundColourId, juce::Colours::transparentWhite);
    wetDryTextBoxSFX.setColour(juce::TextEditor::outlineColourId, juce::Colours::transparentWhite);
    wetDryTextBoxSFX.setColour(juce::TextEditor::textColourId, juce::Colours::black);
    wetDryTextBoxSFX.setJustification(juce::Justification::centred);
    addAndMakeVisible(wetDryTextBoxSFX);

    wetDryAttachmentSFX = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(state, "sfxWetDryMix", wetDrySliderSFX);
}

SpatialFXComponent::~SpatialFXComponent()
{
}

void SpatialFXComponent::resized()
{
    auto area = getLocalBounds().reduced(margin); // Get available space
    int x = area.getX();
    int y = area.getY();
    int textBoxYOffset = 80;

    phaseOffsetLeftSlider.setBounds(x, y, knobSize, knobSize);
    phaseOffsetLeftTextBox.setBounds(x + knobSize / 10, y + textBoxYOffset, 100, 20);
    x += knobSize + margin;

    phaseOffsetRightSlider.setBounds(x, y, knobSize, knobSize);
    phaseOffsetRightTextBox.setBounds(x + knobSize / 10, y + textBoxYOffset, 100, 20);
    x += knobSize + margin;

    modulationRateSliderSFX.setBounds(x, y, knobSize, knobSize);
    modulationRateTextBoxSFX.setBounds(x + knobSize / 10, y + textBoxYOffset, 100, 20);
    x += knobSize + margin;

    modulationDepthSliderSFX.setBounds(x, y, knobSize, knobSize);
    modulationDepthTextBoxSFX.setBounds(x + knobSize / 10, y + textBoxYOffset, 100, 20);
    x += knobSize + margin;

    wetDrySliderSFX.setBounds(x, y, knobSize, knobSize);
    wetDryTextBoxSFX.setBounds(x + knobSize / 10, y + textBoxYOffset, 100, 20);
}

