#include "SoundDescriptors.h"
#include <cmath>

std::vector<SoundDescriptor> describeSound(const SoundCharacter& c, const PerceptionProfile& profile)
{
    std::vector<SoundDescriptor> out;
    auto add = [&](const char* tag, const char* clause, Section section) { out.push_back({ tag, clause, section }); };

    // ---- Stereo image: judged by the width the whole chain really produces (see SoundCharacter::widthChangeDb) ----
    // One of: collapsed to mono, very wide, wide, narrow, slightly narrow, or nothing. The "Mono" switch itself is
    // mentioned too, because the effects after it work on the mono sum even when they put the width back.
    if (c.outputSideMidDb <= -20.0f)
        add("Mono", "collapsed to mono", Section::Input);
    else
    {
        if (c.mono)
            add("Mono Input", "summed to mono before the effects", Section::Input);

        if (c.widthChangeDb >= 4.0f)
            add("Very Wide", "very wide, the left and right sides almost unrelated", Section::Input);
        else if (c.widthChangeDb >= 1.5f)
            add("Wide", "wider than the source", Section::Input);
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
        add("Bright", "brighter overall, with the top end lifted", Section::Input);
    else if (c.brightnessShiftDb <= -3.0f)
        add("Warm & Dark", "warmer and darker overall, with the top end eased back", Section::Input);

    // ---- Delay (Motion Shifter): only spoken about when its mix lets it clearly be heard ---------------------
    if (c.delayMix >= 0.15f)
    {
        // The same measure of delay movement that the Motion bar uses, so the chip and the bar agree
        const float delayMotion = c.delayMix * std::min(1.0f, c.delayDepthMs / 6.0f) * std::min(1.0f, c.delayRateHz / 2.0f);

        if (c.delayDepthMs >= 4.0f && delayMotion >= 0.25f)
        {
            if (c.delayRateHz >= 1.0f)
                add("Swirling", "actively swirling with fast modulated echoes", Section::Motion);
            else
                add("Slow Drift", "a slow, deep modulation drifting underneath", Section::Motion);
        }

        if (c.echoTailSeconds >= 3.0f)
            add("Cascading Echoes", "long, cascading echo trails", Section::Motion);
    }

    // ---- Micro-pitch: silent when its mix is down ---------------------------------------------------------
    if (c.microMix >= 0.15f)
    {
        const float detuneAbs = std::abs(c.detuneCents);
        if (detuneAbs > 15.0f)
            add("Unstable Shimmer", "pitch visibly drifting", Section::MicroPitch);
        else if (detuneAbs >= 3.0f)
            add("Shimmering", "a subtle pitch shimmer", Section::MicroPitch);

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

            out.push_back({ f.tag, std::string(strength) + f.what, Section::Exciter });
        }
    }

    // ---- Reverb: judged together with everything else that adds room (echoes, diffusion) via the Space score ----
    {
        const float space = profile.get(PerceptionAxis::Space);
        if (c.reverbTailSeconds >= 2.0f && c.reverbWet >= 0.25f && space >= 0.45f)
            add("Spacious", "a long, spacious reverb tail", Section::Reverb);
        else if (space < 0.15f && c.reverbWet < 0.25f && c.echoTailSeconds < 1.0f)
            add("Close & Dry", "close and dry", Section::Reverb);
        else if (c.reverbWet >= 0.25f && c.reverbTailSeconds > 0.0f && c.reverbTailSeconds < 1.0f)
            add("Tight Room", "a short, tight room reverb", Section::Reverb);
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
    if (saturation >= 0.8f)
        return "dense and driven";
    if (motion >= 0.6f)
        return "restless and constantly shifting";
    const bool notNarrow = c.widthChangeDb >= 1.5f;

    if (space >= 0.75f && width >= 0.65f && notNarrow)
        return "vast, immersive and dreamlike";
    if (space >= 0.6f)
        return "spacious and atmospheric";
    if (width >= 0.65f && notNarrow && space >= 0.35f)
        return "open and airy";
    // "Close" is judged by the same measured width change the Wide chips use, so it never contradicts them
    if (space < 0.35f && c.widthChangeDb < 1.5f && motion < 0.3f)
        return "close, intimate and focused";
    if (brightness < 0.45f)
        return "warm and mellow";
    return {};
}
