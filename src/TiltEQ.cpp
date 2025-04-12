#include "TiltEQ.h"

void TiltEQ::prepare(const juce::dsp::ProcessSpec& spec) {
    lowShelf.reset();
    highShelf.reset();
    lowShelf.prepare(spec);
    highShelf.prepare(spec);
    updateFilters();
}

void TiltEQ::setTilt(float tiltAmount) {
    tilt = juce::jlimit(-1.0f, 1.0f, tiltAmount);
    needsUpdate = true;
}

void TiltEQ::updateFilters() {
    float gain = tilt * 6.0f; // tilt range ±6dB
    auto lowCoeffs = juce::dsp::IIR::Coefficients<float>::makeLowShelf(44100.0, 200.0, 0.707f, juce::Decibels::decibelsToGain(gain));
    auto highCoeffs = juce::dsp::IIR::Coefficients<float>::makeHighShelf(44100.0, 4000.0, 0.707f, juce::Decibels::decibelsToGain(-gain));
    *lowShelf.coefficients = *lowCoeffs;
    *highShelf.coefficients = *highCoeffs;
    needsUpdate = false;
}

void TiltEQ::process(juce::dsp::AudioBlock<float>& block) {
    if (needsUpdate) updateFilters();
    lowShelf.process(juce::dsp::ProcessContextReplacing<float>(block));
    highShelf.process(juce::dsp::ProcessContextReplacing<float>(block));
}