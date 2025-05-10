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

    constexpr double smoothingTime = 0.05;

    delayMs.reset(sampleRate, smoothingTime);
    modDepth.reset(sampleRate, smoothingTime);
    modRateHz.reset(sampleRate, smoothingTime);
    feedbackL.reset(sampleRate, smoothingTime);
    feedbackR.reset(sampleRate, smoothingTime);
    mix.reset(sampleRate, smoothingTime);
}

void ModDelay::setParams(float dMs, float depth, float rate, float fbL, float fbR, float m) {
    delayMs.setTargetValue(dMs);
    modDepth.setTargetValue(depth);
    rawRate = rate;
    updateEffectiveRate();

    feedbackL.setTargetValue(juce::jlimit(0.0f, 0.95f, fbL));
    feedbackR.setTargetValue(juce::jlimit(0.0f, 0.95f, fbR));
    mix.setTargetValue(juce::jlimit(0.0f, 1.0f, m));
}

void ModDelay::process(juce::dsp::AudioBlock<float>& block) {
    auto* left = block.getChannelPointer(0);
    auto* right = block.getChannelPointer(1);
    const int numSamples = static_cast<int>(block.getNumSamples());

    float rateHz = modRateHz.getNextValue();
    float invSampleRate = 1.0f / sampleRate;

    for (int i = 0; i < numSamples; ++i) {
        if (i % 32 == 0)
            rateHz = modRateHz.getNextValue();

        float dMs = delayMs.getNextValue();
        float depth = modDepth.getNextValue();
        float fbL = feedbackL.getNextValue();
        float fbR = feedbackR.getNextValue();
        float wetMix = mix.getNextValue();

        dMs = std::max(dMs, 5.0f);
        float safeDepth = std::min({ depth, dMs - 5.0f, dMs * 0.4f });

        float mod = calculateModulation(phase, safeDepth);

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
}

void ModDelay::setModulationType(ModulationType newType) {
    modulationType = newType;
}

void ModDelay::setSyncEnabled(bool shouldSync) {
    syncEnabled = shouldSync;
    updateEffectiveRate();
}

void ModDelay::setTempo(float newBpm) {
    bpm = newBpm;
    updateEffectiveRate();
}

float ModDelay::calculateModulation(float currentPhase, float depth) {
    float phaseWrapped = currentPhase - std::floor(currentPhase);

    switch (modulationType) {
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
    modRateHz.setTargetValue(getEffectiveRateHz());
}