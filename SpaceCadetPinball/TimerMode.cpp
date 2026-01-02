#include "pch.h"
#include "TimerMode.h"

#include "pb.h"
#include "TPinballTable.h"
#include "pinball.h"
#include "TTextBox.h"
#include "control.h"
#include "../app/src/main/cpp/SpaceCadetPinballJNI.h"
#include <chrono>

// Static member initialization
TimerMode::Mode TimerMode::s_currentMode = TimerMode::Mode::Classic;
bool TimerMode::s_timerActive = false;
bool TimerMode::s_timerExpired = false;
bool TimerMode::s_waitingForFinalSink = false;
int TimerMode::s_scoreThreshold = 50000;
int TimerMode::s_totalScore = 0;
bool TimerMode::s_modeSelectionPending = false;
int64_t TimerMode::s_endTimeMs = 0;
int64_t TimerMode::s_bonusTimeMs = 0;
int64_t TimerMode::s_pauseTimeMs = 0;

// Session timing statics
int64_t TimerMode::s_sessionStartTimeMs = 0;
int64_t TimerMode::s_sessionPauseTimeMs = 0;
int64_t TimerMode::s_sessionAccumulatedMs = 0;
bool TimerMode::s_sessionActive = false;

int64_t TimerMode::GetCurrentTimeMs()
{
    auto now = std::chrono::steady_clock::now();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch());
    return ms.count();
}

int TimerMode::GetRemainingTimeMs()
{
    if (!s_timerActive || s_timerExpired)
        return 0;
    
    // If paused, calculate remaining from pause time instead of current time
    int64_t currentTime = (s_pauseTimeMs > 0) ? s_pauseTimeMs : GetCurrentTimeMs();
    int64_t remaining = (s_endTimeMs + s_bonusTimeMs) - currentTime;
    return remaining > 0 ? static_cast<int>(remaining) : 0;
}

float TimerMode::GetRemainingTime()
{
    return GetRemainingTimeMs() / 1000.0f;
}

void TimerMode::Init()
{
    s_currentMode = Mode::Classic;
    s_modeSelectionPending = false;
    Reset();
}

void TimerMode::SetMode(Mode mode)
{
    s_currentMode = mode;
    s_modeSelectionPending = false;
}

void TimerMode::StartTimer()
{
    if (s_currentMode != Mode::Timer)
        return;

    s_endTimeMs = GetCurrentTimeMs() + kStartingTimeMs;
    s_bonusTimeMs = 0;
    s_timerActive = true;
    s_timerExpired = false;
    s_waitingForFinalSink = false;
    s_scoreThreshold = kScorePerThreshold;
    s_totalScore = 0;
}

void TimerMode::StopTimer()
{
    s_timerActive = false;
    s_pauseTimeMs = 0;
}

void TimerMode::PauseTimer()
{
    if (s_timerActive && s_pauseTimeMs == 0)
    {
        s_pauseTimeMs = GetCurrentTimeMs();
    }
}

void TimerMode::ResumeTimer()
{
    if (s_timerActive && s_pauseTimeMs > 0)
    {
        // Add the paused duration to the end time
        int64_t pausedDuration = GetCurrentTimeMs() - s_pauseTimeMs;
        s_endTimeMs += pausedDuration;
        s_pauseTimeMs = 0;
    }
}

void TimerMode::Update(float deltaTime)
{
    if (!s_timerActive || s_currentMode != Mode::Timer)
        return;

    if (s_timerExpired)
        return;

    // Check if timer expired using wall clock
    if (GetRemainingTimeMs() <= 0)
    {
        s_timerExpired = true;
        s_waitingForFinalSink = true;

        // Trigger TILT
        if (pb::MainTable)
        {
            pb::MainTable->tilt(0.0f);
        }

        // Display time's up message
        if (pinball::InfoTextBox)
        {
            pinball::InfoTextBox->Display("TIME'S UP!", -1.0, 2);
        }
    }
}

int TimerMode::GetCurrentThresholdIncrement()
{
    // Base threshold is 100k, increases by 25k per rank above 1
    // Rank 1 (Cadet): 100k, Rank 2 (Ensign): 125k, Rank 3: 150k, etc.
    int rank = control::GetPlayerRank();
    return kScorePerThreshold + ((rank - 1) * 25000);
}

int TimerMode::GetScoreProgress()
{
    // Progress within current threshold increment
    int prevThreshold = s_scoreThreshold - GetCurrentThresholdIncrement();
    if (prevThreshold < 0) prevThreshold = 0;
    return s_totalScore - prevThreshold;
}

