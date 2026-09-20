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
        juce::dsp::IIR::Filter<float> dcBlocker;  // per-tap so feedback paths don't corrupt each other
        juce::SmoothedValue<float> modulationSmoother;  // per-tap anti-aliasing for the vibrato LFO
        float pitchPhase = 0.0f;  // continuously ramping read-head phase for real pitch shifting
        float feedbackState = 0.0f;
        float timeOffset = 0.0f;  // Offset from base delay time
        float phaseOffset = 0.0f;  // LFO phase offset
    };

    std::array<DelayTap, NUM_TAPS> tapsL;
    std::array<DelayTap, NUM_TAPS> tapsR;

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
    // Total time budget for each delay line, in seconds. Needs to comfortably exceed
    // delayCentre's max (0.015s, set in PluginProcessor's parameter layout) plus the
    // pitch-shifter's grain crossfade window (up to 0.030s, see process()) so the window
    // isn't squeezed back down to its short, buzzy minimum by a lack of headroom.
    float maxDelayTime = 0.06f;

    bool syncEnabled = false;
    float bpm = 120.0f;

    juce::SmoothedValue<float> smoothedMix;
    juce::SmoothedValue<float> smoothedDetuneCents;
    juce::SmoothedValue<float> smoothedDelayCentre;
    juce::SmoothedValue<float> smoothedStereoSeparation;
    juce::SmoothedValue<float> smoothedFeedback;

    std::mt19937 randomEngine;
    std::uniform_real_distribution<float> randomDistribution;

    // Improved LFO with multiple shapes
    float lfo(float phase);
    float lfoTriangle(float phase);
    void updateTapOffsets();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MicroPitchDetune)
};