#include "pch.h"
#include "HDRLightOverlay.h"
#include "TLightGroup.h"
#include "TLight.h"
#include "HDRConfig.h"
#include <cmath>
#include <cstdio>
#include <cstring>

#ifdef __ANDROID__
#include <android/log.h>
#define HDRLIGHT_LOG(...) __android_log_print(ANDROID_LOG_INFO, "HDRLightOverlay", __VA_ARGS__)
#else
#define HDRLIGHT_LOG(...)
#endif

// Static member initialization
std::vector<HDRLightOverlay::RegisteredGroup> HDRLightOverlay::s_registeredGroups;
std::vector<HDRLightOverlay::RegisteredLight> HDRLightOverlay::s_registeredLights;
std::vector<HDRLightConfig> HDRLightOverlay::s_lightConfigs;
std::vector<HDRLightOverlay::LightState> HDRLightOverlay::s_lightStates;
std::vector<HDRLightOverlay::TestLight> HDRLightOverlay::s_testLights;
bool HDRLightOverlay::s_initialized = false;
GLuint HDRLightOverlay::s_overlayProgram = 0;
GLuint HDRLightOverlay::s_overlayProgramPQ = 0;
GLuint HDRLightOverlay::s_overlayVAO = 0;
GLuint HDRLightOverlay::s_overlayVBO = 0;
static bool s_debugAllLightsOn = false;  // Debug mode - shows all lights regardless of state
static bool s_editMode = false;  // Edit mode - allows dragging lights to reposition
static int s_selectedLightIndex = -1;
static float s_dragOffsetX = 0.0f;
static float s_dragOffsetY = 0.0f;

// Vertex shader for light overlay
// Simple absolute positioning - no scaling or compensation
static const char* s_overlayVertexSrc = R"(#version 300 es
precision highp float;
layout(location = 0) in vec2 aPos;
layout(location = 1) in vec2 aTexCoord;

uniform vec4 uLightRect;  // x, y, width, height in normalized coords (0-1)

out vec2 vLocalCoord;  // 0-1 within the light quad

void main() {
    // aPos is -1 to 1, convert to 0-1
    vec2 localPos = aPos * 0.5 + 0.5;
    
    // Simple absolute positioning
    // uLightRect.xy = center position (0-1)
    // uLightRect.zw = width, height (0-1)
    vec2 pos;
    pos.x = uLightRect.x + (localPos.x - 0.5) * uLightRect.z;
    pos.y = uLightRect.y + (localPos.y - 0.5) * uLightRect.w;
    
    // Convert to clip space (-1 to 1)
    // 0 -> -1, 1 -> +1 for X
    // 0 -> +1, 1 -> -1 for Y (flip for top-left origin)
    pos.x = pos.x * 2.0 - 1.0;
    pos.y = 1.0 - pos.y * 2.0;
    
    gl_Position = vec4(pos, 0.0, 1.0);
    vLocalCoord = aTexCoord;
}
)";

// Fragment shader for HDR light glow (linear output, for FBO rendering)
static const char* s_overlayFragmentSrc = R"(#version 300 es
precision highp float;

in vec2 vLocalCoord;
out vec4 fragColor;

uniform vec3 uLightColor;      // Linear HDR color
uniform float uIntensity;      // Intensity multiplier (nits / SDR_WHITE)
uniform float uGlowRadius;     // Glow falloff radius

void main() {
    // Calculate distance from center for radial gradient
    vec2 center = vec2(0.5, 0.5);
    float dist = length(vLocalCoord - center) * 2.0;
    
    // Smooth radial falloff with glow - brighter core
    float coreFalloff = 1.0 - smoothstep(0.0, 0.2, dist);
    float glowFalloff = 1.0 - smoothstep(0.2, 1.0, dist);
    
    float alpha = coreFalloff * 1.0 + glowFalloff * 0.6;
    alpha *= uGlowRadius;
    alpha = clamp(alpha, 0.0, 1.0);
    
    vec3 hdrColor = uLightColor * uIntensity * 2.0;
    
    fragColor = vec4(hdrColor, alpha);
}
)";

