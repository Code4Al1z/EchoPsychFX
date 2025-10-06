#include "PluginProcessor.h"
#include "PluginEditor.h"

AudioPluginAudioProcessorEditor::AudioPluginAudioProcessorEditor(AudioPluginAudioProcessor& p)
    : AudioProcessorEditor(&p), processorRef(p)
{
    // Create effect components
    widthBalancerComponent = std::make_unique<WidthBalancerComponent>(p.parameters);
    tiltEQComponent = std::make_unique<TiltEQComponent>(p.parameters);
    modDelayComponent = std::make_unique<ModDelayComponent>(p.parameters);
    spatialFXComponent = std::make_unique<SpatialFXComponent>(p.parameters);
    microPitchDetuneComponent = std::make_unique<MicroPitchDetuneComponent>(p.parameters);
    exciterSaturationComponent = std::make_unique<ExciterSaturationComponent>(p.parameters);
    simpleVerbWithPredelayComponent = std::make_unique<SimpleVerbWithPredelayComponent>(p.parameters);

    // Create preset management (must happen after components are created)
    presetManager = std::make_unique<PerceptionPresetManager>(
        *tiltEQComponent, *widthBalancerComponent, *modDelayComponent,
        *spatialFXComponent, *microPitchDetuneComponent, *exciterSaturationComponent,
        *simpleVerbWithPredelayComponent);

    perceptionModeComponent = std::make_unique<PerceptionModeComponent>(*presetManager);

    // Apply custom look and feel
    setLookAndFeel(&pluginLookAndFeel);

    // Add components to UI
    addAndMakeVisible(*widthBalancerComponent);
    addAndMakeVisible(*tiltEQComponent);
    addAndMakeVisible(*modDelayComponent);
    addAndMakeVisible(*spatialFXComponent);
    addAndMakeVisible(*perceptionModeComponent);
    addAndMakeVisible(*microPitchDetuneComponent);
    addAndMakeVisible(*exciterSaturationComponent);
    addAndMakeVisible(*simpleVerbWithPredelayComponent);

    // Setup mode toggle
    addAndMakeVisible(modeToggle);
    modeToggle.setButtonText("Show Perception Modes");
    modeToggle.setColour(juce::ToggleButton::textColourId, juce::Colours::white);
    modeToggle.setColour(juce::ToggleButton::tickColourId, juce::Colours::deeppink);
    modeToggle.onClick = [this]() { updateUIVisibility(); };
    modeToggle.setToggleState(false, juce::dontSendNotification);

    setSize(900, 1000);
    updateUIVisibility();
}

AudioPluginAudioProcessorEditor::~AudioPluginAudioProcessorEditor()
{
    setLookAndFeel(nullptr); // Important to avoid dangling reference
}

void AudioPluginAudioProcessorEditor::updateUIVisibility()
{
    const bool perceptionMode = modeToggle.getToggleState();

    // Show/hide manual mode components
    widthBalancerComponent->setVisible(!perceptionMode);
    tiltEQComponent->setVisible(!perceptionMode);
    modDelayComponent->setVisible(!perceptionMode);
    spatialFXComponent->setVisible(!perceptionMode);
    microPitchDetuneComponent->setVisible(!perceptionMode);
    exciterSaturationComponent->setVisible(!perceptionMode);
    simpleVerbWithPredelayComponent->setVisible(!perceptionMode);

    // Show/hide perception mode component
    perceptionModeComponent->setVisible(perceptionMode);

    // Update toggle button text
    modeToggle.setButtonText(perceptionMode ? "Show Manual Controls" : "Show Perception Modes");
}

void AudioPluginAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
}

void AudioPluginAudioProcessorEditor::resized()
{
    auto bounds = getLocalBounds().reduced(10);

    // Mode toggle at top
    static constexpr int toggleHeight = 30;
    static constexpr int spacing = 10;
    static constexpr int knobSize = 120;

    modeToggle.setBounds(bounds.removeFromTop(toggleHeight));
    bounds.removeFromTop(spacing);

    // Manual mode layout
    auto manualArea = bounds;
    auto perceptionArea = bounds;

    // Top row: WidthBalancer (60%) and TiltEQ (40%)
    auto topRowHeight = 150;
    auto topRow = manualArea.removeFromTop(topRowHeight);

    auto widthBalancerWidth = static_cast<int>(getWidth() * 0.6f);
    widthBalancerComponent->setBounds(topRow.removeFromLeft(widthBalancerWidth));
    topRow.removeFromLeft(spacing);
    tiltEQComponent->setBounds(topRow);

    manualArea.removeFromTop(spacing);

    // ModDelay row
    auto modDelayHeight = knobSize + 24 + spacing * 2;
    modDelayComponent->setBounds(manualArea.removeFromTop(modDelayHeight));
    manualArea.removeFromTop(spacing);

    // SpatialFX row
    auto spatialFXHeight = 300;
    spatialFXComponent->setBounds(manualArea.removeFromTop(spatialFXHeight));
    manualArea.removeFromTop(spacing);

    // MicroPitchDetune row
    auto microPitchHeight = 150;
    microPitchDetuneComponent->setBounds(manualArea.removeFromTop(microPitchHeight));
    manualArea.removeFromTop(spacing);

    // Bottom row: ExciterSaturation (50%) and SimpleVerb (50%)
    auto bottomRowHeight = 150;
    auto bottomRow = manualArea.removeFromTop(bottomRowHeight);

    auto leftComponentArea = bottomRow.removeFromLeft(bottomRow.getWidth() / 2).reduced(5);
    auto rightComponentArea = bottomRow.reduced(5);

    exciterSaturationComponent->setBounds(leftComponentArea);
    simpleVerbWithPredelayComponent->setBounds(rightComponentArea);

    // Perception mode layout (uses full area)
    perceptionModeComponent->setBounds(perceptionArea);
}