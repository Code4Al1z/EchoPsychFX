#include "PerceptionPresetManager.h"

PerceptionPresetManager::PerceptionPresetManager(TiltEQComponent& tiltEQ,
    WidthBalancerComponent& width,
    ModDelayComponent& delay,
    SpatialFXComponent& spatial,
    MicroPitchDetuneComponent& microPitch,
    ExciterSaturationComponent& exciterSaturation,
    SimpleVerbWithPredelayComponent& simpleVerb)
    : tiltEQComponent(tiltEQ)
    , widthComponent(width)
    , delayComponent(delay)
    , spatialFXComponent(spatial)
    , microPitchComponent(microPitch)
    , exciterSaturationComponent(exciterSaturation)
    , simpleVerbComponent(simpleVerb)
{
    initializePresets();
}

void PerceptionPresetManager::applyPreset(const juce::String& presetName)
{
    auto it = presets.find(presetName);
    if (it != presets.end())
    {
        it->second(); // Call the preset lambda
        DBG("Applied preset: " + presetName);
    }
    else
    {
        DBG("Preset not found: " + presetName);
    }
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
    // ModDelay
    delayComponent.setModulationType(type);
    delayComponent.setDelayTime(delayTime);
    delayComponent.setFeedbackLeft(feedbackLeft);
    delayComponent.setFeedbackRight(feedbackRight);
    delayComponent.setMix(modMix);
    delayComponent.setModDepth(delayModDepth);
    delayComponent.setModRate(delayModRate);

    // WidthBalancer
    widthComponent.setWidth(width);
    widthComponent.setIntensity(intensity);
    widthComponent.setMidSideBalance(midSideBalance);
    widthComponent.setMono(mono);

    // TiltEQ
    tiltEQComponent.setTilt(tiltEQ);

    // SpatialFX
    spatialFXComponent.setPhaseOffsetLeft(phaseOffsetL);
    spatialFXComponent.setPhaseOffsetRight(phaseOffsetR);
    spatialFXComponent.setModulationRate(modulationRateL, modulationRateR);
    spatialFXComponent.setModulationDepth(modulationDepthL, modulationDepthR);
    spatialFXComponent.setWetDryMix(wetDryMix);
    spatialFXComponent.setLfoPhaseOffset(lfoPhaseOffset);
    spatialFXComponent.setAllpassFrequency(allpassFrequency);
    spatialFXComponent.setHaasDelayMs(leftHaasMs, rightHaasMs);
    spatialFXComponent.setModShape(modShape);

    // MicroPitchDetune
    microPitchComponent.setDetuneAmount(detuneAmount);
    microPitchComponent.setLfoRate(lfoRate);
    microPitchComponent.setLfoDepth(lfoDepth);
    microPitchComponent.setDelayCentre(delayCentre);
    microPitchComponent.setStereoSeparation(stereoSeparation);
    microPitchComponent.setMix(mix);

    // ExciterSaturation
    exciterSaturationComponent.setDrive(drive);
    exciterSaturationComponent.setMix(exciterMix);
    exciterSaturationComponent.setHighpass(highpass);

    // SimpleVerbWithPredelay
    simpleVerbComponent.setPredelay(predelay);
    simpleVerbComponent.setSize(size);
    simpleVerbComponent.setDamping(damping);
    simpleVerbComponent.setWet(wet);
}

