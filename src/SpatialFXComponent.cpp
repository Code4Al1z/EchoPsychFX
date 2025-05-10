#include "PluginProcessor.h"
#include "SpatialFXComponent.h"

SpatialFXComponent::SpatialFXComponent(juce::AudioProcessorValueTreeState& state)
{
    knobs.push_back(std::make_unique<KnobWithLabel>(createKnob(state, "phaseOffsetL", "Phase L Offset", -180.0f, 180.0f, 1.0f)));
    knobs.push_back(std::make_unique<KnobWithLabel>(createKnob(state, "phaseOffsetR", "Phase R Offset", -180.0f, 180.0f, 1.0f)));
    knobs.push_back(std::make_unique<KnobWithLabel>(createKnob(state, "sfxModRate", "SFX Rate", 0.0f, 10.0f, 0.01f)));
    knobs.push_back(std::make_unique<KnobWithLabel>(createKnob(state, "sfxModDepth", "SFX Depth")));
    knobs.push_back(std::make_unique<KnobWithLabel>(createKnob(state, "sfxWetDryMix", "SFX Wet/Dry")));
    knobs.push_back(std::make_unique<KnobWithLabel>(createKnob(state, "sfxFeedback", "Feedback")));
    knobs.push_back(std::make_unique<KnobWithLabel>(createKnob(state, "sfxLfoPhaseOffset", "LFO Phase Offset", 0.0f, 6.2832f, 0.01f))); // up to 2pi

    // Sync Toggle
    syncToggle = std::make_unique<ToggleWithLabel>();
    syncToggle->toggle = std::make_unique<juce::ToggleButton>();
    syncToggle->toggle->setButtonText("Sync");
    syncToggle->attachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        state, "sfxSync", *syncToggle->toggle);
    addAndMakeVisible(*syncToggle->toggle);

    // Mod Shape ComboBox
    modShapeSelector = std::make_unique<ComboBoxWithLabel>();
    modShapeSelector->comboBox = std::make_unique<juce::ComboBox>();
    modShapeSelector->comboBox->addItem("Sine", 1);
    modShapeSelector->comboBox->addItem("Triangle", 2);
    modShapeSelector->comboBox->addItem("Noise", 3);
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

    // Place sync toggle and mod shape selector
    if (syncToggle) {
        syncToggle->toggle->setBounds(x, y + 20, 80, 30);
        x += 90;
    }

    if (modShapeSelector) {
        modShapeSelector->comboBox->setBounds(x, y + 20, 100, 30);
    }
}

// Individual value setters
void SpatialFXComponent::setPhaseOffsetLeft(float newValue) { knobs[0]->slider->setValue(newValue); }
void SpatialFXComponent::setPhaseOffsetRight(float newValue) { knobs[1]->slider->setValue(newValue); }
void SpatialFXComponent::setModulationRate(float newValue) { knobs[2]->slider->setValue(newValue); }
void SpatialFXComponent::setModulationDepth(float newValue) { knobs[3]->slider->setValue(newValue); }
void SpatialFXComponent::setWetDryMix(float newValue) { knobs[4]->slider->setValue(newValue); }
void SpatialFXComponent::setFeedback(float newValue) { knobs[5]->slider->setValue(newValue); }
void SpatialFXComponent::setLfoPhaseOffset(float newValue) { knobs[6]->slider->setValue(newValue); }
void SpatialFXComponent::setSyncEnabled(bool enabled) { if (syncToggle) syncToggle->toggle->setToggleState(enabled, juce::sendNotification); }
void SpatialFXComponent::setModShape(SpatialFX::ModShape modShape)
{
	if (modShapeSelector)
	{
		int shapeIndex = static_cast<int>(modShape);
		modShapeSelector->comboBox->setSelectedId(shapeIndex);
	}
}