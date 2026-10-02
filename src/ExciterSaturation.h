#pragma once
#include <juce_dsp/juce_dsp.h>
#include <array>
#include <memory>

class ExciterSaturation
{
public:
    // Saturation algorithm types
    enum class SaturationType
    {
        Soft,           // Tanh - smooth, warm
        Hard,           // Soft clip - aggressive
        Tube,           // Asymmetric - even harmonics
        Tape,           // Tape-style compression + saturation
        Transformer,    // Transformer-style saturation
        Digital         // Bit reduction style
    };

    // Harmonic emphasis modes
    enum class HarmonicMode
    {
        Balanced,       // Both odd and even harmonics
        OddOnly,        // Odd harmonics (hollow sound)
        EvenOnly        // Even harmonics (warm, tube-like)
    };

    // Preset structure
    struct Preset
    {
        juce::String name;
        float drive;
        float mix;
        float highpassFreq;
        float toneBrightness;
        float harmonicBalance;
        SaturationType satType;
        HarmonicMode harmonicMode;
        bool autoGainEnabled;

        Preset(const juce::String& n = "Default", float d = 0.5f, float m = 0.5f,
            float hp = 3000.0f, float tb = 0.5f, float hb = 0.5f,
            SaturationType st = SaturationType::Soft,
            HarmonicMode hm = HarmonicMode::Balanced, bool ag = true)
            : name(n), drive(d), mix(m), highpassFreq(hp), toneBrightness(tb),
            harmonicBalance(hb), satType(st), harmonicMode(hm), autoGainEnabled(ag) {
        }
    };

    ExciterSaturation();
    ~ExciterSaturation() = default;

    void prepare(const juce::dsp::ProcessSpec& spec);
    void reset();

    void setDrive(float newDrive);              // 0.0 to 10.0 - matches the exciterDrive APVTS parameter
    void setMix(float newMix);                  // 0.0 (dry) to 1.0 (wet)
    void setHighpass(float freqHz);             // Apply saturation above this freq
    void setToneBrightness(float brightness);   // 0.0 to 1.0 - pre/de-emphasis
    void setHarmonicBalance(float balance);     // 0.0 (dark) to 1.0 (bright)
    void setSaturationType(SaturationType type);
    void setHarmonicMode(HarmonicMode mode);
    void setAutoGainEnabled(bool enabled);

    void process(juce::dsp::AudioBlock<float>& block);

    /** Latency added by the oversampling filters, in samples. The dry path is delayed by the same
        amount so the two stay aligned; the plugin reports this to the host. Valid after prepare(). */
    int getLatencySamples() const noexcept { return latencySamples; }

    // Preset management
    void loadPreset(const Preset& preset);
    Preset getCurrentPreset() const;
    static std::vector<Preset> getFactoryPresets();

private:
    // Parameters
    float drive = 0.5f;
    float mix = 0.5f;
    float highpassFreq = 3000.0f;
    float toneBrightness = 0.5f;
    float harmonicBalance = 0.5f;
    SaturationType saturationType = SaturationType::Soft;
    HarmonicMode harmonicMode = HarmonicMode::Balanced;
    bool autoGainEnabled = true;

    float sampleRate = 44100.0f;

    // 2x oversampling - constructed in prepare() once the real channel count is known
    std::unique_ptr<juce::dsp::Oversampling<float>> oversampling;

    // Pre-saturation highpass
    juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>,
        juce::dsp::IIR::Coefficients<float>> highpass;

    // Pre-emphasis filter (boost highs before saturation)
    juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>,
        juce::dsp::IIR::Coefficients<float>> preEmphasis;

    // De-emphasis filter (compensate after saturation)
    juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>,
        juce::dsp::IIR::Coefficients<float>> deEmphasis;

    // DC blocking filter
    juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>,
        juce::dsp::IIR::Coefficients<float>> dcBlocker;

    // Tone shaping filter
    juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>,
        juce::dsp::IIR::Coefficients<float>> toneFilter;

    // Smoothed parameters to avoid zipper noise
    juce::SmoothedValue<float> smoothedDrive;
    juce::SmoothedValue<float> smoothedMix;

    // Buffers
    juce::AudioBuffer<float> dryBuffer;
    juce::AudioBuffer<float> oversampledBuffer;

    // Latency compensation: the dry signal is delayed to line up with the oversampled wet path
    int latencySamples = 0;
    juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::None> dryDelay;

    // Smoothed level-match gain applied to the saturated band when Auto Gain is on
    float autoGainValue = 1.0f;
    bool autoGainPrimed = false;   // first block with signal snaps straight to the right gain

    // Waveshaping functions
    float curve(float u, SaturationType type);
    float waveshape(float x, SaturationType type, HarmonicMode mode, float driveAmount);
    float softSaturation(float x);
    float hardClip(float x);
    float tubeSaturation(float x);
    float tapeSaturation(float x);
    float transformerSaturation(float x);
    float digitalSaturation(float x);

    // Filter update helpers
    void updateHighpass();
    void updatePreEmphasis();
    void updateDeEmphasis();
    void updateToneFilter();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ExciterSaturation)
};