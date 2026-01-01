#include "OboeMusicPlayer.h"
#include <android/log.h>
#include <fstream>
#include <cstring>
#include <fcntl.h>
#include <unistd.h>

#define LOG_TAG "OboeMusicPlayer"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN, LOG_TAG, __VA_ARGS__)
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

bool OboeMusicPlayer::loadFromAssetsWithCache(AAssetManager* assetManager, const std::string& assetPath, const std::string& cacheDir) {
    LOGI("Loading WAV from assets with cache: %s", assetPath.c_str());
    
    // Check if cached PCM data exists
    std::string cacheFilePath = cacheDir + "/" + assetPath + "_cache.pcm";
    FILE* cacheFile = fopen(cacheFilePath.c_str(), "rb");
    if (cacheFile) {
        LOGI("Found cached PCM data, loading from cache");
        
        // Read header
        int32_t cachedSampleRate, cachedChannels;
        if (fread(&cachedSampleRate, sizeof(int32_t), 1, cacheFile) == 1 &&
            fread(&cachedChannels, sizeof(int32_t), 1, cacheFile) == 1) {
            
            // Get file size to determine audio data size
            fseek(cacheFile, 0, SEEK_END);
            long fileSize = ftell(cacheFile);
            fseek(cacheFile, 2 * sizeof(int32_t), SEEK_SET);
            
            size_t audioDataSize = fileSize - 2 * sizeof(int32_t);
            
            std::lock_guard<std::mutex> lock(mDataMutex);
            mSampleRate = cachedSampleRate;
            mChannels = cachedChannels;
            mAudioData.resize(audioDataSize / sizeof(float));
            
            if (fread(mAudioData.data(), sizeof(float), mAudioData.size(), cacheFile) == mAudioData.size()) {
                fclose(cacheFile);
                mReadIndex = 0;
                mLowPassFilter.setSampleRate(mSampleRate);
                LOGI("Successfully loaded %zu samples from cache", mAudioData.size());
                return true;
            }
        }
        fclose(cacheFile);
        LOGW("Cache file corrupted, loading from assets");
    }
    
    // Load from assets
    if (!loadFromAssets(assetManager, assetPath)) {
        return false;
    }
    
    // Save to cache for future loads
    FILE* saveFile = fopen(cacheFilePath.c_str(), "wb");
    if (saveFile) {
        fwrite(&mSampleRate, sizeof(int32_t), 1, saveFile);
        fwrite(&mChannels, sizeof(int32_t), 1, saveFile);
        fwrite(mAudioData.data(), sizeof(float), mAudioData.size(), saveFile);
        fclose(saveFile);
        LOGI("Saved WAV data to cache: %s", cacheFilePath.c_str());
    } else {
        LOGW("Failed to save WAV to cache: %s", cacheFilePath.c_str());
    }
    
    return true;
}

