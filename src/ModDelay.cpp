#include "ModDelay.h"
#include <juce_core/juce_core.h>

void ModDelay::prepare(const juce::dsp::ProcessSpec& spec) {
    sampleRate = static_cast<float>(spec.sampleRate);

    auto maxDelaySamples = static_cast<size_t>(sampleRate); // 1 second max delay

    // Prepare the delay lines
    delayL.reset();
    delayR.reset();
    delayL.setMaximumDelayInSamples(maxDelaySamples);
    delayR.setMaximumDelayInSamples(maxDelaySamples);

    // Prepare the delay lines for processing
    delayL.prepare(spec);
    delayR.prepare(spec);

    // Reset phase for modulation
    phase = 0.0f;
}

void ModDelay::setParams(float dMs, float depth, float rate, float fb, float m) {
    delayMs = dMs;
    modDepth = depth;
    modRateHz = rate;
    feedback = juce::jlimit(0.0f, 0.95f, fb); // Limit feedback to avoid runaway
    mix = juce::jlimit(0.0f, 1.0f, m); // Limit mix for proper wet/dry balance
}

void ModDelay::process(juce::dsp::AudioBlock<float>& block) {
    auto* left = block.getChannelPointer(0);
    auto* right = block.getChannelPointer(1);
    const auto numSamples = static_cast<int>(block.getNumSamples());

    for (int i = 0; i < numSamples; ++i) {
        // Modulation based on sine wave
        float mod = std::sin(2.0f * juce::MathConstants<float>::pi * phase) * modDepth;

        // Calculate modulated delay times for left and right channels
        float modulatedDelayL = (delayMs + mod) * 0.001f * sampleRate;
        float modulatedDelayR = (delayMs - mod) * 0.001f * sampleRate;

        // Get the delayed samples from each delay line
        float delayedL = delayL.popSample(0, modulatedDelayL, true);
        float delayedR = delayR.popSample(1, modulatedDelayR, true);

        // Apply feedback to the input
        float inputL = left[i];
        float inputR = right[i];

        // Push the new samples (with feedback) into the delay lines
        delayL.pushSample(0, inputL + delayedL * feedback);
        delayR.pushSample(1, inputR + delayedR * feedback);

        // Mix the original input and delayed sample based on the mix parameter
        left[i] = inputL * (1.0f - mix) + delayedL * mix;
        right[i] = inputR * (1.0f - mix) + delayedR * mix;

        // Increment phase for modulation and ensure it stays within [0, 1]
        phase += modRateHz / sampleRate;
        if (phase >= 1.0f)
            phase -= 1.0f;
    }
}
