#pragma once

#include "pch.h"
#include <vector>

#ifdef __ANDROID__
#include <GLES3/gl3.h>
#endif

// HDR Light overlay configuration
struct HDRLightConfig {
    const char* GroupName;      // Name of the light group (e.g., "skill_shot_lights")
    int LightIndex;             // Index within the group
    float X, Y;                 // Position relative to game texture (0-1 normalized)
    float Width, Height;        // Size of the light overlay (0-1 normalized)
    float R, G, B;              // Color (linear, can be > 1.0 for HDR)
    float IntensityOn;          // Intensity when light is on (in nits, e.g., 600)
    float IntensityFlash;       // Intensity when flashing (in nits, e.g., 1000)
    float GlowRadius;           // Radius of glow effect (0-1)
    bool AboveBall;             // True if light is above ball (not occluded), false if below (can be occluded)
};

class TLightGroup;
class TLight;
class TBumper;

// Bumper HDR light configuration - supports dynamic color based on upgrade level
struct HDRBumperConfig {
    const char* BumperName;     // Name of the bumper (e.g., "bump1")
    float X, Y;                 // Position relative to game texture (0-1 normalized)
    float Width, Height;        // Size of the light overlay (0-1 normalized)
    float Colors[4][3];         // RGB colors for each upgrade level (0-3)
    float Intensity;            // Intensity in nits
    float GlowRadius;           // Radius of glow effect (0-1)
};

class HDRLightOverlay {
public:
    static void Init();
    static void Uninit();
    
    // Register a light group for HDR overlay tracking
    static void RegisterLightGroup(const char* groupName, TLightGroup* group);
    
    // Register an individual light for HDR overlay tracking
    static void RegisterIndividualLight(const char* lightName, TLight* light);
    
    // Register a bumper for HDR overlay tracking
    static void RegisterBumper(const char* bumperName, TBumper* bumper);
    
    // Add a light configuration
    static void AddLightConfig(const HDRLightConfig& config);
    
    // Add a bumper configuration
    static void AddBumperConfig(const HDRBumperConfig& config);
    
    // Update light states from game - call each frame before rendering
    static void UpdateLightStates();
    
    // Render HDR light overlays to the HDR framebuffer
    // Called after base game is rendered but before PQ output
    static void RenderOverlays(int textureWidth, int textureHeight);
    
    // Check if any lights are active
    static bool HasActiveLights();
    
    // Add a test light that's always on (for debugging)
    static void AddTestLight(float x, float y, float w, float h, float r, float g, float b, float intensity);
    
    // Debug mode - when true, all configured lights render at full intensity regardless of game state
    static void SetDebugAllLightsOn(bool enabled);
    static bool GetDebugAllLightsOn();
    
    // Render overlays with PQ encoding directly to screen (called after main PQ pass)
    static void RenderOverlaysPQ(int viewportX, int viewportY, int viewportW, int viewportH,
                                  int texWidth, int texHeight, float maxNits);
    
    // Light position editor
    static void SetEditMode(bool enabled);
    static bool GetEditMode();
    static void OnTouchDown(float screenX, float screenY, int viewportX, int viewportY, int viewportW, int viewportH);
    static void OnTouchMove(float screenX, float screenY, int viewportX, int viewportY, int viewportW, int viewportH);
    static void OnTouchUp();
    static bool SaveLightPositions(const char* filepath);
    static bool LoadLightPositions(const char* filepath);
    static void UpdateLightPosition(int configIndex, float x, float y);
    static void UpdateLightSize(int configIndex, float w, float h);
    static int GetSelectedLightIndex();
    static int GetSelectedBumperIndex();
    static bool HasSelection();  // Returns true if any light or bumper is selected
    static const std::vector<HDRLightConfig>& GetLightConfigs();
    static int ResetOutOfBoundsLights();  // Returns count of lights reset
    
