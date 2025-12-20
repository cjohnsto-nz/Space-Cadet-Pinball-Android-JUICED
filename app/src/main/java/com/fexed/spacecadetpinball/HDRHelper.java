package com.fexed.spacecadetpinball;

import android.app.Activity;
import android.content.Context;
import android.hardware.display.DisplayManager;
import android.os.Build;
import android.util.Log;
import android.view.Display;

import javax.microedition.khronos.egl.EGL10;
import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.egl.EGLContext;
import javax.microedition.khronos.egl.EGLDisplay;

public class HDRHelper {
    private static final String TAG = "HDRHelper";
    
    // EGL extension constants
    private static final int EGL_COLOR_COMPONENT_TYPE_EXT = 0x3339;
    private static final int EGL_COLOR_COMPONENT_TYPE_FLOAT_EXT = 0x333B;
    private static final int EGL_GL_COLORSPACE_BT2020_LINEAR_EXT = 0x333F;
    private static final int EGL_GL_COLORSPACE_BT2020_PQ_EXT = 0x3340;
    private static final int EGL_GL_COLORSPACE_SCRGB_LINEAR_EXT = 0x3350;
    
    public static class HDRCapabilities {
        public boolean isSupported = false;
        public boolean isBT2020Supported = false;
        public boolean isPQSupported = false;
        public boolean isScRGBSupported = false;
        public boolean isFP16Supported = false;
        public float maxLuminanceNits = 203.0f;  // SDR default
        public float minLuminanceNits = 0.005f;
        
        @Override
        public String toString() {
            return "HDRCapabilities{" +
                    "isSupported=" + isSupported +
                    ", isBT2020Supported=" + isBT2020Supported +
                    ", isPQSupported=" + isPQSupported +
                    ", isScRGBSupported=" + isScRGBSupported +
                    ", isFP16Supported=" + isFP16Supported +
                    ", maxLuminanceNits=" + maxLuminanceNits +
                    ", minLuminanceNits=" + minLuminanceNits +
                    '}';
        }
    }
    
    public static HDRCapabilities queryHDRCapabilities(Context context) {
        HDRCapabilities caps = new HDRCapabilities();
        
        // Check Android version - HDR support requires Android 8.0+ (API 26)
        if (Build.VERSION.SDK_INT < Build.VERSION_CODES.O) {
            Log.i(TAG, "HDR not supported: Android version too low (requires API 26+)");
            return caps;
        }
        
        // Query display HDR capabilities
        try {
            DisplayManager displayManager = (DisplayManager) context.getSystemService(Context.DISPLAY_SERVICE);
            if (displayManager != null) {
                Display display = displayManager.getDisplay(Display.DEFAULT_DISPLAY);
                if (display != null) {
                    Display.HdrCapabilities hdrCaps = display.getHdrCapabilities();
                    if (hdrCaps != null) {
                        int[] supportedTypes = hdrCaps.getSupportedHdrTypes();
                        
                        for (int type : supportedTypes) {
                            if (type == Display.HdrCapabilities.HDR_TYPE_HDR10) {
                                caps.isBT2020Supported = true;
                                caps.isPQSupported = true;
                                caps.isSupported = true;
                                Log.i(TAG, "HDR10 supported");
                            } else if (type == Display.HdrCapabilities.HDR_TYPE_HLG) {
                                caps.isBT2020Supported = true;
                                caps.isSupported = true;
                                Log.i(TAG, "HLG supported");
                            } else if (type == Display.HdrCapabilities.HDR_TYPE_DOLBY_VISION) {
                                caps.isSupported = true;
                                Log.i(TAG, "Dolby Vision supported");
                            } else if (Build.VERSION.SDK_INT >= 34 && 
                                       type == 4) { // HDR_TYPE_HDR10_PLUS = 4, added in API 34
                                caps.isBT2020Supported = true;
                                caps.isPQSupported = true;
                                caps.isSupported = true;
                                Log.i(TAG, "HDR10+ supported");
                            }
                        }
                        
                        // Get luminance values from HDR capabilities
                        float maxLum = hdrCaps.getDesiredMaxLuminance();
                        float maxAvgLum = hdrCaps.getDesiredMaxAverageLuminance();
                        float minLum = hdrCaps.getDesiredMinLuminance();
                        
                        // Use the higher of max luminance values reported
                        // Some devices report peak in getDesiredMaxLuminance, others in average
                        float peakLum = Math.max(maxLum, maxAvgLum);
                        
                        if (peakLum > 0) {
                            caps.maxLuminanceNits = peakLum;
                        }
                        if (minLum > 0) {
                            caps.minLuminanceNits = minLum;
                        }
                        
                        Log.i(TAG, "Display luminance: peak=" + peakLum + 
                              " nits (max=" + maxLum + ", avgMax=" + maxAvgLum + 
                              ", min=" + minLum + ")");
                    }
                }
            }
        } catch (Exception e) {
            Log.e(TAG, "Error querying display HDR capabilities", e);
        }
        
        // Check EGL extensions for FP16 and colorspace support
        caps.isFP16Supported = checkEGLExtension("EGL_EXT_pixel_format_float");
        if (!caps.isBT2020Supported) {
            caps.isBT2020Supported = checkEGLExtension("EGL_EXT_gl_colorspace_bt2020_linear");
        }
        if (!caps.isPQSupported) {
            caps.isPQSupported = checkEGLExtension("EGL_EXT_gl_colorspace_bt2020_pq");
        }
        caps.isScRGBSupported = checkEGLExtension("EGL_EXT_gl_colorspace_scrgb_linear");
        
        // Update overall support flag
        if (caps.isFP16Supported && (caps.isPQSupported || caps.isScRGBSupported)) {
            caps.isSupported = true;
        }
        
        Log.i(TAG, "Final HDR capabilities: " + caps);
        return caps;
    }
    
    private static boolean checkEGLExtension(String extensionName) {
        try {
            EGL10 egl = (EGL10) EGLContext.getEGL();
            EGLDisplay display = egl.eglGetDisplay(EGL10.EGL_DEFAULT_DISPLAY);
            
            if (display == EGL10.EGL_NO_DISPLAY) {
                return false;
            }
            
            int[] version = new int[2];
            if (!egl.eglInitialize(display, version)) {
                return false;
            }
            
            String extensions = egl.eglQueryString(display, EGL10.EGL_EXTENSIONS);
            egl.eglTerminate(display);
            
            if (extensions != null && extensions.contains(extensionName)) {
                Log.d(TAG, "EGL extension supported: " + extensionName);
                return true;
            }
        } catch (Exception e) {
            Log.e(TAG, "Error checking EGL extension: " + extensionName, e);
        }
        return false;
    }
    
    public static boolean isWideColorGamutSupported(Activity activity) {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
            return activity.getWindow().isWideColorGamut();
        }
        return false;
    }
}
