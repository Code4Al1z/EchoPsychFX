#include "SpatialFX.h"

SpatialFX::SpatialFX()
    : random(juce::Random(juce::Time::currentTimeMillis()))
{
}

void SpatialFX::prepare(const juce::dsp::ProcessSpec& spec)
{
    sampleRate = static_cast<float>(spec.sampleRate);

    // Initialize smoothed parameters
    constexpr double smoothTime = 0.02;
    params.phaseL.reset(sampleRate, smoothTime);
    params.phaseR.reset(sampleRate, smoothTime);
    params.wetDry.reset(sampleRate, 0.01);
    params.lfoDepthL.reset(sampleRate, smoothTime);
    params.lfoDepthR.reset(sampleRate, smoothTime);
    params.lfoRateL.reset(sampleRate, smoothTime);
    params.lfoRateR.reset(sampleRate, smoothTime);
    params.allpassFreq.reset(sampleRate, smoothTime);
    params.haasDelayL.reset(sampleRate, 0.01);
    params.haasDelayR.reset(sampleRate, 0.01);

    allpassL.prepare(spec);
    allpassR.prepare(spec);
    haasDelayL.prepare(spec);
    haasDelayR.prepare(spec);

    initializeDCBlockers();
    dcBlockerL.prepare(spec);
    dcBlockerR.prepare(spec);

    reset();
}

void SpatialFX::reset()
{
    allpassL.reset();
    allpassR.reset();
    haasDelayL.reset();
    haasDelayR.reset();
    dcBlockerL.reset();
    dcBlockerR.reset();

    lfoPhaseL = 0.0f;
    lfoPhaseR = lfoPhaseOffset;
    randomValueL = randomValueR = 0.0f;
    randomSampleCounterL = randomSampleCounterR = 0.0f;
    lastLfoValueL = lastLfoValueR = 0.0f;

    needsFilterUpdate = true;
    lastAllpassFreq = -1.0f;
}

void SpatialFX::setPhaseAmount(float leftPhase, float rightPhase)
{
    params.phaseL.setTargetValue(juce::jlimit(-juce::MathConstants<float>::pi,
        juce::MathConstants<float>::pi, leftPhase));
    params.phaseR.setTargetValue(juce::jlimit(-juce::MathConstants<float>::pi,
        juce::MathConstants<float>::pi, rightPhase));
}

void SpatialFX::setLfoDepth(float depthL, float depthR)
{
    params.lfoDepthL.setTargetValue(juce::jlimit(0.0f, juce::MathConstants<float>::pi, depthL));
    params.lfoDepthR.setTargetValue(juce::jlimit(0.0f, juce::MathConstants<float>::pi, depthR));
}

void SpatialFX::setLfoRate(float rateL, float rateR)
{
    params.lfoRateL.setTargetValue(juce::jlimit(0.0f, 20.0f, rateL));
    params.lfoRateR.setTargetValue(juce::jlimit(0.0f, 20.0f, rateR));
}

void SpatialFX::setLfoWaveform(LfoWaveform wf)
{
    if (isValidWaveform(wf))
    {
        waveform = wf;
        if (wf == LfoWaveform::Random)
        {
            randomSampleCounterL = randomSampleCounterR = 0.0f;
        }
    }
}

void SpatialFX::setLfoPhaseOffset(float offset)
{
    lfoPhaseOffset = std::fmod(offset, juce::MathConstants<float>::twoPi);
    if (lfoPhaseOffset < 0.0f)
        lfoPhaseOffset += juce::MathConstants<float>::twoPi;
}

void SpatialFX::setRandomUpdateRate(float hz)
{
    randomUpdateRateHz = juce::jlimit(1.0f, 50.0f, hz);
}

void SpatialFX::setWetDry(float newWetDry)
{
    params.wetDry.setTargetValue(juce::jlimit(0.0f, 1.0f, newWetDry));
}

void SpatialFX::setAllpassFrequency(float frequency)
{
    float clampedFreq = juce::jlimit(20.0f, sampleRate * 0.45f, frequency);
    params.allpassFreq.setTargetValue(clampedFreq);
    needsFilterUpdate = true;
}

void SpatialFX::setHaasDelayMs(float leftMs, float rightMs)
{
    params.haasDelayL.setTargetValue(juce::jlimit(0.0f, 30.0f, leftMs));
    params.haasDelayR.setTargetValue(juce::jlimit(0.0f, 30.0f, rightMs));
}

void SpatialFX::initializeDCBlockers()
{
    auto dcCoeffs = juce::dsp::IIR::Coefficients<float>::makeHighPass(sampleRate, 20.0f);
    *dcBlockerL.coefficients = *dcCoeffs;
    *dcBlockerR.coefficients = *dcCoeffs;
}

void SpatialFX::updateFilters()
{
    const float freq = params.allpassFreq.getCurrentValue();

    if (std::abs(freq - lastAllpassFreq) < filterUpdateThreshold && !needsFilterUpdate)
        return;

    lastAllpassFreq = freq;
    needsFilterUpdate = false;

    auto coefs = juce::dsp::IIR::Coefficients<float>::makeAllPass(sampleRate, freq);
    *allpassL.coefficients = *coefs;
    *allpassR.coefficients = *coefs;
}

