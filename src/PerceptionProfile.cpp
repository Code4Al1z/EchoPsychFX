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

PerceptionProfile computePerceptionProfile(const ParameterGetter& p)
{
    PerceptionProfile profile;

    // --- Shared building blocks -------------------------------------------------------------------
    const bool mono = p("mono") >= 0.5f;
    const float effectiveWidth = mono ? 0.0f : 1.0f + (p("width") - 1.0f) * p("intensity");   // 0..2, 1 = untouched

    const float delayMix = p("modMix");
    const float feedbackAverage = 0.5f * (p("feedbackL") + p("feedbackR"));
    const float spatialMix = p("sfxWetDryMix");
    const float spatialDepth = 0.5f * (p("sfxModDepthL") + p("sfxModDepthR"));
    const float spatialRate = 0.5f * (p("sfxModRateL") + p("sfxModRateR"));
    const float microMix = p("mix");
    const float exciterMix = p("exciterMix");
    const float exciterDrive = clamp01(p("exciterDrive") / 10.0f);
    const float reverbWet = p("wet");
    const float reverbSize = p("size");

    // --- Brightness: tonal balance. 0.5 = untouched; Tilt EQ, added harmonics and a damped reverb move it
    {
        const float excitement = exciterMix * (0.3f + 0.7f * p("exciterToneBrightness")) * (0.5f + 0.5f * exciterDrive);
        profile.score[0] = clamp01(0.5f + 0.50f * p("tiltEQ") + 0.40f * excitement - 0.30f * reverbWet * p("damping"));
    }

    // --- Width: stereo extent. 0.5 = untouched; the width control, plus things that spread a sound sideways
    {
        const float haasDifference = clamp01(std::abs(p("haasDelayL") - p("haasDelayR")) / 10.0f);
        const float spread = combine({ spatialMix * (0.5f * spatialDepth + 0.5f * haasDifference),
                                       microMix * p("stereoSeparation") * (0.4f + 0.6f * clamp01(std::abs(p("detuneAmount")) / 10.0f)),
                                       delayMix * clamp01(p("modDepth") / 5.0f) * 0.5f });
        profile.score[1] = clamp01(0.5f * effectiveWidth + 0.4f * spread);
    }

    // --- Space: how roomy and distant it sounds - reverb, long echoes and diffusion (low = close and dry)
    {
        const float audibleWet = std::pow(reverbWet, 0.7f);   // a quiet reverb is still clearly heard
        const float reverb = audibleWet * (0.3f + 0.7f * reverbSize * reverbSize) + 0.15f * audibleWet * clamp01(p("predelayMs") / 100.0f);
        const float echoes = delayMix * (0.3f + 0.7f * feedbackAverage) * clamp01(p("delayTime") / 400.0f);
        const float diffuse = microMix * p("diffusion") * 0.3f;
        profile.score[2] = clamp01(1.25f * combine({ reverb, 0.7f * echoes, diffuse }));
    }

    // --- Motion: how much it sways, shifts and drifts over time
    {
        // With Sync on, the Rate knob picks a tempo division instead of a speed; judge it at 120 BPM
        const float delayRateHz = p("sync") >= 0.5f
            ? 2.0f * ModDelay::getSyncCyclesPerBeat(ModDelay::getSyncDivisionIndex(p("modRate")))
            : p("modRate");
        const float delayMotion = delayMix * clamp01(p("modDepth") / 6.0f) * clamp01(delayRateHz / 2.0f);
        const float spatialMotion = spatialMix * clamp01(spatialDepth) * clamp01(spatialRate / 2.0f);
        const float microMotion = microMix * (0.6f * clamp01(p("lfoDepth") / 0.004f) * clamp01(p("lfoRate") / 3.0f)
                                              + 0.4f * clamp01(std::abs(p("detuneAmount")) / 20.0f));
        profile.score[3] = clamp01(1.5f * combine({ delayMotion, spatialMotion, microMotion }));
    }

    // --- Saturation: how much harmonic colouring is added
    profile.score[4] = clamp01(exciterMix * (0.15f + 0.85f * exciterDrive));

    // --- Intensity: how heavily processed the sound is overall (0 = untouched)
    {
        const float parts[] = { delayMix * (0.4f + 0.6f * feedbackAverage),
                                spatialMix,
                                microMix,
                                exciterMix * (0.3f + 0.7f * exciterDrive),
                                reverbWet,
                                clamp01(std::abs(effectiveWidth - 1.0f)),
                                std::abs(p("tiltEQ")) };
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
