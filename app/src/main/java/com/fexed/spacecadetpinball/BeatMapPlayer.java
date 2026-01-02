package com.juiced.spacecadetpinball;

import android.content.Context;
import android.media.MediaPlayer;
import android.os.Handler;
import android.os.Looper;
import android.util.Log;

import org.json.JSONArray;
import org.json.JSONObject;

import java.io.BufferedReader;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.util.ArrayList;
import java.util.List;

/**
 * Plays back a pre-analyzed beat map in sync with music playback.
 * Calls a callback with bass intensity values that can be used to modulate HDR glow.
 */
public class BeatMapPlayer {
    private static final String TAG = "BeatMapPlayer";
    
    public interface BassIntensityListener {
        void onBassIntensity(float intensity);
    }
    
    public interface PositionProvider {
        long getPositionMs();
    }
    
    private static class BeatPoint {
        int timestampMs;
        float intensity;
        
        BeatPoint(int t, float v) {
            this.timestampMs = t;
            this.intensity = v;
        }
    }
    
    private final List<BeatPoint> beats = new ArrayList<>();
    private int durationMs = 0;
    private int currentIndex = 0;
    
    private MediaPlayer mediaPlayer;
    private PositionProvider positionProvider;
    private BassIntensityListener listener;
    private Handler handler;
    private boolean isPlaying = false;
    
    private float baseGlowModifier = 1.0f;
    private float glowRange = 0.5f; // How much the glow can vary (0.5 = 50% variation)
    private boolean useSmoothing = false; // Disabled by default for MIDI data
    private float smoothedIntensity = 0f;
    private float smoothingFactor = 1.0f; // 1.0 = no smoothing (instant)
    
    public BeatMapPlayer() {
        handler = new Handler(Looper.getMainLooper());
    }
    
    /**
     * Load a beat map from assets.
     * @param context Android context
     * @param assetName Name of the JSON file in assets (e.g., "808generative_beats.json")
     * @return true if loaded successfully
     */
    public boolean loadFromAssets(Context context, String assetName) {
        try {
            InputStream is = context.getAssets().open(assetName);
            BufferedReader reader = new BufferedReader(new InputStreamReader(is));
            StringBuilder sb = new StringBuilder();
            String line;
            while ((line = reader.readLine()) != null) {
                sb.append(line);
            }
            reader.close();
            
            return parseJson(sb.toString());
        } catch (Exception e) {
            Log.e(TAG, "Failed to load beat map from assets: " + assetName, e);
            return false;
        }
    }
    
    /**
     * Load a beat map from a JSON string.
     */
    public boolean parseJson(String json) {
        try {
            JSONObject root = new JSONObject(json);
            durationMs = root.getInt("duration_ms");
            
            JSONArray beatsArray = root.getJSONArray("beats");
            beats.clear();
            
            for (int i = 0; i < beatsArray.length(); i++) {
                JSONObject beat = beatsArray.getJSONObject(i);
                int t = beat.getInt("t");
                float v = (float) beat.getDouble("v");
                beats.add(new BeatPoint(t, v));
            }
            
            Log.i(TAG, "Loaded beat map: " + beats.size() + " points, duration " + durationMs + "ms");
            return true;
        } catch (Exception e) {
            Log.e(TAG, "Failed to parse beat map JSON", e);
            return false;
        }
    }
    
    /**
     * Set the MediaPlayer to sync with.
     */
    public void setMediaPlayer(MediaPlayer player) {
        this.mediaPlayer = player;
    }
    
    /**
     * Set a custom position provider (e.g., for Oboe).
     */
    public void setPositionProvider(PositionProvider provider) {
        this.positionProvider = provider;
    }
    
    /**
     * Set the listener for bass intensity updates.
     */
    public void setListener(BassIntensityListener listener) {
        this.listener = listener;
    }
    
    /**
     * Set the base glow modifier (the value when bass is at 0).
     */
    public void setBaseGlowModifier(float base) {
        this.baseGlowModifier = base;
    }
    
    /**
     * Set how much the glow can vary based on bass (0.0 to 1.0).
     * For example, 0.5 means glow can increase by up to 50% of base.
     */
    public void setGlowRange(float range) {
        this.glowRange = range;
    }
    
