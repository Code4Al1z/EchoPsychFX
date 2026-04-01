#include "PluginProcessor.h"
#include "PluginEditor.h"

static constexpr int kMinPixelsPerKnob = 50;
static constexpr int kGap = PluginLookAndFeel::margin;

//==============================================================================
AudioPluginAudioProcessorEditor::AudioPluginAudioProcessorEditor(AudioPluginAudioProcessor& p)
    : AudioProcessorEditor(&p), processorRef(p)
{
    inputControlsComponent = std::make_unique<InputControlsComponent>(p.parameters);
    modDelayComponent = std::make_unique<ModDelayComponent>(p.parameters);
    spatialFXComponent = std::make_unique<SpatialFXComponent>(p.parameters);
    microPitchDetuneComponent = std::make_unique<MicroPitchDetuneComponent>(p.parameters);
    exciterSaturationComponent = std::make_unique<ExciterSaturationComponent>(p.parameters);
    simpleVerbComponent = std::make_unique<SimpleVerbWithPredelayComponent>(p.parameters);

    // PerceptionPresetManager still receives individual component references
    // accessed through the InputControlsComponent wrapper
    presetManager = std::make_unique<PerceptionPresetManager>(
        inputControlsComponent->getTiltEQ(),
        inputControlsComponent->getWidthBalancer(),
        *modDelayComponent,
        *spatialFXComponent,
        *microPitchDetuneComponent,
        *exciterSaturationComponent,
        *simpleVerbComponent);

    perceptionModeComponent = std::make_unique<PerceptionModeComponent>(*presetManager);

    setLookAndFeel(&pluginLookAndFeel);

    addAndMakeVisible(*inputControlsComponent);
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
                // If the user is turning a block back on, clear its auto-hidden state
                // so the layout doesn't immediately re-hide it before the window grows
                if (componentInfoList[idx].userVisible)
                    componentInfoList[idx].autoHidden = false;
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

    // minWidth  = numKnobs * kMinPixelsPerKnob + edge gaps
    // minHeight = enough for one row of knobs at minimum size + label + group label
    //             = kMinPixelsPerKnob (knob arc) + 16 (label) + 16 (textbox) + margins
    static constexpr int kMinKnobH = kMinPixelsPerKnob + 16 + 16 + kGap * 2 + 24; // 24 = group label

    auto add = [&](juce::Component* comp, const juce::String& label,
        int numKnobs, int overrideMinH = 0)
        {
            ComponentInfo info;
            info.component = comp;
            info.label = label;
            info.numKnobs = numKnobs;
            info.userVisible = true;
            info.autoHidden = false;
            info.minWidth = numKnobs * kMinPixelsPerKnob + kGap * 2;
            info.minHeight = overrideMinH > 0 ? overrideMinH : kMinKnobH;
            componentInfoList.push_back(info);
        };

    // Input Controls has two stacked sections — needs more min height
    add(inputControlsComponent.get(), "Input", 4, kMinKnobH + 60);
    add(modDelayComponent.get(), "Mod Delay", 6);
    add(spatialFXComponent.get(), "Spatial", 11);
    add(microPitchDetuneComponent.get(), "Pitch", 6);
    add(exciterSaturationComponent.get(), "Exciter", 3);
    add(simpleVerbComponent.get(), "Verb", 4);
}

