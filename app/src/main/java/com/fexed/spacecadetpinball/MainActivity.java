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
import android.view.KeyEvent;
import android.view.MotionEvent;
import android.view.View;
import android.view.ViewGroup;
import android.widget.RelativeLayout;
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

    private SensorManager sensorManager;
    private Sensor accelerometer;

    // Light editor state
    private boolean lightEditModeEnabled = false;
    private int[] lastViewport = new int[4];  // x, y, w, h

    @SuppressLint("ClickableViewAccessibility")
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        File filesDir = getFilesDir();
        copyAssets(filesDir);
        PrefsHelper.setPrefs(getSharedPreferences("com.fexed.spacecadetpinball", Context.MODE_PRIVATE));

        // Initialize HDR capabilities BEFORE initNative so SDL can use HDR colorspace
        initializeHDR();
        
        initNative(filesDir.getAbsolutePath() + "/");

        sensorManager = (SensorManager) getSystemService(Context.SENSOR_SERVICE);
        accelerometer = sensorManager.getDefaultSensor(Sensor.TYPE_ACCELEROMETER);

        // try {
        //     AssetFileDescriptor afd = getAssets().openFd("PINBALL.mp3");
        //     player.setDataSource(afd.getFileDescriptor(), afd.getStartOffset(), afd.getLength());
        //     player.prepare();
        //     player.setLooping(true);
        //     player.setVolume(PrefsHelper.getVolume()/(float) 100, PrefsHelper.getVolume()/(float) 100);
        //     if (PrefsHelper.getMusic()) player.start();
        // } catch (IOException ignored) {
        //     player = null;
        // }

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

        // The Vibrator instance
        Vibrator vibrator2 = (Vibrator) getSystemService(Context.VIBRATOR_SERVICE);

        mBinding.plunger.setOnTouchListener((v1, event) -> {
            v1.performClick();
            if (event.getAction() == MotionEvent.ACTION_DOWN) {
                SDLActivity.onNativeKeyDown(KeyEvent.KEYCODE_SPACE);
                
                exeHaptic();
                // Start vibration

                long[] timings = new long[30];
                int[] amplitudes = new int[30];

                // Fill the timings with 100ms intervals
                for (int i = 0; i < timings.length; i++) {
                    timings[i] = 100;
                }

                // Fill the amplitudes with increasing intensity
                for (int i = 0; i < amplitudes.length; i++) {
                    amplitudes[i] = (i * 255) / 29; 
                }
                if (Build.VERSION.SDK_INT >= 29) {
                    // Create the vibration effect
                    VibrationEffect effect = VibrationEffect.createWaveform(timings, amplitudes, -1); // -1 means do not repeat

                    // Vibrate with the effect
                    if (vibrator2 != null && vibrator2.hasVibrator()) {
                        vibrator2.vibrate(effect);
                    }
                }
                return true; // Indicate that the touch event has been handled
            }
            if (event.getAction() == MotionEvent.ACTION_UP) {
                SDLActivity.onNativeKeyUp(KeyEvent.KEYCODE_SPACE);
                // Stop vibration
                if (vibrator2 != null) {
                    vibrator2.cancel();
                }
                exeClickH();
                
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
                if (player != null) player.pause();
                mBinding.playpause.setImageDrawable(getContext().getResources().getDrawable(R.drawable.play));
            } else {
                isPlaying = true;
                resumeNativeThread();
                if (player != null) player.start();
                mBinding.playpause.setImageDrawable(getContext().getResources().getDrawable(R.drawable.pause));

            }
        });