bool OboeMusicPlayer::loadCompressedFromAssets(AAssetManager* assetManager, const std::string& assetPath, const std::string& cacheDir) {
    LOGI("Loading compressed audio from assets: %s", assetPath.c_str());
    
    // Check if cached PCM data exists
    std::string cacheFilePath = cacheDir + "/" + assetPath + "_cache.pcm";
    FILE* cacheFile = fopen(cacheFilePath.c_str(), "rb");
    if (cacheFile) {
        LOGI("Found cached PCM data, loading from cache");
        fseek(cacheFile, 0, SEEK_END);
        long cacheSize = ftell(cacheFile);
        fseek(cacheFile, 0, SEEK_SET);
        
        // Read sample rate and channels first
        int32_t cachedSampleRate, cachedChannels;
        if (fread(&cachedSampleRate, sizeof(int32_t), 1, cacheFile) == 1 &&
            fread(&cachedChannels, sizeof(int32_t), 1, cacheFile) == 1) {
            
            size_t audioDataSize = cacheSize - 2 * sizeof(int32_t);
            mAudioData.resize(audioDataSize / sizeof(float));
            
            if (fread(mAudioData.data(), sizeof(float), mAudioData.size(), cacheFile) == mAudioData.size()) {
                mSampleRate = cachedSampleRate;
                mChannels = cachedChannels;
                fclose(cacheFile);
                
                mReadIndex = 0;
                mLowPassFilter.setSampleRate(mSampleRate);
                
                LOGI("Successfully loaded %zu samples from cache", mAudioData.size());
                return true;
            }
        }
        fclose(cacheFile);
        LOGI("Cache file corrupted, will decode fresh");
    }
    
    AAsset* asset = AAssetManager_open(assetManager, assetPath.c_str(), AASSET_MODE_STREAMING);
    if (asset == nullptr) {
        LOGE("Failed to open compressed asset: %s", assetPath.c_str());
        return false;
    }
    
    off_t assetLength = AAsset_getLength(asset);
    LOGI("Compressed asset opened, length=%ld", (long)assetLength);
    
    // Read entire asset into memory for MediaExtractor
    std::vector<char> assetData(assetLength);
    int bytesRead = AAsset_read(asset, assetData.data(), assetLength);
    AAsset_close(asset);
    
    if (bytesRead != assetLength) {
        LOGE("Failed to read compressed asset: read %d of %ld bytes", bytesRead, (long)assetLength);
        return false;
    }
    
    // Create temporary file for MediaExtractor (it needs a file descriptor)
    std::string tempPath = cacheDir + "/temp_audio.tmp";
    FILE* tempFile = fopen(tempPath.c_str(), "wb");
    if (!tempFile) {
        LOGE("Failed to create temporary file");
        return false;
    }
    
    size_t written = fwrite(assetData.data(), 1, assetLength, tempFile);
    fclose(tempFile);
    
    if (written != assetLength) {
        LOGE("Failed to write temporary file");
        remove(tempPath.c_str());
        return false;
    }
    
    // Use MediaExtractor to decode the compressed audio
    AMediaExtractor* extractor = AMediaExtractor_new();
    if (!extractor) {
        LOGE("Failed to create MediaExtractor");
        remove(tempPath.c_str());
        return false;
    }
    
    media_status_t status = AMediaExtractor_setDataSourceFd(extractor, open(tempPath.c_str(), O_RDONLY), 0, assetLength);
    if (status != AMEDIA_OK) {
        LOGE("Failed to set data source for MediaExtractor: %d", status);
        AMediaExtractor_delete(extractor);
        remove(tempPath.c_str());
        return false;
    }
    
    // Find the first audio track
    size_t numTracks = AMediaExtractor_getTrackCount(extractor);
    LOGI("Found %zu tracks", numTracks);
    
    AMediaCodec* codec = nullptr;
    for (size_t i = 0; i < numTracks; i++) {
        AMediaFormat* format = AMediaExtractor_getTrackFormat(extractor, i);
        if (!format) continue;
        
        const char* mime;
        if (AMediaFormat_getString(format, AMEDIAFORMAT_KEY_MIME, &mime)) {
            LOGI("Track %zu MIME: %s", i, mime);
            if (strncmp(mime, "audio/", 6) == 0) {
                // Select this track
                AMediaExtractor_selectTrack(extractor, i);
                
                // Create decoder
                codec = AMediaCodec_createDecoderByType(mime);
                if (codec) {
                    status = AMediaCodec_configure(codec, format, nullptr, nullptr, 0);
                    if (status == AMEDIA_OK) {
                        AMediaFormat_delete(format);
                        break;
                    }
                    AMediaCodec_delete(codec);
                    codec = nullptr;
                }
            }
        }
        AMediaFormat_delete(format);
    }
    
    if (!codec) {
        LOGE("Failed to find and configure audio track");
        AMediaExtractor_delete(extractor);
        remove(tempPath.c_str());
        return false;
    }
    
    // Start decoding
    status = AMediaCodec_start(codec);
    if (status != AMEDIA_OK) {
        LOGE("Failed to start codec: %d", status);
        AMediaCodec_delete(codec);
        AMediaExtractor_delete(extractor);
        remove(tempPath.c_str());
        return false;
    }
    
    // Get format info
    AMediaFormat* format = AMediaExtractor_getTrackFormat(extractor, 0);
    int32_t sampleRate, channels;
    if (AMediaFormat_getInt32(format, AMEDIAFORMAT_KEY_SAMPLE_RATE, &sampleRate)) {
        mSampleRate = sampleRate;
    }
    if (AMediaFormat_getInt32(format, AMEDIAFORMAT_KEY_CHANNEL_COUNT, &channels)) {
        mChannels = channels;
    }
    AMediaFormat_delete(format);
    
    LOGI("Decoding audio: %d Hz, %d channels", mSampleRate, mChannels);
    
    // Decode all samples
    std::vector<int16_t> pcmData;
    const size_t timeoutUs = 5000; // 5ms timeout
    
    while (true) {
        // Get input buffer
        ssize_t inputIndex = AMediaCodec_dequeueInputBuffer(codec, timeoutUs);
        if (inputIndex < 0) {
            if (inputIndex != AMEDIACODEC_INFO_TRY_AGAIN_LATER) {
                break; // End of stream or error
            }
            continue;
        }
        
        size_t inputSize;
        uint8_t* inputBuffer = AMediaCodec_getInputBuffer(codec, inputIndex, &inputSize);
        if (!inputBuffer) {
            LOGE("Failed to get input buffer");
            break;
        }
        
        // Read from extractor
        ssize_t sampleSize = AMediaExtractor_readSampleData(extractor, inputBuffer, inputSize);
        if (sampleSize <= 0) {
            // End of stream
            AMediaCodec_queueInputBuffer(codec, inputIndex, 0, 0, 0, AMEDIACODEC_BUFFER_FLAG_END_OF_STREAM);
            break;
        }
        
        AMediaExtractor_advance(extractor);
        AMediaCodec_queueInputBuffer(codec, inputIndex, 0, sampleSize, 0, 0);
        
        // Get output
        AMediaCodecBufferInfo info;
        ssize_t outputIndex = AMediaCodec_dequeueOutputBuffer(codec, &info, timeoutUs);
        if (outputIndex >= 0) {
            size_t outputSize;
            const uint8_t* outputBuffer = AMediaCodec_getOutputBuffer(codec, outputIndex, &outputSize);
            if (outputBuffer && info.size > 0) {
                // Convert to int16_t and store
                const int16_t* samples = reinterpret_cast<const int16_t*>(outputBuffer);
                size_t sampleCount = info.size / sizeof(int16_t);
                pcmData.insert(pcmData.end(), samples, samples + sampleCount);
            }
            AMediaCodec_releaseOutputBuffer(codec, outputIndex, false);
        } else if (outputIndex == AMEDIACODEC_INFO_OUTPUT_FORMAT_CHANGED) {
            // Format changed, ignore
        } else if (outputIndex != AMEDIACODEC_INFO_TRY_AGAIN_LATER) {
            break; // Error
        }
    }
    
    // Cleanup
    AMediaCodec_stop(codec);
    AMediaCodec_delete(codec);
    AMediaExtractor_delete(extractor);
    remove(tempPath.c_str());
    
    // Convert to float and store
    mAudioData.clear();
    mAudioData.reserve(pcmData.size());
    for (int16_t sample : pcmData) {
        mAudioData.push_back(sample / 32768.0f);
    }
    
    mReadIndex = 0;
    mLowPassFilter.setSampleRate(mSampleRate);
    
    // Save to cache for future loads
    std::string saveCachePath = cacheDir + "/" + assetPath + "_cache.pcm";
    FILE* saveFile = fopen(saveCachePath.c_str(), "wb");
    if (saveFile) {
        fwrite(&mSampleRate, sizeof(int32_t), 1, saveFile);
        fwrite(&mChannels, sizeof(int32_t), 1, saveFile);
        fwrite(mAudioData.data(), sizeof(float), mAudioData.size(), saveFile);
        fclose(saveFile);
        LOGI("Saved decoded audio to cache: %s", saveCachePath.c_str());
    } else {
        LOGW("Failed to save audio to cache: %s", saveCachePath.c_str());
    }
    
    LOGI("Successfully decoded %zu samples from compressed audio", mAudioData.size());
    return true;
}

