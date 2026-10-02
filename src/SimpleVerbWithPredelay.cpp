#include "SimpleVerbWithPredelay.h"

SimpleVerbWithPredelay::SimpleVerbWithPredelay()
{
    // Initialize reverb with sensible defaults
    reverbParams.roomSize = 0.5f;
    reverbParams.damping = 0.3f;
    reverbParams.wetLevel = 1.0f;
    reverbParams.dryLevel = 0.0f;
    reverbParams.width = 1.0f;
    reverbParams.freezeMode = 0.0f;
}

void SimpleVerbWithPredelay::prepare(const juce::dsp::ProcessSpec& spec)
{
    sampleRate = spec.sampleRate;

    // Allocate pre-delay buffer (max 500ms)
    // Must be power of 2 for the bitwise wrap (& mask) to work correctly
    int rawMax = static_cast<int>(sampleRate * 0.5);
    maxPredelaySamples = 1;
    while (maxPredelaySamples < rawMax)
        maxPredelaySamples <<= 1;
    predelayBuffer.setSize(static_cast<int>(spec.numChannels), maxPredelaySamples + 4);

    // Allocate working buffer for reverb processing
    workingBuffer.setSize(static_cast<int>(spec.numChannels),
        static_cast<int>(spec.maximumBlockSize));

    // Setup parameter smoothing
    const float smoothingTimeSec = smoothingTimeMs * 0.001f;
    predelaySmoothed.reset(sampleRate, smoothingTimeSec);
    wetLevelSmoothed.reset(sampleRate, smoothingTimeSec);

    predelaySmoothed.setCurrentAndTargetValue(0.0f);
    wetLevelSmoothed.setCurrentAndTargetValue(targetWetLevel.load(std::memory_order_relaxed));

    // Prepare reverb
    reverb.setParameters(reverbParams);
    reverb.prepare(spec);

    reset();
}

void SimpleVerbWithPredelay::reset()
{
    predelayBuffer.clear();
    workingBuffer.clear();
    reverb.reset();

    predelayWritePos = 0;

    const float currentPredelay = targetPredelayMs.load(std::memory_order_relaxed);
    const float currentWet = targetWetLevel.load(std::memory_order_relaxed);

    predelaySmoothed.setCurrentAndTargetValue(currentPredelay * static_cast<float>(sampleRate) / 1000.0f);
    wetLevelSmoothed.setCurrentAndTargetValue(currentWet);
}

void SimpleVerbWithPredelay::setPredelayTime(float predelayMs)
{
    const float clampedPredelay = juce::jlimit(0.0f, 500.0f, predelayMs);
    targetPredelayMs.store(clampedPredelay, std::memory_order_relaxed);

    const float predelaySamples = clampedPredelay * static_cast<float>(sampleRate) / 1000.0f;
    predelaySmoothed.setTargetValue(predelaySamples);
}

void SimpleVerbWithPredelay::setRoomSize(float size)
{
    juce::SpinLock::ScopedLockType sl(parameterLock);
    reverbParams.roomSize = juce::jlimit(0.0f, 1.0f, size);
    targetRoomSize.store(reverbParams.roomSize, std::memory_order_relaxed);
    needsReverbUpdate.store(true, std::memory_order_release);
}

void SimpleVerbWithPredelay::setDamping(float damping)
{
    juce::SpinLock::ScopedLockType sl(parameterLock);
    reverbParams.damping = juce::jlimit(0.0f, 1.0f, damping);
    needsReverbUpdate.store(true, std::memory_order_release);
}

void SimpleVerbWithPredelay::setWetLevel(float wet)
{
    const float clampedWet = juce::jlimit(0.0f, 1.0f, wet);
    targetWetLevel.store(clampedWet, std::memory_order_relaxed);
    wetLevelSmoothed.setTargetValue(clampedWet);
}

void SimpleVerbWithPredelay::setWidth(float width)
{
    juce::SpinLock::ScopedLockType sl(parameterLock);
    reverbParams.width = juce::jlimit(0.0f, 1.0f, width);
    needsReverbUpdate.store(true, std::memory_order_release);
}

