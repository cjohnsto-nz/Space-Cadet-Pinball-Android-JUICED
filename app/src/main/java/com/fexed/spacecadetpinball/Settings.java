package com.fexed.spacecadetpinball;

import androidx.appcompat.app.AppCompatActivity;

import android.content.Context;
import android.content.Intent;
import android.content.res.Configuration;
import android.net.Uri;
import android.os.Bundle;
import android.os.Looper;
import android.text.Editable;
import android.text.SpannableString;
import android.text.TextWatcher;
import android.text.method.LinkMovementMethod;
import android.text.style.UnderlineSpan;
import android.util.Log;
import android.view.KeyEvent;
import android.view.View;
import android.widget.SeekBar;
import android.widget.TextView;
import android.widget.Toast;

import com.android.volley.NetworkResponse;
import com.android.volley.Request;
import com.android.volley.RequestQueue;
import com.android.volley.Response;
import com.android.volley.toolbox.HttpHeaderParser;
import com.android.volley.toolbox.StringRequest;
import com.android.volley.toolbox.Volley;
import com.fexed.spacecadetpinball.databinding.ActivityMainBinding;
import com.fexed.spacecadetpinball.databinding.ActivitySettingsBinding;

import org.json.JSONException;
import org.json.JSONObject;
import org.libsdl.app.SDLActivity;

import java.security.PublicKey;
import java.util.Random;

public class Settings extends AppCompatActivity {

    private ActivitySettingsBinding mBinding;

    // Load native library for light position saving
    static {
        System.loadLibrary("SpaceCadetPinball");
    }

    private native boolean saveLightPositionsNative(String filepath);
    private native int resetOutOfBoundsLightsNative();
    private native void setLightDebugModeNative(boolean enabled);
    private native void nextLightNative();
    private native void previousLightNative();
    private native void toggleTableLightNative();
    private native void toggleHDRLightNative();
    private native String getCurrentLightInfoNative();
    private native void setParticlesEnabledNative(boolean enabled);

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        mBinding = ActivitySettingsBinding.inflate(getLayoutInflater());
        View view = mBinding.getRoot();
        setContentView(view);
        if (PrefsHelper.getUsername(null) != null) {
            HighScoreHandler.postScore(getApplicationContext(), false);
        }

        int score = PrefsHelper.getHighScore();
        String txt = score + "";
        mBinding.highscoretxtv.setText(txt);

        txt = BuildConfig.VERSION_NAME + " (" + BuildConfig.VERSION_CODE + ")";
        mBinding.verstxtv.setText(txt);

        boolean tiltenabled = PrefsHelper.getTiltButtons();
        mBinding.tiltbtns.setChecked(tiltenabled);
        mBinding.tiltbtns.setOnCheckedChangeListener((compoundButton, b) -> {
            PrefsHelper.setTiltButtons(b);
        });

        boolean customfonts = PrefsHelper.getCustomFonts();
        mBinding.cstmfnts.setChecked(customfonts);
        mBinding.cstmfnts.setOnCheckedChangeListener((compoundButton, b) -> {
            PrefsHelper.setCustomFonts(b);
        });

        boolean plungerpopup = PrefsHelper.getPlungerPopup();
        mBinding.plungerpopup.setChecked(plungerpopup);
        mBinding.plungerpopup.setOnCheckedChangeListener((compoundButton, b) -> {
            PrefsHelper.setPlungerPopup(b);
        });

        boolean remainingballs = PrefsHelper.getRemainingBalls();
        mBinding.remainingballs.setChecked(remainingballs);
        mBinding.remainingballs.setOnCheckedChangeListener((compoundButton, b) -> {
            PrefsHelper.setRemainingBalls(b);
        });

        boolean fullScreenPlunger = PrefsHelper.getFullScreenPlunger();
        mBinding.fullscreenplunger.setChecked(fullScreenPlunger);
        mBinding.fullscreenplunger.setOnCheckedChangeListener((compoundButton, b) -> {
            PrefsHelper.setFullScreenPlunger(b);

            if (!b) {
                PrefsHelper.setShouldShowBottomPlunger(true);
            }
        });

