package com.fexed.spacecadetpinball;

import static com.fexed.spacecadetpinball.PrefsHelper.*;
import android.content.Context;
import android.content.SharedPreferences;
import android.os.Looper;
import android.util.Base64;
import android.util.Log;
import android.widget.Toast;

import com.android.volley.DefaultRetryPolicy;
import com.android.volley.NetworkResponse;
import com.android.volley.Request;
import com.android.volley.RequestQueue;
import com.android.volley.Response;
import com.android.volley.toolbox.HttpHeaderParser;
import com.android.volley.toolbox.StringRequest;
import com.android.volley.toolbox.Volley;

import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;

import java.nio.charset.StandardCharsets;
import java.security.InvalidKeyException;
import java.security.KeyFactory;
import java.security.NoSuchAlgorithmException;
import java.security.interfaces.RSAPublicKey;
import java.security.spec.X509EncodedKeySpec;
import java.text.ParseException;
import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Date;
import java.util.List;
import java.util.Locale;
import java.util.TimeZone;

import javax.crypto.BadPaddingException;
import javax.crypto.Cipher;
import javax.crypto.IllegalBlockSizeException;
import javax.crypto.NoSuchPaddingException;

public class HighScoreHandler {
    static String PUBLIC_KEY = "MIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEApfiCxaNxZR3rDk19rHUt1kZLBUn00pPhwMe6Fq3zDAhiyzerqA3uYmmPYKHqUUIryBT06OPLpClFKmh2j/8d26OAG2BSKr097ohGVkcZK/lZSBq7T2yWzNB9O09bYscctR5MICww7AKptUfoV+6Uflxvt0RT9WliIGpFwZBGgmfzoU/MjJh56NcMcJ0oyxN8ID078Pi7t9Y70dY7EW/Ux1JzBx3kRfcLRXBpxG+lpV+Mg+e3F3wKtqtLP6rZzYZfPmAkqkVHwCde9cv3VIVq2iq23CpD9/eczkrM2vdvWCU+AcEK14yJiaY8VMQq5AL51+xleojyyJgg7RTQYeHocwIDAQAB";
    static String URL = "https://yga9qquubj.execute-api.us-east-1.amazonaws.com/";
    static LeaderboardActivity leaderboardActivity;
    static boolean postHighScore(Context context, int score) {
        boolean updated = false;
        Log.d("RANKS", PrefsHelper.getCheatsUsed() + " " + score);

        if (!PrefsHelper.getCheatsUsed()) {
            int oldscore = PrefsHelper.getHighScore();
            if (score > oldscore) {
                PrefsHelper.setHighScore(score);
                updated = true;
            }
        }
        
        if (PrefsHelper.getHighScore() > 0) postScore(context, true);

        return updated;
    }

    static RSAPublicKey getPublicKey(Context context) {
        RSAPublicKey publicKey = null;
        try {
            byte[] b = Base64.decode(PUBLIC_KEY, Base64.DEFAULT);
            X509EncodedKeySpec keySpec = new X509EncodedKeySpec(b);
            KeyFactory keyFactory = KeyFactory.getInstance("RSA");
            publicKey = (RSAPublicKey) keyFactory.generatePublic(keySpec);
        } catch (Exception e) {
            Log.e("KEY", e.getLocalizedMessage());
            e.printStackTrace();
        }

        Log.d("KEY", publicKey.toString());
        return publicKey;
    }

    static String encode(byte[] toEncode, RSAPublicKey publicKey) {
        try {
            Cipher cipher = Cipher.getInstance("RSA/ECB/PKCS1Padding");
            cipher.init(Cipher.ENCRYPT_MODE, publicKey);
            byte[] encrypted = cipher.doFinal(toEncode);
            return Base64.encodeToString(encrypted, Base64.DEFAULT);
        } catch (NoSuchAlgorithmException | BadPaddingException | IllegalBlockSizeException | InvalidKeyException | NoSuchPaddingException e) {
            e.printStackTrace();
        }
        return Base64.encodeToString(toEncode, Base64.DEFAULT);
    }

    static void getRanking(Context context) {
        List<LeaderboardElement> corpus = new ArrayList<>();
        Response.Listener<String> listener = response -> {
            try {
                JSONArray rankingJSON = new JSONArray(response);
                SimpleDateFormat fmt = new SimpleDateFormat("yyyy-MM-dd'T'HH:mm:ss.SSS'Z'", Locale.UK);
                fmt.setTimeZone(TimeZone.getTimeZone("UTC"));
                for (int i = 0; i < rankingJSON.length(); i++) {
                    JSONObject elem = rankingJSON.getJSONObject(i);
                    Date lastupdate = fmt.parse(elem.getString("updatedAt"));
                    LeaderboardElement player = new LeaderboardElement(elem.getString("username"), elem.getString("_id"), lastupdate, elem.getJSONObject("rank").getInt("score"), elem.getJSONObject("cheatRank").getInt("score"));
                    if (player.normalScore > 0) corpus.add(player);
                }
            } catch (JSONException | ParseException e) {
                e.printStackTrace();
            }
            if (leaderboardActivity != null) leaderboardActivity.onLeaderboardReady(corpus);
        };
        Response.ErrorListener errorListener = error -> {
            Log.e("RANKS", "error: " + error + " " + Arrays.toString(error.getStackTrace()));
            if (leaderboardActivity != null) leaderboardActivity.onLeaderboardError(error.getMessage());
        };
        StringRequest GETRankingRequest = new StringRequest(Request.Method.GET, URL + "dev/ranks/", listener, errorListener);
        RequestQueue queue = Volley.newRequestQueue(context);
        queue.add(GETRankingRequest);
    }


