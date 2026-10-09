#include "PerceptionProfile.h"
#include "ModDelay.h"
#include <algorithm>
#include <cmath>

namespace
{
    float clamp01(float x) noexcept { return std::min(1.0f, std::max(0.0f, x)); }

    // Combines independent contributions into one 0..1 amount: each one fills part of what is left.
    float combine(std::initializer_list<float> parts) noexcept
    {
        float remaining = 1.0f;
        for (float p : parts)
            remaining *= 1.0f - clamp01(p);
        return 1.0f - remaining;
    }
}

float estimateAddedHarmonicsDb(float drive, float mix, int saturationType, int harmonicMode) noexcept
{
    // Added harmonic level (dB re the signal) at Mix = 1, measured at these Drive settings
    static constexpr float kDrive[11] = { 0.0f, 0.5f, 1.0f, 1.5f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 8.0f, 10.0f };

    //                                       0      0.5     1      1.5     2      3      4      5      6      8      10
    static constexpr float oddSoft[11]  = { -51.9f, -40.6f, -34.3f, -30.1f, -27.1f, -23.2f, -20.8f, -19.2f, -18.1f, -16.8f, -16.1f };
    static constexpr float oddTape[11]  = { -39.2f, -31.6f, -27.3f, -24.6f, -22.7f, -20.2f, -18.8f, -17.8f, -17.1f, -16.3f, -15.8f };
    static constexpr float oddTrans[11] = { -58.6f, -46.8f, -39.6f, -34.3f, -29.9f, -22.8f, -19.5f, -17.9f, -17.0f, -15.9f, -15.4f };
    // Hard clipping and bit-crushing add nothing until the signal reaches the clip point (about Drive 1.6 for a
    // -12 dBFS tone); the early part of this curve allows for real program material peaking higher than that
    static constexpr float oddClip[11]  = { -60.0f, -52.0f, -45.0f, -37.0f, -28.7f, -20.8f, -18.4f, -17.1f, -16.4f, -15.6f, -15.2f };
    static constexpr float mixedTube[11] = { -43.7f, -36.9f, -32.4f, -29.1f, -26.5f, -22.9f, -20.7f, -19.2f, -18.1f, -16.8f, -16.1f };

    static constexpr float evenSoft[11] = { -33.4f, -28.0f, -25.1f, -23.3f, -22.1f, -20.3f, -19.0f, -18.0f, -17.3f, -16.3f, -15.7f };
    static constexpr float evenClip[11] = { -33.2f, -27.4f, -24.0f, -21.6f, -20.6f, -18.6f, -17.3f, -16.5f, -15.9f, -15.3f, -15.0f };
    static constexpr float evenTube[11] = { -36.3f, -30.5f, -27.2f, -24.9f, -23.2f, -20.8f, -19.2f, -18.1f, -17.3f, -16.3f, -15.7f };
    static constexpr float evenTape[11] = { -30.3f, -25.5f, -23.0f, -21.5f, -20.4f, -18.8f, -17.8f, -17.1f, -16.6f, -15.9f, -15.5f };
    static constexpr float evenTrans[11] = { -33.3f, -27.8f, -24.7f, -22.9f, -21.6f, -19.6f, -18.0f, -17.0f, -16.4f, -15.6f, -15.2f };

    const float* table = oddSoft;
    switch (harmonicStructureOf(saturationType, harmonicMode))
    {
    case HarmonicStructure::Mixed:
        table = mixedTube;
        break;
    case HarmonicStructure::Odd:
        switch (saturationType)
        {
        case 1: case 5: table = oddClip;  break;   // Hard, Digital
        case 3:         table = oddTape;  break;   // Tape
        case 4:         table = oddTrans; break;   // Transformer
        default:        table = oddSoft;  break;   // Soft, and Tube with its lopsidedness removed
        }
        break;
    case HarmonicStructure::EvenAdded:
        switch (saturationType)
        {
        case 1: case 5: table = evenClip;  break;
        case 2:         table = evenTube;  break;
        case 3:         table = evenTape;  break;
        case 4:         table = evenTrans; break;
        default:        table = evenSoft;  break;
        }
        break;
    }

    const float d = std::min(10.0f, std::max(0.0f, drive));
    int i = 0;
    while (i < 9 && d > kDrive[i + 1])
        ++i;
    const float t = (d - kDrive[i]) / (kDrive[i + 1] - kDrive[i]);
    const float atFullMix = table[i] + t * (table[i + 1] - table[i]);

    // The excited band is layered on top of the dry signal, so the signal grows with Mix as well as the harmonics
    const float m = std::max(mix, 1.0e-4f);
    return atFullMix + 20.0f * std::log10(2.0f * m / (1.0f + m));
}

