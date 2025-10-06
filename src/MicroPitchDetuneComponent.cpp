#include "MicroPitchDetuneComponent.h"

MicroPitchDetuneComponent::MicroPitchDetuneComponent(juce::AudioProcessorValueTreeState& state)
{
    addAndMakeVisible(group);
    group.setColour(juce::GroupComponent::outlineColourId, juce::Colours::white.withAlpha(0.4f));
    group.setColour(juce::GroupComponent::textColourId, juce::Colours::white);

    knobs.push_back(std::make_unique<KnobWithLabel>(createKnob(state, "mix", "Mix")));
    knobs.push_back(std::make_unique<KnobWithLabel>(createKnob(state, "lfoRate", "LFO Rate")));
    knobs.push_back(std::make_unique<KnobWithLabel>(createKnob(state, "lfoDepth", "LFO Depth")));
    knobs.push_back(std::make_unique<KnobWithLabel>(createKnob(state, "delayCentre", "Delay Centre")));
    knobs.push_back(std::make_unique<KnobWithLabel>(createKnob(state, "detuneAmount", "Detune")));
    knobs.push_back(std::make_unique<KnobWithLabel>(createKnob(state, "stereoSeparation", "Stereo Sep.")));
}

MicroPitchDetuneComponent::KnobWithLabel MicroPitchDetuneComponent::createKnob(
    juce::AudioProcessorValueTreeState& state,
    const juce::String& paramID,
    const juce::String& labelText)
{
    KnobWithLabel control;

    control.slider = std::make_unique<juce::Slider>();
    control.slider->setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    control.slider->setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);

    control.slider->setColour(juce::Slider::thumbColourId, juce::Colour(255, 111, 41));
    control.slider->setColour(juce::Slider::trackColourId, juce::Colours::deeppink);
    control.slider->setColour(juce::Slider::backgroundColourId, juce::Colour(123, 0, 70));
    control.slider->setColour(juce::Slider::rotarySliderFillColourId, juce::Colours::deeppink);
    control.slider->setColour(juce::Slider::rotarySliderOutlineColourId, juce::Colour(90, 0, 50));

    addAndMakeVisible(*control.slider);

    control.label = std::make_unique<juce::Label>();
    control.label->setText(labelText, juce::dontSendNotification);
    control.label->setJustificationType(juce::Justification::centred);
    control.label->setColour(juce::Label::textColourId, juce::Colours::white);
    addAndMakeVisible(*control.label);

    control.attachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        state, paramID, *control.slider);

    return control;
}

void MicroPitchDetuneComponent::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(31, 31, 31));
}

void MicroPitchDetuneComponent::resized()
{
    group.setBounds(getLocalBounds());

    auto area = getLocalBounds().reduced(margin);
    const int numKnobs = static_cast<int>(knobs.size());
    const int spacing = 20;
    const int totalWidth = (knobSize * numKnobs) + (spacing * (numKnobs - 1));
    const int startX = area.getX() + (area.getWidth() - totalWidth) / 2;
    const int y = area.getY();

    for (int i = 0; i < numKnobs; ++i)
    {
        const int x = startX + i * (knobSize + spacing);
        knobs[i]->label->setBounds(x, y, knobSize, 20);
        knobs[i]->slider->setBounds(x, y + 20, knobSize, knobSize);
    }
}

void MicroPitchDetuneComponent::setMix(float newValue) { knobs[0]->slider->setValue(newValue); }
void MicroPitchDetuneComponent::setLfoRate(float newValue) { knobs[1]->slider->setValue(newValue); }
void MicroPitchDetuneComponent::setLfoDepth(float newValue) { knobs[2]->slider->setValue(newValue); }
void MicroPitchDetuneComponent::setDelayCentre(float newValue) { knobs[3]->slider->setValue(newValue); }
void MicroPitchDetuneComponent::setDetuneAmount(float newValue) { knobs[4]->slider->setValue(newValue); }
void MicroPitchDetuneComponent::setStereoSeparation(float newValue) { knobs[5]->slider->setValue(newValue); }