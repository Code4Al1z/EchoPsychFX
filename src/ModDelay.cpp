#include "ModDelay.h"
#include <juce_core/juce_core.h>

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
    feedback.reset(sampleRate, smoothingTime);
    mix.reset(sampleRate, smoothingTime);
}

void ModDelay::setParams(float dMs, float depth, float rate, float fb, float m) {
    delayMs.setTargetValue(dMs);
    modDepth.setTargetValue(depth);
    modRateHz.setTargetValue(rate);
    feedback.setTargetValue(juce::jlimit(0.0f, 0.95f, fb));
    mix.setTargetValue(juce::jlimit(0.0f, 1.0f, m));
}

void ModDelay::process(juce::dsp::AudioBlock<float>& block) {
    auto* left = block.getChannelPointer(0);
    auto* right = block.getChannelPointer(1);
    const auto numSamples = static_cast<int>(block.getNumSamples());

    // Smooth parameters once per block
    const float targetDelayMs = delayMs.getTargetValue();
    const float targetDepth = modDepth.getTargetValue();
    const float targetFeedback = feedback.getTargetValue();
    const float targetMix = mix.getTargetValue();

    // Apply smoothing (linear smoothing happens over time, but we just get current value once per block)
    float currentDelayMs = delayMs.getNextValue();
    float currentDepth = modDepth.getNextValue();
    float currentFeedback = feedback.getNextValue();
    float currentMix = mix.getNextValue();

    // Limit modulation depth to avoid weird jumps or phasing
    currentDepth = std::min(currentDepth, currentDelayMs * 0.5f);

    // Apply gentle smoothing to modRateHz (manually - exponential smoothing)
    const float smoothingCoeff = 0.02f; // smaller = smoother, slower
    smoothedModRateHz += smoothingCoeff * (modRateHz.getTargetValue() - smoothedModRateHz);
    float currentRateHz = smoothedModRateHz;

    // Modulation processing
    for (int i = 0; i < numSamples; ++i) {
        float mod = std::sin(2.0f * juce::MathConstants<float>::pi * phase) * currentDepth;

        float modulatedDelayL = (currentDelayMs + mod) * 0.001f * sampleRate;
        float modulatedDelayR = (currentDelayMs - mod) * 0.001f * sampleRate;

        float delayedL = delayL.popSample(0, modulatedDelayL, true);
        float delayedR = delayR.popSample(1, modulatedDelayR, true);

        float inputL = left[i];
        float inputR = right[i];

        delayL.pushSample(0, inputL + delayedL * currentFeedback);
        delayR.pushSample(1, inputR + delayedR * currentFeedback);

        left[i] = inputL * (1.0f - currentMix) + delayedL * currentMix;
        right[i] = inputR * (1.0f - currentMix) + delayedR * currentMix;

        // Advance phase for modulation
        phase += currentRateHz / sampleRate;
        if (phase >= 1.0f)
            phase -= 1.0f;
    }
}

