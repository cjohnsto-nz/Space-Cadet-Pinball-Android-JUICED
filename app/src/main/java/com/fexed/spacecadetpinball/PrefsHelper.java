package com.fexed.spacecadetpinball;

import android.content.SharedPreferences;

public class PrefsHelper {
    static private SharedPreferences prefs;

    private static final String KEY_USERID = "userid";
    private static final String KEY_USERNAME = "username";
    private static final String KEY_HIGHSCORE = "highscore";
    private static final String KEY_CHEATHIGHSCORE = "cheathighscore";
    private static final String KEY_CHEATSUSED = "cheatsused";
    private static final String KEY_VOLUME = "volume";
    private static final String KEY_MUSIC = "music";
    private static final String KEY_FULLSCREENPLUNGER = "fullscreenplunger";
    private static final String KEY_PLUNGERPOPUP = "plungerPopup";
    private static final String KEY_REMAININGBALLS = "remainingballs";
    private static final String KEY_SHOULDSHOWBOTTOMPLUNGER = "shouldshowbottomplunger";
    private static final String KEY_TILTBUTTONS = "tiltbuttons";
    private static final String KEY_CUSTOMFONTS = "customfonts";
    private static final String KEY_HDR_ENABLED = "hdr_enabled";
    private static final String KEY_HDR_MAX_NITS = "hdr_max_nits";
    private static final String KEY_LIGHT_EDIT_MODE = "light_edit_mode";
    private static final String KEY_HDR_GLOW_INTENSITY = "hdr_glow_intensity";
    private static final String KEY_TRAIL_OPACITY = "trail_opacity";
    private static final String KEY_TRAIL_LIFETIME = "trail_lifetime";
    private static final String KEY_CAMERA_TRACKING = "camera_tracking";
    private static final String KEY_CAMERA_ZOOM = "camera_zoom";
    private static final String KEY_ENHANCED_AUDIO = "enhanced_audio";
    private static final String KEY_PARTICLES_ENABLED = "particles_enabled";
    private static final String KEY_BEAT_REACTIVE_GLOW = "beat_reactive_glow";
    private static final String KEY_MUSIC_VOLUME = "music_volume";

    public static SharedPreferences getPrefs() {
        return prefs;
    }

    public static void setPrefs(SharedPreferences prefs) {
        PrefsHelper.prefs = prefs;
    }

    public static boolean getCheatsUsed() {
        return prefs.getBoolean(KEY_CHEATSUSED, false);
    }

    public static void setCheatsUsed(boolean value) {
        prefs.edit().putBoolean(KEY_CHEATSUSED, value).apply();
    }

    public static int getVolume() {
        return prefs.getInt(KEY_VOLUME, 100);
    }

    public static void setVolume(int value) {
        prefs.edit().putInt(KEY_VOLUME, value).apply();
    }

    public static String getUsername(String defaultvalue) {
        return prefs.getString(KEY_USERNAME, defaultvalue);
    }

    public static void setUsername(String value) {
        prefs.edit().putString(KEY_USERNAME, value).apply();
    }

    public static boolean getFullScreenPlunger() {
        return prefs.getBoolean(KEY_FULLSCREENPLUNGER, true);
    }

    public static void setFullScreenPlunger(boolean value) {
        prefs.edit().putBoolean(KEY_FULLSCREENPLUNGER, value).apply();
    }

    public static boolean getPlungerPopup() {
        return prefs.getBoolean(KEY_PLUNGERPOPUP, true);
    }

    public static void setPlungerPopup(boolean value) {
        prefs.edit().putBoolean(KEY_PLUNGERPOPUP, value).apply();
    }

    public static boolean getRemainingBalls() {
        return prefs.getBoolean(KEY_REMAININGBALLS, false);
    }

    public static void setRemainingBalls(boolean value) {
        prefs.edit().putBoolean(KEY_REMAININGBALLS, value).apply();
    }

    public static boolean getShouldShowBottomPlunger() {
        return prefs.getBoolean(KEY_SHOULDSHOWBOTTOMPLUNGER, false);
    }

    public static void setShouldShowBottomPlunger(boolean value) {
        prefs.edit().putBoolean(KEY_SHOULDSHOWBOTTOMPLUNGER, value).apply();
    }

    public static boolean getTiltButtons() {
        return prefs.getBoolean(KEY_TILTBUTTONS, false);
    }

    public static void setTiltButtons(boolean value) {
        prefs.edit().putBoolean(KEY_TILTBUTTONS, value).apply();
    }

    public static boolean getCustomFonts() {
        return prefs.getBoolean(KEY_CUSTOMFONTS, true);
    }

