#include "UIHelpers.h"

namespace UIHelpers
{
    // Color definitions
    namespace Colors
    {
        const juce::Colour background{ 31, 31, 31 };
        const juce::Colour knobThumb{ 255, 111, 41 };
        const juce::Colour track{ juce::Colours::deeppink };
        const juce::Colour knobBackground{ 123, 0, 70 };
        const juce::Colour knobFill{ juce::Colours::deeppink };
        const juce::Colour knobOutline{ 90, 0, 50 };
        const juce::Colour labelText{ juce::Colours::white };
        const juce::Colour groupOutline = juce::Colours::white.withAlpha(0.4f);
    }

    void configureKnob(juce::Slider& slider)
    {
        slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);

        slider.setColour(juce::Slider::thumbColourId, Colors::knobThumb);
        slider.setColour(juce::Slider::trackColourId, Colors::track);
        slider.setColour(juce::Slider::backgroundColourId, Colors::knobBackground);
        slider.setColour(juce::Slider::rotarySliderFillColourId, Colors::knobFill);
        slider.setColour(juce::Slider::rotarySliderOutlineColourId, Colors::knobOutline);
    }

    void configureLabel(juce::Label& label, const juce::String& text)
    {
        label.setText(text, juce::dontSendNotification);
        label.setJustificationType(juce::Justification::centred);
        label.setColour(juce::Label::textColourId, Colors::labelText);
    }

    void configureGroup(juce::GroupComponent& group)
    {
        group.setColour(juce::GroupComponent::outlineColourId, Colors::groupOutline);
        group.setColour(juce::GroupComponent::textColourId, Colors::labelText);
    }

    // KnobWithLabel implementation
    KnobWithLabel::KnobWithLabel(juce::AudioProcessorValueTreeState& state,
        const juce::String& paramID,
        const juce::String& labelText,
        juce::Component& parent)
    {
        slider = std::make_unique<juce::Slider>();
        configureKnob(*slider);
        parent.addAndMakeVisible(*slider);

        label = std::make_unique<juce::Label>();
        configureLabel(*label, labelText);
        parent.addAndMakeVisible(*label);

        attachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
            state, paramID, *slider);
    }

    void KnobWithLabel::setBounds(int x, int y, int width, int height)
    {
        if (label)
            label->setBounds(x, y, width, Dimensions::labelHeight);
        if (slider)
            slider->setBounds(x, y + Dimensions::labelHeight, width, height - Dimensions::labelHeight);
    }
}