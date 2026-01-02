package com.fexed.spacecadetpinball;

import static android.os.VibrationEffect.EFFECT_CLICK;
import static android.os.VibrationEffect.EFFECT_DOUBLE_CLICK;
import static android.os.VibrationEffect.EFFECT_HEAVY_CLICK;
import static android.os.VibrationEffect.EFFECT_TICK;

import android.annotation.SuppressLint;
import android.content.Context;
import android.content.Intent;
import android.content.res.AssetFileDescriptor;
import android.content.res.AssetManager;
import android.content.res.Configuration;
import android.graphics.Color;
import android.graphics.Typeface;
import android.hardware.Sensor;
import android.hardware.SensorEvent;
import android.hardware.SensorEventListener;
import android.hardware.SensorManager;
import android.media.MediaPlayer;
import android.os.Build;
import android.os.Bundle;
import android.os.Handler;
import android.os.Looper;
import android.os.Vibrator;
import android.os.VibrationEffect;
import android.util.Log;
import android.util.TypedValue;
import android.view.KeyEvent;
import android.view.MotionEvent;
import android.view.View;
import android.view.ViewGroup;
import android.widget.RelativeLayout;
import android.widget.TextView;
import android.widget.Toast;

import androidx.constraintlayout.widget.ConstraintLayout;
import androidx.core.content.res.ResourcesCompat;

import org.libsdl.app.SDLActivity;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;

import com.fexed.spacecadetpinball.databinding.ActivityMainBinding;
import com.google.android.material.bottomsheet.BottomSheetBehavior;
import com.google.firebase.analytics.FirebaseAnalytics;

public class MainActivity extends SDLActivity {
    private static final String TAG = "MainActivity";

    private ActivityMainBinding mBinding;
    private Handler plungerTimer;
    private boolean isGameReady = false;
    private boolean isPlaying = true;

    private int ballCount = 0;
    private int remainingBalls = 0;
    private BottomSheetBehavior<ConstraintLayout> bottomSheetBehavior;

    private FirebaseAnalytics firebaseAnalytics;
    private int gamesInSession = 0;

    static private MediaPlayer player = new MediaPlayer();
    private BeatMapPlayer beatMapPlayer;

    private SensorManager sensorManager;
    private Sensor accelerometer;

    // Light editor state
    private boolean lightEditModeEnabled = false;
    private int[] lastViewport = new int[4];  // x, y, w, h

    // Timer mode state
    private boolean timerModeActive = false;
    private Handler timerUpdateHandler;
    private Runnable timerUpdateRunnable;
    private boolean waitingForModeSelection = false;
    private boolean pendingGameStart = false;  // Wait for ball to enter plunger before starting music/timer

    @SuppressLint("ClickableViewAccessibility")
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        File filesDir = getFilesDir();
        copyAssets(filesDir);
        PrefsHelper.setPrefs(getSharedPreferences("com.fexed.spacecadetpinball", Context.MODE_PRIVATE));

        // Initialize HDR capabilities BEFORE initNative so SDL can use HDR colorspace
        initializeHDR();
        
        // Pass enhanced audio setting to initNative (sounds load during init)
        initNative(filesDir.getAbsolutePath() + "/", PrefsHelper.getEnhancedAudio());

        sensorManager = (SensorManager) getSystemService(Context.SENSOR_SERVICE);
        accelerometer = sensorManager.getDefaultSensor(Sensor.TYPE_ACCELEROMETER);

        // Initialize Oboe music player for real-time audio effects
        initOboeMusicPlayer();
        
        // Show loading dialog for audio decoding
        android.app.AlertDialog loadingDialog = new android.app.AlertDialog.Builder(this)
                .setTitle("Loading Audio")
                .setMessage("Decoding compressed audio files...")
                .setCancelable(false)
                .create();
        loadingDialog.show();
        
        // Load audio in background thread so dialog can display
        new Thread(() -> {
            // Get cache directory for temporary files
            String cacheDir = getCacheDir().getAbsolutePath();
            
            // Load WAV with caching to avoid unpacking from APK every launch
            boolean musicLoaded = false;
            if (loadMusicFromAssetsWithCache(getAssets(), "808generative.wav", cacheDir)) {
                Log.i(TAG, "Loaded main music as WAV");
                musicLoaded = true;
            }
            
            if (musicLoaded) {
                setMusicVolume(PrefsHelper.getMusicVolume() / 100.0f);
                
                // Update loading message for mission track
                runOnUiThread(() -> {
                    try {
                        loadingDialog.setMessage("Decoding mission audio...");
                    } catch (Exception e) {
                        // Dialog might be dismissed, ignore
                    }
                });
                
                // Load mission track WAV with caching
                boolean missionLoaded = false;
                if (loadMissionMusicFromAssetsWithCache(getAssets(), "808generativemission.wav", cacheDir)) {
                    Log.i(TAG, "Loaded mission music as WAV");
                    missionLoaded = true;
                }
                
                if (missionLoaded) {
                    // setMissionMusicVolume(0.7f);  // Reduce max volume to 70%
                    setMissionMusicVolume(0.7f);
                    Log.i(TAG, "Mission music track loaded");
                } else {
                    Log.w(TAG, "Failed to load mission music track");
                }
                
                // Music will start after mode selection dialog
                // Don't auto-start here - wait for player to select game mode
                Log.i(TAG, "Oboe music player initialized with WAV file (waiting for mode selection)");
                
                // Initialize beat map player for bass-reactive HDR glow
                beatMapPlayer = new BeatMapPlayer();
                if (beatMapPlayer.loadFromAssets(this, "808generative_beats.json")) {
                    // Use Oboe position instead of MediaPlayer
                    beatMapPlayer.setPositionProvider(() -> getMusicPositionMs());
                    beatMapPlayer.setBaseGlowModifier(PrefsHelper.getHDRGlowIntensity() / 100.0f);
                    beatMapPlayer.setGlowRange(1.0f); // Glow can double on bass hits
                    // No smoothing - use exact MIDI timing for instant attack
                    beatMapPlayer.setListener(glowModifier -> {
                        setHDRGlowModifier(glowModifier);
                    });
                    // Beat map will start after mode selection along with music
                    Log.i(TAG, "Beat map loaded for bass-reactive glow (waiting for mode selection)");
                } else {
                    Log.w(TAG, "No beat map found, HDR glow will not react to music");
                }
            } else {
                Log.e(TAG, "Failed to load music from assets");
            }
            
            // Dismiss loading dialog
            runOnUiThread(() -> {
                try {
                    loadingDialog.dismiss();
                } catch (Exception e) {
                    // Dialog might already be dismissed, ignore
                }
            });
        }).start();
        
        mBinding = ActivityMainBinding.inflate(getLayoutInflater(), mLayout, false);

        RelativeLayout.LayoutParams layoutParams = new RelativeLayout.LayoutParams(ViewGroup.LayoutParams.MATCH_PARENT, ViewGroup.LayoutParams.MATCH_PARENT);
        layoutParams.addRule(RelativeLayout.CENTER_IN_PARENT, RelativeLayout.TRUE);
        mLayout.addView(mBinding.getRoot(), layoutParams);

        mBinding.getRoot().bringToFront();

        new Handler().postDelayed(new Runnable() {
            @Override
            public void run() {
                setFullscreen();
            }
        }, 100);  // delay by 100ms

        bottomSheetBehavior = BottomSheetBehavior.from(mBinding.bottomsheet);
        bottomSheetBehavior.setState(BottomSheetBehavior.STATE_HIDDEN);

        mBinding.left.setOnTouchListener((v1, event) -> {
            v1.performClick();
            if (event.getAction() == MotionEvent.ACTION_DOWN) {
                SDLActivity.onNativeKeyDown(KeyEvent.KEYCODE_Z);
                exeClick();
            }
            if (event.getAction() == MotionEvent.ACTION_UP) {
                SDLActivity.onNativeKeyUp(KeyEvent.KEYCODE_Z);
                exeHaptic();
            }
            return false;
        });


        mBinding.right.setOnTouchListener((v1, event) -> {
            v1.performClick();
            if (event.getAction() == MotionEvent.ACTION_DOWN) {
                SDLActivity.onNativeKeyDown(KeyEvent.KEYCODE_SLASH);
                exeClick();
            }
            if (event.getAction() == MotionEvent.ACTION_UP) {
                SDLActivity.onNativeKeyUp(KeyEvent.KEYCODE_SLASH);
                exeHaptic();
            }
            return false;
        });

        // The Vibrator instances
        Vibrator vibrator2 = (Vibrator) getSystemService(Context.VIBRATOR_SERVICE);
        plungerVibrator = (Vibrator) getSystemService(Context.VIBRATOR_SERVICE);

