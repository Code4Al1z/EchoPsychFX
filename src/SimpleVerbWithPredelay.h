#pragma once
#include <juce_dsp/juce_dsp.h>
#include <juce_audio_basics/juce_audio_basics.h>

/**
 * @brief High-quality reverb with pre-delay and smooth parameter control
 *
 * Features interpolated pre-delay, comprehensive reverb controls,
 * and optimized real-time processing.
 */
class SimpleVerbWithPredelay
{
public:
    SimpleVerbWithPredelay();
    ~SimpleVerbWithPredelay() = default;

    //==============================================================================
    /** Prepares the processor for playback */
    void prepare(const juce::dsp::ProcessSpec& spec);

    /** Resets the internal state */
    void reset();

    //==============================================================================
    /** Sets pre-delay time in milliseconds (0-500ms) */
    void setPredelayTime(float predelayMs);

    /** Sets room size (0.0 - 1.0) */
    void setRoomSize(float size);

    /** Sets damping amount (0.0 - 1.0) */
    void setDamping(float damping);

    /** Sets wet/dry mix (0.0 = dry, 1.0 = wet) */
    void setWetLevel(float wet);

    /** Sets stereo width (0.0 = mono, 1.0 = full stereo) */
    void setWidth(float width);

    /** Sets freeze mode (0.0 = off, 1.0 = on) */
    void setFreezeMode(float freeze);

    /** Sets all parameters at once */
    void setParams(float predelayMs, float size, float damping, float wet);

    /** Sets parameter smoothing time in milliseconds */
    void setSmoothingTime(float timeMs);

    /** Enable/disable reverb */
    void setBypassed(bool shouldBeBypassed);

    //==============================================================================
    /** Processes an audio block */
    void process(juce::dsp::AudioBlock<float>& block);

    //==============================================================================
    /** Returns current pre-delay time in milliseconds */
    float getPredelayTime() const noexcept;

    /** Returns current wet level */
    float getWetLevel() const noexcept;

    /** Returns whether bypassed */
    bool isBypassed() const noexcept { return bypassed.load(std::memory_order_relaxed); }

    /** Returns the reverb tail length in samples */
    int getTailLengthSamples() const noexcept;

private:
    //==============================================================================
    juce::dsp::Reverb reverb;
    juce::dsp::Reverb::Parameters reverbParams;

    // Pre-delay buffer and working buffers
    juce::AudioBuffer<float> predelayBuffer;
    juce::AudioBuffer<float> workingBuffer;

    int predelayWritePos = 0;
    int maxPredelaySamples = 0;
    double sampleRate = 44100.0;

    // Smoothed parameters
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> predelaySmoothed;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> wetLevelSmoothed;

    // Thread-safe parameter storage
    std::atomic<float> targetPredelayMs{ 0.0f };
    std::atomic<float> targetWetLevel{ 0.3f };
    std::atomic<bool> bypassed{ false };

    mutable juce::SpinLock parameterLock;
    std::atomic<bool> needsReverbUpdate{ false };

    float smoothingTimeMs = 50.0f;

    //==============================================================================
    void updateReverbParameters();
    void applyPredelay(juce::dsp::AudioBlock<float>& inputBlock,
        juce::dsp::AudioBlock<float>& outputBlock);

    inline float hermiteInterpolation(float x, float y0, float y1, float y2, float y3) const noexcept;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SimpleVerbWithPredelay)
};