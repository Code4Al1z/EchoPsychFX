#include "TiltEQ.h"

void TiltEQ::prepare(const juce::dsp::ProcessSpec& spec) {
    sampleRate = spec.sampleRate;

    lowShelf.prepare(spec);
    highShelf.prepare(spec);

    // Setup parameter smoothing (20ms default)
    tiltParam.reset(sampleRate, 0.02);
    tiltParam.setCurrentAndTargetValue(0.0f);

    reset();
    updateFilters();
}

void TiltEQ::reset() {
    lowShelf.reset();
    highShelf.reset();
    tiltParam.reset(sampleRate, tiltParam.getCurrentValue());
}

void TiltEQ::setTilt(float tiltAmount) {
    const float clampedTilt = juce::jlimit(-1.0f, 1.0f, tiltAmount);
    tiltParam.setTargetValue(clampedTilt);
    needsUpdate.store(true, std::memory_order_release);
}

void TiltEQ::setLowShelfFrequency(float freqHz) {
    juce::SpinLock::ScopedLockType sl(parameterLock);
    lowFreq = juce::jlimit(20.0f, 20000.0f, freqHz);
    needsUpdate.store(true, std::memory_order_release);
}

void TiltEQ::setHighShelfFrequency(float freqHz) {
    juce::SpinLock::ScopedLockType sl(parameterLock);
    highFreq = juce::jlimit(20.0f, 20000.0f, freqHz);
    needsUpdate.store(true, std::memory_order_release);
}

void TiltEQ::setGainRange(float rangeDb) {
    juce::SpinLock::ScopedLockType sl(parameterLock);
    gainRange = juce::jlimit(0.0f, 24.0f, rangeDb);
    needsUpdate.store(true, std::memory_order_release);
}

void TiltEQ::setQ(float qFactor) {
    juce::SpinLock::ScopedLockType sl(parameterLock);
    this->qFactor = juce::jlimit(0.1f, 10.0f, qFactor);
    needsUpdate.store(true, std::memory_order_release);
}

void TiltEQ::setSmoothingTime(float timeMs) {
    const float timeSec = timeMs * 0.001f;
    tiltParam.reset(sampleRate, timeSec);
}

void TiltEQ::setBypassed(bool shouldBeBypassed) {
    bypassed.store(shouldBeBypassed, std::memory_order_relaxed);
}

void TiltEQ::updateFilters() {
    juce::SpinLock::ScopedLockType sl(parameterLock);

    const float currentTilt = tiltParam.getNextValue();
    const float gain = currentTilt * gainRange;

    // Compute coefficients
    auto lowCoeffs = juce::dsp::IIR::Coefficients<float>::makeLowShelf(
        sampleRate,
        lowFreq,
        qFactor,
        juce::Decibels::decibelsToGain(gain)
    );

    auto highCoeffs = juce::dsp::IIR::Coefficients<float>::makeHighShelf(
        sampleRate,
        highFreq,
        qFactor,
        juce::Decibels::decibelsToGain(-gain)
    );

    // Update filter states (thread-safe assignment)
    *lowShelf.state = *lowCoeffs;
    *highShelf.state = *highCoeffs;
}

void TiltEQ::updateFiltersIfNeeded() {
    // Check if smoothed parameter is moving or if update is needed
    if (tiltParam.isSmoothing() || needsUpdate.load(std::memory_order_acquire)) {
        updateFilters();

        // Only clear the flag if we're not smoothing
        if (!tiltParam.isSmoothing()) {
            needsUpdate.store(false, std::memory_order_release);
        }
    }
}

void TiltEQ::process(juce::dsp::AudioBlock<float>& block) {
    if (bypassed.load(std::memory_order_relaxed))
        return;

    updateFiltersIfNeeded();

    juce::dsp::ProcessContextReplacing<float> context(block);
    lowShelf.process(context);
    highShelf.process(context);
}

void TiltEQ::process(const juce::dsp::ProcessContextNonReplacing<float>& context) {
    if (bypassed.load(std::memory_order_relaxed)) {
        // Copy input to output if bypassed
        context.getOutputBlock().copyFrom(context.getInputBlock());
        return;
    }

    updateFiltersIfNeeded();

    // Process through temporary block
    auto outputBlock = context.getOutputBlock();
    outputBlock.copyFrom(context.getInputBlock());

    juce::dsp::ProcessContextReplacing<float> replacingContext(outputBlock);
    lowShelf.process(replacingContext);
    highShelf.process(replacingContext);
}

float TiltEQ::getMagnitudeForFrequency(float frequency) const {
    juce::SpinLock::ScopedLockType sl(parameterLock);

    // Calculate magnitude response from both filters
    float lowMag = lowShelf.state->getMagnitudeForFrequency(frequency, sampleRate);
    float highMag = highShelf.state->getMagnitudeForFrequency(frequency, sampleRate);

    return lowMag * highMag;
}