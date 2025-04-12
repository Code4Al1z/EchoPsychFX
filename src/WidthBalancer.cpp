#include "WidthBalancer.h"

void WidthBalancer::prepare(const juce::dsp::ProcessSpec&) {}

void WidthBalancer::setWidth(float newWidth) { width = juce::jlimit(0.0f, 2.0f, newWidth); }
void WidthBalancer::setMidGain(float gainDb) { midGain = juce::Decibels::decibelsToGain(gainDb); }
void WidthBalancer::setSideGain(float gainDb) { sideGain = juce::Decibels::decibelsToGain(gainDb); }

void WidthBalancer::process(juce::dsp::AudioBlock<float>& block) {
    auto numChannels = block.getNumChannels();
    if (numChannels < 2) return; // skip mono signals

    auto left = block.getChannelPointer(0);
    auto right = block.getChannelPointer(1);

    for (size_t i = 0; i < block.getNumSamples(); ++i) {
        float mid = (left[i] + right[i]) * 0.5f;
        float side = (left[i] - right[i]) * 0.5f;

        mid *= midGain;
        side *= sideGain * width;

        left[i] = mid + side;
        right[i] = mid - side;
    }
}