// Fragment shader for HDR light glow with PQ encoding (for direct screen output)
static const char* s_overlayFragmentPQSrc = R"(#version 300 es
precision highp float;

in vec2 vLocalCoord;
out vec4 fragColor;

uniform vec3 uLightColor;      // Linear color (0-1 for sRGB primaries)
uniform float uIntensityNits;  // Intensity in nits
uniform float uGlowRadius;     // Glow falloff radius
uniform float uMaxNits;        // Display max nits

// PQ constants
const float m1 = 0.1593017578125;
const float m2 = 78.84375;
const float c1 = 0.8359375;
const float c2 = 18.8515625;
const float c3 = 18.6875;

vec3 linearToPQ(vec3 linear) {
    // Input is in nits, normalize to 0-1 range for 10000 nits
    vec3 Y = linear / 10000.0;
    Y = max(Y, vec3(0.0));
    
    vec3 Ym1 = pow(Y, vec3(m1));
    vec3 numerator = c1 + c2 * Ym1;
    vec3 denominator = 1.0 + c3 * Ym1;
    return pow(numerator / denominator, vec3(m2));
}

void main() {
    // Calculate distance from center for radial gradient
    vec2 center = vec2(0.5, 0.5);
    float dist = length(vLocalCoord - center) * 2.0;
    
    // Smooth radial falloff with intense bright core
    float coreFalloff = 1.0 - smoothstep(0.0, 0.15, dist);  // Very tight bright core
    float glowFalloff = 1.0 - smoothstep(0.15, 1.0, dist);  // Soft glow
    
    // Combine - strong core, softer glow
    float alpha = coreFalloff * 1.0 + glowFalloff * 0.5;
    alpha *= uGlowRadius;
    alpha = clamp(alpha, 0.0, 1.0);
    
    // For saturated HDR colors at full panel capability:
    // Scale the color so the brightest channel hits uMaxNits
    // This preserves color saturation while maximizing brightness
    float maxChannel = max(max(uLightColor.r, uLightColor.g), uLightColor.b);
    vec3 hdrColorNits = (uLightColor / maxChannel) * uMaxNits;
    
    // Convert to PQ
    vec3 pqColor = linearToPQ(hdrColorNits);
    
    // Output with alpha for blending
    fragColor = vec4(pqColor, alpha);
}
)";

void HDRLightOverlay::Init() {
    if (s_initialized) return;
    
    HDRLIGHT_LOG("Initializing HDR light overlay system");
    
    CreateShaders();
    CreateQuad();
    
    s_initialized = true;
}

void HDRLightOverlay::Uninit() {
    if (!s_initialized) return;
    
    if (s_overlayProgram) {
        glDeleteProgram(s_overlayProgram);
        s_overlayProgram = 0;
    }
    if (s_overlayVAO) {
        glDeleteVertexArrays(1, &s_overlayVAO);
        s_overlayVAO = 0;
    }
    if (s_overlayVBO) {
        glDeleteBuffers(1, &s_overlayVBO);
        s_overlayVBO = 0;
    }
    
    s_registeredGroups.clear();
    s_registeredLights.clear();
    s_lightConfigs.clear();
    s_lightStates.clear();
    s_initialized = false;
}

void HDRLightOverlay::RegisterLightGroup(const char* groupName, TLightGroup* group) {
    if (!group) return;
    
    // Check if already registered
    for (const auto& reg : s_registeredGroups) {
        if (strcmp(reg.name, groupName) == 0) return;
    }
    
    s_registeredGroups.push_back({groupName, group});
    HDRLIGHT_LOG("Registered light group: %s with %zu lights", groupName, group->List.size());
}

void HDRLightOverlay::RegisterIndividualLight(const char* lightName, TLight* light) {
    if (!light) return;
    
    // Check if already registered
    for (const auto& reg : s_registeredLights) {
        if (strcmp(reg.name, lightName) == 0) return;
    }
    
    s_registeredLights.push_back({lightName, light});
    HDRLIGHT_LOG("Registered individual light: %s", lightName);
}

