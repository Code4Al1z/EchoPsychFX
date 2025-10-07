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

    for (int columns = 1; columns <= nElements; ++columns)
    {
        int rows = (nElements + columns - 1) / columns;

        float cellWidth = totalWidth / static_cast<float>(columns);
        float cellHeight = totalHeight / static_cast<float>(rows);

        float cellSize = std::min(cellWidth, cellHeight);

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

    return best;
}

PluginLookAndFeel::KnobLayoutResult PluginLookAndFeel::calculateKnobLayout(
    int numKnobs, int availableWidth, int availableHeight, bool allowWideLayout)
{
    KnobLayoutResult result;

    float width = static_cast<float>(availableWidth - margin * 2);
    float height = static_cast<float>(availableHeight - margin * 2 - groupLabelHeight);

    float minCell = knobSize + labelHeight + spacing;
    float maxCell = knobSize + labelHeight + spacing;

    auto grid = findBestSquareGridFit(numKnobs, width, height, minCell, maxCell);

    if (grid.cellSize <= 0 || grid.columns == 0)
    {
        grid.columns = numKnobs;
        grid.rows = 1;
        grid.cellSize = minCell;
    }

    const int elementSize = static_cast<int>(grid.cellSize) - spacing;
    const int totalElementHeight = elementSize + labelHeight;

    const int gridWidth = static_cast<int>(grid.columns * grid.cellSize);
    const int gridHeight = static_cast<int>(grid.rows * grid.cellSize);

    const int startX = (availableWidth - gridWidth) / 2;
    const int startY = (availableHeight - gridHeight) / 2;

    for (int i = 0; i < numKnobs; ++i)
    {
        int col = i % grid.columns;
        int row = i / grid.columns;

        int cellX = startX + static_cast<int>(col * grid.cellSize);
        int cellY = startY + static_cast<int>(row * grid.cellSize);

        int elemX = cellX + (static_cast<int>(grid.cellSize) - elementSize) / 2;
        int elemY = cellY + (static_cast<int>(grid.cellSize) - totalElementHeight) / 2;

        result.knobBounds.emplace_back(elemX, elemY, elementSize, totalElementHeight);
    }

    result.totalWidth = gridWidth;
    result.totalHeight = gridHeight;

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