#include "pch.h"
#include "HDRLightOverlay.h"
#include "HDRRenderer.h"
#include "TLightGroup.h"
#include "TLight.h"
#include "TBumper.h"
#include "HDRConfig.h"
#include "control.h"
#include "pb.h"
#include "TPinballTable.h"
#include <cmath>
#include <cstdio>
#include <cstring>
#include <chrono>

#ifdef __ANDROID__
#include <android/log.h>
#define HDRLIGHT_LOG(...) __android_log_print(ANDROID_LOG_INFO, "HDRLightOverlay", __VA_ARGS__)
#else
#define HDRLIGHT_LOG(...)
#endif

// Static member initialization
std::vector<HDRLightOverlay::RegisteredGroup> HDRLightOverlay::s_registeredGroups;
std::vector<HDRLightOverlay::RegisteredLight> HDRLightOverlay::s_registeredLights;
std::vector<HDRLightOverlay::RegisteredBumper> HDRLightOverlay::s_registeredBumpers;
std::vector<HDRLightConfig> HDRLightOverlay::s_lightConfigs;
std::vector<HDRBumperConfig> HDRLightOverlay::s_bumperConfigs;
std::vector<HDRLightOverlay::LightState> HDRLightOverlay::s_lightStates;
std::vector<HDRLightOverlay::BumperState> HDRLightOverlay::s_bumperStates;
std::vector<HDRLightOverlay::TestLight> HDRLightOverlay::s_testLights;
std::vector<HDRLightPreset> HDRLightOverlay::s_presets;
float HDRLightOverlay::s_debugBallX = 0.0f;
float HDRLightOverlay::s_debugBallY = 0.0f;
bool HDRLightOverlay::s_debugBallEnabled = false;  // Disabled - only used for position tracking
bool HDRLightOverlay::s_ballValid = true;  // True when ball is active and visible
std::vector<HDRLightOverlay::TrailPoint> HDRLightOverlay::s_ballTrail;
float HDRLightOverlay::s_lastBallX = 0.0f;
float HDRLightOverlay::s_lastBallY = 0.0f;
float HDRLightOverlay::s_trailTime = 0.0f;
bool HDRLightOverlay::s_ballTeleported = false;
float HDRLightOverlay::s_glowModifier = 1.0f;
float HDRLightOverlay::s_trailOpacity = 0.85f;
float HDRLightOverlay::s_trailLifetimeSetting = 3.5f;
// Warm-up effect for startup animation
static bool s_wasInStartupAnimation = false;
static float s_warmupProgress = 0.0f;  // 0.0 to 1.0
static float s_warmupDuration = 4.0f;  // seconds to reach full glow
static float s_warmupMinGlow = 0.1f;   // starting glow multiplier (fraction of user setting)
static float s_userGlowSetting = 1.0f; // User's configured glow modifier from options
std::vector<HDRLightOverlay::DebugToggledLight> HDRLightOverlay::s_debugToggledLights;
bool HDRLightOverlay::s_initialized = false;
GLuint HDRLightOverlay::s_overlayProgram = 0;
GLuint HDRLightOverlay::s_overlayProgramPQ = 0;
GLuint HDRLightOverlay::s_overlayVAO = 0;
GLuint HDRLightOverlay::s_overlayVBO = 0;
GLuint HDRLightOverlay::s_trailProgram = 0;
GLuint HDRLightOverlay::s_trailVAO = 0;
GLuint HDRLightOverlay::s_trailVBO = 0;
GLuint HDRLightOverlay::s_trailFBO = 0;
GLuint HDRLightOverlay::s_trailTexture = 0;
GLuint HDRLightOverlay::s_trailCompositeProgram = 0;
int HDRLightOverlay::s_trailFBOWidth = 0;
int HDRLightOverlay::s_trailFBOHeight = 0;
static bool s_debugAllLightsOn = false;  // Debug mode - shows all lights regardless of state
static bool s_editMode = false;  // Edit mode - allows dragging lights to reposition
static int s_selectedLightIndex = -1;  // >= 0 for lights, < -1 for test lights
static int s_selectedBumperIndex = -1; // >= 0 for bumpers
static float s_dragOffsetX = 0.0f;
static float s_dragOffsetY = 0.0f;

// Vertex shader for light overlay
// Supports camera zoom/pan transform
static const char* s_overlayVertexSrc = R"(#version 300 es
precision highp float;
layout(location = 0) in vec2 aPos;
layout(location = 1) in vec2 aTexCoord;

uniform vec4 uLightRect;  // x, y, width, height in normalized coords (0-1)
uniform float uCameraZoom;      // 1.0 = no zoom, 2.0 = 2x zoom, etc.
uniform vec2 uCameraCenter;     // Normalized center point (0-1)

out vec2 vLocalCoord;  // 0-1 within the light quad

void main() {
    // aPos is -1 to 1, convert to 0-1
    vec2 localPos = aPos * 0.5 + 0.5;
    
    // Calculate world position of this vertex
    // uLightRect.xy = center position (0-1)
    // uLightRect.zw = width, height (0-1)
    vec2 worldPos;
    worldPos.x = uLightRect.x + (localPos.x - 0.5) * uLightRect.z;
    worldPos.y = uLightRect.y + (localPos.y - 0.5) * uLightRect.w;
    
    // Apply camera transform (matching new main texture transform)
    // Main texture: texCoord = (screenPos - 0.5) / zoom + 0.5 - (center - 0.5)
    //             = (screenPos - 0.5) / zoom + 1.0 - center
    // Inverse: screenPos = (texPos - 1.0 + center) * zoom + 0.5
    vec2 screenPos = (worldPos - 1.0 + uCameraCenter) * uCameraZoom + 0.5;
    
    // Convert to clip space (-1 to 1)
    // 0 -> -1, 1 -> +1 for X
    // 0 -> +1, 1 -> -1 for Y (flip for top-left origin)
    screenPos.x = screenPos.x * 2.0 - 1.0;
    screenPos.y = 1.0 - screenPos.y * 2.0;
    
    gl_Position = vec4(screenPos, 0.0, 1.0);
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

// Trail vertex shader - takes pre-computed positions with alpha
// Supports camera zoom/pan transform
static const char* s_trailVertexSrc = R"(#version 300 es
precision highp float;

layout(location = 0) in vec2 aPos;      // Position in normalized coords (0-1)
layout(location = 1) in float aAlpha;   // Alpha/fade value
layout(location = 2) in float aEdge;    // 0 = center, 1 = edge (for soft edges)

uniform float uCameraZoom;      // 1.0 = no zoom, 2.0 = 2x zoom, etc.
uniform vec2 uCameraCenter;     // Normalized center point (0-1)

out float vAlpha;
out float vEdge;

void main() {
    // Apply camera transform (matching lights)
    // screenPos = (texPos - 1.0 + center) * zoom + 0.5
    vec2 screenPos = (aPos - 1.0 + uCameraCenter) * uCameraZoom + 0.5;
    
    // Convert to clip space
    vec2 pos;
    pos.x = screenPos.x * 2.0 - 1.0;
    pos.y = 1.0 - screenPos.y * 2.0;
    
    gl_Position = vec4(pos, 0.0, 1.0);
    vAlpha = aAlpha;
    vEdge = aEdge;
}
)";

// Trail fragment shader - renders to texture with linear color (no PQ)
// Uses GL_MAX blending so overlapping areas don't accumulate
static const char* s_trailFragmentSrc = R"(#version 300 es
precision highp float;

in float vAlpha;
in float vEdge;
out vec4 fragColor;

uniform vec3 uTrailColor;
uniform float uIntensityNits;

void main() {
    // Soft edge falloff
    float edgeFade = 1.0 - smoothstep(0.0, 1.0, vEdge);
    float alpha = vAlpha * edgeFade;
    
    if (alpha < 0.01) discard;
    
    // Output linear HDR color with alpha for intensity
    fragColor = vec4(uTrailColor * uIntensityNits / 10000.0, alpha);
}
)";

// Trail composite shader - renders trail texture to screen with PQ encoding
static const char* s_trailCompositeVertexSrc = R"(#version 300 es
precision highp float;

layout(location = 0) in vec2 aPos;

out vec2 vTexCoord;

void main() {
    vTexCoord = aPos * 0.5 + 0.5;
    gl_Position = vec4(aPos, 0.0, 1.0);
}
)";

static const char* s_trailCompositeFragmentSrc = R"(#version 300 es
precision highp float;

in vec2 vTexCoord;
out vec4 fragColor;

uniform sampler2D uTrailTexture;
uniform float uMaxOpacity;

// PQ constants
const float m1 = 0.1593017578125;
const float m2 = 78.84375;
const float c1 = 0.8359375;
const float c2 = 18.8515625;
const float c3 = 18.6875;

vec3 linearToPQ(vec3 linearNormalized) {
    vec3 Y = max(linearNormalized, vec3(0.0));
    vec3 Ym1 = pow(Y, vec3(m1));
    vec3 numerator = c1 + c2 * Ym1;
    vec3 denominator = 1.0 + c3 * Ym1;
    return pow(numerator / denominator, vec3(m2));
}

void main() {
    vec4 trail = texture(uTrailTexture, vTexCoord);
    
    if (trail.a < 0.01) discard;
    
    // Apply max opacity and convert to PQ
    float alpha = trail.a * uMaxOpacity;
    vec3 pqColor = linearToPQ(trail.rgb);
    
    fragColor = vec4(pqColor, alpha);
}
)";

