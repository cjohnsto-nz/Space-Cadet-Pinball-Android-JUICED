#include "OboeMusicPlayer.h"
#include <android/log.h>
#include <fstream>
#include <cstring>

#define LOG_TAG "OboeMusicPlayer"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

// Global instance
OboeMusicPlayer* g_musicPlayer = nullptr;

void initOboeMusicPlayer() {
    if (g_musicPlayer == nullptr) {
        g_musicPlayer = new OboeMusicPlayer();
        LOGI("OboeMusicPlayer initialized");
    }
}

void destroyOboeMusicPlayer() {
    if (g_musicPlayer != nullptr) {
        delete g_musicPlayer;
        g_musicPlayer = nullptr;
        LOGI("OboeMusicPlayer destroyed");
    }
}

OboeMusicPlayer::OboeMusicPlayer() {
    LOGI("OboeMusicPlayer constructor");
}

OboeMusicPlayer::~OboeMusicPlayer() {
    stop();
    LOGI("OboeMusicPlayer destructor");
}

bool OboeMusicPlayer::loadFromFile(const std::string& filePath) {
    LOGI("Loading audio from: %s", filePath.c_str());
    
    std::ifstream file(filePath, std::ios::binary);
    if (!file.is_open()) {
        LOGE("Failed to open file: %s", filePath.c_str());
        return false;
    }

    // Read WAV header
    char header[44];
    file.read(header, 44);
    
    // Parse WAV header
    if (strncmp(header, "RIFF", 4) != 0 || strncmp(header + 8, "WAVE", 4) != 0) {
        LOGE("Not a valid WAV file");
        return false;
    }

    // Get format info
    int16_t audioFormat = *reinterpret_cast<int16_t*>(header + 20);
    mChannels = *reinterpret_cast<int16_t*>(header + 22);
    mSampleRate = *reinterpret_cast<int32_t*>(header + 24);
    int16_t bitsPerSample = *reinterpret_cast<int16_t*>(header + 34);
    int32_t dataSize = *reinterpret_cast<int32_t*>(header + 40);

    LOGI("WAV: format=%d, channels=%d, sampleRate=%d, bits=%d, dataSize=%d",
         audioFormat, mChannels, mSampleRate, bitsPerSample, dataSize);

    if (audioFormat != 1) {  // PCM
        LOGE("Only PCM WAV files are supported");
        return false;
    }

    // Read PCM data
    size_t numSamples = dataSize / (bitsPerSample / 8);
    
    std::lock_guard<std::mutex> lock(mDataMutex);
    mAudioData.clear();
    mAudioData.reserve(numSamples);

    if (bitsPerSample == 16) {
        std::vector<int16_t> rawData(numSamples);
        file.read(reinterpret_cast<char*>(rawData.data()), dataSize);
        
        // Convert to float
        for (size_t i = 0; i < numSamples; i++) {
            mAudioData.push_back(rawData[i] / 32768.0f);
        }
    } else if (bitsPerSample == 32) {
        std::vector<int32_t> rawData(numSamples);
        file.read(reinterpret_cast<char*>(rawData.data()), dataSize);
        
        // Convert to float
        for (size_t i = 0; i < numSamples; i++) {
            mAudioData.push_back(rawData[i] / 2147483648.0f);
        }
    } else {
        LOGE("Unsupported bits per sample: %d", bitsPerSample);
        return false;
    }

    mReadIndex = 0;
    mLowPassFilter.setSampleRate(mSampleRate);
    
    LOGI("Loaded %zu samples", mAudioData.size());
    return true;
}