void HDRLightOverlay::AddLightConfig(const HDRLightConfig& config) {
    s_lightConfigs.push_back(config);
    HDRLIGHT_LOG("Added light config: %s[%d] at (%.2f, %.2f)", 
                 config.GroupName, config.LightIndex, config.X, config.Y);
}

void HDRLightOverlay::AddTestLight(float x, float y, float w, float h, float r, float g, float b, float intensity) {
    s_testLights.push_back({x, y, w, h, r, g, b, intensity});
    HDRLIGHT_LOG("Added test light at (%.2f, %.2f) size (%.2f, %.2f) color (%.1f, %.1f, %.1f) intensity %.0f",
                 x, y, w, h, r, g, b, intensity);
}

void HDRLightOverlay::SetDebugAllLightsOn(bool enabled) {
    s_debugAllLightsOn = enabled;
    HDRLIGHT_LOG("Debug all lights on: %s", enabled ? "true" : "false");
}

bool HDRLightOverlay::GetDebugAllLightsOn() {
    return s_debugAllLightsOn;
}

void HDRLightOverlay::UpdateLightStates() {
    s_lightStates.clear();
    
    static int frameCount = 0;
    frameCount++;
    bool shouldLog = (frameCount % 60 == 0);  // Log once per second at 60fps
    
    if (shouldLog) {
        HDRLIGHT_LOG("UpdateLightStates: %zu configs, %zu groups registered", 
                     s_lightConfigs.size(), s_registeredGroups.size());
    }
    
    for (const auto& config : s_lightConfigs) {
        // First try to find as a light group
        TLightGroup* group = nullptr;
        TLight* light = nullptr;
        
        for (const auto& reg : s_registeredGroups) {
            if (strcmp(reg.name, config.GroupName) == 0) {
                group = reg.group;
                break;
            }
        }
        
        // If found as a group, get the light from the group
        if (group) {
            if (config.LightIndex >= (int)group->List.size()) {
                if (shouldLog) HDRLIGHT_LOG("  Config %s[%d]: index out of range (size=%zu)", 
                                            config.GroupName, config.LightIndex, group->List.size());
                continue;
            }
            light = group->List[config.LightIndex];
        } else {
            // Try to find as an individual light (LightIndex should be 0 for individual lights)
            for (const auto& reg : s_registeredLights) {
                if (strcmp(reg.name, config.GroupName) == 0) {
                    light = reg.light;
                    break;
                }
            }
        }
        
        if (!light) {
            if (shouldLog) HDRLIGHT_LOG("  Config %s[%d]: light not found", config.GroupName, config.LightIndex);
            continue;
        }
        
        LightState state;
        state.config = &config;
        state.light = light;
        state.isOn = (light->BmpIndex1 == 1);
        state.isFlashing = (light->FlasherActive != 0);
        
        // Check if the light group is in rotation animation mode (Message 26/27)
        // During rotation, FlasherFlag2 indicates which light is "lit"
        bool isRotating = group && (group->MessageField2 == 26 || group->MessageField2 == 27);
        bool rotationLit = (light->FlasherFlag2 != 0);
        
        // Calculate current intensity
        if (s_debugAllLightsOn || s_editMode) {
            // Debug mode or edit mode - all lights at full intensity for visibility
            state.currentIntensity = config.IntensityOn;
        } else if (isRotating) {
            // Rotation animation - use FlasherFlag2 to determine which light is lit
            state.currentIntensity = rotationLit ? config.IntensityOn : 0.0f;
        } else if (state.isFlashing) {
            // Flashing - use flash intensity when lit
            state.currentIntensity = (light->Flasher.BmpIndex == 1) ? 
                config.IntensityFlash : 0.0f;
        } else if (state.isOn) {
            state.currentIntensity = config.IntensityOn;
        } else {
            state.currentIntensity = 0.0f;
        }
        
        if (shouldLog) {
            HDRLIGHT_LOG("  Light %s[%d]: on=%d, flashing=%d, flashIdx=%d, intensity=%.0f, debug=%d", 
                         config.GroupName, config.LightIndex, 
                         state.isOn, state.isFlashing, 
                         state.isFlashing ? light->Flasher.BmpIndex : -1,
                         state.currentIntensity, s_debugAllLightsOn);
        }
        
        // Only add if light is actually on (or debug mode)
        if (state.currentIntensity > 0.0f) {
            s_lightStates.push_back(state);
        }
    }
    
    if (shouldLog && !s_lightStates.empty()) {
        HDRLIGHT_LOG("  Active lights: %zu", s_lightStates.size());
    }
}