void PerceptionPresetManager::initializePresets()
{
    // Preset format: modType, delayTime, feedbackL, feedbackR, modMix, modDepth, modRate,
    //                width, intensity, midSideBalance, mono, tiltEQ,
    //                phaseOffsetL, phaseOffsetR, modRateL, modRateR, modDepthL, modDepthR,
    //                wetDryMix, lfoPhaseOffset, allpassFreq, haasL, haasR, modShape,
    //                detune, lfoRate, lfoDepth, delayCentre, stereoSep, mix,
    //                drive, exciterMix, highpass,
    //                predelay, size, damping, wet

    presets["Head Trip"] = [this]() {
        usePreset(ModDelay::ModulationType::Triangle,
            400.0f, 0.7f, 0.75f, 0.6f, 4.0f, 0.2f,
            1.5f, 0.8f, 0.0f, false, 0.1f,
            0.08f, -0.05f, 0.3f, 0.3f, 0.6f, 0.6f,
            0.7f, 0.25f, 2500.0f, 0.5f, 0.6f, SpatialFX::LfoWaveform::Sine,
            3.0f, 0.25f, 0.0015f, 0.006f, 0.8f, 0.5f,
            6.0f, 0.4f, 0.1f,
            0.08f, 0.85f, 0.5f, 0.4f);
        };

    presets["Panic Room"] = [this]() {
        usePreset(ModDelay::ModulationType::Square,
            150.0f, 0.8f, 0.7f, 0.9f, 6.0f, 1.5f,
            0.3f, 0.2f, 0.2f, false, -0.15f,
            0.15f, -0.18f, 1.2f, 0.8f, 0.85f, 1.1f,
            0.9f, 0.2f, 3200.0f, 0.1f, 0.2f, SpatialFX::LfoWaveform::Triangle,
            -5.0f, 3.0f, 0.0008f, 0.003f, 0.3f, 0.7f,
            7.5f, 0.65f, 0.2f,
            0.02f, 0.3f, 0.85f, 0.25f);
        };

    presets["Intimacy"] = [this]() {
        usePreset(ModDelay::ModulationType::Sine,
            80.0f, 0.3f, 0.35f, 0.4f, 1.0f, 0.1f,
            0.8f, 0.9f, -0.1f, false, 0.05f,
            0.03f, -0.02f, 0.15f, 0.15f, 0.25f, 0.25f,
            0.35f, 0.1f, 1600.0f, 0.2f, 0.2f, SpatialFX::LfoWaveform::Sine,
            1.5f, 0.1f, 0.0002f, 0.001f, 0.4f, 0.3f,
            2.0f, 0.2f, 0.05f,
            0.015f, 0.25f, 0.3f, 0.3f);
        };

    presets["Blade Runner"] = [this]() {
        usePreset(ModDelay::ModulationType::SawtoothDown,
            550.0f, 0.65f, 0.6f, 0.7f, 3.0f, 0.3f,
            1.2f, 0.7f, 0.1f, false, -0.08f,
            0.2f, -0.1f, 0.4f, 0.6f, 0.5f, 0.7f,
            0.85f, 0.3f, 4200.0f, 0.7f, 0.3f, SpatialFX::LfoWaveform::Random,
            -2.0f, 0.6f, 0.0018f, 0.004f, 0.75f, 0.6f,
            4.5f, 0.55f, 0.12f,
            0.12f, 0.95f, 0.6f, 0.5f);
        };

    presets["Alien Abduction"] = [this]() {
        usePreset(ModDelay::ModulationType::Sine,
            350.0f, 0.55f, 0.6f, 0.85f, 4.5f, 0.6f,
            0.9f, 0.75f, 0.05f, false, -0.07f,
            0.18f, -0.22f, 0.9f, 0.9f, 0.7f, 0.7f,
            0.8f, 0.45f, 4200.0f, 0.6f, 0.6f, SpatialFX::LfoWaveform::Sine,
            4.0f, 1.2f, 0.001f, 0.0025f, 0.9f, 0.65f,
            5.5f, 0.6f, 0.18f,
            0.1f, 0.9f, 0.4f, 0.55f);
        };

    presets["Glass Tunnel"] = [this]() {
        usePreset(ModDelay::ModulationType::SawtoothUp,
            220.0f, 0.45f, 0.5f, 0.65f, 1.8f, 0.15f,
            0.7f, 0.85f, -0.05f, false, 0.02f,
            0.05f, -0.08f, 0.25f, 0.25f, 0.4f, 0.4f,
            0.65f, 0.2f, 3200.0f, 0.5f, 0.5f, SpatialFX::LfoWaveform::Triangle,
            1.0f, 0.3f, 0.0005f, 0.0015f, 0.55f, 0.4f,
            3.5f, 0.35f, 0.08f,
            0.06f, 0.65f, 0.8f, 0.35f);
        };

    presets["Dream Logic"] = [this]() {
        usePreset(ModDelay::ModulationType::Sine,
            280.0f, 0.5f, 0.55f, 0.75f, 2.2f, 0.25f,
            0.88f, 0.78f, 0.02f, false, -0.03f,
            0.06f, -0.04f, 0.35f, 0.35f, 0.48f, 0.48f,
            0.55f, 0.3f, 3000.0f, 0.4f, 0.4f, SpatialFX::LfoWaveform::Sine,
            1.8f, 0.5f, 0.0007f, 0.002f, 0.6f, 0.45f,
            3.0f, 0.3f, 0.07f,
            0.09f, 0.75f, 0.45f, 0.45f);
        };

    presets["Womb Space"] = [this]() {
        usePreset(ModDelay::ModulationType::Sine,
            90.0f, 0.35f, 0.4f, 0.2f, 0.7f, 0.06f,
            0.55f, 1.0f, -0.25f, true, -0.12f,
            0.02f, -0.015f, 0.08f, 0.08f, 0.15f, 0.15f,
            0.3f, 0.05f, 2000.0f, 0.2f, 0.2f, SpatialFX::LfoWaveform::Sine,
            0.3f, 0.1f, 0.0001f, 0.0004f, 0.25f, 0.2f,
            1.0f, 0.15f, 0.01f,
            0.005f, 0.8f, 0.25f, 0.35f);
        };

    presets["Bipolar Bloom"] = [this]() {
        usePreset(ModDelay::ModulationType::Triangle,
            450.0f, 0.9f, 0.5f, 0.8f, 4.0f, 0.35f,
            1.1f, 0.65f, 0.15f, false, 0.08f,
            0.18f, -0.12f, 0.7f, 0.7f, 0.65f, 0.65f,
            0.75f, 0.3f, 4200.0f, 0.3f, 0.25f, SpatialFX::LfoWaveform::Random,
            -3.5f, 1.5f, 0.0009f, 0.0022f, 0.95f, 0.68f,
            6.5f, 0.5f, 0.11f,
            0.07f, 0.95f, 0.35f, 0.6f);
        };

    presets["Quiet Confidence"] = [this]() {
        usePreset(ModDelay::ModulationType::Triangle,
            120.0f, 0.4f, 0.45f, 0.5f, 1.2f, 0.12f,
            0.75f, 0.9f, -0.02f, false, 0.03f,
            0.04f, -0.03f, 0.2f, 0.2f, 0.35f, 0.35f,
            0.45f, 0.2f, 2400.0f, 0.15f, 0.2f, SpatialFX::LfoWaveform::Sine,
            1.2f, 0.2f, 0.0004f, 0.0012f, 0.45f, 0.3f,
            2.8f, 0.2f, 0.06f,
            0.02f, 0.45f, 0.25f, 0.3f);
        };

    presets["Falling Upwards"] = [this]() {
        usePreset(ModDelay::ModulationType::Sine,
            180.0f, 0.6f, 0.65f, 0.55f, 2.5f, 0.2f,
            1.0f, 0.7f, 0.05f, false, 0.04f,
            0.1f, -0.07f, 0.3f, 0.3f, 0.45f, 0.45f,
            0.6f, 0.22f, 1800.0f, 0.25f, 0.25f, SpatialFX::LfoWaveform::Triangle,
            2.2f, 0.4f, 0.0006f, 0.0018f, 0.5f, 0.5f,
            4.0f, 0.7f, 100.0f,
            0.04f, 0.85f, 0.3f, 0.55f);
        };

    presets["Molten Light"] = [this]() {
        usePreset(ModDelay::ModulationType::Sine,
            270.0f, 0.8f, 0.75f, 0.7f, 3.8f, 0.25f,
            1.3f, 0.9f, 0.1f, false, 0.06f,
            0.18f, -0.15f, 0.5f, 0.65f, 0.75f, 0.75f,
            0.4f, 0.12f, 4200.0f, 0.5f, 0.3f, SpatialFX::LfoWaveform::Sine,
            3.0f, 0.5f, 0.0007f, 0.002f, 0.75f, 0.55f,
            7.5f, 0.75f, 120.0f,
            0.06f, 0.7f, 0.2f, 0.7f);
        };

    presets["Ethereal Echo"] = [this]() {
        usePreset(ModDelay::ModulationType::Sine,
            350.0f, 0.6f, 0.65f, 0.75f, 2.5f, 0.3f,
            1.1f, 0.85f, -0.05f, false, 0.04f,
            0.12f, -0.1f, 0.4f, 0.55f, 0.65f, 0.65f,
            0.3f, 0.09f, 4200.0f, 0.4f, 0.3f, SpatialFX::LfoWaveform::Triangle,
            -2.5f, 1.2f, 0.0008f, 0.0025f, 0.75f, 0.6f,
            5.5f, 0.7f, 0.15f,
            0.05f, 0.9f, 0.15f, 0.65f);
        };

    presets["Lush Dreamscape"] = [this]() {
        usePreset(ModDelay::ModulationType::Triangle,
            400.0f, 0.7f, 0.75f, 0.8f, 3.0f, 0.35f,
            1.2f, 0.9f, 0.1f, false, 0.05f,
            0.15f, -0.12f, 0.5f, 0.65f, 0.75f, 0.75f,
            0.2f, 0.06f, 4200.0f, 0.6f, 0.3f, SpatialFX::LfoWaveform::Sine,
            -3.5f, 1.5f, 0.0009f, 0.0022f, 0.8f, 0.7f,
            6.5f, 0.8f, 0.2f,
            0.06f, 1.0f, 0.3f, 0.75f);
        };

    presets["Skin Contact"] = [this]() {
        usePreset(ModDelay::ModulationType::Triangle,
            90.0f, 0.35f, 0.4f, 0.4f, 1.0f, 0.08f,
            0.75f, 0.95f, -0.12f, false, 0.02f,
            0.02f, -0.018f, 0.12f, 0.2f, 0.5f, 0.5f,
            0.1f, 0.03f, 4200.0f, 0.4f, 0.3f, SpatialFX::LfoWaveform::Random,
            1.1f, 0.15f, 0.0001f, 0.0004f, 0.4f, 0.25f,
            4.0f, 0.45f, 44.0f,
            0.01f, 0.35f, 0.1f, 0.2f);
        };

    presets["Sonic Embrace"] = [this]() {
        usePreset(ModDelay::ModulationType::Sine,
            200.0f, 0.5f, 0.55f, 0.6f, 2.0f, 0.15f,
            0.8f, 0.9f, -0.08f, false, 0.03f,
            0.08f, -0.06f, 0.25f, 0.35f, 0.45f, 0.45f,
            0.25f, 0.1f, 4200.0f, 0.3f, 0.25f, SpatialFX::LfoWaveform::Sine,
            -1.5f, 0.25f, 0.0003f, 0.0008f, 0.5f, 0.35f,
            4.5f, 0.6f, 120.0f,
            0.03f, 0.75f, 0.25f, 0.5f);
        };

    presets["Strobe Heaven"] = [this]() {
        usePreset(ModDelay::ModulationType::Square,
            90.0f, 0.7f, 0.7f, 0.85f, 3.2f, 1.6f,
            0.4f, 0.6f, 0.3f, false, -0.1f,
            0.2f, -0.2f, 1.4f, 1.4f, 0.9f, 0.9f,
            1.0f, 0.2f, 4200.0f, 0.7f, 0.7f, SpatialFX::LfoWaveform::Triangle,
            -4.0f, 2.0f, 0.0012f, 0.0025f, 0.6f, 0.7f,
            8.5f, 0.75f, 150.0f,
            0.02f, 0.6f, 0.1f, 0.9f);
        };

    presets["Glass Flame"] = [this]() {
        usePreset(ModDelay::ModulationType::Sine,
            300.0f, 0.5f, 0.5f, 0.65f, 2.0f, 0.18f,
            1.0f, 0.8f, -0.05f, false, 0.0f,
            0.05f, -0.05f, 0.35f, 0.35f, 0.5f, 0.5f,
            0.65f, 0.15f, 3300.0f, 0.4f, 0.4f, SpatialFX::LfoWaveform::Sine,
            2.0f, 0.3f, 0.0004f, 0.0016f, 0.6f, 0.4f,
            5.5f, 0.8f, 110.0f,
            0.03f, 0.8f, 0.2f, 0.5f);
        };

    presets["Celestial Vault"] = [this]() {
        usePreset(ModDelay::ModulationType::Sine,
            320.0f, 0.6f, 0.6f, 0.4f, 2.0f, 0.15f,
            1.2f, 0.7f, 0.0f, false, -0.05f,
            0.02f, 0.02f, 0.15f, 0.15f, 0.5f, 0.5f,
            0.65f, 0.05f, 2800.0f, 0.5f, 0.5f, SpatialFX::LfoWaveform::Sine,
            2.0f, 0.2f, 0.001f, 0.005f, 0.75f, 0.4f,
            4.5f, 0.35f, 0.12f,
            0.12f, 1.0f, 0.45f, 0.7f);
        };

    presets["Deep Illusion"] = [this]() {
        usePreset(ModDelay::ModulationType::Triangle,
            280.0f, 0.4f, 0.4f, 0.3f, 1.0f, 0.2f,
            1.6f, 0.6f, 0.1f, false, 0.02f,
            -0.02f, 0.03f, 0.2f, 0.2f, 0.35f, 0.35f,
            0.6f, 0.1f, 3600.0f, 0.3f, 0.3f, SpatialFX::LfoWaveform::Triangle,
            2.5f, 0.15f, 0.001f, 0.004f, 0.5f, 0.35f,
            3.5f, 0.25f, 0.09f,
            0.06f, 0.9f, 0.6f, 0.5f);
        };

    presets["Ego Dissolve"] = [this]() {
        usePreset(ModDelay::ModulationType::Sine,
            500.0f, 0.3f, 0.3f, 0.5f, 3.0f, 0.4f,
            1.5f, 0.5f, 0.1f, false, 0.0f,
            0.1f, -0.1f, 0.35f, 0.35f, 0.5f, 0.5f,
            0.55f, 0.2f, 2400.0f, 0.4f, 0.4f, SpatialFX::LfoWaveform::Sine,
            1.5f, 0.3f, 0.0015f, 0.0065f, 0.75f, 0.45f,
            5.5f, 0.3f, 0.1f,
            0.1f, 0.88f, 0.7f, 0.6f);
        };

    presets["Memory Dust"] = [this]() {
        usePreset(ModDelay::ModulationType::Triangle,
            360.0f, 0.5f, 0.5f, 0.3f, 1.5f, 0.25f,
            1.3f, 0.65f, 0.05f, false, 0.03f,
            0.04f, 0.05f, 0.2f, 0.2f, 0.3f, 0.3f,
            0.5f, 0.15f, 1600.0f, 0.08f, 0.08f, SpatialFX::LfoWaveform::Random,
            2.8f, 0.25f, 0.001f, 0.005f, 0.7f, 0.4f,
            4.0f, 0.2f, 0.08f,
            0.07f, 0.9f, 0.55f, 0.45f);
        };

    presets["Gentle Slap"] = [this]() {
        usePreset(ModDelay::ModulationType::Sine,
            100.0f, 0.4f, 0.4f, 0.1f, 0.6f, 0.05f,
            0.2f, 0.4f, 0.0f, false, 0.02f,
            0.02f, -0.02f, 0.1f, 0.1f, 0.15f, 0.15f,
            0.25f, 0.2f, 4200.0f, 0.05f, 0.05f, SpatialFX::LfoWaveform::Sine,
            1.2f, 0.1f, 0.0003f, 0.0012f, 0.4f, 0.3f,
            2.5f, 0.15f, 0.05f,
            0.02f, 0.35f, 0.2f, 0.3f);
        };

    presets["Moon Dance"] = [this]() {
        usePreset(ModDelay::ModulationType::Sine,
            200.0f, 0.5f, 0.5f, 0.6f, 2.0f, 0.15f,
            1.2f, 0.7f, -0.05f, false, 0.02f,
            0.05f, -0.05f, 0.25f, 0.25f, 0.35f, 0.35f,
            0.45f, 0.4f, 4200.0f, 0.2f, 0.2f, SpatialFX::LfoWaveform::Triangle,
            1.5f, 0.25f, 0.0003f, 0.0008f, 0.5f, 0.35f,
            3.0f, 0.5f, 40.0f,
            0.03f, 0.75f, 0.25f, 0.5f);
        };

    presets["Biting Lips"] = [this]() {
        usePreset(ModDelay::ModulationType::Sine,
            80.0f, 0.45f, 0.45f, 0.25f, 0.5f, 0.08f,
            0.3f, 0.9f, -0.05f, false, 0.05f,
            0.02f, -0.02f, 0.1f, 0.1f, 0.15f, 0.15f,
            0.25f, 0.3f, 4200.0f, 0.1f, 0.1f, SpatialFX::LfoWaveform::Random,
            1.2f, 0.1f, 0.0003f, 0.0012f, 0.4f, 0.3f,
            2.5f, 0.15f, 0.05f,
            0.02f, 0.35f, 0.2f, 0.3f);
        };

    presets["Stormy Day"] = [this]() {
        usePreset(ModDelay::ModulationType::Sine,
            300.0f, 0.5f, 0.5f, 0.65f, 2.0f, 0.18f,
            1.0f, 0.8f, -0.05f, false, 0.0f,
            0.05f, -0.05f, 0.35f, 0.35f, 0.5f, 0.5f,
            0.65f, 0.45f, 4200.0f, 0.3f, 0.3f, SpatialFX::LfoWaveform::Random,
            2.0f, 0.3f, 0.0004f, 0.0016f, 0.6f, 0.4f,
            5.5f, 0.8f, 110.0f,
            0.03f, 0.8f, 0.2f, 0.5f);
        };

    presets["Summer Sunset"] = [this]() {
        usePreset(ModDelay::ModulationType::Sine,
            400.0f, 0.6f, 0.6f, 0.6f, 1.5f, 0.15f,
            1.1f, 0.8f, 0.1f, false, -0.04f,
            0.15f, -0.12f, 0.25f, 0.25f, 0.4f, 0.4f,
            0.75f, 0.15f, 4200.0f, 0.35f, 0.15f, SpatialFX::LfoWaveform::Sine,
            -0.8f, 0.8f, 0.0004f, 0.0015f, 0.6f, 0.5f,
            4.0f, 0.5f, 80.0f,
            0.06f, 1.0f, 0.3f, 0.75f);
        };

    presets["Ocean Waves"] = [this]() {
        usePreset(ModDelay::ModulationType::Triangle,
            250.0f, 0.5f, 0.5f, 0.6f, 2.5f, 0.2f,
            1.3f, 0.7f, 0.05f, false, 0.03f,
            0.08f, -0.06f, 0.3f, 0.3f, 0.45f, 0.45f,
            0.55f, 0.12f, 4200.0f, 0.3f, 0.12f, SpatialFX::LfoWaveform::Triangle,
            -1.5f, 0.4f, 0.0006f, 0.0022f, 0.65f, 0.5f,
            4.5f, 0.6f, 120.0f,
            0.05f, 0.9f, 0.25f, 0.55f);
        };

    presets["Crystal Clear"] = [this]() {
        usePreset(ModDelay::ModulationType::Sine,
            110.0f, 0.2f, 0.2f, 0.25f, 0.6f, 0.05f,
            0.6f, 0.6f, -0.15f, false, 0.01f,
            0.01f, -0.01f, 0.05f, 0.05f, 0.1f, 0.1f,
            0.2f, 0.05f, 4200.0f, 0.15f, 0.05f, SpatialFX::LfoWaveform::Sine,
            0.7f, 0.1f, 0.0002f, 0.0008f, 0.5f, 0.2f,
            1.2f, 0.25f, 400.0f,
            0.01f, 0.4f, 0.3f, 0.3f);
        };

    presets["Sweetest Memory"] = [this]() {
        usePreset(ModDelay::ModulationType::Sine,
            220.0f, 0.45f, 0.4f, 0.55f, 1.2f, 0.15f,
            1.2f, 0.65f, -0.05f, false, 0.02f,
            0.07f, -0.04f, 0.28f, 0.28f, 0.38f, 0.38f,
            0.55f, 0.05f, 4200.0f, 0.4f, 0.05f, SpatialFX::LfoWaveform::Sine,
            -1.2f, 0.25f, 0.0003f, 0.0009f, 0.55f, 0.35f,
            3.0f, 0.5f, 90.0f,
            0.06f, 0.8f, 0.3f, 0.65f);
        };
}