//        mBinding.replay.setOnClickListener(view -> {
//            Toast.makeText(getContext(), R.string.restartprompt, Toast.LENGTH_SHORT).show();
//        });


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
            triggerDemoMode();
            Toast.makeText(this, "Demo Mode", Toast.LENGTH_SHORT).show();
            return true;
        });

        firebaseAnalytics = FirebaseAnalytics.getInstance(this);
        firebaseAnalytics.logEvent(FirebaseAnalytics.Event.APP_OPEN, null);
    }

    private void copyAssets(File filesDir) {
        if (!new File(filesDir, "PINBALL.DAT").exists()) {
            AssetManager assetManager = getAssets();
            try {
                for (String asset : assetManager.list("")) {
                    Log.d(TAG, "Copying " + asset);
                    try (InputStream is = assetManager.open(asset)){
                        try (OutputStream os = new FileOutputStream(new File(filesDir, asset))) {
                            byte[] buffer = new byte[1024];
                            int len;
                            while ((len = is.read(buffer)) != -1) {
                                os.write(buffer, 0, len);
                            }
                        }
                    } catch (IOException e) {
                        e.printStackTrace();
                    }
                }
            } catch (IOException e) {
                e.printStackTrace();
            }
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
                runOnUiThread(() -> firebaseAnalytics.logEvent(FirebaseAnalytics.Event.LEVEL_START, null));
            }

            if (state == GameState.FINISHED) {
                runOnUiThread(() -> firebaseAnalytics.logEvent(FirebaseAnalytics.Event.LEVEL_END, null));
            }
        }

        @Override
        public void onBallInPlungerChanged(boolean isBallInPlunger) {
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
            if (type == 1) runOnUiThread(() -> mBinding.missiontxt.setText(fstr));
            else runOnUiThread(() -> mBinding.infotxt.setText(fstr));
        }

        @Override
        public void onClearText(int type) {
            if (type == 1) runOnUiThread(() -> mBinding.missiontxt.setText(""));
            else runOnUiThread(() -> mBinding.infotxt.setText(""));
        }

        @Override
        public void onScorePosted(int score) {
            String str = "SCORE: " + score;
            runOnUiThread(() -> mBinding.txtscore.setText(str));
        }

        @Override
        public void onBallCountUpdated(int count) {
            ballCount = count;
            if (!PrefsHelper.getRemainingBalls()) {
                String str = getString(R.string.balls, count);
                runOnUiThread(() -> mBinding.ballstxt.setText(str));
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
        public void onRemainingBallsRequested(int balls) {
            remainingBalls = balls;
            if (PrefsHelper.getRemainingBalls()) {
                String str = getString(R.string.remainingballs, balls);
                runOnUiThread(() -> mBinding.ballstxt.setText(str));
            }
        }

        @Override
        public void onHapticFeedback(float intensity) {
            runOnUiThread(() -> triggerCollisionHaptic(intensity));
        }
    };

    @Override
    protected void onResume() {
        super.onResume();
        StateHelper.INSTANCE.addListener(mStateListener);
        if (player != null && PrefsHelper.getMusic()) player.start();

        if (!isPlaying) pauseNativeThread();
        if (isGameReady) {
            setVolume(PrefsHelper.getVolume());
            if (player != null) player.setVolume(PrefsHelper.getVolume()/(float) 100, PrefsHelper.getVolume()/(float) 100);
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
            mBinding.ballstxt.setText(str);
        } else {
            String str = getString(R.string.remainingballs, remainingBalls);
            mBinding.ballstxt.setText(str);
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
            mBinding.ballstxt.setTypeface(ResourcesCompat.getFont(getContext(), R.font.bauhauscheavy));
            mBinding.ballstxt.setTextColor(ResourcesCompat.getColor(getResources(), R.color.purple_200, getTheme()));
            mBinding.txtscore.setTypeface(ResourcesCompat.getFont(getContext(), R.font.bauhauscheavy));
            mBinding.txtscore.setTextColor(ResourcesCompat.getColor(getResources(), R.color.purple_200, getTheme()));
            mBinding.infotxt.setTypeface(ResourcesCompat.getFont(getContext(), R.font.bauhauscheavy));
            mBinding.infotxt.setTextColor(ResourcesCompat.getColor(getResources(), R.color.purple_200, getTheme()));
            mBinding.missiontxt.setTypeface(ResourcesCompat.getFont(getContext(), R.font.bauhauscheavy));
            mBinding.missiontxt.setTextColor(ResourcesCompat.getColor(getResources(), R.color.purple_200, getTheme()));
            mBinding.plunger.setTypeface(ResourcesCompat.getFont(getContext(), R.font.bauhauscheavy));
            // not editing the plunger because it's a button (and using its color as default color)
            mBinding.bottomPlunger.setTypeface(ResourcesCompat.getFont(getContext(), R.font.bauhauscheavy));
            mBinding.bottomPlunger.setTextColor(ResourcesCompat.getColor(getResources(), R.color.purple_200, getTheme()));
            mBinding.tiltLeft.setTypeface(ResourcesCompat.getFont(getContext(), R.font.bauhauscheavy));
            mBinding.tiltLeft.setTextColor(ResourcesCompat.getColor(getResources(), R.color.purple_200, getTheme()));
            mBinding.tiltBottom.setTypeface(ResourcesCompat.getFont(getContext(), R.font.bauhauscheavy));
            mBinding.tiltBottom.setTextColor(ResourcesCompat.getColor(getResources(), R.color.purple_200, getTheme()));
            mBinding.tiltRight.setTypeface(ResourcesCompat.getFont(getContext(), R.font.bauhauscheavy));
            mBinding.tiltRight.setTextColor(ResourcesCompat.getColor(getResources(), R.color.purple_200, getTheme()));
            mBinding.left.setTypeface(ResourcesCompat.getFont(getContext(), R.font.bauhauscheavy));
            mBinding.left.setTextColor(ResourcesCompat.getColor(getResources(), R.color.purple_200, getTheme()));
            mBinding.right.setTypeface(ResourcesCompat.getFont(getContext(), R.font.bauhauscheavy));
            mBinding.right.setTextColor(ResourcesCompat.getColor(getResources(), R.color.purple_200, getTheme()));
        } else {
            mBinding.ballstxt.setTypeface(ResourcesCompat.getFont(getContext(), R.font.dosvga));
            mBinding.ballstxt.setTextColor(Color.WHITE);
            mBinding.txtscore.setTypeface(ResourcesCompat.getFont(getContext(), R.font.dosvga));
            mBinding.txtscore.setTextColor(Color.WHITE);
            mBinding.infotxt.setTypeface(ResourcesCompat.getFont(getContext(), R.font.dosvga));
            mBinding.infotxt.setTextColor(Color.WHITE);
            mBinding.missiontxt.setTypeface(ResourcesCompat.getFont(getContext(), R.font.dosvga));
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
        if (player != null) player.pause();
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

    private void putTranslations() {
        int[] ids = getResources().getIntArray(R.array.gametexts_idxs);
        String[] texts = getResources().getStringArray(R.array.gametexts_strings);
        for (int i = 0; i < ids.length; i++) {
            putString(ids[i], texts[i]);
        }
    }

    private native void initNative(String dataPath);

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

    // Override dispatchTouchEvent to intercept touches for light editing
    // Only consume touches when actually dragging a light
    @Override
    public boolean dispatchTouchEvent(MotionEvent event) {
        if (lightEditModeEnabled) {
            int viewportW = getWindow().getDecorView().getWidth();
            int viewportH = getWindow().getDecorView().getHeight();

            float x = event.getX();
            float y = event.getY();

            switch (event.getAction()) {
                case MotionEvent.ACTION_DOWN:
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
        return super.dispatchTouchEvent(event);
    }

    // HDR native methods
    private native void setHDRCapabilities(boolean supported, boolean bt2020, boolean pq, 
                                           boolean scrgb, boolean fp16, float maxNits, float minNits);
    private native boolean isHDRActive();
    private native void setHDREnabled(boolean enabled);

    // Light editor native methods
    private native void setLightEditMode(boolean enabled);
    private native boolean getLightEditMode();
    private native void onLightTouchDown(float screenX, float screenY, int viewportX, int viewportY, int viewportW, int viewportH);
    private native void onLightTouchMove(float screenX, float screenY, int viewportX, int viewportY, int viewportW, int viewportH);
    private native void onLightTouchUp();
    private native boolean saveLightPositions(String filepath);
    private native boolean loadLightPositions(String filepath);
    private native int getSelectedLightIndex();
    private native boolean hasLightSelection();
    private native void setDebugBallPosition(float x, float y);
    private native void enableDebugBall(boolean enabled);
    private native void triggerDemoMode();
}