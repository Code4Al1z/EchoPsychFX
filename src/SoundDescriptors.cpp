#include "SoundDescriptors.h"
#include <cmath>

std::vector<SoundDescriptor> describeSound(const SoundCharacter& c, const PerceptionProfile& profile)
{
    std::vector<SoundDescriptor> out;
    const bool tiltIsSmall = std::abs(c.tilt) * 6.0f < 1.5f;   // the Tilt EQ knob reads about 6 dB per unit
    auto add = [&](const char* tag, const std::string& clause, Section section) { out.push_back({ tag, clause, section }); };

    // ---- Stereo image: judged by the width the whole chain really produces (see SoundCharacter::widthChangeDb) ----
    // One of: collapsed to mono, very wide, wide, narrow, slightly narrow, or nothing. The "Mono" switch itself is
    // mentioned too, because the effects after it work on the mono sum even when they put the width back.
    if (c.outputSideMidDb <= -20.0f)
        add("Mono", "collapsed to mono", Section::Input);
    else
    {
        if (c.mono)
            add("Mono Input", "summed to mono before the effects", Section::Input);

        // A low Width knob next to a "wide" claim looks like an error, so say where the width really comes from
        const std::string fromEffects = c.effectiveWidth <= 1.05f ? " (from the effects, not the Width knob)" : "";

        if (c.widthChangeDb >= 4.0f)
            add("Very Wide", "very wide, the left and right sides almost unrelated" + fromEffects, Section::Input);
        else if (c.widthChangeDb >= 1.5f)
            add("Wide", "wider than the source" + fromEffects, Section::Input);
        else if (c.widthChangeDb <= -4.0f)
            add("Narrow", "much narrower than the source", Section::Input);
        else if (c.widthChangeDb <= -1.5f)
            add("Slightly Narrow", "narrower than the source", Section::Input);
    }

    // ---- Centre / side weighting: only when the dry image is still prominent (not buried in wide ambience) -----
    if (!c.mono && c.decorrelation < 0.7f)
    {
        constexpr float kMinWeightDb = 4.0f;
        if (c.sideVsMidDb <= -kMinWeightDb)
            add("Centre-Weighted", "weighted toward the centre image", Section::Input);
        else if (c.sideVsMidDb >= kMinWeightDb)
            add("Diffuse Sides", "weighted toward the sides", Section::Input);
    }

    // ---- Left / right pull: the image moves toward the channel that arrives FIRST (precedence effect) ----------
    // The Haas delays act on Spatial FX's wet sound only, so the pull is only worth mentioning when enough of it is
    // mixed in. Even a fraction of a millisecond is a clear inter-ear time difference (the head's own maximum is
    // about 0.7 ms), and the factory presets use delays in exactly that range.
    if (c.spatialMix >= 0.3f)
    {
        constexpr float kMinLeadMs = 0.15f;
        if (c.haasLeadMs >= kMinLeadMs)
            add("Pulled Left", "pulled toward the left, the right side arriving a touch later", Section::Spatial);
        else if (c.haasLeadMs <= -kMinLeadMs)
            add("Pulled Right", "pulled toward the right, the left side arriving a touch later", Section::Spatial);
        else if (c.haasMaxMs > 5.0f)
            add("Haas Spread", "spread by a Haas-style delay between the channels", Section::Spatial);
    }

    // ---- Brightness: the predicted change in the highs against the lows over the whole chain --------------------
    if (c.brightnessShiftDb >= 3.0f)
        add("Bright", std::string("brighter overall, with the top end lifted")
                                + (tiltIsSmall ? " (from the effects, not the Tilt EQ)" : ""), Section::Input);
    else if (c.brightnessShiftDb <= -3.0f)
        add("Warm & Dark", std::string("warmer and darker overall, with the top end eased back")
                                    + (tiltIsSmall ? " (from the effects, not the Tilt EQ)" : ""), Section::Input);

    // ---- Delay (Motion Shifter): only spoken about when its mix lets it clearly be heard ---------------------
    if (c.delayMix >= 0.15f)
    {
        // The same measure of delay movement that the Motion bar uses, so the chip and the bar agree
        const float delayMotion = delayMotionOf(c);

        if (c.delayDepthMs >= 4.0f && delayMotion >= 0.25f)
        {
            if (c.delayRateHz >= 1.0f)
                add("Swirling", "actively swirling with fast modulated echoes", Section::Motion);
            else
                add("Slow Drift", "a slow, deep modulation drifting underneath", Section::Motion);
        }

        if (c.echoTailSeconds >= 3.0f)
            add("Cascading Echoes", "long, cascading echo trails", Section::Motion);
        else if (c.echoTailSeconds >= 0.5f)
        {
            if (c.delayTimeMs < 120.0f && c.echoTailSeconds < 1.0f)
                add("Slapback", "a short slapback echo", Section::Motion);
            else
                add("Echo", "a clear echo, repeating about every " + std::to_string(static_cast<int>(std::lround(c.delayTimeMs / 10.0f)) * 10) + " ms",
                    Section::Motion);
        }
    }

    // ---- Overall movement: when the Motion bar is high but no delay chip said why, name the biggest source ------
    {
        bool delayAlreadySaidSo = false;
        for (const auto& d : out)
            delayAlreadySaidSo = delayAlreadySaidSo || d.tag == "Swirling" || d.tag == "Slow Drift";

        if (!delayAlreadySaidSo && profile.get(PerceptionAxis::Motion) >= 0.5f)
        {
            const float echoes = delayMotionOf(c), image = spatialMotionOf(c);
            if (image >= echoes && image >= 0.2f)
                add("Shifting Image", "a stereo image that keeps shifting", Section::Spatial);
            else if (echoes >= 0.2f)
                add("Moving Echoes", "echoes that keep wavering", Section::Motion);
        }
    }

    // ---- Micro-pitch: silent when its mix is down ---------------------------------------------------------
    if (c.microMix >= 0.15f)
    {
        const float detuneAbs = std::abs(c.detuneCents);
        if (detuneAbs > 15.0f)
            add("Unstable Shimmer", "pitch visibly drifting", Section::MicroPitch);
        else if (detuneAbs >= 3.0f)
            add("Shimmering", c.microMix * detuneAbs < 2.0f ? "a subtle pitch shimmer" : "a clear pitch shimmer", Section::MicroPitch);

        // The Diffusion knob spreads the three delay taps apart in time (by up to about 2 ms). That thickens and
        // smears the sound like a chorus does - it does not blur the pitch
        if (c.diffusion > 0.5f)
            add("Thickened", "thickened and smeared in time by closely spaced chorus taps", Section::MicroPitch);
    }

    // ---- Exciter: ONE chip that says which harmonics are added and how strongly ----------------------------
    // (Warm and hollow, or cold and rounded, can never be said together: each combination is a single entry.)
    {
        const float level = c.addedHarmonicsDb;
        const char* strength = nullptr;
        if (level >= -20.0f)      strength = "heavy ";
        else if (level >= -27.0f) strength = "";
        else if (level >= -36.0f) strength = "a touch of ";

        if (strength != nullptr)
        {
            struct Flavour { const char* tag; const char* what; };

            //                                              odd harmonics only                          even harmonics added
            static const Flavour soft[]  = { { "Smooth Saturation", "smooth, soft-clipped saturation adding gentle odd harmonics" },
                                             { "Warm Saturation",   "soft saturation with added even harmonics, warm and full" } };
            static const Flavour hard[]  = { { "Hard Clipping",     "hard clipping adding buzzy, edgy odd harmonics" },
                                             { "Thick Clipping",    "hard clipping with added even harmonics, thick and crunchy" } };
            static const Flavour tube[]  = { { "Smooth Saturation", "smooth, soft-clipped saturation adding gentle odd harmonics" },
                                             { "Rich Tube Warmth",  "tube-style saturation with extra even harmonics, rich and warm" } };
            static const Flavour tape[]  = { { "Tape Compression",  "tape-style compression, rounding the peaks with smooth odd harmonics" },
                                             { "Tape Warmth",       "tape-style saturation with added even harmonics, warm and rounded" } };
            static const Flavour xfmr[]  = { { "Analog Heft",       "weighty transformer-style saturation, mostly gentle third-harmonic body" },
                                             { "Analog Warmth",     "transformer-style saturation with added even harmonics, warm and weighty" } };
            static const Flavour dig[]   = { { "Digital Grit",      "8-bit quantisation grit" },
                                             { "Thick Grit",        "8-bit quantisation grit with added even harmonics, thick and crunchy" } };
            static const Flavour* const byType[] = { soft, hard, tube, tape, xfmr, dig };
            static const Flavour tubeMixed = { "Tube Warmth", "tube-style saturation blending even and odd harmonics, warm and full" };

            const auto structure = harmonicStructureOf(c.saturationType, c.harmonicMode);
            const Flavour& f = structure == HarmonicStructure::Mixed ? tubeMixed
                             : byType[c.saturationType][structure == HarmonicStructure::EvenAdded ? 1 : 0];

            // A light touch must not wear a "Rich" or "Thick" label: the chip shows strength the way the sentence does
            std::string tag = f.tag;
            if (std::string(strength) == "a touch of ")
            {
                for (const char* prefix : { "Rich ", "Thick ", "Smooth " })
                    if (tag.rfind(prefix, 0) == 0) { tag = tag.substr(std::string(prefix).size()); break; }
                tag = "Subtle " + tag;
            }

            out.push_back({ tag, std::string(strength) + f.what, Section::Exciter });
        }
    }

    // ---- Reverb: judged together with everything else that adds room (echoes, diffusion) via the Space score ----
    {
        const float space = profile.get(PerceptionAxis::Space);
        if (c.reverbTailSeconds >= 2.0f && c.reverbWet >= 0.25f && space >= 0.45f)
            add("Spacious", "a long, spacious reverb tail", Section::Reverb);
        else if (space < 0.15f && c.reverbWet < 0.25f && c.echoTailSeconds < 0.5f)
            add("Close & Dry", "close and dry", Section::Reverb);
        else if (c.reverbWet >= 0.25f && c.reverbTailSeconds > 0.0f && c.reverbTailSeconds < 1.0f)
            add("Short Reverb", "a short reverb tail", Section::Reverb);
        else if (c.reverbWet >= 0.25f && c.reverbTailSeconds >= 1.0f)
            add("Reverb Tail", "a reverb tail of about " + std::to_string(static_cast<int>(std::lround(c.reverbTailSeconds))) + " s", Section::Reverb);
    }

    // A pre-delay this long lets the dry sound land clearly before the reverb arrives
    if (c.predelayMs >= 60.0f && c.reverbWet >= 0.25f)
        add("Late Bloom", "a clear gap before the reverb arrives", Section::Reverb);

    return out;
}

