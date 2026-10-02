#include "PluginProcessor.h"
#include "PluginEditor.h"

namespace
{
    using Attr = juce::AudioParameterFloatAttributes;

    // Display `value * scale` with `dp` decimals; the unit is added by the knob/host from the label.
    Attr numeric(const juce::String& unit, int dp, float scale = 1.0f, bool showSign = false)
    {
        return Attr()
            .withLabel(unit)
            .withStringFromValueFunction([=](float v, int)
                {
                    const float shown = v * scale;
                    return (showSign && shown > 0.0f ? juce::String("+") : juce::String())
                        + (dp == 0 ? juce::String(juce::roundToInt(shown)) : juce::String(shown, dp));
                })
            .withValueFromStringFunction([=](const juce::String& text)
                {
                    return text.getFloatValue() / scale;
                });
    }

    // Frequencies read better as "1.2 k" once past 1000 Hz.
    Attr frequency()
    {
        return Attr()
            .withLabel("Hz")
            .withStringFromValueFunction([](float v, int)
                {
                    return v >= 1000.0f ? juce::String(v / 1000.0f, 2) + " k" : juce::String(juce::roundToInt(v));
                })
            .withValueFromStringFunction([](const juce::String& text)
                {
                    const float n = text.getFloatValue();
                    return text.containsIgnoreCase("k") ? n * 1000.0f : n;
                });
    }

    constexpr float kRadToDeg = 57.29578f;

    Attr attributesFor(const juce::String& id)
    {
        if (id == "width" || id == "intensity" || id == "modMix" || id == "feedbackL" || id == "feedbackR"
            || id == "sfxModDepthL" || id == "sfxModDepthR" || id == "sfxWetDryMix" || id == "stereoSeparation"
            || id == "mix" || id == "detuneFeedback" || id == "diffusion" || id == "exciterMix"
            || id == "exciterToneBrightness" || id == "exciterHarmonicBalance" || id == "size" || id == "damping"
            || id == "wet")
            return numeric("%", 0, 100.0f);
        if (id == "midSideBalance")       return numeric("", 2, 1.0f, true);
        if (id == "tiltEQ")               return numeric("dB", 1, 6.0f, true);      // TiltEQ gain range is +/-6 dB
        if (id == "delayTime")            return numeric("ms", 1);
        if (id == "modDepth")             return numeric("ms", 2);
        if (id == "modRate" || id == "sfxModRateL" || id == "sfxModRateR" || id == "lfoRate")
            return numeric("Hz", 2);
        if (id == "phaseOffsetL" || id == "phaseOffsetR")
            return numeric(juce::String::fromUTF8("\xc2\xb0"), 1, kRadToDeg, true);
        if (id == "sfxLfoPhaseOffset")    return numeric(juce::String::fromUTF8("\xc2\xb0"), 0, kRadToDeg);
        if (id == "sfxAllpassFreq" || id == "exciterHighpass") return frequency();
        if (id == "haasDelayL" || id == "haasDelayR") return numeric("ms", 1);
        if (id == "detuneAmount")         return numeric("ct", 1, 1.0f, true);
        if (id == "lfoDepth")             return numeric("ms", 2, 1000.0f);        // stored in seconds
        if (id == "delayCentre")          return numeric("ms", 1, 1000.0f);        // stored in seconds
        if (id == "exciterDrive")         return numeric("", 1);
        if (id == "predelayMs")           return numeric("ms", 1);
        return numeric("", 2);
    }
}

//==============================================================================
AudioPluginAudioProcessor::AudioPluginAudioProcessor()
    : AudioProcessor(BusesProperties()
#if ! JucePlugin_IsMidiEffect
#if ! JucePlugin_IsSynth
        .withInput("Input", juce::AudioChannelSet::stereo(), true)
#endif
        .withOutput("Output", juce::AudioChannelSet::stereo(), true)
#endif
    )
    , parameters(*this, nullptr, "PARAMETERS", createParameterLayout())
    , bpm(120.0)
{
    // Set initial modulation type for ModDelay
    modDelay.setModulationType(ModDelay::ModulationType::Sine);
}

