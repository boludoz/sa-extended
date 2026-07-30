/*
    Plugin-SDK file
    Authors: GTA Community. See more here
    https://github.com/DK22Pac/plugin-sdk
    Do not delete this comment block. Respect others' work!
*/
#pragma once

#include <RenderWare.h>
#include <Vector.h>
#include <Vector2D.h>
#include <Cam.h>
#include "QueuedMode.h"
#include "CamPathSplines.h"
#include "eCamMode.h"
#include "eVehicleType.h"
#include "ePedType.h"
#include "Hud.h"

class CEntity;
class CVector;
class CVehicle;
class CMatrix;
class CPed;
class CSphere;
class CGarage;

enum class eFadeFlag : uint16 {
    FADE_IN,
    FADE_OUT
};

enum class eSwitchType : uint16 {
    NONE,
    INTERPOLATION,
    JUMPCUT
};

//! Values confirmed against `CCam::Process` (0x526FC0), which dispatches
//! 0 -> `LookBehind`, 1 -> `LookRight(false)`, 2 -> `LookRight(true)`.
enum eLookingDirection {
    LOOKING_BEHIND  = 0,
    LOOKING_LEFT    = 1,
    LOOKING_RIGHT   = 2,
    LOOKING_FORWARD = 3,
};

enum class eGroundHeightType : int32 {
    ENTITY_BB_BOTTOM = 0,    // ground height + boundingBoxMin.z of colliding entity
    EXACT_GROUND_HEIGHT = 1, // ignores height of colliding entity at position
    ENTITY_BB_TOP = 2        // ground height + boundingBoxMax.z of colliding entity
};

enum class eMotionBlurType : uint32 {
    NONE = 0,
    SNIPER,
    LIGHT_SCENE,
    SECURITY_CAM,
    CUT_SCENE,
    INTRO,
    INTRO2,
    SNIPER_ZOOM,
    INTRO3,
    INTRO4,
};

struct CamTweak {
    int32 ModelID;
    float Dist;
    float Alt;
    float Angle;
};
VALIDATE_SIZE(CamTweak, 0x10);

