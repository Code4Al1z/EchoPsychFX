#include "PluginLookAndFeel.h"

const juce::Colour PluginLookAndFeel::background{ 27, 17, 31 };
const juce::Colour PluginLookAndFeel::knobThumb{ 255, 107, 0 };
const juce::Colour PluginLookAndFeel::track{ 255, 46, 136 };
const juce::Colour PluginLookAndFeel::knobBackground{ 123, 0, 70 };
const juce::Colour PluginLookAndFeel::knobFill{ 255, 46, 136 };
const juce::Colour PluginLookAndFeel::knobOutline{ 90, 0, 50 };
const juce::Colour PluginLookAndFeel::labelText{ 232, 232, 240 };
const juce::Colour PluginLookAndFeel::groupOutline = juce::Colours::white.withAlpha(0.4f);

//==============================================================================
PluginLookAndFeel::PluginLookAndFeel()
{
    setColour(juce::GroupComponent::outlineColourId, juce::Colours::white.withAlpha(0.5f));
    setColour(juce::GroupComponent::textColourId, juce::Colours::white);
}

//==============================================================================
void PluginLookAndFeel::drawGroupComponentOutline(juce::Graphics& g, int width, int height,
    const juce::String& text, const juce::Justification& justification,
    juce::GroupComponent& component)
{
    const float textPadding = 4.0f;
    const int   textHeight = 20;

    auto font = juce::Font(static_cast<float>(textHeight) * 0.85f, juce::Font::bold);
    g.setFont(font);

    auto textWidth = static_cast<int>(font.getStringWidth(text) + 2.0f * textPadding);

    g.setColour(component.findColour(juce::GroupComponent::outlineColourId));
    g.drawRect(0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height), 1.0f);

    g.setColour(component.findColour(juce::GroupComponent::textColourId));
    g.drawFittedText(text, 10, height - textHeight - 4, textWidth, textHeight, justification, 1);
}

//==============================================================================
// KnobWithLabel
//==============================================================================

PluginLookAndFeel::KnobWithLabel::KnobWithLabel(juce::AudioProcessorValueTreeState& state,
    const juce::String& paramID,
    const juce::String& labelTextStr,
    juce::Component& parent)
{
    slider = std::make_unique<juce::Slider>();
    label = std::make_unique<juce::Label>();

    PluginLookAndFeel::configureKnob(*slider);
    PluginLookAndFeel::configureLabel(*label, labelTextStr);

    parent.addAndMakeVisible(*slider);
    parent.addAndMakeVisible(*label);

    attachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        state, paramID, *slider);
}

void PluginLookAndFeel::KnobWithLabel::setBounds(int x, int y, int width, int height)
{
    // Label takes a proportional slice of the height — minimum 12px, maximum 20px.
    // This makes labels scale down gracefully when blocks are small.
    const int lH = juce::jlimit(12, 20, static_cast<int>(height * 0.22f));
    const int knobH = height - lH;

    // Scale the text box inside the slider proportionally too
    const int tbW = juce::jlimit(40, 70, static_cast<int>(width * 0.85f));
    const int tbH = juce::jlimit(14, 20, static_cast<int>(knobH * 0.22f));
    slider->setTextBoxStyle(juce::Slider::TextBoxBelow, false, tbW, tbH);

    label->setBounds(x, y, width, lH);
    slider->setBounds(x, y + lH, width, knobH);
}

//==============================================================================
// findBestSquareGridFit
//
// Finds the column count that maximises cell size given the available area.
// No min/max cell size constraints — purely geometric best fit.
//==============================================================================

