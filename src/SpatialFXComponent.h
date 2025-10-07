#ifndef ECHOPSYCHFX_SPATIALFXCOMPONENT_H_INCLUDED
#define ECHOPSYCHFX_SPATIALFXCOMPONENT_H_INCLUDED

#include <juce_gui_extra/juce_gui_extra.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include "UIHelpers.h"
#include "SpatialFX.h"
#include <memory>
#include <vector>

class SpatialFXComponent : public juce::Component
{
public:
    SpatialFXComponent(juce::AudioProcessorValueTreeState& state);
    ~SpatialFXComponent() override = default;

    void paint(juce::Graphics& g) override;
    void resized() override;

    void setPhaseOffsetLeft(float newValue);
    void setPhaseOffsetRight(float newValue);
    void setModulationRate(float left, float right);
    void setModulationDepth(float left, float right);
    void setWetDryMix(float newValue);
    void setLfoPhaseOffset(float newValue);
    void setAllpassFrequency(float newValue);
    void setHaasDelayMs(float leftMs, float rightMs);
    void setModShape(SpatialFX::LfoWaveform waveform);

    int getMinimumWidth() const { return UIHelpers::Dimensions::knobSize + UIHelpers::Dimensions::margin * 2; }
    int getMinimumHeight() const { return UIHelpers::Dimensions::knobSize + UIHelpers::Dimensions::labelHeight + UIHelpers::Dimensions::margin * 2 + UIHelpers::Dimensions::groupLabelHeight; }
    int getRequiredHeight(int width) const;

private:
    juce::GroupComponent group{ "spatialFXGroup", "Spatial FX" };
    std::vector<std::unique_ptr<UIHelpers::KnobWithLabel>> knobs;

    std::unique_ptr<juce::ComboBox> modShapeSelector;
    std::unique_ptr<juce::Label> modShapeLabel;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> modShapeAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SpatialFXComponent)
};

#endif