class CCamera : public CPlaceable {
public:
    bool            m_bAboveGroundTrainNodesLoaded{}; // aka: m_bAboveGroundTrainNodesLoaded
    bool            m_bBelowGroundTrainNodesLoaded{}; // aka: m_bBelowGroundTrainNodesLoaded
    bool            m_bCamDirectlyBehind{};           // aka: m_bCamDirectlyBehind
    bool            m_bCamDirectlyInFront{};          // aka: m_bCamDirectlyInFront
    bool            m_bCameraJustRestored{};          // aka: m_bCameraJustRestored
    bool            m_bCutsceneFinished{};            // aka: m_bcutsceneFinished
    bool            m_bCullZoneChecksOn{};            // aka: m_bCullZoneChecksOn
    bool            m_bFirstPersonBeingUsed{};
    bool            m_bJustJumpedOutOf1stPersonBecauseOfTarget{};
    bool            m_bIdleOn{};                      // aka: m_bIdleOn
    bool            m_bInATunnelAndABigVehicle{};     // aka: m_bInATunnelAndABigVehicle
    bool            m_bInitialNodeFound{};            // aka: m_bInitialNodeFound
    bool            m_bInitialNoNodeStaticsSet{};     // aka: m_bInitialNoNodeStaticsSet
    bool            m_bIgnoreFadingStuffForMusic{};   // aka: m_bIgnoreFadingStuffForMusic
    bool            m_bPlayerIsInGarage{};            // aka: m_bPlayerIsInGarage
    bool            m_bPlayerWasOnBike{};             // aka: m_bPlayerWasOnBike
    bool            m_bJustCameOutOfGarage{};         // aka: m_bJustCameOutOfGarage
    bool            m_bJustInitialized{true};         // aka: m_bJustInitalised
    bool            m_bJust_Switched{};              // aka: m_bJust_Switched
    bool            m_bLookingAtPlayer{true};         // aka: m_bLookingAtPlayer
    bool            m_bLookingAtVector{};             // aka: m_bLookingAtVector
    bool            m_bMoveCamToAvoidGeom{};          // aka: m_bMoveCamToAvoidGeom
    bool            m_bObbeCinematicPedCamOn{};       // aka: m_bObbeCinematicPedCamOn
    bool            m_bObbeCinematicCarCamOn{};       // aka: m_bObbeCinematicCarCamOn
    bool            m_bRestoreByJumpCut{};            // aka: m_bRestoreByJumpCut
    bool            m_bUseNearClipScript{};           // aka: m_bUseNearClipScript
    bool            m_bStartInterScript{};            // aka: m_bStartInterScript
    bool            m_bStartingSpline{};             // aka: m_bStartingSpline
    bool            m_bTargetJustBeenOnTrain{};       // aka: m_bTargetJustBeenOnTrain
    bool            m_bTargetJustCameOffTrain{};      // aka: m_bTargetJustCameOffTrain
    bool            m_bUseSpecialFovTrain{};          // aka: m_bUseSpecialFovTrain
    bool            m_bUseTransitionBeta{};           // aka: m_bUseTransitionBeta
    bool            m_bUseScriptZoomValuePed{};       // aka: m_bUseScriptZoomValuePed
    bool            m_bUseScriptZoomValueCar{};       // aka: m_bUseScriptZoomValueCar
    bool            m_bWaitForInterpolToFinish{};     // aka: m_bWaitForInterpolToFinish
    bool            m_bItsOkToLookJustAtThePlayer{};  // aka: m_bItsOkToLookJustAtThePlayer
    bool            m_bWantsToSwitchWidescreenOff{};  // aka: m_bWantsToSwitchWidescreenOff
    bool            m_bWideScreenOn{};                // aka: m_WideScreenOn
    bool            m_b1rstPersonRunCloseToAWall{};   // aka: m_1rstPersonRunCloseToAWall
    bool            m_bHeadBob{};                     // aka: m_bHeadBob
    bool            m_bVehicleSuspenHigh{};           // aka: m_bVehicleSuspenHigh
    bool            m_bEnable1rstPersonCamCntrlsScript{}; // aka: m_bEnable1rstPersonCamCntrlsScript
    bool            m_bAllow1rstPersonWeaponsCamera{}; // aka: m_bAllow1rstPersonWeaponsCamera
    bool            m_bCooperativeCamMode{};          // aka: m_bCooperativeCamMode
    bool            m_bAllowShootingWith2PlayersInCar{true}; // aka: m_bAllowShootingWith2PlayersInCar
    bool            m_bDisableFirstPersonInCar{};     // aka: m_bDisableFirstPersonInCar
    eCamMode        m_nModeForTwoPlayersSeparateCars{ MODE_TWOPLAYER_SEPARATE_CARS }; // aka: m_ModeForTwoPlayersSeparateCars
    eCamMode        m_nModeForTwoPlayersSameCarShootingAllowed{ MODE_TWOPLAYER_IN_CAR_AND_SHOOTING }; // aka: m_ModeForTwoPlayersSameCarShootingAllowed
    eCamMode        m_nModeForTwoPlayersSameCarShootingNotAllowed{ MODE_BEHINDCAR }; // aka: m_ModeForTwoPlayersSameCarShootingNotAllowed
    eCamMode        m_nModeForTwoPlayersNotBothInCar{ MODE_TWOPLAYER }; // aka: m_ModeForTwoPlayersNotBothInCar
    bool            m_bGarageFixedCamPositionSet{};   // aka: m_bGarageFixedCamPositionSet
    bool            m_bDoingSpecialInterp{};          // aka: m_vecDoingSpecialInterPolation
    bool            m_bScriptParametersSetForInterp{}; // aka: m_bScriptParametersSetForInterPol
    bool            m_bFading{};                      // aka: m_bFading
    bool            m_bMusicFading{};                 // aka: m_bMusicFading
    bool            m_bMusicFadedOut{};               // aka: m_bMusicFadedOut
    bool            m_bFailedCullZoneTestPreviously{};// aka: m_bFailedCullZoneTestPreviously
    bool            m_bFadeTargetIsSplashScreen{};   // aka: m_FadeTargetIsSplashScreen
    bool            m_bWorldViewerBeingUsed{};        // aka: WorldViewerBeingUsed
    bool            m_bTransitionJUSTStarted{};       // aka: m_uiTransitionJUSTStarted
    bool            m_bTransitionState{};             // aka: m_uiTransitionState
    uint8           m_nActiveCam{};                   // aka: ActiveCam
    uint32          m_nCamShakeStart{};               // aka: m_uiCamShakeStart
    uint32          m_nFirstPersonCamLastInputTime{}; // aka: m_uiFirstPersonCamLastInputTime
    uint32          m_nLongestTimeInMill{ 5000 };     // aka: m_uiLongestTimeInMill
    uint32          m_nNumberOfTrainCamNodes{};       // aka: m_uiNumberOfTrainCamNodes
    uint32          m_nTimeLastChange{};              // aka: m_uiTimeLastChange
    uint32          m_nTimeWeLeftIdle_StillNoInput{}; // aka: m_uiTimeWeLeftIdle_StillNoInput
    uint32          m_nTimeWeEnteredIdle{};           // aka: m_uiTimeWeEnteredIdle
    uint32          m_nTimeTransitionStart{};         // aka: m_uiTimeTransitionStart
    uint32          m_nTransitionDuration{};          // aka: m_uiTransitionDuration
    uint32          m_nTransitionDurationTargetCoors{}; // aka: m_uiTransitionDurationTargetCoors
    uint32          m_nBlurBlue{};                    // aka: m_BlurBlue
    uint32          m_nBlurGreen{};                   // aka: m_BlurGreen
    uint32          m_nBlurRed{};                     // aka: m_BlurRed
    eMotionBlurType m_nBlurType{};                    // aka: m_BlurType
    uint32          m_nWorkOutSpeedThisNumFrames{4};  // aka: m_iWorkOutSpeedThisNumFrames
    uint32          m_nNumFramesSoFar{};              // aka: m_iNumFramesSoFar
    uint32          m_nCurrentTrainCamNode{};         // aka: m_iCurrentTrainCamNode
    uint32          m_nMotionBlur{};                  // aka: m_motionBlur
    uint32          m_nMotionBlurAddAlpha{};          // aka: m_imotionBlurAddAlpha
    uint32          m_nCheckCullZoneThisNumFrames{6}; // aka: m_iCheckCullZoneThisNumFrames
    uint32          m_nZoneCullFrameNumWereAt{};      // aka: m_iZoneCullFrameNumWereAt
    uint32          m_nWhoIsInControlOfTheCamera{};   // aka: WhoIsInControlOfTheCamera
    int32           m_nCarZoom{2};                    // aka: m_nCarZoom
    float           m_fCarZoomBase{};                 // aka: m_fCarZoomBase
    float           m_fCarZoomTotal{};                // aka: m_fCarZoomTotal
    float           m_fCarZoomSmoothed{};             // aka: m_fCarZoomSmoothed
    float           m_fCarZoomValueScript{};          // aka: m_fCarZoomValueScript
    int32           m_nPedZoom{2};                    // aka: m_nPedZoom
    float           m_fPedZoomBase{};                 // aka: m_fPedZoomBase
    float           m_fPedZoomTotal{};                // aka: m_fPedZoomTotal
    float           m_fPedZoomSmoothed{};             // aka: m_fPedZoomSmoothed
    float           m_fPedZoomValueScript{};          // aka: m_fPedZoomValueScript
    float           m_fCamFrontXNorm{};               // aka: CamFrontXNorm
    float           m_fCamFrontYNorm{};               // aka: CamFrontYNorm
    float           m_fDistanceToWater{};             // aka: DistanceToWater
    float           m_fHeightOfNearestWater{};        // aka: HeightOfNearestWater
    float           m_fFOVDuringInter{};              // aka: FOVDuringInter
    float           m_fLODDistMultiplier{1.f};        // aka: LODDistMultiplier
    float           m_fGenerationDistMultiplier{};    // aka: GenerationDistMultiplier
    float           m_fAlphaSpeedAtStartInter{};      // aka: m_fAlphaSpeedAtStartInter
    float           m_fAlphaWhenInterPol{};           // aka: m_fAlphaWhenInterPol
    float           m_fAlphaDuringInterPol{};         // aka: m_fAlphaDuringInterPol
    float           m_fBetaDuringInterPol{};          // aka: m_fBetaDuringInterPol
    float           m_fBetaSpeedAtStartInter{};       // aka: m_fBetaSpeedAtStartInter
    float           m_fBetaWhenInterPol{};            // aka: m_fBetaWhenInterPol
    float           m_fFOVWhenInterPol{};             // aka: m_fFOVWhenInterPol
    float           m_fFOVSpeedAtStartInter{};        // aka: m_fFOVSpeedAtStartInter
    float           m_fStartingBetaForInterPol{};     // aka: m_fStartingBetaForInterPol
    float           m_fStartingAlphaForInterPol{};    // aka: m_fStartingAlphaForInterPol
    float           m_fPedOrientForBehindOrInFront{}; // aka: m_PedOrientForBehindOrInFront
    float           m_fCameraAverageSpeed{};          // aka: m_CameraAverageSpeed
    float           m_fCameraSpeedSoFar{};            // aka: m_CameraSpeedSoFar
    float           m_fCamShakeForce{};              // aka: m_fCamShakeForce
    float           m_fFovForTrain{70.f};             // aka: m_fFovForTrain
    float           m_fFOV_Wide_Screen{};             // aka: m_fFOV_Wide_Screen
    float           m_fNearClipScript{ 0.9f };        // aka: m_fNearClipScript
    float           m_fOldBetaDiff{};                 // aka: m_fOldBetaDiff
    float           m_fPositionAlongSpline{};        // aka: m_fPositionAlongSpline
    float           m_fScreenReductionPercentage{};   // aka: m_ScreenReductionPercentage
    float           m_fScreenReductionSpeed{};        // aka: m_ScreenReductionSpeed
    float           m_fAlphaForPlayerAnim1rstPerson{};// aka: m_AlphaForPlayerAnim1rstPerson
    float           m_fOrientation{};                 // aka: Orientation
    float           m_fPlayerExhaustion{1.f};         // aka: PlayerExhaustion
    float           m_fSoundDistUp{};                 // aka: SoundDistUp
    float           m_fSoundDistUpAsRead{};           // aka: SoundDistUpAsRead
    float           m_fSoundDistUpAsReadOld{};        // aka: SoundDistUpAsReadOld
    float           m_fAvoidTheGeometryProbsTimer{};  // aka: m_fAvoidTheGeometryProbsTimer
    uint16          m_nAvoidTheGeometryProbsDirn{};   // aka: m_nAvoidTheGeometryProbsDirn
    float           m_fWideScreenReductionAmount{};   // aka: m_fWideScreenReductionAmount
    float           m_fStartingFOVForInterPol{};      // aka: m_fStartingFOVForInterPol
    CCam            m_aCams[3]{};                     // aka: Cams[3]
    CGarage*        m_pToGarageWeAreIn{};             // aka: pToGarageWeAreIn
    CGarage*        m_pToGarageWeAreInForHackAvoidFirstPerson{}; // aka: pToGarageWeAreInForHackAvoidFirstPerson
    CQueuedMode     m_PlayerMode{};                   // aka: m_PlayerMode
    CQueuedMode     m_PlayerWeaponMode{};             // aka: PlayerWeaponMode
    CVector         m_vecPreviousCameraPosition{};    // aka: m_PreviousCameraPosition
    CVector         m_vecRealPreviousCameraPosition{};// aka: m_RealPreviousCameraPosition
    CVector         m_vecAimingTargetCoors{};         // aka: m_cvecAimingTargetCoors
    CVector         m_vecFixedModeVector{};           // aka: m_vecFixedModeVector
    CVector         m_vecFixedModeSource{};           // aka: m_vecFixedModeSource
    CVector         m_vecFixedModeUpOffSet{};         // aka: m_vecFixedModeUpOffSet
    CVector         m_vecCutSceneOffset{};            // aka: m_vecCutSceneOffset
    CVector         m_vecStartingSourceForInterPol{}; // aka: m_cvecStartingSourceForInterPol
    CVector         m_vecStartingTargetForInterPol{}; // aka: m_cvecStartingTargetForInterPol
    CVector         m_vecStartingUpForInterPol{};     // aka: m_cvecStartingUpForInterPol
    CVector         m_vecSourceSpeedAtStartInter{};   // aka: m_cvecSourceSpeedAtStartInter
    CVector         m_vecTargetSpeedAtStartInter{};   // aka: m_cvecTargetSpeedAtStartInter
    CVector         m_vecUpSpeedAtStartInter{};       // aka: m_cvecUpSpeedAtStartInter
    CVector         m_vecSourceWhenInterPol{};        // aka: m_vecSourceWhenInterPol
    CVector         m_vecTargetWhenInterPol{};        // aka: m_vecTargetWhenInterPol
    CVector         m_vecUpWhenInterPol{};            // aka: m_vecUpWhenInterPol
    CVector         m_vecClearGeometryVec{};          // aka: m_vecClearGeometryVec
    CVector         m_vecGameCamPos{};                // aka: m_vecGameCamPos
    CVector         m_vecSourceDuringInter{};         // aka: SourceDuringInter
    CVector         m_vecTargetDuringInter{};         // aka: TargetDuringInter
    CVector         m_vecUpDuringInter{};             // aka: UpDuringInter
    CVector         m_vecAttachedCamOffset{};         // aka: m_vecAttachedCamOffset
    CVector         m_vecAttachedCamLookAt{};         // aka: m_vecAttachedCamLookAt
    float           m_fAttachedCamAngle{};            // aka: m_fAttachedCamAngle
    RwCamera*       m_pRwCamera{};                    // aka: m_pRwCamera
    CEntity*        m_pTargetEntity{};                // aka: pTargetEntity
    CEntity*        m_pAttachedEntity{};              // aka: pAttachedEntity
    CCamPathSplines m_aPathArray[4]{};               // aka: m_arrPathArray[4]
    bool            m_bMirrorActive{};                // aka: m_bMirrorActive
    bool            m_bResetOldMatrix{};              // aka: m_bResetOldMatrix
    CMatrix         m_mCameraMatrix{ CMatrix::Identity() }; // aka: m_cameraMatrix
    CMatrix         m_mCameraMatrixOld{};             // aka: m_cameraMatrixOld
    CMatrix         m_mViewMatrix{};                  // aka: m_viewMatrix
    CMatrix         m_mMatInverse{};                  // aka: m_matInverse
    CMatrix         m_mMatMirrorInverse{};            // aka: m_matMirrorInverse
    CMatrix         m_mMatMirror{};                   // aka: m_matMirror
    CVector         m_avecFrustumNormals[4]{};        // aka: m_vecFrustumNormals[4]
    CVector         m_avecFrustumWorldNormals[4]{};   // aka: m_vecFrustumWorldNormals[4]
    CVector         m_avecFrustumWorldNormals_Mirror[4]{}; // aka: m_vecFrustumWorldNormals_Mirror[4]
    float           m_fFrustumPlaneOffsets[4]{};      // aka: m_fFrustumPlaneOffsets[4]
    float           m_fFrustumPlaneOffsets_Mirror[4]{}; // aka: m_fFrustumPlaneOffsets_Mirror[4]
    CVector         m_vecRightFrustumNormal{};
    CVector         m_vecBottomFrustumNormal{};
    CVector         m_vecTopFrustumNormal{};
    float           field_BF8{};
    float           m_fFadeAlpha{};                   // aka: m_fFloatingFade
    float           m_fEffectsFaderScalingFactor{};   // aka: m_fFloatingFadeMusic
    float           m_fFadeDuration{};                // aka: m_fTimeToFadeOut
    float           m_fTimeToFadeMusic{};             // aka: m_fTimeToFadeMusic
    float           m_fTimeToWaitToFadeMusic{};       // aka: m_fTimeToWaitToFadeMusic
    float           m_fFractionInterToStopMoving{0.25f}; // aka: m_fFractionInterToStopMoving
    float           m_fFractionInterToStopCatchUp{0.75f}; // aka: m_fFractionInterToStopCatchUp
    float           m_fFractionInterToStopMovingTarget{}; // aka: m_fFractionInterToStopMovingTarget
    float           m_fFractionInterToStopCatchUpTarget{}; // aka: m_fFractionInterToStopCatchUpTarget
    float           m_fGaitSwayBuffer{0.85f};         // aka: m_fGaitSwayBuffer
    float           m_fScriptPercentageInterToStopMoving{}; // aka: m_fScriptPercentageInterToStopMoving
    float           m_fScriptPercentageInterToCatchUp{}; // aka: m_fScriptPercentageInterToCatchUp
    uint32          m_nScriptTimeForInterpolation{};  // aka: m_fScriptTimeForInterPolation
    eFadeFlag       m_nFadeInOutFlag{};               // aka: m_iFadingDirection
    int32           m_nModeObbeCamIsInForCar{30};     // aka: m_iModeObbeCamIsInForCar
    eCamMode        m_nModeToGoTo{ MODE_FOLLOWPED };  // aka: m_iModeToGoTo
    eFadeFlag       m_nMusicFadingDirection{};        // aka: m_iMusicFadingDirection
    eSwitchType     m_nTypeOfSwitch{ eSwitchType::INTERPOLATION }; // aka: m_iTypeOfSwitch
    char            _alignC40[2]{};
    uint32          m_nFadeStartTime{};               // aka: m_uiFadeTimeStarted
    uint32          m_nFadeTimeStartedMusic{};        // aka: m_uiFadeTimeStartedMusic
    int32           m_nExtraEntitiesCount{};          // aka: m_numExtrasEntitysToIgnore
    CEntity*        m_pExtraEntity[2]{};              // aka: m_pExtrasEntitysToIgnore[2]
    float           m_fDuckCamMotionFactor{};         // aka: m_duckZMod
    float           m_fDuckAimCamMotionFactor{};      // aka: m_duckZMod_Aim
    float           m_fTrackLinearStartTime{};        // aka: m_vectorTrackStartTime
    float           m_fTrackLinearEndTime{};          // aka: m_vectorTrackEndTime
    CVector         m_vecTrackLinearEndPoint{};       // aka: m_vectorTrackTo
    CVector         m_vecTrackLinearStartPoint{};     // aka: m_vectorTrackFrom
    bool            m_bTrackLinearWithEase{};         // aka: m_bVectorTrackSmoothEnds
    CVector         m_vecTrackLinear{};               // aka: m_VectorTrackScript
    bool            m_bVecTrackLinearProcessed{};     // aka: m_bVectorTrackScript
    float           m_fShakeIntensity{};              // aka: m_DegreeHandShake
    float           m_fStartShakeTime{};              // aka: m_shakeStartTime
    float           m_fEndShakeTime{};                // aka: m_shakeEndTime
    int32           field_C9C{};                      // aka: m_bShakeScript
    int32           m_nShakeType{};                   // aka: m_CurShakeCam
    float           m_fStartZoomTime{};               // aka: m_FOVLerpStartTime
    float           m_fEndZoomTime{};                 // aka: m_FOVLerpEndTime
    float           m_fZoomInFactor{};                // aka: m_FOVLerpStart
    float           m_fZoomOutFactor{};               // aka: m_FOVLerpEnd
    uint8           m_nZoomMode{};                    // aka: m_bFOVLerpSmoothEnds
    bool            m_bFOVLerpProcessed{};            // aka: m_bFOVScript
    float           m_fFOVNew{};                      // aka: m_MyFOV
    float           m_fMoveLinearStartTime{};         // aka: m_vectorMoveStartTime
    float           m_fMoveLinearEndTime{};           // aka: m_vectorMoveEndTime
    CVector         m_vecMoveLinearPosnStart{};       // aka: m_vectorMoveFrom
    CVector         m_vecMoveLinearPosnEnd{};         // aka: m_vectorMoveTo
    bool            m_bMoveLinearWithEase{};          // aka: m_bVectorMoveSmoothEnds
    CVector         m_vecMoveLinear{};                // aka: m_VectorMoveScript
    bool            m_bVecMoveLinearProcessed{};      // aka: m_bVectorMoveScript
    bool            m_bBlockZoom{};                   // aka: m_bPersistFOV
    bool            m_bCameraPersistPosition{};       // aka: m_bPersistCamPos
    bool            m_bCameraPersistTrack{};          // aka: m_bPersistCamLookAt
    bool            m_bCinemaCamera{};                // aka: m_bForceCinemaCam
    CamTweak        m_aCamTweak[5]{};                 // aka: m_VehicleTweaks[5]
    bool            m_bCameraVehicleTweaksInitialized{}; // aka: m_bInitedVehicleCamTweaks
    float           m_fCurrentTweakDistance{};        // aka: m_VehicleTweakLenMod
    float           m_fCurrentTweakAltitude{};        // aka: m_VehicleTweakTargetZMod
    float           m_fCurrentTweakAngle{};           // aka: m_VehicleTweakPitchMod
    int32           m_nCurrentTweakModelIndex{};      // aka: m_VehicleTweakLastModelId
    // the following are unused?
    int32           field_D58{};
    int32           field_D5C{};
    int32           field_D60{};
    int32           field_D64{};
    int32           field_D68{};
    int32           field_D6C{};
    int32           field_D70{};
    int32           field_D74{};

