#ifndef ECHOPSYCHFX_PERCEPTIONCOMPONENT_H_INCLUDED
#define ECHOPSYCHFX_PERCEPTIONCOMPONENT_H_INCLUDED

#include <juce_gui_extra/juce_gui_extra.h>

class PerceptionModeComponent: public juce::Component
{
public:
    PerceptionModeComponent();
    ~PerceptionModeComponent() override;

    void paint(juce::Graphics& g) override;
    void resized() override;

private:

    juce::Label titleLabel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PerceptionModeComponent)
};

#endif // ECHOPSYCHFX_PERCEPTIONCOMPONENT_H_INCLUDED