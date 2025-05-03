#pragma once

#include "PluginProcessor.h"
#include "WidthBalancerComponent.h"
#include "TiltEQComponent.h"
#include "ModDelayComponent.h"
#include "SpatialFXComponent.h"
#include "PerceptionModeComponent.h"
#include "ModDelay.h"
#include "SpatialFX.h"

//==============================================================================
class AudioPluginAudioProcessorEditor final : public juce::AudioProcessorEditor,
    public juce::ComboBox::Listener
{
public:
    explicit AudioPluginAudioProcessorEditor(AudioPluginAudioProcessor&);
    ~AudioPluginAudioProcessorEditor() override;

    void updateUIVisibility();

    void comboBoxChanged(juce::ComboBox* comboBoxThatHasChanged) override;

    //==============================================================================
    void paint(juce::Graphics&) override;
    void resized() override;

    std::unique_ptr<WidthBalancerComponent> widthBalancerComponent;
    std::unique_ptr<TiltEQComponent> tiltEQComponent;
    std::unique_ptr<ModDelayComponent> modDelayComponent;
    std::unique_ptr<SpatialFXComponent> spatialFXComponent;
    std::unique_ptr<PerceptionModeComponent> perceptionModeComponent;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    AudioPluginAudioProcessor& processorRef;

    juce::ToggleButton modeToggle;

    juce::ComboBox modulationTypeComboBox;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AudioPluginAudioProcessorEditor)
};