    static float &m_f3rdPersonCHairMultY;
    static float &m_f3rdPersonCHairMultX;
    static float &m_fMouseAccelVertical;
    static float &m_fMouseAccelHorzntl;
    static bool &m_bUseMouse3rdPerson;
    static bool &bDidWeProcessAnyCinemaCam;

public:
    static void InjectHooks();

    CCamera();
    ~CCamera() override;
    CCamera* Constructor();
    CCamera* Destructor();

    void Init();
    void InitCameraVehicleTweaks();
    void InitialiseScriptableComponents();
    void InitialiseCameraForDebugMode();

    void LoadPathSplines(FILE* file);

    bool IsTargetingActive();
    bool IsExtraEntityToIgnore(CEntity *entity);
    bool IsItTimeForNewCamera(int32 camSequence, int32 startTime); // IsItTimeForNewcam
    bool IsSphereVisible(const CVector& origin, float radius, RwMatrix* transformMatrix);
    bool IsSphereVisible(const CVector& origin, float radius);
    bool IsSphereVisible(const CSphere& sphere) { return IsSphereVisible(sphere.m_vecCenter, sphere.m_fRadius); }
    void LerpFOV(float zoomInFactor, float zoomOutFactor, float timeLimit, bool bEase);

    void Process();
    void ProcessWideScreenOn();
    void ProcessFOVLerp(float ratio);
    void ProcessFOVLerp();
    void ProcessFade();
    void ProcessMusicFade();
    void ProcessScriptedCommands();
    void ProcessShake();
    void ProcessShake(float ratio);
    void ProcessVectorMoveLinear();
    void ProcessVectorMoveLinear(float ratio);
    void ProcessVectorTrackLinear();
    void ProcessVectorTrackLinear(float ratio);
    void ProcessObbeCinemaCameraBoat();
    void ProcessObbeCinemaCameraCar();
    void ProcessObbeCinemaCameraHeli();
    void ProcessObbeCinemaCameraPed();
    void ProcessObbeCinemaCameraPlane();
    void ProcessObbeCinemaCameraTrain();
    static void DontProcessObbeCinemaCamera();

