#include "PerceptionModeComponent.h"

PerceptionModeComponent::PerceptionModeComponent()
{
    titleLabel.setText("Perception Mode", juce::dontSendNotification);
    titleLabel.setFont(juce::Font(20.0f, juce::Font::bold));
    titleLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(titleLabel);
}

PerceptionModeComponent::~PerceptionModeComponent()
{
}

void PerceptionModeComponent::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::darkslategrey);
    g.setColour(juce::Colours::white);
    g.setFont(14.0f);
    g.drawFittedText("This is the Perception Mode UI area.", getLocalBounds().reduced(10), juce::Justification::centred, 3);
}

void PerceptionModeComponent::resized()
{
    titleLabel.setBounds(10, 10, getWidth() - 20, 30);
}

