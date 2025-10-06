#include "MicroPitchDetuneComponent.h"

MicroPitchDetuneComponent::MicroPitchDetuneComponent(juce::AudioProcessorValueTreeState& state)
{
    addAndMakeVisible(group);
    UIHelpers::configureGroup(group);  // USE UIHelpers

    // Much simpler than before!
    knobs.emplace_back(std::make_unique<UIHelpers::KnobWithLabel>(state, "mix", "Mix", *this));
    knobs.emplace_back(std::make_unique<UIHelpers::KnobWithLabel>(state, "lfoRate", "LFO Rate", *this));
    knobs.emplace_back(std::make_unique<UIHelpers::KnobWithLabel>(state, "lfoDepth", "LFO Depth", *this));
    knobs.emplace_back(std::make_unique<UIHelpers::KnobWithLabel>(state, "delayCentre", "Delay Centre", *this));
    knobs.emplace_back(std::make_unique<UIHelpers::KnobWithLabel>(state, "detuneAmount", "Detune", *this));
    knobs.emplace_back(std::make_unique<UIHelpers::KnobWithLabel>(state, "stereoSeparation", "Stereo Sep.", *this));
}

void MicroPitchDetuneComponent::paint(juce::Graphics& g)
{
    g.fillAll(UIHelpers::Colors::background);  // USE UIHelpers
}

void MicroPitchDetuneComponent::resized()
{
    using namespace UIHelpers::Dimensions;  // USE UIHelpers

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

void MicroPitchDetuneComponent::setMix(float newValue) { if (!knobs.empty()) knobs[0]->slider->setValue(newValue); }
void MicroPitchDetuneComponent::setLfoRate(float newValue) { if (knobs.size() > 1) knobs[1]->slider->setValue(newValue); }
void MicroPitchDetuneComponent::setLfoDepth(float newValue) { if (knobs.size() > 2) knobs[2]->slider->setValue(newValue); }
void MicroPitchDetuneComponent::setDelayCentre(float newValue) { if (knobs.size() > 3) knobs[3]->slider->setValue(newValue); }
void MicroPitchDetuneComponent::setDetuneAmount(float newValue) { if (knobs.size() > 4) knobs[4]->slider->setValue(newValue); }
void MicroPitchDetuneComponent::setStereoSeparation(float newValue) { if (knobs.size() > 5) knobs[5]->slider->setValue(newValue); }