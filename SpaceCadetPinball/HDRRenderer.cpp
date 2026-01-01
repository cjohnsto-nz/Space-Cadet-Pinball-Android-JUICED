#include "pch.h"
#include "HDRRenderer.h"
#include "HDRConfig.h"
#include "HDRLightOverlay.h"
#include "render.h"
#include <chrono>

#ifdef __ANDROID__
#include <android/log.h>
#define HDR_LOG(...) __android_log_print(ANDROID_LOG_INFO, "HDRRenderer", __VA_ARGS__)
#define HDR_ERR(...) __android_log_print(ANDROID_LOG_ERROR, "HDRRenderer", __VA_ARGS__)
#else
#define HDR_LOG(...) printf(__VA_ARGS__)
#define HDR_ERR(...) fprintf(stderr, __VA_ARGS__)
#endif

// GL_BGRA_EXT may not be defined on all platforms
#ifndef GL_BGRA_EXT
#define GL_BGRA_EXT 0x80E1
#endif

// Static member initialization
bool HDRRenderer::s_initialized = false;
int HDRRenderer::s_width = 0;
int HDRRenderer::s_height = 0;
int HDRRenderer::s_viewportX = 0;
int HDRRenderer::s_viewportY = 0;
int HDRRenderer::s_viewportW = 0;
int HDRRenderer::s_viewportH = 0;
GLuint HDRRenderer::s_hdrFBO = 0;
GLuint HDRRenderer::s_hdrTexture = 0;
GLuint HDRRenderer::s_sdrTexture = 0;
GLuint HDRRenderer::s_outputProgram = 0;
GLuint HDRRenderer::s_uploadProgram = 0;
GLuint HDRRenderer::s_quadVAO = 0;
GLuint HDRRenderer::s_quadVBO = 0;
float HDRRenderer::s_exposure = 1.0f;
bool HDRRenderer::s_cameraTrackingEnabled = false;  // Match Java default
float HDRRenderer::s_cameraZoom = 2.0f;
float HDRRenderer::s_currentCameraZoom = 1.0f;
float HDRRenderer::s_currentCameraCenterX = 0.5f;
float HDRRenderer::s_currentCameraCenterY = 0.5f;

// Cached uniform locations - output program
GLint HDRRenderer::s_loc_uHDRTexture = -1;
GLint HDRRenderer::s_loc_uMaxNits = -1;
GLint HDRRenderer::s_loc_uSDRWhiteNits = -1;
GLint HDRRenderer::s_loc_uCameraZoom = -1;
GLint HDRRenderer::s_loc_uCameraCenter = -1;
GLint HDRRenderer::s_loc_uViewportSize = -1;
GLint HDRRenderer::s_loc_uViewportOffset = -1;
GLint HDRRenderer::s_loc_uRawBallPos = -1;
GLint HDRRenderer::s_loc_uLastBallPos = -1;
GLint HDRRenderer::s_loc_uBallValid = -1;
GLint HDRRenderer::s_loc_uSmoothedCameraPos = -1;
// Cached uniform locations - upload program
GLint HDRRenderer::s_loc_upload_uTexture = -1;
GLint HDRRenderer::s_loc_upload_uIntensityMultiplier = -1;
GLint HDRRenderer::s_loc_upload_uExposure = -1;

// Vertex shader - simple fullscreen quad (camera transform done in fragment shader)
const char* HDRRenderer::s_vertexShaderSrc = R"(#version 300 es
precision highp float;
layout(location = 0) in vec2 aPos;
layout(location = 1) in vec2 aTexCoord;
out vec2 vTexCoord;

void main() {
    gl_Position = vec4(aPos, 0.0, 1.0);
    vTexCoord = aTexCoord;
}
)";

// Fragment shader - pass through SDR content unchanged
// Just converts sRGB to linear for the HDR pipeline, no boosting
const char* HDRRenderer::s_hdrUploadFragmentSrc = R"(#version 300 es
precision highp float;
in vec2 vTexCoord;
out vec4 fragColor;

uniform sampler2D uTexture;
uniform float uIntensityMultiplier;  // Unused for now, reserved for light overlays
uniform float uExposure;             // Unused for now

