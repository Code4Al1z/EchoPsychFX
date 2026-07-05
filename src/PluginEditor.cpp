#include "PluginProcessor.h"
#include "PluginEditor.h"

using L = PluginLookAndFeel;

AudioPluginAudioProcessorEditor::AudioPluginAudioProcessorEditor(AudioPluginAudioProcessor& p)
    : AudioProcessorEditor(&p), processorRef(p)
{
    inputControlsComponent = std::make_unique<InputControlsComponent>(p.parameters);
    modDelayComponent = std::make_unique<ModDelayComponent>(p.parameters);
    spatialFXComponent = std::make_unique<SpatialFXComponent>(p.parameters);
    microPitchDetuneComponent = std::make_unique<MicroPitchDetuneComponent>(p.parameters);
    exciterSaturationComponent = std::make_unique<ExciterSaturationComponent>(p.parameters);
    simpleVerbComponent = std::make_unique<SimpleVerbWithPredelayComponent>(p.parameters);

    presetManager = std::make_unique<PerceptionPresetManager>(
        inputControlsComponent->getTiltEQ(),
        inputControlsComponent->getWidthBalancer(),
        *modDelayComponent,
        *spatialFXComponent,
        *microPitchDetuneComponent,
        *exciterSaturationComponent,
        *simpleVerbComponent);

    perceptionModeComponent = std::make_unique<PerceptionModeComponent>(*presetManager);

    setLookAndFeel(&pluginLookAndFeel);

    addAndMakeVisible(*inputControlsComponent);
    addAndMakeVisible(*modDelayComponent);
    addAndMakeVisible(*spatialFXComponent);
    addAndMakeVisible(*microPitchDetuneComponent);
    addAndMakeVisible(*exciterSaturationComponent);
    addAndMakeVisible(*simpleVerbComponent);
    addAndMakeVisible(*perceptionModeComponent);

    auto collapseCallback = [this] { onBlockCollapseChanged(); };
    inputControlsComponent->onCollapseChanged = collapseCallback;
    modDelayComponent->onCollapseChanged = collapseCallback;
    spatialFXComponent->onCollapseChanged = collapseCallback;
    microPitchDetuneComponent->onCollapseChanged = collapseCallback;
    exciterSaturationComponent->onCollapseChanged = collapseCallback;
    simpleVerbComponent->onCollapseChanged = collapseCallback;

    addAndMakeVisible(modeToggle);
    modeToggle.setButtonText("Perception Modes");
    modeToggle.setColour(juce::ToggleButton::textColourId, juce::Colours::white);
    modeToggle.setColour(juce::ToggleButton::tickColourId, juce::Colours::deeppink);
    modeToggle.onClick = [this]
        {
            const bool pm = modeToggle.getToggleState();
            perceptionModeComponent->setVisible(pm);
            inputControlsComponent->setVisible(!pm);
            modDelayComponent->setVisible(!pm);
            spatialFXComponent->setVisible(!pm);
            microPitchDetuneComponent->setVisible(!pm);
            exciterSaturationComponent->setVisible(!pm);
            simpleVerbComponent->setVisible(!pm);
            resized();
        };
    modeToggle.setToggleState(false, juce::dontSendNotification);

    setResizable(false, false);
    applyWindowSize();
}

AudioPluginAudioProcessorEditor::~AudioPluginAudioProcessorEditor()
{
    setLookAndFeel(nullptr);
}

void AudioPluginAudioProcessorEditor::onBlockCollapseChanged()
{
    applyWindowSize();
    resized();
}

void AudioPluginAudioProcessorEditor::applyWindowSize()
{
    setSize(calculateWindowWidth(), calculateWindowHeight());
}

int AudioPluginAudioProcessorEditor::calculateWindowWidth() const
{
    const int col1 = juce::jmax(inputControlsComponent->currentWidth(),
        microPitchDetuneComponent->currentWidth());
    const int col2 = juce::jmax(modDelayComponent->currentWidth(),
        exciterSaturationComponent->currentWidth());
    const int col3 = juce::jmax(spatialFXComponent->currentWidth(),
        simpleVerbComponent->currentWidth());

    return L::kEdgePad + col1 + L::kGap + col2 + L::kGap + col3 + L::kEdgePad;
}

int AudioPluginAudioProcessorEditor::calculateWindowHeight() const
{
    const int row1 = juce::jmax(inputControlsComponent->currentHeight(),
        juce::jmax(modDelayComponent->currentHeight(), spatialFXComponent->currentHeight()));
    const int row2 = juce::jmax(microPitchDetuneComponent->currentHeight(),
        juce::jmax(exciterSaturationComponent->currentHeight(), simpleVerbComponent->currentHeight()));

    return L::kEdgePad + L::kModeToggleH + L::kGap + row1 + L::kGap + row2 + L::kEdgePad;
}

void AudioPluginAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(PluginLookAndFeel::background);
}

void AudioPluginAudioProcessorEditor::resized()
{
    auto bounds = getLocalBounds().reduced(L::kEdgePad);

    modeToggle.setBounds(bounds.removeFromTop(L::kModeToggleH));
    bounds.removeFromTop(L::kGap);

    if (modeToggle.getToggleState())
    {
        perceptionModeComponent->setBounds(bounds);
        return;
    }
    perceptionModeComponent->setVisible(false);

    const int col1W = juce::jmax(inputControlsComponent->currentWidth(),
        microPitchDetuneComponent->currentWidth());
    const int col2W = juce::jmax(modDelayComponent->currentWidth(),
        exciterSaturationComponent->currentWidth());
    const int col3W = juce::jmax(spatialFXComponent->currentWidth(),
        simpleVerbComponent->currentWidth());

    const int row1H = juce::jmax(inputControlsComponent->currentHeight(),
        juce::jmax(modDelayComponent->currentHeight(), spatialFXComponent->currentHeight()));
    const int row2H = juce::jmax(microPitchDetuneComponent->currentHeight(),
        juce::jmax(exciterSaturationComponent->currentHeight(), simpleVerbComponent->currentHeight()));

    const int x1 = bounds.getX();
    const int x2 = x1 + col1W + L::kGap;
    const int x3 = x2 + col2W + L::kGap;
    const int y1 = bounds.getY();
    const int y2 = y1 + row1H + L::kGap;

    inputControlsComponent->setBounds(x1, y1, col1W, row1H);
    modDelayComponent->setBounds(x2, y1, col2W, row1H);
    spatialFXComponent->setBounds(x3, y1, col3W, row1H);

    microPitchDetuneComponent->setBounds(x1, y2, col1W, row2H);
    exciterSaturationComponent->setBounds(x2, y2, col2W, row2H);
    simpleVerbComponent->setBounds(x3, y2, col3W, row2H);
}