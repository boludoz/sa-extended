#include "StdInc.h"

#include "Camera.h"

#include "TaskSimpleGangDriveBy.h"
#include "TaskSimpleHoldEntity.h"
#include "TaskSimpleDuck.h"
#include "TaskSimpleSwim.h"
#include "Hud.h"
#include "MBlur.h"
#include "Garages.h"

float& CCamera::m_f3rdPersonCHairMultY = *reinterpret_cast<float*>(0xB6EC10); ///< Where the player will be on the screen in relative coords when quick aiming
float& CCamera::m_f3rdPersonCHairMultX = *reinterpret_cast<float*>(0xB6EC14);
float& CCamera::m_fMouseAccelVertical = *reinterpret_cast<float*>(0xB6EC18);
float& CCamera::m_fMouseAccelHorzntl = *reinterpret_cast<float*>(0xB6EC1C);
bool& CCamera::m_bUseMouse3rdPerson = *reinterpret_cast<bool*>(0xB6EC2E);
bool& CCamera::bDidWeProcessAnyCinemaCam = *reinterpret_cast<bool*>(0xB6EC2D);

CCamera& TheCamera = *reinterpret_cast<CCamera*>(0xB6F028);
bool& gbModelViewer = *reinterpret_cast<bool*>(0xBA6728);
int8& gbCineyCamMessageDisplayed = *(int8*)0x8CC381; // 2
int32& gCameraDirection = *(int32*)0x8CC384;         // 3
eCamMode& gCameraMode = *(eCamMode*)0x8CC388;        // -1
bool gTopSphereCastTest = false; // 0x9655E5
//! Per-zoom camera distance, indexed by `CCamera::GetArrPosForVehicleType` {car, bike, heli, plane, boat}
static float ZOOM_ONE_DISTANCE[5]   = { -1.0f, -0.2f, -3.2f, 0.05f, -2.41f };
static float ZOOM_TWO_DISTANCE[5]   = {  1.0f,  1.4f,  0.65f, 1.9f,   6.49f };
static float ZOOM_THREE_DISTANCE[5] = {  6.0f,  6.0f, 15.9f, 15.9f,  15.0f  };

//! Beyond this swing the aim camera jump-cuts instead of rotating, in degrees
static float MAX_ANGLE_BEFORE_AIMWEAPON_JUMPCUT = 90.0f;
//! How long first person stays put with no input before it gives up, in ms
static uint32 MAX_TIME_IN_FIRST_PERSON_NO_INPUT = 2000;

//! `m_nWhoIsInControlOfTheCamera` values: 0 game, 1 script, 2 obbe cinematic
static constexpr int32 SCRIPT_CAM_CONTROL = 1;

//! `FrontEndMenuManager.m_ControlMethod`: mouse + keyboard
static constexpr auto STANDARD_CONTROLLER_SCREEN = (eController)0;

//! Mode we owe a revert to after the tunnel handling forced a change; -1 when there is none
static int32 CamModeToRestore = -1;
static bool  JustGoneIntoObbeCamera = false;
static float PedTargetCloseHeightOffset = 0.0f;

static bool bSwitchedToObbeCam      = false; // 0xB6EC34
static int8 gCinematicModeSwitchDir = 1;     // 0x8CC471
uint32& gLastTime2PlayerCameraWasOK = *(uint32*)0xB6EC24;    // 0
uint32& gLastTime2PlayerCameraCollided = *(uint32*)0xB6EC28; // 0
bool& gPlayerPedVisible = *(bool*)0x8CC380; // true
uint8& gCurCamColVars = *(uint8*)0x8CCB80;
float& gCurDistForCam = *(float*)0x8CCB84;
float*& gpCamColVars = *(float**)0xB6FE88;
float (&gCamColVars)[28][6] = *(float (*)[28][6])0x8CC8E0;
float& gPedClipDist = *reinterpret_cast<float*>(0xB6EC68);
float& gLastRadiusUsedInCollisionPreventionOfCamera = *reinterpret_cast<float*>(0xB6EC6C);


//! Longest gap between two spline knots that may be skipped over in a single frame
static constexpr auto MAX_CUTJUMP_TIME = 32.0f; // 0x8D0F80

CCam& CCamera::GetActiveCamera() {
    return TheCamera.m_aCams[TheCamera.m_nActiveCam];
}

/*!
 * Advance `marker` to the spline segment covering `timer`.
 *
 * A path is `[numKnots, ...knots]`, each knot `stride` floats wide and starting with its
 * timestamp in seconds. `marker` indexes the *end* knot of the current segment.
 * A segment longer than `MAX_CUTJUMP_TIME` is never stepped over in one call.
 */
static void AdvanceSplineMarker(const float* path, float timer, int32* marker, int32 stride, float lastKnotTime, float segDuration) {
    if (timer >= lastKnotTime) {
        return;
    }

    const auto numKnots = (uint32)path[0];
    const auto lastIdx  = stride * (int32)numKnots - (stride - 1);

    if ((uint32)(*marker - 1) / (uint32)stride > numKnots) { // The marker ran off the end
        *marker = lastIdx;
        return;
    }
    if (segDuration > MAX_CUTJUMP_TIME) {
        return;
    }

    *marker += stride;
    if ((uint32)(*marker - 1) / (uint32)stride > numKnots) {
        *marker = lastIdx;
    }
}

/*!
 * @addr 0x5B2090
 * @brief Sample a cutscene position spline. Each knot is 10 floats: time, point (3),
 *        in-tangent (3), out-tangent (3).
 */
void FindSplinePathPositionVector(float* outPos, const float* path, float timer, int32* marker) {
    constexpr auto STRIDE = 10;

    // Both are read before the marker moves; the original keeps using this segment duration
    // as the denominator below even after stepping forward
    const auto segDuration = (path[*marker] - path[*marker - STRIDE]) * 1000.0f;
    const auto lastKnotTime = path[STRIDE * (int32)path[0] - (STRIDE - 1)] * 1000.0f;

    AdvanceSplineMarker(path, timer, marker, STRIDE, lastKnotTime, segDuration);

    const auto i = *marker;
    const auto t = timer > lastKnotTime
        ? 1.0f
        : std::clamp((timer - path[i - STRIDE] * 1000.0f) / segDuration, 0.0f, 1.0f);

    const auto from    = CVector{ path[i - 9], path[i - 8], path[i - 7] };
    const auto to      = CVector{ path[i + 1], path[i + 2], path[i + 3] };
    const auto ctrlIn  = CVector{ path[i - 3], path[i - 2], path[i - 1] };
    const auto ctrlOut = CVector{ path[i + 4], path[i + 5], path[i + 6] };

    auto& out = *reinterpret_cast<CVector*>(outPos);
    if (ctrlIn.x == from.x && ctrlIn.y == from.y && ctrlIn.z == from.z) { // Degenerate - a straight line will do
        out = from + (to - from) * t;
    } else { // Cubic Bezier
        const auto u = 1.0f - t;
        out = u * u * u * from
            + 3.0f * (u * u * t * ctrlIn + u * t * t * ctrlOut)
            + t * t * t * to;
    }

    out += TheCamera.m_vecCutSceneOffset;
}

/*!
 * @addr 0x5B2330
 * @brief Sample a cutscene scalar (FOV, roll) spline. Each knot is 4 floats: time, value,
 *        in-tangent, out-tangent.
 */
void FindSplinePathPositionFloat(float* outValue, const float* path, float timer, int32* marker) {
    constexpr auto STRIDE = 4;

    const auto segDuration  = (path[*marker] - path[*marker - STRIDE]) * 1000.0f;
    const auto lastKnotTime = path[STRIDE * (int32)path[0] - (STRIDE - 1)] * 1000.0f;

    AdvanceSplineMarker(path, timer, marker, STRIDE, lastKnotTime, segDuration);

    const auto i = *marker;
    // Unlike the vector version, this one recomputes the segment duration after moving
    const auto t = timer > lastKnotTime
        ? 1.0f
        : std::clamp(
              (timer - path[i - STRIDE] * 1000.0f) / ((path[i] - path[i - STRIDE]) * 1000.0f),
              0.0f,
              1.0f
          );

    const auto from    = path[i - 3];
    const auto to      = path[i + 1];
    const auto ctrlIn  = path[i - 1];
    const auto ctrlOut = path[i + 2];

    if (ctrlIn == from) {
        *outValue = from + (to - from) * t;
    } else {
        const auto u = 1.0f - t;
        *outValue = u * u * u * from
                  + 3.0f * (u * u * t * ctrlIn + u * t * t * ctrlOut)
                  + t * t * t * to;
    }
}

void CCamera::InjectHooks() {
    RH_ScopedClass(CCamera);
    RH_ScopedCategoryGlobal();

    RH_ScopedInstall(GetArrPosForVehicleType, 0x50AF00);
    RH_ScopedInstall(GetPositionAlongSpline, 0x50AF80);
    RH_ScopedInstall(GetRoughDistanceToGround, 0x516B00);
    RH_ScopedInstall(InitialiseCameraForDebugMode, 0x50AF90);
    RH_ScopedInstall(IsItTimeForNewCamera, 0x51D770);
    RH_ScopedInstall(ProcessObbeCinemaCameraPed, 0x50B880);
    RH_ScopedInstall(ProcessObbeCinemaCameraCar, 0x5267C0);
    RH_ScopedInstall(ProcessObbeCinemaCameraTrain, 0x526950);
    RH_ScopedInstall(ProcessObbeCinemaCameraHeli, 0x526AE0);
    RH_ScopedInstall(ProcessObbeCinemaCameraPlane, 0x526C80);
    RH_ScopedInstall(ProcessObbeCinemaCameraBoat, 0x526E20);
    RH_ScopedInstall(ProcessWideScreenOn, 0x50B890);
    RH_ScopedInstall(RenderMotionBlur, 0x50B8F0);
    RH_ScopedInstall(SetCameraDirectlyBehindForFollowPed_CamOnAString, 0x50BD40);
    RH_ScopedInstall(SetCameraDirectlyInFrontForFollowPed_CamOnAString, 0x50BD70);
    RH_ScopedInstall(SetCamPositionForFixedMode, 0x50BEC0);
    RH_ScopedInstall(SetFadeColour, 0x50BF00);
    RH_ScopedInstall(SetMotionBlur, 0x50BF40);
    RH_ScopedInstall(SetMotionBlurAlpha, 0x50BF80);
    RH_ScopedInstall(SetNearClipScript, 0x50BF90);
    RH_ScopedInstall(SetNewPlayerWeaponMode, 0x50BFB0);
    RH_ScopedInstall(SetRwCamera, 0x50C100);
    RH_ScopedInstall(SetWideScreenOn, 0x50C140);
    RH_ScopedInstall(SetWideScreenOff, 0x50C150);
    RH_ScopedInstall(StartCooperativeCamMode, 0x50C260);
    RH_ScopedInstall(StopCooperativeCamMode, 0x50C270);
    RH_ScopedInstall(AllowShootingWith2PlayersInCar, 0x50C280);
    RH_ScopedInstall(StoreValuesDuringInterPol, 0x50C290);
    RH_ScopedInstall(ProcessScriptedCommands, 0x516AE0);
    RH_ScopedInstall(FinishCutscene, 0x514950);
    RH_ScopedInstall(LerpFOV, 0x50D280);
    RH_ScopedInstall(UpdateAimingCoors, 0x50CB10);
    RH_ScopedInstall(SetColVarsAimWeapon, 0x50CBF0);
    RH_ScopedInstall(ClearPlayerWeaponMode, 0x50AB10);
    RH_ScopedInstall(DontProcessObbeCinemaCamera, 0x50AB40);
    RH_ScopedInstall(Enable1rstPersonCamCntrlsScript, 0x50AC00);
    RH_ScopedInstall(FindCamFOV, 0x50AD20);
    RH_ScopedInstall(GetFading, 0x50ADE0);
    RH_ScopedInstall(GetFadingDirection, 0x50ADF0);
    RH_ScopedInstall(Get_Just_Switched_Status, 0x50AE10);
    RH_ScopedInstall(GetGameCamPosition, 0x50AE50);

    RH_ScopedInstall(Constructor, 0x51A450);
    RH_ScopedInstall(InitCameraVehicleTweaks, 0x50A3B0);
    RH_ScopedInstall(ApplyVehicleCameraTweaks, 0x50A480);
    RH_ScopedInstall(CamShake, 0x50A9F0);
    RH_ScopedInstall(GetScreenRect, 0x50AB50);
    RH_ScopedInstall(Enable1rstPersonWeaponsCamera, 0x50AC10);
    RH_ScopedInstall(Fade, 0x50AC20);
    RH_ScopedInstall(Find3rdPersonQuickAimPitch, 0x50AD40);
    RH_ScopedInstall(GetCutSceneFinishTime, 0x50AD90);
    RH_ScopedInstall(GetScreenFadeStatus, 0x50AE20);
    RH_ScopedInstall(GetLookingLRBFirstPerson, 0x50AE60);
    RH_ScopedInstall(GetLookDirection, 0x50AE90);
    RH_ScopedInstall(GetLookingForwardFirstPerson, 0x50AED0);
    RH_ScopedInstall(CopyCameraMatrixToRWCam, 0x50AFA0);
    RH_ScopedInstall(CalculateMirroredMatrix, 0x50B380);
    RH_ScopedInstall(DealWithMirrorBeforeConstructRenderList, 0x50B510);
    RH_ScopedInstall(ProcessFade, 0x50B5D0);
    RH_ScopedInstall(ProcessMusicFade, 0x50B6D0);
    RH_ScopedInstall(Restore, 0x50B930);
    RH_ScopedInstall(RestoreWithJumpCut, 0x50BAB0);
    RH_ScopedInstall(SetCamCutSceneOffSet, 0x50BD20);
    RH_ScopedInstall(SetCameraDirectlyBehindForFollowPed_ForAPed_CamOnAString, 0x50BDA0);
    RH_ScopedInstall(SetCameraDirectlyInFrontForFollowPed_ForAPed_CamOnAString, 0x50BE30);
    RH_ScopedInstall(Using1stPersonWeaponMode, 0x50BFF0);
    RH_ScopedInstall(SetParametersForScriptInterpolation, 0x50C030);
    RH_ScopedInstall(SetPercentAlongCutScene, 0x50C070);
    RH_ScopedInstall(SetZoomValueFollowPedScript, 0x50C160);
    RH_ScopedInstall(SetZoomValueCamStringScript, 0x50C1B0);
    RH_ScopedInstall(UpdateTargetEntity, 0x50C360);
    RH_ScopedInstall(TakeControl, 0x50C7C0);
    RH_ScopedInstall(TakeControlNoEntity, 0x50C8B0);
    RH_ScopedInstall(TakeControlAttachToEntity, 0x50C910);
    RH_ScopedInstall(UpdateSoundDistances, 0x515BD0);
    RH_ScopedOverloadedInstall(IsSphereVisible, "matrix", 0x420C40, bool(CCamera::*)(const CVector&, float, RwMatrix*));
    RH_ScopedInstall(TakeControlWithSpline, 0x50CAE0);
    RH_ScopedInstall(SetCamCollisionVarDataSet, 0x50CB60);
    RH_ScopedInstall(SetNearClipBasedOnPedCollision, 0x50CB90);
    RH_ScopedInstall(SetColVarsPed, 0x50CC50);
    RH_ScopedInstall(SetColVarsVehicle, 0x50CCA0);
    RH_ScopedInstall(StartTransitionWhenNotFinishedInter, 0x515BC0);
    RH_ScopedInstall(StartTransition, 0x515200);
    RH_ScopedInstall(CameraGenericModeSpecialCases, 0x50CD30);
    RH_ScopedInstall(CameraPedModeSpecialCases, 0x50CD80);
    RH_ScopedInstall(CameraPedAimModeSpecialCases, 0x50CDA0);
    RH_ScopedInstall(CameraVehicleModeSpecialCases, 0x50CDE0);
    RH_ScopedInstall(IsExtraEntityToIgnore, 0x50CE80);
    RH_ScopedInstall(ConsiderPedAsDucking, 0x50CEB0);
    RH_ScopedInstall(ResetDuckingSystem, 0x50CEF0);
    RH_ScopedInstall(HandleCameraMotionForDucking, 0x50CFA0);
    RH_ScopedInstall(HandleCameraMotionForDuckingDuringAim, 0x50D090);
    RH_ScopedInstall(VectorMoveLinear, 0x50D160);
    RH_ScopedInstall(VectorTrackLinear, 0x50D1D0);
    RH_ScopedInstall(AddShakeSimple, 0x50D240);
    RH_ScopedInstall(InitialiseScriptableComponents, 0x50D2D0);
    RH_ScopedInstall(DrawBordersForWideScreen, 0x514860);
    RH_ScopedInstall(Find3rdPersonCamTargetVector, 0x514970);
    RH_ScopedInstall(AvoidTheGeometry, 0x514030);
    RH_ScopedInstall(CalculateGroundHeight, 0x514B80);
    RH_ScopedInstall(CalculateFrustumPlanes, 0x514D60);
    RH_ScopedInstall(CalculateDerivedValues, 0x5150E0);
    RH_ScopedInstall(ImproveNearClip, 0x516B20);
    RH_ScopedInstall(SetCameraUpForMirror, 0x51A560);
    RH_ScopedInstall(RestoreCameraAfterMirror, 0x51A5A0);
    RH_ScopedInstall(ConeCastCollisionResolve, 0x51A5D0);
    RH_ScopedInstall(TryToStartNewCamMode, 0x51E560);
    RH_ScopedInstall(CameraColDetAndReact, 0x520190);
    RH_ScopedInstall(CamControl, 0x527FA0);
    RH_ScopedInstall(Process, 0x52B730);
    RH_ScopedInstall(DeleteCutSceneCamDataMemory, 0x5B24A0);
    RH_ScopedInstall(LoadPathSplines, 0x5B24D0);
    RH_ScopedInstall(Init, 0x5BC520);

    RH_ScopedOverloadedInstall(ProcessVectorTrackLinear, "0", 0x50D350, void(CCamera::*)(float));
    RH_ScopedOverloadedInstall(ProcessVectorTrackLinear, "1", 0x516440, void(CCamera::*)());
    RH_ScopedOverloadedInstall(ProcessVectorMoveLinear, "0", 0x50D430, void(CCamera::*)(float));
    RH_ScopedOverloadedInstall(ProcessVectorMoveLinear, "1", 0x5164A0, void(CCamera::*)());
    RH_ScopedOverloadedInstall(ProcessFOVLerp, "0", 0x50D510, void(CCamera::*)(float));
    RH_ScopedOverloadedInstall(ProcessFOVLerp, "1", 0x516500, void(CCamera::*)());
    RH_ScopedOverloadedInstall(ProcessShake, "0", 0x51A6F0, void(CCamera::*)());
    RH_ScopedOverloadedInstall(ProcessShake, "1", 0x516560, void(CCamera::*)(float));

    RH_ScopedGlobalInstall(CamShakeNoPos, 0x50A970);
    RH_ScopedGlobalInstall(CameraObscuredByWaterLevel, 0x50B830);
    RH_ScopedGlobalInstall(FindSplinePathPositionVector, 0x5B2090);
    RH_ScopedGlobalInstall(FindSplinePathPositionFloat, 0x5B2330);
}

CCamera* CCamera::Constructor() { this->CCamera::CCamera(); return this; }

// 0x51A450
CCamera::CCamera() : CPlaceable() {
    m_nShakeType = 1;
    m_bMusicFadedOut = false;
    m_matrix = reinterpret_cast<CMatrixLink*>(&m_mCameraMatrix);
    m_fDuckCamMotionFactor = 0.0f;
    m_fDuckAimCamMotionFactor = 0.0f;

    InitialiseScriptableComponents();
}

// 0x50A870
CCamera::~CCamera() {
    m_matrix = nullptr;
}

// 0x5BC520
void CCamera::Init() {
    m_matrix = reinterpret_cast<CMatrixLink*>(&m_mCameraMatrix);
    InitialiseScriptableComponents();
    
    for (auto& camera : m_aCams) {
        camera.Init();
    }

    {
        auto& cam = m_aCams[0];
        cam.m_nMode = MODE_FOLLOWPED;
        cam.m_fTargetCloseInDist = 2.0837801f - 1.85f;
        cam.m_fMinRealGroundDist = 1.85f;
        cam.m_fTargetZoomGroundOne = -0.55f;
        cam.m_fTargetZoomGroundTwo = 1.5f;
        cam.m_fTargetZoomGroundThree = 3.6f;
        cam.m_fTargetZoomOneZExtra = 0.06f;
        cam.m_fTargetZoomTwoZExtra = -0.1f;
        cam.m_fTargetZoomTwoInteriorZExtra = 0.0f;
        cam.m_fTargetZoomThreeZExtra = -0.07f;
        cam.m_fTargetZoomZCloseIn = 0.90040702f;
        cam.m_pCamTargetEntity = nullptr;
        cam.m_fCamBufferedHeight = 0.0f;
        cam.m_fCamBufferedHeightSpeed = 0.0f;
        cam.m_bCamLookingAtVector = false;
        cam.m_fPlayerVelocity = 0.0f;
    }

    {
        auto& cam = m_aCams[1];
        cam.m_nMode = MODE_FOLLOWPED;
        cam.m_pCamTargetEntity = nullptr;
        cam.m_fCamBufferedHeight = 0.0f;
        cam.m_fCamBufferedHeightSpeed = 0.0f;
        cam.m_bCamLookingAtVector = false;
        cam.m_fPlayerVelocity = 0.0f;
    }

    {
        auto& cam = m_aCams[2];
        cam.m_pCamTargetEntity = nullptr;
        cam.m_bCamLookingAtVector = false;
        cam.m_fPlayerVelocity = 0.0f;
    }

    ClearPlayerWeaponMode();

    m_pTargetEntity = FindPlayerEntity();
    CEntity::SafeRegisterRef(m_pTargetEntity);

    m_nCarZoom = 2;
    m_nPedZoom = 2;
    m_fCarZoomBase = m_fCarZoomTotal = m_fCarZoomSmoothed = 0.0f;
    m_fPedZoomBase = m_fPedZoomTotal = m_fPedZoomSmoothed = 0.0f;
    m_pToGarageWeAreIn = nullptr;
    m_bPlayerIsInGarage = false;
    m_bJustCameOutOfGarage = false;
    m_bGarageFixedCamPositionSet = false;
    m_bFirstPersonBeingUsed = false;
    m_bJustJumpedOutOf1stPersonBecauseOfTarget = false;

    if (!FrontEndMenuManager.m_bStartGameLoading) {
        CDraw::FadeValue = 0;
        m_fMouseAccelVertical = notsa::IsFixBugs() ? m_fMouseAccelHorzntl * 0.6f : 0.0015f;
    }
    
    SetMotionBlur(255, 255, 255, 0, eMotionBlurType::NONE);

    m_f3rdPersonCHairMultX = 0.53f;
    m_f3rdPersonCHairMultY = 0.4f;
    gPlayerPedVisible = 1;
    m_bResetOldMatrix = true;
}

// 0x50A3B0
void CCamera::InitCameraVehicleTweaks() {
    m_fCurrentTweakDistance   = 1.0f;
    m_fCurrentTweakAltitude   = 1.0f;
    m_fCurrentTweakAngle      = 0.0f;
    m_nCurrentTweakModelIndex = -1;

    if (!m_bCameraVehicleTweaksInitialized) {
        for (auto& camTweak : m_aCamTweak) {
            camTweak.ModelID = -1;
            camTweak.Dist   = 1.0f;
            camTweak.Alt   = 1.0f;
            camTweak.Angle      = 0.0f;
        }

        m_aCamTweak[0].ModelID = MODEL_RCGOBLIN;
        m_aCamTweak[0].Dist = 1.0f;
        m_aCamTweak[0].Alt = 1.0f;
        m_aCamTweak[0].Angle    = 0.178997f; // todo: magic number

        m_bCameraVehicleTweaksInitialized = true;
    }
}

// 0x50D2D0
void CCamera::InitialiseScriptableComponents() {
    m_fTrackLinearStartTime    = -1.0f;
    m_fTrackLinearEndTime      = -1.0f;
    m_fStartShakeTime          = -1.0f;
    m_fEndShakeTime            = -1.0f;
    m_fEndZoomTime             = -1.0f;
    m_fStartZoomTime           = -1.0f;
    m_fZoomInFactor            = +0.0f;
    m_fZoomOutFactor           = +0.0f;
    m_bTrackLinearWithEase     = true;
    m_nZoomMode                = 1;
    m_bMoveLinearWithEase      = true;
    m_fMoveLinearStartTime     = -1.0f;
    m_fMoveLinearEndTime       = -1.0f;
    m_bBlockZoom               = false;
    m_bCameraPersistPosition   = false;
    m_bCameraPersistTrack      = false;
    m_bVecTrackLinearProcessed = false;
    m_bVecMoveLinearProcessed  = false;
    m_bFOVLerpProcessed        = false;
}

// 0x50AF90
void CCamera::InitialiseCameraForDebugMode() {
}

// 0x50A480
void CCamera::ApplyVehicleCameraTweaks(CVehicle* vehicle) {
    if (vehicle->GetModelIndex() == m_nCurrentTweakModelIndex) {
        return;
    }

    InitCameraVehicleTweaks();
    for (auto& camTweak : m_aCamTweak) {
        if (camTweak.ModelID == vehicle->GetModelIndex()) {
            m_fCurrentTweakDistance = camTweak.Dist;
            m_fCurrentTweakAltitude = camTweak.Alt;
            m_fCurrentTweakAngle    = camTweak.Angle;
            return;
        }
    }
}

// 0x50A9F0
void CCamera::CamShake(float strength, CVector from) {
    auto dist = DistanceBetweenPoints(from, GetActiveCamera().m_vecSource);
    dist = std::clamp(dist, 0.0f, 100.0f);

    float percentShakeForce = 1.0f - dist / 100.f;
    float shakeForce = (m_fCamShakeForce - float(CTimer::GetTimeInMS() - m_nCamShakeStart) / 1000.f) * percentShakeForce;

    float toShakeForce = percentShakeForce * strength * 0.35f;
    if (toShakeForce > std::clamp(shakeForce, 0.0f, 2.0f)) {
        m_fCamShakeForce = toShakeForce;
        m_nCamShakeStart = CTimer::GetTimeInMS();
    }
}

// 0x50A970
void CamShakeNoPos(CCamera* camera, float strength) {
    float oldShake = camera->m_fCamShakeForce - float(CTimer::GetTimeInMS() - camera->m_nCamShakeStart) / 1000.f;

    if (strength > std::clamp(oldShake, 0.0f, 2.0f)) {
        camera->m_fCamShakeForce = strength;
        camera->m_nCamShakeStart = CTimer::GetTimeInMS();
    }
}

// 0x51D770
bool CCamera::IsItTimeForNewCamera(int32 camSequence, int32 startTime) {
    constexpr auto MAX_TIME_ON_ANY_CAM  = 20000.0f; // 0x8CCDF8
    constexpr auto MAX_TIME_ON_THIS_CAM = 15000.0f; // 0x8CCDF0

    //! Cleared once the stick has been pushed far enough, so one push only switches once
    static bool bStickReadyForNewCam = true; // 0x8CCDF4

    if (camSequence < 0) {
        return true;
    }

    const auto timeOnThisCam = (float)(CTimer::GetTimeInMS() - startTime);
    if (timeOnThisCam > MAX_TIME_ON_ANY_CAM) {
        return true;
    }

    // Pushing the right stick far enough switches camera, and picks which way we cycle
    const auto stickX = (float)CPad::GetPad(0)->NewState.RightStickX;
    if (std::abs(stickX) <= 32.0f) {
        bStickReadyForNewCam = true;
    } else if (std::abs(stickX) > 96.0f && bStickReadyForNewCam) {
        gCinematicModeSwitchDir = stickX > 0.0f ? 1 : -1;
        bStickReadyForNewCam = false;
        return true;
    }

    //! Seaplane is classed as a boat, but the camera treats it like a car
    const auto PlayerInBoat = [&] {
        const auto* veh = FindPlayerVehicle();
        return veh && veh->m_nVehicleType == VEHICLE_TYPE_BOAT && m_pTargetEntity->m_nModelIndex != MODEL_SKIMMER;
    };
    const auto LineOfSightClear = [](const CVector& from, const CVector& to) {
        return CWorld::GetIsLineOfSightClear(from, to, true, false, false, false, false, false, false);
    };
    //! Flat offset from the fixed camera spot to the player
    const auto FlatDistToFixed = [&] {
        auto diff = FindPlayerCoors() - m_vecFixedModeSource;
        diff.z = 0.0f;
        return diff;
    };
    const auto MovingAwayFrom = [](const CVector& diff) {
        return DotProduct(FindPlayerSpeed(), diff) > 0.0f;
    };

    switch (camSequence) {
    case MOVIECAM0: {
        if (auto* const veh = FindPlayerVehicle()) {
            if (PlayerInBoat() || veh->m_nModelIndex == MODEL_RHINO) {
                return true;
            }
            if (!LineOfSightClear(veh->GetPosition(), m_aCams[m_nActiveCam].m_vecSource)) {
                return true;
            }
        }
        if (CTimer::GetTimeInMS() > (uint32)(startTime + 5000)) {
            return true;
        }
        RwCameraSetNearClipPlane(Scene.m_pRwCamera, 0.15f);
        return false;
    }
    case MOVIECAM1: {
        if (timeOnThisCam > MAX_TIME_ON_THIS_CAM || PlayerInBoat()) {
            return true;
        }
        if (!LineOfSightClear(FindPlayerCoors(), m_vecFixedModeSource)) {
            return true;
        }
        const auto diff = FlatDistToFixed();
        if (diff.Magnitude() > 40.0f && MovingAwayFrom(diff)) {
            return true;
        }
        return diff.Magnitude() < 4.5f;
    }
    case MOVIECAM2: {
        if (timeOnThisCam > MAX_TIME_ON_THIS_CAM || PlayerInBoat()) {
            return true;
        }
        if (!LineOfSightClear(FindPlayerCoors(), m_vecFixedModeSource)) {
            return true;
        }
        const auto diff = FlatDistToFixed();
        if (diff.Magnitude() < 2.0f) { // Player is right on top of the camera, pull the near clip in
            m_fNearClipScript = std::max(0.05f, diff.Magnitude() * 0.5f);
            m_bUseNearClipScript = true;
        }
        if (diff.Magnitude() > 29.0f && MovingAwayFrom(diff)) {
            return true;
        }
        if (diff.Magnitude() < 2.0f) {
            return true;
        }
        RwCameraSetNearClipPlane(Scene.m_pRwCamera, 0.15f);
        return false;
    }
    case MOVIECAM3: {
        if (timeOnThisCam > MAX_TIME_ON_THIS_CAM) {
            return true;
        }
        if (!LineOfSightClear(FindPlayerCoors(), m_vecFixedModeSource)) {
            return true;
        }
        const auto diff = FlatDistToFixed();
        if (diff.Magnitude() > 48.0f && MovingAwayFrom(diff)) {
            return true;
        }
        RwCameraSetNearClipPlane(Scene.m_pRwCamera, 0.15f);
        return false;
    }
    case 4:
        return CTimer::GetTimeInMS() > (uint32)(startTime + 3000);
    case MOVIECAM5: {
        if (timeOnThisCam > MAX_TIME_ON_THIS_CAM || PlayerInBoat()) {
            return true;
        }
        if (!LineOfSightClear(FindPlayerCoors(), m_vecFixedModeSource)) {
            return true;
        }
        const auto diff = FlatDistToFixed();
        return diff.Magnitude() > 38.0f && MovingAwayFrom(diff);
    }
    case MOVIECAM6:
        return CTimer::GetTimeInMS() > (uint32)(startTime + 3000);
    case MOVIECAM7:
        if (CTimer::GetTimeInMS() <= (uint32)(startTime + 2000)) {
            return false;
        }
        return !m_aCams[m_nActiveCam].m_pCamTargetEntity->GetIsOnScreen();
    case MOVIECAM8: {
        if (timeOnThisCam > MAX_TIME_ON_THIS_CAM || PlayerInBoat()) {
            return true;
        }
        if (!LineOfSightClear(FindPlayerCoors(), m_aCams[m_nActiveCam].m_vecSource)
            || CTimer::GetTimeInMS() > (uint32)(startTime + 1000)
        ) {
            return true;
        }
        m_fNearClipScript = 0.6f;
        m_bUseNearClipScript = true;
        return false;
    }
    case MOVIECAM15: {
        if (timeOnThisCam > MAX_TIME_ON_THIS_CAM) {
            return true;
        }
        if (!FindPlayerVehicle()) {
            return false;
        }
        if (!LineOfSightClear(FindPlayerCoors(), m_vecFixedModeSource)) {
            return true;
        }
        const auto diff = FlatDistToFixed();
        if (diff.Magnitude() > 44.0f && MovingAwayFrom(diff)) {
            return true;
        }
        return diff.Magnitude() < 3.0f;
    }
    case MOVIECAM16: {
        if (timeOnThisCam > MAX_TIME_ON_THIS_CAM) {
            return true;
        }
        if (!FindPlayerVehicle()) {
            return false;
        }
        if (!LineOfSightClear(FindPlayerCoors(), m_vecFixedModeSource)) {
            return true;
        }
        const auto diff = FlatDistToFixed();
        if (diff.Magnitude() > 50.0f) {
            return true;
        }
        return diff.Magnitude() < 3.0f;
    }
    case MOVIECAM17: {
        if (timeOnThisCam > MAX_TIME_ON_THIS_CAM) {
            return true;
        }
        if (!FindPlayerVehicle()) {
            return false;
        }
        if (!LineOfSightClear(FindPlayerCoors(), m_vecFixedModeSource)) {
            return true;
        }
        const auto diff = FlatDistToFixed();
        if (diff.Magnitude() > 50.0f && MovingAwayFrom(diff)) {
            return true;
        }
        return diff.Magnitude() < 2.0f;
    }
    case MOVIECAM18: {
        if (timeOnThisCam > MAX_TIME_ON_THIS_CAM) {
            return true;
        }
        if (!LineOfSightClear(FindPlayerCoors(), m_vecFixedModeSource)) {
            return true;
        }
        const auto diff = FindPlayerCoors() - m_vecFixedModeSource; // Not flattened here
        if (diff.Magnitude() > 57.0f) {
            return true;
        }
        return diff.Magnitude() < 1.0f;
    }
    case MOVIECAM19: {
        if (timeOnThisCam > MAX_TIME_ON_THIS_CAM) {
            return true;
        }
        if (!LineOfSightClear(FindPlayerCoors(), m_vecFixedModeSource)) {
            return true;
        }
        const auto diff = FlatDistToFixed();
        if (diff.Magnitude() > 36.0f) {
            return true;
        }
        return diff.Magnitude() < 2.0f;
    }
    case MOVIECAM20: return !m_aCams[m_nActiveCam].Process_DW_HeliChaseCam(true);
    case MOVIECAM21: return !m_aCams[m_nActiveCam].Process_DW_CamManCam(true);
    case MOVIECAM22: return !m_aCams[m_nActiveCam].Process_DW_BirdyCam(true);
    case MOVIECAM23: return !m_aCams[m_nActiveCam].Process_DW_PlaneSpotterCam(true);
    case MOVIECAM24:
    case MOVIECAM25:
        TheCamera.m_bUseNearClipScript = false;
        return true;
    case MOVIECAMPLANE1: return !m_aCams[m_nActiveCam].Process_DW_PlaneCam1(true);
    case MOVIECAMPLANE2: return !m_aCams[m_nActiveCam].Process_DW_PlaneCam2(true);
    case MOVIECAMPLANE3: return !m_aCams[m_nActiveCam].Process_DW_PlaneCam3(true);
    case CAM_ON_A_STRING_LAST_RESORT:
        return CTimer::GetTimeInMS() > (uint32)(startTime + 5000);
    default:
        return false;
    }
}

