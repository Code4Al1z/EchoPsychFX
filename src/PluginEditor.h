#ifndef ECHOPSYCHFX_PLUGINEDITOR_H_INCLUDED
#define ECHOPSYCHFX_PLUGINEDITOR_H_INCLUDED

#include "PluginProcessor.h"
#include "WidthBalancerComponent.h"
#include "TiltEQComponent.h"
#include "ModDelayComponent.h"
#include "SpatialFXComponent.h"
#include "PerceptionModeComponent.h"
#include "PerceptionPresetManager.h"
#include "MicroPitchDetuneComponent.h"
#include "ExciterSaturationComponent.h"
#include "SimpleVerbWithPredelayComponent.h"
#include "PluginLookAndFeel.h"

class AudioPluginAudioProcessorEditor final : public juce::AudioProcessorEditor
{
public:
    explicit AudioPluginAudioProcessorEditor(AudioPluginAudioProcessor&);
    ~AudioPluginAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    AudioPluginAudioProcessor& processorRef;
    PluginLookAndFeel pluginLookAndFeel;
    juce::ToggleButton modeToggle;

    std::unique_ptr<WidthBalancerComponent>          widthBalancerComponent;
    std::unique_ptr<TiltEQComponent>                 tiltEQComponent;
    std::unique_ptr<ModDelayComponent>               modDelayComponent;
    std::unique_ptr<SpatialFXComponent>              spatialFXComponent;
    std::unique_ptr<MicroPitchDetuneComponent>       microPitchDetuneComponent;
    std::unique_ptr<ExciterSaturationComponent>      exciterSaturationComponent;
    std::unique_ptr<SimpleVerbWithPredelayComponent> simpleVerbComponent;

    std::unique_ptr<PerceptionPresetManager> presetManager;
    std::unique_ptr<PerceptionModeComponent> perceptionModeComponent;

    std::vector<std::unique_ptr<juce::TextButton>> blockToggles;

    struct ComponentInfo
    {
        juce::Component* component = nullptr;
        juce::String     label;
        int              numKnobs = 0;
        bool             userVisible = true;
        // minWidth is computed once in buildComponentInfoList —
        // it is the narrowest this block can usefully render.
        // Used only for row-wrap decisions, not as a render clamp.
        int              minWidth = 0;
    };

    std::vector<ComponentInfo> componentInfoList;

    void buildComponentInfoList();
    void resizeWindowForVisibleBlocks();
    void updateUIVisibility();
    void layoutManualMode(juce::Rectangle<int> area);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AudioPluginAudioProcessorEditor)
};

#endif // ECHOPSYCHFX_PLUGINEDITOR_H_INCLUDED