std::string describeOverallFeel(const SoundCharacter& c, const PerceptionProfile& profile)
{
    const float brightness = profile.get(PerceptionAxis::Brightness);
    const float width = profile.get(PerceptionAxis::Width);
    const float space = profile.get(PerceptionAxis::Space);
    const float motion = profile.get(PerceptionAxis::Motion);
    const float saturation = profile.get(PerceptionAxis::Saturation);

    // The first rule that fits wins. Order = how strongly each one dominates what a listener notices.
    const bool clearlyDriven = c.addedHarmonicsDb >= -24.0f;

    if (c.saturationType == 1 && clearlyDriven)        // hard clipping
        return "tense and aggressive";
    if (c.saturationType == 5 && clearlyDriven)        // bit-crushing
        return "gritty and synthetic";
    // (A big, wide, roomy sound with a heavy but gentle saturator is described by its space, not by the saturator)
    if (saturation >= 0.8f && space < 0.6f)
        return "dense and driven";
    if (motion >= 0.6f)
        return "restless and constantly shifting";
    const bool notNarrow = c.widthChangeDb >= 1.5f;

    if (space >= 0.75f && width >= 0.65f && notNarrow)
        return "vast, immersive and dreamlike";
    if (space >= 0.6f)
        return "spacious and atmospheric";
    if (width >= 0.65f && notNarrow && space >= 0.35f)
        return brightness < 0.45f ? "warm and enveloping" : "open and airy";
    // "Close" is judged by the same measured width change the Wide chips use, so it never contradicts them
    if (space < 0.35f && c.widthChangeDb < 1.5f && motion < 0.3f)
        return "close, intimate and focused";
    if (brightness < 0.45f)
        return "warm and mellow";
    return {};
}
