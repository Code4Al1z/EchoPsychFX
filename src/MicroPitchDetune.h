#pragma once
#include <juce_dsp/juce_dsp.h>
#include <random>
#include <array>

class MicroPitchDetune
{
public:
    // Preset structure for easy parameter management
    struct Preset
    {
        juce::String name;
        float detuneCents;
        float lfoRate;
        float lfoDepth;
        float delayCentre;
        float stereoSeparation;
        float mix;
        float feedback;
        float diffusion;
        bool syncEnabled;

        Preset(const juce::String& n = "Default", float dc = 5.0f, float lr = 0.1f,
            float ld = 0.002f, float del = 0.005f, float ss = 0.5f,
            float m = 0.5f, float fb = 0.0f, float diff = 0.0f, bool sync = false)
            : name(n), detuneCents(dc), lfoRate(lr), lfoDepth(ld),
            delayCentre(del), stereoSeparation(ss), mix(m),
            feedback(fb), diffusion(diff), syncEnabled(sync) {
        }
    };

    MicroPitchDetune();
    ~MicroPitchDetune() = default;

    void prepare(const juce::dsp::ProcessSpec& spec);
    void reset();

    void setParams(float detuneCentsIn, float lfoRateIn, float lfoDepthIn,
        float delayCentreIn, float stereoSeparationIn, float mixIn,
        float feedbackIn = 0.0f, float diffusionIn = 0.0f);
    void setSyncEnabled(bool shouldSync);
    void setBpm(float newBpm);

    void process(juce::dsp::AudioBlock<float>& block);

    // Preset management
    void loadPreset(const Preset& preset);
    Preset getCurrentPreset() const;
    static std::vector<Preset> getFactoryPresets();

private:
    // Multi-tap delay structure for richer sound
    static constexpr int NUM_TAPS = 3;

    struct DelayTap
    {
        juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Lagrange3rd> delay;
        juce::SmoothedValue<float> smoothedDelay;
        float feedback = 0.0f;
        float timeOffset = 0.0f;  // Offset from base delay time
        float phaseOffset = 0.0f;  // LFO phase offset
    };

    std::array<DelayTap, NUM_TAPS> tapsL;
    std::array<DelayTap, NUM_TAPS> tapsR;

    // DC blocking filters for feedback paths
    juce::dsp::IIR::Filter<float> dcBlockerL;
    juce::dsp::IIR::Filter<float> dcBlockerR;

    // One-pole lowpass for anti-aliasing modulation
    juce::SmoothedValue<float> modulationSmoother;

    float sampleRate = 44100.0f;
    float detuneCents = 5.0f;
    float lfoRate = 0.1f;
    float lfoDepth = 0.002f;
    float delayCentre = 0.005f;
    float stereoSeparation = 0.5f;
    float mix = 0.5f;
    float feedback = 0.0f;
    float diffusion = 0.0f;  // Controls how much the taps are spread out

    float modPhase = 0.0f;
    float maxDelayTime = 0.02f;

    bool syncEnabled = false;
    float bpm = 120.0f;

    std::mt19937 randomEngine;
    std::uniform_real_distribution<float> randomDistribution;

    // Improved LFO with multiple shapes
    float lfo(float phase);
    float lfoTriangle(float phase);
    float centsToDelayOffset(float cents, float baseDelay);
    void updateTapOffsets();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MicroPitchDetune)
};