#pragma once

#include <juce_gui_extra/juce_gui_extra.h>
#include <map>
#include <functional>
#include "WidthBalancerComponent.h"
#include "TiltEQComponent.h"
#include "ModDelayComponent.h"
#include "SpatialFXComponent.h"

class PerceptionPresetManager
{
public:
    PerceptionPresetManager(TiltEQComponent& tiltEQ,
        WidthBalancerComponent& width,
        ModDelayComponent& delay,
        SpatialFXComponent& spatial);

    void applyPreset(const juce::String& presetName);

private:
    TiltEQComponent& tiltEQComponent;
    WidthBalancerComponent& widthComponent;
    ModDelayComponent& delayComponent;
    SpatialFXComponent& spatialFXComponent;

    std::map<juce::String, std::function<void()>> presets;
    void initializePresets();
    void usePreset(ModDelay::ModulationType type, float delayTime, float feedbackLeft, float feedbackRight,
        float mix, float delayModDepth, float delayModRate,
        float width, float midSideBalance, float mono, float tiltEQ,
        float phaseOffsetL, float phaseOffsetR, float modulationRate,
        float modulationDepth, float wetDryMix);

};
