#include "PluginProcessor.h"
#include "PluginEditor.h"

// Minimum pixel width per knob for row-wrap decisions.
// Intentionally generous so blocks wrap before becoming illegible.
static constexpr int kMinPixelsPerKnob = 50;
static constexpr int kGap = PluginLookAndFeel::margin;

//==============================================================================
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
    addAndMakeVisible(*microPitchDetuneComponent);
    addAndMakeVisible(*exciterSaturationComponent);
    addAndMakeVisible(*simpleVerbComponent);
    addAndMakeVisible(*perceptionModeComponent);

    addAndMakeVisible(modeToggle);
    modeToggle.setButtonText("Show Perception Modes");
    modeToggle.setColour(juce::ToggleButton::textColourId, juce::Colours::white);
    modeToggle.setColour(juce::ToggleButton::tickColourId, juce::Colours::deeppink);
    modeToggle.onClick = [this] { updateUIVisibility(); };
    modeToggle.setToggleState(false, juce::dontSendNotification);

    buildComponentInfoList();

    for (int i = 0; i < (int)componentInfoList.size(); ++i)
    {
        auto btn = std::make_unique<juce::TextButton>();
        btn->setButtonText(componentInfoList[i].label);
        btn->setClickingTogglesState(true);
        btn->setToggleState(true, juce::dontSendNotification);
        btn->setColour(juce::TextButton::buttonColourId, juce::Colour(80, 20, 50));
        btn->setColour(juce::TextButton::buttonOnColourId, juce::Colour(180, 30, 100));
        btn->setColour(juce::TextButton::textColourOffId, juce::Colours::white.withAlpha(0.45f));
        btn->setColour(juce::TextButton::textColourOnId, juce::Colours::white);

        const int idx = i;
        btn->onClick = [this, idx]
            {
                componentInfoList[idx].userVisible = blockToggles[idx]->getToggleState();
                // Always resize window when visibility changes — grow OR shrink
                resizeWindowForVisibleBlocks();
                resized();
            };

        addAndMakeVisible(*btn);
        blockToggles.push_back(std::move(btn));
    }

    setResizable(true, true);
    resizeWindowForVisibleBlocks();
    updateUIVisibility();
}

AudioPluginAudioProcessorEditor::~AudioPluginAudioProcessorEditor()
{
    setLookAndFeel(nullptr);
}

//==============================================================================
void AudioPluginAudioProcessorEditor::buildComponentInfoList()
{
    componentInfoList.clear();

    // minWidth = numKnobs * kMinPixelsPerKnob + margins
    // This is the width below which row-wrapping should occur.
    // It is NOT a render floor — components render fine below this.
    auto add = [&](juce::Component* comp, const juce::String& label, int numKnobs)
        {
            ComponentInfo info;
            info.component = comp;
            info.label = label;
            info.numKnobs = numKnobs;
            info.userVisible = true;
            info.minWidth = numKnobs * kMinPixelsPerKnob + kGap * 2;
            componentInfoList.push_back(info);
        };

    add(widthBalancerComponent.get(), "Width", 3);
    add(tiltEQComponent.get(), "Tilt EQ", 1);
    add(modDelayComponent.get(), "Mod Delay", 6);
    add(spatialFXComponent.get(), "Spatial", 11);
    add(microPitchDetuneComponent.get(), "Pitch", 6);
    add(exciterSaturationComponent.get(), "Exciter", 3);
    add(simpleVerbComponent.get(), "Verb", 4);
}

//==============================================================================
// resizeWindowForVisibleBlocks
//
// Called whenever block visibility changes (toggle on/off).
// Always recalculates and applies preferred size — grows AND shrinks.
//
// Preferred width  = sum of (numKnobs * 90px) for visible blocks + gaps
// Preferred height = auto based on how many rows that implies + overhead
//==============================================================================

