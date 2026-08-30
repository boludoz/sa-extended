/*
    Plugin-SDK file
    Authors: GTA Community. See more here
    https://github.com/DK22Pac/plugin-sdk
    Do not delete this comment block. Respect others' work!
*/
#pragma once

#include "Base.h"

#include "eCamMode.h"
#include "Vector.h"

/* http://code.google.com/p/mtasa-blue/source/browse/tags/1.3.4/MTA10/game_sa/CCamSA.h */

class CEntity;
class CPed;
class CVehicle;

extern bool& gbFirstPersonRunThisFrame;

//! Which of the "busted" cameras `CCam::ProcessArrestCamOne` settled on
enum eArrestCam : int32 {
    ARRESTCAM_NONE                 = 0,
    ARRESTCAM_DW                   = 1,
    ARRESTCAM_OVERSHOULDER         = 2,
    ARRESTCAM_ALONGGROUND          = 3,
    ARRESTCAM_ALONGGROUND_RIGHT    = 4,
    ARRESTCAM_ALONGGROUND_RIGHT_UP = 5,
    ARRESTCAM_ALONGGROUND_LEFT     = 6,
    ARRESTCAM_ALONGGROUND_LEFT_UP  = 7,
    ARRESTCAM_FROMLAMPPOST         = 8,
};
constexpr auto NUM_ARREST_CAMS = 6u;

