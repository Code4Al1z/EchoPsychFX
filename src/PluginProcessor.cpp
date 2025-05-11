#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
AudioPluginAudioProcessor::AudioPluginAudioProcessor()
    : AudioProcessor(BusesProperties()
#if ! JucePlugin_IsMidiEffect
#if ! JucePlugin_IsSynth
        .withInput("Input", juce::AudioChannelSet::stereo(), true)
#endif
        .withOutput("Output", juce::AudioChannelSet::stereo(), true)
#endif
    ),
    parameters(*this, nullptr, "PARAMETERS", createParameterLayout()),
    widthBalancer(),
	tiltEQ(),
    modDelay(),
    spatialFX(),
    microPitchDetune(),
	exciterSaturation(),
	simpleVerbWithPredelay(),
	bpm(120.0f)
{
    // Optionally set the initial modulation type
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
    return 0.0;
}

int AudioPluginAudioProcessor::getNumPrograms()
{
    return 1; // NB: some hosts don't cope very well if you tell them there are 0 programs,
    // so this should be at least 1, even if you're not really implementing programs.
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
    spec.maximumBlockSize = samplesPerBlock;
    spec.numChannels = getTotalNumOutputChannels();

    if (auto* playHead = getPlayHead())
    {
        juce::AudioPlayHead::CurrentPositionInfo info;
        if (playHead->getCurrentPosition(info))
        {
            if (info.bpm > 0.0)
                bpm = info.bpm;
            else
                bpm = 120.0; // fallback
        }
    }

    widthBalancer.prepare(spec);
    tiltEQ.prepare(spec);
    modDelay.prepare(spec);
    spatialFX.prepare(spec);
    microPitchDetune.prepare(spec);
	exciterSaturation.prepare(spec);
	simpleVerbWithPredelay.prepare(spec);

    dryBuffer.setSize(spec.numChannels, spec.maximumBlockSize);
}

void AudioPluginAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

bool AudioPluginAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
#if JucePlugin_IsMidiEffect
    juce::ignoreUnused(layouts);
    return true;
#else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
        && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
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

    auto totalNumInputChannels = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear(i, 0, buffer.getNumSamples());

    // Wrap the input buffer for processing
    juce::dsp::AudioBlock<float> block(buffer);

    // Copy the input buffer to the dry buffer for later wet/dry mixing
    dryBuffer.makeCopyOf(buffer, true); // true = clear extra space if needed
    juce::dsp::AudioBlock<float> dryBlock(dryBuffer);

    bool syncEnabled = parameters.getRawParameterValue("sync")->load();

#pragma region Tilt EQ
    float tilt = *parameters.getRawParameterValue("tiltEQ");
    tiltEQ.setTilt(tilt);
    tiltEQ.process(block);
#pragma endregion

#pragma region WidthBalancer
    float width = *parameters.getRawParameterValue("width");
    float balance = *parameters.getRawParameterValue("midSideBalance");
    bool mono = parameters.getRawParameterValue("mono")->load();
	float intensity = *parameters.getRawParameterValue("intensity");

    widthBalancer.setWidth(width);
    widthBalancer.setMidSideBalance(balance);
    widthBalancer.setMono(mono);
	widthBalancer.setIntensity(intensity);
    widthBalancer.process(block);
#pragma endregion

#pragma region ModDelay
    float delayTime = *parameters.getRawParameterValue("delayTime");
    float depth = *parameters.getRawParameterValue("modDepth");
    float rate = *parameters.getRawParameterValue("modRate");
    float feedbackL = *parameters.getRawParameterValue("feedbackL"); // Get separate feedback values
    float feedbackR = *parameters.getRawParameterValue("feedbackR");
    int modulationTypeValue = parameters.state.getProperty("modulationType"); // Get the raw value
	float modMix = *parameters.getRawParameterValue("modMix");

    // Convert the raw value to the enum
    ModDelay::ModulationType modulationType = static_cast<ModDelay::ModulationType>(modulationTypeValue);
    modDelay.setModulationType(modulationType);

    modDelay.setTempo(bpm);
    modDelay.setSyncEnabled(syncEnabled);
    modDelay.setParams(delayTime, depth, rate, feedbackL, feedbackR, modMix);

    // Apply modulated delay effect
    modDelay.process(block);
#pragma endregion

#pragma region SpatialFX
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

    int modulationShapeValue = parameters.state.getProperty("modulationShape");
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
#pragma endregion