bool OboeMusicPlayer::loadFromAssets(AAssetManager* assetManager, const std::string& assetPath) {
    LOGI("Loading audio from assets: %s", assetPath.c_str());
    
    AAsset* asset = AAssetManager_open(assetManager, assetPath.c_str(), AASSET_MODE_STREAMING);
    if (asset == nullptr) {
        LOGE("Failed to open asset: %s", assetPath.c_str());
        return false;
    }
    
    off_t assetLength = AAsset_getLength(asset);
    LOGI("Asset opened, length=%ld", (long)assetLength);
    
    if (assetLength < 44) {
        LOGE("Asset too small: length=%ld", (long)assetLength);
        AAsset_close(asset);
        return false;
    }
    
    // Read entire asset into memory
    std::vector<char> assetData(assetLength);
    int bytesRead = AAsset_read(asset, assetData.data(), assetLength);
    AAsset_close(asset);
    
    if (bytesRead != assetLength) {
        LOGE("Failed to read asset: read %d of %ld bytes", bytesRead, (long)assetLength);
        return false;
    }
    
    LOGI("Read %d bytes from asset", bytesRead);
    const char* data = assetData.data();
    
    // Parse WAV header
    if (strncmp(data, "RIFF", 4) != 0 || strncmp(data + 8, "WAVE", 4) != 0) {
        LOGE("Not a valid WAV file (header: %.4s...%.4s)", data, data + 8);
        return false;
    }
    
    LOGI("Valid RIFF/WAVE header found");
    
    // Find fmt chunk - it's not always at offset 12
    int32_t offset = 12;
    int16_t audioFormat = 0;
    int16_t bitsPerSample = 0;
    bool foundFmt = false;
    bool foundData = false;
    const char* pcmData = nullptr;
    int32_t dataSize = 0;
    
    while (offset < assetLength - 8) {
        const char* chunkId = data + offset;
        int32_t chunkSize = *reinterpret_cast<const int32_t*>(data + offset + 4);
        
        LOGI("Chunk: %.4s, size=%d at offset=%d", chunkId, chunkSize, offset);
        
        if (strncmp(chunkId, "fmt ", 4) == 0) {
            audioFormat = *reinterpret_cast<const int16_t*>(data + offset + 8);
            mChannels = *reinterpret_cast<const int16_t*>(data + offset + 10);
            mSampleRate = *reinterpret_cast<const int32_t*>(data + offset + 12);
            bitsPerSample = *reinterpret_cast<const int16_t*>(data + offset + 22);
            foundFmt = true;
            LOGI("fmt: format=%d, channels=%d, sampleRate=%d, bits=%d",
                 audioFormat, mChannels, mSampleRate, bitsPerSample);
        } else if (strncmp(chunkId, "data", 4) == 0) {
            dataSize = chunkSize;
            pcmData = data + offset + 8;
            foundData = true;
            LOGI("data: size=%d", dataSize);
            break; // Found data chunk, stop searching
        }
        
        offset += 8 + chunkSize;
        // Align to even boundary
        if (chunkSize % 2 != 0) offset++;
    }
    
    if (!foundFmt || !foundData) {
        LOGE("Missing fmt or data chunk: fmt=%d, data=%d", foundFmt, foundData);
        return false;
    }
    
    if (audioFormat != 1) {  // PCM
        LOGE("Only PCM WAV files are supported, got format=%d", audioFormat);
        return false;
    }
    
    size_t numSamples = dataSize / (bitsPerSample / 8);
    LOGI("Parsing %zu samples...", numSamples);
    
    std::lock_guard<std::mutex> lock(mDataMutex);
    mAudioData.clear();
    mAudioData.reserve(numSamples);
    
    if (bitsPerSample == 16) {
        const int16_t* samples = reinterpret_cast<const int16_t*>(pcmData);
        for (size_t i = 0; i < numSamples; i++) {
            mAudioData.push_back(samples[i] / 32768.0f);
        }
    } else if (bitsPerSample == 32) {
        const int32_t* samples = reinterpret_cast<const int32_t*>(pcmData);
        for (size_t i = 0; i < numSamples; i++) {
            mAudioData.push_back(samples[i] / 2147483648.0f);
        }
    } else if (bitsPerSample == 24) {
        // 24-bit audio - read 3 bytes per sample
        for (size_t i = 0; i < numSamples; i++) {
            int32_t sample = (pcmData[i*3] & 0xFF) | 
                            ((pcmData[i*3+1] & 0xFF) << 8) | 
                            ((pcmData[i*3+2]) << 16); // Sign extend
            mAudioData.push_back(sample / 8388608.0f);
        }
    } else {
        LOGE("Unsupported bits per sample: %d", bitsPerSample);
        return false;
    }
    
    mReadIndex = 0;
    mLowPassFilter.setSampleRate(mSampleRate);
    
    LOGI("Successfully loaded %zu samples from assets", mAudioData.size());
    return true;
}

