#include "CollapsibleComponent.h"

CollapsibleComponent::CollapsibleComponent(const juce::String& title)
    : title_(title), collapseState_(CollapseState::Expanded)
{
}

int CollapsibleComponent::currentWidth() const
{
    return (collapseState_ == CollapseState::VCollapsed)
        ? PluginLookAndFeel::kHeaderH
        : expandedWidth();
}

int CollapsibleComponent::currentHeight() const
{
    return (collapseState_ == CollapseState::HCollapsed)
        ? PluginLookAndFeel::kHeaderH
        : expandedHeight();
}

CollapsibleComponent::CollapseState CollapsibleComponent::getCollapseState() const
{
    return collapseState_;
}

void CollapsibleComponent::setCollapseState(CollapseState s)
{
    collapseState_ = s;
    resized();
    repaint();
    if (onCollapseChanged)
        onCollapseChanged();
}

void CollapsibleComponent::paint(juce::Graphics& g)
{
    g.fillAll(PluginLookAndFeel::background);
    paintContent(g);

    const bool hCollapsed = (collapseState_ == CollapseState::HCollapsed);
    const bool vCollapsed = (collapseState_ == CollapseState::VCollapsed);

    PluginLookAndFeel::drawCollapsibleHeader(g, getHeaderBounds(), title_,
        hCollapsed || vCollapsed, vCollapsed);
}

void CollapsibleComponent::resized()
{
    const bool expanded = (collapseState_ == CollapseState::Expanded);

    for (int i = 0; i < getNumChildComponents(); ++i)
        getChildComponent(i)->setVisible(expanded);

    if (expanded)
        layoutContent(getContentBounds());
}

void CollapsibleComponent::mouseDown(const juce::MouseEvent& e)
{
    if (!getHeaderBounds().contains(e.getPosition()))
        return;

    if (e.mods.isRightButtonDown())
        setCollapseState(collapseState_ == CollapseState::VCollapsed
            ? CollapseState::Expanded : CollapseState::VCollapsed);
    else
        setCollapseState(collapseState_ == CollapseState::HCollapsed
            ? CollapseState::Expanded : CollapseState::HCollapsed);
}

juce::Rectangle<int> CollapsibleComponent::getHeaderBounds() const
{
    if (collapseState_ == CollapseState::VCollapsed)
        return getLocalBounds();
    return getLocalBounds().removeFromTop(PluginLookAndFeel::kHeaderH);
}

juce::Rectangle<int> CollapsibleComponent::getContentBounds() const
{
    auto b = getLocalBounds();
    b.removeFromTop(PluginLookAndFeel::kHeaderH);
    return b;
}