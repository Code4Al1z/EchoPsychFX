#include "SoundDescriptors.h"
#include <cmath>

std::vector<SoundDescriptor> describeSound(const SoundCharacter& c)
{
    std::vector<SoundDescriptor> out;
    auto add = [&](const char* tag, const char* clause, Section section) { out.push_back({ tag, clause, section }); };

    // ---- Stereo image (one of: mono, very wide, wide, narrow, slightly narrow, or nothing) ----------------
    if (c.mono)
        add("Mono", "collapsed to mono, feeling boxed-in and claustrophobic", Section::Input);
    else if (c.effectiveWidth > 1.3f)
        add("Very Wide", "very wide, pushed well beyond the speakers - feels expansive and larger-than-life", Section::Input);
    else if (c.effectiveWidth > 1.05f)
        add("Wide", "wider than natural, feeling open and airy", Section::Input);
    else if (c.effectiveWidth < 0.7f)
        add("Narrow", "narrow and pulled toward the centre, feeling close and focused", Section::Input);
    else if (c.effectiveWidth < 0.95f)
        add("Slightly Narrow", "slightly narrowed, a touch more centred", Section::Input);

    // ---- Centre / side weighting (after Intensity, as a level difference in dB) --------------------------
    if (!c.mono)
    {
        constexpr float kMinWeightDb = 4.0f;
        if (c.sideVsMidDb <= -kMinWeightDb)
            add("Centre-Weighted", "weighted toward the centre image, feeling solid and grounded", Section::Input);
        else if (c.sideVsMidDb >= kMinWeightDb)
            add("Diffuse Sides", "weighted toward the sides, feeling hazy and enveloping", Section::Input);
    }

    // ---- Left / right pull: the image moves toward the channel that arrives FIRST (precedence effect) ----
    // Even a fraction of a millisecond is a clear inter-ear time difference (the head's own maximum is about
    // 0.7 ms), and the factory presets use delays in exactly that range.
    {
        constexpr float kMinLeadMs = 0.15f;
        if (c.haasLeadMs >= kMinLeadMs)
            add("Pulled Left", "pulled toward the left, the right side arriving a touch later", Section::Spatial);
        else if (c.haasLeadMs <= -kMinLeadMs)
            add("Pulled Right", "pulled toward the right, the left side arriving a touch later", Section::Spatial);
        else if (c.haasMaxMs > 5.0f)
            add("Haas Spread", "spread wide with a Haas-style stereo trick, feeling big without losing focus", Section::Spatial);
    }

    // ---- Brightness (Tilt EQ) -------------------------------------------------------------------------------
    if (c.tilt > 0.15f)
        add("Bright", "brighter and more forward, feeling alert and present", Section::Input);
    else if (c.tilt < -0.15f)
        add("Warm & Dark", "warmer and darker, feeling cosy and enclosed", Section::Input);

    // ---- Delay (Motion Shifter): only spoken about when its mix lets it be heard ---------------------------
    {
        constexpr float kAudibleMix = 0.05f;
        const bool audible = c.delayMix >= kAudibleMix;

        if (!audible)
            add("Inert", "the delay is essentially inaudible, feeling static and untouched", Section::Motion);
        else if (c.delayDepthMs >= 4.0f && c.delayRateHz >= 1.0f)
            add("Swirling", "actively swirling with fast modulated echoes, feeling disorienting and dreamlike", Section::Motion);
        else if (c.delayDepthMs >= 4.0f)
            add("Slow Drift", "a slow, deep modulation drifting underneath, feeling hypnotic", Section::Motion);

        if (audible && c.delayMix >= 0.15f && c.delayFeedback > 0.7f)
            add("Cascading Echoes", "long, cascading echo trails, feeling vast and otherworldly", Section::Motion);
    }

    // ---- Micro-pitch: silent when its mix is down ---------------------------------------------------------
    if (c.microMix >= 0.15f)
    {
        const float detuneAbs = std::abs(c.detuneCents);
        if (detuneAbs > 15.0f)
            add("Unstable Shimmer", "pitch visibly drifting, feeling uncanny and unsettling", Section::MicroPitch);
        else if (detuneAbs >= 3.0f)
            add("Shimmering", "a subtle pitch shimmer, feeling alive and slightly magical", Section::MicroPitch);

        if (c.diffusion > 0.5f)
            add("Blurred Pitch", "blurred and diffuse in pitch, feeling hazy and dreamlike", Section::MicroPitch);
    }

    // ---- Exciter -----------------------------------------------------------------------------------------------
    if (c.exciterMix > 0.15f && c.exciterDrive > 1.0f)
    {
        static const char* satTags[] = {
            "Soft Warmth", "Aggressive Edge", "Tube Warmth",
            "Lo-Fi Character", "Analog Heft", "Cold & Digital"
        };
        static const char* satWords[] = {
            "harmonically excited with a gentle, soft-clipped warmth that feels comforting",
            "harmonically excited with an aggressive, hard-clipped edge that feels tense and confrontational",
            "harmonically excited with a vintage tube warmth that feels nostalgic and cosy",
            "harmonically excited with a lo-fi, tape-worn character that feels nostalgic and familiar",
            "harmonically excited with a weighty, analog-console heft that feels grounded",
            "harmonically excited with a cold, synthetic bite that feels clinical and futuristic"
        };
        add(satTags[c.saturationType], satWords[c.saturationType], Section::Exciter);

        if (c.harmonicMode == 1)
            add("Hollow", "a hollow, reedy harmonic tilt, feeling thin and eerie", Section::Exciter);
        else if (c.harmonicMode == 2)
            add("Rounded", "a warm, rounded harmonic tilt, feeling full and inviting", Section::Exciter);
    }

    // ---- Reverb --------------------------------------------------------------------------------------------------
    if (c.reverbSize > 0.65f && c.reverbWet > 0.45f)
        add("Spacious", "a spacious, distant reverb tail, feeling immersive and awe-inducing", Section::Reverb);
    else if (c.reverbSize < 0.25f && c.reverbWet < 0.25f)
        add("Close & Dry", "close and dry, feeling intimate and immediate", Section::Reverb);

    if (c.predelayMs > 40.0f)
        add("Detached Echo", "a distinct gap before the reverb blooms, like a held breath before it lands", Section::Reverb);

    return out;
}
