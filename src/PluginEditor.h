#ifndef ECHOPSYCHFX_PLUGINEDITOR_H_INCLUDED
#define ECHOPSYCHFX_PLUGINEDITOR_H_INCLUDED

#include "PluginProcessor.h"
#include "WidthBalancerComponent.h"
#include "TiltEQComponent.h"
#include "ModDelayComponent.h"
#include "SpatialFXComponent.h"
#include "PerceptionModeComponent.h"
#include "PerceptionPresetManager.h"
#include "MicroPitchDetuneComponent.h"
#include "ExciterSaturationComponent.h"
#include "SimpleVerbWithPredelayComponent.h"
#include "PluginLookAndFeel.h"

/**
 * @brief Main plugin editor UI
 *
 * Manages the plugin's graphical user interface with two modes:
 * - Manual Mode: Individual control over all effect parameters
 * - Perception Mode: Quick access to psychoacoustic presets
 */
class AudioPluginAudioProcessorEditor final : public juce::AudioProcessorEditor
{
public:
    explicit AudioPluginAudioProcessorEditor(AudioPluginAudioProcessor&);
    ~AudioPluginAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    // Reference to the processor
    AudioPluginAudioProcessor& processorRef;

    // Custom look and feel
    PluginLookAndFeel pluginLookAndFeel;

    // UI Mode toggle
    juce::ToggleButton modeToggle;

    // Effect components (manual mode)
    std::unique_ptr<WidthBalancerComponent> widthBalancerComponent;
    std::unique_ptr<TiltEQComponent> tiltEQComponent;
    std::unique_ptr<ModDelayComponent> modDelayComponent;
    std::unique_ptr<SpatialFXComponent> spatialFXComponent;
    std::unique_ptr<MicroPitchDetuneComponent> microPitchDetuneComponent;
    std::unique_ptr<ExciterSaturationComponent> exciterSaturationComponent;
    std::unique_ptr<SimpleVerbWithPredelayComponent> simpleVerbWithPredelayComponent;

    // Preset management (perception mode)
    std::unique_ptr<PerceptionPresetManager> presetManager;
    std::unique_ptr<PerceptionModeComponent> perceptionModeComponent;

    /** Toggle between manual and perception modes */
    void updateUIVisibility();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AudioPluginAudioProcessorEditor)
};

#endif // ECHOPSYCHFX_PLUGINEDITOR_H_INCLUDED
