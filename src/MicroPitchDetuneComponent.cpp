#include "MicroPitchDetuneComponent.h"

MicroPitchDetuneComponent::MicroPitchDetuneComponent(juce::AudioProcessorValueTreeState& state)
{
    addAndMakeVisible(group);
    UIHelpers::configureGroup(group);

    knobs.emplace_back(std::make_unique<UIHelpers::KnobWithLabel>(state, "detuneAmount", "Detune", *this));
    knobs.emplace_back(std::make_unique<UIHelpers::KnobWithLabel>(state, "lfoRate", "LFO Rate", *this));
    knobs.emplace_back(std::make_unique<UIHelpers::KnobWithLabel>(state, "lfoDepth", "LFO Depth", *this));
    knobs.emplace_back(std::make_unique<UIHelpers::KnobWithLabel>(state, "delayCentre", "Delay", *this));
    knobs.emplace_back(std::make_unique<UIHelpers::KnobWithLabel>(state, "stereoSeparation", "Stereo", *this));
    knobs.emplace_back(std::make_unique<UIHelpers::KnobWithLabel>(state, "mix", "Mix", *this));
}

void MicroPitchDetuneComponent::paint(juce::Graphics& g)
{
    g.fillAll(UIHelpers::Colors::background);
}

void MicroPitchDetuneComponent::resized()
{
    using namespace UIHelpers::Dimensions;

    group.setBounds(getLocalBounds());

    auto area = getLocalBounds().reduced(margin);
    const int availableWidth = area.getWidth();
    const int availableHeight = area.getHeight();
    const int numKnobs = static_cast<int>(knobs.size());

    // Calculate adaptive knob size
    const int maxKnobSize = (availableWidth - margin * (numKnobs + 1)) / numKnobs;
    const int adaptiveKnobSize = juce::jmin(knobSize, maxKnobSize, availableHeight - labelHeight - margin * 2);

    // Check if knobs fit in a single row
    const int requiredWidth = numKnobs * (adaptiveKnobSize + spacing) - spacing + margin * 2;
    const bool useSingleRow = requiredWidth <= availableWidth;

    if (useSingleRow)
    {
        // Single row layout - centered
        const int totalWidth = numKnobs * (adaptiveKnobSize + spacing) - spacing;
        const int startX = area.getX() + (area.getWidth() - totalWidth) / 2;
        const int y = area.getY() + margin;

        for (int i = 0; i < numKnobs; ++i)
        {
            const int x = startX + i * (adaptiveKnobSize + spacing);
            knobs[i]->setBounds(x, y, adaptiveKnobSize, adaptiveKnobSize + labelHeight);
        }
    }
    else
    {
        // Two row layout
        const int knobsPerRow = (numKnobs + 1) / 2;
        const int rowKnobSize = juce::jmin(adaptiveKnobSize, (availableWidth - margin * 2) / knobsPerRow - spacing);
        const int rowHeight = rowKnobSize + labelHeight + margin;

        for (int i = 0; i < numKnobs; ++i)
        {
            const int row = i / knobsPerRow;
            const int col = i % knobsPerRow;
            const int x = area.getX() + margin + col * (rowKnobSize + spacing);
            const int y = area.getY() + margin + row * rowHeight;

            knobs[i]->setBounds(x, y, rowKnobSize, rowKnobSize + labelHeight);
        }
    }
}

void MicroPitchDetuneComponent::setMix(float newValue)
{
    if (knobs.size() > 5) knobs[5]->slider->setValue(newValue);
}

void MicroPitchDetuneComponent::setLfoRate(float newValue)
{
    if (knobs.size() > 1) knobs[1]->slider->setValue(newValue);
}

void MicroPitchDetuneComponent::setLfoDepth(float newValue)
{
    if (knobs.size() > 2) knobs[2]->slider->setValue(newValue);
}

void MicroPitchDetuneComponent::setDelayCentre(float newValue)
{
    if (knobs.size() > 3) knobs[3]->slider->setValue(newValue);
}

void MicroPitchDetuneComponent::setDetuneAmount(float newValue)
{
    if (!knobs.empty()) knobs[0]->slider->setValue(newValue);
}

void MicroPitchDetuneComponent::setStereoSeparation(float newValue)
{
    if (knobs.size() > 4) knobs[4]->slider->setValue(newValue);
}