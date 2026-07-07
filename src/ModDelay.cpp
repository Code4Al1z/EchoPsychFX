#include "ModDelay.h"
#include <juce_core/juce_core.h>
#include <juce_dsp/juce_dsp.h>

void ModDelay::prepare(const juce::dsp::ProcessSpec& spec) {
    sampleRate = static_cast<float>(spec.sampleRate);

    constexpr size_t maxDelaySamples = 44100;
    delayL.reset();
    delayR.reset();
    delayL.setMaximumDelayInSamples(maxDelaySamples);
    delayR.setMaximumDelayInSamples(maxDelaySamples);
    delayL.prepare(spec);
    delayR.prepare(spec);

    modulationTypeCrossfade.reset(sampleRate, 0.02);
    modulationTypeCrossfade.setCurrentAndTargetValue(0.0f);

    resetState();
}

void ModDelay::resetState() {
    phase = 0.0f;
    currentModulationType = ModulationType::Sine;
    targetModulationType = ModulationType::Sine;
    modulationTypeCrossfade.setCurrentAndTargetValue(0.0f);
    params.reset(sampleRate, 0.05);

    currentRateHz = 0.25f;
}

void ModDelay::setParams(float dMs, float depth, float rate, float fbL, float fbR, float m) {
    // Smoothly transition to the new values without resetting the smoothing state
    params.delayMs.setTargetValue(dMs);
    params.modDepth.setTargetValue(depth);

    rawRate = rate;
    updateEffectiveRate();

    params.feedbackL.setTargetValue(juce::jlimit(0.0f, 0.95f, fbL));
    params.feedbackR.setTargetValue(juce::jlimit(0.0f, 0.95f, fbR));
    params.mix.setTargetValue(juce::jlimit(0.0f, 1.0f, m));
}

void ModDelay::process(juce::dsp::AudioBlock<float>& block) {
    // Safety check for stereo
    if (block.getNumChannels() < 2)
        return;

    auto* left = block.getChannelPointer(0);
    auto* right = block.getChannelPointer(1);
    const int numSamples = static_cast<int>(block.getNumSamples());

    float invSampleRate = 1.0f / sampleRate;
    float crossfade = 0.0f;

    for (int i = 0; i < numSamples; ++i) {
        // Get next smoothed values
        float dMs = std::max(params.delayMs.getNextValue(), 5.0f);
        float depth = params.modDepth.getNextValue();
        float rateHz = currentRateHz;
        float fbL = juce::jlimit(0.0f, 0.95f, params.feedbackL.getNextValue());
        float fbR = juce::jlimit(0.0f, 0.95f, params.feedbackR.getNextValue());
        float wetMix = params.mix.getNextValue();
        crossfade = modulationTypeCrossfade.getNextValue();

        // Calculate safe modulation depth - ensure we stay away from boundaries
        float safeDepth = std::min(depth, (dMs - 5.0f) * 0.8f);
        safeDepth = std::max(safeDepth, 0.0f);

        // Calculate modulation for both waveform types (for crossfading)
        float mod1 = calculateModulation(phase, safeDepth, currentModulationType);
        float mod2 = calculateModulation(phase, safeDepth, targetModulationType);
        float mod = juce::jmap(crossfade, 0.0f, 1.0f, mod1, mod2);

        // Calculate delay times with modulation (stereo spreading)
        float delayLInSamples = (dMs + mod) * 0.001f * sampleRate;
        float delayRInSamples = (dMs - mod) * 0.001f * sampleRate;

        // Get input samples
        float inL = left[i];
        float inR = right[i];

        // Read from delay lines
        float outL = delayL.popSample(0, delayLInSamples, true);
        float outR = delayR.popSample(1, delayRInSamples, true);

        // Write to delay lines with feedback
        delayL.pushSample(0, inL + outL * fbL);
        delayR.pushSample(1, inR + outR * fbR);

        // Mix dry and wet signals
        left[i] = inL * (1.0f - wetMix) + outL * wetMix;
        right[i] = inR * (1.0f - wetMix) + outR * wetMix;

        // Advance phase
        phase += rateHz * invSampleRate;
        phase = std::fmod(phase, 1.0f);
    }

    // Check if crossfade is complete
    constexpr float epsilon = 0.001f;
    if (std::abs(modulationTypeCrossfade.getTargetValue() - 1.0f) < epsilon &&
        crossfade >= (1.0f - epsilon)) {
        currentModulationType = targetModulationType;
        modulationTypeCrossfade.setCurrentAndTargetValue(0.0f);
    }
}

void ModDelay::setModulationType(ModulationType newType) {
    if (!isValidModulationType(newType))
        return;

    if (newType != targetModulationType) {
        targetModulationType = newType;
        modulationTypeCrossfade.setTargetValue(1.0f);
    }
}

void ModDelay::setSyncEnabled(bool shouldSync) {
    syncEnabled = shouldSync;
    updateEffectiveRate();
}

void ModDelay::setTempo(float newBpm) {
    bpm = juce::jlimit(20.0f, 999.0f, newBpm);
    updateEffectiveRate();
}

float ModDelay::calculateModulation(float currentPhase, float depth, ModulationType type) {
    float phaseWrapped = currentPhase - std::floor(currentPhase);

    switch (type) {
    case ModulationType::Sine:
        return std::sin(juce::MathConstants<float>::twoPi * phaseWrapped) * depth;
    case ModulationType::Triangle:
        return (1.0f - 4.0f * std::abs(phaseWrapped - 0.5f)) * depth;
    case ModulationType::Square:
        return (phaseWrapped < 0.5f ? 1.0f : -1.0f) * depth;
    case ModulationType::SawtoothUp:
        return (2.0f * phaseWrapped - 1.0f) * depth;
    case ModulationType::SawtoothDown:
        return (1.0f - 2.0f * phaseWrapped) * depth;
    default:
        return 0.0f;
    }
}

float ModDelay::getEffectiveRateHz() const {
    if (syncEnabled) {
        // Note divisions as fractions of a beat: 1/16=0.0625, 1/8=0.125, 1/4=0.25, 1/2=0.5, 1 bar=1.0, 2 bars=2.0
        const float noteDivisions[] = { 0.0625f, 0.125f, 0.25f, 0.5f, 1.0f, 2.0f };
        const int numDivisions = 6;

        // 1. Convert rawRate (0.01 to 10.0 Hz) down to a normalized 0.0 to 1.0 range
        float normalizedRate = (rawRate - 0.01f) / (10.0f - 0.01f);

        // 2. Map that 0.0-1.0 range smoothly across the 6 available array choices (indices 0 to 5)
        int index = juce::jlimit(0, numDivisions - 1, juce::roundToInt(normalizedRate * (numDivisions - 1)));
        float bestDiv = noteDivisions[index];

        // 3. Scale by the host BPM
        float beatsPerSecond = bpm / 60.0f;
        return beatsPerSecond * bestDiv;
    }

    return juce::jlimit(0.01f, 20.0f, rawRate);
}

void ModDelay::updateEffectiveRate() {
    currentRateHz = getEffectiveRateHz();
}

bool ModDelay::isValidModulationType(ModulationType type) const {
    int typeValue = static_cast<int>(type);
    return typeValue >= static_cast<int>(ModulationType::Sine) &&
        typeValue <= static_cast<int>(ModulationType::SawtoothDown);
}