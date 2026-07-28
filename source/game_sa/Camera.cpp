#include "StdInc.h"

#include "Camera.h"

#include "TaskSimpleGangDriveBy.h"
#include "TaskSimpleHoldEntity.h"
#include "TaskSimpleDuck.h"
#include "TaskSimpleSwim.h"
#include "Hud.h"

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
    RH_ScopedInstall(ProcessObbeCinemaCameraPed, 0x50B880);
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
    RH_ScopedInstall(AvoidTheGeometry, 0x514030, { .reversed = false });
    RH_ScopedInstall(CalculateGroundHeight, 0x514B80);
    RH_ScopedInstall(CalculateFrustumPlanes, 0x514D60);
    RH_ScopedInstall(CalculateDerivedValues, 0x5150E0);
    RH_ScopedInstall(ImproveNearClip, 0x516B20);
    RH_ScopedInstall(SetCameraUpForMirror, 0x51A560);
    RH_ScopedInstall(RestoreCameraAfterMirror, 0x51A5A0);
    RH_ScopedInstall(ConeCastCollisionResolve, 0x51A5D0);
    RH_ScopedInstall(TryToStartNewCamMode, 0x51E560, { .reversed = false });
    RH_ScopedInstall(CameraColDetAndReact, 0x520190);
    RH_ScopedInstall(CamControl, 0x527FA0, { .reversed = false });
    RH_ScopedInstall(Process, 0x52B730, { .reversed = false });
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
#ifndef FINAL
    if (auto* vehicle = FindPlayerVehicle()) {
        m_aCams[2].m_vecSource = vehicle->GetPosition();
    } else if (auto* player = FindPlayerPed()) {
        m_aCams[2].m_vecSource = player->GetPosition();
    }

    m_aCams[2].m_fTrueAlpha = 0.0f;
    m_aCams[2].m_fTrueBeta  = 0.0f;
    m_aCams[2].m_nMode = eCamMode::MODE_DEBUG;
