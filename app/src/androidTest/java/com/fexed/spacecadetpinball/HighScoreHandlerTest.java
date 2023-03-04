package com.fexed.spacecadetpinball;

import static org.junit.jupiter.api.Assertions.*;

import android.content.Context;

import androidx.test.core.app.ApplicationProvider;
import androidx.test.filters.SmallTest;
import androidx.test.runner.AndroidJUnit4;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;

@RunWith(AndroidJUnit4.class)
@SmallTest
public class HighScoreHandlerTest {

    public Context context;

    @Before
    public void setUp() {
        context = ApplicationProvider.getApplicationContext();
        PrefsHelper.setPrefs(context.getSharedPreferences("com.fexed.spacecadetpinball", Context.MODE_PRIVATE));
        PrefsHelper.setUsername("unittest_user");
        PrefsHelper.setHighScore(1);
        PrefsHelper.setCheatsUsed(false);
    }

    @After
    public void tearDown() {
        context = null;
    }

    @Test
    public void postHighScore() {
        System.out.println("Posting high score of 2");
        assertTrue(HighScoreHandler.postHighScore(context, 2));
        System.out.println("Posting another high score of 2");
        assertFalse(HighScoreHandler.postHighScore(context, 2));
    }
}