#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
AudioPluginAudioProcessorEditor::AudioPluginAudioProcessorEditor (AudioPluginAudioProcessor& p)
    : AudioProcessorEditor (&p), processorRef (p)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (400, 300);

    bitReductionSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    bitReductionSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 60, 20);
    addAndMakeVisible(bitReductionSlider);

    bitReductionAttachment = new juce::AudioProcessorValueTreeState::SliderAttachment(processorRef.parameters, "bitReduction", bitReductionSlider);
}

AudioPluginAudioProcessorEditor::~AudioPluginAudioProcessorEditor()
{
}

//==============================================================================
void AudioPluginAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    //g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    g.fillAll(juce::Colours::black);
    g.setColour(juce::Colours::white);
    g.setFont(15.0f);

    auto& processor = processorRef;

    g.drawText("Tilt: " + juce::String(processor.getCurrentTilt(), 2),
        10, 20, getWidth(), 20, juce::Justification::left);

    g.drawText("Delay: " + juce::String(processor.getCurrentDelayMs(), 1) + " ms",
        10, 40, getWidth(), 20, juce::Justification::left);

    g.drawText("Stereo Width: " + juce::String(processor.getCurrentWidth(), 2),
        10, 60, getWidth(), 20, juce::Justification::left);
}

void AudioPluginAudioProcessorEditor::resized()
{
    bitReductionSlider.setBounds(10, 10, getWidth() - 20, 30);

}
