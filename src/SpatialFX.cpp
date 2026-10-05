#include "SpatialFX.h"
#include "FilterUtils.h"

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

    const int maxHaasDelaySamples = static_cast<int>(std::ceil(0.040 * spec.sampleRate)) + 1;   // 40 ms, the Haas knob's maximum
    haasDelayL.setMaximumDelayInSamples(maxHaasDelaySamples);
    haasDelayR.setMaximumDelayInSamples(maxHaasDelaySamples);
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
    lfoPhaseR = 0.0f;   // the L/R LFO phase offset is applied when the LFO is read, so it responds live
    randomL = RandomLfoState{};
    randomR = RandomLfoState{};
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
    // The processor sends the current choice every audio buffer, so only react to a real change.
    // (Restarting the random generator on every call made it jump to a new value every buffer.)
    if (!isValidWaveform(wf) || wf == waveform)
        return;

    waveform = wf;

    if (wf == LfoWaveform::Random)
    {
        randomL = RandomLfoState{};
        randomR = RandomLfoState{};
    }
}

void SpatialFX::setLfoPhaseOffset(float offset)
{
    lfoPhaseOffset = std::fmod(offset, juce::MathConstants<float>::twoPi);
    if (lfoPhaseOffset < 0.0f)
        lfoPhaseOffset += juce::MathConstants<float>::twoPi;
}

void SpatialFX::setWetDry(float newWetDry)
{
    params.wetDry.setTargetValue(juce::jlimit(0.0f, 1.0f, newWetDry));
}

void SpatialFX::setAllpassFrequency(float frequency)
{
    float clampedFreq = juce::jlimit(20.0f, sampleRate * 0.45f, frequency);

    // Sent every audio block by the processor; only react to a real change
    if (clampedFreq == params.allpassFreq.getTargetValue())
        return;

    params.allpassFreq.setTargetValue(clampedFreq);
    needsFilterUpdate = true;
}

void SpatialFX::setHaasDelayMs(float leftMs, float rightMs)
{
    params.haasDelayL.setTargetValue(juce::jlimit(0.0f, 40.0f, leftMs));
    params.haasDelayR.setTargetValue(juce::jlimit(0.0f, 40.0f, rightMs));
}

void SpatialFX::initializeDCBlockers()
{
    auto dcCoeffs = juce::dsp::IIR::Coefficients<float>::makeHighPass(sampleRate, 20.0f);
    *dcBlockerL.coefficients = *dcCoeffs;
    *dcBlockerR.coefficients = *dcCoeffs;
}

void SpatialFX::updateFilters()
{
    // The smoother starts at 0 Hz on the first block; an allpass at 0 Hz has infinite coefficients and
    // would poison the filter with NaN, so never go below the control's 20 Hz minimum.
    const float freq = juce::jmax(20.0f, params.allpassFreq.getCurrentValue());

    if (std::abs(freq - lastAllpassFreq) < filterUpdateThreshold && !needsFilterUpdate)
        return;

    lastAllpassFreq = freq;
    needsFilterUpdate = false;

    // Straight into the filters' existing coefficients (no allocation on the audio thread)
    const auto coefs = juce::dsp::IIR::ArrayCoefficients<float>::makeAllPass(sampleRate, freq);
    setCoefficientsInPlace(*allpassL.coefficients, coefs);
    setCoefficientsInPlace(*allpassR.coefficients, coefs);
}

float SpatialFX::nextRandomLfoValue(RandomLfoState& s, float phase, float rateHz)
{
    // Time to glide to each new value: quick enough to keep the stepped sample-and-hold character,
    // slow enough that the rotation it drives never clicks (and never longer than half a cycle).
    const float cycleSeconds = 1.0f / juce::jmax(rateHz, 0.01f);
    const int glideSamples = juce::jmax(1, static_cast<int>(sampleRate * juce::jmin(0.008f, 0.5f * cycleSeconds)));

    if (!s.initialised)
    {
        s.current = s.target = random.nextFloat() * 2.0f - 1.0f;
        s.initialised = true;
    }
    else if (phase < s.lastPhase)   // the phase wrapped round: a new cycle begins
    {
        s.target = random.nextFloat() * 2.0f - 1.0f;
        s.glideSamplesLeft = glideSamples;
        s.step = (s.target - s.current) / static_cast<float>(glideSamples);
    }
    s.lastPhase = phase;

    if (s.glideSamplesLeft > 0)
    {
        s.current += s.step;
        if (--s.glideSamplesLeft == 0)
            s.current = s.target;
    }

    return s.current;
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
        return isLeftChannel ? nextRandomLfoValue(randomL, phase, currentRateL)
                             : nextRandomLfoValue(randomR, phase, currentRateR);

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

    // The allpass frequency is smoothed, so the filters follow it once per block while it moves
    // (and once more at the end of the block, so they land exactly on the target). When it is
    // still nothing is recomputed at all.
    const bool allpassMoving = needsFilterUpdate || params.allpassFreq.isSmoothing();
    if (allpassMoving)
    {
        needsFilterUpdate = true;
        updateFilters();
    }

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

        // Update LFO phases
        lfoPhaseL += twoPi * rateL * invSampleRate;
        lfoPhaseR += twoPi * rateR * invSampleRate;

        if (lfoPhaseL >= twoPi) lfoPhaseL -= twoPi;
        if (lfoPhaseR >= twoPi) lfoPhaseR -= twoPi;

        // Calculate LFO modulation
        // The right LFO is read lfoPhaseOffset radians ahead of the left one. Applying it here rather
        // than once at reset() makes the LFO Phase control audible and live.
        float shiftedPhaseR = lfoPhaseR + lfoPhaseOffset;
        if (shiftedPhaseR >= twoPi) shiftedPhaseR -= twoPi;

        currentRateL = rateL;
        currentRateR = rateR;

        const float lfoModL = getLfoValue(lfoPhaseL, true);
        const float lfoModR = getLfoValue(shiftedPhaseR, false);

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

    if (allpassMoving)
    {
        needsFilterUpdate = true;
        updateFilters();
        needsFilterUpdate = params.allpassFreq.isSmoothing();   // keep following until the ramp ends
    }
}

bool SpatialFX::isValidWaveform(LfoWaveform wf) const
{
    int wfValue = static_cast<int>(wf);
    return wfValue >= static_cast<int>(LfoWaveform::Sine) &&
        wfValue <= static_cast<int>(LfoWaveform::Random);
}