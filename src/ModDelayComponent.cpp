#include "ModDelayComponent.h"

ModDelayComponent::ModDelayComponent(juce::AudioProcessorValueTreeState& state)
    : CollapsibleComponent("Motion Shifter")
{
    addAndMakeVisible(group);
    PluginLookAndFeel::configureGroup(group);

    knobs.emplace_back(std::make_unique<PluginLookAndFeel::KnobWithLabel>(state, "delayTime", "Delay", *this));
    knobs.emplace_back(std::make_unique<PluginLookAndFeel::KnobWithLabel>(state, "modDepth", "Depth", *this));
    knobs.emplace_back(std::make_unique<PluginLookAndFeel::KnobWithLabel>(state, "modRate", "Rate", *this));
    knobs.emplace_back(std::make_unique<PluginLookAndFeel::KnobWithLabel>(state, "modMix", "Mix", *this));
    knobs.emplace_back(std::make_unique<PluginLookAndFeel::KnobWithLabel>(state, "feedbackL", "FB L", *this));
    knobs.emplace_back(std::make_unique<PluginLookAndFeel::KnobWithLabel>(state, "feedbackR", "FB R", *this));

    const std::vector<juce::String> waveformLabels = { "Sin", "Tri", "Sqr", "Sw^", "Sw_" };
    waveformPicker = std::make_unique<PluginLookAndFeel::ShapePicker>(state, "modulationType", waveformLabels, *this);

    syncToggle.setButtonText("Sync");
    syncToggle.setColour(juce::ToggleButton::textColourId, PluginLookAndFeel::labelText);
    syncToggle.setColour(juce::ToggleButton::tickColourId, PluginLookAndFeel::track);
    syncAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        state, "sync", syncToggle);
    addAndMakeVisible(syncToggle);
}

void ModDelayComponent::paintContent(juce::Graphics& g)
{
    g.fillAll(PluginLookAndFeel::background);
}

void ModDelayComponent::layoutContent(juce::Rectangle<int> area)
{
    group.setBounds(area);
    auto inner = area.reduced(PluginLookAndFeel::margin);
    const int totalH = inner.getHeight();

    const int btnH = juce::jlimit(20, 30, static_cast<int>(totalH * 0.15f));
    const int syncW = juce::jlimit(50, 70, static_cast<int>(inner.getWidth() * 0.14f));
    const int pickerW = inner.getWidth() - syncW - PluginLookAndFeel::spacing;
    const int y = inner.getY();

    waveformPicker->setBounds(inner.getX(), y, pickerW, btnH);
    syncToggle.setBounds(inner.getX() + pickerW + PluginLookAndFeel::spacing, y, syncW, btnH);

    const int knobAreaY = y + btnH + PluginLookAndFeel::spacing;
    const int knobAreaH = inner.getBottom() - knobAreaY;
    const int numKnobs = static_cast<int>(knobs.size());

    if (knobAreaH > 0)
    {
        auto layout = PluginLookAndFeel::calculateKnobLayout(numKnobs, inner.getWidth(), knobAreaH, false);
        if ((int)layout.knobBounds.size() < numKnobs) return;

        for (int i = 0; i < numKnobs; ++i)
        {
            auto b = layout.knobBounds[i];
            knobs[i]->setBounds(inner.getX() + b.getX(), knobAreaY + b.getY(), b.getWidth(), b.getHeight());
        }
    }
}

void ModDelayComponent::setModulationType(ModDelay::ModulationType type)
{
    if (waveformPicker)
        waveformPicker->setSelected(static_cast<int>(type) - 1);
}

void ModDelayComponent::setDelayTime(float v) { PluginLookAndFeel::setKnobValue(knobs, 0, v); }
void ModDelayComponent::setModDepth(float v) { PluginLookAndFeel::setKnobValue(knobs, 1, v); }
void ModDelayComponent::setModRate(float v) { PluginLookAndFeel::setKnobValue(knobs, 2, v); }
void ModDelayComponent::setMix(float v) { PluginLookAndFeel::setKnobValue(knobs, 3, v); }
void ModDelayComponent::setFeedbackLeft(float v) { PluginLookAndFeel::setKnobValue(knobs, 4, v); }
void ModDelayComponent::setFeedbackRight(float v) { PluginLookAndFeel::setKnobValue(knobs, 5, v); }
void ModDelayComponent::setSyncEnabled(bool shouldSync) { syncToggle.setToggleState(shouldSync, juce::sendNotification); }