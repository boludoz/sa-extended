/*
    Plugin-SDK file
    Authors: GTA Community. See more here
    https://github.com/DK22Pac/plugin-sdk
    Do not delete this comment block. Respect others' work!
*/
#pragma once

#include "Base.h"

#include "PathFind.h"
#include "NodeAddress.h"
#include "Vector.h"
#include "eCarMission.h"
#include "eCarDrivingStyle.h"

class CVehicle;
class CEntity;

enum eAutoPilotTempAction : int8 {
    TEMPACT_NONE                     = 0,
    TEMPACT_WAIT                     = 1,
    TEMPACT_EMPTYTOBEREUSED          = 2,
    TEMPACT_REVERSE                  = 3,
    TEMPACT_HANDBRAKETURNLEFT        = 4,
    TEMPACT_HANDBRAKETURNRIGHT       = 5,
    TEMPACT_HANDBRAKESTRAIGHT        = 6,
    TEMPACT_TURNLEFT                 = 7,
    TEMPACT_TURNRIGHT                = 8,
    TEMPACT_GOFORWARD                = 9,
    TEMPACT_SWIRVELEFT               = 10,
    TEMPACT_SWIRVERIGHT              = 11,
    TEMPACT_STUCKINTRAFFIC           = 12,
    TEMPACT_REVERSE_LEFT             = 13,
    TEMPACT_REVERSE_RIGHT            = 14,
    TEMPACT_PLANE_FLY_UP             = 15,
    TEMPACT_PLANE_FLY_STRAIGHT       = 16,
    TEMPACT_PLANE_SHARP_LEFT         = 17,
    TEMPACT_PLANE_SHARP_RIGHT        = 18,
    TEMPACT_HEADON_COLLISION         = 19,
    TEMPACT_SWIRVELEFT_STOP          = 20,
    TEMPACT_SWIRVERIGHT_STOP         = 21,
    TEMPACT_REVERSE_STRAIGHT         = 22,
    TEMPACT_BOOST_USE_STEERING_ANGLE = 23,
    TEMPACT_BRAKE                    = 24,
};

class CAutoPilot {
public:
    enum {
        DRIVINGMODE_STOPFORCARS,
        DRIVINGMODE_SLOWDOWNFORCARS,
        DRIVINGMODE_AVOIDCARS,
        DRIVINGMODE_PLOUGHTHROUGH,
        DRIVINGMODE_STOPFORCARS_IGNORELIGHTS,
        DRIVINGMODE_AVOIDCARS_OBEYLIGHTS,
        DRIVINGMODE_AVOIDCARS_STOPFORPEDS_OBEYLIGHTS
    };

    enum {
        CAR_NUM_PATHNODES_LOOKAHEAD = 8
    };

    CNodeAddress         OldNode;
    CNodeAddress         NewNode;
    CNodeAddress         VeryOldNode;
    int32                TimeToLeaveLink;
    int32                TimeToGetToNextLink;
    CCarPathLinkAddress  OldLink;
    CCarPathLinkAddress  NewLink;
    CCarPathLinkAddress  VeryOldLink;

    uint32               LastTimeNotStuck;
    uint32               LastTimeMoving;
    int8                 InvertDirVeryOldLink;
    int8                 InvertDirOldLink;
    int8                 InvertDirNewLink;
    int8                 OldLane;
    int8                 NewLane;
    eCarDrivingStyle     DrivingMode;
    eCarMission          Mission;
    eAutoPilotTempAction TempAction;
    uint32               TempActionFinish;
    uint32               LastTimeWeStartedTempActReverse;
    uint8                WhatToTryForReverse;
    uint8                NumTimesWantingToChangeNodes;
    float                ActualSpeed;
    float                MaxSpeedBuffer;
    uint8                CruiseSpeed;
    int8                 SpeedFromNodes;
    float                SpeedMultiplier;
    uint8                HooverDistFromTarget;
    int8                 SpeedCheat;
    int8                 AimAheadOfTarget;
    uint8                SlowingDownForCar : 1;
    uint8                SlowingDownForPed : 1;
    uint8                AvoidLevelTransitions : 1;
    uint8                bAlwaysInFastLane : 1;
    uint8                bAlwaysInSlowLane : 1;
    uint8                bWarnTargetEntity : 1;
    uint8                bDontGoAgainstTraffic : 1;
    uint8                bLeaveAfterAWhile : 1;
    uint8                bWaitForValidNodes : 1;
    uint8                bCarHasToReverseFirst : 1;
    uint8                AISwitchToStraightLineDistance;
    uint8                FollowCarDistance;
    uint8                TargetReachedDist;
    int8                 LaneChangeCounter;
    int8                 FramesFloating;

    int16                ConstrainAreaMinX;
    int16                ConstrainAreaMaxX;
    int16                ConstrainAreaMinY;
    int16                ConstrainAreaMaxY;

    CVector              TargetCoors;
    CNodeAddress         aPathNodeList[8];
    int16                NumPathNodes;
    CEntity*             pTargetEntity;
    CEntity*             pObstructingEntity;

    int8                 RecordingNumber;
    int8                 Diversion;

    CAutoPilot();
    ~CAutoPilot() = default;

    void ModifySpeed(float target);
    void RemoveOnePathNode();