AudioPluginAudioProcessor::~AudioPluginAudioProcessor()
{


}

//==============================================================================
const juce::String AudioPluginAudioProcessor::getName() const

{
    return JucePlugin_Name;
}

bool AudioPluginAudioProcessor::acceptsMidi() const
{
#if JucePlugin_WantsMidiInput
    return true;
#else
    return false;
#endif
}

bool AudioPluginAudioProcessor::producesMidi() const
{
#if JucePlugin_ProducesMidiOutput
    return true;
#else
    return false;
#endif
}

bool AudioPluginAudioProcessor::isMidiEffect() const
{
#if JucePlugin_IsMidiEffect
    return true;
#else
    return false;
#endif
}

double AudioPluginAudioProcessor::getTailLengthSeconds() const
{
    if (spec.sampleRate <= 0.0)
        return 0.0;

    return static_cast<double>(simpleVerbWithPredelay.getTailLengthSamples()) / spec.sampleRate;
}

int AudioPluginAudioProcessor::getNumPrograms()
{
    return 1;
}

int AudioPluginAudioProcessor::getCurrentProgram()
{
    return 0;
}

void AudioPluginAudioProcessor::setCurrentProgram(int index)
{
    juce::ignoreUnused(index);
}

const juce::String AudioPluginAudioProcessor::getProgramName(int index)
{
    juce::ignoreUnused(index);
    return {};
}

void AudioPluginAudioProcessor::changeProgramName(int index, const juce::String& newName)
{
    juce::ignoreUnused(index, newName);
}

//==============================================================================
void AudioPluginAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = static_cast<juce::uint32>(samplesPerBlock);
    spec.numChannels = static_cast<juce::uint32>(getTotalNumOutputChannels());

    // Prepare all effect processors
    widthBalancer.prepare(spec);
    tiltEQ.prepare(spec);
    modDelay.prepare(spec);
    spatialFX.prepare(spec);
    microPitchDetune.prepare(spec);
    exciterSaturation.prepare(spec);

    // The exciter's oversampling filters add a few samples of latency; tell the host so it can compensate
    setLatencySamples(exciterSaturation.getLatencySamples());
    simpleVerbWithPredelay.prepare(spec);
}

void AudioPluginAudioProcessor::releaseResources()
{
    // Free any spare memory when playback stops
}

bool AudioPluginAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
#if JucePlugin_IsMidiEffect
    juce::ignoreUnused(layouts);
    return true;
#else
    // Only support mono or stereo
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
        && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // Input layout must match output layout
#if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
#endif

    return true;
#endif
}

void AudioPluginAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer,
    juce::MidiBuffer& midiMessages)
{
    juce::ignoreUnused(midiMessages);
    juce::ScopedNoDenormals noDenormals;

    if (auto* playHead = getPlayHead())
    {
        juce::AudioPlayHead::CurrentPositionInfo info;
        if (playHead->getCurrentPosition(info))
        {
            bpm = info.bpm > 0.0 ? info.bpm : 120.0;
        }
    }
    // -------------------------------------

    auto totalNumInputChannels = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // Clear any unused output channels
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear(i, 0, buffer.getNumSamples());

    // Wrap the buffer for DSP processing
    juce::dsp::AudioBlock<float> block(buffer);

    // Get sync state
    bool syncEnabled = parameters.getRawParameterValue("sync")->load();

    //==============================================================================
    // Effect chain processing order (psychoacoustic signal flow)
    //==============================================================================

    // 1. TiltEQ - Spectral balance adjustment
    {
        float tilt = *parameters.getRawParameterValue("tiltEQ");
        tiltEQ.setTilt(tilt);
        tiltEQ.process(block);
    }

    // 2. WidthBalancer - Stereo field manipulation
    {
        float width = *parameters.getRawParameterValue("width");
        float balance = *parameters.getRawParameterValue("midSideBalance");
        bool mono = parameters.getRawParameterValue("mono")->load();
        float intensity = *parameters.getRawParameterValue("intensity");

        widthBalancer.setWidth(width);
        widthBalancer.setMidSideBalance(balance);
        widthBalancer.setMono(mono);
        widthBalancer.setIntensity(intensity);
        widthBalancer.process(block);
    }

    // 3. ModDelay - Modulated delay effects
    {
        float delayTime = *parameters.getRawParameterValue("delayTime");
        float depth = *parameters.getRawParameterValue("modDepth");
        float rate = *parameters.getRawParameterValue("modRate");
        float feedbackL = *parameters.getRawParameterValue("feedbackL");
        float feedbackR = *parameters.getRawParameterValue("feedbackR");
        float modMix = *parameters.getRawParameterValue("modMix");
        int modulationTypeValue = juce::roundToInt(parameters.getRawParameterValue("modulationType")->load()) + 1;

        ModDelay::ModulationType modulationType = static_cast<ModDelay::ModulationType>(modulationTypeValue);
        modDelay.setModulationType(modulationType);
        modDelay.setTempo(static_cast<float>(bpm));
        modDelay.setSyncEnabled(syncEnabled);
        modDelay.setParams(delayTime, depth, rate, feedbackL, feedbackR, modMix);
        modDelay.process(block);
    }

    // 4. SpatialFX - Spatial positioning and phase manipulation
    {
        float phaseOffsetL = *parameters.getRawParameterValue("phaseOffsetL");
        float phaseOffsetR = *parameters.getRawParameterValue("phaseOffsetR");
        float sfxModRateL = *parameters.getRawParameterValue("sfxModRateL");
        float sfxModRateR = *parameters.getRawParameterValue("sfxModRateR");
        float sfxModDepthL = *parameters.getRawParameterValue("sfxModDepthL");
        float sfxModDepthR = *parameters.getRawParameterValue("sfxModDepthR");
        float mixValue = *parameters.getRawParameterValue("sfxWetDryMix");
        float sfxLfoPhaseOffset = *parameters.getRawParameterValue("sfxLfoPhaseOffset");
        float allpassFreq = *parameters.getRawParameterValue("sfxAllpassFreq");
        float haasDelayL = *parameters.getRawParameterValue("haasDelayL");
        float haasDelayR = *parameters.getRawParameterValue("haasDelayR");
        // Choice index 0..3 -> LfoWaveform Sine..Random, which starts at 1
        int modulationShapeValue = juce::roundToInt(parameters.getRawParameterValue("modulationShape")->load()) + 1;

        SpatialFX::LfoWaveform modulationShape = static_cast<SpatialFX::LfoWaveform>(modulationShapeValue);
        spatialFX.setPhaseAmount(phaseOffsetL, phaseOffsetR);
        spatialFX.setLfoRate(sfxModRateL, sfxModRateR);
        spatialFX.setLfoDepth(sfxModDepthL, sfxModDepthR);
        spatialFX.setWetDry(mixValue);
        spatialFX.setLfoPhaseOffset(sfxLfoPhaseOffset);
        spatialFX.setAllpassFrequency(allpassFreq);
        spatialFX.setHaasDelayMs(haasDelayL, haasDelayR);
        spatialFX.setLfoWaveform(modulationShape);
        spatialFX.process(block);
    }

    // 5. MicroPitchDetune - Subtle pitch shifting for thickness
    {
        float detuneAmount = *parameters.getRawParameterValue("detuneAmount");
        float lfoRate = *parameters.getRawParameterValue("lfoRate");
        float lfoDepth = *parameters.getRawParameterValue("lfoDepth");
        float delayCentre = *parameters.getRawParameterValue("delayCentre");
        float stereoSeparation = *parameters.getRawParameterValue("stereoSeparation");
        float mix = *parameters.getRawParameterValue("mix");
        float detuneFeedback = *parameters.getRawParameterValue("detuneFeedback");
        float diffusion = *parameters.getRawParameterValue("diffusion");

        microPitchDetune.setParams(detuneAmount, lfoRate, lfoDepth,
            delayCentre, stereoSeparation, mix, detuneFeedback, diffusion);
        microPitchDetune.setBpm(static_cast<float>(bpm));
        microPitchDetune.process(block);
    }

    // 6. ExciterSaturation - Harmonic enhancement
    {
        float drive = *parameters.getRawParameterValue("exciterDrive");
        float exciterMix = *parameters.getRawParameterValue("exciterMix");
        float highpassFreq = *parameters.getRawParameterValue("exciterHighpass");
        int satTypeValue = juce::roundToInt(parameters.getRawParameterValue("exciterSaturationType")->load());
        int harmonicModeValue = juce::roundToInt(parameters.getRawParameterValue("exciterHarmonicMode")->load());
        float toneBrightness = *parameters.getRawParameterValue("exciterToneBrightness");
        float harmonicBalance = *parameters.getRawParameterValue("exciterHarmonicBalance");
        bool autoGain = *parameters.getRawParameterValue("exciterAutoGain") >= 0.5f;

        exciterSaturation.setDrive(drive);
        exciterSaturation.setMix(exciterMix);
        exciterSaturation.setHighpass(highpassFreq);
        exciterSaturation.setSaturationType(static_cast<ExciterSaturation::SaturationType>(satTypeValue));
        exciterSaturation.setHarmonicMode(static_cast<ExciterSaturation::HarmonicMode>(harmonicModeValue));
        exciterSaturation.setToneBrightness(toneBrightness);
        exciterSaturation.setHarmonicBalance(harmonicBalance);
        exciterSaturation.setAutoGainEnabled(autoGain);
        exciterSaturation.process(block);
    }

    // 7. SimpleVerbWithPredelay - Reverb with pre-delay
    {
        float predelayMs = *parameters.getRawParameterValue("predelayMs");
        float size = *parameters.getRawParameterValue("size");
        float damping = *parameters.getRawParameterValue("damping");
        float wet = *parameters.getRawParameterValue("wet");

        simpleVerbWithPredelay.setParams(predelayMs, size, damping, wet);
        simpleVerbWithPredelay.process(block);
    }
}

