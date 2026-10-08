#include "PluginLookAndFeel.h"

namespace
{
    using juce::Colour;
    constexpr float kPi = juce::MathConstants<float>::pi;
}

const Colour PluginLookAndFeel::background{ 17, 11, 23 };
const Colour PluginLookAndFeel::knobThumb{ 255, 140, 60 };
const Colour PluginLookAndFeel::track{ 255, 46, 136 };
const Colour PluginLookAndFeel::knobBackground{ 74, 30, 70 };
const Colour PluginLookAndFeel::knobFill{ 255, 46, 136 };
const Colour PluginLookAndFeel::knobOutline{ 58, 40, 68 };
const Colour PluginLookAndFeel::labelText{ 236, 230, 246 };
const Colour PluginLookAndFeel::groupOutline = juce::Colours::white.withAlpha(0.10f);
const Colour PluginLookAndFeel::headerBg{ 40, 26, 50 };
const Colour PluginLookAndFeel::headerText{ 245, 240, 252 };
const Colour PluginLookAndFeel::popupRowA{ 24, 16, 31 };
const Colour PluginLookAndFeel::popupRowB{ 36, 24, 46 };

const Colour PluginLookAndFeel::panel{ 28, 19, 37 };
const Colour PluginLookAndFeel::panelRaised{ 38, 27, 49 };
const Colour PluginLookAndFeel::mutedText{ 155, 140, 172 };
const Colour PluginLookAndFeel::knobTrack{ 55, 40, 66 };

const Colour PluginLookAndFeel::brandCyan{ 63, 201, 238 };      // #3fc9ee
const Colour PluginLookAndFeel::brandViolet{ 127, 107, 251 };   // #7f6bfb
const Colour PluginLookAndFeel::accentInput{ 63, 201, 238 };
const Colour PluginLookAndFeel::accentMotion{ 255, 150, 60 };
const Colour PluginLookAndFeel::accentSpatial{ 255, 46, 136 };
const Colour PluginLookAndFeel::accentMicroPitch{ 176, 120, 255 };
const Colour PluginLookAndFeel::accentExciter{ 255, 206, 70 };
const Colour PluginLookAndFeel::accentReverb{ 96, 156, 255 };

PluginLookAndFeel::PluginLookAndFeel()
{
    setColour(juce::GroupComponent::outlineColourId, groupOutline);
    setColour(juce::GroupComponent::textColourId, mutedText);

    setColour(juce::PopupMenu::backgroundColourId, popupRowA);
    setColour(juce::PopupMenu::textColourId, labelText);
    // a soft wash of the brand pink: full-strength pink glared against the dark menu
    setColour(juce::PopupMenu::highlightedBackgroundColourId, track.withAlpha(0.30f));
    setColour(juce::PopupMenu::highlightedTextColourId, juce::Colours::white);

    setColour(juce::AlertWindow::backgroundColourId, panel);
    setColour(juce::AlertWindow::textColourId, labelText);
    setColour(juce::TextEditor::backgroundColourId, background);
    setColour(juce::TextEditor::textColourId, labelText);
    setColour(juce::TextEditor::outlineColourId, groupOutline);
    setColour(juce::TextButton::buttonColourId, panelRaised);
    setColour(juce::TextButton::textColourOffId, labelText);
    setColour(juce::TextButton::textColourOnId, labelText);
    setColour(juce::Label::textColourId, labelText);
}

void PluginLookAndFeel::applyAccent(juce::Component& root, juce::Colour accent)
{
    for (auto* child : root.getChildren())
    {
        if (auto* s = dynamic_cast<juce::Slider*>(child))
        {
            s->setColour(juce::Slider::rotarySliderFillColourId, accent);
            s->setColour(juce::Slider::trackColourId, accent);
            s->setColour(juce::Slider::thumbColourId, accent.brighter(0.3f));
        }
        else if (auto* b = dynamic_cast<juce::TextButton*>(child))
            b->setColour(juce::TextButton::buttonOnColourId, accent);
        else if (auto* tb = dynamic_cast<juce::ToggleButton*>(child))
            tb->setColour(juce::ToggleButton::tickColourId, accent);
        else if (auto* cb = dynamic_cast<juce::ComboBox*>(child))
            cb->setColour(juce::ComboBox::arrowColourId, accent);

        applyAccent(*child, accent);
    }
}

