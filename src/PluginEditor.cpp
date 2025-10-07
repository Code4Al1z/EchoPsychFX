#include "PluginProcessor.h"
#include "PluginEditor.h"

AudioPluginAudioProcessorEditor::AudioPluginAudioProcessorEditor(AudioPluginAudioProcessor& p)
    : AudioProcessorEditor(&p), processorRef(p)
{
    widthBalancerComponent = std::make_unique<WidthBalancerComponent>(p.parameters);
    tiltEQComponent = std::make_unique<TiltEQComponent>(p.parameters);
    modDelayComponent = std::make_unique<ModDelayComponent>(p.parameters);
    spatialFXComponent = std::make_unique<SpatialFXComponent>(p.parameters);
    microPitchDetuneComponent = std::make_unique<MicroPitchDetuneComponent>(p.parameters);
    exciterSaturationComponent = std::make_unique<ExciterSaturationComponent>(p.parameters);
    simpleVerbComponent = std::make_unique<SimpleVerbWithPredelayComponent>(p.parameters);

    presetManager = std::make_unique<PerceptionPresetManager>(
        *tiltEQComponent, *widthBalancerComponent, *modDelayComponent,
        *spatialFXComponent, *microPitchDetuneComponent, *exciterSaturationComponent,
        *simpleVerbComponent);

    perceptionModeComponent = std::make_unique<PerceptionModeComponent>(*presetManager);

    setLookAndFeel(&pluginLookAndFeel);

    addAndMakeVisible(*widthBalancerComponent);
    addAndMakeVisible(*tiltEQComponent);
    addAndMakeVisible(*modDelayComponent);
    addAndMakeVisible(*spatialFXComponent);
    addAndMakeVisible(*perceptionModeComponent);
    addAndMakeVisible(*microPitchDetuneComponent);
    addAndMakeVisible(*exciterSaturationComponent);
    addAndMakeVisible(*simpleVerbComponent);

    addAndMakeVisible(modeToggle);
    modeToggle.setButtonText("Show Perception Modes");
    modeToggle.setColour(juce::ToggleButton::textColourId, juce::Colours::white);
    modeToggle.setColour(juce::ToggleButton::tickColourId, juce::Colours::deeppink);
    modeToggle.onClick = [this]() { updateUIVisibility(); };
    modeToggle.setToggleState(false, juce::dontSendNotification);

    setResizable(true, true);

    calculateMinMaxSizes();
    setResizeLimits(calculatedMinWidth, calculatedMinHeight, calculatedMaxWidth, calculatedMaxHeight);

    // Set initial size to be reasonable (not too small, not max)
    int initialWidth = juce::jlimit(calculatedMinWidth, calculatedMaxWidth, 1200);
    int initialHeight = juce::jlimit(calculatedMinHeight, calculatedMaxHeight, 700);
    setSize(initialWidth, initialHeight);

    updateUIVisibility();
}

AudioPluginAudioProcessorEditor::~AudioPluginAudioProcessorEditor()
{
    setLookAndFeel(nullptr);
}

int AudioPluginAudioProcessorEditor::calculateComponentMinWidth(int numKnobs, bool allowWideLayout) const
{
    if (allowWideLayout)
    {
        return numKnobs * (PluginLookAndFeel::minKnobSize + PluginLookAndFeel::spacing) + PluginLookAndFeel::margin * 2;
    }
    else
    {
        int cols = static_cast<int>(std::ceil(std::sqrt(static_cast<float>(numKnobs))));
        return cols * (PluginLookAndFeel::minKnobSize + PluginLookAndFeel::spacing) + PluginLookAndFeel::margin * 2;
    }
}

int AudioPluginAudioProcessorEditor::calculateComponentMinHeight(int numKnobs, bool allowWideLayout) const
{
    if (allowWideLayout)
    {
        return PluginLookAndFeel::minKnobSize + PluginLookAndFeel::labelHeight + PluginLookAndFeel::margin * 2 + PluginLookAndFeel::groupLabelHeight;
    }
    else
    {
        int cols = static_cast<int>(std::ceil(std::sqrt(static_cast<float>(numKnobs))));
        int rows = (numKnobs + cols - 1) / cols;
        return rows * (PluginLookAndFeel::minKnobSize + PluginLookAndFeel::labelHeight + PluginLookAndFeel::spacing) + PluginLookAndFeel::margin * 2 + PluginLookAndFeel::groupLabelHeight;
    }
}

