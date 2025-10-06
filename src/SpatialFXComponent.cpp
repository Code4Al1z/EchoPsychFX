#include "SpatialFXComponent.h"

SpatialFXComponent::SpatialFXComponent(juce::AudioProcessorValueTreeState& state)
{
    using namespace UIHelpers;

    addAndMakeVisible(group);
    configureGroup(group);

    // Create all knobs - note some have custom ranges
    auto createCustomKnob = [&](const juce::String& paramID, const juce::String& label,
        float min, float max, float step) -> std::unique_ptr<KnobWithLabel>
        {
            auto knob = std::make_unique<KnobWithLabel>();
            knob->slider = std::make_unique<juce::Slider>();
            configureKnob(*knob->slider);
            knob->slider->setRange(min, max, step);
            addAndMakeVisible(*knob->slider);

            knob->label = std::make_unique<juce::Label>();
            configureLabel(*knob->label, label);
            addAndMakeVisible(*knob->label);

            knob->attachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
                state, paramID, *knob->slider);

            return knob;
        };

    knobs.push_back(createCustomKnob("phaseOffsetL", "Phase L", -180.0f, 180.0f, 1.0f));
    knobs.push_back(createCustomKnob("phaseOffsetR", "Phase R", -180.0f, 180.0f, 1.0f));
    knobs.push_back(createCustomKnob("sfxModRateL", "Rate L", 0.0f, 10.0f, 0.01f));
    knobs.push_back(createCustomKnob("sfxModRateR", "Rate R", 0.0f, 10.0f, 0.01f));
    knobs.emplace_back(std::make_unique<KnobWithLabel>(state, "sfxModDepthL", "Depth L", *this));
    knobs.emplace_back(std::make_unique<KnobWithLabel>(state, "sfxModDepthR", "Depth R", *this));
    knobs.emplace_back(std::make_unique<KnobWithLabel>(state, "sfxWetDryMix", "Wet/Dry", *this));
    knobs.push_back(createCustomKnob("sfxLfoPhaseOffset", "LFO Phase", 0.0f, 6.2832f, 0.01f));
    knobs.push_back(createCustomKnob("sfxAllpassFreq", "Allpass Freq", 20.0f, 20000.0f, 1.0f));
    knobs.push_back(createCustomKnob("haasDelayL", "Haas L", 0.0f, 40.0f, 0.1f));
    knobs.push_back(createCustomKnob("haasDelayR", "Haas R", 0.0f, 40.0f, 0.1f));

    // Mod Shape ComboBox
    modShapeSelector = std::make_unique<juce::ComboBox>();
    modShapeSelector->addItem("Sine", 1);
    modShapeSelector->addItem("Triangle", 2);
    modShapeSelector->addItem("Square", 3);
    modShapeSelector->addItem("Random", 4);
    modShapeSelector->setColour(juce::ComboBox::backgroundColourId, Colors::background);
    modShapeSelector->setColour(juce::ComboBox::textColourId, Colors::labelText);
    modShapeSelector->setColour(juce::ComboBox::outlineColourId, Colors::groupOutline);
    modShapeSelector->setColour(juce::ComboBox::arrowColourId, Colors::track);
    modShapeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        state, "modulationShape", *modShapeSelector);
    addAndMakeVisible(*modShapeSelector);

    modShapeLabel = std::make_unique<juce::Label>();
    configureLabel(*modShapeLabel, "Mod Shape");
    addAndMakeVisible(*modShapeLabel);
}

void SpatialFXComponent::paint(juce::Graphics& g)
{
    g.fillAll(UIHelpers::Colors::background);
}

void SpatialFXComponent::resized()
{
    using namespace UIHelpers::Dimensions;

    group.setBounds(getLocalBounds());

    auto area = getLocalBounds().reduced(margin);
    const int numPerRow = 6;
    const int rowHeight = knobSize + labelHeight + margin;

    // Layout knobs in grid
    for (size_t i = 0; i < knobs.size(); ++i)
    {
        const int row = static_cast<int>(i) / numPerRow;
        const int col = static_cast<int>(i) % numPerRow;

        const int knobX = area.getX() + col * (knobSize + margin);
        const int knobY = area.getY() + row * rowHeight;

        knobs[i]->setBounds(knobX, knobY, knobSize, knobSize + labelHeight);
    }

    // Mod shape selector
    const int totalKnobs = static_cast<int>(knobs.size());
    const int modRow = totalKnobs / numPerRow;
    const int modCol = totalKnobs % numPerRow;

    const int modX = area.getX() + modCol * (knobSize + margin);
    const int modY = area.getY() + modRow * rowHeight;

    if (modShapeLabel)
        modShapeLabel->setBounds(modX, modY, knobSize, labelHeight);

    if (modShapeSelector)
        modShapeSelector->setBounds(modX, modY + labelHeight, knobSize, 30);
}

void SpatialFXComponent::setPhaseOffsetLeft(float newValue)
{
    if (!knobs.empty())
        knobs[0]->slider->setValue(newValue);
}

void SpatialFXComponent::setPhaseOffsetRight(float newValue)
{
    if (knobs.size() > 1)
        knobs[1]->slider->setValue(newValue);
}

void SpatialFXComponent::setModulationRate(float left, float right)
{
    if (knobs.size() > 2)
        knobs[2]->slider->setValue(left);
    if (knobs.size() > 3)
        knobs[3]->slider->setValue(right);
}

void SpatialFXComponent::setModulationDepth(float left, float right)
{
    if (knobs.size() > 4)
        knobs[4]->slider->setValue(left);
    if (knobs.size() > 5)
        knobs[5]->slider->setValue(right);
}

void SpatialFXComponent::setWetDryMix(float newValue)
{
    if (knobs.size() > 6)
        knobs[6]->slider->setValue(newValue);
}

void SpatialFXComponent::setLfoPhaseOffset(float newValue)
{
    if (knobs.size() > 7)
        knobs[7]->slider->setValue(newValue);
}

void SpatialFXComponent::setAllpassFrequency(float newValue)
{
    if (knobs.size() > 8)
        knobs[8]->slider->setValue(newValue);
}

void SpatialFXComponent::setHaasDelayMs(float leftMs, float rightMs)
{
    if (knobs.size() > 9)
        knobs[9]->slider->setValue(leftMs);
    if (knobs.size() > 10)
        knobs[10]->slider->setValue(rightMs);
}

void SpatialFXComponent::setModShape(SpatialFX::LfoWaveform waveform)
{
    if (modShapeSelector)
        modShapeSelector->setSelectedId(static_cast<int>(waveform));
}