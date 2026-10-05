#pragma once
#include <juce_dsp/juce_dsp.h>
#include <array>

/** Writes a biquad's coefficients into an existing IIR::Coefficients object without allocating.

    juce::dsp::IIR::Coefficients<float>::makeLowShelf() and friends create a new reference-counted
    object, which is a heap allocation - not something to do on the audio thread. The
    ArrayCoefficients versions return a plain std::array, which this copies in (normalised by a0,
    exactly as the Coefficients constructor does). */
inline void setCoefficientsInPlace(juce::dsp::IIR::Coefficients<float>& destination,
    const std::array<float, 6>& c)
{
    if (destination.coefficients.size() == 5)
    {
        const float a0Inverse = 1.0f / c[3];
        float* d = destination.getRawCoefficients();
        d[0] = c[0] * a0Inverse;
        d[1] = c[1] * a0Inverse;
        d[2] = c[2] * a0Inverse;
        d[3] = c[4] * a0Inverse;
        d[4] = c[5] * a0Inverse;
    }
    else
    {
        // Not a second-order filter yet (only possible before the first update) - build it once
        destination = juce::dsp::IIR::Coefficients<float>(c[0], c[1], c[2], c[3], c[4], c[5]);
    }
}