        boolean music = PrefsHelper.getMusic();
        mBinding.music.setChecked(music);
        mBinding.music.setOnCheckedChangeListener((compoundButton, b) -> {
            PrefsHelper.setMusic(b);
        });

        boolean enhancedAudio = PrefsHelper.getEnhancedAudio();
        mBinding.enhancedAudioSwitch.setChecked(enhancedAudio);
        mBinding.enhancedAudioSwitch.setOnCheckedChangeListener((compoundButton, b) -> {
            PrefsHelper.setEnhancedAudio(b);
            Toast.makeText(this, "Restart app to apply enhanced audio", Toast.LENGTH_SHORT).show();
        });

        // HDR Settings
        HDRHelper.HDRCapabilities hdrCaps = HDRHelper.queryHDRCapabilities(this);
        boolean hdrEnabled = PrefsHelper.getHDREnabled();
        mBinding.hdrswitch.setChecked(hdrEnabled);
        mBinding.hdrswitch.setEnabled(hdrCaps.isSupported);
        
        if (hdrCaps.isSupported) {
            String hdrStatus = "HDR supported (max " + (int)hdrCaps.maxLuminanceNits + " nits)";
            mBinding.hdrStatusText.setText(hdrStatus);
        } else {
            mBinding.hdrStatusText.setText("HDR not supported on this device");
            mBinding.hdrswitch.setChecked(false);
        }
        
        mBinding.hdrswitch.setOnCheckedChangeListener((compoundButton, b) -> {
            PrefsHelper.setHDREnabled(b);
        });

        // HDR Max Nits input
        int savedNits = PrefsHelper.getHDRMaxNits();
        if (savedNits > 0) {
            mBinding.hdrNitsInput.setText(String.valueOf(savedNits));
        }
        mBinding.hdrNitsInput.addTextChangedListener(new TextWatcher() {
            @Override
            public void beforeTextChanged(CharSequence s, int start, int count, int after) {}

            @Override
            public void onTextChanged(CharSequence s, int start, int before, int count) {
                String text = s.toString().trim();
                if (text.isEmpty()) {
                    PrefsHelper.setHDRMaxNits(0); // 0 = auto
                } else {
                    try {
                        int nits = Integer.parseInt(text);
                        PrefsHelper.setHDRMaxNits(nits);
                    } catch (NumberFormatException e) {
                        // Ignore invalid input
                    }
                }
            }

            @Override
            public void afterTextChanged(Editable s) {}
        });