// sRGB to linear conversion
vec3 sRGBToLinear(vec3 srgb) {
    vec3 low = srgb / 12.92;
    vec3 high = pow((srgb + 0.055) / 1.055, vec3(2.4));
    return mix(low, high, step(vec3(0.04045), srgb));
}

void main() {
    // Flip V coordinate to mirror vertically
    vec2 flippedCoord = vec2(vTexCoord.x, 1.0 - vTexCoord.y);
    vec4 sdrColor = texture(uTexture, flippedCoord);
    
    // Swap R and B channels (input is BGRA stored as RGBA)
    sdrColor.rgb = sdrColor.bgr;
    
    // Convert from sRGB to linear - this is required for correct PQ encoding
    // The value 1.0 in linear = SDR white (203 nits)
    vec3 linearColor = sRGBToLinear(sdrColor.rgb);
    
    // Pass through unchanged - SDR content at SDR levels
    fragColor = vec4(linearColor, sdrColor.a);
}
)";

// Fragment shader for HDR to PQ (ST.2084) output
// For debugging: set DEBUG_PASSTHROUGH to 1 to bypass PQ encoding
const char* HDRRenderer::s_pqOutputFragmentSrc = R"(#version 300 es
precision highp float;
in vec2 vTexCoord;
out vec4 fragColor;

uniform sampler2D uHDRTexture;
uniform float uMaxNits;
uniform float uSDRWhiteNits;
uniform float uCameraZoom;
uniform vec2 uCameraCenter;
uniform vec2 uViewportSize;    // actual viewport width, height in pixels
uniform vec2 uViewportOffset;  // viewport X, Y offset from window origin
uniform vec2 uLastBallPos;     // last known valid ball position
uniform float uBallValid;      // 1.0 if ball is valid, 0.0 if in teleporter
uniform vec2 uSmoothedCameraPos; // smoothed camera position with center bias
uniform vec2 uRawBallPos;        // raw ball position for debug line

// ST.2084 PQ constants
const float m1 = 0.1593017578125;
const float m2 = 78.84375;
const float c1 = 0.8359375;
const float c2 = 18.8515625;
const float c3 = 18.6875;

// Linear to PQ encoding
// Input: linear light normalized so 1.0 = 10000 nits
vec3 linearToPQ(vec3 linear) {
    vec3 Lm1 = pow(linear, vec3(m1));
    return pow((c1 + c2 * Lm1) / (1.0 + c3 * Lm1), vec3(m2));
}

// Linear to sRGB gamma (for debug/fallback)
vec3 linearToSRGB(vec3 linear) {
    vec3 low = linear * 12.92;
    vec3 high = 1.055 * pow(linear, vec3(1.0/2.4)) - 0.055;
    return mix(low, high, step(vec3(0.0031308), linear));
}

// BT.709 to BT.2020 color space conversion
vec3 bt709ToBt2020(vec3 color) {
    mat3 M = mat3(
        0.6274, 0.0691, 0.0164,
        0.3293, 0.9195, 0.0880,
        0.0433, 0.0114, 0.8956
    );
    return M * color;
}

// Luminance-dependent saturation boost - brighter colors get more saturation
vec3 applySaturationBoost(vec3 color, float baseAmount, float brightnessScale) {
    float luminance = dot(color, vec3(0.2126, 0.7152, 0.0722));
    
    // Boost saturation more for brighter pixels (luminance-dependent)
    // Smoothstep creates a nice curve: low boost for darks, high boost for brights
    float brightnessFactor = smoothstep(0.2, 0.9, luminance);
    float satAmount = baseAmount + brightnessScale * brightnessFactor;
    
    // Apply saturation boost
    return mix(vec3(luminance), color, 1.0 + satAmount);
}

// Luminance-dependent gamut expansion - brighter colors expand more into BT.2020
vec3 expandGamut(vec3 bt709Color, float baseAmount, float brightnessScale) {
    float luminance = dot(bt709Color, vec3(0.2126, 0.7152, 0.0722));
    
    // Expand gamut more for brighter pixels
    float brightnessFactor = smoothstep(0.15, 0.85, luminance);
    float amount = baseAmount + brightnessScale * brightnessFactor;
    
    // This matrix expands colors beyond BT.709 toward BT.2020
    // Stronger expansion for brighter colors creates more vivid highlights
    mat3 expand = mat3(
        1.0 + 0.15 * amount, -0.075 * amount, -0.075 * amount,
        -0.075 * amount, 1.0 + 0.15 * amount, -0.075 * amount,
        -0.075 * amount, -0.075 * amount, 1.0 + 0.15 * amount
    );
    return expand * bt709Color;
}

