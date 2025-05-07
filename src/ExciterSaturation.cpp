#include "ExciterSaturation.h"

ExciterSaturation::ExciterSaturation() {}

void ExciterSaturation::prepare(const juce::dsp::ProcessSpec& spec)
{
    highpass.state = juce::dsp::IIR::Coefficients<float>::makeHighPass(spec.sampleRate, highpassFreq);
    highpass.prepare(spec);

    dryBuffer.setSize((int)spec.numChannels, (int)spec.maximumBlockSize);
}

void ExciterSaturation::reset()
{
    highpass.reset();
}

void ExciterSaturation::setDrive(float newDrive)
{
    drive = juce::jlimit(0.0f, 1.0f, newDrive);
}

void ExciterSaturation::setMix(float newMix)
{
    mix = juce::jlimit(0.0f, 1.0f, newMix);
}

void ExciterSaturation::setHighpass(float freqHz)
{
    highpassFreq = freqHz;
    *highpass.state = *juce::dsp::IIR::Coefficients<float>::makeHighPass(44100.0, highpassFreq); // TODO: update with actual sample rate if dynamic
}

float ExciterSaturation::waveshape(float x)
{
    // Soft saturation curve
    return std::tanh(drive * x);
}

void ExciterSaturation::process(juce::dsp::AudioBlock<float>& block)
{
	// Check if the block is empty
	if (block.getNumSamples() == 0)
		return;
	// Copy the input buffer to the dry buffer for later wet/dry mixing
	auto numSamples = (int)block.getNumSamples();
	auto numChannels = (int)block.getNumChannels();
	dryBuffer.clear();
	for (int ch = 0; ch < numChannels; ++ch)
		dryBuffer.copyFrom(ch, 0, block.getChannelPointer(ch), numSamples);
	juce::dsp::AudioBlock<float> dryBlock(dryBuffer);
	// Apply highpass before waveshaping
	juce::dsp::AudioBlock<float> hpBlock = block;
	highpass.process(juce::dsp::ProcessContextReplacing<float>(hpBlock));
	// Apply waveshaping
	for (int ch = 0; ch < numChannels; ++ch)
	{
		float* samples = block.getChannelPointer(ch);
		for (int i = 0; i < numSamples; ++i)
		{
			samples[i] = waveshape(samples[i]);
		}
	}
	// Blend dry + wet
	for (int ch = 0; ch < numChannels; ++ch)
	{
		float* wet = block.getChannelPointer(ch);
		float* dry = dryBlock.getChannelPointer(ch);
		for (int i = 0; i < numSamples; ++i)
		{
			wet[i] = mix * wet[i] + (1.0f - mix) * dry[i];
		}
	}
}
