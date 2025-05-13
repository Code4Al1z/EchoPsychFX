#include "PluginLookAndFeel.h"

PluginLookAndFeel::PluginLookAndFeel()
{
    // You can set global colours here
    setColour(juce::GroupComponent::outlineColourId, juce::Colours::white.withAlpha(0.5f));
    setColour(juce::GroupComponent::textColourId, juce::Colours::white);
}

void PluginLookAndFeel::drawGroupComponentOutline(juce::Graphics& g, int width, int height,
    const juce::String& text, const juce::Justification& justification,
    juce::GroupComponent& component)
{
    const float textPadding = 4.0f;
    const int textHeight = 24;

    auto font = juce::Font(textHeight * 0.8f, juce::Font::bold);
    g.setFont(font);

    auto textWidth = font.getStringWidth(text) + 2 * textPadding;

    g.setColour(component.findColour(juce::GroupComponent::outlineColourId));

    // Draw border
    g.drawRect(0.f, 0.f, (float)width, (float)height, 1.0f);


    // Draw text background and label
    g.setColour(component.findColour(juce::GroupComponent::textColourId));
    g.drawFittedText(text, 10, height - 25, textWidth, textHeight, justification, 1);
}