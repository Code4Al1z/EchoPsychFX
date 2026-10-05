#pragma once

#include <juce_core/juce_core.h>
#include <map>
#include <vector>

/** One built-in preset: a name, a description of the feeling it aims for, and a value for each
    plugin parameter, keyed by the same IDs the AudioProcessorValueTreeState uses.

    Numbers are plain parameter values (milliseconds, Hz, 0..1 ...). The choice parameters use
    their option text ("Triangle", "Odd Only") and the on/off ones use true/false. A parameter that
    is not listed keeps whatever value it has. */
struct FactoryPreset
{
    juce::String name;
    juce::String description;
    std::map<juce::String, juce::var> params;

    /** The numeric value of a parameter, or `fallback` if the preset doesn't set it (or it isn't a number). */
    float number(const juce::String& id, float fallback = 0.0f) const;
};

/** All built-in presets, in factory order. Parsed once, on first use. */
const std::vector<FactoryPreset>& getFactoryPresets();
