#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
AudioPluginAudioProcessorEditor::AudioPluginAudioProcessorEditor(AudioPluginAudioProcessor& p)
    : AudioProcessorEditor(&p), processorRef(p)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize(800, 600);

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
    delayAttachment = new juce::AudioProcessorValueTreeState::SliderAttachment(processorRef.parameters, "delayTime", delayTimeSlider);

    addAndMakeVisible(feedbackLSlider);
    feedbackLSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    feedbackLSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
    feedbackLAttachment = new juce::AudioProcessorValueTreeState::SliderAttachment(processorRef.parameters, "feedbackL", feedbackLSlider);

    addAndMakeVisible(feedbackRSlider);
    feedbackRSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    feedbackRSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
    feedbackRAttachment = new juce::AudioProcessorValueTreeState::SliderAttachment(processorRef.parameters, "feedbackR", feedbackRSlider);

    addAndMakeVisible(mixSlider);
    mixSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    mixSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
    mixAttachment = new juce::AudioProcessorValueTreeState::SliderAttachment(processorRef.parameters, "mix", mixSlider);

    addAndMakeVisible(modDepthSlider);
    modDepthSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    modDepthSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
    depthAttachment = new juce::AudioProcessorValueTreeState::SliderAttachment(processorRef.parameters, "modDepth", modDepthSlider);

    addAndMakeVisible(modRateSlider);
    modRateSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    modRateSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
    rateAttachment = new juce::AudioProcessorValueTreeState::SliderAttachment(processorRef.parameters, "modRate", modRateSlider);

    addAndMakeVisible(modulationTypeComboBox);
    modulationTypeComboBox.addItem("Sine", static_cast<int>(ModDelay::ModulationType::Sine) + 1);
    modulationTypeComboBox.addItem("Triangle", static_cast<int>(ModDelay::ModulationType::Triangle) + 1);
    modulationTypeComboBox.addItem("Square", static_cast<int>(ModDelay::ModulationType::Square) + 1);
    modulationTypeComboBox.addItem("Sawtooth Up", static_cast<int>(ModDelay::ModulationType::SawtoothUp) + 1);
    modulationTypeComboBox.addItem("Sawtooth Down", static_cast<int>(ModDelay::ModulationType::SawtoothDown) + 1);
    modulationTypeComboBox.setSelectedId(static_cast<int>(processorRef.modDelay.getModulationType()) + 1); // Set initial value
    modulationTypeComboBox.addListener(this);
    modulationTypeAttachment = new juce::AudioProcessorValueTreeState::ComboBoxAttachment(processorRef.parameters, "modulationType", modulationTypeComboBox);

    delayTimeTextBox.setText("Delay Time");
    delayTimeTextBox.setReadOnly(true);
    delayTimeTextBox.setColour(juce::TextEditor::backgroundColourId, juce::Colours::transparentWhite);
    delayTimeTextBox.setColour(juce::TextEditor::outlineColourId, juce::Colours::transparentWhite);
    delayTimeTextBox.setColour(juce::TextEditor::textColourId, juce::Colours::black);
    delayTimeTextBox.setJustification(juce::Justification::centred);
    addAndMakeVisible(delayTimeTextBox);

    feedbackLTextBox.setText("Feedback L");
    feedbackLTextBox.setReadOnly(true);
    feedbackLTextBox.setColour(juce::TextEditor::backgroundColourId, juce::Colours::transparentWhite);
    feedbackLTextBox.setColour(juce::TextEditor::outlineColourId, juce::Colours::transparentWhite);
    feedbackLTextBox.setColour(juce::TextEditor::textColourId, juce::Colours::black);
    feedbackLTextBox.setJustification(juce::Justification::centred);
    addAndMakeVisible(feedbackLTextBox);

    feedbackRTextBox.setText("Feedback R");
    feedbackRTextBox.setReadOnly(true);
    feedbackRTextBox.setColour(juce::TextEditor::backgroundColourId, juce::Colours::transparentWhite);
    feedbackRTextBox.setColour(juce::TextEditor::outlineColourId, juce::Colours::transparentWhite);
    feedbackRTextBox.setColour(juce::TextEditor::textColourId, juce::Colours::black);
    feedbackRTextBox.setJustification(juce::Justification::centred);
    addAndMakeVisible(feedbackRTextBox);

    mixTextBox.setText("Mix");
    mixTextBox.setReadOnly(true);
    mixTextBox.setColour(juce::TextEditor::backgroundColourId, juce::Colours::transparentWhite);
    mixTextBox.setColour(juce::TextEditor::outlineColourId, juce::Colours::transparentWhite);
    mixTextBox.setColour(juce::TextEditor::textColourId, juce::Colours::black);
    mixTextBox.setJustification(juce::Justification::centred);
    addAndMakeVisible(mixTextBox);

    modDepthTextBox.setText("Mod Depth");
    modDepthTextBox.setReadOnly(true);
    modDepthTextBox.setColour(juce::TextEditor::backgroundColourId, juce::Colours::transparentWhite);
    modDepthTextBox.setColour(juce::TextEditor::outlineColourId, juce::Colours::transparentWhite);
    modDepthTextBox.setColour(juce::TextEditor::textColourId, juce::Colours::black);
    modDepthTextBox.setJustification(juce::Justification::centred);
    addAndMakeVisible(modDepthTextBox);

    modRateTextBox.setText("Mod Rate");
    modRateTextBox.setReadOnly(true);
    modRateTextBox.setColour(juce::TextEditor::backgroundColourId, juce::Colours::transparentWhite);
    modRateTextBox.setColour(juce::TextEditor::outlineColourId, juce::Colours::transparentWhite);
    modRateTextBox.setColour(juce::TextEditor::textColourId, juce::Colours::black);
    modRateTextBox.setJustification(juce::Justification::centred);
    addAndMakeVisible(modRateTextBox);
}