#endif
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
    m_bPlayerWasOnBike = m_pTargetEntity && m_pTargetEntity->GetIsTypeVehicle() && m_pTargetEntity->AsVehicle()->m_vecMoveSpeed.SquaredMagnitude() > 0.3f;

    const auto player = FindPlayerPed();
    assert(player);

    auto something{ true };
    if (m_nWhoIsInControlOfTheCamera == 2) {
        m_nModeObbeCamIsInForCar = m_nModeObbeCamIsInForCar;
        switch (m_nModeObbeCamIsInForCar) {
        case 8:
        case 7: {
            if (player->m_nPedState != PEDSTATE_ARRESTED) {
                something = false;
            }

            if (!FindPlayerVehicle()) {
                CEntity::ChangeEntityReference(m_pTargetEntity, player);
            }

            break;
        }
        }
    }

    if (!m_bLookingAtPlayer && !something || m_bTransitionState) {
        if (m_pTargetEntity) {
            if (!m_bTargetJustBeenOnTrain) {
                return;
            }
        }
        
    }

    bool playerDoingSomethingWhileDriveBy{};
    if ([&, this]() { // Check is player doing drive-by
        if (!FindPlayerVehicle()) {
            return true;
        }

        if (!CGameLogic::IsCoopGameGoingOn()) {
            if (player->GetTaskManager().GetSimplestActiveTaskAs<CTaskSimpleGangDriveBy>()) {
                return true;
            }
        }

        return false;
    }()) {
        CEntity::ChangeEntityReference(m_pTargetEntity, player);

        playerDoingSomethingWhileDriveBy = [this, player] {
            switch (player->m_nPedState) {
            case PEDSTATE_ENTER_CAR:
            case PEDSTATE_CARJACK:
            case PEDSTATE_OPEN_DOOR:
                return true;
            }
            return false;
        }();

        if (!playerDoingSomethingWhileDriveBy) {
            auto& cam = GetActiveCam();
            if (cam.m_pCamTargetEntity != m_pTargetEntity) {
                CEntity::ChangeEntityReference(cam.m_pCamTargetEntity, m_pTargetEntity);
            }
        }
    } else {
        CEntity::ChangeEntityReference(m_pTargetEntity, FindPlayerVehicle());
    }

    const auto canEnterCar = player && player->m_pVehicle && player->m_pVehicle->CanPedOpenLocks(player); // Inverted this variable

    if (canEnterCar && player->m_nPedState == PEDSTATE_ENTER_CAR && !playerDoingSomethingWhileDriveBy) {
        if (m_nCarZoom) {
            CEntity::ChangeEntityReference(m_pTargetEntity, FindPlayerEntity());
        }
    }

    if (canEnterCar) {
        switch (player->m_nPedState) {
        case PEDSTATE_CARJACK:
        case PEDSTATE_OPEN_DOOR: {
            if (!playerDoingSomethingWhileDriveBy) {
                if (m_nCarZoom) {
                    CEntity::ChangeEntityReference(m_pTargetEntity, FindPlayerEntity());
                }
            }

            if (!FindPlayerVehicle()) {
                CEntity::ChangeEntityReference(m_pTargetEntity, player);
            }
        }
        }
    }

    switch (player->m_nPedState) {
    case PEDSTATE_EXIT_CAR:
    case PEDSTATE_DRAGGED_FROM_CAR:
        CEntity::ChangeEntityReference(m_pTargetEntity, player);
    }

    if (m_pTargetEntity->GetIsTypeVehicle()) {
        if (m_nCarZoom == 0) {
            if (player->m_nPedState == PEDSTATE_ARRESTED) {
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

//
void CCamera::ProcessObbeCinemaCameraPlane() {
    assert(0);
}

//
void CCamera::ProcessObbeCinemaCameraTrain() {
    assert(0);
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

//
void CCamera::ProcessObbeCinemaCameraBoat() {
    assert(0);
}

//
void CCamera::ProcessObbeCinemaCameraCar() {
    assert(0);
}

//
void CCamera::ProcessObbeCinemaCameraHeli() {
    assert(0);
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

    plugin::CallMethod<0x52B730, CCamera*>(this);
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
void CCamera::Find3rdPersonCamTargetVector(float range, CVector gunMuzzle, CVector& outSource, CVector& outTarget) {
    const auto pActiveCam = &m_aCams[m_nActiveCam];
    const float tanHalfFOV = std::tan(DegreesToRadians(pActiveCam->m_fFOV * 0.5f));
    const float aspectRatio = CDraw::ms_fAspectRatio;
    
    // Calculate aim target direction (This will be a unit vector)
    CVector dir = m_aCams[m_nActiveCam].m_vecFront;
    
    if (pActiveCam->m_nMode == eCamMode::MODE_TWOPLAYER_IN_CAR_AND_SHOOTING) {
        pActiveCam->Get_TwoPlayer_AimVector(dir);
    } else {
        // Vertical offset
        dir += pActiveCam->m_vecUp * (tanHalfFOV * ((0.5f - m_f3rdPersonCHairMultY) * 2.0f) / aspectRatio);

        // Horizontal offset
        const auto right = pActiveCam->m_vecFront.Cross(pActiveCam->m_vecUp);
        dir += right * (tanHalfFOV * ((m_f3rdPersonCHairMultX - 0.5f) * 2.0f));
        
        // Handle zero magnitude case
        if (dir.Magnitude() <= 0.0f) {
            dir = CVector(1.0f, 0.0f, 0.0f);
        } else {
            dir.Normalise();
        }
    }
    
    // Calculate intersection point with muzzle
    outSource = pActiveCam->m_vecSource;
    outSource += (gunMuzzle - outSource).ProjectOnToNormal(dir);

    // Apply final range to target 
    outTarget = outSource + dir * range;
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
            mi->AnimatePedColModelSkinnedWorld(ped->GetRpClump());

            const auto dotFrontSource = DotProduct(cam.m_vecFront, cam.m_vecSource);

            auto nearest = 1000000.0f;
            // Fixed at 12: the original unrolls 2 x 6 spheres and never looks at `m_nNumSpheres`
            const auto* const spheres = mi->m_pColModel->m_pColData->m_pSpheres;
            for (auto i = 0; i < 12; i++) {
                const auto& sphere = spheres[i];

                auto d = DotProduct(sphere.m_vecCenter, cam.m_vecFront) - dotFrontSource - sphere.m_fRadius;
                if (sphere.m_Surface.m_nPiece == ePedPieceTypes::PED_PIECE_HEAD) {
                    d -= 1.0f * sphere.m_fRadius;
                }
                nearest = std::min(nearest, d);
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
    return plugin::CallMethodAndReturn<bool, 0x51E560, CCamera*, int32>(this, camSequence);
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
    plugin::CallMethod<0x527FA0, CCamera*>(this); // good luck warrior!
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
