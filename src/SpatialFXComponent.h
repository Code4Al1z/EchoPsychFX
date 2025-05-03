#ifndef ECHOPSYCHFX_SPATIALFXCOMPONENT_H_INCLUDED
#define ECHOPSYCHFX_SPATIALFXCOMPONENT_H_INCLUDED

#include <memory>
#include <juce_gui_extra/juce_gui_extra.h>
#include <juce_audio_processors/juce_audio_processors.h>

class SpatialFXComponent: public juce::Component
{
public:
    SpatialFXComponent(juce::AudioProcessorValueTreeState& state);
    ~SpatialFXComponent() override;

    void resized() override;

private:

    int knobSize = 120;
    int margin = 10;

    juce::Slider phaseOffsetLeftSlider, phaseOffsetRightSlider, modulationRateSliderSFX, modulationDepthSliderSFX, wetDrySliderSFX;
    juce::TextEditor phaseOffsetLeftTextBox, phaseOffsetRightTextBox, modulationRateTextBoxSFX, modulationDepthTextBoxSFX, wetDryTextBoxSFX;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> phaseOffsetLeftAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> phaseOffsetRightAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> modulationRateAttachmentSFX;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> modulationDepthAttachmentSFX;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> wetDryAttachmentSFX;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SpatialFXComponent)
};

#endif // ECHOPSYCHFX_SPATIALFXCOMPONENT_H_INCLUDED