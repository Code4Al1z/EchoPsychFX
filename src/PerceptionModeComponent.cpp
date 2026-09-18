#include "PerceptionModeComponent.h"
#include "PluginLookAndFeel.h"

PerceptionModeComponent::PerceptionModeComponent(PerceptionPresetManager& presetManager)
    : presetManagerRef(presetManager)
{
    PluginLookAndFeel::configureLabel(titleLabel, "Perception Mode");
    titleLabel.setFont(juce::Font(20.0f, juce::Font::bold));
    addAndMakeVisible(titleLabel);

    // Add all preset names
    const juce::StringArray presetNames = {
        "Head Trip", "Panic Room", "Intimacy", "Blade Runner", "Alien Abduction",
        "Glass Tunnel", "Dream Logic", "Womb Space", "Bipolar Bloom", "Quiet Confidence",
        "Falling Upwards", "Molten Light", "Ethereal Echo", "Lush Dreamscape", "Skin Contact",
        "Sonic Embrace", "Strobe Heaven", "Glass Flame", "Celestial Vault", "Deep Illusion",
        "Ego Dissolve", "Memory Dust", "Gentle Slap", "Moon Dance", "Biting Lips",
        "Stormy Day", "Summer Sunset", "Ocean Waves", "Crystal Clear", "Sweetest Memory"
    };

    for (int i = 0; i < presetNames.size(); ++i)
        presetSelector.addItem(presetNames[i], i + 1);

    PluginLookAndFeel::configureComboBox(presetSelector);

    presetSelector.onChange = [this]() { comboBoxChanged(&presetSelector); };
    addAndMakeVisible(presetSelector);
}

PerceptionModeComponent::~PerceptionModeComponent()
{
}

void PerceptionModeComponent::resized()
{
    auto area = getLocalBounds().reduced(10);

    titleLabel.setBounds(area.removeFromTop(30));
    area.removeFromTop(10); // spacing

    presetSelector.setBounds(area.removeFromTop(30));
}

void PerceptionModeComponent::comboBoxChanged(juce::ComboBox* comboBoxThatHasChanged)
{
    if (comboBoxThatHasChanged == &presetSelector)
    {
        const auto selectedName = presetSelector.getText();
        DBG("Selected preset: " + selectedName);
        presetManagerRef.applyPreset(selectedName);
    }
}