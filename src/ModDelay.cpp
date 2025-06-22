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
}

void ModDelay::setParams(float dMs, float depth, float rate, float fbL, float fbR, float m) {
    params.delayMs.setTargetValue(dMs);
    params.modDepth.setTargetValue(depth);
    rawRate = rate;
    updateEffectiveRate();
    params.feedbackL.setTargetValue(juce::jlimit(0.0f, 0.95f, fbL));
    params.feedbackR.setTargetValue(juce::jlimit(0.0f, 0.95f, fbR));
    params.mix.setTargetValue(juce::jlimit(0.0f, 1.0f, m));
}

void ModDelay::process(juce::dsp::AudioBlock<float>& block) {
    auto* left = block.getChannelPointer(0);
    auto* right = block.getChannelPointer(1);
    const int numSamples = static_cast<int>(block.getNumSamples());

    float invSampleRate = 1.0f / sampleRate;

    for (int i = 0; i < numSamples; ++i) {
        params.modRateHz.getNextValue();
        params.delayMs.getNextValue();
        params.modDepth.getNextValue();
        params.feedbackL.getNextValue();
        params.feedbackR.getNextValue();
        params.mix.getNextValue();
        modulationTypeCrossfade.getNextValue();

        float dMs = std::max(params.delayMs.getCurrentValue(), 5.0f);
        float depth = params.modDepth.getCurrentValue();
        float fbL = params.feedbackL.getCurrentValue();
        float fbR = params.feedbackR.getCurrentValue();
        float wetMix = params.mix.getCurrentValue();
        float rateHz = params.modRateHz.getCurrentValue();

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
        modulationTypeCrossfade.setTargetValue(0.0f);
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
        return (bpm / 60.0f) / rawRate;
    }
    return rawRate;
}

void ModDelay::updateEffectiveRate() {
    params.modRateHz.setTargetValue(getEffectiveRateHz());
}