    void Restore();
    void RestoreCameraAfterMirror();
    void RestoreWithJumpCut();
    void RenderMotionBlur() const;
    void ResetDuckingSystem(CPed *ped);

    void SetCamCutSceneOffSet(const CVector& offset);
    void SetCamPositionForFixedMode(const CVector& fixedModeSource, const CVector& fixedModeUpOffset);
    void SetCameraDirectlyBehindForFollowPed_CamOnAString();
    void SetCameraDirectlyInFrontForFollowPed_CamOnAString();
    void SetCameraDirectlyBehindForFollowPed_ForAPed_CamOnAString(CPed* targetPed);
    void SetCameraDirectlyInFrontForFollowPed_ForAPed_CamOnAString(CPed* targetPed);
    void SetCameraUpForMirror();
    void SetFadeColour(uint8 red, uint8 green, uint8 blue);
    void SetMotionBlur(uint8 red, uint8 green, uint8 blue, int32 value, eMotionBlurType blurType);
    void SetMotionBlurAlpha(int32 alpha);
    void SetNearClipBasedOnPedCollision(float arg2);
    void SetNearClipScript(float nearClip);
    void SetNewPlayerWeaponMode(eCamMode mode, int16 maxZoom = 0, int16 minZoom = 0);
    void SetParametersForScriptInterpolation(float interpolationToStopMoving, float interpolationToCatchUp, uint32 timeForInterpolation);
    void SetPercentAlongCutScene(float percent);
    void SetRwCamera(RwCamera* camera);
    void SetWideScreenOff();
    void SetWideScreenOn();
    void SetZoomValueCamStringScript(int16 zoomMode);
    void SetZoomValueFollowPedScript(int16 zoomMode);