// 0x50B830
bool CameraObscuredByWaterLevel() {
    const auto& source = TheCamera.m_aCams[TheCamera.m_nActiveCam].m_vecSource;

    float level;
    return CWaterLevel::GetWaterLevel(source.x, source.y, source.z, level, true, nullptr)
        && level >= source.z;
}

// 0x50AB10
void CCamera::ClearPlayerWeaponMode() {
    m_PlayerWeaponMode.m_nMode = 0;
    m_PlayerWeaponMode.m_nMaxZoom = 1;
    m_PlayerWeaponMode.m_nMinZoom = -1;
    m_PlayerWeaponMode.m_fDuration = 0.0f;
}

// 0x50AB40
void CCamera::DontProcessObbeCinemaCamera() {
    bDidWeProcessAnyCinemaCam = false;
}

// 0x50AC00
void CCamera::Enable1rstPersonCamCntrlsScript() {
    m_bEnable1rstPersonCamCntrlsScript = true;
}

// 0x50AC10
void CCamera::Enable1rstPersonWeaponsCamera() {
    m_bAllow1rstPersonWeaponsCamera = true;
}

// 0x50AC20
void CCamera::Fade(float duration, eFadeFlag direction) {
    m_fFadeDuration = duration;
    m_bFading = true;
    m_nFadeInOutFlag = direction;
    m_nFadeStartTime = CTimer::GetTimeInMS();

    if (m_bIgnoreFadingStuffForMusic && direction != eFadeFlag::FADE_OUT) {
        return;
    }
    m_bMusicFading           = true;
    m_nMusicFadingDirection  = direction;

    m_fTimeToFadeMusic       = std::min(std::max(duration * 0.3f, 0.3f), duration); //Can't use std::clamp there, duration can be bigger or smaller than 0.3f
    m_nFadeTimeStartedMusic  = CTimer::GetTimeInMS();
    m_fTimeToWaitToFadeMusic = direction == eFadeFlag::FADE_IN
        ? duration - m_fTimeToFadeMusic
        : 0.f;
    if (direction == eFadeFlag::FADE_IN) {
        m_fTimeToFadeMusic = std::max(m_fTimeToFadeMusic - 0.1f, 0.f);
    }
}

// 0x50AD20
float CCamera::FindCamFOV() const {
    return m_aCams[m_nActiveCam].m_fFOV;
}

/*!
* @addr 0x50AD40
* @return Rotation in radians at which the gun should point at, relative to the camera's vertical angle
*/
float CCamera::Find3rdPersonQuickAimPitch() const {
    const auto& cam = m_aCams[m_nActiveCam];

    // https://mathworld.wolfram.com/images/eps-svg/SOHCAHTOA_500.svg
    const auto adjacent = (0.5f - m_f3rdPersonCHairMultY) * 2.f;
    const auto opposite = std::tan(DegreesToRadians(cam.m_fFOV / 2.0f)) * adjacent;
    const auto relAngle = cam.m_fVerticalAngle - std::atan(opposite / CDraw::ms_fAspectRatio);
    return -relAngle; // Flip it
}

// 0x50AD90
uint32 CCamera::GetCutSceneFinishTime() {
    auto& cam = m_aCams[m_nActiveCam];
    if (cam.m_nMode == eCamMode::MODE_FLYBY) {
        return cam.m_nFinishTime;
    }

    cam = m_aCams[(m_nActiveCam + 1) % 2];
    if (cam.m_nMode == eCamMode::MODE_FLYBY) {
        return cam.m_nFinishTime;
    }

    return 0;
}

// 0x50ADE0
bool CCamera::GetFading() const {
    return m_bFading;
}

// TODO: eFadingDirection
// 0x50ADF0
int32 CCamera::GetFadingDirection() const {
    if (m_bFading)
        return m_nFadeInOutFlag == eFadeFlag::FADE_OUT;
    else
        return 2;
}

// 0x50AE10
bool CCamera::Get_Just_Switched_Status() const {
    return m_bJust_Switched;
}

// 0x50AE20
eNameState CCamera::GetScreenFadeStatus() const {
    if (m_fFadeAlpha == 0.0f) {
        return NAME_DONT_SHOW;
    }
    if (m_fFadeAlpha == 255.0f) {
        return NAME_FADE_IN;
    }

    return NAME_SHOW;
}

// 0x50AE50
CVector* CCamera::GetGameCamPosition() {
    return &m_vecGameCamPos;
}

// 0x50AE60
bool CCamera::GetLookingLRBFirstPerson() const {
    return m_aCams[m_nActiveCam].m_nMode == eCamMode::MODE_1STPERSON
        && m_aCams[m_nActiveCam].m_nDirectionWasLooking != LOOKING_FORWARD;
}

// 0x50AED0
bool CCamera::GetLookingForwardFirstPerson() const {
    return m_aCams[m_nActiveCam].m_nMode == eCamMode::MODE_1STPERSON
        && m_aCams[m_nActiveCam].m_nDirectionWasLooking == LOOKING_FORWARD;
}

// 0x50AE90
int32 CCamera::GetLookDirection() const {
    const auto& cam = m_aCams[m_nActiveCam];
    if (cam.m_nMode != eCamMode::MODE_CAM_ON_A_STRING &&
        cam.m_nMode != eCamMode::MODE_1STPERSON &&
        cam.m_nMode != eCamMode::MODE_BEHINDBOAT &&
        cam.m_nMode != eCamMode::MODE_FOLLOWPED ||
        (cam.m_nDirectionWasLooking == LOOKING_FORWARD)
    ) {
        return LOOKING_FORWARD;
    }

    return cam.m_nDirectionWasLooking; // todo: unsigned/signed
}

// 0x50AF00
bool CCamera::GetArrPosForVehicleType(eVehicleType type, int32& arrPos) {
    switch (type) {
    case VEHICLE_TYPE_MTRUCK:
        arrPos = 0;
        return true;
    case VEHICLE_TYPE_QUAD:
        arrPos = 1;
        return true;
    case VEHICLE_TYPE_HELI:
        arrPos = 2;
        return true;
    case VEHICLE_TYPE_PLANE:
        arrPos = 4;
        return true;
    case VEHICLE_TYPE_BOAT:
        arrPos = 3;
        return true;
    default:
        return false;
    }
}

// 0x50AF80
float CCamera::GetPositionAlongSpline() const {
    return m_fPositionAlongSpline;
}

// 0x516B00
float CCamera::GetRoughDistanceToGround() {
    return m_aCams[m_nActiveCam].m_vecSource.z - CalculateGroundHeight(eGroundHeightType::ENTITY_BB_BOTTOM);
}

// 0x514030
void CCamera::AvoidTheGeometry(const CVector& camPos, const CVector& targetPos, CVector& outCamPos, float fov) {
    constexpr auto fRangePlayerRadius  = 0.5f;  // 0x8CC38C
    constexpr auto fCloseNearClipLimit = 0.15f; // 0x8CC390
    constexpr auto fAvoidProbTimerDamp = 0.9f;  // 0x8CC81C
    constexpr auto fAvoidTweakFOV      = 1.15f; // 0x8CC820

    //! Set at startup; when clear, the second sight test below is skipped entirely
    static auto& bAvoidTest1 = StaticRef<bool>(0xB6EC65);

    //! Filled in by `CWorld::TestSphereAgainstWorld`
    static auto& gaTempSphereColPoints = StaticRef<std::array<CColPoint, 32>>(0xB9B250);

    // Buffered over frames, so they live outside the function in the original
    static auto& AvoidTheWallsFraction      = StaticRef<float>(0xB6EC38);
    static auto& AvoidTheWallsFractionSpeed = StaticRef<float>(0xB6EC3C);

    const auto playerToCam = targetPos - camPos;
    m_vecClearGeometryVec  = CVector{};

    const auto defaultDistFromPlayer = playerToCam.Magnitude();
    const auto groundDistance        = playerToCam.Magnitude2D();

    const auto beta = playerToCam.x == 0.0f && playerToCam.y == 0.0f
        ? CGeneral::GetATanOfXY(m_mCameraMatrix.GetForward().x, m_mCameraMatrix.GetForward().y) // Should never happen
        : CGeneral::GetATanOfXY(playerToCam.x, playerToCam.y);

    const auto alpha = groundDistance == 0.0f && playerToCam.z == 0.0f
        ? 0.0f
        : CGeneral::GetATanOfXY(groundDistance, playerToCam.z);

    auto front = CVector{ std::cos(beta) * std::cos(alpha), std::sin(beta) * std::cos(alpha), std::sin(alpha) };
    outCamPos  = targetPos - defaultDistFromPlayer * front;
    front.Normalise();

    CColPoint cp;
    CEntity*  hitEntity = nullptr;

    // Make sure `CWorld::pIgnoreEntity` is cleared again as soon as possible
    CWorld::pIgnoreEntity = m_pTargetEntity; // So we don't find the player
    if (CWorld::ProcessLineOfSight(targetPos, outCamPos, cp, hitEntity, true, false, false, true, false, false, true, false)) {
        const auto colPointTagToCam = cp.m_vecPoint;
        outCamPos = cp.m_vecPoint;

        // Check the other direction to see if anything would come through the near clip plane.
        // Peds and vehicles are enabled for this one.
        if (bAvoidTest1
            && CWorld::ProcessLineOfSight(outCamPos, targetPos, cp, hitEntity, false, true, true, true, false, false, true, false)
        ) {
            if ((outCamPos - cp.m_vecPoint).Magnitude() < RwCameraGetNearClipPlane(Scene.m_pRwCamera)) {
                outCamPos = cp.m_vecPoint;
            } else if ((outCamPos - colPointTagToCam).Magnitude() < RwCameraGetNearClipPlane(Scene.m_pRwCamera)) {
                outCamPos = colPointTagToCam; // Only fall back if we'd clip through it
            }
        }
    }
    CWorld::pIgnoreEntity = nullptr;

    // Pull the near clip in if the player is about to poke through it
    if (FindPlayerPed()) {
        const auto distWithCol = (targetPos - outCamPos).Magnitude() - fRangePlayerRadius;
        if (distWithCol < RwCameraGetNearClipPlane(Scene.m_pRwCamera)) {
            RwCameraSetNearClipPlane(Scene.m_pRwCamera, std::max(distWithCol, fCloseNearClipLimit));
        }
    }

    const auto nearClip      = RwCameraGetNearClipPlane(Scene.m_pRwCamera);
    const auto nearClipWidth = nearClip * std::tan(DegreesToRadians(fov) * 0.5f) * CDraw::ms_fAspectRatio * fAvoidTweakFOV;

    auto avoidTheWallTargetFraction = 0.0f;

    const auto centreOfCircle = outCamPos + nearClip * front;
    if (CWorld::TestSphereAgainstWorld(centreOfCircle, nearClipWidth, nullptr, true, false, false, true, false, true)) {
        const auto& colPos = gaTempSphereColPoints[0].m_vecPoint;

        auto       centreToCol = colPos - centreOfCircle;
        const auto depthForNoClip = DotProduct(colPos - outCamPos, front);

        if (depthForNoClip > fCloseNearClipLimit && depthForNoClip < 0.90f) {
            if (depthForNoClip < RwCameraGetNearClipPlane(Scene.m_pRwCamera)) {
                RwCameraSetNearClipPlane(Scene.m_pRwCamera, depthForNoClip);
            }
        } else if (depthForNoClip < fCloseNearClipLimit) {
            RwCameraSetNearClipPlane(Scene.m_pRwCamera, fCloseNearClipLimit);
        }

        // Now move the camera so nothing clips
        const auto distOuterSphereToColPoint = nearClipWidth - centreToCol.Magnitude();
        centreToCol.Normalise();

        auto colNormal = gaTempSphereColPoints[0].m_vecNormal;
        colNormal.Normalise();

        const auto colPointSphereOuter = distOuterSphereToColPoint * centreToCol;

        if (DotProduct(colNormal, -centreToCol) < 0.0f) { // Facing away from the camera
            colNormal = -colNormal;
        }

        m_vecClearGeometryVec       = colNormal * DotProduct(colNormal, -colPointSphereOuter);
        avoidTheWallTargetFraction  = 1.0f;

        if (m_pTargetEntity && m_pTargetEntity->GetIsTypePed() && RwCameraGetNearClipPlane(Scene.m_pRwCamera) < 2.0f * fCloseNearClipLimit) {
            // Is the wall directly behind or ahead of the player? (positive == behind)
            const auto dotFwd = DotProduct(colNormal, m_pTargetEntity->GetForwardVector());
            if (dotFwd < 0.0f) {
                m_fAvoidTheGeometryProbsTimer = std::max(0.0f, m_fAvoidTheGeometryProbsTimer) + CTimer::GetTimeStep();
            } else if (dotFwd > 0.5f) {
                m_fAvoidTheGeometryProbsTimer = std::min(0.0f, m_fAvoidTheGeometryProbsTimer) - CTimer::GetTimeStep();
            }

            // Which way to swing round; don't change our mind until the move has finished
            if (m_nAvoidTheGeometryProbsDirn == 0) {
                m_nAvoidTheGeometryProbsDirn = CrossProduct(m_pTargetEntity->GetPosition() - outCamPos, colNormal).z > 0.0f
                    ? (uint16)-1
                    : (uint16)1;
            }
        }
    }

    m_fAvoidTheGeometryProbsTimer *= std::pow(fAvoidProbTimerDamp, CTimer::GetTimeStep());

    WellBufferMe(avoidTheWallTargetFraction, AvoidTheWallsFraction, AvoidTheWallsFractionSpeed, 0.2f, 0.05f, false);
    m_vecClearGeometryVec = AvoidTheWallsFraction * m_vecClearGeometryVec;
    m_bMoveCamToAvoidGeom = true;
}

// 0x50AFA0
void CCamera::CopyCameraMatrixToRWCam(bool bDontStoreOldMatrix) {
    // Last frame's axes. Nothing outside this function touches them; the original keeps
    // them at 0xB6FF90 behind an init bitfield at 0xB6FFC0.
    constexpr CVector UNSET{ -99999.0f, -99999.0f, -99999.0f };
    static auto prevPos   = UNSET;
    static auto prevAt    = UNSET;
    static auto prevUp    = UNSET;
    static auto prevRight = UNSET;

    auto* const frame = RwCameraGetFrame(m_pRwCamera);
    auto* const rwMat = RwFrameGetMatrix(frame);

    if (!bDontStoreOldMatrix) {
        m_mCameraMatrixOld.UpdateMatrix(rwMat);
    }

    // CMatrix -> RwMatrix, with the layout conversion (RW's `at` is our forward, RW's `up` is our up)
    *RwMatrixGetPos(rwMat)   = m_mCameraMatrix.GetPosition();
    *RwMatrixGetAt(rwMat)    = m_mCameraMatrix.GetForward();
    *RwMatrixGetUp(rwMat)    = m_mCameraMatrix.GetUp();
    *RwMatrixGetRight(rwMat) = m_mCameraMatrix.GetRight();

    // Hold each axis at last frame's value while it barely moved, so the matrix doesn't
    // jitter. `UNSET` is far enough away that the first frame through never holds.
    // The original squares it at runtime and compares against the squared distance
    constexpr auto EPS = 1.0e-5f; // 0x8CCC7C for the position, 0x8CCC78 for the three axes

    if ((prevPos - *RwMatrixGetPos(rwMat)).SquaredMagnitude() < EPS * EPS) {
        *RwMatrixGetPos(rwMat) = prevPos;
    }
    if ((prevAt - *RwMatrixGetAt(rwMat)).SquaredMagnitude() < EPS * EPS) {
        *RwMatrixGetAt(rwMat) = prevAt;
    }
    if ((prevUp - *RwMatrixGetUp(rwMat)).SquaredMagnitude() < EPS * EPS) {
        *RwMatrixGetUp(rwMat) = prevUp;
    }
    if ((prevRight - *RwMatrixGetRight(rwMat)).SquaredMagnitude() < EPS * EPS) {
        *RwMatrixGetRight(rwMat) = prevRight;
    }

    prevPos   = *RwMatrixGetPos(rwMat);
    prevAt    = *RwMatrixGetAt(rwMat);
    prevUp    = *RwMatrixGetUp(rwMat);
    prevRight = *RwMatrixGetRight(rwMat);

    RwMatrixUpdate(rwMat);
    RwFrameUpdateObjects(frame);
    RwFrameOrthoNormalize(frame);

    if (m_bResetOldMatrix && !bDontStoreOldMatrix) {
        m_mCameraMatrixOld.UpdateMatrix(rwMat);
        m_bResetOldMatrix = false;
    }
}

// 0x50B380
void CCamera::CalculateMirroredMatrix(CVector posn, float mirrorV, CMatrix *camMatrix, CMatrix* mirrorMatrix) {
    mirrorMatrix->GetPosition() = camMatrix->GetPosition() - posn * 2 * (DotProduct(posn, camMatrix->GetPosition()) - mirrorV);

    const CVector fwd = camMatrix->GetForward() - posn * 2 * DotProduct(posn, camMatrix->GetForward());
    mirrorMatrix->GetForward() = fwd;

    const CVector up = camMatrix->GetUp() - posn * 2 * DotProduct(posn, camMatrix->GetUp());
    mirrorMatrix->GetUp() = up;

    mirrorMatrix->GetRight() = CVector{
        up.y * fwd.z - up.z * fwd.y,
        up.z * fwd.x - up.x * fwd.z,
        up.x * fwd.y - up.y * fwd.x
    };
}

// 0x50B510
void CCamera::DealWithMirrorBeforeConstructRenderList(bool bActiveMirror, CVector mirrorNormal, float mirrorV, CMatrix* matMirror) {
    m_bMirrorActive = bActiveMirror;

    if (!bActiveMirror)
        return;

    if (matMirror)
        m_mMatMirror = *matMirror;
    else
        CalculateMirroredMatrix(mirrorNormal, mirrorV, &m_mCameraMatrix, &m_mMatMirror);

    m_mMatMirrorInverse = Invert(m_mMatMirror);
}

/// III/VC leftover
// 0x50B8F0
void CCamera::RenderMotionBlur() const {
    ZoneScoped;

    if (m_nBlurType != eMotionBlurType::NONE) {
        // CMBlur::MotionBlurRender(); // todo: Add CMBlur::MotionBlurRender is NOP, 0x71D700
    }
}

// 0x50B930
void CCamera::Restore() {
    m_bLookingAtPlayer = true;
    m_bLookingAtVector = false;
    m_nTypeOfSwitch = eSwitchType::INTERPOLATION;
    m_bUseNearClipScript = false;
    m_nModeObbeCamIsInForCar = 30;
    m_fPositionAlongSpline = 0.0f;
    m_bStartingSpline = false;
    m_bScriptParametersSetForInterp = false;
    m_nWhoIsInControlOfTheCamera = 0;

    CVehicle* vehicle = FindPlayerVehicle();
    CPlayerPed* player = FindPlayerPed();

    if (vehicle) {
        m_nModeToGoTo = MODE_CAM_ON_A_STRING;
        CEntity::SafeCleanUpRef(m_pTargetEntity);
        m_pTargetEntity = vehicle;
    } else {
        m_nModeToGoTo = MODE_FOLLOWPED;
        CEntity::SafeCleanUpRef(m_pTargetEntity);
        m_pTargetEntity = player;
    }
    CEntity::SafeRegisterRef(m_pTargetEntity);

    switch (player->m_nPedState) {
    case PEDSTATE_ENTER_CAR:
    case PEDSTATE_CARJACK:
    case PEDSTATE_OPEN_DOOR:
        m_nModeToGoTo = MODE_CAM_ON_A_STRING;
        break;
    }

    if (player->m_nPedState == PEDSTATE_EXIT_CAR) {
        m_nModeToGoTo = MODE_FOLLOWPED;

        CEntity::SafeCleanUpRef(m_pTargetEntity);
        m_pTargetEntity = player;
        CEntity::SafeRegisterRef(m_pTargetEntity);
    }

    CEntity::ClearReference(m_pAttachedEntity);

    m_bEnable1rstPersonCamCntrlsScript = false;
    m_bAllow1rstPersonWeaponsCamera = false;
    m_bUseScriptZoomValuePed = false;
    m_bUseScriptZoomValueCar = false;
    m_fAvoidTheGeometryProbsTimer = 0.0f;
    m_bStartInterScript = true;
    m_bCameraJustRestored = true;
}

// 0x50BAB0
void CCamera::RestoreWithJumpCut() {
    m_bRestoreByJumpCut = true;
    m_bLookingAtPlayer = true;
    m_bLookingAtVector = false;
    m_nTypeOfSwitch = eSwitchType::JUMPCUT;
    m_nWhoIsInControlOfTheCamera = 0;
    m_fPositionAlongSpline = 0.0f;
    m_bStartingSpline = false;
    m_bUseNearClipScript = false;
    m_nModeObbeCamIsInForCar = 30;
    m_bScriptParametersSetForInterp = false;

    CVehicle* vehicle = FindPlayerVehicle();
    CPlayerPed* player = FindPlayerPed();

    if (vehicle) {
        m_nModeToGoTo = MODE_CAM_ON_A_STRING;
        CEntity::SafeCleanUpRef(m_pTargetEntity);
        m_pTargetEntity = vehicle;
    } else {
        m_nModeToGoTo = MODE_FOLLOWPED;
        CEntity::SafeCleanUpRef(m_pTargetEntity);
        m_pTargetEntity = player;
    }
    CEntity::SafeRegisterRef(m_pTargetEntity);

    switch (player->m_nPedState) {
    case PEDSTATE_ENTER_CAR:
    case PEDSTATE_CARJACK:
    case PEDSTATE_OPEN_DOOR:
        m_nModeToGoTo = MODE_CAM_ON_A_STRING;
        break;
    }

    if (player->m_nPedState == PEDSTATE_EXIT_CAR) {
        m_nModeToGoTo = MODE_FOLLOWPED;

        CEntity::SafeCleanUpRef(m_pTargetEntity);
        m_pTargetEntity = player;
        CEntity::SafeRegisterRef(m_pTargetEntity);
    }

    if (!m_bCooperativeCamMode) {
        m_bUseScriptZoomValuePed = false;
        m_bUseScriptZoomValueCar = false;
        return;
    }

    CPlayerPed* player0 = FindPlayerPed(0);
    CPlayerPed* player1 = FindPlayerPed(1);

    if (!player0) {
        m_bUseScriptZoomValuePed = false;
        m_bUseScriptZoomValueCar = false;
        return;
    }

    if (!player1) {
        m_bUseScriptZoomValuePed = false;
        m_bUseScriptZoomValueCar = false;
        return;
    }

    CEntity::SafeCleanUpRef(m_pTargetEntity);

    if (!player0->IsInVehicle() || !player1->IsInVehicle()) {
        m_nModeToGoTo = m_nModeForTwoPlayersNotBothInCar;
        m_pTargetEntity = player0;
        CEntity::SafeRegisterRef(m_pTargetEntity);

        m_bUseScriptZoomValuePed = false;
        m_bUseScriptZoomValueCar = false;
        return;
    }

    if (player0->m_pVehicle == player1->m_pVehicle) {
        if (m_bAllowShootingWith2PlayersInCar) {
            m_nModeToGoTo = m_nModeForTwoPlayersSameCarShootingAllowed;
        } else {
            m_nModeToGoTo = m_nModeForTwoPlayersSameCarShootingNotAllowed;
        }
    } else {
        m_nModeToGoTo = m_nModeForTwoPlayersSeparateCars;
    }

    m_pTargetEntity = player0->m_pVehicle;
    CEntity::SafeRegisterRef(m_pTargetEntity);

    m_bUseScriptZoomValuePed = false;
    m_bUseScriptZoomValueCar = false;
}

// 0x50BD20
void CCamera::SetCamCutSceneOffSet(const CVector& offset) {
    m_vecCutSceneOffset = offset;
}

// 0x50BD40
void CCamera::SetCameraDirectlyBehindForFollowPed_CamOnAString() {
    m_bCamDirectlyBehind = true;
    CPed* player = FindPlayerPed();
    if (player) {
        m_fPedOrientForBehindOrInFront = CGeneral::GetATanOfXY(player->GetForward().x, player->GetForward().y);
    }
}

// 0x50BD70
void CCamera::SetCameraDirectlyInFrontForFollowPed_CamOnAString() {
    m_bCamDirectlyInFront = true;
    CPed* player = FindPlayerPed();
    if (player != nullptr) {
        m_fPedOrientForBehindOrInFront = CGeneral::GetATanOfXY(player->GetForward().x, player->GetForward().y);
    }
}

// unused
// 0x50BDA0
void CCamera::SetCameraDirectlyBehindForFollowPed_ForAPed_CamOnAString(CPed* targetPed) {
    if (!targetPed) {
        return;
    }

    m_bCamDirectlyBehind = true;
    m_bLookingAtPlayer = false;

    TheCamera.m_pTargetEntity = targetPed;
    CEntity::ChangeEntityReference(GetActiveCamera().m_pCamTargetEntity, targetPed);
    m_fPedOrientForBehindOrInFront = targetPed->GetHeading();
}

// 0x50BE30
void CCamera::SetCameraDirectlyInFrontForFollowPed_ForAPed_CamOnAString(CPed* targetPed) {
    if (!targetPed) {
        return;
    }

    m_bLookingAtPlayer = false;
    m_pTargetEntity = targetPed;

    CCam& camera = GetActiveCamera();
    CEntity::SafeCleanUpRef(camera.m_pCamTargetEntity);

    camera.m_pCamTargetEntity = targetPed;
    camera.m_pCamTargetEntity->RegisterReference(camera.m_pCamTargetEntity);

    m_bCamDirectlyInFront = true;
    m_fPedOrientForBehindOrInFront = CGeneral::GetATanOfXY(targetPed->GetForward().x, targetPed->GetForward().y);
}

// 0x50BEC0
void CCamera::SetCamPositionForFixedMode(const CVector& fixedModeSource, const CVector& fixedModeUpOffset) {
    m_vecFixedModeSource = fixedModeSource;
    m_vecFixedModeUpOffSet = fixedModeUpOffset;
    m_bGarageFixedCamPositionSet = false;
}

// 0x50BF00
void CCamera::SetFadeColour(uint8 red, uint8 green, uint8 blue) {
    m_bFadeTargetIsSplashScreen = false;
    if (red == 2 && green == 2 && blue == 2) {
        m_bFadeTargetIsSplashScreen = true;
    }

    CDraw::FadeRed   = red;
    CDraw::FadeGreen = green;
    CDraw::FadeBlue  = blue;
}

// 0x50BF40
void CCamera::SetMotionBlur(uint8 red, uint8 green, uint8 blue, int32 value, eMotionBlurType blurType) {
    m_nBlurRed    = red;
    m_nBlurGreen  = green;
    m_nBlurBlue   = blue;
    m_nBlurType   = blurType;
    m_nMotionBlur = value;
}

// 0x50BF80
void CCamera::SetMotionBlurAlpha(int32 alpha) {
    m_nMotionBlurAddAlpha = alpha;
}

// 0x50BF90
void CCamera::SetNearClipScript(float nearClip) {
    m_fNearClipScript = nearClip;
    m_bUseNearClipScript = true;
}

// 0x50BFB0
void CCamera::SetNewPlayerWeaponMode(eCamMode mode, int16 maxZoom, int16 minZoom) {
    m_PlayerWeaponMode.m_nMode     = mode;
    m_PlayerWeaponMode.m_nMinZoom  = minZoom;
    m_PlayerWeaponMode.m_nMaxZoom  = maxZoom;
    m_PlayerWeaponMode.m_fDuration = 0.0f;
}

// 0x50BFF0
bool CCamera::Using1stPersonWeaponMode() const {
    switch (m_PlayerWeaponMode.m_nMode) {
    case MODE_SNIPER:
    case MODE_M16_1STPERSON:
    case MODE_ROCKETLAUNCHER:
    case MODE_ROCKETLAUNCHER_HS:
    case MODE_HELICANNON_1STPERSON:
    case MODE_CAMERA:
    case MODE_AIMWEAPON_ATTACHED:
        return true;
    default:
        return false;
    }
}

// 0x50C030
void CCamera::SetParametersForScriptInterpolation(float interpolationToStopMoving, float interpolationToCatchUp, uint32 timeForInterpolation) {
    m_nScriptTimeForInterpolation = timeForInterpolation;
    m_bScriptParametersSetForInterp = true;
    m_fScriptPercentageInterToStopMoving = interpolationToStopMoving / 100.0f;
    m_fScriptPercentageInterToCatchUp = interpolationToCatchUp / 100.0f;
}

// 0x50C070
void CCamera::SetPercentAlongCutScene(float percent) {
    auto& cam = m_aCams[m_nActiveCam];
    if (cam.m_nMode == eCamMode::MODE_FLYBY) {
        cam.m_fTimeElapsedFloat = (float)cam.m_nFinishTime * percent / 100.0f;
        return;
    }

    cam = m_aCams[(m_nActiveCam + 1) % 2];
    if (cam.m_nMode == eCamMode::MODE_FLYBY) {
        cam.m_fTimeElapsedFloat = (float)cam.m_nFinishTime * percent / 100.0f;
        return;
    }
}

// 0x50C100
void CCamera::SetRwCamera(RwCamera* camera) {
    m_pRwCamera = camera;
    m_mViewMatrix.Attach(&camera->viewMatrix, false);
}

// 0x50C140
void CCamera::SetWideScreenOn() {
    m_bWideScreenOn = true;
    m_bWantsToSwitchWidescreenOff = false;
}

// 0x50C150
void CCamera::SetWideScreenOff() {
    m_bWantsToSwitchWidescreenOff = m_bWideScreenOn;
}

// 0x50C160
void CCamera::SetZoomValueFollowPedScript(int16 zoomMode) {
    switch (zoomMode) {
    case 1:
        m_fPedZoomValueScript = 1.50f;
        break;
    case 2:
        m_fPedZoomValueScript = 2.90f;
        break;
    default:
        m_fPedZoomValueScript = 0.25f;
    }
    m_bUseScriptZoomValuePed = true;
}

// zoomMode : 0- ZOOM_ONE , 1- ZOOM_TWO , 2- ZOOM_THREE
// 0x50C1B0
void CCamera::SetZoomValueCamStringScript(int16 zoomMode) {
    auto entity = m_aCams[0].m_pCamTargetEntity;

    if (entity->GetStatus() == STATUS_SIMPLE) {
        int32 arrPos{};
        VERIFY(GetArrPosForVehicleType(static_cast<eVehicleType>(entity->AsVehicle()->GetVehicleAppearance()), arrPos));
        m_fCarZoomValueScript = [zoomMode]{
            switch (zoomMode) {
            case 0:
                return std::array{ -1.0f, -0.2f, -3.20f, 0.05f, -2.41f }; // 0x8CC3E0
            case 1:
                return std::array{ +1.0f, +1.4f, +0.65f, 1.90f, +6.49f }; // 0x8CC3F4
            case 2:
                return std::array{ +6.0f, +6.0f, +15.9f, 15.9f, +15.0f }; // 0x8CC408
            default:
                NOTSA_UNREACHABLE("Unexpected zoom mode: {}", zoomMode);
            }
        }()[arrPos];
    
        m_bUseScriptZoomValueCar = true;
    } else {
        SetZoomValueFollowPedScript(zoomMode);
    }
}

// 0x50C260
void CCamera::StartCooperativeCamMode() {
    m_bCooperativeCamMode = true;
    CGameLogic::n2PlayerPedInFocus = eFocusedPlayer::NONE;
}

// 0x50C270
void CCamera::StopCooperativeCamMode() {
    m_bCooperativeCamMode = false;
    CGameLogic::n2PlayerPedInFocus = eFocusedPlayer::NONE;
}

// 0x50C280
void CCamera::AllowShootingWith2PlayersInCar(bool bAllow) {
    m_bAllowShootingWith2PlayersInCar = bAllow;
}

