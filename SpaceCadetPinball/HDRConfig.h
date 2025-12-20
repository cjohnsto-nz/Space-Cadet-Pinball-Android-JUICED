#pragma once

#include <cstdint>
#include <cmath>

namespace HDR {

// HDR capability flags
struct HDRCapabilities {
    bool isSupported;           // Device supports HDR
    bool isBT2020Supported;     // BT.2020 colorspace supported
    bool isPQSupported;         // PQ (Perceptual Quantizer) transfer function supported
    bool isScRGBSupported;      // scRGB linear colorspace supported
    bool isFP16Supported;       // FP16 framebuffer supported
    float maxLuminanceNits;     // Maximum display luminance in nits
    float minLuminanceNits;     // Minimum display luminance in nits
    float sdrWhiteNits;         // SDR reference white level in nits
};

// Global HDR state
extern HDRCapabilities g_hdrCapabilities;
extern bool g_hdrEnabled;

// Reference luminance values (in nits)
namespace Luminance {
    // BT.2100 reference white for SDR content
    constexpr float SDR_WHITE_NITS = 203.0f;
    
    // Typical HDR display capabilities
    constexpr float DEFAULT_MAX_NITS = 1000.0f;
    constexpr float DEFAULT_MIN_NITS = 0.005f;
    
    // Pinball light intensities (in nits)
    constexpr float LIGHT_OFF = 0.0f;
    constexpr float LIGHT_AMBIENT = 50.0f;        // Ambient/background glow
    constexpr float LIGHT_DIM = 150.0f;           // Dim light state
    constexpr float LIGHT_NORMAL = 300.0f;        // Normal lit state
    constexpr float LIGHT_BRIGHT = 500.0f;        // Bright highlight
    constexpr float LIGHT_FLASH_PEAK = 800.0f;    // Peak flasher brightness
    constexpr float LIGHT_SPECULAR = 1000.0f;     // Specular highlights/reflections
    
    // Ball and metallic surface reflections
    constexpr float BALL_HIGHLIGHT = 600.0f;
    constexpr float CHROME_REFLECTION = 700.0f;
}

// Color space conversion utilities
namespace ColorSpace {

    // Convert nits to linear light (normalized to SDR white = 1.0)
    inline float NitsToLinear(float nits) {
        return nits / Luminance::SDR_WHITE_NITS;
    }
    
    // Convert linear light to nits
    inline float LinearToNits(float linear) {
        return linear * Luminance::SDR_WHITE_NITS;
    }
    
    // SMPTE ST.2084 PQ EOTF (Electro-Optical Transfer Function)
    // Converts PQ encoded value to linear light (normalized 0-1 for 10000 nits)
    inline float PQToLinear(float pq) {
        const float m1 = 0.1593017578125f;
        const float m2 = 78.84375f;
        const float c1 = 0.8359375f;
        const float c2 = 18.8515625f;
        const float c3 = 18.6875f;
        
        float Vm2 = powf(pq, 1.0f / m2);
        float num = fmaxf(Vm2 - c1, 0.0f);
        float den = c2 - c3 * Vm2;
        return powf(num / den, 1.0f / m1);
    }
    
    // SMPTE ST.2084 PQ OETF (Opto-Electronic Transfer Function)
    // Converts linear light to PQ encoded value
    // Input: linear light normalized to 10000 nits (so 1.0 = 10000 nits)
    inline float LinearToPQ(float linear) {
        const float m1 = 0.1593017578125f;
        const float m2 = 78.84375f;
        const float c1 = 0.8359375f;
        const float c2 = 18.8515625f;
        const float c3 = 18.6875f;
        
        float Lm1 = powf(linear, m1);
        return powf((c1 + c2 * Lm1) / (1.0f + c3 * Lm1), m2);
    }
    
    // Convert nits directly to PQ encoded value
    inline float NitsToPQ(float nits) {
        // PQ is normalized to 10000 nits
        return LinearToPQ(nits / 10000.0f);
    }
    
    // Convert PQ encoded value to nits
    inline float PQToNits(float pq) {
        return PQToLinear(pq) * 10000.0f;
    }
    
    // sRGB to linear conversion (for SDR content)
    inline float SRGBToLinear(float srgb) {
        if (srgb <= 0.04045f) {
            return srgb / 12.92f;
        }
        return powf((srgb + 0.055f) / 1.055f, 2.4f);
    }
    
    // Linear to sRGB conversion
    inline float LinearToSRGB(float linear) {
        if (linear <= 0.0031308f) {
            return linear * 12.92f;
        }
        return 1.055f * powf(linear, 1.0f / 2.4f) - 0.055f;
    }
    
    // BT.709 to BT.2020 color space conversion matrix (approximate)
    // For more accurate conversion, use full 3x3 matrix
    inline void BT709ToBT2020(float& r, float& g, float& b) {
        float r2020 = 0.6274f * r + 0.3293f * g + 0.0433f * b;
        float g2020 = 0.0691f * r + 0.9195f * g + 0.0114f * b;
        float b2020 = 0.0164f * r + 0.0880f * g + 0.8956f * b;
        r = r2020;
        g = g2020;
        b = b2020;
    }
}

// HDR color type using half-precision floats (FP16)
// Values are in linear light space, can exceed 1.0 for HDR
struct ColorHDR {
    float r;
    float g;
    float b;
    float a;
    
    ColorHDR() : r(0), g(0), b(0), a(1.0f) {}
    ColorHDR(float r_, float g_, float b_, float a_ = 1.0f) : r(r_), g(g_), b(b_), a(a_) {}
    
    // Create from SDR 8-bit color (0-255) with optional HDR intensity multiplier
    static ColorHDR FromSDR(uint8_t r8, uint8_t g8, uint8_t b8, uint8_t a8 = 255, float intensityNits = Luminance::SDR_WHITE_NITS) {
        float scale = ColorSpace::NitsToLinear(intensityNits);
        return ColorHDR(
            ColorSpace::SRGBToLinear(r8 / 255.0f) * scale,
            ColorSpace::SRGBToLinear(g8 / 255.0f) * scale,
            ColorSpace::SRGBToLinear(b8 / 255.0f) * scale,
            a8 / 255.0f
        );
    }
    
    // Create a light color with specified brightness in nits
    static ColorHDR Light(float nits, float r_ = 1.0f, float g_ = 1.0f, float b_ = 1.0f) {
        float intensity = ColorSpace::NitsToLinear(nits);
        return ColorHDR(r_ * intensity, g_ * intensity, b_ * intensity, 1.0f);
    }
    
    // Multiply intensity
    ColorHDR operator*(float scale) const {
        return ColorHDR(r * scale, g * scale, b * scale, a);
    }
    
    // Add colors (for light accumulation)
    ColorHDR operator+(const ColorHDR& other) const {
        return ColorHDR(r + other.r, g + other.g, b + other.b, fmaxf(a, other.a));
    }
};

// Initialize HDR system
void InitHDR();

// Check if HDR is available and enabled
bool IsHDRActive();

// Get the current max luminance the display supports
float GetMaxDisplayNits();

// Set HDR enabled state
void SetHDREnabled(bool enabled);

} // namespace HDR
