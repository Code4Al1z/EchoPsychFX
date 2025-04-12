#include "ModDelay.h"
#include <juce_core/juce_core.h>

void ModDelay::prepare(const juce::dsp::ProcessSpec& spec) {
    sampleRate = spec.sampleRate;
    delayL.prepare(spec);
    delayR.prepare(spec);
}

void ModDelay::setParams(float dMs, float depth, float rate) {
    delayMs = dMs;
    modDepth = depth;
    modRateHz = rate;
}

void ModDelay::process(juce::dsp::AudioBlock<float>& block) {
    auto left = block.getChannelPointer(0);
    auto right = block.getChannelPointer(1);

    for (size_t i = 0; i < block.getNumSamples(); ++i) {
        float mod = std::sin(2.0f * juce::MathConstants<float>::pi * modRateHz * phase / sampleRate) * modDepth;
        phase += modRateHz / sampleRate;

        //if (phase >= sampleRate) phase -= sampleRate;
        if (phase >= 1.0f) phase -= 1.0f;

        float dL = delayMs + mod;
        float dR = delayMs - mod;

        delayL.pushSample(0, left[i]);
        delayR.pushSample(0, right[i]);

        left[i] = delayL.popSample(0, static_cast<int>(dL));  // dL is in samples now
        right[i] = delayR.popSample(0, static_cast<int>(dR));  // same here

    }
}