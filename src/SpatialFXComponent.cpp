#include "PluginProcessor.h"
#include "SpatialFXComponent.h"

SpatialFXComponent::SpatialFXComponent(juce::AudioProcessorValueTreeState& state)
{
    addAndMakeVisible(group);
    group.setColour(juce::GroupComponent::outlineColourId, juce::Colours::white.withAlpha(0.4f));
    group.setColour(juce::GroupComponent::textColourId, juce::Colours::white);

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

void SpatialFXComponent::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(31, 31, 31));
}

void SpatialFXComponent::resized()
{
    group.setBounds(getLocalBounds());

    auto area = getLocalBounds().reduced(margin);
    const int numPerRow = 6;
    const int rowHeight = knobSize + 30;

    for (size_t i = 0; i < knobs.size(); ++i)
    {
        const int row = static_cast<int>(i) / numPerRow;
        const int col = static_cast<int>(i) % numPerRow;

        const int knobX = area.getX() + col * (knobSize + margin);
        const int knobY = area.getY() + row * rowHeight;

        knobs[i]->label->setBounds(knobX, knobY, knobSize, 20);
        knobs[i]->slider->setBounds(knobX, knobY + 20, knobSize, knobSize);
    }

    if (modShapeSelector)
    {
        const int totalKnobs = static_cast<int>(knobs.size());
        const int modRow = totalKnobs / numPerRow;
        const int modCol = totalKnobs % numPerRow;

        const int modX = area.getX() + modCol * (knobSize + margin);
        const int modY = area.getY() + modRow * rowHeight;

        modShapeSelector->comboBox->setBounds(modX, modY + 20, 100, 30);
    }
}

void SpatialFXComponent::setPhaseOffsetLeft(float newValue) { knobs[0]->slider->setValue(newValue); }
void SpatialFXComponent::setPhaseOffsetRight(float newValue) { knobs[1]->slider->setValue(newValue); }
void SpatialFXComponent::setModulationRate(float left, float right)
{
    knobs[2]->slider->setValue(left);
    knobs[3]->slider->setValue(right);
}
void SpatialFXComponent::setModulationDepth(float left, float right)
{
    knobs[4]->slider->setValue(left);
    knobs[5]->slider->setValue(right);
}
void SpatialFXComponent::setWetDryMix(float newValue) { knobs[6]->slider->setValue(newValue); }
void SpatialFXComponent::setLfoPhaseOffset(float newValue) { knobs[7]->slider->setValue(newValue); }
void SpatialFXComponent::setAllpassFrequency(float newValue) { knobs[8]->slider->setValue(newValue); }
void SpatialFXComponent::setHaasDelayMs(float leftMs, float rightMs)
{
    knobs[9]->slider->setValue(leftMs);
    knobs[10]->slider->setValue(rightMs);
}
void SpatialFXComponent::setModShape(SpatialFX::LfoWaveform waveform)
{
    if (modShapeSelector)
        modShapeSelector->comboBox->setSelectedId(static_cast<int>(waveform));
}