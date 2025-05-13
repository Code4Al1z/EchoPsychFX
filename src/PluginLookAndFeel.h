#pragma once

#include <juce_gui_extra/juce_gui_extra.h>

class PluginLookAndFeel : public juce::LookAndFeel_V4
{
public:
    PluginLookAndFeel();

    void drawGroupComponentOutline(juce::Graphics&, int width, int height,
        const juce::String& text, const juce::Justification& justification,
        juce::GroupComponent&) override;
};