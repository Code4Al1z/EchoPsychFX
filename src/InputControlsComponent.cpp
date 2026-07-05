#include "InputControlsComponent.h"

InputControlsComponent::InputControlsComponent(juce::AudioProcessorValueTreeState& state)
    : CollapsibleComponent("Input Controls"), tiltEQ(state), widthBalancer(state)
{
    addAndMakeVisible(tiltEQ);
    addAndMakeVisible(widthBalancer);
}

void InputControlsComponent::paintContent(juce::Graphics& g)
{
    g.fillAll(PluginLookAndFeel::background);
}

void InputControlsComponent::layoutContent(juce::Rectangle<int> area)
{
    auto inner = area.reduced(PluginLookAndFeel::margin);
    const int h = inner.getHeight();

    const int minWidthBalancerH = 80;
    const int tiltH = juce::jmin(kTiltEQFixedHeight, h - minWidthBalancerH - kGap);

    if (tiltH >= 40)
    {
        tiltEQ.setBounds(inner.removeFromTop(tiltH));
        tiltEQ.setVisible(true);
    }
    else
    {
        tiltEQ.setVisible(false);
    }

    inner.removeFromTop(kGap);
    widthBalancer.setBounds(inner);
}