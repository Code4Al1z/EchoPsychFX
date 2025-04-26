#pragma once

#include <juce_gui_extra/juce_gui_extra.h>
#include <juce_dsp/juce_dsp.h>

class SpatialFX {
public:
    SpatialFX();
    ~SpatialFX() = default;

    void prepare(const juce::dsp::ProcessSpec& spec);
    void reset();

    void setPhaseOffsetLeft(float seconds);
    void setPhaseOffsetRight(float seconds);
    void setModulationRate(float hz);
    void setModulationDepth(float seconds);
    void setWetDryMix(float mix); // 0.0 = dry, 1.0 = wet

    void process(juce::dsp::AudioBlock<float>& block, juce::AudioPlayHead* playHead);

private:
    double sampleRate = 44100.0;
    const float maxDelaySeconds = 0.1f; // 100 ms
    const int maxDelaySamples = static_cast<int>(maxDelaySeconds * 48000);
    const double smoothingTime = 0.05; // 50 ms smoothing

    std::atomic<float>* syncParameter = nullptr;
    std::atomic<float> currentBpm{ 120.0f };
    float targetRateOrNote = 1.0f;

    void updateLfoFrequencies();

    juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Linear> delayLineLeft{ maxDelaySamples };
    juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Linear> delayLineRight{ maxDelaySamples };

    juce::dsp::Oscillator<float> lfoLeft;
    juce::dsp::Oscillator<float> lfoRight;

    juce::SmoothedValue<float> phaseOffsetLeft{ 0.0f };
    juce::SmoothedValue<float> phaseOffsetRight{ 0.0f };
    juce::SmoothedValue<float> modulationDepth{ 0.0f };
    juce::SmoothedValue<float> wetDryMix{ 0.0f };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SpatialFX)
};