    static void getCurrentRank(Context context) {
        Response.Listener<String> listener = response -> {
            try {
                SimpleDateFormat fmt = new SimpleDateFormat("yyyy-MM-dd'T'HH:mm:ss.SSS'Z'", Locale.UK);
                fmt.setTimeZone(TimeZone.getTimeZone("UTC"));
                JSONObject elem = new JSONObject(response);
                Date lastupdate = fmt.parse(elem.getString("updatedAt"));
                LeaderboardElement player = new LeaderboardElement(elem.getString("username"), elem.getString("_id"), lastupdate, elem.getJSONObject("rank").getInt("score"), elem.getJSONObject("cheatRank").getInt("score"));
                if (leaderboardActivity != null) leaderboardActivity.onCurrentRankReady(player);
            } catch (JSONException | ParseException e) {
                e.printStackTrace();
            }
        };
        Response.ErrorListener errorListener = error -> {
            Log.e("RANKS", "error: " + error + " " + Arrays.toString(error.getStackTrace()));
        };
        StringRequest GETRankingRequest = new StringRequest(Request.Method.GET, URL + "dev/ranks/" + PrefsHelper.getUserId(), listener, errorListener);
        RequestQueue queue = Volley.newRequestQueue(context);
        queue.add(GETRankingRequest);
    }

    static void postScore(Context context, boolean verbose) {
        if (!PrefsHelper.getUsername("").equals("")) {
            try {
                JSONObject objectToEncode = new JSONObject();
                objectToEncode.put("id", PrefsHelper.getUserId());
                objectToEncode.put("username", PrefsHelper.getUsername(""));
                objectToEncode.put("score", PrefsHelper.getHighScore());
                objectToEncode.put("ranking", 0);
                Log.d("RANKS", "toEncode: " + objectToEncode.toString().trim());

                RSAPublicKey key = getPublicKey(context);
                String encryptedBody = encode(objectToEncode.toString().trim().replace("\n", "").getBytes(StandardCharsets.UTF_8), key);

                JSONObject objectToSend = new JSONObject();
                objectToSend.put("value", encryptedBody.trim());
                Log.d("RANKS", "toSend: " + objectToSend);

                Response.Listener<String> listener = response -> {};
                Response.ErrorListener errorListener = error -> Log.e("RANKS", "error: " + error + " " + Arrays.toString(error.getStackTrace()));
                StringRequest POSTRankingRequest = new StringRequest(Request.Method.POST, URL + "dev/ranks/", listener, errorListener) {
                    @Override
                    protected Response<String> parseNetworkResponse(NetworkResponse response) {
                        String responseJSON = "";
                        if (response != null) {
                            try {
                                JSONObject received = new JSONObject(new String(response.data));
                                String uid = received.getString("_id");
                                String nickname = received.getString("username");
                                int scoreNormal = received.getJSONObject("rank").getInt("score");
                                int scoreCheat = received.getJSONObject("cheatRank").getInt("score");
                                if (PrefsHelper.getUserId().equals("0")) PrefsHelper.setUserId(uid);
                                Log.d("RANKS", "response: " + received.toString());

                                Looper.prepare();
                                if (verbose) {
                                    if (response.statusCode == 200) {
                                        Toast.makeText(context, context.getString(R.string.scoreupload_ok, nickname), Toast.LENGTH_SHORT).show();
                                    } else {
                                        Toast.makeText(context, context.getString(R.string.scoreupload_ok, "" + response.statusCode), Toast.LENGTH_SHORT).show();
                                    }
                                }
                            } catch (JSONException e) {
                                e.printStackTrace();
                            }
                        }
                        return Response.success(responseJSON, HttpHeaderParser.parseCacheHeaders(response));
                    }

                    @Override
                    public String getBodyContentType() {
                        return "application/json; charset=utf-8";
                    }

                    @Override
                    public byte[] getBody() {
                        return objectToSend.toString().getBytes(StandardCharsets.UTF_8);
                    }
                };
                POSTRankingRequest.setRetryPolicy(new DefaultRetryPolicy(60000, 10, 2.0f));
                RequestQueue queue = Volley.newRequestQueue(context);
                queue.add(POSTRankingRequest);
            } catch (JSONException e) {
                e.printStackTrace();
            }
        }
    }
}