void AudioPluginAudioProcessorEditor::resizeWindowForVisibleBlocks()
{
    const int edgePad = kGap * 2;
    const int overhead = 40 + kGap   // mode toggle
        + 28 + kGap   // block button strip
        + edgePad;

    // Gather visible blocks
    std::vector<const ComponentInfo*> vis;
    for (const auto& info : componentInfoList)
        if (info.userVisible) vis.push_back(&info);

    if (vis.empty())
    {
        setResizeLimits(200, overhead + 60, 3000, 2000);
        setSize(400, overhead + 60);
        return;
    }

    // Minimum window width = widest single block's minWidth + edge padding
    int widestMin = 0;
    for (auto* v : vis) widestMin = juce::jmax(widestMin, v->minWidth);
    const int minW = widestMin + edgePad;

    // Minimum window height = one row of the tallest single block + overhead
    const int minH = overhead + 120; // 120px is one usable row

    // Preferred width: aim to fit all visible blocks in ~2 rows.
    // Each knob gets 90px ideal width.
    int totalIdealW = edgePad + kGap * (juce::jmax(0, (int)vis.size() - 1));
    for (auto* v : vis) totalIdealW += v->numKnobs * 90;

    // If they'd all fit in one row at ideal size, use that.
    // Otherwise target ~half on each of two rows.
    // Cap at 90% screen width.
    auto* display = juce::Desktop::getInstance().getDisplays().getPrimaryDisplay();
    const int screenW = display ? static_cast<int>(display->userArea.getWidth() * 0.9f) : 2400;
    const int screenH = display ? static_cast<int>(display->userArea.getHeight() * 0.9f) : 1400;

    int prefW = juce::jlimit(minW, screenW, totalIdealW);

    // Height: simulate how many rows we'd get at prefW and size accordingly
    // Each row gets roughly equal height; aim for ~200px per row of knobs.
    int rowCount = 1;
    {
        int rowW = 0;
        for (int j = 0; j < (int)vis.size(); ++j)
        {
            const int gap = (rowW == 0) ? 0 : kGap;
            if (rowW > 0 && rowW + gap + vis[j]->minWidth > prefW - edgePad)
            {
                ++rowCount;
                rowW = vis[j]->minWidth;
            }
            else
            {
                rowW += gap + vis[j]->minWidth;
            }
        }
    }
    const int prefH = juce::jlimit(minH, screenH, overhead + rowCount * 220);

    setResizeLimits(minW, minH, 3000, 2000);
    setSize(prefW, prefH);
}

//==============================================================================
void AudioPluginAudioProcessorEditor::updateUIVisibility()
{
    const bool perceptionMode = modeToggle.getToggleState();

    for (auto& info : componentInfoList)
        if (info.component) info.component->setVisible(false);

    for (auto& btn : blockToggles)
        btn->setVisible(!perceptionMode);

    perceptionModeComponent->setVisible(perceptionMode);
    modeToggle.setButtonText(perceptionMode ? "Show Manual Controls" : "Show Perception Modes");

    resized();
}

//==============================================================================
void AudioPluginAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(27, 17, 31));
}

//==============================================================================
void AudioPluginAudioProcessorEditor::resized()
{
    auto bounds = getLocalBounds().reduced(kGap);

    modeToggle.setBounds(bounds.removeFromTop(40));
    bounds.removeFromTop(kGap);

    if (modeToggle.getToggleState())
    {
        perceptionModeComponent->setBounds(bounds);
        return;
    }

    // Block visibility button strip — always full width, even splits
    {
        auto strip = bounds.removeFromTop(28);
        bounds.removeFromTop(kGap);

        const int n = (int)blockToggles.size();
        if (n > 0)
        {
            const int totalGaps = kGap * (n - 1);
            const int btnW = (strip.getWidth() - totalGaps) / n;
            int x = strip.getX();
            for (int i = 0; i < n; ++i)
            {
                const int w = (i == n - 1) ? (strip.getRight() - x) : btnW;
                blockToggles[i]->setBounds(x, strip.getY(), w, 28);
                x += w + kGap;
            }
        }
    }

    layoutManualMode(bounds);
}

//==============================================================================
// layoutManualMode — two-pass fluid layout
//
// Pass 1: Row packing using minWidth.
//   Each block declares a minimum width based on its knob count.
//   Blocks are greedily packed into rows: if a block's minWidth fits on
//   the current row (alongside existing blocks + gaps), it goes there.
//   Otherwise it starts a new row. This produces genuine multi-row layout.
//
// Pass 2: Width distribution within each row.
//   Once rows are formed, each row's actual available width is distributed
//   proportionally to numKnobs. The last block in a row fills the remainder
//   exactly to avoid pixel-rounding gaps.
//
// Pass 3: Height distribution across rows.
//   Available height is split proportionally by each row's total weight.
//   Last row fills the remainder.
//
// No auto-hiding. Blocks that are userVisible always render.
// They just get smaller as the window shrinks — calculateKnobLayout handles
// proportional degradation gracefully at any size.
//==============================================================================

