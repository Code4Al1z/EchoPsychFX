#include "FactoryPresets.h"

namespace
{
    // Each preset is its own raw string literal (rather than one huge one) because MSVC refuses a single
    // string literal longer than about 16 KB. The keys are the plugin's parameter IDs, so a preset reads
    // as "set these parameters to these values" and can't be mis-ordered the way a long argument list can.
    const char* const kFactoryPresetJson[] =
    {
R"JSON({
  "name": "Init",
  "description": "Blank / Init: every effect neutral or fully dry, a clean starting point for building a custom sound rather than starting from one of the character presets below.",
  "params": {
    "modulationType": "Sine", "delayTime": 100.0, "feedbackL": 0.0, "feedbackR": 0.0, "modMix": 0.0, "modDepth": 0.0, "modRate": 0.25, "sync": false,
    "width": 1.0, "intensity": 0.0, "midSideBalance": 0.0, "mono": false, "tiltEQ": 0.0,
    "phaseOffsetL": 0.0, "phaseOffsetR": 0.0, "sfxModRateL": 0.01, "sfxModRateR": 0.01, "sfxModDepthL": 0.0, "sfxModDepthR": 0.0, "sfxWetDryMix": 0.0, "sfxLfoPhaseOffset": 0.0, "sfxAllpassFreq": 1000.0, "haasDelayL": 0.0, "haasDelayR": 0.0, "modulationShape": "Sine",
    "detuneAmount": 0.0, "lfoRate": 0.1, "lfoDepth": 0.0, "delayCentre": 0.005, "stereoSeparation": 0.0, "mix": 0.0, "detuneFeedback": 0.0, "diffusion": 0.0,
    "exciterDrive": 0.0, "exciterMix": 0.0, "exciterHighpass": 1000.0, "exciterSaturationType": "Soft", "exciterHarmonicMode": "Natural", "exciterToneBrightness": 0.5, "exciterHarmonicBalance": 0.5, "exciterAutoGain": true,
    "predelayMs": 0.0, "size": 0.0, "damping": 0.3, "wet": 0.0
  }
})JSON",

R"JSON({
  "name": "Head Trip",
  "description": "Swirling, warped, disorienting - a heady psychedelic trip.",
  "params": {
    "modulationType": "Triangle", "delayTime": 400.0, "feedbackL": 0.7, "feedbackR": 0.75, "modMix": 0.6, "modDepth": 4.0, "modRate": 0.2, "sync": false,
    "width": 1.5, "intensity": 0.8, "midSideBalance": 0.0, "mono": false, "tiltEQ": 0.1,
    "phaseOffsetL": 0.08, "phaseOffsetR": -0.05, "sfxModRateL": 0.3, "sfxModRateR": 0.3, "sfxModDepthL": 0.6, "sfxModDepthR": 0.6, "sfxWetDryMix": 0.7, "sfxLfoPhaseOffset": 0.25, "sfxAllpassFreq": 2500.0, "haasDelayL": 0.5, "haasDelayR": 0.6, "modulationShape": "Sine",
    "detuneAmount": 3.0, "lfoRate": 0.25, "lfoDepth": 0.0015, "delayCentre": 0.006, "stereoSeparation": 0.8, "mix": 0.5, "detuneFeedback": 0.45, "diffusion": 0.6,
    "exciterDrive": 6.0, "exciterMix": 0.4, "exciterHighpass": 100.0, "exciterSaturationType": "Tube", "exciterHarmonicMode": "Odd Only", "exciterToneBrightness": 0.6, "exciterHarmonicBalance": 0.55, "exciterAutoGain": true,
    "predelayMs": 80.0, "size": 0.85, "damping": 0.5, "wet": 0.4
  }
})JSON",

R"JSON({
  "name": "Panic Room",
  "description": "Claustrophobic and tense - a rhythmic, harsh pulse that won't let up.",
  "params": {
    "modulationType": "Square", "delayTime": 150.0, "feedbackL": 0.8, "feedbackR": 0.7, "modMix": 0.9, "modDepth": 4.0, "modRate": 8.0, "sync": true,
    "width": 0.3, "intensity": 0.2, "midSideBalance": 0.2, "mono": false, "tiltEQ": -0.15,
    "phaseOffsetL": 0.1, "phaseOffsetR": -0.1, "sfxModRateL": 1.2, "sfxModRateR": 0.8, "sfxModDepthL": 0.85, "sfxModDepthR": 1.1, "sfxWetDryMix": 0.9, "sfxLfoPhaseOffset": 0.2, "sfxAllpassFreq": 3200.0, "haasDelayL": 0.1, "haasDelayR": 0.2, "modulationShape": "Triangle",
    "detuneAmount": -5.0, "lfoRate": 3.0, "lfoDepth": 0.0008, "delayCentre": 0.003, "stereoSeparation": 0.3, "mix": 0.7, "detuneFeedback": 0.15, "diffusion": 0.15,
    "exciterDrive": 7.5, "exciterMix": 0.65, "exciterHighpass": 200.0, "exciterSaturationType": "Hard", "exciterHarmonicMode": "Odd Only", "exciterToneBrightness": 0.75, "exciterHarmonicBalance": 0.6, "exciterAutoGain": false,
    "predelayMs": 20.0, "size": 0.3, "damping": 0.85, "wet": 0.25
  }
})JSON",

R"JSON({
  "name": "Intimacy",
  "description": "Close, warm, gentle - a soft tube glow instead of any harshness.",
  "params": {
    "modulationType": "Sine", "delayTime": 80.0, "feedbackL": 0.3, "feedbackR": 0.35, "modMix": 0.2, "modDepth": 1.0, "modRate": 0.1, "sync": false,
    "width": 0.6, "intensity": 0.9, "midSideBalance": -0.1, "mono": false, "tiltEQ": 0.05,
    "phaseOffsetL": 0.03, "phaseOffsetR": -0.02, "sfxModRateL": 0.15, "sfxModRateR": 0.15, "sfxModDepthL": 0.25, "sfxModDepthR": 0.25, "sfxWetDryMix": 0.2, "sfxLfoPhaseOffset": 0.1, "sfxAllpassFreq": 1600.0, "haasDelayL": 0.2, "haasDelayR": 0.2, "modulationShape": "Sine",
    "detuneAmount": 1.5, "lfoRate": 0.1, "lfoDepth": 0.0002, "delayCentre": 0.001, "stereoSeparation": 0.4, "mix": 0.15, "detuneFeedback": 0.05, "diffusion": 0.2,
    "exciterDrive": 2.0, "exciterMix": 0.2, "exciterHighpass": 50.0, "exciterSaturationType": "Tube", "exciterHarmonicMode": "Add Even", "exciterToneBrightness": 0.4, "exciterHarmonicBalance": 0.45, "exciterAutoGain": true,
    "predelayMs": 15.0, "size": 0.25, "damping": 0.3, "wet": 0.3
  }
})JSON",

