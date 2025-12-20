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
};

class TLightGroup;
class TLight;

class HDRLightOverlay {
public:
    static void Init();
    static void Uninit();
    
    // Register a light group for HDR overlay tracking
    static void RegisterLightGroup(const char* groupName, TLightGroup* group);
    
    // Register an individual light for HDR overlay tracking
    static void RegisterIndividualLight(const char* lightName, TLight* light);
    
    // Add a light configuration
    static void AddLightConfig(const HDRLightConfig& config);
    
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
    static const std::vector<HDRLightConfig>& GetLightConfigs();
    static int ResetOutOfBoundsLights();  // Returns count of lights reset

private:
    struct RegisteredGroup {
        const char* name;
        TLightGroup* group;
    };
    
    struct RegisteredLight {
        const char* name;
        TLight* light;
    };
    
    struct LightState {
        const HDRLightConfig* config;
        TLight* light;
        bool isOn;
        bool isFlashing;
        float currentIntensity;
    };
    
    struct TestLight {
        float x, y, w, h;
        float r, g, b;
        float intensity;
    };
    
    static std::vector<RegisteredGroup> s_registeredGroups;
    static std::vector<RegisteredLight> s_registeredLights;
    static std::vector<HDRLightConfig> s_lightConfigs;
    static std::vector<LightState> s_lightStates;
    static std::vector<TestLight> s_testLights;
    
    static bool s_initialized;
    static GLuint s_overlayProgram;
    static GLuint s_overlayProgramPQ;  // PQ-encoding program for direct screen output
    static GLuint s_overlayVAO;
    static GLuint s_overlayVBO;
    
    static void CreateShaders();
    static void CreateQuad();
    static void RenderSingleLight(const LightState& state, int texWidth, int texHeight);
    static void RenderSingleLightPQ(const LightState& state, float maxNits);
};
