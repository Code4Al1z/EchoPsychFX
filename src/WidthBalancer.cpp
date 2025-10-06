#include "WidthBalancer.h"

void WidthBalancer::prepare(const juce::dsp::ProcessSpec& spec)
{
    sampleRate = spec.sampleRate;

    // Setup parameter smoothing
    const float smoothingTimeSec = smoothingTimeMs * 0.001f;
    widthSmoothed.reset(sampleRate, smoothingTimeSec);
    balanceSmoothed.reset(sampleRate, smoothingTimeSec);
    intensitySmoothed.reset(sampleRate, smoothingTimeSec);

    reset();
}

void WidthBalancer::reset()
{
    widthSmoothed.setCurrentAndTargetValue(targetWidth.load(std::memory_order_relaxed));
    balanceSmoothed.setCurrentAndTargetValue(targetBalance.load(std::memory_order_relaxed));
    intensitySmoothed.setCurrentAndTargetValue(targetIntensity.load(std::memory_order_relaxed));

    // Reset correlation metering
    sumLL = 0.0f;
    sumRR = 0.0f;
    sumLR = 0.0f;
    correlationSampleCount = 0;
    currentCorrelation.store(0.0f, std::memory_order_relaxed);

    // Reset cached gains
    updateBalanceGains(targetBalance.load(std::memory_order_relaxed));
}

void WidthBalancer::setWidth(float width)
{
    const float clampedWidth = juce::jlimit(0.0f, 2.0f, width);
    targetWidth.store(clampedWidth, std::memory_order_relaxed);
    widthSmoothed.setTargetValue(clampedWidth);
}

void WidthBalancer::setMidSideBalance(float balance)
{
    const float clampedBalance = juce::jlimit(-1.0f, 1.0f, balance);
    targetBalance.store(clampedBalance, std::memory_order_relaxed);
    balanceSmoothed.setTargetValue(clampedBalance);
}

void WidthBalancer::setIntensity(float intensity)
{
    const float clampedIntensity = juce::jlimit(0.0f, 1.0f, intensity);
    targetIntensity.store(clampedIntensity, std::memory_order_relaxed);
    intensitySmoothed.setTargetValue(clampedIntensity);
}

void WidthBalancer::setMono(bool shouldBeMono)
{
    mono.store(shouldBeMono, std::memory_order_relaxed);
}

void WidthBalancer::setBypassed(bool shouldBeBypassed)
{
    bypassed.store(shouldBeBypassed, std::memory_order_relaxed);
}

void WidthBalancer::setSmoothingTime(float timeMs)
{
    smoothingTimeMs = juce::jlimit(1.0f, 1000.0f, timeMs);
    const float timeSec = smoothingTimeMs * 0.001f;

    widthSmoothed.reset(sampleRate, timeSec);
    balanceSmoothed.reset(sampleRate, timeSec);
    intensitySmoothed.reset(sampleRate, timeSec);
}

void WidthBalancer::updateBalanceGains(float balance)
{
    // Constant power panning for natural mid-side balance
    // Map -1..1 to 0..pi/2 for one quadrant of sine/cosine
    const float balanceAngle = (balance * 0.5f + 0.5f) * juce::MathConstants<float>::halfPi;

    cachedMidGain = std::cos(balanceAngle);
    cachedSideGain = std::sin(balanceAngle);

    // Handle negative balance (more side)
    if (balance < 0.0f)
    {
        cachedSideGain = 1.0f;
        cachedMidGain = std::cos(std::abs(balance) * juce::MathConstants<float>::halfPi);
    }

    lastBalanceForCache = balance;
}

