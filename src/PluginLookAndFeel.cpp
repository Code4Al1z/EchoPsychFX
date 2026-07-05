#include "PluginLookAndFeel.h"

const juce::Colour PluginLookAndFeel::background{ 27, 17, 31 };
const juce::Colour PluginLookAndFeel::knobThumb{ 255, 107, 0 };
const juce::Colour PluginLookAndFeel::track{ 255, 46, 136 };
const juce::Colour PluginLookAndFeel::knobBackground{ 123, 0, 70 };
const juce::Colour PluginLookAndFeel::knobFill{ 255, 46, 136 };
const juce::Colour PluginLookAndFeel::knobOutline{ 90, 0, 50 };
const juce::Colour PluginLookAndFeel::labelText{ 232, 232, 240 };
const juce::Colour PluginLookAndFeel::groupOutline = juce::Colours::white.withAlpha(0.4f);
const juce::Colour PluginLookAndFeel::headerBg{ 50, 18, 58 };
const juce::Colour PluginLookAndFeel::headerText{ 255, 255, 255 };

PluginLookAndFeel::PluginLookAndFeel()
{
    setColour(juce::GroupComponent::outlineColourId, juce::Colours::white.withAlpha(0.5f));
    setColour(juce::GroupComponent::textColourId, juce::Colours::white);
}

void PluginLookAndFeel::drawGroupComponentOutline(juce::Graphics& g, int width, int height,
    const juce::String& text, const juce::Justification& justification,
    juce::GroupComponent& component)
{
    g.setColour(component.findColour(juce::GroupComponent::outlineColourId));
    g.drawRect(0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height), 1.0f);

    if (text.isNotEmpty())
    {
        const int textHeight = 24;
        auto font = juce::Font(textHeight * 0.8f, juce::Font::bold);
        g.setFont(font);
        const float textPadding = 4.0f;
        auto textWidth = static_cast<int>(juce::GlyphArrangement::getStringWidth(font, text) + 2.0f * textPadding);
        g.setColour(component.findColour(juce::GroupComponent::textColourId));
        g.drawFittedText(text, 10, height - 25, textWidth, textHeight, justification, 1);
    }
}

void PluginLookAndFeel::drawCollapsibleHeader(juce::Graphics& g,
    juce::Rectangle<int> bounds,
    const juce::String& title,
    bool isCollapsed,
    bool isVerticalCollapse)
{
    g.setColour(headerBg);
    g.fillRect(bounds);
    g.setColour(juce::Colours::white.withAlpha(0.3f));
    g.drawRect(bounds.toFloat(), 1.0f);

    const int arrowSize = 10;
    const int pad = 8;

    if (isVerticalCollapse)
    {
        juce::Graphics::ScopedSaveState ss(g);
        g.addTransform(juce::AffineTransform::rotation(
            juce::MathConstants<float>::halfPi,
            bounds.getCentreX(),
            bounds.getCentreY()));

        g.setColour(headerText);
        g.setFont(juce::Font(12.0f, juce::Font::bold));
        g.drawText(title,
            bounds.getCentreX() - bounds.getHeight() / 2 + pad,
            bounds.getCentreY() - bounds.getWidth() / 2,
            bounds.getHeight() - arrowSize - pad * 2,
            bounds.getWidth(),
            juce::Justification::centredLeft, true);
    }
    else
    {
        g.setColour(headerText);
        g.setFont(juce::Font(13.0f, juce::Font::bold));
        g.drawText(title,
            bounds.getX() + pad,
            bounds.getY(),
            bounds.getWidth() - arrowSize - pad * 3,
            bounds.getHeight(),
            juce::Justification::centredLeft, true);

        const float cx = bounds.getRight() - pad - arrowSize / 2.0f;
        const float cy = bounds.getCentreY();
        juce::Path arrow;
        if (isCollapsed)
            arrow.addTriangle(cx - arrowSize / 2.0f, cy - arrowSize / 2.0f,
                cx + arrowSize / 2.0f, cy,
                cx - arrowSize / 2.0f, cy + arrowSize / 2.0f);
        else
            arrow.addTriangle(cx - arrowSize / 2.0f, cy - arrowSize / 2.0f,
                cx + arrowSize / 2.0f, cy - arrowSize / 2.0f,
                cx, cy + arrowSize / 2.0f);
        g.setColour(juce::Colours::deeppink);
        g.fillPath(arrow);
    }
}

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
    const int lH = PluginLookAndFeel::labelHeight;
    label->setBounds(x, y, width, lH);
    slider->setBounds(x, y + lH, width, height - lH);
}

PluginLookAndFeel::GridFitResult PluginLookAndFeel::findBestSquareGridFit(
    int nElements, float totalWidth, float totalHeight,
    float minCellSize, float maxCellSize)
{
    GridFitResult best;
    if (nElements <= 0 || totalWidth <= 0.0f || totalHeight <= 0.0f) return best;

    for (int columns = 1; columns <= nElements; ++columns)
    {
        const int rows = (nElements + columns - 1) / columns;
        const float cellWidth = totalWidth / static_cast<float>(columns);
        const float cellHeight = totalHeight / static_cast<float>(rows);
        const float cellSize = std::min(cellWidth, cellHeight);

        if (cellSize >= minCellSize && cellSize <= maxCellSize && cellSize > best.cellSize)
        {
            best.columns = columns;
            best.rows = rows;
            best.cellSize = cellSize;
        }
    }

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

    if (numKnobs <= 0 || availableWidth <= 4 || availableHeight <= 4)
        return result;

    const float contentW = static_cast<float>(availableWidth);
    const float contentH = static_cast<float>(availableHeight);

    int bestCols = numKnobs, bestRows = 1;
    float bestArc = 0.f;

    if (allowWideLayout)
    {
        bestCols = numKnobs;
        bestRows = 1;
    }
    else
    {
        for (int cols = 1; cols <= numKnobs; ++cols)
        {
            const int rows = (numKnobs + cols - 1) / cols;
            const float cellW = contentW / static_cast<float>(cols);
            const float cellH = contentH / static_cast<float>(rows);
            const float arc = std::min(cellW, cellH - static_cast<float>(labelHeight));
            if (arc > bestArc) { bestArc = arc; bestCols = cols; bestRows = rows; }
        }
    }

    const float cellW = contentW / static_cast<float>(bestCols);
    const float cellH = contentH / static_cast<float>(bestRows);

    result.totalWidth = availableWidth;
    result.totalHeight = availableHeight;
    result.knobBounds.reserve(numKnobs);

    for (int i = 0; i < numKnobs; ++i)
    {
        const int col = i % bestCols;
        const int row = i / bestCols;
        const int cellX = static_cast<int>(col * cellW);
        const int cellY = static_cast<int>(row * cellH);
        const int cw = static_cast<int>((col + 1) * cellW) - cellX;
        const int ch = static_cast<int>((row + 1) * cellH) - cellY;
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