    // Debug ball position tracking
    static void SetDebugBallPosition(float x, float y);  // Set current ball position (normalized 0-1)
    static void NotifyBallTeleported();  // Call when ball teleports (enters/exits sink, respawns, etc.)
    static void EnableDebugBall(bool enabled);  // Toggle debug ball visibility
    static float GetBallX() { return s_debugBallX; }
    static float GetBallY() { return s_debugBallY; }
    static bool IsBallActive() { return s_debugBallEnabled; }  // Returns true if ball position is being tracked
    
    // Global glow intensity modifier (0.0 to 2.0, default 1.0)
    static void SetGlowModifier(float modifier);
    static float GetGlowModifier() { return s_glowModifier; }
    
    // Trail settings
    static void SetTrailOpacity(float opacity);  // 0.0 to 1.0
    static float GetTrailOpacity() { return s_trailOpacity; }
    static void SetTrailLifetime(float seconds);  // in seconds
    static float GetTrailLifetime() { return s_trailLifetimeSetting; }

private:
    struct RegisteredGroup {
        const char* name;
        TLightGroup* group;
    };
    
    struct RegisteredLight {
        const char* name;
        TLight* light;
    };
    
    struct RegisteredBumper {
        const char* name;
        TBumper* bumper;
    };
    
    struct LightState {
        const HDRLightConfig* config;
        TLight* light;
        bool isOn;
        bool isFlashing;
        float currentIntensity;
    };
    
    struct BumperState {
        const HDRBumperConfig* config;
        TBumper* bumper;
        int upgradeLevel;       // 0-3 based on BmpIndex
        float currentIntensity;
        float r, g, b;          // Current color based on upgrade level
    };
    
    struct TestLight {
        float x, y, w, h;
        float r, g, b;
        float intensity;
    };
    
    static std::vector<RegisteredGroup> s_registeredGroups;
    static std::vector<RegisteredLight> s_registeredLights;
    static std::vector<RegisteredBumper> s_registeredBumpers;
    static std::vector<HDRLightConfig> s_lightConfigs;
    static std::vector<HDRBumperConfig> s_bumperConfigs;
    static std::vector<LightState> s_lightStates;
    static std::vector<BumperState> s_bumperStates;
    static std::vector<TestLight> s_testLights;
    
    // Debug ball tracking
    static float s_debugBallX, s_debugBallY;
    static bool s_debugBallEnabled;
    
    // Global glow modifier
    static float s_glowModifier;
    
    // Trail settings (user-configurable)
    static float s_trailOpacity;        // 0.0 to 1.0, default 0.85
    static float s_trailLifetimeSetting; // in seconds, default 3.5
    
    // Ball trail effect - continuous adaptive trail
    struct TrailPoint {
        float x, y;
        float vx, vy;     // velocity at this point
        float timestamp;  // when this point was added
    };
    static std::vector<TrailPoint> s_ballTrail;
    static constexpr int MAX_TRAIL_POINTS = 600;  // Allow many more points for long trails
    static constexpr float TRAIL_LIFETIME = 3.5f;  // seconds before trail fades completely
    static float s_lastBallX, s_lastBallY;
    static float s_trailTime;  // accumulated time for trail aging
    static bool s_ballTeleported;  // flag set when ball teleports, cleared on next position update
    
    static bool s_initialized;
    static GLuint s_overlayProgram;
    static GLuint s_overlayProgramPQ;  // PQ-encoding program for direct screen output
    static GLuint s_overlayVAO;
    static GLuint s_overlayVBO;
    
    // Trail mesh rendering
    static GLuint s_trailProgram;
    static GLuint s_trailVAO;
    static GLuint s_trailVBO;
    static constexpr int MAX_TRAIL_VERTICES = 8192;
    
    // Trail FBO for render-to-texture (fixes self-intersection)
    static GLuint s_trailFBO;
    static GLuint s_trailTexture;
    static GLuint s_trailCompositeProgram;
    static int s_trailFBOWidth;
    static int s_trailFBOHeight;
    
    static void CreateShaders();
    static void CreateQuad();
    static void CreateTrailShader();
    static void RenderSingleLight(const LightState& state, int texWidth, int texHeight);
    static void RenderSingleLightPQ(const LightState& state, float maxNits);
    static void RenderTrailMesh(float maxNits);
};