int AudioPluginAudioProcessorEditor::calculateComponentMaxWidth(int numKnobs, bool allowWideLayout) const
{
    if (allowWideLayout)
    {
        return numKnobs * (PluginLookAndFeel::maxKnobSize + PluginLookAndFeel::spacing) + PluginLookAndFeel::margin * 2;
    }
    else
    {
        int cols = static_cast<int>(std::ceil(std::sqrt(static_cast<float>(numKnobs))));
        return cols * (PluginLookAndFeel::maxKnobSize + PluginLookAndFeel::spacing) + PluginLookAndFeel::margin * 2;
    }
}

int AudioPluginAudioProcessorEditor::calculateComponentMaxHeight(int numKnobs, bool allowWideLayout) const
{
    if (allowWideLayout)
    {
        // For wide layouts, allow 2 rows maximum for reflow
        return (PluginLookAndFeel::maxKnobSize + PluginLookAndFeel::labelHeight) * 2 +
            PluginLookAndFeel::spacing + PluginLookAndFeel::margin * 2 +
            PluginLookAndFeel::groupLabelHeight;
    }
    else
    {
        // For grid layouts, calculate based on optimal grid size
        int cols = static_cast<int>(std::ceil(std::sqrt(static_cast<float>(numKnobs))));
        int rows = (numKnobs + cols - 1) / cols;

        // Cap at maximum of 3 rows to keep components compact
        rows = juce::jmin(rows, 3);

        return rows * (PluginLookAndFeel::maxKnobSize + PluginLookAndFeel::labelHeight) +
            (rows - 1) * PluginLookAndFeel::spacing +
            PluginLookAndFeel::margin * 2 +
            PluginLookAndFeel::groupLabelHeight;
    }
}

void AudioPluginAudioProcessorEditor::calculateMinMaxSizes()
{
    auto components = getComponentInfoList();

    // Calculate minimum width (largest single component min width)
    int maxMinWidth = 0;
    for (const auto& info : components)
    {
        maxMinWidth = juce::jmax(maxMinWidth, info.minWidth);
    }

    // Calculate minimum height - just the smallest component's min height
    // This allows much more flexibility in resizing
    int smallestMinHeight = components.empty() ? 200 : components[0].minHeight;
    for (const auto& info : components)
    {
        smallestMinHeight = juce::jmin(smallestMinHeight, info.minHeight);
    }

    calculatedMinWidth = maxMinWidth + PluginLookAndFeel::margin * 2;
    calculatedMinHeight = smallestMinHeight + PluginLookAndFeel::margin * 2 + 40; // Just one component + toggle

    // Max width: allow components to be side-by-side
    int totalMaxWidth = 0;
    for (const auto& info : components)
    {
        totalMaxWidth += info.maxWidth;
    }
    totalMaxWidth += PluginLookAndFeel::margin * (static_cast<int>(components.size()) + 1);

    // Get screen dimensions to respect user's display
    auto displays = juce::Desktop::getInstance().getDisplays();
    auto* primaryDisplay = displays.getPrimaryDisplay();
    if (primaryDisplay != nullptr)
    {
        auto screenArea = primaryDisplay->userArea;
        int maxScreenHeight = static_cast<int>(screenArea.getHeight() * 0.9f); // 90% of screen height
        int maxScreenWidth = static_cast<int>(screenArea.getWidth() * 0.9f);   // 90% of screen width

        // Max height should be all components stacked, or screen height, whichever is smaller
        int totalStackedHeight = 0;
        for (const auto& info : components)
        {
            totalStackedHeight += info.maxHeight;
        }
        totalStackedHeight += PluginLookAndFeel::margin * static_cast<int>(components.size()) + 40;

        calculatedMaxHeight = juce::jmin(maxScreenHeight, totalStackedHeight);
        calculatedMaxWidth = juce::jmin(maxScreenWidth, totalMaxWidth);
    }
    else
    {
        // Fallback if we can't get screen dimensions
        int totalStackedHeight = 0;
        for (const auto& info : components)
        {
            totalStackedHeight += info.maxHeight;
        }
        totalStackedHeight += PluginLookAndFeel::margin * static_cast<int>(components.size()) + 40;

        calculatedMaxWidth = juce::jmin(2400, totalMaxWidth);
        calculatedMaxHeight = juce::jmin(1400, totalStackedHeight);
    }
}