void main() {
    // Simple approach: zoom around a point, then offset
    float zoom = uCameraZoom > 0.0 ? uCameraZoom : 1.0;
    vec2 center = uCameraCenter;
    
    // Step 1: Zoom around screen center (0.5, 0.5)
    vec2 texCoord = (vTexCoord - 0.5) / zoom + 0.5;
    
    // Step 2: Offset to center on the target position
    // If center is 0.3, we want 0.3 to appear at screen center (0.5)
    // So we need to shift texture coords by (center - 0.5) to move texture LEFT
    // (shifting texture left means the point at center appears at screen center)
    texCoord.x -= (center.x - 0.5);
    texCoord.y -= (center.y - 0.5);
    
    // Show black for out-of-bounds and scoreboard area (past 0.61)
    vec4 hdrColor;
    if (texCoord.x < 0.0 || texCoord.x > 0.61 || texCoord.y < 0.0 || texCoord.y > 1.0) {
        hdrColor = vec4(0.0, 0.0, 0.0, 1.0);
    } else {
        hdrColor = texture(uHDRTexture, texCoord);
    }
    
    /* DEBUG: Draw vertical lines - using corrected transform
    // Main texture: texCoord = (screenPos - 0.5) / zoom + 1.0 - center
    // Inverse: screenPos = (texPos - 1.0 + center) * zoom + 0.5
    
    // Red - TABLE center (texture coord 0.3035)
    float tableCenterTexX = 0.3035;
    float redScreenX = ((tableCenterTexX - 1.0 + center.x) * zoom + 0.5) * uViewportSize.x;
    if (abs(gl_FragCoord.x - redScreenX) < 3.0) {
        hdrColor = vec4(1.0, 0.0, 0.0, 1.0);
    }
    
    // Magenta - table center in texture coords (0.5 * 0.607 = 0.3035)
    float magentaTexX = 0.5 * 0.607;
    float magentaScreenX = ((magentaTexX - 1.0 + center.x) * zoom + 0.5) * uViewportSize.x;
    if (abs(gl_FragCoord.x - magentaScreenX) < 3.0) {
        hdrColor = vec4(1.0, 0.0, 1.0, 1.0);
    }
    
    // Green - raw ball position (transformed)
    float greenScreenX = ((uRawBallPos.x - 1.0 + center.x) * zoom + 0.5) * uViewportSize.x;
    if (abs(gl_FragCoord.x - greenScreenX) < 3.0) {
        hdrColor = vec4(0.0, 1.0, 0.0, 1.0);
    }
    
    // Yellow - last valid ball position (only when invalid, transformed)
    if (uBallValid < 0.5) {
        float yellowScreenX = ((uLastBallPos.x - 1.0 + center.x) * zoom + 0.5) * uViewportSize.x;
        if (abs(gl_FragCoord.x - yellowScreenX) < 3.0) {
            hdrColor = vec4(1.0, 1.0, 0.0, 1.0);
        }
    }
    
    // Cyan - smoothed camera position (transformed)
    float cyanScreenX = ((uSmoothedCameraPos.x - 1.0 + center.x) * zoom + 0.5) * uViewportSize.x;
    if (abs(gl_FragCoord.x - cyanScreenX) < 3.0) {
        hdrColor = vec4(0.0, 1.0, 1.0, 1.0);
    }
    
    // DEBUG: Draw horizontal lines - using corrected transform
    // Magenta horizontal - viewport center Y
    float viewportCenterY = uViewportSize.y * 0.5;
    if (abs(gl_FragCoord.y - viewportCenterY) < 2.0) {
        hdrColor = vec4(1.0, 0.0, 1.0, 1.0);
    }
    
    // Green horizontal - ball Y (transformed)
    float greenScreenY = ((uRawBallPos.y - 1.0 + center.y) * zoom + 0.5) * uViewportSize.y;
    if (abs(gl_FragCoord.y - greenScreenY) < 2.0) {
        hdrColor = vec4(0.0, 1.0, 0.0, 1.0);
    }
    
    // Cyan horizontal - smoothed camera Y (transformed)
    float cyanScreenY = ((uSmoothedCameraPos.y - 1.0 + center.y) * zoom + 0.5) * uViewportSize.y;
    if (abs(gl_FragCoord.y - cyanScreenY) < 2.0) {
        hdrColor = vec4(0.0, 1.0, 1.0, 1.0);
    }
    */
    
    // Gentle gamma lift to darken midtones slightly
    vec3 darkenedColor = pow(hdrColor.rgb, vec3(1.15));
    
    // Calculate luminance for selective processing
    float lum = dot(darkenedColor, vec3(0.2126, 0.7152, 0.0722));
    
    // Saturation boost that scales strongly with brightness
    // Darks: no change (factor ~1.0), Brights: significant boost (factor up to 1.6)
    float satBoost = smoothstep(0.25, 0.75, lum) * 0.6;
    vec3 saturatedColor = mix(vec3(lum), darkenedColor, 1.0 + satBoost);
    
    // Strong gamut expansion for bright pixels only
    // Uses a steeper curve so only brighter pixels get expanded
    float gamutAmount = smoothstep(0.3, 0.7, lum) * 0.8;
    mat3 expand = mat3(
        1.0 + 0.2 * gamutAmount, -0.1 * gamutAmount, -0.1 * gamutAmount,
        -0.1 * gamutAmount, 1.0 + 0.2 * gamutAmount, -0.1 * gamutAmount,
        -0.1 * gamutAmount, -0.1 * gamutAmount, 1.0 + 0.2 * gamutAmount
    );
    vec3 expandedColor = expand * saturatedColor;
    
    // No additional contrast adjustment
    
    // Full HDR PQ path for BT.2020 PQ surface
    // HDR color is in linear space, normalized to SDR white = 1.0
    // Convert to absolute nits, then normalize to 10000 nits for PQ
    vec3 linearNits = expandedColor * uSDRWhiteNits;
    vec3 linearNormalized = linearNits / 10000.0;
    
    // Clamp to display max
    linearNormalized = clamp(linearNormalized, vec3(0.0), vec3(uMaxNits / 10000.0));
    
    // Convert to BT.2020 color space
    vec3 bt2020 = bt709ToBt2020(linearNormalized);
    
    // Apply PQ transfer function
    vec3 pqEncoded = linearToPQ(bt2020);
    
    fragColor = vec4(pqEncoded, hdrColor.a);
}
)";