R"JSON({
  "name": "Blade Runner",
  "description": "Cold, synthetic, neon-lit dystopia, driven by a mechanical pulse.",
  "params": {
    "modulationType": "Sawtooth Down", "delayTime": 550.0, "feedbackL": 0.57, "feedbackR": 0.53, "modMix": 0.64, "modDepth": 3.0, "modRate": 6.0, "sync": true,
    "width": 1.2, "intensity": 0.7, "midSideBalance": 0.1, "mono": false, "tiltEQ": -0.08,
    "phaseOffsetL": 0.1, "phaseOffsetR": -0.1, "sfxModRateL": 0.4, "sfxModRateR": 0.6, "sfxModDepthL": 0.5, "sfxModDepthR": 0.7, "sfxWetDryMix": 0.85, "sfxLfoPhaseOffset": 0.3, "sfxAllpassFreq": 4200.0, "haasDelayL": 0.7, "haasDelayR": 0.3, "modulationShape": "Random",
    "detuneAmount": -2.0, "lfoRate": 0.6, "lfoDepth": 0.0018, "delayCentre": 0.004, "stereoSeparation": 0.75, "mix": 0.6, "detuneFeedback": 0.28, "diffusion": 0.5,
    "exciterDrive": 4.5, "exciterMix": 0.55, "exciterHighpass": 120.0, "exciterSaturationType": "Digital", "exciterHarmonicMode": "Odd Only", "exciterToneBrightness": 0.65, "exciterHarmonicBalance": 0.5, "exciterAutoGain": true,
    "predelayMs": 100.0, "size": 0.89, "damping": 0.6, "wet": 0.45
  }
})JSON",

R"JSON({
  "name": "Alien Abduction",
  "description": "Otherworldly and non-human - electromagnetic hum and a smooth, alien shimmer.",
  "params": {
    "modulationType": "Sine", "delayTime": 350.0, "feedbackL": 0.55, "feedbackR": 0.6, "modMix": 0.78, "modDepth": 4.5, "modRate": 0.6, "sync": false,
    "width": 0.9, "intensity": 0.75, "midSideBalance": 0.05, "mono": false, "tiltEQ": -0.07,
    "phaseOffsetL": 0.1, "phaseOffsetR": -0.1, "sfxModRateL": 0.9, "sfxModRateR": 0.9, "sfxModDepthL": 0.7, "sfxModDepthR": 0.7, "sfxWetDryMix": 0.8, "sfxLfoPhaseOffset": 0.45, "sfxAllpassFreq": 4200.0, "haasDelayL": 0.6, "haasDelayR": 0.6, "modulationShape": "Sine",
    "detuneAmount": 4.0, "lfoRate": 1.2, "lfoDepth": 0.001, "delayCentre": 0.0025, "stereoSeparation": 0.9, "mix": 0.65, "detuneFeedback": 0.5, "diffusion": 0.55,
    "exciterDrive": 5.5, "exciterMix": 0.6, "exciterHighpass": 180.0, "exciterSaturationType": "Transformer", "exciterHarmonicMode": "Add Even", "exciterToneBrightness": 0.7, "exciterHarmonicBalance": 0.6, "exciterAutoGain": true,
    "predelayMs": 100.0, "size": 0.85, "damping": 0.4, "wet": 0.5
  }
})JSON",

R"JSON({
  "name": "Glass Tunnel",
  "description": "Smooth, reflective, and clean - light bouncing down a long glass corridor.",
  "params": {
    "modulationType": "Sawtooth Up", "delayTime": 220.0, "feedbackL": 0.45, "feedbackR": 0.5, "modMix": 0.65, "modDepth": 1.8, "modRate": 0.15, "sync": false,
    "width": 0.7, "intensity": 0.85, "midSideBalance": -0.05, "mono": false, "tiltEQ": 0.02,
    "phaseOffsetL": 0.05, "phaseOffsetR": -0.08, "sfxModRateL": 0.25, "sfxModRateR": 0.25, "sfxModDepthL": 0.4, "sfxModDepthR": 0.4, "sfxWetDryMix": 0.65, "sfxLfoPhaseOffset": 0.2, "sfxAllpassFreq": 3200.0, "haasDelayL": 0.5, "haasDelayR": 0.5, "modulationShape": "Triangle",
    "detuneAmount": 1.0, "lfoRate": 0.3, "lfoDepth": 0.0005, "delayCentre": 0.0015, "stereoSeparation": 0.55, "mix": 0.4, "detuneFeedback": 0.2, "diffusion": 0.65,
    "exciterDrive": 3.5, "exciterMix": 0.35, "exciterHighpass": 80.0, "exciterSaturationType": "Soft", "exciterHarmonicMode": "Add Even", "exciterToneBrightness": 0.75, "exciterHarmonicBalance": 0.5, "exciterAutoGain": true,
    "predelayMs": 60.0, "size": 0.65, "damping": 0.8, "wet": 0.35
  }
})JSON",

R"JSON({
  "name": "Dream Logic",
  "description": "Surreal, illogical flow - warm and hazy the way dreams drift from scene to scene.",
  "params": {
    "modulationType": "Sine", "delayTime": 280.0, "feedbackL": 0.5, "feedbackR": 0.55, "modMix": 0.75, "modDepth": 2.2, "modRate": 0.25, "sync": false,
    "width": 0.88, "intensity": 0.78, "midSideBalance": 0.02, "mono": false, "tiltEQ": -0.03,
    "phaseOffsetL": 0.06, "phaseOffsetR": -0.04, "sfxModRateL": 0.35, "sfxModRateR": 0.35, "sfxModDepthL": 0.48, "sfxModDepthR": 0.48, "sfxWetDryMix": 0.55, "sfxLfoPhaseOffset": 0.3, "sfxAllpassFreq": 3000.0, "haasDelayL": 0.4, "haasDelayR": 0.4, "modulationShape": "Sine",
    "detuneAmount": 1.8, "lfoRate": 0.5, "lfoDepth": 0.0007, "delayCentre": 0.002, "stereoSeparation": 0.6, "mix": 0.45, "detuneFeedback": 0.3, "diffusion": 0.6,
    "exciterDrive": 3.0, "exciterMix": 0.3, "exciterHighpass": 70.0, "exciterSaturationType": "Tape", "exciterHarmonicMode": "Natural", "exciterToneBrightness": 0.55, "exciterHarmonicBalance": 0.5, "exciterAutoGain": true,
    "predelayMs": 90.0, "size": 0.75, "damping": 0.45, "wet": 0.45
  }
})JSON",

