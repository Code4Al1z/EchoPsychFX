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

    constexpr double smoothingTime = 0.05; // 50 ms smoothing

    delayMs.reset(sampleRate, smoothingTime);
    modDepth.reset(sampleRate, smoothingTime);
    modRateHz.reset(sampleRate, smoothingTime);
    feedbackL.reset(sampleRate, smoothingTime);
    feedbackR.reset(sampleRate, smoothingTime);
    mix.reset(sampleRate, smoothingTime);
}

void ModDelay::setParams(float dMs, float depth, float rate, float fbL, float fbR, float m)
{
    delayMs.setTargetValue(dMs);
    modDepth.setTargetValue(depth);
    rawRate = rate;
    modRateHz.setTargetValue(getEffectiveRateHz()); // now smart
    feedbackL.setTargetValue(juce::jlimit(0.0f, 0.95f, fbL));
    feedbackR.setTargetValue(juce::jlimit(0.0f, 0.95f, fbR));
    mix.setTargetValue(juce::jlimit(0.0f, 1.0f, m));
}

void ModDelay::process(juce::dsp::AudioBlock<float>& block) {
    auto* left = block.getChannelPointer(0);
    auto* right = block.getChannelPointer(1);
    const auto numSamples = static_cast<int>(block.getNumSamples());

    float currentRateHz = modRateHz.getNextValue(); // Smooth once per block (optional smoothing inside loop below)

    for (int i = 0; i < numSamples; ++i) {
        // Uncomment this if you want to optimize CPU:
         if (i % 32 == 0)
             currentRateHz = modRateHz.getNextValue();

        float currentDelayMs = delayMs.getNextValue();
        float currentDepth = modDepth.getNextValue();
        float currentFeedbackL = feedbackL.getNextValue();
        float currentFeedbackR = feedbackR.getNextValue();
        float currentMix = mix.getNextValue();

        // Clamp minimum delay time for safety
        currentDelayMs = std::max(currentDelayMs, 5.0f);

        // Smart safety mechanism: limit depth so modulation doesn't push delay below 5ms or too far
        float safeDepth = currentDelayMs - 5.0f;                          // Avoid going below 5ms
        float maxAllowedDepth = currentDelayMs * 0.4f;                   // Max swing is ±40% of delay
        currentDepth = std::min(currentDepth, std::min(safeDepth, maxAllowedDepth));

        // Modulation value based on current phase and selected waveform
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

void ModDelay::setSyncEnabled(bool shouldSync)
{
    syncEnabled = shouldSync;
}

void ModDelay::setTempo(float newBpm)
{
    bpm = newBpm;
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

float ModDelay::getEffectiveRateHz() const
{
    if (syncEnabled && rawRate > 0.0f)
    {
        float beatsPerSecond = bpm / 60.0f;
        return beatsPerSecond / rawRate; // rate interpreted as note division (e.g. 4 = quarter note)
    }
    return rawRate; // regular Hz
}