bool OboeMusicPlayer::loadMissionTrackFromAssets(AAssetManager* assetManager, const std::string& assetPath) {
    LOGI("Loading mission track from assets: %s", assetPath.c_str());
    
    AAsset* asset = AAssetManager_open(assetManager, assetPath.c_str(), AASSET_MODE_STREAMING);
    if (asset == nullptr) {
        LOGE("Failed to open mission asset: %s", assetPath.c_str());
        return false;
    }
    
    off_t assetLength = AAsset_getLength(asset);
    LOGI("Mission asset opened, length=%ld", (long)assetLength);
    
    if (assetLength < 44) {
        LOGE("Mission asset too small: length=%ld", (long)assetLength);
        AAsset_close(asset);
        return false;
    }
    
    // Read entire asset into memory
    std::vector<char> assetData(assetLength);
    int bytesRead = AAsset_read(asset, assetData.data(), assetLength);
    AAsset_close(asset);
    
    if (bytesRead != assetLength) {
        LOGE("Failed to read mission asset: read %d of %ld bytes", bytesRead, (long)assetLength);
        return false;
    }
    
    const char* data = assetData.data();
    
    // Parse WAV header
    if (strncmp(data, "RIFF", 4) != 0 || strncmp(data + 8, "WAVE", 4) != 0) {
        LOGE("Mission track: Not a valid WAV file");
        return false;
    }
    
    // Find fmt and data chunks
    int32_t offset = 12;
    int16_t audioFormat = 0;
    int16_t channels = 0;
    int32_t sampleRate = 0;
    int16_t bitsPerSample = 0;
    bool foundFmt = false;
    bool foundData = false;
    const char* pcmData = nullptr;
    int32_t dataSize = 0;
    
    while (offset < assetLength - 8) {
        const char* chunkId = data + offset;
        int32_t chunkSize = *reinterpret_cast<const int32_t*>(data + offset + 4);
        
        if (strncmp(chunkId, "fmt ", 4) == 0) {
            audioFormat = *reinterpret_cast<const int16_t*>(data + offset + 8);
            channels = *reinterpret_cast<const int16_t*>(data + offset + 10);
            sampleRate = *reinterpret_cast<const int32_t*>(data + offset + 12);
            bitsPerSample = *reinterpret_cast<const int16_t*>(data + offset + 22);
            foundFmt = true;
            LOGI("Mission fmt: format=%d, channels=%d, sampleRate=%d, bits=%d",
                 audioFormat, channels, sampleRate, bitsPerSample);
        } else if (strncmp(chunkId, "data", 4) == 0) {
            dataSize = chunkSize;
            pcmData = data + offset + 8;
            foundData = true;
            break;
        }
        
        offset += 8 + chunkSize;
        if (chunkSize % 2 != 0) offset++;
    }
    
    if (!foundFmt || !foundData) {
        LOGE("Mission track: Missing fmt or data chunk");
        return false;
    }
    
    if (audioFormat != 1) {
        LOGE("Mission track: Only PCM WAV supported");
        return false;
    }
    
    // Verify format matches main track
    if (sampleRate != mSampleRate || channels != mChannels) {
        LOGW("Mission track format differs from main track (sr=%d vs %d, ch=%d vs %d) - may cause issues",
             sampleRate, mSampleRate, channels, mChannels);
    }
    
    size_t numSamples = dataSize / (bitsPerSample / 8);
    
    std::lock_guard<std::mutex> lock(mDataMutex);
    mMissionAudioData.clear();
    mMissionAudioData.reserve(numSamples);
    
    if (bitsPerSample == 16) {
        const int16_t* samples = reinterpret_cast<const int16_t*>(pcmData);
        for (size_t i = 0; i < numSamples; i++) {
            mMissionAudioData.push_back(samples[i] / 32768.0f);
        }
    } else if (bitsPerSample == 24) {
        for (size_t i = 0; i < numSamples; i++) {
            int32_t sample = (pcmData[i*3] & 0xFF) | 
                            ((pcmData[i*3+1] & 0xFF) << 8) | 
                            ((pcmData[i*3+2]) << 16);
            mMissionAudioData.push_back(sample / 8388608.0f);
        }
    } else if (bitsPerSample == 32) {
        const int32_t* samples = reinterpret_cast<const int32_t*>(pcmData);
        for (size_t i = 0; i < numSamples; i++) {
            mMissionAudioData.push_back(samples[i] / 2147483648.0f);
        }
    } else {
        LOGE("Mission track: Unsupported bits per sample: %d", bitsPerSample);
        return false;
    }
    
    LOGI("Successfully loaded mission track: %zu samples", mMissionAudioData.size());
    return true;
}

