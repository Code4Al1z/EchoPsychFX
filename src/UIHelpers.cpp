#include "UIHelpers.h"

namespace UIHelpers
{
    namespace Colors
    {
        const juce::Colour background{ 27, 17, 31 };
        const juce::Colour knobThumb{ 255, 107, 0 };
        const juce::Colour track{ 255, 46, 136 };
        const juce::Colour knobBackground{ 123, 0, 70 };
        const juce::Colour knobFill{ 255, 46, 136 };
        const juce::Colour knobOutline{ 90, 0, 50 };
        const juce::Colour labelText{ 232, 232, 240 };
        const juce::Colour groupOutline = juce::Colours::white.withAlpha(0.4f);
    }

    LayoutResult calculateKnobLayout(int numKnobs, int availableWidth, int availableHeight,
        bool allowMultipleRows)
    {
        using namespace Dimensions;

        LayoutResult result;
        result.knobBounds.reserve(numKnobs);

        if (numKnobs == 0)
        {
            result.requiredWidth = margin * 2;
            result.requiredHeight = margin * 2 + groupLabelHeight;
            return result;
        }

        const int totalKnobHeight = knobSize + labelHeight;
        const int singleRowWidth = numKnobs * knobSize + (numKnobs - 1) * spacing + margin * 2;

        if (!allowMultipleRows || singleRowWidth <= availableWidth)
        {
            result.requiredWidth = singleRowWidth;
            result.requiredHeight = totalKnobHeight + margin * 2 + groupLabelHeight;

            const int startX = margin;
            const int y = margin;

            for (int i = 0; i < numKnobs; ++i)
            {
                const int x = startX + i * (knobSize + spacing);
                result.knobBounds.emplace_back(x, y, knobSize, totalKnobHeight);
            }
        }
        else
        {
            const int knobsPerRow = juce::jmax(1, (availableWidth - margin * 2 + spacing) / (knobSize + spacing));
            const int numRows = (numKnobs + knobsPerRow - 1) / knobsPerRow;

            result.requiredWidth = juce::jmin(singleRowWidth,
                knobsPerRow * knobSize + (knobsPerRow - 1) * spacing + margin * 2);
            result.requiredHeight = numRows * totalKnobHeight + (numRows - 1) * spacing + margin * 2 + groupLabelHeight;

            for (int i = 0; i < numKnobs; ++i)
            {
                const int row = i / knobsPerRow;
                const int col = i % knobsPerRow;
                const int x = margin + col * (knobSize + spacing);
                const int y = margin + row * (totalKnobHeight + spacing);
                result.knobBounds.emplace_back(x, y, knobSize, totalKnobHeight);
            }
        }

        return result;
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