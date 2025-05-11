#ifndef ECHOPSYCHFX_SPATIALFXCOMPONENT_H_INCLUDED
#define ECHOPSYCHFX_SPATIALFXCOMPONENT_H_INCLUDED

#include <memory>
#include <juce_gui_extra/juce_gui_extra.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include "SpatialFX.h"

class SpatialFXComponent : public juce::Component
{
public:
    SpatialFXComponent(juce::AudioProcessorValueTreeState& state);
    ~SpatialFXComponent() override = default;

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

private:
    struct KnobWithLabel {
        std::unique_ptr<juce::Slider> slider;
        std::unique_ptr<juce::Label> label;
        std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attachment;
    };

    struct ComboBoxWithLabel {
        std::unique_ptr<juce::ComboBox> comboBox;
        std::unique_ptr<juce::Label> label;
        std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> attachment;
    };

    KnobWithLabel createKnob(juce::AudioProcessorValueTreeState& state, const juce::String& paramID, const juce::String& labelText,
        float min = 0.0f, float max = 1.0f, float step = 0.01f);

    std::vector<std::unique_ptr<KnobWithLabel>> knobs;
    std::unique_ptr<ComboBoxWithLabel> modShapeSelector;

    int knobSize = 120;
    int margin = 10;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SpatialFXComponent)
};

#endif // ECHOPSYCHFX_SPATIALFXCOMPONENT_H_INCLUDED
