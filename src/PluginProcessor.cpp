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
    modDelay(), // Initialize the ModDelay object
    spatialFX()
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

    widthBalancer.prepare(spec);
    tiltEQ.prepare(spec);
    modDelay.prepare(spec);
    spatialFX.prepare(spec);

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

    // Read parameters from ValueTreeState
    float delayTime = *parameters.getRawParameterValue("delayTime");
    float depth = *parameters.getRawParameterValue("modDepth");
    float rate = *parameters.getRawParameterValue("modRate");
    float feedbackL = *parameters.getRawParameterValue("feedbackL"); // Get separate feedback values
    float feedbackR = *parameters.getRawParameterValue("feedbackR");
    float mixValue = *parameters.getRawParameterValue("mix");
    int modulationTypeValue = parameters.state.getProperty("modulationType"); // Get the raw value
    float phaseOffsetL = *parameters.getRawParameterValue("phaseOffsetL"); // **[HIGHLIGHT] Get Phase Offset L**
    float phaseOffsetR = *parameters.getRawParameterValue("phaseOffsetR"); // **[HIGHLIGHT] Get Phase Offset R**
    float sfxModRate = *parameters.getRawParameterValue("sfxModRate");   // **[HIGHLIGHT] Get SFX Mod Rate**
    float sfxModDepth = *parameters.getRawParameterValue("sfxModDepth");  // **[HIGHLIGHT] Get SFX Mod Depth**


    // Convert the raw value to the enum
    ModDelay::ModulationType modulationType = static_cast<ModDelay::ModulationType>(modulationTypeValue);
    modDelay.setModulationType(modulationType);

    // Set delay parameters
    modDelay.setParams(delayTime, depth, rate, feedbackL, feedbackR, mixValue);

    // Apply modulated delay effect
    modDelay.process(block);

    // Wet/dry mix using the dryBlock
    for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
    {
        auto* dry = dryBlock.getChannelPointer(ch);
        auto* wet = block.getChannelPointer(ch);

        for (int i = 0; i < buffer.getNumSamples(); ++i)
            wet[i] = wet[i] * mixValue + dry[i] * (1.0f - mixValue);
    }

    // Apply tilt EQ
    float tilt = *parameters.getRawParameterValue("tiltEQ");
    tiltEQ.setTilt(tilt);
    tiltEQ.process(block);

    // Apply stereo width and mono/stereo processing
    float width = *parameters.getRawParameterValue("width");
    float balance = *parameters.getRawParameterValue("midSideBalance");
    bool mono = parameters.getRawParameterValue("mono")->load();

    widthBalancer.setWidth(width);
    widthBalancer.setMidSideBalance(balance);
    widthBalancer.setMono(mono);

    // Final stage
    widthBalancer.process(block);

    // **[HIGHLIGHT] Apply SpatialFX processing**
    spatialFX.setPhaseOffset(phaseOffsetL, phaseOffsetR);
    spatialFX.setModulationRate(sfxModRate);
    spatialFX.setModulationDepth(sfxModDepth);
    spatialFX.process(block);
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
    stream.writeInt((int)modDelay.modulationType); // Save the modulation type
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

void AudioPluginAudioProcessor::setPhaseOffsetParameter(float leftOffset, float rightOffset)
{
    spatialFX.setPhaseOffset(leftOffset, rightOffset);
}

void AudioPluginAudioProcessor::setModulationRateParameter(float rate)
{
    spatialFX.setModulationRate(rate);
}

void AudioPluginAudioProcessor::setModulationDepthParameter(float depth)
{
    spatialFX.setModulationDepth(depth);
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
        juce::ParameterID{ "mix", 1 },
        "Mix",
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

    params.push_back(std::make_unique<juce::AudioParameterInt>(
        "modulationType", "Modulation Type",
        (int)ModDelay::ModulationType::Sine,
        (int)ModDelay::ModulationType::SawtoothDown,
        (int)ModDelay::ModulationType::Sine));

    params.push_back(std::make_unique<juce::AudioParameterBool>("sync", "Sync", false));

    // **[HIGHLIGHT] SpatialFX Parameters**
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
        juce::ParameterID{ "sfxModRate", 1 },
        "SFX Rate",
        juce::NormalisableRange<float>(0.01f, 10.0f, 0.01f),
        0.1f,
        juce::AudioParameterFloatAttributes()
        .withStringFromValueFunction(floatToString2dp)
        .withValueFromStringFunction(stringToFloat)));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ "sfxModDepth", 1 },
        "SFX Depth",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f),
        0.0f,
        juce::AudioParameterFloatAttributes()
        .withStringFromValueFunction(floatToString2dp)
        .withValueFromStringFunction(stringToFloat)));

    return { params.begin(), params.end() };
}


//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new AudioPluginAudioProcessor();
}