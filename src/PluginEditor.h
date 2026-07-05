#ifndef ECHOPSYCHFX_PLUGINEDITOR_H_INCLUDED
#define ECHOPSYCHFX_PLUGINEDITOR_H_INCLUDED

#include "PluginProcessor.h"
#include "InputControlsComponent.h"
#include "ModDelayComponent.h"
#include "SpatialFXComponent.h"
#include "PerceptionModeComponent.h"
#include "PerceptionPresetManager.h"
#include "MicroPitchDetuneComponent.h"
#include "ExciterSaturationComponent.h"
#include "SimpleVerbWithPredelayComponent.h"
#include "PluginLookAndFeel.h"

class AudioPluginAudioProcessorEditor final : public juce::AudioProcessorEditor
{
public:
    explicit AudioPluginAudioProcessorEditor(AudioPluginAudioProcessor&);
    ~AudioPluginAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    AudioPluginAudioProcessor& processorRef;
    PluginLookAndFeel pluginLookAndFeel;
    juce::ToggleButton modeToggle;

    std::unique_ptr<InputControlsComponent> inputControlsComponent;
    std::unique_ptr<ModDelayComponent> modDelayComponent;
    std::unique_ptr<SpatialFXComponent> spatialFXComponent;
    std::unique_ptr<MicroPitchDetuneComponent> microPitchDetuneComponent;
    std::unique_ptr<ExciterSaturationComponent> exciterSaturationComponent;
    std::unique_ptr<SimpleVerbWithPredelayComponent> simpleVerbComponent;

    std::unique_ptr<PerceptionPresetManager> presetManager;
    std::unique_ptr<PerceptionModeComponent> perceptionModeComponent;

    void onBlockCollapseChanged();
    void applyWindowSize();
    int calculateWindowWidth() const;
    int calculateWindowHeight() const;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AudioPluginAudioProcessorEditor)
};

#endif