#include "PluginProcessor.h"
#include "ModDelayComponent.h"

ModDelayComponent::ModDelayComponent(juce::AudioProcessorValueTreeState& state)
{
    // Delay Time
    addAndMakeVisible(delayTimeSlider);
    delayTimeSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    delayTimeSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    delayAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(state, "delayTime", delayTimeSlider);

    delayTimeTextBox.setText("Delay Time", false);
    delayTimeTextBox.setJustification(juce::Justification::centred);
    delayTimeTextBox.setColour(juce::TextEditor::backgroundColourId, juce::Colours::transparentBlack);
    delayTimeTextBox.setColour(juce::TextEditor::outlineColourId, juce::Colours::transparentBlack);
    addAndMakeVisible(delayTimeTextBox);

    // Feedback Left
    addAndMakeVisible(feedbackLSlider);
    feedbackLSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    feedbackLSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    feedbackLAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(state, "feedbackL", feedbackLSlider);

    feedbackLTextBox.setText("Feedback L", false);
    feedbackLTextBox.setJustification(juce::Justification::centred);
    feedbackLTextBox.setColour(juce::TextEditor::backgroundColourId, juce::Colours::transparentBlack);
    feedbackLTextBox.setColour(juce::TextEditor::outlineColourId, juce::Colours::transparentBlack);
    addAndMakeVisible(feedbackLTextBox);

    // Feedback Right
    addAndMakeVisible(feedbackRSlider);
    feedbackRSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    feedbackRSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    feedbackRAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(state, "feedbackR", feedbackRSlider);

    feedbackRTextBox.setText("Feedback R", false);
    feedbackRTextBox.setJustification(juce::Justification::centred);
    feedbackRTextBox.setColour(juce::TextEditor::backgroundColourId, juce::Colours::transparentBlack);
    feedbackRTextBox.setColour(juce::TextEditor::outlineColourId, juce::Colours::transparentBlack);
    addAndMakeVisible(feedbackRTextBox);

    // Mix
    addAndMakeVisible(mixSlider);
    mixSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    mixSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    mixAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(state, "mix", mixSlider);

    mixTextBox.setText("Mix", false);
    mixTextBox.setJustification(juce::Justification::centred);
    mixTextBox.setColour(juce::TextEditor::backgroundColourId, juce::Colours::transparentBlack);
    mixTextBox.setColour(juce::TextEditor::outlineColourId, juce::Colours::transparentBlack);
    addAndMakeVisible(mixTextBox);

    // Mod Depth
    addAndMakeVisible(modDepthSlider);
    modDepthSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    modDepthSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    depthAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(state, "modDepth", modDepthSlider);

    modDepthTextBox.setText("Mod Depth", false);
    modDepthTextBox.setJustification(juce::Justification::centred);
    modDepthTextBox.setColour(juce::TextEditor::backgroundColourId, juce::Colours::transparentBlack);
    modDepthTextBox.setColour(juce::TextEditor::outlineColourId, juce::Colours::transparentBlack);
    addAndMakeVisible(modDepthTextBox);

    // Mod Rate
    addAndMakeVisible(modRateSlider);
    modRateSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    modRateSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    rateAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(state, "modRate", modRateSlider);

    modRateTextBox.setText("Mod Rate", false);
    modRateTextBox.setJustification(juce::Justification::centred);
    modRateTextBox.setColour(juce::TextEditor::backgroundColourId, juce::Colours::transparentBlack);
    modRateTextBox.setColour(juce::TextEditor::outlineColourId, juce::Colours::transparentBlack);
    addAndMakeVisible(modRateTextBox);

    // Modulation Type
    addAndMakeVisible(modulationTypeComboBox);
    modulationTypeComboBox.addItem("Sine", 1);
    modulationTypeComboBox.addItem("Triangle", 2);
    modulationTypeComboBox.addItem("Square", 3);
    modulationTypeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(state, "modulationType", modulationTypeComboBox);

    auto modTypeValue = state.getRawParameterValue("modulationType");
    if (modTypeValue != nullptr)
        modulationTypeComboBox.setSelectedId(static_cast<int>(modTypeValue->load()) + 1, juce::dontSendNotification);

    modulationTypeTextBox.setText("Mod Type", false);
    modulationTypeTextBox.setJustification(juce::Justification::centred);
    modulationTypeTextBox.setColour(juce::TextEditor::backgroundColourId, juce::Colours::transparentBlack);
    modulationTypeTextBox.setColour(juce::TextEditor::outlineColourId, juce::Colours::transparentBlack);
    addAndMakeVisible(modulationTypeTextBox);

    modulationTypeComboBox.addListener(this);

    // Sync Toggle
    addAndMakeVisible(syncToggle);
    syncToggle.setButtonText("Sync");
    syncAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(state, "sync", syncToggle);
}

ModDelayComponent::~ModDelayComponent()
{
    modulationTypeComboBox.removeListener(this);
}

void ModDelayComponent::resized()
{
    auto area = getLocalBounds().reduced(margin);
    int labelHeight = 24;
    int textSpacing = 6;

    // If this should be placed to the right of something, reserve that space
    int shiftRight = knobSize + margin * 2;
    area.removeFromLeft(shiftRight); // leave space for TiltEQ

    auto placeKnob = [this, labelHeight, textSpacing](juce::Rectangle<int>& bounds, juce::Component& knob, juce::Component& label)
        {
            knob.setBounds(bounds.removeFromTop(knobSize));
            bounds.removeFromTop(textSpacing);
            label.setBounds(bounds.removeFromTop(labelHeight));
        };

    auto topRow = area.removeFromTop(knobSize + labelHeight + textSpacing + margin);
    auto bottomRow = area.removeFromTop(knobSize + labelHeight + textSpacing);

    auto colWidth = knobSize + margin;

    auto knobArea = topRow.removeFromLeft(colWidth);
    placeKnob(knobArea, delayTimeSlider, delayTimeTextBox);

    knobArea = topRow.removeFromLeft(colWidth);
    placeKnob(knobArea, feedbackLSlider, feedbackLTextBox);

    knobArea = topRow.removeFromLeft(colWidth);
    placeKnob(knobArea, feedbackRSlider, feedbackRTextBox);

    knobArea = topRow.removeFromLeft(colWidth);
    placeKnob(knobArea, mixSlider, mixTextBox);

    knobArea = bottomRow.removeFromLeft(colWidth);
    placeKnob(knobArea, modDepthSlider, modDepthTextBox);

    knobArea = bottomRow.removeFromLeft(colWidth);
    placeKnob(knobArea, modRateSlider, modRateTextBox);

    auto comboArea = bottomRow.removeFromLeft(knobSize + margin);
    modulationTypeComboBox.setBounds(comboArea.removeFromTop(labelHeight));
    modulationTypeTextBox.setBounds(comboArea.removeFromTop(labelHeight));

    syncToggle.setBounds(bottomRow.removeFromLeft(knobSize / 2));
}


void ModDelayComponent::comboBoxChanged(juce::ComboBox* comboBoxThatHasChanged)
{
    // Placeholder if you want to respond to UI selection
    if (comboBoxThatHasChanged == &modulationTypeComboBox)
    {
        // You can print or use this info if needed
        DBG("Modulation type changed to ID: " << modulationTypeComboBox.getSelectedId());
    }
}
