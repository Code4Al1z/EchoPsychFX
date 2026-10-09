#pragma once

#include "PerceptionProfile.h"
#include <string>
#include <vector>

/** One thing worth saying about a sound: a short chip label, the phrase used for it in the written
    description, and which plugin section it is about. */
struct SoundDescriptor
{
    std::string tag;
    std::string clause;
    Section section;
};

/** Turns a SoundCharacter into the feeling chips and description phrases, in the order they are shown.

    Every rule reads the character (the values the ear actually gets), never the raw knobs, and every
    group of related rules is mutually exclusive, so the result cannot say two opposite things about the
    same part of the sound. Where a claim is about the whole sound (how roomy it is), the rule also consults
    the perception profile, so a chip can never disagree with the bars shown beside it.
    Empty means "nothing strongly coloured". */
std::vector<SoundDescriptor> describeSound(const SoundCharacter& character, const PerceptionProfile& profile);

/** The one overall feel of a sound, as a short phrase, or an empty string when it is plain.

    Feelings are deliberately NOT attached to the individual descriptors above (a "warm" chip and a "cold" chip
    can then never meet in one sentence). Instead exactly one feel is chosen here, from the whole picture, by a
    fixed priority list, so a description can only ever have one mood. */
std::string describeOverallFeel(const SoundCharacter& character, const PerceptionProfile& profile);