    public static void setCustomFonts(boolean value) {
        prefs.edit().putBoolean(KEY_CUSTOMFONTS, value).apply();
    }

    public static int getHighScore() {
        return prefs.getInt(KEY_HIGHSCORE, 0);
    }

    public static void setHighScore(int value) {
        prefs.edit().putInt(KEY_HIGHSCORE, value).apply();
    }

    public static String getUserId() {
        return prefs.getString(KEY_USERID, "0");
    }

    public static void setUserId(String value) {
        prefs.edit().putString(KEY_USERID, value).apply();
    }

    public static void setMusic(boolean value) {
        prefs.edit().putBoolean(KEY_MUSIC, value).apply();
    }

    public static boolean getMusic() {
        return prefs.getBoolean(KEY_MUSIC, true);
    }

    public static boolean getHDREnabled() {
        return prefs.getBoolean(KEY_HDR_ENABLED, true);
    }

    public static void setHDREnabled(boolean value) {
        prefs.edit().putBoolean(KEY_HDR_ENABLED, value).apply();
    }

    public static int getHDRMaxNits() {
        return prefs.getInt(KEY_HDR_MAX_NITS, 1750); // 1750 = user's preferred value
    }

    public static void setHDRMaxNits(int value) {
        prefs.edit().putInt(KEY_HDR_MAX_NITS, value).apply();
    }

    public static boolean getLightEditMode() {
        return prefs.getBoolean(KEY_LIGHT_EDIT_MODE, false);
    }

    public static void setLightEditMode(boolean value) {
        prefs.edit().putBoolean(KEY_LIGHT_EDIT_MODE, value).apply();
    }

    public static int getHDRGlowIntensity() {
        return prefs.getInt(KEY_HDR_GLOW_INTENSITY, 25); // 25 = user's preferred value
    }

    public static void setHDRGlowIntensity(int value) {
        prefs.edit().putInt(KEY_HDR_GLOW_INTENSITY, value).apply();
    }

    // Trail opacity (0-100, default 100)
    public static int getTrailOpacity() {
        return prefs.getInt(KEY_TRAIL_OPACITY, 100);
    }

    public static void setTrailOpacity(int value) {
        prefs.edit().putInt(KEY_TRAIL_OPACITY, value).apply();
    }

    // Trail lifetime in tenths of seconds (5-100, default 30 = 3.0 seconds)
    public static int getTrailLifetime() {
        return prefs.getInt(KEY_TRAIL_LIFETIME, 30);
    }

    public static void setTrailLifetime(int value) {
        prefs.edit().putInt(KEY_TRAIL_LIFETIME, value).apply();
    }

    // Camera tracking mode (ball-following zoom)
    public static boolean getCameraTracking() {
        return prefs.getBoolean(KEY_CAMERA_TRACKING, true);
    }

    public static void setCameraTracking(boolean value) {
        prefs.edit().putBoolean(KEY_CAMERA_TRACKING, value).apply();
    }

    // Camera zoom level (100-400, default 139 = 1.39x zoom)
    public static int getCameraZoom() {
        return prefs.getInt(KEY_CAMERA_ZOOM, 120);
    }

    public static void setCameraZoom(int value) {
        prefs.edit().putInt(KEY_CAMERA_ZOOM, value).apply();
    }

    // Enhanced audio
    public static boolean getEnhancedAudio() {
        return prefs.getBoolean(KEY_ENHANCED_AUDIO, true);
    }

    public static void setEnhancedAudio(boolean value) {
        prefs.edit().putBoolean(KEY_ENHANCED_AUDIO, value).apply();
    }

    // Particles enabled (bumper collision particles)
    public static boolean getParticlesEnabled() {
        return prefs.getBoolean(KEY_PARTICLES_ENABLED, false);
    }

    public static void setParticlesEnabled(boolean value) {
        prefs.edit().putBoolean(KEY_PARTICLES_ENABLED, value).apply();
    }

    // Beat-reactive glow (HDR glow pulses with music bass)
    public static boolean getBeatReactiveGlow() {
        return prefs.getBoolean(KEY_BEAT_REACTIVE_GLOW, true);
    }

    public static void setBeatReactiveGlow(boolean value) {
        prefs.edit().putBoolean(KEY_BEAT_REACTIVE_GLOW, value).apply();
    }

    // Music volume (0-100, default 62)
    public static int getMusicVolume() {
        return prefs.getInt(KEY_MUSIC_VOLUME, 60);
    }

    public static void setMusicVolume(int value) {
        prefs.edit().putInt(KEY_MUSIC_VOLUME, value).apply();
    }
}
