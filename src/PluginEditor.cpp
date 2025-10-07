#include "PluginProcessor.h"
#include "PluginEditor.h"

AudioPluginAudioProcessorEditor::AudioPluginAudioProcessorEditor(AudioPluginAudioProcessor& p)
    : AudioProcessorEditor(&p), processorRef(p)
{
    widthBalancerComponent = std::make_unique<WidthBalancerComponent>(p.parameters);
    tiltEQComponent = std::make_unique<TiltEQComponent>(p.parameters);
    modDelayComponent = std::make_unique<ModDelayComponent>(p.parameters);
    spatialFXComponent = std::make_unique<SpatialFXComponent>(p.parameters);
    microPitchDetuneComponent = std::make_unique<MicroPitchDetuneComponent>(p.parameters);
    exciterSaturationComponent = std::make_unique<ExciterSaturationComponent>(p.parameters);
    simpleVerbComponent = std::make_unique<SimpleVerbWithPredelayComponent>(p.parameters);

    presetManager = std::make_unique<PerceptionPresetManager>(
        *tiltEQComponent, *widthBalancerComponent, *modDelayComponent,
        *spatialFXComponent, *microPitchDetuneComponent, *exciterSaturationComponent,
        *simpleVerbComponent);

    perceptionModeComponent = std::make_unique<PerceptionModeComponent>(*presetManager);

    setLookAndFeel(&pluginLookAndFeel);

    addAndMakeVisible(*widthBalancerComponent);
    addAndMakeVisible(*tiltEQComponent);
    addAndMakeVisible(*modDelayComponent);
    addAndMakeVisible(*spatialFXComponent);
    addAndMakeVisible(*perceptionModeComponent);
    addAndMakeVisible(*microPitchDetuneComponent);
    addAndMakeVisible(*exciterSaturationComponent);
    addAndMakeVisible(*simpleVerbComponent);

    addAndMakeVisible(modeToggle);
    modeToggle.setButtonText("Show Perception Modes");
    modeToggle.setColour(juce::ToggleButton::textColourId, juce::Colours::white);
    modeToggle.setColour(juce::ToggleButton::tickColourId, juce::Colours::deeppink);
    modeToggle.onClick = [this]() { updateUIVisibility(); };
    modeToggle.setToggleState(false, juce::dontSendNotification);

    setResizable(true, true);

    using namespace UIHelpers::Dimensions;
    const int calculatedMinWidth = (knobSize * 6 + spacing * 5 + margin * 2) + margin * 2;
    const int calculatedMinHeight = (knobSize + labelHeight + groupLabelHeight + margin * 2) * 7 + margin * 10 + 40;

    setResizeLimits(calculatedMinWidth, calculatedMinHeight, maxWidth, maxHeight);
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

    widthBalancerComponent->setVisible(!perceptionMode);
    tiltEQComponent->setVisible(!perceptionMode);
    modDelayComponent->setVisible(!perceptionMode);
    spatialFXComponent->setVisible(!perceptionMode);
    microPitchDetuneComponent->setVisible(!perceptionMode);
    exciterSaturationComponent->setVisible(!perceptionMode);
    simpleVerbComponent->setVisible(!perceptionMode);

    perceptionModeComponent->setVisible(perceptionMode);

    modeToggle.setButtonText(perceptionMode ? "Show Manual Controls" : "Show Perception Modes");

    resized();
}

void AudioPluginAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(27, 17, 31));
}

void AudioPluginAudioProcessorEditor::layoutManualMode(juce::Rectangle<int> area)
{
    using namespace UIHelpers::Dimensions;

    const int componentHeight = knobSize + labelHeight + groupLabelHeight + margin * 2;
    const int vSpacing = margin;

    auto row1 = area.removeFromTop(componentHeight);
    const int widthBalancerWidth = knobSize * 2 + spacing * 3 + 60 + 80 + margin * 2;
    widthBalancerComponent->setBounds(row1.removeFromLeft(widthBalancerWidth));
    row1.removeFromLeft(vSpacing);
    tiltEQComponent->setBounds(row1);

    area.removeFromTop(vSpacing);

    auto row2 = area.removeFromTop(componentHeight);
    modDelayComponent->setBounds(row2);

    area.removeFromTop(vSpacing);

    auto row3 = area.removeFromTop(componentHeight * 2);
    spatialFXComponent->setBounds(row3);

    area.removeFromTop(vSpacing);

    auto row4 = area.removeFromTop(componentHeight);
    microPitchDetuneComponent->setBounds(row4);

    area.removeFromTop(vSpacing);

    auto row5 = area;
    const int halfWidth = row5.getWidth() / 2;
    exciterSaturationComponent->setBounds(row5.removeFromLeft(halfWidth - vSpacing / 2));
    row5.removeFromLeft(vSpacing);
    simpleVerbComponent->setBounds(row5);
}

void AudioPluginAudioProcessorEditor::layoutPerceptionMode(juce::Rectangle<int> area)
{
    perceptionModeComponent->setBounds(area);
}

void AudioPluginAudioProcessorEditor::resized()
{
    using namespace UIHelpers::Dimensions;

    auto bounds = getLocalBounds();
    bounds.reduce(margin, margin);

    const int toggleHeight = 40;
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
