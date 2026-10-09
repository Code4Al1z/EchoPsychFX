#include "PerceptionPresetManager.h"
#include "SoundDescriptors.h"
#include <algorithm>

PerceptionPresetManager::PerceptionPresetManager(juce::AudioProcessorValueTreeState& apvts)
    : apvtsRef(apvts)
{
    loadUserPresets();
}

namespace
{
    constexpr const char* kOutputTrimId = "outputGain";

    // Saved in the user presets file, so keep these stable
    constexpr const char* kSortIds[] = { "factory", "alphabetical", "brightness", "width", "space", "motion", "saturation", "intensity" };

    // Output trim is a monitoring-level setting, not part of a sound, so presets neither store nor
    // change it and moving it must not turn the preset into "Custom".
    juce::ValueTree withoutOutputTrim(juce::ValueTree state)
    {
        for (int i = state.getNumChildren(); --i >= 0;)
            if (state.getChild(i).getProperty("id").toString() == kOutputTrimId)
                state.removeChild(i, nullptr);

        return state;
    }
}

void PerceptionPresetManager::applyPreset(const juce::String& presetName)
{
    auto userIt = userPresets.find(presetName);
    if (userIt != userPresets.end())
    {
        auto* trimParam = apvtsRef.getParameter(kOutputTrimId);
        const float trimBefore = trimParam != nullptr ? trimParam->getValue() : 0.0f;

        apvtsRef.replaceState(userIt->second.createCopy());

        if (trimParam != nullptr)
            trimParam->setValueNotifyingHost(trimBefore);   // keep the user's output level
        DBG("Applied user preset: " + presetName);

        // The APVTS only flushes parameter changes into its state ValueTree periodically,
        // so snapshot the baseline once that's had a chance to happen rather than reading
        // it back immediately.
        snapshotBaselineSoon();
        return;
    }

    for (const auto& preset : getFactoryPresets())
    {
        if (preset.name == presetName)
        {
            applyFactoryPreset(preset);
            DBG("Applied preset: " + presetName);
            snapshotBaselineSoon();
            return;
        }
    }

    DBG("Preset not found: " + presetName);
}

void PerceptionPresetManager::applyFactoryPreset(const FactoryPreset& preset)
{
    for (const auto& [id, value] : preset.params)
    {
        auto* parameter = apvtsRef.getParameter(id);
        if (parameter == nullptr)
        {
            jassertfalse;   // the preset names a parameter that doesn't exist
            continue;
        }

        float plainValue = 0.0f;

        if (value.isString())
        {
            // Choice parameters are written by their option text, e.g. "Triangle" or "Odd Only"
            const auto* choice = dynamic_cast<juce::AudioParameterChoice*>(parameter);
            const int index = choice != nullptr ? choice->choices.indexOf(value.toString()) : -1;
            if (index < 0)
            {
                jassertfalse;
                continue;
            }
            plainValue = static_cast<float>(index);
        }
        else
        {
            plainValue = static_cast<float>(static_cast<double>(value));   // numbers, and true/false as 1/0
        }

        parameter->beginChangeGesture();
        parameter->setValueNotifyingHost(parameter->convertTo0to1(plainValue));
        parameter->endChangeGesture();
    }
}

juce::StringArray PerceptionPresetManager::getFactoryPresetNames() const
{
    juce::StringArray names;
    for (const auto& preset : getFactoryPresets())
        names.add(preset.name);
    return names;
}

bool PerceptionPresetManager::isFactoryPreset(const juce::String& presetName) const
{
    for (const auto& preset : getFactoryPresets())
        if (preset.name == presetName)
            return true;

    return false;
}

bool PerceptionPresetManager::isUserPreset(const juce::String& presetName) const
{
    return userPresets.find(presetName) != userPresets.end();
}

void PerceptionPresetManager::snapshotBaselineSoon()
{
    // Until the snapshot is taken the old baseline would be compared against the freshly applied
    // values, which flashed "Custom" for a moment after every preset change.
    ++baselinePending;
    juce::Timer::callAfterDelay(120, [this]()
    {
        lastAppliedPresetState = apvtsRef.copyState();
        --baselinePending;
    });
}