R"JSON({
  "name": "Womb Space",
  "description": "Enclosed, muffled, primal safety - dark and contained, not vast.",
  "params": {
    "modulationType": "Sine", "delayTime": 90.0, "feedbackL": 0.35, "feedbackR": 0.4, "modMix": 0.2, "modDepth": 0.7, "modRate": 0.06, "sync": false,
    "width": 0.55, "intensity": 1.0, "midSideBalance": -0.25, "mono": true, "tiltEQ": -0.12,
    "phaseOffsetL": 0.02, "phaseOffsetR": -0.015, "sfxModRateL": 0.08, "sfxModRateR": 0.08, "sfxModDepthL": 0.15, "sfxModDepthR": 0.15, "sfxWetDryMix": 0.3, "sfxLfoPhaseOffset": 0.05, "sfxAllpassFreq": 2000.0, "haasDelayL": 0.2, "haasDelayR": 0.2, "modulationShape": "Sine",
    "detuneAmount": 0.3, "lfoRate": 0.1, "lfoDepth": 0.0001, "delayCentre": 0.001, "stereoSeparation": 0.25, "mix": 0.2, "detuneFeedback": 0.1, "diffusion": 0.3,
    "exciterDrive": 1.0, "exciterMix": 0.15, "exciterHighpass": 20.0, "exciterSaturationType": "Tube", "exciterHarmonicMode": "Add Even", "exciterToneBrightness": 0.15, "exciterHarmonicBalance": 0.4, "exciterAutoGain": true,
    "predelayMs": 5.0, "size": 0.35, "damping": 0.65, "wet": 0.35
  }
})JSON",

R"JSON({
  "name": "Bipolar Bloom",
  "description": "Unstable mood swings - erratic modulation and dynamics that surge and sag.",
  "params": {
    "modulationType": "Triangle", "delayTime": 450.0, "feedbackL": 0.65, "feedbackR": 0.45, "modMix": 0.74, "modDepth": 4.0, "modRate": 0.35, "sync": false,
    "width": 1.1, "intensity": 0.65, "midSideBalance": 0.15, "mono": false, "tiltEQ": 0.08,
    "phaseOffsetL": 0.1, "phaseOffsetR": -0.1, "sfxModRateL": 0.7, "sfxModRateR": 0.7, "sfxModDepthL": 0.65, "sfxModDepthR": 0.65, "sfxWetDryMix": 0.75, "sfxLfoPhaseOffset": 0.3, "sfxAllpassFreq": 4200.0, "haasDelayL": 0.3, "haasDelayR": 0.25, "modulationShape": "Random",
    "detuneAmount": -3.5, "lfoRate": 1.5, "lfoDepth": 0.0009, "delayCentre": 0.0022, "stereoSeparation": 0.95, "mix": 0.68, "detuneFeedback": 0.4, "diffusion": 0.45,
    "exciterDrive": 6.5, "exciterMix": 0.5, "exciterHighpass": 110.0, "exciterSaturationType": "Hard", "exciterHarmonicMode": "Odd Only", "exciterToneBrightness": 0.6, "exciterHarmonicBalance": 0.55, "exciterAutoGain": false,
    "predelayMs": 70.0, "size": 0.89, "damping": 0.35, "wet": 0.49
  }
})JSON",

R"JSON({
  "name": "Quiet Confidence",
  "description": "Calm and grounded - a subtle, weighty confidence rather than showiness.",
  "params": {
    "modulationType": "Triangle", "delayTime": 120.0, "feedbackL": 0.4, "feedbackR": 0.45, "modMix": 0.2, "modDepth": 1.2, "modRate": 0.12, "sync": false,
    "width": 0.5, "intensity": 0.9, "midSideBalance": -0.02, "mono": false, "tiltEQ": 0.03,
    "phaseOffsetL": 0.04, "phaseOffsetR": -0.03, "sfxModRateL": 0.2, "sfxModRateR": 0.2, "sfxModDepthL": 0.35, "sfxModDepthR": 0.35, "sfxWetDryMix": 0.15, "sfxLfoPhaseOffset": 0.2, "sfxAllpassFreq": 2400.0, "haasDelayL": 0.15, "haasDelayR": 0.2, "modulationShape": "Sine",
    "detuneAmount": 1.2, "lfoRate": 0.2, "lfoDepth": 0.0004, "delayCentre": 0.0012, "stereoSeparation": 0.45, "mix": 0.1, "detuneFeedback": 0.1, "diffusion": 0.25,
    "exciterDrive": 2.8, "exciterMix": 0.2, "exciterHighpass": 60.0, "exciterSaturationType": "Transformer", "exciterHarmonicMode": "Natural", "exciterToneBrightness": 0.45, "exciterHarmonicBalance": 0.5, "exciterAutoGain": true,
    "predelayMs": 20.0, "size": 0.45, "damping": 0.25, "wet": 0.3
  }
})JSON",

R"JSON({
  "name": "Falling Upwards",
  "description": "Weightless and paradoxically uplifting - light, airy, floating free.",
  "params": {
    "modulationType": "Sine", "delayTime": 180.0, "feedbackL": 0.53, "feedbackR": 0.57, "modMix": 0.47, "modDepth": 2.5, "modRate": 0.2, "sync": false,
    "width": 1.0, "intensity": 0.7, "midSideBalance": 0.05, "mono": false, "tiltEQ": 0.04,
    "phaseOffsetL": 0.1, "phaseOffsetR": -0.07, "sfxModRateL": 0.3, "sfxModRateR": 0.3, "sfxModDepthL": 0.45, "sfxModDepthR": 0.45, "sfxWetDryMix": 0.6, "sfxLfoPhaseOffset": 0.22, "sfxAllpassFreq": 1800.0, "haasDelayL": 0.25, "haasDelayR": 0.25, "modulationShape": "Triangle",
    "detuneAmount": 2.2, "lfoRate": 0.4, "lfoDepth": 0.0006, "delayCentre": 0.0018, "stereoSeparation": 0.5, "mix": 0.45, "detuneFeedback": 0.25, "diffusion": 0.5,
    "exciterDrive": 4.0, "exciterMix": 0.63, "exciterHighpass": 100.0, "exciterSaturationType": "Digital", "exciterHarmonicMode": "Add Even", "exciterToneBrightness": 0.65, "exciterHarmonicBalance": 0.55, "exciterAutoGain": true,
    "predelayMs": 40.0, "size": 0.8, "damping": 0.3, "wet": 0.45
  }
})JSON",

