#pragma once

#include <juce_gui_extra/juce_gui_extra.h>
#include <map>
#include <functional>
#include "WidthBalancerComponent.h"
#include "TiltEQComponent.h"
#include "ModDelayComponent.h"
#include "SpatialFXComponent.h"
#include "MicroPitchDetuneComponent.h"
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
    PerceptionPresetManager(TiltEQComponent& tiltEQ,
        WidthBalancerComponent& width,
        ModDelayComponent& delay,
        SpatialFXComponent& spatial,
        MicroPitchDetuneComponent& microPitch,
        ExciterSaturationComponent& exciterSaturation,
        SimpleVerbWithPredelayComponent& simpleVerb);

    ~PerceptionPresetManager() = default;

    /** Apply a preset by name */
    void applyPreset(const juce::String& presetName);

private:
    // Component references
    TiltEQComponent& tiltEQComponent;
    WidthBalancerComponent& widthComponent;
    ModDelayComponent& delayComponent;
    SpatialFXComponent& spatialFXComponent;
    MicroPitchDetuneComponent& microPitchComponent;
    ExciterSaturationComponent& exciterSaturationComponent;
    SimpleVerbWithPredelayComponent& simpleVerbComponent;

    // Preset storage
    std::map<juce::String, std::function<void()>> presets;

    /** Initialize all factory presets */
    void initializePresets();

    /** Helper to apply preset parameters to all components */
    void usePreset(ModDelay::ModulationType type, float delayTime, float feedbackLeft, float feedbackRight,
        float modMix, float delayModDepth, float delayModRate,
        float width, float intensity, float midSideBalance, bool mono, float tiltEQ,
        float phaseOffsetL, float phaseOffsetR, float modulationRateL, float modulationRateR,
        float modulationDepthL, float modulationDepthR,
        float wetDryMix, float lfoPhaseOffset, float allpassFrequency, float leftHaasMs,
        float rightHaasMs, SpatialFX::LfoWaveform modShape,
        float detuneAmount, float lfoRate, float lfoDepth, float delayCentre,
        float stereoSeparation, float mix,
        float drive, float exciterMix, float highpass,
        float predelay, float size, float damping, float wet);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PerceptionPresetManager)
};
