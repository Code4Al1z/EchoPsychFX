#include "PerceptionModeComponent.h"

PerceptionModeComponent::PerceptionModeComponent(PerceptionPresetManager& presetManager)
    : presetManagerRef(presetManager)
{
    titleLabel.setText("Perception Mode", juce::dontSendNotification);
    titleLabel.setFont(juce::Font(20.0f, juce::Font::bold));
    titleLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(titleLabel);

    //presetSelector.addItem("Select a Mode...", 1);  // ID 1 (just a placeholder)
    presetSelector.addItem("Head Trip", 1);
    presetSelector.addItem("Panic Room", 2);
    presetSelector.addItem("Intimacy", 3);
    presetSelector.addItem("Blade Runner", 4);
    presetSelector.addItem("Alien Abduction", 5);
    presetSelector.addItem("Glass Tunnel", 6);
    presetSelector.addItem("Dream Logic", 7);
    presetSelector.addItem("Womb Space", 8);
    presetSelector.addItem("Bipolar Bloom", 9);
    presetSelector.addItem("Quiet Confidence", 10);
    presetSelector.addItem("Falling Upwards", 11);
    presetSelector.onChange = [this]() {
        comboBoxChanged(&presetSelector);
        };
    addAndMakeVisible(presetSelector);
}

PerceptionModeComponent::~PerceptionModeComponent()
{
}

//void PerceptionModeComponent::paint(juce::Graphics& g)
//{
//    g.fillAll(juce::Colours::darkslategrey);
//    g.setColour(juce::Colours::white);
//    g.setFont(14.0f);
//    g.drawFittedText("This is the Perception Mode UI area.", getLocalBounds().reduced(10), juce::Justification::centred, 3);
//}

void PerceptionModeComponent::resized()
{
    titleLabel.setBounds(10, 10, getWidth() - 20, 30);
    presetSelector.setBounds(10, 50, getWidth() - 20, 30);
}

void PerceptionModeComponent::comboBoxChanged(juce::ComboBox* comboBoxThatHasChanged)
{
    if (comboBoxThatHasChanged == &presetSelector)
    {
        const auto selectedName = presetSelector.getText();
        DBG("Selected preset name: " + selectedName);
        //if (selectedName != "Select a Mode...")
            presetManagerRef.applyPreset(selectedName);
    }
}