class CCam {
public:
    bool      m_bBelowMinDist;                    // aka: bBelowMinDist
    bool      m_bBehindPlayerDesired;             // aka: bBehindPlayerDesired
    bool      m_bCamLookingAtVector;              // aka: m_bCamLookingAtVector
    bool      m_bCollisionChecksOn;               // aka: m_bCollisionChecksOn
    bool      m_bFixingBeta;                      // aka: m_bFixingBeta
    bool      m_bTheHeightFixerVehicleIsATrain;   // aka: m_bTheHeightFixerVehicleIsATrain
    bool      m_bLookBehindCamWasInFront;         // aka: LookBehindCamWasInFront
    bool      m_bLookingBehind;                   // aka: LookingBehind
    bool      m_bLookingLeft;                     // aka: LookingLeft
    bool      m_bLookingRight;                    // aka: LookingRight
    bool      m_bResetStatics;                    // aka: ResetStatics
    bool      m_bRotating;                        // aka: Rotating
    eCamMode  m_nMode;                            // aka: Mode
    uint32    m_nFinishTime;                      // aka: m_uiFinishTime
    uint32    m_nDoCollisionChecksOnFrameNum;     // aka: m_iDoCollisionChecksOnFrameNum
    uint32    m_nDoCollisionCheckEveryNumOfFrames; // aka: m_iDoCollisionCheckEveryNumOfFrames
    uint32    m_nFrameNumWereAt;                  // aka: m_iFrameNumWereAt
    uint32    m_nRunningVectorArrayPos;
    uint32    m_nRunningVectorCounter;
    uint32    m_nDirectionWasLooking;             // aka: DirectionWasLooking
    float     m_fMaxRoleAngle;
    float     m_fRoll;
    float     m_fRollSpeed;
    float     m_fSyphonModeTargetZOffSet;         // aka: m_fSyphonModeTargetZOffSet
    float     m_fAmountFractionObscured;
    float     m_fAlphaSpeedOverOneFrame;          // aka: m_fAlphaSpeedOverOneFrame
    float     m_fBetaSpeedOverOneFrame;           // aka: m_fBetaSpeedOverOneFrame
    float     m_fBufferedTargetBeta;
    float     m_fBufferedTargetOrientation;
    float     m_fBufferedTargetOrientationSpeed;
    float     m_fCamBufferedHeight;               // aka: m_fCamBufferedHeight
    float     m_fCamBufferedHeightSpeed;          // aka: m_fCamBufferedHeightSpeed
    float     m_fCloseInPedHeightOffset;          // aka: m_fCloseInPedHeightOffset
    float     m_fCloseInPedHeightOffsetSpeed;     // aka: m_fCloseInPedHeightOffsetSpeed
    float     m_fCloseInCarHeightOffset;          // aka: m_fCloseInCarHeightOffset
    float     m_fCloseInCarHeightOffsetSpeed;     // aka: m_fCloseInCarHeightOffsetSpeed
    float     m_fDimensionOfHighestNearCar;       // aka: m_fDimensionOfHighestNearCar
    float     m_fDistanceBeforeChanges;           // aka: m_fDistanceBeforeChanges
    float     m_fFovSpeedOverOneFrame;            // aka: m_fFovSpeedOverOneFrame
    float     m_fMinDistAwayFromCamWhenInterPolating; // aka: m_fMinDistAwayFromCamWhenInterPolating
    float     m_fPedBetweenCameraHeightOffset;    // aka: m_fPedBetweenCameraHeightOffset
    float     m_fPlayerInFrontSyphonAngleOffSet;  // aka: m_fPlayerInFrontSyphonAngleOffSet
    float     m_fRadiusForDead;                   // aka: m_fRadiusForDead
    float     m_fRealGroundDist;                  // aka: m_fRealGroundDist
    float     m_fTargetBeta;
    float     m_fTimeElapsedFloat;                // aka: m_fTimeElapsedFloat
    float     m_fTilt;                            // aka: m_fTilt
    float     m_fTiltSpeed;                       // aka: m_fTiltSpeed
    float     m_fTransitionBeta;                  // aka: m_fTransitionBeta
    float     m_fTrueBeta;                        // aka: m_fTrueBeta
    float     m_fTrueAlpha;                       // aka: m_fTrueAlpha
    float     m_fInitialPlayerOrientation;        // aka: m_fInitialPlayerOrientation
    float     m_fVerticalAngle;                   // aka: Alpha
    float     m_fAlphaSpeed;                      // aka: AlphaSpeed
    float     m_fFOV;                             // aka: FOV
    float     m_fFOVSpeed;                        // aka: FOVSpeed
    float     m_fHorizontalAngle;                 // aka: Beta
    float     m_fBetaSpeed;                       // aka: BetaSpeed
    float     m_fDistance;                        // aka: Distance
    float     m_fDistanceSpeed;                   // aka: DistanceSpeed
    float     m_fCaMinDistance;                   // aka: CA_MIN_DISTANCE
    float     m_fCaMaxDistance;                   // aka: CA_MAX_DISTANCE
    float     m_fSpeedVar;                        // aka: SpeedVar
    float     m_fCameraHeightMultiplier;          // aka: m_fCameraHeightMultiplier
    float     m_fTargetZoomGroundOne;             // aka: m_fTargetZoomGroundOne
    float     m_fTargetZoomGroundTwo;             // aka: m_fTargetZoomGroundTwo
    float     m_fTargetZoomGroundThree;           // aka: m_fTargetZoomGroundThree
    float     m_fTargetZoomOneZExtra;             // aka: m_fTargetZoomOneZExtra
    float     m_fTargetZoomTwoZExtra;             // aka: m_fTargetZoomTwoZExtra
    float     m_fTargetZoomTwoInteriorZExtra;     // aka: m_fTargetZoomTwoInteriorZExtra
    float     m_fTargetZoomThreeZExtra;           // aka: m_fTargetZoomThreeZExtra
    float     m_fTargetZoomZCloseIn;              // aka: m_fTargetZoomZCloseIn
    float     m_fMinRealGroundDist;               // aka: m_fMinRealGroundDist
    float     m_fTargetCloseInDist;               // aka: m_fTargetCloseInDist
    float     m_fBeta_Targeting;                  // aka: Beta_Targeting
    float     m_fX_Targetting;                    // aka: X_Targetting
    float     m_fY_Targetting;                    // aka: Y_Targetting
    int32     m_nCarWeAreFocussingOn;             // aka: CarWeAreFocussingOn
    float     m_fCarWeAreFocussingOnI;            // aka: CarWeAreFocussingOnI
    float     m_fCamBumpedHorz;                   // aka: m_fCamBumpedHorz
    float     m_fCamBumpedVert;                   // aka: m_fCamBumpedVert
    uint32    m_nCamBumpedTime;                   // aka: m_nCamBumpedTime
    CVector   m_vecSourceSpeedOverOneFrame;       // aka: m_cvecSourceSpeedOverOneFrame
    CVector   m_vecTargetSpeedOverOneFrame;       // aka: m_cvecTargetSpeedOverOneFrame
    CVector   m_vecUpOverOneFrame;               // aka: m_cvecUpOverOneFrame
    CVector   m_vecTargetCoorsForFudgeInter;     // aka: m_cvecTargetCoorsForFudgeInter
    CVector   m_vecCamFixedModeVector;            // aka: m_cvecCamFixedModeVector
    CVector   m_vecCamFixedModeSource;            // aka: m_cvecCamFixedModeSource
    CVector   m_vecCamFixedModeUpOffSet;          // aka: m_cvecCamFixedModeUpOffSet
    CVector   m_vecLastAboveWaterCamPosition;     // aka: m_vecLastAboveWaterCamPosition
    CVector   m_vecBufferedPlayerBodyOffset;      // aka: m_vecBufferedPlayerBodyOffset
    CVector   m_vecFront;                         // aka: Front
    CVector   m_vecSource;                        // aka: Source
    CVector   m_vecSourceBeforeLookBehind;        // aka: SourceBeforeLookBehind
    CVector   m_vecUp;                            // aka: Up
    std::array<CVector, 2> m_avecPreviousVectors; // aka: m_arrPreviousVectors[2]
    std::array<CVector, 4> m_avecTargetHistoryPos; // aka: m_aTargetHistoryPos[4]
    std::array<uint32, 4>  m_anTargetHistoryTime; // aka: m_nTargetHistoryTime[4]
    uint32    m_nCurrentHistoryPoints;            // aka: m_nCurrentHistoryPoints
    CEntity*  m_pCamTargetEntity;                 // aka: CamTargetEntity
    float     m_fCameraDistance;                  // aka: m_fCameraDistance
    float     m_fIdealAlpha;                      // aka: m_fIdealAlpha
    float     m_fPlayerVelocity;                  // aka: m_fPlayerVelocity
    CVehicle* m_pLastCarEntered;                  // aka: m_pLastCarEntered
    CPed*     m_pLastPedLookedAt;                 // aka: m_pLastPedLookedAt
    bool      m_bFirstPersonRunAboutActive;       // aka: m_bFirstPersonRunAboutActive

public:
    static void InjectHooks();

