#pragma once

#include "PluginProcessor.h"
#include "WidthBalancerComponent.h"
#include "TiltEQComponent.h"
#include "ModDelayComponent.h"
#include "SpatialFXComponent.h"
#include "PerceptionModeComponent.h"
#include "PerceptionPresetManager.h"
#include "MicroPitchDetuneComponent.h"
#include "ExciterSaturationComponent.h"
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
    std::unique_ptr<PerceptionPresetManager> presetManager;
    std::unique_ptr<PerceptionModeComponent> perceptionModeComponent;
	std::unique_ptr<MicroPitchDetuneComponent> microPitchDetuneComponent;
	std::unique_ptr<ExciterSaturationComponent> exciterSaturationComponent;

private:
    AudioPluginAudioProcessor& processorRef;

    juce::ToggleButton modeToggle;
    juce::ComboBox modulationTypeComboBox;

    //PerceptionPresetManager presetManager;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AudioPluginAudioProcessorEditor)
};