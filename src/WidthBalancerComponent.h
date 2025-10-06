#ifndef ECHOPSYCHFX_WIDTHBALANCERCOMPONENT_H_INCLUDED
#define ECHOPSYCHFX_WIDTHBALANCERCOMPONENT_H_INCLUDED

#include <juce_gui_extra/juce_gui_extra.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include "UIHelpers.h"
#include <memory>

class WidthBalancerComponent : public juce::Component
{
public:
    explicit WidthBalancerComponent(juce::AudioProcessorValueTreeState& state);
    ~WidthBalancerComponent() override = default;

    void paint(juce::Graphics& g) override;
    void resized() override;

    void setWidth(float newValue);
    void setMidSideBalance(float newValue);
    void setMono(bool newValue);
    void setIntensity(float newValue);

private:
    juce::GroupComponent group{ "widthGroup", "Width Balancer" };

    juce::Slider widthSlider;
    juce::Slider midSideSlider;
    juce::Slider intensitySlider;
    juce::ToggleButton monoToggle;

    juce::Label widthLabel;
    juce::Label midSideLabel;
    juce::Label intensityLabel;

    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> widthAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> midSideAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> intensityAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> monoAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(WidthBalancerComponent)
};

#endif