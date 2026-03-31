#ifndef ECHOPSYCHFX_PLUGINLOOKANDFEEL_H_INCLUDED
#define ECHOPSYCHFX_PLUGINLOOKANDFEEL_H_INCLUDED

#include <juce_gui_extra/juce_gui_extra.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include <memory>

class PluginLookAndFeel : public juce::LookAndFeel_V4
{
public:
    PluginLookAndFeel();
    ~PluginLookAndFeel() override = default;

    void drawGroupComponentOutline(juce::Graphics&, int width, int height,
        const juce::String& text, const juce::Justification& justification,
        juce::GroupComponent&) override;

    //==========================================================================
    // Colours
    static const juce::Colour background;
    static const juce::Colour knobThumb;
    static const juce::Colour track;
    static const juce::Colour knobBackground;
    static const juce::Colour knobFill;
    static const juce::Colour knobOutline;
    static const juce::Colour labelText;
    static const juce::Colour groupOutline;

    //==========================================================================
    // Layout constants
    //
    // These are now used only as editor-level hints for block sizing decisions.
    // They are NOT used as hard pixel clamps inside calculateKnobLayout —
    // that function works purely from the space it is given.
    static constexpr int minKnobSize = 36;  // absolute floor for readability
    static constexpr int maxKnobSize = 120;
    static constexpr int margin = 10;
    static constexpr int spacing = 8;
    static constexpr int groupLabelH = 24;  // renamed from groupLabelHeight — same value
    static constexpr int groupLabelHeight = groupLabelH; // alias kept for components that use the old name

    // labelHeight is now dynamic (proportional to knob size) but this constant
    // is kept for components that still reference it for their own sizing logic.
    static constexpr int labelHeight = 18;

    //==========================================================================
    // KnobWithLabel — unchanged API, works with proportional layout
    struct KnobWithLabel
    {
        std::unique_ptr<juce::Slider> slider;
        std::unique_ptr<juce::Label>  label;
        std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attachment;

        KnobWithLabel() = default;

        KnobWithLabel(juce::AudioProcessorValueTreeState& state,
            const juce::String& paramID,
            const juce::String& labelText,
            juce::Component& parent);

        // Sets bounds for the label (top) + slider (below).
        // height is split: labelH = max(12, height * 0.2), rest = knob
        void setBounds(int x, int y, int width, int height);
    };

    //==========================================================================
    // Grid layout
    struct GridFitResult
    {
        int   columns = 0;
        int   rows = 0;
        float cellSize = 0.0f;
    };

    struct KnobLayoutResult
    {
        // Each rect is in the component's local coordinate space,
        // relative to the area passed into calculateKnobLayout.
        // x=0,y=0 = top-left of the available area.
        std::vector<juce::Rectangle<int>> knobBounds;
        int totalWidth = 0;
        int totalHeight = 0;
    };

    //==========================================================================
    // calculateKnobLayout
    //
    // Fills availableWidth x availableHeight with numKnobs knobs arranged in
    // the grid that maximises knob size. No hard pixel clamps — knob size is
    // always derived from the cell size. The caller is responsible for ensuring
    // the area is large enough to be readable.
    //
    // allowWideLayout: if true, forces a single row regardless of aspect ratio.
    //
    // Returns knobBounds relative to (0,0) = top-left of available area.
    // The caller translates by (area.getX(), area.getY()) before applying.
    static KnobLayoutResult calculateKnobLayout(int numKnobs,
        int availableWidth,
        int availableHeight,
        bool allowWideLayout);

    static GridFitResult findBestSquareGridFit(int nElements,
        float totalWidth,
        float totalHeight);

    //==========================================================================
    static void configureKnob(juce::Slider& slider);
    static void configureLabel(juce::Label& label, const juce::String& text);
    static void configureGroup(juce::GroupComponent& group);

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PluginLookAndFeel)
};

#endif