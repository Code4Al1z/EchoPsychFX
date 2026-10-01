#include "CollapsibleComponent.h"

CollapsibleComponent::CollapsibleComponent(const juce::String& title)
    : title_(title), collapseState_(CollapseState::Expanded)
{
}

int CollapsibleComponent::currentWidth() const
{
    return (collapseState_ != CollapseState::Expanded)
        ? juce::jmax(PluginLookAndFeel::kHeaderH, expandedWidth() / 2)
        : expandedWidth();
}

int CollapsibleComponent::currentHeight() const
{
    return (collapseState_ != CollapseState::Expanded)
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

void CollapsibleComponent::setAccent(juce::Colour accent)
{
    accent_ = accent;
    PluginLookAndFeel::applyAccent(*this, accent);
    repaint();
}

void CollapsibleComponent::paint(juce::Graphics& g)
{
    const bool isCollapsed = (collapseState_ != CollapseState::Expanded);

    PluginLookAndFeel::drawPanel(g, isCollapsed ? getHeaderBounds() : getLocalBounds(), PluginLookAndFeel::panel);

    if (!isCollapsed)
        paintContent(g);

    // Header drawn horizontally at the top of the component
    PluginLookAndFeel::drawCollapsibleHeader(g, getHeaderBounds(), title_, isCollapsed, false, accent_);
}

void CollapsibleComponent::resized()
{
    const bool expanded = (collapseState_ == CollapseState::Expanded);

    // Hide children when collapsed
    for (int i = 0; i < getNumChildComponents(); ++i)
        getChildComponent(i)->setVisible(expanded);

    if (expanded)
        layoutContent(getContentBounds());
}

void CollapsibleComponent::mouseDown(const juce::MouseEvent& e)
{
    if (!getHeaderBounds().contains(e.getPosition()))
        return;

    // Normal left-click toggles collapse
    if (e.mods.isLeftButtonDown())
    {
        setCollapseState(collapseState_ == CollapseState::Expanded
            ? CollapseState::HeightCollapsed : CollapseState::Expanded);
    }
}

juce::Rectangle<int> CollapsibleComponent::getHeaderBounds() const
{
    return juce::Rectangle<int>(0, 0, getWidth(), PluginLookAndFeel::kHeaderH);
}

juce::Rectangle<int> CollapsibleComponent::getContentBounds() const
{
    auto b = getLocalBounds();
    b.removeFromTop(PluginLookAndFeel::kHeaderH);
    return b;
}