    void SetCarMission(eCarMission carMission) { Mission = carMission; }
    void SetMission(int8 carMission) { Mission = (eCarMission)carMission; }

    void SetCarMission(eCarMission carMission, uint32 timeOffsetMs);

    //! 0x463490, listed as `CCarCtrl::SetCarMission` but `this` is the auto pilot.
    //! A vehicle already crashing down keeps that mission. Only script commands use it.
    void SetCarMissionUnlessCrashing(eCarMission carMission) {
        if (Mission != MISSION_PLANE_CRASH_AND_BURN && Mission != MISSION_HELI_CRASH_AND_BURN) {
            Mission = carMission;
        }
    }

    void ClearCarMission() { Mission = MISSION_NONE; }

    void SetCruiseSpeed(uint32 s) { assert(s <= UINT8_MAX); CruiseSpeed = (uint8)s; }

    void SetTempAction(eAutoPilotTempAction action, uint32 durMs);
    void ClearTempAct() { TempAction = TEMPACT_NONE; }

    void SetDrivingStyle(eCarDrivingStyle s) { DrivingMode = s; }
};

VALIDATE_SIZE(CAutoPilot, 0x98);
VALIDATE_OFFSET(CAutoPilot, OldNode, 0x00);
VALIDATE_OFFSET(CAutoPilot, NewNode, 0x04);
VALIDATE_OFFSET(CAutoPilot, VeryOldNode, 0x08);
VALIDATE_OFFSET(CAutoPilot, TimeToLeaveLink, 0x0C);
VALIDATE_OFFSET(CAutoPilot, TimeToGetToNextLink, 0x10);
VALIDATE_OFFSET(CAutoPilot, OldLink, 0x14);
VALIDATE_OFFSET(CAutoPilot, NewLink, 0x16);
VALIDATE_OFFSET(CAutoPilot, VeryOldLink, 0x18);
VALIDATE_OFFSET(CAutoPilot, LastTimeNotStuck, 0x1C);
VALIDATE_OFFSET(CAutoPilot, LastTimeMoving, 0x20);
VALIDATE_OFFSET(CAutoPilot, InvertDirVeryOldLink, 0x24);
VALIDATE_OFFSET(CAutoPilot, InvertDirOldLink, 0x25);
VALIDATE_OFFSET(CAutoPilot, InvertDirNewLink, 0x26);
VALIDATE_OFFSET(CAutoPilot, OldLane, 0x27);
VALIDATE_OFFSET(CAutoPilot, NewLane, 0x28);
VALIDATE_OFFSET(CAutoPilot, DrivingMode, 0x29);
VALIDATE_OFFSET(CAutoPilot, Mission, 0x2A);
VALIDATE_OFFSET(CAutoPilot, TempAction, 0x2B);
VALIDATE_OFFSET(CAutoPilot, TempActionFinish, 0x2C);
VALIDATE_OFFSET(CAutoPilot, LastTimeWeStartedTempActReverse, 0x30);
VALIDATE_OFFSET(CAutoPilot, WhatToTryForReverse, 0x34);
VALIDATE_OFFSET(CAutoPilot, NumTimesWantingToChangeNodes, 0x35);
VALIDATE_OFFSET(CAutoPilot, ActualSpeed, 0x38);
VALIDATE_OFFSET(CAutoPilot, MaxSpeedBuffer, 0x3C);
VALIDATE_OFFSET(CAutoPilot, CruiseSpeed, 0x40);
VALIDATE_OFFSET(CAutoPilot, SpeedFromNodes, 0x41);
VALIDATE_OFFSET(CAutoPilot, SpeedMultiplier, 0x44);
VALIDATE_OFFSET(CAutoPilot, HooverDistFromTarget, 0x48);
VALIDATE_OFFSET(CAutoPilot, SpeedCheat, 0x49);
VALIDATE_OFFSET(CAutoPilot, AimAheadOfTarget, 0x4A);
VALIDATE_OFFSET(CAutoPilot, AISwitchToStraightLineDistance, 0x4D);
VALIDATE_OFFSET(CAutoPilot, FollowCarDistance, 0x4E);
VALIDATE_OFFSET(CAutoPilot, TargetReachedDist, 0x4F);
VALIDATE_OFFSET(CAutoPilot, LaneChangeCounter, 0x50);
VALIDATE_OFFSET(CAutoPilot, FramesFloating, 0x51);
VALIDATE_OFFSET(CAutoPilot, ConstrainAreaMinX, 0x52);
VALIDATE_OFFSET(CAutoPilot, ConstrainAreaMaxX, 0x54);
VALIDATE_OFFSET(CAutoPilot, ConstrainAreaMinY, 0x56);
VALIDATE_OFFSET(CAutoPilot, ConstrainAreaMaxY, 0x58);
VALIDATE_OFFSET(CAutoPilot, TargetCoors, 0x5C);
VALIDATE_OFFSET(CAutoPilot, aPathNodeList, 0x68);
VALIDATE_OFFSET(CAutoPilot, NumPathNodes, 0x88);
VALIDATE_OFFSET(CAutoPilot, pTargetEntity, 0x8C);
VALIDATE_OFFSET(CAutoPilot, pObstructingEntity, 0x90);
VALIDATE_OFFSET(CAutoPilot, RecordingNumber, 0x94);
VALIDATE_OFFSET(CAutoPilot, Diversion, 0x95);
