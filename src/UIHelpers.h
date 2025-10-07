#pragma once
#include <juce_gui_extra/juce_gui_extra.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include <memory>

namespace UIHelpers
{
    struct KnobWithLabel
    {
        std::unique_ptr<juce::Slider> slider;
        std::unique_ptr<juce::Label> label;
        std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attachment;

        KnobWithLabel() = default;

        KnobWithLabel(juce::AudioProcessorValueTreeState& state,
            const juce::String& paramID,
            const juce::String& labelText,
            juce::Component& parent);

        void setBounds(int x, int y, int width, int height);
    };
}