void HDRLightOverlay::Init() {
    if (s_initialized) return;
    
    HDRLIGHT_LOG("Initializing HDR light overlay system");
    
    CreateShaders();
    CreateQuad();
    CreateTrailShader();
    
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
    
    // Clean up trail resources
    if (s_trailProgram) {
        glDeleteProgram(s_trailProgram);
        s_trailProgram = 0;
    }
    if (s_trailCompositeProgram) {
        glDeleteProgram(s_trailCompositeProgram);
        s_trailCompositeProgram = 0;
    }
    if (s_trailVAO) {
        glDeleteVertexArrays(1, &s_trailVAO);
        s_trailVAO = 0;
    }
    if (s_trailVBO) {
        glDeleteBuffers(1, &s_trailVBO);
        s_trailVBO = 0;
    }
    if (s_trailFBO) {
        glDeleteFramebuffers(1, &s_trailFBO);
        s_trailFBO = 0;
    }
    if (s_trailTexture) {
        glDeleteTextures(1, &s_trailTexture);
        s_trailTexture = 0;
    }
    s_trailFBOWidth = 0;
    s_trailFBOHeight = 0;
    
    s_registeredGroups.clear();
    s_registeredLights.clear();
    s_registeredBumpers.clear();
    s_lightConfigs.clear();
    s_bumperConfigs.clear();
    s_lightStates.clear();
    s_bumperStates.clear();
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

void HDRLightOverlay::RegisterBumper(const char* bumperName, TBumper* bumper) {
    if (!bumper) return;
    
    // Check if already registered
    for (const auto& reg : s_registeredBumpers) {
        if (strcmp(reg.name, bumperName) == 0) return;
    }
    
    s_registeredBumpers.push_back({bumperName, bumper});
    HDRLIGHT_LOG("Registered bumper: %s", bumperName);
}

void HDRLightOverlay::AddBumperConfig(const HDRBumperConfig& config) {
    s_bumperConfigs.push_back(config);
    HDRLIGHT_LOG("Added bumper config: %s at (%.2f, %.2f)", 
                 config.BumperName, config.X, config.Y);
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

void HDRLightOverlay::ToggleDebugLight(const char* groupName, int lightIndex) {
    // Toggle the debug state for this light
    // Check if this light is already in the debug toggled list
    for (auto& debugLight : s_debugToggledLights) {
        if (debugLight.groupName == groupName && debugLight.lightIndex == lightIndex) {
            // Toggle the existing entry
            debugLight.isOn = !debugLight.isOn;
            HDRLIGHT_LOG("Toggle debug light: %s[%d] -> %s", groupName, lightIndex, debugLight.isOn ? "on" : "off");
            return;
        }
    }
    
    // Not found, add a new entry (starts as ON since we're toggling from off)
    DebugToggledLight newLight;
    newLight.groupName = groupName;
    newLight.lightIndex = lightIndex;
    newLight.isOn = true;
    s_debugToggledLights.push_back(newLight);
    HDRLIGHT_LOG("Toggle debug light: %s[%d] -> on (new)", groupName, lightIndex);
}

void HDRLightOverlay::ClearDebugToggledLights() {
    s_debugToggledLights.clear();
    HDRLIGHT_LOG("Cleared all debug toggled lights");
}

void HDRLightOverlay::UpdateLightStates() {
    s_lightStates.clear();
    
    static int frameCount = 0;
    frameCount++;
    bool shouldLog = (frameCount % 60 == 0);  // Log once per second at 60fps
    
    // Check if we're in startup animation (message 28 on table's main LightGroup)
    bool inStartupAnimation = false;
    if (pb::MainTable && pb::MainTable->LightGroup) {
        int tableLightGroupMsg = pb::MainTable->LightGroup->MessageField2;
        inStartupAnimation = (tableLightGroupMsg == 28);
    }
    
    // Warm-up effect: gradually increase glow during startup animation
    static auto lastFrameTime = std::chrono::steady_clock::now();
    auto currentTime = std::chrono::steady_clock::now();
    float deltaTime = std::chrono::duration<float>(currentTime - lastFrameTime).count();
    lastFrameTime = currentTime;
    
    if (inStartupAnimation) {
        if (!s_wasInStartupAnimation) {
            // Just started - reset warm-up progress
            s_warmupProgress = 0.0f;
            HDRLIGHT_LOG("Startup animation started - beginning warm-up effect (user glow=%.2f)", s_userGlowSetting);
        }
        // Advance warm-up progress
        s_warmupProgress += deltaTime / s_warmupDuration;
        if (s_warmupProgress > 1.0f) s_warmupProgress = 1.0f;
        
        // Apply exponential curve to glow modifier, scaled to user's setting
        // Starts at (minGlow * userSetting) and grows to userSetting
        float t = s_warmupProgress;
        float expFactor = s_warmupMinGlow * expf(t * logf(1.0f / s_warmupMinGlow));
        s_glowModifier = expFactor * s_userGlowSetting;
    } else {
        if (s_wasInStartupAnimation) {
            // Animation just ended - restore to user's configured glow setting
            s_glowModifier = s_userGlowSetting;
            HDRLIGHT_LOG("Startup animation ended - glow restored to user setting %.2f", s_userGlowSetting);
        }
    }
    s_wasInStartupAnimation = inStartupAnimation;
    
    // Enforce light debug mode - turn off all table lights every frame
    if (control_IsLightDebugModeActive()) {
        control_EnforceLightDebugMode();
    }
    
    // Get selected light info for HDR enforcement
    std::string selectedGroupName;
    int selectedLightIndex = -1;
    control_GetSelectedLightInfo(selectedGroupName, selectedLightIndex);
    bool debugModeActive = control_IsLightDebugModeActive();
    
    if (shouldLog) {
        HDRLIGHT_LOG("UpdateLightStates: %zu configs, %zu groups, warmup=%.2f, glowMod=%.3f", 
                     s_lightConfigs.size(), s_registeredGroups.size(),
                     s_warmupProgress, s_glowModifier);
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
        
        // Set default color from config
        state.r = config.R;
        state.g = config.G;
        state.b = config.B;
        
        // Dynamic color support for lights that change color based on BmpIndex2
        // bsink_arrow_lights: changes color based on wormhole destination (BmpIndex2 = 0, 1, 2)
        if (strcmp(config.GroupName, "bsink_arrow_lights") == 0) {
            int colorIndex = light->BmpIndex2;
            if (colorIndex < 0) colorIndex = 0;
            if (colorIndex > 2) colorIndex = 2;
            // Colors match wormhole destinations: Green, Red, Yellow
            static const float bsinkColors[3][3] = {
                {1.0f, 1.0f, 0.0f},  // 0 = Yellow,
                {0.0f, 1.0f, 0.0f},  // 1 = Green
                {1.0f, 0.0f, 0.0f}   // 2 = Red
            };
            state.r = bsinkColors[colorIndex][0];
            state.g = bsinkColors[colorIndex][1];
            state.b = bsinkColors[colorIndex][2];
        }
        
        // Check if the light group is in animation mode
        // Message 26/27: rotation animation - FlasherFlag2 indicates which light is "lit"
        // Message 28: startup random flash - FlasherFlag2 indicates which light is "lit"
        // Message 29: game over random on/off - FlasherFlag2 indicates which light is "lit"
        // Also check the table's main LightGroup since startup/gameover animations use that
        bool isAnimating = false;
        if (group && (group->MessageField2 == 26 || group->MessageField2 == 27 || 
                      group->MessageField2 == 28 || group->MessageField2 == 29)) {
            isAnimating = true;
        }
        // Check table's main LightGroup for global animations (startup, game over)
        if (pb::MainTable && pb::MainTable->LightGroup) {
            int tableLightGroupMsg = pb::MainTable->LightGroup->MessageField2;
            if (tableLightGroupMsg == 28 || tableLightGroupMsg == 29) {
                isAnimating = true;
            }
        }
        // FlasherFlag2 = light is showing "on" state during animation (Message 9)
        // FlasherFlag1 = light is showing "off" state during animation (Message 8)
        // Timer1 != 0 means the light has an active animation timeout
        // During startup animation, lights randomly get Message(9) which sets FlasherFlag2=1
        // and schedules a timeout. When timeout fires, FlasherFlag2 is reset to 0.
        // We detect animation-lit state by checking FlasherFlag2 OR having an active Timer1
        // while FlasherFlag1 is not set (not in explicit "off" animation state)
        bool animationLit = (light->FlasherFlag2 != 0) || 
                           (light->Timer1 != 0 && light->FlasherFlag1 == 0);
        
        // Calculate current intensity
        if (debugModeActive) {
            // Light debug mode - only show the selected light's HDR overlay
            bool isSelectedLight = (strcmp(config.GroupName, selectedGroupName.c_str()) == 0 && 
                                   config.LightIndex == selectedLightIndex);
            // Check if this light has been toggled on via ToggleDebugLight
            bool isDebugToggled = false;
            for (const auto& debugLight : s_debugToggledLights) {
                if (strcmp(debugLight.groupName.c_str(), config.GroupName) == 0 && 
                    debugLight.lightIndex == config.LightIndex) {
                    isDebugToggled = debugLight.isOn;
                    break;
                }
            }
            state.currentIntensity = isDebugToggled ? config.IntensityOn : 0.0f;
        } else if (s_debugAllLightsOn || s_editMode) {
            // Debug mode or edit mode - all lights at full intensity for visibility
            state.currentIntensity = config.IntensityOn;
        } else if (isAnimating) {
            // Animation mode - use FlasherFlag2 to determine which light is lit
            state.currentIntensity = animationLit ? config.IntensityOn : 0.0f;
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
            int tableLightGroupMsg = (pb::MainTable && pb::MainTable->LightGroup) ? 
                                     pb::MainTable->LightGroup->MessageField2 : -1;
            HDRLIGHT_LOG("  Light %s[%d]: on=%d, flashing=%d, intensity=%.0f, anim=%d, animLit=%d, flag1=%d, flag2=%d, timer1=%d, tableMsg=%d", 
                         config.GroupName, config.LightIndex, 
                         state.isOn, state.isFlashing, 
                         state.currentIntensity, isAnimating, animationLit,
                         light->FlasherFlag1, light->FlasherFlag2, light->Timer1, tableLightGroupMsg);
        }
        
        // Only add if light is actually on (or debug mode)
        if (state.currentIntensity > 0.0f) {
            s_lightStates.push_back(state);
        }
    }
    
    if (shouldLog && !s_lightStates.empty()) {
        HDRLIGHT_LOG("  Active lights: %zu", s_lightStates.size());
    }
    
    // Process bumper configs
    s_bumperStates.clear();
    for (const auto& config : s_bumperConfigs) {
        TBumper* bumper = nullptr;
        
        for (const auto& reg : s_registeredBumpers) {
            if (strcmp(reg.name, config.BumperName) == 0) {
                bumper = reg.bumper;
                break;
            }
        }
        
        if (!bumper) {
            if (shouldLog) HDRLIGHT_LOG("  Bumper config %s: bumper not found", config.BumperName);
            continue;
        }
        
        BumperState state;
        state.config = &config;
        state.bumper = bumper;
        state.upgradeLevel = bumper->BmpIndex;
        
        // Clamp upgrade level to valid range
        if (state.upgradeLevel < 0) state.upgradeLevel = 0;
        if (state.upgradeLevel > 3) state.upgradeLevel = 3;
        
        // Get color based on upgrade level
        state.r = config.Colors[state.upgradeLevel][0];
        state.g = config.Colors[state.upgradeLevel][1];
        state.b = config.Colors[state.upgradeLevel][2];
        
        // Bumpers light up when hit (Timer != 0) or in edit/debug mode
        bool isLit = (bumper->Timer != 0);
        if (s_debugAllLightsOn || s_editMode) {
            state.currentIntensity = config.Intensity;
        } else if (isLit) {
            state.currentIntensity = config.Intensity;
        } else {
            state.currentIntensity = 0.0f;
        }
        
        if (shouldLog) {
            HDRLIGHT_LOG("  Bumper %s: level=%d, lit=%d, color=(%.2f,%.2f,%.2f), intensity=%.0f", 
                         config.BumperName, state.upgradeLevel, isLit,
                         state.r, state.g, state.b, state.currentIntensity);
        }
        
        // Only add if bumper is lit (or debug/edit mode)
        if (state.currentIntensity > 0.0f) {
            s_bumperStates.push_back(state);
        }
    }
    
    if (shouldLog && !s_bumperStates.empty()) {
        HDRLIGHT_LOG("  Active bumpers: %zu", s_bumperStates.size());
    }
}

bool HDRLightOverlay::HasActiveLights() {
    return !s_lightStates.empty() || !s_testLights.empty() || !s_bumperStates.empty();
}

void HDRLightOverlay::RenderOverlays(int textureWidth, int textureHeight) {
    // Lazy initialization - create shaders on first render when GL context is ready
    if (!s_initialized) {
        HDRLIGHT_LOG("RenderOverlays: lazy init");
        CreateShaders();
        CreateQuad();
        s_initialized = true;
    }
    
    if (s_lightStates.empty() && s_testLights.empty() && s_bumperStates.empty()) {
        return;
    }
    
    HDRLIGHT_LOG("RenderOverlays: rendering %zu game lights + %zu bumpers + %zu test lights", 
                 s_lightStates.size(), s_bumperStates.size(), s_testLights.size());
    
    // Enable blending for additive light overlay
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);  // Additive blending
    
    glUseProgram(s_overlayProgram);
    glBindVertexArray(s_overlayVAO);
    
    // Render game lights
    for (const auto& state : s_lightStates) {
        RenderSingleLight(state, textureWidth, textureHeight);
    }
    
    // Render bumpers
    for (const auto& state : s_bumperStates) {
        const HDRBumperConfig* config = state.config;
        
        float aspectRatio = (float)textureWidth / (float)textureHeight;
        GLint aspectLoc = glGetUniformLocation(s_overlayProgram, "uAspectRatio");
        glUniform1f(aspectLoc, aspectRatio);
        
        GLint rectLoc = glGetUniformLocation(s_overlayProgram, "uLightRect");
        glUniform4f(rectLoc, config->X, config->Y, config->Width, config->Height);
        
        GLint colorLoc = glGetUniformLocation(s_overlayProgram, "uLightColor");
        glUniform3f(colorLoc, state.r, state.g, state.b);
        
        GLint intensityLoc = glGetUniformLocation(s_overlayProgram, "uIntensity");
        float linearIntensity = state.currentIntensity / 203.0f;  // SDR_WHITE_NITS
        glUniform1f(intensityLoc, linearIntensity);
        
        GLint glowLoc = glGetUniformLocation(s_overlayProgram, "uGlowRadius");
        glUniform1f(glowLoc, config->GlowRadius * s_glowModifier);
        
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
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
        glUniform1f(glowLoc, 1.0f * s_glowModifier);
        
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    }
    
    // Render debug ball if enabled
    if (s_debugBallEnabled) {
        float aspectRatio = (float)textureWidth / (float)textureHeight;
        GLint aspectLoc = glGetUniformLocation(s_overlayProgram, "uAspectRatio");
        glUniform1f(aspectLoc, aspectRatio);
        
        // Debug ball - larger bright magenta circle for visibility
        float debugBallSize = 0.05f;  // Larger size for visibility
        GLint rectLoc = glGetUniformLocation(s_overlayProgram, "uLightRect");
        glUniform4f(rectLoc, s_debugBallX, s_debugBallY, debugBallSize, debugBallSize);
        
        GLint colorLoc = glGetUniformLocation(s_overlayProgram, "uLightColor");
        glUniform3f(colorLoc, 1.0f, 0.0f, 1.0f);  // Magenta color for debug ball
        
        GLint intensityLoc = glGetUniformLocation(s_overlayProgram, "uIntensity");
        glUniform1f(intensityLoc, 5.0f);  // Very bright intensity
        
        GLint glowLoc = glGetUniformLocation(s_overlayProgram, "uGlowRadius");
        glUniform1f(glowLoc, 1.0f);  // Full glow
        
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
    
    // Set color uniform (linear RGB) - use dynamic color from state
    GLint colorLoc = glGetUniformLocation(s_overlayProgram, "uLightColor");
    glUniform3f(colorLoc, state.r, state.g, state.b);
    
    // Set intensity (convert nits to linear multiplier relative to SDR white)
    GLint intensityLoc = glGetUniformLocation(s_overlayProgram, "uIntensity");
    float linearIntensity = state.currentIntensity / HDR::Luminance::SDR_WHITE_NITS;
    glUniform1f(intensityLoc, linearIntensity);
    
    // Set glow radius (apply global modifier)
    GLint glowLoc = glGetUniformLocation(s_overlayProgram, "uGlowRadius");
    glUniform1f(glowLoc, config->GlowRadius * s_glowModifier);
    
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
    
    if (s_lightStates.empty() && s_testLights.empty() && s_bumperStates.empty()) {
        return;
    }
    
    static int pqFrameCount = 0;
    pqFrameCount++;
    if (pqFrameCount % 60 == 0) {
        HDRLIGHT_LOG("RenderOverlaysPQ: rendering %zu game lights + %zu bumpers + %zu test lights, viewport %d,%d %dx%d",
                     s_lightStates.size(), s_bumperStates.size(), s_testLights.size(), 
                     viewportX, viewportY, viewportW, viewportH);
    }
    
    // Set viewport to match the game area
    glViewport(viewportX, viewportY, viewportW, viewportH);
    
    // Enable blending for additive light overlay
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);  // Additive blending
    
    glUseProgram(s_overlayProgramPQ);
    glBindVertexArray(s_overlayVAO);
    
    // Set camera transform uniforms (get from HDRRenderer)
    float cameraZoom = HDRRenderer::GetCurrentCameraZoom();
    float cameraCenterX = HDRRenderer::GetCurrentCameraCenterX();
    float cameraCenterY = HDRRenderer::GetCurrentCameraCenterY();
    
    GLint zoomLoc = glGetUniformLocation(s_overlayProgramPQ, "uCameraZoom");
    GLint centerLoc = glGetUniformLocation(s_overlayProgramPQ, "uCameraCenter");
    glUniform1f(zoomLoc, cameraZoom);
    glUniform2f(centerLoc, cameraCenterX, cameraCenterY);
    
    // Render game lights with PQ encoding
    for (const auto& state : s_lightStates) {
        RenderSingleLightPQ(state, maxNits);
    }
    
    // Render bumpers with PQ encoding
    for (const auto& state : s_bumperStates) {
        const HDRBumperConfig* config = state.config;
        
        GLint rectLoc = glGetUniformLocation(s_overlayProgramPQ, "uLightRect");
        glUniform4f(rectLoc, config->X, config->Y, config->Width, config->Height);
        
        GLint colorLoc = glGetUniformLocation(s_overlayProgramPQ, "uLightColor");
        glUniform3f(colorLoc, state.r, state.g, state.b);
        
        GLint intensityLoc = glGetUniformLocation(s_overlayProgramPQ, "uIntensityNits");
        glUniform1f(intensityLoc, state.currentIntensity);
        
        GLint maxNitsLoc = glGetUniformLocation(s_overlayProgramPQ, "uMaxNits");
        glUniform1f(maxNitsLoc, maxNits);
        
        GLint glowLoc = glGetUniformLocation(s_overlayProgramPQ, "uGlowRadius");
        glUniform1f(glowLoc, config->GlowRadius * s_glowModifier);
        
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
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
        glUniform1f(glowLoc, 1.0f * s_glowModifier);
        
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    }
    
    // Render smooth curved ball trail as triangle strip mesh
    glBindVertexArray(0);  // Unbind current VAO before switching
    RenderTrailMesh(maxNits);
    
    // Re-bind for debug ball rendering
    glUseProgram(s_overlayProgramPQ);
    glBindVertexArray(s_overlayVAO);
    
    // Render debug ball if enabled (PQ encoding) - simple red circle, no glow
    if (s_debugBallEnabled) {
        float debugBallSize = 0.025f;  // Small circle
        GLint rectLoc = glGetUniformLocation(s_overlayProgramPQ, "uLightRect");
        glUniform4f(rectLoc, s_debugBallX, s_debugBallY, debugBallSize, debugBallSize);
        
        GLint colorLoc = glGetUniformLocation(s_overlayProgramPQ, "uLightColor");
        glUniform3f(colorLoc, 1.0f, 0.0f, 0.0f);  // Red color
        
        GLint intensityLoc = glGetUniformLocation(s_overlayProgramPQ, "uIntensityNits");
        glUniform1f(intensityLoc, 500.0f);  // Moderate intensity
        
        GLint maxNitsLoc = glGetUniformLocation(s_overlayProgramPQ, "uMaxNits");
        glUniform1f(maxNitsLoc, maxNits);
        
        GLint glowLoc = glGetUniformLocation(s_overlayProgramPQ, "uGlowRadius");
        glUniform1f(glowLoc, 0.1f);  // Minimal glow - almost solid circle
        
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    }
    
    glBindVertexArray(0);
    glDisable(GL_BLEND);
}

void HDRLightOverlay::RenderSingleLightPQ(const LightState& state, float maxNits) {
    const HDRLightConfig* config = state.config;
    
    // Check for ball occlusion - skip rendering if ball is over this light and light is below ball
    // Note: s_debugBallX/Y are always updated, s_debugBallEnabled only controls debug rendering
    if (!config->AboveBall) {
        float dx = s_debugBallX - config->X;
        float dy = s_debugBallY - config->Y;
        float dist = sqrtf(dx * dx + dy * dy);
        float occlusionRadius = 0.0075f;  // Ball radius for occlusion check (reduced)
        if (dist < occlusionRadius) {
            return;  // Skip rendering - ball is occluding this light
        }
    }
    
    // Set light rectangle uniform (normalized 0-1 coords)
    GLint rectLoc = glGetUniformLocation(s_overlayProgramPQ, "uLightRect");
    glUniform4f(rectLoc, config->X, config->Y, config->Width, config->Height);
    
    // Set color uniform (linear RGB, saturated) - use dynamic color from state
    GLint colorLoc = glGetUniformLocation(s_overlayProgramPQ, "uLightColor");
    glUniform3f(colorLoc, state.r, state.g, state.b);
    
    // Set intensity in nits directly
    GLint intensityLoc = glGetUniformLocation(s_overlayProgramPQ, "uIntensityNits");
    glUniform1f(intensityLoc, state.currentIntensity);
    
    // Set max nits for clamping
    GLint maxNitsLoc = glGetUniformLocation(s_overlayProgramPQ, "uMaxNits");
    glUniform1f(maxNitsLoc, maxNits);
    
    // Set glow radius (apply global modifier)
    GLint glowLoc = glGetUniformLocation(s_overlayProgramPQ, "uGlowRadius");
    glUniform1f(glowLoc, config->GlowRadius * s_glowModifier);
    
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
        s_selectedBumperIndex = -1;
    }
    HDRLIGHT_LOG("Edit mode: %s", enabled ? "enabled" : "disabled");
}

