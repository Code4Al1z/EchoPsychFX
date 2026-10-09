#pragma once

#include <functional>
#include <string>
#include <vector>

/** Looks up the plain (un-normalised) value of a plugin parameter by its ID. */
using ParameterGetter = std::function<float(const char* parameterId)>;

/** The plugin sections, used to say which part of the plugin a descriptor is talking about. */
enum class Section { Input, Motion, Spatial, MicroPitch, Exciter, Reverb };

/** What a set of knob positions actually does, worked out once and shared by everything that talks about it
    (the perception bars, the feeling chips and the description sentence), so they cannot disagree.

    The raw knob values are not what the ear gets: Intensity scales the width and balance, Sync turns the
    delay Rate into a tempo division, and a mix knob at zero silences everything behind it. This struct holds
    the values after all of that has been taken into account. */
struct SoundCharacter
{
    // Input Controls - the stereo image (applied first)
    bool  mono = false;
    float effectiveWidth = 1.0f;   // 0..2, 1 = natural. Width after the Intensity knob; 0 when mono
    float sideVsMidDb = 0.0f;      // balance after Intensity: positive = more side, negative = more centre
    float tilt = 0.0f;             // -1..1, negative = darker

    // Spatial FX
    float spatialMix = 0.0f;
    float spatialDepth = 0.0f;     // average of the two sides, 0..1
    float spatialRate = 0.0f;      // average of the two sides, Hz
    float haasLeadMs = 0.0f;       // right delay minus left delay: positive = the left side arrives first
    float haasMaxMs = 0.0f;
    float phaseSpread = 0.0f;      // |left phase offset - right phase offset|

    // Motion Shifter (modulated delay)
    float delayMix = 0.0f;
    float delayFeedback = 0.0f;    // average of the two sides
    float delayDepthMs = 0.0f;
    float delayRateHz = 0.0f;      // with Sync on, the tempo division judged at 120 BPM
    float delayTimeMs = 0.0f;

    // Micro-Pitch Detune
    float microMix = 0.0f;
    float detuneCents = 0.0f;      // signed
    float diffusion = 0.0f;
    float stereoSeparation = 0.0f;
    float lfoDepth = 0.0f;
    float lfoRate = 0.0f;

    // Exciter Saturation
    float exciterMix = 0.0f;
    float exciterDrive = 0.0f;     // 0..10
    float exciterBrightness = 0.5f;
    int   saturationType = 0;      // index into Soft, Hard, Tube, Tape, Transformer, Digital
    int   harmonicMode = 0;        // 0 Balanced, 1 Odd Only, 2 Even Only

    // Reverb + Predelay
    float reverbWet = 0.0f;
    float reverbSize = 0.0f;
    float reverbDamping = 0.0f;
    float predelayMs = 0.0f;
};

SoundCharacter computeSoundCharacter(const ParameterGetter& parameter);

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

PerceptionProfile computePerceptionProfile(const SoundCharacter& character);
PerceptionProfile computePerceptionProfile(const ParameterGetter& parameter);

/** Short name of an axis, e.g. "Brightness". */
const char* perceptionAxisName(PerceptionAxis axis) noexcept;

/** What the high / low end of an axis means, e.g. "brightest" / "darkest" - for sort labels and tooltips. */
const char* perceptionAxisHighWord(PerceptionAxis axis) noexcept;
const char* perceptionAxisLowWord(PerceptionAxis axis) noexcept;
