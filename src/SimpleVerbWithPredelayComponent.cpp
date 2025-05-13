#include "SimpleVerbWithPredelayComponent.h"

SimpleVerbWithPredelayComponent::SimpleVerbWithPredelayComponent(juce::AudioProcessorValueTreeState& state)
{
    addAndMakeVisible(group);
    group.setColour(juce::GroupComponent::outlineColourId, juce::Colours::white.withAlpha(0.4f));
    group.setColour(juce::GroupComponent::textColourId, juce::Colours::white);

    configureKnob(predelay, "predelayMs", "Predelay", state);
    configureKnob(size, "size", "Size", state);
    configureKnob(damping, "damping", "Damping", state);
    configureKnob(wet, "wet", "Wet", state);
}

SimpleVerbWithPredelayComponent::~SimpleVerbWithPredelayComponent() {}

void SimpleVerbWithPredelayComponent::configureKnob(KnobWithLabel& kwl, const juce::String& paramID, const juce::String& labelText, juce::AudioProcessorValueTreeState& state)
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

void SimpleVerbWithPredelayComponent::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(31, 31, 31));
}

void SimpleVerbWithPredelayComponent::resized()
{
    group.setBounds(getLocalBounds());

    auto area = getLocalBounds().reduced(margin);
    int numKnobs = 4;
    int spacing = 20;

    int totalWidth = (knobSize * numKnobs) + (spacing * (numKnobs - 1));
    int startX = area.getX() + (area.getWidth() - totalWidth) / 2;
    int y = area.getY();

    for (int i = 0; i < numKnobs; ++i)
    {
        KnobWithLabel* kwl = nullptr;
        switch (i)
        {
        case 0: kwl = &predelay; break;
        case 1: kwl = &size; break;
        case 2: kwl = &damping; break;
        case 3: kwl = &wet; break;
        }

        int x = startX + i * (knobSize + spacing);
        kwl->slider.setBounds(x, y, knobSize, knobSize);
        kwl->label.setBounds(x, -5, knobSize, labelHeight);
    }
}

void SimpleVerbWithPredelayComponent::setPredelay(float newValue) { predelay.slider.setValue(newValue); }
void SimpleVerbWithPredelayComponent::setSize(float newValue) { size.slider.setValue(newValue); }
void SimpleVerbWithPredelayComponent::setDamping(float newValue) { damping.slider.setValue(newValue); }
void SimpleVerbWithPredelayComponent::setWet(float newValue) { wet.slider.setValue(newValue); }
