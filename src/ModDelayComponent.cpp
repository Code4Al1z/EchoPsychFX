#include "ModDelayComponent.h"

ModDelayComponent::ModDelayComponent(juce::AudioProcessorValueTreeState& state)
{
    configureKnob(delayTime, "delayTime", "Delay", state);
    configureKnob(feedbackL, "feedbackL", "FB Left", state);
    configureKnob(feedbackR, "feedbackR", "FB Right", state);
    configureKnob(mix, "modMix", "Mod Mix", state);
    configureKnob(modDepth, "modDepth", "Depth", state);
    configureKnob(modRate, "modRate", "Rate", state);

    // Modulation Type ComboBox
    modulationTypeComboBox.addItem("Sine", static_cast<int>(ModDelay::ModulationType::Sine));
    modulationTypeComboBox.addItem("Triangle", static_cast<int>(ModDelay::ModulationType::Triangle));
    modulationTypeComboBox.addItem("Square", static_cast<int>(ModDelay::ModulationType::Square));
    modulationTypeComboBox.addItem("Saw Up", static_cast<int>(ModDelay::ModulationType::SawtoothUp));
    modulationTypeComboBox.addItem("Saw Down", static_cast<int>(ModDelay::ModulationType::SawtoothDown));
    modulationTypeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(state, "modulationType", modulationTypeComboBox);
    addAndMakeVisible(modulationTypeComboBox);

    modTypeLabel.setText("Mod Type");
    modTypeLabel.setReadOnly(true);
    modTypeLabel.setColour(juce::TextEditor::backgroundColourId, juce::Colours::transparentWhite);
    modTypeLabel.setColour(juce::TextEditor::outlineColourId, juce::Colours::transparentWhite);
    modTypeLabel.setColour(juce::TextEditor::textColourId, juce::Colours::black);
    modTypeLabel.setJustification(juce::Justification::centred);
    addAndMakeVisible(modTypeLabel);

    // Sync Toggle
    syncToggle.setButtonText("Sync");
    syncAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(state, "sync", syncToggle);
    addAndMakeVisible(syncToggle);
}

void ModDelayComponent::configureKnob(KnobWithLabel& kwl, const juce::String& paramID, const juce::String& labelText, juce::AudioProcessorValueTreeState& state)
{
    kwl.slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    kwl.slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 20);
    addAndMakeVisible(kwl.slider);

    kwl.label.setText(labelText);
    kwl.label.setReadOnly(true);
    kwl.label.setColour(juce::TextEditor::backgroundColourId, juce::Colours::transparentWhite);
    kwl.label.setColour(juce::TextEditor::outlineColourId, juce::Colours::transparentWhite);
    kwl.label.setColour(juce::TextEditor::textColourId, juce::Colours::black);
    kwl.label.setJustification(juce::Justification::centred);
    addAndMakeVisible(kwl.label);

    kwl.attachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(state, paramID, kwl.slider);
}

void ModDelayComponent::resized()
{
    auto area = getLocalBounds().reduced(margin);
    int numKnobs = 6;

    int textSpacing = 6;

    int totalWidth = (knobSize * numKnobs) + (spacing * (numKnobs - 1));
    int startX = area.getX() + (area.getWidth() - totalWidth) / 2;
    int y = area.getY() + 40; // Leave top for combo/sync

    // --- Stack syncToggle and combo box vertically on the left ---
    auto controlColumn = area.removeFromLeft(knobSize);

    // Sync toggle on top
    syncToggle.setBounds(controlColumn.removeFromTop(30));
    controlColumn.removeFromTop(textSpacing);

    // Combo box under it
    modulationTypeComboBox.setBounds(controlColumn.removeFromTop(30)); // dropdown height ~30
    controlColumn.removeFromTop(textSpacing);
    modTypeLabel.setBounds(controlColumn.removeFromTop(labelHeight));
    area.removeFromLeft(margin); // spacing before knobs

    KnobWithLabel* knobs[] = { &delayTime, &feedbackL, &feedbackR, &mix, &modDepth, &modRate };

    for (int i = 0; i < numKnobs; ++i)
    {
        int x = startX + i * (knobSize + spacing);
        knobs[i]->slider.setBounds(x, y, knobSize, knobSize);
        knobs[i]->label.setBounds(x, y - labelHeight, knobSize, labelHeight);
    }
}

// Parameter setters
void ModDelayComponent::setModulationType(ModDelay::ModulationType type)
{
    modulationTypeComboBox.setSelectedId(static_cast<int>(type));
}

void ModDelayComponent::setDelayTime(float v) { delayTime.slider.setValue(v); }
void ModDelayComponent::setFeedbackLeft(float v) { feedbackL.slider.setValue(v); }
void ModDelayComponent::setFeedbackRight(float v) { feedbackR.slider.setValue(v); }
void ModDelayComponent::setMix(float v) { mix.slider.setValue(v); }
void ModDelayComponent::setModDepth(float v) { modDepth.slider.setValue(v); }
void ModDelayComponent::setModRate(float v) { modRate.slider.setValue(v); }
