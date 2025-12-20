#ifndef SPACECADETPINBALLJNI_H
#define SPACECADETPINBALLJNI_H

#include <string>

class SpaceCadetPinballJNI {
public:
    static void show_error_dialog(std::string title, std::string message);

    static void notifyGameState(int state);

    static void setBallInPlunger(bool state);
    
    static bool isBallInPlunger();

    static void addHighScore(int score);

    static int getHighScore();

    static void displayText(const char* text, int type);

    static void clearText(int type);

    static void postScore(int score);

    static void postBallCount(int count);

    enum GAMESTATE {
        RUNNING = 1,
        FINISHED = 2
    };

    static void cheatsUsed();

    static void gameReady();

    static void postRemainingBalls(int balls);

    static void triggerHapticFeedback(float intensity);

    // HDR support
    static bool queryHDRSupport();
    static float getMaxDisplayLuminance();
    static void setHDRCapabilities(bool supported, bool bt2020, bool pq, bool scrgb, 
                                   bool fp16, float maxNits, float minNits);
};

#endif // SPACECADETPINBALLJNI_H