bool HDRRenderer::Init(int width, int height) {
    if (s_initialized) {
        Uninit();
    }
    
    if (!HDR::IsHDRActive()) {
        HDR_LOG("HDR not active, skipping HDR renderer initialization");
        return false;
    }
    
    s_width = width;
    s_height = height;
    
    HDR_LOG("Initializing HDR renderer: %dx%d", width, height);
    
    // Create FP16 HDR framebuffer
    glGenFramebuffers(1, &s_hdrFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, s_hdrFBO);
    
    // Create FP16 texture for HDR rendering with nearest neighbor filtering
    glGenTextures(1, &s_hdrTexture);
    glBindTexture(GL_TEXTURE_2D, s_hdrTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_HALF_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, s_hdrTexture, 0);
    
    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE) {
        HDR_ERR("HDR framebuffer incomplete: 0x%x", status);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        return false;
    }
    
    // Create SDR input texture with nearest neighbor filtering
    glGenTextures(1, &s_sdrTexture);
    glBindTexture(GL_TEXTURE_2D, s_sdrTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    
    // Create shaders
    s_uploadProgram = CreateProgram(s_vertexShaderSrc, s_hdrUploadFragmentSrc);
    if (s_uploadProgram == 0) {
        HDR_ERR("Failed to create HDR upload shader program");
        return false;
    }
    
    s_outputProgram = CreateProgram(s_vertexShaderSrc, s_pqOutputFragmentSrc);
    if (s_outputProgram == 0) {
        HDR_ERR("Failed to create PQ output shader program");
        return false;
    }
    
    // Create fullscreen quad
    CreateFullscreenQuad();
    
    // Cache uniform locations once at init time
    CacheUniformLocations();
    
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindTexture(GL_TEXTURE_2D, 0);
    
    s_initialized = true;
    HDR_LOG("HDR renderer initialized successfully");
    
    return true;
}