SoundCharacter computeSoundCharacter(const ParameterGetter& p)
{
    SoundCharacter c;

    // --- Stereo image. The width and balance controls are scaled by Intensity (that is how the DSP applies them)
    const float intensity = p("intensity");
    c.mono = p("mono") >= 0.5f;
    c.effectiveWidth = c.mono ? 0.0f : 1.0f + (p("width") - 1.0f) * intensity;

    {
        // Same maths as WidthBalancer: balance picks an angle between "all centre" and "all side"
        const float angle = (p("midSideBalance") * 0.5f + 0.5f) * 1.57079633f;
        const float midGain = 1.0f + (1.41421356f * std::cos(angle) - 1.0f) * intensity;
        const float sideGain = 1.0f + (1.41421356f * std::sin(angle) - 1.0f) * intensity;
        c.sideVsMidDb = 20.0f * std::log10(std::max(sideGain, 1.0e-3f) / std::max(midGain, 1.0e-3f));
    }

    c.tilt = p("tiltEQ");

    // --- Spatial FX
    c.spatialMix = p("sfxWetDryMix");
    c.spatialDepth = 0.5f * (p("sfxModDepthL") + p("sfxModDepthR"));
    c.spatialRate = 0.5f * (p("sfxModRateL") + p("sfxModRateR"));
    c.haasLeadMs = p("haasDelayR") - p("haasDelayL");
    c.haasMaxMs = std::max(p("haasDelayL"), p("haasDelayR"));
    c.phaseSpread = std::abs(p("phaseOffsetL") - p("phaseOffsetR"));

    // --- Motion Shifter
    c.delayMix = p("modMix");
    c.delayFeedback = 0.5f * (p("feedbackL") + p("feedbackR"));
    c.delayDepthMs = p("modDepth");
    c.delayTimeMs = p("delayTime");
    // With Sync on, the Rate knob picks a tempo division instead of a speed; judge it at 120 BPM
    c.delayRateHz = p("sync") >= 0.5f
        ? 2.0f * ModDelay::getSyncCyclesPerBeat(ModDelay::getSyncDivisionIndex(p("modRate")))
        : p("modRate");

    // --- Micro-Pitch Detune
    c.microMix = p("mix");
    c.detuneCents = p("detuneAmount");
    c.diffusion = p("diffusion");
    c.stereoSeparation = p("stereoSeparation");
    c.lfoDepth = p("lfoDepth");
    c.lfoRate = p("lfoRate");

    // --- Exciter Saturation
    c.exciterMix = p("exciterMix");
    c.exciterDrive = p("exciterDrive");
    c.exciterBrightness = p("exciterToneBrightness");
    c.saturationType = std::min(5, std::max(0, static_cast<int>(std::lround(p("exciterSaturationType")))));
    c.harmonicMode = std::min(2, std::max(0, static_cast<int>(std::lround(p("exciterHarmonicMode")))));
    c.addedHarmonicsDb = estimateAddedHarmonicsDb(c.exciterDrive, c.exciterMix, c.saturationType, c.harmonicMode);

    // --- Reverb
    c.reverbWet = p("wet");
    c.reverbSize = p("size");
    c.reverbDamping = p("damping");
    c.predelayMs = p("predelayMs");

    return c;
}

PerceptionProfile computePerceptionProfile(const ParameterGetter& parameter)
{
    return computePerceptionProfile(computeSoundCharacter(parameter));
}