    /**
     * Set smoothing factor (0.0 to 1.0). Lower = smoother, higher = more responsive.
     */
    public void setSmoothingFactor(float factor) {
        this.smoothingFactor = Math.max(0.05f, Math.min(1.0f, factor));
    }
    
    /**
     * Start playing back the beat map in sync with the media player.
     */
    public void start() {
        if (beats.isEmpty() || (mediaPlayer == null && positionProvider == null)) {
            Log.w(TAG, "Cannot start: no beats loaded or no position source set");
            return;
        }
        
        isPlaying = true;
        currentIndex = 0;
        smoothedIntensity = 0f;
        handler.post(updateRunnable);
        Log.i(TAG, "Beat map playback started");
    }
    
    /**
     * Stop playing back the beat map.
     */
    public void stop() {
        isPlaying = false;
        handler.removeCallbacks(updateRunnable);
        Log.i(TAG, "Beat map playback stopped");
    }
    
    /**
     * Pause playback (keeps current position).
     */
    public void pause() {
        isPlaying = false;
        handler.removeCallbacks(updateRunnable);
    }
    
    /**
     * Resume playback.
     */
    public void resume() {
        if (!beats.isEmpty() && (mediaPlayer != null || positionProvider != null)) {
            isPlaying = true;
            handler.post(updateRunnable);
        }
    }
    
    private final Runnable updateRunnable = new Runnable() {
        @Override
        public void run() {
            if (!isPlaying || beats.isEmpty()) {
                return;
            }
            
            try {
                // Get position from provider or MediaPlayer
                int currentPositionMs;
                if (positionProvider != null) {
                    currentPositionMs = (int) positionProvider.getPositionMs();
                } else if (mediaPlayer != null) {
                    currentPositionMs = mediaPlayer.getCurrentPosition();
                } else {
                    return;
                }
                
                // Find the beat point closest to current position
                // Binary search for efficiency
                float targetIntensity = getIntensityAtTime(currentPositionMs);
                
                // Use exact intensity (no smoothing) for precise MIDI data
                float finalIntensity;
                if (useSmoothing) {
                    smoothedIntensity = smoothedIntensity + (targetIntensity - smoothedIntensity) * smoothingFactor;
                    finalIntensity = smoothedIntensity;
                } else {
                    finalIntensity = targetIntensity; // Instant, no smoothing
                }
                
                // Calculate final glow modifier
                float glowModifier = baseGlowModifier + (finalIntensity * glowRange * baseGlowModifier);
                
                // Notify listener
                if (listener != null) {
                    listener.onBassIntensity(glowModifier);
                }
                
            } catch (IllegalStateException e) {
                // MediaPlayer might be in invalid state
                Log.w(TAG, "MediaPlayer state error", e);
            }
            
            // Schedule next update (~60fps for smooth animation)
            if (isPlaying) {
                handler.postDelayed(this, 16);
            }
        }
    };
    
    /**
     * Get interpolated intensity at a specific timestamp.
     */
    private float getIntensityAtTime(int timeMs) {
        if (beats.isEmpty()) return 0f;
        
        // Handle edge cases
        if (timeMs <= beats.get(0).timestampMs) {
            return beats.get(0).intensity;
        }
        if (timeMs >= beats.get(beats.size() - 1).timestampMs) {
            return beats.get(beats.size() - 1).intensity;
        }
        
        // Binary search for the right interval
        int low = 0;
        int high = beats.size() - 1;
        
        while (low < high - 1) {
            int mid = (low + high) / 2;
            if (beats.get(mid).timestampMs <= timeMs) {
                low = mid;
            } else {
                high = mid;
            }
        }
        
        // Interpolate between low and high
        BeatPoint p1 = beats.get(low);
        BeatPoint p2 = beats.get(high);
        
        float t = (float)(timeMs - p1.timestampMs) / (p2.timestampMs - p1.timestampMs);
        return p1.intensity + (p2.intensity - p1.intensity) * t;
    }
    
    /**
     * Check if a beat map is loaded.
     */
    public boolean isLoaded() {
        return !beats.isEmpty();
    }
    
    /**
     * Get the duration of the loaded beat map in milliseconds.
     */
    public int getDurationMs() {
        return durationMs;
    }
}