// 0x50C290
void CCamera::StoreValuesDuringInterPol(CVector* sourceDuringInter, CVector* targetDuringInter, CVector* upDuringInter, float* FOVDuringInter) {
    m_vecSourceDuringInter = *sourceDuringInter;
    m_vecTargetDuringInter = *targetDuringInter;
    m_vecUpDuringInter     = *upDuringInter;
    m_fFOVDuringInter      = *FOVDuringInter;

    auto dist = *sourceDuringInter - m_vecTargetDuringInter;
    m_fBetaDuringInterPol = CGeneral::GetATanOfXY(dist.x, dist.y);

    float distOnGround = dist.Magnitude2D();
    m_fAlphaDuringInterPol = CGeneral::GetATanOfXY(distOnGround, dist.z);
}

// 0x50C360
void CCamera::UpdateTargetEntity() {
    bool inObbeCamCanGoIntoUpdateLoop = false;
    bool pedInMidWayGettingIntoCarState = false;

    m_bPlayerWasOnBike = false;
    if (m_pTargetEntity) {
        if (m_pTargetEntity->GetIsTypeVehicle()) {
            constexpr float USE_BIKE_TRANSITION_SPEED = 0.3f;
            if (m_pTargetEntity->AsVehicle()->m_vecMoveSpeed.SquaredMagnitude() > USE_BIKE_TRANSITION_SPEED) {
                m_bPlayerWasOnBike = true;
            }
        }
    }

    if (m_nWhoIsInControlOfTheCamera == 2) { // OBBE_CAM_CONTROL
        inObbeCamCanGoIntoUpdateLoop = true;
        if (m_nModeObbeCamIsInForCar == MOVIECAM8 || m_nModeObbeCamIsInForCar == MOVIECAM7) {
            if (FindPlayerPed()->m_nPedState != PEDSTATE_ARRESTED) {
                inObbeCamCanGoIntoUpdateLoop = false;
            }

            if (!FindPlayerVehicle()) {
                CEntity::ChangeEntityReference(m_pTargetEntity, FindPlayerPed());
            }
        }
    }

    if (((m_bLookingAtPlayer || inObbeCamCanGoIntoUpdateLoop) && !m_bTransitionState)
        || !m_pTargetEntity
        || m_bTargetJustBeenOnTrain)
    {
        CPlayerPed* player = FindPlayerPed();
        CVehicle* playerVehicle = FindPlayerVehicle();

        if (!playerVehicle
            || (!CGameLogic::IsCoopGameGoingOn()
                && player->GetTaskManager().GetSimplestActiveTaskAs<CTaskSimpleGangDriveBy>()))
        {
            CEntity::ChangeEntityReference(m_pTargetEntity, player);

            if (player->m_nPedState == PEDSTATE_ENTER_CAR
                || player->m_nPedState == PEDSTATE_CARJACK
                || player->m_nPedState == PEDSTATE_OPEN_DOOR)
            {
                pedInMidWayGettingIntoCarState = true;
            }

            if (!pedInMidWayGettingIntoCarState) {
                if (m_pTargetEntity != m_aCams[m_nActiveCam].m_pCamTargetEntity) {
                    CEntity::ChangeEntityReference(m_aCams[m_nActiveCam].m_pCamTargetEntity, m_pTargetEntity);
                }
            }
        } else {
            CEntity::ChangeEntityReference(m_pTargetEntity, playerVehicle);
        }

        bool bTargetCarIsLocked = true;
        if (player && player->m_pVehicle) {
            if (player->m_pVehicle->CanPedOpenLocks(player)) {
                bTargetCarIsLocked = false;
            }
        }

        if (player->m_nPedState == PEDSTATE_ENTER_CAR && !bTargetCarIsLocked) {
            if (!pedInMidWayGettingIntoCarState && m_nCarZoom != 0) {
                CEntity* newTarget = player->m_pVehicle ? static_cast<CEntity*>(player->m_pVehicle) : static_cast<CEntity*>(player);
                CEntity::ChangeEntityReference(m_pTargetEntity, newTarget);
            }
        }

        if ((player->m_nPedState == PEDSTATE_CARJACK || player->m_nPedState == PEDSTATE_OPEN_DOOR) && !bTargetCarIsLocked) {
            if (!pedInMidWayGettingIntoCarState && m_nCarZoom != 0) {
                CEntity::ChangeEntityReference(m_pTargetEntity, player->m_pVehicle);
            }
            if (!player->m_pVehicle) {
                CEntity::ChangeEntityReference(m_pTargetEntity, player);
            }
        }

        if (player->m_nPedState == PEDSTATE_EXIT_CAR) {
            CEntity::ChangeEntityReference(m_pTargetEntity, player);
        }

        if (player->m_nPedState == PEDSTATE_DRAGGED_FROM_CAR) {
            CEntity::ChangeEntityReference(m_pTargetEntity, player);
        }

        if (m_pTargetEntity && m_pTargetEntity->GetIsTypeVehicle()) {
            if (m_nCarZoom == 0 && player->m_nPedState == PEDSTATE_ARRESTED) {
                CEntity::ChangeEntityReference(m_pTargetEntity, player);
            }
        }
    }
}

// 0x50C7C0
void CCamera::TakeControl(CEntity* target, eCamMode modeToGoTo, eSwitchType switchType, int32 whoIsInControlOfTheCamera) {
    if (!m_bCinemaCamera) {
        if (whoIsInControlOfTheCamera == 2 && m_nWhoIsInControlOfTheCamera == 1) {
            return;
        }
    }
    m_nWhoIsInControlOfTheCamera = whoIsInControlOfTheCamera;

    const auto [newGoToMode, newTargetEntity] = [&, this]() -> std::tuple<eCamMode, CEntity*>{
        if (target) {
            return {
                [&, this] {
                    if (modeToGoTo == MODE_NONE) {
                        switch (target->GetType()) {
                        case ENTITY_TYPE_PED:
                            return MODE_FOLLOWPED;
                        case ENTITY_TYPE_VEHICLE:
                            return MODE_CAM_ON_A_STRING;
                        }
                    }
                    return modeToGoTo;
                }(),
                target
            };
        }

        return { modeToGoTo, FindPlayerEntity() };
    }();

    CEntity::ChangeEntityReference(m_pTargetEntity, newTargetEntity);
    m_nModeToGoTo = newGoToMode;

    m_nTypeOfSwitch    = switchType;
    m_bLookingAtPlayer = m_bLookingAtVector = false;
    m_bStartInterScript = true;
}

// 0x50C8B0
void CCamera::TakeControlNoEntity(const CVector& fixedModeVector, eSwitchType switchType, int32 whoIsInControlOfTheCamera) {
    if (whoIsInControlOfTheCamera == 2 && m_nWhoIsInControlOfTheCamera == 1)
        return;

    m_nWhoIsInControlOfTheCamera = whoIsInControlOfTheCamera;
    m_bLookingAtVector           = true;
    m_nModeToGoTo                = MODE_FIXED;
    m_bLookingAtPlayer           = false;
    m_vecFixedModeVector         = fixedModeVector;
    m_nTypeOfSwitch              = switchType;
    m_bStartInterScript          = true;
}

// 0x50C910
void CCamera::TakeControlAttachToEntity(CEntity* target, CEntity* attached, CVector* attachedCamOffset, CVector* attachedCamLookAt, float tilt, eSwitchType switchType, int32 whoIsInControlOfTheCamera) {
    // Obbe only gets the camera if the script doesn't already have it
    if (whoIsInControlOfTheCamera == 2 && m_nWhoIsInControlOfTheCamera == 1) {
        return;
    }
    m_nWhoIsInControlOfTheCamera = whoIsInControlOfTheCamera;

    if (!attached) { // Attach to the player if nothing was passed in
        attached = FindPlayerVehicle(-1, false)
            ? (CEntity*)FindPlayerVehicle(-1, false)
            : (CEntity*)CWorld::Players[CWorld::PlayerInFocus].m_pPed;
    }

    if (target) {
        if (m_pTargetEntity) {
            m_pTargetEntity->CleanUpOldReference(&m_pTargetEntity);
        }
        m_pTargetEntity = target;
        m_pTargetEntity->RegisterReference(&m_pTargetEntity);
        m_bLookingAtVector = false;
    } else {
        m_bLookingAtVector = true;
        m_vecAttachedCamLookAt = *attachedCamLookAt != *attachedCamOffset
            ? *attachedCamLookAt
            : CVector{};
    }

    m_vecAttachedCamOffset = *attachedCamOffset != CVector{}
        ? *attachedCamOffset
        : CVector{ 0.0f, 0.0f, 2.0f }; // Default

    m_fAttachedCamAngle = tilt;

    if (m_pAttachedEntity) {
        m_pAttachedEntity->CleanUpOldReference(&m_pAttachedEntity);
    }
    m_pAttachedEntity = attached;
    m_pAttachedEntity->RegisterReference(&m_pAttachedEntity);

    m_nModeToGoTo       = MODE_ATTACHCAM;
    m_nTypeOfSwitch     = switchType;
    m_bLookingAtPlayer  = false;
    m_bStartInterScript = true;
}

// 0x50CAE0
void CCamera::TakeControlWithSpline(eSwitchType switchType) {
    m_bLookingAtPlayer = false;
    m_bLookingAtVector = false;
    m_bCutsceneFinished = false;
    m_nModeToGoTo = MODE_FLYBY;
    m_nTypeOfSwitch = switchType;
    m_bStartInterScript = true;
}

// 0x50CB10
void CCamera::UpdateAimingCoors(const CVector& aimingTargetCoors) {
    m_vecAimingTargetCoors = aimingTargetCoors;
}

// 0x515BD0
void CCamera::UpdateSoundDistances() {
    //! How many frames the vertical probe result is spread over
    constexpr auto PROBE_PERIOD = 12u;

    const auto isAimingOnFoot = notsa::contains({
        MODE_1STPERSON, MODE_SNIPER, MODE_SNIPER_RUNABOUT, MODE_ROCKETLAUNCHER_RUNABOUT,
        MODE_ROCKETLAUNCHER_RUNABOUT_HS, MODE_M16_1STPERSON_RUNABOUT, MODE_FIGHT_CAM_RUNABOUT,
        MODE_1STPERSON_RUNABOUT, MODE_HELICANNON_1STPERSON, MODE_CAMERA, MODE_M16_1STPERSON,
        MODE_ROCKETLAUNCHER, MODE_ROCKETLAUNCHER_HS
    }, GetActiveCam().m_nMode) && m_pTargetEntity->GetIsTypePed();

    // Probe from a point out in front of the camera - much closer when aiming on foot
    const auto origin = GetPosition() + m_mCameraMatrix.GetForward() * (isAimingOnFoot ? 0.5f : 5.0f);

    const auto phase = CTimer::GetFrameCounter() % PROBE_PERIOD;
    if (phase == 0) {
        m_fSoundDistUpAsReadOld = m_fSoundDistUpAsRead;

        CColPoint cp;
        CEntity*  colEntity;
        m_fSoundDistUpAsRead = CWorld::ProcessVerticalLine(origin, origin.z + 20.0f, cp, colEntity, true, false, false, false, true, false, nullptr)
            ? cp.m_vecPoint.z - origin.z
            : 20.0f;
    }

    // Blend from the old reading to the new one. The original divides by 6 rather than
    // `PROBE_PERIOD`, so the second half of the cycle extrapolates past the new value.
    const auto t = (float)(phase + 1) / 6.0f;
    m_fSoundDistUp = (1.0f - t) * m_fSoundDistUpAsReadOld + t * m_fSoundDistUpAsRead;
}

// unused
// 0x50CB90
void CCamera::SetNearClipBasedOnPedCollision(float nearestDistSq) {
    const auto minNearClip = gpCamColVars[4];
    const auto nearClip    = minNearClip + std::sqrt(nearestDistSq) / gPedClipDist * 0.25f * (0.3f - minNearClip);

    RwCameraSetNearClipPlane(Scene.m_pRwCamera, std::max(nearClip, minNearClip));
}

// TODO: eAimingType
// 0x50CBF0
void CCamera::SetColVarsAimWeapon(int32 aimingType) {
    switch (aimingType) {
    case 0:
        CCamera::SetCamCollisionVarDataSet(0);
        break;
    case 1:
        CCamera::SetCamCollisionVarDataSet(1);
        break;
    case 2:
        CCamera::SetCamCollisionVarDataSet(2);
        break;
    case 3:
        CCamera::SetCamCollisionVarDataSet(3);
        break;
    default:
        return;
    }
}

// 0x50CC50
void CCamera::SetColVarsPed(ePedType pedType, int32 nCamPedZoom) {
    const int32 camColVars = [=] {
        switch (pedType) {
        case PED_TYPE_PLAYER1:
            return nCamPedZoom + 3;
        case PED_TYPE_PLAYER2:
            return nCamPedZoom + 6;
        default:
            return 0;
        }
    }();

    if (camColVars != gCurCamColVars) {
        gCurCamColVars = camColVars;
        gCurDistForCam = 1.0f;
        gpCamColVars = gCamColVars[camColVars];
    }
}

// 0x50CD30
void CCamera::CameraGenericModeSpecialCases(CPed* targetPed) {
    m_nExtraEntitiesCount = 0;

    if (!targetPed) {
        return;
    }

    // Ignore whatever the ped is carrying, in every camera mode
    auto* taskHold = static_cast<CTaskSimpleHoldEntity*>(targetPed->GetIntelligence()->GetTaskHold(false));
    if (!taskHold || !taskHold->m_pEntityToHold) {
        return;
    }

    m_pExtraEntity[m_nExtraEntitiesCount++] = taskHold->m_pEntityToHold;
}

// 0x50CD80
void CCamera::CameraPedModeSpecialCases() {
    CCollision::bCamCollideWithVehicles = true;
    CCollision::bCamCollideWithObjects  = true;
    CCollision::bCamCollideWithPeds     = true;
}

// 0x50CDA0
void CCamera::CameraPedAimModeSpecialCases(CPed* ped) {
    CameraPedModeSpecialCases();

    if (ped->IsInVehicle()) {
        m_pExtraEntity[m_nExtraEntitiesCount++] = ped->m_pVehicle;
    }
}

// 0x50CDE0
void CCamera::CameraVehicleModeSpecialCases(CVehicle* vehicle) {
    float speed = vehicle->m_vecMoveSpeed.Magnitude();

    const auto slow = speed <= 0.2f;
    CCollision::relVelCamCollisionVehiclesSqr = slow ? 0.1f : 1.0f;
    CCollision::bCamCollideWithVehicles = true;
    CCollision::bCamCollideWithPeds     = slow;
    CCollision::bCamCollideWithObjects  = slow;

    if (vehicle->m_pVehicleBeingTowed) {
        m_pExtraEntity[m_nExtraEntitiesCount++] = vehicle->m_pVehicleBeingTowed;
    }
}

// 0x50CE80
bool CCamera::IsExtraEntityToIgnore(CEntity* entity) {
    if (m_nExtraEntitiesCount <= 0) {
        return false;
    }
    return notsa::contains(m_pExtraEntity, entity);
}

// 0x420C40
bool CCamera::IsSphereVisible(const CVector& origin, float radius, RwMatrix* transformMatrix) {
    // In camera space `y` is depth, `x` is right and `z` is up
    auto pos = origin;
    RwV3dTransformPoints(&pos, &pos, 1, transformMatrix);

    return pos.y + radius >= CDraw::ms_fNearClipZ
        && pos.y - radius <= CDraw::ms_fFarClipZ
        && pos.x * m_avecFrustumNormals[0].x + pos.y * m_avecFrustumNormals[0].y <= radius
        && pos.x * m_avecFrustumNormals[1].x + pos.y * m_avecFrustumNormals[1].y <= radius
        && pos.z * m_avecFrustumNormals[2].z + pos.y * m_avecFrustumNormals[2].y <= radius
        && pos.z * m_avecFrustumNormals[3].z + pos.y * m_avecFrustumNormals[3].y <= radius;
}

// 0x420D40 - NOTE: Function has no hook
bool CCamera::IsSphereVisible(const CVector& origin, float radius) {
    return IsSphereVisible(origin, radius, (RwMatrix*)&m_mMatInverse)
        || (m_bMirrorActive && IsSphereVisible(origin, radius, (RwMatrix*)&m_mMatMirrorInverse));
}

// 0x50CEB0
bool CCamera::ConsiderPedAsDucking(CPed* ped) {
    auto task = ped->GetIntelligence()->GetTaskDuck(true);
    return task && ped->bIsDucking && !task->m_bIsAborting;
}

// 0x50CEF0
void CCamera::ResetDuckingSystem(CPed* ped) {
    m_fDuckCamMotionFactor    = 0.0f;
    m_fDuckAimCamMotionFactor = 0.0f;
    if (!ped)
        return;

    auto* task = ped->GetIntelligence()->GetTaskDuck(true);
    if (!task)
        return;

    if (!ped->bIsDucking || task->m_bIsAborting)
        return;

    float factor;
    if (ped->m_vecMoveSpeed.Magnitude() <= 0.000001f)
        factor = 0.3f - 1.0f;
    else
        factor = 0.3f - 0.5f;

    m_fDuckCamMotionFactor    = factor;
    m_fDuckAimCamMotionFactor = -0.35f;
}

// arg5 always used as false
// 0x50CFA0
void CCamera::HandleCameraMotionForDucking(CPed* ped, CVector* source, CVector* targPosn, bool bDontUpdateFactor) {
    auto factor = 0.0f;
    if (ConsiderPedAsDucking(ped)) {
        // The `0.3f` is a global at 0x8CCB94 / 0x8CCB98 that nothing ever writes
        factor = ped->m_vecMoveSpeed.SquaredMagnitude() <= 0.000001f
            ? 0.3f - 1.0f  // Standing still - duck all the way
            : 0.3f - 0.5f; // Duck-walking
    }

    if (!bDontUpdateFactor) {
        m_fDuckCamMotionFactor += (factor - m_fDuckCamMotionFactor) * (CTimer::ms_fTimeStep * 0.1f);
    }

    if (source) {
        source->z += m_fDuckCamMotionFactor;
    }
    if (targPosn) {
        targPosn->z += m_fDuckCamMotionFactor;
    }
}

// arg5 always used as false
// 0x50D090
void CCamera::HandleCameraMotionForDuckingDuringAim(CPed* ped, CVector* source, CVector* targPosn, bool bDontUpdateFactor) {
    // Unlike `HandleCameraMotionForDucking` the move speed plays no part here; the original
    // still computes it at 0x50D0D1 and throws the result away.
    const auto factor = ConsiderPedAsDucking(ped) ? -0.35f : 0.0f;

    if (!bDontUpdateFactor) {
        m_fDuckAimCamMotionFactor += (factor - m_fDuckAimCamMotionFactor) * (CTimer::ms_fTimeStep * 0.13f);
    }

    if (source) {
        source->z += m_fDuckAimCamMotionFactor;
    }
    if (targPosn) {
        targPosn->z += m_fDuckAimCamMotionFactor;
    }
}

// 0x50D160
void CCamera::VectorMoveLinear(CVector* to, CVector* from, float duration, bool bMoveLinearWithEase) {
    float now = (float)CTimer::m_snTimeInMilliseconds;
    m_fMoveLinearStartTime = now;
    m_fMoveLinearEndTime = now + duration;
    m_vecMoveLinearPosnStart = *from;
    m_vecMoveLinearPosnEnd = *to;
    m_bMoveLinearWithEase = bMoveLinearWithEase;
}

// 0x50D1D0
void CCamera::VectorTrackLinear(CVector* to, CVector* from, float duration, bool bEase) {
    float now = (float)CTimer::m_snTimeInMilliseconds;
    m_fTrackLinearStartTime = now;
    m_fTrackLinearEndTime = now + duration;
    // Yes, these are the other way round from `VectorMoveLinear` - see 0x50D1F5/0x50D217.
    // `ProcessVectorTrackLinear` lerps from `EndPoint` towards `StartPoint`, so the naming
    // is backwards, but this is what retail does.
    m_vecTrackLinearEndPoint   = *to;
    m_vecTrackLinearStartPoint = *from;
    m_bTrackLinearWithEase     = bEase;
}

// 0x516400
void CCamera::AddShake(float duration, float a2, float a3, float a4, float a5) {
    return AddShakeSimple(duration, 1, 1.0f);
}

// 0x50D240
void CCamera::AddShakeSimple(float durationMs, int32 type, float intensity) {
    m_fShakeIntensity = intensity;
    m_nShakeType = type;
    m_fStartShakeTime = static_cast<float>(CTimer::GetTimeInMS());
    m_fEndShakeTime = m_fStartShakeTime + durationMs;
}

// 0x50D280
void CCamera::LerpFOV(float zoomInFactor, float zoomOutFactor, float timeLimit, bool bEase) {
    m_fStartZoomTime = static_cast<float>(CTimer::GetTimeInMS());
    m_fEndZoomTime = static_cast<float>(CTimer::GetTimeInMS()) + timeLimit;

    m_nZoomMode = bEase; // TODO: Rename
    m_fZoomInFactor = zoomInFactor;
    m_fZoomOutFactor = zoomOutFactor;
}

// 0x50B5D0
void CCamera::ProcessFade() {
    ZoneScoped;

    if (!m_bFading) {
        return;
    }

    float fadeAlpha = 0.0f;

    if (m_nFadeInOutFlag == eFadeFlag::FADE_OUT) {
        m_fFadeDuration == 0.0f
            ? (m_fFadeAlpha += 0.0f)
            : (m_fFadeAlpha -= CTimer::GetTimeStepInSeconds() / m_fFadeDuration * 255.0f);

        if (m_fFadeAlpha > 0.0f) {
            CDraw::FadeValue = static_cast<uint8>(m_fFadeAlpha);
            return;
        }

        m_bFading = false;
    } else {
        if (m_nFadeInOutFlag == eFadeFlag::FADE_OUT) { // stupid, why not use a switch instead?
            CDraw::FadeValue = static_cast<uint8>(m_fFadeAlpha);
            return;
        }

        if (m_fFadeAlpha >= 255.0f) {
            m_bFading = false;
        }

        fadeAlpha = 255.0f;

        m_fFadeDuration == 0.0f
            ? (m_fFadeAlpha += 255.0f)
            : (m_fFadeAlpha += CTimer::GetTimeStepInSeconds() / m_fFadeDuration * 255.0f);

        if (m_fFadeAlpha < 255.0f) {
            CDraw::FadeValue = static_cast<uint8>(m_fFadeAlpha);
            return;
        }
    }

    m_fFadeAlpha = fadeAlpha;
    CDraw::FadeValue = static_cast<uint8>(m_fFadeAlpha);
}

// 0x50B6D0
void CCamera::ProcessMusicFade() {
    if (!m_bMusicFading)
        return;

    if (m_fTimeToWaitToFadeMusic <= 0.0f) {
        switch (m_nMusicFadingDirection) {
        case eFadeFlag::FADE_OUT: {
            m_fEffectsFaderScalingFactor = m_fTimeToFadeMusic > 0.0f
                ? CTimer::GetTimeStepInSeconds() / m_fTimeToFadeMusic + m_fEffectsFaderScalingFactor
                : 1.f;
            
            if (m_fEffectsFaderScalingFactor >= 1.0f) {
                m_bMusicFadedOut = false;
                m_bMusicFading = false;
                m_fEffectsFaderScalingFactor = 1.0f;
            }
            break;
        }
        case eFadeFlag::FADE_IN: {
            if (m_fEffectsFaderScalingFactor <= 0.0f) {
                m_bMusicFadedOut = true;
                m_bMusicFading = false;
                m_fEffectsFaderScalingFactor = 0.0f;
            }
            m_fEffectsFaderScalingFactor = m_fTimeToFadeMusic > 0.0f
                ? std::max(0.f, m_fEffectsFaderScalingFactor - CTimer::GetTimeStepInSeconds() / m_fTimeToFadeMusic)
                : 0.f;
            break;
        }
        }
    } else {
        m_fTimeToWaitToFadeMusic = m_fTimeToWaitToFadeMusic - CTimer::GetTimeStepInSeconds();
    }

    if (!AudioEngine.IsLoadingTuneActive()) {
        AudioEngine.SetMusicFaderScalingFactor(m_fEffectsFaderScalingFactor);
        AudioEngine.SetEffectsFaderScalingFactor(m_fEffectsFaderScalingFactor);
    }
}

// unused, empty
// 0x50B880
void CCamera::ProcessObbeCinemaCameraPed() {
    // NOP
}

// 0x526C80
void CCamera::ProcessObbeCinemaCameraPlane() {
    static int32 OldMode     = -1;   // 0x8CCEF8
    static int32 TimeForNext = 0;    // 0xB7012C
    // 0x8CC894. One extra entry that is 'safe' in case all the others fail
    static constexpr int32 SequenceOfPlaneCams[7]{ MOVIECAMPLANE1, MOVIECAMPLANE2, MOVIECAM20, MOVIECAM22, MOVIECAMPLANE3, MOVIECAM23, MOVIECAM0 };

    auto i = 0;
    if (!bDidWeProcessAnyCinemaCam) {
        OldMode = -1;
        if (gbCineyCamMessageDisplayed > 0 && !m_bCinemaCamera) {
            gbCineyCamMessageDisplayed--;
            CHud::SetHelpMessage(TheText.Get("CINCAM"), true, false, false);
        }
        bSwitchedToObbeCam = true;
    } else if (CameraObscuredByWaterLevel() || !IsItTimeForNewCamera(SequenceOfPlaneCams[OldMode], TimeForNext)) {
        m_nModeObbeCamIsInForCar = OldMode;
        bDidWeProcessAnyCinemaCam = true;
        return;
    }

    OldMode = (OldMode + gCinematicModeSwitchDir) % 6;
    if (OldMode < 0) {
        OldMode = 5;
    } else if (OldMode > 5) {
        OldMode = 0;
    }
    if (TryToStartNewCamMode(SequenceOfPlaneCams[OldMode])) {
        TimeForNext = CTimer::GetTimeInMS();
        m_nModeObbeCamIsInForCar = OldMode;
        bDidWeProcessAnyCinemaCam = true;
        return;
    }

    while (i <= 6) {
            OldMode = (OldMode + gCinematicModeSwitchDir) % 6;
            if (OldMode < 0) {
                OldMode = 5;
            } else if (OldMode > 5) {
                OldMode = 0;
            }
        ++i;
        if (TryToStartNewCamMode(SequenceOfPlaneCams[OldMode])) {
            if (i < 6) {
        TimeForNext = CTimer::GetTimeInMS();
            m_nModeObbeCamIsInForCar = OldMode;
            bDidWeProcessAnyCinemaCam = true;
            return;
            }
            break;
        }
    }

    // Everything failed, drop back to the on-a-string camera
    const auto alreadyOnString = m_aCams[m_nActiveCam].m_nMode == MODE_CAM_ON_A_STRING;
    OldMode = 6;
    if (!alreadyOnString) {
        TryToStartNewCamMode(CAM_ON_A_STRING_LAST_RESORT);
        TimeForNext = CTimer::GetTimeInMS();
    }

    m_nModeObbeCamIsInForCar = OldMode;
    bDidWeProcessAnyCinemaCam = true;
}


// 0x526950
void CCamera::ProcessObbeCinemaCameraTrain() {
    static int32 OldMode     = -1;   // 0x8CCEF0
    static int32 TimeForNext = 0;    // 0xB70124
    // 0x8CC858. One extra entry that is 'safe' in case all the others fail
    static constexpr int32 SequenceOfTrainCams[7]{ MOVIECAM20, MOVIECAM22, MOVIECAM2, MOVIECAM21, MOVIECAM3, MOVIECAM0, MOVIECAM0 };

    auto i = 0;
    if (!bDidWeProcessAnyCinemaCam) {
        OldMode = -1;
        if (gbCineyCamMessageDisplayed > 0 && !m_bCinemaCamera) {
            gbCineyCamMessageDisplayed--;
            CHud::SetHelpMessage(TheText.Get("CINCAM"), true, false, false);
        }
        bSwitchedToObbeCam = true;
    } else if (!IsItTimeForNewCamera(SequenceOfTrainCams[OldMode], TimeForNext)) {
        m_nModeObbeCamIsInForCar = OldMode;
        bDidWeProcessAnyCinemaCam = true;
        return;
    }

    OldMode = (OldMode + gCinematicModeSwitchDir) % 6;
    if (OldMode < 0) {
        OldMode = 5;
    } else if (OldMode > 5) {
        OldMode = 0;
    }
    if (!TryToStartNewCamMode(SequenceOfTrainCams[OldMode])) {
        while (i <= 6) {
            OldMode = (OldMode + gCinematicModeSwitchDir) % 6;
            if (OldMode < 0) {
                OldMode = 5;
            } else if (OldMode > 5) {
                OldMode = 0;
            }
            ++i;
            if (TryToStartNewCamMode(SequenceOfTrainCams[OldMode])) {
                break;
            }
        }
    }

    TimeForNext = CTimer::GetTimeInMS();
    if (i >= 6) { // Everything failed, fall back to the one mode that is always safe
        OldMode = 6;
        TryToStartNewCamMode(SequenceOfTrainCams[6]);
    }

    m_nModeObbeCamIsInForCar = OldMode;
    bDidWeProcessAnyCinemaCam = true;
}


// 0x50B890
void CCamera::ProcessWideScreenOn() {
    if (m_bWantsToSwitchWidescreenOff) {
        m_bWantsToSwitchWidescreenOff = false;
        m_bWideScreenOn = false;
        m_fWideScreenReductionAmount = 0.0f;
        m_fScreenReductionPercentage = 0.0f;
        m_fFOV_Wide_Screen = 0.0f;
    } else {
        m_fWideScreenReductionAmount = 1.0f;
        m_fScreenReductionPercentage = 30.0f;
        m_fFOV_Wide_Screen = m_aCams[m_nActiveCam].m_fFOV * 0.3f;
    }
}

// 0x516440
void CCamera::ProcessVectorTrackLinear() {
    float now = (float)CTimer::m_snTimeInMilliseconds;
    if (now <= m_fTrackLinearEndTime) {
        float duration = m_fTrackLinearEndTime - m_fTrackLinearStartTime;
        float ratio = (now - m_fTrackLinearStartTime) / duration;
        ProcessVectorTrackLinear(ratio);
    } else if (m_bCameraPersistTrack) {
        m_bVecTrackLinearProcessed = true;
    }
}

// 0x50D350
void CCamera::ProcessVectorTrackLinear(float ratio) {
    m_bVecTrackLinearProcessed = true;
    if (m_bTrackLinearWithEase) {
        float angle = 270.0f - ratio * 180.0f;
        float factor = (std::sin(angle * (PI / 180.0f)) + 1.0f) * 0.5f;
        m_vecTrackLinear.x = m_vecTrackLinearEndPoint.x + factor * (m_vecTrackLinearStartPoint.x - m_vecTrackLinearEndPoint.x);
        m_vecTrackLinear.y = m_vecTrackLinearEndPoint.y + factor * (m_vecTrackLinearStartPoint.y - m_vecTrackLinearEndPoint.y);
        m_vecTrackLinear.z = m_vecTrackLinearEndPoint.z + factor * (m_vecTrackLinearStartPoint.z - m_vecTrackLinearEndPoint.z);
    } else {
        m_vecTrackLinear.x = m_vecTrackLinearEndPoint.x + ratio * (m_vecTrackLinearStartPoint.x - m_vecTrackLinearEndPoint.x);
        m_vecTrackLinear.y = m_vecTrackLinearEndPoint.y + ratio * (m_vecTrackLinearStartPoint.y - m_vecTrackLinearEndPoint.y);
        m_vecTrackLinear.z = m_vecTrackLinearEndPoint.z + ratio * (m_vecTrackLinearStartPoint.z - m_vecTrackLinearEndPoint.z);
    }
}

// 0x526E20
void CCamera::ProcessObbeCinemaCameraBoat() {
    static int32 OldMode     = -1;   // 0x8CCEFC
    static int32 TimeForNext = 0;    // 0xB70130
    // 0x8CC8B0. One extra entry that is 'safe' in case all the others fail
    static constexpr int32 SequenceOfBoatCams[4]{ MOVIECAM20, MOVIECAM3, MOVIECAM18, MOVIECAM0 };

    auto i = 0;
    if (!bDidWeProcessAnyCinemaCam) {
        OldMode = -1;
        if (gbCineyCamMessageDisplayed > 0 && !m_bCinemaCamera) {
            gbCineyCamMessageDisplayed--;
            CHud::SetHelpMessage(TheText.Get("CINCAM"), true, false, false);
        }
        bSwitchedToObbeCam = true;
    } else if (!IsItTimeForNewCamera(SequenceOfBoatCams[OldMode], TimeForNext)) {
        m_nModeObbeCamIsInForCar = OldMode;
        bDidWeProcessAnyCinemaCam = true;
        return;
    }

    OldMode = (OldMode + gCinematicModeSwitchDir) % 3;
    if (OldMode < 0) {
        OldMode = 2;
    } else if (OldMode > 2) {
        OldMode = 0;
    }
    if (TryToStartNewCamMode(SequenceOfBoatCams[OldMode])) {
        TimeForNext = CTimer::GetTimeInMS();
        m_nModeObbeCamIsInForCar = OldMode;
        bDidWeProcessAnyCinemaCam = true;
        return;
    }

    while (i <= 3) {
            OldMode = (OldMode + gCinematicModeSwitchDir) % 3;
            if (OldMode < 0) {
                OldMode = 2;
            } else if (OldMode > 2) {
                OldMode = 0;
            }
        ++i;
        if (TryToStartNewCamMode(SequenceOfBoatCams[OldMode])) {
            if (i < 3) {
        TimeForNext = CTimer::GetTimeInMS();
            m_nModeObbeCamIsInForCar = OldMode;
            bDidWeProcessAnyCinemaCam = true;
            return;
            }
            break;
        }
    }

    // Everything failed, drop back to the on-a-string camera
    const auto alreadyOnString = m_aCams[m_nActiveCam].m_nMode == MODE_CAM_ON_A_STRING;
    OldMode = 3;
    if (!alreadyOnString) {
        TryToStartNewCamMode(CAM_ON_A_STRING_LAST_RESORT);
        TimeForNext = CTimer::GetTimeInMS();
    }

    m_nModeObbeCamIsInForCar = OldMode;
    bDidWeProcessAnyCinemaCam = true;
}


