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

//! Where `CCamera::CamControl` decided the look-behind camera should aim this frame
static inline auto& gTargetCoordsForLookingBehind = StaticRef<CVector>(0xB6F018);

//! How far above the ground the arrest cameras sit. `ARRESTDIST_ABOVE_GROUND` @ 0x8CC7F8
static constexpr auto ARRESTDIST_ABOVE_GROUND = 0.7f;

//! Index into `gCamColVars` used whenever a camera wants the "player outside, medium range" collision set
static constexpr auto CAM_COL_VARS_PLAYER_OUTSIDE_MED_RANGE = 5;

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
    float fUpLimit;          //!< Alpha angle limits
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
    RH_ScopedInstall(Process_1rstPersonPedOnPC, 0x50EB70, { .reversed = false });
    RH_ScopedInstall(Process_1stPerson, 0x517EA0);
    RH_ScopedInstall(Process_AimWeapon, 0x521500, { .reversed = false });
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
    RH_ScopedInstall(Process_FollowCar_SA, 0x5245B0, { .reversed = false });
    RH_ScopedInstall(Process_FollowPedWithMouse, 0x50F970, { .reversed = false });
    RH_ScopedInstall(Process_FollowPed_SA, 0x522D40, { .reversed = false });
    RH_ScopedInstall(Process_M16_1stPerson, 0x5105C0, { .reversed = false });
    RH_ScopedInstall(Process_Rocket, 0x511B50);
    RH_ScopedInstall(Process_SpecialFixedForSyphon, 0x517500);
    RH_ScopedInstall(Process_WheelCam, 0x512110);

    RH_ScopedGlobalInstall(WellBufferMe, 0x509AE0);
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

// inlined -- alpha = vertical angle
void CCam::ClipAlpha() {
    m_fVerticalAngle = std::clamp(
        m_fVerticalAngle,
        DegreesToRadians(-85.5f),
        DegreesToRadians(+60.0f)
    );
}