bool PerceptionPresetManager::matchesLastAppliedPreset() const
{
    if (baselinePending > 0)
        return true;

    if (!lastAppliedPresetState.isValid())
        return true;

    return withoutOutputTrim(apvtsRef.copyState()).isEquivalentTo(withoutOutputTrim(lastAppliedPresetState.createCopy()));
}

namespace
{
    // Below this overall Intensity the sound really is close to untouched; above it, "neutral" would be untrue
    constexpr float kUntouchedIntensity = 0.12f;
}

void PerceptionPresetManager::computeDescriptors(juce::StringArray& tags, juce::StringArray& clauses,
                                                 std::vector<Section>* sections) const
{
    const auto character = computeSoundCharacter([this](const char* id) { return apvtsRef.getRawParameterValue(id)->load(); });

    // When nothing is audible there is nothing to describe: the caller shows "Neutral" instead
    const auto profile = computePerceptionProfile(character);
    if (profile.get(PerceptionAxis::Intensity) < kUntouchedIntensity)
        return;

    for (const auto& descriptor : describeSound(character, profile))
    {
        tags.add(descriptor.tag);
        clauses.add(descriptor.clause);
        if (sections != nullptr)
            sections->push_back(descriptor.section);
    }
}

std::vector<PerceptionPresetManager::FeelingChip> PerceptionPresetManager::generateFeelingChips() const
{
    juce::StringArray tags, clauses;
    std::vector<Section> sections;
    computeDescriptors(tags, clauses, &sections);

    std::vector<FeelingChip> chips;
    for (int i = 0; i < tags.size(); ++i)
        chips.push_back({ tags[i], true, sections[static_cast<size_t>(i)] });

    if (chips.empty())
        chips.push_back({ getLiveProfile().get(PerceptionAxis::Intensity) < kUntouchedIntensity ? "Neutral" : "Subtle" });

    return chips;
}

juce::StringArray PerceptionPresetManager::generateFeelingTags() const
{
    juce::StringArray tags;
    for (const auto& chip : generateFeelingChips())
        tags.add(chip.tag);
    return tags;
}

namespace
{
    // Which sentence a section's clauses belong to: stereo picture, space and time, texture
    int sentenceOf(Section section)
    {
        switch (section)
        {
            case Section::Input:
            case Section::Spatial:    return 0;
            case Section::Motion:
            case Section::Reverb:     return 1;
            case Section::MicroPitch:
            case Section::Exciter:    return 2;
        }
        return 0;
    }

    // "a, b and c" - or "a; b; c" when a clause already contains a comma, so the list never reads as a run-on
    juce::String joinClauses(const juce::StringArray& parts)
    {
        bool hasComma = false;
        for (const auto& part : parts)
            hasComma = hasComma || part.contains(",");

        juce::String joined;
        for (int i = 0; i < parts.size(); ++i)
        {
            if (i > 0)
                joined += hasComma ? "; " : (i == parts.size() - 1 ? " and " : ", ");
            joined += parts[i];
        }
        return joined.substring(0, 1).toUpperCase() + joined.substring(1) + ".";
    }
}

juce::String PerceptionPresetManager::generateBreakdown() const
{
    juce::StringArray tags, clauses;
    std::vector<Section> sections;
    computeDescriptors(tags, clauses, &sections);

    const auto profile = getLiveProfile();

    if (clauses.isEmpty())
    {
        return profile.get(PerceptionAxis::Intensity) < kUntouchedIntensity
            ? "Nothing strongly colored yet - close to a neutral, untouched signal."
            : "A subtle, understated treatment - nothing pushed far in any one direction.";
    }

    // One short sentence per theme instead of one long comma chain
    juce::StringArray groups[3];
    for (int i = 0; i < clauses.size(); ++i)
        groups[sentenceOf(sections[static_cast<size_t>(i)])].add(clauses[i]);

    juce::String result;
    for (const auto& group : groups)
        if (!group.isEmpty())
            result += (result.isEmpty() ? "" : " ") + joinClauses(group);

    const auto character = computeSoundCharacter([this](const char* id) { return apvtsRef.getRawParameterValue(id)->load(); });
    const auto feel = describeOverallFeel(character, profile);
    if (!feel.empty())
        result += " Overall it feels " + juce::String(feel) + ".";

    return result;
}

