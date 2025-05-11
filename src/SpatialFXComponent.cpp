#include "PluginProcessor.h"
#include "SpatialFXComponent.h"

SpatialFXComponent::SpatialFXComponent(juce::AudioProcessorValueTreeState& state)
{
    knobs.push_back(std::make_unique<KnobWithLabel>(createKnob(state, "phaseOffsetL", "Phase L", -180.0f, 180.0f, 1.0f)));
    knobs.push_back(std::make_unique<KnobWithLabel>(createKnob(state, "phaseOffsetR", "Phase R", -180.0f, 180.0f, 1.0f)));

    knobs.push_back(std::make_unique<KnobWithLabel>(createKnob(state, "sfxModRateL", "Rate L", 0.0f, 10.0f, 0.01f)));
    knobs.push_back(std::make_unique<KnobWithLabel>(createKnob(state, "sfxModRateR", "Rate R", 0.0f, 10.0f, 0.01f)));

    knobs.push_back(std::make_unique<KnobWithLabel>(createKnob(state, "sfxModDepthL", "Depth L")));
    knobs.push_back(std::make_unique<KnobWithLabel>(createKnob(state, "sfxModDepthR", "Depth R")));

    knobs.push_back(std::make_unique<KnobWithLabel>(createKnob(state, "sfxWetDryMix", "Wet/Dry")));
    knobs.push_back(std::make_unique<KnobWithLabel>(createKnob(state, "sfxLfoPhaseOffset", "LFO Phase", 0.0f, 6.2832f, 0.01f)));

    knobs.push_back(std::make_unique<KnobWithLabel>(createKnob(state, "sfxAllpassFreq", "Allpass Freq", 20.0f, 20000.0f, 1.0f)));
    knobs.push_back(std::make_unique<KnobWithLabel>(createKnob(state, "haasDelayL", "Haas L", 0.0f, 40.0f, 0.1f)));
    knobs.push_back(std::make_unique<KnobWithLabel>(createKnob(state, "haasDelayR", "Haas R", 0.0f, 40.0f, 0.1f)));

    // Mod Shape ComboBox
    modShapeSelector = std::make_unique<ComboBoxWithLabel>();
    modShapeSelector->comboBox = std::make_unique<juce::ComboBox>();
    modShapeSelector->comboBox->addItem("Sine", 1);
    modShapeSelector->comboBox->addItem("Triangle", 2);
    modShapeSelector->comboBox->addItem("Square", 3);
    modShapeSelector->comboBox->addItem("Random", 4);
    modShapeSelector->attachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        state, "modulationShape", *modShapeSelector->comboBox);
    addAndMakeVisible(*modShapeSelector->comboBox);
}

SpatialFXComponent::KnobWithLabel SpatialFXComponent::createKnob(
    juce::AudioProcessorValueTreeState& state,
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
        control->label->setBounds(x, y - 20, knobSize, 20);
        x += knobSize + margin;
    }

    if (modShapeSelector) {
        modShapeSelector->comboBox->setBounds(x, y + 20, 100, 30);
    }
}

// Individual setters for programmatic updates
void SpatialFXComponent::setPhaseOffsetLeft(float newValue) { knobs[0]->slider->setValue(newValue); }
void SpatialFXComponent::setPhaseOffsetRight(float newValue) { knobs[1]->slider->setValue(newValue); }
void SpatialFXComponent::setModulationRate(float left, float right) {
    knobs[2]->slider->setValue(left);
    knobs[3]->slider->setValue(right);
}
void SpatialFXComponent::setModulationDepth(float left, float right) {
    knobs[4]->slider->setValue(left);
    knobs[5]->slider->setValue(right);
}
void SpatialFXComponent::setWetDryMix(float newValue) { knobs[6]->slider->setValue(newValue); }
void SpatialFXComponent::setLfoPhaseOffset(float newValue) { knobs[7]->slider->setValue(newValue); }
void SpatialFXComponent::setAllpassFrequency(float newValue) { knobs[8]->slider->setValue(newValue); }
void SpatialFXComponent::setHaasDelayMs(float leftMs, float rightMs) {
    knobs[9]->slider->setValue(leftMs);
    knobs[10]->slider->setValue(rightMs);
}
void SpatialFXComponent::setModShape(SpatialFX::LfoWaveform waveform) {
    if (modShapeSelector)
        modShapeSelector->comboBox->setSelectedId(static_cast<int>(waveform));
}