bool HDRLightOverlay::GetEditMode() {
    return s_editMode;
}

int HDRLightOverlay::GetSelectedLightIndex() {
    return s_selectedLightIndex;
}

int HDRLightOverlay::GetSelectedBumperIndex() {
    return s_selectedBumperIndex;
}

bool HDRLightOverlay::HasSelection() {
    return (s_selectedLightIndex >= 0) || (s_selectedLightIndex < -1) || (s_selectedBumperIndex >= 0);
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
        s_lightConfigs[configIndex].PresetName.clear();  // Clear preset when manually editing
    }
}

void HDRLightOverlay::SetDebugBallPosition(float x, float y) {
    // Calculate velocity (assume ~60fps, so dt ~= 0.0167)
    float dt = 0.0167f;
    float vx = (x - s_lastBallX) / dt;
    float vy = (y - s_lastBallY) / dt;
    
    // If teleported, reset velocity to zero and insert break marker
    if (s_ballTeleported) {
        vx = 0.0f;
        vy = 0.0f;
        // Insert a break marker (negative timestamp signals break)
        if (!s_ballTrail.empty()) {
            TrailPoint breakMarker = {0, 0, 0, 0, -1.0f};
            s_ballTrail.insert(s_ballTrail.begin(), breakMarker);
        }
        s_ballTeleported = false;
    }
    
    // Increment trail time
    s_trailTime += dt;
    
    // Always add trail point
    TrailPoint newPoint = {x, y, vx, vy, s_trailTime};
    s_ballTrail.insert(s_ballTrail.begin(), newPoint);
    
    // Remove old points that have exceeded lifetime (use dynamic setting)
    while (!s_ballTrail.empty() && 
           (s_trailTime - s_ballTrail.back().timestamp) > s_trailLifetimeSetting) {
        s_ballTrail.pop_back();
    }
    
    // Also limit by count as safety
    while (s_ballTrail.size() > MAX_TRAIL_POINTS) {
        s_ballTrail.pop_back();
    }
    
    s_lastBallX = x;
    s_lastBallY = y;
    s_debugBallX = x;
    s_debugBallY = y;
}

