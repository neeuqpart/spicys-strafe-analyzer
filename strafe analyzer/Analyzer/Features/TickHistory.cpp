#include "TickHistory.h"
#include "../TickData.h"
#include "../History.h"

#include "../AnalyzerInlines.h"
#include "../AnalyzerMath.h"
#include "../../zgui/menu.h"

#ifndef NOLATENCY
#define NOLATENCY 999999
#endif

template <typename T>
void pop_front(std::vector<T>& v)
{
    if (v.size() > 0)
        v.erase(v.begin());
}

template <class T, class T2, class T3, class S>
double SmoothValueComparedToVector(T value, T2* vecPtr, T3 S::* targetElemPtr, int numElemToAvg)
{
    if (numElemToAvg <= 0 || History::tickHist.empty())
        return value;
    const auto count = std::min<std::size_t>(History::tickHist.size(), static_cast<std::size_t>(numElemToAvg));
    double sum = 0.0;
    for (std::size_t i = 0; i < count; ++i)
        sum += vecPtr[History::tickHist.size() - 1 - i].*targetElemPtr;
    // A conventional moving average smooths display data without inventing a
    // multiplier or dividing by the current value when it is zero.
    return (sum + value) / static_cast<double>(count + 1);
}

void TickHistory::Update(CUserCmd* cmd)
{
    std::lock_guard<std::mutex> lock(History::mutex);

    TickData curTick;

    const int DATA_HEIGHT = g_menu.strafetrainer.dataHeight;
    const int PREV_TICK = static_cast<int>(History::tickHist.size()) - 1;

#pragma region CalculateCurTickFields
    double dbDeltaYaw = 0;

    curTick.vecViewangles = cmd->viewangles;
    curTick.vecPos = player->origin();
    curTick.vecVel = player->velocity();

    curTick.speed = std::hypot(curTick.vecVel.x, curTick.vecVel.y);

    // If tickHist has at least 2 ticks, we can calculate deltaYaw
    // and get old num jumps / strafes
    if (PREV_TICK >= 0)
    {
        dbDeltaYaw = SubtractAngle(curTick.vecViewangles.y, History::tickHist[PREV_TICK].vecViewangles.y);
        curTick.curJumps = History::tickHist[PREV_TICK].curJumps;
        curTick.curStrafes = History::tickHist[PREV_TICK].curStrafes;
    }

    // Mouse dir
    if (dbDeltaYaw > 0)         curTick.mouseDir = MouseDir::LEFT;
    else if (dbDeltaYaw < 0)    curTick.mouseDir = MouseDir::RIGHT;
    else                        curTick.mouseDir = MouseDir::NONE;

    // Key dir
    if ((cmd->buttons & in_moveleft && cmd->buttons & in_moveright) || (cmd->buttons & in_forward && cmd->buttons & in_back))
    {
        curTick.keyDir = KeyDir::COUNTER_STRAFE;
    }
    else
    {
        if (cmd->buttons & in_forward)
        {
            if (cmd->buttons & in_moveleft)             curTick.keyDir = KeyDir::FORWARD_LEFT;
            else if (cmd->buttons & in_moveright)       curTick.keyDir = KeyDir::FORWARD_RIGHT;
            else                                        curTick.keyDir = KeyDir::FORWARD;
        }
        else if (cmd->buttons & in_back)
        {
            if (cmd->buttons & in_moveleft)             curTick.keyDir = KeyDir::BACK_LEFT;
            else if (cmd->buttons & in_moveright)       curTick.keyDir = KeyDir::BACK_RIGHT;
            else                                        curTick.keyDir = KeyDir::BACK;
        }
        else if (cmd->buttons & in_moveleft)            curTick.keyDir = KeyDir::LEFT;
        else if (cmd->buttons & in_moveright)           curTick.keyDir = KeyDir::RIGHT;
        else                                            curTick.keyDir = KeyDir::NONE;
    }

    if (fabs(MouseAndKeyPhiDiff(curTick.mouseDir, curTick.keyDir)) <= (M_PI / 2)
        && static_cast<int>(curTick.mouseDir) != 0 && static_cast<int>(curTick.keyDir) != 0)
    {
        curTick.strafeDir = curTick.mouseDir;
    }
    else if (PREV_TICK >= 0) curTick.strafeDir = History::tickHist[PREV_TICK].strafeDir;

    // Pos type
    if (player->is_in_air()) curTick.posType = PositionType::AIR;
    else curTick.posType = PositionType::GROUND;

    curTick.crouching = (cmd->buttons & in_duck) != 0;

    if (!player->is_in_air() || curTick.speed < 1.0)
    {
        // Keep the original prespeed guide on the ground. A zero target
        // produces an invisible filled bar and pins the vertical guide below it.
        curTick.perfDeltaYaw = 1.18;

        if (HasBeenOnGround(PREV_TICK))
        {
            // Reset all statistics
            curTick.curJumps = 0;
            curTick.curStrafes = 0;
            curTick.strafeDir = MouseDir::NONE;
        }
    }
    else
    {
        // PerfAngle Calculation
        const int ACCEL_LIMIT = 30;
        float airAccel = 10.0f;
        if (Interfaces::sv_airaccelerate)
        {
            const float val = Interfaces::sv_airaccelerate->get_float();
            if (std::isfinite(val) && val >= 0.0f)
                airAccel = val;
        }
        const double tickInterval = (Interfaces::globals && Interfaces::globals->interval_per_tick > 0.0f)
            ? Interfaces::globals->interval_per_tick : 0.0151515;
        const double accelSpeed = std::fmin(tickInterval * 30.0 * airAccel, static_cast<double>(ACCEL_LIMIT));
        curTick.perfDeltaYaw = RAD2DEG(atan2f(accelSpeed, curTick.speed));
    }

    curTick.syncState = GetSyncState(cmd->buttons, curTick.mouseDir);

    if (History::tickHist.size() >= 2)
    {
        // Smoothed/unsmoothed deltaYaw
        if (g_menu.strafetrainer.smooth)
        {
            curTick.deltaYaw = SmoothValueComparedToVector(fabs(dbDeltaYaw), History::tickHist.data(), &TickData::deltaYaw, 4);
        }
        else curTick.deltaYaw = fabs(dbDeltaYaw);

        // Lost speed
        curTick.lostSpeed = HasLostSpeed(g_menu.strafetrainer.speedLossTolerance, curTick.speed);

        // If jumped
        if (HasJumped()) curTick.curJumps++;

        // If strafed (only while airborne)
        if (curTick.posType == PositionType::AIR && HasStrafed(curTick.mouseDir, curTick.keyDir, History::tickHist[PREV_TICK].strafeDir))
        {
            curTick.curStrafes++;
            curTick.latencyAmount = GetSyncLatency(curTick.strafeDir);
        }
        else curTick.latencyAmount = NOLATENCY;
    }
    else
    {
        curTick.deltaYaw = 0;
        curTick.latencyAmount = NOLATENCY;
    }

    // Misc
    curTick.scrolling = (cmd->buttons & in_jump) != 0;

#pragma endregion

    // Push curtick onto our tick history vector
    History::tickHist.push_back(curTick);


    if (History::tickHist.size() > 1)
    {
        // Removes unnecessary Data at the back of the stack
        while (History::tickHist.size() > g_menu.strafetrainer.historySize)
        {
            pop_front(History::tickHist);
        }
    }
}
