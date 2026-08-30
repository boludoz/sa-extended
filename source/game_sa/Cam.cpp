#include "StdInc.h"

#include "Cam.h"
#include "TimeCycle.h"
#include "Camera.h"
#include "Shadows.h"
#include "IdleCam.h"
#include "InterestingEvents.h"
#include "ModelIndices.h"
#include "HandShaker.h"
#include "PostEffects.h"
#include "TaskSimpleClimb.h"
#include "TaskSimpleFight.h"
#include "TaskSimpleSwim.h"
#include "WeaponEffects.h"
#include "cHandlingDataMgr.h"
#include "CullZones.h"
#include "TaskSimpleArrestPed.h"
#include "Messages.h"
#include "WaterLevel.h"
#include "GameLogic.h"
#include "PlayerInfo.h"
#include "TaskSimpleGangDriveBy.h"
#include "WeaponInfo.h"
#include "Draw.h"
#include "ControllerConfigManager.h"
#include "PedClothesDesc.h"
#include "Ragdoll/IKChainManager.h"
#include "IdleCam.h"
#include "Tasks/TaskTypes/TaskSimpleUseGun.h"
#include "Tasks/TaskTypes/TaskComplexProstituteSolicit.h"

auto& gbFirstPersonRunThisFrame = StaticRef<bool>(0xB6EC20);
auto& gLastFrameProcessedDWCineyCam = StaticRef<uint32>(0x8CCB9C);

//! The DW cineycams are numbered 20..28 (heli chase, cam man, birdy, plane spotter,
//! dog fight, fish, plane 1..3). The compiler folds that bias into `gbExitCam`'s base
//! address, so the disassembly reads it as `(*(bool(*)[])0xB6EC5C)[camId]`.
static constexpr auto DW_CINEYCAM_FIRST_ID = 20;
static inline auto& gbExitCam          = StaticRef<std::array<bool, 9>>(0xB6EC70); // Set by the `Process_DW_*` cams
static inline auto& gDWCineyCamEndTime = StaticRef<uint32>(0x8CCBA4);              // Set by the `Process_DW_*` cams
static inline auto& gDWCineyCamStartTime   = StaticRef<uint32>(0x8CCBA0);
static inline auto& gDWLastModeForCineyCam = StaticRef<eCamMode>(0x8CC488);

static inline auto& DWCineyCamLastPos = StaticRef<CVector>(0xB6FE8C);
static inline auto& DWCineyCamLastUp = StaticRef<CVector>(0xB6FE98);
static inline auto& DWCineyCamLastRight = StaticRef<CVector>(0xB6FEA4);
static inline auto& DWCineyCamLastFwd = StaticRef<CVector>(0xB6FEB0);

//! Where `Process_FollowCar_SA` decided the look-behind camera should aim this frame. `gTargetCoordsForLookingBehind` @ 0xB6F018
static CVector gTargetCoordsForLookingBehind{};

//! Per-zoom pitch offset, indexed by `CCamera::GetArrPosForVehicleType`. @ 0x8CC41C, 0x8CC430, 0x8CC444
static constexpr float ZmOneAlphaOffset[5]   = { 0.08f,  0.08f, 0.15f, 0.08f, 0.08f };
static constexpr float ZmTwoAlphaOffset[5]   = { 0.07f,  0.08f, 0.30f, 0.08f, 0.08f };
static constexpr float ZmThreeAlphaOffset[5] = { 0.055f, 0.05f, 0.15f, 0.06f, 0.08f };

// `Process_FollowCar_SA` statics
static float sBlendExtraPos      = 0.0f;     // 0xB7011C
static float MOUSE_INPUT_COUNTER = 0.0f;     // 0xB70118
static bool  gAcquiredAlpha      = false;    // 0xB70114
static float gOldAlpha           = -9999.0f; // 0x8CCEB0
static float gOldBeta            = -9999.0f; // 0x8CCEA8

//! How far above the ground the arrest cameras sit. `ARRESTDIST_ABOVE_GROUND` @ 0x8CC7F8
static constexpr auto ARRESTDIST_ABOVE_GROUND = 0.7f;

//! Index into `gCamColVars` used whenever a camera wants the "player outside, medium range" collision set
static constexpr auto CAM_COL_VARS_PLAYER_OUTSIDE_MED_RANGE = 5;


enum {
    ZOOM_ONE   = 1,
    ZOOM_TWO   = 2,
    ZOOM_THREE = 3
};

constexpr auto AIMWEAPON_FOV_ZOOM_RATE          = 1.0f;       // 0x862F1C
constexpr auto AIMWEAPON_STICK_SENS             = 0.007f;     // 0x8CC4A0
constexpr auto SWIM_CAM_ALPHA_FORCE             = 0.5f;       // 0x862F34
constexpr auto HEADING_TOWARD_PLAYER_BETA_LIMIT = 2.96706f;   // 170 deg
constexpr auto HEADING_TOWARD_PLAYER_FOR_ALPHA  = HALF_PI;
constexpr auto HEADING_TOWARD_PLAYER_ALPHA_MAX  = 0.349066f; // 20 deg
constexpr auto HEADING_TOWARD_PLAYER_ALPHA_RATE = 0.9f;
constexpr auto SPEED_TOL                        = 0.0001f;
constexpr auto SWIM_CAM_ALPHA_EXTRA = DegreesToRadians(-15.0f); // 0x862F38
constexpr auto JETPACK_CAM_ALPHA_FORCE = 3.0f;       // 0x862F3C
constexpr auto JETPACK_CAM_ALPHA_EXTRA = DegreesToRadians(-20.0f); // 0x862F40
static float   gLastCamDist = 1.0f; // 0xB6EC50;
static bool   gForceCamBehindPlayer = false; // 0xB6EC54;
static CVector vecPedPosEst(0.0f, 0.0f, 10000.0f); // 0x8CCC3C
static CVector vecPedPosTrend(0.0f, 0.0f, 0.0f); // 0xB6EC7C


//! Tuning for the follow-ped cameras. `PEDCAM_SET` @ 0x8CC548 -- writable in the original,
//! but only ever changed through the (stripped) debug console.
struct CamFollowPedData {
    float fTargetOffsetZ;    //!< Where on the entity to aim
    float fBaseCamDist;      //!< Base distance of the camera behind the target
    float fBaseCamZ;         //!< Base height of the camera above the target
    float fMinDist;          //!< Min distance the camera is drawn from the ped
    float fMinFollowDist;    //!< Min distance used to follow the ped
    float fDiffAlphaRate;
    float fDiffAlphaCap;
    float fDiffAlphaSwing;
    float fDiffBetaRate;     //!< How fast the camera swings around behind the player
    float fDiffBetaCap;
    float fDiffBetaSwing;
    float fDiffBetaSwingCap;
    float fStickMult;        //!< Tweak for how fast the right stick moves the camera around
    float fUpLimit;          //!< m_fVerticalAngle angle limits
    float fDownLimit;
};

enum eFollowPedCamSet {
    FOLLOW_PED_OUTSIDE = 0,
    FOLLOW_PED_INSIDE,
    FOLLOW_PED_2PLAYER,

    FOLLOW_PED_NUMSETTINGS
};

static constexpr CamFollowPedData PEDCAM_SET[FOLLOW_PED_NUMSETTINGS]{
//    targZ  baseDst baseZ  minDist minFol aRate aCap  aSwing bRate bCap  bSwing bSwingCap stickM upLimit                  downLimit
    { 0.6f,  2.0f,  0.15f,  2.0f,   4.0f,  0.8f, 0.1f, 0.5f,  0.8f, 0.1f, 0.1f,  0.02f,    1.0f,  DegreesToRadians(45.0f), DegreesToRadians(85.0f) }, // On foot
    { 0.6f,  2.0f,  0.15f,  2.0f,   3.0f,  0.9f, 0.1f, 1.0f,  0.8f, 0.1f, 0.3f,  0.05f,    1.0f,  DegreesToRadians(45.0f), DegreesToRadians(45.0f) }, // Interior
    { 0.6f,  2.0f,  0.15f,  2.0f,   4.0f,  0.8f, 0.1f, 0.5f,  0.8f, 0.1f, 0.1f,  0.02f,    1.0f,  DegreesToRadians(45.0f), DegreesToRadians(85.0f) }, // 2 player
};

enum eFollowCarCamSet {
    FOLLOW_CAR_INCAR = 0,
    FOLLOW_CAR_ONBIKE,
    FOLLOW_CAR_INHELI,
    FOLLOW_CAR_INPLANE,
    FOLLOW_CAR_INBOAT,
    FOLLOW_CAR_RCCAR,
    FOLLOW_CAR_RCHELI,

    FOLLOW_CAR_NUMSETTINGS
};

//! Tuning for the follow-car cameras. `CARCAM_SET` @ 0x8CC600
static constexpr CamFollowPedData CARCAM_SET[FOLLOW_CAR_NUMSETTINGS]{
//    targZ  baseDst baseZ  minDist minFol aRate  aCap   aSwing bRate  bCap   bSwing  bSwingCap stickM upLimit                  downLimit
    { 1.3f,  1.0f,  0.40f,  10.0f,  15.0f, 0.5f,  1.0f,  1.0f,  0.85f, 0.2f,  0.075f, 0.05f,    0.80f, DegreesToRadians(45.0f), DegreesToRadians(89.0f) }, // Car
    { 1.1f,  1.0f,  0.10f,  10.0f,  11.0f, 0.5f,  1.0f,  1.0f,  0.85f, 0.2f,  0.075f, 0.05f,    0.75f, DegreesToRadians(45.0f), DegreesToRadians(89.0f) }, // Bike
    { 1.1f,  1.0f,  0.20f,  10.0f,  15.0f, 0.05f, 0.05f, 0.0f,  0.9f,  0.05f, 0.01f,  0.05f,    1.0f,  DegreesToRadians(10.0f), DegreesToRadians(70.0f) }, // Heli
    { 1.1f,  3.5f,  0.20f,  10.0f,  25.0f, 0.5f,  1.0f,  1.0f,  0.75f, 0.1f,  0.005f, 0.20f,    1.0f,  DegreesToRadians(89.0f), DegreesToRadians(89.0f) }, // Plane
    { 1.3f,  1.0f,  0.40f,  10.0f,  15.0f, 0.5f,  1.0f,  0.0f,  0.9f,  0.05f, 0.005f, 0.05f,    1.0f,  DegreesToRadians(20.0f), DegreesToRadians(70.0f) }, // Boat
    { 1.1f,  1.0f,  0.20f,  10.0f,   5.0f, 0.5f,  1.0f,  1.0f,  0.75f, 0.1f,  0.005f, 0.20f,    1.0f,  DegreesToRadians(45.0f), DegreesToRadians(89.0f) }, // RC car
    { 1.1f,  1.0f,  0.20f,  10.0f,   5.0f, 0.5f,  1.0f,  1.0f,  0.75f, 0.1f,  0.005f, 0.20f,    1.0f,  DegreesToRadians(20.0f), DegreesToRadians(70.0f) }, // RC heli
};

//! Used by `Process_Follow_History` and `Process_FollowCar_SA` too
static constexpr auto CAR_FOV_START_SPEED = 0.4f;  // 0x8CC540
static constexpr auto CAR_FOV_FADE_MULT   = 0.98f; // 0x8CC544

static inline auto& gArrestCamInUse         = StaticRef<eArrestCam>(0xB6EC58); // `nUsingWhichCamera`
static inline auto& gStoredCopPed           = StaticRef<CPed*>(0xB6EC5C);
static inline auto& gTimeDWBustedCamStarted = StaticRef<float>(0xB6EC60);

static inline auto& DWCineyCamLastNearClip = StaticRef<float>(0xB6EC08);
static inline auto& DWCineyCamLastFov = StaticRef<float>(0xB6EC0C);

//! Wrap an angle into [-pi, pi)
// 0x509BE0
static void MakeAngleLessThan180(float& angle) {
    for (; angle >= PI; angle -= TWO_PI) {
        ;
    }
    for (; angle < -PI; angle += TWO_PI) {
        ;
    }
}

//! `CGeneral::GetRandomTrueFalse` - always inlined as a bare `rand() < 0x3FFF`
static bool GetRandomTrueFalse() {
    return CGeneral::GetRandomNumber() < 0x3FFF;
}

//! `DW_SINE_ACCEL_DECEL_LERP` - lerp from `from` to `to` easing in and out. Always inlined.
static float SineAccelDecelLerp(float t, float from, float to) {
    return from + (std::sin(DegreesToRadians(270.0f - t * 180.0f)) + 1.0f) * 0.5f * (to - from);
}

//! Sine wave over the [start, end] window, `numWaves` full cycles across it. Always inlined.
static float WaveFunc(uint32 curTime, uint32 startTime, uint32 endTime, int32 numWaves) {
    const auto t = (float)(curTime - startTime) / (float)(endTime - startTime);
    return std::sin(DegreesToRadians(t * (float)numWaves * 360.0f));
}

// 0x509AE0
static void WellBufferMe(float target, float& valueToChange, float& speedSoFar, float topSpeed, float speedStep, bool isAnAngle) {
    const auto valueToTargetDiff = [&] {
        auto d = target - valueToChange;
        if (isAnAngle) {
            for (; d >= DegreesToRadians(180.0f); d -= DegreesToRadians(360.0f)) {
                ;
            }
            for (; d < DegreesToRadians(-180.0f); d += DegreesToRadians(360.0f)) {
                ;
            }
        }
        return d;
    }();

    const auto fullSpeedStep = valueToTargetDiff * topSpeed;
    speedSoFar += std::abs(std::abs(fullSpeedStep - speedSoFar) * CTimer::GetTimeStep() * speedStep);

    if (fullSpeedStep >= 0.0f || fullSpeedStep <= speedSoFar) {
        if (fullSpeedStep > 0.0f && fullSpeedStep < speedSoFar) {
            speedSoFar = fullSpeedStep;
        }
    } else {
        speedSoFar = fullSpeedStep;
    }

    valueToChange += std::min(CTimer::GetTimeStep(), 10.0f) * speedSoFar;
}

// 0x50A0A0
//! Rounds `f` to `numDecimals` decimals, halves away from zero
static float FTrunc(float f, int32 numDecimals) {
    const auto scaled = std::pow(10.0, (double)(numDecimals + 1)) * (double)f + (f < 0.0f ? -5.0 : 5.0);

    double whole;
    std::modf(scaled * 0.1, &whole);

    return (float)(whole / std::pow(10.0, (double)numDecimals));
}

// 0x50A120
static void VecTrunc(CVector& v, int32 numDecimals = 4) {
    v.x = FTrunc(v.x, numDecimals);
    v.y = FTrunc(v.y, numDecimals);
    v.z = FTrunc(v.z, numDecimals);
}

void CCam::InjectHooks() {
    RH_ScopedClass(CCam);
    RH_ScopedCategory("Camera");

    RH_ScopedInstall(Constructor, 0x517730);
    RH_ScopedInstall(Init, 0x50E490);
    RH_ScopedInstall(CacheLastSettingsDWCineyCam, 0x50D7A0);
    RH_ScopedInstall(DoCamBump, 0x50CB30);
    RH_ScopedInstall(Finalise_DW_CineyCams, 0x50DD70);
    RH_ScopedInstall(GetCoreDataForDWCineyCamMode, 0x517130);
    RH_ScopedInstall(GetLookFromLampPostPos, 0x5161A0);
    RH_ScopedInstall(GetVectorsReadyForRW, 0x509CE0);
    RH_ScopedInstall(Get_TwoPlayer_AimVector, 0x513E40);
    RH_ScopedInstall(IsTimeToExitThisDWCineyCamMode, 0x517400);
    RH_ScopedInstall(KeepTrackOfTheSpeed, 0x509DF0);
    RH_ScopedInstall(GetBoatLook_L_R_HeightOffset, 0x509CA0);
    RH_ScopedInstall(GetLookAlongGroundPos, 0x516010);
    RH_ScopedInstall(GetLookOverShoulderPos, 0x515D80);
    RH_ScopedInstall(ProcessDWBustedCam1, 0x512EF0);
    RH_ScopedInstall(LookBehind, 0x520690);
    RH_ScopedInstall(LookRight, 0x520E40);
    RH_ScopedInstall(RotCamIfInFrontCar, 0x50A4F0);
    RH_ScopedInstall(Using3rdPersonMouseCam, 0x50A850);
    RH_ScopedInstall(Process, 0x526FC0);
    RH_ScopedInstall(ProcessArrestCamOne, 0x518500);
    RH_ScopedInstall(ProcessPedsDeadBaby, 0x519250);
    RH_ScopedInstall(Process_1rstPersonPedOnPC, 0x50EB70);
    RH_ScopedInstall(Process_1stPerson, 0x517EA0);
    RH_ScopedInstall(Process_AimWeapon, 0x521500);
    RH_ScopedInstall(Process_AttachedCam, 0x512B10);
    RH_ScopedInstall(Process_Cam_TwoPlayer, 0x525E50);
    RH_ScopedInstall(Process_Cam_TwoPlayer_TestLOSs, 0x513220);
    RH_ScopedInstall(Process_Cam_TwoPlayer_CalcSource, 0x5132D0);
    RH_ScopedInstall(Process_Cam_TwoPlayer_InCarAndShooting, 0x519810);
    RH_ScopedInstall(Process_Cam_TwoPlayer_Separate_Cars, 0x513510);
    RH_ScopedInstall(Process_Cam_TwoPlayer_Separate_Cars_TopDown, 0x513BE0);
    RH_ScopedInstall(Process_DW_BirdyCam, 0x51B850);
    RH_ScopedInstall(Process_DW_CamManCam, 0x51B120);
    RH_ScopedInstall(Process_DW_HeliChaseCam, 0x51A740);
    RH_ScopedInstall(Process_DW_PlaneCam1, 0x51C760);
    RH_ScopedInstall(Process_DW_PlaneCam2, 0x51CC30);
    RH_ScopedInstall(Process_DW_PlaneCam3, 0x51D100);
    RH_ScopedInstall(Process_DW_PlaneSpotterCam, 0x51C250);
    RH_ScopedInstall(Process_Editor, 0x50F3F0);
    RH_ScopedInstall(Process_Fixed, 0x51D470);
    RH_ScopedInstall(Process_FlyBy, 0x5B25F0);
    RH_ScopedInstall(Process_FollowCar_SA, 0x5245B0);
    RH_ScopedInstall(Process_FollowPedWithMouse, 0x50F970);
    RH_ScopedInstall(Process_FollowPed_SA, 0x522D40);
    RH_ScopedInstall(Process_M16_1stPerson, 0x5105C0);
    RH_ScopedInstall(Process_Rocket, 0x511B50);
    RH_ScopedInstall(Process_SpecialFixedForSyphon, 0x517500);
    RH_ScopedInstall(Process_WheelCam, 0x512110);

    RH_ScopedInstall(ClipAlpha, 0x509BE0);
    RH_ScopedInstall(ClipBeta, 0x509C50);
    RH_ScopedInstall(GetWeaponFirstPersonOn, 0x509DC0);
    RH_ScopedGlobalInstall(MakeAngleLessThan180, 0x509BE0);
    RH_ScopedGlobalInstall(IsLampPost, 0x509A30);
    RH_ScopedGlobalInstall(WellBufferMe, 0x509AE0);
    RH_ScopedGlobalInstall(FTrunc, 0x50A0A0);
    RH_ScopedGlobalInstall(VecTrunc, 0x50A120);
}

// 0x517730
CCam::CCam() {
    Init();
}

// 0x50E490
void CCam::Init() {
    m_vecFront = CVector(0, 0, -1);
    m_vecUp = CVector(0, 0, 1);
    m_nMode = eCamMode::MODE_FOLLOWPED;
    m_bRotating = false;
    m_nDoCollisionChecksOnFrameNum = 1;
    m_nDoCollisionCheckEveryNumOfFrames = 9;
    m_nFrameNumWereAt = 0;
    m_bCollisionChecksOn = true;
    m_fRealGroundDist = 0.0f;
    m_fBetaSpeed = 0.0f;
    m_fAlphaSpeed = 0.0f;
    m_fCameraHeightMultiplier = 0.75;
    m_fMaxRoleAngle = DegreesToRadians(20.0f);
    m_fDistance = 30.0f;
    m_fDistanceSpeed = 0.0f;
    m_pLastCarEntered = nullptr;
    m_pLastPedLookedAt = nullptr;
    m_bResetStatics = true;
    m_fHorizontalAngle = 0.0f;
    m_fTilt = 0.0f;
    m_fTiltSpeed = 0.0f;
    m_bFixingBeta = false;
    m_fCaMinDistance = 0.0f;
    m_fCaMaxDistance = 0.0f;
    m_bLookingBehind = false;
    m_bLookingLeft = false;
    m_bLookingRight = false;
    m_fPlayerInFrontSyphonAngleOffSet = DegreesToRadians(20.0f);
    m_fSyphonModeTargetZOffSet = 0.5f;
    m_fRadiusForDead = 1.5f;
    m_nDirectionWasLooking = 3; // TODO: enum
    m_bLookBehindCamWasInFront = 0;
    m_fRoll = 0.0f;
    m_fRollSpeed = 0.0f;
    m_fCloseInPedHeightOffset = 0.0f;
    m_fCloseInPedHeightOffsetSpeed = 0.0f;
    m_fCloseInCarHeightOffset = 0.0f;
    m_fCloseInCarHeightOffsetSpeed = 0.0f;
    m_fPedBetweenCameraHeightOffset = 0.0f;
    m_fTargetBeta = 0.0f;
    m_fBufferedTargetBeta = 0.0f;
    m_fBufferedTargetOrientation = 0.0f;
    m_fBufferedTargetOrientationSpeed = 0.0f;
    m_fDimensionOfHighestNearCar = 0.0;
    m_fBeta_Targeting = 0.0f;
    m_fX_Targetting = 0.0f;
    m_fY_Targetting = 0.0f;
    m_nCarWeAreFocussingOn = 0;
    m_fCarWeAreFocussingOnI = 0.0f;
    m_fCamBumpedHorz = 1.0f;
    m_fCamBumpedVert = 0.0f;
    m_nCamBumpedTime = 0;
    for (int i = 0; i < 4; ++i) {
        m_anTargetHistoryTime[i] = 0;
        m_avecTargetHistoryPos[i] = CVector{};
    }
    m_nCurrentHistoryPoints = 0;
    gPlayerPedVisible = true;
    gbCineyCamMessageDisplayed = 2; // TODO: enum
    gCameraDirection = 3; // TODO: enum
    gCameraMode = (eCamMode)-1;
    gLastTime2PlayerCameraWasOK = 0;
    gLastTime2PlayerCameraCollided = 0;
    TheCamera.m_bCinemaCamera = false;
}

// 0x50D7A0
void CCam::CacheLastSettingsDWCineyCam() {
    DWCineyCamLastUp       = m_vecUp;
    DWCineyCamLastFwd      = m_vecFront;
    DWCineyCamLastRight    = CrossProduct(m_vecFront, m_vecUp);
    DWCineyCamLastFov      = m_fFOV;
    DWCineyCamLastNearClip = RwCameraGetNearClipPlane(Scene.m_pRwCamera);
    DWCineyCamLastPos      = m_vecSource;
}

// 0x50CB30
void CCam::DoCamBump(float horizontal, float vertical) {
    m_fCamBumpedHorz = horizontal;
    m_fCamBumpedVert = vertical;
    m_nCamBumpedTime = CTimer::GetTimeInMS();
}

// 0x50DD70
void CCam::Finalise_DW_CineyCams(const CVector& src, const CVector& dest, float roll, float fov, float nearClip, float shakeDegree) {
    m_vecFront  = (dest - src).Normalized();
    m_vecSource = src;

    // What is this thing?
    {
        auto rightDir = m_vecFront.Cross({ std::sin(roll), 0.0f, std::cos(roll) }).Normalized();
        m_vecUp       = rightDir.Cross(m_vecFront);
        if (m_vecFront.x == 0.0f && m_vecFront.y == 0.0f) {
            m_vecFront.x = m_vecFront.y = 0.0001f;
        }
        rightDir = CrossProduct(m_vecFront, m_vecUp).Normalized();
        m_vecUp  = CrossProduct(rightDir, m_vecFront);
    }

    m_fFOV = fov;
    RwCameraSetNearClipPlane(Scene.m_pRwCamera, 0.4f); // meant to use nearClip here?
    CacheLastSettingsDWCineyCam();
    gLastFrameProcessedDWCineyCam = CTimer::GetFrameCounter();

    gHandShaker[0].Process(shakeDegree);
    m_vecFront = gHandShaker[0].m_resultMat.InverseTransformVector(m_vecFront); // Multiply3x3(front, resultMat) @ 0x59C810
    m_vecFront.Normalise();

    {
        auto rightDir = m_vecFront.Cross({ std::sin(roll), 0.0f, std::cos(roll) }).Normalized();
        m_vecUp       = rightDir.Cross(m_vecFront);
        if (m_vecFront.x == 0.0f && m_vecFront.y == 0.0f) {
            m_vecFront.x = m_vecFront.y = 0.0001f;
        }
        rightDir = m_vecFront.Cross(m_vecUp).Normalized();
        m_vecUp  = rightDir.Cross(m_vecFront);
    }
}

// 0x517130
void CCam::GetCoreDataForDWCineyCamMode(
    CEntity*& entity,
    CVehicle*& vehicle,
    CVector& dest,
    CVector& src,
    CVector& targetUp,
    CVector& targetRight,
    CVector& targetFwd,
    CVector& targetVel,
    float& targetSpeed,
    CVector& targetAngVel,
    float& targetAngSpeed,
    CColSphere& colSphere
) {
    entity         = m_pCamTargetEntity;
    vehicle        = entity->AsVehicle();
    dest           = entity->GetPosition();
    src            = DWCineyCamLastPos;
    targetUp       = entity->GetUpVector();
    targetRight    = entity->GetRightVector();
    targetFwd      = entity->GetForwardVector();
    targetVel      = entity->AsPhysical()->GetMoveSpeed();
    targetSpeed    = targetVel.Magnitude();
    targetAngVel   = entity->AsPhysical()->GetTurnSpeed();
    targetAngSpeed = targetAngVel.Magnitude();

    colSphere.Set(
        entity->GetModelInfo()->GetColModel()->GetBoundRadius(),
        entity->GetBoundCentre(),
        eSurfaceType::SURFACE_DEFAULT
    );
}

// 0x5161A0
bool CCam::GetLookFromLampPostPos(CEntity* target, CPed* cop, const CVector& vecTarget, CVector& vecSource) {
    //! Ideal distance between the target and the lamp post we pick. Read-only global @ 0x8CC8D8.
    constexpr auto BEST_DIST = 17.0f;

    // Objects and dummies only - we're really only after lamp posts and traffic lights
    std::array<CEntity*, 16> inRange{};
    int16 numInRange{};
    CWorld::FindObjectsInRange(vecTarget, 30.0f, true, &numInRange, 15, inRange.data(), false, false, false, true, true);

    CEntity* found = nullptr;
    auto bestDist = 10000.0f;

    for (auto* const e : std::span{ inRange.data(), (size_t)numInRange }) {
        if (!e->GetIsStatic() || e->GetMatrix().GetUp().z <= 0.9f || !IsLampPost((eModelID)e->GetModelIndex())) {
            continue;
        }

        const auto dist = CVector2D{ e->GetPosition() - vecTarget }.Magnitude();
        if (dist <= 5.0f || std::abs(BEST_DIST - dist) >= bestDist) {
            continue;
        }

        // Look from the top of the post back down towards the target
        const auto losStart  = e->GetMatrix().TransformPoint(e->GetColModel()->GetBoundingBox().m_vecMax);
        const auto losTarget = (losStart - vecTarget).Normalized() + vecTarget;

        if (CWorld::GetIsLineOfSightClear(losStart, losTarget, true, false, false, false, false, true, true)) {
            bestDist  = std::abs(BEST_DIST - dist);
            found     = e;
            vecSource = losStart;
        }
    }

    return found != nullptr;
}

// 0x509CE0
void CCam::GetVectorsReadyForRW() {
    m_vecFront.Normalise();
    if (m_vecFront.x == 0.0f && m_vecFront.y == 0.0f) {
        m_vecFront.x = m_vecFront.y = 0.0001f;
    }
    const auto a = CrossProduct(m_vecFront, { 0.0f, 0.0f, 1.0f }).Normalized();
    m_vecUp = CrossProduct(a, m_vecFront);
}

// 0x513E40 -- not tested
void CCam::Get_TwoPlayer_AimVector(CVector& out) {
    const auto player = [&] {
        auto* p1 = FindPlayerPed(PED_TYPE_PLAYER1);
        if (p1->m_pVehicle && !p1->m_pVehicle->IsDriver(p1)) {
            return FindPlayerPed(PED_TYPE_PLAYER2);
        }
        return p1;
    }();

    const auto weaponInfo = player->GetActiveWeapon().GetWeaponInfo(player);
    const auto nearestTargetEntityInScreen = CWeapon::FindNearestTargetEntityWithScreenCoors(
        m_fX_Targetting,
        m_fY_Targetting,
        2 * weaponInfo.m_fWeaponRange,
        player->GetPosition()
    );

    if (nearestTargetEntityInScreen) {
        out = nearestTargetEntityInScreen->GetPosition() - m_vecSource;
    } else {
        const auto right  = CrossProduct(m_vecFront, m_vecUp);
        const auto tanFov = std::tan(m_fFOV * PI / 360.0f);

        out = m_fX_Targetting * m_fY_Targetting * tanFov * right + m_vecFront - tanFov / CDraw::ms_fAspectRatio * m_vecUp;
    }
    out.Normalise();
}

//! `t` is passed by every caller but never read
// 0x517400
bool CCam::IsTimeToExitThisDWCineyCamMode(int32 camId, const CVector& src, const CVector& dst, float t, bool lineOfSightCheck) {
    // Nothing else in the game reads these; the originals are read-only tables in .data
    static constexpr float MIN_DIST[]{   3.f,   3.f,   1.f,   3.f,  5.f,  3.f,   3.f,   3.f,   3.f }; // 0x8CCBCC
    static constexpr float MAX_DIST[]{ 185.f, 100.f, 100.f, 100.f, 30.f, 30.f, 100.f, 100.f, 100.f }; // 0x8CCBF0

    const auto idx = camId - DW_CINEYCAM_FIRST_ID;

    if (gbExitCam[idx]) {
        return true;
    }

    const auto dist           = (dst - src).Magnitude();
    const auto isDistInLimits = dist >= MIN_DIST[idx] && dist <= MAX_DIST[idx];

    auto isLineOfSightClear = true;
    if (lineOfSightCheck) {
        CColPoint cp;
        CEntity*  hitEntity;

        CWorld::pIgnoreEntity  = m_pCamTargetEntity;
        isLineOfSightClear     = !CWorld::ProcessLineOfSight(dst, src, cp, hitEntity, true, true, false, false, false, false, false, false);
        CWorld::pIgnoreEntity  = nullptr;
    }

    // Anything outside the DW cineycam range never expires
    if (camId < DW_CINEYCAM_FIRST_ID || camId > DW_CINEYCAM_FIRST_ID + 8) {
        return false;
    }

    return !isDistInLimits
        || !isLineOfSightClear
        || CTimer::GetTimeInMS() > gDWCineyCamEndTime;
}

// 0x509DF0
void CCam::KeepTrackOfTheSpeed(const CVector& source, const CVector& targetToLookAt, const CVector& up, const float& trueAlpha, const float& trueBeta, const float& fov) {
    // Nothing outside this function ever touches these, so we own them instead of
    // aliasing the original's memory (the init guard there is a bitfield at 0xB6FF8C)
    static CVector prevSource = source;         // 0xB6FF80
    static CVector prevTarget = targetToLookAt; // 0xB6FF74
    static CVector prevUp     = up;             // 0xB6FF68
    static float   prevBeta   = trueBeta;       // 0xB6FF64
    static float   prevAlpha  = trueAlpha;      // 0xB6FF60
    static float   prevFov    = fov;            // 0xB6FF5C

    if (TheCamera.m_bJust_Switched) {
        prevSource = source;
        prevTarget = targetToLookAt;
        prevUp     = up;
    }

    m_vecSourceSpeedOverOneFrame = source - prevSource;
    m_vecTargetSpeedOverOneFrame = targetToLookAt - prevTarget;
    m_vecUpOverOneFrame          = up - prevUp;

    m_fFovSpeedOverOneFrame = fov - prevFov;

    m_fBetaSpeedOverOneFrame = trueBeta - prevBeta;
    MakeAngleLessThan180(m_fBetaSpeedOverOneFrame);

    m_fAlphaSpeedOverOneFrame = trueAlpha - prevAlpha;
    MakeAngleLessThan180(m_fAlphaSpeedOverOneFrame);

    prevSource = source;
    prevTarget = targetToLookAt;
    prevUp     = up;
    prevBeta   = trueBeta;
    prevAlpha  = trueAlpha;
    prevFov    = fov;
}

//! How far the 1st person camera drops when the vehicle is a boat
static constexpr auto ZOFFSET_1RSTPERSON_BOAT = 0.5f;

//! The three "look" modes that put the camera on a string behind a vehicle
bool CCam::IsCamOnAStringMode() const {
    return m_nMode == MODE_CAM_ON_A_STRING || m_nMode == MODE_BEHINDBOAT || m_nMode == MODE_BEHINDCAR;
}

// 0x509CA0
bool CCam::GetBoatLook_L_R_HeightOffset(float& outHeightOffset) const {
    if (!m_pCamTargetEntity) {
        return false;
    }
    const auto* const handling = gHandlingDataMgr.GetBoatPointer(
        (uint8)m_pCamTargetEntity->GetModelInfo()->AsVehicleModelInfoPtr()->m_nHandlingId
    );
    if (!handling) {
        return false;
    }
    outHeightOffset = handling->m_fLookLRBehindCamHeight;
    return true;
}

// 0x520690
bool CCam::LookBehind() {
    const auto isCarCam       = IsCamOnAStringMode() && m_pCamTargetEntity->GetIsTypeVehicle();
    const auto isPed          = m_pCamTargetEntity->GetIsTypePed();
    const auto isFirstPersonCar = m_nMode == MODE_1STPERSON && m_pCamTargetEntity->GetIsTypeVehicle();

    if (!isCarCam && !isFirstPersonCar && !isPed) {
        return false;
    }

    auto targetCoors = m_pCamTargetEntity->GetPosition();
    m_vecFront       = m_pCamTargetEntity->GetPosition() - m_vecSource;

    if (isCarCam) {
        targetCoors      = gTargetCoordsForLookingBehind;
        m_bLookingBehind = true;

        const auto groundDist = m_nMode == MODE_CAM_ON_A_STRING
            ? m_fCaMaxDistance
            : 15.5f; // In a boat

        m_vecSource = m_pCamTargetEntity->GetForwardVector();
        m_vecSource.z += 0.2f;
        m_vecSource = targetCoors + groundDist * m_vecSource;

        auto colTarget = targetCoors;

        CWorld::pIgnoreEntity = m_pCamTargetEntity;
        TheCamera.CameraGenericModeSpecialCases(nullptr);
        TheCamera.CameraVehicleModeSpecialCases(m_pCamTargetEntity->AsVehicle());
        TheCamera.CameraColDetAndReact(&m_vecSource, &colTarget);

        m_vecFront = m_pCamTargetEntity->GetPosition() - m_vecSource;
        GetVectorsReadyForRW();

        TheCamera.ImproveNearClip(m_pCamTargetEntity->AsVehicle(), nullptr, &m_vecSource, &colTarget);
        CWorld::pIgnoreEntity = nullptr;
    }

    if (isFirstPersonCar) {
        m_bLookingBehind = true;

        // For occasions when the spaz user is right up against a wall etc
        RwCameraSetNearClipPlane(Scene.m_pRwCamera, 0.05f);

        auto* const veh = m_pCamTargetEntity->AsVehicle();

        m_vecFront = m_pCamTargetEntity->GetForwardVector();
        m_vecFront.Normalise();

        if (veh->IsBoat()) {
            m_vecSource.z -= ZOFFSET_1RSTPERSON_BOAT;
        }

        switch (veh->GetVehicleAppearance()) {
        case VEHICLE_APPEARANCE_BIKE: {
            m_vecSource += 2.3f * m_vecFront; // Hand-tuned in the original, not derived from the bike
            m_vecFront   = -m_vecFront;
            GetVectorsReadyForRW();
            break;
        }
        case VEHICLE_APPEARANCE_HELI: {
            m_vecFront   = -1.0f * m_pCamTargetEntity->GetUpVector();
            m_vecUp      = m_pCamTargetEntity->GetForwardVector();
            m_vecSource += 0.25f * m_vecFront;
            break;
        }
        default: {
            m_vecSource += 0.25f * m_vecFront;
            m_vecFront   = -m_vecFront;
            break;
        }
        }
    }

    if (isPed) {
        constexpr auto EXTRA_PED_LOOK_BACK_DIST = 2.0f;                                       // 0x8CCE4C
        constexpr float LOOK_BACK_NEAR_Z_MOD_SOURCE[]{ 0.6f, 0.0f, 0.0f };                    // 0x8CCE40
        constexpr float LOOK_BACK_NEAR_Z_MOD_TARGET[]{ 0.6f, 0.6f, 0.6f };                    // 0x8CCE34
        constexpr float SCALE_WHEN_LOOKING_BACK_SWIMMING[]{ -1.0f, -1.0f, -1.0f };            // 0x8CCE28
        constexpr float EXTRA_HEIGHT_WHEN_LOOKING_BACK_SWIMMING[]{ 0.0f, 1.0f, 1.0f };        // 0x8CCE1C

        auto* const ped = m_pCamTargetEntity->AsPed();

        // Straight back along beta, tilted off whatever the ped is standing on
        m_vecSource    = CVector{ -std::cos(m_fHorizontalAngle), -std::sin(m_fHorizontalAngle), 0.0f };
        m_vecSource.z += 0.3f - DotProduct(ped->m_vecGroundNormal, m_vecSource);
        m_vecSource.Normalise();
        m_vecSource = std::max(0.6f, TheCamera.m_fPedZoomSmoothed + EXTRA_PED_LOOK_BACK_DIST) * m_vecSource + targetCoors;

        const auto zoomIdx = TheCamera.m_nPedZoom - 1;
        const auto zDeltaSrc = LOOK_BACK_NEAR_Z_MOD_SOURCE[zoomIdx];
        const auto zDeltaDst = LOOK_BACK_NEAR_Z_MOD_TARGET[zoomIdx];

        if (ped->GetIntelligence()->GetTaskSwim()) { // Swimming needs the camera in front, not behind
            m_vecSource    = targetCoors + (targetCoors - m_vecSource) * SCALE_WHEN_LOOKING_BACK_SWIMMING[zoomIdx];
            m_vecSource.z += EXTRA_HEIGHT_WHEN_LOOKING_BACK_SWIMMING[zoomIdx];
        }

        // Aim a bit higher than the player's crotch once collision pushes the camera in
        m_vecSource.z += zDeltaSrc;
        targetCoors.z += zDeltaDst;

        TheCamera.HandleCameraMotionForDucking(ped, &m_vecSource, &targetCoors, false);

        TheCamera.CameraGenericModeSpecialCases(ped);
        TheCamera.CameraPedModeSpecialCases();
        TheCamera.CameraColDetAndReact(&m_vecSource, &targetCoors);

        m_vecFront = targetCoors - m_vecSource;
        GetVectorsReadyForRW();

        TheCamera.ImproveNearClip(nullptr, ped, &m_vecSource, &targetCoors);

        // Stops the player running into the camera when looking behind at the closest zoom
        if (TheCamera.m_nPedZoom == 1 && RwCameraGetNearClipPlane(Scene.m_pRwCamera) > 0.05f) {
            RwCameraSetNearClipPlane(Scene.m_pRwCamera, 0.05f);
        }
    }

    GetVectorsReadyForRW();
    return true;
}

// 0x520E40
bool CCam::LookRight(bool bLookRight) {
    const auto isCarCam         = IsCamOnAStringMode() && m_pCamTargetEntity->GetIsTypeVehicle();
    const auto isPed            = m_pCamTargetEntity->GetIsTypePed();
    const auto isFirstPersonCar = m_nMode == MODE_1STPERSON && m_pCamTargetEntity->GetIsTypeVehicle();

    auto sideSign = 1.0f;
    if (bLookRight) {
        m_bLookingRight = true;
    } else {
        m_bLookingLeft = true;
        sideSign       = -1.0f;
    }

    if (isCarCam) {
        auto targetCoors = m_pCamTargetEntity->GetPosition();

        const auto groundDist = [&] {
            if (m_nMode == MODE_CAM_ON_A_STRING) {
                return m_fCaMaxDistance;
            }
            if (m_nMode == MODE_BEHINDBOAT) { // Boats sit the camera at a handling-defined height
                auto boatHeightOffset = 0.0f;
                if (GetBoatLook_L_R_HeightOffset(boatHeightOffset) && !CCullZones::Cam1stPersonForPlayer()) {
                    m_vecSource.z = targetCoors.z + boatHeightOffset;
                }
            }
            return 9.0f;
        }();

        const auto carForward = m_pCamTargetEntity->GetForwardVector().Normalized();
        const auto beta       = CGeneral::GetATanOfXY(carForward.x, carForward.y) + sideSign * HALF_PI;

        m_vecSource.x = targetCoors.x + groundDist * std::cos(beta);
        m_vecSource.y = targetCoors.y + groundDist * std::sin(beta);

        const auto& bb = m_pCamTargetEntity->GetColModel()->GetBoundingBox();
        const auto originalHeight = m_vecSource.z;

        auto colTarget = targetCoors;

        CWorld::pIgnoreEntity = m_pCamTargetEntity;
        TheCamera.CameraGenericModeSpecialCases(nullptr);
        TheCamera.CameraVehicleModeSpecialCases(m_pCamTargetEntity->AsVehicle());
        TheCamera.CameraColDetAndReact(&m_vecSource, &colTarget);
        CWorld::pIgnoreEntity = nullptr;

        // Keep the camera above the side of the vehicle it looks over, but never raise it
        auto sideHeight = m_pCamTargetEntity->GetPosition();
        sideHeight.z += m_pCamTargetEntity->GetRightVector().z * (bLookRight ? bb.m_vecMin.x : bb.m_vecMax.x);
        sideHeight.z += m_pCamTargetEntity->GetUpVector().z * bb.m_vecMax.z;

        const auto limitHeight = std::min(originalHeight, std::max(m_vecTargetCoorsForFudgeInter.z, sideHeight.z) + 0.1f);
        m_vecSource.z = std::max(limitHeight, m_vecSource.z);

        // Overwritten again later on by the caller, but the original computes it anyway
        m_vecFront    = m_pCamTargetEntity->GetPosition() - m_vecSource;
        m_vecFront.z += 1.1f;
        if (m_nMode == MODE_BEHINDBOAT) {
            m_vecFront.z += 1.2f;
        }

        GetVectorsReadyForRW();
        return true;
    }

    if (isFirstPersonCar) {
        if (bLookRight) {
            m_bLookingRight = true;
        } else {
            m_bLookingLeft = true;
        }

        // For occasions when the spaz user is right up against a wall etc
        RwCameraSetNearClipPlane(Scene.m_pRwCamera, 0.05f);

        auto* const veh = m_pCamTargetEntity->AsVehicle();

        if (veh->IsBoat()) {
            if (auto* const driver = veh->m_pDriver) { // Look over the driver's shoulder
                constexpr auto BOAT_1STPERSON_LR_OFFSETZ = 0.2f; // 0x8CC494
                constexpr auto BOAT_1STPERSON_L_OFFSETX  = 0.7f; // 0x8CC498
                constexpr auto BOAT_1STPERSON_R_OFFSETX  = 0.3f; // 0x8CC49C

                CVector shoulderPos{};
                driver->SetPedPositionInCar();
                driver->UpdateRwMatrix();
                driver->UpdateRwFrame();
                driver->UpdateRpHAnim();
                driver->GetBonePosition(&shoulderPos, BONE_NECK, true);

                shoulderPos += (bLookRight ? BOAT_1STPERSON_L_OFFSETX : BOAT_1STPERSON_R_OFFSETX) * veh->GetRightVector();
                shoulderPos += BOAT_1STPERSON_LR_OFFSETZ * veh->GetUpVector();
                m_vecSource  = shoulderPos;
            } else {
                m_vecSource.z -= ZOFFSET_1RSTPERSON_BOAT;
            }
        }

        if (!veh->IsBike()) {
            m_vecSource -= 0.35f * veh->GetRightVector();
        }

        m_vecUp = veh->GetUpVector();
        m_vecUp.Normalise();
        m_vecFront = veh->GetForwardVector();
        m_vecFront.Normalise();

        m_vecFront = bLookRight
            ? CrossProduct(m_vecFront, m_vecUp)
            : CrossProduct(m_vecUp, m_vecFront);
        m_vecFront.Normalise();

        if (veh->GetVehicleAppearance() == VEHICLE_APPEARANCE_BIKE) {
            m_vecSource -= 1.45f * m_vecFront;
        }
        return true;
    }

    // Peds fall through with nothing to do, but still count as handled
    return isPed;
}

// 0x50A4F0
bool CCam::RotCamIfInFrontCar(const CVector& targetCoors, float targetOrientation) {
    if (!m_pCamTargetEntity->GetIsTypeVehicle()) {
        return false;
    }
    auto* const veh = m_pCamTargetEntity->AsVehicle();

    // The original guards these with an `if (heli || plane)` that is commented out,
    // so every vehicle gets the aircraft tuning.
    constexpr auto acceptableRange = DegreesToRadians(160.0f);
    constexpr auto betaTopSpeed    = 0.1f;
    constexpr auto betaSpeedStep   = 0.003f;

    const auto isGoingForward = DotProduct(veh->GetMatrix().GetForward(), veh->m_vecMoveSpeed) > 0.1f;

    { // Aim along the direction of travel once we're moving fast enough horizontally
        auto heliDir = veh->m_vecMoveSpeed;
        heliDir.z    = 0.0f;
        if (heliDir.SquaredMagnitude() > sq(0.06f)) {
            targetOrientation = std::atan2(-heliDir.x, heliDir.y) - HALF_PI;
        }
    }

    const auto distMagnitude = CVector2D{ m_vecSource - targetCoors }.Magnitude();

    // Not `MakeAngleLessThan180` - this one is written out inline with a strict `>` bound
    const auto WrapDeltaBeta = [](float& d) {
        for (; d > PI; d -= TWO_PI) {
            ;
        }
        for (; d < -PI; d += TWO_PI) {
            ;
        }
    };

    auto deltaBeta = targetOrientation - m_fHorizontalAngle;
    WrapDeltaBeta(deltaBeta);

    if (std::abs(deltaBeta) > PI - acceptableRange && isGoingForward && !TheCamera.m_bTransitionState) {
        m_bFixingBeta = true;
    }

    // Just came back from looking behind/left/right - snap the camera behind the car
    const auto* const pad = CPad::GetPad(0);
    if (!pad->GetLookBehindForCar() && !pad->GetLookBehindForPed() && !pad->GetLookLeft() && !pad->GetLookRight()) {
        if (m_nDirectionWasLooking != eLookingDirection::LOOKING_FORWARD) {
            TheCamera.m_bCamDirectlyBehind = true;
        }
    }

    if (!m_bFixingBeta && !TheCamera.m_bUseTransitionBeta && !TheCamera.m_bCamDirectlyBehind && !TheCamera.m_bCamDirectlyInFront) {
        return false;
    }

    const auto gotToRotateAndIsSafe = (TheCamera.m_bCamDirectlyBehind || TheCamera.m_bCamDirectlyInFront || TheCamera.m_bUseTransitionBeta)
                                   && &TheCamera.GetActiveCam() == this;

    if (m_bFixingBeta || gotToRotateAndIsSafe) { // Get to the target orientation quickly
        WellBufferMe(targetOrientation, m_fHorizontalAngle, m_fBetaSpeed, betaTopSpeed, betaSpeedStep, true);

        if (&TheCamera.GetActiveCam() == this) {
            if (TheCamera.m_bCamDirectlyBehind) {
                m_fHorizontalAngle = targetOrientation;
            }
            if (TheCamera.m_bCamDirectlyInFront) {
                m_fHorizontalAngle = targetOrientation + PI;
            }
            if (TheCamera.m_bUseTransitionBeta) {
                m_fHorizontalAngle = m_fTransitionBeta;
            }
        }

        const CVector2D frontVec{
            distMagnitude * -std::cos(m_fHorizontalAngle),
            distMagnitude * -std::sin(m_fHorizontalAngle)
        };
        m_vecSource.x = targetCoors.x - frontVec.x;
        m_vecSource.y = targetCoors.y - frontVec.y;

        deltaBeta = targetOrientation - m_fHorizontalAngle;
        WrapDeltaBeta(deltaBeta);

        if (std::abs(deltaBeta) < DegreesToRadians(2.0f)) { // Happy once we're within 2 degrees of beta
            m_bFixingBeta = false;
        }
    }

    TheCamera.m_bCamDirectlyBehind = false;
    TheCamera.m_bCamDirectlyInFront = false;
    return true;
}

// 0x50A850
bool CCam::Using3rdPersonMouseCam() const {
    return CCamera::m_bUseMouse3rdPerson && m_nMode == MODE_FOLLOWPED;
}

// 0x509DC0
bool CCam::GetWeaponFirstPersonOn() {
    return m_pCamTargetEntity && m_pCamTargetEntity->GetIsTypePed() && m_pCamTargetEntity->AsPed()->GetActiveWeapon().m_IsFirstPersonWeaponModeSelected;
}

// 0x509BE0 -- alpha = vertical angle
void CCam::ClipAlpha() {
    while (m_fVerticalAngle >= TWO_PI) {
        m_fVerticalAngle -= TWO_PI;
    }
    while (m_fVerticalAngle < 0.0f) {
        m_fVerticalAngle += TWO_PI;
    }
}

// 0x509C50 -- beta = horizontal angle
void CCam::ClipBeta() {
    if (m_fHorizontalAngle > PI) {
        m_fHorizontalAngle -= TWO_PI;
    } else if (m_fHorizontalAngle < -PI) {
        m_fHorizontalAngle += TWO_PI;
    }
}

// 0x526FC0
void CCam::Process() {
    //! Speed at which `SpeedVar` hits its maximum zoom-out. `CAMTD_SPEEDMAXZOOMOUT`
    constexpr auto SPEED_MAX_ZOOM_OUT = 0.9f;

    //! Linear-exponential (Holt) smoothing weights for the player ped's tracked position
    constexpr auto PLAYERPED_LEVEL_SMOOTHING_CONST_INV = 0.6f; // 0x8CC394
    constexpr auto PLAYERPED_TREND_SMOOTHING_CONST_INV = 0.8f; // 0x8CC398
    constexpr auto PLAYERFIGHT_LEVEL_SMOOTHING_CONST   = 0.9f; // 0x8CC39C

    //! `Process_FollowPedWithMouse` is dead code while this stays set
    static auto& USE_FOLLOWPED_SA_WITH_MOUSE = StaticRef<bool>(0x8CCF00);

    //! Smoothed estimate of the player ped's position, and its estimated velocity
    static auto& vecPedPosEst   = StaticRef<CVector>(0x8CCC3C);
    static auto& vecPedPosTrend = StaticRef<CVector>(0xB6EC7C);

    gIdleCam.IdleCamGeneralProcess(); // Must run, otherwise interesting events keep firing

    if (!m_pCamTargetEntity) {
        m_pCamTargetEntity = TheCamera.m_pTargetEntity;
        m_pCamTargetEntity->RegisterReference(&m_pCamTargetEntity);
    }

    // Heat-seeking missiles drop their reticle at once; ordinary weapon targeting fades out.
    // The Hydra makes that crosshair itself, so leave it alone.
    if (gCrossHair[0].m_bClearImmediately) {
        const auto* const player = FindPlayerPed();
        if (!player || !player->m_pVehicle || player->m_pVehicle->m_nModelIndex != MODEL_HYDRA) {
            CWeaponEffects::ClearCrossHairImmediately(0);
        }
    }

    // Spread the line-of-sight work over several frames
    if (++m_nFrameNumWereAt > m_nDoCollisionCheckEveryNumOfFrames) {
        m_nFrameNumWereAt = 1;
    }
    m_bCollisionChecksOn = m_nFrameNumWereAt == m_nDoCollisionChecksOnFrameNum;

    CVector thisCamsTarget{};
    auto    targetOrientation = 0.0f;
    auto    speedVarDesired   = 0.0f;

    //! m_fHorizontalAngle right behind the target
    const auto CalcTargetOrientation = [this] {
        const auto fwd = m_pCamTargetEntity->GetForwardVector();
        return fwd.x == 0.0f && fwd.y == 0.0f
            ? 0.0f
            : CGeneral::GetATanOfXY(fwd.x, fwd.y);
    };

    if (m_bCamLookingAtVector) {
        thisCamsTarget = m_vecCamFixedModeVector;
    } else if (m_pCamTargetEntity->GetIsTypeVehicle()) {
        thisCamsTarget    = m_pCamTargetEntity->GetPosition();
        targetOrientation = CalcTargetOrientation();

        // Most camera modes react to how fast the player is going, so take only the
        // speed component along the car
        auto fwd2d = CVector{ m_pCamTargetEntity->GetForwardVector().x, m_pCamTargetEntity->GetForwardVector().y, 0.0f };
        fwd2d.Normalise();
        if (const auto len = fwd2d.Magnitude2D(); len != 0.0f) {
            fwd2d.x /= len;
            fwd2d.y /= len;
        }

        const auto& moveSpeed = m_pCamTargetEntity->AsPhysical()->m_vecMoveSpeed;
        const auto  speedX    = fwd2d.x * moveSpeed.x;
        const auto  speedY    = fwd2d.y * moveSpeed.y;
        const auto  speed     = std::sqrt(sq(speedX) + sq(speedY));

        speedVarDesired = speedX + speedY > 0.0f
            ? std::min(1.0f, speed / SPEED_MAX_ZOOM_OUT)
            : -std::min(0.5f, speed / (SPEED_MAX_ZOOM_OUT * 2.0f));

        constexpr auto MULTIPLIER = 0.895f;
        m_fSpeedVar = (1.0f - MULTIPLIER) * speedVarDesired + MULTIPLIER * m_fSpeedVar;

        const auto* const pad = CPad::GetPad(0);
        if (m_nDirectionWasLooking != LOOKING_FORWARD
            && (!pad->GetLookBehindForCar() || pad->GetLookLeft() || pad->GetLookRight())
        ) {
            TheCamera.m_bCamDirectlyBehind = true;
        }
    } else {
        if (m_pCamTargetEntity == FindPlayerPed()) {
            // Smooth the target position while aiming at the player ped, using Holt's method
            auto targetPedPos = FindPlayerPed()->GetPosition();
            if (auto* const climb = FindPlayerPed()->GetIntelligence()->GetTaskClimb()) {
                climb->GetCameraTargetPos(FindPlayerPed(), targetPedPos);
            }

            CVector newPosEst, newTrendEst;
            if ((vecPedPosEst - targetPedPos).SquaredMagnitude() > 9.0f // Ped teleported, or time misbehaved
                || CTimer::GetTimeStep() < 0.2f
                || Using3rdPersonMouseCam() // Looks bad while strafing
                || TheCamera.m_bCamDirectlyBehind
                || TheCamera.m_bCamDirectlyInFront
            ) {
                newPosEst   = targetPedPos;
                newTrendEst = CVector{};
            } else if (FindPlayerPed()->GetIntelligence()->GetTaskFighting() && m_nMode == MODE_AIMWEAPON) {
                const auto levelSmooth = std::pow(PLAYERFIGHT_LEVEL_SMOOTHING_CONST, CTimer::GetTimeStep());
                newPosEst   = levelSmooth * vecPedPosEst + (1.0f - levelSmooth) * targetPedPos;
                newTrendEst = CVector{};
            } else {
                const auto levelSmooth = 1.0f - std::pow(PLAYERPED_LEVEL_SMOOTHING_CONST_INV, CTimer::GetTimeStep());
                const auto trendSmooth = 1.0f - std::pow(PLAYERPED_TREND_SMOOTHING_CONST_INV, CTimer::GetTimeStep());

                // Weighted average of where the ped actually is and where last frame said it would be
                newPosEst   = levelSmooth * targetPedPos + (1.0f - levelSmooth) * (vecPedPosEst + vecPedPosTrend * CTimer::GetTimeStep());
                newPosEst.z = targetPedPos.z;

                newTrendEst   = trendSmooth * (newPosEst - vecPedPosEst) / std::max(1.0f, CTimer::GetTimeStep())
                              + (1.0f - trendSmooth) * vecPedPosTrend;
                newTrendEst.z = 0.0f;
            }

            thisCamsTarget = newPosEst;
            vecPedPosEst   = newPosEst;
            vecPedPosTrend = newTrendEst;
        } else {
            thisCamsTarget = m_pCamTargetEntity->GetPosition();
        }

        targetOrientation = CalcTargetOrientation();
        m_fSpeedVar = speedVarDesired = 0.0f;
    }

    m_nDirectionWasLooking = gCameraDirection;
    gCameraDirection       = LOOKING_FORWARD;

    // Work out the look-behind state before running the camera itself, which always runs
    if (&TheCamera.GetActiveCam() == this) {
        const auto notTransitioning = !TheCamera.m_bTransitionState;

        const auto AbortInterpolation = [] {
            TheCamera.m_bTransitionState       = false;
            TheCamera.m_bDoingSpecialInterp    = false;
            TheCamera.m_bWaitForInterpolToFinish = false;
        };

        const auto* const pad = CPad::GetPad(0);

        if ((IsCamOnAStringMode() || m_nMode == MODE_1STPERSON) && m_pCamTargetEntity->GetIsTypeVehicle()) {
            // L2/R2 are flight controls on aircraft, so no look left/right there
            const auto appearance      = m_pCamTargetEntity->AsVehicle()->GetVehicleAppearance();
            const auto ignoreLeftRight = appearance == VEHICLE_APPEARANCE_HELI || appearance == VEHICLE_APPEARANCE_PLANE;

            auto lookingAt = LOOKING_FORWARD;
            if (pad->GetLookBehindForCar()) {
                AbortInterpolation();
                lookingAt = LOOKING_BEHIND;
            } else if (pad->GetLookLeft() && !ignoreLeftRight) {
                AbortInterpolation();
                lookingAt = LOOKING_LEFT;
            } else if (pad->GetLookRight() && !ignoreLeftRight) {
                AbortInterpolation();
                lookingAt = LOOKING_RIGHT;
            }

            if (m_nDirectionWasLooking != lookingAt) {
                TheCamera.m_bJust_Switched = true;
            }
            gCameraDirection = lookingAt;
        } else if (m_nMode == MODE_FOLLOWPED && m_pCamTargetEntity->GetIsTypePed()) {
            // Look behind is only available in follow-ped mode for a ped
            if (pad->GetLookBehindForPed()) {
                if (m_nDirectionWasLooking != LOOKING_BEHIND && notTransitioning) {
                    TheCamera.m_bJust_Switched = true;
                }
                gCameraDirection = LOOKING_BEHIND;
            } else {
                if (m_nDirectionWasLooking != LOOKING_FORWARD) {
                    gCurDistForCam = 1.0f;
                }
                gCameraDirection = LOOKING_FORWARD;
            }
        } else if (m_nMode == MODE_AIMWEAPON) {
            if (m_nDirectionWasLooking != LOOKING_FORWARD) {
                gCurDistForCam = 1.0f;
            }
            gCameraDirection = LOOKING_FORWARD;
        }
    }

    // Just switched, so it's safe to push the camera back out to the furthest position
    if (TheCamera.m_bJust_Switched) {
        gCurDistForCam = 1.0f;
        TheCamera.m_bResetOldMatrix = true;
    }

    if (m_nMode != MODE_BEHINDCAR && m_nMode != MODE_CAM_ON_A_STRING && m_nMode != MODE_BEHINDBOAT
        && m_nMode != MODE_1STPERSON && m_nMode != MODE_TWOPLAYER_IN_CAR_AND_SHOOTING
    ) {
        CPostEffects::m_bSpeedFXUserFlagCurrentFrame = false;
    }

    gbFirstPersonRunThisFrame = false;

    switch (m_nMode) {
    case MODE_BEHINDCAR:
    case MODE_CAM_ON_A_STRING:
    case MODE_BEHINDBOAT:
        Process_FollowCar_SA(thisCamsTarget, targetOrientation, m_fSpeedVar, speedVarDesired, false);
        break;
    case MODE_FOLLOWPED:
        if (CCamera::m_bUseMouse3rdPerson && !USE_FOLLOWPED_SA_WITH_MOUSE) {
            Process_FollowPedWithMouse(thisCamsTarget, targetOrientation, m_fSpeedVar, speedVarDesired);
        } else {
            Process_FollowPed_SA(thisCamsTarget, targetOrientation, m_fSpeedVar, speedVarDesired, false);
        }
        break;
    case MODE_SNIPER:
    case MODE_M16_1STPERSON:
    case MODE_HELICANNON_1STPERSON:
    case MODE_CAMERA:
        Process_M16_1stPerson(thisCamsTarget, targetOrientation, m_fSpeedVar, speedVarDesired);
        break;
    case MODE_ROCKETLAUNCHER:
        Process_Rocket(thisCamsTarget, targetOrientation, m_fSpeedVar, speedVarDesired, false);
        break;
    case MODE_ROCKETLAUNCHER_HS:
        Process_Rocket(thisCamsTarget, targetOrientation, m_fSpeedVar, speedVarDesired, true);
        break;
    case MODE_WHEELCAM:
        Process_WheelCam(thisCamsTarget, targetOrientation, m_fSpeedVar, speedVarDesired);
        break;
    case MODE_FIXED:
        Process_Fixed(thisCamsTarget, targetOrientation, m_fSpeedVar, speedVarDesired);
        break;
    case MODE_1STPERSON:
        Process_1stPerson(thisCamsTarget, targetOrientation, m_fSpeedVar, speedVarDesired);
        break;
    case MODE_FLYBY:
        Process_FlyBy(thisCamsTarget, targetOrientation, m_fSpeedVar, speedVarDesired);
        break;
    case MODE_PED_DEAD_BABY:
        ProcessPedsDeadBaby();
        TheCamera.m_bPlayerIsInGarage    = false; // Stops mad shit happening when killed in a garage
        TheCamera.m_bJustCameOutOfGarage = false;
        break;
    case MODE_ARRESTCAM_ONE:
        ProcessArrestCamOne();
        break;
    case MODE_ARRESTCAM_TWO:
        break; // `ProcessArrestCamTwo` never made it into the game
    case MODE_SPECIAL_FIXED_FOR_SYPHON:
        Process_SpecialFixedForSyphon(thisCamsTarget, targetOrientation, m_fSpeedVar, speedVarDesired);
        break;
    case MODE_SNIPER_RUNABOUT:
    case MODE_ROCKETLAUNCHER_RUNABOUT:
    case MODE_1STPERSON_RUNABOUT:
    case MODE_M16_1STPERSON_RUNABOUT:
    case MODE_FIGHT_CAM_RUNABOUT:
    case MODE_ROCKETLAUNCHER_RUNABOUT_HS:
        Process_1rstPersonPedOnPC(thisCamsTarget, targetOrientation, m_fSpeedVar, speedVarDesired);
        break;
    case MODE_EDITOR:
        Process_Editor(thisCamsTarget, targetOrientation, m_fSpeedVar, speedVarDesired);
        break;
    case MODE_ATTACHCAM:
        Process_AttachedCam();
        break;
    case MODE_TWOPLAYER:
        Process_Cam_TwoPlayer();
        break;
    case MODE_TWOPLAYER_IN_CAR_AND_SHOOTING:
        Process_Cam_TwoPlayer_InCarAndShooting();
        break;
    case MODE_TWOPLAYER_SEPARATE_CARS:
        Process_Cam_TwoPlayer_Separate_Cars();
        break;
    case MODE_TWOPLAYER_SEPARATE_CARS_TOPDOWN:
        Process_Cam_TwoPlayer_Separate_Cars_TopDown();
        break;
    case MODE_AIMWEAPON:
    case MODE_AIMWEAPON_FROMCAR:
    case MODE_AIMWEAPON_ATTACHED:
        Process_AimWeapon(thisCamsTarget, targetOrientation, m_fSpeedVar, speedVarDesired);
        break;
    case MODE_DW_HELI_CHASE:
        Process_DW_HeliChaseCam(false);
        break;
    case MODE_DW_CAM_MAN:
        Process_DW_CamManCam(false);
        break;
    case MODE_DW_BIRDY:
        Process_DW_BirdyCam(false);
        break;
    case MODE_DW_PLANE_SPOTTER:
        Process_DW_PlaneSpotterCam(false);
        break;
    case MODE_DW_DOG_FIGHT:
    case MODE_DW_FISH:
        // Both cams were cut; all that survives is the flag their bodies used to clear
        TheCamera.m_bUseNearClipScript = false;
        break;
    case MODE_DW_PLANECAM1:
        Process_DW_PlaneCam1(false);
        break;
    case MODE_DW_PLANECAM2:
        Process_DW_PlaneCam2(false);
        break;
    case MODE_DW_PLANECAM3:
        Process_DW_PlaneCam3(false);
        break;
    default: // Invalid camera type - just pick any coordinates
        m_vecSource = CVector{ 0.0f, 0.0f, 0.0f };
        m_vecFront  = CVector{ 0.0f, 1.0f, 0.0f };
        m_vecUp     = CVector{ 0.0f, 0.0f, 1.0f };
        break;
    }

    if (m_nMode < MODE_DW_HELI_CHASE || m_nMode > MODE_DW_PLANECAM3) {
        gDWLastModeForCineyCam = (eCamMode)-1;
    }
    gCameraMode = m_nMode;

    const auto forBetaAlpha = m_vecSource - m_vecTargetCoorsForFudgeInter;
    m_fTrueBeta  = CGeneral::GetATanOfXY(forBetaAlpha.x, forBetaAlpha.y);
    m_fTrueAlpha = CGeneral::GetATanOfXY(forBetaAlpha.Magnitude2D(), forBetaAlpha.z);

    if (!TheCamera.m_bTransitionState) {
        KeepTrackOfTheSpeed(m_vecSource, m_vecTargetCoorsForFudgeInter, m_vecUp, m_fTrueAlpha, m_fTrueBeta, m_fFOV);
    }

    m_bLookingBehind = m_bLookingLeft = m_bLookingRight = false;
    m_vecSourceBeforeLookBehind = m_vecSource;

    if (&TheCamera.GetActiveCam() == this) {
        switch (gCameraDirection) {
        case LOOKING_BEHIND: LookBehind();     break;
        case LOOKING_LEFT:   LookRight(false); break;
        case LOOKING_RIGHT:  LookRight(true);  break;
        default:                               break;
        }
        m_nDirectionWasLooking = gCameraDirection;
    }

    if (TheCamera.m_bFOVLerpProcessed) {
        m_fFOV = TheCamera.m_fFOVNew;
        TheCamera.m_bFOVLerpProcessed = false;
    }

    if (TheCamera.m_bVecMoveLinearProcessed) {
        m_vecSource = TheCamera.m_vecMoveLinear;
        TheCamera.m_bVecMoveLinearProcessed = false;
    }

    if (TheCamera.m_bVecTrackLinearProcessed) {
        m_vecFront = TheCamera.m_vecTrackLinear - m_vecSource;
        m_vecFront.Normalise();
        GetVectorsReadyForRW();
        TheCamera.m_bVecTrackLinearProcessed = false;
    }

    // A nan/inf here poisons `m_mCameraMatrix` and everything reading it (audio, culling, ...) - break here instead, `m_nMode` says which cam did it
    assert(!m_vecSource.HasNanOrInf() && !m_vecFront.HasNanOrInf() && !m_vecUp.HasNanOrInf());
}

// 0x515D80
bool CCam::GetLookOverShoulderPos(CEntity* target, CPed* cop, const CVector& vecTarget, CVector& vecSource) {
    constexpr auto ARRESTDIST_RIGHTOF_COP   = 3.0f; // 0x8CC8CC
    constexpr auto ARRESTDIST_BEHIND_COP    = 5.0f; // 0x8CC8C8
    constexpr auto ARRESTDIST_MINFROM_PLAYER = 8.0f; // 0x8CC8D4

    if (!target || !cop) {
        return false;
    }

    auto copPos      = cop->GetPosition();
    auto copToPlayer = vecTarget - copPos;

    copPos += CrossProduct(copToPlayer, CVector{ 0.0f, 0.0f, 1.0f }).Normalized() * ARRESTDIST_RIGHTOF_COP;

    // Keep the look direction between level and 45 degrees down
    copToPlayer.Normalise();
    if (copToPlayer.z < -0.7071f) {
        copToPlayer.z = -0.7071f;
        if (const auto scale2d = copToPlayer.Magnitude2D() / 0.7071f; scale2d > 0.0f) {
            copToPlayer.x /= scale2d;
            copToPlayer.y /= scale2d;
        }
        copToPlayer.Normalise();
    } else if (copToPlayer.z > 0.0f) {
        copToPlayer.z = 0.0f;
        copToPlayer.Normalise();
    }

    copPos -= ARRESTDIST_BEHIND_COP * copToPlayer;

    copToPlayer = vecTarget - copPos;
    if (const auto dist = copToPlayer.Magnitude(); dist < ARRESTDIST_MINFROM_PLAYER && dist > 0.0f) {
        copToPlayer *= ARRESTDIST_MINFROM_PLAYER / dist;
    }

    vecSource = vecTarget - copToPlayer;
    return true;
}

// 0x516010
bool CCam::GetLookAlongGroundPos(CEntity* target, CPed* cop, const CVector& vecTarget, CVector& vecSource) {
    constexpr auto ARRESTDIST_ALONG_GROUND = 5.0f;  // 0x8CC7F0
    constexpr auto ARRESTDIST_SIDE_GROUND  = 10.0f; // 0x8CC7F4

    if (!target || !cop) {
        return false;
    }

    auto copToPlayer = vecTarget - cop->GetPosition();
    copToPlayer.z    = 0.0f;
    copToPlayer.Normalise();

    vecSource   = vecTarget + ARRESTDIST_ALONG_GROUND * copToPlayer;
    vecSource  += ARRESTDIST_SIDE_GROUND * CrossProduct(copToPlayer, CVector{ 0.0f, 0.0f, 1.0f });
    vecSource.z = vecTarget.z + 5.0f;

    auto groundFound = false;
    const auto groundZ = CWorld::FindGroundZFor3DCoord(vecSource, &groundFound);
    if (groundFound) {
        vecSource.z = groundZ + ARRESTDIST_ABOVE_GROUND;
    }
    return true;
}

// 0x512EF0
bool CCam::ProcessDWBustedCam1(CPed* cop, bool isFirstTime) {
    constexpr auto TIME_TO_START_MOTION = 0.0f;    // 0xB70024
    constexpr auto TIME_TO_APPLY_MOTION = 1000.0f; // 0x8CCCB0
    constexpr auto HEAD_Z_OFFSET        = -0.06f;  // 0x8CCCAC
    constexpr auto T_SCALE              = 0.5f;    // 0x8CCCA8
    const auto     OFFSET_POS_AT_2      = CVector{ 0.0f, 0.0f, -0.5f }; // 0xB70004

    static auto& bKickOffBustedText = StaticRef<bool>(0xB70020);

    if ((isFirstTime && CGeneral::GetRandomNumberInRange(0.0f, 1.0f) > 0.65f)
        || !TheCamera.m_pTargetEntity->GetIsTypePed()
        || !cop
    ) {
        return false;
    }

    const auto t = std::clamp(
        ((float)CTimer::GetTimeInMS() - gTimeDWBustedCamStarted - TIME_TO_START_MOTION) / TIME_TO_APPLY_MOTION,
        0.0f,
        1.0f
    );
    bKickOffBustedText = !(t >= 1.0f && bKickOffBustedText);

    const auto lastSource = m_vecSource;

    m_vecSource = TheCamera.m_pTargetEntity->GetPosition() + OFFSET_POS_AT_2;
    m_fFOV      = 100.0f; // Les wanted the m_fFOV lerp removed

    // Look at the cop's head, drifting down as `t` runs out
    const auto  hier    = GetAnimHierarchyFromSkinClump(cop->GetRpClump());
    const auto& headMat = RpHAnimHierarchyGetMatrixArray(hier)[RpHAnimIDGetIndex(hier, BONE_HEAD)];

    auto lookAt = *RwMatrixGetPos(&headMat);
    lookAt.z   += HEAD_Z_OFFSET - t * T_SCALE;

    m_vecFront = lookAt - m_vecSource;
    m_vecFront.Normalise();
    m_vecUp = CVector{ 0.0f, 0.0f, 1.0f };
    m_vecUp = CrossProduct(CrossProduct(m_vecFront, m_vecUp).Normalized(), m_vecFront);

    if (isFirstTime) {
        // The original's dangling `else` binds to this inner `if`, so the fallback below
        // only ever runs when the first check passed
        if (!CWorld::GetIsLineOfSightClear(m_vecSource, lookAt, true, true, false, true, false, false, true)) {
            return false;
        }
        if (!CWorld::GetIsLineOfSightClear(m_vecSource, lookAt, true, false, false, true, false, false, false)) {
            m_vecSource = lastSource; // Don't go through walls
        }
    }

    // The player gets made visible again on a level restart, so keep hiding them
    TheCamera.m_pTargetEntity->SetIsVisible(false);
    return true;
}

// 0x518500
void CCam::ProcessArrestCamOne() {
    constexpr auto ARRESTCAM_ROTATION_SPEED      = 0.1f;  // 0x8CC7E4
    constexpr auto ARRESTCAM_ROTATION_UP         = 0.05f; // 0x8CC7E8
    constexpr auto ARRESTCAM_S_ROTATION_UP       = 0.1f;  // 0x8CC7EC
    constexpr auto ARRESTCAM_LAMPPOST_ROTATEDIST = 10.0f; // 0x8CC7FC
    constexpr auto ARRESTCAM_LAMPPOST_TRANSLATE  = 0.1f;  // 0x8CC800

    //! Recompute front/up so the camera looks at `target` with a level horizon
    const auto AimAt = [this](const CVector& target) {
        m_vecFront = target - m_vecSource;
        m_vecFront.Normalise();
        m_vecUp = CVector{ 0.0f, 0.0f, 1.0f };
        m_vecUp = CrossProduct(CrossProduct(m_vecFront, m_vecUp).Normalized(), m_vecFront);
    };

    m_fFOV = 45.0f;

    CVector playerPos{};

    if (!m_bResetStatics) {
        if (gArrestCamInUse == ARRESTCAM_DW) {
            TheCamera.m_pTargetEntity->SetIsVisible(false);

            CPed* cop = nullptr;
            if (auto* const player = FindPlayerPed(); player && player->GetPlayerData()->m_pArrestingCop) {
                cop = reinterpret_cast<CPed*>(player->GetPlayerData()->m_pArrestingCop);
            }
            ProcessDWBustedCam1(cop, false);
            return;
        }

        if (TheCamera.m_pTargetEntity->GetIsTypePed()) {
            TheCamera.m_pTargetEntity->AsPed()->GetBonePosition(&playerPos, BONE_SPINE1, true);
        } else if (TheCamera.m_pTargetEntity->GetIsTypeVehicle()) {
            auto* const driver = TheCamera.m_pTargetEntity->AsVehicle()->m_pDriver;
            if (driver && driver->IsPlayer()) {
                driver->GetBonePosition(&playerPos, BONE_SPINE1, true);
            } else {
                playerPos = TheCamera.m_pTargetEntity->GetPosition();
            }
        } else {
            return;
        }

        auto arrestCamPos  = m_vecSource;
        auto gotCameraPos  = false;

        if (gArrestCamInUse == ARRESTCAM_OVERSHOULDER && gStoredCopPed) {
            gotCameraPos = GetLookOverShoulderPos(TheCamera.m_pTargetEntity, gStoredCopPed, playerPos, arrestCamPos);
            // Limit how fast this camera can rotate up once it's running
            arrestCamPos.z = std::min(arrestCamPos.z, m_vecSource.z + ARRESTCAM_S_ROTATION_UP * CTimer::GetTimeStep());
        } else if (gArrestCamInUse > ARRESTCAM_ALONGGROUND && gArrestCamInUse <= ARRESTCAM_ALONGGROUND_LEFT_UP) {
            arrestCamPos = m_vecSource;
            AimAt(playerPos);

            auto right = CrossProduct(m_vecFront, CVector{ 0.0f, 0.0f, 1.0f }).Normalized();
            if (gArrestCamInUse == ARRESTCAM_ALONGGROUND_LEFT || gArrestCamInUse == ARRESTCAM_ALONGGROUND_LEFT_UP) {
                right *= -1.0f; // Rotating the other way is just a flipped right vector
            }

            if (!CWorld::TestSphereAgainstWorld(m_vecSource + 0.5f * right, 0.4f, TheCamera.m_pTargetEntity, true, true, false, true, false, true)) {
                gotCameraPos  = true;
                arrestCamPos += right * ARRESTCAM_ROTATION_SPEED * CTimer::GetTimeStep();

                if (gArrestCamInUse == ARRESTCAM_ALONGGROUND_RIGHT_UP || gArrestCamInUse == ARRESTCAM_ALONGGROUND_LEFT_UP) {
                    arrestCamPos.z += ARRESTCAM_ROTATION_UP * CTimer::GetTimeStep();
                } else { // Otherwise stay a constant distance above the ground
                    auto groundFound = false;
                    const auto groundZ = CWorld::FindGroundZFor3DCoord(arrestCamPos, &groundFound);
                    if (groundFound) {
                        arrestCamPos.z = groundZ + ARRESTDIST_ABOVE_GROUND;
                    }
                }
            }
        } else if (gArrestCamInUse == ARRESTCAM_FROMLAMPPOST) {
            arrestCamPos = m_vecSource;

            m_vecFront   = playerPos - arrestCamPos;
            m_vecFront.z = 0.0f;
            m_vecFront.Normalise();

            m_vecUp    = CVector{ 0.0f, 0.0f, 1.0f };
            const auto right = CrossProduct(m_vecFront, m_vecUp).Normalized();

            m_vecFront   = (playerPos - arrestCamPos) + right * ARRESTCAM_LAMPPOST_ROTATEDIST;
            m_vecFront.z = 0.0f;
            m_vecFront.Normalise();

            if (!CWorld::TestSphereAgainstWorld(arrestCamPos + 0.5f * m_vecFront, 0.4f, TheCamera.m_pTargetEntity, true, true, false, true, false, true)) {
                gotCameraPos  = true;
                arrestCamPos += m_vecFront * ARRESTCAM_LAMPPOST_TRANSLATE * CTimer::GetTimeStep();
            }
        }

        if (gotCameraPos) {
            m_vecSource = arrestCamPos;
        }
        const auto sourceBeforeChange = m_vecSource;
        TheCamera.AvoidTheGeometry(sourceBeforeChange, playerPos, m_vecSource, m_fFOV);

        if (gotCameraPos) {
            AimAt(playerPos);
        }
        return;
    }

    // First frame - pick which of the arrest cameras to use
    gArrestCamInUse = ARRESTCAM_NONE;

    CPed*    handyPed = nullptr;
    CPed*    handyCop = nullptr;
    std::array<eArrestCam, NUM_ARREST_CAMS> tryList{}; // ARRESTCAM_NONE means "stop here"

    if (TheCamera.m_pTargetEntity->GetIsTypePed()) {
        handyPed = TheCamera.m_pTargetEntity->AsPed();
        handyPed->GetBonePosition(&playerPos, BONE_SPINE1, true);

        if (auto* const player = FindPlayerPed(); player && player->GetPlayerData()->m_pArrestingCop) {
            handyCop = reinterpret_cast<CPed*>(player->GetPlayerData()->m_pArrestingCop);
        }

        tryList = handyCop && CGeneral::GetRandomNumberInRange(0.0f, 1.0f) > 0.5f
            ? std::array{ ARRESTCAM_DW, ARRESTCAM_OVERSHOULDER, ARRESTCAM_ALONGGROUND, ARRESTCAM_OVERSHOULDER, ARRESTCAM_FROMLAMPPOST, ARRESTCAM_NONE }
            : std::array{ ARRESTCAM_DW, ARRESTCAM_ALONGGROUND, ARRESTCAM_OVERSHOULDER, ARRESTCAM_FROMLAMPPOST, ARRESTCAM_NONE, ARRESTCAM_NONE };
    } else if (TheCamera.m_pTargetEntity->GetIsTypeVehicle()) {
        auto* const driver = TheCamera.m_pTargetEntity->AsVehicle()->m_pDriver;
        if (driver && driver->IsPlayer()) {
            handyPed = driver;
            handyPed->GetBonePosition(&playerPos, BONE_SPINE1, true);
        } else {
            playerPos = TheCamera.m_pTargetEntity->GetPosition();
        }

        if (auto* const player = FindPlayerPed(); player && player->GetPlayerData()->m_pArrestingCop) {
            handyCop = reinterpret_cast<CPed*>(player->GetPlayerData()->m_pArrestingCop);
        }

        tryList = handyCop && CGeneral::GetRandomNumberInRange(0.0f, 1.0f) > 0.65f
            ? std::array{ ARRESTCAM_OVERSHOULDER, ARRESTCAM_FROMLAMPPOST, ARRESTCAM_ALONGGROUND, ARRESTCAM_OVERSHOULDER, ARRESTCAM_NONE, ARRESTCAM_NONE }
            : std::array{ ARRESTCAM_FROMLAMPPOST, ARRESTCAM_ALONGGROUND, ARRESTCAM_OVERSHOULDER, ARRESTCAM_NONE, ARRESTCAM_NONE, ARRESTCAM_NONE };
    } else { // Camera target isn't a ped or a vehicle
        return;
    }

    if (!CHud::m_BigMessage[STYLE_WHITE_MIDDLE][0]) {
        CMessages::AddBigMessage(TheText.Get("BUSTED"), 5000, STYLE_WHITE_MIDDLE);
    }

    //! Look for a cop nearby that is arresting the player
    const auto FindArrestingCopNearby = [&](auto&& getPos, CVector& outPos) {
        for (auto* const e : handyPed->GetIntelligence()->GetPedScanner().m_apEntities) {
            if (!e) {
                continue;
            }
            auto* const nearbyPed = e->AsPed();
            auto* const arrest    = static_cast<CTaskSimpleArrestPed*>(
                nearbyPed->GetIntelligence()->GetTaskManager().FindActiveTaskByType(TASK_SIMPLE_ARREST_PED)
            );
            if (!arrest || FindPlayerPed() != arrest->m_Ped) {
                continue;
            }
            if (getPos(nearbyPed, outPos)) {
                gStoredCopPed = nearbyPed;
                return true;
            }
        }
        return false;
    };

    auto arrestCamPos = CVector{};

    for (auto tryCam = 0u; gArrestCamInUse == ARRESTCAM_NONE && tryCam < NUM_ARREST_CAMS && tryList[tryCam] > ARRESTCAM_NONE; tryCam++) {
        gStoredCopPed = nullptr;

        auto gotCameraPos = false;
        switch (tryList[tryCam]) {
        case ARRESTCAM_DW: {
            gTimeDWBustedCamStarted = (float)CTimer::GetTimeInMS();
            if (ProcessDWBustedCam1(handyCop, true)) {
                TheCamera.m_pTargetEntity->SetIsVisible(false);
                gArrestCamInUse = ARRESTCAM_DW;
                m_bResetStatics = false;
                return;
            }
            break;
        }
        case ARRESTCAM_OVERSHOULDER: {
            if (handyCop) {
                gotCameraPos  = GetLookOverShoulderPos(TheCamera.m_pTargetEntity, handyCop, playerPos, arrestCamPos);
                gStoredCopPed = handyCop;
                handyCop      = nullptr;
            } else if (handyPed) {
                gotCameraPos = FindArrestingCopNearby(
                    [&](CPed* cop, CVector& out) { return GetLookOverShoulderPos(TheCamera.m_pTargetEntity, cop, playerPos, out); },
                    arrestCamPos
                );
            }
            break;
        }
        case ARRESTCAM_ALONGGROUND: {
            if (handyCop) {
                gotCameraPos  = GetLookAlongGroundPos(TheCamera.m_pTargetEntity, handyCop, playerPos, arrestCamPos);
                gStoredCopPed = handyCop;
                handyCop      = nullptr;
            } else if (handyPed) {
                // The original really does fall back to the over-the-shoulder position here
                gotCameraPos = FindArrestingCopNearby(
                    [&](CPed* cop, CVector& out) { return GetLookOverShoulderPos(TheCamera.m_pTargetEntity, cop, playerPos, out); },
                    arrestCamPos
                );
            }
            break;
        }
        case ARRESTCAM_FROMLAMPPOST: {
            gotCameraPos = GetLookFromLampPostPos(TheCamera.m_pTargetEntity, handyCop, playerPos, arrestCamPos);
            break;
        }
        default:
            break;
        }

        if (!gotCameraPos) {
            continue;
        }

        if (gStoredCopPed) {
            gStoredCopPed->RegisterReference(reinterpret_cast<CEntity**>(&gStoredCopPed));
        }
        gArrestCamInUse = tryList[tryCam];

        // The along-ground camera picks one of five rotation variants
        if (gArrestCamInUse == ARRESTCAM_ALONGGROUND) {
            const auto pick = CGeneral::GetRandomNumberInRange(0.0f, 5.0f);
            gArrestCamInUse = pick < 1.0f ? ARRESTCAM_ALONGGROUND
                            : pick < 2.0f ? ARRESTCAM_ALONGGROUND_RIGHT
                            : pick < 3.0f ? ARRESTCAM_ALONGGROUND_RIGHT_UP
                            : pick < 4.0f ? ARRESTCAM_ALONGGROUND_LEFT
                                          : ARRESTCAM_ALONGGROUND_LEFT_UP;
        }
    }

    m_vecSource = arrestCamPos;
    const auto sourceBeforeChange = m_vecSource;
    TheCamera.AvoidTheGeometry(sourceBeforeChange, playerPos, m_vecSource, m_fFOV);

    AimAt(playerPos);

    if (gArrestCamInUse != ARRESTCAM_NONE) {
        m_bResetStatics = false;
    }
}

// 0x519250
void CCam::ProcessPedsDeadBaby() {
    constexpr auto DEADCAM_HEIGHT_START   = 2.0f;   // 0x8CC804
    constexpr auto DEADCAM_HEIGHT_RATE    = 0.04f;  // 0x8CC808
    constexpr auto DEADCAM_WAFT_AMPLITUDE = 2.0f;   // 0x8CC80C
    constexpr auto DEADCAM_WAFT_RATE      = 600.0f; // 0x8CC810
    constexpr auto DEADCAM_WAFT_TILT_AMP  = -0.35f; // 0x8CC814

    // Written every time the camera resets, never read again
    static auto& startTimeDWDeadCam = StaticRef<float>(0xB70054);
    static auto& cameraRot          = StaticRef<float>(0xB70050);

    CVector playerPos{};
    if (TheCamera.m_pTargetEntity->GetIsTypePed()) {
        TheCamera.m_pTargetEntity->AsPed()->GetBonePosition(&playerPos, BONE_SPINE1, true);
    } else if (TheCamera.m_pTargetEntity->GetIsTypeVehicle()) {
        playerPos    = TheCamera.m_pTargetEntity->GetPosition();
        playerPos.z += TheCamera.m_pTargetEntity->GetColModel()->GetBoundingBox().m_vecMax.z;
    } else {
        return;
    }

    // The camera always ends up level, tilted only along the target's right
    const auto tempRight = [this] {
        auto r = CrossProduct(TheCamera.m_pTargetEntity->GetForwardVector(), CVector{ 0.0f, 0.0f, 1.0f });
        r.z = 0.0f;
        r.Normalise();
        return r;
    };

    CVector deadCamPos;

    if (m_bResetStatics) {
        startTimeDWDeadCam = (float)CTimer::GetTimeInMS();
        cameraRot          = 0.0f;
        TheCamera.m_nTimeLastChange = CTimer::GetTimeInMS();

        deadCamPos    = playerPos;
        deadCamPos.z += DEADCAM_HEIGHT_START;

        auto camWaterLevel = 0.0f;
        if (CWaterLevel::GetWaterLevelNoWaves(deadCamPos, &camWaterLevel, nullptr, nullptr)) {
            deadCamPos.z = std::max(deadCamPos.z, camWaterLevel + 1.5f);
        }

        const auto right = tempRight();

        m_vecFront = playerPos - deadCamPos;
        m_vecFront.Normalise();
        m_vecUp = CrossProduct(right, m_vecFront);
        m_vecUp.Normalise();

        m_bResetStatics = false;
    } else {
        deadCamPos = m_vecSource;

        // Drift upwards while there's room
        if (!CWorld::TestSphereAgainstWorld(deadCamPos + 0.2f * CVector{ 0.0f, 0.0f, 1.0f }, 0.3f, TheCamera.m_pTargetEntity, true, true, false, true, false, true)) {
            deadCamPos.z += DEADCAM_HEIGHT_RATE * CTimer::GetTimeStep();
        }

        const auto right = tempRight();

        const auto timeOffset = (float)(CTimer::GetTimeInMS() - TheCamera.m_nTimeLastChange);
        const auto waftOffset = std::min(1000.0f, timeOffset) / 1000.0f * std::sin(timeOffset / DEADCAM_WAFT_RATE);

        // Waft sideways around the player, but only where nothing is in the way
        auto waftPos = right * DEADCAM_WAFT_AMPLITUDE * waftOffset;
        waftPos.x += playerPos.x;
        waftPos.y += playerPos.y;
        waftPos.z  = deadCamPos.z;

        const auto toWaft = (waftPos - deadCamPos).Normalized();
        if (!CWorld::TestSphereAgainstWorld(deadCamPos + 0.2f * toWaft, 0.3f, TheCamera.m_pTargetEntity, true, true, false, true, false, true)) {
            deadCamPos = waftPos;
        }

        m_vecFront  = CVector{ 0.0f, 0.0f, -1.0f };
        m_vecFront += right * DEADCAM_WAFT_TILT_AMP * (std::min(2000.0f, timeOffset) / 2000.0f) * std::cos(timeOffset / DEADCAM_WAFT_RATE);
        m_vecFront.Normalise();

        m_vecUp = CrossProduct(right, m_vecFront);
        m_vecUp.Normalise();
    }

    m_vecSource = deadCamPos;
    const auto sourceBeforeChange = m_vecSource;
    TheCamera.AvoidTheGeometry(sourceBeforeChange, playerPos, m_vecSource, m_fFOV);

    // Don't want `CCamera` to recalculate the front and up vectors
    TheCamera.m_bMoveCamToAvoidGeom = false;
}

// 0x50EB70
void CCam::Process_1rstPersonPedOnPC(const CVector& ThisCamsTarget, float TargetOrientation, float SpeedVar, float SpeedVarDesired) {
    static auto& vecHeadCamOffset          = StaticRef<CVector>(0x8CCC54);
    static auto& InitialHeadPos            = StaticRef<CVector>(0xB6FFC4);
    static auto& DPadAndWorldFixer         = StaticRef<CVector>(0xB6FFD0); // DPadHorizontal, DPadVertical, DontLookThroughWorldFixer - only ever reset
    static auto& FailedTestTwelveFramesAgo = StaticRef<bool>(0xB6FFDC);

    const auto MaxRotationUp   = DegreesToRadians(60.0f);
    const auto MaxRotationDown = DegreesToRadians(89.5f);

    if (m_nMode != MODE_SNIPER_RUNABOUT) { // Sniper needs to be able to zoom
        m_fFOV = 70.0f;
    }
    TheCamera.m_b1rstPersonRunCloseToAWall = false;

    if (!m_pCamTargetEntity->GetRwObject()) {
        return;
    }

    if (m_pCamTargetEntity->GetIsTypePed()) {
        auto* const targetPed = m_pCamTargetEntity->AsPed();

        // Offset the head bone towards eye height in head space, so it follows looking up/down
        const auto hier = GetAnimHierarchyFromSkinClump(m_pCamTargetEntity->GetRpClump());
        auto&      mat  = RpHAnimHierarchyGetMatrixArray(hier)[RpHAnimIDGetIndex(hier, ConvertPedNode2BoneTag(PED_NODE_HEAD))];

        CVector posn = vecHeadCamOffset;
        RwV3dTransformPoint(&posn, &posn, &mat);

        RwV3d scale{ 0.0f, 0.0f, 0.0f }; // Scale the head away so we're not looking at the inside of it
        RwMatrixScale(&mat, &scale, rwCOMBINEPRECONCAT);

        if (m_bResetStatics) {
            m_fHorizontalAngle            = targetPed->m_fCurrentRotation + HALF_PI;
            m_fVerticalAngle              = 0.0f;
            m_fInitialPlayerOrientation   = m_fHorizontalAngle;
            FailedTestTwelveFramesAgo     = false;
            DPadAndWorldFixer.Reset();
            m_bCollisionChecksOn          = true;
            m_vecBufferedPlayerBodyOffset = InitialHeadPos = posn;
        }

        m_vecBufferedPlayerBodyOffset.y = posn.y;
        if (TheCamera.m_bHeadBob) {
            const auto sway = TheCamera.m_fGaitSwayBuffer;
            m_vecBufferedPlayerBodyOffset.x = sway * m_vecBufferedPlayerBodyOffset.x + (1.0f - sway) * posn.x;
            m_vecBufferedPlayerBodyOffset.z = sway * m_vecBufferedPlayerBodyOffset.z + (1.0f - sway) * posn.z;
            posn = targetPed->GetMatrix().TransformPoint(m_vecBufferedPlayerBodyOffset);
        } else {
            auto headDiff = posn - InitialHeadPos;
            headDiff.z    = 0.0f;

            auto playHead = targetPed->GetMatrix().GetForward();
            playHead.z    = 0.0f;
            playHead.Normalise();

            posn    = targetPed->GetPosition() + playHead * headDiff.Magnitude() * 1.23f;
            posn.z += 0.59f;
        }
        m_vecSource = posn;

        CVector torsoPos;
        targetPed->GetTransformedBonePosition(torsoPos, BONE_SPINE1, true);

        float      fStickX, fStickY;
        bool       bUsingMouse   = false;
        const auto mouseMovement = CPad::GetPad(0)->NewMouseControllerState.GetAmountMouseMoved();
        if (mouseMovement.x == 0.0f && mouseMovement.y == 0.0f) {
            fStickX = -(float)CPad::GetPad(0)->LookAroundLeftRight(targetPed);
            fStickY = (float)CPad::GetPad(0)->LookAroundUpDown(targetPed);
        } else {
            fStickX     = -mouseMovement.x * 3.0f;
            fStickY     = mouseMovement.y * 4.0f;
            bUsingMouse = true;
        }

        float StickBetaOffset, StickAlphaOffset;
        if (bUsingMouse) {
            StickBetaOffset  = TheCamera.m_fMouseAccelHorzntl * fStickX * (m_fFOV / 80.0f);
            StickAlphaOffset = TheCamera.m_fMouseAccelVertical * fStickY * (m_fFOV / 80.0f);
        } else {
            const auto X_Sign = fStickX < 0.0f ? -1.0f : 1.0f;
            const auto Y_Sign = fStickY < 0.0f ? -1.0f : 1.0f;

            StickBetaOffset  = X_Sign * sq(fStickX / 100.0f) * (0.20f / 3.5f * (m_fFOV / 80.0f)) * CTimer::GetTimeStep();
            StickAlphaOffset = Y_Sign * sq(fStickY / 150.0f) * (0.25f / 3.5f * (m_fFOV / 80.0f)) * CTimer::GetTimeStep();
        }

        m_fHorizontalAngle += StickBetaOffset;
        m_fVerticalAngle   += StickAlphaOffset;

        ClipBeta();
        m_fVerticalAngle = std::clamp(m_fVerticalAngle, -MaxRotationDown, MaxRotationUp);

        if (const auto* attachedTo = targetPed->m_pAttachedTo; targetPed->IsPlayer() && attachedTo) {
            auto fDefaultHeading = attachedTo->GetHeading();
            switch ((int32)targetPed->m_fTurretAngleA) { // m_nAttachLookDirn
            case 0: fDefaultHeading += HALF_PI; break; // Forward - heading is taken from the x vector
            case 1: fDefaultHeading += PI;      break; // Left
            case 2: fDefaultHeading -= HALF_PI; break; // Back
            case 3:                             break; // Right
            }

            auto fCamDelta = m_fHorizontalAngle - fDefaultHeading;
            if (fCamDelta > PI) {
                fCamDelta -= TWO_PI;
            } else if (fCamDelta < -PI) {
                fCamDelta += TWO_PI;
            }

            const auto limit   = targetPed->m_fTurretAngleB; // m_fAttachHeadingLimit
            m_fHorizontalAngle = fDefaultHeading + std::clamp(fCamDelta, -limit, limit);
        }

        const auto TargetCoors = m_vecSource + 3.0f * CVector{
            std::cos(m_fHorizontalAngle) * std::cos(m_fVerticalAngle),
            std::sin(m_fHorizontalAngle) * std::cos(m_fVerticalAngle),
            std::sin(m_fVerticalAngle)
        };
        m_vecFront = (TargetCoors - m_vecSource).Normalized();

        // The original nudges the source 0.4 along the front here so its wall-proximity checks don't
        // hit the player, then undoes it. Those checks are `#if 0`'d out, so the pair cancels.

        TheCamera.m_fAlphaForPlayerAnim1rstPerson = m_fVerticalAngle;

        GetVectorsReadyForRW();

        // Keep the entity heading in sync with the camera
        const auto  CamDirection = std::atan2(-m_vecFront.x, m_vecFront.y);
        auto* const camTarget    = TheCamera.m_pTargetEntity->AsPed();
        camTarget->m_fCurrentRotation = CamDirection;
        camTarget->m_fAimingRotation  = CamDirection;
        camTarget->SetHeading(CamDirection);
        camTarget->UpdateRwMatrix();

        if (m_nMode == MODE_SNIPER_RUNABOUT) {
            if (CPad::GetPad(0)->SniperZoomOut()) {
                m_fFOV *= (10000.0f + 255.0f * CTimer::GetTimeStep()) / 10000.0f;
            } else if (CPad::GetPad(0)->SniperZoomIn()) {
                m_fFOV /= (10000.0f + 255.0f * CTimer::GetTimeStep()) / 10000.0f;
            }
            TheCamera.SetMotionBlur(180, 255, 180, 120, eMotionBlurType::SNIPER);
            m_fFOV = std::clamp(m_fFOV, 15.0f, 70.0f);
        }
    }

    m_bResetStatics = false;
    RwCameraSetNearClipPlane(Scene.m_pRwCamera, 0.05f);
}

// 0x517EA0
void CCam::Process_1stPerson(const CVector& target, float orientation, float speedVar, float speedVarWanted) {
    static auto& s_LastWheelieTime = StaticRef<float>(0x8CCD14);
    // Making sure player doesn't see below ground when flipped.
    // Name is made up cuz I found it funny to name it like that.
    static auto& s_GroundFaultProtection = StaticRef<float>(0xB7004C);

    gbFirstPersonRunThisFrame = true;

    m_fFOV = 70.0f;
    if (!m_pCamTargetEntity->GetRwObject()) {
        return;
    }

    if (m_bResetStatics) {
        m_fVerticalAngle   = 0.0f;
        m_fHorizontalAngle = [&] {
            if (m_pCamTargetEntity->GetIsTypePed()) {
                return m_pCamTargetEntity->AsPed()->m_fCurrentRotation + DegreesToRadians(90.0f);
            } else {
                return orientation;
            }
        }();
        m_fInitialPlayerOrientation = m_fHorizontalAngle;

        s_GroundFaultProtection                 = 0.0f;
        TheCamera.m_fAvoidTheGeometryProbsTimer = 0.0f;
    }

    if (m_pCamTargetEntity->GetIsTypePed()) {
        m_bResetStatics = false;
        return;
    }

    const auto wheelieTime = static_cast<float>(CTimer::GetTimeInMS());
    if (s_LastWheelieTime > wheelieTime) {
        s_LastWheelieTime = 0.0f;
    }

    auto* targetVeh = m_pCamTargetEntity->AsVehicle();
    if (targetVeh->IsBike() && targetVeh->AsBike()->bikeFlags.bWheelieForCamera || TheCamera.m_fAvoidTheGeometryProbsTimer > 0.0f) {
        if (wheelieTime - s_LastWheelieTime >= 3000.0f) {
            s_LastWheelieTime = static_cast<float>(CTimer::GetTimeInMS());
        }

        const auto pad1 = CPad::GetPad();
        if (!pad1->NewState.LeftShoulder2 && !pad1->NewState.RightShoulder2) {
            auto* targetBike = targetVeh->AsBike();
            if (Process_WheelCam(target, orientation, speedVar, speedVarWanted)) {
                if (targetBike->bikeFlags.bWheelieForCamera) {
                    TheCamera.m_fAvoidTheGeometryProbsTimer = 50.0f;
                } else {
                    TheCamera.m_fAvoidTheGeometryProbsTimer -= CTimer::GetTimeStep();
                    targetBike->bikeFlags.bWheelieForCamera = true;
                }
                return;
            }
            TheCamera.m_fAvoidTheGeometryProbsTimer = 0.0f;
            targetBike->bikeFlags.bWheelieForCamera = false;

            s_LastWheelieTime = 0.0f;
        }
    }

    const auto& entityWorldMat = [&] {
        if (auto* t = targetVeh->AsBike(); t->IsBike()) {
            t->CalculateLeanMatrix();
            return t->m_mLeanMatrix;
        } else {
            return targetVeh->GetMatrix();
        }
    }();

    const auto dummyPos = [&] {
        const auto* vehStruct = targetVeh->GetVehicleModelInfo()->GetVehicleStruct();
        return vehStruct->m_avDummyPos[targetVeh->IsBoat() ? DUMMY_LIGHT_FRONT_MAIN : DUMMY_SEAT_FRONT] * CVector{0.0f, 1.0f, 1.0f}; // ignore x
    }() + CVector{ 0.0f, 0.08f, 0.62f };

    m_fFOV = 60.0f;
    m_vecSource = entityWorldMat.TransformVector(dummyPos);
    m_vecSource += targetVeh->GetPosition();

    if (targetVeh->IsBike() && targetVeh->m_pDriver) {
        auto*   targetBike = targetVeh->AsBike();
        CVector neckPos{};

        targetVeh->m_pDriver->GetTransformedBonePosition(neckPos, BONE_NECK, true);
        neckPos += targetBike->GetMoveSpeed() * CTimer::GetTimeStep();

        constexpr auto BIKE_1ST_PERSON_ZOFFSET = 0.15f; // 0x8CC7B4
        m_vecSource.z = neckPos.z + BIKE_1ST_PERSON_ZOFFSET;

        const auto right = CrossProduct(m_vecFront, m_vecUp);
        // right *= flt_8CCD0C; (=1.0f)

        if (!CWorld::GetIsLineOfSightClear(
            CrossProduct(m_vecSource, m_vecSource + right),
            CrossProduct(m_vecSource, m_vecSource - right),
            true,
            false,
            false,
            false
        )) {
            m_vecSource = targetBike->GetPosition();
            m_vecSource.z = neckPos.z + BIKE_1ST_PERSON_ZOFFSET + 0.62f;
        }
    } else if (targetVeh->IsBoat()) {
        m_vecSource.z += 0.5f;
    }

    // todo: refactor
    if (targetVeh->IsUpsideDown()) {
        if (s_GroundFaultProtection >= 0.5f) {
            s_GroundFaultProtection = 0.5f;
        } else {
            s_GroundFaultProtection += 0.03f;
        }
    } else if (s_GroundFaultProtection >= 0.0f) {
        s_GroundFaultProtection = 0.0f;
    } else {
        s_GroundFaultProtection -= 0.03f;
    }
    m_vecSource.z += s_GroundFaultProtection;

    m_vecFront = entityWorldMat.GetForward().Normalized();
    m_vecUp    = entityWorldMat.GetUp().Normalized();
    const auto a = CrossProduct(m_vecFront, m_vecUp).Normalized();
    m_vecUp = CrossProduct(a, m_vecFront).Normalized();

    if (float wl{}; CWaterLevel::GetWaterLevel(m_vecSource, wl, true) && m_vecSource.z < wl - 0.3f) {
        ApplyUnderwaterMotionBlur();
    }
    m_bResetStatics = false;
}

// 0x521500
void CCam::Process_AimWeapon(const CVector& ThisCamsTarget, float TargetOrientation, float SpeedVar, float SpeedVarDesired) {
    // `AIMWEAPON_SETTINGS` @ 0x8CC4C0, `tAimingCamData[4]` - indices confirmed at 0x5215AD..0x521612
    enum eAimType { AIMWEAPON_ONFOOT = 0, AIMWEAPON_ONBIKE = 1, AIMWEAPON_INCAR = 2, AIMWEAPON_MELEE = 3 };
    struct tAimingCamData {
        float MaxDist, AngleDist, AngleFalloff, DefaultAlpha, ZShift, RotMaxUp, RotMaxDown;
    };
    static auto& AIMWEAPON_SETTINGS = StaticRef<std::array<tAimingCamData, 4>>(0x8CC4C0);

    constexpr auto AIMWEAPON_STICK_SENS       = 0.007f;    // 0x8CC4A0
    constexpr auto AIMWEAPON_TARGET_SENS      = 0.1f;      // 0x8CC4A4
    constexpr auto AIMWEAPON_FREETARGET_SENS  = 0.1f;      // 0x8CC4A8
    constexpr auto AIMWEAPON_DRIVE_SENS_MULT  = 0.25f;     // 0x8CC4AC
    constexpr auto AIMWEAPON_DRIVE_CLOSE_ENOUGH = 0.174533f; // 0x8CC4B0
    constexpr auto AIMWEAPON_RIFLE1_ZOOM      = 50.0f;     // 0x8CC4B4
    constexpr auto AIMWEAPON_RIFLE2_ZOOM      = 35.0f;     // 0x8CC4B8
    constexpr auto AIMWEAPON_FOV_ZOOM_RATE    = 1.0f;      // 0x862F1C
    constexpr auto fTweakPedAimDirn           = -0.05f;    // 0x862F18

    constexpr auto MELEE_TARGETING_ALPHA = 3.0f;   // 0x862F20
    constexpr auto MELEE_TARGETING_BETA  = 20.0f;  // 0x862F24
    constexpr auto MELEE_FIGHTING_ALPHA  = 0.0f;   // 0x862F28
    constexpr auto MELEE_FIGHTING_BETA   = 70.0f;  // 0x862F2C
    constexpr auto MELEE_ANGLE_RATE      = 0.96f;  // 0x862F30

    constexpr auto FREEAIM_STATIC_LIM_A  = 5;      // 0x8CC534
    constexpr auto FREEAIM_STATIC_LIM_B  = 2;      // 0x8CC538
    constexpr auto PLAYERFIGHT_LEVEL_SMOOTHING_CONST = 0.9f; // 0x8CC39C
    constexpr auto TWEAK_MELEE_TARGETZ   = 0.75f;  // 0x8CCE60
    constexpr auto INCAR_PASSENGER_DOUBLETAP_TIME = 500u; // 0x8CCE54
    constexpr auto STICK_RATE_UP         = 0.8f;   // 0x8CCE5C
    constexpr auto STICK_RATE_DOWN       = 0.5f;   // 0x8CCE58

    static auto& ACQUIRED_FREEAIM_DIRECTION          = StaticRef<bool>(0xB6EC44);
    static auto& ACQUIRED_FREEAIM_INCAR_IDLE_COUNTER = StaticRef<int32>(0xB6EC48);
    static auto& FREEAIM_INCAR_TARGET_TAP_TIME       = StaticRef<uint32>(0xB6EC4C);
    static auto& ACQUIRED_FREEAIM_PEDHEADING         = StaticRef<float>(0x8CC530);
    static auto& FIGHT_AIM_TOWARD_TARGET             = StaticRef<float>(0xB700F4);
    static auto& sAimWeaponMeleeLOSCounter           = StaticRef<float>(0xB700F8);
    static auto& FIGHT_AIM_ANGLE_BETA                = StaticRef<float>(0xB700FC);
    static auto& FIGHT_AIM_ANGLE_ALPHA               = StaticRef<float>(0xB70100);
    static auto& vecWeaponTargetPos                  = StaticRef<CVector>(0xB70104);

    if (!m_pCamTargetEntity->GetIsTypePed()) {
        return;
    }
    auto* const pPed = m_pCamTargetEntity->AsPed();
    if (!pPed->IsPlayer()) {
        return;
    }

    auto* const useGun     = pPed->GetIntelligence()->GetTaskUseGun();
    auto* const weaponInfo = useGun
        ? useGun->m_WeaponInfo
        : CWeaponInfo::GetWeaponInfo(pPed->GetActiveWeapon().m_Type, pPed->GetWeaponSkill());

    auto nAimType = AIMWEAPON_ONFOOT;
    if (pPed->bInVehicle) {
        const auto veh = pPed->m_pVehicle;
        nAimType = veh && (veh->IsBike() || veh->IsSubQuad()) ? AIMWEAPON_ONBIKE : AIMWEAPON_INCAR;
    } else if (pPed->GetIntelligence()->GetTaskJetPack()) {
        nAimType = AIMWEAPON_ONBIKE;
    } else if (pPed->GetActiveWeapon().IsTypeMelee()) {
        nAimType = AIMWEAPON_MELEE;
    }
    const auto& settings = AIMWEAPON_SETTINGS[nAimType];

    // Zoom
    const auto weaponType = pPed->GetActiveWeapon().m_Type;
    auto fDesiredFOV = 70.0f;
    if (weaponType == WEAPON_AK47 || weaponType == WEAPON_M4) {
        fDesiredFOV = AIMWEAPON_RIFLE1_ZOOM;
    } else if (weaponType == WEAPON_COUNTRYRIFLE) {
        fDesiredFOV = AIMWEAPON_RIFLE2_ZOOM;
    }

    if (!TheCamera.m_bTransitionState) {
        if (m_bResetStatics && weaponType != WEAPON_COUNTRYRIFLE) {
            m_fFOV = fDesiredFOV;
        } else {
            const auto rate = AIMWEAPON_FOV_ZOOM_RATE * CTimer::GetTimeStep();
            if (fDesiredFOV > m_fFOV + rate) {
                m_fFOV += rate;
            } else if (fDesiredFOV < m_fFOV - rate) {
                m_fFOV -= rate;
            } else {
                m_fFOV = fDesiredFOV;
            }
        }
    }

    // Where the reticle sits relative to screen centre, as a pair of angles
    float fAimAngleBeta, fAimAngleAlpha;
    if (weaponInfo->GetFireType() == WEAPON_FIRE_MELEE) {
        auto fDesAlpha = MELEE_TARGETING_ALPHA;
        auto fDesBeta  = MELEE_TARGETING_BETA;
        auto fDesToward = 0.0f;

        if (pPed->GetIntelligence()->GetTaskFighting() && pPed->m_nMoveState < PEDMOVE_WALK && pPed->m_pTargetedObject) {
            if (sAimWeaponMeleeLOSCounter > CTimer::GetTimeStep()) {
                sAimWeaponMeleeLOSCounter -= CTimer::GetTimeStep();
            } else if (sAimWeaponMeleeLOSCounter < -CTimer::GetTimeStep()) {
                sAimWeaponMeleeLOSCounter += CTimer::GetTimeStep();
            } else {
                const auto testPos = pPed->GetPosition() + CVector{ 0.0f, 0.0f, 0.75f };
                auto       offset  = CrossProduct(pPed->m_pTargetedObject->GetPosition() - pPed->GetPosition(), CVector{ 0.0f, 0.0f, 1.0f });
                offset *= 2.0f / std::max(0.7f, offset.Magnitude());

                sAimWeaponMeleeLOSCounter = CWorld::GetIsLineOfSightClear(testPos, testPos + offset, true, true, false, true, false, true, true)
                    ? 100.0f
                    : -100.0f;
            }

            if (sAimWeaponMeleeLOSCounter >= 0.0f) {
                fDesAlpha  = MELEE_FIGHTING_ALPHA;
                fDesBeta   = MELEE_FIGHTING_BETA;
                fDesToward = 1.0f;
            }
        }

        if (m_bResetStatics) {
            FIGHT_AIM_ANGLE_ALPHA   = fDesAlpha;
            FIGHT_AIM_ANGLE_BETA    = fDesBeta;
            FIGHT_AIM_TOWARD_TARGET = 0.0f;
        } else if (!TheCamera.m_bTransitionState) { // Don't swing the camera around mid-transition
            const auto rate = std::pow(MELEE_ANGLE_RATE, CTimer::GetTimeStep());
            FIGHT_AIM_ANGLE_ALPHA   = rate * FIGHT_AIM_ANGLE_ALPHA   + (1.0f - rate) * fDesAlpha;
            FIGHT_AIM_ANGLE_BETA    = rate * FIGHT_AIM_ANGLE_BETA    + (1.0f - rate) * fDesBeta;
            FIGHT_AIM_TOWARD_TARGET = rate * FIGHT_AIM_TOWARD_TARGET + (1.0f - rate) * fDesToward;
        }

        fAimAngleAlpha = DegreesToRadians(FIGHT_AIM_ANGLE_ALPHA);
        fAimAngleBeta  = DegreesToRadians(FIGHT_AIM_ANGLE_BETA);
    } else {
        const auto screenAngle = DegreesToRadians(0.5f * m_fFOV);
        fAimAngleBeta  = std::atan(2.0f * (CCamera::m_f3rdPersonCHairMultX - 0.5f) * std::tan(screenAngle));
        fAimAngleAlpha = std::atan(2.0f * (0.5f - CCamera::m_f3rdPersonCHairMultY) * (1.0f / CDraw::GetAspectRatio()) * std::tan(screenAngle));
        FIGHT_AIM_TOWARD_TARGET = 0.0f;
    }

    if (m_bResetStatics) {
        TheCamera.ResetDuckingSystem(pPed);
        m_bRotating          = false;
        m_bCollisionChecksOn = true;
        ACQUIRED_FREEAIM_DIRECTION          = true;
        ACQUIRED_FREEAIM_INCAR_IDLE_COUNTER = 60000;
        ACQUIRED_FREEAIM_PEDHEADING         = -1001.0f;
        FREEAIM_INCAR_TARGET_TAP_TIME       = 0;
        m_fAlphaSpeed = 0.0f;
        m_fBetaSpeed  = 0.0f;

        if (TheCamera.m_bUseMouse3rdPerson && !pPed->m_pTargetedObject) {
            // Keep the direction the camera is already looking in
        } else {
            m_fVerticalAngle = settings.DefaultAlpha;
            if (pPed->bInVehicle && pPed->m_pVehicle) { // Drive-bys always start looking ahead
                m_fHorizontalAngle = pPed->m_fCurrentRotation - HALF_PI - fAimAngleBeta;
                m_fVerticalAngle  += std::asin(std::clamp(pPed->m_pVehicle->GetMatrix().GetForward().z, -1.0f, 1.0f));
            } else if (!pPed->m_pTargetedObject) {
                m_fHorizontalAngle = pPed->m_fCurrentRotation - HALF_PI + fAimAngleBeta;
                if (pPed->bIsStanding) {
                    const auto groundNormalFwd = DotProduct(pPed->m_vecGroundNormal, pPed->GetMatrix().GetForward());
                    m_fVerticalAngle -= std::asin(std::clamp(groundNormalFwd, -1.0f, 1.0f));
                    if (weaponType == WEAPON_EXTINGUISHER) {
                        m_fVerticalAngle += CWeapon::ms_fExtinguisherAimAngle;
                    }
                }
            }
        }
    }

    if (CTheScripts::fCameraHeadingStepWhenPlayerIsAttached > 0.0f) {
        auto fDiff = m_fHorizontalAngle - CTheScripts::fCameraHeadingWhenPlayerIsAttached;
        if (fDiff < 0.0f) {
            fDiff += TWO_PI;
        }
        const auto fDiff2 = TWO_PI - fDiff;
        if (fDiff < CTheScripts::fCameraHeadingStepWhenPlayerIsAttached || fDiff2 < CTheScripts::fCameraHeadingStepWhenPlayerIsAttached) {
            m_fHorizontalAngle                                 = CTheScripts::fCameraHeadingWhenPlayerIsAttached;
            CTheScripts::fCameraHeadingStepWhenPlayerIsAttached = 0.0f;
        } else if (fDiff > fDiff2) {
            m_fHorizontalAngle += CTheScripts::fCameraHeadingStepWhenPlayerIsAttached;
        } else {
            m_fHorizontalAngle -= CTheScripts::fCameraHeadingStepWhenPlayerIsAttached;
        }
    }

    auto vecTargetCoords = ThisCamsTarget;

    // Look at a point roughly above the player's head
    const auto origZ = vecTargetCoords.z;
    pPed->UpdateRpHAnim();

    auto camTargetZ = pPed->GetPosition().z + 0.5f + settings.ZShift;
    if (m_fFOV < 70.0f) {
        camTargetZ += 0.1f * std::min(1.0f, (70.0f - m_fFOV) / (70.0f - AIMWEAPON_RIFLE1_ZOOM));
    }
    vecTargetCoords.z = camTargetZ;
    const auto MOVED_Z = vecTargetCoords.z - origZ;

    // Slide sideways to look over the shoulder
    auto fSide = 0.20f;
    if (weaponInfo && !weaponInfo->flags.bAimWithArm && pPed->AsPlayer()->GetPlayerData()->m_pPedClothesDesc->HasVisibleNewHairCut(1)) {
        fSide += 0.1f;
    } else if (m_fFOV < 70.0f) {
        fSide += 0.1f * std::min(1.0f, (70.0f - m_fFOV) / (70.0f - AIMWEAPON_RIFLE2_ZOOM));
    }

    { // TEST_POSITION_AIM_CAM_USING_CAM (0x8CCE64) is set, so this is the live branch
        const auto tempRight = CrossProduct(m_vecFront, m_vecUp);
        auto       sideMult  = std::clamp(DotProduct(tempRight, pPed->GetMatrix().GetRight()), 0.0f, 1.0f);
        sideMult             = 1.0f - std::acos(sideMult) / HALF_PI;
        vecTargetCoords     += fSide * sideMult * tempRight;
    }

    if (auto* const lockOn = pPed->m_pTargetedObject) {
        CVector tempTarget{};
        if (lockOn->GetIsTypePed() && weaponInfo->GetFireType() != WEAPON_FIRE_MELEE) {
            lockOn->AsPed()->GetTransformedBonePosition(tempTarget, BONE_SPINE1, true);
        } else {
            tempTarget = lockOn->GetPosition();
        }

        if (weaponInfo->GetFireType() == WEAPON_FIRE_MELEE) {
            tempTarget.z += TWEAK_MELEE_TARGETZ * MOVED_Z;
        }

        if (m_bResetStatics || !pPed->GetIntelligence()->GetTaskFighting()) {
            vecWeaponTargetPos = tempTarget;
        } else {
            const auto smooth  = std::pow(PLAYERFIGHT_LEVEL_SMOOTHING_CONST, CTimer::GetTimeStep());
            vecWeaponTargetPos = smooth * vecWeaponTargetPos + (1.0f - smooth) * tempTarget;
        }

        const auto delta       = vecWeaponTargetPos - vecTargetCoords;
        auto       fTargetBeta = std::atan2(-delta.x, delta.y) - HALF_PI;
        auto       fTargetAlpha = std::atan2(delta.z, delta.Magnitude2D());

        if (weaponInfo->GetFireType() == WEAPON_FIRE_MELEE) {
            fTargetAlpha *= std::cos(fAimAngleBeta);
        } else {
            const auto distToCam   = std::min((vecTargetCoords - m_vecSource).Magnitude(), settings.MaxDist);
            const auto distToPed   = delta.Magnitude();
            const auto camToPed    = distToCam + distToPed;
            fAimAngleAlpha *= camToPed / distToPed;
            fAimAngleBeta  *= camToPed / distToPed;
        }

        fTargetBeta  += fAimAngleBeta;
        fTargetAlpha -= fAimAngleAlpha;

        if (fTargetAlpha < -PI) {
            fTargetAlpha += TWO_PI;
        } else if (fTargetAlpha > PI) {
            fTargetAlpha -= TWO_PI;
        }

        const auto rate = m_bResetStatics ? 1000.0f : AIMWEAPON_TARGET_SENS * CTimer::GetTimeStep();

        auto diff = fTargetAlpha - m_fVerticalAngle;
        if (std::abs(diff) < rate) {
            m_fVerticalAngle = fTargetAlpha;
        } else {
            m_fVerticalAngle += diff < 0.0f ? -rate : rate;
        }

        if (fTargetBeta - m_fHorizontalAngle > PI) {
            fTargetBeta -= TWO_PI;
        } else if (fTargetBeta - m_fHorizontalAngle < -PI) {
            fTargetBeta += TWO_PI;
        }

        diff = fTargetBeta - m_fHorizontalAngle;
        if (std::abs(diff) < rate) {
            m_fHorizontalAngle = fTargetBeta;
        } else {
            m_fHorizontalAngle += diff < 0.0f ? -rate : rate;
        }

        m_fAlphaSpeed = 0.0f;
        m_fBetaSpeed  = 0.0f;
    } else if (const auto mouse = CPad::GetPad(0)->NewMouseControllerState.GetAmountMouseMoved();
               TheCamera.m_bUseMouse3rdPerson && !CPad::GetPad(0)->DisablePlayerControls && (mouse.x != 0.0f || mouse.y != 0.0f)) {
        m_fHorizontalAngle += TheCamera.m_fMouseAccelHorzntl * (-mouse.x * 2.5f) * (m_fFOV / 80.0f);
        m_fVerticalAngle   += TheCamera.m_fMouseAccelVertical * (mouse.y * 4.0f) * (m_fFOV / 80.0f);

        m_fAlphaSpeed = m_fBetaSpeed = 0.0f; // No leftover movement once the mouse stops
    } else {
        const auto fStickX = -(float)CPad::GetPad(0)->AimWeaponLeftRight(pPed);
        const auto fStickY = (float)CPad::GetPad(0)->AimWeaponUpDown(pPed);

        auto StickBetaOffset  = sq(AIMWEAPON_STICK_SENS) * std::abs(fStickX) * fStickX * (0.25f / 3.5f * (m_fFOV / 80.0f)) * CTimer::GetTimeStep();
        auto StickAlphaOffset = sq(AIMWEAPON_STICK_SENS) * std::abs(fStickY) * fStickY * (0.15f / 3.5f * (m_fFOV / 80.0f)) * CTimer::GetTimeStep();

        const auto rate = std::pow(
            std::abs(fStickX) < 2.0f && std::abs(fStickY) < 2.0f ? STICK_RATE_DOWN : STICK_RATE_UP,
            CTimer::GetTimeStep()
        );
        m_fBetaSpeed  = rate * m_fBetaSpeed  + (1.0f - rate) * StickBetaOffset;
        m_fAlphaSpeed = rate * m_fAlphaSpeed + (1.0f - rate) * StickAlphaOffset;
        StickBetaOffset  = m_fBetaSpeed;
        StickAlphaOffset = m_fAlphaSpeed;

        const auto veh      = pPed->bInVehicle ? pPed->m_pVehicle : nullptr;
        const auto isDriver = veh && veh->m_pDriver == pPed;

        if (veh && !isDriver) {
            // Double-tap target to spin the drive-by camera around
            static bool s_WasTargetDown = false;
            const auto  targetDown      = CPad::GetPad(0)->GetTarget();
            if (targetDown && !std::exchange(s_WasTargetDown, targetDown)) {
                if (CTimer::GetTimeInMS() - FREEAIM_INCAR_TARGET_TAP_TIME < INCAR_PASSENGER_DOUBLETAP_TIME) {
                    StickBetaOffset  = PI;
                    StickAlphaOffset = 0.0f;
                } else {
                    FREEAIM_INCAR_TARGET_TAP_TIME = CTimer::GetTimeInMS();
                }
            }
            s_WasTargetDown = targetDown;
        } else if (isDriver) {
            if (fStickX != 0.0f || fStickY != 0.0f) {
                ACQUIRED_FREEAIM_INCAR_IDLE_COUNTER = 0;
            } else if (!CPad::GetPad(0)->GetWeapon(pPed)) {
                ACQUIRED_FREEAIM_INCAR_IDLE_COUNTER += (int32)CTimer::GetTimeStepInMS();
            }

            if (ACQUIRED_FREEAIM_INCAR_IDLE_COUNTER > FREEAIM_STATIC_LIM_A) {
                ACQUIRED_FREEAIM_DIRECTION  = false;
                ACQUIRED_FREEAIM_PEDHEADING = pPed->m_fCurrentRotation - HALF_PI + fAimAngleBeta;
            } else if (ACQUIRED_FREEAIM_INCAR_IDLE_COUNTER > FREEAIM_STATIC_LIM_B) {
                auto angleDiff = (pPed->m_fCurrentRotation - HALF_PI) - fAimAngleBeta - m_fHorizontalAngle;
                if (angleDiff > TWO_PI) {
                    angleDiff -= TWO_PI;
                } else if (angleDiff < -TWO_PI) {
                    angleDiff += TWO_PI;
                }

                if (angleDiff < DegreesToRadians(30.0f)) {
                    ACQUIRED_FREEAIM_DIRECTION          = false;
                    ACQUIRED_FREEAIM_INCAR_IDLE_COUNTER = FREEAIM_STATIC_LIM_A + 1;
                    ACQUIRED_FREEAIM_PEDHEADING         = pPed->m_fCurrentRotation - HALF_PI + fAimAngleBeta;
                } else {
                    ACQUIRED_FREEAIM_DIRECTION = true;
                }
            } else {
                ACQUIRED_FREEAIM_DIRECTION = true;
                if (CPad::GetPad(0)->GetWeapon(pPed)) {
                    ACQUIRED_FREEAIM_INCAR_IDLE_COUNTER = 0;
                }
            }
        }

        if (!ACQUIRED_FREEAIM_DIRECTION) {
            auto fTargetBeta = pPed->m_fCurrentRotation - HALF_PI;
            if (ACQUIRED_FREEAIM_PEDHEADING < -1000.0f) {
                ACQUIRED_FREEAIM_PEDHEADING = fTargetBeta;
            } else {
                fTargetBeta = ACQUIRED_FREEAIM_PEDHEADING;
            }

            if (weaponInfo && !weaponInfo->flags.bAimWithArm && weaponInfo->GetFireType() != WEAPON_FIRE_MELEE) {
                pPed->m_fCurrentRotation = pPed->m_fAimingRotation = ACQUIRED_FREEAIM_PEDHEADING + HALF_PI;
                pPed->SetHeading(pPed->m_fCurrentRotation);
                pPed->UpdateRwMatrix();
            }

            fTargetBeta -= fAimAngleBeta;

            auto rate             = AIMWEAPON_FREETARGET_SENS * CTimer::GetTimeStep();
            auto closeEnoughRange = 0.0f;
            if (isDriver) {
                rate             *= AIMWEAPON_DRIVE_SENS_MULT;
                closeEnoughRange  = AIMWEAPON_DRIVE_CLOSE_ENOUGH;
            }

            if (fTargetBeta - m_fHorizontalAngle > PI) {
                fTargetBeta -= TWO_PI;
            } else if (fTargetBeta - m_fHorizontalAngle < -PI) {
                fTargetBeta += TWO_PI;
            }

            auto diff = fTargetBeta - m_fHorizontalAngle;
            if (closeEnoughRange > 0.0f) {
                if (diff > closeEnoughRange) {
                    diff -= closeEnoughRange;
                } else if (diff < -closeEnoughRange) {
                    diff += closeEnoughRange;
                } else {
                    diff = 0.0f;
                }
            }

            if (std::abs(diff) < rate) {
                m_fHorizontalAngle        += diff;
                ACQUIRED_FREEAIM_DIRECTION = true;
            } else {
                m_fHorizontalAngle += diff < 0.0f ? -rate : rate;
            }

            if (isDriver) {
                auto fTargetAlpha = std::asin(std::clamp(pPed->m_pVehicle->GetMatrix().GetForward().z, -1.0f, 1.0f)) + settings.DefaultAlpha;
                if (fTargetAlpha - m_fVerticalAngle > PI) {
                    fTargetAlpha -= TWO_PI;
                } else if (fTargetAlpha - m_fVerticalAngle < -PI) {
                    fTargetAlpha += TWO_PI;
                }

                diff = fTargetAlpha - m_fVerticalAngle;
                if (diff > closeEnoughRange) {
                    diff -= closeEnoughRange;
                } else if (diff < -closeEnoughRange) {
                    diff += closeEnoughRange;
                } else {
                    diff = 0.0f;
                }

                if (std::abs(diff) < rate) {
                    m_fVerticalAngle += diff;
                } else {
                    m_fVerticalAngle += diff < 0.0f ? -rate : rate;
                }
            } else {
                m_fVerticalAngle += StickAlphaOffset;
            }
        } else {
            ACQUIRED_FREEAIM_PEDHEADING = -1001.0f;
            m_fHorizontalAngle += StickBetaOffset;
            m_fVerticalAngle   += StickAlphaOffset;
        }
    }

    ClipBeta();
    m_fVerticalAngle = std::clamp(m_fVerticalAngle, -settings.RotMaxDown, settings.RotMaxUp);

    auto fDefaultDistFromPed = settings.MaxDist;
    fDefaultDistFromPed += m_fVerticalAngle > 0.0f
        ? settings.AngleDist * std::cos(std::min(HALF_PI, settings.AngleFalloff * m_fVerticalAngle))
        : settings.AngleDist * std::cos(m_fVerticalAngle);

    m_vecFront = CVector{
        -std::cos(m_fHorizontalAngle) * std::cos(m_fVerticalAngle),
        -std::sin(m_fHorizontalAngle) * std::cos(m_fVerticalAngle),
        std::sin(m_fVerticalAngle)
    };
    m_vecSource = vecTargetCoords - fDefaultDistFromPed * m_vecFront;

    TheCamera.HandleCameraMotionForDuckingDuringAim(pPed, &m_vecSource, &vecTargetCoords, false);
    m_vecTargetCoorsForFudgeInter = vecTargetCoords;

    CCamera::SetColVarsAimWeapon(nAimType);
    if (m_nDirectionWasLooking == LOOKING_FORWARD) {
        TheCamera.CameraGenericModeSpecialCases(pPed);
        TheCamera.CameraPedAimModeSpecialCases(pPed);
        TheCamera.CameraColDetAndReact(&m_vecSource, &vecTargetCoords);
        TheCamera.ImproveNearClip(nullptr, pPed, &m_vecSource, &vecTargetCoords);
    }

    TheCamera.m_bCamDirectlyBehind  = false;
    TheCamera.m_bCamDirectlyInFront = false;

    // Swing round a bit to look toward the target ped
    if (FIGHT_AIM_TOWARD_TARGET > 0.0f && pPed->m_pTargetedObject) {
        const auto tempTarget = (1.0f - 0.5f * FIGHT_AIM_TOWARD_TARGET) * vecTargetCoords + 0.5f * FIGHT_AIM_TOWARD_TARGET * vecWeaponTargetPos;
        m_vecFront = (tempTarget - m_vecSource).Normalized();
    }

    GetVectorsReadyForRW();

    if (weaponInfo && (!weaponInfo->flags.bAimWithArm || pPed->bIsDucking)
        && weaponInfo->GetFireType() != WEAPON_FIRE_MELEE && !pPed->bInVehicle
    ) {
        auto CamDirection = -101.0f;
        if (weaponType == WEAPON_SPRAYCAN) {
            CamDirection = std::atan2(-m_vecFront.x, m_vecFront.y) - fAimAngleBeta;
        } else if (auto* const lockOn = pPed->m_pTargetedObject) {
            const auto delta = lockOn->GetPosition() - pPed->GetPosition();
            CamDirection     = std::atan2(-delta.x, delta.y);
        } else if (ACQUIRED_FREEAIM_DIRECTION) {
            CamDirection = std::atan2(-m_vecFront.x, m_vecFront.y) - fAimAngleBeta;
        }

        if (CamDirection > -100.0f) {
            pPed->m_fCurrentRotation = CamDirection + fTweakPedAimDirn;
            pPed->m_fAimingRotation  = CamDirection + fTweakPedAimDirn;
            pPed->SetHeading(CamDirection); // Keep the entity heading in sync with the camera
            pPed->UpdateRwMatrix();
        }
        TheCamera.m_pTargetEntity->AsPed()->AsPlayer()->GetPlayerData()->m_fLookPitch = TheCamera.Find3rdPersonQuickAimPitch();
    }

    m_bResetStatics = false;
}

// 0x512B10
void CCam::Process_AttachedCam() {
    //! Above this magnitude the water tint gets scaled down before being used as motion blur. 0x8CC7A8
    constexpr auto UNDERWATER_CAM_COLORMAG_LIMIT = 10.0f;
    //! Motion blur amount used while the attached cam is under water. 0x8CC7A4
    constexpr auto UNDERWATER_CAM_BLUR = 20;

    m_fFOV = 70.0f;

    const auto tiltAngle = DegreesToRadians(TheCamera.m_fAttachedCamAngle);

    // A ped and a vehicle behave the same here
    auto* const attached = TheCamera.m_pAttachedEntity;
    m_vecSource  = attached->GetMatrix().TransformVector(TheCamera.m_vecAttachedCamOffset);
    m_vecSource += attached->GetPosition();

    if (TheCamera.m_bLookingAtVector) {
        m_vecFront  = attached->GetMatrix().TransformVector(TheCamera.m_vecAttachedCamLookAt);
        m_vecFront += attached->GetPosition();
        m_vecFront -= m_vecSource;
    } else { // Look at the target entity instead
        m_vecFront = TheCamera.m_pTargetEntity->GetPosition() - m_vecSource;
    }
    m_vecFront.Normalise();

    auto tempUp    = CVector{ 0.0f, 0.0f, 1.0f };
    auto tempRight = CrossProduct(m_vecFront, tempUp);
    tempRight.Normalise();
    tempUp = CrossProduct(tempRight, m_vecFront);
    tempUp.Normalise(); // Probably not needed

    // Tint the screen with the water colour while we're submerged
    auto waterLevel = 0.0f;
    if (CWaterLevel::GetWaterLevel(m_vecSource.x, m_vecSource.y, m_vecSource.z, waterLevel, true, nullptr)
     && m_vecSource.z < waterLevel - 0.3f
    ) {
        const auto red   = CTimeCycle::GetWaterRed();
        const auto green = CTimeCycle::GetWaterGreen();
        const auto blue  = CTimeCycle::GetWaterBlue();

        auto scale = 1.0f;
        if (const auto mag = std::sqrt(red * red + green * green + blue * blue); mag > UNDERWATER_CAM_COLORMAG_LIMIT) {
            scale = UNDERWATER_CAM_COLORMAG_LIMIT / mag;
        }

        // `CCamera::SetMotionBlur` inlined
        TheCamera.m_nBlurRed    = (int32)(red * scale);
        TheCamera.m_nBlurGreen  = (int32)(green * scale);
        TheCamera.m_nBlurBlue   = (int32)(blue * scale);
        TheCamera.m_nMotionBlur = UNDERWATER_CAM_BLUR;
        TheCamera.m_nBlurType   = eMotionBlurType::LIGHT_SCENE;
    }

    // Roll the up vector so the camera tilts
    m_vecUp = tempUp * std::cos(tiltAngle) + tempRight * std::sin(tiltAngle);

    CWorld::pIgnoreEntity = nullptr;
}

// 0x525E50
// 0x513220
bool CCam::Process_Cam_TwoPlayer_TestLOSs(const CVector& tempSource) {
    CColPoint cp;
    CEntity*  hitEntity = nullptr;

    gCurCamColVars = CAM_COL_VARS_PLAYER_OUTSIDE_MED_RANGE; // This should be set back

    // If either player is obscured from here, this beta is no good
    return !CWorld::ProcessLineOfSight(tempSource, CWorld::Players[0].m_pPed->GetPosition(), cp, hitEntity, true, false, false, false, false, true, true, false)
        && !CWorld::ProcessLineOfSight(tempSource, CWorld::Players[1].m_pPed->GetPosition(), cp, hitEntity, true, false, false, false, false, true, true, false);
}

// 0x5132D0
void CCam::Process_Cam_TwoPlayer_CalcSource(float beta, CVector& outSource, CVector& outLookAt, CVector& outTarget) {
    const auto cosAlpha = std::cos(m_fVerticalAngle);
    outLookAt = CVector{ -std::cos(beta) * cosAlpha, -std::sin(beta) * cosAlpha, std::sin(m_fVerticalAngle) };

    const auto lookAtHor = CVector{ outLookAt.x, outLookAt.y, 0.0f }.Normalized();

    const auto& p0 = CWorld::Players[0].m_pPed->GetPosition();
    const auto& p1 = CWorld::Players[1].m_pPed->GetPosition();

    const auto camDist = 7.0f + (p0 - p1).Magnitude() * 0.67f;

    auto vec1To2 = p0 - p1;
    vec1To2.z    = 0.0f;
    vec1To2.Normalise();

    // Weight the midpoint towards whichever player the camera is looking past
    const auto player0Weight = 0.5f - 0.25f * DotProduct(vec1To2, lookAtHor);

    outTarget = p0 * player0Weight + p1 * (1.0f - player0Weight);
    outSource = outTarget - camDist * outLookAt;

    // Sit a little higher so we see a bit more at the top of the screen
    outSource.z += camDist * 0.1f;
}

// 0x525E50
void CCam::Process_Cam_TwoPlayer() {
    constexpr auto ANGLE_TRIES = 21;
    constexpr auto TEST_STEP   = 0.15f;

    constexpr auto CAM_2PLAYER_COLSWING_MULT = 0.2f;
    constexpr auto CAM_2PLAYER_COLSWING_CAP  = 0.1f;

    constexpr auto CAM_2PLAYER_STOP_MOVEMENT_INPUT_TIME = 1000u;
    constexpr auto CAM_2PLAYER_STOP_STICK_INPUT_TIME    = 1000u;
    constexpr auto TIME_BEFORE_EMERGENCY_2PLAYER_MODE   = 500u;

    constexpr auto AIMWEAPON_STICK_SENS = 0.007f; // 0x8CC4A0

    if (CPad::GetPad(0)->CycleCameraModeUpJustDown()) {
        CGameLogic::n2PlayerPedInFocus = CGameLogic::n2PlayerPedInFocus == eFocusedPlayer::PLAYER1
            ? eFocusedPlayer::NONE
            : eFocusedPlayer::PLAYER1;
    } else if (CPad::GetPad(1)->CycleCameraModeUpJustDown()) {
        CGameLogic::n2PlayerPedInFocus = CGameLogic::n2PlayerPedInFocus == eFocusedPlayer::PLAYER2
            ? eFocusedPlayer::NONE
            : eFocusedPlayer::PLAYER2;
    }

    // Focused on one player - just run that player's normal camera
    if (CGameLogic::n2PlayerPedInFocus != eFocusedPlayer::NONE) {
        const auto  playerIdx = (int32)CGameLogic::n2PlayerPedInFocus;
        auto* const ped       = CWorld::Players[playerIdx].m_pPed;

        if (ped->bInVehicle && ped->m_pVehicle) {
            m_pCamTargetEntity = ped->m_pVehicle;
            Process_FollowCar_SA(m_pCamTargetEntity->GetPosition(), 0.0f, 0.0f, 0.0f, false);
        } else {
            m_pCamTargetEntity = ped;
            Process_FollowPed_SA(ped->GetPosition(), 0.0f, 0.0f, 0.0f, false);
        }
        m_pCamTargetEntity = CWorld::Players[0].m_pPed;
        m_bResetStatics    = false;
        return;
    }

    const auto& camSet = PEDCAM_SET[FOLLOW_PED_2PLAYER];

    m_fVerticalAngle = DegreesToRadians(-30.0f);
    m_fAlphaSpeed    = 0.0f;

    CVector tempSource, tempFront;
    auto    testBeta  = m_fHorizontalAngle;

    // Find a beta with both players in view, sweeping outwards from the current one
    auto testCount = 0;
    while (testCount < ANGLE_TRIES) {
        testBeta = m_fHorizontalAngle + (testCount & 1 ? +1.0f : -1.0f) * (float)((testCount + 1) / 2) * TEST_STEP;

        Process_Cam_TwoPlayer_CalcSource(testBeta, tempSource, tempFront, m_vecTargetCoorsForFudgeInter);
        if (Process_Cam_TwoPlayer_TestLOSs(tempSource)) {
            gLastTime2PlayerCameraWasOK = CTimer::GetTimeInMS();
            break;
        }
        testCount++;
    }

    if (testCount == ANGLE_TRIES) { // Didn't find an angle - don't move
        testBeta = m_fHorizontalAngle;
    }
    if (testCount > 0) {
        gLastTime2PlayerCameraCollided = CTimer::GetTimeInMS();
    }

    if (m_bResetStatics) {
        m_fHorizontalAngle = testBeta;
    }

    //! Bring `beta` into [m_fHorizontalAngle - pi, m_fHorizontalAngle + pi]
    const auto WrapNearCurrentBeta = [this](float beta) {
        if (beta > m_fHorizontalAngle + PI) {
            return beta - TWO_PI;
        }
        if (beta < m_fHorizontalAngle - PI) {
            return beta + TWO_PI;
        }
        return beta;
    };

    auto targetBeta = std::clamp(
        CAM_2PLAYER_COLSWING_MULT * (WrapNearCurrentBeta(testBeta) - m_fHorizontalAngle),
        -CAM_2PLAYER_COLSWING_CAP,
        +CAM_2PLAYER_COLSWING_CAP
    ) + m_fHorizontalAngle;

    //! Nudge `testBeta` one step towards `offset`, then drop the offset if that lands in geometry
    const auto RejectOffsetThatCollides = [&](float& offset) {
        if (offset > 0.01f) {
            testBeta += TEST_STEP;
        } else if (offset < 0.01f) { // The original really does compare against +0.01 on both sides
            testBeta -= TEST_STEP;
        }

        if (std::abs(offset) <= 0.01f) {
            return;
        }
        Process_Cam_TwoPlayer_CalcSource(testBeta, tempSource, tempFront, m_vecTargetCoorsForFudgeInter);
        if (Process_Cam_TwoPlayer_TestLOSs(tempSource)) {
            return;
        }
        if ((offset > 0.0f && m_fBetaSpeed > 0.0f) || (offset < 0.0f && m_fBetaSpeed < 0.0f)) {
            m_fBetaSpeed = 0.0f;
        }
        gLastTime2PlayerCameraCollided = CTimer::GetTimeInMS();
        offset = 0.0f;
    };

    auto targetDiff = 0.0f;
    if (testCount == 0 && CTimer::GetTimeInMS() >= gLastTime2PlayerCameraCollided + CAM_2PLAYER_STOP_MOVEMENT_INPUT_TIME) {
        // Swing round to face the way the pair is moving
        const auto aveSpeed = CWorld::Players[0].m_pPed->m_vecMoveSpeed + CWorld::Players[1].m_pPed->m_vecMoveSpeed;
        if (aveSpeed.SquaredMagnitude() > 0.01f) {
            const auto diffMult = camSet.fDiffBetaSwing * CTimer::GetTimeStep();
            const auto diffCap  = camSet.fDiffBetaSwingCap * CTimer::GetTimeStep();

            auto headingBeta = std::atan2(-aveSpeed.x, aveSpeed.y) - HALF_PI;
            if (headingBeta - targetBeta > PI) {
                headingBeta -= TWO_PI;
            } else if (headingBeta - targetBeta < -PI) {
                headingBeta += TWO_PI;
            }

            targetDiff = std::clamp(
                std::min(1.0f, aveSpeed.Magnitude() * diffMult) * (headingBeta - targetBeta),
                -diffCap,
                +diffCap
            );
        }
        RejectOffsetThatCollides(targetDiff);
    }

    targetBeta = WrapNearCurrentBeta(targetBeta + targetDiff);

    auto camControlBetaSpeed = (targetBeta - m_fHorizontalAngle) / std::max(1.0f, CTimer::GetTimeStep());

    if (testCount == 0 && CTimer::GetTimeInMS() >= gLastTime2PlayerCameraCollided + CAM_2PLAYER_STOP_STICK_INPUT_TIME) {
        // Either player's right stick can push the camera around
        const auto stick = std::clamp(
            (float)(-CPad::GetPad(0)->AimWeaponLeftRight(CWorld::Players[0].m_pPed)
                    -CPad::GetPad(1)->AimWeaponLeftRight(CWorld::Players[1].m_pPed)),
            -128.0f,
            +128.0f
        );

        auto stickBetaOffset = sq(AIMWEAPON_STICK_SENS) * std::abs(stick) * stick * (0.25f / 3.5f * (m_fFOV / 80.0f));
        RejectOffsetThatCollides(stickBetaOffset);

        camControlBetaSpeed += stickBetaOffset;
    }

    camControlBetaSpeed = std::clamp(camControlBetaSpeed, -camSet.fDiffBetaCap, +camSet.fDiffBetaCap);

    const auto betaRate = std::pow(camSet.fDiffBetaRate, CTimer::GetTimeStep());
    m_fBetaSpeed        = camControlBetaSpeed * (1.0f - betaRate) + betaRate * m_fBetaSpeed;
    m_fHorizontalAngle += m_fBetaSpeed * CTimer::GetTimeStep();

    Process_Cam_TwoPlayer_CalcSource(m_fHorizontalAngle, m_vecSource, m_vecFront, m_vecTargetCoorsForFudgeInter);

    // One or both players are blocked - pull in to whatever is in the way and nag about focus mode
    if (testCount == ANGLE_TRIES && CTimer::GetTimeInMS() - gLastTime2PlayerCameraWasOK > TIME_BEFORE_EMERGENCY_2PLAYER_MODE) {
        CColPoint cp;
        CEntity*  hitEntity = nullptr;

        gCurCamColVars = CAM_COL_VARS_PLAYER_OUTSIDE_MED_RANGE;
        if (CWorld::ProcessLineOfSight(m_vecTargetCoorsForFudgeInter, m_vecSource, cp, hitEntity, true, false, false, false, false, true, true, false)) {
            m_vecSource = cp.m_vecPoint;
        }

        if (CTimer::GetTimeInMS() > (uint32)CGameLogic::nPrintFocusHelpTimer && CGameLogic::nPrintFocusHelpCounter < 6) {
            CHud::SetHelpMessage(TheText.Get("WRN2_2P"), false, false, false);
            CGameLogic::nPrintFocusHelpTimer = CTimer::GetTimeInMS() + 60'000;
            CGameLogic::nPrintFocusHelpCounter++;
        }
    }

    m_vecUp = CVector{ 0.0f, 0.0f, 1.0f };
    m_vecUp.Normalise();
    m_vecUp = CrossProduct(m_vecFront, CrossProduct(m_vecUp, m_vecFront).Normalized());
    m_vecUp.Normalise();

    m_fFOV          = 70.0f;
    m_bResetStatics = false;
}

// 0x519810
void CCam::Process_Cam_TwoPlayer_InCarAndShooting() {
    constexpr auto TWOPLAYER_INCARANDSHOOTING_DIST             = 0.5f;
    constexpr auto TWOPLAYER_INCARANDSHOOTING_DIST_MAX_ADD     = 2.0f;
    constexpr auto TWOPLAYER_INCARANDSHOOTING_ALPHA_DEFAULT    = DegreesToRadians(-20.0f);
    constexpr auto TWOPLAYER_INCARANDSHOOTING_ALPHA_STICK_RATE = 0.5f;
    constexpr auto TWOPLAYER_INCARANDSHOOTING_ALPHA_MIN        = DegreesToRadians(-60.0f);
    constexpr auto TWOPLAYER_INCARANDSHOOTING_ALPHA_MAX        = DegreesToRadians(30.0f);
    constexpr auto TWOPLAYER_INCARANDSHOOTING_ALPHA_RATE       = 0.96f;
    constexpr auto TWOPLAYER_INCARANDSHOOTING_ALPHA_SPEED_MULT = 5.0f;
    constexpr auto TWOPLAYER_INHELIANDSHOOTING_ALPHA_MIN       = DegreesToRadians(-60.0f);
    constexpr auto TWOPLAYER_INHELIANDSHOOTING_ALPHA_MAX       = DegreesToRadians(10.0f);
    constexpr auto TWOPLAYER_INHELIANDSHOOTING_ALPHA_SPEED_MULT = 3.0f;

    constexpr auto TWOPLAYER_AIM_STICK_SENS   = 0.004f;
    constexpr auto TWOPLAYER_LOCKON_TRACKRATE = 0.85f;
    constexpr auto TWOPLAYER_LOCKON_TRACKCAP  = 0.01f;

    constexpr auto INCAR_2PLAYER_WEAPON_RANGE_MULTIPLIER = 2.0f;
    constexpr auto MAX_TARGET_WEAPON_RANGE_MULTIPLIER    = 3.0f;

    constexpr auto TEST_CAM_ALPHA_RAISE_MULT      = 0.3f;  // 0x8CCD1C
    constexpr auto TEST_SHIFT_HELI_CAM_TARGET     = 0.6f;  // 0x8CC53C
    constexpr auto CAM_2PLAYER_GUN_FIRE_SHAKE     = 0.03f; // 0x8CCD18

    if (!m_pCamTargetEntity->GetIsTypeVehicle()) {
        return;
    }

    auto* const veh = CWorld::Players[0].m_pPed->m_pVehicle;

    auto       targetCoors      = veh->GetPosition();
    const auto targetOrientation = veh->GetHeading() - HALF_PI;

    // Whichever player isn't driving gets to look around and move the crosshair
    const auto isPlayer0Driving = veh->m_pDriver == CWorld::Players[0].m_pPed;
    auto* const pad2nd = CPad::GetPad(isPlayer0Driving ? 1 : 0);
    auto* const ped2nd = CWorld::Players[isPlayer0Driving ? 1 : 0].m_pPed;

    // Speed widens the m_fFOV, and it eases back to 70 when the car slows down
    const auto fwdSpeed = DotProduct(veh->m_vecMoveSpeed, veh->GetForwardVector());
    if ((veh->IsAutomobile() || veh->IsBike()) && fwdSpeed > CAR_FOV_START_SPEED) {
        m_fFOV += (fwdSpeed - CAR_FOV_START_SPEED) * CTimer::GetTimeStep();
    }
    if (m_fFOV > 70.0f) {
        m_fFOV = 70.0f + (m_fFOV - 70.0f) * std::pow(CAR_FOV_FADE_MULT, CTimer::GetTimeStep());
    }
    m_fFOV = std::clamp(m_fFOV, 70.0f, 100.0f);

    const auto stickX = (float)pad2nd->AimWeaponLeftRight(ped2nd);
    const auto stickY = (float)-pad2nd->AimWeaponUpDown(ped2nd);

    m_fX_Targetting += sq(TWOPLAYER_AIM_STICK_SENS) * std::abs(stickX) * stickX * (0.25f / 3.5f * (m_fFOV / 80.0f)) * CTimer::GetTimeStep();
    m_fY_Targetting += sq(TWOPLAYER_AIM_STICK_SENS) * std::abs(stickY) * stickY * (0.15f / 3.5f * (m_fFOV / 80.0f)) * CTimer::GetTimeStep();

    const auto* const weaponInfo = CWeaponInfo::GetWeaponInfo(ped2nd->GetActiveWeapon().m_Type, ped2nd->GetWeaponSkill());

    auto lockOnX = 0.0f, lockOnY = 0.0f;
    auto* const lockOnTarget = CWeapon::FindNearestTargetEntityWithScreenCoors(
        m_fX_Targetting,
        m_fY_Targetting,
        INCAR_2PLAYER_WEAPON_RANGE_MULTIPLIER * weaponInfo->m_fWeaponRange,
        ped2nd->GetPosition(),
        &lockOnX,
        &lockOnY
    );

    // Drag the crosshair towards the locked-on target, but only while the stick is near centred
    if (lockOnTarget && std::abs(stickX) < 120.0f && std::abs(stickY) < 120.0f) {
        const auto trackCap  = TWOPLAYER_LOCKON_TRACKCAP * CTimer::GetTimeStep();
        const auto trackRate = 1.0f - std::pow(TWOPLAYER_LOCKON_TRACKRATE, CTimer::GetTimeStep());

        m_fX_Targetting += std::clamp((lockOnX - m_fX_Targetting) * trackRate, -trackCap, +trackCap);
        m_fY_Targetting += std::clamp((lockOnY - m_fY_Targetting) * trackRate, -trackCap, +trackCap);
    }

    // Past the edge of the screen the crosshair stops and drags the camera around instead
    auto rotateCam = 0.0f;
    if (m_fX_Targetting > 0.9f) {
        rotateCam = m_fX_Targetting - 0.9f;
        m_fX_Targetting = 0.9f;
    }
    if (m_fX_Targetting < -0.9f) {
        rotateCam = m_fX_Targetting + 0.9f;
        m_fX_Targetting = -0.9f;
    }
    if (m_fY_Targetting > 0.9f) {
        m_fVerticalAngle -= TWOPLAYER_INCARANDSHOOTING_ALPHA_STICK_RATE * (m_fY_Targetting - 0.9f) * CTimer::GetTimeStep();
        m_fY_Targetting = 0.9f;
    }
    if (m_fY_Targetting < -0.9f) {
        m_fVerticalAngle -= TWOPLAYER_INCARANDSHOOTING_ALPHA_STICK_RATE * (m_fY_Targetting + 0.9f) * CTimer::GetTimeStep();
        m_fY_Targetting = -0.9f;
    }

    // Hands off the stick with nothing locked on - drift the crosshair back to its rest spot
    if (std::abs(stickX) < 1.0f && std::abs(stickY) < 1.0f && !lockOnTarget) {
        constexpr auto Y_REST = -0.4f;

        const auto deltaX = m_fX_Targetting;
        const auto deltaY = m_fY_Targetting - Y_REST;
        const auto diff   = std::sqrt(sq(deltaX) + sq(deltaY));
        const auto step   = CTimer::GetTimeStep() * 0.002f;

        if (diff < step) {
            m_fX_Targetting = 0.0f;
            m_fY_Targetting = Y_REST;
        } else {
            m_fX_Targetting -= deltaX * step / diff;
            m_fY_Targetting -= deltaY * step / diff;
        }
    }

    const auto carHeight = veh->GetColModel()->GetBoundingBox().m_vecMax.z;
    const auto carLength = 2.0f * std::abs(veh->GetColModel()->GetBoundingBox().m_vecMin.y);

    auto targetAlpha  = TWOPLAYER_INCARANDSHOOTING_ALPHA_DEFAULT;
    auto camDistance  = TWOPLAYER_INCARANDSHOOTING_DIST + carLength;

    // Full-size helis get a different offset from RC ones
    if (veh->GetVehicleAppearance() == VEHICLE_APPEARANCE_HELI && veh->GetStatus() != STATUS_REMOTE_CONTROLLED) {
        targetCoors += veh->GetUpVector() * TEST_SHIFT_HELI_CAM_TARGET * carHeight;
    } else {
        const auto targetZMod = carHeight * CARCAM_SET[FOLLOW_CAR_INCAR].fTargetOffsetZ - CARCAM_SET[FOLLOW_CAR_INCAR].fBaseCamZ;
        if (targetZMod > 0.0f) {
            targetCoors.z += targetZMod;
            camDistance   += targetZMod;
            targetAlpha   += TEST_CAM_ALPHA_RAISE_MULT * targetZMod / camDistance;
        }
    }

    m_fCaMinDistance = camDistance * 0.9f; // 0.9 because it's a 2D distance and alpha is 25 degrees
    m_fCaMaxDistance = camDistance + TWOPLAYER_INCARANDSHOOTING_DIST_MAX_ADD;

    // Classic cam-on-a-string: hold the camera within [min, max] of the car
    const auto vecDistance = m_vecSource - targetCoors;
    auto distMagnitude = CVector2D{ vecDistance }.Magnitude();
    m_fDistanceBeforeChanges = distMagnitude;
    if (distMagnitude < 0.002f) {
        distMagnitude = 0.002f;
    }

    m_fHorizontalAngle = std::atan2(-(targetCoors.x - m_vecSource.x), targetCoors.y - m_vecSource.y) - HALF_PI;

    if (distMagnitude > m_fCaMaxDistance) {
        m_vecSource.x = targetCoors.x + vecDistance.x * (m_fCaMaxDistance / distMagnitude);
        m_vecSource.y = targetCoors.y + vecDistance.y * (m_fCaMaxDistance / distMagnitude);
    } else if (distMagnitude < m_fCaMinDistance) {
        m_vecSource.x = targetCoors.x + vecDistance.x * (m_fCaMinDistance / distMagnitude);
        m_vecSource.y = targetCoors.y + vecDistance.y * (m_fCaMinDistance / distMagnitude);
    }

    // Tilt with the direction of travel
    if (veh->m_vecMoveSpeed.SquaredMagnitude() > sq(0.01f)) {
        auto speedAlpha = std::atan2(veh->m_vecMoveSpeed.z, veh->m_vecMoveSpeed.Magnitude2D());
        const auto speedAlphaMult = veh->IsHeli()
            ? TWOPLAYER_INHELIANDSHOOTING_ALPHA_SPEED_MULT
            : TWOPLAYER_INCARANDSHOOTING_ALPHA_SPEED_MULT;

        speedAlpha *= std::min(1.0f, speedAlphaMult * (veh->m_vecMoveSpeed.Magnitude() - 0.01f));
        targetAlpha += speedAlpha;

        const auto alphaRate = std::pow(TWOPLAYER_INCARANDSHOOTING_ALPHA_RATE, CTimer::GetTimeStep());
        m_fVerticalAngle = alphaRate * m_fVerticalAngle + (1.0f - alphaRate) * targetAlpha;
    }

    m_fVerticalAngle = veh->IsHeli()
        ? std::clamp(m_fVerticalAngle, TWOPLAYER_INHELIANDSHOOTING_ALPHA_MIN, TWOPLAYER_INHELIANDSHOOTING_ALPHA_MAX)
        : std::clamp(m_fVerticalAngle, TWOPLAYER_INCARANDSHOOTING_ALPHA_MIN, TWOPLAYER_INCARANDSHOOTING_ALPHA_MAX);

    m_vecSource.z = targetCoors.z - m_fCaMaxDistance * std::sin(m_fVerticalAngle);
    RotCamIfInFrontCar(targetCoors, targetOrientation);

    m_vecTargetCoorsForFudgeInter = targetCoors;
    const auto tempSource = m_vecSource;
    TheCamera.AvoidTheGeometry(tempSource, m_vecTargetCoorsForFudgeInter, m_vecSource, m_fFOV);

    { // The 2nd player rotating the camera around the car
        const auto diffX = m_vecSource.x - targetCoors.x;
        const auto diffY = m_vecSource.y - targetCoors.y;
        m_vecSource.x = targetCoors.x + diffX * std::cos(rotateCam) + diffY * std::sin(rotateCam);
        m_vecSource.y = targetCoors.y + diffY * std::cos(rotateCam) - diffX * std::sin(rotateCam);
    }

    m_vecFront      = targetCoors - m_vecSource;
    m_bResetStatics = false;
    GetVectorsReadyForRW();

    if (!veh->CanPedLeanOut(ped2nd)) {
        ped2nd->GetActiveWeapon().Update(ped2nd);
    }

    if (!pad2nd->GetCarGunFired()) {
        return;
    }

    if (veh->CanPedLeanOut(ped2nd)
        || ped2nd->GetActiveWeapon().IsTypeMelee()
        || ped2nd->GetActiveWeapon().m_State != WEAPONSTATE_READY
    ) {
        return;
    }

    const auto shootAt = [&] {
        if (lockOnTarget) {
            return lockOnTarget->GetPosition();
        }
        // Nothing locked on - shoot at wherever the crosshair is pointing
        const auto tanFov = std::tan(DegreesToRadians(m_fFOV) * 0.5f);
        auto dir = m_vecFront
                 + CrossProduct(m_vecFront, m_vecUp) * m_fX_Targetting * tanFov
                 - m_vecUp * (m_fY_Targetting * tanFov / CDraw::ms_fAspectRatio);
        dir *= MAX_TARGET_WEAPON_RANGE_MULTIPLIER * weaponInfo->m_fWeaponRange;
        return dir + m_vecSource;
    }();

    // Quantise the direction relative to the car into one of the four drive-by quadrants
    auto cameraAngle = std::atan2(-(shootAt - m_vecSource).x, (shootAt - m_vecSource).y) - veh->GetHeading();
    if (cameraAngle > PI) {
        cameraAngle -= TWO_PI;
    } else if (cameraAngle < -PI) {
        cameraAngle += TWO_PI;
    }
    auto angle = cameraAngle + PI / 4.0f;
    if (angle < 0.0f) {
        angle += TWO_PI;
    }

    CTaskSimpleGangDriveBy tempTask{ nullptr, nullptr, 100.0f, 100, eDrivebyStyle::AI_ALL_DIRN, ped2nd != veh->m_apPassengers[1] };
    tempTask.SetUpForCameraFire(
        CWeaponInfo::GetWeaponInfo(ped2nd->GetActiveWeapon().m_Type, ped2nd->GetWeaponSkill()),
        (int32)(angle / HALF_PI)
    );
    tempTask.FireGun(ped2nd);

    CamShakeNoPos(&TheCamera, CAM_2PLAYER_GUN_FIRE_SHAKE);
}

// 0x513510
void CCam::Process_Cam_TwoPlayer_Separate_Cars() {
    constexpr auto FOCUS_SWAP_RATE = 0.04f; // `TimeS` @ 0x8CCCB4

    m_fFOV = 80.0f;

    // Look from one car to the other; whichever car we're focussing on stays closest to the camera
    auto* const car0 = CWorld::Players[0].m_pPed->m_pVehicle;
    auto* const car1 = CWorld::Players[1].m_pPed->m_pVehicle;

    const auto to2ndCar = (car1->GetPosition() - car0->GetPosition()).Normalized();

    auto source0  = car0->GetPosition();
    source0.z    += CModelInfo::GetModelInfo(car0->m_nModelIndex)->m_pColModel->GetBoundingBox().m_vecMax.z + 1.0f;
    source0      -= to2ndCar * 6.0f;

    auto source1  = car1->GetPosition();
    source1.z    += CModelInfo::GetModelInfo(car1->m_nModelIndex)->m_pColModel->GetBoundingBox().m_vecMax.z + 1.0f;
    source1      += to2ndCar * 6.0f;

    const auto front0 = (car1->GetPosition() - source0).Normalized();
    const auto front1 = (car0->GetPosition() - source1).Normalized();

    // Side vector to swing the camera out along while interpolating between the two cars
    auto interSide = CVector{ source0.y - source1.y, source1.x - source0.x, 0.0f };
    interSide.Normalise();
    interSide.z = -0.1f; // Look down a wee bit
    interSide.Normalise();

    const auto interVal  = std::sin(m_fCarWeAreFocussingOnI * PI);
    const auto interVal2 = 0.5f + 0.5f * std::cos(m_fCarWeAreFocussingOnI * PI);

    const auto distApart = (car0->GetPosition() - car1->GetPosition()).Magnitude();

    m_vecSource = source0 * interVal2 + source1 * (1.0f - interVal2) - interSide * interVal * distApart * 0.75f;
    m_vecFront  = (front0 * interVal2 + front1 * (1.0f - interVal2)) * (1.0f - interVal) + interSide * interVal;
    m_vecFront.Normalise();

    m_vecTargetCoorsForFudgeInter = m_nCarWeAreFocussingOn
        ? car0->GetPosition()
        : car1->GetPosition();

    m_vecUp = CVector{ 0.0f, 0.0f, 1.0f };
    m_vecUp.Normalise();
    m_vecUp = CrossProduct(m_vecFront, CrossProduct(m_vecUp, m_vecFront).Normalized());
    m_vecUp.Normalise();

    // Looking backwards at speed reads badly, so hand the camera over to the other car
    auto* const mainCar = CWorld::Players[m_nCarWeAreFocussingOn].m_pPed->m_pVehicle;
    auto* const otherCar = CWorld::Players[(m_nCarWeAreFocussingOn + 1) & 1].m_pPed->m_pVehicle;

    auto frontHor = CVector{ m_vecFront.x, m_vecFront.y, 0.0f };
    frontHor.Normalise();

    const auto dot = DotProduct(mainCar->m_vecMoveSpeed, frontHor);
    // Only swap if the other guy wouldn't have the same problem
    if (dot < -0.13f && dot < DotProduct(otherCar->m_vecMoveSpeed, -frontHor)) {
        m_nCarWeAreFocussingOn = (m_nCarWeAreFocussingOn + 1) & 1;
    }

    const auto step = CTimer::GetTimeStep() * FOCUS_SWAP_RATE;
    m_fCarWeAreFocussingOnI = m_nCarWeAreFocussingOn
        ? std::min(m_fCarWeAreFocussingOnI + step, 1.0f)
        : std::max(m_fCarWeAreFocussingOnI - step, 0.0f);
}

// 0x513BE0
void CCam::Process_Cam_TwoPlayer_Separate_Cars_TopDown() {
    m_fFOV = 80.0f;
    const auto p1 = FindPlayerEntity(PED_TYPE_PLAYER1), p2 = FindPlayerEntity(PED_TYPE_PLAYER2);

    const auto p1p2Centroid = (p1->GetPosition() + p2->GetPosition()) / 2.0f;
    const auto p1p2Distance = DistanceBetweenPoints(p1->GetPosition(), p2->GetPosition());

    auto camHeightMult = std::abs([&] {
        if (FindPlayerVehicle(PED_TYPE_PLAYER1)) {
            return FindPlayerVehicle(PED_TYPE_PLAYER2) ? 1.0f : 0.75f;
        }
        return FindPlayerVehicle(PED_TYPE_PLAYER2) ? 0.75f : 0.45f;
    }() - m_fCameraHeightMultiplier);

    if (const auto s = CTimer::GetTimeStep() / 200.0f; camHeightMult >= s) {
        camHeightMult = (camHeightMult >= 0.0f ? s : -s) + m_fCameraHeightMultiplier;
    }
    m_fCameraHeightMultiplier = camHeightMult;

    const auto v21 = std::max(p1p2Distance + 10.0f, 30.0f);
    m_vecSource.Set(
        p1p2Centroid.x,
        p1p2Centroid.y - v21 * std::sin(0.4f),
        p1p2Centroid.z - v21 * -std::cos(0.4f)
    );
    m_vecFront.Set(0.0f, std::sin(0.4f), -std::cos(0.4f));
    m_vecUp.Set(0.0f, m_vecSource.y, std::sin(0.4f));
    m_vecTargetCoorsForFudgeInter = m_vecSource;
}

// 0x51B850
bool CCam::Process_DW_BirdyCam(bool bCheckValid) {
    constexpr auto DEFAULT_TIMEOUT      = 30;    // 0x8CCD74
    constexpr auto SCAN_DIST_AHEAD      = 50.0f; // 0x8CCD70
    constexpr auto SCAN_RANGE_AHEAD     = 2.0f;  // 0x8CCD6C
    constexpr auto SCAN_DIST_BEHIND     = 50.0f; // 0x8CCD68
    constexpr auto SCAN_RANGE_BEHIND    = 1.0f;  // 0x8CCD64
    constexpr auto HEIGHT_TOLERANCE     = 6.0f;  // 0x8CCD60
    constexpr auto TO_OTHER_SCALE       = 1.0f;  // 0x8CCD5C
    constexpr auto MAX_DURATION_MS      = 5000;  // 0x8CCBB0
    constexpr auto MAX_OBJS             = 128;

    static std::array<CVector, 2> lampPostPos{}; // 0xB70080
    static int32 s_timeout = DEFAULT_TIMEOUT;    // 0xB7007C

    TheCamera.m_bUseNearClipScript = false;

    if (!m_pCamTargetEntity || !m_pCamTargetEntity->GetIsTypeVehicle()) {
        return false;
    }

    CEntity*   entity{};
    CVehicle*  vehicle{};
    CVector    dst, src, targetUp, targetRight, targetFwd, targetVel, targetAngVel;
    float      targetSpeed{}, targetAngSpeed{};
    CColSphere sph;
    GetCoreDataForDWCineyCamMode(entity, vehicle, dst, src, targetUp, targetRight, targetFwd, targetVel, targetSpeed, targetAngVel, targetAngSpeed, sph);

    const auto curTime = CTimer::GetTimeInMS();
    const auto camId   = MODE_DW_BIRDY - MODE_DW_HELI_CHASE;

    if (gDWLastModeForCineyCam != MODE_DW_BIRDY || gLastFrameProcessedDWCineyCam < CTimer::GetFrameCounter() - 1) {
        gbExitCam[camId]       = false;
        gDWLastModeForCineyCam = MODE_DW_BIRDY;
        gDWCineyCamStartTime   = curTime;
        gDWCineyCamEndTime     = curTime + MAX_DURATION_MS;
        s_timeout              = DEFAULT_TIMEOUT;

        gHandShaker[0].Reset();

        //! Objects and dummies only - we're really only after lamp posts and traffic lights
        const auto FindLampPostsNear = [](const CVector& at, float radius, std::array<CEntity*, MAX_OBJS>& out) {
            std::array<CEntity*, MAX_OBJS> found{};
            int16 numFound{};
            CWorld::FindObjectsInRange(at, radius, true, &numFound, MAX_OBJS - 1, found.data(), false, false, false, true, true);

            size_t numLamps{};
            for (auto* const e : std::span{ found.data(), (size_t)numFound }) {
                if (e->GetIsStatic() && e->GetMatrix().GetUp().z > 0.9f && IsLampPost((eModelID)e->GetModelIndex())) {
                    out[numLamps++] = e;
                }
            }
            return numLamps;
        };

        // Scan ahead so we might get a shot of the car passing by us, and behind for the far post
        std::array<CEntity*, MAX_OBJS> lampsAhead{}, lampsBehind{};
        const auto numAhead  = FindLampPostsNear(dst + targetFwd * SCAN_DIST_AHEAD * SCAN_RANGE_AHEAD, SCAN_DIST_AHEAD, lampsAhead);
        const auto numBehind = FindLampPostsNear(dst - targetFwd * SCAN_DIST_BEHIND * SCAN_RANGE_BEHIND, SCAN_DIST_BEHIND, lampsBehind);

        //! Top of the post, lowered to roughly human head height
        const auto HeadHeightPosOf = [](CEntity* e) {
            const auto& bb = e->GetColModel()->GetBoundingBox();
            auto pos = e->GetMatrix().TransformPoint(bb.m_vecMax);
            pos.z   -= CGeneral::GetRandomNumberInRange(1.0f, bb.m_vecMax.z - bb.m_vecMin.z * 0.5f);
            return pos;
        };

        // Pick any two lamp posts that can see each other. The world's spatial split makes this
        // effectively random, and we quit as early as we can to keep the ray cast count down.
        auto ok = false;
        for (size_t i = 0; i < numAhead && !ok; i++) {
            auto* const lamp1 = lampsAhead[i];
            if (!lamp1->GetIsStatic() || lamp1->GetMatrix().GetUp().z <= 0.9f || !IsLampPost((eModelID)lamp1->GetModelIndex())) {
                continue;
            }

            const auto losStart1  = HeadHeightPosOf(lamp1);
            const auto losTarget  = (losStart1 - dst).Normalized() + dst;

            // `Abs(losTarget.z - losTarget.z)` in the original, so this is always taken
            if (std::abs(0.0f) >= HEIGHT_TOLERANCE) {
                continue;
            }

            for (size_t j = i; j < numBehind && !ok; j++) {
                auto* const lamp2 = lampsBehind[j];
                if (!lamp2->GetIsStatic() || lamp2->GetMatrix().GetUp().z <= 0.9f || !IsLampPost((eModelID)lamp2->GetModelIndex())) {
                    continue;
                }

                const auto losStart2 = HeadHeightPosOf(lamp2);
                if (CWorld::GetIsLineOfSightClear(losStart1, losStart2, true, false, false, false, false, true, true)
                 && CWorld::GetIsLineOfSightClear(losStart2, losTarget, true, false, false, false, false, true, true)
                ) {
                    src           = losStart1;
                    lampPostPos[0] = losStart1;
                    lampPostPos[1] = losStart2;
                    ok            = true;
                }
            }
        }

        if (!ok) {
            gbExitCam[camId] = true;
            return false;
        }
    }

    const auto t = (float)(curTime - gDWCineyCamStartTime) / (float)(gDWCineyCamEndTime - gDWCineyCamStartTime);

    // Note the posts are used the other way round here
    auto       goingFrom = lampPostPos[1];
    auto       goingTo   = lampPostPos[0];
    const auto toOther   = (goingFrom - goingTo).Normalized();
    goingFrom -= toOther * TO_OTHER_SCALE;
    goingTo   += toOther * TO_OTHER_SCALE * 2.0f;

    if (!gbExitCam[camId]) {
        src = CVector{
            SineAccelDecelLerp(t, goingFrom.x, goingTo.x),
            SineAccelDecelLerp(t, goingFrom.y, goingTo.y),
            SineAccelDecelLerp(t, goingFrom.z, goingTo.z)
        };
    }

    if (IsTimeToExitThisDWCineyCamMode(camId + DW_CINEYCAM_FIRST_ID, src, dst, t, false)) {
        gbExitCam[camId] = true;
        return false;
    }

    // Time out eventually if the car isn't visible from here
    CColPoint cp;
    CEntity*  hitEntity;

    CWorld::pIgnoreEntity = entity;
    const auto clear      = !CWorld::ProcessLineOfSight(dst, src, cp, hitEntity, true, true, false, false, false, false, false, false);
    CWorld::pIgnoreEntity = nullptr;

    if (!clear) {
        if (s_timeout-- == 0) {
            gbExitCam[camId] = true;
            return false;
        }
    } else if (s_timeout++ > DEFAULT_TIMEOUT) {
        s_timeout = DEFAULT_TIMEOUT;
    }

    Finalise_DW_CineyCams(src, dst, 0.0f, 70.0f, 0.3f, 0.0f);
    return true;
}

// 0x51B120
bool CCam::Process_DW_CamManCam(bool bCheckValid) {
    constexpr auto DEFAULT_TIMEOUT     = 100;    // 0x8CCD58
    constexpr auto SCAN_DIST           = 50.0f;  // 0x8CCD54
    constexpr auto MIN_ACCEPT_DIST     = 5.0f;   // 0x8CCD50
    constexpr auto HEIGHT_TOLERANCE    = 6.0f;   // 0x8CCD4C
    constexpr auto RADIUS_AROUND_POST  = 1.0f;   // 0x8CCD48
    constexpr auto LEN_DIV             = 30.0f;  // 0x8CCD44 - distance the m_fFOV lerp takes place over
    constexpr auto FAR_FOV             = 15.0f;  // 0x8CCD40
    constexpr auto NEAR_FOV            = 70.0f;  // 0x8CCD3C
    constexpr auto TIME_TO_ZOOM_IN     = 0.1f;   // 0x8CCD38
    constexpr auto START_FOV           = 70.0f;  // 0x8CCD34
    constexpr auto SHAKE_SPEED_MULT    = 8.0f;   // 0x8CCD30
    constexpr auto SHAKE_MIN           = 0.2f;   // `minTmLen`
    constexpr auto MAX_DURATION_MS     = 10000;  // 0x8CCBAC

    static CVector lampPostPos{};             // 0xB70068
    static int32   s_timeout = DEFAULT_TIMEOUT; // 0xB70074

    TheCamera.m_bUseNearClipScript = false;

    if (!m_pCamTargetEntity || !m_pCamTargetEntity->GetIsTypeVehicle()) {
        return false;
    }

    CEntity*   entity{};
    CVehicle*  vehicle{};
    CVector    dst, src, targetUp, targetRight, targetFwd, targetVel, targetAngVel;
    float      targetSpeed{}, targetAngSpeed{};
    CColSphere sph;
    GetCoreDataForDWCineyCamMode(entity, vehicle, dst, src, targetUp, targetRight, targetFwd, targetVel, targetSpeed, targetAngVel, targetAngSpeed, sph);

    const auto curTime = CTimer::GetTimeInMS();
    const auto camId   = MODE_DW_CAM_MAN - MODE_DW_HELI_CHASE;

    if (gDWLastModeForCineyCam != MODE_DW_CAM_MAN || gLastFrameProcessedDWCineyCam < CTimer::GetFrameCounter() - 1) {
        gbExitCam[camId]       = false;
        gDWLastModeForCineyCam = MODE_DW_CAM_MAN;
        gDWCineyCamStartTime   = curTime;
        gDWCineyCamEndTime     = curTime + MAX_DURATION_MS;
        s_timeout              = DEFAULT_TIMEOUT;

        gHandShaker[0].Reset();

        // Stand the cameraman by a lamp post we might drive towards. Scan ahead so we get a
        // shot of the car passing by. Objects and dummies only.
        std::array<CEntity*, 16> found{};
        int16 numFound{};
        CWorld::FindObjectsInRange(dst + targetFwd * SCAN_DIST, SCAN_DIST, true, &numFound, 15, found.data(), false, false, false, true, true);

        CEntity* bestPost = nullptr;
        auto     bestDist = 10000.0f;

        for (auto* const e : std::span{ found.data(), (size_t)numFound }) {
            if (!e->GetIsStatic() || e->GetMatrix().GetUp().z <= 0.9f || !IsLampPost((eModelID)e->GetModelIndex())) {
                continue;
            }

            const auto dist = (e->GetPosition() - dst).Magnitude2D();
            if (dist >= bestDist || dist <= MIN_ACCEPT_DIST) {
                continue;
            }

            const auto& bb = e->GetColModel()->GetBoundingBox();

            // Top of the post, lowered to roughly human head height
            auto losStart  = e->GetMatrix().TransformPoint(bb.m_vecMax);
            losStart.z    -= bb.m_vecMax.z;
            losStart.z    += bb.m_vecMin.z * 0.5f;

            const auto losTarget = (losStart - dst).Normalized() + dst;

            // `Abs(losTarget.z - losTarget.z)` in the original, so this is always taken
            if (std::abs(0.0f) >= HEIGHT_TOLERANCE) {
                continue;
            }

            if (CWorld::GetIsLineOfSightClear(losStart, losTarget, true, false, false, false, false, true, true)) {
                bestDist    = dist;
                bestPost    = e;
                src         = losStart;
                lampPostPos = losStart;
            }
        }

        if (!bestPost) {
            gbExitCam[camId] = true;
            return false;
        }
    }

    const auto t = (float)(curTime - gDWCineyCamStartTime) / (float)(gDWCineyCamEndTime - gDWCineyCamStartTime);

    // Step off the post so it doesn't obscure the camera
    if (!gbExitCam[camId]) {
        src = lampPostPos + (dst - lampPostPos).Normalized() * RADIUS_AROUND_POST;
    }

    // Zoom in to the m_fFOV we want rather than snapping to it
    auto newFOV = SineAccelDecelLerp(std::clamp((dst - src).Magnitude() / LEN_DIV, 0.0f, 1.0f), NEAR_FOV, FAR_FOV);
    if (t < TIME_TO_ZOOM_IN) {
        newFOV = SineAccelDecelLerp(std::clamp(t / TIME_TO_ZOOM_IN, 0.0f, 1.0f), START_FOV, newFOV);
    }

    if (IsTimeToExitThisDWCineyCamMode(camId + DW_CINEYCAM_FIRST_ID, src, dst, t, false)) {
        gbExitCam[camId] = true;
        return false;
    }

    // Time out eventually if the car isn't visible from here
    CColPoint cp;
    CEntity*  hitEntity;

    CWorld::pIgnoreEntity = entity;
    const auto clear      = !CWorld::ProcessLineOfSight(dst, src, cp, hitEntity, true, true, false, false, false, false, false, false);
    CWorld::pIgnoreEntity = nullptr;

    if (!clear) {
        if (s_timeout-- == 0) {
            gbExitCam[camId] = true;
            return false;
        }
    } else if (s_timeout++ > DEFAULT_TIMEOUT) {
        s_timeout = DEFAULT_TIMEOUT;
    }

    // Shake harder the faster the vehicle is moving
    const auto shakeDegree = std::clamp(std::max(targetVel.Magnitude() * SHAKE_SPEED_MULT, SHAKE_MIN), 0.0f, 1.0f);

    Finalise_DW_CineyCams(src, dst, 0.0f, newFOV, std::lerp(10.0f, 0.3f, newFOV / 70.0f), shakeDegree);
    return true;
}

// 0x51A740
//! Tuning + running state for `Process_DW_HeliChaseCam`. There is only ever one of these.
struct CHeliCamSettings {
    CVector heliMoveTo;
    CVector heliMoveFrom;
    float   heliSpeed;                //!< Speed of the helicopter roughly over its journey
    float   behindStart;              //!< Distance behind the player to start from
    float   heliHeight;               //!< Helicopter height above the player
    float   heliOutSideOfVehicle;     //!< Offset along the car's side vector
    float   zoomInTime;               //!< Time taken to do the first zoom in
    float   zoomFOVStart;             //!< m_fFOV lerp start for the first zoom
    float   zoomFOVEnd;               //!< m_fFOV lerp end for the first zoom
    float   heliSpeedLookInFrontMul;  //!< The heli can look in front of the vehicle
    float   heliRollScale;            //!< Degree of roll in the heli over time
    float   heliClip;                 //!< Near clip distance for the heli
    bool    bLockDest;                //!< Are we locking what we're looking at?
    int32   lockDestTimeOut;
    int32   lockDestHeliTimeoutDefault;
    CVector lockedDest;               //!< The cached destination for the locked look-at
    int32   numAttemptsToFindAValidOne;
    bool    bHeliCollided;            //!< Did the simulated heli hit a building?
    int32   defaultFramesOutOfSightBeforeWeLoseTheCar;
    int32   framesBeenOutOfSight;
    float   heliDistFOVZoomMore;      //!< Car-to-heli distance that starts the second m_fFOV zoom
    float   heliDistFOVZoomMoreMax;   //!< Car-to-heli distance that ends the second m_fFOV zoom
    float   lessFOV;                  //!< Amount of m_fFOV zoom in the second lerp
    float   lenToCarToPushCameraOut;  //!< Min 2D distance, stops a 180 degree flip in camera roll
    float   radiusOfSphereAroundHeli; //!< Keeps the camera out of collision, allowing for the near clip
    float   zoomOutFOV;               //!< m_fFOV we zoom out to when we lose the car
    bool    bWeLostTheCar;
    bool    bNoZoom;                  //!< Disables the zoom at the start
    float   fovZoomBackOutFrom;
    bool    bZoomBackOut;
    int32   zoomBackOutTimeStart;
    int32   zoomBackOutTimeEnd;
    float   timeToZoomOut;            //!< At what fraction of time to start resuming a non-zoomed view
    int32   zoomBackOutSpeed;         //!< How many milliseconds to zoom back out over

    void Reset() {
        bLockDest            = false;
        lockDestTimeOut      = lockDestHeliTimeoutDefault;
        bHeliCollided        = false;
        bWeLostTheCar        = false;
        bNoZoom              = false;
        zoomOutFOV           = zoomFOVStart - zoomFOVEnd;
        framesBeenOutOfSight = defaultFramesOutOfSightBeforeWeLoseTheCar;
        bZoomBackOut         = false;
    }

    // 0x517800
    void SetDefault() {
        heliSpeed                                  = 50.0f;
        behindStart                                = 30.0f;
        heliHeight                                 = 55.0f;
        heliOutSideOfVehicle                       = 50.0f;
        zoomInTime                                 = 0.05f;
        zoomFOVStart                               = 70.0f;
        zoomFOVEnd                                 = 22.0f;
        heliSpeedLookInFrontMul                    = 1.0f;
        heliRollScale                              = 0.0f;
        heliClip                                   = 10.0f;
        lockDestHeliTimeoutDefault                 = 30;
        numAttemptsToFindAValidOne                 = 8;
        defaultFramesOutOfSightBeforeWeLoseTheCar  = 60;
        heliDistFOVZoomMore                        = 100.0f;
        heliDistFOVZoomMoreMax                     = 110.0f;
        lessFOV                                    = 10.0f;
        lenToCarToPushCameraOut                    = 5.0f;
        radiusOfSphereAroundHeli                   = 12.0f;
        timeToZoomOut                              = 0.75f;
        zoomBackOutSpeed                           = 4000;

        Reset();
    }

    // 0x5178B0
    void RandomiseABit() {
        heliSpeed            *= CGeneral::GetRandomNumberInRange(0.1f, 1.5f);
        behindStart          *= CGeneral::GetRandomNumberInRange(0.5f, 1.0f);
        heliHeight           *= CGeneral::GetRandomNumberInRange(0.5f, 1.0f);
        heliOutSideOfVehicle *= CGeneral::GetRandomNumberInRange(0.5f, 1.0f);
        zoomInTime           *= CGeneral::GetRandomNumberInRange(0.5f, 2.0f);
    }
};
VALIDATE_SIZE(CHeliCamSettings, 0x9C);

static inline auto& gHCM = StaticRef<CHeliCamSettings>(0xB6FEC0);

// 0x51A740
bool CCam::Process_DW_HeliChaseCam(bool bCheckValid) {
    constexpr auto MAX_DURATION_MS       = 20000; // 0x8CCBA8
    constexpr auto BUILDING_CHECK_RADIUS = 15.0f; // 0x8CCD28
    constexpr auto SLOW_DOWN_DEGREE      = 0.5f;  // 0x8CCD20

    static CVector slowPos{};           // 0xB70058
    static int32   s_timeSlowTime = 100; // 0x8CCD24

    TheCamera.m_bUseNearClipScript = false;

    // `NUM_HELI_CAM_SETTINGS` is 1, so the random pick always lands on the only entry
    auto& hcm = gHCM;

    if (!m_pCamTargetEntity || !m_pCamTargetEntity->GetIsTypeVehicle()) {
        return false;
    }

    CEntity*   entity{};
    CVehicle*  vehicle{};
    CVector    dst, src, targetUp, targetRight, targetFwd, targetVel, targetAngVel;
    float      targetSpeed{}, targetAngSpeed{};
    CColSphere sph;
    GetCoreDataForDWCineyCamMode(entity, vehicle, dst, src, targetUp, targetRight, targetFwd, targetVel, targetSpeed, targetAngVel, targetAngSpeed, sph);

    const auto curTime = CTimer::GetTimeInMS();
    const auto camId   = MODE_DW_HELI_CHASE - MODE_DW_HELI_CHASE;

    if (gDWLastModeForCineyCam != MODE_DW_HELI_CHASE || gLastFrameProcessedDWCineyCam < CTimer::GetFrameCounter() - 1) {
        gbExitCam[camId]       = false;
        gDWLastModeForCineyCam = MODE_DW_HELI_CHASE;
        gDWCineyCamStartTime   = curTime;
        gDWCineyCamEndTime     = curTime + MAX_DURATION_MS;

        hcm.SetDefault();
        hcm.RandomiseABit();
        gHandShaker[0].Reset();

        // Pick two points in the sky for the heli to fly between. It sits far enough away that
        // it can hover and keep the player in shot: zoom in quickly first, then just track.
        auto isValid = false;
        for (auto i = 0; i < hcm.numAttemptsToFindAValidOne; i++) {
            hcm.heliMoveFrom    = dst - targetFwd * hcm.behindStart;
            hcm.heliMoveTo      = dst + targetFwd * hcm.heliSpeed;
            hcm.heliMoveFrom.z += hcm.heliHeight;
            hcm.heliMoveTo.z   += hcm.heliHeight;

            // Make the heli cross the vehicle's path, randomly
            const auto mul1 = GetRandomTrueFalse() ? -1.0f : 1.0f;
            const auto mul2 = GetRandomTrueFalse() ? -1.0f : 1.0f;

            const auto sideVec = CVector{ targetRight.x, targetRight.y, 0.0f };
            hcm.heliMoveFrom += sideVec * hcm.heliOutSideOfVehicle * mul1;
            hcm.heliMoveTo   += sideVec * hcm.heliOutSideOfVehicle * mul2;

            // Now we know where we're going - is it safe to go there?
            if (CWorld::TestSphereAgainstWorld(hcm.heliMoveFrom, hcm.radiusOfSphereAroundHeli, nullptr, true, true, false, false, false, false)) {
                continue;
            }

            CColPoint cp;
            CEntity*  hitEntity;

            CWorld::pIgnoreEntity = entity;
            const auto clear      = !CWorld::ProcessLineOfSight(dst, hcm.heliMoveFrom, cp, hitEntity, true, true, false, false, false, false, false, false);
            CWorld::pIgnoreEntity = nullptr;

            if (clear) {
                isValid = true;
                break;
            }
        }

        if (!isValid) {
            hcm.Reset();
            gbExitCam[camId] = true;
            return false;
        }

        hcm.bNoZoom      = GetRandomTrueFalse();
        hcm.bZoomBackOut = GetRandomTrueFalse();
    }

    // We found this camera to be invalid last time we processed it
    if (gbExitCam[camId]) {
        return false;
    }

    const auto t = (float)(curTime - gDWCineyCamStartTime) / (float)(gDWCineyCamEndTime - gDWCineyCamStartTime);

    src = CVector{
        std::lerp(hcm.heliMoveFrom.x, hcm.heliMoveTo.x, t),
        std::lerp(hcm.heliMoveFrom.y, hcm.heliMoveTo.y, t),
        std::lerp(hcm.heliMoveFrom.z, hcm.heliMoveTo.z, t)
    };

    dst += targetFwd + targetFwd * targetSpeed * hcm.heliSpeedLookInFrontMul;

    { // Prevent a 180 degree flip when going right over the car
        auto       toCar    = dst - src;
        const auto lenToCar = toCar.Magnitude2D();
        toCar /= lenToCar;

        if (lenToCar < hcm.lenToCarToPushCameraOut) {
            src.x = dst.x - toCar.x * hcm.lenToCarToPushCameraOut;
            src.y = dst.y - toCar.y * hcm.lenToCarToPushCameraOut;
        }
    }

    auto newFOV = hcm.zoomFOVEnd;
    if (t < hcm.zoomInTime && !hcm.bNoZoom) { // Zoom in at the start
        newFOV = SineAccelDecelLerp(t / hcm.zoomInTime, hcm.zoomFOVStart, hcm.zoomFOVEnd);
    }

    // Really far away now - zoom in again a wee bit
    if (const auto dist = (src - dst).Magnitude(); dist > hcm.heliDistFOVZoomMore) {
        const auto zoomT = std::clamp((dist - hcm.heliDistFOVZoomMore) / (hcm.heliDistFOVZoomMoreMax - hcm.heliDistFOVZoomMore), 0.0f, 1.0f);
        newFOV -= SineAccelDecelLerp(zoomT, 0.0f, hcm.lessFOV);
    }

    const auto roll = t * hcm.heliRollScale; // Shouldn't really be used - the scale is 0

    // Hit a building or something - slow the motion right down instead of stopping dead
    if (hcm.bHeliCollided || CWorld::TestSphereAgainstWorld(src, BUILDING_CHECK_RADIUS, nullptr, true, true, false, false, false, false)) {
        if (!hcm.bHeliCollided) {
            hcm.bHeliCollided = true;
            s_timeSlowTime    = 100;
            slowPos           = src;
        }

        if (s_timeSlowTime-- < 0) {
            // The camera can't move any more, but that isn't a reason to quit
            src              = DWCineyCamLastPos;
            gbExitCam[camId] = true;
            return false;
        }
        src = slowPos + (src - slowPos) * SLOW_DOWN_DEGREE;
    }

    if (hcm.bLockDest) {
        dst = hcm.lockedDest;
        if (hcm.lockDestTimeOut-- == 0) {
            gbExitCam[camId] = true;
            return false;
        }
    } else {
        CColPoint cp;
        CEntity*  hitEntity;

        CWorld::pIgnoreEntity = entity;
        const auto clear      = !CWorld::ProcessLineOfSight(dst, src, cp, hitEntity, true, true, false, false, false, false, false, false);
        CWorld::pIgnoreEntity = nullptr;

        if (!clear) {
            hcm.bWeLostTheCar = true;

            if (!hcm.bZoomBackOut && hcm.framesBeenOutOfSight < hcm.defaultFramesOutOfSightBeforeWeLoseTheCar / 4) {
                hcm.fovZoomBackOutFrom   = newFOV; // Show that we lost the car
                hcm.bZoomBackOut         = true;
                hcm.zoomBackOutTimeStart = curTime;
                hcm.zoomBackOutTimeEnd   = curTime + hcm.zoomBackOutSpeed;
            }

            if (hcm.framesBeenOutOfSight-- == 0) { // Damn, we lost him - lock the look-at position
                hcm.lockedDest = dst;
                hcm.bLockDest  = true;
            }
        } else if (++hcm.framesBeenOutOfSight > hcm.defaultFramesOutOfSightBeforeWeLoseTheCar) {
            hcm.framesBeenOutOfSight = hcm.defaultFramesOutOfSightBeforeWeLoseTheCar;
        }
    }

    if (!hcm.bZoomBackOut && t >= hcm.timeToZoomOut) {
        hcm.fovZoomBackOutFrom   = newFOV;
        hcm.bZoomBackOut         = true;
        hcm.zoomBackOutTimeStart = curTime;
        hcm.zoomBackOutTimeEnd   = curTime + hcm.zoomBackOutSpeed;
    }

    if (hcm.bZoomBackOut) {
        const auto zt = std::clamp(
            (float)(curTime - hcm.zoomBackOutTimeStart) / (float)(hcm.zoomBackOutTimeEnd - hcm.zoomBackOutTimeStart),
            0.0f,
            1.0f
        );
        newFOV = SineAccelDecelLerp(zt, hcm.fovZoomBackOutFrom, hcm.zoomFOVStart);
    }

    if (IsTimeToExitThisDWCineyCamMode(camId + DW_CINEYCAM_FIRST_ID, src, dst, t, false)) {
        gbExitCam[camId] = true;
        return false;
    }

    // Generous near clip to keep the z-fighting off the tops of cars
    Finalise_DW_CineyCams(src, dst, roll, newFOV, hcm.heliClip, 1.0f);
    return true;
}

// 0x51C760
bool CCam::Process_DW_PlaneCam1(bool bCheckValid) {
    constexpr auto MIN_DISTANCE_TO_GROUND    = 80.0f;   // 0x8CCDBC
    constexpr auto MIN_CAM_ROUGH_DIST_GROUND = 30.0f;   // 0x8CCDB8
    constexpr auto MAX_DURATION_MS           = 12000;   // 0x8CCBC0
    constexpr auto DIST_FORWARD              = 10.0f;   // 0x8CCDB0
    constexpr auto DIST_SIDE                 = 30.0f;   // 0x8CCDAC
    constexpr auto DIST_UP                   = 15.0f;   // 0x8CCDA8
    constexpr auto PITCH_Y_FACTOR            = -150.0f; // 0x8CCDA4
    constexpr auto AIR_WAVE_FLOATING         = 0.5f;    // 0x8CCD9C
    constexpr auto AIR_WAVE_NUM_WAVES        = 4;       // 0x8CCDA0
    constexpr auto DEFAULT_TIMEOUT           = 100;     // 0x8CCD98

    static float dirMove{};                   // 0xB700C0
    static float randSign  = 1.0f;            // 0x8CCDB4 - initialised data, not a scoped static
    static int32 s_timeout = DEFAULT_TIMEOUT; // 0xB700BC

    TheCamera.m_bUseNearClipScript = false;

    if (!m_pCamTargetEntity || !m_pCamTargetEntity->GetIsTypeVehicle()) {
        return false;
    }

    CEntity*   entity{};
    CVehicle*  vehicle{};
    CVector    dst, src, targetUp, targetRight, targetFwd, targetVel, targetAngVel;
    float      targetSpeed{}, targetAngSpeed{};
    CColSphere sph;
    GetCoreDataForDWCineyCamMode(entity, vehicle, dst, src, targetUp, targetRight, targetFwd, targetVel, targetSpeed, targetAngVel, targetAngSpeed, sph);

    const auto curTime = CTimer::GetTimeInMS();
    const auto camId   = MODE_DW_PLANECAM1 - MODE_DW_HELI_CHASE;

    // A guess rather than a ray cast - we really want the distance to the sector bounding box max z
    if (dst.z < MIN_DISTANCE_TO_GROUND) {
        gbExitCam[camId] = true;
        return false;
    }

    if (gDWLastModeForCineyCam != MODE_DW_PLANECAM1 || gLastFrameProcessedDWCineyCam < CTimer::GetFrameCounter() - 1) {
        gbExitCam[camId]       = false;
        gDWLastModeForCineyCam = MODE_DW_PLANECAM1;
        gDWCineyCamStartTime   = curTime;
        gDWCineyCamEndTime     = curTime + MAX_DURATION_MS;

        CColPoint cp;
        CEntity*  hitEntity;

        CWorld::pIgnoreEntity = entity;
        const auto blocked    = CWorld::ProcessLineOfSight(dst, src, cp, hitEntity, true, true, false, false, false, false, false, false);
        CWorld::pIgnoreEntity = nullptr;

        if (blocked) {
            gbExitCam[camId] = true;
            return false;
        }

        // Go from above to below or the other way around
        dirMove = 1.0f;
        if (GetRandomTrueFalse()) {
            dirMove *= -1.0f;
        }
        if (GetRandomTrueFalse()) {
            randSign = -1.0f;
        }
    } else if (TheCamera.GetRoughDistanceToGround() < MIN_CAM_ROUGH_DIST_GROUND) {
        gbExitCam[camId] = true;
        return false;
    }

    const auto t = (float)(curTime - gDWCineyCamStartTime) / (float)(gDWCineyCamEndTime - gDWCineyCamStartTime);

    const auto fwd   = targetFwd.Normalized();
    const auto right = targetRight.Normalized() * randSign;

    src  = dst + fwd * DIST_FORWARD;
    src += right * DIST_SIDE;
    src += targetUp * DIST_UP;
    src += targetUp * PITCH_Y_FACTOR * (t - 0.5f) * dirMove;
    src += targetUp * AIR_WAVE_FLOATING * WaveFunc(curTime, gDWCineyCamStartTime, gDWCineyCamEndTime, AIR_WAVE_NUM_WAVES);

    // Time out eventually if the plane isn't visible from here
    CColPoint cp;
    CEntity*  hitEntity;

    CWorld::pIgnoreEntity = entity;
    const auto clear      = !CWorld::ProcessLineOfSight(dst, src, cp, hitEntity, true, true, false, false, false, false, false, false);
    CWorld::pIgnoreEntity = nullptr;

    if (!clear) {
        if (s_timeout-- == 0) {
            gbExitCam[camId] = true;
            return false;
        }
    } else if (s_timeout++ > DEFAULT_TIMEOUT) {
        s_timeout = DEFAULT_TIMEOUT;
    }

    if (IsTimeToExitThisDWCineyCamMode(camId + DW_CINEYCAM_FIRST_ID, src, dst, t, false)) {
        gbExitCam[camId] = true;
        return false;
    }

    Finalise_DW_CineyCams(src, dst, 0.0f, 70.0f, 5.0f, 1.0f);
    return true;
}

// 0x51CC30
bool CCam::Process_DW_PlaneCam2(bool bCheckValid) {
    constexpr auto MIN_DISTANCE_TO_GROUND = 80.0f; // 0x8CCDDC
    constexpr auto MAX_DURATION_MS        = 7000;  // 0x8CCBC4
    constexpr auto DIST_FORWARD           = 30.0f; // 0x8CCDD4
    constexpr auto DIST_SIDE              = 30.0f; // 0x8CCDD0
    constexpr auto DIST_UP                = 5.0f;  // 0x8CCDCC
    constexpr auto AIR_WAVE_FLOATING      = 0.5f;  // 0x8CCDC4
    constexpr auto AIR_WAVE_NUM_WAVES     = 4;     // 0x8CCDC8
    constexpr auto DEFAULT_TIMEOUT        = 100;   // 0x8CCDC0

    static float dirMove2{};                  // 0xB700C4
    static float dirMove3{};                  // 0xB700C8
    static float randSign2 = 1.0f;            // 0x8CCDD8 - initialised data, not a scoped static
    static int32 s_timeout = DEFAULT_TIMEOUT; // 0xB700D0

    TheCamera.m_bUseNearClipScript = false;

    if (!m_pCamTargetEntity || !m_pCamTargetEntity->GetIsTypeVehicle()) {
        return false;
    }

    CEntity*   entity{};
    CVehicle*  vehicle{};
    CVector    dst, src, targetUp, targetRight, targetFwd, targetVel, targetAngVel;
    float      targetSpeed{}, targetAngSpeed{};
    CColSphere sph;
    GetCoreDataForDWCineyCamMode(entity, vehicle, dst, src, targetUp, targetRight, targetFwd, targetVel, targetSpeed, targetAngVel, targetAngSpeed, sph);

    const auto curTime = CTimer::GetTimeInMS();
    const auto camId   = MODE_DW_PLANECAM2 - MODE_DW_HELI_CHASE;

    if (dst.z < MIN_DISTANCE_TO_GROUND) {
        gbExitCam[camId] = true;
        return false;
    }

    if (gDWLastModeForCineyCam != MODE_DW_PLANECAM2 || gLastFrameProcessedDWCineyCam < CTimer::GetFrameCounter() - 1) {
        gbExitCam[camId]       = false;
        gDWLastModeForCineyCam = MODE_DW_PLANECAM2;
        gDWCineyCamStartTime   = curTime;
        gDWCineyCamEndTime     = curTime + MAX_DURATION_MS;

        CColPoint cp;
        CEntity*  hitEntity;

        CWorld::pIgnoreEntity = entity;
        const auto blocked    = CWorld::ProcessLineOfSight(dst, src, cp, hitEntity, true, true, false, false, false, false, false, false);
        CWorld::pIgnoreEntity = nullptr;

        if (blocked) {
            gbExitCam[camId] = true;
            return false;
        }

        // Go from above to below or the other way around
        dirMove2 = 1.0f;
        if (GetRandomTrueFalse()) {
            dirMove2 *= -1.0f;
        }
        dirMove3 = 1.0f;
        if (GetRandomTrueFalse()) {
            dirMove3 *= -1.0f;
        }
        if (GetRandomTrueFalse()) {
            randSign2 = -1.0f;
        }
    }

    const auto t = (float)(curTime - gDWCineyCamStartTime) / (float)(gDWCineyCamEndTime - gDWCineyCamStartTime);

    // Swing from in front of the plane round to behind it, closing in sideways as we go
    const auto fwd   = targetFwd.Normalized() * std::lerp(1.0f, -1.0f, t) * dirMove3;
    const auto right = targetRight.Normalized() * (1.0f - t) * randSign2;

    src  = dst + fwd * DIST_FORWARD;
    src += right * DIST_SIDE;
    src += targetUp * DIST_UP;
    src += targetUp * AIR_WAVE_FLOATING * WaveFunc(curTime, gDWCineyCamStartTime, gDWCineyCamEndTime, AIR_WAVE_NUM_WAVES);

    // Time out eventually if the plane isn't visible from here
    CColPoint cp;
    CEntity*  hitEntity;

    CWorld::pIgnoreEntity = entity;
    const auto clear      = !CWorld::ProcessLineOfSight(dst, src, cp, hitEntity, true, true, false, false, false, false, false, false);
    CWorld::pIgnoreEntity = nullptr;

    if (!clear) {
        if (s_timeout-- == 0) {
            gbExitCam[camId] = true;
            return false;
        }
    } else if (s_timeout++ > DEFAULT_TIMEOUT) {
        s_timeout = DEFAULT_TIMEOUT;
    }

    if (IsTimeToExitThisDWCineyCamMode(camId + DW_CINEYCAM_FIRST_ID, src, dst, t, false)) {
        gbExitCam[camId] = true;
        return false;
    }

    Finalise_DW_CineyCams(src, dst, 0.0f, 70.0f, 5.0f, 1.0f);
    return true;
}

// 0x51D100
bool CCam::Process_DW_PlaneCam3(bool bCheckValid) {
    constexpr auto MIN_DISTANCE_TO_GROUND = 80.0f; // 0x8CCDEC
    constexpr auto MAX_DURATION_MS        = 5000;  // 0x8CCBC8
    constexpr auto UP_OFFSET              = 5.0f;  // 0x8CCDE4
    constexpr auto DEFAULT_TIMEOUT        = 100;   // 0x8CCDE0

    static int32 s_timeout = DEFAULT_TIMEOUT; // 0xB700D4, init guarded by bit 0 of 0xB700D8

    TheCamera.m_bUseNearClipScript = false;

    if (!m_pCamTargetEntity || !m_pCamTargetEntity->GetIsTypeVehicle()) {
        return false;
    }

    CEntity*   entity{};
    CVehicle*  vehicle{};
    CVector    dst, src, targetUp, targetRight, targetFwd, targetVel, targetAngVel;
    float      targetSpeed{}, targetAngSpeed{};
    CColSphere sph;
    GetCoreDataForDWCineyCamMode(entity, vehicle, dst, src, targetUp, targetRight, targetFwd, targetVel, targetSpeed, targetAngVel, targetAngSpeed, sph);

    const auto curTime = CTimer::GetTimeInMS();
    const auto camId   = MODE_DW_PLANECAM3 - MODE_DW_HELI_CHASE;

    // A guess rather than a ray cast - we really want the distance to the sector bounding box max z
    if (dst.z < MIN_DISTANCE_TO_GROUND) {
        gbExitCam[camId] = true;
        return false;
    }

    if (gDWLastModeForCineyCam != MODE_DW_PLANECAM3 || gLastFrameProcessedDWCineyCam < CTimer::GetFrameCounter() - 1) {
        gbExitCam[camId]        = false;
        gDWLastModeForCineyCam  = MODE_DW_PLANECAM3;
        gDWCineyCamStartTime    = curTime;
        gDWCineyCamEndTime      = curTime + MAX_DURATION_MS;

        CColPoint cp;
        CEntity*  hitEntity;

        CWorld::pIgnoreEntity = entity;
        const auto blocked    = CWorld::ProcessLineOfSight(dst, src, cp, hitEntity, true, true, false, false, false, false, false, false);
        CWorld::pIgnoreEntity = nullptr;

        if (blocked) {
            gbExitCam[camId] = true;
            return false;
        }
    }

    const auto t = (float)(curTime - gDWCineyCamStartTime) / (float)(gDWCineyCamEndTime - gDWCineyCamStartTime);

    const auto& bb = CModelInfo::GetModelInfo(entity->m_nModelIndex)->m_pColModel->GetBoundingBox();
    const auto  fwdOffset = (bb.m_vecMax.y - bb.m_vecMin.y) * 0.5f * 2.0f;

    src  = dst + targetFwd * fwdOffset;
    src += targetUp * UP_OFFSET;

    // Time out eventually if the plane isn't visible from here
    CColPoint cp;
    CEntity*  hitEntity;

    CWorld::pIgnoreEntity = entity;
    const auto clear      = !CWorld::ProcessLineOfSight(dst, src, cp, hitEntity, true, true, false, false, false, false, false, false);
    CWorld::pIgnoreEntity = nullptr;

    if (!clear) {
        if (s_timeout-- == 0) {
            gbExitCam[camId] = true;
            return false;
        }
    } else if (s_timeout++ > DEFAULT_TIMEOUT) {
        s_timeout = DEFAULT_TIMEOUT;
    }

    if (IsTimeToExitThisDWCineyCamMode(MODE_DW_PLANECAM3 - MODE_DW_HELI_CHASE + DW_CINEYCAM_FIRST_ID, src, dst, t, false)) {
        gbExitCam[camId] = true;
        return false;
    }

    Finalise_DW_CineyCams(src, dst, 0.0f, 70.0f, 5.0f, 1.0f);
    return true;
}

// 0x51C250
bool CCam::Process_DW_PlaneSpotterCam(bool bCheckValid) {
    constexpr auto NUM_ATTEMPTS         = 8;      // 0x8CCD94 - tries at finding ground to stand on
    constexpr auto MAX_HEIGHT_DIFF      = 100.0f; // 0x8CCD90
    constexpr auto TRANS                = 100.0f; // 0x8CCD8C - keep proportional to the height, it caps the angle
    constexpr auto LEN_DIV              = 100.0f; // 0x8CCD88 - distance the m_fFOV lerp takes place over
    constexpr auto NEAR_FOV             = 70.0f;  // 0x8CCD80
    constexpr auto FAR_FOV              = 12.0f;  // 0x8CCD84
    constexpr auto TIME_TO_ZOOM_IN      = 0.1f;   // 0x8CCD7C
    constexpr auto DEFAULT_TIMEOUT      = 100;    // 0x8CCD78
    constexpr auto MAX_DURATION_MS      = 10000;  // 0x8CCBB4

    static CVector spotterLocation{}; // 0xB700A8
    static bool    fovZoom{};         // 0xB700A4
    static float   startFOV = NEAR_FOV; // 0xB700A0
    static int32   s_timeout = DEFAULT_TIMEOUT; // 0xB7009C

    TheCamera.m_bUseNearClipScript = false;

    if (!m_pCamTargetEntity || !m_pCamTargetEntity->GetIsTypeVehicle()) {
        return false;
    }

    CEntity*   entity{};
    CVehicle*  vehicle{};
    CVector    dst, src, targetUp, targetRight, targetFwd, targetVel, targetAngVel;
    float      targetSpeed{}, targetAngSpeed{};
    CColSphere sph;
    GetCoreDataForDWCineyCamMode(entity, vehicle, dst, src, targetUp, targetRight, targetFwd, targetVel, targetSpeed, targetAngVel, targetAngSpeed, sph);

    const auto curTime = CTimer::GetTimeInMS();
    const auto camId   = MODE_DW_PLANE_SPOTTER - MODE_DW_HELI_CHASE;

    if (gDWLastModeForCineyCam != MODE_DW_PLANE_SPOTTER || gLastFrameProcessedDWCineyCam < CTimer::GetFrameCounter() - 1) {
        gbExitCam[camId]       = false;
        gDWLastModeForCineyCam = MODE_DW_PLANE_SPOTTER;
        gDWCineyCamStartTime   = curTime;
        gDWCineyCamEndTime     = curTime + MAX_DURATION_MS;

        // Look for a spot near the ground off to one side that can see up at the plane
        auto losClear = false;
        for (auto i = 0; i < NUM_ATTEMPTS; i++) {
            src    = dst;
            src.z -= MAX_HEIGHT_DIFF;
            src.x += CGeneral::GetRandomNumberInRange(TRANS / 2.0f, TRANS);
            src.y += CGeneral::GetRandomNumberInRange(TRANS / 2.0f, TRANS);

            CColPoint cp;
            CEntity*  hitEntity;

            CWorld::pIgnoreEntity = entity;
            losClear              = !CWorld::ProcessLineOfSight(dst, src, cp, hitEntity, true, true, false, false, false, false, false, false);
            CWorld::pIgnoreEntity = nullptr;

            if (!losClear) { // We found the ground - stand a plane spotter on it
                spotterLocation    = cp.m_vecPoint;
                spotterLocation.z += 2.0f;
                break;
            }
        }

        if (losClear) { // Never found any land
            gbExitCam[camId] = true;
            return false;
        }

        fovZoom = GetRandomTrueFalse();
    }

    const auto t = (float)(curTime - gDWCineyCamStartTime) / (float)(gDWCineyCamEndTime - gDWCineyCamStartTime);

    src = spotterLocation;

    // The plane is right above us - bail before the camera flips 180 degrees
    if ((src - dst).Magnitude2D() < 5.0f) {
        gbExitCam[camId] = true;
        return false;
    }

    auto newFOV = 70.0f;
    if (fovZoom) { // Zoom in to the m_fFOV we want rather than snapping to it
        const auto len = (dst - src).Magnitude();
        newFOV = SineAccelDecelLerp(std::clamp(len / LEN_DIV, 0.0f, 1.0f), NEAR_FOV, FAR_FOV);

        if (t < TIME_TO_ZOOM_IN) {
            newFOV = SineAccelDecelLerp(std::clamp(t / TIME_TO_ZOOM_IN, 0.0f, 1.0f), startFOV, newFOV);
        }
    }

    // Time out eventually if the plane isn't visible from here
    CColPoint cp;
    CEntity*  hitEntity;

    CWorld::pIgnoreEntity = entity;
    const auto clear      = !CWorld::ProcessLineOfSight(dst, src, cp, hitEntity, true, true, false, false, false, false, false, false);
    CWorld::pIgnoreEntity = nullptr;

    if (!clear) {
        if (s_timeout-- == 0) {
            gbExitCam[camId] = true;
            return false;
        }
    } else if (s_timeout++ > DEFAULT_TIMEOUT) {
        s_timeout = DEFAULT_TIMEOUT;
    }

    if (IsTimeToExitThisDWCineyCamMode(camId + DW_CINEYCAM_FIRST_ID, src, dst, t, false)) {
        gbExitCam[camId] = true;
        return false;
    }

    Finalise_DW_CineyCams(src, dst, 0.0f, newFOV, std::lerp(10.0f, 0.3f, newFOV / 70.0f), 1.0f);
    return true;
}

// 0x50F3F0 - debug
void CCam::Process_Editor(const CVector& target, float orientation, float speedVar, float speedVarWanted) {
    static auto& s_LookAtAngle     = StaticRef<float>(0xB6FFE4);
    static auto& s_DoRenderShadows = StaticRef<bool>(0xB7295A);

    if (m_bResetStatics) {
        m_vecSource.Set(796.0f, -937.0f, 40.0f);
        CEntity::SafeCleanUpRef(m_pCamTargetEntity);
        m_pCamTargetEntity = nullptr;
        m_bResetStatics    = false;
    }
    RwCameraSetNearClipPlane(Scene.m_pRwCamera, 0.3f);
    m_fFOV = 70.0f;

    static constexpr float _90DEG_PER_HOUR_IN_RAD_PER_MIN = 0.02617994f;
    const auto* pad = CPad::GetPad(1);
    m_fHorizontalAngle += pad->GetLeftStickX() * _90DEG_PER_HOUR_IN_RAD_PER_MIN / 19.0f;
    m_fVerticalAngle   += DegreesToRadians(static_cast<float>(pad->GetLeftStickY())) / 50.0f;

    m_fVerticalAngle = std::max(m_fVerticalAngle, DegreesToRadians(85.0f));
    if (m_fVerticalAngle >= DegreesToRadians(-85.0f)) {
        if (pad->IsSquareDown()) {
            s_LookAtAngle += 0.1f;
        } else if (pad->IsCrossDown()) {
            s_LookAtAngle -= 0.1f;
        } else {
            s_LookAtAngle = 0.0f;
        }
    } else {
        m_fVerticalAngle = DegreesToRadians(-85.0f);
    }
    s_LookAtAngle = std::clamp(s_LookAtAngle, -70.0f, 70.0f);

    m_vecFront = (m_pCamTargetEntity ? m_pCamTargetEntity->GetPosition() : m_vecSource - m_vecSource).Normalized();
    m_vecSource += s_LookAtAngle * m_vecFront;
    m_vecSource.z = std::min(m_vecSource.z, -450.0f);

    if (pad->IsRightShoulder2Pressed()) {
        FindPlayerEntity()->Teleport(m_vecSource, false);
    }

    const auto ClampByLoop = [](float& value, float min, float max) {
        while (value > max) {
            value -= 1.0f;
        }
        while (value < min) {
            value += 1.0f;
        }
    };
    ClampByLoop(m_vecSource.x, 5.0f, 115.0f);
    ClampByLoop(m_vecSource.y, 5.0f, 115.0f);

    GetVectorsReadyForRW();

    if (!pad->IsLeftShockPressed() && s_DoRenderShadows) {
        CShadows::StoreShadowToBeRendered(
            eShadowType::SHADOW_ADDITIVE,
            gpShadowExplosionTex,
            m_vecSource,
            {12.0f, 0.0f},
            {0.0f, -12.0f},
            128,
            128,
            128,
            128,
            1000.0f,
            false,
            1.0f,
            nullptr,
            false
        );
    }

    if (CHud::m_Wants_To_Draw_Hud) {
        NOTSA_LOG_DEBUG("CamX: {:0.3f} CamY: {:0.3f}  CamZ:  {:0.3f}", m_vecSource.x, m_vecSource.y, m_vecSource.z);
        NOTSA_LOG_DEBUG("Frontx: {:0.3f}, Fronty: {:0.3f}, Frontz: {:0.3f} ", m_vecFront.x, m_vecFront.y, m_vecFront.z);
        NOTSA_LOG_DEBUG("LookAT: {:0.3f}, LookAT: {:0.3f}, LookAT: {:0.3f} ", m_vecSource.x + m_vecFront.x, m_vecSource.y + m_vecFront.y, m_vecSource.z + m_vecFront.z);
    }
}

// 0x51D470
void CCam::Process_Fixed(const CVector& target, float orientation, float speedVar, float speedVarWanted) {
    if (m_nDirectionWasLooking != 3) {
        m_nDirectionWasLooking = 3; // todo: enum
    }

    m_vecSource = m_vecCamFixedModeSource;
    m_vecFront = (target - m_vecSource).Normalized();
    m_vecTargetCoorsForFudgeInter = target;
    GetVectorsReadyForRW();

    // inlined?
    const auto a = CrossProduct(
        m_vecFront,
        (m_vecCamFixedModeUpOffSet + CVector{ 0.0f, 0.0f, 1.0f }).Normalized()
    ).Normalized();
    m_vecUp = CrossProduct(
        a,
        m_vecFront
    );
    m_fFOV = 70.0f;

    if (float wl{}; CWaterLevel::GetWaterLevel(m_vecSource, wl, true) && m_vecSource.z < wl) {
        ApplyUnderwaterMotionBlur();
    }

    if (gAllowScriptedFixedCameraCollision) {
        const auto savedIgnoreEntity = CWorld::pIgnoreEntity;

        CWorld::pIgnoreEntity = FindPlayerVehicle();
        CVector out{};
        float   outDist{1.0f};
        if (TheCamera.ConeCastCollisionResolve(m_vecSource, target, out, 2.0f, 0.1f, outDist)) {
            m_vecSource.y = out.y;
            m_vecSource.z = out.z;
        }

        CWorld::pIgnoreEntity = savedIgnoreEntity;
    }
}

// 0x5B25F0
void CCam::Process_FlyBy(const CVector& target, float orientation, float speedVar, float speedVarWanted) {
    // One marker per spline, remembered between frames so the search picks up where it left off
    static auto& s_srcMarker   = StaticRef<int32>(0xBC4080);
    static auto& s_frontMarker = StaticRef<int32>(0xBC407C);
    static auto& s_rollMarker  = StaticRef<int32>(0xBC4088);
    static auto& s_fovMarker   = StaticRef<int32>(0xBC4084);
    static auto& s_fov         = StaticRef<float>(0xBC4074);
    static auto& s_fovStart    = StaticRef<float>(0xBC4078);

    if (TheCamera.m_bCutsceneFinished) {
        return;
    }

    auto* const srcPath   = TheCamera.m_aPathArray[2].m_pArrPathData;
    auto* const frontPath = TheCamera.m_aPathArray[3].m_pArrPathData;
    auto* const rollPath  = TheCamera.m_aPathArray[1].m_pArrPathData;
    auto* const fovPath   = TheCamera.m_aPathArray[0].m_pArrPathData;

    m_vecUp = CVector{ 0.0f, 0.0f, 1.0f };

    if (TheCamera.m_bStartingSpline) {
        m_fTimeElapsedFloat += CTimer::GetTimeStepNonClipped() * 0.02f * 1000.0f;
    } else {
        m_fTimeElapsedFloat = 0.0f;
        m_nFinishTime       = (uint32)(srcPath[10 * (int32)srcPath[0] - 9] * 1000.0f);

        TheCamera.m_bStartingSpline = true;
        s_srcMarker   = 11;
        s_frontMarker = 11;
        s_rollMarker  = 5;
        s_fovMarker   = 5;
        s_fovStart    = fovPath[2];
        s_fov         = s_fovStart;
    }

    //! Walk a marker forward until it covers the elapsed time
    const auto AdvanceMarker = [this](const float* path, int32& marker, int32 stride) {
        for (auto m = marker; (path[m] - path[1]) * 1000.0f <= m_fTimeElapsedFloat; m += stride) {
            marker = m;
        }
    };

    //! Roll is stored in degrees along the spline; the up vector is built from it directly
    const auto ApplyRoll = [this](float rollDegrees) {
        const auto a = DegreesToRadians(rollDegrees) + HALF_PI;
        m_vecUp.x = std::cos(a);
        m_vecUp.z = std::sin(a);
    };

    auto roll = 0.0f;

    if (m_fTimeElapsedFloat >= (float)m_nFinishTime) { // Past the end - clamp to the last knot
        s_srcMarker   = 10 * ((int32)srcPath[0] - 1) + 1;
        s_frontMarker = 10 * ((int32)frontPath[0] - 1) + 1;
        s_rollMarker  = 4 * (int32)rollPath[0] - 3;
        s_fovMarker   = 4 * (int32)fovPath[0] - 3;

        FindSplinePathPositionVector(&m_vecSource.x, srcPath, m_fTimeElapsedFloat, &s_srcMarker);
        FindSplinePathPositionVector(&m_vecFront.x, frontPath, m_fTimeElapsedFloat, &s_frontMarker);
        FindSplinePathPositionFloat(&roll, rollPath, m_fTimeElapsedFloat, &s_rollMarker);
        ApplyRoll(roll);
        FindSplinePathPositionFloat(&s_fov, fovPath, m_fTimeElapsedFloat, &s_fovMarker);

        TheCamera.m_fPositionAlongSpline = 1.0f;
        s_srcMarker = s_frontMarker = s_rollMarker = s_fovMarker = 0;
    } else {
        TheCamera.m_fPositionAlongSpline = m_fTimeElapsedFloat / (float)m_nFinishTime;

        AdvanceMarker(srcPath, s_srcMarker, 10);
        FindSplinePathPositionVector(&m_vecSource.x, srcPath, m_fTimeElapsedFloat, &s_srcMarker);

        AdvanceMarker(frontPath, s_frontMarker, 10);
        FindSplinePathPositionVector(&m_vecFront.x, frontPath, m_fTimeElapsedFloat, &s_frontMarker);

        AdvanceMarker(rollPath, s_rollMarker, 4);
        FindSplinePathPositionFloat(&roll, rollPath, m_fTimeElapsedFloat, &s_rollMarker);
        ApplyRoll(roll);

        AdvanceMarker(fovPath, s_fovMarker, 4);
        FindSplinePathPositionFloat(&s_fov, fovPath, m_fTimeElapsedFloat, &s_fovMarker);
    }

    // The front spline holds the look-at point, not a direction
    m_vecTargetCoorsForFudgeInter = m_vecFront;
    m_vecFront -= m_vecSource;
    m_vecFront.Normalise();

    m_vecUp = CrossProduct(m_vecFront, CrossProduct(m_vecUp, m_vecFront));
    m_vecUp.Normalise();

    m_fFOV = s_fov;
}

// 0x5245B0
void CCam::Process_FollowCar_SA(const CVector& ThisCamsTarget, float TargetOrientation, float SpeedVar, float SpeedVarDesired, bool bScriptSetAngles) {
    enum { ZOOM_ONE = 1, ZOOM_TWO = 2, ZOOM_THREE = 3 };

    constexpr auto AIMWEAPON_STICK_SENS      = 0.007f;
    constexpr auto CAR_FOV_START_SPEED       = 0.9f;
    constexpr auto CAR_FOV_FADE_MULT         = 0.98f;
    constexpr auto fTestShiftHeliCamTarget   = 0.5f;
    constexpr auto BLEND_EXTRA_POS_RATE      = 0.02f;
    constexpr auto TRAILER_CAMDIST_MULT      = 0.5f;
    constexpr auto BIKE_WITH_PASSENGER_HEIGHT_ADD = 0.4f;
    constexpr auto TEST_CAM_ALPHA_RAISE_MULT = 0.3f;
    constexpr auto STICK_DOWN_LENGTH_ADD     = 1.5f;
    constexpr auto STICK_DOWN_WIDTH_ADD      = 1.2f;
    constexpr auto STICK_DOWN_DIST_LIMIT_MULT = 1.2f;
    constexpr auto CREST_HILL_STICK_MULT     = 0.075f;
    constexpr auto SPEED_TOL                 = 0.0001f;
    constexpr auto MOUSE_INPUT_BUFFER_TIME   = 1.0f;
    constexpr auto FRAMES_PER_SECOND         = 50.0f;
    constexpr auto PROSTITUTE_CAM_ALPHA_ANGLE = 0.1f;
    constexpr auto PROSTITUTE_CAM_ALPHA_RATE  = 0.0035f;
    constexpr auto gStickAlphaFix            = 0.05f;

    if (!m_pCamTargetEntity->GetIsTypeVehicle()) {
        return;
    }
    auto* const pVehicle = m_pCamTargetEntity->AsVehicle();
    auto        vecTargetCoords = ThisCamsTarget;
    auto* const pPad = CPad::GetPad(pVehicle->m_pDriver && pVehicle->m_pDriver->m_nPedType == PED_TYPE_PLAYER2 ? 1 : 0);

    TheCamera.ApplyVehicleCameraTweaks(pVehicle);

    const auto model = pVehicle->GetModelId();
    auto nType = FOLLOW_CAR_INCAR;
    if (model == MODEL_RCBANDIT || model == MODEL_RCBARON || model == MODEL_RCTIGER || model == MODEL_RCCAM) {
        nType = FOLLOW_CAR_RCCAR;
    } else if (model == MODEL_RCRAIDER || model == MODEL_RCGOBLIN) {
        nType = FOLLOW_CAR_RCHELI;
    } else if (pVehicle->IsBike() || pVehicle->IsSubQuad()) {
        nType = FOLLOW_CAR_ONBIKE;
    } else if (pVehicle->IsSubHeli()) {
        nType = FOLLOW_CAR_INHELI;
    } else if (pVehicle->IsSubPlane()) {
        if (model == MODEL_HYDRA && pVehicle->AsAutomobile()->m_wMiscComponentAngle >= CPlane::HARRIER_NOZZLE_SWITCH_LIMIT) {
            nType = FOLLOW_CAR_INHELI;
        } else {
            nType = model == MODEL_VORTEX ? FOLLOW_CAR_INCAR : FOLLOW_CAR_INPLANE;
        }
    } else if (pVehicle->IsSubBoat()) {
        nType = FOLLOW_CAR_INBOAT;
    }
    const auto& set = CARCAM_SET[nType];

    auto fCamDistance = TheCamera.m_fCarZoomSmoothed + set.fBaseCamDist;

    int32 PositionInArray = 0;
    TheCamera.GetArrPosForVehicleType(static_cast<eVehicleType>(pVehicle->GetVehicleAppearance()), PositionInArray);

    auto fCamAlpha = 0.0f;
    if (pVehicle->GetStatus() == STATUS_REMOTE_CONTROLLED) { // Hack, but there's no other way right now
        fCamAlpha += ZmTwoAlphaOffset[PositionInArray];
    } else switch (TheCamera.m_nCarZoom) {
    case ZOOM_ONE:   fCamAlpha += ZmOneAlphaOffset[PositionInArray];   break;
    case ZOOM_TWO:   fCamAlpha += ZmTwoAlphaOffset[PositionInArray];   break;
    case ZOOM_THREE: fCamAlpha += ZmThreeAlphaOffset[PositionInArray]; break;
    }

    auto* const col = pVehicle->GetColModel();
    auto CarHeight  = col->m_boundBox.m_vecMax.z;
    auto CarLength  = 2.0f * std::abs(col->m_boundBox.m_vecMin.y);

    // Add the trailer's length when towing
    if (auto* const towed = pVehicle->m_pVehicleBeingTowed) {
        if (sBlendExtraPos < 1.0f) {
            sBlendExtraPos = std::min(1.0f, sBlendExtraPos + BLEND_EXTRA_POS_RATE * CTimer::GetTimeStep());
        }
        auto* const towedCol = towed->GetColModel();
        CarLength += sBlendExtraPos * TRAILER_CAMDIST_MULT * (towedCol->m_boundBox.m_vecMax - towedCol->m_boundBox.m_vecMin).Magnitude();
        CarHeight += sBlendExtraPos * (std::max(CarHeight, towedCol->m_boundBox.m_vecMax.z) - CarHeight);

        vecTargetCoords = (1.0f - 0.5f * sBlendExtraPos) * vecTargetCoords + 0.5f * sBlendExtraPos * towed->GetPosition();
    } else if (pVehicle->IsSubBike() || pVehicle->IsSubQuad()) {
        if (pVehicle->m_apPassengers[0]) {
            if (sBlendExtraPos < 1.0f) {
                sBlendExtraPos = std::min(1.0f, sBlendExtraPos + BLEND_EXTRA_POS_RATE * CTimer::GetTimeStep());
            }
        } else if (sBlendExtraPos > 0.0f) {
            sBlendExtraPos = std::max(0.0f, sBlendExtraPos - BLEND_EXTRA_POS_RATE * CTimer::GetTimeStep());
        }
        CarHeight += sBlendExtraPos * BIKE_WITH_PASSENGER_HEIGHT_ADD;
    } else {
        sBlendExtraPos = 0.0f;
    }

    CarLength *= TheCamera.m_fCurrentTweakDistance; // Per-vehicle tweak, see `ApplyVehicleCameraTweaks`

    fCamDistance += CarLength;
    const auto fMinDistance = set.fMinDist * CarLength;

    // Full-size helis (not R/C ones) use a different offset
    if (pVehicle->GetVehicleAppearance() == VEHICLE_APPEARANCE_HELI && pVehicle->GetStatus() != STATUS_REMOTE_CONTROLLED) {
        vecTargetCoords += pVehicle->GetMatrix().GetUp() * fTestShiftHeliCamTarget * CarHeight;
    } else if (const auto fTargetZMod = CarHeight * set.fTargetOffsetZ - set.fBaseCamZ; fTargetZMod > 0.0f) {
        vecTargetCoords.z += fTargetZMod;
        fCamDistance      += fTargetZMod;
        fCamAlpha         += TEST_CAM_ALPHA_RAISE_MULT * fTargetZMod / fCamDistance;
    }

    vecTargetCoords.z *= TheCamera.m_fCurrentTweakAltitude;
    fCamAlpha         += TheCamera.m_fCurrentTweakAngle;

    auto fTempMinFollowDist = set.fMinFollowDist;
    if (TheCamera.m_nCarZoom == ZOOM_ONE && (nType == FOLLOW_CAR_INCAR || nType == FOLLOW_CAR_ONBIKE)) {
        fTempMinFollowDist *= 0.65f;
    }
    const auto fFollowDist = std::max(fCamDistance, fTempMinFollowDist);

    // Damn globals passed between functions!
    m_fCaMaxDistance = fCamDistance;
    m_fCaMinDistance = 3.5f;

    if (m_bResetStatics) {
        m_fFOV = 70.0f;
    } else {
        const auto fwdSpeed = DotProduct(pVehicle->m_vecMoveSpeed, pVehicle->GetMatrix().GetForward());
        if ((pVehicle->IsSubAutomobile() || pVehicle->IsSubBike()) && fwdSpeed > CAR_FOV_START_SPEED) {
            m_fFOV += (fwdSpeed - CAR_FOV_START_SPEED) * CTimer::GetTimeStep();
        }
        if (m_fFOV > 70.0f) {
            m_fFOV = 70.0f + (m_fFOV - 70.0f) * std::pow(CAR_FOV_FADE_MULT, CTimer::GetTimeStep());
        }
        m_fFOV = std::clamp(m_fFOV, 70.0f, 100.0f);
    }

    if (m_bResetStatics || TheCamera.m_bCamDirectlyBehind || TheCamera.m_bCamDirectlyInFront) {
        m_bResetStatics             = false;
        m_bRotating                 = false;
        m_bCollisionChecksOn        = true;
        TheCamera.m_bResetOldMatrix = true;

        if (!TheCamera.m_bJustCameOutOfGarage && !bScriptSetAngles) {
            m_fVerticalAngle   = 0.0f;
            m_fHorizontalAngle = pVehicle->GetHeading() - HALF_PI;
            if (TheCamera.m_bCamDirectlyInFront) {
                m_fHorizontalAngle += PI;
            }
        }
        m_fBetaSpeed  = 0.0f;
        m_fAlphaSpeed = 0.0f;
        m_fDistance   = 1000.0f;

        m_vecFront = CVector{
            -std::cos(m_fHorizontalAngle) * std::cos(m_fVerticalAngle),
            -std::sin(m_fHorizontalAngle) * std::cos(m_fVerticalAngle),
            std::sin(m_fVerticalAngle)
        };

        m_avecTargetHistoryPos[0] = vecTargetCoords - fFollowDist * m_vecFront;
        m_anTargetHistoryTime[0]  = CTimer::GetTimeInMS();
        m_avecTargetHistoryPos[1] = vecTargetCoords - fCamDistance * m_vecFront;
        m_nCurrentHistoryPoints   = 0;

        if (!TheCamera.m_bJustCameOutOfGarage && !bScriptSetAngles) {
            m_fVerticalAngle = -fCamAlpha;
        }
    }

    m_vecFront = (vecTargetCoords - m_avecTargetHistoryPos[0]).Normalized();
    const auto fTempLength = (vecTargetCoords - m_avecTargetHistoryPos[1]).Magnitude();

    auto fTargetBeta = std::atan2(-m_vecFront.x, m_vecFront.y) - HALF_PI;
    if (fTargetBeta < -PI) {
        fTargetBeta += TWO_PI;
    }

    auto fHeadingBeta = pVehicle->m_vecMoveSpeed.Magnitude2D() > 0.02f
        ? std::atan2(-pVehicle->m_vecMoveSpeed.x, pVehicle->m_vecMoveSpeed.y) - HALF_PI
        : fTargetBeta;

    if (fHeadingBeta > fTargetBeta + PI) {
        fHeadingBeta -= TWO_PI;
    } else if (fHeadingBeta < fTargetBeta - PI) {
        fHeadingBeta += TWO_PI;
    }

    // Swing the camera based on the vehicle's movement speed
    auto fDiffMult = set.fDiffBetaSwing * CTimer::GetTimeStep();
    auto fDiffCap  = set.fDiffBetaSwingCap * CTimer::GetTimeStep();
    {
        const auto alongFront = DotProduct(m_vecFront, pVehicle->m_vecMoveSpeed);
        const auto fAltSpeed  = (pVehicle->m_vecMoveSpeed - alongFront * m_vecFront).Magnitude();
        fDiffMult = std::min(1.0f, fAltSpeed * fDiffMult);
    }
    fTargetBeta += std::clamp(fDiffMult * (fHeadingBeta - fTargetBeta), -fDiffCap, fDiffCap);

    if (fTargetBeta > m_fHorizontalAngle + PI) {
        fTargetBeta -= TWO_PI;
    } else if (fTargetBeta < m_fHorizontalAngle - PI) {
        fTargetBeta += TWO_PI;
    }
    auto fCamControlBetaSpeed = (fTargetBeta - m_fHorizontalAngle) / std::max(1.0f, CTimer::GetTimeStep());

    auto fTargetAlpha = std::asin(std::clamp(m_vecFront.z, -1.0f, 1.0f));

    if (fTempLength < fCamDistance && fCamDistance > fMinDistance) {
        fCamDistance = std::max(fMinDistance, fTempLength);
    }

    // Stop the player pushing the camera down into the vehicle they're driving
    auto fAlphaUpLimit = set.fUpLimit;
    if (pVehicle->m_vecMoveSpeed.SquaredMagnitude() < sq(0.2f)
        && !(pVehicle->IsBike() && pVehicle->AsBike()->GetNumContactWheels() < 4)
        && !pVehicle->IsSubHeli()
        && !(pVehicle->IsSubPlane() && pVehicle->AsAutomobile()->GetNumContactWheels() == 0)
    ) {
        const auto tempRight = CrossProduct(pVehicle->GetMatrix().GetForward(), CVector{ 0.0f, 0.0f, 1.0f }).Normalized();
        const auto tempUp    = CrossProduct(tempRight, pVehicle->GetMatrix().GetForward()).Normalized();

        if (DotProduct(m_vecFront, tempUp) > 0.0f) {
            const auto& bb = pVehicle->GetColModel()->m_boundBox;

            auto fVehicleHeight = pVehicle->GetHeightAboveRoad();
            fVehicleHeight += vecTargetCoords.z - pVehicle->GetPosition().z;

            const auto fCornerAngle = std::atan2(bb.m_vecMax.x, -bb.m_vecMin.y);

            // Difference between the camera's and the vehicle's heading
            auto fBetaDiff = std::abs(std::sin(m_fHorizontalAngle - (pVehicle->GetHeading() - HALF_PI)));
            fBetaDiff = std::asin(fBetaDiff);

            auto fVehicleDist = fBetaDiff > fCornerAngle
                ? (bb.m_vecMax.x + STICK_DOWN_WIDTH_ADD) / std::cos(std::max(0.0f, HALF_PI - fBetaDiff))
                : (-bb.m_vecMin.y + STICK_DOWN_LENGTH_ADD) / std::cos(fBetaDiff);
            fVehicleDist *= STICK_DOWN_DIST_LIMIT_MULT;

            fAlphaUpLimit = std::atan2(fVehicleHeight, fVehicleDist);

            // Add the car's pitch
            const auto fwd = pVehicle->GetMatrix().GetForward();
            fAlphaUpLimit += std::cos(m_fHorizontalAngle - (pVehicle->GetHeading() - HALF_PI)) * std::atan2(fwd.z, fwd.Magnitude2D());

            // ...and its roll, if the wheels are on the ground
            if (pVehicle->IsAutomobile() && pVehicle->AsAutomobile()->GetNumContactWheels() > 1
                && std::abs(DotProduct(pVehicle->m_vecTurnSpeed, fwd)) < 0.05f
            ) {
                const auto right = pVehicle->GetMatrix().GetRight();
                fAlphaUpLimit += std::cos(m_fHorizontalAngle - (pVehicle->GetHeading() - HALF_PI) + HALF_PI) * std::atan2(right.z, right.Magnitude2D());
            }
        }
    }

    fTargetAlpha -= fCamAlpha;
    if (fTargetAlpha > fAlphaUpLimit) {
        fTargetAlpha = fAlphaUpLimit;
    } else if (fTargetAlpha < -set.fDownLimit) {
        fTargetAlpha = -set.fDownLimit;
    }

    fDiffMult = std::pow(set.fDiffAlphaRate, CTimer::GetTimeStep());
    fDiffCap  = set.fDiffAlphaCap * CTimer::GetTimeStep();
    const auto fTargetDiff = std::clamp((1.0f - fDiffMult) * (fTargetAlpha - m_fVerticalAngle), -fDiffCap, fDiffCap);

    auto StickBetaOffset  = -(float)pPad->AimWeaponLeftRight(nullptr);
    auto StickAlphaOffset = (float)pPad->AimWeaponUpDown(nullptr);

    if (TheCamera.m_bUseMouse3rdPerson) {
        StickAlphaOffset = 0.0f;
    }
    StickBetaOffset  = sq(AIMWEAPON_STICK_SENS) * std::abs(StickBetaOffset) * StickBetaOffset * (0.25f / 3.5f * (m_fFOV / 80.0f));
    StickAlphaOffset = sq(AIMWEAPON_STICK_SENS) * std::abs(StickAlphaOffset) * StickAlphaOffset * (0.15f / 3.5f * (m_fFOV / 80.0f));

    // Vehicles that use the right stick for something else
    auto bFixAlphaAngle = true;
    if (notsa::contains({ MODEL_PACKER, MODEL_DOZER, MODEL_DUMPER, MODEL_CEMENT, MODEL_ANDROM,
                          MODEL_HYDRA, MODEL_TOWTRUCK, MODEL_FORKLIFT, MODEL_TRACTOR }, model)) {
        StickAlphaOffset = 0.0f;
    } else if (model == MODEL_RCTIGER || (pVehicle->IsAutomobile() && pVehicle->handlingFlags.bHydraulicInst)) {
        StickAlphaOffset = 0.0f;
        StickBetaOffset  = 0.0f;
    } else {
        bFixAlphaAngle = false;
    }

    // Stops weirdness on the radar
    if (gCameraDirection != LOOKING_FORWARD) {
        StickAlphaOffset = 0.0f;
        StickBetaOffset  = 0.0f;
    }

    if (nType == FOLLOW_CAR_INCAR && std::abs((float)pPad->GetSteeringUpDown()) > 120.0f
        && pVehicle->m_pDriver && pVehicle->m_pDriver->GetIntelligence()->GetTaskManager().GetActiveTask()
        && pVehicle->m_pDriver->GetIntelligence()->GetTaskManager().GetActiveTask()->GetTaskType() != TASK_COMPLEX_LEAVE_CAR
    ) {
        const auto extra = (float)pPad->GetSteeringUpDown();
        StickAlphaOffset += 0.5f * sq(AIMWEAPON_STICK_SENS) * std::abs(extra) * extra * (0.15f / 3.5f * (m_fFOV / 80.0f));
    }

    // Slow down how fast the camera swings downwards
    if (StickAlphaOffset > 0.0f) {
        StickAlphaOffset *= 0.5f;
    }

    auto bUsingMouse = false;
    if (TheCamera.m_bUseMouse3rdPerson && !pPad->DisablePlayerControls) {
        const auto mouse   = CPad::NewMouseControllerState.GetAmountMouseMoved();
        const auto fStickY = mouse.y * 2.0f;
        const auto fStickX = -mouse.x * 2.0f;

        const auto mouseSteeringOn = pVehicle->IsSubPlane() || pVehicle->IsSubHeli()
            ? CVehicle::m_bEnableMouseFlying
            : CVehicle::m_bEnableMouseSteering;

        if ((fStickX != 0.0f || fStickY != 0.0f) && (!mouseSteeringOn || pPad->GetVehicleMouseLook())) {
            StickAlphaOffset = TheCamera.m_fMouseAccelHorzntl * fStickY * (m_fFOV / 80.0f);
            StickBetaOffset  = TheCamera.m_fMouseAccelHorzntl * fStickX * (m_fFOV / 80.0f);
            m_fAlphaSpeed = m_fBetaSpeed = 0.0f;
            fTargetAlpha  = m_fVerticalAngle;

            // How long after mouse input before the camera returns to normal motion
            MOUSE_INPUT_COUNTER = FRAMES_PER_SECOND * MOUSE_INPUT_BUFFER_TIME;
            bUsingMouse = true;
        } else if (MOUSE_INPUT_COUNTER > 0.0f) {
            StickAlphaOffset = 0.0f;
            StickBetaOffset  = 0.0f;
            m_fAlphaSpeed = m_fBetaSpeed = 0.0f;
            fTargetAlpha  = m_fVerticalAngle;

            MOUSE_INPUT_COUNTER = std::max(0.0f, MOUSE_INPUT_COUNTER - CTimer::GetTimeStep());
            bUsingMouse = true;
        }
    }

    if (auto* const passenger = pVehicle->m_apPassengers[0]) {
        auto* const task = passenger->GetIntelligence()->GetTaskManager().GetActiveTask();
        if (task && task->GetTaskType() == TASK_COMPLEX_PROSTITUTE_SOLICIT
            && static_cast<CTaskComplexProstituteSolicit*>(task)->bMoveCameraDown
        ) {
            StickAlphaOffset = m_fVerticalAngle < fAlphaUpLimit - PROSTITUTE_CAM_ALPHA_ANGLE
                ? PROSTITUTE_CAM_ALPHA_RATE * CTimer::GetTimeStep()
                : 0.0f;
        }
    }

    if (bFixAlphaAngle) {
        // Ease to `-fCamAlpha` once, on first getting into the vehicle
        if (gCameraMode != MODE_CAM_ON_A_STRING) {
            gAcquiredAlpha = false;
        }

        if (!gAcquiredAlpha && std::abs(m_fVerticalAngle + fCamAlpha) > 0.05f) {
            StickAlphaOffset = (-fCamAlpha - m_fVerticalAngle) * gStickAlphaFix;
        } else {
            gAcquiredAlpha = true;
        }
    }

    StickAlphaOffset *= set.fStickMult;
    StickBetaOffset  *= set.fStickMult;

    fDiffMult = std::pow(set.fDiffBetaRate, CTimer::GetTimeStep());
    fDiffCap  = set.fDiffBetaCap;

    fCamControlBetaSpeed = std::clamp(fCamControlBetaSpeed + StickBetaOffset, -fDiffCap, fDiffCap);
    m_fBetaSpeed = fDiffMult * m_fBetaSpeed + (1.0f - fDiffMult) * fCamControlBetaSpeed;
    if (std::abs(m_fBetaSpeed) < SPEED_TOL) {
        m_fBetaSpeed = 0.0f;
    }

    if (bUsingMouse) {
        m_fHorizontalAngle += StickBetaOffset;
    } else {
        m_fHorizontalAngle += m_fBetaSpeed * CTimer::GetTimeStep();
    }

    if (TheCamera.m_bJustCameOutOfGarage) {
        m_fHorizontalAngle = CGeneral::GetATanOfXY(m_vecFront.x, m_vecFront.y) + PI;
    }

    ClipBeta();

    // Help the camera swing up as the vehicle crests a hill
    if (nType <= FOLLOW_CAR_ONBIKE && fTargetAlpha < m_fVerticalAngle && fTempLength >= fCamDistance) {
        auto nWheelsOnGround = 0;
        if (pVehicle->IsAutomobile()) {
            nWheelsOnGround = pVehicle->AsAutomobile()->GetNumContactWheels();
        } else if (pVehicle->IsBike()) {
            nWheelsOnGround = pVehicle->AsBike()->GetNumContactWheels();
        }
        if (nWheelsOnGround > 1) {
            StickAlphaOffset += (fTargetAlpha - m_fVerticalAngle) * CREST_HILL_STICK_MULT;
        }
    }

    m_fAlphaSpeed = fDiffMult * m_fAlphaSpeed + (1.0f - fDiffMult) * StickAlphaOffset;
    if (StickAlphaOffset > 0.0f) { // Slow how fast the camera swings down off the stick
        fDiffCap *= 0.5f;
    }
    m_fAlphaSpeed = std::clamp(m_fAlphaSpeed, -fDiffCap, fDiffCap);
    if (std::abs(m_fAlphaSpeed) < SPEED_TOL) {
        m_fAlphaSpeed = 0.0f;
    }

    if (bUsingMouse) {
        fTargetAlpha     += StickAlphaOffset;
        m_fVerticalAngle += StickAlphaOffset;
    } else {
        fTargetAlpha     += m_fAlphaSpeed * CTimer::GetTimeStep();
        m_fVerticalAngle += fTargetDiff;
    }

    if (m_fVerticalAngle > fAlphaUpLimit) {
        m_fVerticalAngle = fAlphaUpLimit;
        m_fAlphaSpeed    = 0.0f;
    } else if (m_fVerticalAngle < -set.fDownLimit) {
        m_fVerticalAngle = -set.fDownLimit;
        m_fAlphaSpeed    = 0.0f;
    }

    // Deadband so tiny jitter doesn't move the camera at all
    if (std::abs(gOldAlpha - m_fVerticalAngle) < SPEED_TOL) {
        m_fVerticalAngle = gOldAlpha;
    }
    gOldAlpha = m_fVerticalAngle;
    if (std::abs(gOldBeta - m_fHorizontalAngle) < SPEED_TOL) {
        m_fHorizontalAngle = gOldBeta;
    }
    gOldBeta = m_fHorizontalAngle;

    m_vecFront = CVector{
        -std::cos(m_fHorizontalAngle) * std::cos(m_fVerticalAngle),
        -std::sin(m_fHorizontalAngle) * std::cos(m_fVerticalAngle),
        std::sin(m_fVerticalAngle)
    };

    VecTrunc(m_vecSource);
    GetVectorsReadyForRW();

    TheCamera.m_bCamDirectlyBehind  = false;
    TheCamera.m_bCamDirectlyInFront = false;

    m_vecSource = vecTargetCoords - m_vecFront * fCamDistance;
    m_vecTargetCoorsForFudgeInter = vecTargetCoords;

    fTargetAlpha += fCamAlpha;
    const auto historyFront = CVector{
        -std::cos(m_fHorizontalAngle) * std::cos(fTargetAlpha),
        -std::sin(m_fHorizontalAngle) * std::cos(fTargetAlpha),
        std::sin(fTargetAlpha)
    };
    m_avecTargetHistoryPos[2] = m_avecTargetHistoryPos[0];
    m_avecTargetHistoryPos[0] = vecTargetCoords - fFollowDist * historyFront;
    m_avecTargetHistoryPos[1] = vecTargetCoords - fCamDistance * historyFront;

    CCamera::SetColVarsVehicle((eVehicleType)nType, TheCamera.m_nCarZoom);
    if (gCameraDirection == LOOKING_FORWARD) {
        CWorld::pIgnoreEntity = pVehicle;

        TheCamera.CameraGenericModeSpecialCases(nullptr);
        TheCamera.CameraVehicleModeSpecialCases(pVehicle);

        if (pVehicle->vehicleFlags.bIsBig) { // Never restored, same as the original
            gTopSphereCastTest = true;
        }

        TheCamera.CameraColDetAndReact(&m_vecSource, &vecTargetCoords);
        // Collision may have set the clip distance; override it to improve Z-fighting in some cases
        TheCamera.ImproveNearClip(pVehicle, nullptr, &m_vecSource, &vecTargetCoords);

        CWorld::pIgnoreEntity = nullptr;

        VecTrunc(m_vecSource);
    }

    TheCamera.m_bCamDirectlyBehind  = false;
    TheCamera.m_bCamDirectlyInFront = false;

    VecTrunc(m_vecSource);
    GetVectorsReadyForRW();

    gTargetCoordsForLookingBehind = vecTargetCoords;
}

// 0x50F970
void CCam::Process_FollowPedWithMouse(const CVector& ThisCamsTarget, float TargetOrientation, float SpeedVar, float SpeedVarDesired) {
    constexpr auto fTranslateCamUp          = 0.8f;      // 0x8CC7D0
    constexpr auto fStickSens               = 0.01f;     // 0x8CC7CC
    constexpr auto fDefaultAlphaOrient      = -0.22f;    // 0x8CC7D8
    constexpr auto nFadeControlThreshhold   = 45;        // 0x8CC7D4
    constexpr auto fBaseDist                = 1.7f;      // 0x8CC7C0
    constexpr auto fAngleDist               = 2.0f;      // 0x8CC7C4
    constexpr auto fFalloff                 = 3.0f;      // 0x8CC7C8
    constexpr auto fTweakFOV                = 1.1f;      // 0x862F10
    constexpr auto fMouseAvoidGeomReturnRate = 0.9f;     // 0x858C20
    constexpr auto fRangePlayerRadius       = 0.5f;      // 0x8CC38C
    constexpr auto NORMAL_NEAR_CLIP         = 0.3f;      // Derived: 0x858CC8 (0.6) and 0x8CC540 (0.4) are +0.3 / +0.1
    constexpr auto MaxRotationUp            = 0.785398f; // 0x859AB0, 45 deg
    constexpr auto MaxRotationDown          = 1.56207f;  // 0x8630F4, 89.5 deg

    static auto& gaTempSphereColPoints = StaticRef<std::array<CColPoint, 32>>(0xB9B250);

    constexpr uint16 DISABLE_SCRIPT_CONTROLS = 1; // `CPad::DisablePlayerControls` bit set by scripts

    m_fFOV = 70.0f;

    if (!m_pCamTargetEntity->GetIsTypePed()) {
        return;
    }

    if (m_bResetStatics) {
        m_bRotating          = false;
        m_bCollisionChecksOn = true;
        CPad::GetPad(0)->ClearMouseHistory();
        m_bResetStatics = false;
    }

    const auto playerVeh = FindPlayerVehicle();
    const auto HackPlayerOnStoppingTrain = playerVeh && playerVeh->m_nVehicleType == VEHICLE_TYPE_TRAIN;

    auto vecTargetCoords = ThisCamsTarget;
    vecTargetCoords.z += fTranslateCamUp; // Look at the ped's head rather than their torso

    float StickBetaOffset, StickAlphaOffset;
    if (CPad::GetPad(0)->DisablePlayerControls & DISABLE_SCRIPT_CONTROLS) {
        const auto cam2Target = (m_vecSource - vecTargetCoords).Normalized();
        const auto fNewBeta   = cam2Target.z < -0.9f
            ? TargetOrientation + PI
            : std::atan2(cam2Target.y, cam2Target.x);

        StickBetaOffset  = fNewBeta - m_fHorizontalAngle;
        StickAlphaOffset = 0.0f;
    } else {
        float fStickX, fStickY;
        bool  bUsingMouse = false;

        const auto mouse = CPad::GetPad(0)->NewMouseControllerState.GetAmountMouseMoved();
        if ((mouse.x != 0.0f || mouse.y != 0.0f) && !CPad::GetPad(0)->DisablePlayerControls) {
            fStickX     = -mouse.x * 2.5f;
            fStickY     = mouse.y * 4.0f;
            bUsingMouse = true;
        } else { // No mouse - use the second stick to move the camera around
            fStickX = -(float)CPad::GetPad(0)->LookAroundLeftRight(m_pCamTargetEntity->AsPed());
            fStickY = (float)CPad::GetPad(0)->LookAroundUpDown(m_pCamTargetEntity->AsPed());
        }

        if (bUsingMouse) {
            StickBetaOffset  = TheCamera.m_fMouseAccelHorzntl * fStickX * (m_fFOV / 80.0f);
            StickAlphaOffset = TheCamera.m_fMouseAccelVertical * fStickY * (m_fFOV / 80.0f);
        } else {
            StickBetaOffset  = fStickSens * fStickX * (0.25f / 3.5f * (m_fFOV / 80.0f)) * CTimer::GetTimeStep();
            StickAlphaOffset = fStickSens * fStickY * (0.15f / 3.5f * (m_fFOV / 80.0f)) * CTimer::GetTimeStep();
        }
    }

    // Spring the pitch back to the default while fading in
    if ((TheCamera.GetFading() && TheCamera.GetFadingDirection() == +eFadeFlag::FADE_IN && CDraw::FadeValue > nFadeControlThreshhold)
        || CDraw::FadeValue > 200
        || (CPad::GetPad(0)->DisablePlayerControls & DISABLE_SCRIPT_CONTROLS)
    ) {
        if (m_fVerticalAngle < fDefaultAlphaOrient - 0.05f) {
            StickAlphaOffset = 0.05f;
        } else if (m_fVerticalAngle < fDefaultAlphaOrient) {
            StickAlphaOffset = fDefaultAlphaOrient - m_fVerticalAngle;
        } else if (m_fVerticalAngle > fDefaultAlphaOrient + 0.05f) {
            StickAlphaOffset = -0.05f;
        } else if (m_fVerticalAngle > fDefaultAlphaOrient) {
            StickAlphaOffset = fDefaultAlphaOrient - m_fVerticalAngle;
        } else {
            StickAlphaOffset = 0.0f;
        }
    }

    m_fHorizontalAngle += StickBetaOffset;
    m_fVerticalAngle   += StickAlphaOffset;
    ClipBeta();
    m_fVerticalAngle = std::clamp(m_fVerticalAngle, -MaxRotationDown, MaxRotationUp);

    auto fDefaultDistFromPed = fBaseDist;
    fDefaultDistFromPed += m_fVerticalAngle > 0.0f
        ? fAngleDist * std::cos(std::min(HALF_PI, fFalloff * m_fVerticalAngle))
        : fAngleDist * std::cos(m_fVerticalAngle);

    if (TheCamera.m_bUseTransitionBeta) {
        m_fHorizontalAngle = m_fTransitionBeta;
    }
    if (TheCamera.m_bCamDirectlyBehind) {
        m_fHorizontalAngle = TheCamera.m_fPedOrientForBehindOrInFront + PI;
    }
    if (TheCamera.m_bCamDirectlyInFront) {
        m_fHorizontalAngle = TheCamera.m_fPedOrientForBehindOrInFront;
    }
    if (HackPlayerOnStoppingTrain) { // Heading was set when getting on the train
        m_fHorizontalAngle = TargetOrientation;
    }

    m_vecFront = CVector{
        -std::cos(m_fHorizontalAngle) * std::cos(m_fVerticalAngle),
        -std::sin(m_fHorizontalAngle) * std::cos(m_fVerticalAngle),
        std::sin(m_fVerticalAngle)
    };
    m_vecSource = vecTargetCoords - fDefaultDistFromPed * m_vecFront;
    m_vecTargetCoorsForFudgeInter = vecTargetCoords;

    CColPoint colPoint{};
    CEntity*  hitEntity = nullptr;

    CWorld::pIgnoreEntity = m_pCamTargetEntity;
    if (CWorld::ProcessLineOfSight(vecTargetCoords, m_vecSource, colPoint, hitEntity, true, true, true, true, false, false, true, false)) {
        auto       distColToPed = (vecTargetCoords - colPoint.m_vecPoint).Magnitude();
        const auto distCamToCol = fDefaultDistFromPed - distColToPed;

        // A ped in the way can be acceptable, as long as nothing closer to the camera is
        if (hitEntity->GetIsTypePed() && distCamToCol > NORMAL_NEAR_CLIP + 0.1f) {
            if (CWorld::ProcessLineOfSight(colPoint.m_vecPoint, m_vecSource, colPoint, hitEntity, true, true, true, true, false, false, true, false)) {
                distColToPed = (vecTargetCoords - colPoint.m_vecPoint).Magnitude();
                m_vecSource  = colPoint.m_vecPoint;
                if (distColToPed < NORMAL_NEAR_CLIP + 0.3f) {
                    RwCameraSetNearClipPlane(Scene.m_pRwCamera, std::max(0.05f, distColToPed - 0.3f));
                }
            } else {
                RwCameraSetNearClipPlane(Scene.m_pRwCamera, std::min(0.9f, distCamToCol - 0.35f));
            }
        } else {
            m_vecSource = colPoint.m_vecPoint;
            if (distColToPed < NORMAL_NEAR_CLIP + 0.3f) {
                RwCameraSetNearClipPlane(Scene.m_pRwCamera, std::max(0.05f, distColToPed - 0.3f));
            }
        }
    }
    CWorld::pIgnoreEntity = nullptr;

    const auto tanFOV = std::tan(0.5f * DegreesToRadians(m_fFOV)) * fTweakFOV * CDraw::GetAspectRatio();

    auto nearClip      = RwCameraGetNearClipPlane(Scene.m_pRwCamera);
    auto nearClipWidth = nearClip * tanFOV;

    auto* findEntity = CWorld::TestSphereAgainstWorld(m_vecSource + nearClip * m_vecFront, nearClipWidth, nullptr, true, true, false, true, false, true);
    for (auto i = 0; findEntity; i++) {
        auto test  = gaTempSphereColPoints[0].m_vecPoint - m_vecSource;
        auto temp  = DotProduct(test, m_vecFront);
        test       = test - temp * m_vecFront;
        temp       = std::clamp(test.Magnitude() / tanFOV, 0.1f, nearClip);

        if (temp < nearClip) {
            RwCameraSetNearClipPlane(Scene.m_pRwCamera, temp);
        }

        // If the near clip couldn't go close enough, move the camera towards the ped instead
        if (temp == 0.1f) {
            m_vecSource = m_vecSource + 0.3f * (vecTargetCoords - m_vecSource);
        }

        nearClip      = RwCameraGetNearClipPlane(Scene.m_pRwCamera);
        nearClipWidth = nearClip * std::tan(0.5f * DegreesToRadians(m_fFOV)) * fTweakFOV * CDraw::GetAspectRatio();
        findEntity    = CWorld::TestSphereAgainstWorld(m_vecSource + nearClip * m_vecFront, nearClipWidth, nullptr, true, true, false, true, false, true);

        if (i >= 5) {
            break;
        }
    }

    // Buffer how fast the camera moves back away from the player, to smooth things out
    const auto newDistance = (vecTargetCoords - m_vecSource).Magnitude();
    if (newDistance < m_fDistance) {
        m_fDistance = newDistance;
    } else {
        const auto rate = std::pow(fMouseAvoidGeomReturnRate, CTimer::GetTimeStep());
        m_fDistance = rate * m_fDistance + (1.0f - rate) * newDistance;

        if (newDistance > 0.05f) {
            m_vecSource = vecTargetCoords + (m_vecSource - vecTargetCoords) * m_fDistance / newDistance;
        }

        // We may have ended up closer to the player than `AvoidTheGeometry` expected, so we'd clip through them
        if (const auto playCamDistWithCol = m_fDistance - fRangePlayerRadius; playCamDistWithCol < RwCameraGetNearClipPlane(Scene.m_pRwCamera)) {
            RwCameraSetNearClipPlane(Scene.m_pRwCamera, std::max(playCamDistWithCol, 0.1f));
        }
    }

    TheCamera.m_bCamDirectlyBehind  = false;
    TheCamera.m_bCamDirectlyInFront = false;

    GetVectorsReadyForRW();

    // Only force the ped's direction during the dark part of a fade in
    if (TheCamera.GetFadingDirection() == +eFadeFlag::FADE_IN && CDraw::FadeValue > 128) {
        const auto  CamDirection = std::atan2(-m_vecFront.x, m_vecFront.y);
        auto* const camTarget    = TheCamera.m_pTargetEntity->AsPed();
        camTarget->m_fCurrentRotation = CamDirection;
        camTarget->m_fAimingRotation  = CamDirection;
        camTarget->SetHeading(CamDirection);
        camTarget->UpdateRwMatrix();
    }
}

// 0x522D40
void CCam::Process_FollowPed_SA(const CVector &ThisCamsTarget, float TargetOrientation, float SpeedVar, float SpeedVarDesired, bool bScriptSetAngles) {
    if (!m_pCamTargetEntity->GetIsTypePed())
        return;

    if (!((CPed *)m_pCamTargetEntity)->IsPlayer())
        return;

    CPed *pPed = (CPed *)m_pCamTargetEntity;
    CPad *pPad = CPad::GetPad(0);

    if (pPed->GetType() == PED_TYPE_PLAYER2)
    {
        pPad = CPad::GetPad(1);
    }

    CVector vecTargetCoords = ThisCamsTarget;
    bool bIsGettingIntoCar = false;

    int32 nType = FOLLOW_PED_OUTSIDE;

	if (CGame::currArea != AREA_CODE_NORMAL_WORLD)
    {
        nType = FOLLOW_PED_INSIDE;
    }

    float distToPlayer = PEDCAM_SET[nType].fBaseCamDist;
    if (!pPed->bIsStanding && TheCamera.m_nPedZoom == ZOOM_THREE) {
        bool bIsParachuting = pPed->GetIntelligence()->GetUsingParachute();
        if (bIsParachuting) {
            distToPlayer *= 2.0f;
        }
    }

    float fCamDistance = TheCamera.m_fPedZoomSmoothed + distToPlayer;
    float fMinDistance = PEDCAM_SET[nType].fMinDist;
    float fMinFollowDist = PEDCAM_SET[nType].fMinFollowDist;

    float fUpLimit = PEDCAM_SET[nType].fUpLimit;
    float fDownLimit = PEDCAM_SET[nType].fDownLimit;

    if (fCamDistance > gLastCamDist)
        fMinDistance = fCamDistance;
    gLastCamDist = fCamDistance;

    float fCamAlpha = PEDCAM_SET[nType].fBaseCamZ;
    if (TheCamera.m_nPedZoom == ZOOM_ONE)
        fCamAlpha += m_fTargetZoomOneZExtra;
    else if (TheCamera.m_nPedZoom == ZOOM_TWO) {
        if (nType == FOLLOW_PED_INSIDE)
            fCamAlpha += m_fTargetZoomTwoInteriorZExtra;
        else
            fCamAlpha += m_fTargetZoomTwoZExtra;
    } else if (TheCamera.m_nPedZoom == ZOOM_THREE)
        fCamAlpha += m_fTargetZoomThreeZExtra;

    float fForceCamAlphaMult = 0.0f;
    float fForceCamBetaMult = 0.0f;
    if (pPed->GetIntelligence()->GetTaskSwim()) {
        fForceCamBetaMult = 1.0f;
        if (pPed->GetIntelligence()->GetTaskSwim()->m_nSwimState != eSwimState::SWIM_UNDERWATER_SPRINTING) {
            fForceCamAlphaMult = SWIM_CAM_ALPHA_FORCE;
        }
    } else if (pPed->GetIntelligence()->GetTaskJetPack()) {
        fForceCamBetaMult = 0.5f;
        if (!pPed->bIsStanding) {
            fForceCamAlphaMult = JETPACK_CAM_ALPHA_FORCE;
        }
    }

    if (!TheCamera.m_bTransitionState) {
        float fDesiredFOV = 70.0f;
        if (m_bResetStatics)
            m_fFOV = fDesiredFOV;
        else {
            float fRate = AIMWEAPON_FOV_ZOOM_RATE * CTimer::GetTimeStep();
            if (fDesiredFOV > m_fFOV + fRate)
                m_fFOV += fRate;
            else if (fDesiredFOV < m_fFOV - fRate)
                m_fFOV -= fRate;
            else
                m_fFOV = fDesiredFOV;
        }
    }

    vecTargetCoords.z += PEDCAM_SET[nType].fTargetOffsetZ;
    float fFollowDist = std::max(fCamDistance, fMinFollowDist);

    if (m_bResetStatics || TheCamera.m_bCamDirectlyBehind || TheCamera.m_bCamDirectlyInFront || bScriptSetAngles) {
        if (bScriptSetAngles) {
            vecPedPosEst = pPed->GetPosition();
            vecPedPosTrend = CVector(0.0f, 0.0f, 0.0f);

            vecTargetCoords = pPed->GetPosition();
            vecTargetCoords.z += PEDCAM_SET[nType].fTargetOffsetZ;
        }

        TheCamera.ResetDuckingSystem(pPed);
        m_bRotating = false;
        gForceCamBehindPlayer = false;
        m_bCollisionChecksOn = true;
        if (!TheCamera.m_bJustCameOutOfGarage && !bScriptSetAngles) // dont want to change alpha and beta!
        {
            m_fHorizontalAngle = pPed->GetHeading() - HALF_PI;
            if (TheCamera.m_bCamDirectlyInFront)
                m_fHorizontalAngle += PI;
        }

        m_fBetaSpeed = 0.0f;
        m_fAlphaSpeed = 0.0f;
        m_fDistance = 1000.0f;

        if (fCamDistance == TheCamera.m_fPedZoomBase)
            fCamDistance = TheCamera.m_fPedZoomSmoothed = TheCamera.m_fPedZoomTotal;

        if (!TheCamera.m_bJustCameOutOfGarage && !bScriptSetAngles) // dont want to change alpha and beta!
        {
            m_fVerticalAngle = 0.0f;
            if (pPed->bIsStanding) {
                float fGroundNormalFwd = DotProduct(pPed->m_vecGroundNormal, pPed->GetMatrix().GetForward());
                m_fVerticalAngle -= std::asin(std::min(1.0f, std::max(-1.0f, fGroundNormalFwd)));
            }
        }

        m_vecFront = CVector(-std::cos(m_fHorizontalAngle) * std::cos(m_fVerticalAngle), -std::sin(m_fHorizontalAngle) * std::cos(m_fVerticalAngle), std::sin(m_fVerticalAngle));

        // we want to store the last camera position always without including its alpha offset
        m_avecTargetHistoryPos[0] = vecTargetCoords - fFollowDist * m_vecFront;
        m_anTargetHistoryTime[0] = CTimer::GetTimeInMS();
        m_avecTargetHistoryPos[1] = vecTargetCoords - fCamDistance * m_vecFront;
        m_nCurrentHistoryPoints = 0;

        if (!TheCamera.m_bJustCameOutOfGarage && !bScriptSetAngles) // dont want to change alpha and beta!
            m_fVerticalAngle = -fCamAlpha;

        if (pPed->GetIntelligence()->GetTaskSwim() && pPed->GetIntelligence()->GetTaskSwim()->m_nSwimState != eSwimState::SWIM_UNDERWATER_SPRINTING)
            m_fVerticalAngle += SWIM_CAM_ALPHA_EXTRA;
        else if (pPed->GetIntelligence()->GetTaskJetPack())
            m_fVerticalAngle += JETPACK_CAM_ALPHA_EXTRA;

        CPad::GetPad(0)->ClearMouseHistory();
    }

	else if (pPed->m_standingOnEntity) {
        auto* standingPhys = (CPhysical*)pPed->m_standingOnEntity;
        if ((standingPhys->GetIsTypeVehicle() && ((CVehicle*)standingPhys)->IsTrain()) || (standingPhys->m_pAttachedTo && standingPhys->m_pAttachedTo->GetIsTypeVehicle() && ((CVehicle*)standingPhys->m_pAttachedTo)->IsTrain())) {
            static float AMOUNT_OF_SPEED_TO_ADD = 0.01f;
            float fMagnitude = standingPhys->GetMoveSpeed().Magnitude();
            fMagnitude = std::max(0.0f, fMagnitude - AMOUNT_OF_SPEED_TO_ADD) / std::max(AMOUNT_OF_SPEED_TO_ADD, fMagnitude);
            m_avecTargetHistoryPos[0] += fMagnitude * standingPhys->GetMoveSpeed() * CTimer::GetTimeStep();
            m_avecTargetHistoryPos[1] += fMagnitude * standingPhys->GetMoveSpeed() * CTimer::GetTimeStep();
        }
    }

    m_vecFront = vecTargetCoords - m_avecTargetHistoryPos[0];
    m_vecFront.Normalise();
    float fTempLength = (vecTargetCoords - m_avecTargetHistoryPos[1]).Magnitude();
    if (fTempLength < fCamDistance && fCamDistance > PEDCAM_SET[nType].fMinDist)
        fCamDistance = std::max(fMinDistance, fTempLength);

    float fTargetDiff = 0.0f;
    float fDiffMult = 0.0f;
    float fDiffCap = 0.0f;
    float fTargetBeta = std::atan2(-m_vecFront.x, m_vecFront.y) - HALF_PI;
    if (fTargetBeta < -PI)
        fTargetBeta += TWO_PI;

    float fHeadingBeta = pPed->GetHeading() - HALF_PI;
    if (fHeadingBeta - fTargetBeta > PI)
        fHeadingBeta -= TWO_PI;
    else if (fHeadingBeta - fTargetBeta < -PI)
        fHeadingBeta += TWO_PI;

    if (pPad->GetForceCameraBehindPlayer())
        gForceCamBehindPlayer = true;
    else if (gForceCamBehindPlayer) {
        if (pPed->GetMoveSpeed().SquaredMagnitude() > 0.001f || std::abs(fHeadingBeta - fTargetBeta) < 0.01f || pPad->AimWeaponLeftRight(pPed) != 0 || pPad->AimWeaponUpDown(pPed) != 0) {
            gForceCamBehindPlayer = false;
        }
    }

    static float HEADING_TOWARD_PLAYER_BETA_LIMIT = DegreesToRadians(170.0f);
    if (std::abs(fHeadingBeta - fTargetBeta) < HEADING_TOWARD_PLAYER_BETA_LIMIT || gForceCamBehindPlayer || fForceCamBetaMult) {
        fDiffMult = PEDCAM_SET[nType].fDiffBetaSwing * CTimer::GetTimeStep();
        fDiffCap = PEDCAM_SET[nType].fDiffBetaSwingCap * CTimer::GetTimeStep();
        if (gForceCamBehindPlayer || fForceCamBetaMult) {
            static float FORCE_CAM_SPEED_MULT = 0.5f;
            fDiffMult = std::min(1.0f, fDiffMult * FORCE_CAM_SPEED_MULT);
            static float FORCE_CAM_CAP_MULT = 2.0f;
            fDiffCap *= FORCE_CAM_CAP_MULT;

            if (fForceCamBetaMult) {
                fDiffMult *= fForceCamBetaMult;
                fDiffCap *= fForceCamBetaMult;
            }
        } else {
            if (pPed->m_standingOnEntity)
                fDiffMult = std::min(1.0f, (pPed->GetMoveSpeed() - ((CPhysical*)pPed->m_standingOnEntity)->GetMoveSpeed()).Magnitude() * fDiffMult);
            else
                fDiffMult = std::min(1.0f, pPed->GetMoveSpeed().Magnitude() * fDiffMult);
        }

        fTargetDiff = std::max(std::min(fDiffMult * (fHeadingBeta - fTargetBeta), fDiffCap), -fDiffCap);
    }

    fTargetBeta += fTargetDiff;
    if (fTargetBeta > m_fHorizontalAngle + PI)
        fTargetBeta -= TWO_PI;
    else if (fTargetBeta < m_fHorizontalAngle - PI)
        fTargetBeta += TWO_PI;
    float fCamControlBetaSpeed = (fTargetBeta - m_fHorizontalAngle) / std::max(1.0f, CTimer::GetTimeStep());

    float fTargetAlpha = std::asin(std::max(-1.0f, std::min(1.0f, m_vecFront.z)));

	static float HEADING_TOWARD_PLAYER_FOR_ALPHA = HALF_PI;
    static float HEADING_TOWARD_PLAYER_ALPHA_MAX = DegreesToRadians(20.0f);
    if (std::abs(fHeadingBeta - fTargetBeta) > HEADING_TOWARD_PLAYER_FOR_ALPHA && pPed->GetMoveSpeed().SquaredMagnitude() > 0.002f) {
        float fAlphaLimit = std::abs(fHeadingBeta - fTargetBeta) - HEADING_TOWARD_PLAYER_FOR_ALPHA;
        fAlphaLimit = std::min(1.0f, 1.2f * fAlphaLimit / (PI - HEADING_TOWARD_PLAYER_FOR_ALPHA));
        fAlphaLimit = HALF_PI - (HALF_PI - HEADING_TOWARD_PLAYER_ALPHA_MAX) * fAlphaLimit;

        static float HEADING_TOWARD_PLAYER_ALPHA_RATE = 0.90f;
        float fRate = std::pow(HEADING_TOWARD_PLAYER_ALPHA_RATE, CTimer::GetTimeStep());

        if (fTargetAlpha > fAlphaLimit)
            fTargetAlpha = fRate * fTargetAlpha + (1.0f - fRate) * fAlphaLimit;
        else if (fTargetAlpha < -fAlphaLimit)
            fTargetAlpha = fRate * fTargetAlpha - (1.0f - fRate) * fAlphaLimit;
    }

    fTargetDiff = 0.0f;
    if (fForceCamAlphaMult || (gForceCamBehindPlayer && pPed->bIsStanding)) {
        float fGroundAlpha = 0.0f;
        if (pPed->GetIntelligence()->GetTaskJetPack()) {
            fGroundAlpha += JETPACK_CAM_ALPHA_EXTRA;
        } else if (pPed->GetIntelligence()->GetTaskSwim()) {
            fGroundAlpha += SWIM_CAM_ALPHA_EXTRA;
        } else if (pPed->bIsStanding) {
            float fGroundNormalFwd = DotProduct(pPed->m_vecGroundNormal, pPed->GetMatrix().GetForward());
            fGroundAlpha = -std::asin(std::min(1.0f, std::max(-1.0f, fGroundNormalFwd)));
        }

        static float FORCE_CAM_ALPHA_SPEED_MULT = 1.0f;
        fDiffMult = std::min(1.0f, fDiffMult * FORCE_CAM_ALPHA_SPEED_MULT);
        static float FORCE_CAM_ALPHA_CAP_MULT = 4.0f;
        fDiffCap *= FORCE_CAM_ALPHA_CAP_MULT;

        if (fForceCamAlphaMult) {
            fDiffMult *= fForceCamAlphaMult;
            fDiffCap *= fForceCamAlphaMult;
        }

        fTargetDiff = std::max(std::min(fDiffMult * (fGroundAlpha - fTargetAlpha), fDiffCap), -fDiffCap);
    }

    fTargetAlpha += fTargetDiff;
    fTargetAlpha -= fCamAlpha;

    if (fTargetAlpha > fUpLimit)
        fTargetAlpha = fUpLimit;
    else if (fTargetAlpha < -fDownLimit)
        fTargetAlpha = -fDownLimit;

    fDiffMult = std::pow(PEDCAM_SET[nType].fDiffAlphaRate, CTimer::GetTimeStep());
    fDiffCap = PEDCAM_SET[nType].fDiffAlphaCap * CTimer::GetTimeStep();

    fTargetDiff = std::max(std::min((1.0f - fDiffMult) * (fTargetAlpha - m_fVerticalAngle), fDiffCap), -fDiffCap);

    float StickBetaOffset = -float(pPad->AimWeaponLeftRight(pPed));
    float StickAlphaOffset = pPad->AimWeaponUpDown(pPed);

    if (bIsGettingIntoCar) {
        StickBetaOffset = 0;
        StickAlphaOffset = 0;
    }

	else if (pPed->GetIntelligence()->GetTaskUseGun() && pPed->GetIntelligence()->GetTaskUseGun()->m_WeaponInfo && !pPed->GetIntelligence()->GetTaskUseGun()->m_WeaponInfo->flags.bAimWithArm) {
        if (std::abs(pPad->GetPedWalkLeftRight()) > std::abs(StickBetaOffset))
            StickBetaOffset = -float(pPad->GetPedWalkLeftRight());
    }

    if ((StickAlphaOffset || StickBetaOffset) && !(pPad->GetPedWalkLeftRight() || pPad->GetPedWalkUpDown())) {
        CPed *pPlayerPed = FindPlayerPed(0);
        CVector playerForward = pPlayerPed->GetMatrix().GetForward();
        CVector cameraForward = TheCamera.GetForward();

        if (DotProduct(playerForward, cameraForward) > 0.3f) {
            CVector lookAtPos = pPlayerPed->GetPosition() + (5.0f * cameraForward);
            g_ikChainMan.LookAt("FollowPedSA", pPlayerPed, nullptr, 1500, (eBoneTag)-1, &lookAtPos, false);
        }
    }

    StickBetaOffset = AIMWEAPON_STICK_SENS * AIMWEAPON_STICK_SENS * std::abs(StickBetaOffset) * StickBetaOffset * (0.25f / 3.5f * (m_fFOV / 80.0f));
    StickAlphaOffset = AIMWEAPON_STICK_SENS * AIMWEAPON_STICK_SENS * std::abs(StickAlphaOffset) * StickAlphaOffset * (0.15f / 3.5f * (m_fFOV / 80.0f));

    if (pPed->GetIntelligence()->GetTaskClimb()) {
        pPed->GetIntelligence()->GetTaskClimb()->GetCameraStickModifier(pPed, m_fVerticalAngle, m_fHorizontalAngle, StickAlphaOffset, StickBetaOffset);
    }
    else if (pPed->GetIntelligence()->GetTaskManager().GetActiveTask() && pPed->GetIntelligence()->GetTaskManager().GetActiveTask()->GetTaskType() == TASK_COMPLEX_ENTER_CAR_AS_DRIVER) {
        ((CTaskComplexEnterCar *)pPed->GetIntelligence()->GetTaskManager().GetActiveTask())->GetCameraStickModifier(pPed, fCamDistance, m_fVerticalAngle, m_fHorizontalAngle, StickAlphaOffset, StickBetaOffset);
    }

    fDiffMult = std::pow(PEDCAM_SET[nType].fDiffBetaRate, CTimer::GetTimeStep());
    fDiffCap = PEDCAM_SET[nType].fDiffBetaCap;

    fCamControlBetaSpeed += StickBetaOffset;

    if (fCamControlBetaSpeed > fDiffCap)
        fCamControlBetaSpeed = fDiffCap;
    else if (fCamControlBetaSpeed < -fDiffCap)
        fCamControlBetaSpeed = -fDiffCap;

    m_fBetaSpeed = fDiffMult * m_fBetaSpeed + (1.0f - fDiffMult) * fCamControlBetaSpeed;

    if (bIsGettingIntoCar) {
        m_fBetaSpeed = 0;
    }

    static float gBetaSpeedTol = 0.0001f;
    if (std::abs(m_fBetaSpeed) < gBetaSpeedTol)
        m_fBetaSpeed = 0.0f;

	if (TheCamera.m_bUseMouse3rdPerson && pPad->DisablePlayerControls == 0) {
        float fStickX = -CPad::NewMouseControllerState.m_AmountMoved.x * 2.5f;
        StickBetaOffset = TheCamera.m_fMouseAccelHorzntl * fStickX * (m_fFOV / 80.0f);
        m_fHorizontalAngle += StickBetaOffset;
        m_fBetaSpeed = 0.0f;
    } else
    {
        m_fHorizontalAngle += m_fBetaSpeed * CTimer::GetTimeStep();
    }

    ClipBeta();

    m_fAlphaSpeed = fDiffMult * StickAlphaOffset + (1.0f - fDiffMult) * m_fAlphaSpeed;
    if (m_fAlphaSpeed > fDiffCap)
        m_fAlphaSpeed = fDiffCap;
    else if (m_fAlphaSpeed < -fDiffCap)
        m_fAlphaSpeed = -fDiffCap;

    static float gAlphaSpeedTol = 0.0001f;
    if (std::abs(m_fAlphaSpeed) < gAlphaSpeedTol)
        m_fAlphaSpeed = 0.0f;

    if (bIsGettingIntoCar) {
        m_fAlphaSpeed = 0;
    }

    fTargetAlpha += m_fAlphaSpeed * CTimer::GetTimeStep();

    if (TheCamera.m_bUseMouse3rdPerson && pPad->DisablePlayerControls == 0) {
        float fStickY = CPad::NewMouseControllerState.m_AmountMoved.y * 2.5f;
        StickAlphaOffset = TheCamera.m_fMouseAccelHorzntl * fStickY * (m_fFOV / 80.0f);

        if ((TheCamera.GetFading() && TheCamera.GetFadingDirection() == FADING_IN && CDraw::FadeValue > 45) || CDraw::FadeValue > 200) {
            float fDefaultAlphaOrient = -fCamAlpha;

            if (m_fVerticalAngle < fDefaultAlphaOrient - 0.05f)
                StickAlphaOffset = 0.05f;
            else if (m_fVerticalAngle < fDefaultAlphaOrient)
                StickAlphaOffset = fDefaultAlphaOrient - m_fVerticalAngle;
            else if (m_fVerticalAngle > fDefaultAlphaOrient + 0.05f)
                StickAlphaOffset = -0.05f;
            else if (m_fVerticalAngle > fDefaultAlphaOrient)
                StickAlphaOffset = fDefaultAlphaOrient - m_fVerticalAngle;
            else
                StickAlphaOffset = 0.0f;
        }

        m_fVerticalAngle += StickAlphaOffset;
        m_fAlphaSpeed = 0.0f;
    } else
    {
        m_fVerticalAngle += fTargetDiff;
    }

    if (m_fVerticalAngle > fUpLimit) {
        m_fVerticalAngle = fUpLimit;
        m_fAlphaSpeed = 0.0f;
    } else if (m_fVerticalAngle < -fDownLimit) {
        m_fVerticalAngle = -fDownLimit;
        m_fAlphaSpeed = 0.0f;
    }

	static float gOldAlpha = -9999.0f;
    static float gOldBeta = -9999.0f;
    static float gAlphaTol = 0.0001f;
    static float gBetaTol = 0.0001f;

    float dA = std::abs(gOldAlpha - m_fVerticalAngle);
    if (dA < gAlphaTol)
    {
        m_fVerticalAngle = gOldAlpha;
    }

    gOldAlpha = m_fVerticalAngle;

    float dB = std::abs(gOldBeta - m_fHorizontalAngle);
    if (dB < gBetaTol)
        m_fHorizontalAngle = gOldBeta;
    gOldBeta = m_fHorizontalAngle;

	m_vecFront = CVector(-std::cos(m_fHorizontalAngle) * std::cos(m_fVerticalAngle), -std::sin(m_fHorizontalAngle) * std::cos(m_fVerticalAngle), std::sin(m_fVerticalAngle));
    GetVectorsReadyForRW();

    TheCamera.m_bCamDirectlyBehind = false;
    TheCamera.m_bCamDirectlyInFront = false;
    m_vecSource = vecTargetCoords - m_vecFront * fCamDistance;

    VecTrunc(m_vecSource);

    fTargetAlpha += fCamAlpha;
    m_avecTargetHistoryPos[0] = vecTargetCoords - fFollowDist * CVector(-std::cos(m_fHorizontalAngle) * std::cos(fTargetAlpha), -std::sin(m_fHorizontalAngle) * std::cos(fTargetAlpha), std::sin(fTargetAlpha));
    m_avecTargetHistoryPos[1] = vecTargetCoords - fCamDistance * CVector(-std::cos(m_fHorizontalAngle) * std::cos(fTargetAlpha), -std::sin(m_fHorizontalAngle) * std::cos(fTargetAlpha), std::sin(fTargetAlpha));

    if (pPad->GetForceCameraBehindPlayer() && pPad->AimWeaponLeftRight(nullptr)) {
        float fHeadingDiff = m_fHorizontalAngle - (pPed->GetHeading() - HALF_PI);
        if (fHeadingDiff > PI)
            fHeadingDiff -= TWO_PI;
        else if (fHeadingDiff < -PI)
            fHeadingDiff += TWO_PI;

        if (std::abs(fHeadingDiff) < 0.1f * CTimer::GetTimeStep())
            pPed->m_fAimingRotation = m_fHorizontalAngle + HALF_PI;
    }

    TheCamera.HandleCameraMotionForDucking(pPed, &m_vecSource, &vecTargetCoords, false);
    m_vecTargetCoorsForFudgeInter = vecTargetCoords;

    VecTrunc(m_vecSource);

    TheCamera.SetColVarsPed((ePedType)nType, TheCamera.m_nPedZoom);
    if (m_nDirectionWasLooking == LOOKING_FORWARD) {
        TheCamera.CameraGenericModeSpecialCases(pPed);
        TheCamera.CameraPedModeSpecialCases();
        TheCamera.CameraColDetAndReact(&m_vecSource, &vecTargetCoords);
        TheCamera.ImproveNearClip(nullptr, pPed, &m_vecSource, &vecTargetCoords);

        VecTrunc(m_vecSource);
    }

    TheCamera.m_bCamDirectlyBehind = false;
    TheCamera.m_bCamDirectlyInFront = false;

    VecTrunc(m_vecSource);

    GetVectorsReadyForRW();

    bool bProcessedIdleCam = false;
    if (nType == FOLLOW_PED_OUTSIDE && TheCamera.m_nWhoIsInControlOfTheCamera != 2 && pPed->bIsStanding && !CGameLogic::IsCoopGameGoingOn()) {
       if (!TheCamera.m_bFOVLerpProcessed && !TheCamera.m_bVecMoveLinearProcessed && !TheCamera.m_bVecTrackLinearProcessed)
       {
           float v = pPed->m_vecMoveSpeed.SquaredMagnitude();
           if (v <= 0.01f * 0.01f)
           {
               gIdleCam.Process();
               bProcessedIdleCam = true;
           }
       }
    }

    if (!bProcessedIdleCam) {
        gIdleCam.m_IdleTickerFrames = 0;
    }

    this->m_bResetStatics = false;
}

// 0x5105C0
float M16_1STPERSON_SOFTLIMIT_ANG = 0.75f;
float M16_1STPERSON_SOFTLIMIT_MULT = 0.05f;
float M16_1STPERSON_ROTATETHEHELI_SPEED = 0.1f;
float M16_1STPERSON_MOUSEWHEEL_ZOOM_RATE = 7.0f;
float fCameraNearClipMult = 0.15f;

void CCam::Process_M16_1stPerson(const CVector& ThisCamsTarget, float TargetOrientation, float SpeedVar, float SpeedVarDesired) {
    constexpr auto CAM_BUMPED_SWING_PERIOD = 120.0f;
    constexpr auto CAM_BUMPED_MOVE_MULT    = 0.05f;
    constexpr auto CAM_BUMPED_DAMP_RATE    = 0.95f;
    constexpr auto CAM_BUMPED_END_TIME     = 300;
    constexpr auto fDuckingBackOffset      = 0.8f;
    constexpr auto fDuckingRightOffset     = 0.0f;

    if (!m_pCamTargetEntity || !m_pCamTargetEntity->GetIsTypePed()) {
        return;
    }

    auto*       targetPed           = m_pCamTargetEntity->AsPed();
    CVector     TargetCoors;
    const float MaxVerticalRotation = DegreesToRadians(89.5f);
    float       MaxRotationUp       = DegreesToRadians(60.0f);
    float       MaxRotationDown     = DegreesToRadians(85.5f);
    float       fStickX             = 0.0f;
    float       fStickY             = 0.0f;
    float       StickBetaOffset     = 0.0f;
    float       StickAlphaOffset    = 0.0f;
    float       HeightToNose        = 0.10f;
    float       DistToNose          = 0.19f;
    float       DistBack            = 0.3f;

    static bool  FailedTestTwelveFramesAgo = false;
    static float DPadHorizontal;
    static float DPadVertical;
    static float TargetFOV         = 0.0f;
    bool         bAttachedToEntity = targetPed->m_pAttachedTo != nullptr;

    TargetCoors = ThisCamsTarget;

    if (m_bResetStatics) {
        if (TheCamera.m_bUseMouse3rdPerson && !targetPed->m_pTargetedObject && !bAttachedToEntity) {
            // Keep the direction the camera is already looking in
        } else {
            m_fHorizontalAngle = bAttachedToEntity
                ? CTheScripts::fCameraHeadingWhenPlayerIsAttached
                : targetPed->m_fCurrentRotation - HALF_PI;
            m_fVerticalAngle = 0.0f;
        }
        m_fInitialPlayerOrientation = targetPed->m_fCurrentRotation - HALF_PI;
        m_bResetStatics             = false;
        FailedTestTwelveFramesAgo   = false;
        DPadHorizontal              = 0.0f;
        DPadVertical                = 0.0f;
        m_bCollisionChecksOn        = true;
        m_fFOVSpeed                 = 0.0f;
        TargetFOV                   = m_fFOV;
        m_fAlphaSpeed               = 0.0f;
        m_fBetaSpeed                = 0.0f;
    }

    if (m_nMode == MODE_SNIPER || m_nMode == MODE_CAMERA) {
        if (TargetFOV == 0.0f) {
            TargetFOV = m_fFOV;
        }

        if (CPad::GetPad(0)->SniperZoomOut()) {
            m_fFOV     *= (10000.0f + 255.0f * CTimer::GetTimeStep()) / 10000.0f;
            m_fFOVSpeed = 0.0f;
            TargetFOV   = m_fFOV;
        } else if (CPad::GetPad(0)->SniperZoomIn()) {
            m_fFOV     /= (10000.0f + 255.0f * CTimer::GetTimeStep()) / 10000.0f;
            m_fFOVSpeed = 0.0f;
            TargetFOV   = m_fFOV;
        } else if (std::abs(TargetFOV - m_fFOV) > 0.5f) {
            WellBufferMe(TargetFOV, m_fFOV, m_fFOVSpeed, 0.5f, 0.25f, false);
        } else {
            m_fFOVSpeed = 0.0f;
        }

        if (m_fFOV > 70.0f) {
            m_fFOV = 70.0f;
        }

        if (TargetFOV > 70.0f) {
            TargetFOV = 70.0f;
        } else if (m_nMode == MODE_CAMERA) {
            m_fFOV    = std::max(m_fFOV, 3.0f);
            TargetFOV = std::max(TargetFOV, 3.0f);
        } else {
            m_fFOV    = std::max(m_fFOV, 15.0f);
            TargetFOV = std::max(TargetFOV, 15.0f);
        }
        TheCamera.SetMotionBlur(180, 255, 180, 120, eMotionBlurType::SNIPER);
    } else {
        m_fFOV = 70.0f;
    }

    if (bAttachedToEntity && CTheScripts::fCameraHeadingStepWhenPlayerIsAttached > 0.0f) {
        float fDiff = m_fHorizontalAngle - CTheScripts::fCameraHeadingWhenPlayerIsAttached;
        if (fDiff < 0.0f) {
            fDiff += TWO_PI;
        }
        const float fDiff2 = TWO_PI - fDiff;
        if (fDiff < CTheScripts::fCameraHeadingStepWhenPlayerIsAttached || fDiff2 < CTheScripts::fCameraHeadingStepWhenPlayerIsAttached) {
            m_fHorizontalAngle                                 = CTheScripts::fCameraHeadingWhenPlayerIsAttached;
            CTheScripts::fCameraHeadingStepWhenPlayerIsAttached = 0.0f;
        } else if (fDiff > fDiff2) {
            m_fHorizontalAngle += CTheScripts::fCameraHeadingStepWhenPlayerIsAttached;
        } else {
            m_fHorizontalAngle -= CTheScripts::fCameraHeadingStepWhenPlayerIsAttached;
        }
    }

    bool       bUsingMouse   = false;
    const auto mouseMovement = CPad::GetPad(0)->NewMouseControllerState.GetAmountMouseMoved();

    if (mouseMovement.x == 0.0f && mouseMovement.y == 0.0f) {
        fStickX = -(float)CPad::GetPad(0)->LookAroundLeftRight(targetPed);
        fStickY = (float)CPad::GetPad(0)->LookAroundUpDown(targetPed);
    } else {
        fStickX     = -mouseMovement.x * 3.0f;
        fStickY     = mouseMovement.y * 3.0f;
        bUsingMouse = true;

        if (CPad::GetPad(0)->ArePlayerControlsDisabled() || CPad::GetPad(0)->JustOutOfFrontEnd || CTimer::GetTimeStep() <= 0.0f) {
            fStickX = 0.0f;
            fStickY = 0.0f;
        }
    }

    if (bUsingMouse) {
        StickBetaOffset  = TheCamera.m_fMouseAccelHorzntl * fStickX * (m_fFOV / 80.0f);
        StickAlphaOffset = TheCamera.m_fMouseAccelVertical * fStickY * (m_fFOV / 80.0f);
        m_fAlphaSpeed    = 0.0f;
        m_fBetaSpeed     = 0.0f;
    } else {
        if (bAttachedToEntity) {
            StickBetaOffset  = fStickX / 128.0f;
            StickBetaOffset  = std::abs(StickBetaOffset) * StickBetaOffset * (0.14f / 3.5f * (m_fFOV / 80.0f)) * CTimer::GetTimeStep();
            StickAlphaOffset = fStickY / 128.0f;
            StickAlphaOffset = std::abs(StickAlphaOffset) * StickAlphaOffset * (0.12f / 3.5f * (m_fFOV / 80.0f)) * CTimer::GetTimeStep();
        } else {
            const float X_Sign = fStickX < 0.0f ? -1.0f : 1.0f;
            const float Y_Sign = fStickY < 0.0f ? -1.0f : 1.0f;

            StickBetaOffset  = X_Sign * sq(fStickX / 100.0f) * (0.20f / 3.5f * (m_fFOV / 80.0f)) * CTimer::GetTimeStep();
            StickAlphaOffset = Y_Sign * sq(fStickY / 150.0f) * (0.25f / 3.5f * (m_fFOV / 80.0f)) * CTimer::GetTimeStep();
        }

        static float M16_1STPERSON_STICK_RATE_UP   = 0.8f;
        static float M16_1STPERSON_STICK_RATE_DOWN = 0.5f;

        float fRate = (std::abs(fStickX) < 2.0f && std::abs(fStickY) < 2.0f) ? M16_1STPERSON_STICK_RATE_DOWN : M16_1STPERSON_STICK_RATE_UP;
        fRate       = std::pow(fRate, CTimer::GetTimeStep());

        m_fBetaSpeed  = fRate * m_fBetaSpeed + (1.0f - fRate) * StickBetaOffset;
        m_fAlphaSpeed = fRate * m_fAlphaSpeed + (1.0f - fRate) * StickAlphaOffset;

        StickBetaOffset  = m_fBetaSpeed;
        StickAlphaOffset = m_fAlphaSpeed;
    }

    m_fHorizontalAngle += StickBetaOffset;
    m_fVerticalAngle += StickAlphaOffset;

    ClipBeta();

    if (m_nCamBumpedTime > 0) {
        float fAngTime = (float)(CTimer::GetTimeInMS() - m_nCamBumpedTime) / (float)CAM_BUMPED_SWING_PERIOD;
        fAngTime       = std::cos(fAngTime * TWO_PI);
        m_fHorizontalAngle += fAngTime * CAM_BUMPED_MOVE_MULT * m_fCamBumpedHorz;
        m_fVerticalAngle += fAngTime * CAM_BUMPED_MOVE_MULT * m_fCamBumpedVert;

        m_fCamBumpedHorz *= std::pow(CAM_BUMPED_DAMP_RATE, CTimer::GetTimeStep());
        m_fCamBumpedVert *= std::pow(CAM_BUMPED_DAMP_RATE, CTimer::GetTimeStep());

        if (CTimer::GetTimeInMS() > m_nCamBumpedTime + CAM_BUMPED_END_TIME) {
            m_nCamBumpedTime = 0;
        }
    }

    if (targetPed->bIsDucking) {
        DistBack = 0.8f;
    }

    if (bAttachedToEntity) {
        auto* pPlayerPed = targetPed;

        float fCentreAlpha, fCentreBeta;
        switch ((int32)pPlayerPed->m_fTurretAngleA) { // m_nAttachLookDirn
        case 1:
            fCentreAlpha = std::asin(std::clamp<float>(-pPlayerPed->m_pAttachedTo->GetMatrix().GetRight().z, -1.0f, 1.0f));
            fCentreBeta  = pPlayerPed->m_pAttachedTo->GetHeading();
            break;
        case 2:
            fCentreAlpha = std::asin(std::clamp<float>(-pPlayerPed->m_pAttachedTo->GetMatrix().GetForward().z, -1.0f, 1.0f));
            fCentreBeta  = pPlayerPed->m_pAttachedTo->GetHeading() + HALF_PI;
            break;
        case 3:
            fCentreAlpha = std::asin(std::clamp<float>(pPlayerPed->m_pAttachedTo->GetMatrix().GetRight().z, -1.0f, 1.0f));
            fCentreBeta  = pPlayerPed->m_pAttachedTo->GetHeading() - PI;
            break;
        default:
        case 0:
            fCentreAlpha = std::asin(std::clamp<float>(pPlayerPed->m_pAttachedTo->GetMatrix().GetForward().z, -1.0f, 1.0f));
            fCentreBeta  = pPlayerPed->m_pAttachedTo->GetHeading() - HALF_PI;
            break;
        }

        pPlayerPed->PositionAttachedPed();
        pPlayerPed->UpdateRwMatrix();
        pPlayerPed->UpdateRwFrame();
        pPlayerPed->UpdateRpHAnim();

        CVector posn{ 0.0f, 0.0f, 0.0f };
        if (pPlayerPed->m_pAttachedTo->GetIsTypeVehicle() && pPlayerPed->m_pAttachedTo->AsVehicle()->IsBike()) {
            HeightToNose = 0.0f;
            DistBack     = 0.0f;
            targetPed->GetTransformedBonePosition(posn, BONE_HEAD, true);
            MaxRotationUp   = pPlayerPed->m_nTurretPosnMode; // m_fAttachVerticalLimit
            MaxRotationDown = pPlayerPed->m_nTurretPosnMode;
        } else {
            targetPed->GetTransformedBonePosition(posn, BONE_HEAD, true);
        }

        m_vecSource = posn;
        m_vecSource += HeightToNose * targetPed->GetMatrix().GetUp();
        m_vecSource -= DistBack * targetPed->GetMatrix().GetForward();

        float fSoftMoveRate   = M16_1STPERSON_SOFTLIMIT_MULT * CTimer::GetTimeStep();
        float fSoftLimitAngle = pPlayerPed->m_fTurretAngleB * M16_1STPERSON_SOFTLIMIT_ANG; // m_fAttachHeadingLimit
        float fSoftLimitRange = pPlayerPed->m_fTurretAngleB * (1.0f - M16_1STPERSON_SOFTLIMIT_ANG);

        if (fCentreBeta - m_fHorizontalAngle > PI) {
            fCentreBeta -= TWO_PI;
        } else if (fCentreBeta - m_fHorizontalAngle < -PI) {
            fCentreBeta += TWO_PI;
        }

        auto* pHeli = (pPlayerPed->m_pAttachedTo->GetIsTypeVehicle() && pPlayerPed->m_pAttachedTo->AsVehicle()->IsHeli())
            ? reinterpret_cast<CHeli*>(pPlayerPed->m_pAttachedTo)
            : nullptr;

        float fTargetDiff = fCentreBeta - m_fHorizontalAngle;
        if (fTargetDiff > fSoftLimitAngle) {
            fTargetDiff -= fSoftLimitAngle;
            fSoftMoveRate *= std::abs(fTargetDiff);
            if (std::abs(fTargetDiff) > fSoftLimitRange + fSoftMoveRate) {
                fSoftMoveRate = std::abs(fTargetDiff) - fSoftLimitRange;
            }

            if (pHeli && pHeli->m_fYawControl > 0.0f) {
                pHeli->m_fYawControl -= fTargetDiff * M16_1STPERSON_ROTATETHEHELI_SPEED * CTimer::GetTimeStep();
            }
        } else if (fTargetDiff < -fSoftLimitAngle) {
            fTargetDiff += fSoftLimitAngle;
            fSoftMoveRate *= std::abs(fTargetDiff);
            if (std::abs(fTargetDiff) > fSoftLimitRange + fSoftMoveRate) {
                fSoftMoveRate = std::abs(fTargetDiff) - fSoftLimitRange;
            }

            if (pHeli && pHeli->m_fYawControl > 0.0f) {
                pHeli->m_fYawControl -= fTargetDiff * M16_1STPERSON_ROTATETHEHELI_SPEED * CTimer::GetTimeStep();
            }
        } else {
            fTargetDiff = 0.0f;
        }

        if (fTargetDiff != 0.0f) {
            if (fTargetDiff < 0.0f) {
                m_fHorizontalAngle -= fSoftMoveRate;
            } else {
                m_fHorizontalAngle += fSoftMoveRate;
            }
        }

        fSoftMoveRate   = M16_1STPERSON_SOFTLIMIT_MULT * CTimer::GetTimeStep();
        fSoftLimitAngle = MaxRotationUp * M16_1STPERSON_SOFTLIMIT_ANG;
        fSoftLimitRange = MaxRotationUp * (1.0f - M16_1STPERSON_SOFTLIMIT_ANG);

        if (m_fVerticalAngle > fCentreAlpha + fSoftLimitAngle) {
            fTargetDiff = fCentreAlpha - m_fVerticalAngle;
            if (fTargetDiff < -fSoftLimitAngle) {
                fTargetDiff += fSoftLimitAngle;
                fSoftMoveRate *= std::abs(fTargetDiff);
                if (std::abs(fTargetDiff) > fSoftLimitRange + fSoftMoveRate) {
                    fSoftMoveRate = std::abs(fTargetDiff) - fSoftLimitRange;
                }
                m_fVerticalAngle -= fSoftMoveRate;
            }
        }

        fSoftMoveRate   = M16_1STPERSON_SOFTLIMIT_MULT * CTimer::GetTimeStep();
        fSoftLimitAngle = MaxRotationDown * M16_1STPERSON_SOFTLIMIT_ANG;
        fSoftLimitRange = MaxRotationDown * (1.0f - M16_1STPERSON_SOFTLIMIT_ANG);

        if (m_fVerticalAngle < fCentreAlpha - fSoftLimitAngle) {
            fTargetDiff = fCentreAlpha - m_fVerticalAngle;
            if (fTargetDiff > fSoftLimitAngle) {
                fTargetDiff -= fSoftLimitAngle;
                fSoftMoveRate *= std::abs(fTargetDiff);
                if (std::abs(fTargetDiff) > fSoftLimitRange + fSoftMoveRate) {
                    fSoftMoveRate = std::abs(fTargetDiff) - fSoftLimitRange;
                }
                m_fVerticalAngle += fSoftMoveRate;
            }
        }
    } else {
        if (m_fVerticalAngle > MaxRotationUp) {
            m_fVerticalAngle = MaxRotationUp;
        } else if (m_fVerticalAngle < -MaxRotationDown) {
            m_fVerticalAngle = -MaxRotationDown;
        }

        targetPed->UpdateRwMatrix();
        targetPed->UpdateRwFrame();
        targetPed->UpdateRpHAnim();

        CVector posn{ 0.0f, 0.0f, 0.0f };
        targetPed->GetTransformedBonePosition(posn, BONE_HEAD, true);

        m_vecSource = posn;
        m_vecSource.z += HeightToNose;
        if (targetPed->bIsDucking) {
            m_vecSource.x -= fDuckingBackOffset * targetPed->GetMatrix().GetForward().x;
            m_vecSource.y -= fDuckingBackOffset * targetPed->GetMatrix().GetForward().y;
            m_vecSource.x -= fDuckingRightOffset * targetPed->GetMatrix().GetRight().x;
            m_vecSource.y -= fDuckingRightOffset * targetPed->GetMatrix().GetRight().y;
        } else {
            m_vecSource.x -= DistBack * targetPed->GetMatrix().GetForward().x;
            m_vecSource.y -= DistBack * targetPed->GetMatrix().GetForward().y;
        }
    }

    static float alphaLimHeliCam = 1.2f;
    m_fVerticalAngle = std::clamp(m_fVerticalAngle, -alphaLimHeliCam, alphaLimHeliCam);

    m_vecFront = CVector{
        -std::cos(m_fHorizontalAngle) * std::cos(m_fVerticalAngle),
        -std::sin(m_fHorizontalAngle) * std::cos(m_fVerticalAngle),
        std::sin(m_fVerticalAngle)
    };
    TargetCoors = m_vecSource + 3.0f * m_vecFront;
    m_vecSource += 0.4f * m_vecFront;

    if (m_bCollisionChecksOn) {
        const auto IsClearTowards = [this](float betaOffsetDeg) {
            const auto beta  = m_fHorizontalAngle + DegreesToRadians(betaOffsetDeg);
            const auto alpha = m_fVerticalAngle - DegreesToRadians(20.0f);
            const auto to    = m_vecSource + 3.0f * CVector{ std::cos(beta) * std::cos(alpha), std::sin(beta) * std::cos(alpha), std::sin(alpha) };
            return CWorld::GetIsLineOfSightClear(to, m_vecSource, true, true, false, true, false, true, true);
        };

        if (!CWorld::GetIsLineOfSightClear(TargetCoors, m_vecSource, true, true, false, true, false, true, true) || !IsClearTowards(35.0f) || !IsClearTowards(-35.0f)) {
            RwCameraSetNearClipPlane(Scene.m_pRwCamera, 0.05f);
            FailedTestTwelveFramesAgo = true;
        } else {
            FailedTestTwelveFramesAgo = false;
        }
    }

    if (!FailedTestTwelveFramesAgo && m_nMode == MODE_CAMERA) {
        const float fNearClipScale = 1.0f + fCameraNearClipMult * (15.0f - std::min(15.0f, m_fFOV));
        RwCameraSetNearClipPlane(Scene.m_pRwCamera, fNearClipScale * 0.9f);
    }

    m_vecSource -= 0.4f * m_vecFront;

    GetVectorsReadyForRW();
    const float CamDirection                               = std::atan2(-m_vecFront.x, m_vecFront.y);
    TheCamera.m_pTargetEntity->AsPed()->m_fCurrentRotation = CamDirection;
    TheCamera.m_pTargetEntity->AsPed()->m_fAimingRotation  = CamDirection;
}

// 0x511B50
void CCam::Process_Rocket(const CVector& target, float orientation, float speedVar, float speedVarWanted, bool isHeatSeeking) {
    static auto& dword_B6FFF8 = StaticRef<uint32>(0xB6FFF8);
    static auto& dword_B6FFFC = StaticRef<uint32>(0xB6FFFC);
    static auto& byte_B70000  = StaticRef<bool>(0xB70000);

    if (!m_pCamTargetEntity->GetIsTypePed()) {
        return;
    }

    auto* targetPed = m_pCamTargetEntity->AsPed();
    m_fFOV = 70.0f;
    if (m_bResetStatics) {
        if (!CCamera::m_bUseMouse3rdPerson || targetPed->m_pTargetedObject) {
            m_fVerticalAngle = 0.0f;
            m_fHorizontalAngle = targetPed->m_fCurrentRotation - DegreesToRadians(90.0f);
        }
        m_fInitialPlayerOrientation = m_fHorizontalAngle;
        m_bResetStatics             = 0;
        m_bCollisionChecksOn        = true;
        byte_B70000                 = 0;
        dword_B6FFFC                = 0;
        dword_B6FFF8                = 0;
    }
    m_pCamTargetEntity->UpdateRwMatrix();
    m_pCamTargetEntity->UpdateRwFrame();
    CVector headPosition{};
    targetPed->GetTransformedBonePosition(headPosition, eBoneTag::BONE_HEAD, true);
    m_vecSource = headPosition + CVector{0.0f, 0.0f, 0.1f};

    auto*      pad1   = CPad::GetPad(0);
    const auto fov    = m_fFOV / 80.0f;
    const auto amountMouseMoved = pad1->NewMouseControllerState.GetAmountMouseMoved();
    
    if (!amountMouseMoved.IsZero()) {
        m_fHorizontalAngle += -3.0f * amountMouseMoved.x * fov * CCamera::m_fMouseAccelHorzntl;
        m_fVerticalAngle += +4.0f * amountMouseMoved.y * fov * CCamera::m_fMouseAccelVertical;
    } else {
        const auto hv  = (float)-pad1->LookAroundLeftRight(targetPed);
        const auto vv  = (float)pad1->LookAroundUpDown(targetPed);

        m_fHorizontalAngle += sq(hv) / 10000.0f * fov / 17.5f * CTimer::GetTimeStep() * (hv < 0.0f ? -1.0f : 1.0f);
        m_fVerticalAngle   += sq(vv) / 22500.0f * fov / 14.0f * CTimer::GetTimeStep() * (vv < 0.0f ? -1.0f : 1.0f);
    }
    ClipBeta();
    ClipAlpha();

    m_vecFront.Set(
        -(std::cos(m_fHorizontalAngle) * std::cos(m_fVerticalAngle)),
        -(std::sin(m_fHorizontalAngle) * std::cos(m_fVerticalAngle)),
        std::sin(m_fVerticalAngle)
    );
    GetVectorsReadyForRW();

    const auto heading = CGeneral::GetATanOfXY(m_vecFront.x, m_vecFront.y) - DegreesToRadians(90.0f);
    TheCamera.m_pTargetEntity->AsPed()->m_fCurrentRotation = heading;
    TheCamera.m_pTargetEntity->AsPed()->m_fAimingRotation  = heading;

    if (isHeatSeeking) {
        auto* player     = FindPlayerPed();
        auto* playerData = player->GetPlayerData();
        if (!playerData->m_nFireHSMissilePressedTime) {
            playerData->m_nFireHSMissilePressedTime = CTimer::GetTimeInMS();
        }

        const auto hsTarget = CWeapon::PickTargetForHeatSeekingMissile(
            m_vecSource,
            m_vecFront,
            1.2f,
            player,
            false,
            playerData->m_LastHSMissileTarget
        );

        // NOTE: not sure about the second one
        if (hsTarget && CTimer::GetTimeInMS() - playerData->m_nLastHSMissileLOSTime > 1'000) {
            playerData->m_nLastHSMissileLOSTime = CTimer::GetTimeInMS();

            const auto targetUsesCollision = hsTarget->GetUsesCollision();
            const auto playerUsesCollision = player->GetUsesCollision();
            hsTarget->SetUsesCollision(false);
            player->SetUsesCollision(false);

            const auto isClear = CWorld::GetIsLineOfSightClear(
                player->GetPosition(),
                hsTarget->GetPosition(),
                true,
                true,
                false,
                true,
                false,
                true
            );
            player->SetUsesCollision(playerUsesCollision);
            hsTarget->SetUsesCollision(targetUsesCollision);
            playerData->m_bLastHSMissileLOS = isClear;
        }

        if (!playerData->m_bLastHSMissileLOS || !hsTarget || hsTarget != playerData->m_LastHSMissileTarget) {
            playerData->m_nFireHSMissilePressedTime = CTimer::GetTimeInMS();
        }

        if (hsTarget) {
            CWeaponEffects::MarkTarget(
                CrossHairId(0),
                hsTarget->GetPosition(),
                255,
                255,
                255,
                100,
                1.3f,
                true
            );
        }

        auto& crosshair = gCrossHair[CrossHairId(0)];
        const auto time = CTimer::GetTimeInMS() - playerData->m_nFireHSMissilePressedTime;

        crosshair.m_nTimeWhenToDeactivate = 0;
        crosshair.m_color.Set(
            255,
            time <= 1'500 ? 255 : 0,
            time <= 1'500 ? 255 : 0
        );
        crosshair.m_fRotation = time <= 1'500 ? 0.0f : 1.0f;
        playerData->m_LastHSMissileTarget = hsTarget;
    }

    constexpr auto ROCKET_CAM_NEARCLIP_PLANE = 0.15f; // 0x8CCC9C
    RwCameraSetNearClipPlane(Scene.m_pRwCamera, ROCKET_CAM_NEARCLIP_PLANE);
}

// 0x517500
void CCam::Process_SpecialFixedForSyphon(const CVector& target, float orientation, float speedVar, float speedVarWanted) {
    m_vecSource = m_vecCamFixedModeSource;

    m_vecTargetCoorsForFudgeInter    = target;
    m_vecTargetCoorsForFudgeInter.z += m_fSyphonModeTargetZOffSet;

    m_vecFront = target - m_vecSource;

    const auto sourceBeforeChange = m_vecCamFixedModeSource;
    TheCamera.AvoidTheGeometry(sourceBeforeChange, m_vecTargetCoorsForFudgeInter, m_vecSource, m_fFOV);

    m_vecFront.z += m_fSyphonModeTargetZOffSet;
    GetVectorsReadyForRW();

    m_vecUp += m_vecCamFixedModeUpOffSet;
    m_vecUp.Normalise();

    m_vecFront = m_vecUp.Cross(m_vecFront).Normalized().Cross(m_vecUp);
    m_vecFront.Normalise();

    m_fFOV = 70.0f;

    // Keep the player facing whatever they're locked on to, so the camera stays in sync
    if (!m_pCamTargetEntity || !m_pCamTargetEntity->GetIsTypePed()) {
        return;
    }
    auto* const ped = m_pCamTargetEntity->AsPed();
    if (!ped->m_pTargetedObject) {
        return;
    }

    const auto* const wi = CWeaponInfo::GetWeaponInfo(ped->GetActiveWeapon().m_Type, ped->GetWeaponSkill());
    if (!wi) {
        return;
    }
    if (wi->flags.bAimWithArm && !ped->bIsDucking) {
        return;
    }
    if (wi->m_nWeaponFire == eWeaponFire::WEAPON_FIRE_MELEE) {
        return;
    }

    const auto delta = ped->m_pTargetedObject->GetPosition() - ped->GetPosition();

    ped->m_fAimingRotation = ped->m_fCurrentRotation = std::atan2(-delta.x, delta.y);
    ped->SetHeading(ped->m_fCurrentRotation);
    ped->UpdateRwMatrix();
}

// 0x512110
bool CCam::Process_WheelCam(const CVector& target, float orientation, float speedVar, float speedVarWanted) {
    constexpr auto WHEELCAM_CAR_X_OFFSET  = 0.33f; // 0x8CC7DC
    constexpr auto WHEELCAM_BIKE_X_OFFSET = 0.2f;  // 0x8CC7E0

    constexpr auto WHEELCAM_BOAT_OFFSET     = CVector{ -0.5f, -0.8f, 0.3f }; // 0x8CCC60
    constexpr auto WHEELCAM_BOAT_OFFSET_ALT = CVector{ 0.2f, -0.2f, -0.3f }; // 0x8CCC6C
    constexpr auto BOAT_ALT_FWD_SIGN        = -1.0f; // 0x8CCCA4 - `gThisCameraSucks1`
    constexpr auto BOAT_ALT_Z_SIGN          = 1.0f;  // 0x8CCCA0 - `gThisCameraSucks2`

    constexpr auto BOAT_UNDERWATER_CAM_COLORMAG_LIMIT = 10.0f; // 0x8CC7A8
    constexpr auto BOAT_UNDERWATER_CAM_BLUR           = 20;

    m_fFOV = 70.0f;

    CVector colBoxOffset{};

    if (m_pCamTargetEntity->GetIsTypePed()) { // No car - follow the feet
        m_vecSource = m_pCamTargetEntity->GetMatrix().TransformVector(CVector{ -0.3f, -0.5f, 0.1f })
                    + m_pCamTargetEntity->GetPosition();
        m_vecFront  = CVector{ 1.0f, 0.0f, 0.0f };
    } else {
        colBoxOffset    = m_pCamTargetEntity->GetColModel()->GetBoundingBox().m_vecMin;
        colBoxOffset.x -= WHEELCAM_CAR_X_OFFSET;
        colBoxOffset.y  = -2.3f;
        colBoxOffset.z  = 0.3f;

        m_vecSource = m_pCamTargetEntity->GetMatrix().TransformPoint(colBoxOffset);
        m_vecFront  = m_pCamTargetEntity->GetForwardVector();
    }

    CVector tempRight{}, tempUp{};

    //! Level the camera against world up rather than the vehicle's
    const auto LevelAgainstWorldUp = [&] {
        tempUp    = CVector{ 0.0f, 0.0f, 1.0f };
        tempRight = CrossProduct(m_vecFront, tempUp).Normalized();
        tempUp    = CrossProduct(tempRight, m_vecFront).Normalized();
    };

    auto* const veh        = m_pCamTargetEntity->GetIsTypeVehicle() ? m_pCamTargetEntity->AsVehicle() : nullptr;
    const auto  appearance = veh ? veh->GetVehicleAppearance() : VEHICLE_APPEARANCE_NONE;

    if (veh && (appearance == VEHICLE_APPEARANCE_HELI || appearance == VEHICLE_APPEARANCE_PLANE)) {
        tempRight      = m_pCamTargetEntity->GetRightVector();
        tempUp         = m_pCamTargetEntity->GetUpVector();
        colBoxOffset.x = -1.55f;
        m_vecSource    = m_pCamTargetEntity->GetMatrix().TransformPoint(colBoxOffset);
    } else if (veh && veh->IsBoat()) {
        LevelAgainstWorldUp();

        if (auto* const driver = veh->m_pDriver) { // Ride on the driver's head
            CVector pos{};
            driver->GetBonePosition(&pos, BONE_HEAD, true);

            pos   += veh->m_vecMoveSpeed * CTimer::GetTimeStep();
            pos   += WHEELCAM_BOAT_OFFSET.x * tempRight;
            pos   += WHEELCAM_BOAT_OFFSET.y * veh->GetForwardVector();
            pos.z += WHEELCAM_BOAT_OFFSET.z;

            if (veh->m_nModelIndex == MODEL_PREDATOR) {
                pos   += WHEELCAM_BOAT_OFFSET_ALT.x * tempRight;
                pos   += WHEELCAM_BOAT_OFFSET_ALT.y * (veh->GetForwardVector() * BOAT_ALT_FWD_SIGN);
                pos.z += WHEELCAM_BOAT_OFFSET_ALT.z * BOAT_ALT_Z_SIGN;
            }

            m_vecSource = pos;
        } else {
            m_vecSource.z += 2.0f * WHEELCAM_BOAT_OFFSET.z;
        }
    } else if (veh && veh->IsBike()) {
        tempRight = m_pCamTargetEntity->GetRightVector();
        tempUp    = CVector{ 0.0f, 0.0f, 1.0f };

        m_vecFront = CrossProduct(m_vecUp, tempRight);
        m_vecFront.Normalise();

        colBoxOffset.x += WHEELCAM_CAR_X_OFFSET - WHEELCAM_BIKE_X_OFFSET;

        m_vecSource  = m_pCamTargetEntity->GetPosition();
        m_vecSource += m_pCamTargetEntity->GetRightVector() * colBoxOffset.x;
        m_vecSource += m_vecFront * colBoxOffset.y;
        m_vecSource += m_vecUp * colBoxOffset.z;
    } else {
        // Trains reverse, so face the way they're actually going
        if (veh && veh->IsTrain() && DotProduct(veh->m_vecMoveSpeed, m_vecFront) < 0.0f) {
            m_vecFront = -m_vecFront;
        }
        LevelAgainstWorldUp();
    }

    // Under water - tint the screen with the water colour
    auto camWaterLevel = 0.0f;
    if (CWaterLevel::GetWaterLevel(m_vecSource.x, m_vecSource.y, m_vecSource.z, camWaterLevel, true, nullptr)
        && m_vecSource.z < camWaterLevel - 0.3f
    ) {
        const auto& colours = CTimeCycle::m_CurrentColours;
        const auto  mag     = std::sqrt(sq(colours.m_fWaterRed) + sq(colours.m_fWaterGreen) + sq(colours.m_fWaterBlue));
        const auto  scale   = mag > BOAT_UNDERWATER_CAM_COLORMAG_LIMIT ? BOAT_UNDERWATER_CAM_COLORMAG_LIMIT / mag : 1.0f;

        TheCamera.SetMotionBlur(
            (uint8)(colours.m_fWaterRed * scale),
            (uint8)(colours.m_fWaterGreen * scale),
            (uint8)(colours.m_fWaterBlue * scale),
            BOAT_UNDERWATER_CAM_BLUR,
            eMotionBlurType::LIGHT_SCENE
        );
    }

    // Rotate the up and right vectors a bit so the camera tilts
    const auto angle = 0.4f * std::cos((float)(CTimer::GetTimeInMS() & 131071) * (TWO_PI / 131072.0f));
    m_vecUp = tempUp * std::cos(angle) + tempRight * std::sin(angle);

    // Bad matrices have been seen coming through here, so normalise to be safe
    m_vecUp.Normalise();
    m_vecFront.Normalise();

    CColPoint cp;
    CEntity*  hitEntity = nullptr;

    CWorld::pIgnoreEntity = m_pCamTargetEntity; // So we don't find the player
    const auto blocked = CWorld::ProcessLineOfSight(m_vecSource, m_pCamTargetEntity->GetPosition(), cp, hitEntity, true, false, false, true, false, false, true, false);
    CWorld::pIgnoreEntity = nullptr;

    return !blocked;
}

// based on 0x51847C - 0x5184EC
void CCam::ApplyUnderwaterMotionBlur() {
    static constexpr uint32 UNDERWATER_CAM_BLUR      = 20;    // 0x8CC7A4
    static constexpr float  UNDERWATER_CAM_MAG_LIMIT = 10.0f; // 0x8CC7A8

    const auto colorMag = std::sqrt(
        sq(CTimeCycle::GetWaterRed()) +
        sq(CTimeCycle::GetWaterGreen()) +
        sq(CTimeCycle::GetWaterBlue())
    );

    const auto factor = (colorMag <= UNDERWATER_CAM_MAG_LIMIT) ? 1.0f : UNDERWATER_CAM_MAG_LIMIT / colorMag;

    TheCamera.SetMotionBlur(
        static_cast<uint32>(factor * CTimeCycle::GetWaterRed()),
        static_cast<uint32>(factor * CTimeCycle::GetWaterGreen()),
        static_cast<uint32>(factor * CTimeCycle::GetWaterBlue()),
        UNDERWATER_CAM_BLUR,
        eMotionBlurType::LIGHT_SCENE
    );
}

// 0x509A30
bool IsLampPost(eModelID modelId) {
    using namespace ModelIndices;

    return notsa::contains<eModelID>(
        {
            MI_SINGLESTREETLIGHTS1,
            MI_SINGLESTREETLIGHTS2,
            MI_SINGLESTREETLIGHTS3,
            MI_BOLLARDLIGHT,
            MI_MLAMPPOST,
            MI_STREETLAMP1,
            MI_STREETLAMP2,
            MI_TELPOLE02,
            MI_TRAFFICLIGHTS_MIAMI,
            MI_TRAFFICLIGHTS_TWOVERTICAL,
            MI_TRAFFICLIGHTS_3,
            MI_TRAFFICLIGHTS_4,
            MI_TRAFFICLIGHTS_GAY,
            MI_TRAFFICLIGHTS_5,
        },
        modelId
    );
}