void HDRLightOverlay::DecayTrail() {
    // Decay trail without adding new points (call when ball is inactive)
    float dt = 0.0167f;
    s_trailTime += dt;
    
    // Remove old points that have exceeded lifetime
    while (!s_ballTrail.empty() && 
           (s_trailTime - s_ballTrail.back().timestamp) > s_trailLifetimeSetting) {
        s_ballTrail.pop_back();
    }
}

void HDRLightOverlay::NotifyBallTeleported() {
    s_ballTeleported = true;
}

void HDRLightOverlay::EnableDebugBall(bool enabled) {
    s_debugBallEnabled = enabled;
    HDRLIGHT_LOG("Debug ball %s", enabled ? "enabled" : "disabled");
}

void HDRLightOverlay::SetGlowModifier(float modifier) {
    s_userGlowSetting = modifier;  // Store user's setting
    s_glowModifier = modifier;     // Apply immediately
    HDRLIGHT_LOG("Glow modifier set to %.2f", modifier);
}

void HDRLightOverlay::SetTrailOpacity(float opacity) {
    s_trailOpacity = opacity;
    HDRLIGHT_LOG("Trail opacity set to %.2f", opacity);
}

void HDRLightOverlay::SetTrailLifetime(float seconds) {
    s_trailLifetimeSetting = seconds;
    HDRLIGHT_LOG("Trail lifetime set to %.2f seconds", seconds);
}

bool HDRLightOverlay::ShouldBlockTouch(float screenX, float screenY, int viewportX, int viewportY, int viewportW, int viewportH) {
    if (!s_editMode) return false;
    
    // Convert screen coords to normalized canvas coords (0-1)
    float normX = (screenX - viewportX) / viewportW;
    float normY = (screenY - viewportY) / viewportH;
    
    // Settings button detection area based on user touches at X=0.57, Y=0.02-0.06
    float detectionAreaMinX = 0.5f;   // Start at middle
    float detectionAreaMaxX = 0.65f;  // Cover middle-right area
    float detectionAreaMinY = 0.0f;   // Extend to very top of screen
    float detectionAreaMaxY = 0.15f;  // Cover upper area
    
    // Check if touch is within settings button detection area
    bool inButtonBounds = (normX >= detectionAreaMinX && normX <= detectionAreaMaxX &&
                          normY >= detectionAreaMinY && normY <= detectionAreaMaxY);
    
    if (inButtonBounds) {
        HDRLIGHT_LOG("Touch on settings button at norm (%.3f, %.3f), blocking touch", normX, normY);
        return true;  // Block this touch
    }
    
    return false;  // Don't block this touch
}

