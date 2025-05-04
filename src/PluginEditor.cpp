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

        // Width balancer full-width on top
        auto widthBalancerArea = area.removeFromTop(150);
        widthBalancerComponent->setBounds(widthBalancerArea);
        area.removeFromTop(spacing);

        // Split next row horizontally between TiltEQ and ModDelay
        auto tiltAndDelayRow = area.removeFromTop(knobSize + 24 + 10 + 10); // height for knob + label + spacing

        auto tiltWidth = knobSize + spacing * 2;
        auto tiltEQArea = tiltAndDelayRow.removeFromLeft(tiltWidth);
        tiltEQComponent->setBounds(tiltEQArea);

        tiltAndDelayRow.removeFromLeft(spacing); // spacing between the two

        auto modDelayWidth = knobSize * 6 + spacing * 6; // 6 knobs with spacing
        auto modDelayArea = tiltAndDelayRow.removeFromLeft(modDelayWidth);
        modDelayComponent->setBounds(modDelayArea);

        area.removeFromTop(spacing);

        auto spatialFXArea = area.removeFromTop(150);
        spatialFXComponent->setBounds(spatialFXArea);
    }
}