void HDRRenderer::CacheUniformLocations() {
    // Cache output program uniforms
    s_loc_uHDRTexture = glGetUniformLocation(s_outputProgram, "uHDRTexture");
    s_loc_uMaxNits = glGetUniformLocation(s_outputProgram, "uMaxNits");
    s_loc_uSDRWhiteNits = glGetUniformLocation(s_outputProgram, "uSDRWhiteNits");
    s_loc_uCameraZoom = glGetUniformLocation(s_outputProgram, "uCameraZoom");
    s_loc_uCameraCenter = glGetUniformLocation(s_outputProgram, "uCameraCenter");
    s_loc_uViewportSize = glGetUniformLocation(s_outputProgram, "uViewportSize");
    s_loc_uViewportOffset = glGetUniformLocation(s_outputProgram, "uViewportOffset");
    s_loc_uRawBallPos = glGetUniformLocation(s_outputProgram, "uRawBallPos");
    s_loc_uLastBallPos = glGetUniformLocation(s_outputProgram, "uLastBallPos");
    s_loc_uBallValid = glGetUniformLocation(s_outputProgram, "uBallValid");
    s_loc_uSmoothedCameraPos = glGetUniformLocation(s_outputProgram, "uSmoothedCameraPos");
    
    // Cache upload program uniforms
    s_loc_upload_uTexture = glGetUniformLocation(s_uploadProgram, "uTexture");
    s_loc_upload_uIntensityMultiplier = glGetUniformLocation(s_uploadProgram, "uIntensityMultiplier");
    s_loc_upload_uExposure = glGetUniformLocation(s_uploadProgram, "uExposure");
    
    HDR_LOG("Cached uniform locations: tex=%d, maxNits=%d, zoom=%d, uploadTex=%d", 
            s_loc_uHDRTexture, s_loc_uMaxNits, s_loc_uCameraZoom, s_loc_upload_uTexture);
}

void HDRRenderer::Uninit() {
    if (s_hdrFBO) {
        glDeleteFramebuffers(1, &s_hdrFBO);
        s_hdrFBO = 0;
    }
    if (s_hdrTexture) {
        glDeleteTextures(1, &s_hdrTexture);
        s_hdrTexture = 0;
    }
    if (s_sdrTexture) {
        glDeleteTextures(1, &s_sdrTexture);
        s_sdrTexture = 0;
    }
    if (s_uploadProgram) {
        glDeleteProgram(s_uploadProgram);
        s_uploadProgram = 0;
    }
    if (s_outputProgram) {
        glDeleteProgram(s_outputProgram);
        s_outputProgram = 0;
    }
    if (s_quadVAO) {
        glDeleteVertexArrays(1, &s_quadVAO);
        s_quadVAO = 0;
    }
    if (s_quadVBO) {
        glDeleteBuffers(1, &s_quadVBO);
        s_quadVBO = 0;
    }
    
    s_initialized = false;
    HDR_LOG("HDR renderer uninitialized");
}

bool HDRRenderer::IsInitialized() {
    return s_initialized;
}

bool HDRRenderer::ShouldUseHDR() {
    return s_initialized && HDR::IsHDRActive();
}

