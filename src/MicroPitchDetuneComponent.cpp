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
    const int numKnobs = static_cast<int>(knobs.size());

    auto layout = UIHelpers::calculateKnobLayout(numKnobs, area.getWidth(), area.getHeight(), true);

    for (int i = 0; i < numKnobs; ++i)
    {
        auto bounds = layout.knobBounds[i];
        bounds.translate(area.getX(), area.getY());
        knobs[i]->setBounds(bounds.getX(), bounds.getY(), bounds.getWidth(), bounds.getHeight());
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
