#pragma once

#include <juce_gui_extra/juce_gui_extra.h>
#include <juce_dsp/juce_dsp.h>

class SpatialFX {
public:
    enum class ModShape { Sine = 1, Triangle, Noise };

    SpatialFX();
    ~SpatialFX() = default;

    void prepare(const juce::dsp::ProcessSpec& spec);
    void reset();

    void setParams(float leftOffset, float rightOffset, float rate, float depth, float mix, ModShape modShape);
    void setSyncEnabled(bool shouldSync);
    void setFeedback(float fb);
    void setLfoPhaseOffset(float radians);

    void process(juce::dsp::AudioBlock<float>& block, juce::AudioPlayHead* playHead);

    float getEffectiveRateHz() const;

private:
    void updateLfoFrequencies();
    void updateLfoWaveforms();

    float sampleRate = 44100.0f;
    const float maxDelaySeconds = 0.1f; // 100 ms
    const float minDelaySeconds = 0.001f;
    const float maxModDepthSeconds = maxDelaySeconds - minDelaySeconds;
    const float smoothingTime = 0.05f;

    int maxDelaySamples = static_cast<int>(maxDelaySeconds * 48000.0f);
    const float minDelaySamples = minDelaySeconds * 48000.0f;

    std::atomic<float> currentBpm{ 120.0f };
    std::atomic<float> feedbackAmount{ 0.0f };

    float targetRateOrNote = 1.0f;
    bool syncEnabled = false;
    std::atomic<float>* syncParameter = nullptr;
    float lfoPhaseOffsetRadians = juce::MathConstants<float>::halfPi;

    ModShape modulationShape = ModShape::Sine;

    juce::SmoothedValue<float> phaseOffsetLeft{ 0.0f };
    juce::SmoothedValue<float> phaseOffsetRight{ 0.0f };
    juce::SmoothedValue<float> modulationDepth{ 0.5f };
    juce::SmoothedValue<float> wetDryMix{ 0.5f };

    juce::SmoothedValue<float> smoothedNoiseLeft{ 0.05f };
    juce::SmoothedValue<float> smoothedNoiseRight{ 0.05f };

    juce::dsp::Oscillator<float> lfoLeft;
    juce::dsp::Oscillator<float> lfoRight;

    juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Lagrange3rd> delayLineLeft;
    juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Lagrange3rd> delayLineRight;


    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SpatialFX)
};