void SimpleVerbWithPredelay::setFreezeMode(float freeze)
{
    juce::SpinLock::ScopedLockType sl(parameterLock);
    reverbParams.freezeMode = juce::jlimit(0.0f, 1.0f, freeze);
    needsReverbUpdate.store(true, std::memory_order_release);
}

void SimpleVerbWithPredelay::setParams(float predelayMs, float size, float damping, float wet)
{
    setPredelayTime(predelayMs);
    setRoomSize(size);
    setDamping(damping);
    setWetLevel(wet);
}

void SimpleVerbWithPredelay::setSmoothingTime(float timeMs)
{
    smoothingTimeMs = juce::jlimit(1.0f, 1000.0f, timeMs);
    const float timeSec = smoothingTimeMs * 0.001f;

    predelaySmoothed.reset(sampleRate, timeSec);
    wetLevelSmoothed.reset(sampleRate, timeSec);
}

void SimpleVerbWithPredelay::setBypassed(bool shouldBeBypassed)
{
    bypassed.store(shouldBeBypassed, std::memory_order_relaxed);
}

void SimpleVerbWithPredelay::updateReverbParameters()
{
    if (needsReverbUpdate.load(std::memory_order_acquire))
    {
        juce::SpinLock::ScopedLockType sl(parameterLock);
        reverb.setParameters(reverbParams);
        needsReverbUpdate.store(false, std::memory_order_release);
    }
}

void SimpleVerbWithPredelay::applyPredelay(juce::dsp::AudioBlock<float>& inputBlock,
    juce::dsp::AudioBlock<float>& outputBlock)
{
    const int numChannels = static_cast<int>(inputBlock.getNumChannels());
    const int numSamples = static_cast<int>(inputBlock.getNumSamples());

    for (int i = 0; i < numSamples; ++i)
    {
        // Advance smoother once per sample. The delay is kept to at least one sample: the 4-point
        // interpolation below needs the sample one step "newer" than the read position, which for a
        // shorter delay would not have been written yet.
        const float currentDelaySamples = juce::jmax(1.0f, predelaySmoothed.getNextValue());
        const int delaySamplesInt = static_cast<int>(currentDelaySamples);
        const float delayFraction = currentDelaySamples - static_cast<float>(delaySamplesInt);

        const int currentWritePos = (predelayWritePos + i) & (maxPredelaySamples - 1);

        for (int ch = 0; ch < numChannels; ++ch)
        {
            // Write input sample to circular buffer
            predelayBuffer.getWritePointer(ch)[currentWritePos] = inputBlock.getSample(ch, i);

            // The wanted point lies between the samples N and N + 1 steps back, a fraction
            // (1 - delayFraction) of the way from the older one to the newer one. (It used to
            // interpolate towards newer samples by delayFraction, which delayed the signal by N - f
            // instead of N + f and could read samples that had not been written yet.)
            const int newer = currentWritePos - delaySamplesInt;

            const int idx0 = (newer - 2 + maxPredelaySamples) & (maxPredelaySamples - 1);
            const int idx1 = (newer - 1 + maxPredelaySamples) & (maxPredelaySamples - 1);
            const int idx2 = (newer + maxPredelaySamples) & (maxPredelaySamples - 1);
            const int idx3 = (newer + 1 + maxPredelaySamples) & (maxPredelaySamples - 1);

            const float* buf = predelayBuffer.getReadPointer(ch);
            outputBlock.setSample(ch, i,
                hermiteInterpolation(1.0f - delayFraction, buf[idx0], buf[idx1], buf[idx2], buf[idx3]));
        }
    }

    predelayWritePos = (predelayWritePos + numSamples) & (maxPredelaySamples - 1);
}

inline float SimpleVerbWithPredelay::hermiteInterpolation(float x, float y0, float y1, float y2, float y3) const noexcept
{
    // 4-point Hermite interpolation (smoother than linear)
    const float c0 = y1;
    const float c1 = 0.5f * (y2 - y0);
    const float c2 = y0 - 2.5f * y1 + 2.0f * y2 - 0.5f * y3;
    const float c3 = 0.5f * (y3 - y0) + 1.5f * (y1 - y2);

    return ((c3 * x + c2) * x + c1) * x + c0;
}

