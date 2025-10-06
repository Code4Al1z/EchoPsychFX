#pragma once
#include <juce_dsp/juce_dsp.h>
#include <array>

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

    void setDrive(float newDrive);              // 0.0 to 1.0
    void setMix(float newMix);                  // 0.0 (dry) to 1.0 (wet)
    void setHighpass(float freqHz);             // Apply saturation above this freq
    void setToneBrightness(float brightness);   // 0.0 to 1.0 - pre/de-emphasis
    void setHarmonicBalance(float balance);     // 0.0 (dark) to 1.0 (bright)
    void setSaturationType(SaturationType type);
    void setHarmonicMode(HarmonicMode mode);
    void setAutoGainEnabled(bool enabled);

    void process(juce::dsp::AudioBlock<float>& block);

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

    // 2x oversampling
    juce::dsp::Oversampling<float> oversampling;

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

    // RMS metering for auto-gain
    std::array<float, 2> inputRMS = { 0.0f, 0.0f };
    std::array<float, 2> outputRMS = { 0.0f, 0.0f };

    // Waveshaping functions
    float waveshape(float x, SaturationType type);
    float softSaturation(float x);
    float hardClip(float x);
    float tubeSaturation(float x);
    float tapeSaturation(float x);
    float transformerSaturation(float x);
    float digitalSaturation(float x);

    // Harmonic filtering
    float applyHarmonicMode(float x, HarmonicMode mode);

    // Gain compensation
    float calculateGainCompensation();

    // Filter update helpers
    void updateHighpass();
    void updatePreEmphasis();
    void updateDeEmphasis();
    void updateToneFilter();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ExciterSaturation)
};