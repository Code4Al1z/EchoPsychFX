#ifndef ECHOPSYCHFX_MICROPITCHDETUNECOMPONENT_H_INCLUDED
#define ECHOPSYCHFX_MICROPITCHDETUNECOMPONENT_H_INCLUDED

#include "CollapsibleComponent.h"
#include "PluginLookAndFeel.h"
#include <juce_audio_processors/juce_audio_processors.h>
#include <memory>
#include <vector>

class MicroPitchDetuneComponent : public CollapsibleComponent
{
public:
    static constexpr int kExpandedW = 3 * PluginLookAndFeel::kKnobCell + PluginLookAndFeel::margin * 2;
    static constexpr int kExpandedH = PluginLookAndFeel::kHeaderH + 2 * PluginLookAndFeel::kKnobCell + PluginLookAndFeel::margin * 2;

    explicit MicroPitchDetuneComponent(juce::AudioProcessorValueTreeState& state);
    ~MicroPitchDetuneComponent() override = default;

    int expandedWidth() const override { return kExpandedW; }
    int expandedHeight() const override { return kExpandedH; }

    void paintContent(juce::Graphics& g) override;
    void layoutContent(juce::Rectangle<int> area) override;

    void setDetuneAmount(float v);
    void setLfoRate(float v);
    void setLfoDepth(float v);
    void setDelayCentre(float v);
    void setStereoSeparation(float v);
    void setMix(float v);

private:
    juce::GroupComponent group{ "microPitchDetuneGroup", "Micro-Pitch Detune" };
    std::vector<std::unique_ptr<PluginLookAndFeel::KnobWithLabel>> knobs;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MicroPitchDetuneComponent)
};

#endif