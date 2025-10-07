#include "SpatialFXComponent.h"

SpatialFXComponent::SpatialFXComponent(juce::AudioProcessorValueTreeState& state)
{
    addAndMakeVisible(group);
    PluginLookAndFeel::configureGroup(group);

    knobs.emplace_back(std::make_unique<PluginLookAndFeel::KnobWithLabel>(state, "phaseOffsetLeft", "Phase L", *this));
    knobs.emplace_back(std::make_unique<PluginLookAndFeel::KnobWithLabel>(state, "phaseOffsetRight", "Phase R", *this));
    knobs.emplace_back(std::make_unique<PluginLookAndFeel::KnobWithLabel>(state, "modulationRateLeft", "Rate L", *this));
    knobs.emplace_back(std::make_unique<PluginLookAndFeel::KnobWithLabel>(state, "modulationRateRight", "Rate R", *this));
    knobs.emplace_back(std::make_unique<PluginLookAndFeel::KnobWithLabel>(state, "modulationDepthLeft", "Depth L", *this));
    knobs.emplace_back(std::make_unique<PluginLookAndFeel::KnobWithLabel>(state, "modulationDepthRight", "Depth R", *this));
    knobs.emplace_back(std::make_unique<PluginLookAndFeel::KnobWithLabel>(state, "wetDryMix", "Mix", *this));
    knobs.emplace_back(std::make_unique<PluginLookAndFeel::KnobWithLabel>(state, "lfoPhaseOffset", "LFO Phase", *this));
    knobs.emplace_back(std::make_unique<PluginLookAndFeel::KnobWithLabel>(state, "allpassFrequency", "Allpass", *this));
    knobs.emplace_back(std::make_unique<PluginLookAndFeel::KnobWithLabel>(state, "haasDelayMsLeft", "Haas L", *this));
    knobs.emplace_back(std::make_unique<PluginLookAndFeel::KnobWithLabel>(state, "haasDelayMsRight", "Haas R", *this));

    modShapeSelector = std::make_unique<juce::ComboBox>("modShapeSelector");
    modShapeSelector->addItem("Sine", 1);
    modShapeSelector->addItem("Triangle", 2);
    modShapeSelector->addItem("Saw", 3);
    modShapeSelector->addItem("Square", 4);
    addAndMakeVisible(*modShapeSelector);

    modShapeLabel = std::make_unique<juce::Label>("modShapeLabel", "Mod Shape");
    PluginLookAndFeel::configureLabel(*modShapeLabel, "Mod Shape");
    addAndMakeVisible(*modShapeLabel);

    modShapeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        state, "modShape", *modShapeSelector);
}

void SpatialFXComponent::paint(juce::Graphics& g)
{
    g.fillAll(PluginLookAndFeel::background);
}

void SpatialFXComponent::resized()
{
    group.setBounds(getLocalBounds());

    auto area = getLocalBounds().reduced(PluginLookAndFeel::margin);

    const int comboHeight = 26;
    const int comboWidth = 120;

    auto comboArea = area.removeFromTop(comboHeight);
    modShapeLabel->setBounds(comboArea.removeFromLeft(80));
    comboArea.removeFromLeft(PluginLookAndFeel::spacing / 2);
    modShapeSelector->setBounds(comboArea.removeFromLeft(comboWidth));

    area.removeFromTop(PluginLookAndFeel::spacing);

    const int numKnobs = static_cast<int>(knobs.size());
    auto layout = PluginLookAndFeel::calculateKnobLayout(numKnobs, area.getWidth(), area.getHeight(), true);

    for (int i = 0; i < numKnobs; ++i)
    {
        if (i >= static_cast<int>(layout.knobBounds.size()))
            break;

        auto bounds = layout.knobBounds[i];
        bounds.translate(area.getX(), area.getY());
        knobs[i]->setBounds(bounds.getX(), bounds.getY(), bounds.getWidth(), bounds.getHeight());
    }
}

void SpatialFXComponent::setPhaseOffsetLeft(float newValue)
{
    if (!knobs.empty()) knobs[0]->slider->setValue(newValue);
}

void SpatialFXComponent::setPhaseOffsetRight(float newValue)
{
    if (knobs.size() > 1) knobs[1]->slider->setValue(newValue);
}

void SpatialFXComponent::setModulationRate(float left, float right)
{
    if (knobs.size() > 2) knobs[2]->slider->setValue(left);
    if (knobs.size() > 3) knobs[3]->slider->setValue(right);
}

void SpatialFXComponent::setModulationDepth(float left, float right)
{
    if (knobs.size() > 4) knobs[4]->slider->setValue(left);
    if (knobs.size() > 5) knobs[5]->slider->setValue(right);
}

void SpatialFXComponent::setWetDryMix(float newValue)
{
    if (knobs.size() > 6) knobs[6]->slider->setValue(newValue);
}

void SpatialFXComponent::setLfoPhaseOffset(float newValue)
{
    if (knobs.size() > 7) knobs[7]->slider->setValue(newValue);
}

void SpatialFXComponent::setAllpassFrequency(float newValue)
{
    if (knobs.size() > 8) knobs[8]->slider->setValue(newValue);
}

void SpatialFXComponent::setHaasDelayMs(float leftMs, float rightMs)
{
    if (knobs.size() > 9) knobs[9]->slider->setValue(leftMs);
    if (knobs.size() > 10) knobs[10]->slider->setValue(rightMs);
}

void SpatialFXComponent::setModShape(SpatialFX::LfoWaveform waveform)
{
    if (modShapeSelector)
        modShapeSelector->setSelectedId(static_cast<int>(waveform) + 1);
}