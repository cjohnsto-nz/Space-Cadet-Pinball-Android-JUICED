#include "pch.h"
#include "HDRConfig.h"

#ifdef __ANDROID__
#include <android/log.h>
#define HDR_LOG(...) __android_log_print(ANDROID_LOG_INFO, "HDRConfig", __VA_ARGS__)
#else
#define HDR_LOG(...) printf(__VA_ARGS__)
#endif

namespace HDR {

// Global HDR state
HDRCapabilities g_hdrCapabilities = {
    false,  // isSupported
    false,  // isBT2020Supported
    false,  // isPQSupported
    false,  // isScRGBSupported
    false,  // isFP16Supported
    Luminance::DEFAULT_MAX_NITS,
    Luminance::DEFAULT_MIN_NITS,
    Luminance::SDR_WHITE_NITS
};

bool g_hdrEnabled = false;
bool g_hdrUserEnabled = true;  // User preference from Java side

void InitHDR() {
    // HDR is enabled if hardware supports it AND user has enabled it
    g_hdrEnabled = g_hdrCapabilities.isSupported && g_hdrUserEnabled;
    HDR_LOG("InitHDR: supported=%d, userEnabled=%d, active=%d", 
            g_hdrCapabilities.isSupported, g_hdrUserEnabled, g_hdrEnabled);
}

bool IsHDRActive() {
    return g_hdrEnabled;
}

float GetMaxDisplayNits() {
    if (IsHDRActive()) {
        return g_hdrCapabilities.maxLuminanceNits;
    }
    return Luminance::SDR_WHITE_NITS;
}

void SetHDREnabled(bool enabled) {
    g_hdrUserEnabled = enabled;
    g_hdrEnabled = enabled && g_hdrCapabilities.isSupported;
    HDR_LOG("SetHDREnabled: userEnabled=%d, active=%d", enabled, g_hdrEnabled);
}

} // namespace HDR
