#include "PerceptionPresetManager.h"

PerceptionPresetManager::PerceptionPresetManager(juce::AudioProcessorValueTreeState& apvts)
    : apvtsRef(apvts)
{
    loadUserPresets();
}

namespace
{
    constexpr const char* kOutputTrimId = "outputGain";

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
        juce::Timer::callAfterDelay(120, [this]() { lastAppliedPresetState = apvtsRef.copyState(); });
        return;
    }

    for (const auto& preset : getFactoryPresets())
    {
        if (preset.name == presetName)
        {
            applyFactoryPreset(preset);
            DBG("Applied preset: " + presetName);
            juce::Timer::callAfterDelay(120, [this]() { lastAppliedPresetState = apvtsRef.copyState(); });
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

bool PerceptionPresetManager::matchesLastAppliedPreset() const
{
    if (!lastAppliedPresetState.isValid())
        return true;

    return withoutOutputTrim(apvtsRef.copyState()).isEquivalentTo(withoutOutputTrim(lastAppliedPresetState.createCopy()));
}

void PerceptionPresetManager::computeDescriptors(juce::StringArray& tags, juce::StringArray& clauses) const
{
    auto raw = [this](const char* id) { return apvtsRef.getRawParameterValue(id)->load(); };
    auto add = [&](const juce::String& tag, const juce::String& clause) { tags.add(tag); clauses.add(clause); };

    // Stereo image
    const bool isMono = raw("mono") >= 0.5f;
    const float width = raw("width");
    const float midSide = raw("midSideBalance");

    if (isMono)
        add("Mono", "collapsed to mono, feeling boxed-in and claustrophobic");
    else if (width > 1.3f)
        add("Very Wide", "very wide, pushed well beyond the speakers - feels expansive and larger-than-life");
    else if (width > 1.05f)
        add("Wide", "wider than natural, feeling open and airy");
    else if (width < 0.7f)
        add("Narrow", "narrow and pulled toward the centre, feeling close and focused");
    else if (width < 0.95f)
        add("Slightly Narrow", "slightly narrowed, a touch more centred");

    if (!isMono)
    {
        // Mid/Side Balance: -1 = all mid (centre), +1 = all side. Matches the slider's left/right ends.
        if (midSide < -0.3f)
            add("Centre-Weighted", "weighted toward the centre image, feeling solid and grounded");
        else if (midSide > 0.3f)
            add("Diffuse Sides", "weighted toward the sides, feeling hazy and enveloping");
    }

    // Left/right pull, from phase and Haas offsets together
    const float phaseL = raw("phaseOffsetL");
    const float phaseR = raw("phaseOffsetR");
    const float haasL = raw("haasDelayL");
    const float haasR = raw("haasDelayR");
    const float pull = (phaseR - phaseL) * 5.0f + (haasR - haasL) * 0.1f;

    if (pull > 0.3f)
        add("Pulled Right", "pulled toward the right, a subtle sense of asymmetry");
    else if (pull < -0.3f)
        add("Pulled Left", "pulled toward the left, a subtle sense of asymmetry");
    else if (haasL > 5.0f || haasR > 5.0f)
        add("Haas Spread", "spread wide with a Haas-style stereo trick, feeling big without losing focus");

    // Brightness / tilt
    const float tilt = raw("tiltEQ");
    if (tilt > 0.15f)
        add("Bright", "brighter and more forward, feeling alert and present");
    else if (tilt < -0.15f)
        add("Warm & Dark", "warmer and darker, feeling cosy and enclosed");

    // Delay movement
    const float modDepth = raw("modDepth");
    const float modRate = raw("modRate");
    const float feedbackAvg = (raw("feedbackL") + raw("feedbackR")) * 0.5f;

    if (modDepth > 4.0f && modRate > 0.5f)
        add("Swirling", "actively swirling with fast modulated echoes, feeling disorienting and dreamlike");
    else if (modDepth > 4.0f)
        add("Slow Drift", "a slow, deep modulation drifting underneath, feeling hypnotic");
    else if (modDepth < 0.3f && feedbackAvg < 0.05f)
        add("Inert", "the delay is essentially inaudible, feeling static and untouched");

    if (feedbackAvg > 0.7f)
        add("Cascading Echoes", "long, cascading echo trails, feeling vast and otherworldly");

    // Micro-pitch detune
    const float detune = raw("detuneAmount");
    const float detuneAbs = detune < 0.0f ? -detune : detune;
    const float diffusion = raw("diffusion");

    if (detuneAbs > 15.0f)
        add("Unstable Shimmer", "pitch visibly drifting, feeling uncanny and unsettling");
    else if (detuneAbs > 3.0f)
        add("Shimmering", "a subtle pitch shimmer, feeling alive and slightly magical");

    if (diffusion > 0.5f)
        add("Blurred Pitch", "blurred and diffuse in pitch, feeling hazy and dreamlike");

    // Exciter / saturation character
    const float exciterMix = raw("exciterMix");
    const float exciterDrive = raw("exciterDrive");

    if (exciterMix > 0.15f && exciterDrive > 1.0f)
    {
        static const char* satTags[] = {
            "Soft Warmth", "Aggressive Edge", "Tube Warmth",
            "Lo-Fi Character", "Analog Heft", "Cold & Digital"
        };
        static const char* satWords[] = {
            "a gentle, soft-clipped warmth that feels comforting",
            "an aggressive, hard-clipped edge that feels tense and confrontational",
            "a vintage tube warmth that feels nostalgic and cosy",
            "a lo-fi, tape-worn character that feels nostalgic and familiar",
            "a weighty, analog-console heft that feels grounded",
            "a cold, synthetic bite that feels clinical and futuristic"
        };
        const int satType = juce::jlimit(0, 5, juce::roundToInt(raw("exciterSaturationType")));
        add(satTags[satType], juce::String("harmonically excited with ") + satWords[satType]);

        const int harmMode = juce::roundToInt(raw("exciterHarmonicMode"));
        if (harmMode == 1)
            add("Hollow", "a hollow, reedy harmonic tilt, feeling thin and eerie");
        else if (harmMode == 2)
            add("Rounded", "a warm, rounded harmonic tilt, feeling full and inviting");
    }

    // Reverb space
    const float size = raw("size");
    const float wet = raw("wet");
    const float predelay = raw("predelayMs");

    if (size > 0.65f && wet > 0.45f)
        add("Spacious", "a spacious, distant reverb tail, feeling immersive and awe-inducing");
    else if (size < 0.25f && wet < 0.25f)
        add("Close & Dry", "close and dry, feeling intimate and immediate");

    if (predelay > 40.0f)
        add("Detached Echo", "a distinct gap before the reverb blooms, like a held breath before it lands");
}

juce::StringArray PerceptionPresetManager::generateFeelingTags() const
{
    juce::StringArray tags, clauses;
    computeDescriptors(tags, clauses);

    if (tags.isEmpty())
        tags.add("Neutral");

    return tags;
}

juce::String PerceptionPresetManager::generateBreakdown() const
{
    juce::StringArray tags, clauses;
    computeDescriptors(tags, clauses);

    if (clauses.isEmpty())
        return "Nothing strongly colored yet - close to a neutral, untouched signal.";

    juce::String result = clauses[0].substring(0, 1).toUpperCase() + clauses[0].substring(1);
    for (int i = 1; i < clauses.size(); ++i)
        result += (i == clauses.size() - 1 ? ", and " : ", ") + clauses[i];
    result += ".";
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
    for (auto& entry : userPresets)
    {
        auto* presetXml = root.createNewChildElement("Preset");
        presetXml->setAttribute("name", entry.first);
        if (auto stateXml = entry.second.createXml())
            presetXml->addChildElement(stateXml.release());
    }
    root.writeTo(getUserPresetsFile());
}
