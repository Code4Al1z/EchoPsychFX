#include "ModDelay.h"
#include <juce_core/juce_core.h>
#include <juce_dsp/juce_dsp.h>

void ModDelay::prepare(const juce::dsp::ProcessSpec& spec) {
    sampleRate = static_cast<float>(spec.sampleRate);

    auto maxDelaySamples = static_cast<size_t>(sampleRate); // 1 second max delay
    delayL.reset();
    delayR.reset();
    delayL.setMaximumDelayInSamples(maxDelaySamples);
    delayR.setMaximumDelayInSamples(maxDelaySamples);
    delayL.prepare(spec);
    delayR.prepare(spec);

    phase = 0.0f;

    // Set smoothing time (50 ms = 0.05 seconds)
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
    modRateHz.setTargetValue(rate);
    feedbackL.setTargetValue(juce::jlimit(0.0f, 0.95f, fbL));
    feedbackR.setTargetValue(juce::jlimit(0.0f, 0.95f, fbR));
    mix.setTargetValue(juce::jlimit(0.0f, 1.0f, m));
}

void ModDelay::process(juce::dsp::AudioBlock<float>& block) {
    auto* left = block.getChannelPointer(0);
    auto* right = block.getChannelPointer(1);
    const auto numSamples = static_cast<int>(block.getNumSamples());

    for (int i = 0; i < numSamples; ++i) {
        // Get the next smoothed parameter values for this sample
        float currentDelayMs = delayMs.getNextValue();
        float currentDepth = modDepth.getNextValue();
        float currentFeedbackL = feedbackL.getNextValue();
        float currentFeedbackR = feedbackR.getNextValue();
        float currentMix = mix.getNextValue();
        float currentRateHz = modRateHz.getNextValue();

        currentDepth = std::min(currentDepth, currentDelayMs * 0.5f);

        // Calculate modulation value based on selected waveform and phase
        float mod = calculateModulation(currentRateHz, currentDepth, phase);

        float modulatedDelayL = (currentDelayMs + mod) * 0.001f * sampleRate;
        float modulatedDelayR = (currentDelayMs - mod) * 0.001f * sampleRate;

        float delayedL = delayL.popSample(0, modulatedDelayL, true);
        float delayedR = delayR.popSample(1, modulatedDelayR, true);

        float inputL = left[i];
        float inputR = right[i];

        delayL.pushSample(0, inputL + delayedL * currentFeedbackL);
        delayR.pushSample(1, inputR + delayedR * currentFeedbackR);

        left[i] = inputL * (1.0f - currentMix) + delayedL * currentMix;
        right[i] = inputR * (1.0f - currentMix) + delayedR * currentMix;

        phase += currentRateHz / sampleRate;
        if (phase >= 1.0f)
            phase -= 1.0f;
    }
}

void ModDelay::setModulationType(ModulationType newType)
{
    modulationType = newType;
}

float ModDelay::calculateModulation(float rateHz, float depth, float currentPhase)
{
    switch (modulationType)
    {
    case ModulationType::Sine:
        return std::sin(2.0f * juce::MathConstants<float>::pi * currentPhase) * depth;
    case ModulationType::Triangle:
        return (2.0f * std::abs(2.0f * (currentPhase - std::floor(currentPhase + 0.5f))) - 1.0f) * depth;
    case ModulationType::Square:
        return (currentPhase < 0.5f ? 1.0f : -1.0f) * depth;
    case ModulationType::SawtoothUp:
        return (2.0f * (currentPhase - std::floor(currentPhase))) * depth - depth;
    case ModulationType::SawtoothDown:
        return (1.0f - 2.0f * (currentPhase - std::floor(currentPhase))) * depth - depth;
    default:
        return 0.0f;
    }
}