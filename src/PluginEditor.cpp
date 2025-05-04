#include "PluginProcessor.h"
#include "PluginEditor.h"

AudioPluginAudioProcessorEditor::AudioPluginAudioProcessorEditor(AudioPluginAudioProcessor& p)
    : AudioProcessorEditor(&p), processorRef(p)
{
    widthBalancerComponent = std::make_unique<WidthBalancerComponent>(p.parameters);
    tiltEQComponent = std::make_unique<TiltEQComponent>(p.parameters);
    modDelayComponent = std::make_unique<ModDelayComponent>(p.parameters);
    spatialFXComponent = std::make_unique<SpatialFXComponent>(p.parameters);

    // Now that the above components are created, it's safe to pass them to the preset manager
    presetManager = std::make_unique<PerceptionPresetManager>(*tiltEQComponent, *widthBalancerComponent, *modDelayComponent, *spatialFXComponent);

    perceptionModeComponent = std::make_unique<PerceptionModeComponent>(*presetManager);

    addAndMakeVisible(*widthBalancerComponent);
    addAndMakeVisible(*tiltEQComponent);
    addAndMakeVisible(*modDelayComponent);
    addAndMakeVisible(*spatialFXComponent);
    addAndMakeVisible(*perceptionModeComponent);
    
    addAndMakeVisible(modeToggle);
    modeToggle.setButtonText("Show Perception Modes");
    modeToggle.onClick = [this]() {
        updateUIVisibility();
        };

    modeToggle.setToggleState(false, juce::dontSendNotification);
    setSize(900, 600);
    updateUIVisibility();
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
    int knobSize = 120;

    modeToggle.setBounds(bounds.removeFromTop(toggleHeight));
    bounds.removeFromTop(spacing);

    // Define the areas for both sets of components
    auto mainUIArea = bounds;
    auto perceptionModeArea = bounds; // Perception mode takes the whole remaining area

    auto topRowHeight = 150;
    auto topRow = mainUIArea.removeFromTop(topRowHeight);

    auto widthBalancerWidth = getWidth() * 0.6;
    auto widthBalancerArea = topRow.removeFromLeft(widthBalancerWidth);
    widthBalancerComponent->setBounds(widthBalancerArea);

    topRow.removeFromLeft(spacing);
    auto tiltEQArea = topRow;
    tiltEQComponent->setBounds(tiltEQArea);

    mainUIArea.removeFromTop(spacing);

    auto modDelayHeight = knobSize + 24 + 10 + 10;
    auto modDelayArea = mainUIArea.removeFromTop(modDelayHeight);
    modDelayComponent->setBounds(modDelayArea);

    mainUIArea.removeFromTop(spacing);

    auto spatialFXHeight = 150;
    auto spatialFXArea = mainUIArea.removeFromTop(spatialFXHeight);
    spatialFXComponent->setBounds(spatialFXArea);

    // Always set the bounds for the perception mode component
    perceptionModeComponent->setBounds(perceptionModeArea);
}