        mBinding.plunger.setOnTouchListener((v1, event) -> {
            if (v1 == null || event == null) return false;
            v1.performClick();
            
            switch (event.getAction()) {
                case MotionEvent.ACTION_DOWN:
                    // Start drag-based plunger control
                    isDraggingPlunger = true;
                    plungerStartY = event.getY();
                    plungerCurrentY = plungerStartY;
                    lastHapticPosition = -1f; // Reset haptic position for new drag
                    Log.d(TAG, "Plunger ACTION_DOWN: startY=" + plungerStartY);
                    SDLActivity.onNativeKeyDown(KeyEvent.KEYCODE_SPACE);
                    // Removed exeClick() to eliminate unwanted vibration
                    return true;
                    
                case MotionEvent.ACTION_MOVE:
                    if (isDraggingPlunger) {
                        plungerCurrentY = event.getY();
                        float dragDistance = plungerCurrentY - plungerStartY;
                        
                        // Calculate drag percentage (0 to 1, where 1 is max drag)
                        float dragPercentage = Math.max(0, Math.min(1, dragDistance / plungerMaxDragDistance));
                        
                        // Plunger move logging disabled
                        // Log.d(TAG, "Plunger ACTION_MOVE: currentY=" + plungerCurrentY + 
                        //       ", dragDistance=" + dragDistance + 
                        //       ", dragPercentage=" + dragPercentage);
                        
                        // Update plunger position visually (optional - could add visual feedback)
                        // For now, we'll just provide proportional haptic feedback
                        
                        // Provide haptic feedback at 5% power level steps
                        if (plungerVibrator != null && plungerVibrator.hasVibrator()) {
                            // Calculate which 5% step we're at (0-20 steps)
                            int currentStep = (int)(dragPercentage / 0.05f);
                            int lastStep = (int)(lastHapticPosition / 0.05f);
                            
                            // Only trigger haptic when crossing a 5% threshold
                            if (currentStep != lastStep && dragPercentage > 0.01f) {
                                // Use proper haptic effects based on power level
                                VibrationEffect effect;
                                if (dragPercentage < 0.33f) {
                                    // Light haptic for low power (0-33%)
                                    effect = VibrationEffect.createPredefined(EFFECT_TICK);
                                } else if (dragPercentage < 0.67f) {
                                    // Medium haptic for moderate power (33-67%)
                                    effect = VibrationEffect.createPredefined(EFFECT_CLICK);
                                } else {
                                    // Strong haptic for high power (67-100%)
                                    effect = VibrationEffect.createPredefined(EFFECT_HEAVY_CLICK);
                                }
                                plungerVibrator.vibrate(effect);
                                lastHapticPosition = dragPercentage;
                            }
                        }
                        
                        // Plunger update logging disabled
                        // Log.d(TAG, "Calling updatePlungerPosition(" + dragPercentage + ")");
                        updatePlungerPosition(dragPercentage);
                        
                        return true;
                    }
                    break;
                    
                case MotionEvent.ACTION_UP:
                case MotionEvent.ACTION_CANCEL:
                    if (isDraggingPlunger) {
                        isDraggingPlunger = false;
                        
                        float finalDragDistance = plungerCurrentY - plungerStartY;
                        float dragPercentage = Math.max(0, Math.min(1, finalDragDistance / plungerMaxDragDistance));
                        
                        // Plunger ACTION_UP logging disabled
                        // Log.d(TAG, "Plunger ACTION_UP: finalDragDistance=" + finalDragDistance + 
                        //       ", dragPercentage=" + dragPercentage);
                        
                        // Launch with force proportional to drag distance
                        if (dragPercentage > 0.1f) { // Minimum 10% drag to launch
                            // Provide final launch haptic feedback using proper haptic effects
                            if (plungerVibrator != null && plungerVibrator.hasVibrator()) {
                                VibrationEffect effect;
                                if (dragPercentage < 0.5f) {
                                    // Light launch
                                    effect = VibrationEffect.createPredefined(EFFECT_CLICK);
                                } else {
                                    // Strong launch (50-100%)
                                    effect = VibrationEffect.createPredefined(EFFECT_HEAVY_CLICK);
                                }
                                plungerVibrator.vibrate(effect);
                            }
                            
                            // Plunger launch logging disabled
                            // Log.d(TAG, "Calling setPlungerLaunchPower(" + dragPercentage + ")");
                            setPlungerLaunchPower(dragPercentage);
                        } else {
                            // Plunger drag too small logging disabled
                            // Log.d(TAG, "Drag too small, not launching (dragPercentage=" + dragPercentage + ")");
                        }
                        
                        SDLActivity.onNativeKeyUp(KeyEvent.KEYCODE_SPACE);
                        exeClickH();
                        
                        // Stop any ongoing vibration
                        if (plungerVibrator != null) {
                            plungerVibrator.cancel();
                        }
                        
                        return true;
                    }
                    break;
            }
            return false;
        });

        mBinding.bottomPlunger.setOnTouchListener((v1, event) -> {
            v1.performClick();
            if (event.getAction() == MotionEvent.ACTION_DOWN) {
                SDLActivity.onNativeKeyDown(KeyEvent.KEYCODE_SPACE);
                exeHaptic();
            }
            if (event.getAction() == MotionEvent.ACTION_UP) {
                SDLActivity.onNativeKeyUp(KeyEvent.KEYCODE_SPACE);
                exeHaptic();
            }
            return false;
        });

        mBinding.replay.setOnLongClickListener(view -> {
            exeHaptic();
            
            // Stop all music before restart
            stopMusic();
            if (beatMapPlayer != null) beatMapPlayer.stop();
            
            // Clean up timer mode if active
            if (timerModeActive) {
                onTimerModeGameOver();
            }
            resetTimerMode();
            
            // Trigger native restart
            SDLActivity.onNativeKeyDown(KeyEvent.KEYCODE_F2);
            SDLActivity.onNativeKeyUp(KeyEvent.KEYCODE_F2);
            PrefsHelper.setCheatsUsed(false);
            mBinding.cheatAlert.setVisibility(View.INVISIBLE);
            return true;
        });

        mBinding.playpause.setOnClickListener(view -> {
            exeHaptic();
            if (isPlaying) {
                isPlaying = false;
                pauseNativeThread();
                pauseMusic(); // Oboe
                if (beatMapPlayer != null) beatMapPlayer.pause();
                if (timerModeActive) pauseTimerMode();
                pauseSessionTimer(); // Pause session time tracking
                mBinding.playpause.setImageDrawable(getContext().getResources().getDrawable(R.drawable.play));
            } else {
                isPlaying = true;
                resumeNativeThread();
                resumeMusic(); // Oboe
                if (beatMapPlayer != null) beatMapPlayer.resume();
                if (timerModeActive) resumeTimerMode();
                resumeSessionTimer(); // Resume session time tracking
                mBinding.playpause.setImageDrawable(getContext().getResources().getDrawable(R.drawable.pause));
            }
        });

        mBinding.tiltLeft.setOnTouchListener((v1, event) -> {
            v1.performClick();
            if (event.getAction() == MotionEvent.ACTION_DOWN) {
                SDLActivity.onNativeKeyDown(KeyEvent.KEYCODE_X);
            }
            if (event.getAction() == MotionEvent.ACTION_UP) {
                SDLActivity.onNativeKeyUp(KeyEvent.KEYCODE_X);
            }
            return false;
        });

        mBinding.tiltRight.setOnTouchListener((v1, event) -> {
            v1.performClick();
            if (event.getAction() == MotionEvent.ACTION_DOWN) {
                SDLActivity.onNativeKeyDown(KeyEvent.KEYCODE_PERIOD);
            }
            if (event.getAction() == MotionEvent.ACTION_UP) {
                SDLActivity.onNativeKeyUp(KeyEvent.KEYCODE_PERIOD);
            }
            return false;
        });

        mBinding.tiltBottom.setOnTouchListener((v1, event) -> {
            v1.performClick();
            if (event.getAction() == MotionEvent.ACTION_DOWN) {
                SDLActivity.onNativeKeyDown(KeyEvent.KEYCODE_DPAD_UP);
            }
            if (event.getAction() == MotionEvent.ACTION_UP) {
                SDLActivity.onNativeKeyUp(KeyEvent.KEYCODE_DPAD_UP);
            }
            return false;
        });

        mBinding.settingsbtn.setOnClickListener(view -> {
            exeHaptic();
            Intent i = new Intent(this, Settings.class);
            startActivity(i);
        });
        
        mBinding.settingsbtn.setOnLongClickListener(view -> {
            exeHaptic();
            if (isLightDebugPanelVisible()) {
                hideLightDebugPanel();
            } else {
                showLightDebugPanel();
            }
            return true;
        });

        setupLightDebugPanel();