    static void SetCamCollisionVarDataSet(int32 index);
    static void SetColVarsAimWeapon(int32 aimingType);
    static void SetColVarsPed(ePedType pedType, int32 nCamPedZoom);
    static void SetColVarsVehicle(eVehicleType vehicleType, int32 camVehicleZoom);

    void StartCooperativeCamMode();
    void StopCooperativeCamMode();
    void StartTransition(eCamMode newCamMode);
    void StartTransitionWhenNotFinishedInter(eCamMode newCamMode);

    void StoreValuesDuringInterPol(CVector *sourceDuringInter, CVector *targetDuringInter, CVector *upDuringInter, float *FOVDuringInter);

    void TakeControl(CEntity *target, eCamMode modeToGoTo, eSwitchType switchType, int32 whoIsInControlOfTheCamera);
    void TakeControlNoEntity(const CVector& fixedModeVector, eSwitchType switchType, int32 whoIsInControlOfTheCamera);
    void TakeControlAttachToEntity(CEntity* target, CEntity* attached, CVector* attachedCamOffset, CVector* attachedCamLookAt, float attachedCamAngle, eSwitchType switchType, int32 whoIsInControlOfTheCamera);
    void TakeControlWithSpline(eSwitchType switchType);

    bool TryToStartNewCamMode(int32 camSequence);

