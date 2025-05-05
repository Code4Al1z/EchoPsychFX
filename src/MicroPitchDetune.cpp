#include "MicroPitchDetune.h"
#include "MicroPitchDetune.h"
#include "MicroPitchDetune.h"
#include "MicroPitchDetune.h"
#include <cmath>
#include <algorithm>

MicroPitchDetune::MicroPitchDetune()
    : randomEngine(std::random_device{}()),
    randomDistribution(0.0f, 1.0f)
{
}

void MicroPitchDetune::prepare(const juce::dsp::ProcessSpec& spec)
{
    sampleRate = spec.sampleRate;
    delayL.prepare(spec);
    delayR.prepare(spec);

    const int maxDelaySamples = static_cast<int>(sampleRate * maxDelayTime) + 1;
    delayL.setMaximumDelayInSamples(maxDelaySamples);
    delayR.setMaximumDelayInSamples(maxDelaySamples);

    delayL.setDelay(delayCentre * sampleRate);
    delayR.setDelay(delayCentre * sampleRate);

    lfoPhaseOffset = randomDistribution(randomEngine) * juce::MathConstants<float>::twoPi;
    modPhase = 0.0f;
}

void MicroPitchDetune::setParams(float detuneCentsIn, float lfoRateIn, float lfoDepthIn,
    float delayCentreIn, float stereoSeparationIn, float mixIn)
{
    detuneCents = detuneCentsIn;

    // Detune affects lfoDepth internally
    lfoDepth = juce::jlimit(0.0f, 0.01f, std::abs(detuneCentsIn) / 1000.0f * 0.005f);

    // Override with explicit lfoDepth if provided
    lfoDepth = juce::jlimit(0.0f, 0.01f, lfoDepthIn);

    lfoRate = juce::jlimit(0.01f, 20.0f, lfoRateIn);

    delayCentre = juce::jlimit(0.001f, maxDelayTime * 0.8f, delayCentreIn);
    if (sampleRate > 0.0f)
    {
        delayL.setDelay(delayCentre * sampleRate);
        delayR.setDelay(delayCentre * sampleRate);
    }

    stereoSeparation = juce::jlimit(0.0f, 1.0f, stereoSeparationIn);
    mix = juce::jlimit(0.0f, 1.0f, mixIn);
}

void MicroPitchDetune::setSyncEnabled(bool shouldSync)
{
    syncEnabled = shouldSync;
}

void MicroPitchDetune::setBpm(float newBpm)
{
    bpm = newBpm;
}

float MicroPitchDetune::lfo(float phase)
{
    return std::sin(juce::MathConstants<float>::twoPi * phase);
}

void MicroPitchDetune::process(juce::dsp::AudioBlock<float>& block)
{
    auto numSamples = block.getNumSamples();
    auto numChannels = block.getNumChannels();

    for (size_t i = 0; i < numSamples; ++i)
    {
        float lfoValueL = lfo(modPhase);
        float lfoValueR = lfo(modPhase + stereoSeparation * juce::MathConstants<float>::pi);

        float delayTimeL = delayCentre + lfoValueL * lfoDepth;
        float delayTimeR = delayCentre + lfoValueR * lfoDepth;

        delayL.setDelay(delayTimeL * sampleRate);
        delayR.setDelay(delayTimeR * sampleRate);

        for (size_t ch = 0; ch < numChannels; ++ch)
        {
            float inSample = block.getSample(ch, i);
            float wetSample = 0.0f;

            if (ch == 0)
            {
                wetSample = delayL.popSample(0);
                delayL.pushSample(0, inSample);
            }
            else if (ch == 1)
            {
                wetSample = delayR.popSample(0);
                delayR.pushSample(0, inSample);
            }
            else
            {
                // Fallback: dry signal if more than 2 channels
                wetSample = inSample;
            }

            float finalSample = juce::jmap(mix, inSample, wetSample);
            block.setSample(ch, i, finalSample);
        }

        float rate = syncEnabled ? (bpm / 60.0f / lfoRate) : lfoRate;
        modPhase += rate / sampleRate;

        if (modPhase >= 1.0f)
            modPhase -= 1.0f;
    }
}