void SpatialFX::updateRandomLfo(bool isLeftChannel, float& counter, float& value)
{
    const float samplesPerUpdate = sampleRate / randomUpdateRateHz;

    if (counter <= 0.0f)
    {
        value = random.nextFloat() * 2.0f - 1.0f;
        counter = samplesPerUpdate;
    }
    counter -= 1.0f;
}

float SpatialFX::calculateTriangleWave(float phase) const
{
    const float normalizedPhase = phase / juce::MathConstants<float>::twoPi;

    if (normalizedPhase < 0.25f)
        return 4.0f * normalizedPhase;
    else if (normalizedPhase < 0.75f)
        return 2.0f - 4.0f * normalizedPhase;
    else
        return -4.0f + 4.0f * normalizedPhase;
}

float SpatialFX::getLfoValue(float phase, bool isLeftChannel)
{
    switch (waveform)
    {
    case LfoWaveform::Sine:
        return std::sin(phase);

    case LfoWaveform::Triangle:
        return calculateTriangleWave(phase);

    case LfoWaveform::Square:
        return phase < juce::MathConstants<float>::pi ? 1.0f : -1.0f;

    case LfoWaveform::Random:
        if (isLeftChannel)
        {
            updateRandomLfo(true, randomSampleCounterL, randomValueL);
            return randomValueL;
        }
        else
        {
            updateRandomLfo(false, randomSampleCounterR, randomValueR);
            return randomValueR;
        }

    default:
        return 0.0f;
    }
}

void SpatialFX::process(juce::dsp::AudioBlock<float>& block)
{
    if (block.getNumChannels() < 2)
        return;

    // Early exit if completely dry and stable
    if (params.wetDry.getTargetValue() <= 0.0f && !params.wetDry.isSmoothing())
        return;

    auto* leftData = block.getChannelPointer(0);
    auto* rightData = block.getChannelPointer(1);
    const size_t numSamples = block.getNumSamples();

    const float invSampleRate = 1.0f / sampleRate;
    const float twoPi = juce::MathConstants<float>::twoPi;

    for (size_t i = 0; i < numSamples; ++i)
    {
        const float dryL = leftData[i];
        const float dryR = rightData[i];

        // Get smoothed parameters
        const float smoothedPhaseL = params.phaseL.getNextValue();
        const float smoothedPhaseR = params.phaseR.getNextValue();
        const float smoothedDepthL = params.lfoDepthL.getNextValue();
        const float smoothedDepthR = params.lfoDepthR.getNextValue();
        const float smoothedWetDry = params.wetDry.getNextValue();
        const float haasTimeL = params.haasDelayL.getNextValue();
        const float haasTimeR = params.haasDelayR.getNextValue();
        const float rateL = params.lfoRateL.getNextValue();
        const float rateR = params.lfoRateR.getNextValue();

        // Update filters only when needed
        params.allpassFreq.getNextValue(); // Consume the value
        if (needsFilterUpdate)
            updateFilters();

        // Update LFO phases
        lfoPhaseL += twoPi * rateL * invSampleRate;
        lfoPhaseR += twoPi * rateR * invSampleRate;

        if (lfoPhaseL >= twoPi) lfoPhaseL -= twoPi;
        if (lfoPhaseR >= twoPi) lfoPhaseR -= twoPi;

        // Calculate LFO modulation
        const float lfoModL = getLfoValue(lfoPhaseL, true);
        const float lfoModR = getLfoValue(lfoPhaseR, false);

        lastLfoValueL = lfoModL;
        lastLfoValueR = lfoModR;

        // Apply modulation to phase
        const float phaseL = smoothedPhaseL + smoothedDepthL * lfoModL;
        const float phaseR = smoothedPhaseR + smoothedDepthR * lfoModR;

        // Phase rotation
        const float cosL = std::cos(phaseL);
        const float sinL = std::sin(phaseL);
        const float cosR = std::cos(phaseR);
        const float sinR = std::sin(phaseR);

        const float shiftedL = dryL * cosL - dryR * sinL;
        const float shiftedR = dryR * cosR + dryL * sinR;

        // Haas effect
        haasDelayL.setDelay(haasTimeL * sampleRate * 0.001f);
        haasDelayR.setDelay(haasTimeR * sampleRate * 0.001f);

        const float delayedL = haasDelayL.popSample(0);
        const float delayedR = haasDelayR.popSample(0);
        haasDelayL.pushSample(0, shiftedL);
        haasDelayR.pushSample(0, shiftedR);

        // Allpass filtering
        float filteredL = allpassL.processSample(delayedL);
        float filteredR = allpassR.processSample(delayedR);

        // DC blocking
        filteredL = dcBlockerL.processSample(filteredL);
        filteredR = dcBlockerR.processSample(filteredR);

        // Equal-power crossfade
        const float wetGain = std::sqrt(smoothedWetDry);
        const float dryGain = std::sqrt(1.0f - smoothedWetDry);

        leftData[i] = dryL * dryGain + filteredL * wetGain;
        rightData[i] = dryR * dryGain + filteredR * wetGain;
    }
}

bool SpatialFX::isValidWaveform(LfoWaveform wf) const
{
    int wfValue = static_cast<int>(wf);
    return wfValue >= static_cast<int>(LfoWaveform::Sine) &&
        wfValue <= static_cast<int>(LfoWaveform::Random);
}