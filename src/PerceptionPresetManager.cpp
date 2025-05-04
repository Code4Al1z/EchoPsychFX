#include "PerceptionPresetManager.h"

PerceptionPresetManager::PerceptionPresetManager(TiltEQComponent& tiltEQ,
	WidthBalancerComponent& width,
	ModDelayComponent& delay,
	SpatialFXComponent& spatial)
	: tiltEQComponent(tiltEQ), widthComponent(width), delayComponent(delay), spatialFXComponent(spatial)
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
            usePreset(ModDelay::ModulationType::Triangle,
                3.0f, 0.2f, -0.02f, 0.7f, 4.0f, 0.002f,
                0.8f, 0.5f, 0.0f, -0.2f,
                0.1f, -0.15f, 0.8f, 0.4f, 0.6f);
        };

    presets["Panic Room"] = [this]()
        {
            usePreset(ModDelay::ModulationType::Square,
                1.5f, 1.0f, 0.015f, 0.6f, 1.2f, 0.0015f,
                0.2f, 0.6f, 0.1f, 0.1f,
                0.25f, -0.25f, 1.0f, 0.9f, 0.7f);
        };

    presets["Intimacy"] = [this]()
        {
            usePreset(ModDelay::ModulationType::Sine,
                0.1f, 0.3f, 0.002f, 0.25f, 0.3f, 0.0005f,
                0.6f, 0.4f, 0.0f, -0.1f,
                0.05f, -0.05f, 0.3f, 0.2f, 0.3f);
        };

    presets["Blade Runner"] = [this]()
        {
            usePreset(ModDelay::ModulationType::SawtoothDown,
                2.2f, 0.5f, 0.5f, 0.5f, 2.5f, 0.003f,
                1.0f, 0.2f, 0.05f, -0.3f,
                0.3f, -0.2f, 0.5f, 0.3f, 0.8f);
        };

    presets["Alien Abduction"] = [this]()
        {
            usePreset(ModDelay::ModulationType::Sine,
                1.8f, 0.4f, 0.4f, 0.65f, 2.0f, 0.0022f,
                0.9f, 0.5f, 0.0f, 0.0f,
                0.4f, -0.4f, 1.2f, 1.0f, 0.7f);
        };

    presets["Glass Tunnel"] = [this]()
        {
            usePreset(ModDelay::ModulationType::SawtoothUp,
                0.9f, 0.2f, 0.15f, 0.5f, 0.6f, 0.0012f,
                0.75f, 0.6f, 0.0f, 0.15f,
                0.2f, -0.2f, 0.45f, 0.35f, 0.6f);
        };
}

void PerceptionPresetManager::usePreset(ModDelay::ModulationType type, float delayTime, float feedbackLeft, float feedbackRight,
										float mix, float delayModDepth, float delayModRate,
										float width, float midSideBalance, float mono, float tiltEQ,
										float phaseOffsetL, float phaseOffsetR, float modulationRate,
										float modulationDepth, float wetDryMix)
{
	delayComponent.setModulationType(type);
	delayComponent.setDelayTime(delayTime);
	delayComponent.setFeedbackLeft(feedbackLeft);
	delayComponent.setFeedbackRight(feedbackRight);
	delayComponent.setMix(mix);
	delayComponent.setModDepth(delayModDepth);
	delayComponent.setModRate(delayModRate);

	widthComponent.setWidth(width);
	widthComponent.setMidSideBalance(midSideBalance);
	widthComponent.setMono(mono);

	tiltEQComponent.setTilt(tiltEQ);

	spatialFXComponent.setPhaseOffsetLeft(phaseOffsetL);
	spatialFXComponent.setPhaseOffsetRight(phaseOffsetR);
	spatialFXComponent.setModulationRate(modulationRate);
	spatialFXComponent.setModulationDepth(modulationDepth);
	spatialFXComponent.setWetDryMix(wetDryMix);
}
