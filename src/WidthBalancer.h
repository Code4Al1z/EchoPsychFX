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

    void prepare(const juce::dsp::ProcessSpec& spec);
    void reset();

    void setWidth(float width);
    void setMidSideBalance(float balance);
    void setIntensity(float intensity);
    void setMono(bool shouldBeMono);
    void setBypassed(bool shouldBeBypassed);
    void setSmoothingTime(float timeMs);

    void process(juce::dsp::AudioBlock<float>& block);

    float getWidth() const noexcept { return targetWidth.load(std::memory_order_relaxed); }
    float getMidSideBalance() const noexcept { return targetBalance.load(std::memory_order_relaxed); }
    float getIntensity() const noexcept { return targetIntensity.load(std::memory_order_relaxed); }
    bool isMono() const noexcept { return mono.load(std::memory_order_relaxed); }
    bool isBypassed() const noexcept { return bypassed.load(std::memory_order_relaxed); }
    float getStereoCorrelation() const noexcept { return currentCorrelation.load(std::memory_order_relaxed); }

private:
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> widthSmoothed;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> balanceSmoothed;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> intensitySmoothed;

    std::atomic<float> targetWidth{ 1.0f };
    std::atomic<float> targetBalance{ 0.0f };
    std::atomic<float> targetIntensity{ 1.0f };
    std::atomic<bool> mono{ false };
    std::atomic<bool> bypassed{ false };
    std::atomic<float> currentCorrelation{ 0.0f };

    float sumLL = 0.0f;
    float sumRR = 0.0f;
    float sumLR = 0.0f;
    int correlationSampleCount = 0;
    static constexpr int correlationWindowSize = 2048;

    double sampleRate = 44100.0;
    float smoothingTimeMs = 20.0f;

    float cachedMidGain = 1.0f;
    float cachedSideGain = 0.0f;
    float lastBalanceForCache = 0.0f;

    // Cache for optimization
    struct ProcessCache {
        bool paramsStable = false;
        float width = 1.0f;
        float balance = 0.0f;
        float intensity = 1.0f;
        float effectiveWidth = 1.0f;
        float effectiveMidGain = 1.0f;
        float effectiveSideGain = 0.0f;
    } cache;

    void updateBalanceGains(float balance);
    void updateProcessCache();
    void updateCorrelation(const float* left, const float* right, size_t numSamples);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(WidthBalancer)
};