// 0x5267C0
void CCamera::ProcessObbeCinemaCameraCar() {
    static int32 OldMode     = -1;   // 0x8CCEEC
    static int32 TimeForNext = 0;    // 0xB70120
    // 0x8CC828. One extra entry that is 'safe' in case all the others fail
    static constexpr int32 SequenceOfCarCams[12]{ MOVIECAM20, MOVIECAM22, MOVIECAM7, MOVIECAM3, MOVIECAM22, MOVIECAM1,
                                              MOVIECAM21, MOVIECAM8, MOVIECAM2, MOVIECAM22, MOVIECAM5, MOVIECAM6 };

    auto i = 0;
    if (!bDidWeProcessAnyCinemaCam) {
        OldMode = -1;
        if (gbCineyCamMessageDisplayed > 0 && !m_bCinemaCamera) {
            gbCineyCamMessageDisplayed--;
            CHud::SetHelpMessage(TheText.Get("CINCAM"), true, false, false);
        }
        bSwitchedToObbeCam = true;
    } else if (!IsItTimeForNewCamera(SequenceOfCarCams[OldMode], TimeForNext)) {
        m_nModeObbeCamIsInForCar = OldMode;
        bDidWeProcessAnyCinemaCam = true;
        return;
    }

    OldMode = (OldMode + gCinematicModeSwitchDir) % 11;
    if (OldMode < 0) {
        OldMode = 10;
    } else if (OldMode > 10) {
        OldMode = 0;
    }
    if (!TryToStartNewCamMode(SequenceOfCarCams[OldMode])) {
        while (i <= 11) {
            OldMode = (OldMode + gCinematicModeSwitchDir) % 11;
            if (OldMode < 0) {
                OldMode = 10;
            } else if (OldMode > 10) {
                OldMode = 0;
            }
            ++i;
            if (TryToStartNewCamMode(SequenceOfCarCams[OldMode])) {
                break;
            }
        }
    }

    TimeForNext = CTimer::GetTimeInMS();
    if (i >= 11) { // Everything failed, fall back to the one mode that is always safe
        OldMode = 11;
        TryToStartNewCamMode(SequenceOfCarCams[11]);
    }

    m_nModeObbeCamIsInForCar = OldMode;
    bDidWeProcessAnyCinemaCam = true;
}


// 0x526AE0
void CCamera::ProcessObbeCinemaCameraHeli() {
    static int32 OldMode     = -1;   // 0x8CCEF4
    static int32 TimeForNext = 0;    // 0xB70128
    // 0x8CC874. One extra entry that is 'safe' in case all the others fail
    static constexpr int32 SequenceOfHeliCams[8]{ MOVIECAMPLANE1, MOVIECAMPLANE2, MOVIECAM23, MOVIECAM20,
                                               MOVIECAM22, MOVIECAMPLANE3, MOVIECAM23, MOVIECAM0 };

    auto i = 0;
    if (!bDidWeProcessAnyCinemaCam) {
        OldMode = -1;
        if (gbCineyCamMessageDisplayed > 0 && !m_bCinemaCamera) {
            gbCineyCamMessageDisplayed--;
            CHud::SetHelpMessage(TheText.Get("CINCAM"), true, false, false);
        }
        bSwitchedToObbeCam = true;
    } else if (CameraObscuredByWaterLevel() || !IsItTimeForNewCamera(SequenceOfHeliCams[OldMode], TimeForNext)) {
        m_nModeObbeCamIsInForCar = OldMode;
        bDidWeProcessAnyCinemaCam = true;
        return;
    }

    OldMode = (OldMode + gCinematicModeSwitchDir) % 7;
    if (OldMode < 0) {
        OldMode = 6;
    } else if (OldMode > 6) {
        OldMode = 0;
    }
    if (TryToStartNewCamMode(SequenceOfHeliCams[OldMode])) {
        TimeForNext = CTimer::GetTimeInMS();
        m_nModeObbeCamIsInForCar = OldMode;
        bDidWeProcessAnyCinemaCam = true;
        return;
    }

    while (i <= 7) {
            OldMode = (OldMode + gCinematicModeSwitchDir) % 7;
            if (OldMode < 0) {
                OldMode = 6;
            } else if (OldMode > 6) {
                OldMode = 0;
            }
        ++i;
        if (TryToStartNewCamMode(SequenceOfHeliCams[OldMode])) {
            if (i < 7) {
        TimeForNext = CTimer::GetTimeInMS();
            m_nModeObbeCamIsInForCar = OldMode;
            bDidWeProcessAnyCinemaCam = true;
            return;
            }
            break;
        }
    }

    // Everything failed, drop back to the on-a-string camera
    const auto alreadyOnString = m_aCams[m_nActiveCam].m_nMode == MODE_CAM_ON_A_STRING;
    OldMode = 7;
    if (!alreadyOnString) {
        TryToStartNewCamMode(CAM_ON_A_STRING_LAST_RESORT);
        TimeForNext = CTimer::GetTimeInMS();
    }

    m_nModeObbeCamIsInForCar = OldMode;
    bDidWeProcessAnyCinemaCam = true;
}


// 0x50D430
void CCamera::ProcessVectorMoveLinear(float ratio) {
    m_bVecMoveLinearProcessed = true;
    if (m_bMoveLinearWithEase) {
        float angle = 270.0f - ratio * 180.0f;
        float factor = (std::sin(angle * (PI / 180.0f)) + 1.0f) * 0.5f;
        m_vecMoveLinear.x = m_vecMoveLinearPosnStart.x + factor * (m_vecMoveLinearPosnEnd.x - m_vecMoveLinearPosnStart.x);
        m_vecMoveLinear.y = m_vecMoveLinearPosnStart.y + factor * (m_vecMoveLinearPosnEnd.y - m_vecMoveLinearPosnStart.y);
        m_vecMoveLinear.z = m_vecMoveLinearPosnStart.z + factor * (m_vecMoveLinearPosnEnd.z - m_vecMoveLinearPosnStart.z);
    } else {
        m_vecMoveLinear.x = m_vecMoveLinearPosnStart.x + ratio * (m_vecMoveLinearPosnEnd.x - m_vecMoveLinearPosnStart.x);
        m_vecMoveLinear.y = m_vecMoveLinearPosnStart.y + ratio * (m_vecMoveLinearPosnEnd.y - m_vecMoveLinearPosnStart.y);
        m_vecMoveLinear.z = m_vecMoveLinearPosnStart.z + ratio * (m_vecMoveLinearPosnEnd.z - m_vecMoveLinearPosnStart.z);
    }
}

// 0x516500
void CCamera::ProcessFOVLerp() {
    float now = (float)CTimer::m_snTimeInMilliseconds;
    if (now <= m_fEndZoomTime) {
        float duration = m_fEndZoomTime - m_fStartZoomTime;
        float ratio = (now - m_fStartZoomTime) / duration;
        ProcessFOVLerp(ratio);
    } else if (m_bBlockZoom) {
        m_bFOVLerpProcessed = true;
    }
}

// 0x50D510
void CCamera::ProcessFOVLerp(float ratio) {
    m_bFOVLerpProcessed = true;
    if (m_nZoomMode) {
        float angle = 270.0f - ratio * 180.0f;
        float factor = (std::sin(angle * (PI / 180.0f)) + 1.0f) * 0.5f;
        m_fFOVNew = m_fZoomInFactor + factor * (m_fZoomOutFactor - m_fZoomInFactor);
    } else {
        m_fFOVNew = m_fZoomInFactor + ratio * (m_fZoomOutFactor - m_fZoomInFactor);
    }
}

// 0x5164A0
void CCamera::ProcessVectorMoveLinear() {
    float now = (float)CTimer::m_snTimeInMilliseconds;
    if (now <= m_fMoveLinearEndTime) {
        float duration = m_fMoveLinearEndTime - m_fMoveLinearStartTime;
        float ratio = (now - m_fMoveLinearStartTime) / duration;
        ProcessVectorMoveLinear(ratio);
    } else if (m_bCameraPersistPosition) {
        m_bVecMoveLinearProcessed = true;
    }
}

// 0x51A6F0
void CCamera::ProcessShake() {
    const auto now = (float)CTimer::m_snTimeInMilliseconds;
    if (now <= m_fEndShakeTime) {
        ProcessShake((now - m_fStartShakeTime) / (m_fEndShakeTime - m_fStartShakeTime));
    }
}

//! Guards the one-shot `gHandShaker` setup done by `ProcessShake`
static inline auto& gbJiggleInit = StaticRef<bool>(0xB70048);

//! `ratio` is passed by both callers but never read.
//! IDA types this as returning `CVector*`: that's the pointer to a dead stack temporary
//! the trailing `CrossProduct` happens to leave in `eax`, and neither caller reads it.
// 0x516560
void CCamera::ProcessShake(float ratio) {
    auto& cam = m_aCams[m_nActiveCam];

    // Everything here except `m_lim.z` of shaker 2, `m_twitchFreq` and `m_twitchVel`
    // repeats what `CHandShaker::SetDefaults` already wrote - the game rewrites it anyway.
    if (!gbJiggleInit) {
        gbJiggleInit = true;

        struct ShakerTweak {
            CVector lim;
            int32   twitchFreq;
            float   twitchVel;
        };
        static constexpr ShakerTweak tweaks[]{
            { { 0.02f, 0.02f, 0.01f }, 15, 0.001f  }, // gHandShaker[1]
            { { 0.02f, 0.02f, 0.04f }, 20, 0.001f  }, // gHandShaker[2]
            { { 0.02f, 0.02f, 0.01f }, 10, 0.0005f }, // gHandShaker[3]
            { { 0.02f, 0.02f, 0.01f }, 20, 0.002f  }, // gHandShaker[4]
            { { 0.02f, 0.02f, 0.01f },  2, 0.003f  }, // gHandShaker[5]
        };
        for (auto&& [i, tweak] : rngv::enumerate(tweaks)) {
            auto& hs = gHandShaker[i + 1];

            hs.m_lim    = tweak.lim;
            hs.m_motion.Set(0.0002f, 0.0002f, 0.0001f);
            hs.m_slow.Set(1.3f, 1.3f, 1.4f);

            hs.m_scaleReactionMin = 0.3f;  // Scale of the reaction, based on how far away angularily we are from it
            hs.m_scaleReactionMax = 1.0f;
            hs.m_twitchFreq       = tweak.twitchFreq;
            hs.m_twitchVel        = tweak.twitchVel;
        }
    }

    auto& hs = gHandShaker[m_nShakeType];
    hs.Process(m_fShakeIntensity);

    const auto roll = hs.m_ang.z * m_fShakeIntensity;

    cam.m_vecFront = hs.m_resultMat.InverseTransformVector(cam.m_vecFront); // Multiply3x3(front, resultMat)
    cam.m_vecFront.Normalise();

    cam.m_vecUp.Set(std::sin(roll), 0.0f, std::cos(roll));
    cam.m_vecUp = cam.m_vecFront.Cross(cam.m_vecUp).Normalized().Cross(cam.m_vecFront);

    if (cam.m_vecFront.x == 0.0f && cam.m_vecFront.y == 0.0f) {
        cam.m_vecFront.x = cam.m_vecFront.y = 0.0001f;
    }

    cam.m_vecUp = cam.m_vecFront.Cross(cam.m_vecUp).Normalized().Cross(cam.m_vecFront);
}

// inlined - 0x52B845
// 0x516AE0
void CCamera::ProcessScriptedCommands() {
    ProcessVectorMoveLinear();
    ProcessVectorTrackLinear();
    ProcessFOVLerp();
}

// 0x52B730
void CCamera::Process() {
    ZoneScoped;

    constexpr auto NORMAL_NEAR_CLIP              = 0.3f;
    constexpr auto NEAR_CLIP_PED_VIEW_OBSCURED   = 0.05f;
    constexpr auto BETA_DIFF_FOR_CUT_OFF_DOPPLER = 0.3f;
    constexpr auto ABOVEBELOWWATER               = 0.6f;

    static float MinDistCamAwayFromPlayWhenInter = 1.3f;
    static float DrunkRotation                   = 0.0f;
    static bool  bBlurSet                        = false;
    static bool  WasPreviouslyInterSyhonFollowPed = false;

    ResetMadeInvisibleObjects();

    m_bJust_Switched = false;
    m_vecRealPreviousCameraPosition = GetPosition();

    if (m_bLookingAtPlayer || m_bTargetJustBeenOnTrain || m_nWhoIsInControlOfTheCamera == 2) {
        UpdateTargetEntity();
    }

    if (!m_pTargetEntity) {
        m_pTargetEntity = FindPlayerPed();
        m_pTargetEntity->RegisterReference(&m_pTargetEntity);
    }

    auto& activeCam = m_aCams[m_nActiveCam];
    auto& otherCam  = m_aCams[(m_nActiveCam + 1) % 2];
    for (auto* cam : { &activeCam, &otherCam }) {
        if (!cam->m_pCamTargetEntity) {
            cam->m_pCamTargetEntity = m_pTargetEntity;
            cam->m_pCamTargetEntity->RegisterReference(&cam->m_pCamTargetEntity);
        }
    }

    CamControl();
    ProcessScriptedCommands();

    if (m_bFading) {
        ProcessFade();
    }
    if (m_bMusicFading) {
        ProcessMusicFade();
    }
    if (m_bWideScreenOn) {
        ProcessWideScreenOn();
    }

    RwCameraSetNearClipPlane(Scene.m_pRwCamera, NORMAL_NEAR_CLIP);

    float BetaBefore = (activeCam.m_vecFront.x == 0.0f && activeCam.m_vecFront.y == 0.0f)
        ? 0.0f
        : CGeneral::GetATanOfXY(activeCam.m_vecFront.x, activeCam.m_vecFront.y);

    activeCam.Process();

    float BetaAfter = (activeCam.m_vecFront.x == 0.0f && activeCam.m_vecFront.y == 0.0f)
        ? 0.0f
        : CGeneral::GetATanOfXY(activeCam.m_vecFront.x, activeCam.m_vecFront.y);

    if (m_bTransitionState) {
        if (CTimer::GetTimeInMS() > (m_nTimeTransitionStart + m_nTransitionDuration)) {
            m_bTransitionState         = false;
            m_bDoingSpecialInterp      = false;
            m_bWaitForInterpolToFinish = false;
        }
    }

    if (m_bUseNearClipScript) {
        RwCameraSetNearClipPlane(Scene.m_pRwCamera, m_fNearClipScript);
    }

    float BetaDiff = BetaAfter - BetaBefore;
    while (BetaDiff >= PI) {
        BetaDiff -= TWO_PI;
    }
    while (BetaDiff < -PI) {
        BetaDiff += TWO_PI;
    }
    if (std::abs(BetaDiff) > BETA_DIFF_FOR_CUT_OFF_DOPPLER) {
        m_bJust_Switched = true;
    }

    bool WasDoingACarLookThingy = false;
    if (activeCam.m_nDirectionWasLooking != LOOKING_FORWARD && m_pTargetEntity->GetIsTypeVehicle()) {
        WasDoingACarLookThingy = true;
    }

    ProcessShake();

    CVector FinalSource{}, FinalFront{}, FinalUp{}, TempTargetWhenInterPol{};
    float   FinalFOV = 0.0f;

    if (!m_bTransitionState || WasDoingACarLookThingy) {
        FinalSource = activeCam.m_vecSource;
        FinalUp     = activeCam.m_vecUp;
        if (m_bMoveCamToAvoidGeom) {
            FinalSource += m_vecClearGeometryVec;
            FinalFront = activeCam.m_vecTargetCoorsForFudgeInter - FinalSource;
            FinalFront.Normalise();
            CVector TempRight = CrossProduct(FinalFront, FinalUp);
            TempRight.Normalise();
            FinalUp = CrossProduct(TempRight, FinalFront);
            FinalUp.Normalise();
        } else {
            FinalFront = activeCam.m_vecFront;
            FinalUp    = activeCam.m_vecUp;
        }
        FinalFOV = activeCam.m_fFOV;
        WasPreviouslyInterSyhonFollowPed = false;
    } else {
        uint32 TimeInInterpolation = CTimer::GetTimeInMS() - m_nTimeTransitionStart;
        TimeInInterpolation = std::min(TimeInInterpolation, m_nTransitionDuration);
        float InterValue = (float)TimeInInterpolation / (float)m_nTransitionDuration;
        float InterFraction = 0.0f;
        float TargetInterFraction = 0.0f;

        float TempInterValue = (float)TimeInInterpolation / (float)m_nTransitionDurationTargetCoors;
        TempInterValue = std::clamp(TempInterValue, 0.0f, 1.0f);

        if (TempInterValue <= m_fFractionInterToStopMovingTarget) {
            if (m_fFractionInterToStopMovingTarget == 0.0f) {
                TargetInterFraction = 0.0f;
            } else {
                TargetInterFraction = (m_fFractionInterToStopMovingTarget - TempInterValue) / m_fFractionInterToStopMovingTarget;
            }
            TargetInterFraction = 0.5f - (0.5f * std::cos(TargetInterFraction * PI));
            m_vecTargetWhenInterPol = m_vecStartingTargetForInterPol + (m_vecTargetSpeedAtStartInter * TargetInterFraction);
            TempTargetWhenInterPol  = m_vecTargetWhenInterPol;
        } else {
            if (m_fFractionInterToStopCatchUpTarget == 0.0f) {
                TargetInterFraction = 1.0f;
            } else {
                TargetInterFraction = (TempInterValue - m_fFractionInterToStopMovingTarget) / m_fFractionInterToStopCatchUpTarget;
            }
            TargetInterFraction = 0.5f - (0.5f * std::cos(TargetInterFraction * PI));
            if (m_fFractionInterToStopMovingTarget == 0.0f) {
                m_vecTargetWhenInterPol = m_vecStartingTargetForInterPol;
            }
            TempTargetWhenInterPol = m_vecTargetWhenInterPol + ((activeCam.m_vecTargetCoorsForFudgeInter - m_vecTargetWhenInterPol) * TargetInterFraction);
        }

        if (InterValue <= m_fFractionInterToStopMoving) {
            if (m_fFractionInterToStopMoving == 0.0f) {
                InterFraction = 0.0f;
            } else {
                InterFraction = (m_fFractionInterToStopMoving - InterValue) / m_fFractionInterToStopMoving;
            }
            InterFraction = 0.5f - (0.5f * std::cos(InterFraction * PI));

            m_vecSourceWhenInterPol = m_vecStartingSourceForInterPol + (m_vecSourceSpeedAtStartInter * InterFraction);
            if (m_bLookingAtPlayer) {
                CVector TempForGroundDist = m_vecSourceWhenInterPol - TempTargetWhenInterPol;
                if (TempForGroundDist.Magnitude2D() < MinDistCamAwayFromPlayWhenInter) {
                    float VecAngle = CGeneral::GetATanOfXY(TempForGroundDist.x, TempForGroundDist.y);
                    m_vecSourceWhenInterPol.x = TempTargetWhenInterPol.x + MinDistCamAwayFromPlayWhenInter * std::cos(VecAngle);
                    m_vecSourceWhenInterPol.y = TempTargetWhenInterPol.y + MinDistCamAwayFromPlayWhenInter * std::sin(VecAngle);
                }
            }

            m_vecUpWhenInterPol = m_vecStartingUpForInterPol + (m_vecUpSpeedAtStartInter * InterFraction);
            m_fFOVWhenInterPol  = m_fStartingFOVForInterPol + (m_fFOVSpeedAtStartInter * InterFraction);
            FinalSource = m_vecSourceWhenInterPol;

            FinalFront = TempTargetWhenInterPol - FinalSource;
            StoreValuesDuringInterPol(&FinalSource, &m_vecTargetWhenInterPol, &m_vecUpWhenInterPol, &m_fFOVWhenInterPol);
            FinalFront.Normalise();

            if (m_bLookingAtPlayer) {
                FinalUp = CVector(0.0f, 0.0f, 1.0f);
            } else {
                FinalUp = m_vecUpWhenInterPol;
            }

            FinalUp.Normalise();
            if (activeCam.m_nMode != MODE_TOPDOWN && activeCam.m_nMode != MODE_TOP_DOWN_PED) {
                FinalFront.Normalise();
                FinalUp.Normalise();
                CVector TestTempRight = CrossProduct(FinalFront, FinalUp);
                TestTempRight.Normalise();
                FinalUp = CrossProduct(TestTempRight, FinalFront);
                FinalUp.Normalise();
            } else {
                FinalFront.Normalise();
                CVector TestTempRight = CVector(-1.0f, 0.0f, 0.0f);
                FinalUp = CrossProduct(FinalFront, TestTempRight);
                FinalUp.Normalise();
            }

            FinalFOV = m_fFOVWhenInterPol;
        } else if (InterValue > m_fFractionInterToStopMoving && InterValue <= 1.0f) {
            if (m_fFractionInterToStopCatchUp == 0.0f) {
                InterFraction = 1.0f;
            } else {
                InterFraction = (InterValue - m_fFractionInterToStopMoving) / m_fFractionInterToStopCatchUp;
            }

            InterFraction = 0.5f - (0.5f * std::cos(InterFraction * PI));

            FinalSource = m_vecSourceWhenInterPol + ((activeCam.m_vecSource - m_vecSourceWhenInterPol) * InterFraction);

            if (m_bLookingAtPlayer) {
                CVector TempForGroundDist = FinalSource - TempTargetWhenInterPol;
                if (TempForGroundDist.Magnitude2D() < MinDistCamAwayFromPlayWhenInter) {
                    float VecAngle = CGeneral::GetATanOfXY(TempForGroundDist.x, TempForGroundDist.y);
                    FinalSource.x = TempTargetWhenInterPol.x + MinDistCamAwayFromPlayWhenInter * std::cos(VecAngle);
                    FinalSource.y = TempTargetWhenInterPol.y + MinDistCamAwayFromPlayWhenInter * std::sin(VecAngle);
                }
            }

            FinalFOV = m_fFOVWhenInterPol + ((activeCam.m_fFOV - m_fFOVWhenInterPol) * InterFraction);
            FinalUp  = m_vecUpWhenInterPol + ((activeCam.m_vecUp - m_vecUpWhenInterPol) * InterFraction);

            FinalFront = TempTargetWhenInterPol - FinalSource;
            StoreValuesDuringInterPol(&FinalSource, &TempTargetWhenInterPol, &FinalUp, &FinalFOV);
            FinalFront.Normalise();
            if (m_bLookingAtPlayer) {
                FinalUp = CVector(0.0f, 0.0f, 1.0f);
            }

            if (activeCam.m_nMode != MODE_TOPDOWN && activeCam.m_nMode != MODE_TOP_DOWN_PED) {
                FinalFront.Normalise();
                FinalUp.Normalise();
                CVector TestTempRight = CrossProduct(FinalFront, FinalUp);
                TestTempRight.Normalise();
                FinalUp = CrossProduct(TestTempRight, FinalFront);
                FinalUp.Normalise();
            } else {
                FinalFront.Normalise();
                CVector TestTempRight = CVector(-1.0f, 0.0f, 0.0f);
                FinalUp = CrossProduct(FinalFront, TestTempRight);
                FinalUp.Normalise();
            }

            FinalFOV = m_fFOVWhenInterPol;
        }

        CVector VecForBetaAlpha = FinalSource - TempTargetWhenInterPol;
        float DistOnGround = VecForBetaAlpha.Magnitude2D();
        float TempTrueAlpha = CGeneral::GetATanOfXY(DistOnGround, VecForBetaAlpha.z);
        float TempTrueBeta  = CGeneral::GetATanOfXY(VecForBetaAlpha.x, VecForBetaAlpha.y);

        activeCam.KeepTrackOfTheSpeed(FinalSource, TempTargetWhenInterPol, FinalUp, TempTrueAlpha, TempTrueBeta, FinalFOV);
    }

    if (m_bTransitionState && !m_bLookingAtVector && m_bLookingAtPlayer
        && !CCullZones::CamStairsForPlayer() && !m_bPlayerIsInGarage
    ) {
        CColPoint colPoint{};
        CEntity*  hitEntity = nullptr;
        if (CWorld::ProcessLineOfSight(m_pTargetEntity->GetPosition(), FinalSource, colPoint, hitEntity,
                                       true, false, false, true, false, true, true, false)) {
            FinalSource = colPoint.m_vecPoint;
            RwCameraSetNearClipPlane(Scene.m_pRwCamera, NEAR_CLIP_PED_VIEW_OBSCURED);
        }
    }

    if (CMBlur::Drunkness > 0.0f) {
        float angle = DegreesToRadians(DrunkRotation);
        float drunk = CMBlur::Drunkness * -0.020f;
        FinalSource.x += std::cos(angle) * drunk;
        FinalSource.z += std::sin(angle) * drunk;

        FinalUp.Normalise();
        drunk = CMBlur::Drunkness * 0.05f;
        FinalUp.x += std::cos(angle) * drunk;
        FinalUp.y += std::sin(angle) * drunk;
        FinalUp.Normalise();

        FinalFront.Normalise();
        drunk = CMBlur::Drunkness * -0.1f;
        FinalFront.x += std::cos(angle) * drunk;
        FinalFront.y += std::sin(angle) * drunk;
        FinalFront.Normalise();

        CVector TestTempRight = CrossProduct(FinalFront, FinalUp);
        TestTempRight.Normalise();
        FinalUp = CrossProduct(TestTempRight, FinalFront);
        FinalUp.Normalise();

        DrunkRotation += 5.0f;
    }

    CVector Right = CrossProduct(FinalUp, FinalFront);
    GetMatrix().GetRight()    = Right;
    GetMatrix().GetForward()  = FinalFront;
    GetMatrix().GetUp()       = FinalUp;
    GetMatrix().GetPosition() = FinalSource;
    m_mCameraMatrix = GetMatrix();

    float CurrentShakeForce = m_fCamShakeForce - (float)(CTimer::GetTimeInMS() - m_nCamShakeStart) * 0.00028f;
    CurrentShakeForce = std::clamp(CurrentShakeForce, 0.0f, 2.0f);
    const float blurDelta = CurrentShakeForce;
    uint16 Random = CGeneral::GetRandomNumber();
    CurrentShakeForce *= 0.1f;

    GetMatrix().GetPosition().x += (float)((Random & 0x000F) - 7) * CurrentShakeForce;
    GetMatrix().GetPosition().y += (float)(((Random & 0x00F0) >> 4) - 7) * CurrentShakeForce;
    GetMatrix().GetPosition().z += (float)(((Random & 0x0F00) >> 8) - 7) * CurrentShakeForce;
    m_mCameraMatrix = GetMatrix();

    if (CurrentShakeForce > 0.0f && m_nBlurType != eMotionBlurType::SNIPER) {
        int32 alpha = 25 + (int32)(blurDelta * 255.0f);
        if (alpha > 150) {
            alpha = 150;
        }
        SetMotionBlurAlpha(alpha);
    }

    if (activeCam.m_nMode == MODE_1STPERSON && FindPlayerVehicle() && FindPlayerVehicle()->GetMatrix().GetUp().z < 0.2f) {
        SetMotionBlur(255, 255, 255, 240, eMotionBlurType::SNIPER);
        bBlurSet = true;
    } else if (bBlurSet) {
        bBlurSet = false;
    }

    CDraw::SetFOV(FinalFOV);
    CalculateDerivedValues(false, true);

    CopyCameraMatrixToRWCam(false);
    m_vecGameCamPos = GetPosition();

    UpdateSoundDistances();

    if (CCutsceneMgr::ms_running && !CCutsceneMgr::ms_useLodMultiplier) {
        m_fLODDistMultiplier = 1.0f;
    } else {
        m_fLODDistMultiplier = 70.0f / CDraw::GetFOV();
    }
    m_fGenerationDistMultiplier = m_fLODDistMultiplier;
    m_fLODDistMultiplier *= CRenderer::ms_lodDistScale;

    float farclip = RwCameraGetFarClipPlane(Scene.m_pRwCamera);
    farclip *= 100.0f;
    int32 nd = (int32)farclip;
    farclip = (float)nd / 100.0f;
    RwCameraSetFarClipPlane(Scene.m_pRwCamera, farclip);

    CDraw::SetNearClipZ(RwCameraGetNearClipPlane(m_pRwCamera));
    CDraw::SetFarClipZ(RwCameraGetFarClipPlane(m_pRwCamera));

    if (m_bJustInitialized || m_bJust_Switched) {
        m_vecPreviousCameraPosition = GetPosition();
        m_bJustInitialized = false;
    }

    CVector DistDiff = GetPosition() - m_vecPreviousCameraPosition;
    m_fCameraSpeedSoFar += DistDiff.Magnitude();
    m_nNumFramesSoFar++;

    if (m_nNumFramesSoFar == m_nWorkOutSpeedThisNumFrames) {
        m_fCameraAverageSpeed = m_fCameraSpeedSoFar / (float)m_nWorkOutSpeedThisNumFrames;
        m_fCameraSpeedSoFar   = 0.0f;
        m_nNumFramesSoFar     = 0;
    }

    m_vecPreviousCameraPosition = GetPosition();

    float OrientPlusPi = m_fOrientation + PI;
    if (activeCam.m_nDirectionWasLooking != LOOKING_FORWARD) {
        if (activeCam.m_nMode != MODE_TOP_DOWN_PED) {
            activeCam.m_vecSource = activeCam.m_vecSourceBeforeLookBehind;
            m_fOrientation = OrientPlusPi;
        }
    }

    if (m_bTransitionState) {
        if (otherCam.m_pCamTargetEntity != nullptr && m_pTargetEntity != nullptr) {
            if (m_pTargetEntity->GetIsTypePed() && !otherCam.m_pCamTargetEntity->GetIsTypeVehicle()) {
                if (activeCam.m_nMode != MODE_TOP_DOWN_PED) {
                    if (otherCam.m_nDirectionWasLooking != LOOKING_FORWARD) {
                        otherCam.m_vecSource = otherCam.m_vecSourceBeforeLookBehind;
                        m_fOrientation = OrientPlusPi;
                    }
                }
            }
        }
    }

    m_bCameraJustRestored = false;
    m_bMoveCamToAvoidGeom = false;

    float LocalWaterHeight = 0.0f;
    CVector TestPos = GetPosition() + (0.4f * GetForward());
    bool bWaterFound = CWaterLevel::GetWaterLevel(TestPos.x, TestPos.y, TestPos.z, LocalWaterHeight, true, nullptr);

    if (!bWaterFound || LocalWaterHeight < TestPos.z - ABOVEBELOWWATER) {
        CWeather::UnderWaterness = 0.0f;
    } else {
        CWeather::WaterDepth = std::max(0.0f, LocalWaterHeight - TestPos.z);
        if (LocalWaterHeight > TestPos.z + ABOVEBELOWWATER) {
            CWeather::UnderWaterness = 1.0f;
        } else {
            CWeather::UnderWaterness = 1.0f - (TestPos.z - (LocalWaterHeight - ABOVEBELOWWATER)) / (ABOVEBELOWWATER + ABOVEBELOWWATER);
        }
    }
}

// 0x514860
void CCamera::DrawBordersForWideScreen() {
    CRect rect;
    GetScreenRect(&rect);
    if (m_nBlurType == eMotionBlurType::NONE || m_nBlurType == eMotionBlurType::LIGHT_SCENE) {
        m_nMotionBlurAddAlpha = 80;
    }
    RwRenderStateSet(rwRENDERSTATETEXTURERASTER, RWRSTATE(NULL));
    CSprite2d::DrawRect({ -5.f, -5.f,     SCREEN_WIDTH + 5.f, rect.top         }, { 0, 0, 0, 255 });
    CSprite2d::DrawRect({ -5.f, rect.bottom, SCREEN_WIDTH + 5.f, SCREEN_HEIGHT + 5.f }, { 0, 0, 0, 255 });
}

// 0x4748A0
bool CCamera::VectorMoveRunning() const {
    return CTimer::m_snTimeInMilliseconds <= m_fMoveLinearEndTime;
}

// 0x474891
bool CCamera::VectorTrackRunning() const {
    return CTimer::m_snTimeInMilliseconds <= m_fTrackLinearEndTime;
}

// 0x514950
void CCamera::FinishCutscene() {
    SetPercentAlongCutScene(100.0f);
    m_fPositionAlongSpline = 1.0f;
    m_bCutsceneFinished = true;
}

// 0x514970
bool CCamera::Find3rdPersonCamTargetVector(float fRange, CVector vecGunMuzzle, CVector& vecSource, CVector& vecTarget) {
    float fScreenAngle, fScreenPosMult;

    fScreenAngle     = DegreesToRadians(0.5f * m_aCams[m_nActiveCam].m_fFOV);
    fScreenPosMult   = 2.0f * (m_f3rdPersonCHairMultX - 0.5f);
    float fRightMult = fScreenPosMult * std::tan(fScreenAngle);

    fScreenPosMult   = 2.0f * (0.5f - m_f3rdPersonCHairMultY);
    float fUpMult    = fScreenPosMult * (1.0f / CDraw::GetAspectRatio()) * std::tan(fScreenAngle);

    vecSource        = m_aCams[m_nActiveCam].m_vecSource;
    vecTarget        = m_aCams[m_nActiveCam].m_vecFront;

    if (m_aCams[m_nActiveCam].m_nMode == eCamMode::MODE_TWOPLAYER_IN_CAR_AND_SHOOTING) {
        m_aCams[m_nActiveCam].Get_TwoPlayer_AimVector(vecTarget);
    } else {
        vecTarget += m_aCams[m_nActiveCam].m_vecUp * fUpMult;
        vecTarget += CrossProduct(m_aCams[m_nActiveCam].m_vecFront, m_aCams[m_nActiveCam].m_vecUp) * fRightMult;

        vecTarget.Normalise();
    }

    float fGunVec = DotProduct(vecGunMuzzle - vecSource, vecTarget);
    vecSource += fGunVec * vecTarget;

    vecTarget = vecSource + fRange * vecTarget;

    return true;
}

