// Consistency test for the insight chips and description text.
//
// Applies every factory preset and 20,000 random settings (drawn from the real parameter ranges)
// and fails if the chips/description ever contradict each other. Build with
//   cmake -B build -DECHOPSYCHFX_BUILD_TESTS=ON && cmake --build build --target EchoPsychFXTests
// and run the resulting EchoPsychFXTests executable. Pass "-v" to print every preset's text.
#include "PluginProcessor.h"
#include "PerceptionPresetManager.h"
#include "FactoryPresets.h"
#include <cstdio>
#include <map>
#include <random>
#include <string>
#include <vector>

namespace
{
    std::string lower(std::string s)
    {
        for (auto& c : s) c = static_cast<char>(tolower(static_cast<unsigned char>(c)));
        return s;
    }

    int failures = 0;
    void check(bool ok, const std::string& what)
    {
        if (!ok) { ++failures; std::printf("FAIL: %s\n", what.c_str()); }
    }
}

int main(int argc, char** argv)
{
    juce::ScopedJuceInitialiser_GUI init;
    AudioPluginAudioProcessor processor;
    processor.setPlayConfigDetails(2, 2, 44100, 512);
    processor.prepareToPlay(44100, 512);
    PerceptionPresetManager manager(processor.parameters);
    const bool verbose = argc > 1 && std::string(argv[1]) == "-v";

    // Word pairs that must never appear together in one description
    const std::vector<std::pair<std::string, std::string>> clash={
        {"collapsed to mono","wider than"},{"collapsed to mono","very wide"},{"narrower than","wider than"},{"narrower than","very wide"},
        {"much narrower","wider than"},{"much narrower","very wide"},{"eased back","lifted"},{"darker overall","brighter overall"},
        {"close and dry","spacious"},{"tight room","spacious reverb"},{"tight room","close and dry"},{"close and dry","long, cascading"},
        {"smooth","edgy"},{"smooth","buzzy"},{"smooth","gritty"},{"rounding","buzzy"},
        // overall feel against the facts
        {"close, intimate","wider than"},{"close, intimate","very wide"},{"close, intimate","spacious"},{"close, intimate","swirling"},
        {"vast, immersive","narrower than"},{"vast, immersive","collapsed to mono"},{"vast, immersive","close and dry"},
        {"open and airy","narrower than"},{"open and airy","collapsed to mono"},{"open and airy","close and dry"},
        {"spacious and atmospheric","close and dry"},{"warm and mellow","brighter overall"},{"warm and mellow","lifted"},
        {"tense and aggressive","smooth"},{"tense and aggressive","rounding"},{"gritty and synthetic","smooth"},{"gritty and synthetic","tape"},
        {"restless","collapsed to mono"}};

    const auto describe = [&](const std::string& context)
    {
        const auto tags = manager.generateFeelingTags();
        const auto text = manager.generateBreakdown().toStdString();
        const auto lowered = lower(text);

        check(!tags.isEmpty(), context + ": no chips at all");
        check(text.size() > 10, context + ": empty description");
        check(!text.empty() && text.back() == '.', context + ": description does not end in a full stop");
        check(text.find("  ") == std::string::npos, context + ": double space in description");

        for (int i = 0; i < tags.size(); ++i)
            for (int j = i + 1; j < tags.size(); ++j)
                check(tags[i] != tags[j], context + ": duplicate chip " + tags[i].toStdString());

        for (const auto& pair : clash)
            check(lowered.find(pair.first) == std::string::npos || lowered.find(pair.second) == std::string::npos,
                  context + ": '" + pair.first + "' contradicts '" + pair.second + "' in: " + text);

        if (verbose)
            std::printf("%-18s [%s]\n   %s\n", context.c_str(), tags.joinIntoString(" | ").toRawUTF8(), text.c_str());
    };

    for (const auto& preset : getFactoryPresets())
    {
        manager.applyPreset(preset.name);
        describe(preset.name.toStdString());
    }

    std::vector<juce::RangedAudioParameter*> params;
    for (auto* p : processor.getParameters())
        if (auto* ranged = dynamic_cast<juce::RangedAudioParameter*>(p))
            params.push_back(ranged);

    std::mt19937 rng(21);
    std::uniform_real_distribution<float> uniform(0.0f, 1.0f);
    const int randomRuns = 20000;
    for (int run = 0; run < randomRuns; ++run)
    {
        for (auto* p : params)
        {
            float v = uniform(rng);
            // Mix knobs sit at zero a third of the time, as they do in real patches
            if (uniform(rng) < 0.35f && (p->paramID == "modMix" || p->paramID == "mix" || p->paramID == "exciterMix"
                                         || p->paramID == "wet" || p->paramID == "sfxWetDryMix"))
                v = 0.0f;
            p->setValueNotifyingHost(v);
        }
        if (failures < 20)   // keep the report readable if something is badly broken
            describe("random #" + std::to_string(run));
    }

    std::printf("%d presets and %d random settings checked, %d problems found\n",
                static_cast<int>(getFactoryPresets().size()), randomRuns, failures);
    return failures == 0 ? 0 : 1;
}
