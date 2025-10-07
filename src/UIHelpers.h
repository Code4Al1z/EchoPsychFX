#pragma once
#include <juce_gui_extra/juce_gui_extra.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include <memory>

namespace UIHelpers
{
    namespace Colors
    {
        extern const juce::Colour background;
        extern const juce::Colour knobThumb;
        extern const juce::Colour track;
        extern const juce::Colour knobBackground;
        extern const juce::Colour knobFill;
        extern const juce::Colour knobOutline;
        extern const juce::Colour labelText;
        extern const juce::Colour groupOutline;
    }

    namespace Dimensions
    {
        constexpr int knobSize = 100;
        constexpr int margin = 10;
        constexpr int labelHeight = 20;
        constexpr int spacing = 20;
        constexpr int groupLabelHeight = 25;
    }

    struct LayoutResult
    {
        int requiredWidth;
        int requiredHeight;
        std::vector<juce::Rectangle<int>> knobBounds;
    };

    LayoutResult calculateKnobLayout(int numKnobs, int availableWidth, int availableHeight,
        bool allowMultipleRows = true);

    void configureKnob(juce::Slider& slider);
    void configureLabel(juce::Label& label, const juce::String& text);
    void configureGroup(juce::GroupComponent& group);

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