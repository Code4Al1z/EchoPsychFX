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
    simpleVerbComponent = std::make_unique<SimpleVerbWithPredelayComponent>(p.parameters);

    // Create preset management (must happen after components are created)
    presetManager = std::make_unique<PerceptionPresetManager>(
        *tiltEQComponent, *widthBalancerComponent, *modDelayComponent,
        *spatialFXComponent, *microPitchDetuneComponent, *exciterSaturationComponent,
        *simpleVerbComponent);

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
    addAndMakeVisible(*simpleVerbComponent);

    // Setup mode toggle
    addAndMakeVisible(modeToggle);
    modeToggle.setButtonText("Show Perception Modes");
    modeToggle.setColour(juce::ToggleButton::textColourId, juce::Colours::white);
    modeToggle.setColour(juce::ToggleButton::tickColourId, juce::Colours::deeppink);
    modeToggle.onClick = [this]() { updateUIVisibility(); };
    modeToggle.setToggleState(false, juce::dontSendNotification);

    // Enable resizing with intelligent constraints
    setResizable(true, true);
    setResizeLimits(minWidth, minHeight, maxWidth, maxHeight);

    // Set default size (optimized for most displays)
    setSize(1300, 850);
    updateUIVisibility();
}

AudioPluginAudioProcessorEditor::~AudioPluginAudioProcessorEditor()
{
    setLookAndFeel(nullptr);
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
    simpleVerbComponent->setVisible(!perceptionMode);

    // Show/hide perception mode component
    perceptionModeComponent->setVisible(perceptionMode);

    // Update toggle button text
    modeToggle.setButtonText(perceptionMode ? "Show Manual Controls" : "Show Perception Modes");

    // Trigger layout update
    resized();
}

void AudioPluginAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(27, 17, 31)); // Dark background
}

void AudioPluginAudioProcessorEditor::layoutManualMode(juce::Rectangle<int> area)
{
    const int totalWidth = area.getWidth();
    const int totalHeight = area.getHeight();

    // Adaptive spacing based on window size
    const int margin = juce::jmax(8, totalWidth / 150);
    const int vSpacing = juce::jmax(8, totalHeight / 100);

    area.reduce(margin, margin);

    // Determine layout strategy based on available space
    const bool isNarrow = totalWidth < 1100;
    const bool isWide = totalWidth >= 1500;
    const bool isTall = totalHeight >= 900;

    //==============================================================================
    // ROW 1: Width Balancer + Tilt EQ (flexible arrangement)
    //==============================================================================
    auto row1 = area.removeFromTop(juce::jmax(140, totalHeight / 6));

    if (isNarrow)
    {
        // Stack vertically if too narrow
        auto widthArea = row1.removeFromTop(row1.getHeight() / 2);
        widthBalancerComponent->setBounds(widthArea.reduced(2));
        row1.removeFromTop(vSpacing);
        tiltEQComponent->setBounds(row1.reduced(2));
    }
    else
    {
        // Side by side with flexible proportions
        const int widthBalancerWidth = isWide ? (totalWidth * 60 / 100) : (totalWidth * 55 / 100);
        widthBalancerComponent->setBounds(row1.removeFromLeft(widthBalancerWidth).reduced(2));
        row1.removeFromLeft(margin);
        tiltEQComponent->setBounds(row1.reduced(2));
    }

    area.removeFromTop(vSpacing);

    //==============================================================================
    // ROW 2: Mod Delay (full width, adaptive height)
    //==============================================================================
    auto row2 = area.removeFromTop(juce::jmax(120, totalHeight / 8));
    modDelayComponent->setBounds(row2.reduced(2));
    area.removeFromTop(vSpacing);

    //==============================================================================
    // ROW 3: Spatial FX (needs more vertical space)
    //==============================================================================
    const int spatialHeight = isTall ? (totalHeight / 4) : juce::jmax(200, totalHeight / 5);
    auto row3 = area.removeFromTop(juce::jmin(spatialHeight, area.getHeight() / 3));
    spatialFXComponent->setBounds(row3.reduced(2));
    area.removeFromTop(vSpacing);

    //==============================================================================
    // ROW 4: Micro Pitch Detune (full width)
    //==============================================================================
    auto row4 = area.removeFromTop(juce::jmax(120, totalHeight / 9));
    microPitchDetuneComponent->setBounds(row4.reduced(2));
    area.removeFromTop(vSpacing);

    //==============================================================================
    // ROW 5: Exciter + Reverb (flexible arrangement)
    //==============================================================================
    auto row5 = area; // Use remaining space

    if (isNarrow || row5.getHeight() > row5.getWidth() / 3)
    {
        // Stack vertically if narrow or have extra height
        const int halfHeight = row5.getHeight() / 2;
        exciterSaturationComponent->setBounds(row5.removeFromTop(halfHeight - vSpacing / 2).reduced(2));
        row5.removeFromTop(vSpacing);
        simpleVerbComponent->setBounds(row5.reduced(2));
    }
    else
    {
        // Side by side
        const int halfWidth = row5.getWidth() / 2;
        exciterSaturationComponent->setBounds(row5.removeFromLeft(halfWidth - margin / 2).reduced(2));
        row5.removeFromLeft(margin);
        simpleVerbComponent->setBounds(row5.reduced(2));
    }
}

void AudioPluginAudioProcessorEditor::layoutPerceptionMode(juce::Rectangle<int> area)
{
    const int margin = juce::jmax(10, area.getWidth() / 130);
    area.reduce(margin, margin);

    // Perception mode gets full available space
    perceptionModeComponent->setBounds(area);
}

void AudioPluginAudioProcessorEditor::resized()
{
    auto bounds = getLocalBounds();
    const int margin = juce::jmax(12, getWidth() / 100);
    const int toggleHeight = 40;

    bounds.reduce(margin, margin);

    // Mode toggle at top
    modeToggle.setBounds(bounds.removeFromTop(toggleHeight));
    bounds.removeFromTop(margin);

    const bool perceptionMode = modeToggle.getToggleState();

    if (perceptionMode)
    {
        layoutPerceptionMode(bounds);
    }
    else
    {
        layoutManualMode(bounds);
    }
}