R"JSON({
  "name": "Molten Light",
  "description": "Hot, viscous, radiant - a warm glowing overdrive at its core.",
  "params": {
    "modulationType": "Sine", "delayTime": 270.0, "feedbackL": 0.7, "feedbackR": 0.66, "modMix": 0.59, "modDepth": 3.8, "modRate": 0.25, "sync": false,
    "width": 1.3, "intensity": 0.9, "midSideBalance": 0.1, "mono": false, "tiltEQ": 0.06,
    "phaseOffsetL": 0.1, "phaseOffsetR": -0.1, "sfxModRateL": 0.5, "sfxModRateR": 0.65, "sfxModDepthL": 0.75, "sfxModDepthR": 0.75, "sfxWetDryMix": 0.4, "sfxLfoPhaseOffset": 0.12, "sfxAllpassFreq": 4200.0, "haasDelayL": 0.5, "haasDelayR": 0.3, "modulationShape": "Sine",
    "detuneAmount": 3.0, "lfoRate": 0.5, "lfoDepth": 0.0007, "delayCentre": 0.002, "stereoSeparation": 0.75, "mix": 0.45, "detuneFeedback": 0.35, "diffusion": 0.55,
    "exciterDrive": 7.5, "exciterMix": 0.61, "exciterHighpass": 120.0, "exciterSaturationType": "Tube", "exciterHarmonicMode": "Add Even", "exciterToneBrightness": 0.55, "exciterHarmonicBalance": 0.6, "exciterAutoGain": true,
    "predelayMs": 60.0, "size": 0.7, "damping": 0.2, "wet": 0.51
  }
})JSON",

R"JSON({
  "name": "Ethereal Echo",
  "description": "Airy, ghostly, spacious - as light and untouched as an echo can be.",
  "params": {
    "modulationType": "Sine", "delayTime": 350.0, "feedbackL": 0.47, "feedbackR": 0.5, "modMix": 0.58, "modDepth": 2.5, "modRate": 0.3, "sync": false,
    "width": 1.1, "intensity": 0.85, "midSideBalance": -0.05, "mono": false, "tiltEQ": 0.04,
    "phaseOffsetL": 0.1, "phaseOffsetR": -0.1, "sfxModRateL": 0.4, "sfxModRateR": 0.55, "sfxModDepthL": 0.65, "sfxModDepthR": 0.65, "sfxWetDryMix": 0.3, "sfxLfoPhaseOffset": 0.09, "sfxAllpassFreq": 4200.0, "haasDelayL": 0.4, "haasDelayR": 0.3, "modulationShape": "Triangle",
    "detuneAmount": -2.5, "lfoRate": 1.2, "lfoDepth": 0.0008, "delayCentre": 0.0025, "stereoSeparation": 0.75, "mix": 0.49, "detuneFeedback": 0.2, "diffusion": 0.7,
    "exciterDrive": 5.5, "exciterMix": 0.57, "exciterHighpass": 150.0, "exciterSaturationType": "Soft", "exciterHarmonicMode": "Add Even", "exciterToneBrightness": 0.75, "exciterHarmonicBalance": 0.5, "exciterAutoGain": true,
    "predelayMs": 50.0, "size": 0.75, "damping": 0.15, "wet": 0.48
  }
})JSON",

R"JSON({
  "name": "Lush Dreamscape",
  "description": "Rich, thick, enveloping - a lush, nostalgic dream you sink into.",
  "params": {
    "modulationType": "Triangle", "delayTime": 400.0, "feedbackL": 0.7, "feedbackR": 0.75, "modMix": 0.74, "modDepth": 3.0, "modRate": 0.35, "sync": false,
    "width": 1.2, "intensity": 0.9, "midSideBalance": 0.1, "mono": false, "tiltEQ": 0.05,
    "phaseOffsetL": 0.1, "phaseOffsetR": -0.1, "sfxModRateL": 0.5, "sfxModRateR": 0.65, "sfxModDepthL": 0.75, "sfxModDepthR": 0.75, "sfxWetDryMix": 0.2, "sfxLfoPhaseOffset": 0.06, "sfxAllpassFreq": 4200.0, "haasDelayL": 0.6, "haasDelayR": 0.3, "modulationShape": "Sine",
    "detuneAmount": -3.5, "lfoRate": 1.5, "lfoDepth": 0.0009, "delayCentre": 0.0022, "stereoSeparation": 0.8, "mix": 0.7, "detuneFeedback": 0.4, "diffusion": 0.75,
    "exciterDrive": 6.5, "exciterMix": 0.8, "exciterHighpass": 200.0, "exciterSaturationType": "Tape", "exciterHarmonicMode": "Natural", "exciterToneBrightness": 0.6, "exciterHarmonicBalance": 0.55, "exciterAutoGain": true,
    "predelayMs": 60.0, "size": 0.85, "damping": 0.3, "wet": 0.45
  }
})JSON",

R"JSON({
  "name": "Skin Contact",
  "description": "Tactile and close - warm human contact, never harsh, never diffuse.",
  "params": {
    "modulationType": "Triangle", "delayTime": 60.0, "feedbackL": 0.35, "feedbackR": 0.4, "modMix": 0.4, "modDepth": 1.0, "modRate": 0.08, "sync": false,
    "width": 0.9, "intensity": 0.95, "midSideBalance": -0.12, "mono": false, "tiltEQ": 0.02,
    "phaseOffsetL": 0.02, "phaseOffsetR": -0.018, "sfxModRateL": 0.12, "sfxModRateR": 0.2, "sfxModDepthL": 0.5, "sfxModDepthR": 0.5, "sfxWetDryMix": 0.1, "sfxLfoPhaseOffset": 0.03, "sfxAllpassFreq": 4200.0, "haasDelayL": 0.4, "haasDelayR": 0.3, "modulationShape": "Random",
    "detuneAmount": 1.1, "lfoRate": 0.15, "lfoDepth": 0.0001, "delayCentre": 0.001, "stereoSeparation": 0.4, "mix": 0.25, "detuneFeedback": 0.1, "diffusion": 0.15,
    "exciterDrive": 4.0, "exciterMix": 0.45, "exciterHighpass": 44.0, "exciterSaturationType": "Tape", "exciterHarmonicMode": "Natural", "exciterToneBrightness": 0.35, "exciterHarmonicBalance": 0.45, "exciterAutoGain": true,
    "predelayMs": 10.0, "size": 0.35, "damping": 0.1, "wet": 0.12
  }
})JSON",

