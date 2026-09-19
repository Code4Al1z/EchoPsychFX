#pragma once

#include <juce_gui_extra/juce_gui_extra.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include <map>
#include <functional>
#include "WidthBalancerComponent.h"
#include "TiltEQComponent.h"
#include "ModDelayComponent.h"
#include "SpatialFXComponent.h"
#include "MicroPitchDetuneComponent.h"
#include "ExciterSaturation.h"
#include "ExciterSaturationComponent.h"
#include "SimpleVerbWithPredelayComponent.h"

/**
 * @brief Manages psychoacoustic perception presets
 *
 * Coordinates multiple effect components to create specific
 * psychoacoustic experiences through preset combinations
 */
class PerceptionPresetManager
{
public:
    PerceptionPresetManager(juce::AudioProcessorValueTreeState& apvts,
        TiltEQComponent& tiltEQ,
        WidthBalancerComponent& width,
        ModDelayComponent& delay,
        SpatialFXComponent& spatial,
        MicroPitchDetuneComponent& microPitch,
        ExciterSaturationComponent& exciterSaturation,
        SimpleVerbWithPredelayComponent& simpleVerb);

    ~PerceptionPresetManager() = default;

    /** Apply a preset by name (checks user presets first, then factory presets) */
    void applyPreset(const juce::String& presetName);

    /** True if presetName names one of the built-in, read-only factory presets */
    bool isFactoryPreset(const juce::String& presetName) const;

    /** True if presetName names a user-saved preset */
    bool isUserPreset(const juce::String& presetName) const;

    /** Names of all user-saved presets, alphabetically */
    juce::StringArray getUserPresetNames() const;

    /** False once the live APVTS state has drifted from whichever preset was last applied
        via applyPreset() - the cue the UI uses to switch its label to "Custom". True if no
        preset has been applied yet this session (nothing to have drifted from). */
    bool matchesLastAppliedPreset() const;

    /** Builds a plain-language description of what the *current* live parameter values
        (not any named preset) would sound/feel like - width and pull, brightness, delay
        movement, pitch drift, exciter character, and reverb space. Always reflects the
        live APVTS state, so it stays accurate for factory presets, user presets, and
        "Custom" alike. */
    juce::String generateBreakdown() const;

    /** Saves the current plugin state as a user preset. Fails (returns false) if
        presetName is empty or collides with a read-only factory preset name. */
    bool saveCurrentAsUserPreset(const juce::String& presetName);

    /** Renames a user preset. Fails if oldName isn't a user preset, or newName is
        empty or collides with a factory preset. */
    bool renameUserPreset(const juce::String& oldName, const juce::String& newName);

    /** Deletes a user preset. Fails (returns false) if presetName isn't a user preset. */
    bool deleteUserPreset(const juce::String& presetName);

private:
    // Live plugin state - user presets are captured from and restored to this directly
    juce::AudioProcessorValueTreeState& apvtsRef;

    // Component references
    TiltEQComponent& tiltEQComponent;
    WidthBalancerComponent& widthComponent;
    ModDelayComponent& delayComponent;
    SpatialFXComponent& spatialFXComponent;
    MicroPitchDetuneComponent& microPitchComponent;
    ExciterSaturationComponent& exciterSaturationComponent;
    SimpleVerbWithPredelayComponent& simpleVerbComponent;

    // Factory preset storage
    std::map<juce::String, std::function<void()>> presets;

    // User preset storage, persisted to disk
    std::map<juce::String, juce::ValueTree> userPresets;

    // Snapshot of the APVTS state taken just after the most recently applied preset finished
    // landing in it, for matchesLastAppliedPreset() to diff the live state against. Invalid
    // until the first applyPreset() call.
    juce::ValueTree lastAppliedPresetState;

    /** Initialize all factory presets */
    void initializePresets();

    juce::File getUserPresetsFile() const;
    void loadUserPresets();
    void saveUserPresetsToDisk() const;

    /** Helper to apply preset parameters to all components */
    void usePreset(ModDelay::ModulationType type, float delayTime, float feedbackLeft, float feedbackRight,
        float modMix, float delayModDepth, float delayModRate, bool syncEnabled,
        float width, float intensity, float midSideBalance, bool mono, float tiltEQ,
        float phaseOffsetL, float phaseOffsetR, float modulationRateL, float modulationRateR,
        float modulationDepthL, float modulationDepthR,
        float wetDryMix, float lfoPhaseOffset, float allpassFrequency, float leftHaasMs,
        float rightHaasMs, SpatialFX::LfoWaveform modShape,
        float detuneAmount, float lfoRate, float lfoDepth, float delayCentre,
        float stereoSeparation, float mix, float detuneFeedback, float diffusion,
        float drive, float exciterMix, float highpass,
        ExciterSaturation::SaturationType saturationType, ExciterSaturation::HarmonicMode harmonicMode,
        float toneBrightness, float harmonicBalance, bool autoGainEnabled,
        float predelay, float size, float damping, float wet);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PerceptionPresetManager)
};