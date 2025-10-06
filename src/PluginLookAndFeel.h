#ifndef ECHOPSYCHFX_PLUGINLOOKANDFEEL_H_INCLUDED
#define ECHOPSYCHFX_PLUGINLOOKANDFEEL_H_INCLUDED

#include <juce_gui_extra/juce_gui_extra.h>

/**
 * @brief Custom look and feel for the plugin UI
 * 
 * Provides consistent styling for group components and other UI elements
 */
class PluginLookAndFeel : public juce::LookAndFeel_V4
{
public:
    PluginLookAndFeel();
    ~PluginLookAndFeel() override = default;

    void drawGroupComponentOutline(juce::Graphics&, int width, int height,
        const juce::String& text, const juce::Justification& justification,
        juce::GroupComponent&) override;

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PluginLookAndFeel)
};

#endif // ECHOPSYCHFX_PLUGINLOOKANDFEEL_H_INCLUDED