    CCam();

    void Init();

    void CacheLastSettingsDWCineyCam();
    void DoCamBump(float horizontal, float vertical);
    void Finalise_DW_CineyCams(const CVector& src, const CVector& dest, float roll, float fov, float nearClip, float shakeDegree);
    void GetCoreDataForDWCineyCamMode(CEntity*& entity, CVehicle*& vehicle, CVector& dest, CVector& src, CVector& targetUp, CVector& targetRight, CVector& targetFwd, CVector& targetVel, float& targetSpeed, CVector& targetAngVel, float& targetAngSpeed, CColSphere& colSphere);
    bool GetLookAlongGroundPos(CEntity* target, CPed* cop, const CVector& vecTarget, CVector& vecSource);
    bool GetLookOverShoulderPos(CEntity* target, CPed* cop, const CVector& vecTarget, CVector& vecSource);
    bool GetLookFromLampPostPos(CEntity* target, CPed* cop, const CVector& vecTarget, CVector& vecSource);
    bool ProcessDWBustedCam1(CPed* cop, bool isFirstTime);
    void GetVectorsReadyForRW();
    void Get_TwoPlayer_AimVector(CVector&);
    bool IsTimeToExitThisDWCineyCamMode(int32 camId, const CVector& src, const CVector& dst, float t, bool lineOfSightCheck);
    void KeepTrackOfTheSpeed(const CVector& source, const CVector& targetToLookAt, const CVector& up, const float& trueAlpha, const float& trueBeta, const float& fov);
    bool GetBoatLook_L_R_HeightOffset(float& outHeightOffset) const;
    bool IsCamOnAStringMode() const;
    bool LookBehind();
    bool LookLeft() { return LookRight(false); }
    bool LookRight(bool bLookRight);
    bool RotCamIfInFrontCar(const CVector& targetCoors, float targetOrientation);
    bool Using3rdPersonMouseCam() const;
    bool GetWeaponFirstPersonOn();
    void ClipAlpha();
    void ClipBeta();

    void Process();
    void ProcessArrestCamOne();
    void ProcessPedsDeadBaby();
    void Process_1rstPersonPedOnPC(const CVector&, float, float, float);
    void Process_1stPerson(const CVector&, float, float, float);
    void Process_AimWeapon(const CVector&, float, float, float);
    void Process_AttachedCam();
    void Process_Cam_TwoPlayer();
    bool Process_Cam_TwoPlayer_TestLOSs(const CVector& tempSource);
    void Process_Cam_TwoPlayer_CalcSource(float beta, CVector& outSource, CVector& outLookAt, CVector& outTarget);
    void Process_Cam_TwoPlayer_InCarAndShooting();
    void Process_Cam_TwoPlayer_Separate_Cars();
    void Process_Cam_TwoPlayer_Separate_Cars_TopDown();
    bool Process_DW_BirdyCam(bool bCheckValid);
    bool Process_DW_CamManCam(bool bCheckValid);
    bool Process_DW_HeliChaseCam(bool bCheckValid);
    bool Process_DW_PlaneCam1(bool bCheckValid);
    bool Process_DW_PlaneCam2(bool bCheckValid);
    bool Process_DW_PlaneCam3(bool bCheckValid);
    bool Process_DW_PlaneSpotterCam(bool bCheckValid);
    void Process_Editor(const CVector& target, float orientation, float speedVar, float speedVarWanted);
    void Process_Fixed(const CVector& target, float orientation, float speedVar, float speedVarWanted);
    void Process_FlyBy(const CVector& target, float orientation, float speedVar, float speedVarWanted);
    void Process_FollowCar_SA(const CVector& target, float orientation, float speedVar, float speedVarWanted, bool);
    void Process_FollowPedWithMouse(const CVector& target, float orientation, float speedVar, float speedVarWanted);
    void Process_FollowPed_SA(const CVector& target, float orientation, float speedVar, float speedVarWanted, bool);
    void Process_M16_1stPerson(const CVector& target, float orientation, float speedVar, float speedVarWanted);
    void Process_Rocket(const CVector& target, float orientation, float speedVar, float speedVarWanted, bool isHeatSeeking);
    void Process_SpecialFixedForSyphon(const CVector& target, float orientation, float speedVar, float speedVarWanted);
    bool Process_WheelCam(const CVector& target, float orientation, float speedVar, float speedVarWanted);

    // inlined
    void ApplyUnderwaterMotionBlur();

private:
    CCam* Constructor() {
        this->CCam::CCam();
        return this;
    }
};

VALIDATE_SIZE(CCam, 0x238);

bool  IsLampPost(eModelID modelId);
