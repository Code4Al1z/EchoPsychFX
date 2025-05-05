#include "PluginProcessor.h"
#include "SpatialFXComponent.h"

SpatialFXComponent::SpatialFXComponent(juce::AudioProcessorValueTreeState& state)
{
    knobs.push_back(std::make_unique<KnobWithLabel>(createKnob(state, "phaseOffsetL", "Phase L Offset", -180.0f, 180.0f, 1.0f)));
    knobs.push_back(std::make_unique<KnobWithLabel>(createKnob(state, "phaseOffsetR", "Phase R Offset", -180.0f, 180.0f, 1.0f)));
    knobs.push_back(std::make_unique<KnobWithLabel>(createKnob(state, "sfxModRate", "SFX Rate", 0.0f, 10.0f, 0.01f)));
    knobs.push_back(std::make_unique<KnobWithLabel>(createKnob(state, "sfxModDepth", "SFX Depth")));
    knobs.push_back(std::make_unique<KnobWithLabel>(createKnob(state, "sfxWetDryMix", "SFX Wet/Dry")));
}

SpatialFXComponent::KnobWithLabel SpatialFXComponent::createKnob(juce::AudioProcessorValueTreeState& state,
    const juce::String& paramID,
    const juce::String& labelText,
    float min, float max, float step)
{
    KnobWithLabel control;
    control.slider = std::make_unique<juce::Slider>();
    control.slider->setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    control.slider->setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
    control.slider->setRange(min, max, step);
    addAndMakeVisible(*control.slider);

    control.label = std::make_unique<juce::Label>();
    control.label->setText(labelText, juce::dontSendNotification);
    control.label->setJustificationType(juce::Justification::centred);
    control.label->setColour(juce::Label::backgroundColourId, juce::Colours::transparentWhite);
    control.label->setColour(juce::Label::textColourId, juce::Colours::white);
    addAndMakeVisible(*control.label);

    control.attachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        state, paramID, *control.slider);

    return control;
}

void SpatialFXComponent::resized()
{
    auto area = getLocalBounds().reduced(margin);
    int x = area.getX();
    int y = area.getY();
    
    for (auto& control : knobs)
    {
        control->slider->setBounds(x, y, knobSize, knobSize);
        control->label->setBounds(x + knobSize / 10, -5, 100, 20);
        x += knobSize + margin;
    }
}

void SpatialFXComponent::setPhaseOffsetLeft(float newValue)
{
    knobs[0]->slider->setValue(newValue);
}

void SpatialFXComponent::setPhaseOffsetRight(float newValue)
{
    knobs[1]->slider->setValue(newValue);
}

void SpatialFXComponent::setModulationRate(float newValue)
{
    knobs[2]->slider->setValue(newValue);
}

void SpatialFXComponent::setModulationDepth(float newValue)
{
    knobs[3]->slider->setValue(newValue);
}

void SpatialFXComponent::setWetDryMix(float newValue)
{
    knobs[4]->slider->setValue(newValue);
}