void AudioPluginAudioProcessorEditor::layoutManualMode(juce::Rectangle<int> area)
{
    const int availW = area.getWidth();
    const int availH = area.getHeight();

    // Hide everything first
    for (auto& info : componentInfoList)
        if (info.component) info.component->setVisible(false);

    // Collect visible blocks
    std::vector<int> active; // indices into componentInfoList
    for (int i = 0; i < (int)componentInfoList.size(); ++i)
        if (componentInfoList[i].userVisible && componentInfoList[i].component)
            active.push_back(i);

    if (active.empty()) return;

    // ── Pass 1: Row packing by minWidth ──────────────────────────────────────
    struct Row
    {
        std::vector<int> indices; // indices into active[]
        float            weight = 0.f;
    };

    std::vector<Row> rows;
    Row currentRow;
    int currentRowMinW = 0;

    for (int j = 0; j < (int)active.size(); ++j)
    {
        const int ai = active[j];
        const int minW = componentInfoList[ai].minWidth;
        const int gap = currentRow.indices.empty() ? 0 : kGap;
        const int needed = currentRowMinW + gap + minW;

        // Wrap if this block's minimum width doesn't fit on the current row
        if (!currentRow.indices.empty() && needed > availW)
        {
            rows.push_back(currentRow);
            currentRow = {};
            currentRowMinW = 0;
        }

        currentRow.indices.push_back(j);
        currentRow.weight += static_cast<float>(componentInfoList[ai].numKnobs);
        currentRowMinW += (currentRowMinW == 0 ? 0 : kGap) + minW;
    }
    if (!currentRow.indices.empty())
        rows.push_back(currentRow);

    const int rowCount = (int)rows.size();

    // ── Pass 2: Width distribution within each row ────────────────────────────
    // For each row, distribute availW proportionally by numKnobs.
    // Last block in row fills remainder to avoid rounding gaps.
    std::vector<std::vector<int>> resolvedWidths(rowCount);

    for (int r = 0; r < rowCount; ++r)
    {
        const auto& row = rows[r];
        const int   n = (int)row.indices.size();
        const int   gaps = kGap * (n - 1);
        const int   wForBlocks = availW - gaps;

        resolvedWidths[r].resize(n);
        int allocated = 0;

        for (int j = 0; j < n; ++j)
        {
            const int ai = active[row.indices[j]];
            const float share = static_cast<float>(componentInfoList[ai].numKnobs) / row.weight;

            if (j == n - 1)
                resolvedWidths[r][j] = wForBlocks - allocated;
            else
            {
                resolvedWidths[r][j] = static_cast<int>(share * wForBlocks);
                allocated += resolvedWidths[r][j];
            }
        }
    }

    // ── Pass 3: Height distribution across rows ───────────────────────────────
    const int totalRowGaps = kGap * juce::jmax(0, rowCount - 1);
    const int heightForRows = availH - totalRowGaps;

    float totalRowWeight = 0.f;
    for (const auto& row : rows) totalRowWeight += row.weight;

    std::vector<int> rowHeights(rowCount);
    int remainingH = heightForRows;

    for (int r = 0; r < rowCount; ++r)
    {
        if (r == rowCount - 1)
        {
            rowHeights[r] = remainingH;
        }
        else
        {
            rowHeights[r] = static_cast<int>(
                static_cast<float>(heightForRows) * rows[r].weight / totalRowWeight);
            remainingH -= rowHeights[r];
        }
    }

    // ── Apply bounds ──────────────────────────────────────────────────────────
    int y = area.getY();

    for (int r = 0; r < rowCount; ++r)
    {
        const auto& row = rows[r];
        int x = area.getX();

        for (int j = 0; j < (int)row.indices.size(); ++j)
        {
            const int ai = active[row.indices[j]];
            const int bw = resolvedWidths[r][j];
            const int bh = rowHeights[r];

            componentInfoList[ai].component->setBounds(x, y, bw, bh);
            componentInfoList[ai].component->setVisible(true);

            x += bw + kGap;
        }

        y += rowHeights[r] + kGap;
    }
}