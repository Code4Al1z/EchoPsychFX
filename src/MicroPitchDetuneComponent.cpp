#include "MicroPitchDetuneComponent.h"

MicroPitchDetuneComponent::MicroPitchDetuneComponent(juce::AudioProcessorValueTreeState& state)
{
    addAndMakeVisible(group);
    group.setColour(juce::GroupComponent::outlineColourId, juce::Colours::white.withAlpha(0.4f));
    group.setColour(juce::GroupComponent::textColourId, juce::Colours::white);

    configureKnob(mix, "mix", "Mix", state);
    configureKnob(lfoRate, "lfoRate", "LFO Rate", state);
    configureKnob(lfoDepth, "lfoDepth", "LFO Depth", state);
    configureKnob(delayCentre, "delayCentre", "Delay Centre", state);
    configureKnob(detuneAmount, "detuneAmount", "Detune", state);
    configureKnob(stereoSeparation, "stereoSeparation", "Stereo Sep.", state);
}

MicroPitchDetuneComponent::~MicroPitchDetuneComponent()
{
}

void MicroPitchDetuneComponent::configureKnob(KnobWithLabel& kwl, const juce::String& paramID, const juce::String& labelText, juce::AudioProcessorValueTreeState& state)
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

void MicroPitchDetuneComponent::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(31, 31, 31));
}

void MicroPitchDetuneComponent::resized()
{
    group.setBounds(getLocalBounds());

    auto area = getLocalBounds().reduced(margin);
    int numKnobs = 6;
    int spacing = 20;

    int totalWidth = (knobSize * numKnobs) + (spacing * (numKnobs - 1));
    int startX = area.getX() + (area.getWidth() - totalWidth) / 2;
    int y = area.getY();

    for (int i = 0; i < numKnobs; ++i)
    {
        KnobWithLabel* kwl = nullptr;
        switch (i)
        {
        case 0: kwl = &mix; break;
        case 1: kwl = &lfoRate; break;
        case 2: kwl = &lfoDepth; break;
        case 3: kwl = &delayCentre; break;
        case 4: kwl = &detuneAmount; break;
        case 5: kwl = &stereoSeparation; break;
        }

        int x = startX + i * (knobSize + spacing);
        kwl->slider.setBounds(x, y, knobSize, knobSize);
        kwl->label.setBounds(x, -5, knobSize, labelHeight);
    }
}

void MicroPitchDetuneComponent::setMix(float newValue) { mix.slider.setValue(newValue); }
void MicroPitchDetuneComponent::setLfoRate(float newValue) { lfoRate.slider.setValue(newValue); }
void MicroPitchDetuneComponent::setLfoDepth(float newValue) { lfoDepth.slider.setValue(newValue); }
void MicroPitchDetuneComponent::setDelayCentre(float newValue) { delayCentre.slider.setValue(newValue); }
void MicroPitchDetuneComponent::setDetuneAmount(float newValue) { detuneAmount.slider.setValue(newValue); }
void MicroPitchDetuneComponent::setStereoSeparation(float newValue) { stereoSeparation.slider.setValue(newValue); }