R"JSON({
  "name": "Sonic Embrace",
  "description": "Enveloping, wraparound warmth - a full-bodied embrace.",
  "params": {
    "modulationType": "Sine", "delayTime": 200.0, "feedbackL": 0.39, "feedbackR": 0.42, "modMix": 0.55, "modDepth": 2.0, "modRate": 0.15, "sync": false,
    "width": 0.8, "intensity": 0.9, "midSideBalance": -0.08, "mono": false, "tiltEQ": 0.03,
    "phaseOffsetL": 0.08, "phaseOffsetR": -0.06, "sfxModRateL": 0.25, "sfxModRateR": 0.35, "sfxModDepthL": 0.45, "sfxModDepthR": 0.45, "sfxWetDryMix": 0.25, "sfxLfoPhaseOffset": 0.1, "sfxAllpassFreq": 4200.0, "haasDelayL": 0.3, "haasDelayR": 0.25, "modulationShape": "Sine",
    "detuneAmount": -1.5, "lfoRate": 0.25, "lfoDepth": 0.0003, "delayCentre": 0.001, "stereoSeparation": 0.5, "mix": 0.32, "detuneFeedback": 0.2, "diffusion": 0.5,
    "exciterDrive": 4.5, "exciterMix": 0.54, "exciterHighpass": 120.0, "exciterSaturationType": "Transformer", "exciterHarmonicMode": "Natural", "exciterToneBrightness": 0.5, "exciterHarmonicBalance": 0.5, "exciterAutoGain": true,
    "predelayMs": 30.0, "size": 0.66, "damping": 0.25, "wet": 0.41
  }
})JSON",

R"JSON({
  "name": "Strobe Heaven",
  "description": "Rhythmic, flashing, euphoric - a driving, synced strobe of a beat.",
  "params": {
    "modulationType": "Square", "delayTime": 90.0, "feedbackL": 0.7, "feedbackR": 0.7, "modMix": 0.85, "modDepth": 2.5, "modRate": 10.0, "sync": true,
    "width": 1.3, "intensity": 0.8, "midSideBalance": 0.3, "mono": false, "tiltEQ": -0.1,
    "phaseOffsetL": 0.1, "phaseOffsetR": -0.1, "sfxModRateL": 1.4, "sfxModRateR": 1.4, "sfxModDepthL": 0.9, "sfxModDepthR": 0.9, "sfxWetDryMix": 1.0, "sfxLfoPhaseOffset": 0.2, "sfxAllpassFreq": 4200.0, "haasDelayL": 0.7, "haasDelayR": 0.7, "modulationShape": "Triangle",
    "detuneAmount": -4.0, "lfoRate": 2.0, "lfoDepth": 0.0012, "delayCentre": 0.0025, "stereoSeparation": 0.6, "mix": 0.7, "detuneFeedback": 0.3, "diffusion": 0.35,
    "exciterDrive": 8.5, "exciterMix": 0.75, "exciterHighpass": 150.0, "exciterSaturationType": "Digital", "exciterHarmonicMode": "Odd Only", "exciterToneBrightness": 0.85, "exciterHarmonicBalance": 0.55, "exciterAutoGain": false,
    "predelayMs": 10.0, "size": 0.35, "damping": 0.45, "wet": 0.3
  }
})JSON",

R"JSON({
  "name": "Glass Flame",
  "description": "Cool glass meeting hot flame - smooth surfaces with a biting edge.",
  "params": {
    "modulationType": "Sine", "delayTime": 300.0, "feedbackL": 0.44, "feedbackR": 0.44, "modMix": 0.6, "modDepth": 2.0, "modRate": 0.18, "sync": false,
    "width": 1.0, "intensity": 0.8, "midSideBalance": -0.05, "mono": false, "tiltEQ": 0.0,
    "phaseOffsetL": 0.05, "phaseOffsetR": -0.05, "sfxModRateL": 0.35, "sfxModRateR": 0.35, "sfxModDepthL": 0.5, "sfxModDepthR": 0.5, "sfxWetDryMix": 0.65, "sfxLfoPhaseOffset": 0.15, "sfxAllpassFreq": 3300.0, "haasDelayL": 0.4, "haasDelayR": 0.4, "modulationShape": "Sine",
    "detuneAmount": 2.0, "lfoRate": 0.3, "lfoDepth": 0.0004, "delayCentre": 0.0016, "stereoSeparation": 0.6, "mix": 0.4, "detuneFeedback": 0.3, "diffusion": 0.5,
    "exciterDrive": 5.5, "exciterMix": 0.8, "exciterHighpass": 110.0, "exciterSaturationType": "Hard", "exciterHarmonicMode": "Odd Only", "exciterToneBrightness": 0.7, "exciterHarmonicBalance": 0.5, "exciterAutoGain": true,
    "predelayMs": 30.0, "size": 0.75, "damping": 0.2, "wet": 0.45
  }
})JSON",

R"JSON({
  "name": "Celestial Vault",
  "description": "Vast, cosmic, reverent - a huge, weighty space that dwarfs everything in it.",
  "params": {
    "modulationType": "Sine", "delayTime": 320.0, "feedbackL": 0.53, "feedbackR": 0.53, "modMix": 0.34, "modDepth": 2.0, "modRate": 0.15, "sync": false,
    "width": 1.2, "intensity": 0.7, "midSideBalance": 0.0, "mono": false, "tiltEQ": -0.05,
    "phaseOffsetL": 0.02, "phaseOffsetR": 0.02, "sfxModRateL": 0.15, "sfxModRateR": 0.15, "sfxModDepthL": 0.5, "sfxModDepthR": 0.5, "sfxWetDryMix": 0.65, "sfxLfoPhaseOffset": 0.05, "sfxAllpassFreq": 2800.0, "haasDelayL": 0.5, "haasDelayR": 0.5, "modulationShape": "Sine",
    "detuneAmount": 2.0, "lfoRate": 0.2, "lfoDepth": 0.001, "delayCentre": 0.005, "stereoSeparation": 0.75, "mix": 0.36, "detuneFeedback": 0.15, "diffusion": 0.6,
    "exciterDrive": 4.5, "exciterMix": 0.32, "exciterHighpass": 120.0, "exciterSaturationType": "Transformer", "exciterHarmonicMode": "Add Even", "exciterToneBrightness": 0.6, "exciterHarmonicBalance": 0.5, "exciterAutoGain": true,
    "predelayMs": 100.0, "size": 0.96, "damping": 0.45, "wet": 0.37
  }
})JSON",