// 0x514B80
float CCamera::CalculateGroundHeight(eGroundHeightType type) {
    static auto& lastCalcCamPos    = StaticRef<CVector>(0xB70034);
    static auto& exactGroundHeight = StaticRef<float>(0xB70030);
    static auto& bbTopZ            = StaticRef<float>(0xB7002C);
    static auto& bbBottomZ         = StaticRef<float>(0xB70028);

    const auto& camPos = GetPosition();

    // Possibly update the positions (If the camera has moved enough)
    const auto CheckDelta = [](float d) { return std::abs(d) > 20.f; };
    if (CheckDelta(lastCalcCamPos.x - camPos.x) || CheckDelta(lastCalcCamPos.y - camPos.y) || CheckDelta(lastCalcCamPos.z - camPos.z)) { // Check if there's enough of a delta
        CColPoint cp;
        CEntity* hitEntity;
        if (CWorld::ProcessVerticalLine({ camPos.x, camPos.y, 1000.f }, -1000.f, cp, hitEntity, true, false, false, false, true)) {
            const auto& hitEntPos = hitEntity->GetPosition();
            const auto& hitBB = hitEntity->GetColModel()->GetBoundingBox();

            exactGroundHeight = cp.m_vecPoint.z;

            bbTopZ = hitEntPos.z + hitBB.m_vecMax.z;

            const auto bbsz = hitBB.GetSize();
            bbBottomZ = std::max(
                0.f,
                bbsz.x > 120.f || bbsz.y > 120.f
                    ? exactGroundHeight
                    : hitEntPos.z + hitBB.m_vecMin.z
            );
        }
        lastCalcCamPos = camPos;
    }

    switch (type) {
    case eGroundHeightType::ENTITY_BB_TOP:       return bbTopZ;
    case eGroundHeightType::EXACT_GROUND_HEIGHT: return exactGroundHeight;
    case eGroundHeightType::ENTITY_BB_BOTTOM:    return bbBottomZ;
    default:                                     NOTSA_UNREACHABLE();
    }
}

// 0x514D60
void CCamera::CalculateFrustumPlanes(bool bForMirror) {
    float halfFovRad = CDraw::ms_fFOV * (PI / 360.0f);
    float cosHalfFov = std::cos(halfFovRad);
    float sinHalfFov = std::sin(halfFovRad);

    m_avecFrustumNormals[0].x = cosHalfFov;
    m_avecFrustumNormals[0].y = -sinHalfFov;
    m_avecFrustumNormals[0].z = 0.0f;

    m_avecFrustumNormals[1].x = -cosHalfFov;
    m_avecFrustumNormals[1].y = -sinHalfFov;
    m_avecFrustumNormals[1].z = 0.0f;

    float aspect = (float)RsGlobal.maximumHeight / (float)RsGlobal.maximumWidth;
    float aspectSin = aspect * sinHalfFov;
    float aspectCos = aspect * cosHalfFov;

    m_avecFrustumNormals[2].x = 0.0f;
    m_avecFrustumNormals[2].y = -aspectSin;
    m_avecFrustumNormals[2].z = -aspectCos;

    m_avecFrustumNormals[3].x = 0.0f;
    m_avecFrustumNormals[3].y = -aspectSin;
    m_avecFrustumNormals[3].z = aspectCos;

    const CVector& camPos = GetPosition();
    if (!bForMirror) {
        TransformVectors(m_avecFrustumWorldNormals, 4, m_mCameraMatrix, m_avecFrustumNormals);
        for (int32 i = 0; i < 4; i++) {
            m_fFrustumPlaneOffsets[i] = DotProduct(m_avecFrustumWorldNormals[i], camPos);
        }
    } else {
        TransformVectors(m_avecFrustumWorldNormals_Mirror, 4, m_mCameraMatrix, m_avecFrustumNormals);
        for (int32 i = 0; i < 4; i++) {
            m_fFrustumPlaneOffsets_Mirror[i] = DotProduct(m_avecFrustumWorldNormals_Mirror[i], camPos);
        }
    }
}

// 0x5150E0
void CCamera::CalculateDerivedValues(bool bForMirror, bool bUpdateOrientation) {
    m_mMatInverse = Invert(m_mCameraMatrix);

    CalculateFrustumPlanes(bForMirror);

    auto& forward = m_mCameraMatrix.GetForward();
    if (forward.x == 0.0f && forward.y == 0.0f) {
        forward.x = 0.0001f; // Just enough to stop it crashing - and the orientation is left alone
    } else if (bUpdateOrientation) {
        m_fOrientation = std::atan2(forward.x, forward.y);
    }

    // The front vector normalised as if z were 0, used by other bits of code
    m_fCamFrontXNorm = forward.x;
    m_fCamFrontYNorm = forward.y;

    const auto len = std::sqrt(m_fCamFrontXNorm * m_fCamFrontXNorm + m_fCamFrontYNorm * m_fCamFrontYNorm);
    if (len == 0.0f) {
        m_fCamFrontXNorm = 1.0f; // `m_fCamFrontYNorm` is already 0 here
    } else {
        m_fCamFrontXNorm /= len;
        m_fCamFrontYNorm /= len;
    }
}

// 0x516B20
void CCamera::ImproveNearClip(CVehicle* vehicle, CPed* ped, CVector* source, CVector* targPosn) {
    auto& cam = m_aCams[m_nActiveCam];

    if ((*source - *targPosn).Magnitude() > 10.0f) {
        const auto nearClip = 1.0f * gCurDistForCam;
        if (nearClip > RwCameraGetNearClipPlane(Scene.m_pRwCamera)) {
            RwCameraSetNearClipPlane(Scene.m_pRwCamera, nearClip);
        }
    }

    if (vehicle) {
        if (vehicle->m_nVehicleSubType == VEHICLE_TYPE_HELI || vehicle->m_nVehicleSubType == VEHICLE_TYPE_PLANE) {
            if (gCurDistForCam > 0.3f) {
                const auto groundZ = CalculateGroundHeight(eGroundHeightType::ENTITY_BB_BOTTOM);
                if (cam.m_vecSource.z - groundZ > 10.0f) {
                    const auto nearClip = std::min(5.0f * gCurDistForCam, (*source - *targPosn).Magnitude() * 0.1f);
                    if (nearClip > RwCameraGetNearClipPlane(Scene.m_pRwCamera)) {
                        RwCameraSetNearClipPlane(Scene.m_pRwCamera, nearClip);
                    }
                }
            } else if (vehicle->m_nVehicleSubType == VEHICLE_TYPE_HELI) {
                RwCameraSetNearClipPlane(Scene.m_pRwCamera, 0.1f);
            }
        }
    } else if (ped) {
        if (!ped->bIsStanding) {
            auto bUsingParachute = ped->GetIntelligence()->GetUsingParachute();

            if (const auto* swimTask = ped->GetIntelligence()->GetTaskSwim()) {
                // The original reuses the register holding the parachute flag, so from here on
                // the "allowed to clip in close" test is the swim state instead
                bUsingParachute = swimTask->m_nSwimState == eSwimState::SWIM_UNDERWATER_SPRINTING;

                auto waterLevel = 0.0f;
                const auto gotWaterLevel = CWaterLevel::GetWaterLevel(source->x, source->y, source->z, waterLevel, false, nullptr);

                if (gotWaterLevel && std::abs(waterLevel - source->z) < 0.3f) {
                    RwCameraSetNearClipPlane(Scene.m_pRwCamera, 0.1f); // Right at the surface
                } else if (bUsingParachute && m_nPedZoom == 1) {
                    RwCameraSetNearClipPlane(Scene.m_pRwCamera, 0.1f);
                }
            } else if (bUsingParachute || ped->GetIntelligence()->GetTaskJetPack()) {
                if (GetRoughDistanceToGround() > 10.0f) {
                    const auto nearClip = std::min(2.0f * gCurDistForCam, (*source - *targPosn).Magnitude() * 0.3f);
                    if (nearClip > RwCameraGetNearClipPlane(Scene.m_pRwCamera)) {
                        RwCameraSetNearClipPlane(Scene.m_pRwCamera, nearClip);
                    }
                }
            }
        } else {
            // Pull the near plane in until it clears every collision sphere of the ped we're behind
            const auto radiusTerm = std::sin(DegreesToRadians(90.0f - cam.m_fFOV * 0.5f)) * gLastRadiusUsedInCollisionPreventionOfCamera;

            auto* const mi = CModelInfo::GetModelInfo(ped->m_nModelIndex)->AsPedModelInfoPtr();
            auto nearest = 1000000.0f;

            if (ped->GetRpClump()) {
                auto* const hitColModel = mi->AnimatePedColModelSkinnedWorld(ped->GetRpClump());
                const auto dotFrontSource = DotProduct(cam.m_vecFront, cam.m_vecSource);

                if (hitColModel && hitColModel->m_pColData && hitColModel->m_pColData->m_pSpheres) {
                    const auto* const spheres = hitColModel->m_pColData->m_pSpheres;
                    const auto numSpheres = std::min<uint16>(12, hitColModel->m_pColData->m_nNumSpheres);
                    for (auto i = 0; i < numSpheres; i++) {
                        const auto& sphere = spheres[i];

                        auto d = DotProduct(sphere.m_vecCenter, cam.m_vecFront) - dotFrontSource - sphere.m_fRadius;
                        if (sphere.m_Surface.m_nPiece == ePedPieceTypes::PED_PIECE_HEAD) {
                            d -= 1.0f * sphere.m_fRadius;
                        }
                        nearest = std::min(nearest, d);
                    }
                }
            }

            auto nearClip = std::clamp(std::min(nearest, radiusTerm), 0.02f, 0.3f);
            nearClip = (float)(int32)(nearClip * 100.0f) * 0.01f; // Truncate to 2 decimals

            RwCameraSetNearClipPlane(Scene.m_pRwCamera, nearClip);
        }
    }

    auto nearestPed = 1000000.0f;
    CCollision::CheckPeds(*source, cam.m_vecFront, nearestPed);
}

static CMatrix& preMirrorMat = *(CMatrix*)0xB6FE40;

// 0x51A560
void CCamera::SetCameraUpForMirror() {
    preMirrorMat = m_mCameraMatrix;
    m_mCameraMatrix = m_mMatMirror;
    CopyCameraMatrixToRWCam(true);
    CalculateDerivedValues(true, false);
}

// 0x51A5A0
void CCamera::RestoreCameraAfterMirror() {
    SetMatrix(preMirrorMat);
    CopyCameraMatrixToRWCam(true);
    CalculateDerivedValues(false, false);
}

// 0x51A5D0
bool CCamera::ConeCastCollisionResolve(const CVector& pos, const CVector& lookAt, CVector& outDest, float radius, float minDist, float& outDist) {
    if (pos == lookAt) {
        return false;
    }

    if (CCollision::CameraConeCastVsWorldCollision(CSphere{ lookAt, radius }, CSphere{ pos, radius }, outDist, minDist)) {
        outDest = lerp(lookAt, pos, outDist);
        return true;
    } else {
        outDest = pos;
        outDist = 1.f;
        return false;
    }
}

// 0x51E560
bool CCamera::TryToStartNewCamMode(int32 camSequence) {
    constexpr auto DISTCAM1 = 20.0f, DISTCAM1_RELEASE = 40.0f, DISTCAM1_RELEASE_TOO_CLOSE = 4.5f;
    constexpr auto DISTCAM2 = 16.0f, DISTCAM2_RELEASE = 29.0f, DISTCAM2_RELEASE_TOO_CLOSE = 2.0f;
    constexpr auto DISTCAM3 = 30.0f;
    constexpr auto DISTCAM5 = 30.0f;
    constexpr auto HELI_CAM_DIST_AWAY_ONE   = 34.0f, HELI_CAM_MAX_DIST_AWAY_ONE   = 44.0f, HELI_CAM_DIST_TOO_CLOSE_ONE   = 3.0f;
    constexpr auto HELI_CAM_DIST_AWAY_TWO   = 30.0f, HELI_CAM_MAX_DIST_AWAY_TWO   = 50.0f, HELI_CAM_DIST_TOO_CLOSE_TWO   = 3.0f;
    constexpr auto HELI_CAM_DIST_AWAY_THREE = 25.0f, HELI_CAM_MAX_DIST_AWAY_THREE = 50.0f, HELI_CAM_DIST_TOO_CLOSE_THREE = 2.0f;
    constexpr auto HELI_CAM_DIST_AWAY_FOUR  = 23.0f, HELI_CAM_MAX_DIST_AWAY_FOUR  = 57.0f, HELI_CAM_DIST_TOO_CLOSE_FOUR  = 1.0f;
    constexpr auto HELI_CAM_MAX_DIST_AWAY_FIVE = 36.0f, HELI_CAM_DIST_TOO_CLOSE_FIVE = 2.0f;
    constexpr auto NORMAL_NEAR_CLIP = 0.3f;

    static auto& fHeliMinHeightAboveWater     = StaticRef<float>(0x8CC8C0); // 1.0
    static auto& fSeaplaneMinHeightAboveWater = StaticRef<float>(0x8CC8C4); // -2.0

    //! Seaplane is classed as a boat, but the camera treats it like a car
    const auto PlayerInBoat = [&] {
        const auto* veh = FindPlayerVehicle();
        return veh && veh->m_nVehicleType == VEHICLE_TYPE_BOAT && m_pTargetEntity->m_nModelIndex != MODEL_SKIMMER;
    };
    const auto LineOfSightClear = [](const CVector& from, const CVector& to) {
        return CWorld::GetIsLineOfSightClear(from, to, true, false, false, false, false, false, false);
    };
    const auto FlatSpeedDir = [] {
        auto dir = FindPlayerSpeed();
        dir.z = 0.0f;
        dir.Normalise();
        return dir;
    };
    //! Keep the camera clear of the water, further up for a seaplane
    const auto ClampAboveWater = [&](CVector& coors, float surfaceZ) {
        const auto* veh = FindPlayerVehicle();
        const auto  minHeight = veh && veh->m_nVehicleType == VEHICLE_TYPE_BOAT
            ? fSeaplaneMinHeightAboveWater
            : fHeliMinHeightAboveWater;
        coors.z = std::max(coors.z, surfaceZ + minHeight);
    };
    const auto StartFixedCam = [&](const CVector& coors) {
        SetCamPositionForFixedMode(coors, CVector{});
        TakeControl(FindPlayerEntity(), MODE_FIXED, eSwitchType::JUMPCUT, 2);
        return !CameraObscuredByWaterLevel();
    };
    const auto StartDWCam = [&](bool ok, eCamMode mode) {
        if (!ok) {
            return false;
        }
        TakeControl(FindPlayerEntity(), mode, eSwitchType::JUMPCUT, 2);
        return !CameraObscuredByWaterLevel();
    };

    // A wheel cam on `veh` is only worth starting if the camera's spot is actually clear
    const auto TryWheelCam = [&](CVehicle* veh) {
        auto camPos = veh->GetMatrix().TransformVector(CVector{ -1.4f, -2.3f, 0.3f }) + veh->GetPosition();
        if (!LineOfSightClear(veh->GetPosition(), camPos)) {
            return false;
        }
        TakeControl(veh, MODE_WHEELCAM, eSwitchType::JUMPCUT, 2);
        return true;
    };

    //! Cop car behind us, roughly pointing the same way
    const auto FindChasingCop = [&](bool requirePhysics) -> CVehicle* {
        for (auto& veh : GetVehiclePool()->GetAllValid()) {
            if (!veh.IsAutomobile() || &veh == FindPlayerVehicle() || !veh.vehicleFlags.bIsLawEnforcer) {
                continue;
            }
            if (requirePhysics && veh.GetStatus() != STATUS_PHYSICS) {
                continue;
            }
            const auto diff = veh.GetPosition() - FindPlayerCoors();
            if ((FindPlayerCoors() - veh.GetPosition()).Magnitude() >= 30.0f) {
                continue;
            }
            const auto& playerFwd = FindPlayerVehicle()->GetMatrix().GetForward();
            if (diff.x * playerFwd.x + diff.y * playerFwd.y >= 0.0f) {
                continue;
            }
            const auto& copFwd = veh.GetMatrix().GetForward();
            if (copFwd.x * playerFwd.x + copFwd.y * playerFwd.y > 0.8f) {
                return &veh;
            }
        }
        return nullptr;
    };

    switch (camSequence) {
    case MOVIECAM0: { // Wheel cam on the player
        auto* const veh = FindPlayerVehicle();
        if (!veh || PlayerInBoat() || veh->m_nModelIndex == MODEL_RHINO) {
            return false;
        }
        return TryWheelCam(veh);
    }
    case MOVIECAM1: { // Fixed cam just above the road, quite far away
        auto coors = FindPlayerCoors();
        const auto dir = FlatSpeedDir();
        coors += dir * DISTCAM1 + CVector{ dir.y * 3.0f, -dir.x * 3.0f, 0.0f };

        if (PlayerInBoat()) {
            return false;
        }

        bool found;
        if (auto z = CWorld::FindGroundZFor3DCoord({ coors.x, coors.y, coors.z + 5.0f }, &found); found) {
            coors.z = z + 1.5f;
        } else if (z = CWorld::FindRoofZFor3DCoord(coors.x, coors.y, coors.z - 5.0f, &found); found) {
            coors.z = z + 1.5f;
        }

        if (!LineOfSightClear(FindPlayerCoors(), coors)) {
            return false;
        }

        auto diff = FindPlayerCoors() - coors;
        diff.z = 0.0f;
        if (diff.Magnitude() > DISTCAM1_RELEASE && DotProduct(FindPlayerSpeed(), diff) > 0.0f) {
            return false;
        }
        if (diff.Magnitude() < DISTCAM1_RELEASE_TOO_CLOSE) {
            return false;
        }
        return StartFixedCam(coors);
    }
    case MOVIECAM2: { // Fixed right in front, just above the road
        if (PlayerInBoat()) {
            return false;
        }

        auto coors = FindPlayerCoors();
        const auto dir = FlatSpeedDir();
        coors += dir * DISTCAM2 + CVector{ dir.y * 2.5f, -dir.x * 2.5f, 0.0f };

        bool found;
        if (auto z = CWorld::FindGroundZFor3DCoord({ coors.x, coors.y, coors.z + 5.0f }, &found); found) {
            coors.z = z + 0.5f;
        } else if (z = CWorld::FindRoofZFor3DCoord(coors.x, coors.y, coors.z - 5.0f, &found); found) {
            coors.z = z + 0.5f;
        }

        if (!LineOfSightClear(FindPlayerCoors(), coors)) {
            return false;
        }

        auto diff = FindPlayerCoors() - coors;
        diff.z = 0.0f;
        if (diff.Magnitude() > DISTCAM2_RELEASE && DotProduct(FindPlayerSpeed(), diff) > 0.0f) {
            return false;
        }
        if (diff.Magnitude() < DISTCAM2_RELEASE_TOO_CLOSE) {
            return false;
        }

        SetCamPositionForFixedMode(coors, CVector{});
        TakeControl(FindPlayerEntity(), MODE_FIXED, eSwitchType::JUMPCUT, 2);
        RwCameraSetNearClipPlane(Scene.m_pRwCamera, NORMAL_NEAR_CLIP * 0.5f);
        return !CameraObscuredByWaterLevel();
    }
    case MOVIECAM3: { // Fixed cam quite high up
        auto coors = FindPlayerCoors();
        const auto dir = FlatSpeedDir();
        coors += dir * DISTCAM3 + CVector{ dir.y * 8.0f, -dir.x * 8.0f, 0.0f };
        coors.z += 16.0f;

        if (!LineOfSightClear(FindPlayerCoors(), coors)) {
            return false;
        }

        SetCamPositionForFixedMode(coors, CVector{});
        TakeControl(FindPlayerEntity(), MODE_FIXED, eSwitchType::JUMPCUT, 2);
        if (CameraObscuredByWaterLevel()) {
            return false;
        }
        RwCameraSetNearClipPlane(Scene.m_pRwCamera, NORMAL_NEAR_CLIP * 0.5f);
        return true;
    }
    case MOVIECAM5: { // Fixed cam just above the roofs of cars
        auto coors = FindPlayerCoors();
        const auto dir = FlatSpeedDir();
        coors += dir * DISTCAM5 + CVector{ -dir.y * 6.0f, dir.x * 6.0f, 0.0f };

        bool found;
        if (auto z = CWorld::FindGroundZFor3DCoord({ coors.x, coors.y, coors.z + 5.0f }, &found); found) {
            coors.z = z + 3.5f;
        } else if (z = CWorld::FindRoofZFor3DCoord(coors.x, coors.y, coors.z - 5.0f, &found); found) {
            coors.z = z + 3.5f;
        }

        if (!LineOfSightClear(FindPlayerCoors(), coors)) {
            return false;
        }
        return StartFixedCam(coors);
    }
    case MOVIECAM6: // Standard camera
        TakeControl(FindPlayerEntity(), MODE_1STPERSON, eSwitchType::JUMPCUT, 2);
        return true;
    case MOVIECAM7: { // Chase cam - try to find a copper chasing us
        if (FindPlayerPed()->GetWantedLevel() < eWantedLevel::WANTED_LEVEL_1 || !FindPlayerVehicle() || PlayerInBoat()) {
            return false;
        }
        auto* const cop = FindChasingCop(true);
        if (!cop) {
            return false;
        }
        TakeControl(cop, MODE_CAM_ON_A_STRING, eSwitchType::JUMPCUT, 2);
        return !CameraObscuredByWaterLevel();
    }
    case MOVIECAM8: { // Wheel cam on a copper chasing us
        if (FindPlayerPed()->GetWantedLevel() < eWantedLevel::WANTED_LEVEL_1 || !FindPlayerVehicle() || PlayerInBoat()) {
            return false;
        }
        auto* const cop = FindChasingCop(false);
        if (!cop) {
            return false;
        }
        return TryWheelCam(cop) && !CameraObscuredByWaterLevel();
    }
    case MOVIECAM15: { // Straight in front of the player
        if (!FindPlayerVehicle()) {
            return false;
        }
        auto coors = FindPlayerCoors();
        coors += FlatSpeedDir() * HELI_CAM_DIST_AWAY_ONE;
        coors.z = FindPlayerCoors().z + 0.5f;
        if (FindPlayerVehicle()->m_nVehicleType == VEHICLE_TYPE_BOAT) {
            coors.z += 1.0f;
        }

        if (!LineOfSightClear(FindPlayerCoors(), coors)) {
            return false;
        }

        const auto camDiff = FindPlayerCoors() - coors;
        if (camDiff.Magnitude() > HELI_CAM_MAX_DIST_AWAY_ONE && DotProduct(FindPlayerSpeed(), camDiff) > 0.0f) {
            return false;
        }
        if (camDiff.Magnitude() < HELI_CAM_DIST_TOO_CLOSE_ONE) {
            return false;
        }
        return StartFixedCam(coors);
    }
    case MOVIECAM16: { // Underneath, in front of and to the side of the player
        if (!FindPlayerVehicle()) {
            return false;
        }
        auto coors = FindPlayerCoors();
        auto dir   = FlatSpeedDir();
        const auto ang = CGeneral::GetATanOfXY(dir.x, dir.y) + DegreesToRadians(60.0f);
        dir += CVector{ std::cos(ang), std::sin(ang), 0.0f };
        dir.Normalise();
        coors += dir * HELI_CAM_DIST_AWAY_TWO;
        coors.z = FindPlayerCoors().z - 5.5f;

        bool  found = false;
        float surfaceZ = CWorld::FindRoofZFor3DCoord(coors.x, coors.y, coors.z, &found);
        if (found) {
            coors.z = surfaceZ + 0.5f;
        } else if (CWaterLevel::GetWaterLevelNoWaves(coors, &surfaceZ)) {
            ClampAboveWater(coors, surfaceZ);
        }

        if (!LineOfSightClear(FindPlayerCoors(), coors)) {
            return false;
        }

        const auto camDiff = FindPlayerCoors() - coors;
        if (camDiff.Magnitude() > HELI_CAM_MAX_DIST_AWAY_TWO || camDiff.Magnitude() < HELI_CAM_DIST_TOO_CLOSE_TWO) {
            return false;
        }
        return StartFixedCam(coors);
    }
    case MOVIECAM17: { // Behind the player, slightly to the side
        if (!FindPlayerVehicle()) {
            return false;
        }
        auto coors = FindPlayerCoors();
        auto dir   = FlatSpeedDir();
        const auto ang = CGeneral::GetATanOfXY(dir.x, dir.y) + DegreesToRadians(190.0f);
        dir += CVector{ std::cos(ang), std::sin(ang), 0.0f };
        dir.Normalise();
        coors += dir * HELI_CAM_DIST_AWAY_THREE;
        coors.z = FindPlayerCoors().z - 1.0f;

        bool  found = false;
        float surfaceZ = CWorld::FindRoofZFor3DCoord(coors.x, coors.y, coors.z, &found);
        if (found) {
            coors.z = surfaceZ + 0.5f;
        } else if (CWaterLevel::GetWaterLevelNoWaves(coors, &surfaceZ)) {
            ClampAboveWater(coors, surfaceZ);
        }

        if (!LineOfSightClear(FindPlayerCoors(), coors)) {
            return false;
        }

        const auto camDiff = FindPlayerCoors() - coors;
        if (camDiff.Magnitude() > HELI_CAM_MAX_DIST_AWAY_THREE && DotProduct(FindPlayerSpeed(), camDiff) > 0.0f) {
            return false;
        }
        if (camDiff.Magnitude() < HELI_CAM_DIST_TOO_CLOSE_THREE) {
            return false;
        }
        return StartFixedCam(coors);
    }
    case MOVIECAM18: { // Directly above the player
        auto coors = FindPlayerCoors();
        const auto* veh = FindPlayerVehicle();
        coors.z += veh && veh->m_nVehicleType == VEHICLE_TYPE_BOAT ? HELI_CAM_DIST_AWAY_FOUR : -HELI_CAM_DIST_AWAY_FOUR;

        auto dir = FindPlayerSpeed();
        const auto ang = CGeneral::GetATanOfXY(dir.x, dir.y) + DegreesToRadians(145.0f);
        dir += CVector{ std::cos(ang), std::sin(ang), 0.0f };
        dir.z = 0.0f;
        dir.Normalise();
        coors += dir * 15.0f;

        bool found = false;
        // NOTE: the original compares the returned Z against `TRUE` instead of checking `found` - kept as is
        float groundZ = CWorld::FindGroundZFor3DCoord(coors, &found);
        if (groundZ == 1.0f) {
            if (coors.z < groundZ) {
                coors.z = groundZ + 0.5f;
            }
        } else if (CWaterLevel::GetWaterLevelNoWaves(coors, &groundZ)) {
            ClampAboveWater(coors, groundZ);
        }

        if (!LineOfSightClear(FindPlayerCoors(), coors)) {
            return false;
        }

        const auto camDiff = FindPlayerCoors() - coors;
        if (camDiff.Magnitude() > HELI_CAM_MAX_DIST_AWAY_FOUR || camDiff.Magnitude() < HELI_CAM_DIST_TOO_CLOSE_FOUR) {
            return false;
        }
        return StartFixedCam(coors);
    }
    case MOVIECAM19: { // Directly above the player, to the side
        auto coors = FindPlayerCoors();
        const auto* veh = FindPlayerVehicle();
        coors.z += veh && veh->m_nVehicleType == VEHICLE_TYPE_BOAT ? 4.0f : -1.0f;

        auto dir = FindPlayerSpeed();
        const auto ang = CGeneral::GetATanOfXY(dir.x, dir.y) + DegreesToRadians(28.0f);
        dir += CVector{ std::cos(ang), std::sin(ang), 0.0f };
        dir.z = 0.0f;
        dir.Normalise();
        coors += dir * 12.5f;

        bool found = false;
        // NOTE: same `== TRUE` quirk as `MOVIECAM18`
        float groundZ = CWorld::FindGroundZFor3DCoord(coors, &found);
        if (groundZ == 1.0f) {
            if (coors.z < groundZ) {
                coors.z = groundZ + 0.5f;
            }
        } else if (CWaterLevel::GetWaterLevelNoWaves(coors, &groundZ)) {
            ClampAboveWater(coors, groundZ);
        }

        if (!LineOfSightClear(FindPlayerCoors(), coors)) {
            return false;
        }

        const auto camDiff = FindPlayerCoors() - coors;
        if (camDiff.Magnitude() > HELI_CAM_MAX_DIST_AWAY_FIVE || camDiff.Magnitude() < HELI_CAM_DIST_TOO_CLOSE_FIVE) {
            return false;
        }
        return StartFixedCam(coors);
    }
    case MOVIECAM20: return StartDWCam(m_aCams[m_nActiveCam].Process_DW_HeliChaseCam(true),    MODE_DW_HELI_CHASE);
    case MOVIECAM21: return StartDWCam(m_aCams[m_nActiveCam].Process_DW_CamManCam(true),       MODE_DW_CAM_MAN);
    case MOVIECAM22: return StartDWCam(m_aCams[m_nActiveCam].Process_DW_BirdyCam(true),        MODE_DW_BIRDY);
    case MOVIECAM23: return StartDWCam(m_aCams[m_nActiveCam].Process_DW_PlaneSpotterCam(true), MODE_DW_PLANE_SPOTTER);
    case MOVIECAM24:
    case MOVIECAM25:
        TheCamera.m_bUseNearClipScript = false;
        return false;
    case MOVIECAMPLANE1: return StartDWCam(m_aCams[m_nActiveCam].Process_DW_PlaneCam1(true), MODE_DW_PLANECAM1);
    case MOVIECAMPLANE2: return StartDWCam(m_aCams[m_nActiveCam].Process_DW_PlaneCam2(true), MODE_DW_PLANECAM2);
    case MOVIECAMPLANE3: return StartDWCam(m_aCams[m_nActiveCam].Process_DW_PlaneCam3(true), MODE_DW_PLANECAM3);
    case CAM_ON_A_STRING_LAST_RESORT:
        TakeControl(FindPlayerEntity(), MODE_CAM_ON_A_STRING, eSwitchType::JUMPCUT, 2);
        return true;
    default:
        return false;
    }
}

// 0x520190
bool CCamera::CameraColDetAndReact(CVector* source, CVector* target) {
    // Lowest point of the col model of the vehicle we're ignoring, cached per model
    static int32   cachedModelIndex = 0;    // 0xB700F0
    static float   cachedLowestZ    = 0.0f; // 0xB700EC
    static CVector lastSourceCoors{};       // 0xB700DC, the original guards its init with bit 0 of 0xB700E8

    const auto delta = *source - *target;
    auto rad = delta.Magnitude() * gpCamColVars[0] * 0.2939f; // gRadiusScalarForLengthToVehicle @ 0x8CCB90

    if (gCurCamColVars >= 10) {
        if (auto* const ignore = CWorld::pIgnoreEntity) {
            // Note: the sub type, not the type - a vortex counts as a car here
            if (ignore->GetIsTypeVehicle() && ignore->AsVehicle()->m_nVehicleSubType == VEHICLE_TYPE_AUTOMOBILE) {
                if (ignore->m_nModelIndex != cachedModelIndex) {
                    cachedLowestZ = 100.0f;
                    if (const auto* colData = ignore->GetColModel()->m_pColData) {
                        for (const auto& sphere : colData->GetSpheres()) {
                            cachedLowestZ = std::min(cachedLowestZ, sphere.m_vecCenter.z - sphere.m_fRadius);
                        }
                    }
                    cachedModelIndex = ignore->m_nModelIndex;
                }

                const auto heightOverChassis = DotProduct(*target - ignore->GetPosition(), ignore->GetMatrix().GetUp()) - cachedLowestZ;
                rad = std::min(rad, std::min(std::max(heightOverChassis, 0.2f), gpCamColVars[1]));
            } else {
                const auto half = CModelInfo::GetModelInfo(ignore->m_nModelIndex)->m_pColModel->GetBoundingBox().GetSize() * 0.5f;
                rad = std::min(rad, std::min(std::min({ half.x, half.y, half.z }), gpCamColVars[1]));
            }
        }
    }

    rad = std::min(rad, gpCamColVars[1]);
    rad = std::max(rad, 0.65f); // Yes, in this order - 0x5203EF then 0x520404

    auto minDist = gpCamColVars[2];
    if (gCurCamColVars <= 9) {
        minDist = (gCurCamColVars > 3 ? 0.3f : 0.18f) / delta.Magnitude();
    }

    auto bIsBike = false;
    if (gCurCamColVars >= 10) {
        auto* const ignore = CWorld::pIgnoreEntity;
        if (ignore && ignore->GetIsTypeVehicle() && ignore->AsVehicle()->m_nVehicleType == VEHICLE_TYPE_BIKE) {
            minDist = 0.05f;
            bIsBike = true;
        }
    }

    gLastRadiusUsedInCollisionPreventionOfCamera = rad;

    auto     outDist = 0.0f;
    CVector  outDest;
    const auto bColResolved = ConeCastCollisionResolve(*source, *target, outDest, rad, minDist, outDist);

    if (bColResolved && outDist <= gpCamColVars[3]) {
        RwCameraSetNearClipPlane(Scene.m_pRwCamera, gpCamColVars[4]);
    }

    if (outDist < gCurDistForCam) {
        gCurDistForCam = outDist;
    } else {
        // Only creep back out while the camera itself is holding still
        if ((*source - lastSourceCoors).SquaredMagnitude() > 0.01f * 0.01f) {
            gCurDistForCam += std::min(
                (outDist - gCurDistForCam) * (CTimer::ms_fTimeStep * gpCamColVars[5]),
                0.05f
            );
        }
        lastSourceCoors = *source;
    }

    gCurDistForCam = std::min(gCurDistForCam, 1.0f);

    *source = *target + delta * gCurDistForCam;

    if (bIsBike && gCurDistForCam < 0.5f) {
        RwCameraSetNearClipPlane(Scene.m_pRwCamera, 0.05f);
    }

    return bColResolved;
}

