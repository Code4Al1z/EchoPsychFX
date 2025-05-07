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

class PerceptionPresetManager
{
public:
    PerceptionPresetManager(TiltEQComponent& tiltEQ,
        WidthBalancerComponent& width,
        ModDelayComponent& delay,
        SpatialFXComponent& spatial,
        MicroPitchDetuneComponent& microPitch,
	    ExciterSaturationComponent& exciterSaturation);

    void applyPreset(const juce::String& presetName);

private:
    TiltEQComponent& tiltEQComponent;
    WidthBalancerComponent& widthComponent;
    ModDelayComponent& delayComponent;
    SpatialFXComponent& spatialFXComponent;
	MicroPitchDetuneComponent& microPitchComponent;
	ExciterSaturationComponent& exciterSaturationComponent;

    std::map<juce::String, std::function<void()>> presets;
    void initializePresets();
    void usePreset(ModDelay::ModulationType type, float delayTime, float feedbackLeft, float feedbackRight,
        float modMix, float delayModDepth, float delayModRate,
        float width, float intensity, float midSideBalance, bool mono, float tiltEQ,
        float phaseOffsetL, float phaseOffsetR, float modulationRate, float modulationDepth, float wetDryMix,
        float detuneAmount, float lfoRate, float lfoDepth, float delayCentre, float stereoSeparation, float mix,
        float drive, float exciterMix, float highpass);

};