bool HDRLightOverlay::HasActiveLights() {
    return !s_lightStates.empty() || !s_testLights.empty();
}

void HDRLightOverlay::RenderOverlays(int textureWidth, int textureHeight) {
    // Lazy initialization - create shaders on first render when GL context is ready
    if (!s_initialized) {
        HDRLIGHT_LOG("RenderOverlays: lazy init");
        CreateShaders();
        CreateQuad();
        s_initialized = true;
    }
    
    if (s_lightStates.empty() && s_testLights.empty()) {
        return;
    }
    
    HDRLIGHT_LOG("RenderOverlays: rendering %zu game lights + %zu test lights", 
                 s_lightStates.size(), s_testLights.size());
    
    // Enable blending for additive light overlay
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);  // Additive blending
    
    glUseProgram(s_overlayProgram);
    glBindVertexArray(s_overlayVAO);
    
    // Render game lights
    for (const auto& state : s_lightStates) {
        RenderSingleLight(state, textureWidth, textureHeight);
    }
    
    // Render test lights (always on)
    for (const auto& test : s_testLights) {
        float aspectRatio = (float)textureWidth / (float)textureHeight;
        GLint aspectLoc = glGetUniformLocation(s_overlayProgram, "uAspectRatio");
        glUniform1f(aspectLoc, aspectRatio);
        
        GLint rectLoc = glGetUniformLocation(s_overlayProgram, "uLightRect");
        glUniform4f(rectLoc, test.x, test.y, test.w, test.h);
        
        GLint colorLoc = glGetUniformLocation(s_overlayProgram, "uLightColor");
        glUniform3f(colorLoc, test.r, test.g, test.b);
        
        GLint intensityLoc = glGetUniformLocation(s_overlayProgram, "uIntensity");
        float linearIntensity = test.intensity / 203.0f;  // SDR_WHITE_NITS
        glUniform1f(intensityLoc, linearIntensity);
        
        GLint glowLoc = glGetUniformLocation(s_overlayProgram, "uGlowRadius");
        glUniform1f(glowLoc, 1.0f);
        
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    }
    
    glBindVertexArray(0);
    glDisable(GL_BLEND);
}

