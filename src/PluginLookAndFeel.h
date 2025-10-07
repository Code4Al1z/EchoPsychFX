#ifndef ECHOPSYCHFX_PLUGINLOOKANDFEEL_H_INCLUDED
#define ECHOPSYCHFX_PLUGINLOOKANDFEEL_H_INCLUDED

#include <juce_gui_extra/juce_gui_extra.h>

class PluginLookAndFeel : public juce::LookAndFeel_V4
{
public:
    PluginLookAndFeel();
    ~PluginLookAndFeel() override = default;

    void drawGroupComponentOutline(juce::Graphics&, int width, int height,
        const juce::String& text, const juce::Justification& justification,
        juce::GroupComponent&) override;

    // Colors
    static const juce::Colour background;
    static const juce::Colour knobThumb;
    static const juce::Colour track;
    static const juce::Colour knobBackground;
    static const juce::Colour knobFill;
    static const juce::Colour knobOutline;
    static const juce::Colour labelText;
    static const juce::Colour groupOutline;

    // Dimensions
    static constexpr int knobSize = 100;
    static constexpr int margin = 10;
    static constexpr int labelHeight = 20;
    static constexpr int spacing = 20;
    static constexpr int groupLabelHeight = 30;

    // Grid layout
    struct GridFitResult
    {
        int columns = 0;
        int rows = 0;
        float cellSize = 0.0f;
    };

    struct KnobLayoutResult
    {
        std::vector<juce::Rectangle<int>> knobBounds;
        int totalWidth;
        int totalHeight;
    };

    static GridFitResult findBestSquareGridFit(int nElements,
        float totalWidth,
        float totalHeight,
        float minCellSize,
        float maxCellSize);

    static KnobLayoutResult calculateKnobLayout(int numKnobs, int availableWidth, int availableHeight, bool allowWideLayout);

    static void configureKnob(juce::Slider& slider);
    static void configureLabel(juce::Label& label, const juce::String& text);
    static void configureGroup(juce::GroupComponent& group);

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PluginLookAndFeel)
};

#endif