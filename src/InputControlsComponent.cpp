#include "InputControlsComponent.h"

InputControlsComponent::InputControlsComponent(juce::AudioProcessorValueTreeState& state)
    : tiltEQ(state), widthBalancer(state)
{
    addAndMakeVisible(tiltEQ);
    addAndMakeVisible(widthBalancer);
}

void InputControlsComponent::paint(juce::Graphics& g)
{
    g.fillAll(PluginLookAndFeel::background);
}

void InputControlsComponent::resized()
{
    if (getWidth() <= 0 || getHeight() <= 0) return;

    auto area = getLocalBounds();
    const int h = area.getHeight();

    // TiltEQ gets its fixed height, but only if there's room.
    // If the block is shorter than the fixed height + a minimum for WidthBalancer,
    // TiltEQ shrinks proportionally so WidthBalancer always gets at least 80px.
    const int minWidthBalancerH = 80;
    const int tiltH = juce::jmin(kTiltEQFixedHeight,
        h - minWidthBalancerH - kGap);

    if (tiltH >= 40)
    {
        tiltEQ.setBounds(area.removeFromTop(tiltH));
        tiltEQ.setVisible(true);
    }
    else
    {
        tiltEQ.setVisible(false);
    }

    area.removeFromTop(kGap);
    widthBalancer.setBounds(area);
}