#include "UIHelpers.h"
#include "PluginLookAndFeel.h"

namespace UIHelpers
{
    KnobWithLabel::KnobWithLabel(juce::AudioProcessorValueTreeState& state,
        const juce::String& paramID,
        const juce::String& labelText,
        juce::Component& parent)
    {
        slider = std::make_unique<juce::Slider>();
        label = std::make_unique<juce::Label>();

        PluginLookAndFeel::configureKnob(*slider);
        PluginLookAndFeel::configureLabel(*label, labelText);

        parent.addAndMakeVisible(*slider);
        parent.addAndMakeVisible(*label);

        attachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
            state, paramID, *slider);
    }

    void KnobWithLabel::setBounds(int x, int y, int width, int height)
    {
        const int labelH = PluginLookAndFeel::labelHeight;
        const int knobH = height - labelH;

        label->setBounds(x, y, width, labelH);
        slider->setBounds(x, y + labelH, width, knobH);
    }
}