juce::StringArray PerceptionPresetManager::getUserPresetNames() const
{
    juce::StringArray names;
    for (auto& entry : userPresets)
        names.add(entry.first);
    names.sort(true);
    return names;
}

bool PerceptionPresetManager::saveCurrentAsUserPreset(const juce::String& presetName)
{
    if (presetName.isEmpty() || isFactoryPreset(presetName))
        return false;

    userPresets[presetName] = withoutOutputTrim(apvtsRef.copyState());
    saveUserPresetsToDisk();
    return true;
}

bool PerceptionPresetManager::renameUserPreset(const juce::String& oldName, const juce::String& newName)
{
    if (newName.isEmpty() || isFactoryPreset(newName))
        return false;

    auto it = userPresets.find(oldName);
    if (it == userPresets.end())
        return false;

    auto state = it->second;
    userPresets.erase(it);
    userPresets[newName] = state;
    saveUserPresetsToDisk();
    return true;
}

bool PerceptionPresetManager::deleteUserPreset(const juce::String& presetName)
{
    auto it = userPresets.find(presetName);
    if (it == userPresets.end())
        return false;

    userPresets.erase(it);
    saveUserPresetsToDisk();
    return true;
}

juce::File PerceptionPresetManager::getUserPresetsFile() const
{
    auto dir = juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
        .getChildFile("EchoPsychFX");
    dir.createDirectory();
    return dir.getChildFile("UserPresets.xml");
}

void PerceptionPresetManager::loadUserPresets()
{
    auto file = getUserPresetsFile();
    if (!file.existsAsFile())
        return;

    auto root = juce::XmlDocument::parse(file);
    if (root == nullptr || !root->hasTagName("UserPresets"))
        return;

    const auto savedSort = root->getStringAttribute("sortMode");
    for (int i = 0; i < static_cast<int>(std::size(kSortIds)); ++i)
        if (savedSort == kSortIds[i])
            sortMode = static_cast<PresetSort>(i);
    sortReverse = root->getBoolAttribute("sortReverse", false);

    for (auto* presetXml = root->getFirstChildElement(); presetXml != nullptr;
        presetXml = presetXml->getNextElement())
    {
        auto name = presetXml->getStringAttribute("name");
        auto* stateXml = presetXml->getFirstChildElement();
        if (name.isEmpty() || stateXml == nullptr)
            continue;

        userPresets[name] = juce::ValueTree::fromXml(*stateXml);
    }
}

void PerceptionPresetManager::saveUserPresetsToDisk() const
{
    juce::XmlElement root("UserPresets");
    root.setAttribute("sortMode", kSortIds[static_cast<int>(sortMode)]);
    root.setAttribute("sortReverse", sortReverse);
    for (auto& entry : userPresets)
    {
        auto* presetXml = root.createNewChildElement("Preset");
        presetXml->setAttribute("name", entry.first);
        if (auto stateXml = entry.second.createXml())
            presetXml->addChildElement(stateXml.release());
    }
    root.writeTo(getUserPresetsFile());
}

//==============================================================================
// Sorting and perception profiles

float PerceptionPresetManager::defaultValueOf(const char* parameterId) const
{
    if (auto* parameter = apvtsRef.getParameter(parameterId))
        return parameter->convertFrom0to1(parameter->getDefaultValue());

    return 0.0f;
}

PerceptionProfile PerceptionPresetManager::profileOfFactoryPreset(const FactoryPreset& preset) const
{
    return computePerceptionProfile([this, &preset](const char* id) { return preset.number(id, defaultValueOf(id)); });
}

