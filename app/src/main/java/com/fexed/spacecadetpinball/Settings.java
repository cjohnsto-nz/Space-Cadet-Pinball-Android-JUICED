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
import org.w3c.dom.Text;

import java.security.PublicKey;
import java.util.Random;

public class Settings extends AppCompatActivity {

    private ActivitySettingsBinding mBinding;

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
}