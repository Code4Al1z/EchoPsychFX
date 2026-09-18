#include "SpatialFXComponent.h"

SpatialFXComponent::SpatialFXComponent(juce::AudioProcessorValueTreeState& state)
    : CollapsibleComponent("Spatial FX")
{
    addAndMakeVisible(group);
    PluginLookAndFeel::configureGroup(group);

    knobs.emplace_back(std::make_unique<PluginLookAndFeel::KnobWithLabel>(state, "phaseOffsetL", "Phase L", *this));
    knobs.emplace_back(std::make_unique<PluginLookAndFeel::KnobWithLabel>(state, "phaseOffsetR", "Phase R", *this));
    knobs.emplace_back(std::make_unique<PluginLookAndFeel::KnobWithLabel>(state, "sfxModRateL", "Rate L", *this));
    knobs.emplace_back(std::make_unique<PluginLookAndFeel::KnobWithLabel>(state, "sfxModRateR", "Rate R", *this));
    knobs.emplace_back(std::make_unique<PluginLookAndFeel::KnobWithLabel>(state, "sfxModDepthL", "Depth L", *this));
    knobs.emplace_back(std::make_unique<PluginLookAndFeel::KnobWithLabel>(state, "sfxModDepthR", "Depth R", *this));
    knobs.emplace_back(std::make_unique<PluginLookAndFeel::KnobWithLabel>(state, "sfxWetDryMix", "Mix", *this));
    knobs.emplace_back(std::make_unique<PluginLookAndFeel::KnobWithLabel>(state, "sfxLfoPhaseOffset", "LFO Phase", *this));
    knobs.emplace_back(std::make_unique<PluginLookAndFeel::KnobWithLabel>(state, "sfxAllpassFreq", "Allpass", *this));
    knobs.emplace_back(std::make_unique<PluginLookAndFeel::KnobWithLabel>(state, "haasDelayL", "Haas L", *this));
    knobs.emplace_back(std::make_unique<PluginLookAndFeel::KnobWithLabel>(state, "haasDelayR", "Haas R", *this));

    modShapeSelector = std::make_unique<juce::ComboBox>("modShapeSelector");
    modShapeSelector->addItem("Sine", 1);
    modShapeSelector->addItem("Triangle", 2);
    modShapeSelector->addItem("Square", 3);
    modShapeSelector->addItem("Random", 4);
    addAndMakeVisible(*modShapeSelector);

    modShapeLabel = std::make_unique<juce::Label>("modShapeLabel", "Mod Shape");
    PluginLookAndFeel::configureLabel(*modShapeLabel, "Mod Shape");
    addAndMakeVisible(*modShapeLabel);

    modShapeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        state, "modulationShape", *modShapeSelector);

    for (auto& k : knobs)
        k->slider->setNumDecimalPlacesToDisplay(2);
}

void SpatialFXComponent::paintContent(juce::Graphics& g)
{
    g.fillAll(PluginLookAndFeel::background);
}

void SpatialFXComponent::layoutContent(juce::Rectangle<int> area)
{
    group.setBounds(area);
    auto inner = area.reduced(PluginLookAndFeel::margin);
    const int totalH = inner.getHeight();

    const int headerH = juce::jlimit(20, 28, static_cast<int>(totalH * 0.12f));
    const int labelW = juce::jlimit(50, 90, static_cast<int>(inner.getWidth() * 0.35f));
    const int comboW = inner.getWidth() - labelW - PluginLookAndFeel::spacing;

    modShapeLabel->setBounds(inner.getX(), inner.getY(), labelW, headerH);
    modShapeSelector->setBounds(inner.getX() + labelW + PluginLookAndFeel::spacing, inner.getY(), comboW, headerH);

    const int knobAreaY = inner.getY() + headerH + PluginLookAndFeel::spacing;
    const int knobAreaH = inner.getBottom() - knobAreaY;
    const int numKnobs = static_cast<int>(knobs.size());

    if (knobAreaH > 0)
    {
        auto layout = PluginLookAndFeel::calculateKnobLayout(numKnobs, inner.getWidth(), knobAreaH, false);
        if ((int)layout.knobBounds.size() < numKnobs) return;

        for (int i = 0; i < numKnobs; ++i)
        {
            auto b = layout.knobBounds[i];
            knobs[i]->setBounds(inner.getX() + b.getX(), knobAreaY + b.getY(), b.getWidth(), b.getHeight());
        }
    }
}

void SpatialFXComponent::setPhaseOffsetLeft(float v) { PluginLookAndFeel::setKnobValue(knobs, 0, v); }
void SpatialFXComponent::setPhaseOffsetRight(float v) { PluginLookAndFeel::setKnobValue(knobs, 1, v); }
void SpatialFXComponent::setModulationRate(float l, float r)
{
    PluginLookAndFeel::setKnobValue(knobs, 2, l);
    PluginLookAndFeel::setKnobValue(knobs, 3, r);
}
void SpatialFXComponent::setModulationDepth(float l, float r)
{
    PluginLookAndFeel::setKnobValue(knobs, 4, l);
    PluginLookAndFeel::setKnobValue(knobs, 5, r);
}
void SpatialFXComponent::setWetDryMix(float v) { PluginLookAndFeel::setKnobValue(knobs, 6, v); }
void SpatialFXComponent::setLfoPhaseOffset(float v) { PluginLookAndFeel::setKnobValue(knobs, 7, v); }
void SpatialFXComponent::setAllpassFrequency(float v) { PluginLookAndFeel::setKnobValue(knobs, 8, v); }
void SpatialFXComponent::setHaasDelayMs(float l, float r)
{
    PluginLookAndFeel::setKnobValue(knobs, 9, l);
    PluginLookAndFeel::setKnobValue(knobs, 10, r);
}
void SpatialFXComponent::setModShape(SpatialFX::LfoWaveform waveform)
{
    if (modShapeSelector)
        modShapeSelector->setSelectedId(static_cast<int>(waveform));
}