std::vector<AudioPluginAudioProcessorEditor::ComponentInfo> AudioPluginAudioProcessorEditor::getComponentInfoList()
{
    std::vector<ComponentInfo> infos;

    auto addComponentInfo = [&](juce::Component* comp, int numKnobs, bool allowWideLayout) {
        ComponentInfo info;
        info.component = comp;
        info.numKnobs = numKnobs;
        info.allowWideLayout = allowWideLayout;

        info.minWidth = calculateComponentMinWidth(numKnobs, allowWideLayout);
        info.minHeight = calculateComponentMinHeight(numKnobs, allowWideLayout);
        info.maxWidth = calculateComponentMaxWidth(numKnobs, allowWideLayout);
        info.maxHeight = calculateComponentMaxHeight(numKnobs, allowWideLayout);

        info.sizeWeight = static_cast<float>(numKnobs);

        infos.push_back(info);
        };

    addComponentInfo(widthBalancerComponent.get(), 2, false);
    addComponentInfo(tiltEQComponent.get(), 1, false);
    addComponentInfo(modDelayComponent.get(), 6, true);
    addComponentInfo(spatialFXComponent.get(), 11, false);
    addComponentInfo(microPitchDetuneComponent.get(), 6, true);
    addComponentInfo(exciterSaturationComponent.get(), 3, false);
    addComponentInfo(simpleVerbComponent.get(), 4, false);

    return infos;
}

void AudioPluginAudioProcessorEditor::updateUIVisibility()
{
    const bool perceptionMode = modeToggle.getToggleState();

    widthBalancerComponent->setVisible(!perceptionMode);
    tiltEQComponent->setVisible(!perceptionMode);
    modDelayComponent->setVisible(!perceptionMode);
    spatialFXComponent->setVisible(!perceptionMode);
    microPitchDetuneComponent->setVisible(!perceptionMode);
    exciterSaturationComponent->setVisible(!perceptionMode);
    simpleVerbComponent->setVisible(!perceptionMode);

    perceptionModeComponent->setVisible(perceptionMode);

    modeToggle.setButtonText(perceptionMode ? "Show Manual Controls" : "Show Perception Modes");

    resized();
}

void AudioPluginAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(27, 17, 31));
}

void AudioPluginAudioProcessorEditor::layoutManualMode(juce::Rectangle<int> area)
{
    auto components = getComponentInfoList();
    const int availableWidth = area.getWidth();
    const int availableHeight = area.getHeight();
    const int spacing = PluginLookAndFeel::margin;

    // Step 1: Determine which components fit in each row
    std::vector<std::vector<int>> rows;
    std::vector<int> currentRow;
    int currentRowWidth = 0;

    for (int i = 0; i < static_cast<int>(components.size()); ++i)
    {
        const auto& info = components[i];
        int neededWidth = info.minWidth;

        if (currentRow.empty())
        {
            currentRow.push_back(i);
            currentRowWidth = neededWidth;
        }
        else
        {
            int potentialWidth = currentRowWidth + spacing + neededWidth;

            if (potentialWidth <= availableWidth)
            {
                currentRow.push_back(i);
                currentRowWidth = potentialWidth;
            }
            else
            {
                rows.push_back(currentRow);
                currentRow.clear();
                currentRow.push_back(i);
                currentRowWidth = neededWidth;
            }
        }
    }

    if (!currentRow.empty())
    {
        rows.push_back(currentRow);
    }

    // Step 2: Calculate row heights to FILL the entire available height
    int totalSpacing = spacing * juce::jmax(0, static_cast<int>(rows.size()) - 1);
    int heightForComponents = availableHeight - totalSpacing;

    std::vector<int> rowHeights;
    std::vector<float> rowWeights;
    std::vector<int> rowMinHeights;
    std::vector<int> rowMaxHeights;
    float totalWeight = 0.0f;

    // Calculate weight and min/max height for each row
    for (const auto& row : rows)
    {
        float rowWeight = 0.0f;
        int minHeight = 0;
        int maxHeight = 0;

        for (int compIdx : row)
        {
            const auto& info = components[compIdx];
            rowWeight += info.sizeWeight;
            minHeight = juce::jmax(minHeight, info.minHeight);
            maxHeight = juce::jmax(maxHeight, info.maxHeight);
        }

        rowWeights.push_back(rowWeight);
        rowMinHeights.push_back(minHeight);
        rowMaxHeights.push_back(maxHeight);
        totalWeight += rowWeight;
    }

    // Calculate total min height needed
    int totalMinHeight = 0;
    for (int h : rowMinHeights)
        totalMinHeight += h;

    // Distribute height based on weights, but respect min/max constraints
    int remainingHeight = heightForComponents;
    for (int i = 0; i < static_cast<int>(rows.size()); ++i)
    {
        int rowHeight;

        if (i == static_cast<int>(rows.size()) - 1)
        {
            // Last row gets all remaining height
            rowHeight = juce::jlimit(rowMinHeights[i], rowMaxHeights[i], remainingHeight);
        }
        else
        {
            // Distribute proportionally based on weight
            float weightRatio = rowWeights[i] / totalWeight;
            int idealHeight = static_cast<int>(heightForComponents * weightRatio);

            // Clamp to min/max
            rowHeight = juce::jlimit(rowMinHeights[i], rowMaxHeights[i], idealHeight);
            remainingHeight -= rowHeight;
        }

        rowHeights.push_back(rowHeight);
    }

    // Step 3: Layout each row
    int y = area.getY();

    for (int rowIdx = 0; rowIdx < static_cast<int>(rows.size()); ++rowIdx)
    {
        const auto& row = rows[rowIdx];
        int rowHeight = rowHeights[rowIdx];

        // Calculate total weight and min width for this row
        float totalWeightInRow = 0.0f;
        int totalMinWidthInRow = 0;

        for (int compIdx : row)
        {
            const auto& info = components[compIdx];
            totalMinWidthInRow += info.minWidth;
            totalWeightInRow += info.sizeWeight;
        }

        totalMinWidthInRow += spacing * juce::jmax(0, static_cast<int>(row.size()) - 1);
        int extraWidth = juce::jmax(0, availableWidth - totalMinWidthInRow);

        // Layout components in this row to FILL the width
        int x = area.getX();
        int remainingWidth = availableWidth;

        for (int idx = 0; idx < static_cast<int>(row.size()); ++idx)
        {
            int compIdx = row[idx];
            const auto& info = components[compIdx];

            int compWidth;
            if (idx == static_cast<int>(row.size()) - 1)
            {
                // Last component in row gets all remaining width
                compWidth = remainingWidth;
            }
            else
            {
                // Calculate component width based on weight
                compWidth = info.minWidth;
                if (extraWidth > 0 && totalWeightInRow > 0.0f)
                {
                    float weightRatio = info.sizeWeight / totalWeightInRow;
                    int additionalWidth = static_cast<int>(extraWidth * weightRatio);
                    compWidth += additionalWidth;
                }
                compWidth = juce::jlimit(info.minWidth, info.maxWidth, compWidth);
            }

            // Set bounds - components will resize and recalculate their internal layout
            info.component->setBounds(x, y, compWidth, rowHeight);

            x += compWidth + spacing;
            remainingWidth -= (compWidth + spacing);
        }

        y += rowHeight + spacing;
    }
}

