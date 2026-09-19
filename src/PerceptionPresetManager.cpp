#include "PerceptionPresetManager.h"

PerceptionPresetManager::PerceptionPresetManager(juce::AudioProcessorValueTreeState& apvts,
    TiltEQComponent& tiltEQ,
    WidthBalancerComponent& width,
    ModDelayComponent& delay,
    SpatialFXComponent& spatial,
    MicroPitchDetuneComponent& microPitch,
    ExciterSaturationComponent& exciterSaturation,
    SimpleVerbWithPredelayComponent& simpleVerb)
    : apvtsRef(apvts)
    , tiltEQComponent(tiltEQ)
    , widthComponent(width)
    , delayComponent(delay)
    , spatialFXComponent(spatial)
    , microPitchComponent(microPitch)
    , exciterSaturationComponent(exciterSaturation)
    , simpleVerbComponent(simpleVerb)
{
    initializePresets();
    loadUserPresets();
}

void PerceptionPresetManager::applyPreset(const juce::String& presetName)
{
    auto userIt = userPresets.find(presetName);
    if (userIt != userPresets.end())
    {
        apvtsRef.replaceState(userIt->second.createCopy());
        DBG("Applied user preset: " + presetName);

        // The APVTS only flushes parameter changes into its state ValueTree periodically,
        // so snapshot the baseline once that's had a chance to happen rather than reading
        // it back immediately.
        juce::Timer::callAfterDelay(120, [this]() { lastAppliedPresetState = apvtsRef.copyState(); });
        return;
    }

    auto it = presets.find(presetName);
    if (it != presets.end())
    {
        it->second(); // Call the preset lambda
        DBG("Applied preset: " + presetName);
        juce::Timer::callAfterDelay(120, [this]() { lastAppliedPresetState = apvtsRef.copyState(); });
    }
    else
    {
        DBG("Preset not found: " + presetName);
    }
}

bool PerceptionPresetManager::isFactoryPreset(const juce::String& presetName) const
{
    return presets.find(presetName) != presets.end();
}

bool PerceptionPresetManager::isUserPreset(const juce::String& presetName) const
{
    return userPresets.find(presetName) != userPresets.end();
}