//==============================================================================
void AudioPluginAudioProcessorEditor::resizeWindowForVisibleBlocks()
{
    const int edgePad = kGap * 2;
    const int overhead = 40 + kGap + 28 + kGap + edgePad;

    // Only count blocks the user wants visible (not auto-hidden — that's layout's job)
    std::vector<const ComponentInfo*> vis;
    for (const auto& info : componentInfoList)
        if (info.userVisible) vis.push_back(&info);

    if (vis.empty())
    {
        setResizeLimits(200, overhead + 60, 3000, 2000);
        setSize(400, overhead + 60);
        return;
    }

    // ── Minimum window size ───────────────────────────────────────────────────
    // Width: widest single block's minWidth (at minimum, one block must fit per row)
    int widestMin = 0;
    for (auto* v : vis) widestMin = juce::jmax(widestMin, v->minWidth);
    const int minW = widestMin + edgePad;

    // Height: tallest single block's minHeight + overhead (one block always fits)
    int tallestMin = 0;
    for (auto* v : vis) tallestMin = juce::jmax(tallestMin, v->minHeight);
    const int minH = overhead + tallestMin;

    // ── Preferred window size ─────────────────────────────────────────────────
    // Width: all blocks side by side at 90px per knob
    int totalIdealW = edgePad + kGap * juce::jmax(0, (int)vis.size() - 1);
    for (auto* v : vis) totalIdealW += v->numKnobs * 90;

    auto* display = juce::Desktop::getInstance().getDisplays().getPrimaryDisplay();
    const int screenW = display ? static_cast<int>(display->userArea.getWidth() * 0.9f) : 2400;
    const int screenH = display ? static_cast<int>(display->userArea.getHeight() * 0.9f) : 1400;

    int prefW = juce::jlimit(minW, screenW, totalIdealW);

    // Simulate row count at prefW to estimate preferred height
    int rowCount = 1;
    {
        int rowW = 0;
        for (auto* v : vis)
        {
            const int gap = (rowW == 0) ? 0 : kGap;
            if (rowW > 0 && rowW + gap + v->minWidth > prefW - edgePad)
            {
                ++rowCount;
                rowW = v->minWidth;
            }
            else
            {
                rowW += gap + v->minWidth;
            }
        }
    }

    // Height: 220px per row, but at least minHeight per row
    int prefH = overhead;
    for (int r = 0; r < rowCount; ++r)
        prefH += 220 + (r > 0 ? kGap : 0);
    prefH = juce::jlimit(minH, screenH, prefH);

    setResizeLimits(minW, minH, 3000, 2000);

    // Always apply preferred size when blocks change — grows AND shrinks
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
void AudioPluginAudioProcessorEditor::layoutManualMode(juce::Rectangle<int> area)
{
    const int availW = area.getWidth();
    const int availH = area.getHeight();

    // Reset auto-hidden state from previous layout pass
    for (auto& info : componentInfoList)
        info.autoHidden = false;

    // Up to 2 passes: pass 0 = try with all userVisible blocks,
    // pass 1 = retry after marking undersized blocks as autoHidden
    for (int pass = 0; pass < 2; ++pass)
    {
        // Hide all first
        for (auto& info : componentInfoList)
            if (info.component) info.component->setVisible(false);

        // Collect active blocks (userVisible and not autoHidden)
        std::vector<int> active;
        for (int i = 0; i < (int)componentInfoList.size(); ++i)
            if (componentInfoList[i].userVisible && !componentInfoList[i].autoHidden
                && componentInfoList[i].component)
                active.push_back(i);

        if (active.empty()) break;

        // ── Pass 1: Row packing by minWidth ──────────────────────────────────
        struct Row { std::vector<int> indices; float weight = 0.f; };
        std::vector<Row> rows;
        Row currentRow;
        int currentRowMinW = 0;

        for (int j = 0; j < (int)active.size(); ++j)
        {
            const int ai = active[j];
            const int minW = componentInfoList[ai].minWidth;
            const int gap = currentRow.indices.empty() ? 0 : kGap;

            if (!currentRow.indices.empty() && currentRowMinW + gap + minW > availW)
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

        // ── Pass 2: Width distribution ────────────────────────────────────────
        std::vector<std::vector<int>> resolvedWidths(rowCount);
        for (int r = 0; r < rowCount; ++r)
        {
            const auto& row = rows[r];
            const int   n = (int)row.indices.size();
            const int   wForBlocks = availW - kGap * (n - 1);
            resolvedWidths[r].resize(n);
            int allocated = 0;
            for (int j = 0; j < n; ++j)
            {
                const int   ai = active[row.indices[j]];
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

        // ── Pass 3: Height distribution ───────────────────────────────────────
        const int heightForRows = availH - kGap * juce::jmax(0, rowCount - 1);
        float totalRowWeight = 0.f;
        for (const auto& row : rows) totalRowWeight += row.weight;

        std::vector<int> rowHeights(rowCount);
        int remainingH = heightForRows;
        for (int r = 0; r < rowCount; ++r)
        {
            if (r == rowCount - 1)
                rowHeights[r] = remainingH;
            else
            {
                rowHeights[r] = static_cast<int>(
                    static_cast<float>(heightForRows) * rows[r].weight / totalRowWeight);
                remainingH -= rowHeights[r];
            }
        }

        // ── Check minimums — auto-hide any block that's too small ─────────────
        bool anyNewlyHidden = false;
        for (int r = 0; r < rowCount; ++r)
        {
            for (int j = 0; j < (int)rows[r].indices.size(); ++j)
            {
                const int ai = active[rows[r].indices[j]];
                const int bw = resolvedWidths[r][j];
                const int bh = rowHeights[r];

                if (bw < componentInfoList[ai].minWidth ||
                    bh < componentInfoList[ai].minHeight)
                {
                    componentInfoList[ai].autoHidden = true;
                    anyNewlyHidden = true;
                }
            }
        }

        // If we hid something new on pass 0, redo the layout without those blocks
        if (anyNewlyHidden && pass == 0)
            continue;

        // ── Apply bounds ──────────────────────────────────────────────────────
        int y = area.getY();
        for (int r = 0; r < rowCount; ++r)
        {
            const auto& row = rows[r];
            int x = area.getX();
            for (int j = 0; j < (int)row.indices.size(); ++j)
            {
                const int ai = active[row.indices[j]];
                componentInfoList[ai].component->setBounds(
                    x, y, resolvedWidths[r][j], rowHeights[r]);
                componentInfoList[ai].component->setVisible(true);
                x += resolvedWidths[r][j] + kGap;
            }
            y += rowHeights[r] + kGap;
        }
        break;
    }

    // ── Sync toggle button appearance ─────────────────────────────────────────
    // Dim buttons for auto-hidden blocks; restore full opacity for visible ones
    for (int i = 0; i < (int)componentInfoList.size(); ++i)
    {
        const bool hidden = componentInfoList[i].autoHidden;
        blockToggles[i]->setAlpha(hidden ? 0.4f : 1.0f);
        // Keep the toggle in its user state — don't change getToggleState()
    }

    // ── Update resize limits to reflect what's currently rendered ─────────────
    // After auto-hiding, the true minimum window size may be smaller.
    // Recalculate based on what's actually visible now.
    {
        const int edgePad = kGap * 2;
        const int overhead = 40 + kGap + 28 + kGap + edgePad;

        int minW = 0, minH = 0;
        for (const auto& info : componentInfoList)
        {
            if (info.userVisible && !info.autoHidden)
            {
                minW = juce::jmax(minW, info.minWidth);
                minH = juce::jmax(minH, info.minHeight);
            }
        }
        if (minW == 0) minW = 200;
        if (minH == 0) minH = 120;

        setResizeLimits(minW + edgePad, minH + overhead, 3000, 2000);
    }
}