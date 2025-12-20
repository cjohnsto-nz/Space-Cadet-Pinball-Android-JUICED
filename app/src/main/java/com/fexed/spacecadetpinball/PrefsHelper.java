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
        return prefs.getBoolean(KEY_CUSTOMFONTS, false);
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
        return prefs.getBoolean(KEY_MUSIC, false);
    }

    public static boolean getHDREnabled() {
        return prefs.getBoolean(KEY_HDR_ENABLED, true);
    }

    public static void setHDREnabled(boolean value) {
        prefs.edit().putBoolean(KEY_HDR_ENABLED, value).apply();
    }

    public static int getHDRMaxNits() {
        return prefs.getInt(KEY_HDR_MAX_NITS, 0); // 0 = use auto-detected value
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
}
