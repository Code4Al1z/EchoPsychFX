#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "PluginLookAndFeel.h"
#include <functional>

class CollapsibleComponent : public juce::Component
{
public:
    enum class CollapseState { Expanded, HeightCollapsed, WidthCollapsed };

    explicit CollapsibleComponent(const juce::String& title);

    virtual int expandedWidth() const = 0;
    virtual int expandedHeight() const = 0;
    virtual void layoutContent(juce::Rectangle<int> contentArea) = 0;
    virtual void paintContent(juce::Graphics& g) {}

    int currentWidth() const;
    int currentHeight() const;

    CollapseState getCollapseState() const;
    void setCollapseState(CollapseState s);

    /** Sets the section's accent colour and tints its header and every control inside it. */
    void setAccent(juce::Colour accent);
    juce::Colour getAccent() const { return accent_; }

    std::function<void()> onCollapseChanged;

    void paint(juce::Graphics& g) override;
    void resized() override;
    void mouseDown(const juce::MouseEvent& e) override;

protected:
    juce::Rectangle<int> getHeaderBounds() const;
    juce::Rectangle<int> getContentBounds() const;

private:
    juce::String title_;
    juce::Colour accent_{ PluginLookAndFeel::track };
    CollapseState collapseState_;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CollapsibleComponent)
};