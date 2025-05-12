#include "PerceptionPresetManager.h"

PerceptionPresetManager::PerceptionPresetManager(TiltEQComponent& tiltEQ,
    WidthBalancerComponent& width,
    ModDelayComponent& delay,
    SpatialFXComponent& spatial,
    MicroPitchDetuneComponent& microPitch,
    ExciterSaturationComponent& exciterSaturation,
    SimpleVerbWithPredelayComponent& simpleVerb)
	: tiltEQComponent(tiltEQ), widthComponent(width), delayComponent(delay), spatialFXComponent(spatial),
	microPitchComponent(microPitch), exciterSaturationComponent(exciterSaturation), simpleVerbComponent(simpleVerb)
{
    initializePresets();
}

void PerceptionPresetManager::applyPreset(const juce::String& presetName)
{
    auto it = presets.find(presetName);
    if (it != presets.end())
        it->second(); // Call the associated lambda
    else
        DBG("Preset not found: " + presetName);
}

void PerceptionPresetManager::initializePresets()
{
    presets["Head Trip"] = [this]()
        {
            usePreset(ModDelay::ModulationType::Triangle, // modType
                400.0f, 0.7f, 0.75f, // delayTime, feedbackL, feedbackR
                0.6f, 4.0f, 0.2f, // modMix, modDepth, modRate
                1.5f, 0.8f, 0.0f, false, 0.1f, // width, intensity, midSideBalance, mono, tiltEQ
                0.08f, -0.05f, 0.3f, 0.3f, 0.6f, 0.6f, // phaseOffsetL, phaseOffsetR, modulationRateL, modulationRateR, modulationDepthL, modulationDepthR
                0.7f, 0.25f, 2500.0f, 0.5f, 0.5f, SpatialFX::LfoWaveform::Sine, // wetDryMix, lfoPhaseOffset, allpassFrequency, leftHaasMs, rightHaasMs, SpatialFX::LfoWaveform modShape
                3.0f, 0.25f, 0.0015f, 0.006f, 0.8f, 0.5f, // detuneAmount, lfoRate, lfoDepth, delayCentre, stereoSeparation, mix
                6.0f, 0.4f, 0.1f, // drive, exciterMix, highpass
                0.08f, 0.85f, 0.5f, 0.4f); // predelay, size, damping, wet
        };

    presets["Panic Room"] = [this]()
        {
            usePreset(ModDelay::ModulationType::Square, // modType
                150.0f, 0.8f, 0.7f, // delayTime, feedbackL, feedbackR
                0.9f, 6.0f, 1.5f, // modMix, modDepth, modRate
                0.3f, 0.2f, 0.2f, false, -0.15f, // width, intensity, midSideBalance, mono, tiltEQ
                0.15f, -0.18f, 1.2f, 0.8f, 0.85f, 1.1f, // phaseOffsetL, phaseOffsetR, modulationRateL, modulationRateR, modulationDepthL, modulationDepthR
                0.9f, 0.2f, 3200.0f, 0.1f, 0.2f, SpatialFX::LfoWaveform::Triangle, // wetDryMix, lfoPhaseOffset, allpassFrequency, leftHaasMs, rightHaasMs, SpatialFX::LfoWaveform modShape
                -5.0f, 3.0f, 0.0008f, 0.003f, 0.3f, 0.7f, // detuneAmount, lfoRate, lfoDepth, delayCentre, stereoSeparation, mix
                7.5f, 0.65f, 0.2f, // drive, exciterMix, highpass
                0.02f, 0.3f, 0.85f, 0.25f); // predelay, size, damping, wet
        };

    presets["Intimacy"] = [this]()
        {
            usePreset(ModDelay::ModulationType::Sine, // modType
                80.0f, 0.3f, 0.35f, // delayTime, feedbackL, feedbackR
                0.4f, 1.0f, 0.1f, // modMix, modDepth, modRate
                0.8f, 0.9f, -0.1f, false, 0.05f, // width, intensity, midSideBalance, mono, tiltEQ
                0.03f, -0.02f, 0.15f, 0.15f, 0.25f, 0.25f, // phaseOffsetL, phaseOffsetR, modulationRateL, modulationRateR, modulationDepthL, modulationDepthR
                0.35f, 0.1f, 1600.0f, 0.2f, 0.2f, SpatialFX::LfoWaveform::Sine, // wetDryMix, lfoPhaseOffset, allpassFrequency, leftHaasMs, rightHaasMs, SpatialFX::LfoWaveform modShape
                1.5f, 0.1f, 0.0002f, 0.001f, 0.4f, 0.3f, // detuneAmount, lfoRate, lfoDepth, delayCentre, stereoSeparation, mix
                2.0f, 0.2f, 0.05f, // drive, exciterMix, highpass
                0.015f, 0.25f, 0.3f, 0.3f); // predelay, size, damping, wet
        };

    presets["Blade Runner"] = [this]()
        {
            usePreset(ModDelay::ModulationType::SawtoothDown, // modType
                550.0f, 0.65f, 0.6f, // delayTime, feedbackL, feedbackR
                0.7f, 3.0f, 0.3f, // modMix, modDepth, modRate
                1.2f, 0.7f, 0.1f, false, -0.08f, // width, intensity, midSideBalance, mono, tiltEQ
                0.2f, -0.1f, 0.4f, 0.6f, 0.5f, 0.7f, // phaseOffsetL, phaseOffsetR, modulationRateL, modulationRateR, modulationDepthL, modulationDepthR
                0.85f, 0.3f, 4200.0f, 0.7f, 0.3f, SpatialFX::LfoWaveform::Random, // wetDryMix, lfoPhaseOffset, allpassFrequency, leftHaasMs, rightHaasMs, SpatialFX::LfoWaveform modShape
                -2.0f, 0.6f, 0.0018f, 0.004f, 0.75f, 0.6f, // detuneAmount, lfoRate, lfoDepth, delayCentre, stereoSeparation, mix
                4.5f, 0.55f, 0.12f, // drive, exciterMix, highpass
                0.12f, 0.95f, 0.6f, 0.5f); // predelay, size, damping, wet
        };

    presets["Alien Abduction"] = [this]()
        {
            usePreset(ModDelay::ModulationType::Sine, // modType
                350.0f, 0.55f, 0.6f, // delayTime, feedbackL, feedbackR
                0.85f, 4.5f, 0.6f, // modMix, modDepth, modRate
                0.9f, 0.75f, 0.05f, false, -0.07f, // width, intensity, midSideBalance, mono, tiltEQ
                0.18f, -0.22f, 0.9f, 0.9f, 0.7f, 0.7f, // phaseOffsetL, phaseOffsetR, modulationRateL, modulationRateR, modulationDepthL, modulationDepthR
                0.8f, 0.45f, 4200.0f, 0.6f, 0.6f, SpatialFX::LfoWaveform::Sine, // wetDryMix, lfoPhaseOffset, allpassFrequency, leftHaasMs, rightHaasMs, modShape
                4.0f, 1.2f, 0.001f, 0.0025f, 0.9f, 0.65f, // detuneAmount, lfoRate, lfoDepth, delayCentre, stereoSeparation, mix
                5.5f, 0.6f, 0.18f, // drive, exciterMix, highpass
                0.1f, 0.9f, 0.4f, 0.55f); // predelay, size, damping, wet
        };

    presets["Glass Tunnel"] = [this]()
        {
            usePreset(ModDelay::ModulationType::SawtoothUp, // modType
                220.0f, 0.45f, 0.5f, // delayTime, feedbackL, feedbackR
                0.65f, 1.8f, 0.15f, // modMix, modDepth, modRate
                0.7f, 0.85f, -0.05f, false, 0.02f, // width, intensity, midSideBalance, mono, tiltEQ
                0.05f, -0.08f, 0.25f, 0.25f, 0.4f, 0.4f, // phaseOffsetL, phaseOffsetR, modulationRateL, modulationRateR, modulationDepthL, modulationDepthR
                0.65f, 0.2f, 3200.0f, 0.5f, 0.5f, SpatialFX::LfoWaveform::Triangle, // wetDryMix, lfoPhaseOffset, allpassFrequency, leftHaasMs, rightHaasMs, modShape
                1.0f, 0.3f, 0.0005f, 0.0015f, 0.55f, 0.4f, // detuneAmount, lfoRate, lfoDepth, delayCentre, stereoSeparation, mix
                3.5f, 0.35f, 0.08f, // drive, exciterMix, highpass
                0.06f, 0.65f, 0.8f, 0.35f); // predelay, size, damping, wet
        };

    presets["Dream Logic"] = [this]()
        {
            usePreset(ModDelay::ModulationType::Sine, // modType
                280.0f, 0.5f, 0.55f, // delayTime, feedbackL, feedbackR
                0.75f, 2.2f, 0.25f, // modMix, modDepth, modRate
                0.88f, 0.78f, 0.02f, false, -0.03f, // width, intensity, midSideBalance, mono, tiltEQ
                0.06f, -0.04f, 0.35f, 0.35f, 0.48f, 0.48f, // phaseOffsetL, phaseOffsetR, modulationRateL, modulationRateR, modulationDepthL, modulationDepthR
                0.55f, 0.3f, 3000.0f, 0.4f, 0.4f, SpatialFX::LfoWaveform::Sine, // wetDryMix, lfoPhaseOffset, allpassFrequency, leftHaasMs, rightHaasMs, modShape
                1.8f, 0.5f, 0.0007f, 0.002f, 0.6f, 0.45f, // detuneAmount, lfoRate, lfoDepth, delayCentre, stereoSeparation, mix
                3.0f, 0.3f, 0.07f, // drive, exciterMix, highpass
                0.09f, 0.75f, 0.45f, 0.45f); // predelay, size, damping, wet
        };

    presets["Womb Space"] = [this]()
        {
            usePreset(ModDelay::ModulationType::Sine, // modType
                90.0f, 0.35f, 0.4f, // delayTime, feedbackL, feedbackR
                0.2f, 0.7f, 0.06f, // modMix, modDepth, modRate
                0.55f, 1.0f, -0.25f, true, -0.12f, // width, intensity, midSideBalance, mono, tiltEQ
                0.02f, -0.015f, 0.08f, 0.08f, 0.15f, 0.15f, // phaseOffsetL, phaseOffsetR, modulationRateL, modulationRateR, modulationDepthL, modulationDepthR
                0.3f, 0.05f, 2000.0f, 0.2f, 0.2f, SpatialFX::LfoWaveform::Sine, // wetDryMix, lfoPhaseOffset, allpassFrequency, leftHaasMs, rightHaasMs, modShape
                0.3f, 0.1f, 0.0001f, 0.0004f, 0.25f, 0.2f, // detuneAmount, lfoRate, lfoDepth, delayCentre, stereoSeparation, mix
                1.0f, 0.15f, 0.01f, // drive, exciterMix, highpass
                0.005f, 0.8f, 0.25f, 0.35f); // predelay, size, damping, wet
        };

    presets["Bipolar Bloom"] = [this]()
        {
            usePreset(ModDelay::ModulationType::Triangle, // modType
                450.0f, 0.9f, 0.5f, // delayTime, feedbackL, feedbackR
                0.8f, 4.0f, 0.35f, // modMix, modDepth, modRate
                1.1f, 0.65f, 0.15f, false, 0.08f, // width, intensity, midSideBalance, mono, tiltEQ
                0.18f, -0.12f, 0.7f, 0.7f, 0.65f, 0.65f, // phaseOffsetL, phaseOffsetR, modulationRateL, modulationRateR, modulationDepthL, modulationDepthR
                0.75f, 0.3f, 4200.0f, 0.3f, 0.25f, SpatialFX::LfoWaveform::Random, // wetDryMix, lfoPhaseOffset, allpassFrequency, leftHaasMs, rightHaasMs, modShape
                -3.5f, 1.5f, 0.0009f, 0.0022f, 0.95f, 0.68f, // detuneAmount, lfoRate, lfoDepth, delayCentre, stereoSeparation, mix
                6.5f, 0.5f, 0.11f, // drive, exciterMix, highpass
                0.07f, 0.95f, 0.35f, 0.6f); // preDelay, size, damping, wet
        };

    presets["Quiet Confidence"] = [this]()
        {
            usePreset(ModDelay::ModulationType::Triangle, // modType
                120.0f, 0.4f, 0.45f, // delayTime, feedbackL, feedbackR
                0.5f, 1.2f, 0.12f, // modMix, modDepth, modRate
                0.75f, 0.9f, -0.02f, false, 0.03f, // width, intensity, midSideBalance, mono, tiltEQ
                0.04f, -0.03f, 0.2f, 0.2f, 0.35f, 0.35f, // phaseOffsetL, phaseOffsetR, modulationRateL, modulationRateR, modulationDepthL, modulationDepthR
                0.45f, 0.2f, 2400.0f, 0.15f, 0.2f, SpatialFX::LfoWaveform::Sine, // wetDryMix, lfoPhaseOffset, allpassFrequency, leftHaasMs, rightHaasMs, modShape
                1.2f, 0.2f, 0.0004f, 0.0012f, 0.45f, 0.3f, // detuneAmount, lfoRate, lfoDepth, delayCentre, stereoSeparation, mix
                2.8f, 0.2f, 0.06f, // drive, exciterMix, highpass
                0.02f, 0.45f, 0.25f, 0.3f); // preDelay, size, damping, wet
        };

    presets["Falling Upwards"] = [this]()
        {
            usePreset(ModDelay::ModulationType::Sine, // modType
                180.0f, 0.6f, 0.65f, // delayTime, feedbackL, feedbackR
                0.55f, 2.5f, 0.2f, // modMix, modDepth, modRate
                1.0f, 0.7f, 0.05f, false, 0.04f, // width, intensity, midSideBalance, mono, tiltEQ
                0.1f, -0.07f, 0.3f, 0.3f, 0.45f, 0.45f, // phaseOffsetL, phaseOffsetR, modulationRateL, modulationRateR, modulationDepthL, modulationDepthR
                0.6f, 0.22f, 1800.0f, 0.25f, 0.25f, SpatialFX::LfoWaveform::Triangle, // wetDryMix, lfoPhaseOffset, allpassFrequency, leftHaasMs, rightHaasMs, modShape
                2.2f, 0.4f, 0.0006f, 0.0018f, 0.5f, 0.5f, // detuneAmount, lfoRate, lfoDepth, delayCentre, stereoSeparation, mix
                4.0f, 0.7f, 100.0f, // drive, exciterMix, highpass
                0.04f, 0.85f, 0.3f, 0.55f); // preDelay, size, damping, wet
        };

    presets["Molten Light"] = [this]()
        {
            usePreset(ModDelay::ModulationType::Sine, // modType
                270.0f, 0.8f, 0.75f, // delayTime, feedbackL, feedbackR
                0.7f, 3.8f, 0.25f, // modMix, modDepth, modRate
                1.3f, 0.9f, 0.1f, false, 0.06f, // width, intensity, midSideBalance, mono, tiltEQ
                0.18f, -0.15f, 0.5f, 0.65f, 0.75f, 0.75f, // phaseOffsetL, phaseOffsetR, modulationRateL, modulationRateR, modulationDepthL, modulationDepthR
                0.4f, 0.12f, 4200.0f, 0.5f, 0.3f, SpatialFX::LfoWaveform::Sine, // wetDryMix, lfoPhaseOffset, allpassFrequency, leftHaasMs, rightHaasMs, SpatialFX::LfoWaveform modShape
                3.0f, 0.5f, 0.0007f, 0.002f, 0.75f, 0.55f, // detuneAmount, lfoRate, lfoDepth, delayCentre, stereoSeparation, mix
                7.5f, 0.75f, 120.0f, // drive, exciterMix, highpass
                0.06f, 0.7f, 0.2f, 0.7f); // preDelay, size, damping, wet
        };

    presets["Ethereal Echo"] = [this]()
        {
            usePreset(ModDelay::ModulationType::Sine, // modType
                350.0f, 0.6f, 0.65f, // delayTime, feedbackL, feedbackR
                0.75f, 2.5f, 0.3f, // modMix, modDepth, modRate
                1.1f, 0.85f, -0.05f, false, 0.04f, // width, intensity, midSideBalance, mono, tiltEQ
                0.12f, -0.1f, 0.4f, 0.55f, 0.65f, 0.65f, // phaseOffsetL, phaseOffsetR, modulationRateL, modulationRateR, modulationDepthL, modulationDepthR
                0.3f, 0.09f, 4200.0f, 0.4f, 0.3f, SpatialFX::LfoWaveform::Triangle, // wetDryMix, lfoPhaseOffset, allpassFrequency, leftHaasMs, rightHaasMs, SpatialFX::LfoWaveform modShape
                -2.5f, 1.2f, 0.0008f, 0.0025f, 0.75f, 0.6f, // detuneAmount, lfoRate, lfoDepth, delayCentre, stereoSeparation, mix
                5.5f, 0.7f, 0.15f, // drive, exciterMix, highpass
                0.05f, 0.9f, 0.15f, 0.65f); // preDelay, size, damping, wet
        };

    presets["Lush Dreamscape"] = [this]()
        {
            usePreset(ModDelay::ModulationType::Triangle, // modType
                400.0f, 0.7f, 0.75f, // delayTime, feedbackL, feedbackR
                0.8f, 3.0f, 0.35f, // modMix, modDepth, modRate
                1.2f, 0.9f, 0.1f, false, 0.05f, // width, intensity, midSideBalance, mono, tiltEQ
                0.15f, -0.12f, 0.5f, 0.65f, 0.75f, 0.75f, // phaseOffsetL, phaseOffsetR, modulationRateL, modulationRateR, modulationDepthL, modulationDepthR
                0.2f, 0.06f, 4200.0f, 0.6f, 0.3f, SpatialFX::LfoWaveform::Sine, // wetDryMix, lfoPhaseOffset, allpassFrequency, leftHaasMs, rightHaasMs, SpatialFX::LfoWaveform modShape
                -3.5f, 1.5f, 0.0009f, 0.0022f, 0.8f, 0.7f, // detuneAmount, lfoRate, lfoDepth, delayCentre, stereoSeparation, mix
                6.5f, 0.8f, 0.2f, // drive, exciterMix, highpass
                0.06f, 1.0f, 0.3f, 0.75f); // preDelay, size, damping, wet
        };

    presets["Skin Contact"] = [this]()
        {
            usePreset(ModDelay::ModulationType::Triangle, // modType
                90.0f, 0.35f, 0.4f, // delayTime, feedbackL, feedbackR
                0.4f, 1.0f, 0.08f, // modMix, modDepth, modRate
                0.75f, 0.95f, -0.12f, false, 0.02f, // width, intensity, midSideBalance, mono, tiltEQ
                0.02f, -0.018f, 0.12f, 0.2f, 0.5f, 0.5f, // phaseOffsetL, phaseOffsetR, modulationRateL, modulationRateR, modulationDepthL, modulationDepthR
                0.1f, 0.03f, 4200.0f, 0.4f, 0.3f, SpatialFX::LfoWaveform::Random, // wetDryMix, lfoPhaseOffset, allpassFrequency, leftHaasMs, rightHaasMs, SpatialFX::LfoWaveform modShape
                1.1f, 0.15f, 0.0001f, 0.0004f, 0.4f, 0.25f, // detuneAmount, lfoRate, lfoDepth, delayCentre, stereoSeparation, mix
                4.0f, 0.45f, 44.0f, // drive, exciterMix, highpass
                0.01f, 0.35f, 0.1f, 0.2f); // preDelay, size, damping, wet
        };

    presets["Sonic Embrace"] = [this]()
        {
            usePreset(ModDelay::ModulationType::Sine, // modType
                200.0f, 0.5f, 0.55f, // delayTime, feedbackL, feedbackR
                0.6f, 2.0f, 0.15f, // modMix, modDepth, modRate
                0.8f, 0.9f, -0.08f, false, 0.03f, // width, intensity, midSideBalance, mono, tiltEQ
                0.08f, -0.06f, 0.25f, 0.35f, 0.45f, 0.45f, // phaseOffsetL, phaseOffsetR, modulationRateL, modulationRateR, modulationDepthL, modulationDepthR
                0.25f, 0.1f, 4200.0f, 0.3f, 0.25f, SpatialFX::LfoWaveform::Sine, // wetDryMix, lfoPhaseOffset, allpassFrequency, leftHaasMs, rightHaasMs, SpatialFX::LfoWaveform modShape
                -1.5f, 0.25f, 0.0003f, 0.0008f, 0.5f, 0.35f, // detuneAmount, lfoRate, lfoDepth, delayCentre, stereoSeparation, mix
                4.5f, 0.6f, 120.0f, // drive, exciterMix, highpass
                0.03f, 0.75f, 0.25f, 0.5f); // preDelay, size, damping, wet
        };

    presets["Strobe Heaven"] = [this]()
        {
            usePreset(ModDelay::ModulationType::Square, // modType
                90.0f, 0.7f, 0.7f, // delayTime, feedbackL, feedbackR
                0.85f, 3.2f, 1.6f, // modMix, modDepth, modRate
                0.4f, 0.6f, 0.3f, false, -0.1f, // width, intensity, midSideBalance, mono, tiltEQ
                0.2f, -0.2f, 1.4f, 1.4f, 0.9f, 0.9f, // phaseOffsetL, phaseOffsetR, modRateL, modRateR, modDepthL, modDepthR
                1.0f, 0.2f, 4200.0f, 0.7f, 0.7f, SpatialFX::LfoWaveform::Triangle, // wetDryMix, lfoPhaseOffset, allpassFreq, leftHaasMs, rightHaasMs, modShape
                -4.0f, 2.0f, 0.0012f, 0.0025f, 0.6f, 0.7f, // detuneAmount, lfoRate, lfoDepth, delayCentre, stereoSeparation, mix
                8.5f, 0.75f, 150.0f, // drive, exciterMix, highpass
                0.02f, 0.6f, 0.1f, 0.9f); // preDelay, size, damping, wet
        };

    presets["Glass Flame"] = [this]()
        {
            usePreset(ModDelay::ModulationType::Sine, // modType
                300.0f, 0.5f, 0.5f, // delayTime, feedbackL, feedbackR
                0.65f, 2.0f, 0.18f, // modMix, modDepth, modRate
                1.0f, 0.8f, -0.05f, false, 0.0f, // width, intensity, midSideBalance, mono, tiltEQ
                0.05f, -0.05f, 0.35f, 0.35f, 0.5f, 0.5f, // phaseOffsetL, phaseOffsetR, modRateL, modRateR, modDepthL, modDepthR
                0.65f, 0.15f, 3300.0f, 0.4f, 0.4f, SpatialFX::LfoWaveform::Sine, // wetDryMix, lfoPhaseOffset, allpassFreq, leftHaasMs, rightHaasMs, modShape
                2.0f, 0.3f, 0.0004f, 0.0016f, 0.6f, 0.4f, // detuneAmount, lfoRate, lfoDepth, delayCentre, stereoSeparation, mix
                5.5f, 0.8f, 110.0f, // drive, exciterMix, highpass
                0.03f, 0.8f, 0.2f, 0.5f); // preDelay, size, damping, wet
        };

    presets["Celestial Vault"] = [this]()
        {
            usePreset(ModDelay::ModulationType::Sine, // modType
                320.0f, 0.6f, 0.6f, // delayTime, feedbackL, feedbackR
                0.4f, 2.0f, 0.15f, // modMix, modDepth, modRate
                1.2f, 0.7f, 0.0f, false, -0.05f, // width, intensity, midSideBalance, mono, tiltEQ
                0.02f, 0.02f, 0.15f, 0.15f, 0.5f, 0.5f, // phaseOffsetL, phaseOffsetR, modRateL, modRateR, modDepthL, modDepthR
                0.65f, 0.05f, 2800.0f, 0.5f, 0.5f, SpatialFX::LfoWaveform::Sine, // wetDryMix, lfoPhaseOffset, allpassFreq, leftHaasMs, rightHaasMs, modShape
                2.0f, 0.2f, 0.001f, 0.005f, 0.75f, 0.4f, // detuneAmount, lfoRate, lfoDepth, delayCentre, stereoSeparation, mix
                4.5f, 0.35f, 0.12f, // drive, exciterMix, highpass
                0.12f, 1.0f, 0.45f, 0.7f); // preDelay, size, damping, wet
        };

    presets["Deep Illusion"] = [this]()
        {
            usePreset(ModDelay::ModulationType::Triangle, // modType
                280.0f, 0.4f, 0.4f, // delayTime, feedbackL, feedbackR
                0.3f, 1.0f, 0.2f, // modMix, modDepth, modRate
                1.6f, 0.6f, 0.1f, false, 0.02f, // width, intensity, midSideBalance, mono, tiltEQ
                -0.02f, 0.03f, 0.2f, 0.2f, 0.35f, 0.35f, // phaseOffsetL, phaseOffsetR, modRateL, modRateR, modDepthL, modDepthR
                0.6f, 0.1f, 3600.0f, 0.3f, 0.3f, SpatialFX::LfoWaveform::Triangle, // wetDryMix, lfoPhaseOffset, allpassFreq, leftHaasMs, rightHaasMs, modShape
                2.5f, 0.15f, 0.001f, 0.004f, 0.5f, 0.35f, // detuneAmount, lfoRate, lfoDepth, delayCentre, stereoSeparation, mix
                3.5f, 0.25f, 0.09f, // drive, exciterMix, highpass
                0.06f, 0.9f, 0.6f, 0.5f); // preDelay, size, damping, wet
        };

    presets["Ego Dissolve"] = [this]()
        {
            usePreset(ModDelay::ModulationType::Sine, // modType
                500.0f, 0.3f, 0.3f, // delayTime, feedbackL, feedbackR
                0.5f, 3.0f, 0.4f, // modMix, modDepth, modRate
                1.5f, 0.5f, 0.1f, false, 0.0f, // width, intensity, midSideBalance, mono, tiltEQ
                0.1f, -0.1f, 0.35f, 0.35f, 0.5f, 0.5f, // phaseOffsetL, phaseOffsetR, modRateL, modRateR, modDepthL, modDepthR
                0.55f, 0.2f, 2400.0f, 0.4f, 0.4f, SpatialFX::LfoWaveform::Sine, // wetDryMix, lfoPhaseOffset, allpassFreq, leftHaasMs, rightHaasMs, modShape
                1.5f, 0.3f, 0.0015f, 0.0065f, 0.75f, 0.45f, // detuneAmount, lfoRate, lfoDepth, delayCentre, stereoSeparation, mix
                5.5f, 0.3f, 0.1f, // drive, exciterMix, highpass
                0.1f, 0.88f, 0.7f, 0.6f); // preDelay, size, damping, wet
        };

    presets["Memory Dust"] = [this]()
        {
            usePreset(ModDelay::ModulationType::Triangle, // modType
                360.0f, 0.5f, 0.5f, // delayTime, feedbackL, feedbackR
                0.3f, 1.5f, 0.25f, // modMix, modDepth, modRate
                1.3f, 0.65f, 0.05f, false, 0.03f, // width, intensity, midSideBalance, mono, tiltEQ
                0.04f, 0.05f, 0.2f, 0.2f, 0.3f, 0.3f, // phaseOffsetL, phaseOffsetR, modulationRateL, modulationRateR, modulationDepthL, modulationDepthR
                0.5f, 0.15f, 1600.0f, 0.08f, 0.08f, SpatialFX::LfoWaveform::Random, // wetDryMix, lfoPhaseOffset, allpassFrequency, leftHaasMs, rightHaasMs, modShape
                2.8f, 0.25f, 0.001f, 0.005f, 0.7f, 0.4f, // detuneAmount, lfoRate, lfoDepth, delayCentre, stereoSeparation, mix
                4.0f, 0.2f, 0.08f, // drive, exciterMix, highpass
                0.07f, 0.9f, 0.55f, 0.45f); // preDelay, size, damping, wet
        };

    presets["Gentle Slap"] = [this]()
        {
            usePreset(ModDelay::ModulationType::Sine, // modType
                100.0f, 0.4f, 0.4f, // delayTime, feedbackL, feedbackR
                0.1f, 0.6f, 0.05f, // modMix, modDepth, modRate
                0.2f, 0.4f, 0.0f, false, 0.02f, // width, intensity, midSideBalance, mono, tiltEQ
                0.02f, -0.02f, 0.1f, 0.1f, 0.15f, 0.15f, // phaseOffsetL, phaseOffsetR, modulationRateL, modulationRateR, modulationDepthL, modulationDepthR
                0.25f, 0.2f, 4200.0f, 0.05f, 0.05f, SpatialFX::LfoWaveform::Sine, // wetDryMix, lfoPhaseOffset, allpassFrequency, leftHaasMs, rightHaasMs, SpatialFX::LfoWaveform modShape
                1.2f, 0.1f, 0.0003f, 0.0012f, 0.4f, 0.3f, // detuneAmount, lfoRate, lfoDepth, delayCentre, stereoSeparation, mix
                2.5f, 0.15f, 0.05f, // drive, exciterMix, highpass
                0.02f, 0.35f, 0.2f, 0.3f); // preDelay, size, damping, wet
        };

    presets["Moon Dance"] = [this]()
        {
            usePreset(ModDelay::ModulationType::Sine, // modType
                200.0f, 0.5f, 0.5f, // delayTime, feedbackL, feedbackR
                0.6f, 2.0f, 0.15f, // modMix, modDepth, modRate
                1.2f, 0.7f, -0.05f, false, 0.02f, // width, intensity, midSideBalance, mono, tiltEQ
                0.05f, -0.05f, 0.25f, 0.25f, 0.35f, 0.35f, // phaseOffsetL, phaseOffsetR, modulationRateL, modulationRateR, modulationDepthL, modulationDepthR
                0.45f, 0.4f, 4200.0f, 0.2f, 0.2f, SpatialFX::LfoWaveform::Triangle, // wetDryMix, lfoPhaseOffset, allpassFrequency, leftHaasMs, rightHaasMs, SpatialFX::LfoWaveform modShape
                1.5f, 0.25f, 0.0003f, 0.0008f, 0.5f, 0.35f, // detuneAmount, lfoRate, lfoDepth, delayCentre, stereoSeparation, mix
                3.0f, 0.5f, 40.0f, // drive, exciterMix, highpass
                0.03f, 0.75f, 0.25f, 0.5f); // preDelay, size, damping, wet
        };

    presets["Biting Lips"] = [this]()
        {
            usePreset(ModDelay::ModulationType::Sine, // modType
                80.0f, 0.45f, 0.45f, // delayTime, feedbackL, feedbackR
                0.25f, 0.5f, 0.08f, // modMix, modDepth, modRate
                0.3f, 0.9f, -0.05f, false, 0.05f, // width, intensity, midSideBalance, mono, tiltEQ
                0.02f, -0.02f, 0.1f, 0.1f, 0.15f, 0.15f, // phaseOffsetL, phaseOffsetR, modulationRateL, modulationRateR, modulationDepthL, modulationDepthR
                0.25f, 0.3f, 4200.0f, 0.1f, 0.1f, SpatialFX::LfoWaveform::Random, // wetDryMix, lfoPhaseOffset, allpassFrequency, leftHaasMs, rightHaasMs, SpatialFX::LfoWaveform modShape
                1.2f, 0.1f, 0.0003f, 0.0012f, 0.4f, 0.3f, // detuneAmount, lfoRate, lfoDepth, delayCentre, stereoSeparation, mix
                2.5f, 0.15f, 0.05f,// drive, exciterMix, highpass
                0.02f, 0.35f, 0.2f, 0.3f); // preDelay size damping wet
        };

    presets["Stormy Day"] = [this]()
        {
            usePreset(ModDelay::ModulationType::Sine, // modType
                300.0f, 0.5f, 0.5f, // delayTime, feedbackL, feedbackR
                0.65f, 2.0f, 0.18f, // modMix, modDepth, modRate
                1.0f, 0.8f, -0.05f, false, 0.0f, // width, intensity, midSideBalance, mono, tiltEQ
                0.05f, -0.05f, 0.35f, 0.35f, 0.5f, 0.5f, // phaseOffsetL, phaseOffsetR, modulationRateL, modulationRateR, modulationDepthL, modulationDepthR
                0.65f, 0.45f, 4200.0f, 0.3f, 0.3f, SpatialFX::LfoWaveform::Random, // wetDryMix, lfoPhaseOffset, allpassFrequency, leftHaasMs, rightHaasMs, SpatialFX::LfoWaveform modShape
                2.0f, 0.3f, 0.0004f, 0.0016f, 0.6f, 0.4f,// detuneAmount, lfoRate, lfoDepth, delayCentre, stereoSeparation, mix
                5.5f, 0.8f, 110.0f,// drive, exciterMix, highpass
                0.03f, 0.8f, 0.2f, 0.5f); // preDelay, size, damping, wet
        };

    presets["Summer Sunset"] = [this]()
        {
            usePreset(ModDelay::ModulationType::Sine, // modType
                400.0f, 0.6f, 0.6f, // delayTime, feedbackL, feedbackR
                0.6f, 1.5f, 0.15f, // modMix, modDepth, modRate
                1.1f, 0.8f, 0.1f, false, -0.04f, // width, intensity, midSideBalance, mono, tiltEQ
                0.15f, -0.12f, 0.25f, 0.25f, 0.4f, 0.4f, // phaseOffsetL, phaseOffsetR, modulationRateL, modulationRateR, modulationDepthL, modulationDepthR
                0.75f, 0.15f, 4200.0f, 0.35f, 0.15f, SpatialFX::LfoWaveform::Sine, // wetDryMix, lfoPhaseOffset, allpassFrequency, leftHaasMs, rightHaasMs, modShape
                -0.8f, 0.8f, 0.0004f, 0.0015f, 0.6f, 0.5f, // detuneAmount, lfoRate, lfoDepth, delayCentre, stereoSeparation, mix
                4.0f, 0.5f, 80.0f, // drive, exciterMix, highpass
                0.06f, 1.0f, 0.3f, 0.75f); // preDelay, size, damping, wet
        };

    presets["Ocean Waves"] = [this]()
        {
            usePreset(ModDelay::ModulationType::Triangle, // modType
                250.0f, 0.5f, 0.5f, // delayTime, feedbackL, feedbackR
                0.6f, 2.5f, 0.2f, // modMix, modDepth, modRate
                1.3f, 0.7f, 0.05f, false, 0.03f, // width, intensity, midSideBalance, mono, tiltEQ
                0.08f, -0.06f, 0.3f, 0.3f, 0.45f, 0.45f, // phaseOffsetL, phaseOffsetR, modulationRateL, modulationRateR, modulationDepthL, modulationDepthR
                0.55f, 0.12f, 4200.0f, 0.3f, 0.12f, SpatialFX::LfoWaveform::Triangle, // wetDryMix, lfoPhaseOffset, allpassFrequency, leftHaasMs, rightHaasMs, modShape
                -1.5f, 0.4f, 0.0006f, 0.0022f, 0.65f, 0.5f,// detuneAmount, lfoRate, lfoDepth, delayCentre, stereoSeparation, mix
                4.5f, 0.6f, 120.0f,// drive, exciterMix, highpass
                0.05f, 0.9f, 0.25f, 0.55f); // preDelay, size, damping, wet
        };

    presets["Crystal Clear"] = [this]()
        {
            usePreset(ModDelay::ModulationType::Sine, // modType
                110.0f, 0.2f, 0.2f, // delayTime, feedbackL, feedbackR
                0.25f, 0.6f, 0.05f, // modMix, modDepth, modRate
                0.6f, 0.6f, -0.15f, false, 0.01f, // width, intensity, midSideBalance, mono, tiltEQ
                0.01f, -0.01f, 0.05f, 0.05f, 0.1f, 0.1f, // phaseOffsetL, phaseOffsetR, modulationRateL, modulationRateR, modulationDepthL, modulationDepthR
                0.2f, 0.05f, 4200.0f, 0.15f, 0.05f, SpatialFX::LfoWaveform::Sine, // wetDryMix, lfoPhaseOffset, allpassFrequency, leftHaasMs, rightHaasMs, modShape
                0.7f, 0.1f, 0.0002f, 0.0008f, 0.5f, 0.2f,// detuneAmount, lfoRate, lfoDepth, delayCentre, stereoSeparation, mix
                1.2f, 0.25f, 400.0f, // drive, exciterMix, highpass
                0.01f, 0.4f, 0.3f, 0.3f); // preDelay, size, damping, wet
        };

    presets["Sweetest Memory"] = [this]()
        {
            usePreset(ModDelay::ModulationType::Sine, // modType
                220.0f, 0.45f, 0.4f, // delayTime, feedbackL, feedbackR
                0.55f, 1.2f, 0.15f, // modMix, modDepth, modRate
                1.2f, 0.65f, -0.05f, false, 0.02f, // width, intensity, midSideBalance, mono, tiltEQ
                0.07f, -0.04f, 0.28f, 0.28f, 0.38f, 0.38f, // phaseOffsetL, phaseOffsetR, modulationRateL, modulationRateR, modulationDepthL, modulationDepthR
                0.55f, 0.05f, 4200.0f, 0.4f, 0.05f, SpatialFX::LfoWaveform::Sine, // wetDryMix, lfoPhaseOffset, allpassFrequency, leftHaasMs, rightHaasMs, modShape
                -1.2f, 0.25f, 0.0003f, 0.0009f, 0.55f, 0.35f,// detuneAmount, lfoRate, lfoDepth, delayCentre, stereoSeparation, mix
                3.0f, 0.5f, 90.0f, // drive, exciterMix, highpass
                0.06f, 0.8f, 0.3f, 0.65f); // preDelay, size, damping, wet
		};
}