// 0x527FA0
void CCamera::CamControl() {
    static eCamMode GameMode;
    static eCamMode ReqMode;
    static int16    ReqMinZoom, ReqMaxZoom;
    static int      LastPedState;
    static eCamMode ThePickedArrestMode;
    static bool     PlaceForFixedWhenSniperFound = false;

    bool     NeedToDoAJumpCutForGameCam          = false;
    bool     TargetIsBoat                        = false;
    bool     IsInArrestCam                       = false;
    bool     PretendPlayerInAGarage              = false;
    CVector  CenterOfGarage;
    CVector  CenterOfDoorOne;
    CVector  CenterOfDoorTwo;
    CVector  PlayerPosition;
    CVector  DoorFront;
    CVector  TheCamPosition;
    CEntity* pDoorWeAreUnder                   = nullptr;
    int      DoorWeAreUnder                    = 0;
    float    DistanceCarToDoorOne              = 0.0f;
    float    DistanceCarToDoorTwo              = 0.0f;
    float    CamDistanceAwayForGarage          = 13.0f;
    int      ModeAtStartOfCamControl           = m_aCams[m_nActiveCam].m_nMode;
    m_bObbeCinematicPedCamOn                   = false;
    m_bObbeCinematicCarCamOn                   = false;
    m_bUseTransitionBeta                       = false;
    m_bUseSpecialFovTrain                      = false;
    m_bJustCameOutOfGarage                     = false;
    m_bTargetJustCameOffTrain                  = false;
    m_bInATunnelAndABigVehicle                 = false;
    m_bJustJumpedOutOf1stPersonBecauseOfTarget = false;
    JustGoneIntoObbeCamera                     = false;

    if ((m_aCams[m_nActiveCam].m_pCamTargetEntity == nullptr) && (m_pTargetEntity == nullptr)) {
        m_pTargetEntity->CleanUpOldReference(&m_pTargetEntity);
        m_pTargetEntity = CWorld::Players[CWorld::PlayerInFocus].m_pPed;
        m_pTargetEntity->RegisterReference(&m_pTargetEntity);
    }

    assert(m_pTargetEntity != nullptr && "No Target set yet for camera");
    m_nZoneCullFrameNumWereAt++;

    if (m_nZoneCullFrameNumWereAt > m_nCheckCullZoneThisNumFrames) {
        m_nZoneCullFrameNumWereAt = 1;
    }
    if (m_nZoneCullFrameNumWereAt == m_nCheckCullZoneThisNumFrames) {
        m_bCullZoneChecksOn = true;
    } else {
        m_bCullZoneChecksOn = false;
    }

    if (m_bCullZoneChecksOn == true) {
        if (CCullZones::CamCloseInForPlayer()) {
            m_bFailedCullZoneTestPreviously = true;
        } else {
            m_bFailedCullZoneTestPreviously = false;
        }
    }

    if (m_bLookingAtPlayer == true) {
        CPad::GetPad(0)->DisablePlayerControls &= ~1; // EnableControlsCamera
        FindPlayerPed()->m_bIsVisible = true;
    }

    // `GTA_IDLECAM` is off in the retail PC build: neither this block nor `CanWeBeInIdleMode`
    // appear in 0x527FA0, and the source's `CanWeBeInIdleMode` opens with a plain `return false`.
    m_bIdleOn = false;

    if ((!CTimer::m_UserPause && !CTimer::m_CodePause)) {
        if (m_bIdleOn == false) {
            float CarTargetCloseHeightOffset = 0.0f;
            float PedTargetCloseHeightOffset = 0.0f;

            if (m_bTargetJustBeenOnTrain == true) {
                bool  NeedToRestoreCamera   = false;
                float TrainSpeed            = 0.0f;
                bool  ComingToAStopAStation = true;

                if ((m_pTargetEntity->GetIsTypeVehicle()) == false) {
                    NeedToRestoreCamera = true;

                } else {
                    if (m_pTargetEntity->AsVehicle()->m_nVehicleType != VEHICLE_TYPE_TRAIN) {
                        NeedToRestoreCamera = true;
                    }
                }

                if (NeedToRestoreCamera) {
                    Restore();
                    m_bTargetJustCameOffTrain = true;
                    m_bTargetJustBeenOnTrain  = false;
                    SetWideScreenOff();
                }
            }

            if (m_pTargetEntity->GetIsTypeVehicle()) {
                if (CamModeToRestore > 0) {
                    bool bCollisionCheckIsStillBad = false;
                    if (bCollisionCheckIsStillBad) {
                    } else {
                        ReqMode          = static_cast<eCamMode>(CamModeToRestore);
                        CamModeToRestore = -1;
                    }
                }

                if (m_pTargetEntity->AsVehicle()->m_nVehicleType == VEHICLE_TYPE_TRAIN) {
                    ReqMode = MODE_BEHINDCAR;
                } else {
                    if (m_pTargetEntity->AsVehicle()->m_nVehicleType == VEHICLE_TYPE_BOAT) {
                        if ((m_pTargetEntity->GetModelIndex()) != MODEL_SKIMMER) {
                            TargetIsBoat = true;
                        }
                    }

                    if ((CPad::GetPad(0)->CycleCameraModeUpJustDown() || CPad::GetPad(0)->sub_540530())
                        && CReplay::Mode != MODE_PLAYBACK && !m_bWideScreenOn && !m_bFailedCullZoneTestPreviously
                        && (m_bLookingAtPlayer == true || m_nWhoIsInControlOfTheCamera == 2) && !CGameLogic::IsCoopGameGoingOn()) {
                        if (CPad::GetPad(0)->CycleCameraModeUpJustDown()) {
                            m_nCarZoom -= 1;
                        } else {
                            m_nCarZoom += 1;
                        }

                        if (m_nCarZoom > 5) {
                            m_nCarZoom = 0;
                        } else if (m_nCarZoom < 0) {
                            m_nCarZoom = 5;
                        }

                        if (m_nCarZoom == 4) {
                            if (CPad::GetPad(0)->CycleCameraModeUpJustDown()) {
                                m_nCarZoom = ZOOM_THREE;
                            } else {
                                m_nCarZoom = 5;
                            }
                        } else if (m_nCarZoom == 0 && m_bDisableFirstPersonInCar) {
                            if (CPad::GetPad(0)->CycleCameraModeUpJustDown()) {
                                m_nCarZoom = 5;
                            } else {
                                m_nCarZoom = ZOOM_ONE;
                            }
                        }
                    }
                    if (m_bFailedCullZoneTestPreviously && m_nCarZoom != 4 && m_nCarZoom != 0) {
                        ReqMode = MODE_CAM_ON_A_STRING;
                    }
                    int32 iVehicleType = m_pTargetEntity->AsVehicle()->m_nVehicleType;

                    if (iVehicleType == VEHICLE_TYPE_BOAT) {
                        if ((m_pTargetEntity->GetModelIndex()) == MODEL_SKIMMER) {
                            iVehicleType = VEHICLE_TYPE_AUTOMOBILE;
                        }
                    }

                    if (iVehicleType == VEHICLE_TYPE_AUTOMOBILE || iVehicleType == VEHICLE_TYPE_BIKE) {
                        CAttributeZone* pZoneLikeAGarage = nullptr;
                        if (iVehicleType == VEHICLE_TYPE_BIKE && CCullZones::CamStairsForPlayer()) {
                            if ((pZoneLikeAGarage = CCullZones::FindZoneWithStairsAttributeForPlayer()) != nullptr) {
                                PretendPlayerInAGarage = true;
                            }
                        }

                        if (CGarages::IsPointInAGarageCameraZone(m_pTargetEntity->GetPosition()) || PretendPlayerInAGarage) {
                            CObject *pDoor1, *pDoor2;
                            if ((m_bGarageFixedCamPositionSet == false && m_bLookingAtPlayer == true) || m_nWhoIsInControlOfTheCamera == 2) {
                                if (m_pToGarageWeAreIn != nullptr || pZoneLikeAGarage != nullptr) {
                                    PlayerPosition = m_pTargetEntity->GetPosition();

                                    if (m_pToGarageWeAreIn != nullptr) {
                                        m_pToGarageWeAreIn->FindDoorsWithGarage(&pDoor1, &pDoor2);

                                        if (pDoor1 != nullptr) {
                                            DoorWeAreUnder       = 1;
                                            CenterOfDoorOne.x    = pDoor1->GetPosition().x;
                                            CenterOfDoorOne.y    = pDoor1->GetPosition().y;
                                            CenterOfDoorOne.z    = 0;
                                            PlayerPosition.z     = 0;
                                            DistanceCarToDoorOne = (PlayerPosition - CenterOfDoorOne).Magnitude();
                                        } else if (pDoor2 != nullptr) {
                                            if (DoorWeAreUnder == 1) {
                                                CenterOfDoorTwo.x    = pDoor2->GetPosition().x;
                                                CenterOfDoorTwo.y    = pDoor2->GetPosition().y;
                                                CenterOfDoorTwo.z    = 0;
                                                PlayerPosition.z     = 0;
                                                DistanceCarToDoorTwo = (PlayerPosition - CenterOfDoorTwo).Magnitude();
                                                if (DistanceCarToDoorTwo < DistanceCarToDoorOne) {
                                                    DoorWeAreUnder = 2;
                                                }
                                            } else {
                                                DoorWeAreUnder = 2;
                                            }
                                        } else {
                                            DoorWeAreUnder    = 1;
                                            CenterOfDoorOne.x = m_pTargetEntity->GetPosition().x;
                                            CenterOfDoorOne.y = m_pTargetEntity->GetPosition().y;
                                            CenterOfDoorTwo.z = 0;
                                        }
                                    } else {
                                        assert(PretendPlayerInAGarage && "Cullzone garage stuff gone wrong");
                                        DoorWeAreUnder  = 1;
                                        CenterOfDoorOne = m_aCams[m_nActiveCam].m_vecSource;
                                        CenterOfGarage  = PlayerPosition;
                                        if ((CenterOfGarage - CenterOfDoorOne).Magnitude2D() > 15.0f) {
                                            bool    bFoundExitDirn  = true;
                                            CVector vecTestExitDirn = m_pTargetEntity->GetPosition() - CenterOfGarage;
                                            vecTestExitDirn.z       = 0.0f;
                                            vecTestExitDirn.Normalise();

                                            float fMaxZoneDim      = (float)std::max(std::abs(pZoneLikeAGarage->zoneDef.m_vec1X) + std::abs(pZoneLikeAGarage->zoneDef.m_vec2X), std::abs(pZoneLikeAGarage->zoneDef.m_vec1Y) + std::abs(pZoneLikeAGarage->zoneDef.m_vec2Y));

                                            CVector vecTestExitPos = m_pTargetEntity->GetPosition() + 2.0f * fMaxZoneDim * vecTestExitDirn;
                                            if (!CWorld::GetIsLineOfSightClear(m_pTargetEntity->GetPosition(), vecTestExitPos, true, false, false, false, false, false, true)) {
                                                vecTestExitPos = m_pTargetEntity->GetPosition() - 2.0f * fMaxZoneDim * vecTestExitDirn;
                                                if (!CWorld::GetIsLineOfSightClear(m_pTargetEntity->GetPosition(), vecTestExitPos, true, false, false, false, false, false, true)) {
                                                    bFoundExitDirn = false;
                                                }
                                            }

                                            if (bFoundExitDirn) {
                                                CenterOfDoorOne = vecTestExitPos;
                                            }
                                        }
                                    }

                                    //now find the normal out the way
                                    if (m_pToGarageWeAreIn != nullptr) {
                                        CenterOfGarage = CVector((m_pToGarageWeAreIn->m_fLeftCoord + m_pToGarageWeAreIn->m_fRightCoord) / 2, (m_pToGarageWeAreIn->m_fFrontCoord + m_pToGarageWeAreIn->m_fBackCoord) / 2, 0);
                                    } else {
                                        CenterOfDoorOne.z = 0.0f;
                                        if (pZoneLikeAGarage == nullptr) {
                                            CenterOfGarage = CVector(m_pTargetEntity->GetPosition().x, m_pTargetEntity->GetPosition().y, 0);
                                        }
                                    }

                                    if (DoorWeAreUnder == 1) {
                                        DoorFront = CenterOfDoorOne - CenterOfGarage;
                                    } else {
                                        assert(DoorWeAreUnder == 2 && "Get Mark Something worng with garages");
                                        DoorFront = CenterOfDoorTwo - CenterOfGarage;
                                    }

                                    bool  GroundFound;
                                    float GroundZ  = 0.0f;
                                    float HeightUp = 3.1f;
                                    PlayerPosition = m_pTargetEntity->GetPosition();
                                    GroundZ        = CWorld::FindGroundZFor3DCoord({ PlayerPosition.x, PlayerPosition.y, PlayerPosition.z }, &GroundFound);
                                    //assert(GroundFound && "Could find the ground are we in the sky?");
                                    if (GroundFound == false) //this shouldn't be happening but been having
                                    //lots of complaints about the game hanging so this is the best alternative
                                    {
                                        GroundZ = PlayerPosition.z - 0.20f;
                                    }
                                    DoorFront.z = 0;
                                    DoorFront.Normalise();

                                    if (DoorWeAreUnder == 1) {
                                        if (m_pToGarageWeAreIn == nullptr && pZoneLikeAGarage != nullptr) {
                                            CamDistanceAwayForGarage = 3.75f + 0.7f * std::max(std::abs(pZoneLikeAGarage->zoneDef.m_vec1X) + std::abs(pZoneLikeAGarage->zoneDef.m_vec2X), std::abs(pZoneLikeAGarage->zoneDef.m_vec1Y) + std::abs(pZoneLikeAGarage->zoneDef.m_vec2Y));
                                            TheCamPosition           = CenterOfGarage + CamDistanceAwayForGarage * DoorFront;
                                        } else {
                                            TheCamPosition = CenterOfDoorOne + (CamDistanceAwayForGarage * DoorFront);
                                        }
                                    } else {
                                        assert(DoorWeAreUnder == 2);
                                        TheCamPosition = CenterOfDoorTwo + (CamDistanceAwayForGarage * DoorFront);
                                    }
                                    TheCamPosition.z = GroundZ + HeightUp;

                                    CVector door1(0, 0, 0);
                                    CVector door2(0, 0, 0);
                                    CVector directionOfDoor1;
                                    CVector directionOfDoor2;
                                    CVector doorDir;

                                    if (pDoor1) {
                                        door1            = pDoor1->m_pDummyObject->GetPosition();
                                        directionOfDoor1 = pDoor1->m_pDummyObject->GetMatrix().GetRight();
                                        doorDir          = directionOfDoor1;
                                    }

                                    if (pDoor2) {
                                        door2            = pDoor2->m_pDummyObject->GetPosition();
                                        directionOfDoor2 = pDoor2->m_pDummyObject->GetMatrix().GetRight();
                                        doorDir          = directionOfDoor2;
                                    }

                                    CVector posForCamera;
                                    if (pDoor1) {
                                        if (pDoor2) {
                                            posForCamera = (door2 - door1) * 0.5f + door1;
                                        } else {
                                            posForCamera = door1;
                                        }
                                    } else if (pDoor2) {
                                        posForCamera = door2;
                                    } else {
                                        posForCamera = PlayerPosition;
                                        doorDir      = posForCamera - CenterOfGarage;
                                        doorDir.z    = 0.0f;
                                        doorDir.Normalise();
                                        //										assert(false); // See me  - DW
                                    }

                                    static float gLenFromDoor    = -10.0f;
                                    static float gHeightFromDoor = 2.0f;
                                    TheCamPosition               = posForCamera + doorDir * gLenFromDoor;
                                    TheCamPosition.z += gHeightFromDoor;

                                    SetCamPositionForFixedMode((TheCamPosition), CVector(0, 0, 0));
                                    m_bGarageFixedCamPositionSet = true;
                                }
                            }

                            if ((CGarages::CameraShouldBeOutside() || PretendPlayerInAGarage) && m_bGarageFixedCamPositionSet == true && (m_bLookingAtPlayer == true || m_nWhoIsInControlOfTheCamera == 2)) {
                                if (m_pToGarageWeAreIn != nullptr || pZoneLikeAGarage != nullptr) {
                                    ReqMode             = MODE_FIXED;
                                    m_bPlayerIsInGarage = true;
                                }
                            } else {
                                if (m_bPlayerIsInGarage) {
                                    m_bJustCameOutOfGarage = true;
                                    m_bPlayerIsInGarage    = false;
                                }
                                ReqMode = MODE_CAM_ON_A_STRING;
                            }
                        } else {
                            if (m_bPlayerIsInGarage) {
                                m_bJustCameOutOfGarage = true;
                                m_bPlayerIsInGarage    = false;
                            }
                            pDoorWeAreUnder              = nullptr;
                            m_bGarageFixedCamPositionSet = false;
                            ReqMode                      = MODE_CAM_ON_A_STRING;
                        }
                    } else if (iVehicleType == VEHICLE_TYPE_BOAT) {
                        ReqMode = MODE_BEHINDBOAT;
                    }

                    eVehicleAppearance VehicleApperance = ((CVehicle*)(m_pTargetEntity))->GetVehicleAppearance();
                    int                PositionInArray  = 0;
                    GetArrPosForVehicleType(static_cast<eVehicleType>(VehicleApperance), PositionInArray);

                    if ((m_nCarZoom == 0) && (!m_bPlayerIsInGarage)) {
                        m_fCarZoomBase = 0.0f;
                        ReqMode        = MODE_1STPERSON;
                    } else if (m_nCarZoom == ZOOM_ONE) {
                        m_fCarZoomBase = ZOOM_ONE_DISTANCE[PositionInArray];
                    } else if (m_nCarZoom == ZOOM_TWO) {
                        m_fCarZoomBase = ZOOM_TWO_DISTANCE[PositionInArray];
                    } else if (m_nCarZoom == ZOOM_THREE) {
                        m_fCarZoomBase = ZOOM_THREE_DISTANCE[PositionInArray];
                    }

                    if ((m_nCarZoom == 4) && (!m_bPlayerIsInGarage)) {
                        m_fCarZoomBase = 1.0f;
                    }

                    if (m_fCarZoomTotal == 0.0f) {
                        m_fCarZoomTotal = m_fCarZoomBase;
                    }

                    if (m_bUseScriptZoomValueCar == true) {
                        if (m_fCarZoomSmoothed < m_fCarZoomValueScript) {
                            m_fCarZoomSmoothed = std::min(m_fCarZoomValueScript, m_fCarZoomSmoothed + CTimer::GetTimeStep() * 0.12f);
                        } else {
                            m_fCarZoomSmoothed = std::max(m_fCarZoomValueScript, m_fCarZoomSmoothed - CTimer::GetTimeStep() * 0.12f);
                        }
                    } else if (m_bFailedCullZoneTestPreviously) {
                        float ZoomedInVal          = -0.65f;
                        CarTargetCloseHeightOffset = 0.65f;
                        if (m_fCarZoomSmoothed < ZoomedInVal) {
                            m_fCarZoomSmoothed = std::min(ZoomedInVal, m_fCarZoomSmoothed + CTimer::GetTimeStep() * 0.12f);
                        } else {
                            m_fCarZoomSmoothed = std::max(ZoomedInVal, m_fCarZoomSmoothed - CTimer::GetTimeStep() * 0.12f);
                        }
                    } else {
                        if (m_fCarZoomSmoothed < m_fCarZoomBase) {
                            m_fCarZoomSmoothed = std::min(m_fCarZoomBase, m_fCarZoomSmoothed + CTimer::GetTimeStep() * 0.12f);
                        } else {
                            m_fCarZoomSmoothed = std::max(m_fCarZoomBase, m_fCarZoomSmoothed - CTimer::GetTimeStep() * 0.12f);
                        }
                        //this is to indicate that we have just come out of first person mode
                        if ((m_nCarZoom == ZOOM_THREE) && (m_fCarZoomBase == 0.0f)) {
                            m_fCarZoomSmoothed = m_fCarZoomBase;
                        }
                    }

                    WellBufferMe(CarTargetCloseHeightOffset, m_aCams[m_nActiveCam].m_fCloseInCarHeightOffset, m_aCams[m_nActiveCam].m_fCloseInCarHeightOffsetSpeed, 0.1f, 0.25f, false);
                }
            } else if (m_pTargetEntity->GetIsTypePed()) {
                assert(m_pTargetEntity->GetIsTypePed() && "What in gods green earth is going on?");
                if ((CPad::GetPad(0)->CycleCameraModeUpJustDown() || CPad::GetPad(0)->sub_540530())
                    && CReplay::Mode != MODE_PLAYBACK && !m_bWideScreenOn && !m_bFailedCullZoneTestPreviously && !m_bFirstPersonBeingUsed
                    && (m_bLookingAtPlayer == true || m_nWhoIsInControlOfTheCamera == 2) && !CGameLogic::IsCoopGameGoingOn()) {
                    if (CPad::GetPad(0)->CycleCameraModeUpJustDown()) {
                        m_nPedZoom -= 1;
                    } else {
                        m_nPedZoom += 1;
                    }

                    if (m_nPedZoom > ZOOM_THREE) {
                        m_nPedZoom = ZOOM_ONE;
                    } else if (m_nPedZoom < ZOOM_ONE) {
                        m_nPedZoom = ZOOM_THREE;
                    }
                }

                ReqMode = MODE_FOLLOWPED;

                if (((m_bLookingAtPlayer == true) || (m_bEnable1rstPersonCamCntrlsScript)) && (m_pTargetEntity->GetIsTypePed()) && ((m_bWideScreenOn == false) || (m_bEnable1rstPersonCamCntrlsScript))
                    && m_aCams[0].Using3rdPersonMouseCam() == false) {
                    if (m_bFirstPersonBeingUsed) {
                        if (CPad::GetPad(0)->GetPedWalkLeftRight() || CPad::GetPad(0)->GetPedWalkUpDown()
                            || CPad::GetPad(0)->NewState.ButtonSquare || CPad::GetPad(0)->NewState.ButtonTriangle
                            || CPad::GetPad(0)->NewState.ButtonCross || CPad::GetPad(0)->NewState.ButtonCircle
                            || CPad::GetPad(0)->NewState.Select) {
                            m_bFirstPersonBeingUsed = false;
                        }

                        else {
                            if ((CTimer::GetTimeInMS() - m_nFirstPersonCamLastInputTime) > MAX_TIME_IN_FIRST_PERSON_NO_INPUT) {
                                m_bFirstPersonBeingUsed = false;
                            } else if (CPad::GetPad(0)->GetEnterTargeting()) {
                                m_bFirstPersonBeingUsed                    = false;
                                m_bJustJumpedOutOf1stPersonBecauseOfTarget = true;
                            }
                        }
                    }
                } else {
                    m_bFirstPersonBeingUsed = false;
                }

                if (FindPlayerPed()->IsPedInControl() == false || FindPlayerPed()->GetPlayerData()->m_fMoveBlendRatio > 0.0f) {
                    m_bFirstPersonBeingUsed = false;
                }

                if (m_bFirstPersonBeingUsed) {
                    ReqMode = MODE_1STPERSON;
                    CPad::GetPad(0)->DisablePlayerControls |= 1; // DisableControlsCamera
                }

                switch (m_nPedZoom) {
                case ZOOM_ONE:
                    m_fPedZoomBase = m_aCams[m_nActiveCam].m_fTargetZoomGroundOne; //ZOOM_PED_ONE_DISTANCE;
                    break;
                case ZOOM_THREE:
                    m_fPedZoomBase = m_aCams[m_nActiveCam].m_fTargetZoomGroundThree; //ZOOM_PED_THREE_DISTANCE;
                    break;
                case ZOOM_TWO:
                default:
                    m_fPedZoomBase = m_aCams[m_nActiveCam].m_fTargetZoomGroundTwo; //ZOOM_PED_TWO_DISTANCE;
                    break;
                }

                if (m_bUseScriptZoomValuePed == true) {
                    if (m_fPedZoomSmoothed < m_fPedZoomValueScript) {
                        m_fPedZoomSmoothed = std::min(m_fPedZoomValueScript, m_fPedZoomSmoothed + CTimer::GetTimeStep() * 0.12f);
                    } else {
                        m_fPedZoomSmoothed = std::max(m_fPedZoomValueScript, m_fPedZoomSmoothed - CTimer::GetTimeStep() * 0.12f);
                    }
                } else if (m_bFailedCullZoneTestPreviously) {
                    static float PedZoomedInVal = 0.5f;
                    PedTargetCloseHeightOffset  = 0.7f;
                    if (m_fPedZoomSmoothed < PedZoomedInVal) {
                        m_fPedZoomSmoothed = std::min(PedZoomedInVal, m_fPedZoomSmoothed + CTimer::GetTimeStep() * 0.12f);
                    } else {
                        m_fPedZoomSmoothed = std::max(PedZoomedInVal, m_fPedZoomSmoothed - CTimer::GetTimeStep() * 0.12f);
                    }
                } else {
                    if (m_fPedZoomSmoothed < m_fPedZoomBase) {
                        m_fPedZoomSmoothed = std::min(m_fPedZoomBase, m_fPedZoomSmoothed + CTimer::GetTimeStep() * 0.12f);
                    } else {
                        m_fPedZoomSmoothed = std::max(m_fPedZoomBase, m_fPedZoomSmoothed - CTimer::GetTimeStep() * 0.12f);
                    }

                    if (m_nPedZoom == ZOOM_THREE && m_fPedZoomBase == 0.0f) {
                        m_fPedZoomSmoothed = m_fPedZoomBase;
                    }
                }

                WellBufferMe(PedTargetCloseHeightOffset, m_aCams[m_nActiveCam].m_fCloseInPedHeightOffset, m_aCams[m_nActiveCam].m_fCloseInPedHeightOffsetSpeed, 0.1f, 0.025f, false);

                CAttributeZone* pZoneLikeAGarage = nullptr;
                if (CCullZones::CamStairsForPlayer()) {
                    if ((pZoneLikeAGarage = CCullZones::FindZoneWithStairsAttributeForPlayer()) != nullptr) {
                        PretendPlayerInAGarage = true;
                    }
                }
                if (CGarages::IsPointInAGarageCameraZone(m_pTargetEntity->GetPosition()) || PretendPlayerInAGarage) {
                    if ((m_bGarageFixedCamPositionSet == false) && (m_bLookingAtPlayer == true)) {
                        if ((m_pToGarageWeAreIn != nullptr) || (PretendPlayerInAGarage)) {
                            if (m_pToGarageWeAreIn != nullptr) {
                                PlayerPosition = m_pTargetEntity->GetPosition();

                                CObject *pDoor1, *pDoor2;
                                m_pToGarageWeAreIn->FindDoorsWithGarage(&pDoor1, &pDoor2);

                                if (pDoor1 != nullptr) {
                                    DoorWeAreUnder       = 1;
                                    CenterOfDoorOne.x    = pDoor1->GetPosition().x;
                                    CenterOfDoorOne.y    = pDoor1->GetPosition().y;
                                    CenterOfDoorOne.z    = 0;
                                    PlayerPosition.z     = 0;
                                    DistanceCarToDoorOne = (PlayerPosition - CenterOfDoorOne).Magnitude();
                                } else if (pDoor2 != nullptr) {
                                    if (DoorWeAreUnder == 1) {
                                        CenterOfDoorTwo.x    = pDoor2->GetPosition().x;
                                        CenterOfDoorTwo.y    = pDoor2->GetPosition().y;
                                        CenterOfDoorTwo.z    = 0;
                                        PlayerPosition.z     = 0;
                                        DistanceCarToDoorTwo = (PlayerPosition - CenterOfDoorTwo).Magnitude();
                                        if (DistanceCarToDoorTwo < DistanceCarToDoorOne) {
                                            DoorWeAreUnder = 2;
                                        }
                                    } else {
                                        DoorWeAreUnder = 2;
                                    }
                                } else {
                                    DoorWeAreUnder    = 1;
                                    CenterOfDoorOne.x = m_pTargetEntity->GetPosition().x;
                                    CenterOfDoorOne.y = m_pTargetEntity->GetPosition().y;
                                    CenterOfDoorTwo.z = 0;
                                }
                            } else {
                                assert(PretendPlayerInAGarage && "Cullzone garage stuff gone wrong");
                                DoorWeAreUnder  = 1;
                                CenterOfDoorOne = m_aCams[m_nActiveCam].m_vecSource;

                                if (pZoneLikeAGarage) {
                                    CenterOfGarage = pZoneLikeAGarage->zoneDef.FindCenter();
                                    if ((CenterOfGarage - CenterOfDoorOne).Magnitude2D() > 15.0f) {
                                        bool    bFoundExitDirn  = true;
                                        CVector vecTestExitDirn = m_pTargetEntity->GetPosition() - CenterOfGarage;
                                        vecTestExitDirn.z       = 0.0f;
                                        vecTestExitDirn.Normalise();

                                        float fMaxZoneDim      = (float)std::max(std::abs(pZoneLikeAGarage->zoneDef.m_vec1X) + std::abs(pZoneLikeAGarage->zoneDef.m_vec2X), std::abs(pZoneLikeAGarage->zoneDef.m_vec1Y) + std::abs(pZoneLikeAGarage->zoneDef.m_vec2Y));

                                        CVector vecTestExitPos = m_pTargetEntity->GetPosition() + 2.0f * fMaxZoneDim * vecTestExitDirn;
                                        if (!CWorld::GetIsLineOfSightClear(m_pTargetEntity->GetPosition(), vecTestExitPos, true, false, false, false, false, false, true)) {
                                            vecTestExitPos = m_pTargetEntity->GetPosition() - 2.0f * fMaxZoneDim * vecTestExitDirn;
                                            if (!CWorld::GetIsLineOfSightClear(m_pTargetEntity->GetPosition(), vecTestExitPos, true, false, false, false, false, false, true)) {
                                                bFoundExitDirn = false;
                                            }
                                        }

                                        if (bFoundExitDirn) {
                                            CenterOfDoorOne = vecTestExitPos;
                                        }
                                    }
                                }
                            }

                            if (m_pToGarageWeAreIn != nullptr) {
                                CenterOfGarage = CVector((m_pToGarageWeAreIn->m_fLeftCoord + m_pToGarageWeAreIn->m_fRightCoord) / 2, (m_pToGarageWeAreIn->m_fFrontCoord + m_pToGarageWeAreIn->m_fBackCoord) / 2, 0);
                            } else {
                                CenterOfDoorOne.z = 0.0f;
                                if (pZoneLikeAGarage == nullptr) {
                                    CenterOfGarage = CVector(m_pTargetEntity->GetPosition().x, m_pTargetEntity->GetPosition().y, 0);
                                }
                            }

                            if (DoorWeAreUnder == 1) {
                                DoorFront = CenterOfDoorOne - CenterOfGarage;
                            } else {
                                assert(DoorWeAreUnder == 2 && "Get Mark Something worng with garages");
                                DoorFront = CenterOfDoorTwo - CenterOfGarage;
                            }

                            bool  GroundFound;
                            float GroundZ  = 0.0f;
                            float HeightUp = 3.1f;
                            PlayerPosition = m_pTargetEntity->GetPosition();
                            GroundZ        = CWorld::FindGroundZFor3DCoord({ PlayerPosition.x, PlayerPosition.y, PlayerPosition.z }, &GroundFound);
                            if (GroundFound == false) {
                                GroundZ = PlayerPosition.z - 0.20f;
                            }
                            DoorFront.z = 0;

                            if (DoorWeAreUnder == 1) {
                                if ((m_pToGarageWeAreIn == nullptr) && (PretendPlayerInAGarage)) {
                                    DoorFront.Normalise();
                                    if (pZoneLikeAGarage) {
                                        CamDistanceAwayForGarage = 3.75f + 0.7f * std::max(std::abs(pZoneLikeAGarage->zoneDef.m_vec1X) + std::abs(pZoneLikeAGarage->zoneDef.m_vec2X), std::abs(pZoneLikeAGarage->zoneDef.m_vec1Y) + std::abs(pZoneLikeAGarage->zoneDef.m_vec2Y));
                                        TheCamPosition           = CenterOfGarage + CamDistanceAwayForGarage * DoorFront;
                                    } else {
                                        CamDistanceAwayForGarage = 3.75f;
                                        TheCamPosition           = CenterOfDoorOne + CamDistanceAwayForGarage * DoorFront;
                                    }
                                } else {
                                    DoorFront.Normalise();
                                    TheCamPosition = CenterOfDoorOne + (CamDistanceAwayForGarage * DoorFront);
                                }
                            } else {
                                assert(DoorWeAreUnder == 2);
                                DoorFront.Normalise();
                                TheCamPosition = CenterOfDoorTwo + (CamDistanceAwayForGarage * DoorFront);
                            }

                            if ((m_nPedZoom == 4) && (!PretendPlayerInAGarage)) {
                                TheCamPosition = CenterOfGarage;
                                TheCamPosition.z += FindPlayerPed()->GetPosition().z + 2.1f;
                                if (m_pToGarageWeAreIn != nullptr) {
                                    if (TheCamPosition.z > m_pToGarageWeAreIn->m_fRightCoord) {
                                        TheCamPosition.z = m_pToGarageWeAreIn->m_fRightCoord;
                                    }
                                }
                            } else {
                                TheCamPosition.z = GroundZ + HeightUp;
                            }

                            SetCamPositionForFixedMode((TheCamPosition), CVector(0, 0, 0));
                            m_bGarageFixedCamPositionSet = true;
                        }

                        CVector door1(0, 0, 0);
                        CVector door2(0, 0, 0);
                        CVector directionOfDoor1;
                        CVector directionOfDoor2;
                        CVector doorDir;
                        if (m_pToGarageWeAreIn != nullptr) {
                            CObject *pDoor1, *pDoor2;
                            m_pToGarageWeAreIn->FindDoorsWithGarage(&pDoor1, &pDoor2);

                            if (pDoor1) {
                                door1            = pDoor1->m_pDummyObject->GetPosition();
                                directionOfDoor1 = pDoor1->m_pDummyObject->GetMatrix().GetRight();
                                doorDir          = directionOfDoor1;
                            }

                            if (pDoor2) {
                                door2            = pDoor2->m_pDummyObject->GetPosition();
                                directionOfDoor2 = pDoor2->m_pDummyObject->GetMatrix().GetRight();
                                doorDir          = directionOfDoor2;
                            }
                            CVector posForCamera;
                            if (pDoor1) {
                                if (pDoor2) {
                                    posForCamera = (door2 - door1) * 0.5f + door1;
                                } else {
                                    posForCamera = door1;
                                }
                            } else if (pDoor2) {
                                posForCamera = door2;
                            } else {
                                posForCamera = PlayerPosition;
                                doorDir      = posForCamera - CenterOfGarage;
                                doorDir.z    = 0.0f;
                                doorDir.Normalise();
                            }

                            static float gLenFromDoor    = -10.0f;
                            static float gHeightFromDoor = 2.0f;
                            TheCamPosition               = posForCamera + doorDir * gLenFromDoor;
                            TheCamPosition.z += gHeightFromDoor;
                            SetCamPositionForFixedMode((TheCamPosition), CVector(0, 0, 0));
                            m_bGarageFixedCamPositionSet = true;
                        }
                    }

                    if (((CGarages::CameraShouldBeOutside() || (PretendPlayerInAGarage)) && (m_bLookingAtPlayer == true) && (m_bGarageFixedCamPositionSet == true))) {
                        if ((m_pToGarageWeAreIn != nullptr) || (PretendPlayerInAGarage)) {
                            ReqMode             = MODE_FIXED;
                            m_bPlayerIsInGarage = true;
                        }
                    } else {
                        if (m_bPlayerIsInGarage) {
                            m_bJustCameOutOfGarage = true;
                            m_bPlayerIsInGarage    = false;
                        }
                        ReqMode = MODE_FOLLOWPED;
                    }
                } else {
                    if (m_bPlayerIsInGarage) {
                        m_bJustCameOutOfGarage = true;
                        m_bPlayerIsInGarage    = false;
                    }
                    m_bGarageFixedCamPositionSet = false;
                }

                if (!CPad::GetPad(0)->GetTarget() && m_PlayerWeaponMode.m_nMode != MODE_NONE
                    && m_PlayerWeaponMode.m_nMode != MODE_HELICANNON_1STPERSON
                    && m_PlayerWeaponMode.m_nMode != MODE_AIMWEAPON_FROMCAR
                    && m_PlayerWeaponMode.m_nMode != MODE_AIMWEAPON_ATTACHED
                    && !(m_PlayerWeaponMode.m_nMode == MODE_CAMERA && FindPlayerPed()->m_pAttachedTo)) {
                    ClearPlayerWeaponMode();
                }

                if (m_PlayerMode.m_nMode != MODE_NONE) {
                    ReqMode = static_cast<eCamMode>(m_PlayerMode.m_nMode);
                }

                if ((m_PlayerWeaponMode.m_nMode != MODE_NONE) && (PretendPlayerInAGarage == false)) {
                    if (m_PlayerWeaponMode.m_nMode == MODE_SNIPER || m_PlayerWeaponMode.m_nMode == MODE_ROCKETLAUNCHER
                        || m_PlayerWeaponMode.m_nMode == MODE_ROCKETLAUNCHER_HS || m_PlayerWeaponMode.m_nMode == MODE_M16_1STPERSON
                        || m_PlayerWeaponMode.m_nMode == MODE_HELICANNON_1STPERSON || m_PlayerWeaponMode.m_nMode == MODE_SNIPER
                        || m_PlayerWeaponMode.m_nMode == MODE_CAMERA || m_aCams[m_nActiveCam].GetWeaponFirstPersonOn()) {
                        if ((CWorld::Players[CWorld::PlayerInFocus].m_pPed->GetPedState()) == PEDSTATE_SEEK_CAR) {
                            if ((ReqMode != MODE_TOP_DOWN_PED) && (!(m_aCams[m_nActiveCam].GetWeaponFirstPersonOn()))) //if we are in top down or 1rst person mode want to stay in it
                            {
                                ReqMode = MODE_FOLLOWPED;
                            } else {
                                ReqMode = static_cast<eCamMode>(m_PlayerWeaponMode.m_nMode);
                            }
                        } else {
                            ReqMode = static_cast<eCamMode>(m_PlayerWeaponMode.m_nMode);
                        }
                    } else if (ReqMode != MODE_TOP_DOWN_PED) {
                        float        Crim_Player_Cam_Angle         = 0.0f;
                        float        CurrentDistOnGround           = 0.0f;
                        float        DistanceOnGround              = 0.0f;
                        float        Length                        = 0.0f;
                        float        CrimToPlayerDist              = 0.0f;
                        float        PlayerToCamDistance           = 0.0f;
                        static float MinDistBetweenPlayerDeadCrim  = 3.0f; //used for when the target is deadied
                        static float MinDistBetweenPlayerAliveCrim = 1.5f; //3.0f;
                        static float MaxDistBetweenPlayerCrim      = 4.0f;
                        static float SpecialFixedCamDist           = 4.0f;
                        static float MaxPlayerToCameraDist         = 10.0f;

                        float   PlayerToCrimAngle                  = 0.0f;
                        float   PlayerToCamAngle                   = 0.0f;
                        bool    TargetDyingOrDead                  = false;
                        CVector PlayerPos                          = m_pTargetEntity->GetPosition();
                        CVector PlayerToCrimVector                 = m_vecAimingTargetCoors - PlayerPos;
                        CVector PlayerToCamVector                  = m_aCams[m_nActiveCam].m_vecSource - PlayerPos;

                        if ((m_bPlayerIsInGarage == false) || (PretendPlayerInAGarage == false)) //we only want to go into syhpon filter esque modes
                                                                                                 //if not in a garage
                        {
                            float DistToUseRePedStatus = 0.0f;
                            if (FindPlayerPed()->m_pTargetedObject != nullptr || FindPlayerPed()->GetPlayerData()->m_bFreeAiming) {
                                if (FindPlayerPed()->m_pTargetedObject && FindPlayerPed()->m_pTargetedObject->GetIsTypePed()
                                    && (((CPed*)FindPlayerPed()->m_pTargetedObject)->GetPedState() == PEDSTATE_DEAD || ((CPed*)FindPlayerPed()->m_pTargetedObject)->GetPedState() == PEDSTATE_DIE)) {
                                    TargetDyingOrDead    = true;
                                    DistToUseRePedStatus = MinDistBetweenPlayerDeadCrim;
                                }

                                CrimToPlayerDist             = std::sqrt(PlayerToCrimVector.x * PlayerToCrimVector.x + PlayerToCrimVector.y * PlayerToCrimVector.y);
                                PlayerToCamDistance          = std::sqrt(PlayerToCamVector.x * PlayerToCamVector.x + PlayerToCamVector.y * PlayerToCamVector.y);

                                PlayerToCamAngle             = CGeneral::GetATanOfXY(PlayerToCamVector.x, PlayerToCamVector.y);
                                PlayerToCrimAngle            = CGeneral::GetATanOfXY(PlayerToCrimVector.x, PlayerToCrimVector.y);

                                ReqMode                      = static_cast<eCamMode>(m_PlayerWeaponMode.m_nMode);

                                float SpecialFixedSyphonDist = 0.0f;
                                if (ReqMode == MODE_AIMWEAPON && TargetDyingOrDead
                                    && FindPlayerPed()->m_pTargetedObject != nullptr) // && !FindPlayerPed()->GetWeapon()->IsTypeMelee())
                                {
                                    if (!m_bTransitionState || m_aCams[m_nActiveCam].m_nMode == MODE_SPECIAL_FIXED_FOR_SYPHON) {
                                        if (m_aCams[m_nActiveCam].m_nMode == MODE_SPECIAL_FIXED_FOR_SYPHON && FindPlayerPed()->m_pTargetedObject->GetIsTypePed()) {
                                            DistToUseRePedStatus = MaxDistBetweenPlayerCrim;
                                        }

                                        if (CrimToPlayerDist < DistToUseRePedStatus) {
                                            ReqMode                = MODE_SPECIAL_FIXED_FOR_SYPHON;
                                            SpecialFixedSyphonDist = SpecialFixedCamDist;

                                            if (TargetDyingOrDead) {
                                                if (ReqMode == MODE_SYPHON_CRIM_IN_FRONT) {
                                                    SpecialFixedSyphonDist = 5.0f;
                                                } else {
                                                    SpecialFixedSyphonDist = 5.6f;
                                                }
                                                ReqMode = MODE_SPECIAL_FIXED_FOR_SYPHON;
                                            }
                                        }
                                    }
                                }
                                if (ReqMode == MODE_SPECIAL_FIXED_FOR_SYPHON) {
                                    if (PlaceForFixedWhenSniperFound == false) {
                                        CVector   TheFixedCamPositionToGoTo;
                                        CColPoint TempCol;
                                        CEntity*  TempHit         = nullptr;

                                        TheFixedCamPositionToGoTo = m_pTargetEntity->GetPosition();
                                        TheFixedCamPositionToGoTo.x += std::cos(PlayerToCamAngle) * SpecialFixedSyphonDist;
                                        TheFixedCamPositionToGoTo.y += std::sin(PlayerToCamAngle) * SpecialFixedSyphonDist;
                                        TheFixedCamPositionToGoTo.z += 1.15f;
                                        //Lets Do A Quick Collision check
                                        if (CWorld::ProcessLineOfSight(m_pTargetEntity->GetPosition(), TheFixedCamPositionToGoTo, TempCol, TempHit, true, false, false, true, false, true, true, false)) {
                                            SetCamPositionForFixedMode(TempCol.m_vecPoint, CVector(0, 0, 0));
                                        } else {
                                            SetCamPositionForFixedMode(TheFixedCamPositionToGoTo, CVector(0, 0, 0));
                                        }

                                        PlaceForFixedWhenSniperFound = true;
                                    }
                                } else {
                                    PlaceForFixedWhenSniperFound = false;
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    if (m_bCooperativeCamMode && CWorld::Players[0].m_pPed && CWorld::Players[1].m_pPed) {
        if (CWorld::Players[0].m_pPed->bInVehicle && CWorld::Players[1].m_pPed->bInVehicle && CWorld::Players[0].m_pPed->m_pVehicle && CWorld::Players[1].m_pPed->m_pVehicle) { // Both players are in a car.
            if (CWorld::Players[0].m_pPed->m_pVehicle == CWorld::Players[1].m_pPed->m_pVehicle) {                                                                               // Both players are in the same car
                if (m_bAllowShootingWith2PlayersInCar) {
                    ReqMode         = m_nModeForTwoPlayersSameCarShootingAllowed;
                    m_pTargetEntity = CWorld::Players[0].m_pPed->m_pVehicle;
                } else {
                    ReqMode         = m_nModeForTwoPlayersSameCarShootingNotAllowed;
                    m_pTargetEntity = CWorld::Players[0].m_pPed->m_pVehicle;
                }
            } else { // Players are each in their own car.
                ReqMode         = m_nModeForTwoPlayersSeparateCars;
                m_pTargetEntity = CWorld::Players[0].m_pPed->m_pVehicle;
            }
        } else {
            ReqMode = m_nModeForTwoPlayersNotBothInCar;
        }
    }

    CPed*       PlayerPed;
    bool        bJustArrested       = false;
    static bool bPreviouslyArrested = false;
    bool        bJustNotArrested    = false;
    PlayerPed                       = CWorld::Players[CWorld::PlayerInFocus].m_pPed;

    if (PlayerPed->GetPedState() == PEDSTATE_ARRESTED) {
        bPreviouslyArrested = true;
    } else if (bPreviouslyArrested) {
        bJustNotArrested    = true;
        bPreviouslyArrested = false;
    }

    if (LastPedState != PEDSTATE_ARRESTED && PlayerPed->GetPedState() == PEDSTATE_ARRESTED) {
        if (m_nCarZoom != 0 || !m_pTargetEntity->GetIsTypeVehicle()) {
            bJustArrested = true;
        }
    } else {
        bJustArrested = false;
    }

    LastPedState = PlayerPed->GetPedState();

    if (bJustArrested == true) {
        ReqMode                               = MODE_ARRESTCAM_ONE;
        ThePickedArrestMode                   = ReqMode;
        m_aCams[m_nActiveCam].m_bResetStatics = true;
    } else if ((PlayerPed->GetPedState()) == PEDSTATE_ARRESTED) {
        ReqMode = ThePickedArrestMode;
    }

    if (CWorld::Players[CWorld::PlayerInFocus].m_pPed->GetPedState() == PEDSTATE_DEAD) {
        m_bObbeCinematicCarCamOn = false;

        if (m_aCams[m_nActiveCam].m_nMode == MODE_PED_DEAD_BABY) {
            ReqMode = MODE_PED_DEAD_BABY;
        } else if (m_aCams[m_nActiveCam].m_nMode == MODE_ARRESTCAM_ONE) {
            ReqMode = MODE_ARRESTCAM_ONE;
        } else {
            bool bUseArrestCam = false;
            if (m_pTargetEntity->GetIsTypePed()) {
                CPed* pTempTargetPed = (CPed*)m_pTargetEntity;

                int       i;
                const int N                = (int)std::size(pTempTargetPed->GetIntelligence()->GetPedScanner().m_apEntities);
                CEntity** ppNearbyEntities = pTempTargetPed->GetIntelligence()->GetPedEntities();
                for (i = 0; i < N; i++) {
                    CEntity* pNearbyEntity = ppNearbyEntities[i];
                    if (pNearbyEntity) {
                        assert(pNearbyEntity->GetType() == ENTITY_TYPE_PED);
                        CPed*                 pNearbyPed  = (CPed*)pNearbyEntity;
                        CTaskSimpleArrestPed* pTaskArrest = (CTaskSimpleArrestPed*)pNearbyPed->GetIntelligence()->FindTaskByType(TASK_SIMPLE_ARREST_PED);
                        if (pTaskArrest && pTaskArrest->m_Ped == FindPlayerPed()) {
                            if ((pNearbyPed->GetPosition() - pTempTargetPed->GetPosition()).Magnitude() < 4.0f) {
                                bUseArrestCam                         = true;
                                ReqMode                               = MODE_ARRESTCAM_ONE;
                                m_aCams[m_nActiveCam].m_bResetStatics = true;
                                break;
                            }
                        }
                    }
                }
            }

            if (bUseArrestCam == false) {
                ReqMode                               = MODE_PED_DEAD_BABY;
                m_aCams[m_nActiveCam].m_bResetStatics = true;
            }
        }
    }

    if (m_bRestoreByJumpCut == true) {
        if (!((ReqMode == MODE_FOLLOWPED) || (ReqMode == MODE_M16_1STPERSON) || (ReqMode == MODE_SNIPER) || (ReqMode == MODE_ROCKETLAUNCHER) || (ReqMode == MODE_ROCKETLAUNCHER_HS) || (ReqMode == MODE_CAMERA) || (ReqMode == MODE_SYPHON) || (ReqMode == MODE_SYPHON_CRIM_IN_FRONT)
              || (ReqMode == MODE_SPECIAL_FIXED_FOR_SYPHON) || (ReqMode == MODE_CAM_ON_A_STRING)
              || (ReqMode == MODE_BEHINDCAR) || (m_bUseMouse3rdPerson))) {
            SetCameraDirectlyBehindForFollowPed_CamOnAString();
        }

        ReqMode                                       = m_nModeToGoTo;
        m_aCams[m_nActiveCam].m_nMode                 = ReqMode;
        m_bJust_Switched                              = true;
        m_aCams[m_nActiveCam].m_bResetStatics         = true;
        m_aCams[m_nActiveCam].m_vecCamFixedModeVector = m_vecFixedModeVector;
        m_aCams[m_nActiveCam].m_pCamTargetEntity->CleanUpOldReference(&m_aCams[m_nActiveCam].m_pCamTargetEntity);
        m_aCams[m_nActiveCam].m_pCamTargetEntity = m_pTargetEntity;
        m_aCams[m_nActiveCam].m_pCamTargetEntity->RegisterReference(&m_aCams[m_nActiveCam].m_pCamTargetEntity);
        m_aCams[m_nActiveCam].m_vecCamFixedModeSource        = m_vecFixedModeSource;
        m_aCams[m_nActiveCam].m_vecCamFixedModeUpOffSet      = m_vecFixedModeUpOffSet;
        m_aCams[m_nActiveCam].m_bCamLookingAtVector          = false;
        m_aCams[m_nActiveCam].m_vecLastAboveWaterCamPosition = m_aCams[(m_nActiveCam + 1) % 2].m_vecLastAboveWaterCamPosition;
        m_bRestoreByJumpCut                                  = false;
        m_aCams[m_nActiveCam].m_bResetStatics                = true;
        m_fCarZoomSmoothed                                   = m_fCarZoomBase;
        m_fPedZoomSmoothed                                   = m_fPedZoomBase;
        m_bTransitionState                                   = false;
        m_bDoingSpecialInterp                                = false;
    }

    if (gbModelViewer) {
        ReqMode = MODE_MODELVIEW;
    }

    bool CanTryObbeCam = true;

    if (m_pTargetEntity != nullptr) {
        if (m_pTargetEntity->GetIsTypeVehicle() == false) {
            if (m_nPedZoom == 5) {
                m_bObbeCinematicPedCamOn = true;
            }
        } else {
            if (m_nCarZoom == 5) {
                m_bObbeCinematicCarCamOn = true;
            }
        }
    }

    if (FindPlayerVehicle() != nullptr) {
        if ((FindPlayerVehicle()->m_nVehicleType) == VEHICLE_TYPE_TRAIN) {
            m_bObbeCinematicCarCamOn = true;
        }
    }

    if (m_pTargetEntity != nullptr) {
        if (m_pTargetEntity->GetIsTypeVehicle()) {
            CPlayerPed* PlayerPed;
            PlayerPed = FindPlayerPed();
            if (PlayerPed) {
                if ((PlayerPed->GetPedState() == PEDSTATE_ARRESTED) || (PlayerPed->GetPedState() == PEDSTATE_DEAD)) {
                    CanTryObbeCam            = false;
                    m_bObbeCinematicPedCamOn = false;
                    if (PlayerPed->GetPedState() == PEDSTATE_ARRESTED) {
                        ReqMode = MODE_ARRESTCAM_ONE;
                    } else if (PlayerPed->GetPedState() == PEDSTATE_DEAD) {
                        ReqMode = MODE_PED_DEAD_BABY;
                    }
                }
            }
        }
    }

    if ((m_bTargetJustBeenOnTrain == true) || (ReqMode == MODE_PED_DEAD_BABY)
        || (ReqMode == MODE_PED_DEAD_BABY) || (ReqMode == MODE_PLAYER_FALLEN_WATER)
        || (ReqMode == MODE_SYPHON_CRIM_IN_FRONT) || (ReqMode == MODE_SYPHON)
        || (ReqMode == MODE_SNIPER) || (ReqMode == MODE_SPECIAL_FIXED_FOR_SYPHON)
        || (ReqMode == MODE_ROCKETLAUNCHER) || (ReqMode == MODE_ROCKETLAUNCHER_HS) || (ReqMode == MODE_PLAYER_FALLEN_WATER)
        || (ReqMode == MODE_ARRESTCAM_ONE) || (ReqMode == MODE_ARRESTCAM_TWO)
        || (ReqMode == MODE_M16_1STPERSON) || (ReqMode == MODE_FIGHT_CAM)
        || (ReqMode == MODE_SNIPER_RUNABOUT) || (ReqMode == MODE_ROCKETLAUNCHER_RUNABOUT) || (ReqMode == MODE_ROCKETLAUNCHER_RUNABOUT_HS)
        || (ReqMode == MODE_M16_1STPERSON_RUNABOUT) || (ReqMode == MODE_FIGHT_CAM_RUNABOUT)
        || (ReqMode == MODE_1STPERSON_RUNABOUT) || (ReqMode == MODE_HELICANNON_1STPERSON)
        || (ReqMode == MODE_CAMERA)
        || (m_nWhoIsInControlOfTheCamera == SCRIPT_CAM_CONTROL) || (m_bJustCameOutOfGarage) || (m_bPlayerIsInGarage)
        || (m_aCams[m_nActiveCam].m_nMode == MODE_PED_DEAD_BABY)) {
        CanTryObbeCam = false;
    }

    if (m_bCinemaCamera) {
        m_bObbeCinematicCarCamOn = true;
        CanTryObbeCam            = true;
        assert(m_pTargetEntity);
        assert(m_pTargetEntity->GetIsTypeVehicle());
    }

    if ((m_bObbeCinematicPedCamOn) && (CanTryObbeCam == true)) {
        //		ProcessObbeCinemaCameraPed();
    } else if ((m_bObbeCinematicCarCamOn) && (CanTryObbeCam == true)) {
        CPostEffects::m_bSpeedFXUserFlagCurrentFrame = false; // no speed blur please....

        if (m_pTargetEntity->GetIsTypeVehicle()) {
            int32 vehicleType = m_pTargetEntity->AsVehicle()->m_nVehicleType;

            if (vehicleType == VEHICLE_TYPE_PLANE) {
                ProcessObbeCinemaCameraPlane();
            } else if ((m_pTargetEntity->AsVehicle()->GetVehicleAppearance() == VEHICLE_APPEARANCE_HELI)) {
                ProcessObbeCinemaCameraHeli();
            } else if (vehicleType == VEHICLE_TYPE_BOAT) {
                ProcessObbeCinemaCameraBoat();
            } else if (vehicleType == VEHICLE_TYPE_TRAIN) {
                ProcessObbeCinemaCameraTrain();
            } else {
                ProcessObbeCinemaCameraCar();
            }
        }
    } else {
        if (m_bPlayerIsInGarage) {
            if (m_bObbeCinematicCarCamOn) {
                NeedToDoAJumpCutForGameCam = true;
            }
        }
        CanTryObbeCam = false;
        DontProcessObbeCinemaCamera();
    }

    if (m_bLookingAtPlayer == true) {
        if ((ReqMode == MODE_TOPDOWN) || (ReqMode == MODE_1STPERSON) || (ReqMode == MODE_TOP_DOWN_PED)) //always jump cut for this
        {
            NeedToDoAJumpCutForGameCam = true;
        } else if ((ReqMode == MODE_CAM_ON_A_STRING) || (ReqMode == MODE_BEHINDBOAT)) {
            switch (m_aCams[m_nActiveCam].m_nMode) {
            case MODE_TOPDOWN:
            case MODE_1STPERSON:
            case MODE_TOP_DOWN_PED:
                NeedToDoAJumpCutForGameCam = true;
                break;
            }
        } else if (ReqMode == MODE_FIXED) {
            if (m_aCams[m_nActiveCam].m_nMode == MODE_TOPDOWN) {
                NeedToDoAJumpCutForGameCam = true;
            }
        }

        if ((ReqMode == MODE_AIMWEAPON || ReqMode == MODE_AIMWEAPON_FROMCAR || ReqMode == MODE_AIMWEAPON_ATTACHED)
            && m_pTargetEntity && m_pTargetEntity->GetIsTypePed()) {
            bool bIsUsingJetPack = false;

            if (m_pTargetEntity->GetIsTypePed() && m_pTargetEntity->AsPed()->GetIntelligence()->GetTaskJetPack()) {
                bIsUsingJetPack = true;
            }

            if (ReqMode == MODE_AIMWEAPON && m_aCams[m_nActiveCam].m_nMode == MODE_FOLLOWPED && !bIsUsingJetPack) {
                float fTargetBeta = 0.0f;
                if (m_pTargetEntity->AsPed()->m_pTargetedObject) {
                    CVector vecAimDelta = m_pTargetEntity->AsPed()->m_pTargetedObject->GetPosition() - m_pTargetEntity->GetPosition();
                    fTargetBeta         = std::atan2(-vecAimDelta.x, vecAimDelta.y) - HALF_PI;
                } else {
                    fTargetBeta = m_pTargetEntity->GetHeading() - HALF_PI;
                }

                if (fTargetBeta > m_aCams[m_nActiveCam].m_fHorizontalAngle + PI) {
                    fTargetBeta -= TWO_PI;
                } else if (fTargetBeta < m_aCams[m_nActiveCam].m_fHorizontalAngle - PI) {
                    fTargetBeta += TWO_PI;
                }

                if (std::abs(fTargetBeta - m_aCams[m_nActiveCam].m_fHorizontalAngle) > DegreesToRadians(MAX_ANGLE_BEFORE_AIMWEAPON_JUMPCUT)) {
                    NeedToDoAJumpCutForGameCam = true;
                } else if ((m_pTargetEntity->GetPosition() - GetMatrix().GetPosition()).Magnitude() > 1.5f * (TheCamera.m_fPedZoomSmoothed + 2.0f)) {
                    NeedToDoAJumpCutForGameCam = true;
                }

                if (TheCamera.m_bUseMouse3rdPerson) {
                    NeedToDoAJumpCutForGameCam = false;
                }
            } else {
                NeedToDoAJumpCutForGameCam = true;
            }
        }

        if ((ReqMode == MODE_TWOPLAYER && m_aCams[m_nActiveCam].m_nMode != MODE_TWOPLAYER_IN_CAR_AND_SHOOTING)
            || (ReqMode == MODE_TWOPLAYER_IN_CAR_AND_SHOOTING && m_aCams[m_nActiveCam].m_nMode != MODE_TWOPLAYER)
            || (m_aCams[m_nActiveCam].m_nMode == MODE_TWOPLAYER && ReqMode != MODE_TWOPLAYER_IN_CAR_AND_SHOOTING)
            || (m_aCams[m_nActiveCam].m_nMode == MODE_TWOPLAYER_IN_CAR_AND_SHOOTING && ReqMode != MODE_TWOPLAYER)) {
            NeedToDoAJumpCutForGameCam = true;
        }

        if (ReqMode == MODE_TOPDOWN) {
            if ((m_aCams[m_nActiveCam].m_nMode == MODE_TOP_DOWN_PED) || (m_aCams[m_nActiveCam].m_nMode == MODE_PED_DEAD_BABY)) {
                NeedToDoAJumpCutForGameCam = false;
            }

        } else if (ReqMode == MODE_TOP_DOWN_PED) {
            if ((m_aCams[m_nActiveCam].m_nMode == MODE_TOPDOWN) || (m_aCams[m_nActiveCam].m_nMode == MODE_PED_DEAD_BABY)) {
                NeedToDoAJumpCutForGameCam = false;
            }
        }

        if ((((ReqMode == MODE_1STPERSON) || (ReqMode == MODE_SNIPER)
              || (ReqMode == MODE_M16_1STPERSON) || (ReqMode == MODE_ROCKETLAUNCHER) || (ReqMode == MODE_ROCKETLAUNCHER_HS)
              || (ReqMode == MODE_SNIPER_RUNABOUT) || (ReqMode == MODE_ROCKETLAUNCHER_RUNABOUT) || (ReqMode == MODE_ROCKETLAUNCHER_RUNABOUT_HS)
              || (ReqMode == MODE_M16_1STPERSON_RUNABOUT) || (ReqMode == MODE_FIGHT_CAM_RUNABOUT)
              || (ReqMode == MODE_1STPERSON_RUNABOUT) || (ReqMode == MODE_HELICANNON_1STPERSON)
              || (ReqMode == MODE_ARRESTCAM_ONE) || (ReqMode == MODE_ARRESTCAM_TWO) || (ReqMode == MODE_CAMERA)))
            && (m_pTargetEntity->GetIsTypePed())) {
            NeedToDoAJumpCutForGameCam = true;
        } else if ((ReqMode == MODE_FIXED) && (m_bPlayerIsInGarage == true)) //this is for going into this mode
        {
            if (((m_aCams[m_nActiveCam].m_nMode == MODE_SNIPER) || (m_aCams[m_nActiveCam].m_nMode == MODE_HELICANNON_1STPERSON)
                 || (m_aCams[m_nActiveCam].m_nMode == MODE_ROCKETLAUNCHER) || (m_aCams[m_nActiveCam].m_nMode == MODE_ROCKETLAUNCHER_HS) || (m_aCams[m_nActiveCam].m_nMode == MODE_M16_1STPERSON)
                 || (m_aCams[m_nActiveCam].m_nMode == MODE_TOP_DOWN_PED) || (PretendPlayerInAGarage)
                 || (m_aCams[m_nActiveCam].m_nMode == MODE_1STPERSON) || (m_aCams[m_nActiveCam].m_nMode == MODE_SNIPER_RUNABOUT)
                 || (m_aCams[m_nActiveCam].m_nMode == MODE_ROCKETLAUNCHER_RUNABOUT) || (m_aCams[m_nActiveCam].m_nMode == MODE_ROCKETLAUNCHER_RUNABOUT_HS) || (m_aCams[m_nActiveCam].m_nMode == MODE_M16_1STPERSON_RUNABOUT)
                 || (m_aCams[m_nActiveCam].m_nMode == MODE_FIGHT_CAM_RUNABOUT) || (m_aCams[m_nActiveCam].m_nMode == MODE_1STPERSON_RUNABOUT)
                 || (m_aCams[m_nActiveCam].m_nMode == MODE_CAMERA))
                && (m_pTargetEntity != nullptr) && (m_pTargetEntity->GetIsTypeVehicle())) //just came out of these modes
            {
                NeedToDoAJumpCutForGameCam = true;
            }
        }

        else if (ReqMode == MODE_FOLLOWPED) {
            bool JumpCutForSyphons = false;
            if (m_aCams[m_nActiveCam].m_nMode == MODE_AIMWEAPON) {
                if (m_pTargetEntity->AsPed()->CanWeRunAndFireWithWeapon() && !m_pTargetEntity->AsPed()->bIsDucking) {
                    float fPedHeading = m_pTargetEntity->GetHeading() - HALF_PI;
                    if (fPedHeading > m_aCams[m_nActiveCam].m_fHorizontalAngle + PI) {
                        fPedHeading -= TWO_PI;
                    } else if (fPedHeading < m_aCams[m_nActiveCam].m_fHorizontalAngle - PI) {
                        fPedHeading += TWO_PI;
                    }

                    if (std::abs(fPedHeading - m_aCams[m_nActiveCam].m_fHorizontalAngle) > DegreesToRadians(MAX_ANGLE_BEFORE_AIMWEAPON_JUMPCUT)
                        || !m_pTargetEntity->AsPed()->bIsStanding) {
                        JumpCutForSyphons = true;
                    }
                    if (TheCamera.m_bUseMouse3rdPerson) {
                        JumpCutForSyphons      = false;
                        m_bJustCameOutOfGarage = true;
                    }
                }
            }

            if ((m_aCams[m_nActiveCam].m_nMode == MODE_1STPERSON) || (m_aCams[m_nActiveCam].m_nMode == MODE_SNIPER)
                || (m_aCams[m_nActiveCam].m_nMode == MODE_M16_1STPERSON) || (m_aCams[m_nActiveCam].m_nMode == MODE_ROCKETLAUNCHER) || (m_aCams[m_nActiveCam].m_nMode == MODE_ROCKETLAUNCHER_HS)
                || (m_aCams[m_nActiveCam].m_nMode == MODE_PED_DEAD_BABY) || (m_aCams[m_nActiveCam].m_nMode == MODE_ARRESTCAM_ONE)
                || (m_aCams[m_nActiveCam].m_nMode == MODE_ARRESTCAM_TWO) || (m_aCams[m_nActiveCam].m_nMode == MODE_PILLOWS_PAPS)
                || (m_aCams[m_nActiveCam].m_nMode == MODE_SNIPER_RUNABOUT) || (m_aCams[m_nActiveCam].m_nMode == MODE_ROCKETLAUNCHER_RUNABOUT) || (m_aCams[m_nActiveCam].m_nMode == MODE_ROCKETLAUNCHER_RUNABOUT_HS)
                || (m_aCams[m_nActiveCam].m_nMode == MODE_M16_1STPERSON_RUNABOUT) || (m_aCams[m_nActiveCam].m_nMode == MODE_FIGHT_CAM_RUNABOUT)
                || (m_aCams[m_nActiveCam].m_nMode == MODE_1STPERSON_RUNABOUT) || (m_aCams[m_nActiveCam].m_nMode == MODE_HELICANNON_1STPERSON)
                || (m_aCams[m_nActiveCam].m_nMode == MODE_TOPDOWN) || (m_aCams[m_nActiveCam].m_nMode == MODE_TOP_DOWN_PED)
                || (m_aCams[m_nActiveCam].m_nMode == MODE_CAMERA) || (JumpCutForSyphons) || (bJustNotArrested)) {
                if (!m_bJustCameOutOfGarage) {
                    if ((m_aCams[m_nActiveCam].m_nMode == MODE_SNIPER) || (m_aCams[m_nActiveCam].m_nMode == MODE_ROCKETLAUNCHER) || (m_aCams[m_nActiveCam].m_nMode == MODE_ROCKETLAUNCHER_HS)
                        || (m_aCams[m_nActiveCam].m_nMode == MODE_M16_1STPERSON) || (m_aCams[m_nActiveCam].m_nMode == MODE_1STPERSON)
                        || (m_aCams[m_nActiveCam].m_nMode == MODE_SNIPER_RUNABOUT) || (m_aCams[m_nActiveCam].m_nMode == MODE_ROCKETLAUNCHER_RUNABOUT) || (m_aCams[m_nActiveCam].m_nMode == MODE_ROCKETLAUNCHER_RUNABOUT_HS)
                        || (m_aCams[m_nActiveCam].m_nMode == MODE_M16_1STPERSON_RUNABOUT) || (m_aCams[m_nActiveCam].m_nMode == MODE_FIGHT_CAM_RUNABOUT)
                        || (m_aCams[m_nActiveCam].m_nMode == MODE_1STPERSON_RUNABOUT) || (m_aCams[m_nActiveCam].m_nMode == MODE_HELICANNON_1STPERSON)
                        || (m_aCams[m_nActiveCam].m_nMode == MODE_CAMERA)) {
                        float CamDirection;
                        CamDirection                                 = CGeneral::GetATanOfXY(m_aCams[m_nActiveCam].m_vecFront.x, m_aCams[m_nActiveCam].m_vecFront.y) - (PI / 2);
                        m_pTargetEntity->AsPed()->m_fCurrentRotation = CamDirection;
                        m_pTargetEntity->AsPed()->m_fAimingRotation  = CamDirection;
                    }

                    NeedToDoAJumpCutForGameCam = true;
                    m_bUseTransitionBeta       = true;
                    if (m_aCams[m_nActiveCam].m_nMode == MODE_TOP_DOWN_PED) {
                        CVector CamToPlayer = m_aCams[m_nActiveCam].m_vecSource - (FindPlayerPed()->GetPosition());
                        CamToPlayer.z       = 0;
                        CamToPlayer.Normalise();
                        if ((CamToPlayer.x = 0.001f) && (CamToPlayer.y = 0.001f)) {
                            CamToPlayer.y = 1.0f;
                        }
                        m_aCams[m_nActiveCam].m_fTransitionBeta = CGeneral::GetATanOfXY(CamToPlayer.x, CamToPlayer.y);
                    } else {
                        m_aCams[m_nActiveCam].m_fTransitionBeta = (CGeneral::GetATanOfXY(m_aCams[m_nActiveCam].m_vecFront.x, m_aCams[m_nActiveCam].m_vecFront.y) + PI);
                    }
                }
            }

        } else if (ReqMode == MODE_LIGHTHOUSE) {
            NeedToDoAJumpCutForGameCam = true;
        } else if (ReqMode == MODE_ARRESTCAM_ONE || ReqMode == MODE_ARRESTCAM_TWO
                   || ReqMode == MODE_PED_DEAD_BABY) {
            NeedToDoAJumpCutForGameCam = true;
        } else if (m_aCams[m_nActiveCam].m_nMode == MODE_PED_DEAD_BABY && ReqMode != MODE_PED_DEAD_BABY) {
            NeedToDoAJumpCutForGameCam = true;
        }

        if (ReqMode != m_aCams[m_nActiveCam].m_nMode) {
            if (m_aCams[m_nActiveCam].m_pCamTargetEntity == nullptr) {
                assert(m_pTargetEntity && "Mark- King of sex but not programming");
                NeedToDoAJumpCutForGameCam = true;
            }
        }

        if (m_bPlayerIsInGarage) {
            if (m_pToGarageWeAreIn != nullptr) {
                if ((m_pToGarageWeAreIn->m_nType == BOMBSHOP_TIMED) || (m_pToGarageWeAreIn->m_nType == BOMBSHOP_ENGINE) || (m_pToGarageWeAreIn->m_nType == BOMBSHOP_REMOTE)) {
                    if (m_pTargetEntity->GetIsTypeVehicle()) {
                        if (m_pTargetEntity->AsVehicle()->GetModelIndex() == MODEL_MRWHOOP) //check that van is entirely inside
                        {
                            if (ReqMode != m_aCams[m_nActiveCam].m_nMode) {
                                NeedToDoAJumpCutForGameCam = true;
                            }
                        }
                    }
                }
            }

            if (m_aCams[m_nActiveCam].m_pCamTargetEntity) {
                CVector v1  = m_aCams[m_nActiveCam].m_pCamTargetEntity->GetPosition() - m_vecFixedModeSource;
                CVector v2  = m_aCams[m_nActiveCam].m_pCamTargetEntity->GetPosition() - m_aCams[m_nActiveCam].m_vecSource;
                float   dot = DotProduct(v1, v2);
                if (dot < 0.0f) {
                    NeedToDoAJumpCutForGameCam = true;
                }
            }
        }

        if (ReqMode != m_aCams[m_nActiveCam].m_nMode && (!m_bTransitionState || NeedToDoAJumpCutForGameCam == true)) {
            if (NeedToDoAJumpCutForGameCam == true) {
                if (!(m_bPlayerIsInGarage) || (m_bJustCameOutOfGarage)) //also can keep this in
                //just in case they cahnge there mind about interpolating
                {
                    if (!((ReqMode == MODE_FOLLOWPED) || (ReqMode == MODE_M16_1STPERSON) || (ReqMode == MODE_SNIPER) || (ReqMode == MODE_ROCKETLAUNCHER) || (ReqMode == MODE_ROCKETLAUNCHER_HS) || (ReqMode == MODE_CAMERA) || (ReqMode == MODE_SYPHON) || (ReqMode == MODE_1STPERSON) || (ReqMode == MODE_SYPHON_CRIM_IN_FRONT)
                          || (ReqMode == MODE_SPECIAL_FIXED_FOR_SYPHON) || (m_bUseMouse3rdPerson))) {
                        SetCameraDirectlyBehindForFollowPed_CamOnAString();
                    }
                }
                m_aCams[m_nActiveCam].m_nMode                 = ReqMode;
                m_bJust_Switched                              = true;
                m_aCams[m_nActiveCam].m_vecCamFixedModeVector = m_vecFixedModeVector;
                m_aCams[m_nActiveCam].m_pCamTargetEntity->CleanUpOldReference(&m_aCams[m_nActiveCam].m_pCamTargetEntity);
                m_aCams[m_nActiveCam].m_pCamTargetEntity = m_pTargetEntity;
                m_aCams[m_nActiveCam].m_pCamTargetEntity->RegisterReference(&m_aCams[m_nActiveCam].m_pCamTargetEntity);
                m_aCams[m_nActiveCam].m_vecCamFixedModeSource        = m_vecFixedModeSource;
                m_aCams[m_nActiveCam].m_vecCamFixedModeUpOffSet      = m_vecFixedModeUpOffSet;
                m_aCams[m_nActiveCam].m_bCamLookingAtVector          = m_bLookingAtVector;
                m_aCams[m_nActiveCam].m_vecLastAboveWaterCamPosition = m_aCams[(m_nActiveCam + 1) % 2].m_vecLastAboveWaterCamPosition;
                m_fCarZoomSmoothed                                   = m_fCarZoomBase;
                m_fPedZoomSmoothed                                   = m_fPedZoomBase;
                m_bTransitionState                                   = false;
                m_bDoingSpecialInterp                                = false;

                m_bStartInterScript                                  = false;
                m_aCams[m_nActiveCam].m_bResetStatics                = true;
            } else if (m_bWaitForInterpolToFinish == false) {
                StartTransition(ReqMode);
            }
        } else if ((m_bTransitionState) && (ReqMode != m_aCams[m_nActiveCam].m_nMode)) //&&((ReqMode==MODE_SYPHON)||(ReqMode==MODE_SYPHON_CRIM_IN_FRONT)))
        {
            bool SafeToInterpolate = true;

            CVector PlayerFarAway;

            if (m_bWaitForInterpolToFinish == false) {
                if ((m_bLookingAtPlayer) && (m_bTransitionState)) {
                    PlayerFarAway.x = FindPlayerPed()->GetPosition().x - GetMatrix().GetPosition().x;
                    PlayerFarAway.y = FindPlayerPed()->GetPosition().y - GetMatrix().GetPosition().y;
                    PlayerFarAway.z = FindPlayerPed()->GetPosition().z - GetMatrix().GetPosition().z;

                    if (m_pTargetEntity != nullptr) {
                        if (m_pTargetEntity->GetIsTypePed()) {
                            if (PlayerFarAway.Magnitude() > 17.5f) {
                                if ((ReqMode == MODE_SYPHON) || (ReqMode == MODE_SYPHON_CRIM_IN_FRONT)) {
                                    m_bWaitForInterpolToFinish = true;
                                }
                            }
                        }
                    }
                }
            }

            if (m_bWaitForInterpolToFinish == true) {
                SafeToInterpolate = false;
            }

            if (SafeToInterpolate) {
                StartTransitionWhenNotFinishedInter(ReqMode);
            }
        } else if ((ReqMode == MODE_FIXED) && (m_pTargetEntity != m_aCams[m_nActiveCam].m_pCamTargetEntity)) {
            if (m_bPlayerIsInGarage) {
                if (!m_bTransitionState) {
                    StartTransition(ReqMode);
                } else {
                    StartTransitionWhenNotFinishedInter(ReqMode);
                }
            }
        }
    } else {
        bool bJumpToFirstPerson       = false;
        bool bFirstPersonWeaponActive = false;
        if ((m_bEnable1rstPersonCamCntrlsScript == true) || (m_bAllow1rstPersonWeaponsCamera == true)) {
            if (ReqMode == MODE_1STPERSON) {
                if (m_aCams[m_nActiveCam].m_nMode != ReqMode) {
                    bJumpToFirstPerson = true;
                }
            } else if (((m_PlayerWeaponMode.m_nMode == MODE_SNIPER) || (m_PlayerWeaponMode.m_nMode == MODE_1STPERSON)
                        || (m_PlayerWeaponMode.m_nMode == MODE_ROCKETLAUNCHER) || (m_PlayerWeaponMode.m_nMode == MODE_ROCKETLAUNCHER_HS))
                       && (CPad::GetPad(0)->GetTarget())
                       && (m_bAllow1rstPersonWeaponsCamera)) {
                bJumpToFirstPerson       = true;
                bFirstPersonWeaponActive = true;
            } else {
                if (m_aCams[m_nActiveCam].m_nMode != m_nModeToGoTo) {
                    m_bStartInterScript = true;
                    m_nTypeOfSwitch     = eSwitchType::JUMPCUT;
                    CPad::GetPad(0)->DisablePlayerControls &= ~1;
                }
            }
        }

        if (((!m_bTransitionState) && (m_bStartInterScript == true)) && (m_nTypeOfSwitch == eSwitchType::INTERPOLATION)) {
            ReqMode = m_nModeToGoTo;
            StartTransition(ReqMode);
        } else if ((m_bTransitionState && (m_bStartInterScript == true)) && (m_nTypeOfSwitch == eSwitchType::INTERPOLATION)) {
            ReqMode = m_nModeToGoTo;
            StartTransitionWhenNotFinishedInter(ReqMode);
        } else if (((m_bStartInterScript == true) && (m_nTypeOfSwitch == eSwitchType::JUMPCUT)) || (bJumpToFirstPerson)) {
            m_bTransitionState    = false;
            m_bDoingSpecialInterp = false;
            if ((m_bEnable1rstPersonCamCntrlsScript == true) && (ReqMode == MODE_1STPERSON)) {
                m_aCams[m_nActiveCam].m_nMode = ReqMode;
            } else if (bFirstPersonWeaponActive) {
                m_aCams[m_nActiveCam].m_nMode = static_cast<eCamMode>(m_PlayerWeaponMode.m_nMode);
            } else {
                m_aCams[m_nActiveCam].m_nMode = m_nModeToGoTo;
            }
            m_bJust_Switched                              = true;

            m_aCams[m_nActiveCam].m_bResetStatics         = true;
            m_aCams[m_nActiveCam].m_vecCamFixedModeVector = m_vecFixedModeVector;
            m_aCams[m_nActiveCam].m_pCamTargetEntity->CleanUpOldReference(&m_aCams[m_nActiveCam].m_pCamTargetEntity);
            m_aCams[m_nActiveCam].m_pCamTargetEntity = m_pTargetEntity;
            m_aCams[m_nActiveCam].m_pCamTargetEntity->RegisterReference(&m_aCams[m_nActiveCam].m_pCamTargetEntity);
            m_aCams[m_nActiveCam].m_vecCamFixedModeSource        = m_vecFixedModeSource;
            m_aCams[m_nActiveCam].m_vecCamFixedModeUpOffSet      = m_vecFixedModeUpOffSet;
            m_aCams[m_nActiveCam].m_bCamLookingAtVector          = m_bLookingAtVector;
            m_aCams[m_nActiveCam].m_vecLastAboveWaterCamPosition = m_aCams[(m_nActiveCam + 1) % 2].m_vecLastAboveWaterCamPosition;
            m_bJust_Switched                                     = true;
            m_fCarZoomSmoothed                                   = m_fCarZoomBase;
            m_fPedZoomSmoothed                                   = m_fPedZoomBase;
        }
    }
    m_bStartInterScript = false;

    if (m_aCams[m_nActiveCam].m_pCamTargetEntity == nullptr) {
        m_aCams[m_nActiveCam].m_pCamTargetEntity->CleanUpOldReference(&m_aCams[m_nActiveCam].m_pCamTargetEntity);
        m_aCams[m_nActiveCam].m_pCamTargetEntity = m_pTargetEntity;
        m_aCams[m_nActiveCam].m_pCamTargetEntity->RegisterReference(&m_aCams[m_nActiveCam].m_pCamTargetEntity);
    }

    if (m_aCams[m_nActiveCam].m_nMode == MODE_FLYBY || (m_pTargetEntity->GetIsTypePed() && (m_aCams[m_nActiveCam].m_nMode == MODE_1STPERSON || m_aCams[m_nActiveCam].m_nMode == MODE_SNIPER || m_aCams[m_nActiveCam].m_nMode == MODE_M16_1STPERSON || m_aCams[m_nActiveCam].m_nMode == MODE_CAMERA || m_aCams[m_nActiveCam].m_nMode == MODE_HELICANNON_1STPERSON || m_aCams[m_nActiveCam].m_nMode == MODE_ROCKETLAUNCHER || m_aCams[m_nActiveCam].m_nMode == MODE_ROCKETLAUNCHER_HS))) {
        if (FindPlayerPed()->m_bIsVisible) {
            FindPlayerPed()->m_bIsVisible = false;

            CTaskSimpleHoldEntity* pTask  = FindPlayerPed()->GetIntelligence()->GetTaskHold();

            if (pTask && pTask->GetHeldEntity()) {
                pTask->GetHeldEntity()->m_bIsVisible = false;
            }
        }
    } else {
        FindPlayerPed()->m_bIsVisible = true;
    }

    if (m_aCams[m_nActiveCam].m_nMode == MODE_FIXED) {
        FindPlayerPed()->m_bIsVisible = gPlayerPedVisible;
    }

    bool JustGoneOutOfObbeCam = false;
    if (CanTryObbeCam == false) {
        if (m_nWhoIsInControlOfTheCamera == 2) {
            RestoreWithJumpCut();
            JustGoneOutOfObbeCam = true;
            SetCameraDirectlyBehindForFollowPed_CamOnAString();
        }
    }
    if ((ModeAtStartOfCamControl != m_aCams[m_nActiveCam].m_nMode) || (JustGoneOutOfObbeCam) || (m_aCams[m_nActiveCam].m_nMode == MODE_FOLLOWPED) || (m_aCams[m_nActiveCam].m_nMode == MODE_CAM_ON_A_STRING)) {
        if ((CPad::GetPad(0)->sub_540530()) && (CReplay::Mode != MODE_PLAYBACK) && ((m_bLookingAtPlayer == true) || (m_nWhoIsInControlOfTheCamera == 2)) && (m_bWideScreenOn == false)) {
            if (m_nWhoIsInControlOfTheCamera == 2) {
                if (JustGoneIntoObbeCamera == true) {
                    if (!CPad::GetPad(0)->DisablePlayerControls) {
                        AudioEngine.ReportFrontendAudioEvent(AE_FRONTEND_DISPLAY_INFO);
                    }
                }
            } else {
                AudioEngine.ReportFrontendAudioEvent(AE_FRONTEND_DISPLAY_INFO);
            }
        }
    }
}

// 0x5B24A0
void CCamera::DeleteCutSceneCamDataMemory() {
    for (auto& splines : m_aPathArray) {
        delete splines.m_pArrPathData;
        splines.m_pArrPathData = nullptr;
    }
}

// 0x5B24D0
void CCamera::LoadPathSplines(FILE* file) {
    DeleteCutSceneCamDataMemory();

    char* line = CFileLoader::LoadLine(file);
    if (!line) {
        return;
    }

    int32 splineIndex = -1;
    bool bNeedsNewAllocation = true;
    int32 numPoints = 0;
    float* pWritePtr = nullptr;

    while (line) {
        if (line[0] != '\0' && line[0] != '#') {
            if (numPoints == 0) {
                if (bNeedsNewAllocation) {
                    splineIndex++;
                    if (splineIndex > 3) {
                        break;
                    }
                    sscanf(line, "%d", &numPoints);

                    uint32 allocSize;
                    if (splineIndex == 0 || splineIndex == 1) {
                        allocSize = numPoints * 16 + 4; // 4 floats per point + 1 float for count
                    } else {
                        allocSize = numPoints * 40 + 4; // 10 floats per point + 1 float for count
                    }

                    m_aPathArray[splineIndex].m_pArrPathData = (float*)operator new(allocSize);
                    m_aPathArray[splineIndex].m_pArrPathData[0] = (float)numPoints;
                    pWritePtr = &m_aPathArray[splineIndex].m_pArrPathData[1];
                    bNeedsNewAllocation = false;
                } else if (line[0] == ';') {
                    bNeedsNewAllocation = true;
                }
            } else {
                numPoints--;
                char* token = strtok(line, ", \t");
                while (token) {
                    *pWritePtr = (float)atof(token);
                    pWritePtr++;
                    token = strtok(nullptr, ", \t");
                }
            }
        }

        line = CFileLoader::LoadLine(file);
    }
}

// 0x50AB50
void CCamera::GetScreenRect(CRect* rect) const {
    rect->left  = 0.0f;
    rect->right = SCREEN_WIDTH;

    if (m_bWideScreenOn) {
        rect->top    = (float)(RsGlobal.maximumHeight / 2) * m_fScreenReductionPercentage / 100.f - SCREEN_SCALE_Y(22.0f);
        rect->bottom = SCREEN_HEIGHT - (RsGlobal.maximumHeight / 2) * m_fScreenReductionPercentage / 100.f - SCREEN_SCALE_Y(14.0f);
    } else {
        rect->top    = 0.0f;
        rect->bottom = SCREEN_HEIGHT;
    }
}

// 0x50CB60
void CCamera::SetCamCollisionVarDataSet(int32 index) {
    if (index == gCurCamColVars) {
        return;
    }

    gCurCamColVars = index;
    gCurDistForCam = 1.0f;
    gpCamColVars   = gCamColVars[index];
}

// 0x50CCA0
void CCamera::SetColVarsVehicle(eVehicleType vehicleType, int32 camVehicleZoom) {
    switch (vehicleType) {
        case VEHICLE_TYPE_AUTOMOBILE:
        case VEHICLE_TYPE_PLANE:
            SetCamCollisionVarDataSet(camVehicleZoom + 9);
            return;
        case VEHICLE_TYPE_MTRUCK:
            SetCamCollisionVarDataSet(camVehicleZoom + 12);
            return;
        case VEHICLE_TYPE_QUAD:
            SetCamCollisionVarDataSet(camVehicleZoom + 15);
            return;
        case VEHICLE_TYPE_HELI:
            SetCamCollisionVarDataSet(camVehicleZoom + 18);
            return;
        case VEHICLE_TYPE_BOAT:
            SetCamCollisionVarDataSet(camVehicleZoom + 21);
            return;
        case VEHICLE_TYPE_TRAIN:
            SetCamCollisionVarDataSet(camVehicleZoom + 24);
            return;
    }
}

// 0x515BC0
void CCamera::StartTransitionWhenNotFinishedInter(eCamMode newCamMode) {
    m_bDoingSpecialInterp = true;
    StartTransition(newCamMode);
}

// 0x515200
/**
 * @brief Initiates a camera transition to a new camera mode.
 * 
 * This function handles the transition between different camera modes, setting up all necessary parameters
 * for a smooth camera movement. It manages aspects such as:
 * - Camera rotation and positioning
 * - Transition timing and interpolation fractions
 * - Special handling for weapon modes
 * - Entity references and target updates
 * 
 * The transition process includes:
 * 1. Setting up default transition values
 * 2. Handling player rotation for weapon modes
 * 3. Setting up the new camera parameters
 * 4. Managing specific camera mode transitions
 * 5. Initializing transition state and interpolation values
 * 6. Storing starting speeds and final transition parameters
 * 
 * @param newCamMode The camera mode to transition to (type eCamMode)
 * 
 * @note This function is central to the game's camera system and affects how the camera behaves
 * when switching between different views (e.g., from following a ped to aiming a weapon).
 * 
 * @see eCamMode
 * @see CCam
 */
void CCamera::StartTransition(eCamMode newCamMode) {
    CCam& activeCam             = m_aCams[m_nActiveCam];
    const auto activeCamMode    = activeCam.m_nMode;

    // Unused flag, not used in the game.
    // In GTA III/VC it was used for the Colt Python.
    m_bItsOkToLookJustAtThePlayer = false;

    // Default values
    m_bUseTransitionBeta          = false;
    m_fFractionInterToStopMoving  = 0.25f;
    m_fFractionInterToStopCatchUp = 0.75f;

    // Handle player rotation for weapon modes
    if (m_pTargetEntity && m_pTargetEntity->GetIsTypePed() && notsa::contains({ MODE_SNIPER, MODE_ROCKETLAUNCHER, MODE_ROCKETLAUNCHER_HS, MODE_M16_1STPERSON, MODE_SNIPER_RUNABOUT, MODE_ROCKETLAUNCHER_RUNABOUT, MODE_ROCKETLAUNCHER_RUNABOUT_HS, MODE_M16_1STPERSON_RUNABOUT, MODE_FIGHT_CAM_RUNABOUT, MODE_HELICANNON_1STPERSON, MODE_CAMERA, MODE_1STPERSON_RUNABOUT }, activeCamMode)) {
        const float angle                            = CGeneral::GetATanOfXY(activeCam.m_vecFront.x, activeCam.m_vecFront.y) - HALF_PI;
        m_pTargetEntity->AsPed()->m_fCurrentRotation = angle;
        m_pTargetEntity->AsPed()->m_fAimingRotation  = angle;
    }

    // Setup new camera
    activeCam.m_vecCamFixedModeVector = m_vecFixedModeVector;
    CEntity::ChangeEntityReference(activeCam.m_pCamTargetEntity, m_pTargetEntity);

    activeCam.m_vecCamFixedModeSource   = m_vecFixedModeSource;
    activeCam.m_vecCamFixedModeUpOffSet = m_vecFixedModeUpOffSet;
    activeCam.m_bCamLookingAtVector     = m_bLookingAtVector;
    if (m_bItsOkToLookJustAtThePlayer) {
        activeCam.m_nMode = newCamMode;
    }

    // Handle specific camera mode transitions
    switch (newCamMode) {
    case MODE_BEHINDCAR:
    case MODE_BEHINDBOAT:
        activeCam.m_fBetaSpeed = 0.0f;
        break;
    case MODE_FOLLOWPED: {
        if (m_bJustCameOutOfGarage) {
            activeCam.m_fHorizontalAngle = CGeneral::GetATanOfXY(activeCam.m_vecFront.x, activeCam.m_vecFront.y) + PI;
            activeCam.m_fTransitionBeta  = 0.0f;
        }

        m_bCamDirectlyInFront |= m_bTargetJustCameOffTrain;

        if (activeCamMode == MODE_CAM_ON_A_STRING) {
            m_bUseTransitionBeta        = true;
            const float angle           = CGeneral::GetATanOfXY(activeCam.m_vecFront.x, activeCam.m_vecFront.y);
            activeCam.m_fTransitionBeta = angle + (fabs(angle) <= HALF_PI ? DegreesToRadians(235.0f) : DegreesToRadians(55.0f));
        }
        break;
    }
    case MODE_SNIPER:
    case MODE_ROCKETLAUNCHER:
    case MODE_M16_1STPERSON:
    case MODE_SNIPER_RUNABOUT:
    case MODE_ROCKETLAUNCHER_RUNABOUT:
    case MODE_1STPERSON_RUNABOUT:
    case MODE_M16_1STPERSON_RUNABOUT:
    case MODE_FIGHT_CAM_RUNABOUT:
    case MODE_HELICANNON_1STPERSON:
    case MODE_CAMERA:
    case MODE_ROCKETLAUNCHER_HS:
    case MODE_ROCKETLAUNCHER_RUNABOUT_HS: {
        CEntity* vehicle             = FindPlayerVehicle();
        CMatrix* playerMat           = vehicle ? &vehicle->GetMatrix() : &FindPlayerPed()->GetMatrix();
        activeCam.m_fHorizontalAngle = CGeneral::GetATanOfXY(playerMat->GetForward().x, playerMat->GetForward().y);
        activeCam.m_fVerticalAngle   = 0.0f;
        break;
    }
    case MODE_CAM_ON_A_STRING: {
        if (m_bLookingAtPlayer && !m_bJustCameOutOfGarage) {
            m_bUseTransitionBeta = true;
            const float angle    = CGeneral::GetATanOfXY(activeCam.m_vecFront.x, activeCam.m_vecFront.y);
            if (activeCamMode == MODE_FIXED) { // Ghidra
                activeCam.m_fTransitionBeta = angle;
                break;
            }

            // Reconstruced + android simplified
            activeCam.m_fTransitionBeta = angle + (fabs(angle) <= HALF_PI ? DegreesToRadians(235.0f) : DegreesToRadians(55.0f));
        }
        break;
    }
    case MODE_PED_DEAD_BABY:
        activeCam.m_fVerticalAngle = DegreesToRadians(15.0f);
        break;
    }

    // Backup horizontal angle before Init.
    const float horizAngle = activeCam.m_fHorizontalAngle;

    int targetCoorsDuration = 600; // Like android version instead bool.
    m_nTransitionDuration   = 1'350;

    // Switch active camera
    if (activeCamMode == MODE_FOLLOWPED && newCamMode == MODE_CAM_ON_A_STRING
        || activeCamMode == MODE_CAM_ON_A_STRING && newCamMode == MODE_FOLLOWPED) {
        activeCam.m_nMode = newCamMode;
    } else {
        activeCam.Init();
        activeCam.m_nMode            = newCamMode;
        activeCam.m_fHorizontalAngle = horizAngle;
    }

    [&]() -> const void {
        if (newCamMode == MODE_CAM_ON_A_STRING && notsa::contains({ MODE_SYPHON_CRIM_IN_FRONT, MODE_FOLLOWPED, MODE_SYPHON, MODE_SPECIAL_FIXED_FOR_SYPHON, MODE_AIMWEAPON }, activeCamMode)) {
            m_fFractionInterToStopMoving  = 0.1f;
            m_fFractionInterToStopCatchUp = 0.9f;
            m_nTransitionDuration         = 750;
            return;
        }

        switch (activeCamMode) {
        case MODE_SYPHON_CRIM_IN_FRONT:
            if (newCamMode == MODE_SYPHON) {
                m_nTransitionDuration = 1'800;
                return;
            }
            break;
        case MODE_SPECIAL_FIXED_FOR_SYPHON:
            m_fFractionInterToStopMoving  = 0.2f;  // dword_8CCCCC
            m_fFractionInterToStopCatchUp = 0.8f;  // *&dword_8CCCC8
            m_nTransitionDuration         = 1'000; // dword_8CCCC4
            return;
        case MODE_FIXED:
            m_fFractionInterToStopMoving  = 0.05f;
            m_fFractionInterToStopCatchUp = 0.95f;
            return;
        }

        if (m_bPlayerWasOnBike && newCamMode == MODE_FOLLOWPED) {
            if (activeCamMode == MODE_CAM_ON_A_STRING) {
                m_nTransitionDuration         = 800;
                m_fFractionInterToStopMoving  = 0.02f;
                m_fFractionInterToStopCatchUp = 0.98f;
                return;
            }
        } else {
            switch (newCamMode) {
            case MODE_CAM_ON_A_STRING:
            case MODE_BEHINDBOAT:
                if (notsa::contains({ MODE_SNIPER_RUNABOUT, MODE_ROCKETLAUNCHER_RUNABOUT, MODE_ROCKETLAUNCHER_RUNABOUT_HS, MODE_1STPERSON_RUNABOUT, MODE_M16_1STPERSON_RUNABOUT, MODE_FIGHT_CAM_RUNABOUT, MODE_CAMERA }, activeCamMode)) {
                    m_fFractionInterToStopMoving  = 0.0f;
                    m_fFractionInterToStopCatchUp = 1.0f;
                    m_nTransitionDuration         = 1;
                    return;
                }
                break;
            case MODE_AIMWEAPON:
                m_fFractionInterToStopMoving  = 0.0f; // dword_B70044 ?
                m_fFractionInterToStopCatchUp = 1.0f; // *&dword_8CCCC0
                m_nTransitionDuration         = 400;  // dword_8CCCBC
                targetCoorsDuration           = 350;
                return;
            }

            if (!notsa::contains({ MODE_FOLLOWPED, MODE_SYPHON_CRIM_IN_FRONT, MODE_SYPHON, MODE_SPECIAL_FIXED_FOR_SYPHON }, newCamMode)) {
                m_nTransitionDuration = 1'350;
                return;
            }
        }
        if (!notsa::contains({ MODE_SYPHON_CRIM_IN_FRONT, MODE_FOLLOWPED, MODE_SYPHON, MODE_AIMWEAPON }, activeCamMode)) {
            m_nTransitionDuration = 1'350;
            return;
        }
        m_fFractionInterToStopMoving  = 0.1f;
        m_fFractionInterToStopCatchUp = 0.9f;
        m_nTransitionDuration         = 350;
        targetCoorsDuration           = 350;
    }();

    // Initialize transition state
    m_bTransitionState       = true;
    m_nTimeTransitionStart   = CTimer::GetTimeInMS();
    m_bTransitionJUSTStarted = true;

    // Store starting interpolation values
    if (m_bDoingSpecialInterp) {
        m_vecStartingSourceForInterPol = m_vecSourceDuringInter;
        m_vecStartingTargetForInterPol = m_vecTargetDuringInter;
        m_vecStartingUpForInterPol     = m_vecUpDuringInter;
        m_fStartingAlphaForInterPol    = m_fAlphaDuringInterPol;
        m_fStartingBetaForInterPol     = m_fBetaDuringInterPol;
    } else {
        m_vecStartingSourceForInterPol = activeCam.m_vecSource;
        m_vecStartingTargetForInterPol = activeCam.m_vecTargetCoorsForFudgeInter;
        m_vecStartingUpForInterPol     = activeCam.m_vecUp;
        m_fStartingAlphaForInterPol    = activeCam.m_fTrueAlpha;
        m_fStartingBetaForInterPol     = activeCam.m_fTrueBeta;
    }

    // Update active camera parameters
    activeCam.m_bCamLookingAtVector     = m_bLookingAtVector;
    activeCam.m_vecCamFixedModeVector   = m_vecFixedModeVector;
    activeCam.m_vecCamFixedModeSource   = m_vecFixedModeSource;
    activeCam.m_vecCamFixedModeUpOffSet = m_vecFixedModeUpOffSet;
    activeCam.m_nMode                   = newCamMode;
    CEntity::ChangeEntityReference(activeCam.m_pCamTargetEntity, m_pTargetEntity);

    // Store starting speeds
    m_fStartingFOVForInterPol    = activeCam.m_fFOV;
    m_vecSourceSpeedAtStartInter = activeCam.m_vecSourceSpeedOverOneFrame;
    m_vecTargetSpeedAtStartInter = activeCam.m_vecTargetSpeedOverOneFrame;
    m_vecUpSpeedAtStartInter     = activeCam.m_vecUpOverOneFrame;
    m_fAlphaSpeedAtStartInter    = activeCam.m_fAlphaSpeedOverOneFrame;
    m_fBetaSpeedAtStartInter     = activeCam.m_fBetaSpeedOverOneFrame;
    m_fFOVSpeedAtStartInter      = activeCam.m_fFovSpeedOverOneFrame;

    // Setup final transition parameters
    if (m_bLookingAtPlayer) {
        m_fFractionInterToStopMovingTarget  = 0.0f;
        m_fFractionInterToStopCatchUpTarget = 1.0f;
        m_nTransitionDurationTargetCoors    = targetCoorsDuration;
    } else {
        if (m_bScriptParametersSetForInterp) {
            m_fFractionInterToStopMoving  = m_fScriptPercentageInterToStopMoving;
            m_fFractionInterToStopCatchUp = m_fScriptPercentageInterToCatchUp;
            m_nTransitionDuration         = m_nScriptTimeForInterpolation;
        }
        m_nTransitionDurationTargetCoors    = m_nTransitionDuration;
        m_fFractionInterToStopMovingTarget  = m_fFractionInterToStopMoving;
        m_fFractionInterToStopCatchUpTarget = m_fFractionInterToStopCatchUp;
    }
}

auto CCamera::GetFrustumPoints() -> std::array<CVector, 5> {
    CVector pts[5]{};

    // First, the corners
    const auto farPlane  = RwCameraGetFarClipPlane(m_pRwCamera);
    const auto farVWSize = CVector2D{ *RwCameraGetViewWindow(m_pRwCamera) } * farPlane;
    const auto corners   = CRect{ -farVWSize, farVWSize }.GetCorners3D(farPlane);

    // Copy it into pts
    rng::copy(corners, pts);

    // Last is the center point
    pts[4] = CVector{ 0.f, 0.f, 0.f };

    // Transform them to world space
    RwV3dTransformPoints(pts, pts, 5, GetRwMatrix());

    // top left, top right, bottom right, bottom left, center
    return std::to_array(pts);
}
