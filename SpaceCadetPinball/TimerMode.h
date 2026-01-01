#pragma once

// Timer Mode - A time-based game mode where:
// - Player starts with 3 minutes
// - Score is hidden, instead adds to timer (10s per 50,000 points)
// - Unlimited balls, but crashes (sink without replay/extra ball) remove 30 seconds
// - When timer expires, game goes into TILT and ends on final sink

class TimerMode
{
public:
    // Game mode types
    enum class Mode
    {
        Classic = 0,
        Timer = 1
    };

    // Initialize timer mode system
    static void Init();

    // Set the current game mode
    static void SetMode(Mode mode);
    static Mode GetMode() { return s_currentMode; }
    static bool IsTimerMode() { return s_currentMode == Mode::Timer; }

    // Timer control
    static void StartTimer();  // Start with 3 minutes
    static void StopTimer();
    static void PauseTimer();
    static void ResumeTimer();
    static void Update(float deltaTime);  // Called each frame

    // Get remaining time
    static int GetRemainingTimeMs();
    static float GetRemainingTime();

    // Add time based on score (10s per 50,000 points)
    static void OnScoreAdded(int scoreAdded);

    // Penalty for ball crash (no replay/extra ball)
    // halfPenalty: true if grace timer was active (half the penalty)
    static void OnBallCrash(bool halfPenalty = false);

    // Check if timer has expired
    static bool HasExpired() { return s_timerExpired; }

    // Check if waiting for final sink after timer expired
    static bool IsWaitingForFinalSink() { return s_waitingForFinalSink; }

    // Called when ball sinks after timer expired - triggers game over
    static void OnFinalSink();

    // Reset for new game
    static void Reset();

    // Score tracking for time bonuses
    static int GetScoreThreshold() { return s_scoreThreshold; }
    static int GetTotalScore() { return s_totalScore; }
    // Get current threshold increment based on rank (100k + 25k per rank)
    static int GetCurrentThresholdIncrement();
    // Get progress to next bonus
    static int GetScoreProgress();

    // Mode selection state
    static bool IsModeSelectionPending() { return s_modeSelectionPending; }
    static void SetModeSelectionPending(bool pending) { s_modeSelectionPending = pending; }

private:
    static Mode s_currentMode;
    static bool s_timerActive;
    static bool s_timerExpired;
    static bool s_waitingForFinalSink;
    static int s_scoreThreshold;         // Next score threshold for time bonus
    static int s_totalScore;             // Accumulated score for threshold tracking
    static bool s_modeSelectionPending;  // Waiting for player to select mode
    
    // Wall-clock based timing for accuracy
    static int64_t s_endTimeMs;          // When timer expires (wall clock ms)
    static int64_t s_bonusTimeMs;        // Accumulated bonus time in ms
    static int64_t s_pauseTimeMs;        // When timer was paused (0 if not paused)
    static int64_t GetCurrentTimeMs();   // Get current wall clock time

    static constexpr int64_t kStartingTimeMs = 180000;    // 3 minutes in ms
    static constexpr int64_t kTimePerThresholdMs = 15000; // 15 seconds in ms
    static constexpr int kScorePerThreshold = 100000;     // 100,000 points
    static constexpr int64_t kCrashPenaltyMs = 30000;     // 30 seconds in ms
};