void SimpleVerbWithPredelay::process(juce::dsp::AudioBlock<float>& block)
{
    if (bypassed.load(std::memory_order_relaxed))
        return;

    updateReverbParameters();

    const int numChannels = static_cast<int>(block.getNumChannels());
    const int numSamples = static_cast<int>(block.getNumSamples());

    // Ensure working buffer is large enough
    if (workingBuffer.getNumSamples() < numSamples)
        workingBuffer.setSize(numChannels, numSamples, false, false, true);

    // Apply pre-delay
    juce::dsp::AudioBlock<float> delayedBlock(workingBuffer.getArrayOfWritePointers(),
        numChannels, 0, numSamples);
    applyPredelay(block, delayedBlock);

    // Process through reverb
    juce::dsp::ProcessContextReplacing<float> reverbContext(delayedBlock);
    reverb.process(reverbContext);

    // Mix wet/dry with smoothed wet level
    const bool isSmoothing = wetLevelSmoothed.isSmoothing();

    if (isSmoothing)
    {
        // Per-sample smoothing when wet level is changing
        for (int i = 0; i < numSamples; ++i)
        {
            const float wetGain = wetLevelSmoothed.getNextValue();
            const float dryGain = 1.0f - wetGain;

            for (int ch = 0; ch < numChannels; ++ch)
            {
                const float dry = block.getSample(ch, i);
                const float wet = delayedBlock.getSample(ch, i);
                block.setSample(ch, i, dry * dryGain + wet * wetGain);
            }
        }
    }
    else
    {
        // Optimized path when wet level is stable
        const float wetGain = wetLevelSmoothed.getCurrentValue();
        const float dryGain = 1.0f - wetGain;

        for (int ch = 0; ch < numChannels; ++ch)
        {
            float* dryBuffer = block.getChannelPointer(ch);
            const float* wetBuffer = delayedBlock.getChannelPointer(ch);

            for (int i = 0; i < numSamples; ++i)
            {
                dryBuffer[i] = dryBuffer[i] * dryGain + wetBuffer[i] * wetGain;
            }
        }
    }

}


float SimpleVerbWithPredelay::getPredelayTime() const noexcept
{
    return targetPredelayMs.load(std::memory_order_relaxed);
}

float SimpleVerbWithPredelay::getWetLevel() const noexcept
{
    return targetWetLevel.load(std::memory_order_relaxed);
}

int SimpleVerbWithPredelay::getTailLengthSamples() const noexcept
{
    return computeTailLengthSamples(targetPredelayMs.load(std::memory_order_relaxed),
        targetRoomSize.load(std::memory_order_relaxed),
        targetWetLevel.load(std::memory_order_relaxed), sampleRate);
}

int SimpleVerbWithPredelay::computeTailLengthSamples(float predelayMs, float roomSize, float wetLevel,
    double sampleRate) noexcept
{
    if (wetLevel <= 0.0f)
        return 0;

    // juce::Reverb is a Freeverb: parallel comb filters whose feedback is roomSize * 0.28 + 0.7 and
    // whose delays average about 31 ms. Each trip round a comb loses -20*log10(feedback) dB, so
    // decaying by 60 dB takes 60 / that many trips. (Measured against the real reverb this lands
    // within a few percent for small rooms and ~15% long at size 1.0, where damping shortens the
    // real tail - a safe side to err on for a host deciding when to stop processing.)
    constexpr double averageCombSeconds = 0.0312;
    const double feedback = juce::jlimit(0.0f, 1.0f, roomSize) * 0.28 + 0.7;
    const double decaySeconds = averageCombSeconds * 60.0 / (-20.0 * std::log10(feedback));

    return static_cast<int>(std::ceil((predelayMs * 0.001 + decaySeconds) * sampleRate));
}
