#include "SupportFooter.h"
#include "PluginLookAndFeel.h"

namespace
{
    const juce::Font footerFont(11.0f);
    const juce::String linkText("Report a Bug");
}

juce::String SupportFooter::versionText()
{
    return juce::String(JucePlugin_VersionString) + " beta";
}

SupportFooter::SupportFooter(juce::AudioProcessor& p, juce::AudioProcessorValueTreeState& s)
    : processor(p), state(s)
{
    setMouseCursor(juce::MouseCursor::NormalCursor);
    setTooltip("Opens an email to " + juce::String(supportEmail()) + " and copies your settings and system details to the clipboard");
}

juce::Rectangle<int> SupportFooter::linkBounds() const
{
    const int w = juce::roundToInt(juce::GlyphArrangement::getStringWidthInt(footerFont, linkText)) + 6;
    return getLocalBounds().removeFromRight(w);
}

void SupportFooter::paint(juce::Graphics& g)
{
    using L = PluginLookAndFeel;
    g.setFont(footerFont);

    const auto link = linkBounds();
    g.setColour(hoveringLink ? L::brandCyan : L::mutedText);
    g.drawText(linkText, link, juce::Justification::centredRight, false);

    // Version sits just left of the link, separated by a middle dot
    g.setColour(L::mutedText.withAlpha(0.8f));
    g.drawText("EchoPsychFX v" + versionText() + "   \xc2\xb7", getLocalBounds().withRight(link.getX()),
               juce::Justification::centredRight, false);
}

void SupportFooter::mouseMove(const juce::MouseEvent& e)
{
    const bool over = linkBounds().contains(e.getPosition());
    if (over != hoveringLink)
    {
        hoveringLink = over;
        setMouseCursor(over ? juce::MouseCursor::PointingHandCursor : juce::MouseCursor::NormalCursor);
        repaint();
    }
}

void SupportFooter::mouseExit(const juce::MouseEvent&)
{
    if (hoveringLink)
    {
        hoveringLink = false;
        setMouseCursor(juce::MouseCursor::NormalCursor);
        repaint();
    }
}

void SupportFooter::mouseUp(const juce::MouseEvent& e)
{
    if (linkBounds().contains(e.getPosition()))
        reportBug();
}

juce::String SupportFooter::buildReport() const
{
    const juce::PluginHostType host;

    juce::String report;
    report << "EchoPsychFX " << versionText() << "\n"
           << "Host: " << host.getHostDescription() << "\n"
           << "Format: " << juce::AudioProcessor::getWrapperTypeDescription(processor.wrapperType) << "\n"
           << "OS: " << juce::SystemStats::getOperatingSystemName() << "\n"
           << "CPU: " << juce::SystemStats::getCpuModel() << "\n"
           << "Sample rate: " << juce::String(processor.getSampleRate(), 0) << " Hz, buffer: "
           << processor.getBlockSize() << " samples\n\n"
           << "Settings:\n";

    for (auto* parameter : processor.getParameters())
        if (auto* ranged = dynamic_cast<juce::RangedAudioParameter*>(parameter))
            report << "  " << ranged->paramID << " = " << ranged->getText(ranged->getValue(), 32) << "\n";

    return report;
}

void SupportFooter::reportBug()
{
    // The full report goes on the clipboard (a mailto: link is too short to hold every setting);
    // the email itself carries a short template.
    juce::SystemClipboard::copyTextToClipboard(buildReport());

    const juce::String subject = "EchoPsychFX " + versionText() + " bug report";
    const juce::String body = "What happened, and what did you expect?\n\n\n\n"
                              "Steps to reproduce:\n\n\n\n"
                              "--- Technical details (version, host, settings) are on your clipboard: paste them below ---\n";

    juce::URL("mailto:" + juce::String(supportEmail()))
        .withParameter("subject", subject)
        .withParameter("body", body)
        .launchInDefaultBrowser();
}