R"JSON({
  "name": "Deep Illusion",
  "description": "Murky and uncertain - a hazy, tape-warped trick of perception.",
  "params": {
    "modulationType": "Triangle", "delayTime": 280.0, "feedbackL": 0.35, "feedbackR": 0.35, "modMix": 0.26, "modDepth": 1.0, "modRate": 0.2, "sync": false,
    "width": 1.6, "intensity": 0.6, "midSideBalance": 0.1, "mono": false, "tiltEQ": 0.02,
    "phaseOffsetL": -0.02, "phaseOffsetR": 0.03, "sfxModRateL": 0.2, "sfxModRateR": 0.2, "sfxModDepthL": 0.35, "sfxModDepthR": 0.35, "sfxWetDryMix": 0.6, "sfxLfoPhaseOffset": 0.1, "sfxAllpassFreq": 3600.0, "haasDelayL": 0.3, "haasDelayR": 0.3, "modulationShape": "Triangle",
    "detuneAmount": 2.5, "lfoRate": 0.15, "lfoDepth": 0.001, "delayCentre": 0.004, "stereoSeparation": 0.5, "mix": 0.32, "detuneFeedback": 0.35, "diffusion": 0.45,
    "exciterDrive": 3.5, "exciterMix": 0.23, "exciterHighpass": 90.0, "exciterSaturationType": "Tape", "exciterHarmonicMode": "Natural", "exciterToneBrightness": 0.35, "exciterHarmonicBalance": 0.5, "exciterAutoGain": true,
    "predelayMs": 60.0, "size": 0.8, "damping": 0.6, "wet": 0.41
  }
})JSON",

R"JSON({
  "name": "Ego Dissolve",
  "description": "Dissolving self - boundaries blur into a maximally diffuse, expansive haze.",
  "params": {
    "modulationType": "Sine", "delayTime": 500.0, "feedbackL": 0.3, "feedbackR": 0.3, "modMix": 0.46, "modDepth": 3.0, "modRate": 0.4, "sync": false,
    "width": 1.5, "intensity": 0.5, "midSideBalance": 0.1, "mono": false, "tiltEQ": 0.0,
    "phaseOffsetL": 0.1, "phaseOffsetR": -0.1, "sfxModRateL": 0.35, "sfxModRateR": 0.35, "sfxModDepthL": 0.5, "sfxModDepthR": 0.5, "sfxWetDryMix": 0.55, "sfxLfoPhaseOffset": 0.2, "sfxAllpassFreq": 2400.0, "haasDelayL": 0.4, "haasDelayR": 0.4, "modulationShape": "Sine",
    "detuneAmount": 1.5, "lfoRate": 0.3, "lfoDepth": 0.0015, "delayCentre": 0.0065, "stereoSeparation": 0.75, "mix": 0.45, "detuneFeedback": 0.55, "diffusion": 0.8,
    "exciterDrive": 5.5, "exciterMix": 0.3, "exciterHighpass": 100.0, "exciterSaturationType": "Digital", "exciterHarmonicMode": "Add Even", "exciterToneBrightness": 0.55, "exciterHarmonicBalance": 0.6, "exciterAutoGain": true,
    "predelayMs": 100.0, "size": 0.88, "damping": 0.7, "wet": 0.54
  }
})JSON",

R"JSON({
  "name": "Memory Dust",
  "description": "Faded and decayed - the gritty, tape-worn residue of an old memory.",
  "params": {
    "modulationType": "Triangle", "delayTime": 360.0, "feedbackL": 0.5, "feedbackR": 0.5, "modMix": 0.28, "modDepth": 1.5, "modRate": 0.25, "sync": false,
    "width": 1.3, "intensity": 0.65, "midSideBalance": 0.05, "mono": false, "tiltEQ": 0.03,
    "phaseOffsetL": 0.04, "phaseOffsetR": 0.05, "sfxModRateL": 0.2, "sfxModRateR": 0.2, "sfxModDepthL": 0.3, "sfxModDepthR": 0.3, "sfxWetDryMix": 0.5, "sfxLfoPhaseOffset": 0.15, "sfxAllpassFreq": 1600.0, "haasDelayL": 0.08, "haasDelayR": 0.08, "modulationShape": "Random",
    "detuneAmount": 2.8, "lfoRate": 0.25, "lfoDepth": 0.001, "delayCentre": 0.005, "stereoSeparation": 0.7, "mix": 0.4, "detuneFeedback": 0.25, "diffusion": 0.4,
    "exciterDrive": 4.0, "exciterMix": 0.2, "exciterHighpass": 80.0, "exciterSaturationType": "Tape", "exciterHarmonicMode": "Odd Only", "exciterToneBrightness": 0.3, "exciterHarmonicBalance": 0.45, "exciterAutoGain": true,
    "predelayMs": 70.0, "size": 0.85, "damping": 0.55, "wet": 0.41
  }
})JSON",

R"JSON({
  "name": "Gentle Slap",
  "description": "Light, quick, playful - a simple, tempo-locked slapback echo.",
  "params": {
    "modulationType": "Sine", "delayTime": 100.0, "feedbackL": 0.4, "feedbackR": 0.4, "modMix": 0.1, "modDepth": 0.6, "modRate": 4.0, "sync": true,
    "width": 0.9, "intensity": 0.5, "midSideBalance": 0.0, "mono": false, "tiltEQ": 0.02,
    "phaseOffsetL": 0.02, "phaseOffsetR": -0.02, "sfxModRateL": 0.1, "sfxModRateR": 0.1, "sfxModDepthL": 0.15, "sfxModDepthR": 0.15, "sfxWetDryMix": 0.25, "sfxLfoPhaseOffset": 0.2, "sfxAllpassFreq": 4200.0, "haasDelayL": 0.05, "haasDelayR": 0.05, "modulationShape": "Sine",
    "detuneAmount": 1.2, "lfoRate": 0.1, "lfoDepth": 0.0003, "delayCentre": 0.0012, "stereoSeparation": 0.4, "mix": 0.3, "detuneFeedback": 0.05, "diffusion": 0.1,
    "exciterDrive": 2.5, "exciterMix": 0.15, "exciterHighpass": 50.0, "exciterSaturationType": "Soft", "exciterHarmonicMode": "Natural", "exciterToneBrightness": 0.5, "exciterHarmonicBalance": 0.5, "exciterAutoGain": true,
    "predelayMs": 20.0, "size": 0.35, "damping": 0.2, "wet": 0.3
  }
})JSON",

