#include "ModDelay.h"
#include <juce_core/juce_core.h>
#include <juce_dsp/juce_dsp.h>

void ModDelay::prepare(const juce::dsp::ProcessSpec& spec) {
    sampleRate = static_cast<float>(spec.sampleRate);

    constexpr size_t maxDelaySamples = 44100; // 1 second max
    delayL.reset();
    delayR.reset();
    delayL.setMaximumDelayInSamples(maxDelaySamples);
    delayR.setMaximumDelayInSamples(maxDelaySamples);
    delayL.prepare(spec);
    delayR.prepare(spec);

    phase = 0.0f;
    currentModulationType = ModulationType::Sine;
    targetModulationType = ModulationType::Sine;
    modulationTypeCrossfade.reset(sampleRate, 0.02); // Short smoothing for modulation type
    modulationTypeCrossfade.setCurrentAndTargetValue(0.0f);

    constexpr double smoothingTime = 0.05; // Adjust as needed

    targetDelayMs.reset(sampleRate, smoothingTime);
    currentDelayMs.reset(sampleRate, smoothingTime);
    currentDelayMs.setCurrentAndTargetValue(0.0f); // Initial value
    targetModDepth.reset(sampleRate, smoothingTime);
    currentModDepth.reset(sampleRate, smoothingTime);
    currentModDepth.setCurrentAndTargetValue(0.0f); // Initial value
    targetModRateHz.reset(sampleRate, smoothingTime);
    currentModRateHz.reset(sampleRate, smoothingTime);
    currentModRateHz.setCurrentAndTargetValue(0.0f); // Initial value
    targetFeedbackL.reset(sampleRate, smoothingTime);
    currentFeedbackL.reset(sampleRate, smoothingTime);
    currentFeedbackL.setCurrentAndTargetValue(0.0f); // Initial value
    targetFeedbackR.reset(sampleRate, smoothingTime);
    currentFeedbackR.reset(sampleRate, smoothingTime);
    currentFeedbackR.setCurrentAndTargetValue(0.0f); // Initial value
    targetMix.reset(sampleRate, smoothingTime);
    currentMix.reset(sampleRate, smoothingTime);
    currentMix.setCurrentAndTargetValue(0.0f); // Initial value
}

void ModDelay::setParams(float dMs, float depth, float rate, float fbL, float fbR, float m) {
    targetDelayMs.setTargetValue(dMs);
    targetModDepth.setTargetValue(depth);
    rawRate = rate;
    updateEffectiveRate();
    targetFeedbackL.setTargetValue(juce::jlimit(0.0f, 0.95f, fbL));
    targetFeedbackR.setTargetValue(juce::jlimit(0.0f, 0.95f, fbR));
    targetMix.setTargetValue(juce::jlimit(0.0f, 1.0f, m));
}

void ModDelay::process(juce::dsp::AudioBlock<float>& block) {
    auto* left = block.getChannelPointer(0);
    auto* right = block.getChannelPointer(1);
    const int numSamples = static_cast<int>(block.getNumSamples());

    float invSampleRate = 1.0f / sampleRate;

    for (int i = 0; i < numSamples; ++i) {
        currentModRateHz.getNextValue(); // Advance smoothing
        currentDelayMs.getNextValue();
        currentModDepth.getNextValue();
        currentFeedbackL.getNextValue();
        currentFeedbackR.getNextValue();
        currentMix.getNextValue();
        modulationTypeCrossfade.getNextValue();

        float dMs = std::max(currentDelayMs.getCurrentValue(), 5.0f);
        float depth = currentModDepth.getCurrentValue();
        float fbL = currentFeedbackL.getCurrentValue();
        float fbR = currentFeedbackR.getCurrentValue();
        float wetMix = currentMix.getCurrentValue();
        float rateHz = currentModRateHz.getCurrentValue();

        float safeDepth = std::min({ depth, dMs - 5.0f, dMs * 0.4f });

        float mod1 = calculateModulation(phase, safeDepth, currentModulationType);
        float mod2 = calculateModulation(phase, safeDepth, targetModulationType);
        float mod = juce::jmap(modulationTypeCrossfade.getCurrentValue(), 0.0f, 1.0f, mod1, mod2);

        float delayLInSamples = (dMs + mod) * 0.001f * sampleRate;
        float delayRInSamples = (dMs - mod) * 0.001f * sampleRate;

        float inL = left[i];
        float inR = right[i];

        float outL = delayL.popSample(0, delayLInSamples, true);
        float outR = delayR.popSample(1, delayRInSamples, true);

        delayL.pushSample(0, inL + outL * fbL);
        delayR.pushSample(1, inR + outR * fbR);

        left[i] = inL * (1.0f - wetMix) + outL * wetMix;
        right[i] = inR * (1.0f - wetMix) + outR * wetMix;

        phase += rateHz * invSampleRate;
        if (phase >= 1.0f)
            phase -= 1.0f;
    }
    if (modulationTypeCrossfade.getTargetValue() == 1.0f && modulationTypeCrossfade.getCurrentValue() >= 1.0f) {
        currentModulationType = targetModulationType;
        modulationTypeCrossfade.setTargetValue(0.0f); // Reset for future changes
    }
}

void ModDelay::setModulationType(ModulationType newType) {
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
    bpm = newBpm;
    updateEffectiveRate();
}

float ModDelay::calculateModulation(float currentPhase, float depth, ModulationType type) {
    float phaseWrapped = currentPhase - std::floor(currentPhase);

    switch (type) {
    case ModulationType::Sine:
        return std::sin(juce::MathConstants<float>::twoPi * phaseWrapped) * depth;
    case ModulationType::Triangle:
        return (2.0f * std::abs(2.0f * (phaseWrapped - 0.5f)) - 1.0f) * depth;
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
    if (syncEnabled && rawRate > 0.0f) {
        return (bpm / 60.0f) / rawRate; // rate interpreted as note division
    }
    return rawRate;
}

void ModDelay::updateEffectiveRate() {
    targetModRateHz.setTargetValue(getEffectiveRateHz());
}