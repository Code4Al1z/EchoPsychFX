#include "TiltEQ.h"
#include "FilterUtils.h"

void TiltEQ::prepare(const juce::dsp::ProcessSpec& spec) {
    sampleRate = spec.sampleRate;

    lowShelf.prepare(spec);
    highShelf.prepare(spec);

    // Setup parameter smoothing (20ms default)
    tiltParam.reset(sampleRate, 0.02);
    tiltParam.setCurrentAndTargetValue(0.0f);

    reset();
    updateFilters(0);
}

void TiltEQ::reset() {
    lowShelf.reset();
    highShelf.reset();
    tiltParam.reset(sampleRate, tiltParam.getCurrentValue());
}

void TiltEQ::setTilt(float tiltAmount) {
    const float clampedTilt = juce::jlimit(-1.0f, 1.0f, tiltAmount);

    // The processor sends the current value every audio block; only react to a real change, so a
    // still knob costs nothing (rebuilding the filters every block used to allocate on the audio thread)
    if (clampedTilt == tiltParam.getTargetValue())
        return;

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

void TiltEQ::updateFilters(int numSamples) {
    juce::SpinLock::ScopedLockType sl(parameterLock);

    const float currentTilt = tiltParam.skip(numSamples);
    const float gain = currentTilt * gainRange;

    // Positive tilt is BRIGHT (as the slider says): the lows come down and the highs come up. (It used to be the
    // other way round, so the slider moved the sound the opposite way to its "Dark < Tilt > Bright" label.)
    // The coefficients go straight into the filters' existing state (no allocation)
    using Array = juce::dsp::IIR::ArrayCoefficients<float>;
    setCoefficientsInPlace(*lowShelf.state, Array::makeLowShelf(sampleRate, lowFreq, qFactor, juce::Decibels::decibelsToGain(-gain)));
    setCoefficientsInPlace(*highShelf.state, Array::makeHighShelf(sampleRate, highFreq, qFactor, juce::Decibels::decibelsToGain(gain)));
}

void TiltEQ::updateFiltersIfNeeded(int numSamples) {
    // Check if smoothed parameter is moving or if update is needed
    if (tiltParam.isSmoothing() || needsUpdate.load(std::memory_order_acquire)) {
        updateFilters(numSamples);

        // Only clear the flag if we're not smoothing
        if (!tiltParam.isSmoothing()) {
            needsUpdate.store(false, std::memory_order_release);
        }
    }
}

void TiltEQ::process(juce::dsp::AudioBlock<float>& block) {
    if (bypassed.load(std::memory_order_relaxed))
        return;

    updateFiltersIfNeeded(static_cast<int>(block.getNumSamples()));

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

    updateFiltersIfNeeded(static_cast<int>(context.getInputBlock().getNumSamples()));

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