void HDRLightOverlay::RenderSingleLight(const LightState& state, int texWidth, int texHeight) {
    const HDRLightConfig* config = state.config;
    
    // Set light rectangle uniform (normalized 0-1 coords)
    GLint rectLoc = glGetUniformLocation(s_overlayProgram, "uLightRect");
    glUniform4f(rectLoc, config->X, config->Y, config->Width, config->Height);
    
    // Set color uniform (linear RGB) - boost saturation
    GLint colorLoc = glGetUniformLocation(s_overlayProgram, "uLightColor");
    glUniform3f(colorLoc, config->R, config->G, config->B);
    
    // Set intensity (convert nits to linear multiplier relative to SDR white)
    GLint intensityLoc = glGetUniformLocation(s_overlayProgram, "uIntensity");
    float linearIntensity = state.currentIntensity / HDR::Luminance::SDR_WHITE_NITS;
    glUniform1f(intensityLoc, linearIntensity);
    
    // Set glow radius
    GLint glowLoc = glGetUniformLocation(s_overlayProgram, "uGlowRadius");
    glUniform1f(glowLoc, config->GlowRadius);
    
    // Draw the quad
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

void HDRLightOverlay::RenderOverlaysPQ(int viewportX, int viewportY, int viewportW, int viewportH,
                                        int texWidth, int texHeight, float maxNits) {
    // Lazy initialization
    if (!s_initialized) {
        HDRLIGHT_LOG("RenderOverlaysPQ: lazy init");
        CreateShaders();
        CreateQuad();
        s_initialized = true;
    }
    
    if (s_lightStates.empty() && s_testLights.empty()) {
        return;
    }
    
    static int pqFrameCount = 0;
    pqFrameCount++;
    if (pqFrameCount % 60 == 0) {
        HDRLIGHT_LOG("RenderOverlaysPQ: rendering %zu game lights + %zu test lights, viewport %d,%d %dx%d, configs=%zu, groups=%zu",
                     s_lightStates.size(), s_testLights.size(), viewportX, viewportY, viewportW, viewportH,
                     s_lightConfigs.size(), s_registeredGroups.size());
    }
    
    // Set viewport to match the game area
    glViewport(viewportX, viewportY, viewportW, viewportH);
    
    // Enable blending for additive light overlay
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);  // Additive blending
    
    glUseProgram(s_overlayProgramPQ);
    glBindVertexArray(s_overlayVAO);
    
    // Render game lights with PQ encoding
    for (const auto& state : s_lightStates) {
        RenderSingleLightPQ(state, maxNits);
    }
    
    // Render test lights with PQ encoding
    for (const auto& test : s_testLights) {
        GLint rectLoc = glGetUniformLocation(s_overlayProgramPQ, "uLightRect");
        glUniform4f(rectLoc, test.x, test.y, test.w, test.h);
        
        GLint colorLoc = glGetUniformLocation(s_overlayProgramPQ, "uLightColor");
        glUniform3f(colorLoc, test.r, test.g, test.b);
        
        GLint intensityLoc = glGetUniformLocation(s_overlayProgramPQ, "uIntensityNits");
        glUniform1f(intensityLoc, test.intensity);
        
        GLint maxNitsLoc = glGetUniformLocation(s_overlayProgramPQ, "uMaxNits");
        glUniform1f(maxNitsLoc, maxNits);
        
        GLint glowLoc = glGetUniformLocation(s_overlayProgramPQ, "uGlowRadius");
        glUniform1f(glowLoc, 1.0f);
        
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    }
    
    glBindVertexArray(0);
    glDisable(GL_BLEND);
}

