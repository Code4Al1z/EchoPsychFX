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

    std::unique_ptr<WidthBalancerComponent> widthBalancerComponent;
    std::unique_ptr<TiltEQComponent> tiltEQComponent;
    std::unique_ptr<ModDelayComponent> modDelayComponent;
    std::unique_ptr<SpatialFXComponent> spatialFXComponent;
    std::unique_ptr<MicroPitchDetuneComponent> microPitchDetuneComponent;
    std::unique_ptr<ExciterSaturationComponent> exciterSaturationComponent;
    std::unique_ptr<SimpleVerbWithPredelayComponent> simpleVerbComponent;

    std::unique_ptr<PerceptionPresetManager> presetManager;
    std::unique_ptr<PerceptionModeComponent> perceptionModeComponent;

    struct ComponentInfo
    {
        juce::Component* component;
        int numKnobs;
        bool allowWideLayout;
        int minWidth;
        int minHeight;
        int maxWidth;
        int maxHeight;
        float sizeWeight;
    };

    std::vector<ComponentInfo> getComponentInfoList();
    void updateUIVisibility();
    void layoutManualMode(juce::Rectangle<int> area);
    void layoutPerceptionMode(juce::Rectangle<int> area);
    void calculateMinMaxSizes();
    int calculateActualContentHeight();

    int calculateComponentMinWidth(int numKnobs, bool allowWideLayout) const;
    int calculateComponentMinHeight(int numKnobs, bool allowWideLayout) const;
    int calculateComponentMaxWidth(int numKnobs, bool allowWideLayout) const;
    int calculateComponentMaxHeight(int numKnobs, bool allowWideLayout) const;

    int calculatedMinWidth = 700;
    int calculatedMinHeight = 500;
    int calculatedMaxWidth = 2400;
    int calculatedMaxHeight = 1800;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AudioPluginAudioProcessorEditor)
};

#endif // ECHOPSYCHFX_PLUGINEDITOR_H_INCLUDED