bool PerceptionPresetManager::matchesLastAppliedPreset() const
{
    if (!lastAppliedPresetState.isValid())
        return true;

    return apvtsRef.copyState().isEquivalentTo(lastAppliedPresetState);
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
        if (midSide > 0.3f)
            add("Centre-Weighted", "weighted toward the centre image, feeling solid and grounded");
        else if (midSide < -0.3f)
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

    userPresets[presetName] = apvtsRef.copyState();
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

void PerceptionPresetManager::usePreset(ModDelay::ModulationType type, float delayTime, float feedbackLeft, float feedbackRight,
    float modMix, float delayModDepth, float delayModRate, bool syncEnabled,
    float width, float intensity, float midSideBalance, bool mono, float tiltEQ,
    float phaseOffsetL, float phaseOffsetR, float modulationRateL, float modulationRateR, float modulationDepthL, float modulationDepthR,
    float wetDryMix, float lfoPhaseOffset, float allpassFrequency, float leftHaasMs, float rightHaasMs, SpatialFX::LfoWaveform modShape,
    float detuneAmount, float lfoRate, float lfoDepth, float delayCentre, float stereoSeparation, float mix, float detuneFeedback, float diffusion,
    float drive, float exciterMix, float highpass,
    ExciterSaturation::SaturationType saturationType, ExciterSaturation::HarmonicMode harmonicMode,
    float toneBrightness, float harmonicBalance, bool autoGainEnabled,
    float predelay, float size, float damping, float wet)
{
    // ModDelay
    delayComponent.setModulationType(type);
    delayComponent.setDelayTime(delayTime);
    delayComponent.setFeedbackLeft(feedbackLeft);
    delayComponent.setFeedbackRight(feedbackRight);
    delayComponent.setMix(modMix);
    delayComponent.setModDepth(delayModDepth);
    delayComponent.setModRate(delayModRate);
    delayComponent.setSyncEnabled(syncEnabled);

    // WidthBalancer
    widthComponent.setWidth(width);
    widthComponent.setIntensity(intensity);
    widthComponent.setMidSideBalance(midSideBalance);
    widthComponent.setMono(mono);

    // TiltEQ
    tiltEQComponent.setTilt(tiltEQ);

    // SpatialFX
    spatialFXComponent.setPhaseOffsetLeft(phaseOffsetL);
    spatialFXComponent.setPhaseOffsetRight(phaseOffsetR);
    spatialFXComponent.setModulationRate(modulationRateL, modulationRateR);
    spatialFXComponent.setModulationDepth(modulationDepthL, modulationDepthR);
    spatialFXComponent.setWetDryMix(wetDryMix);
    spatialFXComponent.setLfoPhaseOffset(lfoPhaseOffset);
    spatialFXComponent.setAllpassFrequency(allpassFrequency);
    spatialFXComponent.setHaasDelayMs(leftHaasMs, rightHaasMs);
    spatialFXComponent.setModShape(modShape);

    // MicroPitchDetune
    microPitchComponent.setDetuneAmount(detuneAmount);
    microPitchComponent.setLfoRate(lfoRate);
    microPitchComponent.setLfoDepth(lfoDepth);
    microPitchComponent.setDelayCentre(delayCentre);
    microPitchComponent.setStereoSeparation(stereoSeparation);
    microPitchComponent.setMix(mix);
    microPitchComponent.setDetuneFeedback(detuneFeedback);
    microPitchComponent.setDiffusion(diffusion);

    // ExciterSaturation
    exciterSaturationComponent.setDrive(drive);
    exciterSaturationComponent.setMix(exciterMix);
    exciterSaturationComponent.setHighpass(highpass);
    exciterSaturationComponent.setSaturationType(static_cast<int>(saturationType));
    exciterSaturationComponent.setHarmonicMode(static_cast<int>(harmonicMode));
    exciterSaturationComponent.setToneBrightness(toneBrightness);
    exciterSaturationComponent.setHarmonicBalance(harmonicBalance);
    exciterSaturationComponent.setAutoGain(autoGainEnabled);

    // SimpleVerbWithPredelay
    simpleVerbComponent.setPredelay(predelay);
    simpleVerbComponent.setSize(size);
    simpleVerbComponent.setDamping(damping);
    simpleVerbComponent.setWet(wet);
}

void PerceptionPresetManager::initializePresets()
{
    // Preset format: modType, delayTime, feedbackL, feedbackR, modMix, modDepth, modRate, sync,
    //                width, intensity, midSideBalance, mono, tiltEQ,
    //                phaseOffsetL, phaseOffsetR, modRateL, modRateR, modDepthL, modDepthR,
    //                wetDryMix, lfoPhaseOffset, allpassFreq, haasL, haasR, modShape,
    //                detune, lfoRate, lfoDepth, delayCentre, stereoSep, mix, detuneFeedback, diffusion,
    //                drive, exciterMix, highpass, saturationType, harmonicMode, toneBrightness, harmonicBalance, autoGain,
    //                predelay, size, damping, wet

    using Sat = ExciterSaturation::SaturationType;
    using Harm = ExciterSaturation::HarmonicMode;

    // Blank / Init: every effect neutral or fully dry, a clean starting point for
    // building a custom sound rather than starting from one of the character presets below.
    presets["Init"] = [this]() {
        usePreset(ModDelay::ModulationType::Sine,
            100.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.25f, false,
            1.0f, 0.0f, 0.0f, false, 0.0f,
            0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 1000.0f, 0.0f, 0.0f, SpatialFX::LfoWaveform::Sine,
            0.0f, 0.1f, 0.0f, 0.005f, 0.0f, 0.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 1000.0f, Sat::Soft, Harm::Balanced, 0.5f, 0.5f, true,
            0.0f, 0.0f, 0.3f, 0.0f);
        };

    // Swirling, warped, disorienting - a heady psychedelic trip.
    presets["Head Trip"] = [this]() {
        usePreset(ModDelay::ModulationType::Triangle,
            400.0f, 0.7f, 0.75f, 0.6f, 4.0f, 0.2f, false,
            1.5f, 0.8f, 0.0f, false, 0.1f,
            0.08f, -0.05f, 0.3f, 0.3f, 0.6f, 0.6f,
            0.7f, 0.25f, 2500.0f, 0.5f, 0.6f, SpatialFX::LfoWaveform::Sine,
            3.0f, 0.25f, 0.0015f, 0.006f, 0.8f, 0.5f, 0.45f, 0.6f,
            6.0f, 0.4f, 100.0f, Sat::Tube, Harm::OddOnly, 0.6f, 0.55f, true,
            80.0f, 0.85f, 0.5f, 0.4f);
        };

    // Claustrophobic and tense - a rhythmic, harsh pulse that won't let up.
    presets["Panic Room"] = [this]() {
        usePreset(ModDelay::ModulationType::Square,
            150.0f, 0.8f, 0.7f, 0.9f, 6.0f, 1.5f, true,
            0.3f, 0.2f, 0.2f, false, -0.15f,
            0.15f, -0.18f, 1.2f, 0.8f, 0.85f, 1.1f,
            0.9f, 0.2f, 3200.0f, 0.1f, 0.2f, SpatialFX::LfoWaveform::Triangle,
            -5.0f, 3.0f, 0.0008f, 0.003f, 0.3f, 0.7f, 0.15f, 0.15f,
            7.5f, 0.65f, 200.0f, Sat::Hard, Harm::OddOnly, 0.75f, 0.6f, false,
            20.0f, 0.3f, 0.85f, 0.25f);
        };

    // Close, warm, gentle - a soft tube glow instead of any harshness.
    presets["Intimacy"] = [this]() {
        usePreset(ModDelay::ModulationType::Sine,
            80.0f, 0.3f, 0.35f, 0.4f, 1.0f, 0.1f, false,
            0.8f, 0.9f, -0.1f, false, 0.05f,
            0.03f, -0.02f, 0.15f, 0.15f, 0.25f, 0.25f,
            0.35f, 0.1f, 1600.0f, 0.2f, 0.2f, SpatialFX::LfoWaveform::Sine,
            1.5f, 0.1f, 0.0002f, 0.001f, 0.4f, 0.3f, 0.05f, 0.2f,
            2.0f, 0.2f, 50.0f, Sat::Tube, Harm::EvenOnly, 0.4f, 0.45f, true,
            15.0f, 0.25f, 0.3f, 0.3f);
        };

    // Cold, synthetic, neon-lit dystopia, driven by a mechanical pulse.
    presets["Blade Runner"] = [this]() {
        usePreset(ModDelay::ModulationType::SawtoothDown,
            550.0f, 0.65f, 0.6f, 0.7f, 3.0f, 0.3f, true,
            1.2f, 0.7f, 0.1f, false, -0.08f,
            0.2f, -0.1f, 0.4f, 0.6f, 0.5f, 0.7f,
            0.85f, 0.3f, 4200.0f, 0.7f, 0.3f, SpatialFX::LfoWaveform::Random,
            -2.0f, 0.6f, 0.0018f, 0.004f, 0.75f, 0.6f, 0.35f, 0.5f,
            4.5f, 0.55f, 120.0f, Sat::Digital, Harm::OddOnly, 0.65f, 0.5f, true,
            100.0f, 0.95f, 0.6f, 0.5f);
        };

    // Otherworldly and non-human - electromagnetic hum and a smooth, alien shimmer.
    presets["Alien Abduction"] = [this]() {
        usePreset(ModDelay::ModulationType::Sine,
            350.0f, 0.55f, 0.6f, 0.85f, 4.5f, 0.6f, false,
            0.9f, 0.75f, 0.05f, false, -0.07f,
            0.18f, -0.22f, 0.9f, 0.9f, 0.7f, 0.7f,
            0.8f, 0.45f, 4200.0f, 0.6f, 0.6f, SpatialFX::LfoWaveform::Sine,
            4.0f, 1.2f, 0.001f, 0.0025f, 0.9f, 0.65f, 0.5f, 0.55f,
            5.5f, 0.6f, 180.0f, Sat::Transformer, Harm::EvenOnly, 0.7f, 0.6f, true,
            100.0f, 0.9f, 0.4f, 0.55f);
        };

    // Smooth, reflective, and clean - light bouncing down a long glass corridor.
    presets["Glass Tunnel"] = [this]() {
        usePreset(ModDelay::ModulationType::SawtoothUp,
            220.0f, 0.45f, 0.5f, 0.65f, 1.8f, 0.15f, false,
            0.7f, 0.85f, -0.05f, false, 0.02f,
            0.05f, -0.08f, 0.25f, 0.25f, 0.4f, 0.4f,
            0.65f, 0.2f, 3200.0f, 0.5f, 0.5f, SpatialFX::LfoWaveform::Triangle,
            1.0f, 0.3f, 0.0005f, 0.0015f, 0.55f, 0.4f, 0.2f, 0.65f,
            3.5f, 0.35f, 80.0f, Sat::Soft, Harm::EvenOnly, 0.75f, 0.5f, true,
            60.0f, 0.65f, 0.8f, 0.35f);
        };

    // Surreal, illogical flow - warm and hazy the way dreams drift from scene to scene.
    presets["Dream Logic"] = [this]() {
        usePreset(ModDelay::ModulationType::Sine,
            280.0f, 0.5f, 0.55f, 0.75f, 2.2f, 0.25f, false,
            0.88f, 0.78f, 0.02f, false, -0.03f,
            0.06f, -0.04f, 0.35f, 0.35f, 0.48f, 0.48f,
            0.55f, 0.3f, 3000.0f, 0.4f, 0.4f, SpatialFX::LfoWaveform::Sine,
            1.8f, 0.5f, 0.0007f, 0.002f, 0.6f, 0.45f, 0.3f, 0.6f,
            3.0f, 0.3f, 70.0f, Sat::Tape, Harm::Balanced, 0.55f, 0.5f, true,
            90.0f, 0.75f, 0.45f, 0.45f);
        };

    // Enclosed, muffled, primal safety - dark and contained, not vast.
    presets["Womb Space"] = [this]() {
        usePreset(ModDelay::ModulationType::Sine,
            90.0f, 0.35f, 0.4f, 0.2f, 0.7f, 0.06f, false,
            0.55f, 1.0f, -0.25f, true, -0.12f,
            0.02f, -0.015f, 0.08f, 0.08f, 0.15f, 0.15f,
            0.3f, 0.05f, 2000.0f, 0.2f, 0.2f, SpatialFX::LfoWaveform::Sine,
            0.3f, 0.1f, 0.0001f, 0.0004f, 0.25f, 0.2f, 0.1f, 0.3f,
            1.0f, 0.15f, 10.0f, Sat::Tube, Harm::EvenOnly, 0.15f, 0.4f, true,
            5.0f, 0.8f, 0.25f, 0.35f);
        };

    // Unstable mood swings - erratic modulation and dynamics that surge and sag.
    presets["Bipolar Bloom"] = [this]() {
        usePreset(ModDelay::ModulationType::Triangle,
            450.0f, 0.9f, 0.5f, 0.8f, 4.0f, 0.35f, false,
            1.1f, 0.65f, 0.15f, false, 0.08f,
            0.18f, -0.12f, 0.7f, 0.7f, 0.65f, 0.65f,
            0.75f, 0.3f, 4200.0f, 0.3f, 0.25f, SpatialFX::LfoWaveform::Random,
            -3.5f, 1.5f, 0.0009f, 0.0022f, 0.95f, 0.68f, 0.4f, 0.45f,
            6.5f, 0.5f, 110.0f, Sat::Hard, Harm::OddOnly, 0.6f, 0.55f, false,
            70.0f, 0.95f, 0.35f, 0.6f);
        };

    // Calm and grounded - a subtle, weighty confidence rather than showiness.
    presets["Quiet Confidence"] = [this]() {
        usePreset(ModDelay::ModulationType::Triangle,
            120.0f, 0.4f, 0.45f, 0.5f, 1.2f, 0.12f, false,
            0.75f, 0.9f, -0.02f, false, 0.03f,
            0.04f, -0.03f, 0.2f, 0.2f, 0.35f, 0.35f,
            0.45f, 0.2f, 2400.0f, 0.15f, 0.2f, SpatialFX::LfoWaveform::Sine,
            1.2f, 0.2f, 0.0004f, 0.0012f, 0.45f, 0.3f, 0.1f, 0.25f,
            2.8f, 0.2f, 60.0f, Sat::Transformer, Harm::Balanced, 0.45f, 0.5f, true,
            20.0f, 0.45f, 0.25f, 0.3f);
        };

    // Weightless and paradoxically uplifting - light, airy, floating free.
    presets["Falling Upwards"] = [this]() {
        usePreset(ModDelay::ModulationType::Sine,
            180.0f, 0.6f, 0.65f, 0.55f, 2.5f, 0.2f, false,
            1.0f, 0.7f, 0.05f, false, 0.04f,
            0.1f, -0.07f, 0.3f, 0.3f, 0.45f, 0.45f,
            0.6f, 0.22f, 1800.0f, 0.25f, 0.25f, SpatialFX::LfoWaveform::Triangle,
            2.2f, 0.4f, 0.0006f, 0.0018f, 0.5f, 0.5f, 0.25f, 0.5f,
            4.0f, 0.7f, 100.0f, Sat::Digital, Harm::EvenOnly, 0.65f, 0.55f, true,
            40.0f, 0.85f, 0.3f, 0.55f);
        };

    // Hot, viscous, radiant - a warm glowing overdrive at its core.
    presets["Molten Light"] = [this]() {
        usePreset(ModDelay::ModulationType::Sine,
            270.0f, 0.8f, 0.75f, 0.7f, 3.8f, 0.25f, false,
            1.3f, 0.9f, 0.1f, false, 0.06f,
            0.18f, -0.15f, 0.5f, 0.65f, 0.75f, 0.75f,
            0.4f, 0.12f, 4200.0f, 0.5f, 0.3f, SpatialFX::LfoWaveform::Sine,
            3.0f, 0.5f, 0.0007f, 0.002f, 0.75f, 0.55f, 0.35f, 0.55f,
            7.5f, 0.75f, 120.0f, Sat::Tube, Harm::EvenOnly, 0.55f, 0.6f, true,
            60.0f, 0.7f, 0.2f, 0.7f);
        };

    // Airy, ghostly, spacious - as light and untouched as an echo can be.
    presets["Ethereal Echo"] = [this]() {
        usePreset(ModDelay::ModulationType::Sine,
            350.0f, 0.6f, 0.65f, 0.75f, 2.5f, 0.3f, false,
            1.1f, 0.85f, -0.05f, false, 0.04f,
            0.12f, -0.1f, 0.4f, 0.55f, 0.65f, 0.65f,
            0.3f, 0.09f, 4200.0f, 0.4f, 0.3f, SpatialFX::LfoWaveform::Triangle,
            -2.5f, 1.2f, 0.0008f, 0.0025f, 0.75f, 0.6f, 0.2f, 0.7f,
            5.5f, 0.7f, 150.0f, Sat::Soft, Harm::EvenOnly, 0.75f, 0.5f, true,
            50.0f, 0.9f, 0.15f, 0.65f);
        };

    // Rich, thick, enveloping - a lush, nostalgic dream you sink into.
    presets["Lush Dreamscape"] = [this]() {
        usePreset(ModDelay::ModulationType::Triangle,
            400.0f, 0.7f, 0.75f, 0.8f, 3.0f, 0.35f, false,
            1.2f, 0.9f, 0.1f, false, 0.05f,
            0.15f, -0.12f, 0.5f, 0.65f, 0.75f, 0.75f,
            0.2f, 0.06f, 4200.0f, 0.6f, 0.3f, SpatialFX::LfoWaveform::Sine,
            -3.5f, 1.5f, 0.0009f, 0.0022f, 0.8f, 0.7f, 0.4f, 0.75f,
            6.5f, 0.8f, 200.0f, Sat::Tape, Harm::Balanced, 0.6f, 0.55f, true,
            60.0f, 1.0f, 0.3f, 0.75f);
        };

    // Tactile and close - warm human contact, never harsh, never diffuse.
    presets["Skin Contact"] = [this]() {
        usePreset(ModDelay::ModulationType::Triangle,
            90.0f, 0.35f, 0.4f, 0.4f, 1.0f, 0.08f, false,
            0.75f, 0.95f, -0.12f, false, 0.02f,
            0.02f, -0.018f, 0.12f, 0.2f, 0.5f, 0.5f,
            0.1f, 0.03f, 4200.0f, 0.4f, 0.3f, SpatialFX::LfoWaveform::Random,
            1.1f, 0.15f, 0.0001f, 0.0004f, 0.4f, 0.25f, 0.1f, 0.15f,
            4.0f, 0.45f, 44.0f, Sat::Tube, Harm::EvenOnly, 0.35f, 0.45f, true,
            10.0f, 0.35f, 0.1f, 0.2f);
        };

    // Enveloping, wraparound warmth - a full-bodied embrace.
    presets["Sonic Embrace"] = [this]() {
        usePreset(ModDelay::ModulationType::Sine,
            200.0f, 0.5f, 0.55f, 0.6f, 2.0f, 0.15f, false,
            0.8f, 0.9f, -0.08f, false, 0.03f,
            0.08f, -0.06f, 0.25f, 0.35f, 0.45f, 0.45f,
            0.25f, 0.1f, 4200.0f, 0.3f, 0.25f, SpatialFX::LfoWaveform::Sine,
            -1.5f, 0.25f, 0.0003f, 0.0008f, 0.5f, 0.35f, 0.2f, 0.5f,
            4.5f, 0.6f, 120.0f, Sat::Transformer, Harm::Balanced, 0.5f, 0.5f, true,
            30.0f, 0.75f, 0.25f, 0.5f);
        };

    // Rhythmic, flashing, euphoric - a driving, synced strobe of a beat.
    presets["Strobe Heaven"] = [this]() {
        usePreset(ModDelay::ModulationType::Square,
            90.0f, 0.7f, 0.7f, 0.85f, 3.2f, 1.6f, true,
            0.4f, 0.6f, 0.3f, false, -0.1f,
            0.2f, -0.2f, 1.4f, 1.4f, 0.9f, 0.9f,
            1.0f, 0.2f, 4200.0f, 0.7f, 0.7f, SpatialFX::LfoWaveform::Triangle,
            -4.0f, 2.0f, 0.0012f, 0.0025f, 0.6f, 0.7f, 0.3f, 0.35f,
            8.5f, 0.75f, 150.0f, Sat::Digital, Harm::OddOnly, 0.85f, 0.55f, false,
            20.0f, 0.6f, 0.1f, 0.9f);
        };

    // Cool glass meeting hot flame - smooth surfaces with a biting edge.
    presets["Glass Flame"] = [this]() {
        usePreset(ModDelay::ModulationType::Sine,
            300.0f, 0.5f, 0.5f, 0.65f, 2.0f, 0.18f, false,
            1.0f, 0.8f, -0.05f, false, 0.0f,
            0.05f, -0.05f, 0.35f, 0.35f, 0.5f, 0.5f,
            0.65f, 0.15f, 3300.0f, 0.4f, 0.4f, SpatialFX::LfoWaveform::Sine,
            2.0f, 0.3f, 0.0004f, 0.0016f, 0.6f, 0.4f, 0.3f, 0.5f,
            5.5f, 0.8f, 110.0f, Sat::Hard, Harm::OddOnly, 0.7f, 0.5f, true,
            30.0f, 0.8f, 0.2f, 0.5f);
        };

    // Vast, cosmic, reverent - a huge, weighty space that dwarfs everything in it.
    presets["Celestial Vault"] = [this]() {
        usePreset(ModDelay::ModulationType::Sine,
            320.0f, 0.6f, 0.6f, 0.4f, 2.0f, 0.15f, false,
            1.2f, 0.7f, 0.0f, false, -0.05f,
            0.02f, 0.02f, 0.15f, 0.15f, 0.5f, 0.5f,
            0.65f, 0.05f, 2800.0f, 0.5f, 0.5f, SpatialFX::LfoWaveform::Sine,
            2.0f, 0.2f, 0.001f, 0.005f, 0.75f, 0.4f, 0.15f, 0.6f,
            4.5f, 0.35f, 120.0f, Sat::Transformer, Harm::EvenOnly, 0.6f, 0.5f, true,
            100.0f, 1.0f, 0.45f, 0.7f);
        };

    // Murky and uncertain - a hazy, tape-warped trick of perception.
    presets["Deep Illusion"] = [this]() {
        usePreset(ModDelay::ModulationType::Triangle,
            280.0f, 0.4f, 0.4f, 0.3f, 1.0f, 0.2f, false,
            1.6f, 0.6f, 0.1f, false, 0.02f,
            -0.02f, 0.03f, 0.2f, 0.2f, 0.35f, 0.35f,
            0.6f, 0.1f, 3600.0f, 0.3f, 0.3f, SpatialFX::LfoWaveform::Triangle,
            2.5f, 0.15f, 0.001f, 0.004f, 0.5f, 0.35f, 0.35f, 0.45f,
            3.5f, 0.25f, 90.0f, Sat::Tape, Harm::Balanced, 0.35f, 0.5f, true,
            60.0f, 0.9f, 0.6f, 0.5f);
        };

    // Dissolving self - boundaries blur into a maximally diffuse, expansive haze.
    presets["Ego Dissolve"] = [this]() {
        usePreset(ModDelay::ModulationType::Sine,
            500.0f, 0.3f, 0.3f, 0.5f, 3.0f, 0.4f, false,
            1.5f, 0.5f, 0.1f, false, 0.0f,
            0.1f, -0.1f, 0.35f, 0.35f, 0.5f, 0.5f,
            0.55f, 0.2f, 2400.0f, 0.4f, 0.4f, SpatialFX::LfoWaveform::Sine,
            1.5f, 0.3f, 0.0015f, 0.0065f, 0.75f, 0.45f, 0.55f, 0.8f,
            5.5f, 0.3f, 100.0f, Sat::Digital, Harm::EvenOnly, 0.55f, 0.6f, true,
            100.0f, 0.88f, 0.7f, 0.6f);
        };

    // Faded and decayed - the gritty, tape-worn residue of an old memory.
    presets["Memory Dust"] = [this]() {
        usePreset(ModDelay::ModulationType::Triangle,
            360.0f, 0.5f, 0.5f, 0.3f, 1.5f, 0.25f, false,
            1.3f, 0.65f, 0.05f, false, 0.03f,
            0.04f, 0.05f, 0.2f, 0.2f, 0.3f, 0.3f,
            0.5f, 0.15f, 1600.0f, 0.08f, 0.08f, SpatialFX::LfoWaveform::Random,
            2.8f, 0.25f, 0.001f, 0.005f, 0.7f, 0.4f, 0.25f, 0.4f,
            4.0f, 0.2f, 80.0f, Sat::Tape, Harm::OddOnly, 0.3f, 0.45f, true,
            70.0f, 0.9f, 0.55f, 0.45f);
        };

    // Light, quick, playful - a simple, tempo-locked slapback echo.
    presets["Gentle Slap"] = [this]() {
        usePreset(ModDelay::ModulationType::Sine,
            100.0f, 0.4f, 0.4f, 0.1f, 0.6f, 0.05f, true,
            0.2f, 0.4f, 0.0f, false, 0.02f,
            0.02f, -0.02f, 0.1f, 0.1f, 0.15f, 0.15f,
            0.25f, 0.2f, 4200.0f, 0.05f, 0.05f, SpatialFX::LfoWaveform::Sine,
            1.2f, 0.1f, 0.0003f, 0.0012f, 0.4f, 0.3f, 0.05f, 0.1f,
            2.5f, 0.15f, 50.0f, Sat::Soft, Harm::Balanced, 0.5f, 0.5f, true,
            20.0f, 0.35f, 0.2f, 0.3f);
        };

    // Gentle rhythmic sway under a cool night sky - a synced, shimmering waltz.
    presets["Moon Dance"] = [this]() {
        usePreset(ModDelay::ModulationType::Sine,
            200.0f, 0.5f, 0.5f, 0.6f, 2.0f, 0.15f, true,
            1.2f, 0.7f, -0.05f, false, 0.02f,
            0.05f, -0.05f, 0.25f, 0.25f, 0.35f, 0.35f,
            0.45f, 0.4f, 4200.0f, 0.2f, 0.2f, SpatialFX::LfoWaveform::Triangle,
            1.5f, 0.25f, 0.0003f, 0.0008f, 0.5f, 0.35f, 0.2f, 0.45f,
            3.0f, 0.5f, 40.0f, Sat::Digital, Harm::EvenOnly, 0.6f, 0.5f, true,
            30.0f, 0.75f, 0.25f, 0.5f);
        };

    // Sharp sensual tension - a sting of edge and bite, distinct from a gentle slap.
    presets["Biting Lips"] = [this]() {
        usePreset(ModDelay::ModulationType::Sine,
            80.0f, 0.45f, 0.45f, 0.25f, 0.5f, 0.08f, false,
            0.3f, 0.9f, -0.05f, false, 0.1f,
            0.02f, -0.02f, 0.1f, 0.1f, 0.15f, 0.15f,
            0.25f, 0.3f, 4200.0f, 0.1f, 0.1f, SpatialFX::LfoWaveform::Random,
            1.2f, 0.1f, 0.0003f, 0.0012f, 0.4f, 0.3f, 0.15f, 0.2f,
            3.5f, 0.15f, 90.0f, Sat::Hard, Harm::OddOnly, 0.6f, 0.55f, true,
            20.0f, 0.35f, 0.2f, 0.3f);
        };

    // Turbulent, dark weather - rough and erratic, distinct from Glass Flame's clean bite.
    presets["Stormy Day"] = [this]() {
        usePreset(ModDelay::ModulationType::Sine,
            300.0f, 0.5f, 0.5f, 0.65f, 2.0f, 0.18f, false,
            1.0f, 0.8f, -0.05f, false, -0.1f,
            0.05f, -0.05f, 0.35f, 0.35f, 0.5f, 0.5f,
            0.65f, 0.45f, 4200.0f, 0.3f, 0.3f, SpatialFX::LfoWaveform::Random,
            2.0f, 0.3f, 0.0004f, 0.0016f, 0.6f, 0.4f, 0.35f, 0.5f,
            6.5f, 0.8f, 90.0f, Sat::Hard, Harm::OddOnly, 0.35f, 0.5f, false,
            30.0f, 0.8f, 0.35f, 0.5f);
        };

    // Warm, golden, relaxed - the glow of a fading summer evening.
    presets["Summer Sunset"] = [this]() {
        usePreset(ModDelay::ModulationType::Sine,
            400.0f, 0.6f, 0.6f, 0.6f, 1.5f, 0.15f, false,
            1.1f, 0.8f, 0.1f, false, -0.04f,
            0.15f, -0.12f, 0.25f, 0.25f, 0.4f, 0.4f,
            0.75f, 0.15f, 4200.0f, 0.35f, 0.15f, SpatialFX::LfoWaveform::Sine,
            -0.8f, 0.8f, 0.0004f, 0.0015f, 0.6f, 0.5f, 0.15f, 0.4f,
            4.0f, 0.5f, 80.0f, Sat::Tube, Harm::EvenOnly, 0.55f, 0.5f, true,
            60.0f, 1.0f, 0.3f, 0.75f);
        };

    // Flowing, rolling, natural - the analog warmth of vast rolling water.
    presets["Ocean Waves"] = [this]() {
        usePreset(ModDelay::ModulationType::Triangle,
            250.0f, 0.5f, 0.5f, 0.6f, 2.5f, 0.2f, false,
            1.3f, 0.7f, 0.05f, false, 0.03f,
            0.08f, -0.06f, 0.3f, 0.3f, 0.45f, 0.45f,
            0.55f, 0.12f, 4200.0f, 0.3f, 0.12f, SpatialFX::LfoWaveform::Triangle,
            -1.5f, 0.4f, 0.0006f, 0.0022f, 0.65f, 0.5f, 0.2f, 0.55f,
            4.5f, 0.6f, 120.0f, Sat::Tape, Harm::Balanced, 0.5f, 0.5f, true,
            50.0f, 0.9f, 0.25f, 0.55f);
        };

    // Pristine and transparent - minimal coloration, maximum clarity.
    presets["Crystal Clear"] = [this]() {
        usePreset(ModDelay::ModulationType::Sine,
            110.0f, 0.2f, 0.2f, 0.25f, 0.6f, 0.05f, false,
            0.6f, 0.6f, -0.15f, false, 0.01f,
            0.01f, -0.01f, 0.05f, 0.05f, 0.1f, 0.1f,
            0.2f, 0.05f, 4200.0f, 0.15f, 0.05f, SpatialFX::LfoWaveform::Sine,
            0.7f, 0.1f, 0.0002f, 0.0008f, 0.5f, 0.2f, 0.0f, 0.05f,
            1.2f, 0.25f, 400.0f, Sat::Soft, Harm::Balanced, 0.8f, 0.5f, true,
            10.0f, 0.4f, 0.3f, 0.3f);
        };

    // Sweet and nostalgic - a warm, fond, tape-like glow of remembrance.
    presets["Sweetest Memory"] = [this]() {
        usePreset(ModDelay::ModulationType::Sine,
            220.0f, 0.45f, 0.4f, 0.55f, 1.2f, 0.15f, false,
            1.2f, 0.65f, -0.05f, false, 0.02f,
            0.07f, -0.04f, 0.28f, 0.28f, 0.38f, 0.38f,
            0.55f, 0.05f, 4200.0f, 0.4f, 0.05f, SpatialFX::LfoWaveform::Sine,
            -1.2f, 0.25f, 0.0003f, 0.0009f, 0.55f, 0.35f, 0.15f, 0.45f,
            3.0f, 0.5f, 90.0f, Sat::Tape, Harm::EvenOnly, 0.5f, 0.5f, true,
            60.0f, 0.8f, 0.3f, 0.65f);
        };
}