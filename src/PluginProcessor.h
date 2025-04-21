#pragma once

#include "WidthBalancer.h"
#include "TiltEQ.h"
#include "ModDelay.h"
#include "SpatialFX.h"

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>


//==============================================================================
class AudioPluginAudioProcessor final : public juce::AudioProcessor
{
public:
    //==============================================================================
    AudioPluginAudioProcessor();
    ~AudioPluginAudioProcessor() override;

    //==============================================================================
    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;

    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    using AudioProcessor::processBlock;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram(int index) override;
    const juce::String getProgramName(int index) override;
    void changeProgramName(int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    // Parameter setters to control the SpatialFX module
    void setPhaseOffsetParameter(float leftOffset, float rightOffset);
    void setModulationRateParameter(float rate);
    void setModulationDepthParameter(float depth);

    juce::AudioProcessorValueTreeState parameters;

    //==============================================================================
    WidthBalancer widthBalancer;
    TiltEQ tiltEQ;
    ModDelay modDelay;
    SpatialFX spatialFX;

private:
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    juce::dsp::AudioBlock<float> tempBlock;
    juce::AudioBuffer<float> dryBuffer;

    juce::dsp::ProcessSpec spec;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AudioPluginAudioProcessor)
};