void HDRLightOverlay::OnTouchDown(float screenX, float screenY, int viewportX, int viewportY, int viewportW, int viewportH) {
    if (!s_editMode) return;
    
    // Check if touch should be blocked (settings button area)
    if (ShouldBlockTouch(screenX, screenY, viewportX, viewportY, viewportW, viewportH)) {
        return;  // Don't select lights if touching settings button
    }
    
    // Convert screen coords to normalized canvas coords (0-1)
    float normX = (screenX - viewportX) / viewportW;
    float normY = (screenY - viewportY) / viewportH;
    
    HDRLIGHT_LOG("Touch down at screen (%.1f, %.1f) viewport(%d,%d,%d,%d) -> norm (%.3f, %.3f)", 
                 screenX, screenY, viewportX, viewportY, viewportW, viewportH, normX, normY);
    HDRLIGHT_LOG("Have %zu light configs, %zu bumper configs, %zu test lights", 
                 s_lightConfigs.size(), s_bumperConfigs.size(), s_testLights.size());
    
    // Find nearest light or bumper - use larger selection radius
    float minDist = 0.15f;  // Max distance to select (increased)
    s_selectedLightIndex = -1;
    s_selectedBumperIndex = -1;
    
    // Check lights first
    for (int i = 0; i < (int)s_lightConfigs.size(); i++) {
        const auto& config = s_lightConfigs[i];
        // Skip locked lights
        if (config.Locked) {
            continue;
        }
        float dx = normX - config.X;
        float dy = normY - config.Y;
        float dist = sqrtf(dx * dx + dy * dy);
        
        if (dist < minDist) {
            minDist = dist;
            s_selectedLightIndex = i;
            s_selectedBumperIndex = -1;
            s_dragOffsetX = dx;
            s_dragOffsetY = dy;
        }
    }
    
    // Check bumpers with separate, larger selection radius
    float bumperMinDist = 0.20f;  // Larger radius for bumpers
    HDRLIGHT_LOG("Checking %zu bumpers for selection (bumperMinDist=%.2f)", s_bumperConfigs.size(), bumperMinDist);
    for (int i = 0; i < (int)s_bumperConfigs.size(); i++) {
        const auto& config = s_bumperConfigs[i];
        // Skip locked bumpers
        if (config.Locked) {
            continue;
        }
        float dx = normX - config.X;
        float dy = normY - config.Y;
        float dist = sqrtf(dx * dx + dy * dy);
        
        HDRLIGHT_LOG("  Bumper %d at (%.3f,%.3f) dist=%.3f", i, config.X, config.Y, dist);
        
        if (dist < bumperMinDist) {
            bumperMinDist = dist;
            s_selectedBumperIndex = i;
            s_selectedLightIndex = -1;  // Bumper takes priority
            s_dragOffsetX = dx;
            s_dragOffsetY = dy;
            HDRLIGHT_LOG("    -> Selected bumper %d!", i);
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
            s_selectedBumperIndex = -1;
            s_dragOffsetX = dx;
            s_dragOffsetY = dy;
        }
    }
    
    if (s_selectedBumperIndex >= 0) {
        HDRLIGHT_LOG("Selected bumper config %d at (%.3f, %.3f)", 
                     s_selectedBumperIndex, 
                     s_bumperConfigs[s_selectedBumperIndex].X,
                     s_bumperConfigs[s_selectedBumperIndex].Y);
    } else if (s_selectedLightIndex >= 0) {
        HDRLIGHT_LOG("Selected light config %d", s_selectedLightIndex);
    } else if (s_selectedLightIndex < -1) {
        HDRLIGHT_LOG("Selected test light %d", -(s_selectedLightIndex + 1));
    } else {
        HDRLIGHT_LOG("Nothing selected (minDist was %.3f)", minDist);
    }
}

