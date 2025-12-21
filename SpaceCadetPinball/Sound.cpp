#include "pch.h"
#include "Sound.h"
#include <cmath>
#include <algorithm>

int Sound::num_channels;
bool Sound::enabled_flag = false;
int* Sound::TimeStamps = nullptr;
int Sound::Volume = MIX_MAX_VOLUME;
bool Sound::limiter_enabled = true;
float Sound::limiter_envelope = 1.0f;

bool Sound::Init(int channels, bool enableFlag, int volume)
{
	Volume = volume;
	Mix_Init(MIX_INIT_MID_Proxy);
	auto result = Mix_OpenAudio(MIX_DEFAULT_FREQUENCY, MIX_DEFAULT_FORMAT, 2, 1024);
	
	// Register post-mix callback for audio limiting
	Mix_SetPostMix(AudioPostMix, nullptr);
	limiter_envelope = 1.0f;
	
	SetChannels(channels);
	Enable(enableFlag);
	return !result;
}

void Sound::Enable(bool enableFlag)
{
	enabled_flag = enableFlag;
	if (!enableFlag)
		Mix_HaltChannel(-1);
}

void Sound::Activate()
{
	Mix_Resume(-1);
}

void Sound::Deactivate()
{
	Mix_Pause(-1);
}

void Sound::Close()
{
	delete[] TimeStamps;
	TimeStamps = nullptr;
	Mix_CloseAudio();
	Mix_Quit();
}

void Sound::PlaySound(Mix_Chunk* wavePtr, int time)
{
	if (wavePtr && enabled_flag)
	{
		if (Mix_Playing(-1) == num_channels)
		{
			auto oldestChannel = std::min_element(TimeStamps, TimeStamps + num_channels) - TimeStamps;
			Mix_HaltChannel(oldestChannel);
		}

		// Apply pre-limiter gain reduction to give headroom
		int reducedVolume = static_cast<int>(Volume * PRE_LIMITER_GAIN);
		Mix_VolumeChunk(wavePtr, reducedVolume);
		
		auto channel = Mix_PlayChannel(-1, wavePtr, 0);
		if (channel != -1)
			TimeStamps[channel] = time;
	}
}

Mix_Chunk* Sound::LoadWaveFile(const std::string& lpName)
{
	auto wavFile = fopen(lpName.c_str(), "r");
	if (!wavFile)
		return nullptr;
	fclose(wavFile);

	return Mix_LoadWAV(lpName.c_str());
}

void Sound::FreeSound(Mix_Chunk* wave)
{
	if (wave)
		Mix_FreeChunk(wave);
}

void Sound::SetChannels(int channels)
{
	if (channels <= 0)
		channels = 8;

	num_channels = channels;
	delete[] TimeStamps;
	TimeStamps = new int[num_channels]();
	Mix_AllocateChannels(num_channels);
	SetVolume(Volume);
}

void Sound::SetVolume(int volume)
{
	Volume = volume;
	Mix_Volume(-1, volume);
}

void Sound::SetLimiterEnabled(bool enabled)
{
	limiter_enabled = enabled;
	if (enabled)
		limiter_envelope = 1.0f;
}

void Sound::AudioPostMix(void* udata, Uint8* stream, int len)
{
	if (!limiter_enabled)
		return;
	
	// Process as 16-bit signed samples (SDL default format)
	Sint16* samples = reinterpret_cast<Sint16*>(stream);
	int numSamples = len / sizeof(Sint16);
	
	// Get audio spec for sample rate
	int frequency = MIX_DEFAULT_FREQUENCY;
	
	// Calculate time constants based on sample rate
	float attackCoef = 1.0f - std::exp(-1.0f / (LIMITER_ATTACK * frequency));
	float releaseCoef = 1.0f - std::exp(-1.0f / (LIMITER_RELEASE * frequency));
	
	const float maxSample = 32767.0f;
	const float threshold = LIMITER_THRESHOLD * maxSample;
	
	for (int i = 0; i < numSamples; i++)
	{
		float sample = static_cast<float>(samples[i]);
		float absSample = std::fabs(sample);
		
		// Calculate target gain
		float targetGain = 1.0f;
		if (absSample > threshold)
		{
			// Soft knee compression above threshold
			targetGain = threshold / absSample;
		}
		
		// Envelope follower with fast attack, slow release
		if (targetGain < limiter_envelope)
		{
			// Attack - reduce gain quickly
			limiter_envelope += attackCoef * (targetGain - limiter_envelope);
		}
		else
		{
			// Release - restore gain slowly
			limiter_envelope += releaseCoef * (targetGain - limiter_envelope);
		}
		
		// Apply gain and clamp
		sample *= limiter_envelope;
		sample = std::max(-maxSample, std::min(maxSample, sample));
		
		samples[i] = static_cast<Sint16>(sample);
	}
}
