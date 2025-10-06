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

    // Enable resizing with constraints
    setResizable(true, true);
    setResizeLimits(800, 700, 1600, 1400); // min and max width/height

    // Set default size
    setSize(1000, 900);
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

    // Trigger layout update
    resized();
}

void AudioPluginAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
}

void AudioPluginAudioProcessorEditor::resized()
{
    auto bounds = getLocalBounds();
    const int margin = 15;
    const int spacing = 12;
    const int toggleHeight = 35;

    bounds.reduce(margin, margin);

    // Mode toggle at top
    modeToggle.setBounds(bounds.removeFromTop(toggleHeight));
    bounds.removeFromTop(spacing);

    const bool perceptionMode = modeToggle.getToggleState();

    if (perceptionMode)
    {
        // Perception mode uses full available space
        perceptionModeComponent->setBounds(bounds);
    }
    else
    {
        // Manual mode layout - responsive grid system
        const int totalWidth = bounds.getWidth();
        const int totalHeight = bounds.getHeight();

        // Calculate proportional heights based on available space
        const float row1Ratio = 0.18f;  // Width Balancer & Tilt EQ
        const float row2Ratio = 0.16f;  // Mod Delay
        const float row3Ratio = 0.28f;  // Spatial FX (needs more space)
        const float row4Ratio = 0.16f;  // Micro Pitch Detune
        const float row5Ratio = 0.22f;  // Exciter & Reverb

        int row1Height = juce::jmax(120, static_cast<int>(totalHeight * row1Ratio));
        int row2Height = juce::jmax(100, static_cast<int>(totalHeight * row2Ratio));
        int row3Height = juce::jmax(180, static_cast<int>(totalHeight * row3Ratio));
        int row4Height = juce::jmax(100, static_cast<int>(totalHeight * row4Ratio));
        int row5Height = juce::jmax(120, static_cast<int>(totalHeight * row5Ratio));

        // Row 1: Width Balancer (60%) and Tilt EQ (40%)
        auto row1 = bounds.removeFromTop(row1Height);
        const int widthBalancerWidth = static_cast<int>(totalWidth * 0.58f);
        widthBalancerComponent->setBounds(row1.removeFromLeft(widthBalancerWidth).reduced(2));
        row1.removeFromLeft(spacing);
        tiltEQComponent->setBounds(row1.reduced(2));
        bounds.removeFromTop(spacing);

        // Row 2: Mod Delay (full width)
        auto row2 = bounds.removeFromTop(row2Height);
        modDelayComponent->setBounds(row2.reduced(2));
        bounds.removeFromTop(spacing);

        // Row 3: Spatial FX (full width - needs more vertical space)
        auto row3 = bounds.removeFromTop(row3Height);
        spatialFXComponent->setBounds(row3.reduced(2));
        bounds.removeFromTop(spacing);

        // Row 4: Micro Pitch Detune (full width)
        auto row4 = bounds.removeFromTop(row4Height);
        microPitchDetuneComponent->setBounds(row4.reduced(2));
        bounds.removeFromTop(spacing);

        // Row 5: Exciter Saturation (50%) and Simple Reverb (50%)
        auto row5 = bounds.removeFromTop(row5Height);
        const int halfWidth = row5.getWidth() / 2;
        exciterSaturationComponent->setBounds(row5.removeFromLeft(halfWidth - spacing / 2).reduced(2));
        row5.removeFromLeft(spacing);
        simpleVerbWithPredelayComponent->setBounds(row5.reduced(2));
    }
}