PluginLookAndFeel::GridFitResult PluginLookAndFeel::findBestSquareGridFit(
    int nElements, float totalWidth, float totalHeight)
{
    GridFitResult best;

    if (nElements <= 0 || totalWidth <= 0.0f || totalHeight <= 0.0f)
        return best;

    for (int columns = 1; columns <= nElements; ++columns)
    {
        const int   rows = (nElements + columns - 1) / columns;
        const float cellWidth = totalWidth / static_cast<float>(columns);
        const float cellHeight = totalHeight / static_cast<float>(rows);
        const float cellSize = std::min(cellWidth, cellHeight);

        if (cellSize > best.cellSize)
        {
            best.columns = columns;
            best.rows = rows;
            best.cellSize = cellSize;
        }
    }

    return best;
}

//==============================================================================
// calculateKnobLayout
//
// Works purely from available space — no hard pixel clamps.
// Knob size = cellSize * 0.78 (leaves room for label and breathing space).
// Label height = cellSize * 0.22, clamped to [12, 20].
//==============================================================================

PluginLookAndFeel::KnobLayoutResult PluginLookAndFeel::calculateKnobLayout(
    int numKnobs, int availableWidth, int availableHeight, bool allowWideLayout)
{
    KnobLayoutResult result;

    if (numKnobs <= 0 || availableWidth <= 0 || availableHeight <= 0)
        return result;

    const float contentW = static_cast<float>(availableWidth);
    const float contentH = static_cast<float>(availableHeight);

    GridFitResult grid;

    if (allowWideLayout)
    {
        // Force single row
        grid.columns = numKnobs;
        grid.rows = 1;
        grid.cellSize = contentW / static_cast<float>(numKnobs);
    }
    else
    {
        grid = findBestSquareGridFit(numKnobs, contentW, contentH);
    }

    // Absolute fallback — should never be reached
    if (grid.columns <= 0 || grid.cellSize <= 0.0f)
    {
        grid.columns = numKnobs;
        grid.rows = 1;
        grid.cellSize = contentW / static_cast<float>(numKnobs);
    }

    // knob cell occupies the full cell — KnobWithLabel::setBounds splits it
    // internally into label + knob proportionally.
    const float cellSize = grid.cellSize;

    // Centre the grid within the available area
    const float gridW = grid.columns * cellSize;
    const float gridH = grid.rows * cellSize;

    const int startX = static_cast<int>((contentW - gridW) * 0.5f);
    const int startY = static_cast<int>((contentH - gridH) * 0.5f);

    result.totalWidth = static_cast<int>(gridW);
    result.totalHeight = static_cast<int>(gridH);
    result.knobBounds.reserve(numKnobs);

    for (int i = 0; i < numKnobs; ++i)
    {
        const int col = i % grid.columns;
        const int row = i / grid.columns;

        const int cellX = startX + static_cast<int>(col * cellSize);
        const int cellY = startY + static_cast<int>(row * cellSize);

        // Small inset so knobs don't butt against each other
        const int inset = juce::jmax(2, static_cast<int>(cellSize * 0.04f));

        result.knobBounds.emplace_back(
            cellX + inset,
            cellY + inset,
            static_cast<int>(cellSize) - inset * 2,
            static_cast<int>(cellSize) - inset * 2);
    }

    return result;
}

//==============================================================================
void PluginLookAndFeel::configureKnob(juce::Slider& slider)
{
    slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 18);
    slider.setColour(juce::Slider::rotarySliderFillColourId, knobFill);
    slider.setColour(juce::Slider::thumbColourId, knobThumb);
    slider.setColour(juce::Slider::trackColourId, knobBackground);
    slider.setColour(juce::Slider::rotarySliderOutlineColourId, knobOutline);
}

void PluginLookAndFeel::configureLabel(juce::Label& label, const juce::String& text)
{
    label.setText(text, juce::dontSendNotification);
    label.setJustificationType(juce::Justification::centred);
    label.setColour(juce::Label::textColourId, labelText);
}

void PluginLookAndFeel::configureGroup(juce::GroupComponent& group)
{
    group.setColour(juce::GroupComponent::outlineColourId, groupOutline);
    group.setColour(juce::GroupComponent::textColourId, juce::Colours::white);
}