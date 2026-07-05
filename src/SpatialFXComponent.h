#ifndef ECHOPSYCHFX_SPATIALFXCOMPONENT_H_INCLUDED
#define ECHOPSYCHFX_SPATIALFXCOMPONENT_H_INCLUDED

#include "CollapsibleComponent.h"
#include "PluginLookAndFeel.h"
#include "SpatialFX.h"
#include <juce_audio_processors/juce_audio_processors.h>
#include <memory>
#include <vector>

class SpatialFXComponent : public CollapsibleComponent
{
public:
    static constexpr int kComboRowH = 30;
    static constexpr int kExpandedW = 4 * PluginLookAndFeel::kKnobCell + PluginLookAndFeel::margin * 2;
    static constexpr int kExpandedH = PluginLookAndFeel::kHeaderH + kComboRowH + PluginLookAndFeel::spacing + 3 * PluginLookAndFeel::kKnobCell + PluginLookAndFeel::margin * 2;

    explicit SpatialFXComponent(juce::AudioProcessorValueTreeState& state);
    ~SpatialFXComponent() override = default;

    int expandedWidth() const override { return kExpandedW; }
    int expandedHeight() const override { return kExpandedH; }

    void paintContent(juce::Graphics& g) override;
    void layoutContent(juce::Rectangle<int> area) override;

    void setPhaseOffsetLeft(float v);
    void setPhaseOffsetRight(float v);
    void setModulationRate(float l, float r);
    void setModulationDepth(float l, float r);
    void setWetDryMix(float v);
    void setLfoPhaseOffset(float v);
    void setAllpassFrequency(float v);
    void setHaasDelayMs(float l, float r);
    void setModShape(SpatialFX::LfoWaveform waveform);

private:
    juce::GroupComponent group{ "spatialFXGroup", "Spatial FX" };
    std::vector<std::unique_ptr<PluginLookAndFeel::KnobWithLabel>> knobs;
    std::unique_ptr<juce::ComboBox> modShapeSelector;
    std::unique_ptr<juce::Label> modShapeLabel;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> modShapeAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SpatialFXComponent)
};

#endif