void HDRRenderer::UploadTexture(const ColorRgba* pixels, int width, int height) {
    if (!s_initialized) {
        HDR_LOG("UploadTexture: not initialized");
        return;
    }
    
    // Clear any pending GL errors
    while (glGetError() != GL_NO_ERROR) {}
    
    // Check if texture size matches - resize if needed
    if (width != s_width || height != s_height) {
        HDR_LOG("UploadTexture: resizing textures from %dx%d to %dx%d", s_width, s_height, width, height);
        s_width = width;
        s_height = height;
        
        // Resize SDR texture with nearest neighbor filtering
        glBindTexture(GL_TEXTURE_2D, s_sdrTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        
        // Resize HDR texture with nearest neighbor filtering
        glBindTexture(GL_TEXTURE_2D, s_hdrTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_HALF_FLOAT, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    }
    
    // Upload SDR pixels to texture using glTexSubImage2D (faster - no reallocation)
    // Note: ColorRgba is BGRA format - we swap R/B in the shader
    glBindTexture(GL_TEXTURE_2D, s_sdrTexture);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
    
    GLenum err = glGetError();
    if (err != GL_NO_ERROR) {
        HDR_ERR("GL error after texture upload: 0x%x", err);
    }
    
    // Render to HDR framebuffer with default intensity (SDR white)
    glBindFramebuffer(GL_FRAMEBUFFER, s_hdrFBO);
    glViewport(0, 0, s_width, s_height);
    glClear(GL_COLOR_BUFFER_BIT);
    
    glUseProgram(s_uploadProgram);
    
    // Set uniforms using cached locations
    glUniform1i(s_loc_upload_uTexture, 0);
    glUniform1f(s_loc_upload_uIntensityMultiplier, 1.0f);  // Default SDR intensity
    glUniform1f(s_loc_upload_uExposure, s_exposure);
    
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, s_sdrTexture);
    
    // Draw fullscreen quad
    glBindVertexArray(s_quadVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
    
    err = glGetError();
    if (err != GL_NO_ERROR) {
        HDR_ERR("GL error after FBO render: 0x%x", err);
    }
    
    // Light overlays are now rendered in Present() after PQ encoding
    
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void HDRRenderer::ApplyHDRIntensity(int x, int y, int width, int height, float intensityNits) {
    if (!s_initialized) return;
    
    // Convert nits to linear multiplier (relative to SDR white)
    float linearIntensity = HDR::ColorSpace::NitsToLinear(intensityNits);
    
    // This would require a more complex implementation with scissor test
    // or a separate render pass for light regions
    // For now, this is a placeholder for per-region HDR intensity
    
    HDR_LOG("ApplyHDRIntensity: region (%d,%d,%d,%d) intensity=%.1f nits (linear=%.2f)", 
            x, y, width, height, intensityNits, linearIntensity);
}

void HDRRenderer::Present(int screenWidth, int screenHeight) {
    if (!s_initialized) {
        HDR_LOG("Present called but not initialized");
        return;
    }
    
    // Calculate aspect-ratio-preserving viewport (fit, not stretch)
    float texAspect = (float)s_width / (float)s_height;
    float screenAspect = (float)screenWidth / (float)screenHeight;
    
    int viewportX, viewportY, viewportW, viewportH;
    
    if (texAspect > screenAspect) {
        // Texture is wider - fit to width, letterbox top/bottom
        viewportW = screenWidth;
        viewportH = (int)(screenWidth / texAspect);
        viewportX = 0;
        viewportY = (screenHeight - viewportH) / 2;
    } else {
        // Texture is taller - fit to height, pillarbox left/right
        viewportH = screenHeight;
        viewportW = (int)(screenHeight * texAspect);
        viewportX = (screenWidth - viewportW) / 2;
        viewportY = 0;
    }
    
    // HDR_LOG("Present: screen %dx%d, texture %dx%d, viewport %d,%d %dx%d", 
    //        screenWidth, screenHeight, s_width, s_height, viewportX, viewportY, viewportW, viewportH);
    
    // Store viewport for touch coordinate conversion
    s_viewportX = viewportX;
    s_viewportY = viewportY;
    s_viewportW = viewportW;
    s_viewportH = viewportH;
    
    // Bind default framebuffer (screen)
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    
    // Clear the entire screen first (for letterbox/pillarbox areas)
    glViewport(0, 0, screenWidth, screenHeight);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    
    // Set viewport to maintain aspect ratio
    glViewport(viewportX, viewportY, viewportW, viewportH);
    
    glUseProgram(s_outputProgram);
    
    // Set uniforms using cached locations (avoid glGetUniformLocation every frame)
    glUniform1i(s_loc_uHDRTexture, 0);
    glUniform1f(s_loc_uMaxNits, HDR::GetMaxDisplayNits());
    glUniform1f(s_loc_uSDRWhiteNits, HDR::Luminance::SDR_WHITE_NITS);
    
    // Set viewport size and offset for screen-space calculations
    glUniform2f(s_loc_uViewportSize, (float)viewportW, (float)viewportH);
    glUniform2f(s_loc_uViewportOffset, (float)viewportX, (float)viewportY);
    
    // DEBUG: Log viewport info - disabled due to spam
    // static int vpLogCount = 0;
    // if (++vpLogCount >= 120) {
    //     vpLogCount = 0;
    //     HDR_LOG("Viewport: offset=(%d,%d) size=(%d,%d) screen=(%d,%d) uniformLocs=(%d,%d)", 
    //             viewportX, viewportY, viewportW, viewportH, screenWidth, screenHeight,
    //             viewportSizeLoc, viewportOffsetLoc);
    // }
    
    // Store current camera state for light overlay rendering
    float currentZoom = 1.0f;
    float cameraCenterX = 0.5f;
    float cameraCenterY = 0.5f;
    
    // Variables for camera tracking (declared outside conditional)
    float ballX = 0.3f, ballY = 0.5f;
    bool ballValid = false;
    static float lastValidBallX = 0.3f;
    static float lastValidBallY = 0.5f;
    static float smoothedCameraX = 0.3f;
    static float smoothedCameraY = 0.5f;
    
    // Only apply camera tracking if enabled
    if (s_cameraTrackingEnabled) {
        // DEBUG: Disable zoom but still pass ball position for green line
        ballX = HDRLightOverlay::GetBallX();
        ballY = HDRLightOverlay::GetBallY();
        ballValid = HDRLightOverlay::IsBallValid();
        
        // Only update last valid position if ball is valid
        if (ballValid) {
            lastValidBallX = ballX;
            lastValidBallY = ballY;
        }
        
        // Calculate target position (use last valid when ball is invalid)
        float targetX = ballValid ? ballX : lastValidBallX;
        float targetY = ballValid ? ballY : lastValidBallY;
        
        // Table center in texture coordinates (center of table, not including scoreboard)
        // Table is ~60.7% of texture width, so center is at 0.607/2 = 0.3035
        float tableCenterX = 0.3035f;
        float tableCenterY = 0.5f;
        
        // Bias target 25% toward table center
        // float biasedTargetX = targetX * 0.75f + tableCenterX * 0.25f;
        // float biasedTargetY = targetY * 0.75f + tableCenterY * 0.25f;
        // 50% toward table center
        float biasedTargetX = targetX * 0.5f + tableCenterX * 0.5f;
        float biasedTargetY = targetY * 0.5f + tableCenterY * 0.5f;
        
        // Smooth the camera position (lerp toward biased target)
        // float smoothFactor = 0.08f;  // Lower = smoother/slower
        float smoothFactor = 0.03f;
        smoothedCameraX += (biasedTargetX - smoothedCameraX) * smoothFactor;
        smoothedCameraY += (biasedTargetY - smoothedCameraY) * smoothFactor;
        
        // Follow cyan (smoothedCamera) - invert both X and Y
        cameraCenterX = 1.0f - smoothedCameraX -0.2f / s_cameraZoom;  // Invert X and add offset
        cameraCenterY = 1.0f - smoothedCameraY;  // Invert Y
        currentZoom = s_cameraZoom;
        
        // Also pass raw ball position for green debug line
        glUniform2f(s_loc_uRawBallPos, ballX, ballY);
    }
    
    // Set camera tracking uniforms using cached locations
    glUniform1f(s_loc_uCameraZoom, currentZoom);
    glUniform2f(s_loc_uCameraCenter, cameraCenterX, cameraCenterY);
    
    // Pass last valid position and validity flag
    glUniform2f(s_loc_uLastBallPos, lastValidBallX, lastValidBallY);
    glUniform1f(s_loc_uBallValid, ballValid ? 1.0f : 0.0f);
    glUniform2f(s_loc_uSmoothedCameraPos, smoothedCameraX, smoothedCameraY);
    
    // Store camera state for light overlay - use SAME position as main texture
    s_currentCameraZoom = currentZoom;
    s_currentCameraCenterX = cameraCenterX;  // Same as what we pass to the shader
    s_currentCameraCenterY = cameraCenterY;
    
    // HDR_LOG("Present: maxNits=%.1f, sdrWhite=%.1f", HDR::GetMaxDisplayNits(), HDR::Luminance::SDR_WHITE_NITS);
    
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, s_hdrTexture);
    
    // Disable depth test and blending for fullscreen quad
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
    
    // Profiling
    static float maxQuadMs = 0, maxOverlayMs = 0;
    static int presentProfileCounter = 0;
    auto tQuadStart = std::chrono::steady_clock::now();
    
    // Draw fullscreen quad
    glBindVertexArray(s_quadVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
    
    glUseProgram(0);
    
    auto tQuadEnd = std::chrono::steady_clock::now();
    
    // Render HDR light overlays on top of the PQ-encoded output
    // These need to be rendered directly to screen with their own PQ encoding
    HDRLightOverlay::UpdateLightStates();
    if (HDRLightOverlay::HasActiveLights()) {
        // Pass viewport info so overlays render in the correct position
        HDRLightOverlay::RenderOverlaysPQ(viewportX, viewportY, viewportW, viewportH, 
                                          s_width, s_height, HDR::GetMaxDisplayNits());
    }
    
    auto tOverlayEnd = std::chrono::steady_clock::now();
    
    // Track max times
    float quadMs = std::chrono::duration<float, std::milli>(tQuadEnd - tQuadStart).count();
    float overlayMs = std::chrono::duration<float, std::milli>(tOverlayEnd - tQuadEnd).count();
    if (quadMs > maxQuadMs) maxQuadMs = quadMs;
    if (overlayMs > maxOverlayMs) maxOverlayMs = overlayMs;
    
    if (++presentProfileCounter >= 120) {
        HDR_LOG("Present breakdown - Quad: %.1fms, Overlays: %.1fms", maxQuadMs, maxOverlayMs);
        presentProfileCounter = 0;
        maxQuadMs = maxOverlayMs = 0;
    }
    
    // Check for GL errors
    GLenum err = glGetError();
    if (err != GL_NO_ERROR) {
        HDR_ERR("GL error in Present: 0x%x", err);
    }
}

void HDRRenderer::SetExposure(float exposure) {
    s_exposure = exposure;
}

void HDRRenderer::SetCameraTracking(bool enabled, float zoom) {
    s_cameraTrackingEnabled = enabled;
    s_cameraZoom = zoom;
    HDR_LOG("Camera tracking %s, zoom=%.2fx", enabled ? "enabled" : "disabled", zoom);
}

GLuint HDRRenderer::CompileShader(GLenum type, const char* source) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);
    
    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(shader, 512, nullptr, infoLog);
        HDR_ERR("Shader compilation failed: %s", infoLog);
        glDeleteShader(shader);
        return 0;
    }
    
    return shader;
}

