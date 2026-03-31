#include "ModDelayComponent.h"

ModDelayComponent::ModDelayComponent(juce::AudioProcessorValueTreeState& state)
{
    addAndMakeVisible(group);
    PluginLookAndFeel::configureGroup(group);

    knobs.emplace_back(std::make_unique<PluginLookAndFeel::KnobWithLabel>(state, "delayTime", "Delay", *this));
    knobs.emplace_back(std::make_unique<PluginLookAndFeel::KnobWithLabel>(state, "modDepth", "Depth", *this));
    knobs.emplace_back(std::make_unique<PluginLookAndFeel::KnobWithLabel>(state, "modRate", "Rate", *this));
    knobs.emplace_back(std::make_unique<PluginLookAndFeel::KnobWithLabel>(state, "modMix", "Mix", *this));
    knobs.emplace_back(std::make_unique<PluginLookAndFeel::KnobWithLabel>(state, "feedbackL", "FB L", *this));
    knobs.emplace_back(std::make_unique<PluginLookAndFeel::KnobWithLabel>(state, "feedbackR", "FB R", *this));

    const std::vector<std::pair<juce::String, ModDelay::ModulationType>> waveformData = {
        { "Sin",  ModDelay::ModulationType::Sine        },
        { "Tri",  ModDelay::ModulationType::Triangle    },
        { "Sqr",  ModDelay::ModulationType::Square      },
        { "Sw^",  ModDelay::ModulationType::SawtoothUp  },
        { "Sw_",  ModDelay::ModulationType::SawtoothDown}
    };

    int idx = 0;
    for (auto& [label, type] : waveformData)
    {
        auto* btn = waveformButtons.add(new juce::TextButton(label));
        btn->setColour(juce::TextButton::buttonColourId, juce::Colours::darkgrey);
        btn->setColour(juce::TextButton::textColourOnId, PluginLookAndFeel::labelText);
        btn->setColour(juce::TextButton::textColourOffId, PluginLookAndFeel::labelText.withAlpha(0.7f));
        btn->onClick = [this, idx] { updateWaveformSelection(idx); };
        addAndMakeVisible(btn);
        ++idx;
    }

    hiddenCombo = std::make_unique<juce::ComboBox>();
    modulationTypeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        state, "modulationType", *hiddenCombo);
    hiddenCombo->setVisible(false);
    addAndMakeVisible(*hiddenCombo);

    syncToggle.setButtonText("Sync");
    syncToggle.setColour(juce::ToggleButton::textColourId, PluginLookAndFeel::labelText);
    syncToggle.setColour(juce::ToggleButton::tickColourId, PluginLookAndFeel::track);
    syncAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        state, "sync", syncToggle);
    addAndMakeVisible(syncToggle);

    updateWaveformSelection(0);
}

void ModDelayComponent::paint(juce::Graphics& g)
{
    g.fillAll(PluginLookAndFeel::background);
}

void ModDelayComponent::resized()
{
    if (getWidth() <= 0 || getHeight() <= 0) return;
    group.setBounds(getLocalBounds());

    auto area = getLocalBounds().reduced(PluginLookAndFeel::margin);
    const int totalH = area.getHeight();

    // Button strip height: proportional, min 20px max 30px
    const int btnH = juce::jlimit(20, 30, static_cast<int>(totalH * 0.15f));

    // Distribute waveform buttons + sync toggle across full width
    // 5 waveform buttons + 1 sync toggle = 6 slots, sync gets ~1.4x a button
    const int nBtns = waveformButtons.size();
    const int totalSlots = nBtns * 10 + 14; // relative units: each btn=10, sync=14
    const int availW = area.getWidth();
    const float unitW = static_cast<float>(availW) / static_cast<float>(totalSlots);

    int x = area.getX();
    const int y = area.getY();

    for (int i = 0; i < nBtns; ++i)
    {
        const int bw = (i == nBtns - 1)
            ? (area.getRight() - static_cast<int>(unitW * 14.0f) - x)
            : static_cast<int>(unitW * 10.0f);
        waveformButtons[i]->setBounds(x, y, bw, btnH);
        x += bw;
    }
    syncToggle.setBounds(x, y, area.getRight() - x, btnH);

    // Knob area: everything below the button strip
    const int knobAreaY = y + btnH + PluginLookAndFeel::spacing;
    const int knobAreaH = area.getBottom() - knobAreaY;
    const int numKnobs = static_cast<int>(knobs.size());

    if (knobAreaH > 0)
    {
        auto layout = PluginLookAndFeel::calculateKnobLayout(
            numKnobs, area.getWidth(), knobAreaH, false);

        if ((int)layout.knobBounds.size() < numKnobs) return;

        for (int i = 0; i < numKnobs; ++i)
        {
            auto b = layout.knobBounds[i];
            knobs[i]->setBounds(
                area.getX() + b.getX(),
                knobAreaY + b.getY(),
                b.getWidth(),
                b.getHeight());
        }
    }
}

void ModDelayComponent::updateWaveformSelection(int index)
{
    if (index < 0 || index >= waveformButtons.size()) return;

    selectedWaveform = index;
    for (int i = 0; i < waveformButtons.size(); ++i)
    {
        auto* btn = waveformButtons[i];
        const bool sel = (i == index);
        btn->setColour(juce::TextButton::buttonColourId,
            sel ? PluginLookAndFeel::track : juce::Colours::darkgrey);
        btn->setAlpha(sel ? 1.0f : 0.7f);
    }

    if (hiddenCombo)
        hiddenCombo->setSelectedId(index + 1, juce::sendNotification);
}

void ModDelayComponent::setModulationType(ModDelay::ModulationType type)
{
    const int index = static_cast<int>(type) - 1;
    if (index != selectedWaveform) updateWaveformSelection(index);
}

void ModDelayComponent::setDelayTime(float v) { if (!knobs.empty())       knobs[0]->slider->setValue(v); }
void ModDelayComponent::setModDepth(float v) { if (knobs.size() > 1)     knobs[1]->slider->setValue(v); }
void ModDelayComponent::setModRate(float v) { if (knobs.size() > 2)     knobs[2]->slider->setValue(v); }
void ModDelayComponent::setMix(float v) { if (knobs.size() > 3)     knobs[3]->slider->setValue(v); }
void ModDelayComponent::setFeedbackLeft(float v) { if (knobs.size() > 4)     knobs[4]->slider->setValue(v); }
void ModDelayComponent::setFeedbackRight(float v) { if (knobs.size() > 5)     knobs[5]->slider->setValue(v); }