#pragma once

#include "PluginProcessor.h"
#include "ModDelay.h" // Include ModDelay header

//==============================================================================
class AudioPluginAudioProcessorEditor final : public juce::AudioProcessorEditor,
    public juce::ComboBox::Listener
{
public:
    explicit AudioPluginAudioProcessorEditor(AudioPluginAudioProcessor&);
    ~AudioPluginAudioProcessorEditor() override;

    //==============================================================================
    void paint(juce::Graphics&) override;
    void resized() override;

    void TiltEQResized();
    void WidthBalancerResized();
    void WidthBalancerGUI();
    void ModDelayGUI();
    void TiltEQGUI();
    void ModDelayResized();

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    AudioPluginAudioProcessor& processorRef;

    int knobSize = 120;
    int margin = 10;

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
    //==============================================================================

    // ModDelay GUI
    juce::Slider delayTimeSlider, feedbackLSlider, feedbackRSlider, mixSlider, modDepthSlider, modRateSlider;
    juce::ComboBox modulationTypeComboBox; // New combo box for modulation type

    juce::ScopedPointer<juce::AudioProcessorValueTreeState::SliderAttachment> delayAttachment, feedbackLAttachment, feedbackRAttachment, mixAttachment, depthAttachment, rateAttachment;
    juce::ScopedPointer<juce::AudioProcessorValueTreeState::ComboBoxAttachment> modulationTypeAttachment; // Attachment for combo box

    juce::TextEditor delayTimeTextBox, feedbackLTextBox, feedbackRTextBox, mixTextBox, modDepthTextBox, modRateTextBox;

    void comboBoxChanged(juce::ComboBox* comboBoxThatHasChanged) override; // Correct callback

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AudioPluginAudioProcessorEditor)
};