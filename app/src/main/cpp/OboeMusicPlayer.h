#pragma once

#include <oboe/Oboe.h>
#include <vector>
#include <string>
#include <atomic>
#include <mutex>
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>
#include <media/NdkMediaExtractor.h>
#include <media/NdkMediaCodec.h>
#include <media/NdkMediaFormat.h>

/**
 * Low-pass filter using a simple one-pole IIR filter
 * Creates a "muffled" effect by removing high frequencies
 * Supports smooth fade transitions between bypass and filtered states
 */
class LowPassFilter {
public:
    LowPassFilter() : mTargetCutoff(200.0f), mCurrentCutoff(20000.0f), mSampleRate(48000), 
                      mEnabled(false), mBypassCutoff(20000.0f), mFilteredCutoff(200.0f),
                      mFadeSpeed(0.0005f) {
        calculateCoefficients();
    }

    void setSampleRate(int32_t sampleRate) {
        mSampleRate = sampleRate;
        calculateCoefficients();
    }

    void setCutoffFrequency(float freq) {
        mFilteredCutoff = freq;
        if (mEnabled) {
            mTargetCutoff = freq;
        }
    }

    void setEnabled(bool enabled) {
        mEnabled = enabled;
        // Set target cutoff - will smoothly interpolate
        mTargetCutoff = enabled ? mFilteredCutoff : mBypassCutoff;
    }

    bool isEnabled() const { return mEnabled; }

    void process(float* buffer, int32_t numFrames, int32_t numChannels) {
        for (int32_t i = 0; i < numFrames; i++) {
            // Smoothly interpolate cutoff frequency toward target
            if (mCurrentCutoff != mTargetCutoff) {
                if (mCurrentCutoff < mTargetCutoff) {
                    mCurrentCutoff += (mTargetCutoff - mCurrentCutoff) * mFadeSpeed;
                    if (mCurrentCutoff > mTargetCutoff - 1.0f) mCurrentCutoff = mTargetCutoff;
                } else {
                    mCurrentCutoff -= (mCurrentCutoff - mTargetCutoff) * mFadeSpeed;
                    if (mCurrentCutoff < mTargetCutoff + 1.0f) mCurrentCutoff = mTargetCutoff;
                }
                calculateCoefficients();
            }
            
            // Skip processing if essentially bypassed (very high cutoff)
            if (mCurrentCutoff >= mBypassCutoff - 100.0f) continue;

            if (numChannels >= 1) {
                float inputL = buffer[i * numChannels];
                // Low-pass: output = alpha * input + (1 - alpha) * prevOutput
                float outputL = mAlpha * inputL + (1.0f - mAlpha) * mPrevOutputL;
                mPrevOutputL = outputL;
                buffer[i * numChannels] = outputL;
            }
            if (numChannels >= 2) {
                float inputR = buffer[i * numChannels + 1];
                float outputR = mAlpha * inputR + (1.0f - mAlpha) * mPrevOutputR;
                mPrevOutputR = outputR;
                buffer[i * numChannels + 1] = outputR;
            }
        }
    }

private:
    void calculateCoefficients() {
        // RC low-pass filter coefficient
        // alpha = dt / (rc + dt) where rc = 1 / (2 * pi * cutoff)
        float rc = 1.0f / (2.0f * M_PI * mCurrentCutoff);
        float dt = 1.0f / mSampleRate;
        mAlpha = dt / (rc + dt);
    }

    float mTargetCutoff;      // Target cutoff to fade toward
    float mCurrentCutoff;     // Current interpolated cutoff
    int32_t mSampleRate;
    bool mEnabled;
    float mAlpha;
    
    float mBypassCutoff;      // Cutoff when filter is "off" (20kHz = essentially bypass)
    float mFilteredCutoff;    // Cutoff when filter is active (200Hz = muffled bass)
    float mFadeSpeed;         // How fast to interpolate (0.0005 = smooth fade)

    // Filter state (stereo)
    float mPrevOutputL = 0.0f;
    float mPrevOutputR = 0.0f;
};

