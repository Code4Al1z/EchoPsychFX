#ifndef ECHOPSYCHFX_TILTEQCOMPONENT_H_INCLUDED
#define ECHOPSYCHFX_TILTEQCOMPONENT_H_INCLUDED

#include <juce_gui_extra/juce_gui_extra.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include "UIHelpers.h"
#include <memory>

class TiltEQComponent : public juce::Component
{
public:
    TiltEQComponent(juce::AudioProcessorValueTreeState& state);
    ~TiltEQComponent() override = default;

    void paint(juce::Graphics& g) override;
    void resized() override;

    void setTilt(float newValue);

private:
    juce::GroupComponent group{ "tiltEQGroup", "TiltEQ" };

    juce::Slider tiltSlider;
    juce::Label tiltLabel;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> tiltAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TiltEQComponent)
};

#endif