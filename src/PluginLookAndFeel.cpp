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
    const int availableKnobH = height - labelH;

    // The knob arc is usually square. 
    // We want the slider box to be snug against the label.
    int knobSide = std::min(width, availableKnobH);

    label->setBounds(x, y, width, labelH);

    // Position the slider immediately below the label (y + labelH)
    // and make its height equal to knobSide so it doesn't "float" down.
    slider->setBounds(x, y + labelH, width, knobSide);
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
    result.totalWidth = 0;
    result.totalHeight = 0;

    availableHeight -= 20; // Leave extra space for bottom label

    if (numKnobs <= 0 || availableWidth <= 4 || availableHeight <= 4)
        return result;

    const float contentW = static_cast<float>(availableWidth);
    const float contentH = static_cast<float>(availableHeight);

    // Find best column count — maximise min(cellW, cellH) so the knob arc is as large as possible
    int bestCols = numKnobs, bestRows = 1;
    float bestArc = 0.f;

    for (int cols = 1; cols <= numKnobs; ++cols)
    {
        const int   rows = (numKnobs + cols - 1) / cols;
        const float cellW = contentW / static_cast<float>(cols);
        // Each cell height includes the label (labelHeight px) — knob arc gets the rest
        const float cellH = contentH / static_cast<float>(rows);
        const float arc = std::min(cellW, cellH - static_cast<float>(labelHeight));

        if (arc > bestArc)
        {
            bestArc = arc;
            bestCols = cols;
            bestRows = rows;
        }
    }

    if (allowWideLayout)
    {
        bestCols = numKnobs;
        bestRows = 1;
    }

    // Rectangular cell dimensions — use full available space
    const float cellW = contentW / static_cast<float>(bestCols);
    const float cellH = contentH / static_cast<float>(bestRows);

    result.totalWidth = availableWidth;
    result.totalHeight = availableHeight;
    result.knobBounds.reserve(numKnobs);

    for (int i = 0; i < numKnobs; ++i)
    {
        const int col = i % bestCols;
        const int row = i / bestCols;

        // Integer cell edges — avoid rounding drift accumulating across columns
        const int cellX = static_cast<int>(col * cellW);
        const int cellY = static_cast<int>(row * cellH);
        const int cw = static_cast<int>((col + 1) * cellW) - cellX;
        const int ch = static_cast<int>((row + 1) * cellH) - cellY;

        // Each knob rect is the full cell — KnobWithLabel::setBounds splits it into
        // labelHeight (top) + knob arc (rest). No hard clamps on knob size.
        result.knobBounds.emplace_back(cellX, cellY, cw, ch);
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