    void UpdateAimingCoors(const CVector& aimingTargetCoors);
    void UpdateSoundDistances();
    void UpdateTargetEntity();
    bool Using1stPersonWeaponMode() const;

    bool VectorMoveRunning() const;
    void VectorMoveLinear(CVector* to, CVector* from, float duration, bool bMoveLinearWithEase);

    bool VectorTrackRunning() const;
    void VectorTrackLinear(CVector* to, CVector* from, float duration, bool bEase);

    void AllowShootingWith2PlayersInCar(bool bAllow);
    void ApplyVehicleCameraTweaks(CVehicle* vehicle);
    void AvoidTheGeometry(const CVector& camPos, const CVector& targetPos, CVector& outCamPos, float fov);

    void CalculateDerivedValues(bool bForMirror, bool bUpdateOrientation);
    void CalculateFrustumPlanes(bool bForMirror);
    float CalculateGroundHeight(eGroundHeightType type);
    void CalculateMirroredMatrix(CVector posn, float mirrorV, CMatrix* camMatrix, CMatrix* mirrorMatrix);
    void CamControl();

    void AddShake(float duration, float a2, float a3, float a4, float a5);
    void AddShakeSimple(float duration, int32 type, float intensity);
    void CamShake(float strength, CVector from);
    bool CameraColDetAndReact(CVector* source, CVector* target);
    void CameraGenericModeSpecialCases(CPed* targetPed);
    void CameraPedAimModeSpecialCases(CPed* ped);
    void CameraPedModeSpecialCases();
    void CameraVehicleModeSpecialCases(CVehicle* vehicle);
    void ClearPlayerWeaponMode();
    bool ConeCastCollisionResolve(const CVector& pos, const CVector& lookAt, CVector& outDest, float rad, float minDist, float& outDist);
    bool ConsiderPedAsDucking(CPed* ped);
    void CopyCameraMatrixToRWCam(bool bDontStoreOldMatrix);
    void DealWithMirrorBeforeConstructRenderList(bool bActiveMirror, CVector mirrorNormal, float mirrorV, CMatrix* matMirror);
    void DeleteCutSceneCamDataMemory();
    void DrawBordersForWideScreen();

