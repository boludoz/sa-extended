#include "StdInc.h"

#include "AutoPilot.h"

// 0x6D5E20
CAutoPilot::CAutoPilot() {
    OldNode.m_wAreaId     = (uint16)-1;
    NewNode.m_wAreaId     = (uint16)-1;
    VeryOldNode.m_wAreaId = (uint16)-1;

    OldLink               = CCarPathLinkAddress();
    NewLink               = CCarPathLinkAddress();
    VeryOldLink           = CCarPathLinkAddress();

    for (int32 i = 0; i < CAR_NUM_PATHNODES_LOOKAHEAD; ++i) {
        aPathNodeList[i].m_wAreaId = (uint16)-1;
    }

    InvertDirOldLink     = 1;
    InvertDirNewLink     = 1;
    InvertDirVeryOldLink = 1;

    SlowingDownForCar    = false;
    SlowingDownForPed    = false;
    AvoidLevelTransitions = false;
    bAlwaysInFastLane    = false;
    bAlwaysInSlowLane    = false;
    bWarnTargetEntity    = false;
    bDontGoAgainstTraffic = false;
    bLeaveAfterAWhile    = false;
    bWaitForValidNodes   = false;
    bCarHasToReverseFirst = false;

    TimeToLeaveLink      = 0;
    TimeToGetToNextLink  = 1000;

    OldLane              = 0;
    NewLane              = 0;

    DrivingMode          = DRIVING_STYLE_STOP_FOR_CARS;
    Mission              = MISSION_NONE;
    TempAction           = TEMPACT_NONE;

    CruiseSpeed          = 10;
    ActualSpeed          = 10.0f;

    NumPathNodes         = 0;
    pTargetEntity        = nullptr;

    const uint32 timeMs  = CTimer::m_snTimeInMilliseconds;
    LastTimeNotStuck     = timeMs;
    LastTimeMoving       = timeMs;

    SpeedFromNodes       = 0;
    SpeedMultiplier      = 1.0f;
    RecordingNumber      = -1;
    NumTimesWantingToChangeNodes = 0;
    AISwitchToStraightLineDistance = 20;
    TempActionFinish     = 0;
    LastTimeWeStartedTempActReverse = 0;

    WhatToTryForReverse  = 0;
    SpeedCheat           = 0;
    AimAheadOfTarget     = 0;
    FollowCarDistance    = 10;
    TargetReachedDist    = 10;
    Diversion            = 0;

    LaneChangeCounter    = (rand() & 7) + 2;
    FramesFloating       = 0;
    ConstrainAreaMinX    = 0;
    ConstrainAreaMaxX    = 0;
    ConstrainAreaMinY    = 0;
    ConstrainAreaMaxY    = 0;
    pObstructingEntity   = nullptr;
    MaxSpeedBuffer       = 0.0f;
    HooverDistFromTarget = 0;
}

// 0x41B980
void CAutoPilot::ModifySpeed(float target) {
    plugin::CallMethod<0x41B980, CAutoPilot*, float>(this, target);
}

// 0x41B950
void CAutoPilot::RemoveOnePathNode() {
    --NumPathNodes;
    for (int16 c = 0; c < NumPathNodes; ++c) {
        aPathNodeList[c] = aPathNodeList[c + 1];
    }
}

void CAutoPilot::SetCarMission(eCarMission carMission, uint32 timeOffsetMs) {
    Mission = carMission;
    LastTimeNotStuck = CTimer::GetTimeInMS() + timeOffsetMs;
}

// notsa
void CAutoPilot::SetTempAction(eAutoPilotTempAction action, uint32 durMs) {
    TempAction = action;
    TempActionFinish = CTimer::GetTimeInMS() + durMs;
}
