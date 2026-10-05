#pragma once

#include <juce_gui_extra/juce_gui_extra.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include <map>
#include <functional>
#include "FactoryPresets.h"
#include "PerceptionProfile.h"

/** How the preset list is ordered. */
enum class PresetSort
{
    FactoryOrder,    // the curated order the presets ship in
    Alphabetical,
    Brightness,      // from here on: sorted by a perception axis, strongest first (see PerceptionProfile.h)
    Width,
    Space,
    Motion,
    Saturation,
    Intensity
};

/**
 * @brief Manages psychoacoustic perception presets
 *
 * Coordinates multiple effect components to create specific
 * psychoacoustic experiences through preset combinations
 */
class PerceptionPresetManager
{
public:
    explicit PerceptionPresetManager(juce::AudioProcessorValueTreeState& apvts);

    ~PerceptionPresetManager() = default;

    /** Apply a preset by name (checks user presets first, then factory presets) */
    void applyPreset(const juce::String& presetName);

    /** Names of the built-in presets, in factory order */
    juce::StringArray getFactoryPresetNames() const;

    /** The preset names in the current sort order: the factory presets first, then the user presets.
        The factory preset called "Init" always stays first, whatever the order. */
    struct SortedNames { juce::StringArray factory; juce::StringArray user; };
    SortedNames getSortedPresetNames() const;

    PresetSort getSortMode() const noexcept { return sortMode; }
    bool getSortReverse() const noexcept { return sortReverse; }

    /** Changes the order of the preset list and remembers it between sessions. */
    void setSort(PresetSort mode, bool reverse);

    /** e.g. "Brightness - brightest first", for labels. */
    static juce::String describeSort(PresetSort mode, bool reverse);

    /** The perception profile of a named factory or user preset (neutral if there is no such preset). */
    PerceptionProfile getProfileOf(const juce::String& presetName) const;

    /** The perception profile of the live parameter values, whatever preset they came from. */
    PerceptionProfile getLiveProfile() const;

    /** True if presetName names one of the built-in, read-only factory presets */
    bool isFactoryPreset(const juce::String& presetName) const;

    /** True if presetName names a user-saved preset */
    bool isUserPreset(const juce::String& presetName) const;

    /** Names of all user-saved presets, alphabetically */
    juce::StringArray getUserPresetNames() const;

    /** False once the live APVTS state has drifted from whichever preset was last applied
        via applyPreset() - the cue the UI uses to switch its label to "Custom". True if no
        preset has been applied yet this session (nothing to have drifted from). */
    bool matchesLastAppliedPreset() const;

    /** Short (1-3 word), glance-readable mood/character tags describing the *current* live
        parameter values - e.g. "Spacious", "Pulled Right", "Unstable Shimmer". Always
        reflects the live APVTS state, so it stays accurate for factory presets, user
        presets, and "Custom" alike. Returns {"Neutral"} if nothing stands out. */
    juce::StringArray generateFeelingTags() const;

    /** Builds a fuller plain-language description of what the *current* live parameter
        values (not any named preset) would sound and feel like - width and pull,
        brightness, delay movement, pitch drift, exciter character, and reverb space -
        each paired with the psychoacoustic feeling it tends to evoke. Always reflects the
        live APVTS state, so it stays accurate for factory presets, user presets, and
        "Custom" alike. */
    juce::String generateBreakdown() const;

    /** Saves the current plugin state as a user preset. Fails (returns false) if
        presetName is empty or collides with a read-only factory preset name. */
    bool saveCurrentAsUserPreset(const juce::String& presetName);

    /** Renames a user preset. Fails if oldName isn't a user preset, or newName is
        empty or collides with a factory preset. */
    bool renameUserPreset(const juce::String& oldName, const juce::String& newName);

    /** Deletes a user preset. Fails (returns false) if presetName isn't a user preset. */
    bool deleteUserPreset(const juce::String& presetName);

private:
    // Live plugin state - user presets are captured from and restored to this directly
    juce::AudioProcessorValueTreeState& apvtsRef;

    // List order, saved alongside the user presets
    PresetSort sortMode = PresetSort::FactoryOrder;
    bool sortReverse = false;

    // User preset storage, persisted to disk
    std::map<juce::String, juce::ValueTree> userPresets;

    // Snapshot of the APVTS state taken just after the most recently applied preset finished
    // landing in it, for matchesLastAppliedPreset() to diff the live state against. Invalid
    // until the first applyPreset() call.
    juce::ValueTree lastAppliedPresetState;

    /** Plain parameter value for the profile maths: the preset's own, or the parameter's default. */
    float defaultValueOf(const char* parameterId) const;
    PerceptionProfile profileOfFactoryPreset(const FactoryPreset& preset) const;
    PerceptionProfile profileOfUserPreset(const juce::ValueTree& state) const;

    /** Sets every parameter a factory preset lists. */
    void applyFactoryPreset(const FactoryPreset& preset);

    juce::File getUserPresetsFile() const;
    void loadUserPresets();
    void saveUserPresetsToDisk() const;

    /** Shared logic behind generateFeelingTags()/generateBreakdown(): walks the live APVTS
        values once and, for each trait that stands out, appends a matching (short tag,
        full sentence) pair at the same index in both arrays. */
    void computeDescriptors(juce::StringArray& tags, juce::StringArray& clauses) const;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PerceptionPresetManager)
};