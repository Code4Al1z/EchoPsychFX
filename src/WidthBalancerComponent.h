#ifndef ECHOPSYCHFX_WIDTHBALANCERCOMPONENT_H_INCLUDED
#define ECHOPSYCHFX_WIDTHBALANCERCOMPONENT_H_INCLUDED

#include <memory>
#include <juce_gui_extra/juce_gui_extra.h>
#include <juce_audio_processors/juce_audio_processors.h>

class WidthBalancerComponent : public juce::Component
{
public:
    WidthBalancerComponent(juce::AudioProcessorValueTreeState& state);
    ~WidthBalancerComponent() override;

    void resized() override;

private:

    int knobSize = 120;
    int margin = 10;

    juce::Slider widthSlider, midSideSlider;
    juce::ToggleButton monoToggle;

    juce::TextEditor widthTextBox;
    juce::TextEditor midSideTextBox;

    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> widthAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> midSideAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> monoAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(WidthBalancerComponent)
};

#endif // ECHOPSYCHFX_WIDTHBALANCERCOMPONENT_H_INCLUDED