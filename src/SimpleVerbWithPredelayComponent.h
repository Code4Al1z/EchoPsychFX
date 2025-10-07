#ifndef ECHOPSYCHFX_SIMPLEVERBWITHPREDELAYCOMPONENT_H_INCLUDED
#define ECHOPSYCHFX_SIMPLEVERBWITHPREDELAYCOMPONENT_H_INCLUDED

#include <juce_gui_extra/juce_gui_extra.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include "UIHelpers.h"
#include <memory>
#include <vector>

class SimpleVerbWithPredelayComponent : public juce::Component
{
public:
    SimpleVerbWithPredelayComponent(juce::AudioProcessorValueTreeState& state);
    ~SimpleVerbWithPredelayComponent() override = default;

    void paint(juce::Graphics& g) override;
    void resized() override;

    void setPredelay(float newValue);
    void setSize(float newValue);
    void setDamping(float newValue);
    void setWet(float newValue);

    int getMinimumWidth() const { return UIHelpers::Dimensions::knobSize + UIHelpers::Dimensions::margin * 2; }
    int getMinimumHeight() const { return UIHelpers::Dimensions::knobSize + UIHelpers::Dimensions::labelHeight + UIHelpers::Dimensions::margin * 2 + UIHelpers::Dimensions::groupLabelHeight; }
    int getRequiredHeight(int width) const;

private:
    juce::GroupComponent group{ "simpleVerbWithPredelayGroup", "Simple Verb With Predelay" };
    std::vector<std::unique_ptr<UIHelpers::KnobWithLabel>> knobs;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SimpleVerbWithPredelayComponent)
};

#endif