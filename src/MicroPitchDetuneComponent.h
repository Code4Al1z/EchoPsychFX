#ifndef ECHOPSYCHFX_MICROPITCHDETUNECOMPONENT_H_INCLUDED
#define ECHOPSYCHFX_MICROPITCHDETUNECOMPONENT_H_INCLUDED

#include <juce_gui_extra/juce_gui_extra.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include "PluginLookAndFeel.h"
#include <memory>
#include <vector>

class MicroPitchDetuneComponent : public juce::Component
{
public:
    MicroPitchDetuneComponent(juce::AudioProcessorValueTreeState& state);
    ~MicroPitchDetuneComponent() override = default;

    void paint(juce::Graphics& g) override;
    void resized() override;

    void setMix(float newValue);
    void setLfoRate(float newValue);
    void setLfoDepth(float newValue);
    void setDelayCentre(float newValue);
    void setDetuneAmount(float newValue);
    void setStereoSeparation(float newValue);

    int getMinimumWidth() const { return PluginLookAndFeel::knobSize + PluginLookAndFeel::margin * 2; }
    int getMinimumHeight() const { return PluginLookAndFeel::knobSize + PluginLookAndFeel::labelHeight + PluginLookAndFeel::margin * 2 + PluginLookAndFeel::groupLabelHeight; }

private:
    juce::GroupComponent group{ "microPitchDetuneGroup", "Micro-Pitch Detune" };
    std::vector<std::unique_ptr<PluginLookAndFeel::KnobWithLabel>> knobs;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MicroPitchDetuneComponent)
};

#endif