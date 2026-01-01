#pragma once

#include "pch.h"
#include "HDRConfig.h"
#include "gdrv.h"

#ifdef __ANDROID__
#include <GLES3/gl3.h>
#include <GLES3/gl3ext.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>
#else
#include <SDL_opengl.h>
#endif

class HDRRenderer {
public:
    static bool Init(int width, int height);
    static void Uninit();
    static bool IsInitialized();
    
    // Upload SDR texture data and apply HDR intensity
    static void UploadTexture(const ColorRgba* pixels, int width, int height);
    
    // Apply HDR intensity to a specific region (for lights)
    static void ApplyHDRIntensity(int x, int y, int width, int height, float intensityNits);
    
    // Render the HDR framebuffer to screen with PQ encoding
    static void Present(int screenWidth, int screenHeight);
    
    // Get the HDR framebuffer texture for blending
    static GLuint GetHDRTexture() { return s_hdrTexture; }
    
    // Set overall scene exposure/brightness
    static void SetExposure(float exposure);
    
    // Check if we should use HDR path
    static bool ShouldUseHDR();
    
    // Camera tracking mode
    static void SetCameraTracking(bool enabled, float zoom);
    static bool IsCameraTrackingEnabled() { return s_cameraTrackingEnabled; }
    static float GetCameraZoom() { return s_cameraZoom; }
    static float GetCurrentCameraZoom() { return s_currentCameraZoom; }
    static float GetCurrentCameraCenterX() { return s_currentCameraCenterX; }
    static float GetCurrentCameraCenterY() { return s_currentCameraCenterY; }
    
    // Get current viewport (for touch coordinate conversion)
    static int GetViewportX() { return s_viewportX; }
    static int GetViewportY() { return s_viewportY; }
    static int GetViewportW() { return s_viewportW; }
    static int GetViewportH() { return s_viewportH; }

private:
    static bool s_initialized;
    static int s_width;
    static int s_height;
    
    // Current viewport (updated in Present)
    static int s_viewportX;
    static int s_viewportY;
    static int s_viewportW;
    static int s_viewportH;
    
    // OpenGL resources
    static GLuint s_hdrFBO;           // HDR framebuffer object
    static GLuint s_hdrTexture;       // FP16 HDR texture (GL_RGBA16F)
    static GLuint s_sdrTexture;       // Input SDR texture (GL_RGBA8)
    static GLuint s_outputProgram;    // Shader for HDR to PQ conversion
    static GLuint s_uploadProgram;    // Shader for SDR to HDR conversion
    static GLuint s_quadVAO;          // Fullscreen quad VAO
    static GLuint s_quadVBO;          // Fullscreen quad VBO
    
    static float s_exposure;
    
    // Camera tracking
    static bool s_cameraTrackingEnabled;
    static float s_cameraZoom;
    static float s_currentCameraZoom;      // Current frame's zoom (for overlay rendering)
    static float s_currentCameraCenterX;   // Current frame's camera center X
    static float s_currentCameraCenterY;   // Current frame's camera center Y
    
    // Cached uniform locations (avoid glGetUniformLocation every frame)
    // Output program uniforms
    static GLint s_loc_uHDRTexture;
    static GLint s_loc_uMaxNits;
    static GLint s_loc_uSDRWhiteNits;
    static GLint s_loc_uCameraZoom;
    static GLint s_loc_uCameraCenter;
    static GLint s_loc_uViewportSize;
    static GLint s_loc_uViewportOffset;
    static GLint s_loc_uRawBallPos;
    static GLint s_loc_uLastBallPos;
    static GLint s_loc_uBallValid;
    static GLint s_loc_uSmoothedCameraPos;
    // Upload program uniforms
    static GLint s_loc_upload_uTexture;
    static GLint s_loc_upload_uIntensityMultiplier;
    static GLint s_loc_upload_uExposure;
    
    static void CacheUniformLocations();
    
    // Shader compilation helpers
    static GLuint CompileShader(GLenum type, const char* source);
    static GLuint CreateProgram(const char* vertexSrc, const char* fragmentSrc);
    static void CreateFullscreenQuad();
    
    // Shader sources
    static const char* s_vertexShaderSrc;
    static const char* s_hdrUploadFragmentSrc;
    static const char* s_pqOutputFragmentSrc;
};
