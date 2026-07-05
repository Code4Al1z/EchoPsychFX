#ifndef ECHOPSYCHFX_EXCITERSATURATIONCOMPONENT_H_INCLUDED
#define ECHOPSYCHFX_EXCITERSATURATIONCOMPONENT_H_INCLUDED

#include "CollapsibleComponent.h"
#include "PluginLookAndFeel.h"
#include <juce_audio_processors/juce_audio_processors.h>
#include <memory>
#include <vector>

class ExciterSaturationComponent : public CollapsibleComponent
{
public:
    static constexpr int kExpandedW = 3 * PluginLookAndFeel::kKnobCell + PluginLookAndFeel::margin * 2;
    static constexpr int kExpandedH = PluginLookAndFeel::kHeaderH + PluginLookAndFeel::kKnobCell + PluginLookAndFeel::margin * 2;

    explicit ExciterSaturationComponent(juce::AudioProcessorValueTreeState& state);
    ~ExciterSaturationComponent() override = default;

    int expandedWidth() const override { return kExpandedW; }
    int expandedHeight() const override { return kExpandedH; }

    void paintContent(juce::Graphics& g) override;
    void layoutContent(juce::Rectangle<int> area) override;

    void setDrive(float v);
    void setMix(float v);
    void setHighpass(float v);

private:
    juce::GroupComponent group{ "exciterSaturationGroup", "Exciter Saturation" };
    std::vector<std::unique_ptr<PluginLookAndFeel::KnobWithLabel>> knobs;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ExciterSaturationComponent)
};

#endif