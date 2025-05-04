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
    int knobSize = 120;

    modeToggle.setBounds(bounds.removeFromTop(toggleHeight));
    bounds.removeFromTop(spacing);

    if (perceptionModeComponent->isVisible())
    {
        perceptionModeComponent->setBounds(bounds);
    }
    else
    {
        auto area = bounds;

        // First row: WidthBalancer on the left, TiltEQ on the right
        auto topRowHeight = 150;
        auto topRow = area.removeFromTop(topRowHeight);

        auto widthBalancerWidth = getWidth() * 0.6;
        auto widthBalancerArea = topRow.removeFromLeft(widthBalancerWidth);
        widthBalancerComponent->setBounds(widthBalancerArea);

        topRow.removeFromLeft(spacing); // spacing between components

        auto tiltEQArea = topRow;
        tiltEQComponent->setBounds(tiltEQArea);

        area.removeFromTop(spacing);

        // Second row: ModDelay full width
        auto modDelayHeight = knobSize + 24 + 10 + 10;
        auto modDelayArea = area.removeFromTop(modDelayHeight);
        modDelayComponent->setBounds(modDelayArea);

        area.removeFromTop(spacing);

        // Bottom: SpatialFX
        auto spatialFXHeight = 150;
        auto spatialFXArea = area.removeFromTop(spatialFXHeight);
        spatialFXComponent->setBounds(spatialFXArea);
    }
}

