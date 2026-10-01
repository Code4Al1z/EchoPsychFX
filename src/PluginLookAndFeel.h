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

    void drawRotarySlider(juce::Graphics&, int x, int y, int width, int height, float sliderPos,
        float rotaryStartAngle, float rotaryEndAngle, juce::Slider&) override;

    void drawLinearSlider(juce::Graphics&, int x, int y, int width, int height, float sliderPos,
        float minSliderPos, float maxSliderPos, juce::Slider::SliderStyle, juce::Slider&) override;

    juce::Label* createSliderTextBox(juce::Slider&) override;

    void drawToggleButton(juce::Graphics&, juce::ToggleButton&, bool highlighted, bool down) override;

    void drawButtonBackground(juce::Graphics&, juce::Button&, const juce::Colour& backgroundColour,
        bool highlighted, bool down) override;

    void drawComboBox(juce::Graphics&, int width, int height, bool isButtonDown,
        int buttonX, int buttonY, int buttonW, int buttonH, juce::ComboBox&) override;

    juce::Font getComboBoxFont(juce::ComboBox&) override;

    void drawPopupMenuItem(juce::Graphics&, const juce::Rectangle<int>& area,
        bool isSeparator, bool isActive, bool isHighlighted, bool isTicked, bool hasSubMenu,
        const juce::String& text, const juce::String& shortcutKeyText,
        const juce::Drawable* icon, const juce::Colour* textColour) override;

    static const juce::Colour background;
    static const juce::Colour knobThumb;
    static const juce::Colour track;
    static const juce::Colour knobBackground;
    static const juce::Colour knobFill;
    static const juce::Colour knobOutline;
    static const juce::Colour labelText;
    static const juce::Colour groupOutline;
    static const juce::Colour headerBg;
    static const juce::Colour headerText;
    static const juce::Colour popupRowA;
    static const juce::Colour popupRowB;

    // Surfaces and text
    static const juce::Colour panel;
    static const juce::Colour panelRaised;
    static const juce::Colour mutedText;
    static const juce::Colour knobTrack;

    // One accent per processing stage, in signal-flow order
    static const juce::Colour accentInput;
    static const juce::Colour accentMotion;
    static const juce::Colour accentSpatial;
    static const juce::Colour accentMicroPitch;
    static const juce::Colour accentExciter;
    static const juce::Colour accentReverb;

    /** Recursively tints every control under `root` with the given accent. */
    static void applyAccent(juce::Component& root, juce::Colour accent);

    static constexpr int minKnobSize = 50;
    static constexpr int maxKnobSize = 120;
    static constexpr int margin = 10;
    static constexpr int labelHeight = 20;
    static constexpr int spacing = 15;
    static constexpr int groupLabelHeight = 20;

    static constexpr int kKnobCell = 110;
    static constexpr int kHeaderH = 28;
    static constexpr int kGap = 10;
    static constexpr int kEdgePad = 10;
    static constexpr int kModeToggleH = 36;
    static constexpr int kPresetBarH = 84;
    static constexpr int kInsightDrawerH = 62;

    struct KnobWithLabel
    {
        std::unique_ptr<juce::Slider> slider;
        std::unique_ptr<juce::Label> label;
        std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attachment;

        KnobWithLabel() = default;

        KnobWithLabel(juce::AudioProcessorValueTreeState& state,
            const juce::String& paramID,
            const juce::String& labelText,
            juce::Component& parent);

        void setBounds(int x, int y, int width, int height);
    };

    struct ShapePicker
    {
        juce::OwnedArray<juce::TextButton> buttons;
        std::unique_ptr<juce::ComboBox> hiddenCombo;
        std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> attachment;
        int selectedIndex = 0;

        ShapePicker() = default;

        ShapePicker(juce::AudioProcessorValueTreeState& state,
            const juce::String& paramID,
            const std::vector<juce::String>& labels,
            juce::Component& parent);

        void setSelected(int index);
        void setBounds(int x, int y, int width, int height);

    private:
        void updateVisuals(int index);
    };

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

    static void drawCollapsibleHeader(juce::Graphics& g,
        juce::Rectangle<int> bounds,
        const juce::String& title,
        bool isCollapsed,
        bool isVerticalCollapse,
        juce::Colour accent = juce::Colour(255, 46, 136));

    /** Draws a rounded panel card (used behind every section). */
    static void drawPanel(juce::Graphics& g, juce::Rectangle<int> bounds, juce::Colour fill, float corner = 8.0f);

    static void configureKnob(juce::Slider& slider);
    static void configureLabel(juce::Label& label, const juce::String& text);
    static void configureGroup(juce::GroupComponent& group);
    static void configureComboBox(juce::ComboBox& box);

    static void setKnobValue(const std::vector<std::unique_ptr<KnobWithLabel>>& knobs, int index, float value);

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PluginLookAndFeel)
};

#endif