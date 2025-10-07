#ifndef ECHOPSYCHFX_EXCITERSATURATIONCOMPONENT_H_INCLUDED
#define ECHOPSYCHFX_EXCITERSATURATIONCOMPONENT_H_INCLUDED

#include <juce_gui_extra/juce_gui_extra.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include "PluginLookAndFeel.h"
#include <memory>
#include <vector>

class ExciterSaturationComponent : public juce::Component
{
public:
    ExciterSaturationComponent(juce::AudioProcessorValueTreeState& state);
    ~ExciterSaturationComponent() override = default;

    void paint(juce::Graphics& g) override;
    void resized() override;

    void setDrive(float newValue);
    void setMix(float newValue);
    void setHighpass(float newValue);

    int getMinimumWidth() const { return PluginLookAndFeel::knobSize + PluginLookAndFeel::margin * 2; }
    int getMinimumHeight() const { return PluginLookAndFeel::knobSize + PluginLookAndFeel::labelHeight + PluginLookAndFeel::margin * 2 + PluginLookAndFeel::groupLabelHeight; }

private:
    juce::GroupComponent group{ "exciterSaturationGroup", "Exciter Saturation" };
    std::vector<std::unique_ptr<PluginLookAndFeel::KnobWithLabel>> knobs;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ExciterSaturationComponent)
};

#endif