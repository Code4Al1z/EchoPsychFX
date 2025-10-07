#include "PluginLookAndFeel.h"

const juce::Colour PluginLookAndFeel::background{ 27, 17, 31 };
const juce::Colour PluginLookAndFeel::knobThumb{ 255, 107, 0 };
const juce::Colour PluginLookAndFeel::track{ 255, 46, 136 };
const juce::Colour PluginLookAndFeel::knobBackground{ 123, 0, 70 };
const juce::Colour PluginLookAndFeel::knobFill{ 255, 46, 136 };
const juce::Colour PluginLookAndFeel::knobOutline{ 90, 0, 50 };
const juce::Colour PluginLookAndFeel::labelText{ 232, 232, 240 };
const juce::Colour PluginLookAndFeel::groupOutline = juce::Colours::white.withAlpha(0.4f);

PluginLookAndFeel::PluginLookAndFeel()
{
    setColour(juce::GroupComponent::outlineColourId, juce::Colours::white.withAlpha(0.5f));
    setColour(juce::GroupComponent::textColourId, juce::Colours::white);
}

void PluginLookAndFeel::drawGroupComponentOutline(juce::Graphics& g, int width, int height,
    const juce::String& text, const juce::Justification& justification,
    juce::GroupComponent& component)
{
    const float textPadding = 4.0f;
    const int textHeight = 24;

    auto font = juce::Font(textHeight * 0.8f, juce::Font::bold);
    g.setFont(font);

    auto textWidth = static_cast<int>(font.getStringWidth(text) + 2.0f * textPadding);

    g.setColour(component.findColour(juce::GroupComponent::outlineColourId));
    g.drawRect(0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height), 1.0f);

    g.setColour(component.findColour(juce::GroupComponent::textColourId));
    g.drawFittedText(text, 10, height - 25, textWidth, textHeight, justification, 1);
}

PluginLookAndFeel::KnobWithLabel::KnobWithLabel(juce::AudioProcessorValueTreeState& state,
    const juce::String& paramID,
    const juce::String& labelText,
    juce::Component& parent)
{
    slider = std::make_unique<juce::Slider>();
    label = std::make_unique<juce::Label>();

    PluginLookAndFeel::configureKnob(*slider);
    PluginLookAndFeel::configureLabel(*label, labelText);

    parent.addAndMakeVisible(*slider);
    parent.addAndMakeVisible(*label);

    attachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        state, paramID, *slider);
}

void PluginLookAndFeel::KnobWithLabel::setBounds(int x, int y, int width, int height)
{
    const int labelH = PluginLookAndFeel::labelHeight;
    const int knobH = height - labelH;

    label->setBounds(x, y, width, labelH);
    slider->setBounds(x, y + labelH, width, knobH);
}

PluginLookAndFeel::GridFitResult PluginLookAndFeel::findBestSquareGridFit(
    int nElements,
    float totalWidth,
    float totalHeight,
    float minCellSize,
    float maxCellSize)
{
    GridFitResult best;

    if (nElements <= 0 || totalWidth <= 0.0f || totalHeight <= 0.0f)
        return best;

    // Try to find the layout (cols x rows) that yields the largest valid cellSize
    for (int columns = 1; columns <= nElements; ++columns)
    {
        const int rows = (nElements + columns - 1) / columns; // ceil div
        const float cellWidth = totalWidth / static_cast<float>(columns);
        const float cellHeight = totalHeight / static_cast<float>(rows);
        const float cellSize = std::min(cellWidth, cellHeight); // square cell

        if (cellSize >= minCellSize && cellSize <= maxCellSize)
        {
            if (cellSize > best.cellSize)
            {
                best.columns = columns;
                best.rows = rows;
                best.cellSize = cellSize;
            }
        }
    }

    // If nothing matched the min/max bounds, pick the best possible (largest cellSize)
    if (best.columns == 0)
    {
        float bestCell = 0.0f;
        for (int columns = 1; columns <= nElements; ++columns)
        {
            const int rows = (nElements + columns - 1) / columns;
            const float cellWidth = totalWidth / static_cast<float>(columns);
            const float cellHeight = totalHeight / static_cast<float>(rows);
            const float cellSize = std::min(cellWidth, cellHeight);
            if (cellSize > bestCell)
            {
                bestCell = cellSize;
                best.columns = columns;
                best.rows = rows;
                best.cellSize = cellSize;
            }
        }
    }

    return best;
}

PluginLookAndFeel::KnobLayoutResult PluginLookAndFeel::calculateKnobLayout(
    int numKnobs, int availableWidth, int availableHeight, bool allowWideLayout)
{
    KnobLayoutResult result;

    if (numKnobs <= 0 || availableWidth <= 0 || availableHeight <= 0)
        return result;

    // compute available area
    const float contentW = static_cast<float>(availableWidth - margin * 2);
    const float contentH = static_cast<float>(availableHeight - margin * 2 - groupLabelHeight);

    if (contentW <= 0.0f || contentH <= 0.0f)
        return result;

    // Each cell includes the spacing around the knob
    const float minCell = static_cast<float>(minKnobSize) + labelHeight + spacing;
    const float maxCell = static_cast<float>(maxKnobSize) + labelHeight + spacing;

    auto grid = findBestSquareGridFit(numKnobs, contentW, contentH, minCell, maxCell);

    if (allowWideLayout)
    {
        // Single row using full width
        grid.rows = 1;
        grid.columns = numKnobs;
        grid.cellSize = contentW / static_cast<float>(numKnobs);
    }

    // Fallback if grid invalid
    if (grid.columns <= 0 || grid.rows <= 0 || grid.cellSize <= 0.0f)
    {
        grid.columns = numKnobs;
        grid.rows = 1;
        grid.cellSize = contentW / static_cast<float>(grid.columns);
    }

    if (grid.columns <= 0 || grid.cellSize <= 0.0f)
        return result;

    // Now, elementSize is the knob size inside the cell
    const int elementSize = juce::jlimit(PluginLookAndFeel::minKnobSize,
        PluginLookAndFeel::maxKnobSize,
        static_cast<int>(std::floor(grid.cellSize - spacing)));
    const int totalElementHeight = elementSize + labelHeight;

    const int gridWidth = static_cast<int>(std::round(grid.columns * grid.cellSize));
    const int gridHeight = static_cast<int>(std::round(grid.rows * grid.cellSize));

    const int startX = (availableWidth - gridWidth) / 2;
    const int startY = (availableHeight - gridHeight) / 2 + (groupLabelHeight / 2);

    result.totalWidth = gridWidth;
    result.totalHeight = gridHeight;
    result.knobBounds.reserve(numKnobs);

    for (int i = 0; i < numKnobs; ++i)
    {
        const int col = i % grid.columns;
        const int row = i / grid.columns;

        const int cellX = startX + static_cast<int>(std::round(col * grid.cellSize));
        const int cellY = startY + static_cast<int>(std::round(row * grid.cellSize));

        // Center the knob inside the cell, spacing is included
        const int elemX = cellX + (static_cast<int>(grid.cellSize) - elementSize) / 2;
        const int elemY = cellY + (static_cast<int>(grid.cellSize) - totalElementHeight) / 2;

        result.knobBounds.emplace_back(elemX, elemY, elementSize, totalElementHeight);
    }

    return result;
}



void PluginLookAndFeel::configureKnob(juce::Slider& slider)
{
    slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
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