    void Enable1rstPersonCamCntrlsScript();
    void Enable1rstPersonWeaponsCamera();

    void Fade(float duration, eFadeFlag direction);
    void Find3rdPersonCamTargetVector(float range, CVector vecGunMuzzle, CVector& outSource, CVector& outTarget);
    float Find3rdPersonQuickAimPitch() const;
    float FindCamFOV() const;
    void FinishCutscene();

    bool GetArrPosForVehicleType(eVehicleType type, int32& arrPos);
    uint32 GetCutSceneFinishTime();
    [[nodiscard]] bool GetFading() const;
    [[nodiscard]] int32 GetFadingDirection() const;
    CVector* GetGameCamPosition();
    int32 GetLookDirection() const;
    bool GetLookingForwardFirstPerson() const;
    bool GetLookingLRBFirstPerson() const;
    [[nodiscard]] float GetPositionAlongSpline() const;
    float GetRoughDistanceToGround();
    [[nodiscard]] enum eNameState GetScreenFadeStatus() const;
    void GetScreenRect(CRect* rect) const;
    [[nodiscard]] bool Get_Just_Switched_Status() const;

    void HandleCameraMotionForDucking(CPed* ped, CVector* source, CVector* targPosn, bool arg5);
    void HandleCameraMotionForDuckingDuringAim(CPed* ped, CVector* source, CVector* targPosn, bool arg5);
    void ImproveNearClip(CVehicle* vehicle, CPed* ped, CVector* source, CVector* targPosn);

