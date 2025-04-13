#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
AudioPluginAudioProcessorEditor::AudioPluginAudioProcessorEditor (AudioPluginAudioProcessor& p)
    : AudioProcessorEditor (&p), processorRef (p)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (600, 600);

    WidthBalancerGUI();

    TiltEQGUI();

}

void AudioPluginAudioProcessorEditor::TiltEQGUI()
{
    tiltSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    tiltSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 20);
    tiltSlider.setRange(-1.0, 1.0, 0.01);
    tiltSlider.setSkewFactorFromMidPoint(0.0); // "Flat" in the center
    addAndMakeVisible(tiltSlider);

    tiltTextBox.setText("Tilt");
    tiltTextBox.setReadOnly(true);
    tiltTextBox.setColour(juce::TextEditor::backgroundColourId, juce::Colours::transparentWhite);
    tiltTextBox.setColour(juce::TextEditor::outlineColourId, juce::Colours::transparentWhite); // Remove the outline
    tiltTextBox.setColour(juce::TextEditor::textColourId, juce::Colours::black);
    tiltTextBox.setJustification(juce::Justification::centred); // Center text
    addAndMakeVisible(tiltTextBox);

    tiltAttachment = new juce::AudioProcessorValueTreeState::SliderAttachment(processorRef.parameters, "tiltEQ", tiltSlider);
}

void AudioPluginAudioProcessorEditor::WidthBalancerGUI()
{
    widthSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    widthSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
    addAndMakeVisible(widthSlider);

    midSideSlider.setSliderStyle(juce::Slider::LinearBar);
    midSideSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
    addAndMakeVisible(midSideSlider);

    monoToggle.setButtonText("Mono");
    addAndMakeVisible(monoToggle);

    widthTextBox.setText("Width");
    widthTextBox.setReadOnly(true); // Prevent the user from editing the text
    widthTextBox.setColour(juce::TextEditor::backgroundColourId, juce::Colours::transparentWhite);
    widthTextBox.setColour(juce::TextEditor::outlineColourId, juce::Colours::transparentWhite); // Remove the outline
    widthTextBox.setColour(juce::TextEditor::textColourId, juce::Colours::black); // Set the text color
    widthTextBox.setJustification(juce::Justification::centred); // Center text
    addAndMakeVisible(widthTextBox);

    midSideTextBox.setText("Mid-Side");
    midSideTextBox.setReadOnly(true);
    midSideTextBox.setColour(juce::TextEditor::backgroundColourId, juce::Colours::transparentWhite);
    midSideTextBox.setColour(juce::TextEditor::outlineColourId, juce::Colours::transparentWhite); // Remove the outline
    midSideTextBox.setColour(juce::TextEditor::textColourId, juce::Colours::black);
    addAndMakeVisible(midSideTextBox);

    widthAttachment = new juce::AudioProcessorValueTreeState::SliderAttachment(processorRef.parameters, "width", widthSlider);

    midSideAttachment = new juce::AudioProcessorValueTreeState::SliderAttachment(processorRef.parameters, "midSideBalance", midSideSlider);

    monoAttachment = new juce::AudioProcessorValueTreeState::ButtonAttachment(processorRef.parameters, "mono", monoToggle);
}

AudioPluginAudioProcessorEditor::~AudioPluginAudioProcessorEditor()
{
}

//==============================================================================
void AudioPluginAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    //g.fillAll(juce::Colours::black);
    g.setColour(juce::Colours::white);
    g.setFont(15.0f);

    auto& processor = processorRef;

    /*g.drawText("Tilt: " + juce::String(processor.getCurrentTilt(), 2),
        10, 20, getWidth(), 20, juce::Justification::left);

    g.drawText("Delay: " + juce::String(processor.getCurrentDelayMs(), 1) + " ms",
        10, 40, getWidth(), 20, juce::Justification::left);

    g.drawText("Stereo Width: " + juce::String(processor.getCurrentWidth(), 2),
        10, 60, getWidth(), 20, juce::Justification::left);*/
}

void AudioPluginAudioProcessorEditor::resized()
{
    WidthBalancerResized();

    // TiltEQ
    tiltSlider.setBounds(210, 100, knobSize, knobSize);

    tiltTextBox.setBounds(tiltSlider.getX() + tiltSlider.getWidth() / 3, tiltSlider.getY() + 150, 60, 20);
    tiltTextBox.toFront(false);
}

void AudioPluginAudioProcessorEditor::WidthBalancerResized()
{
    // Place the widthSlider in the left corner, 200x200 pixels
    widthSlider.setBounds(0, 0, knobSize, knobSize);

    // Place the midSideSlider next to it on the right
    midSideSlider.setBounds(210, 40, getWidth() - 220, 20); // 10 pixels gap, 20 pixels high

    // Place the monoToggle below the midSideSlider
    monoToggle.setBounds(210, 70, getWidth() - 220, 50); // 10 pixels gap, 50 pixels high

    // Position the text boxes
    widthTextBox.setBounds(widthSlider.getX() + widthSlider.getWidth() / 3, widthSlider.getY() + 150, 60, 20);
    midSideTextBox.setBounds(midSideSlider.getX(), midSideSlider.getY() - 20, 80, 20);

    // Make sure the text boxes are in front of the sliders.
    widthTextBox.toFront(false);
    midSideTextBox.toFront(false);

}
