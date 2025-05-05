#include "MicroPitchDetuneComponent.h"

MicroPitchDetuneComponent::MicroPitchDetuneComponent(juce::AudioProcessorValueTreeState& state)
{
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

void MicroPitchDetuneComponent::resized()
{
    auto area = getLocalBounds().reduced(margin);
    int columns = 3;
    int spacing = 20;
    int labelSpacing = 6;

    auto knobArea = juce::Rectangle<int>(0, 0, knobSize, knobSize + labelHeight + labelSpacing);

    for (int i = 0; i < 6; ++i)
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

        int x = (i % columns) * (knobSize + spacing);
        int y = (i / columns) * (knobSize + labelHeight + spacing);
        auto base = area.withPosition(area.getX() + x, area.getY() + y);

        kwl->slider.setBounds(base.removeFromTop(knobSize));
        kwl->label.setBounds(kwl->slider.getX(), kwl->slider.getBottom() + labelSpacing, knobSize, labelHeight);
    }
}

void MicroPitchDetuneComponent::setMix(float newValue) { mix.slider.setValue(newValue, juce::dontSendNotification); }
void MicroPitchDetuneComponent::setLfoRate(float newValue) { lfoRate.slider.setValue(newValue, juce::dontSendNotification); }
void MicroPitchDetuneComponent::setLfoDepth(float newValue) { lfoDepth.slider.setValue(newValue, juce::dontSendNotification); }
void MicroPitchDetuneComponent::setDelayCentre(float newValue) { delayCentre.slider.setValue(newValue, juce::dontSendNotification); }
void MicroPitchDetuneComponent::setDetuneAmount(float newValue) { detuneAmount.slider.setValue(newValue, juce::dontSendNotification); }
void MicroPitchDetuneComponent::setStereoSeparation(float newValue) { stereoSeparation.slider.setValue(newValue, juce::dontSendNotification); }
