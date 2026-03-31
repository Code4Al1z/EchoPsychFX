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
    if (getWidth() <= 0 || getHeight() <= 0) return;
    group.setBounds(getLocalBounds());

    auto area = getLocalBounds().reduced(PluginLookAndFeel::margin);
    const int totalH = area.getHeight();

    // Header strip: Mod Shape label + combo box
    // Height proportional, min 20 max 28
    const int headerH = juce::jlimit(20, 28, static_cast<int>(totalH * 0.12f));

    // Label gets ~35% of width, combo gets rest
    const int labelW = juce::jlimit(50, 90, static_cast<int>(area.getWidth() * 0.35f));
    const int comboW = area.getWidth() - labelW - PluginLookAndFeel::spacing;

    modShapeLabel->setBounds(area.getX(), area.getY(), labelW, headerH);
    modShapeSelector->setBounds(area.getX() + labelW + PluginLookAndFeel::spacing,
        area.getY(), comboW, headerH);

    // Knob area below header
    const int knobAreaY = area.getY() + headerH + PluginLookAndFeel::spacing;
    const int knobAreaH = area.getBottom() - knobAreaY;
    const int numKnobs = static_cast<int>(knobs.size());

    if (knobAreaH > 0)
    {
        auto layout = PluginLookAndFeel::calculateKnobLayout(
            numKnobs, area.getWidth(), knobAreaH, false);

        if ((int)layout.knobBounds.size() < numKnobs) return;

        for (int i = 0; i < numKnobs; ++i)
        {
            auto b = layout.knobBounds[i];
            knobs[i]->setBounds(
                area.getX() + b.getX(),
                knobAreaY + b.getY(),
                b.getWidth(),
                b.getHeight());
        }
    }
}

void SpatialFXComponent::setPhaseOffsetLeft(float v) { if (!knobs.empty())   knobs[0]->slider->setValue(v); }
void SpatialFXComponent::setPhaseOffsetRight(float v) { if (knobs.size() > 1) knobs[1]->slider->setValue(v); }
void SpatialFXComponent::setModulationRate(float l, float r)
{
    if (knobs.size() > 2) knobs[2]->slider->setValue(l);
    if (knobs.size() > 3) knobs[3]->slider->setValue(r);
}
void SpatialFXComponent::setModulationDepth(float l, float r)
{
    if (knobs.size() > 4) knobs[4]->slider->setValue(l);
    if (knobs.size() > 5) knobs[5]->slider->setValue(r);
}
void SpatialFXComponent::setWetDryMix(float v) { if (knobs.size() > 6)  knobs[6]->slider->setValue(v); }
void SpatialFXComponent::setLfoPhaseOffset(float v) { if (knobs.size() > 7)  knobs[7]->slider->setValue(v); }
void SpatialFXComponent::setAllpassFrequency(float v) { if (knobs.size() > 8)  knobs[8]->slider->setValue(v); }
void SpatialFXComponent::setHaasDelayMs(float l, float r)
{
    if (knobs.size() > 9)  knobs[9]->slider->setValue(l);
    if (knobs.size() > 10) knobs[10]->slider->setValue(r);
}
void SpatialFXComponent::setModShape(SpatialFX::LfoWaveform waveform)
{
    if (modShapeSelector)
        modShapeSelector->setSelectedId(static_cast<int>(waveform) + 1);
}