bool OboeMusicPlayer::loadPCMData(const int16_t* data, size_t numSamples, int32_t sampleRate, int32_t channels) {
    std::lock_guard<std::mutex> lock(mDataMutex);
    
    mSampleRate = sampleRate;
    mChannels = channels;
    
    mAudioData.clear();
    mAudioData.reserve(numSamples);
    
    for (size_t i = 0; i < numSamples; i++) {
        mAudioData.push_back(data[i] / 32768.0f);
    }
    
    mReadIndex = 0;
    mLowPassFilter.setSampleRate(mSampleRate);
    
    LOGI("Loaded %zu PCM samples, rate=%d, channels=%d", numSamples, sampleRate, channels);
    return true;
}

bool OboeMusicPlayer::start() {
    if (mAudioData.empty()) {
        LOGE("No audio data loaded");
        return false;
    }

    oboe::AudioStreamBuilder builder;
    builder.setDirection(oboe::Direction::Output)
           ->setPerformanceMode(oboe::PerformanceMode::None)  // Balanced mode for stable playback
           ->setSharingMode(oboe::SharingMode::Shared)        // Shared mode is more stable
           ->setFormat(oboe::AudioFormat::Float)
           ->setChannelCount(mChannels)
           ->setSampleRate(mSampleRate)
           ->setBufferCapacityInFrames(mSampleRate / 10)      // 100ms buffer for stability
           ->setDataCallback(this);

    oboe::Result result = builder.openStream(mStream);
    if (result != oboe::Result::OK) {
        LOGE("Failed to open stream: %s", oboe::convertToText(result));
        return false;
    }

    result = mStream->requestStart();
    if (result != oboe::Result::OK) {
        LOGE("Failed to start stream: %s", oboe::convertToText(result));
        mStream->close();
        mStream.reset();
        return false;
    }

    mIsPlaying = true;
    LOGI("Playback started");
    return true;
}

void OboeMusicPlayer::stop() {
    mIsPlaying = false;
    if (mStream) {
        mStream->requestStop();
        mStream->close();
        mStream.reset();
        LOGI("Playback stopped");
    }
    mReadIndex = 0;
}

void OboeMusicPlayer::pause() {
    mIsPlaying = false;
    if (mStream) {
        mStream->requestPause();
        LOGI("Playback paused");
    }
}

void OboeMusicPlayer::resume() {
    if (mStream && !mIsPlaying) {
        mStream->requestStart();
        mIsPlaying = true;
        LOGI("Playback resumed");
    }
}

int64_t OboeMusicPlayer::getPositionMs() const {
    if (mAudioData.empty() || mSampleRate == 0) return 0;
    
    size_t currentFrame = mReadIndex / mChannels;
    return (currentFrame * 1000) / mSampleRate;
}

oboe::DataCallbackResult OboeMusicPlayer::onAudioReady(
        oboe::AudioStream* stream, void* audioData, int32_t numFrames) {
    
    if (!mIsPlaying) {
        // Output silence
        memset(audioData, 0, numFrames * mChannels * sizeof(float));
        return oboe::DataCallbackResult::Continue;
    }

    float* output = static_cast<float*>(audioData);
    size_t samplesNeeded = numFrames * mChannels;
    size_t samplesWritten = 0;
    float volume = mVolume.load();

    std::lock_guard<std::mutex> lock(mDataMutex);
    
    while (samplesWritten < samplesNeeded) {
        size_t currentIndex = mReadIndex.load();
        
        if (currentIndex >= mAudioData.size()) {
            if (mLooping) {
                mReadIndex = 0;
                currentIndex = 0;
            } else {
                // Fill remaining with silence
                memset(output + samplesWritten, 0, 
                       (samplesNeeded - samplesWritten) * sizeof(float));
                mIsPlaying = false;
                return oboe::DataCallbackResult::Continue;
            }
        }

        size_t samplesToRead = std::min(samplesNeeded - samplesWritten, 
                                         mAudioData.size() - currentIndex);
        
        for (size_t i = 0; i < samplesToRead; i++) {
            output[samplesWritten + i] = mAudioData[currentIndex + i] * volume;
        }
        
        mReadIndex = currentIndex + samplesToRead;
        samplesWritten += samplesToRead;
    }

    // Apply low-pass filter if enabled (muffled effect)
    mLowPassFilter.process(output, numFrames, mChannels);

    return oboe::DataCallbackResult::Continue;
}
