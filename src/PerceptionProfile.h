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
    float addedHarmonicsDb = -100.0f; // level of the harmonics the exciter adds, in dB relative to the signal
    float exciterHighpassHz = 1000.0f;
    float exciterHarmonicBalance = 0.5f;

    // Reverb + Predelay
    float reverbWet = 0.0f;
    float reverbSize = 0.0f;
    float reverbDamping = 0.0f;
    float predelayMs = 0.0f;

    // ---- What the settings add up to. These are predicted from measurements of the real plugin (a calibration
    // run over hundreds of settings: see the notes in PerceptionProfile.cpp), so the chips and bars describe what
    // is actually heard rather than just where the knobs are.
    float decorrelation = 0.0f;       // 0..1: how much of the output is unrelated left/right ambience
    float widthChangeDb = 0.0f;       // change in side-to-mid level against a typical stereo source (+ = wider)
    float outputSideMidDb = -4.7f;    // the resulting side-to-mid level for that source (about -4.7 dB untouched)
    float brightnessShiftDb = 0.0f;   // change in the highs relative to the lows (+ = brighter)
    float reverbTailSeconds = 0.0f;   // time for the reverb to fall by 40 dB (0 when it is not audible)
    float echoTailSeconds = 0.0f;     // the same for the echo repeats
};

/** Which harmonics the exciter really produces. This is not simply the Harmonics menu: the "Odd Only" setting
    only differs from "Balanced" for the Tube curve, because every other curve is already symmetric and so
    only makes odd harmonics, and "Even Only" adds even harmonics ON TOP of the odd ones rather than
    replacing them. */
enum class HarmonicStructure { Odd, Mixed, EvenAdded };

inline HarmonicStructure harmonicStructureOf(int saturationType, int harmonicMode) noexcept
{
    if (harmonicMode == 2)
        return HarmonicStructure::EvenAdded;
    if (harmonicMode == 1)
        return HarmonicStructure::Odd;
    return saturationType == 2 ? HarmonicStructure::Mixed : HarmonicStructure::Odd;   // only Tube is lopsided
}

/** Estimated level (dB relative to the signal, about -15 dB at the most) of the harmonics the exciter adds.
    The numbers come from measuring the real exciter with a sine wave at -12 dBFS, for every curve and
    structure at drives from 0 to 10, with the Mix knob folded in. Lower than about -36 dB is not audible. */
float estimateAddedHarmonicsDb(float drive, float mix, int saturationType, int harmonicMode) noexcept;

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
