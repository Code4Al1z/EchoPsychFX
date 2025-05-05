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

    void setWidth(float newValue);
    void setMidSideBalance(float newValue);
    void setMono(bool newValue);
    void setIntensity(float newValue);

private:

    int knobSize = 100;
    int sliderHeight = 50;
    int labelHeight = 20;
    int margin = 10;

    juce::Slider widthSlider, midSideSlider, intensitySlider;
    juce::ToggleButton monoToggle;

    juce::TextEditor widthTextBox;
    juce::TextEditor midSideTextBox;
    juce::TextEditor intensityTextBox;

    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> widthAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> midSideAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> monoAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> intensityAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(WidthBalancerComponent)
};

#endif // ECHOPSYCHFX_WIDTHBALANCERCOMPONENT_H_INCLUDED