void WidthBalancer::updateCorrelation(const float* left, const float* right, size_t numSamples)
{
    // Accumulate correlation metrics
    for (size_t i = 0; i < numSamples; ++i)
    {
        const float l = left[i];
        const float r = right[i];

        sumLL += l * l;
        sumRR += r * r;
        sumLR += l * r;

        ++correlationSampleCount;

        // Calculate and reset when window is full
        if (correlationSampleCount >= correlationWindowSize)
        {
            const float denominator = std::sqrt(sumLL * sumRR);

            if (denominator > 1e-10f) // Avoid division by zero
            {
                const float correlation = sumLR / denominator;
                currentCorrelation.store(juce::jlimit(-1.0f, 1.0f, correlation),
                    std::memory_order_relaxed);
            }
            else
            {
                currentCorrelation.store(0.0f, std::memory_order_relaxed);
            }

            // Reset accumulators
            sumLL = 0.0f;
            sumRR = 0.0f;
            sumLR = 0.0f;
            correlationSampleCount = 0;
        }
    }
}

void WidthBalancer::process(juce::dsp::AudioBlock<float>& block)
{
    // Early exit for mono or bypassed
    if (block.getNumChannels() < 2 || bypassed.load(std::memory_order_relaxed))
        return;

    float* left = block.getChannelPointer(0);
    float* right = block.getChannelPointer(1);
    const size_t numSamples = block.getNumSamples();

    // Check if we're in mono mode
    const bool monoMode = mono.load(std::memory_order_relaxed);

    if (monoMode)
    {
        // Fast mono collapse
        for (size_t i = 0; i < numSamples; ++i)
        {
            const float mono = (left[i] + right[i]) * 0.5f;
            left[i] = mono;
            right[i] = mono;
        }

        currentCorrelation.store(1.0f, std::memory_order_relaxed);
        return;
    }

    // Check if any parameters are smoothing
    const bool isSmoothing = widthSmoothed.isSmoothing() ||
        balanceSmoothed.isSmoothing() ||
        intensitySmoothed.isSmoothing();

    if (isSmoothing)
    {
        // Per-sample processing with smoothing
        for (size_t i = 0; i < numSamples; ++i)
        {
            const float currentWidth = widthSmoothed.getNextValue();
            const float currentBalance = balanceSmoothed.getNextValue();
            const float currentIntensity = intensitySmoothed.getNextValue();

            // Update balance gains if changed significantly
            if (std::abs(currentBalance - lastBalanceForCache) > 0.001f)
                updateBalanceGains(currentBalance);

            // Apply intensity scaling
            const float effectiveWidth = 1.0f + (currentWidth - 1.0f) * currentIntensity;
            const float effectiveMidGain = 1.0f + (cachedMidGain - 1.0f) * currentIntensity;
            const float effectiveSideGain = cachedSideGain * currentIntensity;

            // Mid-side encoding
            const float l = left[i];
            const float r = right[i];
            float mid = (l + r) * 0.5f;
            float side = (l - r) * 0.5f;

            // Apply width
            side *= effectiveWidth;

            // Apply mid-side balance
            mid *= effectiveMidGain;
            side *= effectiveSideGain;

            // Mid-side decoding with proper gain compensation
            left[i] = mid + side;
            right[i] = mid - side;
        }
    }
    else
    {
        // Optimized path when parameters are stable
        const float currentWidth = widthSmoothed.getCurrentValue();
        const float currentBalance = balanceSmoothed.getCurrentValue();
        const float currentIntensity = intensitySmoothed.getCurrentValue();

        // Update balance gains if needed
        if (std::abs(currentBalance - lastBalanceForCache) > 0.001f)
            updateBalanceGains(currentBalance);

        // Pre-compute scaled parameters
        const float effectiveWidth = 1.0f + (currentWidth - 1.0f) * currentIntensity;
        const float effectiveMidGain = 1.0f + (cachedMidGain - 1.0f) * currentIntensity;
        const float effectiveSideGain = cachedSideGain * currentIntensity;

        // Process block
        for (size_t i = 0; i < numSamples; ++i)
        {
            const float l = left[i];
            const float r = right[i];

            // Mid-side encoding
            float mid = (l + r) * 0.5f;
            float side = (l - r) * 0.5f;

            // Apply width
            side *= effectiveWidth;

            // Apply mid-side balance
            mid *= effectiveMidGain;
            side *= effectiveSideGain;

            // Mid-side decoding
            left[i] = mid + side;
            right[i] = mid - side;
        }
    }

    // Update stereo correlation meter
    updateCorrelation(left, right, numSamples);
}