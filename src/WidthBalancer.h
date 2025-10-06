#pragma once
#include <juce_dsp/juce_dsp.h>
#include <juce_audio_basics/juce_audio_basics.h>

/**
 * @brief Professional stereo width and mid-side balance processor
 *
 * Features smooth parameter changes, optimized processing, and stereo
 * correlation metering. Uses constant-power panning for natural results.
 */
class WidthBalancer {
public:
    WidthBalancer() = default;
    ~WidthBalancer() = default;

    //==============================================================================
    /** Prepares the processor for playback */
    void prepare(const juce::dsp::ProcessSpec& spec);

    /** Resets the internal state */
    void reset();

    //==============================================================================
    /**
     * Sets stereo width
     * 0.0 = mono, 1.0 = normal stereo, 2.0 = extra wide
     */
    void setWidth(float width);

    /**
     * Sets mid-side balance using constant power panning
     * -1.0 = only side, 0.0 = balanced, +1.0 = only mid
     */
    void setMidSideBalance(float balance);

    /**
     * Sets overall effect intensity
     * 0.0 = bypass, 1.0 = full effect
     */
    void setIntensity(float intensity);

    /**
     * Forces mono output (collapses to center)
     */
    void setMono(bool shouldBeMono);

    /**
     * Enable/disable processing
     */
    void setBypassed(bool shouldBeBypassed);

    /**
     * Sets parameter smoothing time in milliseconds
     */
    void setSmoothingTime(float timeMs);

    //==============================================================================
    /** Processes an audio block */
    void process(juce::dsp::AudioBlock<float>& block);

    //==============================================================================
    /** Returns current width setting */
    float getWidth() const noexcept { return targetWidth.load(std::memory_order_relaxed); }

    /** Returns current mid-side balance */
    float getMidSideBalance() const noexcept { return targetBalance.load(std::memory_order_relaxed); }

    /** Returns current intensity */
    float getIntensity() const noexcept { return targetIntensity.load(std::memory_order_relaxed); }

    /** Returns mono state */
    bool isMono() const noexcept { return mono.load(std::memory_order_relaxed); }

    /** Returns bypass state */
    bool isBypassed() const noexcept { return bypassed.load(std::memory_order_relaxed); }

    /**
     * Returns stereo correlation coefficient for the last processed block
     * -1.0 = perfectly out of phase, 0.0 = mono, +1.0 = perfectly correlated
     */
    float getStereoCorrelation() const noexcept { return currentCorrelation.load(std::memory_order_relaxed); }

private:
    //==============================================================================
    // Smoothed parameters
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> widthSmoothed;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> balanceSmoothed;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> intensitySmoothed;

    // Target values (lock-free)
    std::atomic<float> targetWidth{ 1.0f };
    std::atomic<float> targetBalance{ 0.0f };
    std::atomic<float> targetIntensity{ 1.0f };
    std::atomic<bool> mono{ false };
    std::atomic<bool> bypassed{ false };

    // Correlation metering
    std::atomic<float> currentCorrelation{ 0.0f };
    float sumLL = 0.0f;
    float sumRR = 0.0f;
    float sumLR = 0.0f;
    int correlationSampleCount = 0;
    static constexpr int correlationWindowSize = 2048;

    double sampleRate = 44100.0;
    float smoothingTimeMs = 20.0f;

    // Pre-computed balance gains (avoid trig in audio thread)
    float cachedMidGain = 1.0f;
    float cachedSideGain = 0.0f;
    float lastBalanceForCache = 0.0f;

    //==============================================================================
    void updateBalanceGains(float balance);
    void updateCorrelation(const float* left, const float* right, size_t numSamples);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(WidthBalancer)
};