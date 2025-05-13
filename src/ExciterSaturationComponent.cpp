#include "ExciterSaturationComponent.h"

ExciterSaturationComponent::ExciterSaturationComponent(juce::AudioProcessorValueTreeState& state)
{
    addAndMakeVisible(group);
    group.setColour(juce::GroupComponent::outlineColourId, juce::Colours::white.withAlpha(0.4f));
    group.setColour(juce::GroupComponent::textColourId, juce::Colours::white);

    configureKnob(drive, "exciterDrive", "Drive", state);
    configureKnob(mix, "exciterMix", "Mix", state);
    configureKnob(highpass, "exciterHighpass", "Highpass", state);
}

ExciterSaturationComponent::~ExciterSaturationComponent() {}

void ExciterSaturationComponent::configureKnob(KnobWithLabel& kwl, const juce::String& paramID, const juce::String& labelText, juce::AudioProcessorValueTreeState& state)
{
    kwl.slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    kwl.slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 20);

    // Colour the knob (thumb), filled track, and background
    kwl.slider.setColour(juce::Slider::thumbColourId, juce::Colour(255, 111, 41));                   // knob
    kwl.slider.setColour(juce::Slider::trackColourId, juce::Colours::deeppink);                 // filled portion
    kwl.slider.setColour(juce::Slider::backgroundColourId, juce::Colour(123, 0, 70));       // background track
    kwl.slider.setColour(juce::Slider::rotarySliderFillColourId, juce::Colours::deeppink);      // for rotary fill
    kwl.slider.setColour(juce::Slider::rotarySliderOutlineColourId, juce::Colour(90, 0, 50));      // rotary outline

    addAndMakeVisible(kwl.slider);

    kwl.label.setText(labelText);
    kwl.label.setReadOnly(true);
    kwl.label.setColour(juce::TextEditor::backgroundColourId, juce::Colours::transparentWhite);
    kwl.label.setColour(juce::TextEditor::outlineColourId, juce::Colours::transparentWhite);
    kwl.label.setColour(juce::TextEditor::textColourId, juce::Colours::black);
    kwl.label.setJustification(juce::Justification::centred);
    addAndMakeVisible(kwl.label);

    kwl.attachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(state, paramID, kwl.slider);
}

void ExciterSaturationComponent::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(31, 31, 31));
}

void ExciterSaturationComponent::resized()
{
    group.setBounds(getLocalBounds());

    auto area = getLocalBounds().reduced(margin);
    int numKnobs = 3;
    int spacing = 20;

    int totalWidth = (knobSize * numKnobs) + (spacing * (numKnobs - 1));
    int startX = area.getX() + (area.getWidth() - totalWidth) / 2;
    int y = area.getY();

    for (int i = 0; i < numKnobs; ++i)
    {
        KnobWithLabel* kwl = nullptr;
        switch (i)
        {
        case 0: kwl = &drive; break;
        case 1: kwl = &mix; break;
        case 2: kwl = &highpass; break;
        }

        int x = startX + i * (knobSize + spacing);
        kwl->slider.setBounds(x, y, knobSize, knobSize);
        kwl->label.setBounds(x, -5, knobSize, labelHeight);
    }
}

void ExciterSaturationComponent::setDrive(float newValue) { drive.slider.setValue(newValue); }
void ExciterSaturationComponent::setMix(float newValue) { mix.slider.setValue(newValue); }
void ExciterSaturationComponent::setHighpass(float newValue) { highpass.slider.setValue(newValue); }
