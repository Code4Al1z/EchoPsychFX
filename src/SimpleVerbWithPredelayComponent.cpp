#include "SimpleVerbWithPredelayComponent.h"

SimpleVerbWithPredelayComponent::SimpleVerbWithPredelayComponent(juce::AudioProcessorValueTreeState& state)
{
    addAndMakeVisible(group);
    UIHelpers::configureGroup(group);

    knobs.emplace_back(std::make_unique<UIHelpers::KnobWithLabel>(state, "predelayMs", "Predelay", *this));
    knobs.emplace_back(std::make_unique<UIHelpers::KnobWithLabel>(state, "size", "Size", *this));
    knobs.emplace_back(std::make_unique<UIHelpers::KnobWithLabel>(state, "damping", "Damping", *this));
    knobs.emplace_back(std::make_unique<UIHelpers::KnobWithLabel>(state, "wet", "Wet", *this));
}

void SimpleVerbWithPredelayComponent::paint(juce::Graphics& g)
{
    g.fillAll(UIHelpers::Colors::background);
}

void SimpleVerbWithPredelayComponent::resized()
{
    using namespace UIHelpers::Dimensions;

    group.setBounds(getLocalBounds());

    auto area = getLocalBounds().reduced(margin);
    const int numKnobs = static_cast<int>(knobs.size());
    const int totalWidth = (knobSize * numKnobs) + (spacing * (numKnobs - 1));
    const int startX = area.getX() + (area.getWidth() - totalWidth) / 2;
    const int y = area.getY();

    for (int i = 0; i < numKnobs; ++i)
    {
        const int x = startX + i * (knobSize + spacing);
        knobs[i]->setBounds(x, y, knobSize, knobSize + labelHeight);
    }
}

void SimpleVerbWithPredelayComponent::setPredelay(float newValue)
{
    if (!knobs.empty())
        knobs[0]->slider->setValue(newValue);
}

void SimpleVerbWithPredelayComponent::setSize(float newValue)
{
    if (knobs.size() > 1)
        knobs[1]->slider->setValue(newValue);
}

void SimpleVerbWithPredelayComponent::setDamping(float newValue)
{
    if (knobs.size() > 2)
        knobs[2]->slider->setValue(newValue);
}

void SimpleVerbWithPredelayComponent::setWet(float newValue)
{
    if (knobs.size() > 3)
        knobs[3]->slider->setValue(newValue);
}