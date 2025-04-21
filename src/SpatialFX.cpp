#include "SpatialFX.h"

void SpatialFX::prepare(const juce::dsp::ProcessSpec& spec) {
    sampleRate = spec.sampleRate;
    lfoLeft.prepare(spec);
    lfoRight.prepare(spec);
    lfoLeft.setFrequency(modulationRate);
    lfoRight.setFrequency(modulationRate);
}

void SpatialFX::process(juce::dsp::AudioBlock<float>& block) {
    if (block.getNumChannels() < 2)
        return;

    auto left = block.getChannelPointer(0);
    auto right = block.getChannelPointer(1);
    size_t numSamples = block.getNumSamples();

    for (size_t i = 0; i < numSamples; ++i) {
        float modLeft = std::sin(2.0f * juce::MathConstants<float>::pi * phaseLeft) * modulationDepth;
        float modRight = std::sin(2.0f * juce::MathConstants<float>::pi * phaseRight) * modulationDepth;

        // Simple static phase offset (using a basic delay)
        size_t leftDelay = static_cast<size_t>(sampleRate * currentPhaseOffsetLeft);
        size_t rightDelay = static_cast<size_t>(sampleRate * currentPhaseOffsetRight);

        float delayedLeft = (i >= leftDelay) ? left[i - leftDelay] : 0.0f;
        float delayedRight = (i >= rightDelay) ? right[i - rightDelay] : 0.0f;

        left[i] = delayedLeft + modLeft; // Combine with modulation
        right[i] = delayedRight + modRight;

        phaseLeft += modulationRate / sampleRate;
        if (phaseLeft >= 1.0f)
            phaseLeft -= 1.0f;

        phaseRight += modulationRate / sampleRate;
        if (phaseRight >= 1.0f)
            phaseRight -= 1.0f;
    }
}

void SpatialFX::setPhaseOffset(float leftOffset, float rightOffset) {
    currentPhaseOffsetLeft = leftOffset;
    currentPhaseOffsetRight = rightOffset;
}

void SpatialFX::setModulationRate(float rateHz) {
    modulationRate = rateHz;
    if (sampleRate > 0) {
        lfoLeft.setFrequency(modulationRate);
        lfoRight.setFrequency(modulationRate);
    }
}

void SpatialFX::setModulationDepth(float depth) {
    modulationDepth = depth;
}