void PerceptionPresetManager::usePreset(ModDelay::ModulationType type, float delayTime, float feedbackLeft, float feedbackRight,
    float modMix, float delayModDepth, float delayModRate,
    float width, float intensity, float midSideBalance, bool mono, float tiltEQ,
    float phaseOffsetL, float phaseOffsetR, float modulationRateL, float modulationRateR, float modulationDepthL, float modulationDepthR,
    float wetDryMix, float lfoPhaseOffset, float allpassFrequency, float leftHaasMs, float rightHaasMs, SpatialFX::LfoWaveform modShape,
    float detuneAmount, float lfoRate, float lfoDepth, float delayCentre, float stereoSeparation, float mix,
    float drive, float exciterMix, float highpass,
    float predelay, float size, float damping, float wet)
{
    delayComponent.setModulationType(type);
    delayComponent.setDelayTime(delayTime);
    delayComponent.setFeedbackLeft(feedbackLeft);
    delayComponent.setFeedbackRight(feedbackRight);
    delayComponent.setMix(modMix);
    delayComponent.setModDepth(delayModDepth);
    delayComponent.setModRate(delayModRate);

    widthComponent.setWidth(width);
    widthComponent.setIntensity(intensity);
    widthComponent.setMidSideBalance(midSideBalance);
    widthComponent.setMono(mono);

    tiltEQComponent.setTilt(tiltEQ);

    spatialFXComponent.setPhaseOffsetLeft(phaseOffsetL);
    spatialFXComponent.setPhaseOffsetRight(phaseOffsetR);
    spatialFXComponent.setModulationRate(modulationRateL, modulationRateR);
    spatialFXComponent.setModulationDepth(modulationDepthL, modulationDepthR);
    spatialFXComponent.setWetDryMix(wetDryMix);
	spatialFXComponent.setLfoPhaseOffset(lfoPhaseOffset);
    spatialFXComponent.setAllpassFrequency(allpassFrequency);
	spatialFXComponent.setHaasDelayMs(leftHaasMs, rightHaasMs);
	spatialFXComponent.setModShape(modShape);

    microPitchComponent.setDetuneAmount(detuneAmount);
    microPitchComponent.setLfoRate(lfoRate);
    microPitchComponent.setLfoDepth(lfoDepth);
    microPitchComponent.setDelayCentre(delayCentre);
    microPitchComponent.setStereoSeparation(stereoSeparation);
    microPitchComponent.setMix(mix);

	exciterSaturationComponent.setDrive(drive);
	exciterSaturationComponent.setMix(exciterMix);
	exciterSaturationComponent.setHighpass(highpass);

	simpleVerbComponent.setPredelay(predelay);
	simpleVerbComponent.setSize(size);
	simpleVerbComponent.setDamping(damping);
	simpleVerbComponent.setWet(wet);
}