PerceptionProfile PerceptionPresetManager::profileOfUserPreset(const juce::ValueTree& state) const
{
    // A saved state lists each parameter as a child with an "id" and a "value" (plain units)
    std::map<juce::String, float> values;
    for (const auto& child : state)
        if (child.hasProperty("id") && child.hasProperty("value"))
            values[child.getProperty("id").toString()] = static_cast<float>(static_cast<double>(child.getProperty("value")));

    return computePerceptionProfile([this, &values](const char* id)
        {
            const auto it = values.find(id);
            return it != values.end() ? it->second : defaultValueOf(id);
        });
}

PerceptionProfile PerceptionPresetManager::getProfileOf(const juce::String& presetName) const
{
    const auto userIt = userPresets.find(presetName);
    if (userIt != userPresets.end())
        return profileOfUserPreset(userIt->second);

    for (const auto& preset : getFactoryPresets())
        if (preset.name == presetName)
            return profileOfFactoryPreset(preset);

    return {};
}

PerceptionProfile PerceptionPresetManager::getLiveProfile() const
{
    return computePerceptionProfile([this](const char* id)
        {
            if (auto* value = apvtsRef.getRawParameterValue(id))
                return value->load();
            return 0.0f;
        });
}

PerceptionPresetManager::SortedNames PerceptionPresetManager::getSortedPresetNames() const
{
    struct Entry { juce::String name; float score = 0.0f; int order = 0; };

    const bool byAxis = sortMode >= PresetSort::Brightness;
    const int axis = static_cast<int>(sortMode) - static_cast<int>(PresetSort::Brightness);

    // Strongest first for an axis; otherwise the natural order. Ties fall back to the natural order.
    auto sortEntries = [&](std::vector<Entry>& entries, bool naturalIsAlphabetical)
    {
        std::stable_sort(entries.begin(), entries.end(), [&](const Entry& a, const Entry& b)
            {
                if (byAxis && a.score != b.score)
                    return a.score > b.score;

                if (naturalIsAlphabetical)
                    return a.name.compareIgnoreCase(b.name) < 0;

                return a.order < b.order;
            });

        if (sortReverse)
            std::reverse(entries.begin(), entries.end());
    };

    std::vector<Entry> factory, user;
    int index = 0;
    for (const auto& preset : getFactoryPresets())
        factory.push_back({ preset.name, byAxis ? profileOfFactoryPreset(preset).score[axis] : 0.0f, index++ });

    for (const auto& [name, state] : userPresets)
        user.push_back({ name, byAxis ? profileOfUserPreset(state).score[axis] : 0.0f, 0 });

    // Alphabetical sorts both lists by name; the factory list otherwise keeps its curated order
    sortEntries(factory, sortMode == PresetSort::Alphabetical);
    sortEntries(user, true);

    SortedNames result;

    // "Init" is the blank starting point, so it stays on top in every order
    for (const auto& entry : factory)
        if (entry.name == "Init")
            result.factory.add(entry.name);
    for (const auto& entry : factory)
        if (entry.name != "Init")
            result.factory.add(entry.name);
    for (const auto& entry : user)
        result.user.add(entry.name);

    return result;
}

void PerceptionPresetManager::setSort(PresetSort mode, bool reverse)
{
    sortMode = mode;
    sortReverse = reverse;
    saveUserPresetsToDisk();
}

juce::String PerceptionPresetManager::describeSort(PresetSort mode, bool reverse)
{
    switch (mode)
    {
    case PresetSort::FactoryOrder: return reverse ? "Factory order, reversed" : "Factory order";
    case PresetSort::Alphabetical: return reverse ? "Z to A" : "A to Z";
    default:
    {
        const auto axis = static_cast<PerceptionAxis>(static_cast<int>(mode) - static_cast<int>(PresetSort::Brightness));
        return juce::String(perceptionAxisName(axis)) + " - " + (reverse ? perceptionAxisLowWord(axis) : perceptionAxisHighWord(axis)) + " first";
    }
    }
}
