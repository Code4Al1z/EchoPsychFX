#include "ExciterSaturationComponent.h"

ExciterSaturationComponent::ExciterSaturationComponent(juce::AudioProcessorValueTreeState& state)
{
    addAndMakeVisible(group);
    UIHelpers::configureGroup(group);

    knobs.emplace_back(std::make_unique<UIHelpers::KnobWithLabel>(state, "exciterDrive", "Drive", *this));
    knobs.emplace_back(std::make_unique<UIHelpers::KnobWithLabel>(state, "exciterMix", "Mix", *this));
    knobs.emplace_back(std::make_unique<UIHelpers::KnobWithLabel>(state, "exciterHighpass", "Highpass", *this));
}

void ExciterSaturationComponent::paint(juce::Graphics& g)
{
    g.fillAll(UIHelpers::Colors::background);
}

void ExciterSaturationComponent::resized()
{
    using namespace UIHelpers::Dimensions;

    group.setBounds(getLocalBounds());

    auto area = getLocalBounds().reduced(margin);
    const int numKnobs = static_cast<int>(knobs.size());
    const int availableWidth = area.getWidth();
    const int availableHeight = area.getHeight();

    // Calculate adaptive knob size
    const int maxKnobWidth = (availableWidth - margin * (numKnobs + 1)) / numKnobs;
    const int adaptiveKnobSize = juce::jmin(knobSize, maxKnobWidth, availableHeight - labelHeight - margin * 2);

    // Center knobs horizontally
    const int totalWidth = numKnobs * (adaptiveKnobSize + spacing) - spacing;
    const int startX = area.getX() + (area.getWidth() - totalWidth) / 2;
    const int y = area.getY() + margin;

    for (int i = 0; i < numKnobs; ++i)
    {
        const int x = startX + i * (adaptiveKnobSize + spacing);
        knobs[i]->setBounds(x, y, adaptiveKnobSize, adaptiveKnobSize + labelHeight);
    }
}

void ExciterSaturationComponent::setDrive(float newValue)
{
    if (!knobs.empty()) knobs[0]->slider->setValue(newValue);
}

void ExciterSaturationComponent::setMix(float newValue)
{
    if (knobs.size() > 1) knobs[1]->slider->setValue(newValue);
}

void ExciterSaturationComponent::setHighpass(float newValue)
{
    if (knobs.size() > 2) knobs[2]->slider->setValue(newValue);
}