void HDRLightOverlay::RenderSingleLightPQ(const LightState& state, float maxNits) {
    const HDRLightConfig* config = state.config;
    
    // Set light rectangle uniform (normalized 0-1 coords)
    GLint rectLoc = glGetUniformLocation(s_overlayProgramPQ, "uLightRect");
    glUniform4f(rectLoc, config->X, config->Y, config->Width, config->Height);
    
    // Set color uniform (linear RGB, saturated)
    GLint colorLoc = glGetUniformLocation(s_overlayProgramPQ, "uLightColor");
    glUniform3f(colorLoc, config->R, config->G, config->B);
    
    // Set intensity in nits directly
    GLint intensityLoc = glGetUniformLocation(s_overlayProgramPQ, "uIntensityNits");
    glUniform1f(intensityLoc, state.currentIntensity);
    
    // Set max nits for clamping
    GLint maxNitsLoc = glGetUniformLocation(s_overlayProgramPQ, "uMaxNits");
    glUniform1f(maxNitsLoc, maxNits);
    
    // Set glow radius
    GLint glowLoc = glGetUniformLocation(s_overlayProgramPQ, "uGlowRadius");
    glUniform1f(glowLoc, config->GlowRadius);
    
    // Draw the quad
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

void HDRLightOverlay::CreateShaders() {
    // Compile vertex shader (shared between both programs)
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &s_overlayVertexSrc, nullptr);
    glCompileShader(vertexShader);
    
    GLint success;
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(vertexShader, 512, nullptr, infoLog);
        HDRLIGHT_LOG("Vertex shader error: %s", infoLog);
    }
    
    // Compile fragment shader (linear output)
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &s_overlayFragmentSrc, nullptr);
    glCompileShader(fragmentShader);
    
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(fragmentShader, 512, nullptr, infoLog);
        HDRLIGHT_LOG("Fragment shader error: %s", infoLog);
    }
    
    // Link linear program
    s_overlayProgram = glCreateProgram();
    glAttachShader(s_overlayProgram, vertexShader);
    glAttachShader(s_overlayProgram, fragmentShader);
    glLinkProgram(s_overlayProgram);
    
    glGetProgramiv(s_overlayProgram, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(s_overlayProgram, 512, nullptr, infoLog);
        HDRLIGHT_LOG("Program link error: %s", infoLog);
    }
    
    glDeleteShader(fragmentShader);
    
    // Compile PQ fragment shader
    GLuint fragmentShaderPQ = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShaderPQ, 1, &s_overlayFragmentPQSrc, nullptr);
    glCompileShader(fragmentShaderPQ);
    
    glGetShaderiv(fragmentShaderPQ, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(fragmentShaderPQ, 512, nullptr, infoLog);
        HDRLIGHT_LOG("PQ Fragment shader error: %s", infoLog);
    }
    
    // Link PQ program
    s_overlayProgramPQ = glCreateProgram();
    glAttachShader(s_overlayProgramPQ, vertexShader);
    glAttachShader(s_overlayProgramPQ, fragmentShaderPQ);
    glLinkProgram(s_overlayProgramPQ);
    
    glGetProgramiv(s_overlayProgramPQ, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(s_overlayProgramPQ, 512, nullptr, infoLog);
        HDRLIGHT_LOG("PQ Program link error: %s", infoLog);
    }
    
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShaderPQ);
    
    HDRLIGHT_LOG("Created overlay shaders: linear=%d, PQ=%d", s_overlayProgram, s_overlayProgramPQ);
}

void HDRLightOverlay::CreateQuad() {
    // Simple quad vertices
    float quadVertices[] = {
        // Position    // TexCoord
        -1.0f, -1.0f,  0.0f, 0.0f,
        -1.0f,  1.0f,  0.0f, 1.0f,
         1.0f, -1.0f,  1.0f, 0.0f,
         1.0f,  1.0f,  1.0f, 1.0f,
    };
    
    glGenVertexArrays(1, &s_overlayVAO);
    glGenBuffers(1, &s_overlayVBO);
    
    glBindVertexArray(s_overlayVAO);
    glBindBuffer(GL_ARRAY_BUFFER, s_overlayVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);
    
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);
    
    glBindVertexArray(0);
}

// Light position editor implementation
void HDRLightOverlay::SetEditMode(bool enabled) {
    s_editMode = enabled;
    if (!enabled) {
        s_selectedLightIndex = -1;
    }
    HDRLIGHT_LOG("Edit mode: %s", enabled ? "enabled" : "disabled");
}

bool HDRLightOverlay::GetEditMode() {
    return s_editMode;
}

int HDRLightOverlay::GetSelectedLightIndex() {
    return s_selectedLightIndex;
}

const std::vector<HDRLightConfig>& HDRLightOverlay::GetLightConfigs() {
    return s_lightConfigs;
}

void HDRLightOverlay::UpdateLightPosition(int configIndex, float x, float y) {
    if (configIndex >= 0 && configIndex < (int)s_lightConfigs.size()) {
        s_lightConfigs[configIndex].X = x;
        s_lightConfigs[configIndex].Y = y;
    }
}

void HDRLightOverlay::UpdateLightSize(int configIndex, float w, float h) {
    if (configIndex >= 0 && configIndex < (int)s_lightConfigs.size()) {
        s_lightConfigs[configIndex].Width = w;
        s_lightConfigs[configIndex].Height = h;
    }
}

