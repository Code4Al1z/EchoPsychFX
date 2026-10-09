#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>

/** A thin strip along the bottom of the plugin: the version number on the left-hand side of the link, and a
    "Report a Bug" link that opens a pre-filled email and puts the technical details on the clipboard. */
class SupportFooter : public juce::Component, public juce::SettableTooltipClient
{
public:
    static constexpr int kHeight = 18;

    /** Where bug reports go. One constant so it is trivial to change before release. */
    static const char* supportEmail() { return "trailblaiz@trailblaiz.studio"; }

    SupportFooter(juce::AudioProcessor& processor, juce::AudioProcessorValueTreeState& state);

    void paint(juce::Graphics& g) override;
    void mouseMove(const juce::MouseEvent& e) override;
    void mouseExit(const juce::MouseEvent& e) override;
    void mouseUp(const juce::MouseEvent& e) override;

    /** Version text as shown to the user, e.g. "0.9.0 beta". */
    static juce::String versionText();

private:
    juce::AudioProcessor& processor;
    juce::AudioProcessorValueTreeState& state;
    bool hoveringLink = false;

    juce::Rectangle<int> linkBounds() const;
    juce::String buildReport() const;
    void reportBug();
};
