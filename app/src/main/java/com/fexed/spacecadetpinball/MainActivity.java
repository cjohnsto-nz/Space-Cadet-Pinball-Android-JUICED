package com.fexed.spacecadetpinball;

import android.content.Context;
import android.content.Intent;
import android.content.res.AssetManager;
import android.content.res.Configuration;
import android.graphics.Color;
import android.graphics.Typeface;
import android.os.Bundle;
import android.os.Handler;
import android.os.Looper;
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

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        File filesDir = getFilesDir();
        copyAssets(filesDir);
        initNative(filesDir.getAbsolutePath() + "/");

        mBinding = ActivityMainBinding.inflate(getLayoutInflater(), mLayout, false);

        RelativeLayout.LayoutParams layoutParams = new RelativeLayout.LayoutParams(ViewGroup.LayoutParams.MATCH_PARENT, ViewGroup.LayoutParams.MATCH_PARENT);
        layoutParams.addRule(RelativeLayout.CENTER_IN_PARENT, RelativeLayout.TRUE);
        mLayout.addView(mBinding.getRoot(), layoutParams);

        mBinding.getRoot().bringToFront();

        bottomSheetBehavior = BottomSheetBehavior.from(mBinding.bottomsheet);
        bottomSheetBehavior.setState(BottomSheetBehavior.STATE_HIDDEN);

        mBinding.left.setOnTouchListener((v1, event) -> {
            v1.performClick();
            if (event.getAction() == MotionEvent.ACTION_DOWN) {
                SDLActivity.onNativeKeyDown(KeyEvent.KEYCODE_Z);
            }
            if (event.getAction() == MotionEvent.ACTION_UP) {
                SDLActivity.onNativeKeyUp(KeyEvent.KEYCODE_Z);
            }
            return false;
        });


        mBinding.right.setOnTouchListener((v1, event) -> {
            v1.performClick();
            if (event.getAction() == MotionEvent.ACTION_DOWN) {
                SDLActivity.onNativeKeyDown(KeyEvent.KEYCODE_SLASH);
            }
            if (event.getAction() == MotionEvent.ACTION_UP) {
                SDLActivity.onNativeKeyUp(KeyEvent.KEYCODE_SLASH);
            }
            return false;
        });


        mBinding.plunger.setOnTouchListener((v1, event) -> {
            v1.performClick();
            if (event.getAction() == MotionEvent.ACTION_DOWN) {
                SDLActivity.onNativeKeyDown(KeyEvent.KEYCODE_SPACE);
            }
            if (event.getAction() == MotionEvent.ACTION_UP) {
                SDLActivity.onNativeKeyUp(KeyEvent.KEYCODE_SPACE);
            }
            return false;
        });


        mBinding.bottomPlunger.setOnTouchListener((v1, event) -> {
            v1.performClick();
            if (event.getAction() == MotionEvent.ACTION_DOWN) {
                SDLActivity.onNativeKeyDown(KeyEvent.KEYCODE_SPACE);
            }
            if (event.getAction() == MotionEvent.ACTION_UP) {
                SDLActivity.onNativeKeyUp(KeyEvent.KEYCODE_SPACE);
            }
            return false;
        });

        mBinding.replay.setOnLongClickListener(view -> {
            SDLActivity.onNativeKeyDown(KeyEvent.KEYCODE_F2);
            SDLActivity.onNativeKeyUp(KeyEvent.KEYCODE_F2);
            getSharedPreferences("com.fexed.spacecadetpinball", Context.MODE_PRIVATE).edit().putBoolean("cheatsused", false).apply();
            mBinding.cheatAlert.setVisibility(View.INVISIBLE);
            return true;
        });

        mBinding.playpause.setOnClickListener(view -> {
            if (isPlaying) {
                isPlaying = false;
                pauseNativeThread();
                mBinding.playpause.setImageDrawable(getContext().getResources().getDrawable(R.drawable.play));
            } else {
                isPlaying = true;
                resumeNativeThread();
                mBinding.playpause.setImageDrawable(getContext().getResources().getDrawable(R.drawable.pause));

            }
        });

        mBinding.replay.setOnClickListener(view -> {
            Toast.makeText(getContext(), R.string.restartprompt, Toast.LENGTH_SHORT).show();
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
            Intent i = new Intent(this, Settings.class);
            startActivity(i);
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

    private void setFullscreen() {
        int ui_Options = View.SYSTEM_UI_FLAG_LAYOUT_STABLE
                | View.SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION
                | View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN
                | View.SYSTEM_UI_FLAG_HIDE_NAVIGATION
                | View.SYSTEM_UI_FLAG_FULLSCREEN
                | View.SYSTEM_UI_FLAG_IMMERSIVE_STICKY;
        getWindow().getDecorView().setSystemUiVisibility(ui_Options);
    }

    private StateHelper.IStateListener mStateListener = new StateHelper.IStateListener() {
        @Override
        public void onStateChanged(int state) {
            //runOnUiThread(() -> mBinding.replay.setVisibility(state == GameState.RUNNING ? View.GONE : View.VISIBLE));
            setVolume(getSharedPreferences("com.fexed.spacecadetpinball", Context.MODE_PRIVATE).getInt("volume", 100));
            putTranslations();
            putString(26, getSharedPreferences("com.fexed.spacecadetpinball", Context.MODE_PRIVATE).getString("username", "Player 1"));

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
            boolean fullScreenPlunger = getSharedPreferences("com.fexed.spacecadetpinball", Context.MODE_PRIVATE).getBoolean("fullscreenplunger", true);
            if (fullScreenPlunger) {
                runOnUiThread(() -> mBinding.plunger.setVisibility(isBallInPlunger ? View.VISIBLE : View.INVISIBLE));
                if (isBallInPlunger) {
                    plungerTimer = new Handler(Looper.getMainLooper());
                    plungerTimer.postDelayed(() -> runOnUiThread(() -> {
                        if (getSharedPreferences("com.fexed.spacecadetpinball", Context.MODE_PRIVATE).getBoolean("plungerPopup", true)) {
                            Toast.makeText(getContext(), R.string.plungerhint, Toast.LENGTH_LONG).show();
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
            return HighScoreHandler.getHighScore(getContext());
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
            String str = "" + score;
            runOnUiThread(() -> mBinding.txtscore.setText(str));
        }

        @Override
        public void onBallCountUpdated(int count) {
            ballCount = count;
            if (!getSharedPreferences("com.fexed.spacecadetpinball", Context.MODE_PRIVATE).getBoolean("remainingballs", false)) {
                String str = getString(R.string.balls, count);
                runOnUiThread(() -> mBinding.ballstxt.setText(str));
            }
        }

        @Override
        public void onCheatsUsed() {
            getSharedPreferences("com.fexed.spacecadetpinball", Context.MODE_PRIVATE).edit().putBoolean("cheatsused", true).apply();
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
            if (getSharedPreferences("com.fexed.spacecadetpinball", Context.MODE_PRIVATE).getBoolean("remainingballs", false)) {
                String str = getString(R.string.remainingballs, balls);
                runOnUiThread(() -> mBinding.ballstxt.setText(str));
            }
        }
    };

    @Override
    protected void onResume() {
        super.onResume();
        if (!isPlaying) pauseNativeThread();
        setFullscreen();
        StateHelper.INSTANCE.addListener(mStateListener);

        boolean tiltenabled = getSharedPreferences("com.fexed.spacecadetpinball", Context.MODE_PRIVATE).getBoolean("tiltbuttons", true);

        if (tiltenabled) {
            mBinding.tiltLeft.setVisibility(View.VISIBLE);
            mBinding.tiltRight.setVisibility(View.VISIBLE);
            mBinding.tiltBottom.setVisibility(View.VISIBLE);
        } else {
            mBinding.tiltLeft.setVisibility(View.GONE);
            mBinding.tiltRight.setVisibility(View.GONE);
            mBinding.tiltBottom.setVisibility(View.GONE);
        }

        if (!getSharedPreferences("com.fexed.spacecadetpinball", Context.MODE_PRIVATE).getBoolean("remainingballs", false)) {
            String str = getString(R.string.balls, ballCount);
            mBinding.ballstxt.setText(str);
        } else {
            String str = getString(R.string.remainingballs, remainingBalls);
            mBinding.ballstxt.setText(str);
        }

        if (getSharedPreferences("com.fexed.spacecadetpinball", Context.MODE_PRIVATE).getBoolean("shouldshowbottomplunger", false)) {
            getSharedPreferences("com.fexed.spacecadetpinball", Context.MODE_PRIVATE).edit().putBoolean("shouldshowbottomplunger", false).apply();
            bottomSheetBehavior.setState(BottomSheetBehavior.STATE_EXPANDED);
        }

        boolean customfonts = getSharedPreferences("com.fexed.spacecadetpinball", Context.MODE_PRIVATE).getBoolean("customfonts", true);

        if (customfonts) {
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

        if (isGameReady) setVolume(getSharedPreferences("com.fexed.spacecadetpinball", Context.MODE_PRIVATE).getInt("volume", 100));
        getSharedPreferences("com.fexed.spacecadetpinball", Context.MODE_PRIVATE).edit().putBoolean("cheatsused", checkCheatsUsed()).apply();
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
}