        firebaseAnalytics = FirebaseAnalytics.getInstance(this);
        firebaseAnalytics.logEvent(FirebaseAnalytics.Event.APP_OPEN, null);
    }

    private void copyAssets(File filesDir) {
        if (!new File(filesDir, "PINBALL.DAT").exists()) {
            AssetManager assetManager = getAssets();
            copyAssetFolder(assetManager, "", filesDir);
        }
        // Always check and copy enhanced audio folder (may be added after initial install)
        File enhancedDir = new File(filesDir, "enhanced");
        if (!enhancedDir.exists()) {
            AssetManager assetManager = getAssets();
            copyAssetFolder(assetManager, "enhanced", enhancedDir);
        }
        // Always check and copy music file (may be added after initial install)
        File musicFile = new File(filesDir, "808generative.mp3");
        if (!musicFile.exists()) {
            AssetManager assetManager = getAssets();
            copyAssetFile(assetManager, "808generative.mp3", musicFile);
        }
    }
    
    private void copyAssetFolder(AssetManager assetManager, String assetPath, File targetDir) {
        try {
            String[] assets = assetManager.list(assetPath);
            if (assets == null || assets.length == 0) {
                // It's a file, copy it
                copyAssetFile(assetManager, assetPath, new File(targetDir.getParent(), new File(assetPath).getName()));
            } else {
                // It's a directory
                if (!targetDir.exists()) {
                    targetDir.mkdirs();
                }
                for (String asset : assets) {
                    String fullAssetPath = assetPath.isEmpty() ? asset : assetPath + "/" + asset;
                    String[] subAssets = assetManager.list(fullAssetPath);
                    if (subAssets != null && subAssets.length > 0) {
                        // It's a subdirectory, recurse
                        copyAssetFolder(assetManager, fullAssetPath, new File(targetDir, asset));
                    } else {
                        // It's a file
                        copyAssetFile(assetManager, fullAssetPath, new File(targetDir, asset));
                    }
                }
            }
        } catch (IOException e) {
            e.printStackTrace();
        }
    }
    
    private void copyAssetFile(AssetManager assetManager, String assetPath, File targetFile) {
        try (InputStream is = assetManager.open(assetPath);
             OutputStream os = new FileOutputStream(targetFile)) {
            Log.d(TAG, "Copying " + assetPath + " to " + targetFile.getAbsolutePath());
            byte[] buffer = new byte[4096];
            int len;
            while ((len = is.read(buffer)) != -1) {
                os.write(buffer, 0, len);
            }
        } catch (IOException e) {
            // Silently ignore - might be a directory
        }
    }

    private void initializeHDR() {
        HDRHelper.HDRCapabilities caps = HDRHelper.queryHDRCapabilities(this);
        Log.i(TAG, "HDR Capabilities: " + caps);
        
        // Check if user has set a custom max nits value
        int userMaxNits = PrefsHelper.getHDRMaxNits();
        float maxNits = caps.maxLuminanceNits;
        if (userMaxNits > 0) {
            maxNits = (float) userMaxNits;
            Log.i(TAG, "Using user-configured max nits: " + userMaxNits);
        }
        
        // Check if user has HDR enabled in preferences
        boolean hdrEnabled = PrefsHelper.getHDREnabled();
        
        // Pass HDR enabled state to native code first
        setHDREnabled(hdrEnabled);
        
        // Pass HDR capabilities to native code
        setHDRCapabilities(
            caps.isSupported,
            caps.isBT2020Supported,
            caps.isPQSupported,
            caps.isScRGBSupported,
            caps.isFP16Supported,
            maxNits,
            caps.minLuminanceNits
        );
        
        if (caps.isSupported && hdrEnabled) {
            Log.i(TAG, "HDR is supported and enabled! Max luminance: " + maxNits + " nits");
        } else if (caps.isSupported) {
            Log.i(TAG, "HDR is supported but disabled by user");
        } else {
            Log.i(TAG, "HDR is not supported on this device, using SDR rendering");
        }
        
        // Apply saved glow intensity modifier
        int glowPercent = PrefsHelper.getHDRGlowIntensity();
        float glowModifier = glowPercent / 100.0f;
        setHDRGlowModifier(glowModifier);
        Log.i(TAG, "HDR glow modifier set to " + glowPercent + "% (" + glowModifier + ")");
        
        // Apply saved trail settings
        int trailOpacityPercent = PrefsHelper.getTrailOpacity();
        float trailOpacity = trailOpacityPercent / 100.0f;
        setTrailOpacity(trailOpacity);
        
        int trailLifetimeTenths = PrefsHelper.getTrailLifetime();
        float trailLifetime = trailLifetimeTenths / 10.0f;
        setTrailLifetime(trailLifetime);
        Log.i(TAG, "Trail settings: opacity=" + trailOpacityPercent + "%, lifetime=" + trailLifetime + "s");
        
        // Apply saved camera tracking settings
        boolean cameraTrackingEnabled = PrefsHelper.getCameraTracking();
        int cameraZoomPercent = PrefsHelper.getCameraZoom();
        float cameraZoom = cameraZoomPercent / 100.0f;
        setCameraTracking(cameraTrackingEnabled, cameraZoom);
        Log.i(TAG, "Camera tracking: enabled=" + cameraTrackingEnabled + ", zoom=" + cameraZoom + "x");
        
        // Apply saved particles enabled setting
        boolean particlesEnabled = PrefsHelper.getParticlesEnabled();
        setParticlesEnabled(particlesEnabled);
        Log.i(TAG, "Particles enabled: " + particlesEnabled);
        
        // Apply saved music enabled setting
        boolean musicEnabled = PrefsHelper.getMusic();
        setMusicEnabled(musicEnabled);
        Log.i(TAG, "Music enabled: " + musicEnabled);
    }

    private final SensorEventListener accelerometerListener = new SensorEventListener() {
//        @Override
//        public void onSensorChanged(SensorEvent event) {
//            float ax = event.values[0];
//            float ay = event.values[1];
//            float az = event.values[2];
//
//            // Calculate the magnitude of acceleration
//            double magnitude = Math.sqrt(ax * ax + ay * ay + az * az);
//            // Check if the magnitude exceeds a certain threshold, indicating a "jolt"
//            if (magnitude > (2.5f * 9.8f)) {
//                onJoltDetected();
//            }
//        }

        @Override
        public void onSensorChanged(SensorEvent event) {
            if (event.sensor.getType() == Sensor.TYPE_ACCELEROMETER) {
                float x = event.values[0];
                float y = event.values[1];
                float z = event.values[2];
                float JOLT_THRESHOLD = (2.5f * 9.8f);
                // Assuming JOLT_THRESHOLD is some value you've determined to be a "jolt"
                if (Math.abs(x) > JOLT_THRESHOLD) {
                    if (x > 0) {
                        // Jolt to the right
                        triggerRightEffect();
                    } else {
                        // Jolt to the left
                        triggerLeftEffect();
                    }
                } else if (Math.abs(y) > JOLT_THRESHOLD) {
                    if (y > 0) {
                        // Jolt upwards
                        //triggerUpEffect();
                    } else {
                        // Jolt downwards
//                        triggerDownEffect();
                    }
                } else if (Math.abs(z) > JOLT_THRESHOLD) {
                    if (z > 0) {
                        // Jolt with the screen facing up
                        triggerFaceUpEffect();
                    } else {
                        // Jolt with the screen facing down
//                        triggerFaceDownEffect();
                    }
                }
            }
        }


        @Override
        public void onAccuracyChanged(Sensor sensor, int accuracy) {
            // Handle changes in sensor accuracy if necessary
        }
    };

    private void triggerBottomTiltUp() {
        SDLActivity.onNativeKeyUp(KeyEvent.KEYCODE_DPAD_UP);
    }

    private void triggerFaceUpEffect() {
        SDLActivity.onNativeKeyDown(KeyEvent.KEYCODE_DPAD_UP);
        new Handler().postDelayed(this::triggerBottomTiltUp, 100);  // Delay of 100ms
        exeClickD();
    }

    private void triggerLeftTiltUp() {
        SDLActivity.onNativeKeyUp(KeyEvent.KEYCODE_X);
    }

    private void triggerLeftEffect() {
        SDLActivity.onNativeKeyDown(KeyEvent.KEYCODE_X);
        new Handler().postDelayed(this::triggerLeftTiltUp, 100);  // Delay of 100ms
        exeClickD();
    }

    private void triggerRightTiltUp() {
        SDLActivity.onNativeKeyUp(KeyEvent.KEYCODE_PERIOD);
    }

    private void triggerRightEffect() {
        SDLActivity.onNativeKeyDown(KeyEvent.KEYCODE_X);
        new Handler().postDelayed(this::triggerRightTiltUp, 100);  // Delay of 100ms
        exeClickD();
    }


    private void exeHaptic() {
        // Get the vibrator service
        Vibrator vibrator = (Vibrator) getSystemService(Context.VIBRATOR_SERVICE);
        if (Build.VERSION.SDK_INT >= 29) {
            // Create a VibrationEffect with your desired pattern and amplitude

            VibrationEffect vibrationEffect = VibrationEffect.createPredefined(EFFECT_TICK);

            // Vibrate with the given VibrationEffect
            vibrator.vibrate(vibrationEffect);
        } else {
            // For older devices, use a simple pattern
//            vibrator.vibrate(20);
        }
    }

    private void exeClick() {
        // Get the vibrator service
        Vibrator vibrator = (Vibrator) getSystemService(Context.VIBRATOR_SERVICE);
        if (Build.VERSION.SDK_INT >= 29) {
            // Create a VibrationEffect with your desired pattern and amplitude

            VibrationEffect vibrationEffect = VibrationEffect.createPredefined(EFFECT_CLICK);

            // Vibrate with the given VibrationEffect
            vibrator.vibrate(vibrationEffect);
        } else {
            // For older devices, use a simple pattern
//            vibrator.vibrate(20);
        }
    }

    private void exeClickH() {
        // Get the vibrator service
        Vibrator vibrator = (Vibrator) getSystemService(Context.VIBRATOR_SERVICE);
        if (Build.VERSION.SDK_INT >= 29) {
            // Create a VibrationEffect with your desired pattern and amplitude

            VibrationEffect vibrationEffect = VibrationEffect.createPredefined(EFFECT_HEAVY_CLICK);

            // Vibrate with the given VibrationEffect
            vibrator.vibrate(vibrationEffect);
        } else {
            // For older devices, use a simple pattern
//            vibrator.vibrate(20);
        }
    }


    private void exeClickD() {
        // Get the vibrator service
        Vibrator vibrator = (Vibrator) getSystemService(Context.VIBRATOR_SERVICE);
        if (Build.VERSION.SDK_INT >= 29) {
            // Create a VibrationEffect with your desired pattern and amplitude

            VibrationEffect vibrationEffect = VibrationEffect.createPredefined(EFFECT_DOUBLE_CLICK);

            // Vibrate with the given VibrationEffect
            vibrator.vibrate(vibrationEffect);
        } else {
            // For older devices, use a simple pattern
//            vibrator.vibrate(20);
        }
    }

    private void triggerCollisionHaptic(float intensity) {
        Vibrator vibrator = (Vibrator) getSystemService(Context.VIBRATOR_SERVICE);
        if (vibrator == null || !vibrator.hasVibrator()) return;

        if (Build.VERSION.SDK_INT >= 29) {
            // Use predefined haptic effects for better HD haptics feel
            VibrationEffect effect;
            if (intensity >= 0.7f) {
                // Heavy collision - use heavy click
                effect = VibrationEffect.createPredefined(EFFECT_HEAVY_CLICK);
            } else if (intensity >= 0.3f) {
                // Medium collision - use click
                effect = VibrationEffect.createPredefined(EFFECT_CLICK);
            } else {
                // Light collision - use tick
                effect = VibrationEffect.createPredefined(EFFECT_TICK);
            }
            vibrator.vibrate(effect);
        } else if (Build.VERSION.SDK_INT >= 26) {
            // Fallback for older devices
            float clampedIntensity = Math.max(0.0f, Math.min(1.0f, intensity));
            int amplitude = (int) (clampedIntensity * 255);
            amplitude = Math.max(1, amplitude);
            long duration = (long) (5 + clampedIntensity * 25);
            VibrationEffect effect = VibrationEffect.createOneShot(duration, amplitude);
            vibrator.vibrate(effect);
        }
    }

    private void setFullscreen() {
        int ui_Options = View.SYSTEM_UI_FLAG_LAYOUT_STABLE
                | View.SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION
                | View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN
                | View.SYSTEM_UI_FLAG_HIDE_NAVIGATION
                | View.SYSTEM_UI_FLAG_FULLSCREEN
                | View.SYSTEM_UI_FLAG_IMMERSIVE_STICKY;
        getWindow().getDecorView().setSystemUiVisibility(ui_Options);
    }

    private final StateHelper.IStateListener mStateListener = new StateHelper.IStateListener() {
        @Override
        public void onStateChanged(int state) {
            setVolume(PrefsHelper.getVolume());
            if (player != null) player.setVolume(PrefsHelper.getVolume()/(float) 100, PrefsHelper.getVolume()/(float) 100);
            putTranslations();
            putString(26, PrefsHelper.getUsername("Player 1"));  //TODO abstract ids

            if (state == GameState.RUNNING) {
                gamesInSession++;
                runOnUiThread(() -> {
                    firebaseAnalytics.logEvent(FirebaseAnalytics.Event.LEVEL_START, null);
                    // Show mode selection dialog on new game
                    showModeSelectionDialog();
                });
            }

            if (state == GameState.FINISHED) {
                // Stop session timer and capture final stats before UI updates
                stopSessionTimer();
                final int finalScore = getTotalScore();
                final long finalPlayTimeMs = getSessionTimeMs();
                final int finalRank = getPlayerRank();
                final int finalOuterProgress = getOuterCircleProgress();
                final int finalOuterTotal = getOuterCircleTotal();
                
                runOnUiThread(() -> {
                    firebaseAnalytics.logEvent(FirebaseAnalytics.Event.LEVEL_END, null);
                    // Clean up timer mode if active
                    if (timerModeActive) {
                        onTimerModeGameOver();
                    }
                    // Show game over summary dialog
                    showGameOverSummary(finalScore, finalPlayTimeMs, finalRank, 
                                       finalOuterProgress, finalOuterTotal);
                });
            }
        }

        @Override
        public void onBallInPlungerChanged(boolean isBallInPlunger) {
            // Start music and timer when ball first enters plunger after mode selection
            if (isBallInPlunger && pendingGameStart) {
                pendingGameStart = false;
                runOnUiThread(() -> {
                    // Show appropriate UI elements based on mode
                    mBinding.missiontxt.setVisibility(View.VISIBLE);
                    mBinding.infotxt.setVisibility(View.VISIBLE);
                    
                    if (timerModeActive) {
                        // Timer mode: show timer and score progress, hide ball count
                        mBinding.txtTimer.setVisibility(View.VISIBLE);
                        mBinding.txtscore.setVisibility(View.VISIBLE);
                        mBinding.ballstxt.setVisibility(View.GONE);
                        // Start timer
                        startTimerMode();
                    } else {
                        // Classic mode: show ball count and score
                        mBinding.ballstxt.setVisibility(View.VISIBLE);
                        mBinding.txtscore.setVisibility(View.VISIBLE);
                        mBinding.txtTimer.setVisibility(View.GONE);
                        mBinding.txtTimerBonus.setVisibility(View.GONE);
                    }
                    
                    // Start music
                    if (PrefsHelper.getMusic()) {
                        startMusic();
                        if (beatMapPlayer != null && PrefsHelper.getBeatReactiveGlow()) {
                            beatMapPlayer.start();
                        }
                    }
                });
            }
            
            if (PrefsHelper.getFullScreenPlunger()) {
                runOnUiThread(() -> mBinding.plunger.setVisibility(isBallInPlunger ? View.VISIBLE : View.INVISIBLE));
                if (isBallInPlunger) {
                    plungerTimer = new Handler(Looper.getMainLooper());
                    plungerTimer.postDelayed(() -> runOnUiThread(() -> {
                        if (PrefsHelper.getPlungerPopup()) {
//                            Toast.makeText(getContext(), R.string.plungerhint, Toast.LENGTH_LONG).show();
                        }
                        mBinding.plunger.setVisibility(View.VISIBLE);
                    }), 3000);
                } else {
                    if (plungerTimer != null) {
                        plungerTimer.removeCallbacksAndMessages(null);
                        plungerTimer = null;
                    }
                }
            } else {
                runOnUiThread(() -> bottomSheetBehavior.setState(BottomSheetBehavior.STATE_EXPANDED));
            }
        }

        @Override
        public void onHighScorePresented(int score) {
            if (HighScoreHandler.postHighScore(getContext(), score)) {
                runOnUiThread(() -> Toast.makeText(getContext(), getString(R.string.newhighscore, score), Toast.LENGTH_LONG).show());
                Bundle bndl = new Bundle();
                bndl.putInt(FirebaseAnalytics.Param.SCORE, score);
                runOnUiThread(() -> firebaseAnalytics.logEvent(FirebaseAnalytics.Event.POST_SCORE, bndl));
            }
        }

        @Override
        public int onHighScoreRequested() {
            return PrefsHelper.getHighScore();
        }

        @Override
        public void onStringPresented(String str, int type) {
            final String fstr = str.replace("\n", " ");
            if (type == 1) runOnUiThread(() -> setTextWithBackground(mBinding.missiontxt, fstr));
            else runOnUiThread(() -> setTextWithBackground(mBinding.infotxt, fstr));
        }

        @Override
        public void onClearText(int type) {
            if (type == 1) runOnUiThread(() -> setTextWithBackground(mBinding.missiontxt, ""));
            else runOnUiThread(() -> setTextWithBackground(mBinding.infotxt, ""));
        }

        @Override
        public void onScorePosted(int score) {
            String str = "SCORE: " + score;
            runOnUiThread(() -> setTextWithBackground(mBinding.txtscore, str));
        }

        @Override
        public void onBallCountUpdated(int count) {
            ballCount = count;
            if (!PrefsHelper.getRemainingBalls()) {
                String str = getString(R.string.balls, count);
                runOnUiThread(() -> setTextWithBackground(mBinding.ballstxt, str));
            }
        }

        @Override
        public void onCheatsUsed() {
            PrefsHelper.setCheatsUsed(true);
            runOnUiThread(() -> mBinding.cheatAlert.setVisibility(View.VISIBLE));
            runOnUiThread(() -> firebaseAnalytics.logEvent("cheat_used", null));
        }

        @Override
        public void onGameReady() {
            isGameReady = true;
        }

        @Override
        public void onStartupSequenceStarted() {
            isGameReady = false;
        }

        @Override
        public void onRemainingBallsRequested(int balls) {
            remainingBalls = balls;
            if (PrefsHelper.getRemainingBalls()) {
                String str = getString(R.string.remainingballs, balls);
                runOnUiThread(() -> setTextWithBackground(mBinding.ballstxt, str));
            }
        }

        @Override
        public void onHapticFeedback(float intensity) {
            runOnUiThread(() -> triggerCollisionHaptic(intensity));
        }

        @Override
        public void onBallCapturedChanged(boolean captured) {
            // Enable low-pass filter when ball is captured (in hole/sink) for muffled effect
            setMusicLowPassEnabled(captured);
        }

        @Override
        public void onMissionActiveChanged(boolean active) {
            // Enable/disable mission music track overlay
            setMissionMusicEnabled(active);
            Log.i(TAG, "Mission music " + (active ? "enabled" : "disabled"));
        }
    };

    @Override
    protected void onResume() {
        super.onResume();
        StateHelper.INSTANCE.addListener(mStateListener);
        if (PrefsHelper.getMusic()) {
            resumeMusic(); // Oboe
            if (beatMapPlayer != null && PrefsHelper.getBeatReactiveGlow()) {
                beatMapPlayer.resume();
            }
        }

        if (!isPlaying) pauseNativeThread();
        if (isGameReady) {
            setVolume(PrefsHelper.getVolume());
            setMusicVolume(PrefsHelper.getMusicVolume() / 100.0f); // Oboe
        }
        PrefsHelper.setCheatsUsed(checkCheatsUsed());
        setTiltButtons();
        setCustomFonts();
        setBallsText();
        configurePlunger();
        setFullscreen();
        sensorManager.registerListener(accelerometerListener, accelerometer, SensorManager.SENSOR_DELAY_NORMAL);
        
        // Sync light edit mode from preferences
        lightEditModeEnabled = PrefsHelper.getLightEditMode();
        setLightEditMode(lightEditModeEnabled);
    }

    private void setBallsText() {
        if (!PrefsHelper.getRemainingBalls()) {
            String str = getString(R.string.balls, ballCount);
            setTextWithBackground(mBinding.ballstxt, str);
        } else {
            String str = getString(R.string.remainingballs, remainingBalls);
            setTextWithBackground(mBinding.ballstxt, str);
        }
    }

    private void configurePlunger() {
        if (PrefsHelper.getShouldShowBottomPlunger()) {
            PrefsHelper.setShouldShowBottomPlunger(false);
            bottomSheetBehavior.setState(BottomSheetBehavior.STATE_EXPANDED);
            mBinding.plunger.setVisibility(View.INVISIBLE);
        }
    }

    private void setTiltButtons() {
        if (PrefsHelper.getTiltButtons()) {
            mBinding.tiltLeft.setVisibility(View.VISIBLE);
            mBinding.tiltRight.setVisibility(View.VISIBLE);
            mBinding.tiltBottom.setVisibility(View.VISIBLE);
        } else {
            mBinding.tiltLeft.setVisibility(View.GONE);
            mBinding.tiltRight.setVisibility(View.GONE);
            mBinding.tiltBottom.setVisibility(View.GONE);
        }
    }

    private void setCustomFonts() {
        if (PrefsHelper.getCustomFonts()) {
            mBinding.ballstxt.setTypeface(ResourcesCompat.getFont(getContext(), R.font.nes_arcade));
            mBinding.ballstxt.setTextColor(Color.WHITE);
            mBinding.ballstxt.setTextSize(TypedValue.COMPLEX_UNIT_SP, 8);
            mBinding.txtscore.setTypeface(ResourcesCompat.getFont(getContext(), R.font.nes_arcade));
            mBinding.txtscore.setTextColor(Color.WHITE);
            mBinding.txtscore.setTextSize(TypedValue.COMPLEX_UNIT_SP, 8);
            mBinding.infotxt.setTypeface(ResourcesCompat.getFont(getContext(), R.font.nes_arcade));
            mBinding.infotxt.setTextColor(Color.WHITE);
            mBinding.infotxt.setTextSize(TypedValue.COMPLEX_UNIT_SP, 8);
            mBinding.missiontxt.setTypeface(ResourcesCompat.getFont(getContext(), R.font.nes_arcade));
            mBinding.missiontxt.setTextColor(Color.WHITE);
            mBinding.missiontxt.setTextSize(TypedValue.COMPLEX_UNIT_SP, 8);
            mBinding.plunger.setTypeface(ResourcesCompat.getFont(getContext(), R.font.nes_arcade));
            mBinding.plunger.setTextSize(TypedValue.COMPLEX_UNIT_SP, 8);
            // not editing the plunger because it's a button (and using its color as default color)
            mBinding.bottomPlunger.setTypeface(ResourcesCompat.getFont(getContext(), R.font.nes_arcade));
            mBinding.bottomPlunger.setTextColor(Color.WHITE);
            mBinding.bottomPlunger.setTextSize(TypedValue.COMPLEX_UNIT_SP, 8);
            mBinding.tiltLeft.setTypeface(ResourcesCompat.getFont(getContext(), R.font.nes_arcade));
            mBinding.tiltLeft.setTextColor(Color.WHITE);
            mBinding.tiltLeft.setTextSize(TypedValue.COMPLEX_UNIT_SP, 8);
            mBinding.tiltBottom.setTypeface(ResourcesCompat.getFont(getContext(), R.font.nes_arcade));
            mBinding.tiltBottom.setTextColor(Color.WHITE);
            mBinding.tiltBottom.setTextSize(TypedValue.COMPLEX_UNIT_SP, 8);
            mBinding.tiltRight.setTypeface(ResourcesCompat.getFont(getContext(), R.font.nes_arcade));
            mBinding.tiltRight.setTextColor(Color.WHITE);
            mBinding.tiltRight.setTextSize(TypedValue.COMPLEX_UNIT_SP, 8);
            mBinding.left.setTypeface(ResourcesCompat.getFont(getContext(), R.font.nes_arcade));
            mBinding.left.setTextColor(Color.WHITE);
            mBinding.left.setTextSize(TypedValue.COMPLEX_UNIT_SP, 8);
            mBinding.right.setTypeface(ResourcesCompat.getFont(getContext(), R.font.nes_arcade));
            mBinding.right.setTextColor(Color.WHITE);
            mBinding.right.setTextSize(TypedValue.COMPLEX_UNIT_SP, 8);
            mBinding.txtTimer.setTypeface(ResourcesCompat.getFont(getContext(), R.font.nes_arcade));
            mBinding.txtTimer.setTextSize(TypedValue.COMPLEX_UNIT_SP, 8);
            mBinding.txtTimerBonus.setTypeface(ResourcesCompat.getFont(getContext(), R.font.nes_arcade));
            mBinding.txtTimerBonus.setTextSize(TypedValue.COMPLEX_UNIT_SP, 8);
        } else {
            mBinding.ballstxt.setTypeface(Typeface.DEFAULT);
            mBinding.ballstxt.setTextColor(Color.WHITE);
            mBinding.txtscore.setTypeface(Typeface.DEFAULT);
            mBinding.txtscore.setTextColor(Color.WHITE);
            mBinding.infotxt.setTypeface(Typeface.DEFAULT);
            mBinding.infotxt.setTextColor(Color.WHITE);
            mBinding.missiontxt.setTypeface(Typeface.DEFAULT);
            mBinding.missiontxt.setTextColor(Color.WHITE);
            mBinding.plunger.setTypeface(Typeface.DEFAULT);
            mBinding.tiltLeft.setTypeface(Typeface.DEFAULT);
            mBinding.tiltLeft.setTextColor(Color.WHITE);
            mBinding.tiltBottom.setTypeface(Typeface.DEFAULT);
            mBinding.tiltBottom.setTextColor(Color.WHITE);
            mBinding.tiltRight.setTypeface(Typeface.DEFAULT);
            mBinding.tiltRight.setTextColor(Color.WHITE);
            mBinding.left.setTypeface(Typeface.DEFAULT);
            mBinding.left.setTextColor(Color.WHITE);
            mBinding.right.setTypeface(Typeface.DEFAULT);
            mBinding.right.setTextColor(Color.WHITE);
            mBinding.bottomPlunger.setTypeface(Typeface.DEFAULT);
            mBinding.bottomPlunger.setTextColor(Color.WHITE);
        }
    }

    @Override
    public void onConfigurationChanged(Configuration newConfig) {
        super.onConfigurationChanged(newConfig);

        if (newConfig.orientation == Configuration.ORIENTATION_LANDSCAPE) {
            mBinding.infotxt.setVisibility(View.GONE);
            mBinding.missiontxt.setVisibility(View.GONE);
            mBinding.txtscore.setVisibility(View.GONE);
            mBinding.ballstxt.setVisibility(View.GONE);
        } else {
            mBinding.infotxt.setVisibility(View.VISIBLE);
            mBinding.missiontxt.setVisibility(View.VISIBLE);
            mBinding.txtscore.setVisibility(View.VISIBLE);
            mBinding.ballstxt.setVisibility(View.VISIBLE);
        }
    }

    @Override
    protected void onPause() {
        super.onPause();
        StateHelper.INSTANCE.removeListener(mStateListener);
        pauseMusic(); // Oboe
        if (beatMapPlayer != null) beatMapPlayer.pause();
//        sensorManager.registerListener(accelerometerListener, accelerometer, SensorManager.SENSOR_DELAY_NORMAL);
        sensorManager.unregisterListener(accelerometerListener);
    }

    @Override
    protected void onStop() {
        Bundle bndl = new Bundle();
        bndl.putInt("games_per_session", gamesInSession);
        firebaseAnalytics.logEvent("games_per_session", bndl);
        super.onStop();
    }

    @Override
    protected String getMainFunction() {
        return "main";
    }

    @Override
    protected String[] getLibraries() {
        return new String[] {
                "SDL2",
                "SpaceCadetPinball"
        };
    }

    private void setTextWithBackground(TextView textView, String text) {
        textView.setText(text);
        // Hide background if text is empty or only whitespace
        if (text == null || text.trim().isEmpty()) {
            textView.setBackground(null);
        } else {
            textView.setBackgroundResource(R.drawable.text_background);
        }
    }

    private void putTranslations() {
        int[] ids = getResources().getIntArray(R.array.gametexts_idxs);
        String[] texts = getResources().getStringArray(R.array.gametexts_strings);
        for (int i = 0; i < ids.length; i++) {
            putString(ids[i], texts[i]);
        }
    }

    private native void initNative(String dataPath, boolean enhancedAudio);

    private native void setVolume(int vol);

    private native void putString(int id, String str);

    private native boolean checkCheatsUsed();

    // Light editor public methods
    public void toggleLightEditMode() {
        lightEditModeEnabled = !lightEditModeEnabled;
        setLightEditMode(lightEditModeEnabled);
        if (lightEditModeEnabled) {
            Toast.makeText(this, "Light Edit Mode ON - drag lights to reposition", Toast.LENGTH_LONG).show();
        } else {
            Toast.makeText(this, "Light Edit Mode OFF", Toast.LENGTH_SHORT).show();
        }
    }
    
    public void saveLightConfig() {
        String path = getFilesDir().getAbsolutePath() + "/light_positions.cfg";
        if (saveLightPositions(path)) {
            Toast.makeText(this, "Light positions saved to " + path, Toast.LENGTH_LONG).show();
        } else {
            Toast.makeText(this, "Failed to save light positions", Toast.LENGTH_SHORT).show();
        }
    }
    public void loadLightConfig() {
        String path = getFilesDir().getAbsolutePath() + "/light_positions.cfg";
        if (loadLightPositions(path)) {
            Toast.makeText(this, "Light positions loaded", Toast.LENGTH_SHORT).show();
        }
    }
    
    public void toggleDebugBall() {
        enableDebugBall(true);  // Enable debug ball
        Toast.makeText(this, "Debug ball enabled - red dot shows tracked position", Toast.LENGTH_LONG).show();
    }

    // Track if we're currently dragging a light
    private boolean isDraggingLight = false;

    // Plunger drag state
    private boolean isDraggingPlunger = false;
    private float plungerStartY = 0f;
    private float plungerCurrentY = 0f;
    private float plungerMaxDragDistance = 400.0f; // Maximum drag distance in pixels (increased for better control)
    private float lastHapticPosition = -1f; // Track last position for haptic feedback
    private long lastHapticTime = 0L; // Track last haptic time for rate limiting
    private Vibrator plungerVibrator;

    // Track if we're dragging in debug mode
    private boolean isDraggingDebugLight = false;

    @Override
    public boolean dispatchTouchEvent(MotionEvent event) {
        if (event == null) return false;
        
        // When light debug panel is visible, check if touch is on the panel or on the game area
        if (isLightDebugPanelVisible()) {
            float x = event.getX();
            float y = event.getY();
            
            // Check if touch is within the debug panel bounds
            int[] panelLocation = new int[2];
            mBinding.lightDebugPanel.getLocationOnScreen(panelLocation);
            int panelLeft = panelLocation[0];
            int panelTop = panelLocation[1];
            int panelRight = panelLeft + mBinding.lightDebugPanel.getWidth();
            int panelBottom = panelTop + mBinding.lightDebugPanel.getHeight();
            
            boolean touchOnPanel = (x >= panelLeft && x <= panelRight && y >= panelTop && y <= panelBottom);
            
            if (touchOnPanel) {
                // Let Android UI handle panel touches
                return super.dispatchTouchEvent(event);
            }
            
            // Touch is outside panel - use it to reposition the selected HDR light
            int viewportW = getWindow().getDecorView().getWidth();
            int viewportH = getWindow().getDecorView().getHeight();
            
            switch (event.getAction()) {
            case MotionEvent.ACTION_DOWN:
                // Start dragging the selected debug light
                isDraggingDebugLight = true;
                onDebugLightTouchDown(x, y, 0, 0, viewportW, viewportH);
                return true;
            case MotionEvent.ACTION_MOVE:
                if (isDraggingDebugLight) {
                    onDebugLightTouchMove(x, y, 0, 0, viewportW, viewportH);
                    return true;
                }
                break;
            case MotionEvent.ACTION_UP:
            case MotionEvent.ACTION_CANCEL:
                if (isDraggingDebugLight) {
                    onDebugLightTouchUp();
                    isDraggingDebugLight = false;
                    return true;
                }
                break;
            }
            return true;
        }
        
        try {
            // Handle light editing touches
            if (lightEditModeEnabled) {
                int viewportW = getWindow().getDecorView().getWidth();
                int viewportH = getWindow().getDecorView().getHeight();

                float x = event.getX();
                float y = event.getY();

                switch (event.getAction()) {
                case MotionEvent.ACTION_DOWN:
                    // Check if touch should be blocked (settings button area)
                    if (shouldBlockTouch(x, y, 0, 0, viewportW, viewportH)) {
                        return super.dispatchTouchEvent(event); // Pass to Android UI
                    }
                    // Check if touch is near a light or bumper - native code will set selection
                    onLightTouchDown(x, y, 0, 0, viewportW, viewportH);
                    // Check if a light or bumper was selected
                    isDraggingLight = hasLightSelection();
                    if (isDraggingLight) {
                        return true; // Consume touch if we selected something
                    }
                    break;
                case MotionEvent.ACTION_MOVE:
                    if (isDraggingLight) {
                        onLightTouchMove(x, y, 0, 0, viewportW, viewportH);
                        return true; // Consume while dragging
                    }
                    break;
                case MotionEvent.ACTION_UP:
                case MotionEvent.ACTION_CANCEL:
                    if (isDraggingLight) {
                        onLightTouchUp();
                        isDraggingLight = false;
                        return true;
                    }
                    break;
                }
            }
        } catch (Exception e) {
            Log.e(TAG, "Error in dispatchTouchEvent", e);
        }
        return super.dispatchTouchEvent(event);
    }

    // HDR native methods
    private native void setHDRCapabilities(boolean supported, boolean bt2020, boolean pq, 
                                           boolean scrgb, boolean fp16, float maxNits, float minNits);
    private native boolean isHDRActive();
    private native void setHDREnabled(boolean enabled);
    private native void setParticlesEnabled(boolean enabled);
    private native void setMusicEnabled(boolean enabled);

    // Light editor native methods
    private native void setLightEditMode(boolean enabled);
    private native boolean getLightEditMode();
    private native boolean shouldBlockTouch(float screenX, float screenY, int viewportX, int viewportY, int viewportW, int viewportH);
    private native void onLightTouchDown(float screenX, float screenY, int viewportX, int viewportY, int viewportW, int viewportH);
    private native void onLightTouchMove(float screenX, float screenY, int viewportX, int viewportY, int viewportW, int viewportH);
    private native void onLightTouchUp();

    // Debug light repositioning native methods (for debug mode without editor)
    private native void onDebugLightTouchDown(float screenX, float screenY, int viewportX, int viewportY, int viewportW, int viewportH);
    private native void onDebugLightTouchMove(float screenX, float screenY, int viewportX, int viewportY, int viewportW, int viewportH);
    private native void onDebugLightTouchUp();
    private native boolean saveLightPositions(String filepath);
    private native boolean loadLightPositions(String filepath);
    private native int getSelectedLightIndex();
    private native boolean hasLightSelection();
    private native void setDebugBallPosition(float x, float y);
    private native void enableDebugBall(boolean enabled);
    private native void triggerDemoMode();
    private native void setHDRGlowModifier(float modifier);
    private native void setTrailOpacity(float opacity);
    private native void setTrailLifetime(float seconds);
    private native void setCameraTracking(boolean enabled, float zoom);
    private native void toggleTableLightNative();
    private native void toggleHDRLightNative();
    private native String getCurrentLightInfoNative();
    private native void turnOffAllLightsNative();

    // Plunger control native methods
    private native void updatePlungerPosition(float position);
    private native void setPlungerLaunchPower(float power);

    // Light debug mode native methods
    private native void setLightDebugModeNative(boolean enabled);
    private native void nextLightNative();
    private native void previousLightNative();

    // Preset management native methods
    private native int getPresetCountNative();
    private native String getPresetNameNative(int index);
    private native void createPresetFromCurrentLightNative(String presetName);
    private native void applyPresetToCurrentLightNative(String presetName);
    private native String getCurrentLightPresetNative();
    private native void clearPresetFromCurrentLightNative();
    private native void savePresetsNative(String filepath);
    private native void loadPresetsNative(String filepath);

    // Live property editing native methods
    private native float[] getCurrentLightPropertiesNative();
    private native void setCurrentLightColorNative(float r, float g, float b);
    private native void setCurrentLightSizeNative(float width, float height);
    private native void setCurrentLightIntensityNative(float intensityOn, float intensityFlash);
    private native void setCurrentLightGlowNative(float glowRadius);
    private native void nudgeCurrentLightNative(float dx, float dy);
    private native void setCurrentLightLockedNative(boolean locked);

    // Enhanced audio native method
    private native void setEnhancedAudio(boolean enabled);

    // Oboe music player native methods
    private native void initOboeMusicPlayer();
    private native void destroyOboeMusicPlayer();
    private native boolean loadMusicFromFile(String path);
    private native boolean loadMusicFromAssets(android.content.res.AssetManager assetManager, String assetPath);
    private native boolean startMusic();
    private native void stopMusic();
    private native void pauseMusic();
    private native void resumeMusic();
    private native void setMusicVolume(float volume);
    private native void setMusicLowPassEnabled(boolean enabled);
    private native void setMusicLowPassCutoff(float freq);
    private native long getMusicPositionMs();
    private native boolean isMusicPlaying();

    // Mission track native methods
    private native boolean loadMissionMusicFromAssets(android.content.res.AssetManager assetManager, String assetPath);
    private native boolean loadMissionMusicCompressed(android.content.res.AssetManager assetManager, String assetPath, String cacheDir);
    private native void setMissionMusicEnabled(boolean enabled);
    private native void setMissionMusicVolume(float volume);

    // Compressed audio native methods
    private native boolean loadCompressedMusicFromAssets(android.content.res.AssetManager assetManager, String assetPath, String cacheDir);
    
    // WAV with cache native methods
    private native boolean loadMusicFromAssetsWithCache(android.content.res.AssetManager assetManager, String assetPath, String cacheDir);
    private native boolean loadMissionMusicFromAssetsWithCache(android.content.res.AssetManager assetManager, String assetPath, String cacheDir);

    // Timer mode native methods
    private native void setTimerMode(boolean enabled);
    private native boolean isTimerMode();
    private native int getTimerRemainingMs();
    private native void startTimerMode();
    private native void resetTimerMode();
    private native int getTimerScoreProgress();
    private native int getTimerThresholdIncrement();
    private native void pauseTimerMode();
    private native void resumeTimerMode();

    // Session timer native methods (works for both modes)
    private native void startSessionTimer();
    private native void stopSessionTimer();
    private native void pauseSessionTimer();
    private native void resumeSessionTimer();
    private native long getSessionTimeMs();

    // Game stats native methods
    private native int getPlayerRank();
    private native int getOuterCircleProgress();
    private native int getOuterCircleTotal();
    private native int getTotalScore();

    // Flag to prevent slider feedback loops
    private boolean isUpdatingSliders = false;

    // Nudge amount (in normalized coordinates)
    private static final float NUDGE_AMOUNT = 0.0005f;

    // ... (rest of the code remains the same)
    private void updateLightDebugInfo() {
        String lightInfo = getCurrentLightInfoNative();
        if (lightInfo != null && !lightInfo.isEmpty()) {
            mBinding.currentLightText.setText("Current: " + lightInfo);
        } else {
            mBinding.currentLightText.setText("Current: None");
        }
        String presetName = getCurrentLightPresetNative();
        if (presetName != null && !presetName.isEmpty()) {
            mBinding.currentPresetText.setText(presetName);
        } else {
            mBinding.currentPresetText.setText("(none)");
        }
        updateSlidersFromLight();
    }

    private void updateSlidersFromLight() {
        isUpdatingSliders = true;
        try {
            float[] props = getCurrentLightPropertiesNative();
            if (props != null && props.length >= 12) {
                mBinding.sliderR.setProgress((int)(props[0] * 100));
                mBinding.valueR.setText(String.format("%.1f", props[0]));
                mBinding.sliderG.setProgress((int)(props[1] * 100));
                mBinding.valueG.setText(String.format("%.1f", props[1]));
                mBinding.sliderB.setProgress((int)(props[2] * 100));
                mBinding.valueB.setText(String.format("%.1f", props[2]));
                mBinding.sliderSize.setProgress((int)(props[3] * 1000));
                mBinding.valueSize.setText(String.format("%.3f", props[3]));
                mBinding.sliderIntensity.setProgress((int)props[5]);
                mBinding.valueIntensity.setText(String.format("%.0f", props[5]));
                mBinding.sliderGlow.setProgress((int)(props[7] * 100));
                mBinding.valueGlow.setText(String.format("%.1f", props[7]));
                mBinding.valuePosX.setText(String.format("X:%.4f", props[9]));
                mBinding.valuePosY.setText(String.format(" Y:%.4f", props[10]));
                mBinding.checkLocked.setChecked(props[11] > 0.5f);
            }
        } finally {
            isUpdatingSliders = false;
        }
    }

    private void setupLightDebugPanel() {
        mBinding.prevLightBtn.setOnClickListener(v -> { previousLightNative(); updateLightDebugInfo(); });
        mBinding.nextLightBtn.setOnClickListener(v -> { nextLightNative(); updateLightDebugInfo(); });
        mBinding.toggleTableLightBtn.setOnClickListener(v -> toggleTableLightNative());
        mBinding.toggleHDRLightBtn.setOnClickListener(v -> toggleHDRLightNative());
        mBinding.closeLightDebugBtn.setOnClickListener(v -> hideLightDebugPanel());
        mBinding.swapSideBtn.setOnClickListener(v -> swapDebugPanelSide());
        mBinding.createPresetBtn.setOnClickListener(v -> showCreatePresetDialog());
        mBinding.applyPresetBtn.setOnClickListener(v -> showApplyPresetDialog());
        mBinding.clearPresetBtn.setOnClickListener(v -> { clearPresetFromCurrentLightNative(); updateLightDebugInfo(); });
        mBinding.nudgeUpBtn.setOnClickListener(v -> { nudgeCurrentLightNative(0, -NUDGE_AMOUNT); updateSlidersFromLight(); });
        mBinding.nudgeDownBtn.setOnClickListener(v -> { nudgeCurrentLightNative(0, NUDGE_AMOUNT); updateSlidersFromLight(); });
        mBinding.nudgeLeftBtn.setOnClickListener(v -> { nudgeCurrentLightNative(-NUDGE_AMOUNT, 0); updateSlidersFromLight(); });
        mBinding.nudgeRightBtn.setOnClickListener(v -> { nudgeCurrentLightNative(NUDGE_AMOUNT, 0); updateSlidersFromLight(); });
        mBinding.checkLocked.setOnCheckedChangeListener((buttonView, isChecked) -> {
            if (!isUpdatingSliders) {
                setCurrentLightLockedNative(isChecked);
            }
        });
        mBinding.saveLightBtn.setOnClickListener(v -> {
            String path = getFilesDir().getAbsolutePath() + "/light_positions.cfg";
            if (saveLightPositions(path)) {
                Toast.makeText(this, "Light saved to config", Toast.LENGTH_SHORT).show();
            } else {
                Toast.makeText(this, "Failed to save", Toast.LENGTH_SHORT).show();
            }
        });
        setupPropertySliders();
    }

    private void setupPropertySliders() {
        mBinding.sliderR.setOnSeekBarChangeListener(new android.widget.SeekBar.OnSeekBarChangeListener() {
            public void onProgressChanged(android.widget.SeekBar sb, int p, boolean u) {
                if (isUpdatingSliders || !u) return;
                float r = p / 100.0f; mBinding.valueR.setText(String.format("%.1f", r));
                setCurrentLightColorNative(r, mBinding.sliderG.getProgress()/100f, mBinding.sliderB.getProgress()/100f);
            }
            public void onStartTrackingTouch(android.widget.SeekBar sb) {}
            public void onStopTrackingTouch(android.widget.SeekBar sb) {}
        });
        mBinding.sliderG.setOnSeekBarChangeListener(new android.widget.SeekBar.OnSeekBarChangeListener() {
            public void onProgressChanged(android.widget.SeekBar sb, int p, boolean u) {
                if (isUpdatingSliders || !u) return;
                float g = p / 100.0f; mBinding.valueG.setText(String.format("%.1f", g));
                setCurrentLightColorNative(mBinding.sliderR.getProgress()/100f, g, mBinding.sliderB.getProgress()/100f);
            }
            public void onStartTrackingTouch(android.widget.SeekBar sb) {}
            public void onStopTrackingTouch(android.widget.SeekBar sb) {}
        });
        mBinding.sliderB.setOnSeekBarChangeListener(new android.widget.SeekBar.OnSeekBarChangeListener() {
            public void onProgressChanged(android.widget.SeekBar sb, int p, boolean u) {
                if (isUpdatingSliders || !u) return;
                float b = p / 100.0f; mBinding.valueB.setText(String.format("%.1f", b));
                setCurrentLightColorNative(mBinding.sliderR.getProgress()/100f, mBinding.sliderG.getProgress()/100f, b);
            }
            public void onStartTrackingTouch(android.widget.SeekBar sb) {}
            public void onStopTrackingTouch(android.widget.SeekBar sb) {}
        });
        mBinding.sliderSize.setOnSeekBarChangeListener(new android.widget.SeekBar.OnSeekBarChangeListener() {
            public void onProgressChanged(android.widget.SeekBar sb, int p, boolean u) {
                if (isUpdatingSliders || !u) return;
                float sz = p / 1000.0f; mBinding.valueSize.setText(String.format("%.3f", sz));
                setCurrentLightSizeNative(sz, sz);
            }
            public void onStartTrackingTouch(android.widget.SeekBar sb) {}
            public void onStopTrackingTouch(android.widget.SeekBar sb) {}
        });
        mBinding.sliderIntensity.setOnSeekBarChangeListener(new android.widget.SeekBar.OnSeekBarChangeListener() {
            public void onProgressChanged(android.widget.SeekBar sb, int p, boolean u) {
                if (isUpdatingSliders || !u) return;
                mBinding.valueIntensity.setText(String.format("%d", p));
                setCurrentLightIntensityNative(p, p * 1.5f);
            }
            public void onStartTrackingTouch(android.widget.SeekBar sb) {}
            public void onStopTrackingTouch(android.widget.SeekBar sb) {}
        });
        mBinding.sliderGlow.setOnSeekBarChangeListener(new android.widget.SeekBar.OnSeekBarChangeListener() {
            public void onProgressChanged(android.widget.SeekBar sb, int p, boolean u) {
                if (isUpdatingSliders || !u) return;
                float glow = p / 100.0f; mBinding.valueGlow.setText(String.format("%.1f", glow));
                setCurrentLightGlowNative(glow);
            }
            public void onStartTrackingTouch(android.widget.SeekBar sb) {}
            public void onStopTrackingTouch(android.widget.SeekBar sb) {}
        });
    }

    private void showCreatePresetDialog() {
        android.app.AlertDialog.Builder builder = new android.app.AlertDialog.Builder(this);
        builder.setTitle("Create Preset");
        final android.widget.EditText input = new android.widget.EditText(this);
        input.setHint("Preset name");
        input.setTextColor(Color.WHITE);
        input.setHintTextColor(Color.GRAY);
        builder.setView(input);
        builder.setPositiveButton("Create", (d, w) -> {
            String name = input.getText().toString().trim();
            if (!name.isEmpty()) {
                createPresetFromCurrentLightNative(name);
                savePresetsNative(getFilesDir().getAbsolutePath() + "/light_presets.cfg");
                Toast.makeText(this, "Preset created", Toast.LENGTH_SHORT).show();
                updateLightDebugInfo();
            }
        });
        builder.setNegativeButton("Cancel", (d, w) -> d.cancel());
        builder.show();
    }

    private void showApplyPresetDialog() {
        int count = getPresetCountNative();
        if (count == 0) { 
            Toast.makeText(this, "No presets", Toast.LENGTH_SHORT).show(); 
            return; 
        }
        String[] names = new String[count];
        for (int i = 0; i < count; i++) names[i] = getPresetNameNative(i);
        android.app.AlertDialog.Builder builder = new android.app.AlertDialog.Builder(this);
        builder.setTitle("Apply Preset");
        builder.setItems(names, (d, w) -> { 
            applyPresetToCurrentLightNative(names[w]); 
            updateLightDebugInfo(); 
        });
        builder.setNegativeButton("Cancel", (d, w) -> d.cancel());
        builder.show();
    }

    public void showLightDebugPanel() {
        mBinding.lightDebugPanel.setVisibility(View.VISIBLE);
        mBinding.lightDebugPanel.bringToFront();
        // Hide UI text elements
        mBinding.missiontxt.setVisibility(View.INVISIBLE);
        mBinding.ballstxt.setVisibility(View.INVISIBLE);
        mBinding.infotxt.setVisibility(View.INVISIBLE);
        mBinding.txtscore.setVisibility(View.INVISIBLE);
        setLightDebugModeNative(true);
        turnOffAllLightsNative();
        loadPresetsNative(getFilesDir().getAbsolutePath() + "/light_presets.cfg");
        updateLightDebugInfo();
        Toast.makeText(this, "Light Debug ON", Toast.LENGTH_SHORT).show();
    }

    public void hideLightDebugPanel() {
        mBinding.lightDebugPanel.setVisibility(View.GONE);
        // Show UI text elements again
        mBinding.missiontxt.setVisibility(View.VISIBLE);
        mBinding.ballstxt.setVisibility(View.VISIBLE);
        mBinding.infotxt.setVisibility(View.VISIBLE);
        mBinding.txtscore.setVisibility(View.VISIBLE);
        setLightDebugModeNative(false);
        Toast.makeText(this, "Light Debug OFF", Toast.LENGTH_SHORT).show();
    }

    // Track which side the debug panel is on
    private boolean debugPanelOnRight = false;

    private void swapDebugPanelSide() {
        debugPanelOnRight = !debugPanelOnRight;
        androidx.constraintlayout.widget.ConstraintLayout.LayoutParams params = 
            (androidx.constraintlayout.widget.ConstraintLayout.LayoutParams) mBinding.lightDebugPanel.getLayoutParams();
        
        if (debugPanelOnRight) {
            // Move to right side
            params.startToStart = androidx.constraintlayout.widget.ConstraintLayout.LayoutParams.UNSET;
            params.endToEnd = androidx.constraintlayout.widget.ConstraintLayout.LayoutParams.PARENT_ID;
            params.setMarginStart(0);
            params.setMarginEnd(8);
        } else {
            // Move to left side
            params.endToEnd = androidx.constraintlayout.widget.ConstraintLayout.LayoutParams.UNSET;
            params.startToStart = androidx.constraintlayout.widget.ConstraintLayout.LayoutParams.PARENT_ID;
            params.setMarginEnd(0);
            params.setMarginStart(8);
        }
        mBinding.lightDebugPanel.setLayoutParams(params);
    }

    public boolean isLightDebugPanelVisible() {
        return mBinding != null && mBinding.lightDebugPanel.getVisibility() == View.VISIBLE;
    }

    // Timer Mode Methods
    // Track pending bonus to group multiple bonuses
    private int pendingBonusSeconds = 0;
    private Runnable bonusHideRunnable = null;

    private void showModeSelectionDialog() {
        waitingForModeSelection = true;
        
        // Hide all text UI until game begins
        mBinding.txtscore.setVisibility(View.GONE);
        mBinding.ballstxt.setVisibility(View.GONE);
        mBinding.txtTimer.setVisibility(View.GONE);
        mBinding.txtTimerBonus.setVisibility(View.GONE);
        mBinding.missiontxt.setVisibility(View.GONE);
        mBinding.infotxt.setVisibility(View.GONE);
        
        android.app.AlertDialog.Builder builder = new android.app.AlertDialog.Builder(this);
        builder.setTitle("Select Game Mode");
        builder.setCancelable(false);
        
        String[] modes = {"Classic Mode", "Timer Mode"};
        builder.setItems(modes, (dialog, which) -> {
            waitingForModeSelection = false;
            pendingGameStart = true;  // Wait for ball to enter plunger before starting music/timer
            
            // Start session timer for both modes
            startSessionTimer();
            
            if (which == 0) {
                // Classic mode - will show ball count and score when game starts
                setTimerMode(false);
                timerModeActive = false;
            } else {
                // Timer mode - will show timer and score progress when game starts
                setTimerMode(true);
                timerModeActive = true;
                startTimerUpdateLoop();
            }
            // UI elements will be shown when ball first enters plunger
        });
        builder.show();
    }

    private void startTimerUpdateLoop() {
        if (timerUpdateHandler == null) {
            timerUpdateHandler = new Handler(Looper.getMainLooper());
        }
        
        timerUpdateRunnable = new Runnable() {
            @Override
            public void run() {
                if (timerModeActive && isTimerMode()) {
                    int remainingMs = getTimerRemainingMs();
                    int seconds = remainingMs / 1000;
                    int minutes = seconds / 60;
                    seconds = seconds % 60;
                    
                    String timerText = String.format("%d:%02d", minutes, seconds);
                    mBinding.txtTimer.setText(timerText);
                    
                    // Change color based on time remaining
                    // White normally, orange when <=60s, red when <=30s
                    if (remainingMs <= 30000) {
                        mBinding.txtTimer.setTextColor(Color.RED);
                    } else if (remainingMs <= 60000) {
                        mBinding.txtTimer.setTextColor(Color.parseColor("#FF8800")); // Orange
                    } else {
                        mBinding.txtTimer.setTextColor(Color.WHITE);
                    }
                    
                    // Update score progress display (score/threshold for next +15s)
                    // Threshold is 100k + 25k per rank
                    int scoreProgress = getTimerScoreProgress();
                    int threshold = getTimerThresholdIncrement();
                    String progressText = String.format("%,d / %,d", scoreProgress, threshold);
                    setTextWithBackground(mBinding.txtscore, progressText);
                    
                    // Continue updating every 100ms
                    timerUpdateHandler.postDelayed(this, 100);
                }
            }
        };
        
        timerUpdateHandler.post(timerUpdateRunnable);
    }

    private void stopTimerUpdateLoop() {
        if (timerUpdateHandler != null && timerUpdateRunnable != null) {
            timerUpdateHandler.removeCallbacks(timerUpdateRunnable);
        }
        timerModeActive = false;
    }

    // Show time bonus/penalty popup (groups multiple bonuses)
    public void showTimerBonus(int secondsChange) {
        runOnUiThread(() -> {
            // Cancel any pending hide
            if (bonusHideRunnable != null && timerUpdateHandler != null) {
                timerUpdateHandler.removeCallbacks(bonusHideRunnable);
            }
            
            // Accumulate bonus
            pendingBonusSeconds += secondsChange;
            
            // Update display
            String text;
            if (pendingBonusSeconds > 0) {
                text = "+" + pendingBonusSeconds;
                mBinding.txtTimerBonus.setTextColor(Color.parseColor("#00FF00")); // Green
            } else {
                text = String.valueOf(pendingBonusSeconds);
                mBinding.txtTimerBonus.setTextColor(Color.RED);
            }
            mBinding.txtTimerBonus.setText(text);
            mBinding.txtTimerBonus.setVisibility(View.VISIBLE);
            
            // Schedule hide after 2 seconds
            bonusHideRunnable = () -> {
                mBinding.txtTimerBonus.setVisibility(View.GONE);
                pendingBonusSeconds = 0;
            };
            if (timerUpdateHandler != null) {
                timerUpdateHandler.postDelayed(bonusHideRunnable, 2000);
            }
        });
    }

    public void onTimerModeGameOver() {
        runOnUiThread(() -> {
            stopTimerUpdateLoop();
            mBinding.txtTimer.setVisibility(View.GONE);
            mBinding.txtTimerBonus.setVisibility(View.GONE);
        });
    }

    // Rank names for display
    private static final String[] RANK_NAMES = {
        "Cadet", "Ensign", "Lieutenant", "Captain", "Lt. Commander",
        "Commander", "Commodore", "Admiral", "Fleet Admiral"
    };

    private String getRankName(int rank) {
        if (rank < 1) return "Cadet";
        if (rank > RANK_NAMES.length) return "Fleet Admiral";
        return RANK_NAMES[rank - 1];
    }

    public void showGameOverSummary(int totalScore, long playTimeMs, int rank, 
                                     int outerCircleProgress, int outerCircleTotal) {
        // Format play time as MM:SS
        int totalSeconds = (int)(playTimeMs / 1000);
        int minutes = totalSeconds / 60;
        int seconds = totalSeconds % 60;
        String timeStr = String.format("%d:%02d", minutes, seconds);

        // Format score with commas
        String scoreStr = String.format("%,d", totalScore);

        // Get rank name
        String rankName = getRankName(rank);

        // Build mission points display (outer circle progress)
        String missionPointsStr;
        if (outerCircleTotal > 0) {
            missionPointsStr = String.format("%d / %d", outerCircleProgress, outerCircleTotal);
        } else {
            missionPointsStr = String.valueOf(outerCircleProgress);
        }

        // Build the summary message
        StringBuilder message = new StringBuilder();
        message.append("Score: ").append(scoreStr).append("\n\n");
        message.append("Time Played: ").append(timeStr).append("\n\n");
        message.append("Rank: ").append(rankName).append("\n\n");
        message.append("Mission Points: ").append(missionPointsStr);

        // Show dialog
        android.app.AlertDialog.Builder builder = new android.app.AlertDialog.Builder(this);
        builder.setTitle("Game Over");
        builder.setMessage(message.toString());
        builder.setPositiveButton("OK", (dialog, which) -> dialog.dismiss());
        builder.setCancelable(true);
        builder.show();
    }
}