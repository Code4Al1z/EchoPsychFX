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
    parameters(*this, nullptr, "PARAMETERS", createParameterLayout())
{
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
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int AudioPluginAudioProcessor::getCurrentProgram()
{
    return 0;
}

void AudioPluginAudioProcessor::setCurrentProgram (int index)
{
    juce::ignoreUnused (index);
}

const juce::String AudioPluginAudioProcessor::getProgramName (int index)
{
    juce::ignoreUnused (index);
    return {};
}

void AudioPluginAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
    juce::ignoreUnused (index, newName);
}

//==============================================================================
void AudioPluginAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = samplesPerBlock;
    spec.numChannels = getTotalNumOutputChannels();

    widthBalancer.prepare(spec);
    tiltEQ.prepare(spec);
    modDelay.prepare(spec);

    dryBuffer.setSize(spec.numChannels, spec.maximumBlockSize);

}

void AudioPluginAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

bool AudioPluginAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
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
    float feedback = *parameters.getRawParameterValue("feedback");
    float mixValue = *parameters.getRawParameterValue("mix");

    // Set delay parameters
    modDelay.setParams(delayTime, depth, rate, feedback, mixValue);

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
}



//==============================================================================
bool AudioPluginAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* AudioPluginAudioProcessor::createEditor()
{
    return new AudioPluginAudioProcessorEditor (*this);
}

//==============================================================================
void AudioPluginAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    juce::MemoryOutputStream stream(destData, true);
    parameters.state.writeToStream(stream);
}

void AudioPluginAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    juce::ValueTree tree = juce::ValueTree::readFromData(data, sizeInBytes);

    if (tree.isValid())
        parameters.state = tree;
}

juce::AudioProcessorValueTreeState::ParameterLayout AudioPluginAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    // WidthBalancer
    params.push_back(std::make_unique<juce::AudioParameterFloat>("width", "Width", juce::NormalisableRange<float>(0.0f, 2.0f, 0.01f), 1.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("midSideBalance", "Mid/Side Balance", juce::NormalisableRange<float>(-1.0f, 1.0f, 0.01f), 0.0f));
    params.push_back(std::make_unique<juce::AudioParameterBool>("mono", "Mono", false));

    // TiltEQ
    params.push_back(std::make_unique<juce::AudioParameterFloat>("tiltEQ", "Tilt EQ", juce::NormalisableRange<float>(-1.0f, 1.0f, 0.01f), 0.0f));

	// ModDelay
    params.push_back(std::make_unique<juce::AudioParameterFloat>("delayTime", "Delay Time", 1.0f, 2000.0f, 400.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("feedback", "Feedback", 0.0f, 0.95f, 0.4f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("mix", "Mix", 0.0f, 1.0f, 0.5f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("modDepth", "Mod Depth", 0.0f, 10.0f, 2.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("modRate", "Mod Rate", 0.01f, 10.0f, 0.25f));
    params.push_back(std::make_unique<juce::AudioParameterBool>("sync", "Sync", false));

    return { params.begin(), params.end() };
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new AudioPluginAudioProcessor();
}