        // Glow Intensity SeekBar
        int savedGlow = PrefsHelper.getHDRGlowIntensity();
        mBinding.glowIntensityBar.setProgress(savedGlow);
        mBinding.glowIntensityValue.setText(savedGlow + "%");
        mBinding.glowIntensityBar.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                mBinding.glowIntensityValue.setText(progress + "%");
                if (fromUser) {
                    PrefsHelper.setHDRGlowIntensity(progress);
                }
            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {}

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {}
        });

        // Particles toggle
        boolean particlesEnabled = PrefsHelper.getParticlesEnabled();
        mBinding.particlesSwitch.setChecked(particlesEnabled);
        mBinding.particlesSwitch.setOnCheckedChangeListener((compoundButton, b) -> {
            PrefsHelper.setParticlesEnabled(b);
            setParticlesEnabledNative(b);
        });

        // Beat-reactive glow toggle
        boolean beatReactiveGlow = PrefsHelper.getBeatReactiveGlow();
        mBinding.beatReactiveSwitch.setChecked(beatReactiveGlow);
        mBinding.beatReactiveSwitch.setOnCheckedChangeListener((compoundButton, b) -> {
            PrefsHelper.setBeatReactiveGlow(b);
        });

        // Light editor toggle
        mBinding.lightEditSwitch.setOnCheckedChangeListener((compoundButton, b) -> {
            PrefsHelper.setLightEditMode(b);
            if (b) {
                Toast.makeText(this, "Light Edit Mode ON - go back to game and drag lights", Toast.LENGTH_LONG).show();
            }
        });
        mBinding.lightEditSwitch.setChecked(PrefsHelper.getLightEditMode());

        // Trail Opacity SeekBar
        int savedOpacity = PrefsHelper.getTrailOpacity();
        mBinding.trailOpacityBar.setProgress(savedOpacity);
        mBinding.trailOpacityValue.setText(savedOpacity + "%");
        mBinding.trailOpacityBar.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                mBinding.trailOpacityValue.setText(progress + "%");
                if (fromUser) {
                    PrefsHelper.setTrailOpacity(progress);
                }
            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {}

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {}
        });

        // Trail Lifetime SeekBar (stored as tenths of seconds, 5-100 = 0.5s to 10s)
        int savedLifetime = PrefsHelper.getTrailLifetime();
        mBinding.trailLifetimeBar.setProgress(savedLifetime);
        float lifetimeSeconds = savedLifetime / 10.0f;
        mBinding.trailLifetimeValue.setText(String.format("%.1fs", lifetimeSeconds));
        mBinding.trailLifetimeBar.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                // Minimum 5 (0.5s)
                if (progress < 5) progress = 5;
                float seconds = progress / 10.0f;
                mBinding.trailLifetimeValue.setText(String.format("%.1fs", seconds));
                if (fromUser) {
                    PrefsHelper.setTrailLifetime(progress);
                }
            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {}

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {}
        });

        // Camera Tracking toggle
        boolean cameraTracking = PrefsHelper.getCameraTracking();
        mBinding.cameraTrackingSwitch.setChecked(cameraTracking);
        mBinding.cameraTrackingSwitch.setOnCheckedChangeListener((compoundButton, b) -> {
            PrefsHelper.setCameraTracking(b);
        });

        // Camera Zoom SeekBar (100-400, default 200 = 2x)
        int savedZoom = PrefsHelper.getCameraZoom();
        mBinding.cameraZoomBar.setProgress(savedZoom - 100); // SeekBar is 0-300, representing 100-400
        mBinding.cameraZoomValue.setText(String.format("%.1fx", savedZoom / 100.0f));
        mBinding.cameraZoomBar.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                int zoom = progress + 100; // Convert 0-300 to 100-400
                mBinding.cameraZoomValue.setText(String.format("%.1fx", zoom / 100.0f));
                if (fromUser) {
                    PrefsHelper.setCameraZoom(zoom);
                }
            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {}

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {}
        });

        mBinding.saveLightsBtn.setOnClickListener(v -> {
            String path = getFilesDir().getAbsolutePath() + "/light_positions.cfg";
            if (saveLightPositionsNative(path)) {
                Toast.makeText(this, "Light positions saved!", Toast.LENGTH_SHORT).show();
            } else {
                Toast.makeText(this, "Failed to save light positions", Toast.LENGTH_SHORT).show();
            }
        });
        
        mBinding.resetLightsBtn.setOnClickListener(v -> {
            int count = resetOutOfBoundsLightsNative();
            if (count > 0) {
                Toast.makeText(this, "Reset " + count + " out-of-bounds lights", Toast.LENGTH_SHORT).show();
            } else {
                Toast.makeText(this, "All lights are within bounds", Toast.LENGTH_SHORT).show();
            }
        });

        // Light Debug Mode
        mBinding.lightDebugSwitch.setOnCheckedChangeListener((compoundButton, b) -> {
            setLightDebugModeNative(b);
            if (b) {
                updateCurrentLightInfo();
                Toast.makeText(this, "Light Debug Mode ON - use buttons to test lights", Toast.LENGTH_LONG).show();
            } else {
                mBinding.currentLightText.setText("Current: None");
            }
        });
        
        mBinding.prevLightBtn.setOnClickListener(v -> {
            previousLightNative();
            updateCurrentLightInfo();
        });
        
        mBinding.nextLightBtn.setOnClickListener(v -> {
            nextLightNative();
            updateCurrentLightInfo();
        });
        
        mBinding.toggleTableLightBtn.setOnClickListener(v -> {
            toggleTableLightNative();
        });
        
        mBinding.toggleHDRLightBtn.setOnClickListener(v -> {
            toggleHDRLightNative();
        });

        mBinding.inpttxtusername.setText(PrefsHelper.getUsername("Player 1"));
        mBinding.inpttxtusername.addTextChangedListener(new TextWatcher() {
            @Override
            public void beforeTextChanged(CharSequence charSequence, int i, int i1, int i2) {

            }

            @Override
            public void onTextChanged(CharSequence charSequence, int i, int i1, int i2) {
                PrefsHelper.setUsername(charSequence.toString());
            }

            @Override
            public void afterTextChanged(Editable editable) {

            }
        });

        mBinding.gplaytxtv.setOnClickListener(v -> {
            Intent browserIntent = new Intent(Intent.ACTION_VIEW, Uri.parse("https://play.google.com/store/apps/dev?id=6687966458279653723"));
            startActivity(browserIntent);
        });
        mBinding.githubtxtv.setOnClickListener(v -> {
            Intent browserIntent = new Intent(Intent.ACTION_VIEW, Uri.parse("https://github.com/fexed/Pinball-on-Android/releases/latest"));
            startActivity(browserIntent);
        });

        mBinding.volumebar.setProgress(PrefsHelper.getVolume());
        mBinding.volumebar.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            @Override
            public void onProgressChanged(SeekBar seekBar, int percentage, boolean b) {
                PrefsHelper.setVolume(percentage);
            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {
            }

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {
            }
        });

        // Music Volume slider
        mBinding.musicVolumebar.setProgress(PrefsHelper.getMusicVolume());
        mBinding.musicVolumebar.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            @Override
            public void onProgressChanged(SeekBar seekBar, int percentage, boolean b) {
                PrefsHelper.setMusicVolume(percentage);
            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {
            }

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {
            }
        });

        if (PrefsHelper.getCheatsUsed()) {
            mBinding.cheatindicatorlbl.setText(R.string.cheat_used);
            mBinding.cheatAlertSttngs.setVisibility(View.VISIBLE);
        }
        else {
            mBinding.cheatindicatorlbl.setText(R.string.cheat_notused);
            mBinding.cheatAlertSttngs.setVisibility(View.INVISIBLE);
        }

        mBinding.gmaxbtn.setOnClickListener(v -> {
            SDLActivity.onNativeKeyDown(KeyEvent.KEYCODE_G);
            SDLActivity.onNativeKeyUp(KeyEvent.KEYCODE_G);
            SDLActivity.onNativeKeyDown(KeyEvent.KEYCODE_M);
            SDLActivity.onNativeKeyUp(KeyEvent.KEYCODE_M);
            SDLActivity.onNativeKeyDown(KeyEvent.KEYCODE_A);
            SDLActivity.onNativeKeyUp(KeyEvent.KEYCODE_A);
            SDLActivity.onNativeKeyDown(KeyEvent.KEYCODE_X);
            SDLActivity.onNativeKeyUp(KeyEvent.KEYCODE_X);
        });

        mBinding.rmaxbtn.setOnClickListener(v -> {
            SDLActivity.onNativeKeyDown(KeyEvent.KEYCODE_R);
            SDLActivity.onNativeKeyUp(KeyEvent.KEYCODE_R);
            SDLActivity.onNativeKeyDown(KeyEvent.KEYCODE_M);
            SDLActivity.onNativeKeyUp(KeyEvent.KEYCODE_M);
            SDLActivity.onNativeKeyDown(KeyEvent.KEYCODE_A);
            SDLActivity.onNativeKeyUp(KeyEvent.KEYCODE_A);
            SDLActivity.onNativeKeyDown(KeyEvent.KEYCODE_X);
            SDLActivity.onNativeKeyUp(KeyEvent.KEYCODE_X);
        });

        mBinding.onemaxbtn.setOnClickListener(v -> {
            SDLActivity.onNativeKeyDown(KeyEvent.KEYCODE_1);
            SDLActivity.onNativeKeyUp(KeyEvent.KEYCODE_1);
            SDLActivity.onNativeKeyDown(KeyEvent.KEYCODE_M);
            SDLActivity.onNativeKeyUp(KeyEvent.KEYCODE_M);
            SDLActivity.onNativeKeyDown(KeyEvent.KEYCODE_A);
            SDLActivity.onNativeKeyUp(KeyEvent.KEYCODE_A);
            SDLActivity.onNativeKeyDown(KeyEvent.KEYCODE_X);
            SDLActivity.onNativeKeyUp(KeyEvent.KEYCODE_X);
        });

        mBinding.bmaxbtn.setOnClickListener(v -> {
            SDLActivity.onNativeKeyDown(KeyEvent.KEYCODE_B);
            SDLActivity.onNativeKeyUp(KeyEvent.KEYCODE_B);
            SDLActivity.onNativeKeyDown(KeyEvent.KEYCODE_M);
            SDLActivity.onNativeKeyUp(KeyEvent.KEYCODE_M);
            SDLActivity.onNativeKeyDown(KeyEvent.KEYCODE_A);
            SDLActivity.onNativeKeyUp(KeyEvent.KEYCODE_A);
            SDLActivity.onNativeKeyDown(KeyEvent.KEYCODE_X);
            SDLActivity.onNativeKeyUp(KeyEvent.KEYCODE_X);
        });

        mBinding.omaxbtn.setOnClickListener(v -> {
            SDLActivity.onNativeKeyDown(KeyEvent.KEYCODE_O);
            SDLActivity.onNativeKeyUp(KeyEvent.KEYCODE_O);
            SDLActivity.onNativeKeyDown(KeyEvent.KEYCODE_M);
            SDLActivity.onNativeKeyUp(KeyEvent.KEYCODE_M);
            SDLActivity.onNativeKeyDown(KeyEvent.KEYCODE_A);
            SDLActivity.onNativeKeyUp(KeyEvent.KEYCODE_A);
            SDLActivity.onNativeKeyDown(KeyEvent.KEYCODE_X);
            SDLActivity.onNativeKeyUp(KeyEvent.KEYCODE_X);
        });

        mBinding.lmaxbtn.setOnClickListener(v -> {
            SDLActivity.onNativeKeyDown(KeyEvent.KEYCODE_L);
            SDLActivity.onNativeKeyUp(KeyEvent.KEYCODE_L);
            SDLActivity.onNativeKeyDown(KeyEvent.KEYCODE_M);
            SDLActivity.onNativeKeyUp(KeyEvent.KEYCODE_M);
            SDLActivity.onNativeKeyDown(KeyEvent.KEYCODE_A);
            SDLActivity.onNativeKeyUp(KeyEvent.KEYCODE_A);
            SDLActivity.onNativeKeyDown(KeyEvent.KEYCODE_X);
            SDLActivity.onNativeKeyUp(KeyEvent.KEYCODE_X);
        });

        mBinding.hdntestbtn.setOnClickListener(v -> {
            SDLActivity.onNativeKeyDown(KeyEvent.KEYCODE_H);
            SDLActivity.onNativeKeyUp(KeyEvent.KEYCODE_H);
            SDLActivity.onNativeKeyDown(KeyEvent.KEYCODE_I);
            SDLActivity.onNativeKeyUp(KeyEvent.KEYCODE_I);
            SDLActivity.onNativeKeyDown(KeyEvent.KEYCODE_D);
            SDLActivity.onNativeKeyUp(KeyEvent.KEYCODE_D);
            SDLActivity.onNativeKeyDown(KeyEvent.KEYCODE_D);
            SDLActivity.onNativeKeyUp(KeyEvent.KEYCODE_D);
            SDLActivity.onNativeKeyDown(KeyEvent.KEYCODE_E);
            SDLActivity.onNativeKeyUp(KeyEvent.KEYCODE_E);
            SDLActivity.onNativeKeyDown(KeyEvent.KEYCODE_N);
            SDLActivity.onNativeKeyUp(KeyEvent.KEYCODE_N);
            SDLActivity.onNativeKeyDown(KeyEvent.KEYCODE_SPACE);
            SDLActivity.onNativeKeyUp(KeyEvent.KEYCODE_SPACE);
            SDLActivity.onNativeKeyDown(KeyEvent.KEYCODE_T);
            SDLActivity.onNativeKeyUp(KeyEvent.KEYCODE_T);
            SDLActivity.onNativeKeyDown(KeyEvent.KEYCODE_E);
            SDLActivity.onNativeKeyUp(KeyEvent.KEYCODE_E);
            SDLActivity.onNativeKeyDown(KeyEvent.KEYCODE_S);
            SDLActivity.onNativeKeyUp(KeyEvent.KEYCODE_S);
            SDLActivity.onNativeKeyDown(KeyEvent.KEYCODE_T);
            SDLActivity.onNativeKeyUp(KeyEvent.KEYCODE_T);
        });

        mBinding.rankingbtn.setOnClickListener(v -> {
            LeaderboardActivity.isCheatRanking = false;
            Intent i = new Intent(this, LeaderboardActivity.class);
            startActivity(i);
        });

        mBinding.cheatrankingbtn.setOnClickListener(v -> {
            LeaderboardActivity.isCheatRanking = true;
            Intent i = new Intent(this, LeaderboardActivity.class);
            startActivity(i);
        });

        checkLatestRelease();
    }

    public void checkLatestRelease() {
        String URL = "https://api.github.com/repos/fexed/Pinball-on-Android/releases/latest";
        Response.Listener<String> listener = response -> {};
        Response.ErrorListener errorListener = response -> {};
        StringRequest GETReleaseRequest = new StringRequest(Request.Method.GET, URL, listener, errorListener) {
            @Override
            protected Response<String> parseNetworkResponse(NetworkResponse response) {
                String responseJSON = "";
                if (response != null) {
                    try {
                        JSONObject received = new JSONObject(new String(response.data));
                        String tag = received.getString("tag_name");Log.d("VERS", "current: " + BuildConfig.VERSION_NAME + " - GitHub: " + tag);
                        String downloadurl = received.getJSONArray("assets").getJSONObject(0).getString("browser_download_url");
                        Log.d("VERS", "current: " + BuildConfig.VERSION_NAME + " - GitHub: " + tag);

                        if (!tag.equals(BuildConfig.VERSION_NAME)) {
                            runOnUiThread(() -> {
                                SpannableString content = new SpannableString(getString(R.string.newversdl, tag));
                                content.setSpan(new UnderlineSpan(), 0, getString(R.string.newversdl, tag).length(), 0);
                                mBinding.githubtxtv.setText(content);
                                mBinding.githubtxtv.setOnClickListener(v -> {
                                    Intent browserIntent = new Intent(Intent.ACTION_VIEW, Uri.parse(downloadurl));
                                    startActivity(browserIntent);
                                });
                                Toast.makeText(Settings.this, getString(R.string.newvers, tag), Toast.LENGTH_LONG).show();
                            });
                        } else {
                            runOnUiThread(() -> {
                                SpannableString content = new SpannableString(getString(R.string.nonewvers));
                                content.setSpan(new UnderlineSpan(), 0, getString(R.string.nonewvers).length(), 0);
                                mBinding.githubtxtv.setText(content);
                            });
                        }
                    } catch (JSONException e) {
                        e.printStackTrace();
                    }
                }
                return Response.success(responseJSON, HttpHeaderParser.parseCacheHeaders(response));
            }
        };
        RequestQueue queue = Volley.newRequestQueue(getApplicationContext());
        queue.add(GETReleaseRequest);
    }
    
    private void updateCurrentLightInfo() {
        String lightInfo = getCurrentLightInfoNative();
        if (lightInfo != null && !lightInfo.isEmpty()) {
            mBinding.currentLightText.setText("Current: " + lightInfo);
        } else {
            mBinding.currentLightText.setText("Current: None");
        }
    }
}