//==============================================================================
bool AudioPluginAudioProcessor::hasEditor() const
{
    return true;
}

juce::AudioProcessorEditor* AudioPluginAudioProcessor::createEditor()
{
    return new AudioPluginAudioProcessorEditor(*this);
}

//==============================================================================
void AudioPluginAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    juce::MemoryOutputStream stream(destData, true);
    parameters.state.writeToStream(stream);
}

void AudioPluginAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    juce::ValueTree tree = juce::ValueTree::readFromData(data, static_cast<size_t>(sizeInBytes));

    if (tree.isValid())
    {
        parameters.state = tree;

        // Restore modulation type
        if (tree.hasProperty("modulationType"))
        {
            modDelay.setModulationType(
                static_cast<ModDelay::ModulationType>(static_cast<int>(tree.getProperty("modulationType")) + 1));
        }
    }
}

//==============================================================================
// This creates new instances of the plugin
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new AudioPluginAudioProcessor();
}

juce::AudioProcessorValueTreeState::ParameterLayout AudioPluginAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;


    //==============================================================================
    // WidthBalancer Parameters
    //==============================================================================
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ "width", 1 },
        "Width",
        juce::NormalisableRange<float>(0.0f, 2.0f, 0.01f),
        1.0f,
        attributesFor("width")));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ "midSideBalance", 1 },
        "Mid/Side Balance",
        juce::NormalisableRange<float>(-1.0f, 1.0f, 0.01f),
        0.0f,
        attributesFor("midSideBalance")));

    params.push_back(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID{ "mono", 1 },
        "Mono",
        false));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ "intensity", 1 },
        "Intensity",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f),
        0.5f,
        attributesFor("intensity")));

    //==============================================================================
    // TiltEQ Parameters
    //==============================================================================
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ "tiltEQ", 1 },
        "Tilt EQ",
        juce::NormalisableRange<float>(-1.0f, 1.0f, 0.01f),
        0.0f,
        attributesFor("tiltEQ")));

    //==============================================================================
    // ModDelay Parameters
    //==============================================================================
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ "delayTime", 1 },
        "Delay Time",
        juce::NormalisableRange<float>(1.0f, 2000.0f, 0.1f),
        400.0f,
        attributesFor("delayTime")));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ "feedbackL", 1 },
        "Feedback L",
        juce::NormalisableRange<float>(0.0f, 0.95f, 0.01f),
        0.4f,
        attributesFor("feedbackL")));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ "feedbackR", 1 },
        "Feedback R",
        juce::NormalisableRange<float>(0.0f, 0.95f, 0.01f),
        0.4f,
        attributesFor("feedbackR")));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ "modMix", 1 },
        "Mod Mix",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f),
        0.5f,
        attributesFor("modMix")));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ "modDepth", 1 },
        "Mod Depth",
        juce::NormalisableRange<float>(0.0f, 10.0f, 0.01f),
        2.0f,
        attributesFor("modDepth")));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ "modRate", 1 },
        "Mod Rate",
        juce::NormalisableRange<float>(0.01f, 10.0f, 0.01f),
        0.25f,
        attributesFor("modRate")));

    params.push_back(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID{ "modulationType", 1 },
        "Modulation Type",
        juce::StringArray{ "Sine", "Triangle", "Square", "Sawtooth Up", "Sawtooth Down" },
        0)); // Default: Sine

    params.push_back(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID{ "sync", 1 },
        "Sync",
        false));

    //==============================================================================
    // SpatialFX Parameters
    //==============================================================================
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ "phaseOffsetL", 1 },
        "Phase L Offset",
        juce::NormalisableRange<float>(-0.1f, 0.1f, 0.001f),
        0.0f,
        attributesFor("phaseOffsetL")));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ "phaseOffsetR", 1 },
        "Phase R Offset",
        juce::NormalisableRange<float>(-0.1f, 0.1f, 0.001f),
        0.0f,
        attributesFor("phaseOffsetR")));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ "sfxModRateL", 1 },
        "SFX Rate L",
        juce::NormalisableRange<float>(0.01f, 10.0f, 0.01f),
        0.1f,
        attributesFor("sfxModRateL")));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ "sfxModRateR", 1 },
        "SFX Rate R",
        juce::NormalisableRange<float>(0.01f, 10.0f, 0.01f),
        0.1f,
        attributesFor("sfxModRateR")));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ "sfxModDepthL", 1 },
        "SFX Depth L",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f),
        0.5f,
        attributesFor("sfxModDepthL")));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ "sfxModDepthR", 1 },
        "SFX Depth R",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f),
        0.5f,
        attributesFor("sfxModDepthR")));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ "sfxWetDryMix", 1 },
        "SFX Wet/Dry",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f),
        0.5f,
        attributesFor("sfxWetDryMix")));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ "sfxLfoPhaseOffset", 1 },
        "LFO Phase",
        juce::NormalisableRange<float>(0.0f, juce::MathConstants<float>::twoPi, 0.01f),
        juce::MathConstants<float>::halfPi,
        attributesFor("sfxLfoPhaseOffset")));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ "sfxAllpassFreq", 1 },
        "Allpass Freq",
        juce::NormalisableRange<float>(20.0f, 20000.0f, 1.0f),
        1000.0f,
        attributesFor("sfxAllpassFreq")));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ "haasDelayL", 1 },
        "Haas Delay L",
        juce::NormalisableRange<float>(0.0f, 40.0f, 0.1f),
        0.0f,
        attributesFor("haasDelayL")));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ "haasDelayR", 1 },
        "Haas Delay R",
        juce::NormalisableRange<float>(0.0f, 40.0f, 0.1f),
        0.0f,
        attributesFor("haasDelayR")));

    params.push_back(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID{ "modulationShape", 1 },
        "Modulation Shape",
        juce::StringArray{ "Sine", "Triangle", "Square", "Random" },
        0)); // Default: Sine

    //==============================================================================
    // MicroPitchDetune Parameters
    //==============================================================================
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ "detuneAmount", 1 },
        "Detune Amount",
        juce::NormalisableRange<float>(-50.0f, 50.0f, 0.1f),
        0.0f,
        attributesFor("detuneAmount")));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ "lfoRate", 1 },
        "LFO Rate",
        juce::NormalisableRange<float>(0.01f, 20.0f, 0.01f),
        0.3f,
        attributesFor("lfoRate")));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ "lfoDepth", 1 },
        "LFO Depth",
        juce::NormalisableRange<float>(0.0f, 0.01f, 0.0001f),
        0.002f,
        attributesFor("lfoDepth")));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ "delayCentre", 1 },
        "Delay Centre",
        juce::NormalisableRange<float>(0.001f, 0.015f, 0.0001f),
        0.005f,
        attributesFor("delayCentre")));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ "stereoSeparation", 1 },
        "Stereo Separation",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f),
        0.5f,
        attributesFor("stereoSeparation")));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ "mix", 1 },
        "Mix",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f),
        0.5f,
        attributesFor("mix")));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ "detuneFeedback", 1 },
        "Detune Feedback",
        juce::NormalisableRange<float>(0.0f, 0.7f, 0.01f),
        0.0f,
        attributesFor("detuneFeedback")));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ "diffusion", 1 },
        "Diffusion",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f),
        0.0f,
        attributesFor("diffusion")));

    //==============================================================================
    // ExciterSaturation Parameters
    //==============================================================================
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ "exciterDrive", 1 },
        "Exciter Drive",
        juce::NormalisableRange<float>(0.0f, 10.0f, 0.01f),
        0.5f,
        attributesFor("exciterDrive")));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ "exciterMix", 1 },
        "Exciter Mix",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f),
        0.5f,
        attributesFor("exciterMix")));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ "exciterHighpass", 1 },
        "Exciter Highpass",
        juce::NormalisableRange<float>(20.0f, 8000.0f, 1.0f),
        1000.0f,
        attributesFor("exciterHighpass")));

    params.push_back(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID{ "exciterSaturationType", 1 },
        "Exciter Saturation Type",
        juce::StringArray{ "Soft", "Hard", "Tube", "Tape", "Transformer", "Digital" },
        0)); // Default: Soft

    params.push_back(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID{ "exciterHarmonicMode", 1 },
        "Exciter Harmonic Mode",
        juce::StringArray{ "Balanced", "Odd Only", "Even Only" },
        0)); // Default: Balanced

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ "exciterToneBrightness", 1 },
        "Exciter Tone Brightness",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f),
        0.5f,
        attributesFor("exciterToneBrightness")));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ "exciterHarmonicBalance", 1 },
        "Exciter Harmonic Balance",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f),
        0.5f,
        attributesFor("exciterHarmonicBalance")));

    params.push_back(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID{ "exciterAutoGain", 1 },
        "Exciter Auto Gain",
        true));

    //==============================================================================
    // SimpleVerbWithPredelay Parameters
    //==============================================================================
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ "predelayMs", 1 },
        "Pre-delay",
        juce::NormalisableRange<float>(0.0f, 100.0f, 0.1f),
        20.0f,
        attributesFor("predelayMs")));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ "size", 1 },
        "Reverb Size",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f),
        0.5f,
        attributesFor("size")));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ "damping", 1 },
        "Damping",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f),
        0.3f,
        attributesFor("damping")));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ "wet", 1 },
        "Wet Level",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f),
        0.5f,
        attributesFor("wet")));

    return { params.begin(), params.end() };
}