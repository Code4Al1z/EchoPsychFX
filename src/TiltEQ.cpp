#include "TiltEQ.h"

void TiltEQ::prepare(const juce::dsp::ProcessSpec& spec) {
    sampleRate = spec.sampleRate;
    lowShelf.prepare(spec);
    highShelf.prepare(spec);
    updateFilters();
}

void TiltEQ::setTilt(float tiltAmount) {
    tilt = juce::jlimit(-1.0f, 1.0f, tiltAmount);
    needsUpdate = true;
}

void TiltEQ::updateFilters() {
    float gain = tilt * 6.0f; // ±6 dB tilt
    auto low = juce::dsp::IIR::Coefficients<float>::makeLowShelf(sampleRate, 200.0f, 0.707f, juce::Decibels::decibelsToGain(gain));
    auto high = juce::dsp::IIR::Coefficients<float>::makeHighShelf(sampleRate, 4000.0f, 0.707f, juce::Decibels::decibelsToGain(-gain));

    *lowShelf.coefficients = *low;
    *highShelf.coefficients = *high;

    needsUpdate = false;
}

void TiltEQ::process(juce::dsp::AudioBlock<float>& block) {
    if (needsUpdate)
        updateFilters();

    juce::dsp::ProcessContextReplacing<float> context(block);
    lowShelf.process(context);
    highShelf.process(context);
}
