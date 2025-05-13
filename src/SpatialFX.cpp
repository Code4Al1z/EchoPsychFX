#include "SpatialFX.h"

#include <random>

SpatialFX::SpatialFX() {}

void SpatialFX::prepare(const juce::dsp::ProcessSpec& spec)
{
    sampleRate = static_cast<float>(spec.sampleRate);

    targetPhaseL.reset(sampleRate, 0.02);
    targetPhaseR.reset(sampleRate, 0.02);
    wetDry.reset(sampleRate, 0.01);
    lfoDepthL.reset(sampleRate, 0.02);
    lfoDepthR.reset(sampleRate, 0.02);
    lfoRateL.reset(sampleRate, 0.02);
    lfoRateR.reset(sampleRate, 0.02);
    allpassFrequency.reset(sampleRate, 0.02);
    haasMsL.reset(sampleRate, 0.01);
    haasMsR.reset(sampleRate, 0.01);

    updateFilters();

    allpassL.reset();
    allpassR.reset();
    haasDelayL.reset();
    haasDelayR.reset();

    haasDelayL.prepare(spec);
    haasDelayR.prepare(spec);
}

void SpatialFX::reset()
{
    allpassL.reset();
    allpassR.reset();
    haasDelayL.reset();
    haasDelayR.reset();
    lfoPhaseL = 0.0f;
    lfoPhaseR = lfoPhaseOffset;
}

void SpatialFX::setPhaseAmount(float leftPhase, float rightPhase)
{
    targetPhaseL.setTargetValue(leftPhase);
    targetPhaseR.setTargetValue(rightPhase);
}

void SpatialFX::setLfoDepth(float depthL_, float depthR_)
{
    lfoDepthL.setTargetValue(depthL_);
    lfoDepthR.setTargetValue(depthR_);
}

void SpatialFX::setLfoRate(float rateL_, float rateR_)
{
    lfoRateL.setTargetValue(rateL_);
    lfoRateR.setTargetValue(rateR_);
}

void SpatialFX::setLfoWaveform(LfoWaveform wf)
{
    waveform = wf;
}

void SpatialFX::setLfoPhaseOffset(float offset)
{
    lfoPhaseOffset = offset;
    lfoPhaseR = lfoPhaseOffset;
}

void SpatialFX::setWetDry(float newWetDry)
{
    wetDry.setTargetValue(juce::jlimit(0.0f, 1.0f, newWetDry));
}

void SpatialFX::setAllpassFrequency(float frequency)
{
    allpassFrequency.setTargetValue(frequency);
    updateFilters();
}

void SpatialFX::setHaasDelayMs(float leftMs, float rightMs)
{
    haasMsL.setTargetValue(juce::jlimit(0.0f, 30.0f, leftMs));
    haasMsR.setTargetValue(juce::jlimit(0.0f, 30.0f, rightMs));
}

void SpatialFX::updateFilters()
{
    auto omega = juce::MathConstants<float>::twoPi * allpassFrequency.getNextValue() / sampleRate;
    float cosw = std::cos(omega);
    float alpha = std::sin(omega);

    juce::dsp::IIR::Coefficients<float>::Ptr coefs = new juce::dsp::IIR::Coefficients<float>(
        (1.0f - alpha) / 2.0f,
        0.0f,
        (alpha - 1.0f) / 2.0f,
        1.0f,
        0.0f,
        -1.0f
    );

    allpassL.coefficients = coefs;
    allpassR.coefficients = coefs;
}

void SpatialFX::updateLfo()
{
    float lfoIncL = juce::MathConstants<float>::twoPi * lfoRateL.getNextValue() / sampleRate;
    float lfoIncR = juce::MathConstants<float>::twoPi * lfoRateR.getNextValue() / sampleRate;

    lfoPhaseL += lfoIncL;
    lfoPhaseR += lfoIncR;

    if (lfoPhaseL > juce::MathConstants<float>::twoPi) lfoPhaseL -= juce::MathConstants<float>::twoPi;
    if (lfoPhaseR > juce::MathConstants<float>::twoPi) lfoPhaseR -= juce::MathConstants<float>::twoPi;
}

float SpatialFX::getLfoValue(float phase)
{
    switch (waveform)
    {
    case LfoWaveform::Sine:     return std::sin(phase);
    case LfoWaveform::Triangle: return 2.0f * std::abs(2.0f * (phase / juce::MathConstants<float>::twoPi) - 1.0f) - 1.0f;
    case LfoWaveform::Square:   return std::sin(phase) >= 0.0f ? 1.0f : -1.0f;
    case LfoWaveform::Random:   return juce::Random::getSystemRandom().nextFloat() * 2.0f - 1.0f;
    default: return 0.0f;
    }
}

void SpatialFX::process(juce::dsp::AudioBlock<float>& block)
{
    updateLfo();

    auto* leftData = block.getChannelPointer(0);
    auto* rightData = block.getNumChannels() > 1 ? block.getChannelPointer(1) : leftData;

    for (size_t i = 0; i < block.getNumSamples(); ++i)
    {
        float dryL = leftData[i];
        float dryR = rightData[i];

        // Smooth values
        float smoothedPhaseL = targetPhaseL.getNextValue();
        float smoothedPhaseR = targetPhaseR.getNextValue();
        float smoothedDepthL = lfoDepthL.getNextValue();
        float smoothedDepthR = lfoDepthR.getNextValue();
        float smoothedWetDry = wetDry.getNextValue();

        float haasTimeL = haasMsL.getNextValue();
        float haasTimeR = haasMsR.getNextValue();
        haasDelayL.setDelay(haasTimeL * sampleRate * 0.001f);
        haasDelayR.setDelay(haasTimeR * sampleRate * 0.001f);

        // Modulated phase shift
        float phaseL = smoothedPhaseL + smoothedDepthL * std::sin(lfoPhaseL);
        float phaseR = smoothedPhaseR + smoothedDepthR * std::sin(lfoPhaseR);

        float shiftedL = dryL * std::cos(phaseL) - dryR * std::sin(phaseL);
        float shiftedR = dryR * std::cos(phaseR) + dryL * std::sin(phaseR);

        // Apply Haas delay
        float delayedL = haasDelayL.popSample(0);
        float delayedR = haasDelayR.popSample(0);
        haasDelayL.pushSample(0, shiftedL);
        haasDelayR.pushSample(0, shiftedR);

        // Allpass filter on delayed signal
        float wetL = allpassL.processSample(delayedL);
        float wetR = allpassR.processSample(delayedR);

        // Wet/dry mix
        leftData[i] = juce::jmap(smoothedWetDry, dryL, wetL);
        rightData[i] = juce::jmap(smoothedWetDry, dryR, wetR);
    }
}