PerceptionProfile computePerceptionProfile(const SoundCharacter& c)
{
    PerceptionProfile profile;

    // --- Shared building blocks -------------------------------------------------------------------
    const float effectiveWidth = c.effectiveWidth;   // 0..2, 1 = untouched
    const float exciterDrive = clamp01(c.exciterDrive / 10.0f);

    // --- Brightness: tonal balance. 0.5 = untouched; Tilt EQ, added harmonics and a damped reverb move it
    {
        const float excitement = c.exciterMix * (0.3f + 0.7f * c.exciterBrightness) * (0.5f + 0.5f * exciterDrive);
        profile.score[0] = clamp01(0.5f + 0.50f * c.tilt + 0.40f * excitement - 0.30f * c.reverbWet * c.reverbDamping);
    }

    // --- Width: stereo extent. 0.5 = untouched; the width control, plus things that spread a sound sideways
    {
        const float haasDifference = clamp01(std::abs(c.haasLeadMs) / 10.0f);
        const float spread = combine({ c.spatialMix * (0.5f * c.spatialDepth + 0.5f * haasDifference),
                                       c.microMix * c.stereoSeparation * (0.4f + 0.6f * clamp01(std::abs(c.detuneCents) / 10.0f)),
                                       c.delayMix * clamp01(c.delayDepthMs / 5.0f) * 0.5f });
        profile.score[1] = clamp01(0.5f * effectiveWidth + 0.4f * spread);
    }

    // --- Space: how roomy and distant it sounds - reverb, long echoes and diffusion (low = close and dry)
    {
        const float audibleWet = std::pow(c.reverbWet, 0.7f);   // a quiet reverb is still clearly heard
        const float reverb = audibleWet * (0.3f + 0.7f * c.reverbSize * c.reverbSize) + 0.15f * audibleWet * clamp01(c.predelayMs / 100.0f);
        const float echoes = c.delayMix * (0.3f + 0.7f * c.delayFeedback) * clamp01(c.delayTimeMs / 400.0f);
        const float diffuse = c.microMix * c.diffusion * 0.3f;
        profile.score[2] = clamp01(1.25f * combine({ reverb, 0.7f * echoes, diffuse }));
    }

    // --- Motion: how much it sways, shifts and drifts over time
    {
        const float delayMotion = c.delayMix * clamp01(c.delayDepthMs / 6.0f) * clamp01(c.delayRateHz / 2.0f);
        const float spatialMotion = c.spatialMix * clamp01(c.spatialDepth) * clamp01(c.spatialRate / 2.0f);
        const float microMotion = c.microMix * (0.6f * clamp01(c.lfoDepth / 0.004f) * clamp01(c.lfoRate / 3.0f)
                                                + 0.4f * clamp01(std::abs(c.detuneCents) / 20.0f));
        profile.score[3] = clamp01(1.5f * combine({ delayMotion, spatialMotion, microMotion }));
    }

    // --- Saturation: how much harmonic colouring is added, from the measured level of the added harmonics.
    //     -40 dB or less is not audible (0), about -27 dB is clearly there (0.5), -15 dB is the most the exciter adds (1)
    profile.score[4] = clamp01((c.addedHarmonicsDb + 40.0f) / 25.0f);

    // --- Intensity: how heavily processed the sound is overall (0 = untouched)
    {
        const float parts[] = { c.delayMix * (0.4f + 0.6f * c.delayFeedback),
                                c.spatialMix,
                                c.microMix,
                                c.exciterMix * (0.3f + 0.7f * exciterDrive),
                                c.reverbWet,
                                clamp01(std::abs(effectiveWidth - 1.0f)),
                                std::abs(c.tilt) };
        float remaining = 1.0f;
        for (float part : parts)
            remaining *= 1.0f - 0.45f * clamp01(part);
        profile.score[5] = 1.0f - remaining;
    }

    return profile;
}

const char* perceptionAxisName(PerceptionAxis axis) noexcept
{
    switch (axis)
    {
    case PerceptionAxis::Brightness: return "Brightness";
    case PerceptionAxis::Width:      return "Width";
    case PerceptionAxis::Space:      return "Space";
    case PerceptionAxis::Motion:     return "Motion";
    case PerceptionAxis::Saturation: return "Saturation";
    case PerceptionAxis::Intensity:  return "Intensity";
    }
    return "";
}

const char* perceptionAxisHighWord(PerceptionAxis axis) noexcept
{
    switch (axis)
    {
    case PerceptionAxis::Brightness: return "brightest";
    case PerceptionAxis::Width:      return "widest";
    case PerceptionAxis::Space:      return "roomiest";
    case PerceptionAxis::Motion:     return "most movement";
    case PerceptionAxis::Saturation: return "most driven";
    case PerceptionAxis::Intensity:  return "most processed";
    }
    return "";
}

const char* perceptionAxisLowWord(PerceptionAxis axis) noexcept
{
    switch (axis)
    {
    case PerceptionAxis::Brightness: return "darkest";
    case PerceptionAxis::Width:      return "narrowest";
    case PerceptionAxis::Space:      return "driest";
    case PerceptionAxis::Motion:     return "most static";
    case PerceptionAxis::Saturation: return "cleanest";
    case PerceptionAxis::Intensity:  return "most subtle";
    }
    return "";
}