void PluginLookAndFeel::drawPanel(juce::Graphics& g, juce::Rectangle<int> bounds, juce::Colour fill, float corner)
{
    auto r = bounds.toFloat();
    g.setColour(fill);
    g.fillRoundedRectangle(r, corner);
    g.setColour(juce::Colours::white.withAlpha(0.06f));
    g.drawRoundedRectangle(r.reduced(0.5f), corner, 1.0f);
}

void PluginLookAndFeel::drawGroupComponentOutline(juce::Graphics& g, int width, int height,
    const juce::String& text, const juce::Justification&, juce::GroupComponent& component)
{
    // Untitled groups only exist to host layout, so they draw nothing. Titled ones become sub-cards.
    if (text.isEmpty())
        return;

    drawPanel(g, { 0, 0, width, height }, panelRaised.withAlpha(0.55f), 6.0f);

    g.setColour(component.findColour(juce::GroupComponent::textColourId));
    g.setFont(juce::Font(11.0f, juce::Font::bold));
    g.drawText(text.toUpperCase(), 10, 4, width - 20, groupLabelHeight - 4, juce::Justification::centredLeft, true);
}

void PluginLookAndFeel::drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
    float sliderPos, float startAngle, float endAngle, juce::Slider& slider)
{
    const auto bounds = juce::Rectangle<int>(x, y, width, height).toFloat().reduced(4.0f);
    const float diameter = juce::jmin(bounds.getWidth(), bounds.getHeight());
    if (diameter < 8.0f)
        return;

    const auto centre = bounds.getCentre();
    const float radius = diameter * 0.5f;
    const float arcThickness = juce::jlimit(3.0f, 6.0f, radius * 0.16f);
    const float arcRadius = radius - arcThickness * 0.5f;
    const float angle = startAngle + sliderPos * (endAngle - startAngle);
    const bool enabled = slider.isEnabled();

    const auto accent = slider.findColour(juce::Slider::rotarySliderFillColourId)
        .withMultipliedAlpha(enabled ? 1.0f : 0.4f);

    // Bipolar controls fill outward from the centre detent instead of from the left stop.
    const bool bipolar = slider.getMinimum() < 0.0 && slider.getMaximum() > 0.0;
    const float zeroPos = bipolar
        ? static_cast<float>(slider.valueToProportionOfLength(0.0)) : 0.0f;
    const float zeroAngle = startAngle + zeroPos * (endAngle - startAngle);

    juce::Path trackArc;
    trackArc.addCentredArc(centre.x, centre.y, arcRadius, arcRadius, 0.0f, startAngle, endAngle, true);
    g.setColour(knobTrack);
    g.strokePath(trackArc, juce::PathStrokeType(arcThickness, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    if (std::abs(angle - zeroAngle) > 0.001f)
    {
        juce::Path valueArc;
        valueArc.addCentredArc(centre.x, centre.y, arcRadius, arcRadius, 0.0f,
            juce::jmin(zeroAngle, angle), juce::jmax(zeroAngle, angle), true);

        // soft glow underneath, crisp arc on top
        g.setColour(accent.withAlpha(0.22f * accent.getFloatAlpha()));
        g.strokePath(valueArc, juce::PathStrokeType(arcThickness + 4.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
        g.setColour(accent);
        g.strokePath(valueArc, juce::PathStrokeType(arcThickness, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    }

    // Knob cap
    const float capRadius = arcRadius - arcThickness * 0.5f - radius * 0.10f;
    if (capRadius > 3.0f)
    {
        juce::ColourGradient cap(panelRaised.brighter(0.25f), centre.x, centre.y - capRadius,
            panel.darker(0.3f), centre.x, centre.y + capRadius, false);
        g.setGradientFill(cap);
        g.fillEllipse(centre.x - capRadius, centre.y - capRadius, capRadius * 2.0f, capRadius * 2.0f);
        g.setColour(juce::Colours::white.withAlpha(0.10f));
        g.drawEllipse(centre.x - capRadius, centre.y - capRadius, capRadius * 2.0f, capRadius * 2.0f, 1.0f);

        // Pointer
        const float pointerInner = capRadius * 0.38f;
        const float pointerOuter = capRadius * 0.86f;
        const float s = std::sin(angle), c = std::cos(angle);
        g.setColour(enabled ? labelText : mutedText);
        g.drawLine(centre.x + s * pointerInner, centre.y - c * pointerInner,
            centre.x + s * pointerOuter, centre.y - c * pointerOuter,
            juce::jlimit(2.0f, 3.0f, capRadius * 0.12f));
    }
}

void PluginLookAndFeel::drawLinearSlider(juce::Graphics& g, int x, int y, int width, int height,
    float sliderPos, float, float, juce::Slider::SliderStyle style, juce::Slider& slider)
{
    if (style != juce::Slider::LinearHorizontal)
    {
        LookAndFeel_V4::drawLinearSlider(g, x, y, width, height, sliderPos, 0, 0, style, slider);
        return;
    }

    const auto accent = slider.findColour(juce::Slider::trackColourId);
    const float cy = y + height * 0.5f;
    const float trackH = 6.0f;
    const float left = static_cast<float>(x) + 6.0f;
    const float right = static_cast<float>(x + width) - 6.0f;

    g.setColour(knobTrack);
    g.fillRoundedRectangle(left, cy - trackH * 0.5f, right - left, trackH, trackH * 0.5f);

    // Bipolar sliders fill outward from the centre detent.
    const bool bipolar = slider.getMinimum() < 0.0 && slider.getMaximum() > 0.0;
    const float zeroX = bipolar
        ? juce::jmap(static_cast<float>(slider.valueToProportionOfLength(0.0)), left, right) : left;
    const float fromX = juce::jmin(zeroX, sliderPos);
    const float toX = juce::jmax(zeroX, sliderPos);

    g.setColour(accent);
    g.fillRoundedRectangle(fromX, cy - trackH * 0.5f, toX - fromX, trackH, trackH * 0.5f);

    if (bipolar)
    {
        g.setColour(mutedText.withAlpha(0.7f));
        g.fillRect(zeroX - 0.5f, cy - 7.0f, 1.0f, 14.0f);
    }

    const float thumb = 14.0f;
    g.setColour(accent.withAlpha(0.25f));
    g.fillEllipse(sliderPos - thumb * 0.75f, cy - thumb * 0.75f, thumb * 1.5f, thumb * 1.5f);
    g.setColour(labelText);
    g.fillEllipse(sliderPos - thumb * 0.5f, cy - thumb * 0.5f, thumb, thumb);
}

juce::Label* PluginLookAndFeel::createSliderTextBox(juce::Slider& slider)
{
    auto* l = LookAndFeel_V4::createSliderTextBox(slider);
    l->setFont(juce::Font(12.5f));
    l->setJustificationType(juce::Justification::centred);
    l->setColour(juce::Label::textColourId, labelText);
    l->setColour(juce::Label::backgroundColourId, juce::Colours::transparentBlack);
    l->setColour(juce::Label::outlineColourId, juce::Colours::transparentBlack);
    l->setColour(juce::Label::textWhenEditingColourId, juce::Colours::white);
    l->setColour(juce::Label::backgroundWhenEditingColourId, background);
    l->setColour(juce::Label::outlineWhenEditingColourId, slider.findColour(juce::Slider::rotarySliderFillColourId));
    return l;
}

void PluginLookAndFeel::drawToggleButton(juce::Graphics& g, juce::ToggleButton& button, bool highlighted, bool)
{
    const auto b = button.getLocalBounds().toFloat();
    const float h = juce::jmin(18.0f, b.getHeight());
    const float w = h * 1.8f;
    const auto sw = juce::Rectangle<float>(b.getX() + 1.0f, b.getCentreY() - h * 0.5f, w, h);
    const bool on = button.getToggleState();
    const auto accent = button.findColour(juce::ToggleButton::tickColourId);

    g.setColour(on ? accent.withAlpha(highlighted ? 1.0f : 0.85f) : knobTrack.brighter(highlighted ? 0.2f : 0.0f));
    g.fillRoundedRectangle(sw, h * 0.5f);
    const float knobD = h - 6.0f;
    const float kx = on ? sw.getRight() - 3.0f - knobD : sw.getX() + 3.0f;
    g.setColour(on ? juce::Colours::white : mutedText);
    g.fillEllipse(kx, sw.getCentreY() - knobD * 0.5f, knobD, knobD);

    g.setColour(on ? labelText : mutedText);
    g.setFont(juce::Font(13.0f));
    g.drawText(button.getButtonText(), juce::Rectangle<float>(sw.getRight() + 7.0f, b.getY(), b.getRight() - sw.getRight() - 7.0f, b.getHeight()),
        juce::Justification::centredLeft, true);
}

void PluginLookAndFeel::drawButtonBackground(juce::Graphics& g, juce::Button& button, const juce::Colour& bg,
    bool highlighted, bool down)
{
    auto r = button.getLocalBounds().toFloat().reduced(0.5f);
    auto fill = bg;
    if (button.getToggleState())
        fill = button.findColour(juce::TextButton::buttonOnColourId);
    if (!button.isEnabled())
        fill = fill.withMultipliedAlpha(0.4f);
    if (down)
        fill = fill.darker(0.2f);
    else if (highlighted)
        fill = fill.brighter(0.15f);

    g.setColour(fill);
    g.fillRoundedRectangle(r, 5.0f);
    g.setColour(juce::Colours::white.withAlpha(0.08f));
    g.drawRoundedRectangle(r, 5.0f, 1.0f);
}

void PluginLookAndFeel::drawComboBox(juce::Graphics& g, int width, int height, bool,
    int, int, int, int, juce::ComboBox& box)
{
    auto r = juce::Rectangle<float>(0.0f, 0.0f, (float)width, (float)height).reduced(0.5f);
    g.setColour(panelRaised);
    g.fillRoundedRectangle(r, 6.0f);
    g.setColour(box.hasKeyboardFocus(true) ? box.findColour(juce::ComboBox::arrowColourId)
        : juce::Colours::white.withAlpha(0.10f));
    g.drawRoundedRectangle(r, 6.0f, 1.0f);

    const float cx = width - 14.0f, cy = height * 0.5f;
    juce::Path chevron;
    chevron.startNewSubPath(cx - 4.0f, cy - 2.0f);
    chevron.lineTo(cx, cy + 2.5f);
    chevron.lineTo(cx + 4.0f, cy - 2.0f);
    g.setColour(box.findColour(juce::ComboBox::arrowColourId));
    g.strokePath(chevron, juce::PathStrokeType(2.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
}

juce::Font PluginLookAndFeel::getComboBoxFont(juce::ComboBox&)
{
    return juce::Font(14.0f);
}

void PluginLookAndFeel::drawPopupMenuItem(juce::Graphics& g, const juce::Rectangle<int>& area,
    bool isSeparator, bool isActive, bool isHighlighted, bool isTicked, bool hasSubMenu,
    const juce::String& text, const juce::String& shortcutKeyText,
    const juce::Drawable* icon, const juce::Colour* textColour)
{
    if (!isSeparator && !(isHighlighted && isActive))
    {
        const int rowH = juce::jmax(1, area.getHeight());
        const int rowIndex = area.getY() / rowH;
        g.setColour((rowIndex % 2 == 0) ? popupRowA : popupRowB);
        g.fillRect(area);
    }

    LookAndFeel_V4::drawPopupMenuItem(g, area, isSeparator, isActive, isHighlighted, isTicked, hasSubMenu,
        text, shortcutKeyText, icon, textColour);
}

void PluginLookAndFeel::drawCollapsibleHeader(juce::Graphics& g,
    juce::Rectangle<int> bounds,
    const juce::String& title,
    bool isCollapsed,
    bool isVerticalCollapse,
    juce::Colour accent)
{
    juce::ignoreUnused(isVerticalCollapse);

    auto r = bounds.toFloat();
    juce::Path header;
    header.addRoundedRectangle(r.getX(), r.getY(), r.getWidth(), r.getHeight(), 8.0f, 8.0f, true, true, isCollapsed, isCollapsed);
    g.setColour(headerBg);
    g.fillPath(header);

    // accent stripe + title
    g.setColour(accent);
    g.fillRoundedRectangle(r.getX() + 10.0f, r.getCentreY() - 7.0f, 4.0f, 14.0f, 2.0f);

    g.setColour(headerText);
    g.setFont(juce::Font(13.5f, juce::Font::bold));
    g.drawText(title, bounds.getX() + 22, bounds.getY(), bounds.getWidth() - 50, bounds.getHeight(),
        juce::Justification::centredLeft, true);

    const float cx = r.getRight() - 16.0f;
    const float cy = r.getCentreY();
    juce::Path chevron;
    if (isCollapsed)
    {
        chevron.startNewSubPath(cx - 2.5f, cy - 4.5f);
        chevron.lineTo(cx + 2.5f, cy);
        chevron.lineTo(cx - 2.5f, cy + 4.5f);
    }
    else
    {
        chevron.startNewSubPath(cx - 4.5f, cy - 2.5f);
        chevron.lineTo(cx, cy + 2.5f);
        chevron.lineTo(cx + 4.5f, cy - 2.5f);
    }
    g.setColour(accent);
    g.strokePath(chevron, juce::PathStrokeType(2.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
}

PluginLookAndFeel::KnobWithLabel::KnobWithLabel(juce::AudioProcessorValueTreeState& state,
    const juce::String& paramID,
    const juce::String& labelTextStr,
    juce::Component& parent)
{
    slider = std::make_unique<juce::Slider>();
    label = std::make_unique<juce::Label>();

    PluginLookAndFeel::configureKnob(*slider);
    PluginLookAndFeel::configureLabel(*label, labelTextStr);

    parent.addAndMakeVisible(*slider);
    parent.addAndMakeVisible(*label);

    attachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        state, paramID, *slider);

    // Show the parameter's unit next to its value ("400.0 ms"); the attachment installs a text
    // function without it. Typed entry still works since the parameter parser ignores the suffix.
    if (auto* param = state.getParameter(paramID))
    {
        slider->textFromValueFunction = [param](double v)
            {
                auto text = param->getText(param->convertTo0to1(static_cast<float>(v)), 0);
                const auto unit = param->getLabel();
                return unit.isEmpty() ? text : text + (text.endsWithChar('k') ? "" : " ") + unit;
            };
        slider->updateText();
    }
}

void PluginLookAndFeel::KnobWithLabel::setBounds(int x, int y, int width, int height)
{
    const int lH = PluginLookAndFeel::labelHeight;
    // leave a little air below each knob's value readout so it doesn't touch the next row's label
    label->setBounds(x, y + 2, width, lH);
    slider->setBounds(x, y + lH + 2, width, juce::jmax(0, height - lH - 8));
}

namespace
{
    // Small waveform glyph drawn inside a ShapePicker button.
    class ShapeButton : public juce::TextButton
    {
    public:
        explicit ShapeButton(const juce::String& name) : juce::TextButton(name), key(name) {}

        void paintButton(juce::Graphics& g, bool highlighted, bool down) override
        {
            auto r = getLocalBounds().toFloat().reduced(1.0f);
            const bool sel = getToggleState();
            const auto accent = findColour(juce::TextButton::buttonOnColourId);

            g.setColour(sel ? accent.withAlpha(0.22f)
                : PluginLookAndFeel::panelRaised.brighter(down ? 0.2f : highlighted ? 0.1f : 0.0f));
            g.fillRoundedRectangle(r, 5.0f);
            g.setColour(sel ? accent : juce::Colours::white.withAlpha(0.08f));
            g.drawRoundedRectangle(r, 5.0f, sel ? 1.5f : 1.0f);

            auto icon = r.reduced(juce::jmax(6.0f, r.getWidth() * 0.22f), juce::jmax(4.0f, r.getHeight() * 0.24f));
            if (icon.getWidth() < 6.0f || icon.getHeight() < 4.0f)
                return;

            juce::Path p;
            const float L = icon.getX(), R = icon.getRight(), T = icon.getY(), B = icon.getBottom();
            const float M = icon.getCentreY(), W = icon.getWidth();
            auto at = [&](float u, float v) { return juce::Point<float>(L + W * u, B - icon.getHeight() * v); };

            if (key == "Sin")
            {
                p.startNewSubPath(at(0.0f, 0.5f));
                for (int i = 1; i <= 32; ++i)
                {
                    const float u = i / 32.0f;
                    p.lineTo(at(u, 0.5f + 0.5f * std::sin(u * 2.0f * kPi)));
                }
            }
            else if (key == "Tri")
            {
                p.startNewSubPath(at(0.0f, 0.5f)); p.lineTo(at(0.25f, 1.0f));
                p.lineTo(at(0.75f, 0.0f)); p.lineTo(at(1.0f, 0.5f));
            }
            else if (key == "Sqr")
            {
                p.startNewSubPath(at(0.0f, 0.0f)); p.lineTo(at(0.0f, 1.0f)); p.lineTo(at(0.5f, 1.0f));
                p.lineTo(at(0.5f, 0.0f)); p.lineTo(at(1.0f, 0.0f)); p.lineTo(at(1.0f, 1.0f));
            }
            else if (key == "Sw^")   // ramp up, then drop
            {
                p.startNewSubPath(at(0.0f, 0.0f)); p.lineTo(at(0.5f, 1.0f));
                p.lineTo(at(0.5f, 0.0f)); p.lineTo(at(1.0f, 1.0f));
            }
            else if (key == "Sw_")   // drop, then ramp down
            {
                p.startNewSubPath(at(0.0f, 1.0f)); p.lineTo(at(0.5f, 0.0f));
                p.lineTo(at(0.5f, 1.0f)); p.lineTo(at(1.0f, 0.0f));
            }
            else   // Rnd - stepped sample & hold
            {
                const float v[] = { 0.55f, 0.15f, 0.85f, 0.35f, 0.7f };
                p.startNewSubPath(at(0.0f, v[0]));
                for (int i = 0; i < 5; ++i)
                {
                    p.lineTo(at(i / 5.0f, v[i]));
                    p.lineTo(at((i + 1) / 5.0f, v[i]));
                    if (i < 4) p.lineTo(at((i + 1) / 5.0f, v[i + 1]));
                }
            }
            juce::ignoreUnused(T, M, R);

            g.setColour(sel ? accent.brighter(0.3f) : PluginLookAndFeel::mutedText);
            g.strokePath(p, juce::PathStrokeType(1.8f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
        }

    private:
        juce::String key;
    };
}

PluginLookAndFeel::ShapePicker::ShapePicker(juce::AudioProcessorValueTreeState& state,
    const juce::String& paramID,
    const std::vector<juce::String>& labels,
    juce::Component& parent)
{
    int idx = 0;
    for (auto& labelText : labels)
    {
        auto* btn = new ShapeButton(labelText);
        buttons.add(btn);
        btn->setClickingTogglesState(false);
        btn->setTooltip(labelText);
        const int i = idx;
        btn->onClick = [this, i] { setSelected(i); };
        parent.addAndMakeVisible(btn);
        ++idx;
    }

    hiddenCombo = std::make_unique<juce::ComboBox>();
    for (int i = 0; i < static_cast<int>(labels.size()); ++i)
        hiddenCombo->addItem(labels[static_cast<size_t>(i)], i + 1);

    // The attachment pushes the parameter's current value into the combo. Do not write a
    // selection of our own afterwards - that used to reset the parameter to index 0 every
    // time the editor opened.
    attachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(state, paramID, *hiddenCombo);
    hiddenCombo->onChange = [this] { updateVisuals(hiddenCombo->getSelectedId() - 1); };
    hiddenCombo->setVisible(false);
    parent.addAndMakeVisible(*hiddenCombo);

    updateVisuals(hiddenCombo->getSelectedId() - 1);
}

void PluginLookAndFeel::ShapePicker::updateVisuals(int index)
{
    selectedIndex = index;
    for (int i = 0; i < buttons.size(); ++i)
        buttons[i]->setToggleState(i == index, juce::dontSendNotification);
}

void PluginLookAndFeel::ShapePicker::setSelected(int index)
{
    if (index < 0 || index >= buttons.size())
        return;

    if (hiddenCombo)
        hiddenCombo->setSelectedId(index + 1, juce::sendNotification);
    else
        updateVisuals(index);
}

void PluginLookAndFeel::ShapePicker::setBounds(int x, int y, int width, int height)
{
    const int n = buttons.size();
    if (n == 0)
        return;

    // One row of equally sized glyph buttons with a small gap between them.
    const int gap = 4;
    const int bw = (width - gap * (n - 1)) / n;
    for (int i = 0; i < n; ++i)
        buttons[i]->setBounds(x + i * (bw + gap), y, bw, height);

    if (hiddenCombo)
        hiddenCombo->setBounds(x, y, 0, 0);
}

PluginLookAndFeel::GridFitResult PluginLookAndFeel::findBestSquareGridFit(
    int nElements, float totalWidth, float totalHeight,
    float minCellSize, float maxCellSize)
{
    GridFitResult best;
    if (nElements <= 0 || totalWidth <= 0.0f || totalHeight <= 0.0f) return best;

    for (int columns = 1; columns <= nElements; ++columns)
    {
        const int rows = (nElements + columns - 1) / columns;
        const float cellWidth = totalWidth / static_cast<float>(columns);
        const float cellHeight = totalHeight / static_cast<float>(rows);
        const float cellSize = std::min(cellWidth, cellHeight);

        if (cellSize >= minCellSize && cellSize <= maxCellSize && cellSize > best.cellSize)
        {
            best.columns = columns;
            best.rows = rows;
            best.cellSize = cellSize;
        }
    }

    if (best.columns == 0)
    {
        float bestCell = 0.0f;
        for (int columns = 1; columns <= nElements; ++columns)
        {
            const int rows = (nElements + columns - 1) / columns;
            const float cellWidth = totalWidth / static_cast<float>(columns);
            const float cellHeight = totalHeight / static_cast<float>(rows);
            const float cellSize = std::min(cellWidth, cellHeight);
            if (cellSize > bestCell)
            {
                bestCell = cellSize;
                best.columns = columns;
                best.rows = rows;
                best.cellSize = cellSize;
            }
        }
    }
    return best;
}

PluginLookAndFeel::KnobLayoutResult PluginLookAndFeel::calculateKnobLayout(
    int numKnobs, int availableWidth, int availableHeight, bool allowWideLayout)
{
    KnobLayoutResult result;
    result.totalWidth = 0;
    result.totalHeight = 0;

    if (numKnobs <= 0 || availableWidth <= 4 || availableHeight <= 4)
        return result;

    const float contentW = static_cast<float>(availableWidth);
    const float contentH = static_cast<float>(availableHeight);

    int bestCols = numKnobs, bestRows = 1;
    float bestArc = 0.f;

    if (allowWideLayout)
    {
        bestCols = numKnobs;
        bestRows = 1;
    }
    else
    {
        for (int cols = 1; cols <= numKnobs; ++cols)
        {
            const int rows = (numKnobs + cols - 1) / cols;
            const float cellW = contentW / static_cast<float>(cols);
            const float cellH = contentH / static_cast<float>(rows);
            const float arc = std::min(cellW, cellH - static_cast<float>(labelHeight));
            if (arc > bestArc) { bestArc = arc; bestCols = cols; bestRows = rows; }
        }
    }

    const float rawCellW = contentW / static_cast<float>(bestCols);
    const float rawCellH = contentH / static_cast<float>(bestRows);
    const float cellSize = juce::jlimit(static_cast<float>(minKnobSize),
        static_cast<float>(maxKnobSize), std::min(rawCellW, rawCellH));

    const float gridW = cellSize * static_cast<float>(bestCols);
    const float gridH = cellSize * static_cast<float>(bestRows);
    const float offsetX = juce::jmax(0.0f, (contentW - gridW) * 0.5f);
    const float offsetY = juce::jmax(0.0f, (contentH - gridH) * 0.5f);

    result.totalWidth = availableWidth;
    result.totalHeight = availableHeight;
    result.knobBounds.reserve(numKnobs);

    for (int i = 0; i < numKnobs; ++i)
    {
        const int col = i % bestCols;
        const int row = i / bestCols;
        const int cellX = static_cast<int>(offsetX + col * cellSize);
        const int cellY = static_cast<int>(offsetY + row * cellSize);
        result.knobBounds.emplace_back(cellX, cellY, static_cast<int>(cellSize), static_cast<int>(cellSize));
    }
    return result;
}

void PluginLookAndFeel::configureKnob(juce::Slider& slider)
{
    slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    slider.setRotaryParameters(kPi * 1.25f, kPi * 2.75f, true);
    slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 78, 20);
    slider.setColour(juce::Slider::rotarySliderFillColourId, knobFill);
    slider.setColour(juce::Slider::thumbColourId, knobThumb);
    slider.setColour(juce::Slider::trackColourId, knobFill);
    slider.setColour(juce::Slider::rotarySliderOutlineColourId, knobOutline);
    slider.setColour(juce::Slider::textBoxTextColourId, labelText);
    slider.setColour(juce::Slider::textBoxBackgroundColourId, juce::Colours::transparentBlack);
    slider.setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
}

void PluginLookAndFeel::configureLabel(juce::Label& label, const juce::String& text)
{
    label.setText(text, juce::dontSendNotification);
    label.setJustificationType(juce::Justification::centred);
    label.setFont(juce::Font(12.0f, juce::Font::bold));
    label.setColour(juce::Label::textColourId, mutedText);
}

void PluginLookAndFeel::configureGroup(juce::GroupComponent& group)
{
    group.setColour(juce::GroupComponent::outlineColourId, groupOutline);
    group.setColour(juce::GroupComponent::textColourId, mutedText);
}

void PluginLookAndFeel::configureComboBox(juce::ComboBox& box)
{
    box.setColour(juce::ComboBox::backgroundColourId, panelRaised);
    box.setColour(juce::ComboBox::textColourId, labelText);
    box.setColour(juce::ComboBox::outlineColourId, groupOutline);
    box.setColour(juce::ComboBox::arrowColourId, track);
    box.setColour(juce::ComboBox::focusedOutlineColourId, track);
}

void PluginLookAndFeel::setKnobValue(const std::vector<std::unique_ptr<KnobWithLabel>>& knobs, int index, float value)
{
    if (index >= 0 && index < static_cast<int>(knobs.size()) && knobs[static_cast<size_t>(index)]->slider)
        knobs[static_cast<size_t>(index)]->slider->setValue(value);
}