R"JSON({
  "name": "Moon Dance",
  "description": "Gentle rhythmic sway under a cool night sky - a synced, shimmering waltz.",
  "params": {
    "modulationType": "Sine", "delayTime": 200.0, "feedbackL": 0.39, "feedbackR": 0.39, "modMix": 0.51, "modDepth": 2.0, "modRate": 4.0, "sync": true,
    "width": 1.2, "intensity": 0.7, "midSideBalance": -0.05, "mono": false, "tiltEQ": 0.02,
    "phaseOffsetL": 0.05, "phaseOffsetR": -0.05, "sfxModRateL": 0.25, "sfxModRateR": 0.25, "sfxModDepthL": 0.35, "sfxModDepthR": 0.35, "sfxWetDryMix": 0.45, "sfxLfoPhaseOffset": 0.4, "sfxAllpassFreq": 4200.0, "haasDelayL": 0.2, "haasDelayR": 0.2, "modulationShape": "Triangle",
    "detuneAmount": 1.5, "lfoRate": 0.25, "lfoDepth": 0.0003, "delayCentre": 0.001, "stereoSeparation": 0.5, "mix": 0.29, "detuneFeedback": 0.2, "diffusion": 0.45,
    "exciterDrive": 3.0, "exciterMix": 0.41, "exciterHighpass": 40.0, "exciterSaturationType": "Digital", "exciterHarmonicMode": "Add Even", "exciterToneBrightness": 0.6, "exciterHarmonicBalance": 0.5, "exciterAutoGain": true,
    "predelayMs": 30.0, "size": 0.66, "damping": 0.25, "wet": 0.37
  }
})JSON",

R"JSON({
  "name": "Biting Lips",
  "description": "Sharp sensual tension - a sting of edge and bite, distinct from a gentle slap.",
  "params": {
    "modulationType": "Sine", "delayTime": 80.0, "feedbackL": 0.45, "feedbackR": 0.45, "modMix": 0.25, "modDepth": 0.5, "modRate": 0.08, "sync": false,
    "width": 0.3, "intensity": 0.9, "midSideBalance": -0.05, "mono": false, "tiltEQ": 0.1,
    "phaseOffsetL": 0.02, "phaseOffsetR": -0.02, "sfxModRateL": 0.1, "sfxModRateR": 0.1, "sfxModDepthL": 0.15, "sfxModDepthR": 0.15, "sfxWetDryMix": 0.25, "sfxLfoPhaseOffset": 0.3, "sfxAllpassFreq": 4200.0, "haasDelayL": 0.1, "haasDelayR": 0.1, "modulationShape": "Random",
    "detuneAmount": 1.2, "lfoRate": 0.1, "lfoDepth": 0.0003, "delayCentre": 0.0012, "stereoSeparation": 0.4, "mix": 0.3, "detuneFeedback": 0.15, "diffusion": 0.2,
    "exciterDrive": 3.5, "exciterMix": 0.15, "exciterHighpass": 90.0, "exciterSaturationType": "Hard", "exciterHarmonicMode": "Odd Only", "exciterToneBrightness": 0.6, "exciterHarmonicBalance": 0.55, "exciterAutoGain": true,
    "predelayMs": 20.0, "size": 0.35, "damping": 0.2, "wet": 0.3
  }
})JSON",

R"JSON({
  "name": "Stormy Day",
  "description": "Turbulent, dark weather - rough and erratic, distinct from Glass Flame's clean bite.",
  "params": {
    "modulationType": "Sine", "delayTime": 340.0, "feedbackL": 0.45, "feedbackR": 0.45, "modMix": 0.65, "modDepth": 3.5, "modRate": 0.35, "sync": false,
    "width": 1.0, "intensity": 0.8, "midSideBalance": -0.05, "mono": false, "tiltEQ": -0.25,
    "phaseOffsetL": 0.05, "phaseOffsetR": -0.05, "sfxModRateL": 0.35, "sfxModRateR": 0.35, "sfxModDepthL": 0.5, "sfxModDepthR": 0.5, "sfxWetDryMix": 0.65, "sfxLfoPhaseOffset": 0.45, "sfxAllpassFreq": 4200.0, "haasDelayL": 0.3, "haasDelayR": 0.3, "modulationShape": "Random",
    "detuneAmount": 2.0, "lfoRate": 0.3, "lfoDepth": 0.0004, "delayCentre": 0.0016, "stereoSeparation": 0.6, "mix": 0.4, "detuneFeedback": 0.35, "diffusion": 0.5,
    "exciterDrive": 6.5, "exciterMix": 0.8, "exciterHighpass": 90.0, "exciterSaturationType": "Hard", "exciterHarmonicMode": "Odd Only", "exciterToneBrightness": 0.35, "exciterHarmonicBalance": 0.5, "exciterAutoGain": false,
    "predelayMs": 30.0, "size": 0.7, "damping": 0.55, "wet": 0.5
  }
})JSON",

R"JSON({
  "name": "Summer Sunset",
  "description": "Warm, golden, relaxed - the glow of a fading summer evening.",
  "params": {
    "modulationType": "Sine", "delayTime": 400.0, "feedbackL": 0.6, "feedbackR": 0.6, "modMix": 0.6, "modDepth": 1.5, "modRate": 0.15, "sync": false,
    "width": 1.1, "intensity": 0.8, "midSideBalance": 0.1, "mono": false, "tiltEQ": -0.04,
    "phaseOffsetL": 0.1, "phaseOffsetR": -0.1, "sfxModRateL": 0.25, "sfxModRateR": 0.25, "sfxModDepthL": 0.4, "sfxModDepthR": 0.4, "sfxWetDryMix": 0.75, "sfxLfoPhaseOffset": 0.15, "sfxAllpassFreq": 4200.0, "haasDelayL": 0.35, "haasDelayR": 0.15, "modulationShape": "Sine",
    "detuneAmount": -0.8, "lfoRate": 0.8, "lfoDepth": 0.0004, "delayCentre": 0.0015, "stereoSeparation": 0.6, "mix": 0.5, "detuneFeedback": 0.15, "diffusion": 0.4,
    "exciterDrive": 4.0, "exciterMix": 0.5, "exciterHighpass": 80.0, "exciterSaturationType": "Tube", "exciterHarmonicMode": "Add Even", "exciterToneBrightness": 0.55, "exciterHarmonicBalance": 0.5, "exciterAutoGain": true,
    "predelayMs": 60.0, "size": 0.8, "damping": 0.3, "wet": 0.5
  }
})JSON",