void HDRLightOverlay::OnTouchMove(float screenX, float screenY, int viewportX, int viewportY, int viewportW, int viewportH) {
    // Check if anything is selected (light >= 0, test light < -1, or bumper >= 0)
    bool hasSelection = (s_selectedLightIndex >= 0) || (s_selectedLightIndex < -1) || (s_selectedBumperIndex >= 0);
    if (!s_editMode || !hasSelection) return;
    
    // Convert screen coords to normalized canvas coords
    float normX = (screenX - viewportX) / viewportW;
    float normY = (screenY - viewportY) / viewportH;
    
    // Apply drag offset to get new position
    float newX = normX - s_dragOffsetX;
    float newY = normY - s_dragOffsetY;
    
    if (s_selectedBumperIndex >= 0) {
        // Check if bumper is locked before moving
        if (!s_bumperConfigs[s_selectedBumperIndex].Locked) {
            // Update bumper config position
            s_bumperConfigs[s_selectedBumperIndex].X = newX;
            s_bumperConfigs[s_selectedBumperIndex].Y = newY;
        }
    } else if (s_selectedLightIndex >= 0) {
        // Check if light is locked before moving
        if (!s_lightConfigs[s_selectedLightIndex].Locked) {
            // Update light config position
            s_lightConfigs[s_selectedLightIndex].X = newX;
            s_lightConfigs[s_selectedLightIndex].Y = newY;
        }
    } else if (s_selectedLightIndex < -1) {
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
    
    if (s_selectedBumperIndex >= 0 && s_selectedBumperIndex < (int)s_bumperConfigs.size()) {
        const auto& config = s_bumperConfigs[s_selectedBumperIndex];
        HDRLIGHT_LOG("Bumper %d new position: (%.4f, %.4f)", s_selectedBumperIndex, config.X, config.Y);
    } else if (s_selectedLightIndex >= 0 && s_selectedLightIndex < (int)s_lightConfigs.size()) {
        const auto& config = s_lightConfigs[s_selectedLightIndex];
        HDRLIGHT_LOG("Light %d new position: (%.4f, %.4f)", s_selectedLightIndex, config.X, config.Y);
    }
    // Keep selection for reference, don't clear indices
}

bool HDRLightOverlay::SaveLightPositions(const char* filepath) {
    HDRLIGHT_LOG("SaveLightPositions called: %zu configs, %zu bumpers, %zu test lights", 
                 s_lightConfigs.size(), s_bumperConfigs.size(), s_testLights.size());
    
    if (s_lightConfigs.empty() && s_bumperConfigs.empty() && s_testLights.empty()) {
        HDRLIGHT_LOG("No configs to save!");
        return false;
    }
    
    FILE* f = fopen(filepath, "w");
    if (!f) {
        HDRLIGHT_LOG("Failed to open %s for writing", filepath);
        return false;
    }
    
    fprintf(f, "# HDR Light Positions v2\n");
    fprintf(f, "# Format: group,index,x,y,w,h,r,g,b,locked,intensityOn,intensityFlash,glowRadius,presetName\n\n");
    
    for (const auto& config : s_lightConfigs) {
        fprintf(f, "%s,%d,%.6f,%.6f,%.6f,%.6f,%.3f,%.3f,%.3f,%d,%.1f,%.1f,%.3f,%s\n",
                config.GroupName, config.LightIndex,
                config.X, config.Y, config.Width, config.Height,
                config.R, config.G, config.B, config.Locked ? 1 : 0,
                config.IntensityOn, config.IntensityFlash, config.GlowRadius,
                config.PresetName.empty() ? "" : config.PresetName.c_str());
    }
    
    fprintf(f, "\n# Bumpers\n");
    fprintf(f, "# Format: group,index,x,y,w,h,r,g,b,locked\n");
    for (const auto& config : s_bumperConfigs) {
        fprintf(f, "bumper_%s,0,%.6f,%.6f,%.6f,%.6f,0,0,0,%d\n",
                config.BumperName,
                config.X, config.Y, config.Width, config.Height,
                config.Locked ? 1 : 0);
    }
    
    fprintf(f, "\n# Test lights\n");
    for (size_t i = 0; i < s_testLights.size(); i++) {
        const auto& test = s_testLights[i];
        fprintf(f, "test,%zu,%.6f,%.6f,%.6f,%.6f,%.3f,%.3f,%.3f\n",
                i, test.x, test.y, test.w, test.h, test.r, test.g, test.b);
    }
    
    fclose(f);
    HDRLIGHT_LOG("Saved %zu light configs, %zu bumpers to %s", s_lightConfigs.size(), s_bumperConfigs.size(), filepath);
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
        int index, locked;
        float x, y, w, h, r, g, b;
        float intensityOn = 600.0f, intensityFlash = 1000.0f, glowRadius = 0.5f;
        char presetName[64] = "";
        
        // Try new v2 format first (with intensity, glow, preset)
        int parsed = sscanf(line, "%63[^,],%d,%f,%f,%f,%f,%f,%f,%f,%d,%f,%f,%f,%63[^\n]",
                   group, &index, &x, &y, &w, &h, &r, &g, &b, &locked,
                   &intensityOn, &intensityFlash, &glowRadius, presetName);
        
        // Fall back to old format if v2 parsing didn't get all fields
        if (parsed < 10) {
            parsed = sscanf(line, "%63[^,],%d,%f,%f,%f,%f,%f,%f,%f,%d",
                       group, &index, &x, &y, &w, &h, &r, &g, &b, &locked);
        }
        
        if (parsed >= 10) {
            if (strcmp(group, "test") == 0) {
                // Update test light
                if (index < (int)s_testLights.size()) {
                    s_testLights[index].x = x;
                    s_testLights[index].y = y;
                    s_testLights[index].w = w;
                    s_testLights[index].h = h;
                }
            } else if (strncmp(group, "bumper_", 7) == 0 && strncmp(group, "bumper_target_lights", 20) != 0) {
                // Update bumper config - extract bumper name after "bumper_"
                // Note: exclude "bumper_target_lights" which is a light group, not a bumper
                const char* bumperName = group + 7;
                for (auto& config : s_bumperConfigs) {
                    if (strcmp(config.BumperName, bumperName) == 0) {
                        config.X = x;
                        config.Y = y;
                        config.Width = w;
                        config.Height = h;
                        config.Locked = (locked != 0);
                        HDRLIGHT_LOG("Loaded bumper %s position: (%.4f, %.4f) locked=%d", bumperName, x, y, config.Locked);
                        break;
                    }
                }
            } else {
                // Find matching light config and update
                for (auto& config : s_lightConfigs) {
                    if (strcmp(config.GroupName, group) == 0 && config.LightIndex == index) {
                        HDRLIGHT_LOG("Loading %s[%d]: pos (%.6f, %.6f) -> (%.6f, %.6f) preset=%s", 
                                     group, index, config.X, config.Y, x, y, presetName);
                        config.X = x;
                        config.Y = y;
                        config.Width = w;
                        config.Height = h;
                        config.R = r;
                        config.G = g;
                        config.B = b;
                        config.Locked = (locked != 0);
                        config.IntensityOn = intensityOn;
                        config.IntensityFlash = intensityFlash;
                        config.GlowRadius = glowRadius;
                        config.PresetName = presetName;
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

// ============== Preset Management ==============

void HDRLightOverlay::AddPreset(const HDRLightPreset& preset) {
    // Check if preset with same name already exists
    for (auto& existing : s_presets) {
        if (existing.Name == preset.Name) {
            existing = preset;  // Update existing
            
            // Propagate changes to all lights using this preset
            int updatedCount = 0;
            for (auto& config : s_lightConfigs) {
                if (config.PresetName == preset.Name) {
                    config.Width = preset.Width;
                    config.Height = preset.Height;
                    config.R = preset.R;
                    config.G = preset.G;
                    config.B = preset.B;
                    config.IntensityOn = preset.IntensityOn;
                    config.IntensityFlash = preset.IntensityFlash;
                    config.GlowRadius = preset.GlowRadius;
                    config.AboveBall = preset.AboveBall;
                    updatedCount++;
                }
            }
            HDRLIGHT_LOG("Updated existing preset: %s (propagated to %d lights)", preset.Name.c_str(), updatedCount);
            return;
        }
    }
    s_presets.push_back(preset);
    HDRLIGHT_LOG("Added new preset: %s", preset.Name.c_str());
}

void HDRLightOverlay::UpdatePreset(const std::string& name, const HDRLightPreset& preset) {
    for (auto& existing : s_presets) {
        if (existing.Name == name) {
            existing = preset;
            existing.Name = name;  // Keep original name
            
            // Update all lights using this preset
            for (auto& config : s_lightConfigs) {
                if (config.PresetName == name) {
                    config.Width = preset.Width;
                    config.Height = preset.Height;
                    config.R = preset.R;
                    config.G = preset.G;
                    config.B = preset.B;
                    config.IntensityOn = preset.IntensityOn;
                    config.IntensityFlash = preset.IntensityFlash;
                    config.GlowRadius = preset.GlowRadius;
                    config.AboveBall = preset.AboveBall;
                }
            }
            HDRLIGHT_LOG("Updated preset %s and %zu linked lights", name.c_str(), s_lightConfigs.size());
            return;
        }
    }
    HDRLIGHT_LOG("Preset not found for update: %s", name.c_str());
}

void HDRLightOverlay::DeletePreset(const std::string& name) {
    for (auto it = s_presets.begin(); it != s_presets.end(); ++it) {
        if (it->Name == name) {
            // Clear preset reference from all lights using it
            for (auto& config : s_lightConfigs) {
                if (config.PresetName == name) {
                    config.PresetName.clear();
                }
            }
            s_presets.erase(it);
            HDRLIGHT_LOG("Deleted preset: %s", name.c_str());
            return;
        }
    }
}

const HDRLightPreset* HDRLightOverlay::GetPreset(const std::string& name) {
    for (const auto& preset : s_presets) {
        if (preset.Name == name) {
            return &preset;
        }
    }
    return nullptr;
}

const std::vector<HDRLightPreset>& HDRLightOverlay::GetAllPresets() {
    return s_presets;
}

void HDRLightOverlay::AssignPresetToLight(int configIndex, const std::string& presetName) {
    if (configIndex < 0 || configIndex >= (int)s_lightConfigs.size()) return;
    
    const HDRLightPreset* preset = GetPreset(presetName);
    if (!preset) {
        HDRLIGHT_LOG("Cannot assign preset %s - not found", presetName.c_str());
        return;
    }
    
    auto& config = s_lightConfigs[configIndex];
    config.PresetName = presetName;
    config.Width = preset->Width;
    config.Height = preset->Height;
    config.R = preset->R;
    config.G = preset->G;
    config.B = preset->B;
    config.IntensityOn = preset->IntensityOn;
    config.IntensityFlash = preset->IntensityFlash;
    config.GlowRadius = preset->GlowRadius;
    config.AboveBall = preset->AboveBall;
    
    HDRLIGHT_LOG("Assigned preset %s to light %s[%d]", presetName.c_str(), config.GroupName, config.LightIndex);
}

void HDRLightOverlay::ClearPresetFromLight(int configIndex) {
    if (configIndex < 0 || configIndex >= (int)s_lightConfigs.size()) return;
    s_lightConfigs[configIndex].PresetName.clear();
}

HDRLightPreset HDRLightOverlay::CreatePresetFromLight(int configIndex, const std::string& presetName) {
    HDRLightPreset preset;
    preset.Name = presetName;
    
    if (configIndex >= 0 && configIndex < (int)s_lightConfigs.size()) {
        const auto& config = s_lightConfigs[configIndex];
        preset.Width = config.Width;
        preset.Height = config.Height;
        preset.R = config.R;
        preset.G = config.G;
        preset.B = config.B;
        preset.IntensityOn = config.IntensityOn;
        preset.IntensityFlash = config.IntensityFlash;
        preset.GlowRadius = config.GlowRadius;
        preset.AboveBall = config.AboveBall;
    } else {
        // Default values
        preset.Width = 0.04f;
        preset.Height = 0.04f;
        preset.R = 1.0f;
        preset.G = 1.0f;
        preset.B = 1.0f;
        preset.IntensityOn = 600.0f;
        preset.IntensityFlash = 1000.0f;
        preset.GlowRadius = 1.0f;
        preset.AboveBall = false;
    }
    
    return preset;
}

bool HDRLightOverlay::SavePresets(const char* filepath) {
    if (s_presets.empty()) {
        HDRLIGHT_LOG("No presets to save");
        return false;
    }
    
    FILE* f = fopen(filepath, "w");
    if (!f) {
        HDRLIGHT_LOG("Failed to open %s for writing presets", filepath);
        return false;
    }
    
    fprintf(f, "# HDR Light Presets\n");
    fprintf(f, "# Format: name,w,h,r,g,b,intensityOn,intensityFlash,glowRadius,aboveBall\n\n");
    
    for (const auto& preset : s_presets) {
        fprintf(f, "%s,%.6f,%.6f,%.3f,%.3f,%.3f,%.1f,%.1f,%.3f,%d\n",
                preset.Name.c_str(),
                preset.Width, preset.Height,
                preset.R, preset.G, preset.B,
                preset.IntensityOn, preset.IntensityFlash,
                preset.GlowRadius,
                preset.AboveBall ? 1 : 0);
    }
    
    fclose(f);
    HDRLIGHT_LOG("Saved %zu presets to %s", s_presets.size(), filepath);
    return true;
}

bool HDRLightOverlay::LoadPresets(const char* filepath) {
    FILE* f = fopen(filepath, "r");
    if (!f) {
        HDRLIGHT_LOG("No saved presets at %s", filepath);
        return false;
    }
    
    s_presets.clear();
    
    char line[256];
    while (fgets(line, sizeof(line), f)) {
        if (line[0] == '#' || line[0] == '\n') continue;
        
        char name[64];
        float w, h, r, g, b, intensityOn, intensityFlash, glowRadius;
        int aboveBall;
        
        if (sscanf(line, "%63[^,],%f,%f,%f,%f,%f,%f,%f,%f,%d",
                   name, &w, &h, &r, &g, &b, &intensityOn, &intensityFlash, &glowRadius, &aboveBall) == 10) {
            HDRLightPreset preset;
            preset.Name = name;
            preset.Width = w;
            preset.Height = h;
            preset.R = r;
            preset.G = g;
            preset.B = b;
            preset.IntensityOn = intensityOn;
            preset.IntensityFlash = intensityFlash;
            preset.GlowRadius = glowRadius;
            preset.AboveBall = (aboveBall != 0);
            s_presets.push_back(preset);
            HDRLIGHT_LOG("Loaded preset: %s", name);
        }
    }
    
    fclose(f);
    HDRLIGHT_LOG("Loaded %zu presets from %s", s_presets.size(), filepath);
    return true;
}

int HDRLightOverlay::GetPresetCount() {
    return (int)s_presets.size();
}

const char* HDRLightOverlay::GetPresetNameByIndex(int index) {
    if (index < 0 || index >= (int)s_presets.size()) return nullptr;
    return s_presets[index].Name.c_str();
}

void HDRLightOverlay::UpdateLightColor(const char* groupName, int lightIndex, float r, float g, float b) {
    for (auto& config : s_lightConfigs) {
        if (strcmp(config.GroupName, groupName) == 0 && config.LightIndex == lightIndex) {
            config.R = r;
            config.G = g;
            config.B = b;
            config.PresetName.clear();  // Clear preset when manually editing
            HDRLIGHT_LOG("Updated color for %s[%d]: (%.2f, %.2f, %.2f)", groupName, lightIndex, r, g, b);
            return;
        }
    }
}

void HDRLightOverlay::UpdateLightIntensity(const char* groupName, int lightIndex, float intensityOn, float intensityFlash) {
    for (auto& config : s_lightConfigs) {
        if (strcmp(config.GroupName, groupName) == 0 && config.LightIndex == lightIndex) {
            config.IntensityOn = intensityOn;
            config.IntensityFlash = intensityFlash;
            config.PresetName.clear();  // Clear preset when manually editing
            HDRLIGHT_LOG("Updated intensity for %s[%d]: on=%.1f flash=%.1f", groupName, lightIndex, intensityOn, intensityFlash);
            return;
        }
    }
}

void HDRLightOverlay::UpdateLightGlow(const char* groupName, int lightIndex, float glowRadius) {
    for (auto& config : s_lightConfigs) {
        if (strcmp(config.GroupName, groupName) == 0 && config.LightIndex == lightIndex) {
            config.GlowRadius = glowRadius;
            config.PresetName.clear();  // Clear preset when manually editing
            HDRLIGHT_LOG("Updated glow for %s[%d]: %.2f", groupName, lightIndex, glowRadius);
            return;
        }
    }
}

void HDRLightOverlay::UpdateLightLocked(const char* groupName, int lightIndex, bool locked) {
    for (auto& config : s_lightConfigs) {
        if (strcmp(config.GroupName, groupName) == 0 && config.LightIndex == lightIndex) {
            config.Locked = locked;
            HDRLIGHT_LOG("Updated locked for %s[%d]: %s", groupName, lightIndex, locked ? "true" : "false");
            return;
        }
    }
}

void HDRLightOverlay::NudgeLight(const char* groupName, int lightIndex, float dx, float dy) {
    for (auto& config : s_lightConfigs) {
        if (strcmp(config.GroupName, groupName) == 0 && config.LightIndex == lightIndex) {
            config.X += dx;
            config.Y += dy;
            HDRLIGHT_LOG("Nudged %s[%d] by (%.4f, %.4f) to (%.4f, %.4f)", groupName, lightIndex, dx, dy, config.X, config.Y);
            return;
        }
    }
    HDRLIGHT_LOG("NudgeLight: light %s[%d] not found in configs", groupName, lightIndex);
}

int HDRLightOverlay::ResetOutOfBoundsLights() {
    int resetCount = 0;
    // Game canvas aspect ratio is ~0.61 (width/height)
    // X coordinates are in range 0-0.61, Y in range 0-1
    const float canvasAspect = 0.61f;
    const float margin = 0.02f;  // Small margin inside viewport
    const float minX = margin;
    const float maxX = canvasAspect - margin;
    const float minY = margin;
    const float maxY = 1.0f - margin;
    
    for (auto& config : s_lightConfigs) {
        bool outOfBounds = false;
        
        // Check against actual canvas bounds (0.61 width, not 1.0)
        if (config.X < 0.0f || config.X > canvasAspect || 
            config.Y < 0.0f || config.Y > 1.0f) {
            outOfBounds = true;
        }
        
        // Also check for NaN
        if (config.X != config.X || config.Y != config.Y) {
            outOfBounds = true;
        }
        
        if (outOfBounds) {
            // Clamp to valid range
            config.X = std::max(minX, std::min(maxX, config.X));
            config.Y = std::max(minY, std::min(maxY, config.Y));
            
            // If still invalid (e.g., NaN), reset to center
            if (config.X != config.X || config.Y != config.Y) {  // NaN check
                config.X = canvasAspect / 2.0f;
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
        if (test.x < 0.0f || test.x > canvasAspect || test.y < 0.0f || test.y > 1.0f ||
            test.x != test.x || test.y != test.y) {
            test.x = std::max(minX, std::min(maxX, test.x));
            test.y = std::max(minY, std::min(maxY, test.y));
            resetCount++;
        }
    }
    
    HDRLIGHT_LOG("Reset %d out-of-bounds lights", resetCount);
    return resetCount;
}

void HDRLightOverlay::CreateTrailShader() {
    // Compile trail vertex shader
    GLuint vertShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertShader, 1, &s_trailVertexSrc, nullptr);
    glCompileShader(vertShader);
    
    GLint success;
    glGetShaderiv(vertShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(vertShader, 512, nullptr, infoLog);
        HDRLIGHT_LOG("Trail vertex shader error: %s", infoLog);
    }
    
    // Compile trail fragment shader
    GLuint fragShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragShader, 1, &s_trailFragmentSrc, nullptr);
    glCompileShader(fragShader);
    
    glGetShaderiv(fragShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(fragShader, 512, nullptr, infoLog);
        HDRLIGHT_LOG("Trail fragment shader error: %s", infoLog);
    }
    
    // Link program
    s_trailProgram = glCreateProgram();
    glAttachShader(s_trailProgram, vertShader);
    glAttachShader(s_trailProgram, fragShader);
    glLinkProgram(s_trailProgram);
    
    glGetProgramiv(s_trailProgram, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(s_trailProgram, 512, nullptr, infoLog);
        HDRLIGHT_LOG("Trail program link error: %s", infoLog);
    }
    
    glDeleteShader(vertShader);
    glDeleteShader(fragShader);
    
    // Create trail VAO and VBO
    glGenVertexArrays(1, &s_trailVAO);
    glGenBuffers(1, &s_trailVBO);
    
    glBindVertexArray(s_trailVAO);
    glBindBuffer(GL_ARRAY_BUFFER, s_trailVBO);
    // Allocate buffer for dynamic trail vertices: x, y, alpha, edge per vertex
    glBufferData(GL_ARRAY_BUFFER, MAX_TRAIL_VERTICES * 4 * sizeof(float), nullptr, GL_DYNAMIC_DRAW);
    
    // Position (x, y)
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // Alpha
    glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);
    // Edge
    glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(2);
    
    glBindVertexArray(0);
    
    // Create composite shader for rendering trail texture to screen
    GLuint compVertShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(compVertShader, 1, &s_trailCompositeVertexSrc, nullptr);
    glCompileShader(compVertShader);
    
    glGetShaderiv(compVertShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(compVertShader, 512, nullptr, infoLog);
        HDRLIGHT_LOG("Trail composite vertex shader error: %s", infoLog);
    }
    
    GLuint compFragShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(compFragShader, 1, &s_trailCompositeFragmentSrc, nullptr);
    glCompileShader(compFragShader);
    
    glGetShaderiv(compFragShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(compFragShader, 512, nullptr, infoLog);
        HDRLIGHT_LOG("Trail composite fragment shader error: %s", infoLog);
    }
    
    s_trailCompositeProgram = glCreateProgram();
    glAttachShader(s_trailCompositeProgram, compVertShader);
    glAttachShader(s_trailCompositeProgram, compFragShader);
    glLinkProgram(s_trailCompositeProgram);
    
    glGetProgramiv(s_trailCompositeProgram, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(s_trailCompositeProgram, 512, nullptr, infoLog);
        HDRLIGHT_LOG("Trail composite program link error: %s", infoLog);
    }
    
    glDeleteShader(compVertShader);
    glDeleteShader(compFragShader);
    
    HDRLIGHT_LOG("Trail shader created successfully");
}

void HDRLightOverlay::RenderTrailMesh(float maxNits) {
    if (s_ballTrail.size() < 2) return;
    if (s_trailProgram == 0) {
        static int warnCount = 0;
        if (warnCount++ < 5) HDRLIGHT_LOG("Trail program not initialized!");
        return;
    }
    
    // Build smoothed trail using Catmull-Rom spline interpolation
    // First, build a list of valid points with their properties
    struct SmoothPoint {
        float x, y, fade, vx, vy;
    };
    std::vector<SmoothPoint> validPoints;
    validPoints.reserve(s_ballTrail.size());
    
    for (size_t i = 0; i < s_ballTrail.size(); i++) {
        const TrailPoint& p = s_ballTrail[i];
        
        // Check for break marker (negative timestamp from NotifyBallTeleported)
        if (p.timestamp < 0) {
            // Insert a break marker into validPoints
            if (!validPoints.empty()) {
                validPoints.push_back({0, 0, -1.0f, 0, 0});
            }
            continue;
        }
        
        // Calculate age-based fade
        float age = s_trailTime - p.timestamp;
        float fade = 1.0f - (age / s_trailLifetimeSetting);
        if (fade <= 0.0f) continue;
        
        // Calculate velocity-based opacity
        float vel = sqrtf(p.vx * p.vx + p.vy * p.vy);
        float velFactor = fminf(vel / 1.5f, 1.0f);
        velFactor = sqrtf(velFactor);
        fade *= velFactor;
        
        // Skip points too close to ball
        float ballDx = p.x - s_debugBallX;
        float ballDy = p.y - s_debugBallY;
        float ballDist = sqrtf(ballDx * ballDx + ballDy * ballDy);
        if (ballDist < 0.012f) continue;
        
        // Check for teleportation by distance - compare to LAST added valid point
        // If distance is too large, insert a break marker before this point
        if (!validPoints.empty() && validPoints.back().fade >= 0) {
            float segDx = p.x - validPoints.back().x;
            float segDy = p.y - validPoints.back().y;
            float segDist = sqrtf(segDx * segDx + segDy * segDy);
            if (segDist > 0.15f) {
                // Insert a break marker (fade = -1) before this point
                validPoints.push_back({0, 0, -1.0f, 0, 0});
            }
        }
        
        // Add this point
        validPoints.push_back({p.x, p.y, fade, p.vx, p.vy});
    }
    
    if (validPoints.size() < 2) return;
    
    // Split validPoints into separate segments at break markers
    std::vector<std::vector<SmoothPoint>> segments;
    segments.push_back(std::vector<SmoothPoint>());
    
    for (size_t i = 0; i < validPoints.size(); i++) {
        if (validPoints[i].fade < 0) {
            // Break marker - start a new segment if current one has points
            if (!segments.back().empty()) {
                segments.push_back(std::vector<SmoothPoint>());
            }
        } else {
            segments.back().push_back(validPoints[i]);
        }
    }
    
    // Remove empty segments
    segments.erase(std::remove_if(segments.begin(), segments.end(),
        [](const std::vector<SmoothPoint>& seg) { return seg.size() < 2; }), segments.end());
    
    if (segments.empty()) return;
    
    // Catmull-Rom interpolation helper lambda
    auto catmullRom = [](float p0, float p1, float p2, float p3, float t) -> float {
        float t2 = t * t;
        float t3 = t2 * t;
        return 0.5f * ((2.0f * p1) +
                       (-p0 + p2) * t +
                       (2.0f * p0 - 5.0f * p1 + 4.0f * p2 - p3) * t2 +
                       (-p0 + 3.0f * p1 - 3.0f * p2 + p3) * t3);
    };
    
    // Catmull-Rom derivative (tangent) helper lambda
    auto catmullRomDerivative = [](float p0, float p1, float p2, float p3, float t) -> float {
        float t2 = t * t;
        return 0.5f * ((-p0 + p2) +
                       2.0f * (2.0f * p0 - 5.0f * p1 + 4.0f * p2 - p3) * t +
                       3.0f * (-p0 + 3.0f * p1 - 3.0f * p2 + p3) * t2);
    };
    
    // Build vertices for all segments and track segment boundaries
    std::vector<float> vertices;
    std::vector<std::pair<size_t, size_t>> segmentRanges;  // start vertex, count
    vertices.reserve(validPoints.size() * 8 * 4);
    
    for (const auto& segment : segments) {
        size_t startVertex = vertices.size() / 4;
        
        for (size_t i = 0; i < segment.size(); i++) {
            // Get 4 control points for Catmull-Rom (clamp at ends)
            size_t i0 = (i > 0) ? i - 1 : 0;
            size_t i1 = i;
            size_t i2 = (i + 1 < segment.size()) ? i + 1 : i;
            size_t i3 = (i + 2 < segment.size()) ? i + 2 : i2;
            
            bool hasNext = (i + 1 < segment.size());
            
            // If no next point, just render raw point
            if (!hasNext) {
                const SmoothPoint& sp = segment[i];
                float widthFactor = sqrtf(sp.fade);
                float width = 0.006f * widthFactor;
                
                float dx = sp.vx, dy = sp.vy;
                float len = sqrtf(dx * dx + dy * dy);
                if (len < 0.0001f) { dx = 0; dy = 1; len = 1; }
                dx /= len; dy /= len;
                float perpX = -dy, perpY = dx;
                
                vertices.push_back(sp.x + perpX * width);
                vertices.push_back(sp.y + perpY * width);
                vertices.push_back(sp.fade);
                vertices.push_back(0.3f);
                vertices.push_back(sp.x - perpX * width);
                vertices.push_back(sp.y - perpY * width);
                vertices.push_back(sp.fade);
                vertices.push_back(0.3f);
                continue;
            }
            
            // Interpolate between i1 and i2
            int numSteps = 3;
            for (int step = 0; step <= numSteps; step++) {
                float t = (float)step / (float)numSteps;
                
                float x = catmullRom(segment[i0].x, segment[i1].x, segment[i2].x, segment[i3].x, t);
                float y = catmullRom(segment[i0].y, segment[i1].y, segment[i2].y, segment[i3].y, t);
                float fade = segment[i1].fade * (1.0f - t) + segment[i2].fade * t;
                
                float dx = catmullRomDerivative(segment[i0].x, segment[i1].x, segment[i2].x, segment[i3].x, t);
                float dy = catmullRomDerivative(segment[i0].y, segment[i1].y, segment[i2].y, segment[i3].y, t);
                float len = sqrtf(dx * dx + dy * dy);
                if (len > 0.0001f) { dx /= len; dy /= len; }
                else {
                    dx = segment[i2].x - segment[i1].x;
                    dy = segment[i2].y - segment[i1].y;
                    len = sqrtf(dx * dx + dy * dy);
                    if (len > 0.0001f) { dx /= len; dy /= len; }
                    else { dx = 0; dy = 1; }
                }
                
                float perpX = -dy;
                float perpY = dx;
                float widthFactor = sqrtf(fade);
                float width = 0.006f * widthFactor;
                
                vertices.push_back(x + perpX * width);
                vertices.push_back(y + perpY * width);
                vertices.push_back(fade);
                vertices.push_back(0.3f);
                vertices.push_back(x - perpX * width);
                vertices.push_back(y - perpY * width);
                vertices.push_back(fade);
                vertices.push_back(0.3f);
            }
        }
        
        size_t vertexCount = (vertices.size() / 4) - startVertex;
        if (vertexCount >= 4) {
            segmentRanges.push_back({startVertex, vertexCount});
        }
    }
    
    if (vertices.empty() || segmentRanges.empty()) return;
    
    // Clamp to buffer size to prevent overflow
    size_t maxFloats = MAX_TRAIL_VERTICES * 4;
    if (vertices.size() > maxFloats) {
        vertices.resize(maxFloats);
        // Recalculate segment ranges that fit
        segmentRanges.erase(std::remove_if(segmentRanges.begin(), segmentRanges.end(),
            [maxFloats](const std::pair<size_t, size_t>& range) {
                return range.first >= maxFloats / 4;
            }), segmentRanges.end());
    }
    
    static int trailLogCount = 0;
    if (trailLogCount++ % 60 == 0) {
        HDRLIGHT_LOG("Trail: %zu segments, %zu vertices, program=%u", 
                     segmentRanges.size(), vertices.size() / 4, s_trailProgram);
    }
    
    // Get current viewport size for FBO
    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);
    int viewWidth = viewport[2];
    int viewHeight = viewport[3];
    
    // Create or resize trail FBO if needed
    if (s_trailFBO == 0 || s_trailFBOWidth != viewWidth || s_trailFBOHeight != viewHeight) {
        // Clean up old resources
        if (s_trailFBO != 0) {
            glDeleteFramebuffers(1, &s_trailFBO);
            glDeleteTextures(1, &s_trailTexture);
        }
        
        // Create FBO
        glGenFramebuffers(1, &s_trailFBO);
        glBindFramebuffer(GL_FRAMEBUFFER, s_trailFBO);
        
        // Create texture for trail rendering
        glGenTextures(1, &s_trailTexture);
        glBindTexture(GL_TEXTURE_2D, s_trailTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, viewWidth, viewHeight, 0, GL_RGBA, GL_HALF_FLOAT, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, s_trailTexture, 0);
        
        GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
        if (status != GL_FRAMEBUFFER_COMPLETE) {
            HDRLIGHT_LOG("Trail FBO incomplete: 0x%x", status);
        }
        
        s_trailFBOWidth = viewWidth;
        s_trailFBOHeight = viewHeight;
        
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        HDRLIGHT_LOG("Created trail FBO: %dx%d", viewWidth, viewHeight);
    }
    
    // Upload vertices
    glBindBuffer(GL_ARRAY_BUFFER, s_trailVBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, vertices.size() * sizeof(float), vertices.data());
    
    // === PASS 1: Render trail to texture with GL_MAX blending ===
    glBindFramebuffer(GL_FRAMEBUFFER, s_trailFBO);
    glViewport(0, 0, s_trailFBOWidth, s_trailFBOHeight);
    
    // Clear trail texture
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    
    // Use GL_MAX blending - overlapping areas take maximum, no accumulation
    glBlendEquation(GL_MAX);
    glBlendFunc(GL_ONE, GL_ONE);
    
    glUseProgram(s_trailProgram);
    glBindVertexArray(s_trailVAO);
    
    // Set camera transform uniforms (get from HDRRenderer)
    float cameraZoom = HDRRenderer::GetCurrentCameraZoom();
    float cameraCenterX = HDRRenderer::GetCurrentCameraCenterX();
    float cameraCenterY = HDRRenderer::GetCurrentCameraCenterY();
    
    GLint trailZoomLoc = glGetUniformLocation(s_trailProgram, "uCameraZoom");
    GLint trailCenterLoc = glGetUniformLocation(s_trailProgram, "uCameraCenter");
    glUniform1f(trailZoomLoc, cameraZoom);
    glUniform2f(trailCenterLoc, cameraCenterX, cameraCenterY);
    
    // Set uniforms for texture pass (no PQ encoding)
    GLint colorLoc = glGetUniformLocation(s_trailProgram, "uTrailColor");
    glUniform3f(colorLoc, 0.8f, 0.9f, 1.0f);
    
    GLint intensityLoc = glGetUniformLocation(s_trailProgram, "uIntensityNits");
    glUniform1f(intensityLoc, 400.0f);
    
    // Draw each segment separately
    for (const auto& range : segmentRanges) {
        glDrawArrays(GL_TRIANGLE_STRIP, (GLint)range.first, (GLsizei)range.second);
    }
    
    glBindVertexArray(0);
    
    // === PASS 2: Composite trail texture to screen with PQ encoding ===
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);
    
    // Restore standard alpha blending for compositing
    glBlendEquation(GL_FUNC_ADD);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    glUseProgram(s_trailCompositeProgram);
    
    // Bind trail texture
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, s_trailTexture);
    
    GLint texLoc = glGetUniformLocation(s_trailCompositeProgram, "uTrailTexture");
    glUniform1i(texLoc, 0);
    
    GLint opacityLoc = glGetUniformLocation(s_trailCompositeProgram, "uMaxOpacity");
    glUniform1f(opacityLoc, s_trailOpacity);
    
    // Draw fullscreen quad using the overlay VAO
    glBindVertexArray(s_overlayVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
    
    // Restore additive blending for other overlays
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
}
