#ifndef ECHOPSYCHFX_INPUTCONTROLSCOMPONENT_H_INCLUDED
#define ECHOPSYCHFX_INPUTCONTROLSCOMPONENT_H_INCLUDED

#include <juce_gui_extra/juce_gui_extra.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include "TiltEQComponent.h"
#include "WidthBalancerComponent.h"

/**
 * @brief Combined block holding TiltEQ (top, fixed height) and
 *        WidthBalancer (bottom, fills remaining space).
 *
 * Both sub-components draw their own GroupComponent borders.
 * This wrapper has no border of its own — it is transparent.
 *
 * Fixed TiltEQ height = kTiltEQFixedHeight (px).
 * Change that constant here if you need more or less space for TiltEQ.
 */
class InputControlsComponent : public juce::Component
{
public:
    // Fixed pixel height reserved for the TiltEQ section at the top.
    // Enough for one knob + label + group label + margins.
    // Adjust this if TiltEQ feels too cramped or too spacious.
    static constexpr int kTiltEQFixedHeight = 130;

    InputControlsComponent(juce::AudioProcessorValueTreeState& state);
    ~InputControlsComponent() override = default;

    void paint(juce::Graphics& g) override;
    void resized() override;

    // Forwarding setters — called by PerceptionPresetManager
    void setTilt(float v) { tiltEQ.setTilt(v); }
    void setWidth(float v) { widthBalancer.setWidth(v); }
    void setIntensity(float v) { widthBalancer.setIntensity(v); }
    void setMidSideBalance(float v) { widthBalancer.setMidSideBalance(v); }
    void setMono(bool v) { widthBalancer.setMono(v); }

    // Direct access for PerceptionPresetManager which holds references
    TiltEQComponent& getTiltEQ() { return tiltEQ; }
    WidthBalancerComponent& getWidthBalancer() { return widthBalancer; }

private:
    TiltEQComponent       tiltEQ;
    WidthBalancerComponent widthBalancer;

    static constexpr int kGap = PluginLookAndFeel::margin;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(InputControlsComponent)
};

#endif // ECHOPSYCHFX_INPUTCONTROLSCOMPONENT_H_INCLUDED