bool OboeMusicPlayer::loadMissionTrackFromAssetsWithCache(AAssetManager* assetManager, const std::string& assetPath, const std::string& cacheDir) {
    LOGI("Loading mission WAV from assets with cache: %s", assetPath.c_str());
    
    // Check if cached PCM data exists
    std::string cacheFilePath = cacheDir + "/" + assetPath + "_cache.pcm";
    FILE* cacheFile = fopen(cacheFilePath.c_str(), "rb");
    if (cacheFile) {
        LOGI("Found cached mission PCM data, loading from cache");
        
        // Read header
        int32_t cachedSampleRate, cachedChannels;
        if (fread(&cachedSampleRate, sizeof(int32_t), 1, cacheFile) == 1 &&
            fread(&cachedChannels, sizeof(int32_t), 1, cacheFile) == 1) {
            
            // Get file size to determine audio data size
            fseek(cacheFile, 0, SEEK_END);
            long fileSize = ftell(cacheFile);
            fseek(cacheFile, 2 * sizeof(int32_t), SEEK_SET);
            
            size_t audioDataSize = fileSize - 2 * sizeof(int32_t);
            
            std::lock_guard<std::mutex> lock(mDataMutex);
            mMissionSampleRate = cachedSampleRate;
            mMissionChannels = cachedChannels;
            mMissionAudioData.resize(audioDataSize / sizeof(float));
            
            if (fread(mMissionAudioData.data(), sizeof(float), mMissionAudioData.size(), cacheFile) == mMissionAudioData.size()) {
                fclose(cacheFile);
                LOGI("Successfully loaded %zu mission samples from cache", mMissionAudioData.size());
                return true;
            }
        }
        fclose(cacheFile);
        LOGW("Mission cache file corrupted, loading from assets");
    }
    
    // Load from assets
    if (!loadMissionTrackFromAssets(assetManager, assetPath)) {
        return false;
    }
    
    // Save to cache for future loads
    FILE* saveFile = fopen(cacheFilePath.c_str(), "wb");
    if (saveFile) {
        fwrite(&mMissionSampleRate, sizeof(int32_t), 1, saveFile);
        fwrite(&mMissionChannels, sizeof(int32_t), 1, saveFile);
        fwrite(mMissionAudioData.data(), sizeof(float), mMissionAudioData.size(), saveFile);
        fclose(saveFile);
        LOGI("Saved mission WAV data to cache: %s", cacheFilePath.c_str());
    } else {
        LOGW("Failed to save mission WAV to cache: %s", cacheFilePath.c_str());
    }
    
    return true;
}

