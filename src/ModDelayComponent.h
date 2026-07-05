#ifndef ECHOPSYCHFX_MODDELAYCOMPONENT_H_INCLUDED
#define ECHOPSYCHFX_MODDELAYCOMPONENT_H_INCLUDED

#include "CollapsibleComponent.h"
#include "PluginLookAndFeel.h"
#include "ModDelay.h"
#include <juce_audio_processors/juce_audio_processors.h>
#include <memory>
#include <vector>

class ModDelayComponent : public CollapsibleComponent
{
public:
    static constexpr int kWaveformRowH = 30;
    static constexpr int kExpandedW = 3 * PluginLookAndFeel::kKnobCell + PluginLookAndFeel::margin * 2;
    static constexpr int kExpandedH = PluginLookAndFeel::kHeaderH + kWaveformRowH + PluginLookAndFeel::spacing + 2 * PluginLookAndFeel::kKnobCell + PluginLookAndFeel::margin * 2;

    explicit ModDelayComponent(juce::AudioProcessorValueTreeState& state);
    ~ModDelayComponent() override = default;

    int expandedWidth() const override { return kExpandedW; }
    int expandedHeight() const override { return kExpandedH; }

    void paintContent(juce::Graphics& g) override;
    void layoutContent(juce::Rectangle<int> area) override;

    void setModulationType(ModDelay::ModulationType type);
    void setDelayTime(float v);
    void setModDepth(float v);
    void setModRate(float v);
    void setMix(float v);
    void setFeedbackLeft(float v);
    void setFeedbackRight(float v);

private:
    juce::GroupComponent group{ "modDelayGroup", "Motion Shifter" };
    std::vector<std::unique_ptr<PluginLookAndFeel::KnobWithLabel>> knobs;
    juce::OwnedArray<juce::TextButton> waveformButtons;
    std::unique_ptr<juce::ComboBox> hiddenCombo;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> modulationTypeAttachment;
    juce::ToggleButton syncToggle;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> syncAttachment;
    int selectedWaveform = 0;

    void updateWaveformSelection(int index);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ModDelayComponent)
};

#endif