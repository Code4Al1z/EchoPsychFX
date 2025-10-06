#ifndef ECHOPSYCHFX_PLUGINPROCESSOR_H_INCLUDED
#define ECHOPSYCHFX_PLUGINPROCESSOR_H_INCLUDED

#include "WidthBalancer.h"
#include "TiltEQ.h"
#include "ModDelay.h"
#include "SpatialFX.h"
#include "MicroPitchDetune.h"
#include "ExciterSaturation.h"
#include "SimpleVerbWithPredelay.h"

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>

/**
 * @brief Main audio processor for the psychoacoustic effects plugin
 *
 * Implements a chain of audio effects designed for psychoacoustic manipulation:
 * - TiltEQ: Spectral tilt control
 * - WidthBalancer: Stereo width and mid-side processing
 * - ModDelay: Modulated delay effects
 * - SpatialFX: Spatial positioning and phase manipulation
 * - MicroPitchDetune: Subtle pitch shifting for thickness
 * - ExciterSaturation: Harmonic enhancement
 * - SimpleVerbWithPredelay: Reverb with pre-delay
 */
class AudioPluginAudioProcessor final : public juce::AudioProcessor
{
public:
    AudioPluginAudioProcessor();
    ~AudioPluginAudioProcessor() override;

    //==============================================================================
    // Plugin lifecycle
    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;

    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    using AudioProcessor::processBlock;

    //==============================================================================
    // Editor
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    // Plugin info
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    // Program management
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram(int index) override;
    const juce::String getProgramName(int index) override;
    void changeProgramName(int index, const juce::String& newName) override;

    //==============================================================================
    // State management
    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    //==============================================================================
    // Public members
    juce::AudioProcessorValueTreeState parameters;

    // Effect processors
    WidthBalancer widthBalancer;
    TiltEQ tiltEQ;
    ModDelay modDelay;
    SpatialFX spatialFX;
    MicroPitchDetune microPitchDetune;
    ExciterSaturation exciterSaturation;
    SimpleVerbWithPredelay simpleVerbWithPredelay;

private:
    //==============================================================================
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    // Processing state
    juce::AudioBuffer<float> dryBuffer;
    double bpm = 120.0;
    juce::dsp::ProcessSpec spec;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AudioPluginAudioProcessor)
};

#endif // ECHOPSYCHFX_PLUGINPROCESSOR_H_INCLUDED