#pragma region MicroPitchDetune
    float detuneAmount = *parameters.getRawParameterValue("detuneAmount");
    float lfoRate = *parameters.getRawParameterValue("lfoRate");
    float lfoDepth = *parameters.getRawParameterValue("lfoDepth");
    float delayCentre = *parameters.getRawParameterValue("delayCentre");
    float stereoSeparation = *parameters.getRawParameterValue("stereoSeparation");
    float mix = *parameters.getRawParameterValue("mix");

	microPitchDetune.setParams(detuneAmount, lfoRate, lfoDepth,
		delayCentre, stereoSeparation, mix);
	microPitchDetune.setBpm(bpm);
	microPitchDetune.process(block);
#pragma endregion

#pragma region ExciterSaturation
	float drive = *parameters.getRawParameterValue("exciterDrive");
	float exciterMix = *parameters.getRawParameterValue("exciterMix");
	float highpassFreq = *parameters.getRawParameterValue("exciterHighpass");
	exciterSaturation.setDrive(drive);
	exciterSaturation.setMix(exciterMix);
	exciterSaturation.setHighpass(highpassFreq);
	exciterSaturation.process(block);
#pragma endregion

#pragma region SimpleVerbWithPredelay
	float predelayMs = *parameters.getRawParameterValue("predelayMs");
	float size = *parameters.getRawParameterValue("size");
	float damping = *parameters.getRawParameterValue("damping");
	float wet = *parameters.getRawParameterValue("wet");
	simpleVerbWithPredelay.setParams(predelayMs, size, damping, wet);
	simpleVerbWithPredelay.process(block);
#pragma endregion
}



//==============================================================================
bool AudioPluginAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
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
    stream.writeInt((int)modDelay.getModulationType()); // Save the modulation type
}

void AudioPluginAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    juce::ValueTree tree = juce::ValueTree::readFromData(data, sizeInBytes);

    if (tree.isValid())
    {
        parameters.state = tree;
        if (tree.hasProperty("modulationType"))
        {
            modDelay.setModulationType(static_cast<ModDelay::ModulationType>((int)tree.getProperty("modulationType")));
        }
    }
}