void AudioPluginAudioProcessorEditor::layoutPerceptionMode(juce::Rectangle<int> area)
{
    perceptionModeComponent->setBounds(area);
}

void AudioPluginAudioProcessorEditor::resized()
{
    auto bounds = getLocalBounds();
    bounds.reduce(PluginLookAndFeel::margin, PluginLookAndFeel::margin);

    const int toggleHeight = 40;
    modeToggle.setBounds(bounds.removeFromTop(toggleHeight));
    bounds.removeFromTop(PluginLookAndFeel::margin);

    const bool perceptionMode = modeToggle.getToggleState();

    if (perceptionMode)
    {
        layoutPerceptionMode(bounds);
    }
    else
    {
        layoutManualMode(bounds);
    }
}

int AudioPluginAudioProcessorEditor::calculateActualContentHeight()
{
    auto components = getComponentInfoList();
    const int availableWidth = getWidth() - PluginLookAndFeel::margin * 2;
    const int spacing = PluginLookAndFeel::margin;

    // Simulate the layout to calculate actual height needed
    std::vector<std::vector<int>> rows;
    std::vector<int> currentRow;
    int currentRowWidth = 0;

    for (int i = 0; i < static_cast<int>(components.size()); ++i)
    {
        const auto& info = components[i];
        int neededWidth = info.minWidth;

        if (currentRow.empty())
        {
            currentRow.push_back(i);
            currentRowWidth = neededWidth;
        }
        else
        {
            int potentialWidth = currentRowWidth + spacing + neededWidth;

            if (potentialWidth <= availableWidth)
            {
                currentRow.push_back(i);
                currentRowWidth = potentialWidth;
            }
            else
            {
                rows.push_back(currentRow);
                currentRow.clear();
                currentRow.push_back(i);
                currentRowWidth = neededWidth;
            }
        }
    }

    if (!currentRow.empty())
    {
        rows.push_back(currentRow);
    }

    // Calculate minimum height for each row
    int totalHeight = 0;
    for (const auto& row : rows)
    {
        int maxHeightInRow = 0;
        for (int compIdx : row)
        {
            const auto& info = components[compIdx];
            maxHeightInRow = juce::jmax(maxHeightInRow, info.minHeight);
        }
        totalHeight += maxHeightInRow;
    }

    // Add spacing and margins
    totalHeight += spacing * juce::jmax(0, static_cast<int>(rows.size()) - 1);
    totalHeight += PluginLookAndFeel::margin * 2 + 40; // margins + toggle

    return totalHeight;
}