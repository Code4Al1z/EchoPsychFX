#ifndef ECHOPSYCHFX_INPUTCONTROLSCOMPONENT_H_INCLUDED
#define ECHOPSYCHFX_INPUTCONTROLSCOMPONENT_H_INCLUDED

#include "CollapsibleComponent.h"
#include "TiltEQComponent.h"
#include "WidthBalancerComponent.h"

class InputControlsComponent : public CollapsibleComponent
{
public:
    static constexpr int kTiltEQFixedHeight = 130;
    static constexpr int kWidthH = 220;
    static constexpr int kExpandedW = 2 * PluginLookAndFeel::kKnobCell + PluginLookAndFeel::margin * 2;
    static constexpr int kExpandedH = PluginLookAndFeel::kHeaderH + kTiltEQFixedHeight + PluginLookAndFeel::spacing + kWidthH + PluginLookAndFeel::margin * 2;
    static constexpr int kGap = PluginLookAndFeel::margin;

    explicit InputControlsComponent(juce::AudioProcessorValueTreeState& state);
    ~InputControlsComponent() override = default;

    int expandedWidth() const override { return kExpandedW; }
    int expandedHeight() const override { return kExpandedH; }

    void paintContent(juce::Graphics& g) override;
    void layoutContent(juce::Rectangle<int> area) override;

    void setTilt(float v) { tiltEQ.setTilt(v); }
    void setWidth(float v) { widthBalancer.setWidth(v); }
    void setIntensity(float v) { widthBalancer.setIntensity(v); }
    void setMidSideBalance(float v) { widthBalancer.setMidSideBalance(v); }
    void setMono(bool v) { widthBalancer.setMono(v); }

    TiltEQComponent& getTiltEQ() { return tiltEQ; }
    WidthBalancerComponent& getWidthBalancer() { return widthBalancer; }

private:
    TiltEQComponent tiltEQ;
    WidthBalancerComponent widthBalancer;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(InputControlsComponent)
};

#endif // ECHOPSYCHFX_INPUTCONTROLSCOMPONENT_H_INCLUDED