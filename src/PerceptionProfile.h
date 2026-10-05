#pragma once

#include <functional>

/** A rough "perception profile": six 0..1 scores describing what a set of plugin settings does to how a
    sound is perceived. They are worked out from the parameter values (not from audio), in the same
    spirit as the feeling chips, so they apply equally to factory presets, user presets and whatever the
    knobs are set to right now. They are heuristics for finding and comparing sounds, not measurements. */
enum class PerceptionAxis { Brightness, Width, Space, Motion, Saturation, Intensity };

constexpr int kNumPerceptionAxes = 6;

struct PerceptionProfile
{
    // 0..1 each. Brightness and Width are 0.5 when the sound is spectrally / spatially untouched.
    float score[kNumPerceptionAxes] = { 0.5f, 0.5f, 0.0f, 0.0f, 0.0f, 0.0f };

    float get(PerceptionAxis axis) const noexcept { return score[static_cast<int>(axis)]; }
};

/** Looks up the plain (un-normalised) value of a plugin parameter by its ID. */
using ParameterGetter = std::function<float(const char* parameterId)>;

PerceptionProfile computePerceptionProfile(const ParameterGetter& parameter);

/** Short name of an axis, e.g. "Brightness". */
const char* perceptionAxisName(PerceptionAxis axis) noexcept;

/** What the high / low end of an axis means, e.g. "brightest" / "darkest" - for sort labels and tooltips. */
const char* perceptionAxisHighWord(PerceptionAxis axis) noexcept;
const char* perceptionAxisLowWord(PerceptionAxis axis) noexcept;