    bool ShouldPedControlsBeRelative();
    void SetToSphereMap(float);
    float GetCutsceneBarHeight();
    int32 GetCamDirectlyBehind();

    auto& GetActiveCam() { return m_aCams[m_nActiveCam]; }
    
    /*!
    * @addr notsa
    * @brief Get frustum points of the camera in world space: top left, top right, bottom right, bottom left + the center (0, 0, 0)
    */
    auto GetFrustumPoints() -> std::array<CVector, 5>;

    //! Get the camera's front normal (Whatever that is)
    auto GetFrontNormal2D() const { return CVector2D{ m_fCamFrontXNorm, m_fCamFrontYNorm }; }

public:
    static CCam& GetActiveCamera(); // TODO: Replace this with `TheCamera.GetActiveCam()`

    RwMatrix* GetRwMatrix() { return RwFrameGetMatrix(RwCameraGetFrame(m_pRwCamera)); }
    CMatrix& GetViewMatrix() { return m_mViewMatrix; }

    bool IsSphereVisibleInMirror(const CVector& origin, float radius) {
        return m_bMirrorActive && IsSphereVisible(origin, radius, (RwMatrix*)&m_mMatMirrorInverse);
    }
};
VALIDATE_SIZE(CCamera, 0xD78);

extern CCamera& TheCamera;
extern bool& gbModelViewer;
extern int8& gbCineyCamMessageDisplayed;
extern bool& gPlayerPedVisible;
extern uint8& gCurCamColVars;
extern float& gCurDistForCam;
extern int32& gCameraDirection;
extern eCamMode& gCameraMode;
extern bool gTopSphereCastTest;
extern uint32& gLastTime2PlayerCameraWasOK;
extern uint32& gLastTime2PlayerCameraCollided;
extern float*& gpCamColVars;
extern float (&gCamColVars)[28][6];
static inline auto& gpMadeInvisibleEntities = StaticRef<std::array<CEntity*, 10>>(0x9655A0);
static inline auto& gNumEntitiesSetInvisible = StaticRef<uint32>(0x9655DC);

void CamShakeNoPos(CCamera* camera, float strength);

//! Whether the active camera is under the water surface
bool CameraObscuredByWaterLevel();

//! Obbe cinema camera sequences, as indexed by `CCamera::TryToStartNewCamMode`. The gaps are
//! modes that were compiled out; the switch there covers 0..29.
enum eObbeCamSequence {
    MOVIECAM0 = 0,  //!< Wheel cam on the player
    MOVIECAM1,      //!< Fixed cam just above the road, quite far away
    MOVIECAM2,      //!< Fixed right in front, just above the road
    MOVIECAM3,      //!< Fixed cam quite high up
    MOVIECAM5 = 5,  //!< Fixed cam just above the roofs of cars
    MOVIECAM6,      //!< Standard camera
    MOVIECAM7,      //!< Chase cam on a copper chasing us
    MOVIECAM8,      //!< Wheel cam on a copper chasing us

    MOVIECAM14 = 14,
    MOVIECAM15,     //!< Straight in front of the player
    MOVIECAM16,     //!< Underneath, in front of and to the side of the player
    MOVIECAM17,     //!< Behind the player, slightly to the side
    MOVIECAM18,     //!< Directly above the player
    MOVIECAM19,     //!< Directly above the player, to the side
    MOVIECAM20,     //!< Heli chase
    MOVIECAM21,     //!< Cam man
    MOVIECAM22,     //!< Bird
    MOVIECAM23,     //!< Plane spotter
    MOVIECAM24,     //!< Dog fight - disabled on PC
    MOVIECAM25,     //!< Fish cam - disabled on PC
    MOVIECAMPLANE1,
    MOVIECAMPLANE2,
    MOVIECAMPLANE3,
    CAM_ON_A_STRING_LAST_RESORT,
};

void FindSplinePathPositionVector(float* outPos, const float* path, float timer, int32* marker);
void FindSplinePathPositionFloat(float* outValue, const float* path, float timer, int32* marker);
