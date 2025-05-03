#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
AudioPluginAudioProcessorEditor::AudioPluginAudioProcessorEditor(AudioPluginAudioProcessor& p)
    : AudioProcessorEditor(&p), processorRef(p)
{
    widthBalancerComponent = std::make_unique<WidthBalancerComponent>(p.parameters);
    tiltEQComponent = std::make_unique<TiltEQComponent>(p.parameters);
    modDelayComponent = std::make_unique<ModDelayComponent>(p.parameters);
    spatialFXComponent = std::make_unique<SpatialFXComponent>(p.parameters);
    perceptionModeComponent = std::make_unique<PerceptionModeComponent>();

    addAndMakeVisible(*widthBalancerComponent);
    addAndMakeVisible(*tiltEQComponent);
    addAndMakeVisible(*modDelayComponent);
    addAndMakeVisible(*spatialFXComponent);
    addAndMakeVisible(*perceptionModeComponent);
    perceptionModeComponent->setVisible(false);

    addAndMakeVisible(modeToggle);
    modeToggle.setButtonText("Show Perception Modes");
    modeToggle.onClick = [this]() {
        updateUIVisibility();
        };

    setSize(900, 600);
}

AudioPluginAudioProcessorEditor::~AudioPluginAudioProcessorEditor()
{
}

void AudioPluginAudioProcessorEditor::updateUIVisibility()
{
    bool perceptionMode = modeToggle.getToggleState();

    widthBalancerComponent->setVisible(!perceptionMode);
    tiltEQComponent->setVisible(!perceptionMode);
    modDelayComponent->setVisible(!perceptionMode);
    spatialFXComponent->setVisible(!perceptionMode);

    perceptionModeComponent->setVisible(perceptionMode);
}

void AudioPluginAudioProcessorEditor::comboBoxChanged(juce::ComboBox* comboBoxThatHasChanged)
{
    if (comboBoxThatHasChanged == &modulationTypeComboBox)
    {
        int selectedId = modulationTypeComboBox.getSelectedId();
        DBG("Modulation type changed to ID: " << selectedId);
    }
}

//==============================================================================
void AudioPluginAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
    g.setColour(juce::Colours::white);
    g.setFont(15.0f);
}

void AudioPluginAudioProcessorEditor::resized()
{
    auto bounds = getLocalBounds().reduced(10);
    int toggleHeight = 30;
    int spacing = 10;

    modeToggle.setBounds(bounds.removeFromTop(toggleHeight));
    bounds.removeFromTop(spacing);

    if (perceptionModeComponent->isVisible())
    {
        perceptionModeComponent->setBounds(bounds);
    }
    else
    {
        int componentHeight = (bounds.getHeight() - 3 * spacing) / 4;

        widthBalancerComponent->setBounds(bounds.removeFromTop(componentHeight));
        bounds.removeFromTop(spacing);

        tiltEQComponent->setBounds(bounds.removeFromTop(componentHeight));
        bounds.removeFromTop(spacing);

        modDelayComponent->setBounds(bounds.removeFromTop(componentHeight));
        bounds.removeFromTop(spacing);

        spatialFXComponent->setBounds(bounds);
    }
}