GLuint HDRRenderer::CreateProgram(const char* vertexSrc, const char* fragmentSrc) {
    GLuint vertexShader = CompileShader(GL_VERTEX_SHADER, vertexSrc);
    if (vertexShader == 0) return 0;
    
    GLuint fragmentShader = CompileShader(GL_FRAGMENT_SHADER, fragmentSrc);
    if (fragmentShader == 0) {
        glDeleteShader(vertexShader);
        return 0;
    }
    
    GLuint program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);
    
    GLint success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(program, 512, nullptr, infoLog);
        HDR_ERR("Program linking failed: %s", infoLog);
        glDeleteProgram(program);
        program = 0;
    }
    
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    
    return program;
}

void HDRRenderer::CreateFullscreenQuad() {
    // Fullscreen quad vertices: position (x,y) and texcoord (u,v)
    float quadVertices[] = {
        // Position    // TexCoord
        -1.0f,  1.0f,  0.0f, 0.0f,  // top-left
        -1.0f, -1.0f,  0.0f, 1.0f,  // bottom-left
         1.0f,  1.0f,  1.0f, 0.0f,  // top-right
         1.0f, -1.0f,  1.0f, 1.0f,  // bottom-right
    };
    
    glGenVertexArrays(1, &s_quadVAO);
    glGenBuffers(1, &s_quadVBO);
    
    glBindVertexArray(s_quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, s_quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);
    
    // Position attribute
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    // TexCoord attribute
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);
    
    glBindVertexArray(0);
}