juce::AudioProcessorValueTreeState::ParameterLayout AudioPluginAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    // Helper lambdas
    auto floatToString2dp = [](float value, int) {
        return juce::String(value, 2); // show 2 decimals
        };
    auto stringToFloat = [](const juce::String& text) {
        return text.getFloatValue(); // parse from string
        };

    // WidthBalancer
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ "width", 1 },
        "Width",
        juce::NormalisableRange<float>(0.0f, 2.0f, 0.01f),
        1.0f,
        juce::AudioParameterFloatAttributes()
        .withStringFromValueFunction(floatToString2dp)
        .withValueFromStringFunction(stringToFloat)));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ "midSideBalance", 1 },
        "Mid/Side Balance",
        juce::NormalisableRange<float>(-1.0f, 1.0f, 0.01f),
        0.0f,
        juce::AudioParameterFloatAttributes()
        .withStringFromValueFunction(floatToString2dp)
        .withValueFromStringFunction(stringToFloat)));

    params.push_back(std::make_unique<juce::AudioParameterBool>("mono", "Mono", false));

	params.push_back(std::make_unique<juce::AudioParameterFloat>(
		juce::ParameterID{ "intensity", 1 },
		"Intensity",
		juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f),
		0.5f,
		juce::AudioParameterFloatAttributes()
		.withStringFromValueFunction(floatToString2dp)
		.withValueFromStringFunction(stringToFloat)));

    // TiltEQ
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ "tiltEQ", 1 },
        "Tilt EQ",
        juce::NormalisableRange<float>(-1.0f, 1.0f, 0.01f),
        0.0f,
        juce::AudioParameterFloatAttributes()
        .withStringFromValueFunction(floatToString2dp)
        .withValueFromStringFunction(stringToFloat)));

    // ModDelay
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ "delayTime", 1 },
        "Delay Time",
        juce::NormalisableRange<float>(1.0f, 2000.0f, 0.1f),
        400.0f,
        juce::AudioParameterFloatAttributes()
        .withLabel("ms")
        .withStringFromValueFunction(floatToString2dp)
        .withValueFromStringFunction(stringToFloat)));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ "feedbackL", 1 },
        "Feedback L",
        juce::NormalisableRange<float>(0.0f, 0.95f),
        0.4f,
        juce::AudioParameterFloatAttributes()
        .withStringFromValueFunction(floatToString2dp)
        .withValueFromStringFunction(stringToFloat)));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ "feedbackR", 1 },
        "Feedback R",
        juce::NormalisableRange<float>(0.0f, 0.95f),
        0.4f,
        juce::AudioParameterFloatAttributes()
        .withStringFromValueFunction(floatToString2dp)
        .withValueFromStringFunction(stringToFloat)));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ "modMix", 1 },
        "Mod Mix",
        juce::NormalisableRange<float>(0.0f, 1.0f),
        0.5f,
        juce::AudioParameterFloatAttributes()
        .withStringFromValueFunction(floatToString2dp)
        .withValueFromStringFunction(stringToFloat)));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ "modDepth", 1 },
        "Mod Depth",
        juce::NormalisableRange<float>(0.0f, 10.0f),
        2.0f,
        juce::AudioParameterFloatAttributes()
        .withStringFromValueFunction(floatToString2dp)
        .withValueFromStringFunction(stringToFloat)));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ "modRate", 1 },
        "Mod Rate",
        juce::NormalisableRange<float>(0.01f, 10.0f),
        0.25f,
        juce::AudioParameterFloatAttributes()
        .withLabel("Hz")
        .withStringFromValueFunction(floatToString2dp)
        .withValueFromStringFunction(stringToFloat)));

    params.push_back(std::make_unique<juce::AudioParameterChoice>(
        "modulationType",
        "Modulation Type",
        juce::StringArray{ "Sine", "Triangle", "Square", "Sawtooth Up", "Sawtooth Down" },
        1  // Default index: 1 = "Sine"
    ));

    params.push_back(std::make_unique<juce::AudioParameterBool>("sync", "Sync", false));

    // SpatialFX Parameters**
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ "phaseOffsetL", 1 },
        "Phase L Offset",
        juce::NormalisableRange<float>(-0.1f, 0.1f, 0.001f),
        0.0f,
        juce::AudioParameterFloatAttributes()
        .withStringFromValueFunction(floatToString2dp)
        .withValueFromStringFunction(stringToFloat)));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ "phaseOffsetR", 1 },
        "Phase R Offset",
        juce::NormalisableRange<float>(-0.1f, 0.1f, 0.001f),
        0.0f,
        juce::AudioParameterFloatAttributes()
        .withStringFromValueFunction(floatToString2dp)
        .withValueFromStringFunction(stringToFloat)));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ "sfxModRateL", 1 },
        "SFX Rate L",
        juce::NormalisableRange<float>(0.01f, 10.0f, 0.01f),
        0.1f,
        juce::AudioParameterFloatAttributes().withStringFromValueFunction(floatToString2dp).withValueFromStringFunction(stringToFloat)));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ "sfxModRateR", 1 },
        "SFX Rate R",
        juce::NormalisableRange<float>(0.01f, 10.0f, 0.01f),
        0.1f,
        juce::AudioParameterFloatAttributes().withStringFromValueFunction(floatToString2dp).withValueFromStringFunction(stringToFloat)));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ "sfxModDepthL", 1 },
        "SFX Depth L",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f),
        0.5f,
        juce::AudioParameterFloatAttributes().withStringFromValueFunction(floatToString2dp).withValueFromStringFunction(stringToFloat)));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ "sfxModDepthR", 1 },
        "SFX Depth R",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f),
        0.5f,
        juce::AudioParameterFloatAttributes().withStringFromValueFunction(floatToString2dp).withValueFromStringFunction(stringToFloat)));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ "sfxWetDryMix", 1 },
        "SFX Wet/Dry",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.5f),
        0.0f,
        juce::AudioParameterFloatAttributes()
        .withStringFromValueFunction(floatToString2dp)
        .withValueFromStringFunction(stringToFloat)));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ "sfxLfoPhaseOffset", 1 },
        "LFO Phase",
        juce::NormalisableRange<float>(0.0f, juce::MathConstants<float>::twoPi, 0.01f),
        juce::MathConstants<float>::halfPi,
        juce::AudioParameterFloatAttributes()
        .withStringFromValueFunction(floatToString2dp)
        .withValueFromStringFunction(stringToFloat)
    ));

	params.push_back(std::make_unique<juce::AudioParameterFloat>(
		juce::ParameterID{ "sfxAllpassFreq", 1 },
		"Allpass Freq",
		juce::NormalisableRange<float>(20.0f, 20000.0f, 1.0f),
		1000.0f,
		juce::AudioParameterFloatAttributes()
		.withStringFromValueFunction(floatToString2dp)
		.withValueFromStringFunction(stringToFloat)
	));

	params.push_back(std::make_unique<juce::AudioParameterFloat>(
		juce::ParameterID{ "haasDelayL", 1 },
		"Haas Delay L",
		juce::NormalisableRange<float>(0.0f, 40.0f, 0.1f),
		0.0f,
		juce::AudioParameterFloatAttributes()
		.withStringFromValueFunction(floatToString2dp)
		.withValueFromStringFunction(stringToFloat)
	));

	params.push_back(std::make_unique<juce::AudioParameterFloat>(
		juce::ParameterID{ "haasDelayR", 1 },
		"Haas Delay R",
		juce::NormalisableRange<float>(0.0f, 40.0f, 0.1f),
		0.0f,
		juce::AudioParameterFloatAttributes()
		.withStringFromValueFunction(floatToString2dp)
		.withValueFromStringFunction(stringToFloat)
	));

    params.push_back(std::make_unique<juce::AudioParameterChoice>(
        "modulationShape",
        "Modulation Shape",
        juce::StringArray{ "Sine", "Triangle", "Square", "Random" },
        1
    ));

	// MicroPitchDetune
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ "detuneAmount", 1 },
        "Detune Amount",
        juce::NormalisableRange<float>(-50.0f, 50.0f, 0.1f),
        0.0f,
        juce::AudioParameterFloatAttributes()
        .withStringFromValueFunction(floatToString2dp)
        .withValueFromStringFunction(stringToFloat)));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ "lfoRate", 1 },
        "LFO Rate",
        juce::NormalisableRange<float>(0.01f, 20.0f, 0.01f),
        0.3f,
        juce::AudioParameterFloatAttributes()
        .withStringFromValueFunction(floatToString2dp)
        .withValueFromStringFunction(stringToFloat)
    ));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ "lfoDepth", 1 },
        "LFO Depth",
        juce::NormalisableRange<float>(0.0f, 0.01f, 0.0001f),
        0.002f,
        juce::AudioParameterFloatAttributes()
        .withStringFromValueFunction(floatToString2dp)
        .withValueFromStringFunction(stringToFloat)
    ));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ "delayCentre", 1 },
        "Delay Centre",
        juce::NormalisableRange<float>(0.001f, 0.015f, 0.0001f),
        0.005f,
        juce::AudioParameterFloatAttributes()
        .withStringFromValueFunction(floatToString2dp)
        .withValueFromStringFunction(stringToFloat)
    ));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ "stereoSeparation", 1 },
        "Stereo Separation",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f),
        0.5f,
        juce::AudioParameterFloatAttributes()
        .withStringFromValueFunction(floatToString2dp)
        .withValueFromStringFunction(stringToFloat)
    ));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ "mix", 1 },
        "Mix",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f),
        0.5f,
        juce::AudioParameterFloatAttributes()
        .withStringFromValueFunction(floatToString2dp)
        .withValueFromStringFunction(stringToFloat)
    ));

    // ExciterSaturation
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ "exciterDrive", 1 },
        "Drive",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f),
        0.5f,
        juce::AudioParameterFloatAttributes()
        .withStringFromValueFunction(floatToString2dp)
        .withValueFromStringFunction(stringToFloat)
    ));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ "exciterMix", 1 },
        "Exciter Mix",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f),
        0.5f,
        juce::AudioParameterFloatAttributes()
        .withStringFromValueFunction(floatToString2dp)
        .withValueFromStringFunction(stringToFloat)
    ));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ "exciterHighpass", 1 },
        "Highpass Freq",
        juce::NormalisableRange<float>(100.0f, 8000.0f, 1.0f),
        1000.0f,
        juce::AudioParameterFloatAttributes()
        .withStringFromValueFunction(floatToString2dp)
        .withValueFromStringFunction(stringToFloat)
    ));

	// SimpleVerbWithPredelay
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ "predelayMs", 1 },
        "Pre-delay (ms)",
        juce::NormalisableRange<float>(0.0f, 100.0f, 0.1f),
        20.0f,
        juce::AudioParameterFloatAttributes()
        .withStringFromValueFunction(floatToString2dp)
        .withValueFromStringFunction(stringToFloat)
    ));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ "size", 1 },
        "Reverb Size",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f),
        0.5f,
        juce::AudioParameterFloatAttributes()
        .withStringFromValueFunction(floatToString2dp)
        .withValueFromStringFunction(stringToFloat)
    ));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ "damping", 1 },
        "Damping",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f),
        0.3f,
        juce::AudioParameterFloatAttributes()
        .withStringFromValueFunction(floatToString2dp)
        .withValueFromStringFunction(stringToFloat)
    ));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ "wet", 1 },
        "Wet Level",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f),
        0.5f,
        juce::AudioParameterFloatAttributes()
        .withStringFromValueFunction(floatToString2dp)
        .withValueFromStringFunction(stringToFloat)
    ));


    return { params.begin(), params.end() };
}


//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new AudioPluginAudioProcessor();
}