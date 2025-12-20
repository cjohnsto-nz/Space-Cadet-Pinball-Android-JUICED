#include "pch.h"
#include "HDRRenderer.h"
#include "HDRConfig.h"

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
GLuint HDRRenderer::s_hdrFBO = 0;
GLuint HDRRenderer::s_hdrTexture = 0;
GLuint HDRRenderer::s_sdrTexture = 0;
GLuint HDRRenderer::s_outputProgram = 0;
GLuint HDRRenderer::s_uploadProgram = 0;
GLuint HDRRenderer::s_quadVAO = 0;
GLuint HDRRenderer::s_quadVBO = 0;
float HDRRenderer::s_exposure = 1.0f;

// Vertex shader - simple fullscreen quad
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

// Subtle vibrance boost - increases saturation of less saturated colors more
vec3 applyVibrance(vec3 color, float amount) {
    float luminance = dot(color, vec3(0.2126, 0.7152, 0.0722));
    float maxComponent = max(max(color.r, color.g), color.b);
    float minComponent = min(min(color.r, color.g), color.b);
    float saturation = (maxComponent > 0.0) ? (maxComponent - minComponent) / maxComponent : 0.0;
    
    // Vibrance affects less saturated colors more
    float vibranceAmount = amount * (1.0 - saturation);
    
    return mix(vec3(luminance), color, 1.0 + vibranceAmount);
}

// Gentle gamut expansion - slightly push colors toward BT.2020 primaries
vec3 expandGamut(vec3 bt709Color, float amount) {
    // This matrix slightly expands colors beyond BT.709 toward BT.2020
    // amount of 0.0 = no expansion, 1.0 = full expansion
    mat3 expand = mat3(
        1.0 + 0.1 * amount, -0.05 * amount, -0.05 * amount,
        -0.05 * amount, 1.0 + 0.1 * amount, -0.05 * amount,
        -0.05 * amount, -0.05 * amount, 1.0 + 0.1 * amount
    );
    return expand * bt709Color;
}

void main() {
    vec4 hdrColor = texture(uHDRTexture, vTexCoord);
    
    // Apply vibrance boost (0.25 = 25% boost on desaturated colors)
    vec3 vibranceColor = applyVibrance(hdrColor.rgb, 0.25);
    
    // Apply gamut expansion (0.35 = 35% toward wider gamut)
    vec3 expandedColor = expandGamut(vibranceColor, 0.35);
    
    // Subtle midpoint adjustment (darken midtones slightly for more punch)
    // Using a gentle power curve: values < 1.0 darken midtones
    expandedColor = pow(expandedColor, vec3(1.1));
    
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
    
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindTexture(GL_TEXTURE_2D, 0);
    
    s_initialized = true;
    HDR_LOG("HDR renderer initialized successfully");
    
    return true;
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
    
    // Upload SDR pixels to texture using glTexImage2D (more compatible than glTexSubImage2D)
    // Note: ColorRgba is BGRA format - we swap R/B in the shader
    glBindTexture(GL_TEXTURE_2D, s_sdrTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
    
    GLenum err = glGetError();
    if (err != GL_NO_ERROR) {
        HDR_ERR("GL error after texture upload: 0x%x", err);
    }
    
    // Render to HDR framebuffer with default intensity (SDR white)
    glBindFramebuffer(GL_FRAMEBUFFER, s_hdrFBO);
    glViewport(0, 0, s_width, s_height);
    glClear(GL_COLOR_BUFFER_BIT);
    
    glUseProgram(s_uploadProgram);
    
    // Set uniforms
    GLint texLoc = glGetUniformLocation(s_uploadProgram, "uTexture");
    GLint intensityLoc = glGetUniformLocation(s_uploadProgram, "uIntensityMultiplier");
    GLint exposureLoc = glGetUniformLocation(s_uploadProgram, "uExposure");
    
    HDR_LOG("UploadTexture: uniform locations tex=%d, intensity=%d, exposure=%d", texLoc, intensityLoc, exposureLoc);
    
    glUniform1i(texLoc, 0);
    glUniform1f(intensityLoc, 1.0f);  // Default SDR intensity
    glUniform1f(exposureLoc, s_exposure);
    
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
    
    HDR_LOG("Present: screen %dx%d, texture %dx%d, viewport %d,%d %dx%d", 
            screenWidth, screenHeight, s_width, s_height, viewportX, viewportY, viewportW, viewportH);
    
    // Bind default framebuffer (screen)
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    
    // Clear the entire screen first (for letterbox/pillarbox areas)
    glViewport(0, 0, screenWidth, screenHeight);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    
    // Set viewport to maintain aspect ratio
    glViewport(viewportX, viewportY, viewportW, viewportH);
    
    glUseProgram(s_outputProgram);
    
    // Set uniforms
    GLint texLoc = glGetUniformLocation(s_outputProgram, "uHDRTexture");
    GLint maxNitsLoc = glGetUniformLocation(s_outputProgram, "uMaxNits");
    GLint sdrWhiteLoc = glGetUniformLocation(s_outputProgram, "uSDRWhiteNits");
    
    HDR_LOG("Present: uniform locations tex=%d, maxNits=%d, sdrWhite=%d", texLoc, maxNitsLoc, sdrWhiteLoc);
    
    glUniform1i(texLoc, 0);
    glUniform1f(maxNitsLoc, HDR::GetMaxDisplayNits());
    glUniform1f(sdrWhiteLoc, HDR::Luminance::SDR_WHITE_NITS);
    
    HDR_LOG("Present: maxNits=%.1f, sdrWhite=%.1f", HDR::GetMaxDisplayNits(), HDR::Luminance::SDR_WHITE_NITS);
    
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, s_hdrTexture);
    
    // Disable depth test and blending for fullscreen quad
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
    
    // Draw fullscreen quad
    glBindVertexArray(s_quadVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
    
    glUseProgram(0);
    
    // Check for GL errors
    GLenum err = glGetError();
    if (err != GL_NO_ERROR) {
        HDR_ERR("GL error in Present: 0x%x", err);
    }
}

void HDRRenderer::SetExposure(float exposure) {
    s_exposure = exposure;
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
