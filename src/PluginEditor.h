#pragma once

#include "PluginProcessor.h"

//==============================================================================
class AudioPluginAudioProcessorEditor final : public juce::AudioProcessorEditor
{
public:
    explicit AudioPluginAudioProcessorEditor (AudioPluginAudioProcessor&);
    void TiltEQGUI();
    ~AudioPluginAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

    void WidthBalancerResized();

    void WidthBalancerGUI();

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    AudioPluginAudioProcessor& processorRef;

    int knobSize = 200;

    // WidthBalancer GUI
    juce::Slider widthSlider, midSideSlider;
    juce::ToggleButton monoToggle;

    juce::TextEditor widthTextBox;
    juce::TextEditor midSideTextBox;

    juce::ScopedPointer<juce::AudioProcessorValueTreeState::SliderAttachment> widthAttachment;
    juce::ScopedPointer<juce::AudioProcessorValueTreeState::SliderAttachment> midSideAttachment;
    juce::ScopedPointer<juce::AudioProcessorValueTreeState::ButtonAttachment> monoAttachment;
    //==============================================================================

    //TiltEQ GUI
    juce::Slider tiltSlider;
    juce::ScopedPointer <juce::AudioProcessorValueTreeState::SliderAttachment> tiltAttachment;

    juce::TextEditor tiltTextBox;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioPluginAudioProcessorEditor)
};