AudioPluginAudioProcessorEditor::~AudioPluginAudioProcessorEditor()
{
}

//==============================================================================
void AudioPluginAudioProcessorEditor::paint(juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));

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
    feedbackLSlider.setBounds(x, 230, knobSize, knobSize); x += knobSize + margin;
    feedbackRSlider.setBounds(x, 230, knobSize, knobSize); x += knobSize + margin;
    mixSlider.setBounds(x, 230, knobSize, knobSize); x += knobSize + margin;
    modDepthSlider.setBounds(x, 230, knobSize, knobSize); x += knobSize + margin;
    modRateSlider.setBounds(x, 230, knobSize, knobSize);

    modulationTypeComboBox.setBounds(10, 40 + knobSize, 150, 30); // Position the combo box

    delayTimeTextBox.setBounds(delayTimeSlider.getX() + delayTimeSlider.getWidth() / 3, delayTimeSlider.getY() + 80, 70, 20);
    feedbackLTextBox.setBounds(feedbackLSlider.getX() + feedbackLSlider.getWidth() / 3, feedbackLSlider.getY() + 80, 70, 20);
    feedbackRTextBox.setBounds(feedbackRSlider.getX() + feedbackRSlider.getWidth() / 3, feedbackRSlider.getY() + 80, 70, 20);
    mixTextBox.setBounds(mixSlider.getX() + mixSlider.getWidth() / 3, mixSlider.getY() + 80, 70, 20);
    modDepthTextBox.setBounds(modDepthSlider.getX() + modDepthSlider.getWidth() / 3, modDepthSlider.getY() + 80, 70, 20);
    modRateTextBox.setBounds(modRateSlider.getX() + modRateSlider.getWidth() / 3, modRateSlider.getY() + 80, 70, 20); // Finish this line
    modRateTextBox.toFront(false); // Ensure it's in front

    delayTimeTextBox.toFront(false);
    feedbackLTextBox.toFront(false);
    feedbackRTextBox.toFront(false);
    mixTextBox.toFront(false);
    modDepthTextBox.toFront(false);
}

void AudioPluginAudioProcessorEditor::comboBoxChanged(juce::ComboBox* comboBoxThatHasChanged)
{
    if (comboBoxThatHasChanged == &modulationTypeComboBox)
    {
        if (modulationTypeComboBox.getSelectedId() > 0)
        {
            processorRef.parameters.getParameter("modulationType")->setValueNotifyingHost(
                static_cast<float>(modulationTypeComboBox.getSelectedId() - 1) / static_cast<float>(static_cast<int>(ModDelay::ModulationType::SawtoothDown))
            );
        }
    }
}
