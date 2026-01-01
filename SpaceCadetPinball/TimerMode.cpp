#include "pch.h"
#include "TimerMode.h"

#include "pb.h"
#include "TPinballTable.h"
#include "pinball.h"
#include "TTextBox.h"
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
    
    int64_t remaining = (s_endTimeMs + s_bonusTimeMs) - GetCurrentTimeMs();
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

void TimerMode::OnScoreAdded(int scoreAdded)
{
    if (s_currentMode != Mode::Timer || !s_timerActive || s_timerExpired)
        return;

    s_totalScore += scoreAdded;

    // Check if we crossed threshold(s) - group multiple bonuses
    int bonusCount = 0;
    while (s_totalScore >= s_scoreThreshold)
    {
        s_scoreThreshold += kScorePerThreshold;
        bonusCount++;
    }
    
    // Add all bonus time at once (exactly 10000ms per bonus)
    if (bonusCount > 0)
    {
        s_bonusTimeMs += bonusCount * kTimePerThresholdMs;
        
        // Notify Java UI with grouped bonus (e.g., +20 for two thresholds)
        int totalBonus = bonusCount * static_cast<int>(kTimePerThresholdMs / 1000);
        SpaceCadetPinballJNI::notifyTimerBonus(totalBonus);
    }
}

void TimerMode::OnBallCrash()
{
    if (s_currentMode != Mode::Timer || !s_timerActive || s_timerExpired)
        return;

    s_bonusTimeMs -= kCrashPenaltyMs;

    // Notify Java UI with penalty (negative value)
    SpaceCadetPinballJNI::notifyTimerBonus(-static_cast<int>(kCrashPenaltyMs / 1000));

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
}