R"JSON({
  "name": "Ocean Waves",
  "description": "Flowing, rolling, natural - the analog warmth of vast rolling water.",
  "params": {
    "modulationType": "Triangle", "delayTime": 250.0, "feedbackL": 0.3, "feedbackR": 0.3, "modMix": 0.43, "modDepth": 2.5, "modRate": 0.2, "sync": false,
    "width": 1.3, "intensity": 0.7, "midSideBalance": 0.05, "mono": false, "tiltEQ": 0.03,
    "phaseOffsetL": 0.08, "phaseOffsetR": -0.06, "sfxModRateL": 0.3, "sfxModRateR": 0.3, "sfxModDepthL": 0.45, "sfxModDepthR": 0.45, "sfxWetDryMix": 0.55, "sfxLfoPhaseOffset": 0.12, "sfxAllpassFreq": 4200.0, "haasDelayL": 0.3, "haasDelayR": 0.12, "modulationShape": "Triangle",
    "detuneAmount": -1.5, "lfoRate": 0.4, "lfoDepth": 0.0006, "delayCentre": 0.0022, "stereoSeparation": 0.65, "mix": 0.37, "detuneFeedback": 0.2, "diffusion": 0.55,
    "exciterDrive": 4.5, "exciterMix": 0.44, "exciterHighpass": 120.0, "exciterSaturationType": "Tape", "exciterHarmonicMode": "Natural", "exciterToneBrightness": 0.5, "exciterHarmonicBalance": 0.5, "exciterAutoGain": true,
    "predelayMs": 50.0, "size": 0.9, "damping": 0.25, "wet": 0.33
  }
})JSON",

R"JSON({
  "name": "Crystal Clear",
  "description": "Pristine and transparent - minimal coloration, maximum clarity.",
  "params": {
    "modulationType": "Sine", "delayTime": 110.0, "feedbackL": 0.2, "feedbackR": 0.2, "modMix": 0.25, "modDepth": 0.6, "modRate": 0.05, "sync": false,
    "width": 1.0, "intensity": 0.6, "midSideBalance": 0.0, "mono": false, "tiltEQ": 0.01,
    "phaseOffsetL": 0.01, "phaseOffsetR": -0.01, "sfxModRateL": 0.05, "sfxModRateR": 0.05, "sfxModDepthL": 0.1, "sfxModDepthR": 0.1, "sfxWetDryMix": 0.2, "sfxLfoPhaseOffset": 0.05, "sfxAllpassFreq": 4200.0, "haasDelayL": 0.15, "haasDelayR": 0.05, "modulationShape": "Sine",
    "detuneAmount": 0.7, "lfoRate": 0.1, "lfoDepth": 0.0002, "delayCentre": 0.001, "stereoSeparation": 0.5, "mix": 0.2, "detuneFeedback": 0.0, "diffusion": 0.05,
    "exciterDrive": 1.2, "exciterMix": 0.25, "exciterHighpass": 400.0, "exciterSaturationType": "Soft", "exciterHarmonicMode": "Natural", "exciterToneBrightness": 0.8, "exciterHarmonicBalance": 0.5, "exciterAutoGain": true,
    "predelayMs": 10.0, "size": 0.4, "damping": 0.3, "wet": 0.3
  }
})JSON",

R"JSON({
  "name": "Sweetest Memory",
  "description": "Sweet and nostalgic - a warm, fond, tape-like glow of remembrance.",
  "params": {
    "modulationType": "Sine", "delayTime": 220.0, "feedbackL": 0.35, "feedbackR": 0.31, "modMix": 0.43, "modDepth": 1.2, "modRate": 0.15, "sync": false,
    "width": 1.2, "intensity": 0.65, "midSideBalance": -0.05, "mono": false, "tiltEQ": 0.02,
    "phaseOffsetL": 0.07, "phaseOffsetR": -0.04, "sfxModRateL": 0.28, "sfxModRateR": 0.28, "sfxModDepthL": 0.38, "sfxModDepthR": 0.38, "sfxWetDryMix": 0.55, "sfxLfoPhaseOffset": 0.05, "sfxAllpassFreq": 4200.0, "haasDelayL": 0.4, "haasDelayR": 0.05, "modulationShape": "Sine",
    "detuneAmount": -1.2, "lfoRate": 0.25, "lfoDepth": 0.0003, "delayCentre": 0.001, "stereoSeparation": 0.55, "mix": 0.29, "detuneFeedback": 0.15, "diffusion": 0.45,
    "exciterDrive": 3.0, "exciterMix": 0.41, "exciterHighpass": 90.0, "exciterSaturationType": "Tape", "exciterHarmonicMode": "Add Even", "exciterToneBrightness": 0.5, "exciterHarmonicBalance": 0.5, "exciterAutoGain": true,
    "predelayMs": 60.0, "size": 0.7, "damping": 0.3, "wet": 0.48
  }
})JSON"
    };

    std::vector<FactoryPreset> parseFactoryPresets()
    {
        std::vector<FactoryPreset> presets;

        for (const char* text : kFactoryPresetJson)
        {
            juce::var parsed;
            if (!juce::JSON::parse(juce::String(juce::CharPointer_UTF8(text)), parsed).wasOk() || !parsed.isObject())
            {
                jassertfalse;   // a typo in the data above
                continue;
            }

            FactoryPreset preset;
            preset.name = parsed["name"].toString();
            preset.description = parsed["description"].toString();

            if (auto* params = parsed["params"].getDynamicObject())
                for (const auto& property : params->getProperties())
                    preset.params[property.name.toString()] = property.value;

            presets.push_back(std::move(preset));
        }

        return presets;
    }
}

float FactoryPreset::number(const juce::String& id, float fallback) const
{
    const auto it = params.find(id);
    if (it == params.end() || !(it->second.isDouble() || it->second.isInt() || it->second.isInt64() || it->second.isBool()))
        return fallback;

    return static_cast<float>(static_cast<double>(it->second));
}

const std::vector<FactoryPreset>& getFactoryPresets()
{
    static const std::vector<FactoryPreset> presets = parseFactoryPresets();
    return presets;
}