bool OboeMusicPlayer::loadMissionTrackCompressed(AAssetManager* assetManager, const std::string& assetPath, const std::string& cacheDir) {
    LOGI("Loading compressed mission track from assets: %s", assetPath.c_str());
    
    // Check if cached PCM data exists
    std::string cacheFilePath = cacheDir + "/" + assetPath + "_cache.pcm";
    FILE* cacheFile = fopen(cacheFilePath.c_str(), "rb");
    if (cacheFile) {
        LOGI("Found cached mission PCM data, loading from cache");
        fseek(cacheFile, 0, SEEK_END);
        long cacheSize = ftell(cacheFile);
        fseek(cacheFile, 0, SEEK_SET);
        
        // Read sample rate and channels first
        int32_t cachedSampleRate, cachedChannels;
        if (fread(&cachedSampleRate, sizeof(int32_t), 1, cacheFile) == 1 &&
            fread(&cachedChannels, sizeof(int32_t), 1, cacheFile) == 1) {
            
            size_t audioDataSize = cacheSize - 2 * sizeof(int32_t);
            mMissionAudioData.resize(audioDataSize / sizeof(float));
            
            if (fread(mMissionAudioData.data(), sizeof(float), mMissionAudioData.size(), cacheFile) == mMissionAudioData.size()) {
                mMissionSampleRate = cachedSampleRate;
                mMissionChannels = cachedChannels;
                fclose(cacheFile);
                
                LOGI("Successfully loaded %zu mission samples from cache", mMissionAudioData.size());
                return true;
            }
        }
        fclose(cacheFile);
        LOGI("Mission cache file corrupted, will decode fresh");
    }
    
    AAsset* asset = AAssetManager_open(assetManager, assetPath.c_str(), AASSET_MODE_STREAMING);
    if (asset == nullptr) {
        LOGE("Failed to open compressed mission asset: %s", assetPath.c_str());
        return false;
    }
    
    off_t assetLength = AAsset_getLength(asset);
    LOGI("Compressed mission asset opened, length=%ld", (long)assetLength);
    
    // Read entire asset into memory for MediaExtractor
    std::vector<char> assetData(assetLength);
    int bytesRead = AAsset_read(asset, assetData.data(), assetLength);
    AAsset_close(asset);
    
    if (bytesRead != assetLength) {
        LOGE("Failed to read compressed mission asset: read %d of %ld bytes", bytesRead, (long)assetLength);
        return false;
    }
    
    // Create temporary file for MediaExtractor
    std::string tempPath = cacheDir + "/temp_mission_audio.tmp";
    FILE* tempFile = fopen(tempPath.c_str(), "wb");
    if (!tempFile) {
        LOGE("Failed to create temporary mission file");
        return false;
    }
    
    size_t written = fwrite(assetData.data(), 1, assetLength, tempFile);
    fclose(tempFile);
    
    if (written != assetLength) {
        LOGE("Failed to write temporary mission file");
        remove(tempPath.c_str());
        return false;
    }
    
    // Use MediaExtractor to decode the compressed audio
    AMediaExtractor* extractor = AMediaExtractor_new();
    if (!extractor) {
        LOGE("Failed to create MediaExtractor for mission track");
        remove(tempPath.c_str());
        return false;
    }
    
    media_status_t status = AMediaExtractor_setDataSourceFd(extractor, open(tempPath.c_str(), O_RDONLY), 0, assetLength);
    if (status != AMEDIA_OK) {
        LOGE("Failed to set data source for mission MediaExtractor: %d", status);
        AMediaExtractor_delete(extractor);
        remove(tempPath.c_str());
        return false;
    }
    
    // Find the first audio track
    size_t numTracks = AMediaExtractor_getTrackCount(extractor);
    LOGI("Found %zu tracks in mission audio", numTracks);
    
    AMediaCodec* codec = nullptr;
    int32_t missionSampleRate = 48000;
    int32_t missionChannels = 2;
    
    for (size_t i = 0; i < numTracks; i++) {
        AMediaFormat* format = AMediaExtractor_getTrackFormat(extractor, i);
        if (!format) continue;
        
        const char* mime;
        if (AMediaFormat_getString(format, AMEDIAFORMAT_KEY_MIME, &mime)) {
            LOGI("Mission track %zu MIME: %s", i, mime);
            if (strncmp(mime, "audio/", 6) == 0) {
                // Select this track
                AMediaExtractor_selectTrack(extractor, i);
                
                // Get format info
                AMediaFormat_getInt32(format, AMEDIAFORMAT_KEY_SAMPLE_RATE, &missionSampleRate);
                AMediaFormat_getInt32(format, AMEDIAFORMAT_KEY_CHANNEL_COUNT, &missionChannels);
                
                // Create decoder
                codec = AMediaCodec_createDecoderByType(mime);
                if (codec) {
                    status = AMediaCodec_configure(codec, format, nullptr, nullptr, 0);
                    if (status == AMEDIA_OK) {
                        AMediaFormat_delete(format);
                        break;
                    }
                    AMediaCodec_delete(codec);
                    codec = nullptr;
                }
            }
        }
        AMediaFormat_delete(format);
    }
    
    if (!codec) {
        LOGE("Failed to find and configure mission audio track");
        AMediaExtractor_delete(extractor);
        remove(tempPath.c_str());
        return false;
    }
    
    // Start decoding
    status = AMediaCodec_start(codec);
    if (status != AMEDIA_OK) {
        LOGE("Failed to start mission codec: %d", status);
        AMediaCodec_delete(codec);
        AMediaExtractor_delete(extractor);
        remove(tempPath.c_str());
        return false;
    }
    
    LOGI("Decoding mission audio: %d Hz, %d channels", missionSampleRate, missionChannels);
    
    // Check format compatibility
    if (missionSampleRate != mSampleRate || missionChannels != mChannels) {
        LOGW("Mission track format differs from main track (sr=%d vs %d, ch=%d vs %d) - may cause issues",
             missionSampleRate, mSampleRate, missionChannels, mChannels);
    }
    
    // Decode all samples
    std::vector<int16_t> pcmData;
    const size_t timeoutUs = 5000; // 5ms timeout
    
    while (true) {
        // Get input buffer
        ssize_t inputIndex = AMediaCodec_dequeueInputBuffer(codec, timeoutUs);
        if (inputIndex < 0) {
            if (inputIndex != AMEDIACODEC_INFO_TRY_AGAIN_LATER) {
                break; // End of stream or error
            }
            continue;
        }
        
        size_t inputSize;
        uint8_t* inputBuffer = AMediaCodec_getInputBuffer(codec, inputIndex, &inputSize);
        if (!inputBuffer) {
            LOGE("Failed to get mission input buffer");
            break;
        }
        
        // Read from extractor
        ssize_t sampleSize = AMediaExtractor_readSampleData(extractor, inputBuffer, inputSize);
        if (sampleSize <= 0) {
            // End of stream
            AMediaCodec_queueInputBuffer(codec, inputIndex, 0, 0, 0, AMEDIACODEC_BUFFER_FLAG_END_OF_STREAM);
            break;
        }
        
        AMediaExtractor_advance(extractor);
        AMediaCodec_queueInputBuffer(codec, inputIndex, 0, sampleSize, 0, 0);
        
        // Get output
        AMediaCodecBufferInfo info;
        ssize_t outputIndex = AMediaCodec_dequeueOutputBuffer(codec, &info, timeoutUs);
        if (outputIndex >= 0) {
            size_t outputSize;
            const uint8_t* outputBuffer = AMediaCodec_getOutputBuffer(codec, outputIndex, &outputSize);
            if (outputBuffer && info.size > 0) {
                // Convert to int16_t and store
                const int16_t* samples = reinterpret_cast<const int16_t*>(outputBuffer);
                size_t sampleCount = info.size / sizeof(int16_t);
                pcmData.insert(pcmData.end(), samples, samples + sampleCount);
            }
            AMediaCodec_releaseOutputBuffer(codec, outputIndex, false);
        } else if (outputIndex == AMEDIACODEC_INFO_OUTPUT_FORMAT_CHANGED) {
            // Format changed, ignore
        } else if (outputIndex != AMEDIACODEC_INFO_TRY_AGAIN_LATER) {
            break; // Error
        }
    }
    
    // Cleanup
    AMediaCodec_stop(codec);
    AMediaCodec_delete(codec);
    AMediaExtractor_delete(extractor);
    remove(tempPath.c_str());
    
    // Convert to float and store
    std::lock_guard<std::mutex> lock(mDataMutex);
    mMissionAudioData.clear();
    mMissionAudioData.reserve(pcmData.size());
    
    for (int16_t sample : pcmData) {
        mMissionAudioData.push_back(sample / 32768.0f);
    }
    
    LOGI("Mission track size: %zu samples (main track: %zu)", mMissionAudioData.size(), mAudioData.size());
    
    // Save to cache for future loads
    std::string saveCachePath = cacheDir + "/" + assetPath + "_cache.pcm";
    FILE* saveFile = fopen(saveCachePath.c_str(), "wb");
    if (saveFile) {
        fwrite(&mMissionSampleRate, sizeof(int32_t), 1, saveFile);
        fwrite(&mMissionChannels, sizeof(int32_t), 1, saveFile);
        fwrite(mMissionAudioData.data(), sizeof(float), mMissionAudioData.size(), saveFile);
        fclose(saveFile);
        LOGI("Saved decoded mission audio to cache: %s", saveCachePath.c_str());
    } else {
        LOGW("Failed to save mission audio to cache: %s", saveCachePath.c_str());
    }
    
    LOGI("Successfully decoded %zu samples from compressed mission audio", mMissionAudioData.size());
    return true;
}

