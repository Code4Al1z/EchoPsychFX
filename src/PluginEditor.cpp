#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
AudioPluginAudioProcessorEditor::AudioPluginAudioProcessorEditor (AudioPluginAudioProcessor& p)
    : AudioProcessorEditor (&p), processorRef (p)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (800, 600);

    WidthBalancerGUI();

    TiltEQGUI();

    ModDelayGUI();

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

void AudioPluginAudioProcessorEditor::ModDelayGUI()
{
    addAndMakeVisible(delayTimeSlider);
    delayTimeSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    delayTimeSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
    delayTimeSlider.setRange(1.0, 1000.0); // ms
    delayAttachment = new juce::AudioProcessorValueTreeState::SliderAttachment(processorRef.parameters, "delayTime", delayTimeSlider);

    addAndMakeVisible(feedbackSlider);
    feedbackSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    feedbackSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
    feedbackSlider.setRange(0.0, 0.95);
    feedbackAttachment = new juce::AudioProcessorValueTreeState::SliderAttachment(processorRef.parameters, "feedback", feedbackSlider);

    addAndMakeVisible(mixSlider);
    mixSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    mixSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
    mixSlider.setRange(0.0, 1.0);
    mixAttachment = new juce::AudioProcessorValueTreeState::SliderAttachment(processorRef.parameters, "mix", mixSlider);

    addAndMakeVisible(modDepthSlider);
    modDepthSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    modDepthSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
    modDepthSlider.setRange(0.0, 20.0); // percent or ms depending on interpretation
    depthAttachment = new juce::AudioProcessorValueTreeState::SliderAttachment(processorRef.parameters, "modDepth", modDepthSlider);

    addAndMakeVisible(modRateSlider);
    modRateSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    modRateSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
    modRateSlider.setRange(0.01, 10.0); // Hz
    rateAttachment = new juce::AudioProcessorValueTreeState::SliderAttachment(processorRef.parameters, "modRate", modRateSlider);

    addAndMakeVisible(syncToggle);
    syncToggle.setButtonText("Sync");
    syncAttachment = new juce::AudioProcessorValueTreeState::ButtonAttachment(processorRef.parameters, "syncToBPM", syncToggle);

	delayTimeTextBox.setText("Delay Time");
	delayTimeTextBox.setReadOnly(true);
	delayTimeTextBox.setColour(juce::TextEditor::backgroundColourId, juce::Colours::transparentWhite);
	delayTimeTextBox.setColour(juce::TextEditor::outlineColourId, juce::Colours::transparentWhite); // Remove the outline
	delayTimeTextBox.setColour(juce::TextEditor::textColourId, juce::Colours::black);
	delayTimeTextBox.setJustification(juce::Justification::centred); // Center text
	addAndMakeVisible(delayTimeTextBox);

	feedbackTextBox.setText("Feedback");
	feedbackTextBox.setReadOnly(true);
	feedbackTextBox.setColour(juce::TextEditor::backgroundColourId, juce::Colours::transparentWhite);
	feedbackTextBox.setColour(juce::TextEditor::outlineColourId, juce::Colours::transparentWhite); // Remove the outline
	feedbackTextBox.setColour(juce::TextEditor::textColourId, juce::Colours::black);
	feedbackTextBox.setJustification(juce::Justification::centred); // Center text
	addAndMakeVisible(feedbackTextBox);

	mixTextBox.setText("Mix");
	mixTextBox.setReadOnly(true);
	mixTextBox.setColour(juce::TextEditor::backgroundColourId, juce::Colours::transparentWhite);
	mixTextBox.setColour(juce::TextEditor::outlineColourId, juce::Colours::transparentWhite); // Remove the outline
	mixTextBox.setColour(juce::TextEditor::textColourId, juce::Colours::black);
	mixTextBox.setJustification(juce::Justification::centred); // Center text
	addAndMakeVisible(mixTextBox);

	modDepthTextBox.setText("Mod Depth");
	modDepthTextBox.setReadOnly(true);
	modDepthTextBox.setColour(juce::TextEditor::backgroundColourId, juce::Colours::transparentWhite);
	modDepthTextBox.setColour(juce::TextEditor::outlineColourId, juce::Colours::transparentWhite); // Remove the outline
	modDepthTextBox.setColour(juce::TextEditor::textColourId, juce::Colours::black);
	modDepthTextBox.setJustification(juce::Justification::centred); // Center text
    addAndMakeVisible(modDepthTextBox);

    modRateTextBox.setText("Mod Rate");
    modRateTextBox.setReadOnly(true);
    modRateTextBox.setColour(juce::TextEditor::backgroundColourId, juce::Colours::transparentWhite);
    modRateTextBox.setColour(juce::TextEditor::outlineColourId, juce::Colours::transparentWhite); // Remove the outline
    modRateTextBox.setColour(juce::TextEditor::textColourId, juce::Colours::black);
    modRateTextBox.setJustification(juce::Justification::centred); // Center text
    addAndMakeVisible(modRateTextBox);
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

}

void AudioPluginAudioProcessorEditor::resized()
{
    WidthBalancerResized();

    TiltEQResized();

    ModDelayResized();
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
    widthTextBox.setBounds(widthSlider.getX() + widthSlider.getWidth() / 3, widthSlider.getY() + 80, 60, 20);
    midSideTextBox.setBounds(midSideSlider.getX(), midSideSlider.getY() - 20, 80, 20);

    // Make sure the text boxes are in front of the sliders.
    widthTextBox.toFront(false);
    midSideTextBox.toFront(false);

}

void AudioPluginAudioProcessorEditor::TiltEQResized()
{
    tiltSlider.setBounds(210, 100, knobSize, knobSize);

    tiltTextBox.setBounds(tiltSlider.getX() + tiltSlider.getWidth() / 3, tiltSlider.getY() + 80, 60, 20);
    tiltTextBox.toFront(false);
}

void AudioPluginAudioProcessorEditor::ModDelayResized()
{
    int x = margin;

    delayTimeSlider.setBounds(x, 230, knobSize, knobSize); x += knobSize + margin;
    feedbackSlider.setBounds(x, 230, knobSize, knobSize); x += knobSize + margin;
    mixSlider.setBounds(x, 230, knobSize, knobSize); x += knobSize + margin;
    modDepthSlider.setBounds(x, 230, knobSize, knobSize); x += knobSize + margin;
    modRateSlider.setBounds(x, 230, knobSize, knobSize);

    syncToggle.setBounds(10, knobSize + 30, 100, 30);

	delayTimeTextBox.setBounds(delayTimeSlider.getX() + delayTimeSlider.getWidth() / 3, delayTimeSlider.getY() + 80, 70, 20);
	feedbackTextBox.setBounds(feedbackSlider.getX() + feedbackSlider.getWidth() / 3, feedbackSlider.getY() + 80, 70, 20);
	mixTextBox.setBounds(mixSlider.getX() + mixSlider.getWidth() / 3, mixSlider.getY() + 80, 70, 20);
	modDepthTextBox.setBounds(modDepthSlider.getX() + modDepthSlider.getWidth() / 3, modDepthSlider.getY() + 80, 70, 20);
	modRateTextBox.setBounds(modRateSlider.getX() + modRateSlider.getWidth() / 3, modRateSlider.getY() + 80, 70, 20);
}
