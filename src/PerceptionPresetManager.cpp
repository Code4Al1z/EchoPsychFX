#include "PerceptionPresetManager.h"

PerceptionPresetManager::PerceptionPresetManager(TiltEQComponent& tiltEQ,
    WidthBalancerComponent& width,
    ModDelayComponent& delay,
    SpatialFXComponent& spatial,
    MicroPitchDetuneComponent& microPitch,
    ExciterSaturationComponent& exciterSaturation)
	: tiltEQComponent(tiltEQ), widthComponent(width), delayComponent(delay), spatialFXComponent(spatial), microPitchComponent(microPitch), exciterSaturationComponent(exciterSaturation)
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
                0.08f, -0.05f, 0.3f, 0.6f, 0.7f, // phaseOffsetL, phaseOffsetR, modulationRate, modulationDepth, wetDryMix
                3.0f, 0.25f, 0.0015f, 0.006f, 0.8f, 0.5f, // detuneAmount, lfoRate, lfoDepth, delayCentre, stereoSeparation, mix
                6.0f, 0.4f, 0.1f); // drive, exciterMix, highpass
        };

    presets["Panic Room"] = [this]()
        {
            usePreset(ModDelay::ModulationType::Square, // modType
                150.0f, 0.8f, 0.7f, // delayTime, feedbackL, feedbackR
                0.9f, 6.0f, 1.5f, // modMix, modDepth, modRate
                0.3f, 0.2f, 0.2f, false, -0.15f, // width, intensity, midSideBalance, mono, tiltEQ
                0.15f, -0.18f, 1.2f, 0.85f, 0.9f, // phaseOffsetL, phaseOffsetR, modulationRate, modulationDepth, wetDryMix
                -5.0f, 3.0f, 0.0008f, 0.003f, 0.3f, 0.7f, // detuneAmount, lfoRate, lfoDepth, delayCentre, stereoSeparation, mix
                7.5f, 0.65f, 0.2f); // drive, exciterMix, highpass
        };

    presets["Intimacy"] = [this]()
        {
            usePreset(ModDelay::ModulationType::Sine, // modType
                80.0f, 0.3f, 0.35f, // delayTime, feedbackL, feedbackR
                0.4f, 1.0f, 0.1f, // modMix, modDepth, modRate
                0.8f, 0.9f, -0.1f, false, 0.05f, // width, intensity, midSideBalance, mono, tiltEQ
                0.03f, -0.02f, 0.15f, 0.25f, 0.35f, // phaseOffsetL, phaseOffsetR, modulationRate, modulationDepth, wetDryMix
                1.5f, 0.1f, 0.0002f, 0.001f, 0.4f, 0.3f, // detuneAmount, lfoRate, lfoDepth, delayCentre, stereoSeparation, mix
                2.0f, 0.2f, 0.05f); // drive, exciterMix, highpass
        };

    presets["Blade Runner"] = [this]()
        {
            usePreset(ModDelay::ModulationType::SawtoothDown, // modType
                550.0f, 0.65f, 0.6f, // delayTime, feedbackL, feedbackR
                0.7f, 3.0f, 0.3f, // modMix, modDepth, modRate
                1.2f, 0.7f, 0.1f, false, -0.08f, // width, intensity, midSideBalance, mono, tiltEQ
                0.2f, -0.1f, 0.4f, 0.5f, 0.85f, // phaseOffsetL, phaseOffsetR, modulationRate, modulationDepth, wetDryMix
                -2.0f, 0.6f, 0.0018f, 0.004f, 0.75f, 0.6f, // detuneAmount, lfoRate, lfoDepth, delayCentre, stereoSeparation, mix
                4.5f, 0.55f, 0.12f); // drive, exciterMix, highpass
        };

    presets["Alien Abduction"] = [this]()
        {
            usePreset(ModDelay::ModulationType::Sine, // modType
                350.0f, 0.55f, 0.6f, // delayTime, feedbackL, feedbackR
                0.85f, 4.5f, 0.6f, // modMix, modDepth, modRate
                0.9f, 0.75f, 0.05f, false, -0.07f, // width, intensity, midSideBalance, mono, tiltEQ
                0.18f, -0.22f, 0.9f, 0.7f, 0.8f, // phaseOffsetL, phaseOffsetR, modulationRate, modulationDepth, wetDryMix
                4.0f, 1.2f, 0.001f, 0.0025f, 0.9f, 0.65f, // detuneAmount, lfoRate, lfoDepth, delayCentre, stereoSeparation, mix
                5.5f, 0.6f, 0.18f); // drive, exciterMix, highpass
        };

    presets["Glass Tunnel"] = [this]()
        {
            usePreset(ModDelay::ModulationType::SawtoothUp, // modType
                220.0f, 0.45f, 0.5f, // delayTime, feedbackL, feedbackR
                0.65f, 1.8f, 0.15f, // modMix, modDepth, modRate
                0.7f, 0.85f, -0.05f, false, 0.02f, // width, intensity, midSideBalance, mono, tiltEQ
                0.05f, -0.08f, 0.25f, 0.4f, 0.65f, // phaseOffsetL, phaseOffsetR, modulationRate, modulationDepth, wetDryMix
                1.0f, 0.3f, 0.0005f, 0.0015f, 0.55f, 0.4f, // detuneAmount, lfoRate, lfoDepth, delayCentre, stereoSeparation, mix
                3.5f, 0.35f, 0.08f); // drive, exciterMix, highpass
        };

    presets["Dream Logic"] = [this]()
        {
            usePreset(ModDelay::ModulationType::Sine, // modType
                280.0f, 0.5f, 0.55f, // delayTime, feedbackL, feedbackR
                0.75f, 2.2f, 0.25f, // modMix, modDepth, modRate
                0.88f, 0.78f, 0.02f, false, -0.03f, // width, intensity, midSideBalance, mono, tiltEQ
                0.06f, -0.04f, 0.35f, 0.48f, 0.55f, // phaseOffsetL, phaseOffsetR, modulationRate, modulationDepth, wetDryMix
                1.8f, 0.5f, 0.0007f, 0.002f, 0.6f, 0.45f, // detuneAmount, lfoRate, lfoDepth, delayCentre, stereoSeparation, mix
                3.0f, 0.3f, 0.07f); // drive, exciterMix, highpass
        };

    presets["Womb Space"] = [this]()
        {
            usePreset(ModDelay::ModulationType::Sine, // modType
                50.0f, 0.25f, 0.3f, // delayTime, feedbackL, feedbackR
                0.3f, 0.8f, 0.08f, // modMix, modDepth, modRate
                0.65f, 0.95f, -0.15f, true, 0.01f, // width, intensity, midSideBalance, mono, tiltEQ
                0.01f, -0.015f, 0.1f, 0.18f, 0.25f, // phaseOffsetL, phaseOffsetR, modulationRate, modulationDepth, wetDryMix
                0.5f, 0.15f, 0.00015f, 0.0005f, 0.35f, 0.2f, // detuneAmount, lfoRate, lfoDepth, delayCentre, stereoSeparation, mix
                1.5f, 0.25f, 0.03f); // drive, exciterMix, highpass
        };

    presets["Bipolar Bloom"] = [this]()
        {
            usePreset(ModDelay::ModulationType::Triangle, // modType
                450.0f, 0.9f, 0.5f, // delayTime, feedbackL, feedbackR
                0.8f, 4.0f, 0.35f, // modMix, modDepth, modRate
                1.1f, 0.65f, 0.15f, false, 0.08f, // width, intensity, midSideBalance, mono, tiltEQ
                0.18f, -0.12f, 0.7f, 0.65f, 0.75f, // phaseOffsetL, phaseOffsetR, modulationRate, modulationDepth, wetDryMix
                -3.5f, 1.5f, 0.0009f, 0.0022f, 0.95f, 0.68f, // detuneAmount, lfoRate, lfoDepth, delayCentre, stereoSeparation, mix
                6.5f, 0.5f, 0.11f); // drive, exciterMix, highpass
        };

    presets["Quiet Confidence"] = [this]()
        {
            usePreset(ModDelay::ModulationType::Triangle, // modType
                120.0f, 0.4f, 0.45f, // delayTime, feedbackL, feedbackR
                0.5f, 1.2f, 0.12f, // modMix, modDepth, modRate
                0.75f, 0.9f, -0.02f, false, 0.03f, // width, intensity, midSideBalance, mono, tiltEQ
                0.04f, -0.03f, 0.2f, 0.35f, 0.45f, // phaseOffsetL, phaseOffsetR, modulationRate, modulationDepth, wetDryMix
                1.2f, 0.2f, 0.0004f, 0.0012f, 0.45f, 0.3f, // detuneAmount, lfoRate, lfoDepth, delayCentre, stereoSeparation, mix
                2.8f, 0.2f, 0.06f); // drive, exciterMix, highpass
        };

    presets["Falling Upwards"] = [this]()
        {
            usePreset(ModDelay::ModulationType::Sine, // modType
                180.0f, 0.6f, 0.65f, // delayTime, feedbackL, feedbackR
                0.55f, 2.5f, 0.2f, // modMix, modDepth, modRate
                1.0f, 0.7f, 0.05f, false, 0.04f, // width, intensity, midSideBalance, mono, tiltEQ
                0.1f, -0.07f, 0.3f, 0.45f, 0.6f, // phaseOffsetL, phaseOffsetR, modulationRate, modulationDepth, wetDryMix
                2.2f, 0.4f, 0.0006f, 0.0018f, 0.5f, 0.5f, // detuneAmount, lfoRate, lfoDepth, delayCentre, stereoSeparation, mix
                4.0f, 0.7f, 100.0f); // drive, exciterMix, highpass
        };

    presets["Molten Light"] = [this]()
        {
            usePreset(ModDelay::ModulationType::SawtoothDown, // modType
                270.0f, 0.8f, 0.75f, // delayTime, feedbackL, feedbackR
                0.7f, 3.8f, 0.25f, // modMix, modDepth, modRate
                1.3f, 0.9f, 0.1f, false, 0.06f, // width, intensity, midSideBalance, mono, tiltEQ
                0.18f, -0.15f, 0.5f, 0.65f, 0.75f, // phaseOffsetL, phaseOffsetR, modulationRate, modulationDepth, wetDryMix
                3.0f, 0.5f, 0.0007f, 0.002f, 0.7f, 0.55f, // detuneAmount, lfoRate, lfoDepth, delayCentre, stereoSeparation, mix
                7.5f, 0.85f, 120.0f); // drive, exciterMix, highpass
        };

    presets["Ethereal Echo"] = [this]()
        {
            usePreset(ModDelay::ModulationType::Sine, // modType
                350.0f, 0.6f, 0.65f, // delayTime, feedbackL, feedbackR
                0.75f, 2.5f, 0.3f, // modMix, modDepth, modRate
                1.1f, 0.85f, -0.05f, false, 0.04f, // width, intensity, midSideBalance, mono, tiltEQ
                0.12f, -0.1f, 0.4f, 0.55f, 0.65f, // phaseOffsetL, phaseOffsetR, modulationRate, modulationDepth, wetDryMix
                -2.5f, 1.2f, 0.0008f, 0.0025f, 0.75f, 0.6f, // detuneAmount, lfoRate, lfoDepth, delayCentre, stereoSeparation, mix
                5.5f, 0.7f, 0.15f); // drive, exciterMix, highpass
        };

    presets["Lush Dreamscape"] = [this]()
        {
            usePreset(ModDelay::ModulationType::Triangle, // modType
                400.0f, 0.7f, 0.75f, // delayTime, feedbackL, feedbackR
                0.8f, 3.0f, 0.35f, // modMix, modDepth, modRate
                1.2f, 0.9f, 0.1f, false, 0.05f, // width, intensity, midSideBalance, mono, tiltEQ
                0.15f, -0.12f, 0.5f, 0.65f, 0.75f, // phaseOffsetL, phaseOffsetR, modulationRate, modulationDepth, wetDryMix
                -3.5f, 1.5f, 0.0009f, 0.0022f, 0.8f, 0.7f, // detuneAmount, lfoRate, lfoDepth, delayCentre, stereoSeparation, mix
                6.5f, 0.8f, 0.2f); // drive, exciterMix, highpass
        };

    presets["Skin Contact"] = [this]()
        {
            usePreset(ModDelay::ModulationType::Triangle, // modType
                90.0f, 0.35f, 0.4f, // delayTime, feedbackL, feedbackR
                0.4f, 1.0f, 0.08f, // modMix, modDepth, modRate
                0.6f, 0.95f, -0.12f, true, 0.02f, // width, intensity, midSideBalance, mono, tiltEQ
                0.02f, -0.018f, 0.12f, 0.2f, 0.3f, // phaseOffsetL, phaseOffsetR, modulationRate, modulationDepth, wetDryMix
                1.1f, 0.15f, 0.0001f, 0.0004f, 0.4f, 0.25f, // detuneAmount, lfoRate, lfoDepth, delayCentre, stereoSeparation, mix
                4.0f, 0.6f, 90.0f); // drive, exciterMix, highpass
        };

    presets["Sonic Embrace"] = [this]()
        {
            usePreset(ModDelay::ModulationType::Sine, // modType
                200.0f, 0.5f, 0.55f, // delayTime, feedbackL, feedbackR
                0.6f, 2.0f, 0.15f, // modMix, modDepth, modRate
                0.8f, 0.9f, -0.08f, false, 0.03f, // width, intensity, midSideBalance, mono, tiltEQ
                0.08f, -0.06f, 0.25f, 0.35f, 0.45f, // phaseOffsetL, phaseOffsetR, modulationRate, modulationDepth, wetDryMix
                -1.5f, 0.25f, 0.0003f, 0.0008f, 0.5f, 0.35f, // detuneAmount, lfoRate, lfoDepth, delayCentre, stereoSeparation, mix
                4.5f, 0.6f, 120.0f); // drive, exciterMix, highpass
        };

    presets["Strobe Heaven"] = [this]()
        {
            usePreset(ModDelay::ModulationType::Square, // modType
                90.0f, 0.7f, 0.7f, // delayTime, feedbackL, feedbackR
                0.85f, 3.2f, 1.6f, // modMix, modDepth, modRate
                0.4f, 0.6f, 0.3f, false, -0.1f, // width, intensity, midSideBalance, mono, tiltEQ
                0.2f, -0.2f, 1.4f, 0.9f, 1.0f, // phaseOffsetL, phaseOffsetR, modulationRate, modulationDepth, wetDryMix
                -4.0f, 2.0f, 0.0012f, 0.0025f, 0.6f, 0.7f, // detuneAmount, lfoRate, lfoDepth, delayCentre, stereoSeparation, mix
                8.5f, 0.75f, 150.0f); // drive, exciterMix, highpass
        };

    presets["Glass Flame"] = [this]()
        {
            usePreset(ModDelay::ModulationType::Sine, // modType
                300.0f, 0.5f, 0.5f, // delayTime, feedbackL, feedbackR
                0.65f, 2.0f, 0.18f, // modMix, modDepth, modRate
                1.0f, 0.8f, -0.05f, false, 0.0f, // width, intensity, midSideBalance, mono, tiltEQ
                0.05f, -0.05f, 0.35f, 0.5f, 0.65f, // phaseOffsetL, phaseOffsetR, modulationRate, modulationDepth, wetDryMix
                2.0f, 0.3f, 0.0004f, 0.0016f, 0.6f, 0.4f, // detuneAmount, lfoRate, lfoDepth, delayCentre, stereoSeparation, mix
                5.5f, 0.8f, 110.0f); // drive, exciterMix, highpass
        };
}

void PerceptionPresetManager::usePreset(ModDelay::ModulationType type, float delayTime, float feedbackLeft, float feedbackRight,
    float modMix, float delayModDepth, float delayModRate,
    float width, float intensity, float midSideBalance, bool mono, float tiltEQ,
    float phaseOffsetL, float phaseOffsetR, float modulationRate, float modulationDepth, float wetDryMix,
    float detuneAmount, float lfoRate, float lfoDepth, float delayCentre, float stereoSeparation, float mix,
    float drive, float exciterMix, float highpass)
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
    spatialFXComponent.setModulationRate(modulationRate);
    spatialFXComponent.setModulationDepth(modulationDepth);
    spatialFXComponent.setWetDryMix(wetDryMix);

    microPitchComponent.setDetuneAmount(detuneAmount);
    microPitchComponent.setLfoRate(lfoRate);
    microPitchComponent.setLfoDepth(lfoDepth);
    microPitchComponent.setDelayCentre(delayCentre);
    microPitchComponent.setStereoSeparation(stereoSeparation);
    microPitchComponent.setMix(mix);

	exciterSaturationComponent.setDrive(drive);
	exciterSaturationComponent.setMix(exciterMix);
	exciterSaturationComponent.setHighpass(highpass);
}