void OboeMusicPlayer::setMissionTrackEnabled(bool enabled) {
    mMissionEnabled = enabled;
    LOGI("Mission track %s", enabled ? "enabled" : "disabled");
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
    
    // Determine target mission volume based on enabled state
    float missionTargetVol = mMissionEnabled ? mMissionTargetVolume.load() : 0.0f;
    bool hasMissionTrack = !mMissionAudioData.empty();
    
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
            // Main track
            float sample = mAudioData[currentIndex + i] * volume;
            
            // Mix in mission track with smooth fade
            if (hasMissionTrack) {
                // Smooth fade toward target volume
                if (mMissionCurrentVolume < missionTargetVol) {
                    mMissionCurrentVolume += kMissionFadeSpeed;
                    if (mMissionCurrentVolume > missionTargetVol) 
                        mMissionCurrentVolume = missionTargetVol;
                } else if (mMissionCurrentVolume > missionTargetVol) {
                    mMissionCurrentVolume -= kMissionFadeSpeed;
                    if (mMissionCurrentVolume < missionTargetVol) 
                        mMissionCurrentVolume = missionTargetVol;
                }
                
                // Add mission track if volume > 0 and within bounds
                if (mMissionCurrentVolume > 0.001f && currentIndex + i < mMissionAudioData.size()) {
                    sample += mMissionAudioData[currentIndex + i] * mMissionCurrentVolume * volume;
                }
            }
            
            output[samplesWritten + i] = sample;
        }
        
        mReadIndex = currentIndex + samplesToRead;
        samplesWritten += samplesToRead;
    }

    // Apply low-pass filter if enabled (muffled effect)
    mLowPassFilter.process(output, numFrames, mChannels);

    return oboe::DataCallbackResult::Continue;
}