void TimerMode::OnScoreAdded(int scoreAdded)
{
    if (s_currentMode != Mode::Timer || !s_timerActive || s_timerExpired)
        return;

    s_totalScore += scoreAdded;

    // Check if we crossed threshold(s) - group multiple bonuses
    // Threshold increment is based on current rank (100k + 25k per rank)
    int bonusCount = 0;
    while (s_totalScore >= s_scoreThreshold)
    {
        s_scoreThreshold += GetCurrentThresholdIncrement();
        bonusCount++;
    }
    
    // Add all bonus time at once (exactly 15000ms per bonus)
    if (bonusCount > 0)
    {
        s_bonusTimeMs += bonusCount * kTimePerThresholdMs;
        
        // Notify Java UI with grouped bonus (e.g., +30 for two thresholds)
        int totalBonus = bonusCount * static_cast<int>(kTimePerThresholdMs / 1000);
        SpaceCadetPinballJNI::notifyTimerBonus(totalBonus);
    }
}

void TimerMode::OnBallCrash(bool halfPenalty)
{
    if (s_currentMode != Mode::Timer || !s_timerActive || s_timerExpired)
        return;

    int64_t penalty = halfPenalty ? (kCrashPenaltyMs / 2) : kCrashPenaltyMs;
    s_bonusTimeMs -= penalty;

    // Notify Java UI with penalty (negative value)
    SpaceCadetPinballJNI::notifyTimerBonus(-static_cast<int>(penalty / 1000));

    // Check if timer expired from penalty
    if (GetRemainingTimeMs() <= 0)
    {
        s_timerExpired = true;
        s_waitingForFinalSink = true;

        // Trigger TILT
        if (pb::MainTable)
        {
            pb::MainTable->tilt(0.0f);
        }

        if (pinball::InfoTextBox)
        {
            pinball::InfoTextBox->Display("TIME'S UP!", -1.0, 2);
        }
    }
}

void TimerMode::OnFinalSink()
{
    if (!s_waitingForFinalSink)
        return;

    s_waitingForFinalSink = false;
    s_timerActive = false;

    // Set ball count to 0 to properly trigger game over state
    if (pb::MainTable)
    {
        pb::MainTable->ChangeBallCount(0);
        pb::MainTable->ExtraBalls = 0;
    }

    // End the game
    pb::end_game();
}

void TimerMode::Reset()
{
    s_endTimeMs = 0;
    s_bonusTimeMs = 0;
    s_timerActive = false;
    s_timerExpired = false;
    s_waitingForFinalSink = false;
    s_scoreThreshold = kScorePerThreshold;
    s_totalScore = 0;
    
    // Also reset session timer
    s_sessionStartTimeMs = 0;
    s_sessionPauseTimeMs = 0;
    s_sessionAccumulatedMs = 0;
    s_sessionActive = false;
}

// Session time tracking (works for both Classic and Timer modes)
void TimerMode::StartSessionTimer()
{
    s_sessionStartTimeMs = GetCurrentTimeMs();
    s_sessionPauseTimeMs = 0;
    s_sessionAccumulatedMs = 0;
    s_sessionActive = true;
}

void TimerMode::StopSessionTimer()
{
    if (s_sessionActive)
    {
        // Capture final time before stopping
        if (s_sessionPauseTimeMs == 0)
        {
            s_sessionAccumulatedMs += GetCurrentTimeMs() - s_sessionStartTimeMs;
        }
        s_sessionActive = false;
    }
}

void TimerMode::PauseSessionTimer()
{
    if (s_sessionActive && s_sessionPauseTimeMs == 0)
    {
        // Accumulate time up to now
        s_sessionAccumulatedMs += GetCurrentTimeMs() - s_sessionStartTimeMs;
        s_sessionPauseTimeMs = GetCurrentTimeMs();
    }
}

void TimerMode::ResumeSessionTimer()
{
    if (s_sessionActive && s_sessionPauseTimeMs > 0)
    {
        // Reset start time to now (accumulated time already saved)
        s_sessionStartTimeMs = GetCurrentTimeMs();
        s_sessionPauseTimeMs = 0;
    }
}

int64_t TimerMode::GetSessionTimeMs()
{
    if (!s_sessionActive)
    {
        return s_sessionAccumulatedMs;
    }
    
    if (s_sessionPauseTimeMs > 0)
    {
        // Paused - return accumulated time only
        return s_sessionAccumulatedMs;
    }
    
    // Running - return accumulated + current segment
    return s_sessionAccumulatedMs + (GetCurrentTimeMs() - s_sessionStartTimeMs);
}
