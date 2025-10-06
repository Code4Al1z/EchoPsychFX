#pragma once
#include <juce_dsp/juce_dsp.h>
#include <juce_audio_basics/juce_audio_basics.h>

/**
 * @brief High-quality tilt equalizer with smooth parameter changes
 *
 * Implements a professional tilt EQ using complementary low and high shelves.
 * Thread-safe and optimized for real-time audio processing.
 */
class TiltEQ {
public:
    TiltEQ() = default;
    ~TiltEQ() = default;

    //==============================================================================
    /** Prepares the processor for playback */
    void prepare(const juce::dsp::ProcessSpec& spec);

    /** Resets the internal filter states */
    void reset();

    //==============================================================================
    /** Sets the tilt amount: -1.0 (warm/bass boost) to +1.0 (bright/treble boost) */
    void setTilt(float tiltAmount);

    /** Sets the low shelf frequency (default: 200 Hz) */
    void setLowShelfFrequency(float freqHz);

    /** Sets the high shelf frequency (default: 4000 Hz) */
    void setHighShelfFrequency(float freqHz);

    /** Sets the maximum gain range in dB (default: ±6 dB) */
    void setGainRange(float rangeDb);

    /** Sets the Q factor for both shelves (default: 0.707) */
    void setQ(float qFactor);

    /** Sets the parameter smoothing time in milliseconds (default: 20ms) */
    void setSmoothingTime(float timeMs);

    /** Enable/disable the tilt EQ */
    void setBypassed(bool shouldBeBypassed);

    //==============================================================================
    /** Processes an audio block */
    void process(juce::dsp::AudioBlock<float>& block);

    /** Processes a const block (non-replacing) */
    void process(const juce::dsp::ProcessContextNonReplacing<float>& context);

    //==============================================================================
    /** Returns current tilt amount */
    float getTilt() const noexcept { return tiltParam.getTargetValue(); }

    /** Returns current low shelf frequency */
    float getLowShelfFrequency() const noexcept { return lowFreq; }

    /** Returns current high shelf frequency */
    float getHighShelfFrequency() const noexcept { return highFreq; }

    /** Returns whether the EQ is bypassed */
    bool isBypassed() const noexcept { return bypassed.load(std::memory_order_relaxed); }

    /** Returns the frequency response magnitude at a given frequency */
    float getMagnitudeForFrequency(float frequency) const;

private:
    //==============================================================================
    using Filter = juce::dsp::IIR::Filter<float>;
    using Coefficients = juce::dsp::IIR::Coefficients<float>;
    using FilterChain = juce::dsp::ProcessorDuplicator<Filter, Coefficients>;

    FilterChain lowShelf, highShelf;

    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> tiltParam;

    float lowFreq = 200.0f;
    float highFreq = 4000.0f;
    float gainRange = 6.0f;
    float qFactor = 0.707f;

    double sampleRate = 44100.0;
    std::atomic<bool> bypassed{ false };

    mutable juce::SpinLock parameterLock;
    std::atomic<bool> needsUpdate{ true };

    //==============================================================================
    void updateFilters();
    void updateFiltersIfNeeded();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TiltEQ)
};