#include "ExciterSaturationComponent.h"

ExciterSaturationComponent::ExciterSaturationComponent(juce::AudioProcessorValueTreeState& state)
    : CollapsibleComponent("Exciter Saturation")
{
    addAndMakeVisible(group);
    PluginLookAndFeel::configureGroup(group);

    knobs.emplace_back(std::make_unique<PluginLookAndFeel::KnobWithLabel>(state, "exciterDrive", "Drive", *this));
    knobs.emplace_back(std::make_unique<PluginLookAndFeel::KnobWithLabel>(state, "exciterMix", "Mix", *this));
    knobs.emplace_back(std::make_unique<PluginLookAndFeel::KnobWithLabel>(state, "exciterHighpass", "Highpass", *this));
    knobs.emplace_back(std::make_unique<PluginLookAndFeel::KnobWithLabel>(state, "exciterToneBrightness", "Tone", *this));
    knobs.emplace_back(std::make_unique<PluginLookAndFeel::KnobWithLabel>(state, "exciterHarmonicBalance", "Harmonics", *this));

    // A button-row picker doesn't fit two of them side by side in this panel's width without
    // wrapping into unreadably thin rows, so use compact dropdowns here instead.
    PluginLookAndFeel::configureComboBox(saturationTypeBox);
    for (auto& label : { "Soft", "Hard", "Tube", "Tape", "Xfmr", "Digi" })
        saturationTypeBox.addItem(label, saturationTypeBox.getNumItems() + 1);
    saturationTypeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        state, "exciterSaturationType", saturationTypeBox);
    addAndMakeVisible(saturationTypeBox);

    PluginLookAndFeel::configureComboBox(harmonicModeBox);
    for (auto& label : { "Both", "Odd", "Even" })
        harmonicModeBox.addItem(label, harmonicModeBox.getNumItems() + 1);
    harmonicModeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        state, "exciterHarmonicMode", harmonicModeBox);
    addAndMakeVisible(harmonicModeBox);

    autoGainToggle.setButtonText("Auto Gain");
    autoGainToggle.setColour(juce::ToggleButton::textColourId, PluginLookAndFeel::labelText);
    autoGainToggle.setColour(juce::ToggleButton::tickColourId, PluginLookAndFeel::track);
    autoGainAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        state, "exciterAutoGain", autoGainToggle);
    addAndMakeVisible(autoGainToggle);
}

void ExciterSaturationComponent::paintContent(juce::Graphics& g)
{
    g.fillAll(PluginLookAndFeel::background);
}

void ExciterSaturationComponent::layoutContent(juce::Rectangle<int> area)
{
    group.setBounds(area);
    auto inner = area.reduced(PluginLookAndFeel::margin);

    const int btnH = juce::jlimit(20, 30, static_cast<int>(inner.getHeight() * 0.15f));
    const int toggleW = juce::jlimit(70, 100, static_cast<int>(inner.getWidth() * 0.22f));
    const int pickerW = (inner.getWidth() - toggleW - PluginLookAndFeel::spacing * 2) / 2;
    const int y = inner.getY();

    saturationTypeBox.setBounds(inner.getX(), y, pickerW, btnH);
    harmonicModeBox.setBounds(inner.getX() + pickerW + PluginLookAndFeel::spacing, y, pickerW, btnH);
    autoGainToggle.setBounds(inner.getX() + 2 * pickerW + PluginLookAndFeel::spacing * 2, y, toggleW, btnH);

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

void ExciterSaturationComponent::setDrive(float v) { PluginLookAndFeel::setKnobValue(knobs, 0, v); }
void ExciterSaturationComponent::setMix(float v) { PluginLookAndFeel::setKnobValue(knobs, 1, v); }
void ExciterSaturationComponent::setHighpass(float v) { PluginLookAndFeel::setKnobValue(knobs, 2, v); }
void ExciterSaturationComponent::setToneBrightness(float v) { PluginLookAndFeel::setKnobValue(knobs, 3, v); }
void ExciterSaturationComponent::setHarmonicBalance(float v) { PluginLookAndFeel::setKnobValue(knobs, 4, v); }
void ExciterSaturationComponent::setSaturationType(int index)
{
    saturationTypeBox.setSelectedId(index + 1, juce::sendNotification);
}
void ExciterSaturationComponent::setHarmonicMode(int index)
{
    harmonicModeBox.setSelectedId(index + 1, juce::sendNotification);
}
void ExciterSaturationComponent::setAutoGain(bool enabled) { autoGainToggle.setToggleState(enabled, juce::sendNotification); }