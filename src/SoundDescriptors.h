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
    same part of the sound. Empty means "nothing strongly coloured". */
std::vector<SoundDescriptor> describeSound(const SoundCharacter& character);
