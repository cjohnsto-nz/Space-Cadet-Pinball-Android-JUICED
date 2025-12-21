#pragma once


class Sound
{
public:
	static bool Init(int channels, bool enableFlag, int volume);
	static void Enable(bool enableFlag);
	static void Activate();
	static void Deactivate();
	static void Close();
	static void PlaySound(Mix_Chunk* wavePtr, int time);
	static Mix_Chunk* LoadWaveFile(const std::string& lpName);
	static void FreeSound(Mix_Chunk* wave);
	static void SetChannels(int channels);
	static void SetVolume(int volume);
	static void SetLimiterEnabled(bool enabled);
private:
	static int num_channels;
	static bool enabled_flag;
	static int* TimeStamps;
	static int Volume;
	
	// Audio limiter state
	static bool limiter_enabled;
	static float limiter_envelope;
	static constexpr float LIMITER_THRESHOLD = 0.9f;  // Start limiting at 90% of max
	static constexpr float LIMITER_ATTACK = 0.001f;   // Fast attack (1ms)
	static constexpr float LIMITER_RELEASE = 0.05f;   // Slower release (50ms)
	
	// Pre-limiter attenuation (0.0-1.0) to give headroom when multiple sounds play
	static constexpr float PRE_LIMITER_GAIN = 0.5f;  // -6dB reduction
	
	static void AudioPostMix(void* udata, Uint8* stream, int len);
};
