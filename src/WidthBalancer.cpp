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

    for (size_t i = 0; i < block.getNumSamples(); ++i) {
        float l = left[i];
        float r = right[i];

        float mid = (l + r) * 0.5f;
        float side = (l - r) * 0.5f;

        // Apply width
        side *= width;

        // Apply mid/side balance
        float midScale = 1.0f - std::abs(midSideBalance);
        float sideScale = 1.0f - midScale;

        if (midSideBalance > 0.0f) {
            // Favor mid
            mid *= 1.0f;
            side *= 1.0f - midSideBalance;
        }
        else {
            // Favor side
            mid *= 1.0f + midSideBalance;
            side *= 1.0f;
        }

        if (mono) {
            float monoSignal = mid;
            left[i] = monoSignal;
            right[i] = monoSignal;
        }
        else {
            left[i] = mid + side;
            right[i] = mid - side;
        }
    }
}