/**
 * Oboe-based music player with real-time audio effects
 */
class OboeMusicPlayer : public oboe::AudioStreamDataCallback {
public:
    OboeMusicPlayer();
    ~OboeMusicPlayer();

    // Load audio from file (WAV format for simplicity)
    bool loadFromFile(const std::string& filePath);
    
    // Load audio from file with PCM caching (fast on subsequent loads)
    bool loadFromFileWithCache(const std::string& filePath, const std::string& cacheDir);
    
    // Load audio from Android assets
    bool loadFromAssets(AAssetManager* assetManager, const std::string& assetPath);
    
    // Load audio from Android assets with PCM caching
    bool loadFromAssetsWithCache(AAssetManager* assetManager, const std::string& assetPath, const std::string& cacheDir);
    
    // Load compressed audio from assets (MP3, AAC, etc.)
    bool loadCompressedFromAssets(AAssetManager* assetManager, const std::string& assetPath, const std::string& cacheDir);
    
    // Load raw PCM data
    bool loadPCMData(const int16_t* data, size_t numSamples, int32_t sampleRate, int32_t channels);

    // Playback control
    bool start();
    void stop();
    void pause();
    void resume();
    bool isPlaying() const { return mIsPlaying; }

    // Volume control (0.0 to 1.0)
    void setVolume(float volume) { mVolume = volume; }
    float getVolume() const { return mVolume; }

    // Looping
    void setLooping(bool loop) { mLooping = loop; }
    bool isLooping() const { return mLooping; }

    // Mission track support (layered on top of main track)
    bool loadMissionTrackFromFile(const std::string& filePath);
    bool loadMissionTrackFromFileWithCache(const std::string& filePath, const std::string& cacheDir);
    bool loadMissionTrackFromAssets(AAssetManager* assetManager, const std::string& assetPath);
    bool loadMissionTrackFromAssetsWithCache(AAssetManager* assetManager, const std::string& assetPath, const std::string& cacheDir);
    bool loadMissionTrackCompressed(AAssetManager* assetManager, const std::string& assetPath, const std::string& cacheDir);
    void setMissionTrackEnabled(bool enabled);
    bool isMissionTrackEnabled() const { return mMissionEnabled; }
    void setMissionVolume(float volume) { mMissionTargetVolume = volume; }

    // Low-pass filter control (for muffled effect when ball is captured)
    void setLowPassEnabled(bool enabled) { mLowPassFilter.setEnabled(enabled); }
    bool isLowPassEnabled() const { return mLowPassFilter.isEnabled(); }
    void setLowPassCutoff(float freq) { mLowPassFilter.setCutoffFrequency(freq); }

    // Get current playback position in milliseconds
    int64_t getPositionMs() const;

    // Oboe callback
    oboe::DataCallbackResult onAudioReady(oboe::AudioStream* stream, void* audioData, int32_t numFrames) override;

private:
    std::shared_ptr<oboe::AudioStream> mStream;
    std::vector<float> mAudioData;  // Interleaved float samples
    std::atomic<size_t> mReadIndex{0};
    std::atomic<bool> mIsPlaying{false};
    std::atomic<float> mVolume{1.0f};
    std::atomic<bool> mLooping{true};
    
    int32_t mSampleRate = 48000;
    int32_t mChannels = 2;
    
    LowPassFilter mLowPassFilter;
    std::mutex mDataMutex;
    
    // Mission track (layered on top, same position as main track)
    std::vector<float> mMissionAudioData;
    std::atomic<bool> mMissionEnabled{false};
    std::atomic<float> mMissionTargetVolume{1.0f};
    float mMissionCurrentVolume = 0.0f;  // For smooth fade
    static constexpr float kMissionFadeSpeed = 0.000003f;  // Slow fade rate per sample (~5 seconds)
    int32_t mMissionSampleRate = 48000;
    int32_t mMissionChannels = 2;
};

// Global instance for JNI access
extern OboeMusicPlayer* g_musicPlayer;

// JNI helper functions
void initOboeMusicPlayer();
void destroyOboeMusicPlayer();