// 0x509C50 -- beta = horizontal angle
void CCam::ClipBeta() {
    if (m_fHorizontalAngle < DegreesToRadians(-180.0f)) {
        m_fHorizontalAngle += DegreesToRadians(360.0f);
    } else {
        m_fHorizontalAngle -= DegreesToRadians(360.0f);
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

    //! Beta right behind the target
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
    m_fFOV      = 100.0f; // Les wanted the FOV lerp removed

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
void CCam::Process_1rstPersonPedOnPC(const CVector& target, float orientation, float speedVar, float speedVarWanted) {
    static auto& v3d_8CCC54   = StaticRef<CVector>(0x8CCC54);
    static auto& byte_B6FFDC  = StaticRef<bool>(0xB6FFDC);
    static auto& v3d_B6FFC4   = StaticRef<CVector>(0xB6FFC4);
    static auto& v3d_B6FFD0   = StaticRef<CVector>(0xB6FFD0);

    if (m_nMode != MODE_SNIPER_RUNABOUT) {
        m_fFOV = 70.0f;
    }

    if (!m_pCamTargetEntity->GetRwObject()) {
        return;
    }

    if (!m_pCamTargetEntity->GetIsTypePed()) {
        m_bResetStatics = false;
        RwCameraSetNearClipPlane(Scene.m_pRwCamera, 0.05f);
        return;
    }

    const auto hier = GetAnimHierarchyFromSkinClump(m_pCamTargetEntity->GetRpClump());
    const auto aIdx = RpHAnimIDGetIndex(hier, ConvertPedNode2BoneTag(2)); // todo: enum
    auto&      aMat = RpHAnimHierarchyGetMatrixArray(hier)[aIdx];
    auto*      targetPed = m_pCamTargetEntity->AsPed();

    CVector pointIn = v3d_8CCC54;
    RwV3dTransformPoint(&pointIn, &pointIn, &aMat);
    RwV3d v3dZero{ 0.0f };
    RwMatrixScale(&aMat, &v3dZero, rwCOMBINEPRECONCAT);

    if (m_bResetStatics) {
        // unnecessary entity ped check
        m_fVerticalAngle = 0.0f;
        byte_B6FFDC      = false;
        v3d_B6FFD0.Reset();
        m_fHorizontalAngle            = targetPed->m_fCurrentRotation + DegreesToRadians(90.0f);
        m_bCollisionChecksOn          = true;
        m_fInitialPlayerOrientation   = m_fHorizontalAngle;
        m_vecBufferedPlayerBodyOffset = v3d_B6FFC4 = pointIn;
    }
    m_vecBufferedPlayerBodyOffset.y = pointIn.y;

    if (TheCamera.m_bHeadBob) {
        m_vecBufferedPlayerBodyOffset.x = lerp(
            pointIn.x,
            m_vecBufferedPlayerBodyOffset.x,
            TheCamera.m_fScriptPercentageInterToCatchUp
        );

        m_vecBufferedPlayerBodyOffset.z = lerp(
            pointIn.z,
            m_vecBufferedPlayerBodyOffset.z,
            TheCamera.m_fScriptPercentageInterToCatchUp
        );

        m_vecSource = targetPed->GetMatrix().TransformPoint(m_vecBufferedPlayerBodyOffset);
    } else {
        const auto targetFwd = targetPed->GetForward().Normalized();
        const auto mag       = (pointIn - v3d_B6FFC4).Magnitude2D();

        m_vecSource = targetFwd * mag * 1.23f + targetPed->GetPosition() + CVector{ 0.0f, 0.0f, 0.59f };
    }

    CVector spinePos{};
    targetPed->GetTransformedBonePosition(spinePos, BONE_SPINE1, true);

    // TODO: Put in a function name e.g. 'HandleFreeMouseControl'?
    auto*      pad1   = CPad::GetPad(0);
    const auto fov    = m_fFOV / 80.0f;
    const auto amountMouseMoved = pad1->NewMouseControllerState.GetAmountMouseMoved();

    if (!amountMouseMoved.IsZero()) {
        m_fHorizontalAngle += -3.0f * amountMouseMoved.x * fov * CCamera::m_fMouseAccelHorzntl;
        m_fVerticalAngle += +4.0f * amountMouseMoved.y * fov * CCamera::m_fMouseAccelVertical;
    } else {
        const auto hv = (float)-pad1->LookAroundLeftRight(targetPed);
        const auto vv = (float)pad1->LookAroundUpDown(targetPed);

        m_fHorizontalAngle += sq(hv) / 10000.0f * fov / 17.5f * CTimer::GetTimeStep() * (hv < 0.0f ? -1.0f : 1.0f);
        m_fVerticalAngle += sq(vv) / 22500.0f * fov / 14.0f * CTimer::GetTimeStep() * (vv < 0.0f ? -1.0f : 1.0f);
    }
    ClipBeta();
    ClipAlpha();

    if (const auto* a = targetPed->m_pAttachedTo; targetPed->IsPlayer() && a) {
        // enum?
        switch (targetPed->m_fTurretAngleA) {
        case 0u:
            m_fHorizontalAngle -= a->GetHeading() + DegreesToRadians(90.0f);
            break;
        case 1u:
            m_fHorizontalAngle -= a->GetHeading() + DegreesToRadians(180.0f);
            break;
        case 2u:
            m_fHorizontalAngle -= a->GetHeading() + DegreesToRadians(-90.0f);
            break;
        case 3u:
            m_fHorizontalAngle -= a->GetHeading();
            break;
        default:
            // NOTE(yukani): If this is fired, gimme a call. 0x50F0ED
            NOTSA_UNREACHABLE();
            break;
        }

        // ...
    }
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
void CCam::Process_AimWeapon(const CVector&, float, float, float) {
    NOTSA_UNREACHABLE();
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

    // Speed widens the FOV, and it eases back to 70 when the car slows down
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
    constexpr auto LEN_DIV             = 30.0f;  // 0x8CCD44 - distance the FOV lerp takes place over
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

    // Zoom in to the FOV we want rather than snapping to it
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
    float   zoomFOVStart;             //!< FOV lerp start for the first zoom
    float   zoomFOVEnd;               //!< FOV lerp end for the first zoom
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
    float   heliDistFOVZoomMore;      //!< Car-to-heli distance that starts the second FOV zoom
    float   heliDistFOVZoomMoreMax;   //!< Car-to-heli distance that ends the second FOV zoom
    float   lessFOV;                  //!< Amount of FOV zoom in the second lerp
    float   lenToCarToPushCameraOut;  //!< Min 2D distance, stops a 180 degree flip in camera roll
    float   radiusOfSphereAroundHeli; //!< Keeps the camera out of collision, allowing for the near clip
    float   zoomOutFOV;               //!< FOV we zoom out to when we lose the car
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
    constexpr auto LEN_DIV              = 100.0f; // 0x8CCD88 - distance the FOV lerp takes place over
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
    if (fovZoom) { // Zoom in to the FOV we want rather than snapping to it
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
void CCam::Process_FollowCar_SA(const CVector&, float, float, float, bool) {
    NOTSA_UNREACHABLE();
}

// 0x50F970
void CCam::Process_FollowPedWithMouse(const CVector&, float, float, float) {
    NOTSA_UNREACHABLE();
}

// 0x522D40
void CCam::Process_FollowPed_SA(const CVector&, float, float, float, bool) {
    NOTSA_UNREACHABLE();
}

// 0x5105C0
void CCam::Process_M16_1stPerson(const CVector&, float, float, float) {
    NOTSA_UNREACHABLE();
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

// 0x4D58A0
int32 ConvertPedNode2BoneTag(int32 simpleId) {
    const auto map = notsa::make_mapping<int32, int32>({
        { 1,  3 },
        { 2,  5 },
        { 3,  32},
        { 4,  22},
        { 5,  34},
        { 6,  24},
        { 7,  41},
        { 8,  51},
        { 9,  43},
        { 10, 53},
        { 11, 52},
        { 12, 42},
        { 13, 33},
        { 14, 23},
        { 15, 31},
        { 16, 21},
        { 17, 4 },
        { 18, 8 },
    });
    return notsa::find_value_or(map, simpleId, -1);
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
