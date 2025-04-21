#include "WidthBalancer.h"

void WidthBalancer::prepare(const juce::dsp::ProcessSpec&) {}

void WidthBalancer::setWidth(float newWidth) {
    width = juce::jlimit(0.0f, 2.0f, newWidth);
}

void WidthBalancer::setMidSideBalance(float balance) {
    midSideBalance = juce::jlimit(-1.0f, 1.0f, balance);
}

void WidthBalancer::setMono(bool shouldMono) {
    mono = shouldMono;
}

void WidthBalancer::process(juce::dsp::AudioBlock<float>& block) {
    if (block.getNumChannels() < 2)
        return;

    auto left = block.getChannelPointer(0);
    auto right = block.getChannelPointer(1);
    size_t numSamples = block.getNumSamples();

    float currentWidth = width; // Could be a smoothed value
    float currentMidSideBalance = midSideBalance; // Could be a smoothed value
    float currentIntensity = intensity; // 0.0 to 1.0, non-linearly scaled

    // Apply intensity scaling (example - adjust curve as needed)
    float scaledWidth = 1.0f + (currentWidth - 1.0f) * currentIntensity * 0.5f; // Subtle width change
    float scaledMidSideBalance = currentMidSideBalance * currentIntensity * 0.75f; // More subtle balance

    for (size_t i = 0; i < numSamples; ++i) {
        float l = left[i];
        float r = right[i];

        float mid = (l + r) * 0.5f;
        float side = (l - r) * 0.5f;

        // Apply width (subtly)
        side *= scaledWidth;

        // Apply mid/side balance (subtly) - Using constant power for natural feel
        float balanceAngle = scaledMidSideBalance * juce::MathConstants<float>::pi / 2.0f;
        float midGain = std::cos(balanceAngle);
        float sideGain = std::sin(balanceAngle);

        mid *= midGain;
        side *= sideGain;

        if (mono) {
            left[i] = mid;
            right[i] = mid;
        }
        else {
            left[i] = mid + side;
            right[i] = mid - side;
        }
    }
}

void WidthBalancer::setIntensity(float newIntensity) {
    intensity = juce::jlimit(0.0f, 1.0f, newIntensity);
}