void HDRLightOverlay::OnTouchDown(float screenX, float screenY, int viewportX, int viewportY, int viewportW, int viewportH) {
    if (!s_editMode) return;
    
    // Convert screen coords to normalized canvas coords (0-1)
    float normX = (screenX - viewportX) / viewportW;
    float normY = (screenY - viewportY) / viewportH;
    
    HDRLIGHT_LOG("Touch down at screen (%.1f, %.1f) viewport(%d,%d,%d,%d) -> norm (%.3f, %.3f)", 
                 screenX, screenY, viewportX, viewportY, viewportW, viewportH, normX, normY);
    HDRLIGHT_LOG("Have %zu light configs and %zu test lights", s_lightConfigs.size(), s_testLights.size());
    
    // Find nearest light - use larger selection radius
    float minDist = 0.15f;  // Max distance to select (increased)
    s_selectedLightIndex = -1;
    
    for (int i = 0; i < (int)s_lightConfigs.size(); i++) {
        const auto& config = s_lightConfigs[i];
        float dx = normX - config.X;
        float dy = normY - config.Y;
        float dist = sqrtf(dx * dx + dy * dy);
        
        if (dist < minDist) {
            minDist = dist;
            s_selectedLightIndex = i;
            s_dragOffsetX = dx;
            s_dragOffsetY = dy;
        }
    }
    
    // Also check test lights
    for (int i = 0; i < (int)s_testLights.size(); i++) {
        const auto& test = s_testLights[i];
        float dx = normX - test.x;
        float dy = normY - test.y;
        float dist = sqrtf(dx * dx + dy * dy);
        
        if (dist < minDist) {
            minDist = dist;
            s_selectedLightIndex = -(i + 1);  // Negative index for test lights
            s_dragOffsetX = dx;
            s_dragOffsetY = dy;
        }
    }
    
    if (s_selectedLightIndex >= 0) {
        HDRLIGHT_LOG("Selected light config %d", s_selectedLightIndex);
    } else if (s_selectedLightIndex < -1) {
        HDRLIGHT_LOG("Selected test light %d", -(s_selectedLightIndex + 1));
    }
}

void HDRLightOverlay::OnTouchMove(float screenX, float screenY, int viewportX, int viewportY, int viewportW, int viewportH) {
    if (!s_editMode || s_selectedLightIndex == -1) return;
    
    // Convert screen coords to normalized canvas coords
    float normX = (screenX - viewportX) / viewportW;
    float normY = (screenY - viewportY) / viewportH;
    
    // Apply drag offset to get new position
    float newX = normX - s_dragOffsetX;
    float newY = normY - s_dragOffsetY;
    
    if (s_selectedLightIndex >= 0) {
        // Update light config position
        s_lightConfigs[s_selectedLightIndex].X = newX;
        s_lightConfigs[s_selectedLightIndex].Y = newY;
    } else {
        // Update test light position
        int testIdx = -(s_selectedLightIndex + 1);
        if (testIdx < (int)s_testLights.size()) {
            s_testLights[testIdx].x = newX;
            s_testLights[testIdx].y = newY;
        }
    }
}

void HDRLightOverlay::OnTouchUp() {
    if (!s_editMode) return;
    
    if (s_selectedLightIndex >= 0 && s_selectedLightIndex < (int)s_lightConfigs.size()) {
        const auto& config = s_lightConfigs[s_selectedLightIndex];
        HDRLIGHT_LOG("Light %d new position: (%.4f, %.4f)", s_selectedLightIndex, config.X, config.Y);
    }
    // Keep selection for reference, don't clear s_selectedLightIndex
}

