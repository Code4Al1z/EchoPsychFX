#ifndef ECHOPSYCHFX_SIMPLEVERBWITHPREDELAYCOMPONENT_H_INCLUDED
#define ECHOPSYCHFX_SIMPLEVERBWITHPREDELAYCOMPONENT_H_INCLUDED

#include "CollapsibleComponent.h"
#include "PluginLookAndFeel.h"
#include <juce_audio_processors/juce_audio_processors.h>
#include <memory>
#include <vector>

class SimpleVerbWithPredelayComponent : public CollapsibleComponent
{
public:
    static constexpr int kExpandedW = 4 * PluginLookAndFeel::kKnobCell + PluginLookAndFeel::margin * 2;
    static constexpr int kExpandedH = PluginLookAndFeel::kHeaderH + PluginLookAndFeel::kKnobCell + PluginLookAndFeel::margin * 2;

    explicit SimpleVerbWithPredelayComponent(juce::AudioProcessorValueTreeState& state);
    ~SimpleVerbWithPredelayComponent() override = default;

    int expandedWidth() const override { return kExpandedW; }
    int expandedHeight() const override { return kExpandedH; }

    void paintContent(juce::Graphics& g) override;
    void layoutContent(juce::Rectangle<int> area) override;

    void setPredelay(float v);
    void setSize(float v);
    void setDamping(float v);
    void setWet(float v);

private:
    juce::GroupComponent group{ "simpleVerbWithPredelayGroup", "Simple Verb With Predelay" };
    std::vector<std::unique_ptr<PluginLookAndFeel::KnobWithLabel>> knobs;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SimpleVerbWithPredelayComponent)
};

#endif