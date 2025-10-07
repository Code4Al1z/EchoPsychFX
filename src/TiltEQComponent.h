#ifndef ECHOPSYCHFX_TILTEQCOMPONENT_H_INCLUDED
#define ECHOPSYCHFX_TILTEQCOMPONENT_H_INCLUDED

#include <juce_gui_extra/juce_gui_extra.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include "PluginLookAndFeel.h"
#include <memory>
#include <vector>

class TiltEQComponent : public juce::Component
{
public:
    TiltEQComponent(juce::AudioProcessorValueTreeState& state);
    ~TiltEQComponent() override = default;

    void paint(juce::Graphics& g) override;
    void resized() override;

    void setTilt(float newValue);

    int getMinimumWidth() const { return PluginLookAndFeel::minKnobSize + PluginLookAndFeel::margin * 2; }
    int getMinimumHeight() const { return PluginLookAndFeel::minKnobSize + PluginLookAndFeel::labelHeight + PluginLookAndFeel::margin * 2 + PluginLookAndFeel::groupLabelHeight; }

private:
    juce::GroupComponent group{ "tiltEQGroup", "TiltEQ" };
    std::vector<std::unique_ptr<PluginLookAndFeel::KnobWithLabel>> knobs;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TiltEQComponent)
};

#endif