bool HDRLightOverlay::SaveLightPositions(const char* filepath) {
    HDRLIGHT_LOG("SaveLightPositions called: %zu configs, %zu test lights", 
                 s_lightConfigs.size(), s_testLights.size());
    
    if (s_lightConfigs.empty() && s_testLights.empty()) {
        HDRLIGHT_LOG("No light configs to save!");
        return false;
    }
    
    FILE* f = fopen(filepath, "w");
    if (!f) {
        HDRLIGHT_LOG("Failed to open %s for writing", filepath);
        return false;
    }
    
    fprintf(f, "# HDR Light Positions\n");
    fprintf(f, "# Format: group,index,x,y,w,h,r,g,b\n\n");
    
    for (const auto& config : s_lightConfigs) {
        fprintf(f, "%s,%d,%.6f,%.6f,%.6f,%.6f,%.3f,%.3f,%.3f\n",
                config.GroupName, config.LightIndex,
                config.X, config.Y, config.Width, config.Height,
                config.R, config.G, config.B);
    }
    
    fprintf(f, "\n# Test lights\n");
    for (size_t i = 0; i < s_testLights.size(); i++) {
        const auto& test = s_testLights[i];
        fprintf(f, "test,%zu,%.6f,%.6f,%.6f,%.6f,%.3f,%.3f,%.3f\n",
                i, test.x, test.y, test.w, test.h, test.r, test.g, test.b);
    }
    
    fclose(f);
    HDRLIGHT_LOG("Saved %zu light configs to %s", s_lightConfigs.size(), filepath);
    return true;
}

bool HDRLightOverlay::LoadLightPositions(const char* filepath) {
    FILE* f = fopen(filepath, "r");
    if (!f) {
        HDRLIGHT_LOG("No saved light positions at %s", filepath);
        return false;
    }
    
    char line[256];
    while (fgets(line, sizeof(line), f)) {
        if (line[0] == '#' || line[0] == '\n') continue;
        
        char group[64];
        int index;
        float x, y, w, h, r, g, b;
        
        if (sscanf(line, "%63[^,],%d,%f,%f,%f,%f,%f,%f,%f",
                   group, &index, &x, &y, &w, &h, &r, &g, &b) == 9) {
            
            if (strcmp(group, "test") == 0) {
                // Update test light
                if (index < (int)s_testLights.size()) {
                    s_testLights[index].x = x;
                    s_testLights[index].y = y;
                    s_testLights[index].w = w;
                    s_testLights[index].h = h;
                }
            } else {
                // Find matching config and update
                for (auto& config : s_lightConfigs) {
                    if (strcmp(config.GroupName, group) == 0 && config.LightIndex == index) {
                        config.X = x;
                        config.Y = y;
                        config.Width = w;
                        config.Height = h;
                        break;
                    }
                }
            }
        }
    }
    
    fclose(f);
    HDRLIGHT_LOG("Loaded light positions from %s", filepath);
    return true;
}

int HDRLightOverlay::ResetOutOfBoundsLights() {
    int resetCount = 0;
    const float margin = 0.05f;  // Small margin inside viewport
    const float minX = margin;
    const float maxX = 1.0f - margin;
    const float minY = margin;
    const float maxY = 1.0f - margin;
    
    for (auto& config : s_lightConfigs) {
        bool outOfBounds = false;
        
        if (config.X < 0.0f || config.X > 1.0f || 
            config.Y < 0.0f || config.Y > 1.0f) {
            outOfBounds = true;
        }
        
        if (outOfBounds) {
            // Clamp to valid range
            config.X = std::max(minX, std::min(maxX, config.X));
            config.Y = std::max(minY, std::min(maxY, config.Y));
            
            // If still invalid (e.g., NaN), reset to center
            if (config.X != config.X || config.Y != config.Y) {  // NaN check
                config.X = 0.5f;
                config.Y = 0.5f;
            }
            
            resetCount++;
            HDRLIGHT_LOG("Reset out-of-bounds light %s[%d] to (%.3f, %.3f)", 
                         config.GroupName, config.LightIndex, config.X, config.Y);
        }
    }
    
    // Also reset test lights
    for (size_t i = 0; i < s_testLights.size(); i++) {
        auto& test = s_testLights[i];
        if (test.x < 0.0f || test.x > 1.0f || test.y < 0.0f || test.y > 1.0f) {
            test.x = std::max(minX, std::min(maxX, test.x));
            test.y = std::max(minY, std::min(maxY, test.y));
            resetCount++;
        }
    }
    
    HDRLIGHT_LOG("Reset %d out-of-bounds lights", resetCount);
    return resetCount;
}
