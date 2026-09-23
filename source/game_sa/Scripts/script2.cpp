#pragma warning(disable: 4101 4244 4805 4060 4189 4505 4702)

#include "StdInc.h"
#include "UserDisplay.h"
#include "TheCarGenerators.h"
#include "CarGenerator.h"
#include "Shadows.h"
#include "Tasks/TaskTypes/TaskSimpleCarSetPedInAsPassenger.h"
#include "Weather.h"
#include "OnscreenTimer.h"
#include "Camera.h"
#include "CarAI.h"
#include "PedDamageResponseCalculator.h"
#include "EventDamage.h"
#include "Coronas.h"
#include "Clock.h"
#include "ScriptResourceManager.h"
#include "Darkel.h"
#include "WaterLevel.h"
#include "PathFind.h"
#include "CarEnterExit.h"
#include "MonsterTruck.h"
#include "QuadBike.h"
#include "Heli.h"
#include "Plane.h"
#include "Bike.h"
#include "Bmx.h"
#include "Trailer.h"
#include "Hud.h"
#include "Rubbish.h"
#include "Credits.h"
#include "Tasks/TaskTypes/TaskSimpleDuckToggle.h"
#include "Tasks/TaskTypes/TaskComplexUseSwatRope.h"
#include "MBlur.h"
#include "eAreaCodes.h"
#include "SetPieces.h"
#include "eCheats.h"
#include "Ragdoll/IKChainManager.h"
#include "Tasks/TaskTypes/TaskSimplePause.h"
#include "Tasks/TaskTypes/TaskSimpleStandStill.h"
#include "Tasks/TaskTypes/TaskComplexFallAndGetUp.h"
#include "Tasks/TaskTypes/TaskComplexJump.h"
#include "Tasks/TaskTypes/TaskSimpleTriggerLookAt.h"
#include "Tasks/TaskTypes/TaskSimpleSay.h"
#include "Tasks/TaskTypes/TaskSimpleAffectSecondaryBehaviour.h"
#include "Tasks/TaskTypes/TaskSimpleShakeFist.h"
#include "Tasks/TaskTypes/TaskSimpleCower.h"
#include "Tasks/TaskTypes/TaskSimpleHandsUp.h"
#include "Tasks/TaskTypes/TaskSimpleDuck.h"
#include "Tasks/TaskTypes/TaskSimpleUseAtm.h"
#include "Tasks/TaskTypes/TaskSimpleScratchHead.h"
#include "Tasks/TaskTypes/TaskSimpleLookAbout.h"
#include "Tasks/TaskTypes/TaskComplexEnterCarAsPassengerTimed.h"
#include "Tasks/TaskTypes/TaskComplexEnterCarAsDriverTimed.h"
#include "Tasks/TaskTypes/TaskComplexLeaveCarAndFlee.h"
#include "Tasks/TaskTypes/TaskComplexDriveToPoint.h"
#include "Tasks/TaskTypes/TaskComplexDriveWander.h"
#include "Tasks/TaskTypes/TaskComplexGoToPointAndStandStill.h"
#include "Tasks/TaskTypes/TaskComplexGoToPointAndStandStillTimed.h"
#include "Tasks/TaskTypes/TaskSimpleAchieveHeading.h"
#include "Tasks/TaskTypes/TaskComplexFollowPointRoute.h"
#include "Tasks/TaskTypes/SeekEntity/TaskComplexSeekEntityStandard.h"
#include "Tasks/TaskTypes/TaskComplexFleePoint.h"
#include "Tasks/TaskTypes/TaskComplexFleeEntity.h"
#include "Tasks/TaskTypes/TaskComplexSmartFleePoint.h"
#include "Tasks/TaskTypes/TaskComplexSmartFleeEntity.h"
#include "Tasks/TaskTypes/TaskComplexWanderStandard.h"
#include "Tasks/TaskTypes/TaskComplexFollowNodeRoute.h"
#include "Tasks/TaskTypes/TaskComplexGoToPointAnyMeans.h"
#include "DecisionMakers/DecisionMakerTypesFileLoader.h"
#include "Scripted2dEffects.h"
#include "Attractors/PedAttractorPedPlacer.h"
#include "Tasks/TaskTypes/TaskComplexUseEffect.h"
#include "Tasks/TaskTypes/TaskComplexWanderCriminal.h"
#include "Tasks/TaskComplexUseSequence.h"
#include "Tasks/TaskTypes/TaskComplexBeInGroup.h"
#include "Tasks/TaskTypes/TaskComplexLeaveCar.h"
#include "Tasks/TaskTypes/TaskComplexEnterCarAsPassenger.h"
#include "Tasks/TaskTypes/TaskComplexLeaveAnyCar.h"
#include "Tasks/TaskTypes/TaskSimpleCarDrive.h"
#include "Events/EventLeaderEnteredCarAsDriver.h"
#include "Events/EventGroupEvent.h"
#include "PedGroups.h"
#include "Tasks/TaskTypes/TaskSimpleGunControl.h"
#include "Tasks/TaskTypes/TaskComplexGoToPointShooting.h"
#include "Tasks/TaskTypes/TaskSimpleSetStayInSamePlace.h"
#include "Tasks/TaskTypes/TaskSimpleClearLookAt.h"
#include "Tasks/TaskTypes/TaskSimpleThrowControl.h"
#include "Tasks/TaskTypes/TaskComplexDestroyCar.h"
#include "Tasks/TaskTypes/TaskComplexEvasiveDiveAndGetUp.h"
#include "Tasks/TaskTypes/TaskComplexShuffleSeats.h"
#include "Tasks/TaskTypes/TaskComplexPartnerChat.h"
#include "Tasks/TaskTypes/TaskSimpleTogglePedThreatScanner.h"
#include "Tasks/TaskTypes/TaskComplexDiveFromAttachedEntityAndGetUp.h"
#include "Tasks/TaskTypes/TaskComplexSitDownThenIdleThenStandUp.h"
#include "Tasks/TaskTypes/TaskComplexDrivePointRoute.h"
#include "Tasks/TaskTypes/TaskComplexGoToPointAiming.h"
#include "Tasks/TaskTypes/TaskSimpleCarSetTempAction.h"
#include "Tasks/TaskTypes/SeekEntity/TaskComplexSeekEntityRadiusAngleOffset.h"
#include "Conversations/Conversations.h"
#include "Glass.h"
#include "BreakManager_c.h"
#include "Tasks/TaskTypes/TaskComplexCarDriveMission.h"
#include "Tasks/TaskTypes/TaskSimpleHoldEntity.h"
#include "Tasks/TaskTypes/TaskSimpleSwim.h"
#include "Checkpoints.h"
#include "EntryExitManager.h"
#include "Tasks/TaskTypes/TaskSimpleSetCharIgnoreWeaponRangeFlag.h"
#include "Tasks/TaskTypes/TaskSimpleCarSetPedOut.h"
#include "Tasks/TaskTypes/TaskComplexUseGoggles.h"
#include "Tasks/TaskTypes/TaskComplexSignalAtPed.h"
#include "Tasks/TaskTypes/TaskComplexProstituteSolicit.h"
#include "Tasks/TaskTypes/TaskComplexFacial.h"
#include "cHandlingDataMgr.h"
#include "PointLights.h"
#include "PostEffects.h"
#include "MenuSystem.h"
#include "AttractorScanner.h"
#include "Events/EventAttractor.h"
#include "GangWars.h"
#include "HudColours.h"
#include "Tasks/TaskTypes/TaskComplexPartnerGreet.h"
#include "Tasks/TaskTypes/TaskComplexWalkAlongsidePed.h"
#include "Tasks/TaskTypes/TaskSimpleDie.h"
#include "Tasks/TaskTypes/TaskSimpleSetKindaStayInSamePlace.h"
#include "Birds.h"
#include "StuntJumpManager.h"
#include "Interior/InteriorManager_c.h"
#include "EntryExit.h"
#include "Tasks/TaskTypes/TaskSimpleGangDriveBy.h"
#include "Tasks/TaskTypes/TaskComplexUseMobilePhone.h"
#include "Tasks/TaskTypes/TaskComplexFleeAnyMeans.h"
#include "Tasks/TaskTypes/TaskComplexFollowPatrolRoute.h"


#include "RunningScript.h"
#include "TheScripts.h"
#include "CarCtrl.h"
#include "Garages.h"
#include "Pickups.h"
#include "Restart.h"
#include "Shopping.h"
#include "TagManager.h"
#include "Camera.h"
#include "Clock.h"
#include "FireManager.h"
#include "Game.h"
#include "ModelIndices.h"
#include "Pad.h"
#include "PlayerPed.h"
#include "Pools/Pools.h"
#include "Ropes.h"
#include "Timer.h"
#include "World.h"
#include "TheZones.h"
#include "MenuManager.h"
#include "Radar.h"
#include "CutsceneMgr.h"
#include "CivilianPed.h"
#include "EmergencyPed.h"
#include "CopPed.h"
#include "PedGroups.h"
#include "Tasks/PedScriptedTaskRecord.h"
#include "PedType.h"
#include "Coronas.h"
#include "Font.h"
#include "Streaming.h"
#include "StreamingInfo.h"
#include "Explosion.h"
#include "Messages.h"
#include "Text/Text.h"
#include "Automobile.h"
#include "Train.h"
#include "WaterLevel.h"
#include "MissionCleanup.h"
#include "tHandlingData.h"
#include "Wanted.h"
#include "Darkel.h"
#include "Gangs.h"
#include "Cheat.h"
#include "AudioEngine.h"
#include "Models/ModelInfo.h"
#include "Models/VehicleModelInfo.h"
#include "Tasks/TaskManager.h"
#include "TaskSimpleStandStill.h"
#include "TaskSimplePlayerOnFoot.h"
#include "TaskComplexDie.h"
#include "TaskComplexClimb.h"
#include "TaskSimpleJetPack.h"
#include "TaskSimpleSlideToCoord.h"
#include "TaskComplexGoToPointAndStandStill.h"
#include "TaskSimpleSetCharDecisionMaker.h"
#include "TaskComplexUseSequence.h"
#include "TaskComplexSequence.h"
#include "TaskComplexKillPedOnFoot.h"
#include "SeekEntity/TaskComplexSeekEntity.h"
#include "SeekEntity/TaskComplexSeekEntityStandard.h"
#include "TaskComplexSeekEntityAiming.h"
#include "TaskComplexGoPickUpEntity.h"
#include "Tasks/TaskTypes/TaskSimpleCarSetPedInAsDriver.h"
#include "CustomCarPlateMgr.h"
#include "PedClothesDesc.h"
#include "VehicleRecording.h"

using i8  = int8;
using i16 = int16;
using i32 = int32;
using u8  = uint8;
using u16 = uint16;
using u32 = uint32;
using f32 = float;
using u_native = uintptr;

using CTaskTypes = eTaskType;
using eCarLockState = eCarLock;

#define PCPointer m_IP
#define PCStack m_IPStack
#define StackDepth m_StackDepth
#define Locals m_LocalVars
#define bActive m_IsActive
#define CmpFlag m_CondResult
#define IsThisAMissionScript m_UsesMissionCleanup
#define bIsThisAStreamedScript m_IsExternal
#define bIsThisAMiniGameScript m_IsTextBlockOverride
#define ScriptBrainType m_ExternalType
#define ActivateTime m_WakeTime
#define AndOrState m_AndOrState
#define NotForLatestExpression m_NotFlag
#define DeatharrestCheckEnabled m_IsDeathArrestCheckEnabled
#define DoneDeatharrest m_DoneDeathArrest
#define EndOfScriptedCutscenePC m_SceneSkipIP
#define ThisMustBeTheOnlyMissionRunning m_ThisMustBeTheOnlyMissionRunning
#define BaseAddressOfThisScript m_BaseIP
#define ScriptName m_szName

#define DoDeatharrestCheck DoDeathArrestCheck

#define SCOPE_GLOBAL VAR_GLOBAL
#define SCOPE_LOCAL VAR_LOCAL

#define FO_CHAR_TOKEN_DELIMITER 0x7E
#define FO_CHAR_TOKEN_DIALOGUE 0x7A

#define MARKFUNCTION(x)

#undef ScriptParams
#define ScriptParams (*reinterpret_cast<i32(*)[32]>(0xA43C78))

#define GetPointerToScriptVariable(scope) reinterpret_cast<i32*>(CRunningScript::GetPointerToScriptVariable(static_cast<eScriptVariableType>(scope)))
#define GetPointerToLocalVariable(idx) reinterpret_cast<i32*>(CRunningScript::GetPointerToLocalVariable(idx))
#define GetPointerToLocalArrayElement(base, idx, sz) reinterpret_cast<i32*>(CRunningScript::GetPointerToLocalArrayElement(base, idx, sz))

#define HasModelLoaded IsModelLoaded
constexpr int32 MODELID_PLAYER_PED = 0;

enum eStreamingFlagsCompat {
    STRFLAG_NONE = 0,
    STRFLAG_ONHARDDRIVE = 1 << 0,
    STRFLAG_DONTDELETE = STREAMING_KEEP_IN_MEMORY,
    STRFLAG_MISSION_REQUIRED = STREAMING_MISSION_REQUIRED,
    STRFLAG_FORCE_LOAD = STREAMING_PRIORITY_REQUEST,
    STRFLAG_PRIORITY_LOAD = STREAMING_PRIORITY_REQUEST,
    STRFLAG_LOADSCENE = STREAMING_LOADING_SCENE,
    STRFLAG_READFROMHD = 1 << 6,
    STRFLAG_CPU_TEXTURE = 1 << 7,
};

#define UNUSED_CHAR  PED_UNKNOWN
#define RANDOM_CHAR  PED_GAME
#define MISSION_CHAR PED_MISSION
#define REPLAY_CHAR  PED_GAME_MISSION

#define PEDTYPE_COP     PED_TYPE_COP
#define PEDTYPE_MEDIC   PED_TYPE_MEDIC
#define PEDTYPE_FIRE    PED_TYPE_FIREMAN

#define MAX(a, b) (((a) > (b)) ? (a) : (b))
#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#define ABS(a) std::abs(a)
#define DEGTORAD(d) DegreesToRadians(d)

#define m_PrefsShowSubtitles m_bShowSubtitles

#define COMMAND_AND_OR COMMAND_ANDOR
#define NO_ANDS_OR_ORS ANDOR_NONE
#define NUMBER_OF_ANDS1 ANDS_1
#define NUMBER_OF_ANDS8 ANDS_8
#define NUMBER_OF_ORS1 ORS_1
#define NUMBER_OF_ORS8 ORS_8
#define COMMAND_EXPLODE_CAR_IN_CUTSCENE_SHAKE_AND_BIT COMMAND_EXPLODE_CAR_IN_CUTSCENE_SHAKE_AND_BITS
#define COMMAND_REMOVE_ALL_USER_3D_MARKERS COMMAND_REMOVE_ALLUSER_3D_MARKERS
#define COMMAND_ENABLE_DISABLED_ATTACTORS_ON_OBJECT ((eScriptCommands)0x0A5F)
#define COMMAND_IS_XBOX_PLAYER2_PRESSING_START ((eScriptCommands)0x0A4E)

#define m_pMyVehicle m_pVehicle
#define AutoPilot m_autoPilot
#define pHandling m_pHandlingData

template <typename PoolT, typename ElemT>
struct ScriptPoolProxy {
    PoolT* pool;
    ElemT* GetAt(int32 handle) const { return pool ? pool->GetAtRef(handle) : nullptr; }
    int32 GetIndex(const ElemT* obj) const { return pool ? pool->GetRef(obj) : -1; }
    int32 GetJustIndex(const ElemT* obj) const { return pool ? (int32)pool->GetIndex(obj) : -1; }
    PoolT* operator->() const { return pool; }
    int32 GetSize() const { return pool ? (int32)pool->GetSize() : 0; }
    ElemT* GetSlot(int32 idx) const { return pool ? pool->GetAt(idx) : nullptr; }
};

struct CPoolsCompat {
    static ScriptPoolProxy<CPedPool, CPed> GetPedPool() { return { ::GetPedPool() }; }
    static ScriptPoolProxy<CVehiclePool, CVehicle> GetVehiclePool() { return { ::GetVehiclePool() }; }
    static ScriptPoolProxy<CObjectPool, CObject> GetObjectPool() { return { ::GetObjectPool() }; }
};
#define CPools CPoolsCompat

#define CLEANUP_CAR     MISSION_CLEANUP_ENTITY_TYPE_VEHICLE
#define CLEANUP_CHAR    MISSION_CLEANUP_ENTITY_TYPE_PED
#define CLEANUP_OBJECT  MISSION_CLEANUP_ENTITY_TYPE_OBJECT

#define Score m_nMoney
#define PlayerState m_nPlayerState
#define PLAYERSTATE_HASDIED PLAYERSTATE_HAS_DIED

#define ObjectCreatedBy m_nObjectType
#define MISSION_OBJECT OBJECT_MISSION
#define MISSION_BRAIN_OBJECT OBJECT_MISSION2

#define GetVehicleCreatedBy GetCreatedBy
#define m_nVehicleFlags vehicleFlags

#define Transmission m_transmissionData
#define m_fMaxFlatVelocity m_MaxFlatVelocity
#define m_fEngineAcceleration m_EngineAcceleration
#define m_fMaxVelocity m_MaxVelocity

#define GetTimeInMilliseconds GetTimeInMS

#define PED_DRIVING PEDSTATE_DRIVING
#define WANTED_CLEAN eWantedLevel::WANTED_CLEAN
#define m_PoliceBackOff m_bPoliceBackOff

#define m_eDoorLockState m_nDoorLock

#define WEAPONTYPE_UNARMED WEAPON_UNARMED
#define ANIM_STD_PED ANIM_GROUP_DEFAULT
#define ANIM_STD_KO_FRONT ANIM_ID_KO_SHOT_FRONT_0
#define TASK_PRIORITY_PRIMARY TASK_PRIMARY_PRIMARY

#define m_nHealth m_fHealth
#define m_colour1 m_nPrimaryColor
#define m_colour2 m_nSecondaryColor
#define m_motionAnimGroup m_nAnimGroup
#define m_nAutomobileFlags autoFlags

#define BLIPTYPE_CONTACT BLIP_CONTACT_POINT
#define BLIPTYPE_COORDS BLIP_COORD
#define BLIPDISPLAY_BOTH BLIP_DISPLAY_BOTH

#define ZONE_INFORMATION ZONE_TYPE_INFO
#define aGangStrengths GangStrength
#define ZoneInfoIndex m_ZoneInfoIndex
#define FindZoneByLabelAndReturnIndex FindZoneByLabel

#define pMessage Text
#define MI_TYPE_VEHICLE ModelInfoType::MODEL_INFO_VEHICLE
#define m_acquaintances GetAcquaintance()
#define m_nWeaponSlot m_nSlot

#define m_PlayerGroup m_nPlayerGroup
#define m_GroupStuffDisabled m_bGroupStuffDisabled
#define bBeatInfoPresent IsBeatInfoPresent
#define ANIM_CARRY_BOX ANIM_GROUP_CARRY
#define m_PrefsUseWideScreen m_bWidescreenOn

#define SetWantedLevel(l) SetWantedLevel(static_cast<eWantedLevel>(l))
#define SetWantedLevelNoDrop(l) SetWantedLevelNoDrop(static_cast<eWantedLevel>(l))

#define GetPadState(pl, btn) GetPadState((uint16)(pl), static_cast<eButtonId>(btn))
#define GetCorrectPedModelIndexForEmergencyServiceType(pt, pi) GetCorrectPedModelIndexForEmergencyServiceType(static_cast<ePedType>(pt), (uint32*)(pi))
inline void ScriptSetCharCoordinates(CRunningScript* s, CPed* p, float x, float y, float z, bool wg, bool off) { if (p) s->SetCharCoordinates(*p, CVector(x, y, z), wg, off); }
#define SetCharCoordinates(p, x, y, z, wg, off) ScriptSetCharCoordinates(this, (p), (x), (y), (z), (wg), (off))

#define AddBigMessage(t, tm, s) AddBigMessage((t), (tm), static_cast<eMessageStyle>(s))
#define AddBigMessageQ(t, tm, s) AddBigMessageQ((t), (tm), static_cast<eMessageStyle>(s))
#define AddMessage AddMessageQ
#define AddMessageJumpQ AddMessageJump

#define GenerateNewOne(c, m, t, a, mn, e, msg) GenerateNewOne((c), (m), static_cast<ePickupType>(t), (a), (mn), (e), (msg))
#define SetGangWeapons(g, w1, w2, w3) SetGangWeapons((g), static_cast<eWeaponType>(w1), static_cast<eWeaponType>(w2), static_cast<eWeaponType>(w3))
#define SetBlipSprite(b, s) SetBlipSprite((b), static_cast<eRadarSprite>(s))
#define SetCoordBlip(t, p, c, d, s) SetCoordBlip(static_cast<eBlipType>(t), (p), static_cast<eBlipColour>(c), static_cast<eBlipDisplay>(d), (s))
#define AddEntityToList(h, t) AddEntityToList((h), static_cast<MissionCleanUpEntityType>(t))
#define GetActualScriptThingIndex(r, t) GetActualScriptThingIndex((r), static_cast<eScriptThingType>(t))
#define GetUniqueScriptThingIndex(r, t) GetUniqueScriptThingIndex((r), static_cast<eScriptThingType>(t))
#define GetNewUniqueScriptThingIndex(r, t) GetNewUniqueScriptThingIndex((r), static_cast<eScriptThingType>(t))
#define SetDefaultTaskAllocatorType(t) SetDefaultTaskAllocatorType(static_cast<ePedGroupDefaultTaskAllocatorType>(t))
#define GetPedFlag(t) GetPedFlag(static_cast<ePedType>(t))
#define QueryModelsKilledByPlayer(m, p) QueryModelsKilledByPlayer(static_cast<eModelID>(m), (p))
#define SetTextureAndModel(t, m, p) SetTextureAndModel((t), (m), static_cast<eClothesTexturePart>(p))

inline bool operator>(eWantedLevel a, int32 b) { return static_cast<int32>(a) > b; }
inline bool operator>(eWantedLevel a, uint32 b) { return static_cast<uint32>(a) > b; }
inline bool operator<(eWantedLevel a, int32 b) { return static_cast<int32>(a) < b; }
inline bool operator>=(eWantedLevel a, int32 b) { return static_cast<int32>(a) >= b; }
inline bool operator<=(eWantedLevel a, int32 b) { return static_cast<int32>(a) <= b; }
inline bool operator==(eWantedLevel a, int32 b) { return static_cast<int32>(a) == b; }
inline bool operator!=(eWantedLevel a, int32 b) { return static_cast<int32>(a) != b; }


using u64 = uint64;
using i64 = int64;
using namespace ModelIndices;

#define RADTODEG(d) RadiansToDegrees(d)
#define BLIPTYPE_CAR BLIP_CAR
#define BLIPTYPE_CHAR BLIP_CHAR
#define BLIPTYPE_OBJECT BLIP_OBJECT
#define BLIPTYPE_PICKUP BLIP_PICKUP

#define CLEANUP_CAR MISSION_CLEANUP_ENTITY_TYPE_VEHICLE
#define CLEANUP_CHAR MISSION_CLEANUP_ENTITY_TYPE_PED
#define CLEANUP_OBJECT MISSION_CLEANUP_ENTITY_TYPE_OBJECT
#define CLEANUP_PARTICLE MISSION_CLEANUP_ENTITY_TYPE_PARTICLE
#define CLEANUP_GROUP MISSION_CLEANUP_ENTITY_TYPE_GROUP
#define CLEANUP_ATTRACTOR MISSION_CLEANUP_ENTITY_TYPE_PED_QUEUE
#define CLEANUP_SEQUENCE_TASK MISSION_CLEANUP_ENTITY_TYPE_TASK_SEQUENCE
#define CLEANUP_DECISION_MAKER MISSION_CLEANUP_ENTITY_TYPE_DECISION_MAKER
#define CLEANUP_SEARCHLIGHT MISSION_CLEANUP_ENTITY_TYPE_SEARCHLIGHT
#define CLEANUP_CHECKPOINT MISSION_CLEANUP_ENTITY_TYPE_CHECKPOINT
#define CLEANUP_TEXTURE_DICTIONARY MISSION_CLEANUP_ENTITY_TYPE_TXD
#define CLEANUP_PEDGROUP MISSION_CLEANUP_ENTITY_TYPE_GROUP

#define pPassengers m_apPassengers
#define CLEANUP_EFFECT_SYSTEM MISSION_CLEANUP_ENTITY_TYPE_PARTICLE
#define BLIPTYPE_SEARCHLIGHT BLIP_SPOTLIGHT
#define pFXSystem m_pFxSystem
#define pCustomPlateMaterial m_pPlateMaterial
#define OverrideLights m_nOverrideLights
using TTaskComplexSeekEntityRadiusAngleOffset = CTaskComplexSeekEntityRadiusAngleOffset;

#define TakeControlAttachToEntity(target, attached, off, rot, fov, sw, who)     TakeControlAttachToEntity((target), (attached), &(off), &(rot), (fov), static_cast<eSwitchType>(sw), (who))

#define m_fDesiredHeading m_fAimingRotation
#define m_fCurrentHeading m_fCurrentRotation

#define m_WeaponSlots m_aWeapons
#define m_nCurrentWeapon m_nActiveWeaponSlot

#define fMass m_fMass
#define fTurnMass m_fTurnMass
#define fBuoyancyConstant m_fBuoyancyConstant

#define PED_ARRESTED PEDSTATE_ARRESTED
#define m_fScriptThrottleControl field_9A0

#define ZONE_NAVIGATION ZONE_TYPE_NAVI
#define DoesPointLieWithinZoneWithGivenName FindZone

#define AddMessageWithNumber AddMessageWithNumberQ
#define AddMessageWithNumberJumpQ AddMessageJumpQWithNumber

#define ForceWeather(w) ForceWeather(static_cast<eWeatherType>(w))
#define ForceWeatherNow(w) ForceWeatherNow(static_cast<eWeatherType>(w))

#define ChangeBlipColour(b, c) ChangeBlipColour((b), static_cast<eBlipColour>(c))
#define Fade(t, f) Fade((t), static_cast<eFadeFlag>(f))
#define TakeControl(e, m, s, i) TakeControl((e), static_cast<eCamMode>(m), static_cast<eSwitchType>(s), (i))
#define TakeControlNoEntity(pos, s, i) TakeControlNoEntity((pos), static_cast<eSwitchType>(s), (i))
#define StoreShadowToBeRendered(t, p, fx, fy, sx, sy, i, r, g, b) StoreShadowToBeRendered((t), *(p), (fx), (fy), (sx), (sy), (i), (r), (g), (b))

#define SetShootingAccuracy SetWeaponAccuracy
#define TotalNumCollectables m_nTotalNumCollectables
#define CollectablesPickedUp m_nCollectablesPickedUp
#define PED_SPHERE_HEAD PED_COL_SPHERE_HEAD
#define ChangeGarageType(g, t, m) ChangeGarageType((g), static_cast<eGarageType>(t), (m))

#define CreateCarGenerator(x, y, z, a, m, c1, c2, f, al, d, minD, maxD, ipl, ign) CreateCarGenerator(CVector((x), (y), (z)), (a), (m), (c1), (c2), (f), (al), (d), (minD), (maxD), (ipl), (ign))
#define GetCarGenerator(idx) Get(idx)
#define AddClock(v, g, d) AddClock((v), (g), static_cast<eTimerDirection>(d))
#define AddCounter(v, t, g, i) AddCounter((v), static_cast<eOnscreenCounter>(t), (g), (i))

// Weapon mappings
#define m_eWeaponType m_Type
#define GetWeaponType GetType

// CPhysical flags & collision records
#define bExtraHeavy bMakeMassTwiceAsBig
#define bIsInWater bSubmergedInWater
#define bNotDamagedByBullets bBulletProof
#define bNotDamagedByFlames bFireProof
#define bNotDamagedByCollisions bCollisionProof
#define bNotDamagedByMelee bMeleeProof
#define bOnlyDamagedByPlayer bInvulnerable
#define bIgnoresExplosions bExplosionProof
#define bUsesCollisionRecords bCanBeCollidedWith
#define m_nNoOfCollisionRecords m_nNumEntitiesCollided
#define m_aCollisionRecordPtrs m_apCollidedEntities

// Automobile flags & wheels
#define bTaxiLightOn bTaxiLight
#define bDontDamageOnRoof bDoesNotGetDamagedUpsideDown
#define bTakePanelDamage bCanBeVisiblyDamaged
#define bLostTraction bIsBoggedDownInSand
#define bSoftSuspension bIsMonsterTruck
#define m_aWheelColPoints m_wheelColPoint
#define m_aWheelRatios m_fWheelsSuspensionCompression

// Vehicle appearance
#define APR_NOTHING VEHICLE_APPEARANCE_NONE
#define APR_CAR VEHICLE_APPEARANCE_AUTOMOBILE
#define APR_BIKE VEHICLE_APPEARANCE_BIKE
#define APR_HELI VEHICLE_APPEARANCE_HELI
#define APR_BOAT VEHICLE_APPEARANCE_BOAT
#define APR_PLANE VEHICLE_APPEARANCE_PLANE

// Camera
#define Cams m_aCams
#define ActiveCam m_nActiveCam

// Wanted
#define SetMaximumWantedLevel(l) SetMaximumWantedLevel(static_cast<eWantedLevel>(l))
#define m_Wanted m_pWanted
#define m_EverybodyBackOff m_bEverybodyBackOff
#define m_fMultiplier m_Multiplier

// Resource manager
#define AddToResourceManager(id, t, s) AddToResourceManager((id), static_cast<eScriptResourceType>(t), (s))
#define RemoveFromResourceManager(id, t, s) RemoveFromResourceManager((id), static_cast<eScriptResourceType>(t), (s))

// Corona
#define RegisterCorona(id, ent, r, g, b, a, pos, rad, fFarClip, type, flare, refl, los, trail, fNormX, bShootRefl, fNormY, bUnkn, fNormZ, bFade, bOnlyUnderwater) \
    RegisterCorona((uint32)(id), (ent), (uint8)(r), (uint8)(g), (uint8)(b), (uint8)(a), (pos), (rad), (fFarClip), \
        static_cast<eCoronaType>(type), static_cast<eCoronaFlareType>(flare), static_cast<eCoronaReflType>(refl), static_cast<eCoronaLOSCheck>(los), static_cast<eCoronaTrail>(trail), \
        (fNormX), (bShootRefl), (fNormY), (bUnkn), (fNormZ), (bFade), (bOnlyUnderwater))

// PathFind
#define FindNodeClosestToCoors(c, t, ...) FindNodeClosestToCoors((c), static_cast<ePathType>(t), __VA_ARGS__)
#define FindNodePointer(addr) GetPathNode(addr)

// Missing model / stat aliases
#define MODELID_CAR_VORTEX MODEL_VORTEX
#define MISSIONS_ATTEMPTED STAT_MISSION_ATTEMPTS

// Messages
#define AddBigMessage(t, tm, s) AddBigMessage((t), (tm), static_cast<eMessageStyle>(s))
#define AddBigMessageWithNumber(t, tm, s, ...) AddBigMessageWithNumber((t), (tm), static_cast<eMessageStyle>(s), __VA_ARGS__)
#define AddBigMessageQ(t, tm, s) AddBigMessageQ((t), (tm), static_cast<eMessageStyle>(s))
#define AddBigMessageWithNumberQ(t, tm, s, ...) AddBigMessageWithNumberQ((t), (tm), static_cast<eMessageStyle>(s), __VA_ARGS__)
#define ClearThisPrintBigNow(s) ClearThisPrintBigNow(static_cast<eMessageStyle>(s))

// tScriptText
#define ScriptTextAtX Pos.x
#define ScriptTextAtY Pos.y
#define ScriptTextLabel GXTKey
#define ScriptTextXScale Scale.x
#define ScriptTextYScale Scale.y
#define ScriptTextColor Color
#define ScriptTextJustify Justify
#define ScriptTextCentre IsCentered
#define ScriptTextWrapX WrapX
#define ScriptTextCentreSize CentreSize
#define ScriptTextBackgrnd HasBg
#define ScriptTextProportional IsProportional
#define ScriptTextFontStyle FontStyle
#define ScriptTextBeforeFade IsDrawBeforeFade
#define ScriptTextRightJustify HasRightJustify
#define ScriptTextDropShadowColour DropShadowColor
#define ScriptTextDropShadow DropShadow

// Additional aliases
#define CharCreatedBy m_nCreatedBy
#define SetStayInSamePlaceFlag SetStayInSamePlace
#define m_nArmour m_fArmour
#define MaxArmour m_nMaxArmour
#define m_iplIndex m_IplIndex
#define m_MenuActive m_bMenuActive
#define ScriptRectBeforeFade m_bDrawBeforeFade
#define PEDTYPE_CIVMALE PED_TYPE_CIVMALE
#define PEDTYPE_CIVFEMALE PED_TYPE_CIVFEMALE
#define PEDTYPE_COP PED_TYPE_COP
#define VEHICLE_TYPE_MONSTERTRUCK VEHICLE_TYPE_MTRUCK
#define VEHICLE_TYPE_QUADBIKE VEHICLE_TYPE_QUAD
#define m_nBikeFlags bikeFlags

// Window / tScriptRectangle
#define eWindowType m_nType
#define ScriptSpriteIndex m_nTextureId
#define ScriptRectMinX cornerA.x
#define ScriptRectMinY cornerA.y
#define ScriptRectMaxX cornerB.x
#define ScriptRectMaxY cornerB.y
#define ScriptRectRotation m_nAngle
#define ScriptRectColour m_nTransparentColor
#define pTitle gxt1
#define pText gxt2
#define WINDOW_NONE eScriptRectangleType::INACTIVE
#define WINDOW_HEADER_AND_TEXT eScriptRectangleType::TITLE_AND_MESSAGE
#define WINDOW_HEADER_NO_TEXT eScriptRectangleType::TEXT
#define WINDOW_SOLID_COLOUR eScriptRectangleType::MONOCOLOR
#define WINDOW_SPRITE_NO_ROTATION eScriptRectangleType::TEXTURED
#define WINDOW_SPRITE_WITH_ROTATION eScriptRectangleType::MONOCOLOR_ANGLED

// Multiply3x3 & matrix
inline CVector Multiply3x3(const CMatrix& m, const CVector& v) { return m.TransformVector(v); }
inline CVector Multiply3x3(const CVector& v, const CMatrix& m) { return m.InverseTransformVector(v); }
#define m_pMat m_matrix

// Clean up & cutscenes & timers & 3d markers
#define CLEANUP_TEXTURE_DICTIONARY MISSION_CLEANUP_ENTITY_TYPE_TXD
#define FreezeTimers m_bPaused
#define DISABLE_CUTSCENES (1 << 7)
#define PlaceMarkerSet(id, type, ...) PlaceMarkerSet((id), static_cast<e3dMarkerType>(type), __VA_ARGS__)
#define SetStatus(s) SetStatus(static_cast<eEntityStatus>(s))

// PlayerInfo & Ped & Vehicle
#define m_MoneyCarried m_nMoneyCount
#define bFreeHealthCare m_bFreeHealthCare
#define pRemoteVehicle m_pRemoteVehicle
#define m_nAmmoTotal m_TotalAmmo
#define m_pPedIntelligence m_pIntelligence
#define pPassengers m_apPassengers
#define pDriver m_pDriver
#define ForcedRandomSeed m_nForcedRandomRouteSeed
#define m_pMouseLockOnRecruitPed m_p3rdPersonMouseTarget
#define pNext m_pNext
#define m_pAttachToEntity m_pAttachedTo
#define GetNearbyPeds GetPedEntities

// Audio & Stats
#define RetuneRadio(r) RetuneRadio(static_cast<eRadioID>(r))
#define SetStatValue(s, v) SetStatValue(static_cast<eStats>(s), (v))
#define RegisterFastestTime(s, t) RegisterFastestTime(static_cast<eStats>(s), (t))

// Cheats
#define IsCheatActive(c) IsActive(c)

// Pickups

// Vehicle types & damage
#define VEHICLE_TYPE_CAR VEHICLE_TYPE_AUTOMOBILE
#define Damage m_damageManager
#define CAR_ON_FIRE_HEALTH 250.0f

// Remote car wrapper
#define GivePlayerRemoteControlledCar(x, y, z, r, m) GivePlayerRemoteControlledCar(CVector((x), (y), (z)), (r), (int16)(m))

// Automobile & Heli
#define TellHeliToGoToCoors(x, y, z, minH, lowH) TellHeliToGoToCoors(CVector((x), (y), (z)), (minH), (lowH))
#define CreateDeadPedPickupCoors(x, y, z) CreateDeadPedPickupCoors(*(x), *(y), *(z))

// Handling & wheels
#define mFlags m_nModelFlags
#define pHandling m_pHandlingData
#define m_aWheelAngularVelocity m_wheelSpeed

// Area & Streaming
#define eVisibleArea eAreaCodes
#define RemoveBuildingsNotInArea(a) RemoveBuildingsNotInArea(static_cast<eAreaCodes>(a))
#define SetMissionDoesntRequireAnimations SetMissionDoesntRequireAnim

// Radar Blips
#define SetShortRangeCoordBlip(t, c, col, d, s) SetShortRangeCoordBlip((t), (c), static_cast<eBlipColour>(col), static_cast<eBlipDisplay>(d), (s))

// Physical flags & entities
#define bCoorsFrozenByScript bDontApplySpeed
#define bInfiniteMassFixed bCollidable
#define m_pNOCollisionVehicle m_pEntityIgnoredCollision
#define m_areaCode m_AreaCode

// Audio & CJ Mood
#define SetCJMood(m, ...) SetCJMood(static_cast<eCJMood>(m), __VA_ARGS__)
#define tAudioEvent eAudioEvents

// Anim block
#define m_loaded IsLoaded

// Set pieces

// PlayerInfo
#define nBestCarTwoWheelsTimeMs m_nBestCarTwoWheelsTimeMs
#define fBestCarTwoWheelsDistM m_fBestCarTwoWheelsDistM
#define nBestBikeWheelieTimeMs m_nBestBikeWheelieTimeMs
#define fBestBikeWheelieDistM m_fBestBikeWheelieDistM
#define nBestBikeStoppieTimeMs m_nBestBikeStoppieTimeMs
#define fBestBikeStoppieDistM m_fBestBikeStoppieDistM
#define PlayerPedData m_PlayerData
#define bCanDoDriveBy m_bCanDoDriveBy
#define FireProof m_bFireProof

// Population & Tasks
#define AddPed(t, m, ...) AddPed(static_cast<ePedType>(t), static_cast<eModelID>(m), __VA_ARGS__)
#define SetTask(t, p, ...) SetTask((t), static_cast<ePrimaryTasks>(p), __VA_ARGS__)

// Weapon & Cutscenes & Cheats & Models & Damage
#define pLastDamageEntity m_pLastDamageEntity
#define m_LastTimeGunFired m_nGunFiringTime
#define m_eStoredWeapon m_nSavedWeapon
#define WasCutsceneSkipped() ms_wasCutsceneSkipped
#define MODELID_BOAT_PREDATOR MODEL_PREDATOR
#define EnableLegimateCheat(c) ApplyCheat(c)

// Cam
#define Source m_vecSource
#define Front m_vecFront
#define Up m_vecUp
#define WorldViewerBeingUsed m_bWorldViewerBeingUsed

// Wheels & Damage
#define GetWheelStatus(w) GetWheelStatus(static_cast<eCarWheel>(w))

// PlayerPedData & PlayerInfo
#define m_pClothes m_pPedClothesDesc
#define m_bFadeDrunkenness m_nFadeDrunkenness
#define MaxHealth m_nMaxHealth
#define MaxArmour m_nMaxArmour

// Vehicle
#define m_nNoOfStaticFrames m_nFakePhysics

// Stats
#define RegisterBestPosition(s, p) RegisterBestPosition(static_cast<eStats>(s), (p))
#define IncrementStat(s, v) IncrementStat(static_cast<eStats>(s), (v))
#define GetStatValue(s) GetStatValue(static_cast<eStats>(s))

// Aliases for Tasks & Models
#define TTaskComplexSeekEntityStandard CTaskComplexSeekEntityStandard
#define MODELID_CAR_TAXI MODEL_TAXI
#define MODELID_CAR_CABBIE MODEL_CABBIE

// Resource / Thing types
#define UNIQUE_SCRIPT_SPHERE SCRIPT_THING_SPHERE
#define UNIQUE_SCRIPT_EFFECT_SYSTEM SCRIPT_THING_EFFECT_SYSTEM
#define UNIQUE_SCRIPT_SEARCHLIGHT SCRIPT_THING_SEARCH_LIGHT
#define UNIQUE_SCRIPT_CHECKPOINT SCRIPT_THING_CHECKPOINT
#define UNIQUE_SCRIPT_SEQUENCE_TASK SCRIPT_THING_SEQUENCE_TASK
#define UNIQUE_SCRIPT_FIRE SCRIPT_THING_FIRE
#define UNIQUE_SCRIPT_ATTRACTOR SCRIPT_THING_2D_EFFECT
#define UNIQUE_SCRIPT_DECISION_MAKER SCRIPT_THING_DECISION_MAKER
#define UNIQUE_SCRIPT_PEDGROUP SCRIPT_THING_PED_GROUP

// Stats mappings
#define PROGRESS_MADE STAT_PROGRESS_MADE
#define TOTAL_PROGRESS STAT_TOTAL_PROGRESS
#define FURTHEST_HOOP STAT_FURTHEST_HOOP
#define DISTANCE_TRAVELLED_ON_FOOT STAT_DISTANCE_TRAVELLED_ON_FOOT
#define DISTANCE_TRAVELLED_BY_CAR STAT_DISTANCE_TRAVELLED_BY_CAR
#define DISTANCE_TRAVELLED_BY_MOTORBIKE STAT_DISTANCE_TRAVELLED_BY_MOTORBIKE
#define DISTANCE_TRAVELLED_BY_BOAT STAT_DISTANCE_TRAVELLED_BY_BOAT
#define DISTANCE_TRAVELLED_BY_GOLF_CART STAT_DISTANCE_TRAVELLED_BY_GOLF_CART
#define DISTANCE_TRAVELLED_BY_HELICOPTER STAT_DISTANCE_TRAVELLED_BY_HELICOPTER
#define DISTANCE_TRAVELLED_BY_PLANE STAT_DISTANCE_TRAVELLED_BY_PLANE
#define LONGEST_WHEELIE_DISTANCE STAT_LONGEST_WHEELIE_DISTANCE
#define LONGEST_STOPPIE_DISTANCE STAT_LONGEST_STOPPIE_DISTANCE
#define LONGEST_2_WHEELS_DISTANCE STAT_LONGEST_2_WHEELS_DISTANCE
#define WEAPON_BUDGET STAT_WEAPON_BUDGET
#define FASHION_BUDGET STAT_FASHION_BUDGET
#define PROPERTY_BUDGET STAT_PROPERTY_BUDGET
#define AUTO_REPAIR_AND_PAINTING_BUDGET STAT_AUTO_REPAIR_AND_PAINTING_BUDGET
#define LONGEST_WHEELIE_TIME STAT_LONGEST_WHEELIE_TIME
#define LONGEST_STOPPIE_TIME STAT_LONGEST_STOPPIE_TIME
#define LONGEST_2_WHEELS_TIME STAT_LONGEST_2_WHEELS_TIME
#define FOOD_BUDGET STAT_FOOD_BUDGET
#define FAT STAT_FAT
#define STAMINA STAT_STAMINA
#define MUSCLE STAT_MUSCLE
#define MAX_HEALTH STAT_MAX_HEALTH
#define SEX_APPEAL STAT_SEX_APPEAL
#define DISTANCE_TRAVELLED_BY_SWIMMING STAT_DISTANCE_TRAVELLED_BY_SWIMMING
#define DISTANCE_TRAVELLED_BY_BICYCLE STAT_DISTANCE_TRAVELLED_BY_BICYCLE
#define DISTANCE_TRAVELLED_ON_TREADMILL STAT_DISTANCE_TRAVELLED_ON_TREADMILL
#define DISTANCE_TRAVELLED_ON_EXERCISE_BIKE STAT_DISTANCE_TRAVELLED_ON_EXERCISE_BIKE
#define TATTOO_BUDGET STAT_TATTOO_BUDGET
#define HAIRDRESSING_BUDGET STAT_HAIRDRESSING_BUDGET
#define GIRLFRIEND_BUDGET STAT_GIRLFRIEND_BUDGET
#define PROSTITUTE_BUDGET STAT_PROSTITUTE_BUDGET
#define FURNITURE_BUDGET STAT_FURNITURE_BUDGET
#define MONEY_SPENT_GAMBLING STAT_MONEY_SPENT_GAMBLING
#define MONEY_MADE_FROM_PIMPING STAT_MONEY_MADE_FROM_PIMPING
#define MONEY_WON_GAMBLING STAT_MONEY_WON_GAMBLING
#define BIGGEST_GAMBLING_WIN STAT_BIGGEST_GAMBLING_WIN
#define BIGGEST_GAMBLING_LOSS STAT_BIGGEST_GAMBLING_LOSS
#define LARGEST_BURGLARY_SWAG STAT_LARGEST_BURGLARY_SWAG
#define MONEY_MADE_FROM_BURGLARY STAT_MONEY_MADE_FROM_BURGLARY
#define MONEY_SPENT_BUILDING_PROPERTY STAT_MONEY_SPENT_BUILDING_PROPERTY
#define LONGEST_TREADMILL_TIME STAT_LONGEST_TREADMILL_TIME
#define LONGEST_EXERCISE_BIKE_TIME STAT_LONGEST_EXERCISE_BIKE_TIME
#define HEAVIEST_WEIGHT_ON_BENCH_PRESS STAT_HEAVIEST_WEIGHT_ON_BENCH_PRESS
#define HEAVIEST_WEIGHT_ON_DUMBBELLS STAT_HEAVIEST_WEIGHT_ON_DUMBBELLS
#define BEST_TIME_IN_8_TRACK STAT_BEST_TIME_IN_8_TRACK
#define BMX_BEST_TIME STAT_BMX_BEST_TIME
#define LIGHTEST_WEIGHT STAT_LIGHTEST_WEIGHT
#define LONGEST_CHASE_TIME_WITH_5_OR_MORE_STARS STAT_LONGEST_CHASE_TIME_WITH_5_OR_MORE_STARS
#define LAST_CHASE_TIME_WITH_5_OR_MORE_STARS STAT_LAST_CHASE_TIME_WITH_5_OR_MORE_STARS
#define WAGE_BILL STAT_WAGE_BILL
#define STRIP_CLUB_BUDGET STAT_STRIP_CLUB_BUDGET
#define CAR_MODIFICATION_BUDGET STAT_CAR_MODIFICATION_BUDGET
#define TIME_SPENT_SHOPPING STAT_TIME_SPENT_SHOPPING
#define TIME_SPENT_GAMBLING STAT_TIME_SPENT_GAMBLING
#define TIME_SPENT_ON_LONGEST_MISSION STAT_TIME_SPENT_ON_LONGEST_MISSION
#define TIME_SPENT_ON_QUICKEST_MISSION STAT_TIME_SPENT_ON_QUICKEST_MISSION
#define AVERAGE_MISSION_TIME STAT_AVERAGE_MISSION_TIME
#define DRUGS_BUDGET STAT_DRUGS_BUDGET
#define TOTAL_SHOPPING_BUDGET STAT_TOTAL_SHOPPING_BUDGET
#define TIME_SPENT_UNDERWATER STAT_TIME_SPENT_UNDERWATER
#define TOTAL_RESPECT STAT_TOTAL_RESPECT
#define GIRLFRIEND_RESPECT STAT_GIRLFRIEND_RESPECT
#define CLOTHES_RESPECT STAT_CLOTHES_RESPECT
#define FITNESS_RESPECT STAT_FITNESS_RESPECT
#define RESPECT STAT_RESPECT
#define PISTOL_SKILL STAT_PISTOL_SKILL
#define SILENCED_PISTOL_SKILL STAT_SILENCED_PISTOL_SKILL
#define DESERT_EAGLE_SKILL STAT_DESERT_EAGLE_SKILL
#define SHOTGUN_SKILL STAT_SHOTGUN_SKILL
#define SAWN_OFF_SHOTGUN_SKILL STAT_SAWN_OFF_SHOTGUN_SKILL
#define COMBAT_SHOTGUN_SKILL STAT_COMBAT_SHOTGUN_SKILL
#define MACHINE_PISTOL_SKILL STAT_MACHINE_PISTOL_SKILL
#define SMG_SKILL STAT_SMG_SKILL
#define AK_47_SKILL STAT_AK_47_SKILL
#define M4_SKILL STAT_M4_SKILL
#define RIFLE_SKILL STAT_RIFLE_SKILL
#define APPEARANCE STAT_APPEARANCE
#define GAMBLING STAT_GAMBLING
#define PEOPLE_WASTED_BY_OTHERS STAT_PEOPLE_WASTED_BY_OTHERS
#define PEOPLE_YOUVE_WASTED STAT_PEOPLE_YOUVE_WASTED
#define ROAD_VEHICLES_DESTROYED STAT_ROAD_VEHICLES_DESTROYED
#define BOATS_DESTROYED STAT_BOATS_DESTROYED
#define PLANES_HELICOPTERS_DESTROYED STAT_PLANES_HELICOPTERS_DESTROYED
#define COST_OF_PROPERTY_DAMAGED STAT_COST_OF_PROPERTY_DAMAGED
#define BULLETS_FIRED STAT_BULLETS_FIRED
#define KGS_OF_EXPLOSIVES_USED STAT_KGS_OF_EXPLOSIVES_USED
#define BULLETS_THAT_HIT STAT_BULLETS_THAT_HIT
#define TIRES_POPPED_WITH_GUNFIRE STAT_TIRES_POPPED_WITH_GUNFIRE
#define NUMBER_OF_HEADSHOTS STAT_NUMBER_OF_HEADSHOTS
#define TOTAL_NUMBER_OF_WANTED_STARS_ATTAINED STAT_TOTAL_NUMBER_OF_WANTED_STARS_ATTAINED
#define TOTAL_NUMBER_OF_WANTED_STARS_EVADED STAT_TOTAL_NUMBER_OF_WANTED_STARS_EVADED
#define TIMES_BUSTED STAT_TIMES_BUSTED
#define DAYS_PASSED_IN_GAME STAT_DAYS_PASSED_IN_GAME
#define NUMBER_OF_HOSPITAL_VISITS STAT_NUMBER_OF_HOSPITAL_VISITS
#define SAFEHOUSE_VISITS STAT_SAFEHOUSE_VISITS
#define TIMES_CHEATED STAT_TIMES_CHEATED
#define VEHICLE_RESPRAYS STAT_VEHICLE_RESPRAYS
#define MAXIMUM_INSANE_JUMP_DISTANCE STAT_MAXIMUM_INSANE_JUMP_DISTANCE
#define MAXIMUM_INSANE_JUMP_HEIGHT STAT_MAXIMUM_INSANE_JUMP_HEIGHT
#define MAXIMUM_INSANE_JUMP_FLIPS STAT_MAXIMUM_INSANE_JUMP_FLIPS
#define MAXIMUM_INSANE_JUMP_ROTATION STAT_MAXIMUM_INSANE_JUMP_ROTATION
#define BEST_INSANE_STUNT_AWARDED STAT_BEST_INSANE_STUNT_AWARDED
#define UNIQUE_JUMPS_FOUND STAT_UNIQUE_JUMPS_FOUND
#define UNIQUE_JUMPS_DONE STAT_UNIQUE_JUMPS_DONE
#define MISSION_ATTEMPTS STAT_MISSION_ATTEMPTS
#define MISSIONS_PASSED STAT_MISSIONS_PASSED
#define TOTAL_NUMBER_OF_MISSIONS_IN_GAME STAT_TOTAL_NUMBER_OF_MISSIONS_IN_GAME
#define CASH_MADE_IN_A_TAXI STAT_CASH_MADE_IN_A_TAXI
#define PASSENGERS_DROPPED_OFF STAT_PASSENGERS_DROPPED_OFF
#define PEOPLE_SAVED_IN_AN_AMBULANCE STAT_PEOPLE_SAVED_IN_AN_AMBULANCE
#define CRIMINALS_KILLED_ON_VIGILANTE_MISSION STAT_CRIMINALS_KILLED_ON_VIGILANTE_MISSION
#define TOTAL_FIRES_EXTINGUISHED STAT_TOTAL_FIRES_EXTINGUISHED
#define PACKAGES_DELIVERED STAT_PACKAGES_DELIVERED
#define ASSASSINATIONS STAT_ASSASSINATIONS
#define LAST_DANCE_SCORE STAT_LAST_DANCE_SCORE
#define HIGHEST_VIGILANTE_MISSION_LEVEL STAT_HIGHEST_VIGILANTE_MISSION_LEVEL
#define HIGHEST_PARAMEDIC_MISSION_LEVEL STAT_HIGHEST_PARAMEDIC_MISSION_LEVEL
#define HIGHEST_FIREFIGHTER_MISSION_LEVEL STAT_HIGHEST_FIREFIGHTER_MISSION_LEVEL
#define DRIVING_SKILL STAT_DRIVING_SKILL
#define NUMBER_OF_TRUCK_MISSIONS_PASSED STAT_NUMBER_OF_TRUCK_MISSIONS_PASSED
#define MONEY_MADE_IN_TRUCK STAT_MONEY_MADE_IN_TRUCK
#define RECRUITED_GANG_MEMBERS_KILLED STAT_RECRUITED_GANG_MEMBERS_KILLED
#define ARMOR STAT_ARMOR
#define ENERGY STAT_ENERGY
#define PHOTOGRAPHS_TAKEN STAT_PHOTOGRAPHS_TAKEN
#define RAMPAGES_ATTEMPTED STAT_RAMPAGES_ATTEMPTED
#define RAMPAGES_PASSED STAT_RAMPAGES_PASSED
#define FLIGHT_TIME STAT_FLIGHT_TIME
#define TIMES_DROWNED STAT_TIMES_DROWNED
#define NUMBER_OF_GIRLS_PIMPED STAT_NUMBER_OF_GIRLS_PIMPED
#define BEST_POSITION_IN_8_TRACK STAT_BEST_POSITION_IN_8_TRACK
#define TIME_ON_JETPACK STAT_TIME_ON_JETPACK
#define SHOOTING_RANGE_LEVELS_PASSED STAT_SHOOTING_RANGE_LEVELS_PASSED
#define MOST_CARS_PARKED_ON_VALET_PARKING STAT_MOST_CARS_PARKED_ON_VALET_PARKING
#define KILLS_SINCE_LAST_CHECKPOINT STAT_KILLS_SINCE_LAST_CHECKPOINT
#define TOTAL_LEGITIMATE_KILLS STAT_TOTAL_LEGITIMATE_KILLS
#define NUMBER_OF_BLOODRING_KILLS STAT_NUMBER_OF_BLOODRING_KILLS
#define TOTAL_TIME_IN_BLOODRING STAT_TOTAL_TIME_IN_BLOODRING
#define NO_MORE_HURRICANES_FLAG STAT_NO_MORE_HURRICANES_FLAG
#define CITY_UNLOCKED STAT_CITY_UNLOCKED
#define NUMBER_OF_POLICE_BRIBES STAT_NUMBER_OF_POLICE_BRIBES
#define NUMBER_OF_CARS_STOLEN STAT_NUMBER_OF_CARS_STOLEN
#define CURRENT_NUMBER_OF_GIRLFRIENDS STAT_CURRENT_NUMBER_OF_GIRLFRIENDS
#define NUMBER_OF_DISASTROUS_DATES STAT_NUMBER_OF_DISASTROUS_DATES
#define NUMBER_OF_GIRLS_DATED STAT_NUMBER_OF_GIRLS_DATED
#define NUMBER_OF_TIMES_SCORED_WITH_A_GIRL STAT_NUMBER_OF_TIMES_SCORED_WITH_A_GIRL
#define NUMBER_OF_SUCCESSFUL_DATES STAT_NUMBER_OF_SUCCESSFUL_DATES
#define NUMBER_OF_GIRLS_DUMPED STAT_NUMBER_OF_GIRLS_DUMPED
#define NUMBER_OF_PROSTITUTES_VISITED STAT_NUMBER_OF_PROSTITUTES_VISITED
#define NUMBER_OF_HOUSES_BURGLED STAT_NUMBER_OF_HOUSES_BURGLED
#define NUMBER_OF_SAFES_CRACKED STAT_NUMBER_OF_SAFES_CRACKED
#define BURGULAR_STATUS STAT_BURGULAR_STATUS
#define NUMBER_OF_STOLEN_ITEMS_SOLD STAT_NUMBER_OF_STOLEN_ITEMS_SOLD
#define NUMBER_OF_EIGHT_BALLS_IN_POOL STAT_NUMBER_OF_EIGHT_BALLS_IN_POOL
#define TOTAL_NUMBER_OF_WINS_PLAYING_POOL STAT_TOTAL_NUMBER_OF_WINS_PLAYING_POOL
#define TOTAL_NUMBER_OF_LOSSES_PLAYING_POOL STAT_TOTAL_NUMBER_OF_LOSSES_PLAYING_POOL
#define TIMES_VISITED_THE_GYM STAT_TIMES_VISITED_THE_GYM
#define BODY_TYPE STAT_BODY_TYPE
#define NUMBER_OF_MEALS_EATEN STAT_NUMBER_OF_MEALS_EATEN
#define MONEY_MADE_IN_QUARRY STAT_MONEY_MADE_IN_QUARRY
#define TIME_TAKEN_TO_COMPLETE_QUARRY STAT_TIME_TAKEN_TO_COMPLETE_QUARRY
#define NUMBER_OF_FURNITURE_PURCHASED STAT_NUMBER_OF_FURNITURE_PURCHASED
#define FAVORITE_CONSOLE_GAME STAT_FAVORITE_CONSOLE_GAME
#define HIGHEST_CIVILIAN_PEDS_KILLED_ON_RAMPAGE STAT_HIGHEST_CIVILIAN_PEDS_KILLED_ON_RAMPAGE
#define HIGHEST_POLICE_PEDS_KILLED_ON_RAMPAGE STAT_HIGHEST_POLICE_PEDS_KILLED_ON_RAMPAGE
#define HIGHEST_CIVILIAN_VEHICLES_DESTROYED_ON_RAMPAGE STAT_HIGHEST_CIVILIAN_VEHICLES_DESTROYED_ON_RAMPAGE
#define HIGHEST_POLICE_VEHICLES_DESTROYED_ON_RAMPAGE STAT_HIGHEST_POLICE_VEHICLES_DESTROYED_ON_RAMPAGE
#define HIGHEST_NUMBER_OF_TANKS_DESTROYED_ON_RAMPAGE STAT_HIGHEST_NUMBER_OF_TANKS_DESTROYED_ON_RAMPAGE
#define PIMPING_LEVEL STAT_PIMPING_LEVEL
#define KICKSTART_BEST_SCORE STAT_KICKSTART_BEST_SCORE
#define BEST_LAP_TIME_IN_8_TRACK STAT_BEST_LAP_TIME_IN_8_TRACK
#define NUMBER_OF_VEHICLES_EXPORTED STAT_NUMBER_OF_VEHICLES_EXPORTED
#define NUMBER_OF_VEHICLES_IMPORTED STAT_NUMBER_OF_VEHICLES_IMPORTED
#define HIGHEST_BASKETBALL_SCORE STAT_HIGHEST_BASKETBALL_SCORE
#define FIRES_STARTED STAT_FIRES_STARTED
#define AMOUNT_OF_DRUGS_SOLD STAT_AMOUNT_OF_DRUGS_SOLD
#define AMOUNT_OF_DRUGS_BOUGHT STAT_AMOUNT_OF_DRUGS_BOUGHT
#define BEST_LAP_TIME_IN_DIRT_TRACK STAT_BEST_LAP_TIME_IN_DIRT_TRACK
#define BEST_TIME_IN_DIRT_TRACK STAT_BEST_TIME_IN_DIRT_TRACK
#define BEST_POSITION_IN_DIRT_TRACK STAT_BEST_POSITION_IN_DIRT_TRACK
#define NRG_500_BEST_TIME STAT_NRG_500_BEST_TIME
#define FLYING_SKILL STAT_FLYING_SKILL
#define RESPECT_MISSION STAT_RESPECT_MISSION
#define LUNG_CAPACITY STAT_LUNG_CAPACITY
#define GAME_GORE_RATING STAT_GAME_GORE_RATING
#define GAME_SEX_RATING STAT_GAME_SEX_RATING
#define RESPECT_MISSION_TOTAL STAT_RESPECT_MISSION_TOTAL
#define BIKE_SKILL STAT_BIKE_SKILL
#define CYCLING_SKILL STAT_CYCLING_SKILL
#define SNAPSHOTS_TAKEN STAT_SNAPSHOTS_TAKEN
#define TOTAL_SNAPSHOTS STAT_TOTAL_SNAPSHOTS
#define LUCK STAT_LUCK
#define TERRITORIES_TAKEN_OVER STAT_TERRITORIES_TAKEN_OVER
#define TERRITORIES_LOST STAT_TERRITORIES_LOST
#define TERRITORIES_HELD STAT_TERRITORIES_HELD
#define HIGHEST_NUMBER_OF_TERRITORIES_HELD STAT_HIGHEST_NUMBER_OF_TERRITORIES_HELD
#define GANG_MEMBERS_RECRUITED STAT_GANG_MEMBERS_RECRUITED
#define ENEMY_GANG_MEMBERS_KILLED STAT_ENEMY_GANG_MEMBERS_KILLED
#define FRIENDLY_GANG_MEMBERS_KILLED STAT_FRIENDLY_GANG_MEMBERS_KILLED
#define HORSESHOES_COLLECTED STAT_HORSESHOES_COLLECTED
#define TOTAL_HORSESHOES STAT_TOTAL_HORSESHOES
#define OYSTERS_COLLECTED STAT_OYSTERS_COLLECTED
#define TOTAL_OYSTERS STAT_TOTAL_OYSTERS
#define CALORIES STAT_CALORIES
#define KEEPIE_UPPY_BEST_TIME STAT_KEEPIE_UPPY_BEST_TIME
#define PROGRESS_WITH_DENISE STAT_PROGRESS_WITH_DENISE
#define PROGRESS_WITH_MICHELLE STAT_PROGRESS_WITH_MICHELLE
#define PROGRESS_WITH_HELENA STAT_PROGRESS_WITH_HELENA
#define PROGRESS_WITH_BARBARA STAT_PROGRESS_WITH_BARBARA
#define PROGRESS_WITH_KATIE STAT_PROGRESS_WITH_KATIE
#define PROGRESS_WITH_MILLIE STAT_PROGRESS_WITH_MILLIE
#define BEST_POSITION_IN_LOWRIDER_RACE STAT_BEST_POSITION_IN_LOWRIDER_RACE
#define BEST_TIME_IN_LOWRIDER_RACE STAT_BEST_TIME_IN_LOWRIDER_RACE
#define BEST_POSITION_IN_LITTLE_LOOP STAT_BEST_POSITION_IN_LITTLE_LOOP
#define BEST_TIME_IN_LITTLE_LOOP STAT_BEST_TIME_IN_LITTLE_LOOP
#define BEST_POSITION_IN_BACKROAD_WANDERER STAT_BEST_POSITION_IN_BACKROAD_WANDERER
#define BEST_TIME_IN_BACKROAD_WANDERER STAT_BEST_TIME_IN_BACKROAD_WANDERER
#define BEST_POSITION_IN_CITY_CIRCUIT STAT_BEST_POSITION_IN_CITY_CIRCUIT
#define BEST_TIME_IN_CITY_CIRCUIT STAT_BEST_TIME_IN_CITY_CIRCUIT
#define BEST_POSITION_IN_VINEWOOD STAT_BEST_POSITION_IN_VINEWOOD
#define BEST_TIME_IN_VINEWOOD STAT_BEST_TIME_IN_VINEWOOD
#define BEST_POSITION_IN_FREEWAY STAT_BEST_POSITION_IN_FREEWAY
#define BEST_TIME_IN_FREEWAY STAT_BEST_TIME_IN_FREEWAY
#define BEST_POSITION_IN_INTO_THE_COUNTRY STAT_BEST_POSITION_IN_INTO_THE_COUNTRY
#define BEST_TIME_IN_INTO_THE_COUNTRY STAT_BEST_TIME_IN_INTO_THE_COUNTRY
#define BEST_POSITION_IN_BADLANDS_A STAT_BEST_POSITION_IN_BADLANDS_A
#define BEST_TIME_IN_BADLANDS_A STAT_BEST_TIME_IN_BADLANDS_A
#define BEST_POSITION_IN_BADLANDS_B STAT_BEST_POSITION_IN_BADLANDS_B
#define BEST_TIME_IN_BADLANDS_B STAT_BEST_TIME_IN_BADLANDS_B
#define BEST_POSITION_IN_DIRTBIKE_DANGER STAT_BEST_POSITION_IN_DIRTBIKE_DANGER
#define BEST_TIME_IN_DIRTBIKE_DANGER STAT_BEST_TIME_IN_DIRTBIKE_DANGER
#define BEST_POSITION_IN_BANDITO_COUNTY STAT_BEST_POSITION_IN_BANDITO_COUNTY
#define BEST_TIME_IN_BANDITO_COUNTY STAT_BEST_TIME_IN_BANDITO_COUNTY
#define BEST_POSITION_IN_GO_GO_KART STAT_BEST_POSITION_IN_GO_GO_KART
#define BEST_TIME_IN_GO_GO_KART STAT_BEST_TIME_IN_GO_GO_KART
#define BEST_POSITION_IN_SAN_FIERRO_FASTLANE STAT_BEST_POSITION_IN_SAN_FIERRO_FASTLANE
#define BEST_TIME_IN_SAN_FIERRO_FASTLANE STAT_BEST_TIME_IN_SAN_FIERRO_FASTLANE
#define BEST_POSITION_IN_SAN_FIERRO_HILLS STAT_BEST_POSITION_IN_SAN_FIERRO_HILLS
#define BEST_TIME_IN_SAN_FIERRO_HILLS STAT_BEST_TIME_IN_SAN_FIERRO_HILLS
#define BEST_POSITION_IN_COUNTRY_ENDURANCE STAT_BEST_POSITION_IN_COUNTRY_ENDURANCE
#define BEST_TIME_IN_COUNTRY_ENDURANCE STAT_BEST_TIME_IN_COUNTRY_ENDURANCE
#define BEST_POSITION_IN_SF_TO_LV STAT_BEST_POSITION_IN_SF_TO_LV
#define BEST_TIME_IN_SF_TO_LV STAT_BEST_TIME_IN_SF_TO_LV
#define BEST_POSITION_IN_DAM_RIDER STAT_BEST_POSITION_IN_DAM_RIDER
#define BEST_TIME_IN_DAM_RIDER STAT_BEST_TIME_IN_DAM_RIDER
#define BEST_POSITION_IN_DESERT_TRICKS STAT_BEST_POSITION_IN_DESERT_TRICKS
#define BEST_TIME_IN_DESERT_TRICKS STAT_BEST_TIME_IN_DESERT_TRICKS
#define BEST_POSITION_IN_LV_RINGROAD STAT_BEST_POSITION_IN_LV_RINGROAD
#define BEST_TIME_IN_LV_RINGROAD STAT_BEST_TIME_IN_LV_RINGROAD
#define BEST_TIME_IN_WORLD_WAR_ACES STAT_BEST_TIME_IN_WORLD_WAR_ACES
#define BEST_TIME_IN_BARNSTORMING STAT_BEST_TIME_IN_BARNSTORMING
#define BEST_TIME_IN_MILITARY_SERVICE STAT_BEST_TIME_IN_MILITARY_SERVICE
#define BEST_TIME_IN_CHOPPER_CHECKPOINT STAT_BEST_TIME_IN_CHOPPER_CHECKPOINT
#define BEST_TIME_IN_WHIRLY_BIRD_WAYPOINT STAT_BEST_TIME_IN_WHIRLY_BIRD_WAYPOINT
#define BEST_TIME_IN_HELI_HELL STAT_BEST_TIME_IN_HELI_HELL
#define DRIVE_THRU_MISSION_ACCOMPLISHED STAT_DRIVE_THRU_MISSION_ACCOMPLISHED
#define MANAGEMENT_ISSUES_MISSION_ACCOMPLISHED STAT_MANAGEMENT_ISSUES_MISSION_ACCOMPLISHED
#define YAY_KA_BOOM_BOOM_MISSION_ACCOMPLISHED STAT_YAY_KA_BOOM_BOOM_MISSION_ACCOMPLISHED
#define FISH_IN_A_BARREL_MISSION_ACCOMPLISHED STAT_FISH_IN_A_BARREL_MISSION_ACCOMPLISHED
#define BREAKING_THE_BANK_AT_CALIGULAS_MISSION_ACCOMPLISHED STAT_BREAKING_THE_BANK_AT_CALIGULAS_MISSION_ACCOMPLISHED
#define A_HOME_IN_THE_HILLS_MISSION_ACCOMPLISHED STAT_A_HOME_IN_THE_HILLS_MISSION_ACCOMPLISHED
#define MAYBE_SET_RIOT_MODE STAT_MAYBE_SET_RIOT_MODE
#define RYDERS_MISSION_ROBBING_UNCLE_SAM_ACCOMPLISHED STAT_RYDERS_MISSION_ROBBING_UNCLE_SAM_ACCOMPLISHED
#define MIKE_TORENO_MISSION_ACCOMPLISHED STAT_MIKE_TORENO_MISSION_ACCOMPLISHED
#define ARCHITECTURAL_ESPIONAGE_MISSION_ACCOMPLISHED STAT_ARCHITECTURAL_ESPIONAGE_MISSION_ACCOMPLISHED
#define JIZZY_MISSION_ACCOMPLISHED STAT_JIZZY_MISSION_ACCOMPLISHED
#define REUNITING_THE_FAMILIES_MISSION_ACCOMPLISHED STAT_REUNITING_THE_FAMILIES_MISSION_ACCOMPLISHED
#define SMALL_TOWN_BANK_MISSION_ACCOMPLISHED STAT_SMALL_TOWN_BANK_MISSION_ACCOMPLISHED
#define PHOTO_OPPORTUNITY_MISSION_ACCOMPLISHED STAT_PHOTO_OPPORTUNITY_MISSION_ACCOMPLISHED
#define DON_PEYOTE_MISSION_ACCOMPLISHED STAT_DON_PEYOTE_MISSION_ACCOMPLISHED
#define LOCAL_LIQUOR_STORE_MISSION_ACCOMPLISHED STAT_LOCAL_LIQUOR_STORE_MISSION_ACCOMPLISHED
#define BADLANDS_MISSION_ACCOMPLISHED STAT_BADLANDS_MISSION_ACCOMPLISHED
#define PLAYING_TIME STAT_PLAYING_TIME
#define HIDDEN_PACKAGES_FOUND STAT_HIDDEN_PACKAGES_FOUND
#define TAGS_SPRAYED STAT_TAGS_SPRAYED
#define LEAST_FAVORITE_GANG STAT_LEAST_FAVORITE_GANG
#define GANG_MEMBERS_WASTED STAT_GANG_MEMBERS_WASTED
#define CRIMINALS_WASTED STAT_CRIMINALS_WASTED
#define MOST_FAVORITE_RADIO_STATION STAT_MOST_FAVORITE_RADIO_STATION
#define LEAST_FAVORITE_RADIO_STATION STAT_LEAST_FAVORITE_RADIO_STATION
#define CURRENT_WEAPON_SKILL STAT_CURRENT_WEAPON_SKILL
#define WEAPON_SKILL_LEVELS STAT_WEAPON_SKILL_LEVELS
#define PILOT_RANKING STAT_PILOT_RANKING
#define STRONGEST_GANG STAT_STRONGEST_GANG
#define MONEY_LOST_GAMBLING STAT_MONEY_LOST_GAMBLING
#define RIOT_MISSION_ACCOMPLISHED STAT_RIOT_MISSION_ACCOMPLISHED
#define GANG_STRENGTH STAT_GANG_STRENGTH
#define TERRITORY_UNDER_CONTROL STAT_TERRITORY_UNDER_CONTROL
#define ARE_YOU_GOING_TO_SAN_FIERRO_MISSION_ACCOMPLISHED STAT_ARE_YOU_GOING_TO_SAN_FIERRO_MISSION_ACCOMPLISHED
#define HIGH_NOON_MISSION_ACCOMPLISHED STAT_HIGH_NOON_MISSION_ACCOMPLISHED
#define THE_GREEN_SABRE_MISSION_ACCOMPLISHED STAT_THE_GREEN_SABRE_MISSION_ACCOMPLISHED
#define MAYBE_CATALINA_MEETING STAT_MAYBE_CATALINA_MEETING
#define MAYBE_WU_ZI_MEETING STAT_MAYBE_WU_ZI_MEETING

// Weapon mappings
#define WEAPONTYPE_UNARMED WEAPON_UNARMED
#define WEAPONTYPE_BRASSKNUCKLE WEAPON_BRASSKNUCKLE
#define WEAPONTYPE_GOLFCLUB WEAPON_GOLFCLUB
#define WEAPONTYPE_NIGHTSTICK WEAPON_NIGHTSTICK
#define WEAPONTYPE_KNIFE WEAPON_KNIFE
#define WEAPONTYPE_BASEBALLBAT WEAPON_BASEBALLBAT
#define WEAPONTYPE_SHOVEL WEAPON_SHOVEL
#define WEAPONTYPE_POOL_CUE WEAPON_POOL_CUE
#define WEAPONTYPE_KATANA WEAPON_KATANA
#define WEAPONTYPE_CHAINSAW WEAPON_CHAINSAW
#define WEAPONTYPE_DILDO1 WEAPON_DILDO1
#define WEAPONTYPE_DILDO2 WEAPON_DILDO2
#define WEAPONTYPE_VIBE1 WEAPON_VIBE1
#define WEAPONTYPE_VIBE2 WEAPON_VIBE2
#define WEAPONTYPE_FLOWERS WEAPON_FLOWERS
#define WEAPONTYPE_CANE WEAPON_CANE
#define WEAPONTYPE_GRENADE WEAPON_GRENADE
#define WEAPONTYPE_TEARGAS WEAPON_TEARGAS
#define WEAPONTYPE_MOLOTOV WEAPON_MOLOTOV
#define WEAPONTYPE_ROCKET WEAPON_ROCKET
#define WEAPONTYPE_ROCKET_HS WEAPON_ROCKET_HS
#define WEAPONTYPE_FREEFALL_BOMB WEAPON_FREEFALL_BOMB
#define WEAPONTYPE_PISTOL WEAPON_PISTOL
#define WEAPONTYPE_PISTOL_SILENCED WEAPON_PISTOL_SILENCED
#define WEAPONTYPE_DESERT_EAGLE WEAPON_DESERT_EAGLE
#define WEAPONTYPE_SHOTGUN WEAPON_SHOTGUN
#define WEAPONTYPE_SAWNOFF_SHOTGUN WEAPON_SAWNOFF_SHOTGUN
#define WEAPONTYPE_SPAS12_SHOTGUN WEAPON_SPAS12_SHOTGUN
#define WEAPONTYPE_MICRO_UZI WEAPON_MICRO_UZI
#define WEAPONTYPE_MP5 WEAPON_MP5
#define WEAPONTYPE_AK47 WEAPON_AK47
#define WEAPONTYPE_M4 WEAPON_M4
#define WEAPONTYPE_TEC9 WEAPON_TEC9
#define WEAPONTYPE_COUNTRYRIFLE WEAPON_COUNTRYRIFLE
#define WEAPONTYPE_SNIPERRIFLE WEAPON_SNIPERRIFLE
#define WEAPONTYPE_RLAUNCHER WEAPON_RLAUNCHER
#define WEAPONTYPE_RLAUNCHER_HS WEAPON_RLAUNCHER_HS
#define WEAPONTYPE_FLAMETHROWER WEAPON_FLAMETHROWER
#define WEAPONTYPE_MINIGUN WEAPON_MINIGUN
#define WEAPONTYPE_REMOTE_SATCHEL_CHARGE WEAPON_REMOTE_SATCHEL_CHARGE
#define WEAPONTYPE_DETONATOR WEAPON_DETONATOR
#define WEAPONTYPE_SPRAYCAN WEAPON_SPRAYCAN
#define WEAPONTYPE_EXTINGUISHER WEAPON_EXTINGUISHER
#define WEAPONTYPE_CAMERA WEAPON_CAMERA
#define WEAPONTYPE_NIGHTVISION WEAPON_NIGHTVISION
#define WEAPONTYPE_INFRARED WEAPON_INFRARED
#define WEAPONTYPE_PARACHUTE WEAPON_PARACHUTE
#define WEAPONTYPE_LAST_WEAPON WEAPON_LAST_WEAPON
#define WEAPONTYPE_ARMOUR WEAPON_ARMOUR
#define WEAPONTYPE_RAMMEDBYCAR WEAPON_RAMMEDBYCAR
#define WEAPONTYPE_RUNOVERBYCAR WEAPON_RUNOVERBYCAR
#define WEAPONTYPE_EXPLOSION WEAPON_EXPLOSION
#define WEAPONTYPE_UZI_DRIVEBY WEAPON_UZI_DRIVEBY
#define WEAPONTYPE_DROWNING WEAPON_DROWNING
#define WEAPONTYPE_FALL WEAPON_FALL
#define WEAPONTYPE_UNIDENTIFIED WEAPON_UNIDENTIFIED
#define WEAPONTYPE_ANYMELEE WEAPON_ANYMELEE
#define WEAPONTYPE_ANYWEAPON WEAPON_ANYWEAPON
#define WEAPONTYPE_FLARE WEAPON_FLARE

// MARK: 0To99

// func: sa 0x465E60
OpcodeResult CRunningScript::ProcessCommands0To99(i32 CurrCommand)
{
    MARKFUNCTION(0x465E60);

    i32 PlayerIndex;
    bool LatestCmpFlagResult;
    CVector TempCoors;
    f32 NewZ, NewY, NewX;
    i32* pGlobalVar;
    i32* pGlobalVar2;
    i32* pLocalVar;
    i32* pLocalVar2;

    switch (CurrCommand)
    {
        case COMMAND_NOP:
        {
            return OR_CONTINUE;
        }
        case COMMAND_WAIT:
        {
            CollectParameters(1);
            ActivateTime = CTimer::m_snTimeInMilliseconds + ScriptParams[0];
            return OR_WAIT;
        }
        case COMMAND_GOTO:
        {
            CollectParameters(1);
            UpdatePC(ScriptParams[0]);
            return OR_CONTINUE;
        }
        case COMMAND_SHAKE_CAM:
        {
            CollectParameters(1);
            CamShakeNoPos(&TheCamera, static_cast<f32>(ScriptParams[0]) * 0.001f);
            return OR_CONTINUE;
        }
        case COMMAND_SET_VAR_INT:
        {
            pGlobalVar = GetPointerToScriptVariable(SCOPE_GLOBAL);
            CollectParameters(1);
            *pGlobalVar = ScriptParams[0];
            return OR_CONTINUE;
        }
        case COMMAND_SET_VAR_FLOAT:
        {
            pGlobalVar = GetPointerToScriptVariable(SCOPE_GLOBAL);
            CollectParameters(1);
            *(f32*)pGlobalVar = *(f32*)&ScriptParams[0];
            return OR_CONTINUE;
        }
        case COMMAND_SET_LVAR_INT:
        {
            pLocalVar = GetPointerToScriptVariable(SCOPE_LOCAL);
            CollectParameters(1);
            *pLocalVar = ScriptParams[0];
            return OR_CONTINUE;
        }
        case COMMAND_SET_LVAR_FLOAT:
        {
            pLocalVar = GetPointerToScriptVariable(SCOPE_LOCAL);
            CollectParameters(1);
            *(f32*)pLocalVar = *(f32*)&ScriptParams[0];
            return OR_CONTINUE;
        }
        case COMMAND_ADD_VAL_TO_INT_VAR:
        {
            pGlobalVar = GetPointerToScriptVariable(SCOPE_GLOBAL);
            CollectParameters(1);
            *pGlobalVar += ScriptParams[0];
            return OR_CONTINUE;
        }
        case COMMAND_ADD_VAL_TO_FLOAT_VAR:
        {
            pGlobalVar = GetPointerToScriptVariable(SCOPE_GLOBAL);
            CollectParameters(1);
            *(f32*)pGlobalVar += *(f32*)&ScriptParams[0];
            return OR_CONTINUE;
        }
        case COMMAND_ADD_VAL_TO_INT_LVAR:
        {
            pLocalVar = GetPointerToScriptVariable(SCOPE_LOCAL);
            CollectParameters(1);
            *pLocalVar += ScriptParams[0];
            return OR_CONTINUE;
        }
        case COMMAND_ADD_VAL_TO_FLOAT_LVAR:
        {
            pLocalVar = GetPointerToScriptVariable(SCOPE_LOCAL);
            CollectParameters(1);
            *(f32*)pLocalVar += *(f32*)&ScriptParams[0];
            return OR_CONTINUE;
        }
        case COMMAND_SUB_VAL_FROM_INT_VAR:
        {
            pGlobalVar = GetPointerToScriptVariable(SCOPE_GLOBAL);
            CollectParameters(1);
            *pGlobalVar -= ScriptParams[0];
            return OR_CONTINUE;
        }
        case COMMAND_SUB_VAL_FROM_FLOAT_VAR:
        {
            pGlobalVar = GetPointerToScriptVariable(SCOPE_GLOBAL);
            CollectParameters(1);
            *(f32*)pGlobalVar -= *(f32*)&ScriptParams[0];
            return OR_CONTINUE;
        }
        case COMMAND_SUB_VAL_FROM_INT_LVAR:
        {
            pLocalVar = GetPointerToScriptVariable(SCOPE_LOCAL);
            CollectParameters(1);
            *pLocalVar -= ScriptParams[0];
            return OR_CONTINUE;
        }
        case COMMAND_SUB_VAL_FROM_FLOAT_LVAR:
        {
            pLocalVar = GetPointerToScriptVariable(SCOPE_LOCAL);
            CollectParameters(1);
            *(f32*)pLocalVar -= *(f32*)&ScriptParams[0];
            return OR_CONTINUE;
        }
        case COMMAND_MULT_INT_VAR_BY_VAL:
        {
            pGlobalVar = GetPointerToScriptVariable(SCOPE_GLOBAL);
            CollectParameters(1);
            *pGlobalVar *= ScriptParams[0];
            return OR_CONTINUE;
        }
        case COMMAND_MULT_FLOAT_VAR_BY_VAL:
        {
            pGlobalVar = GetPointerToScriptVariable(SCOPE_GLOBAL);
            CollectParameters(1);
            *(f32*)pGlobalVar *= *(f32*)&ScriptParams[0];
            return OR_CONTINUE;
        }
        case COMMAND_MULT_INT_LVAR_BY_VAL:
        {
            pLocalVar = GetPointerToScriptVariable(SCOPE_LOCAL);
            CollectParameters(1);
            *pLocalVar *= ScriptParams[0];
            return OR_CONTINUE;
        }
        case COMMAND_MULT_FLOAT_LVAR_BY_VAL:
        {
            pLocalVar = GetPointerToScriptVariable(SCOPE_LOCAL);
            CollectParameters(1);
            *(f32*)pLocalVar *= *(f32*)&ScriptParams[0];
            return OR_CONTINUE;
        }
        case COMMAND_DIV_INT_VAR_BY_VAL:
        {
            pGlobalVar = GetPointerToScriptVariable(SCOPE_GLOBAL);
            CollectParameters(1);
            *pGlobalVar /= ScriptParams[0];
            return OR_CONTINUE;
        }
        case COMMAND_DIV_FLOAT_VAR_BY_VAL:
        {
            pGlobalVar = GetPointerToScriptVariable(SCOPE_GLOBAL);
            CollectParameters(1);
            *(f32*)pGlobalVar /= *(f32*)&ScriptParams[0];
            return OR_CONTINUE;
        }
        case COMMAND_DIV_INT_LVAR_BY_VAL:
        {
            pLocalVar = GetPointerToScriptVariable(SCOPE_LOCAL);
            CollectParameters(1);
            *pLocalVar /= ScriptParams[0];
            return OR_CONTINUE;
        }
        case COMMAND_DIV_FLOAT_LVAR_BY_VAL:
        {
            pLocalVar = GetPointerToScriptVariable(SCOPE_LOCAL);
            CollectParameters(1);
            *(f32*)pLocalVar /= *(f32*)&ScriptParams[0];
            return OR_CONTINUE;
        }
        case COMMAND_IS_INT_VAR_GREATER_THAN_NUMBER:
        {
            pGlobalVar = GetPointerToScriptVariable(SCOPE_GLOBAL);
            CollectParameters(1);
            LatestCmpFlagResult = (*pGlobalVar > ScriptParams[0]);
            UpdateCompareFlag(LatestCmpFlagResult);
            return OR_CONTINUE;
        }
        case COMMAND_IS_INT_LVAR_GREATER_THAN_NUMBER:
        {
            pLocalVar = GetPointerToScriptVariable(SCOPE_LOCAL);
            CollectParameters(1);
            LatestCmpFlagResult = (*pLocalVar > ScriptParams[0]);
            UpdateCompareFlag(LatestCmpFlagResult);
            return OR_CONTINUE;
        }
        case COMMAND_IS_NUMBER_GREATER_THAN_INT_VAR:
        {
            CollectParameters(1);
            pGlobalVar = GetPointerToScriptVariable(SCOPE_GLOBAL);
            LatestCmpFlagResult = (ScriptParams[0] > *pGlobalVar);
            UpdateCompareFlag(LatestCmpFlagResult);
            return OR_CONTINUE;
        }
        case COMMAND_IS_NUMBER_GREATER_THAN_INT_LVAR:
        {
            CollectParameters(1);
            pLocalVar = GetPointerToScriptVariable(SCOPE_LOCAL);
            LatestCmpFlagResult = (ScriptParams[0] > *pLocalVar);
            UpdateCompareFlag(LatestCmpFlagResult);
            return OR_CONTINUE;
        }
        case COMMAND_IS_INT_VAR_GREATER_THAN_INT_VAR:
        {
            pGlobalVar = GetPointerToScriptVariable(SCOPE_GLOBAL);
            pGlobalVar2 = GetPointerToScriptVariable(SCOPE_GLOBAL);
            LatestCmpFlagResult = (*pGlobalVar > *pGlobalVar2);
            UpdateCompareFlag(LatestCmpFlagResult);
            return OR_CONTINUE;
        }
        case COMMAND_IS_INT_LVAR_GREATER_THAN_INT_LVAR:
        {
            pLocalVar = GetPointerToScriptVariable(SCOPE_LOCAL);
            pLocalVar2 = GetPointerToScriptVariable(SCOPE_LOCAL);
            LatestCmpFlagResult = (*pLocalVar > *pLocalVar2);
            UpdateCompareFlag(LatestCmpFlagResult);
            return OR_CONTINUE;
        }
        case COMMAND_IS_INT_VAR_GREATER_THAN_INT_LVAR:
        {
            pGlobalVar = GetPointerToScriptVariable(SCOPE_GLOBAL);
            pLocalVar = GetPointerToScriptVariable(SCOPE_LOCAL);
            LatestCmpFlagResult = (*pGlobalVar > *pLocalVar);
            UpdateCompareFlag(LatestCmpFlagResult);
            return OR_CONTINUE;
        }
        case COMMAND_IS_INT_LVAR_GREATER_THAN_INT_VAR:
        {
            pLocalVar = GetPointerToScriptVariable(SCOPE_LOCAL);
            pGlobalVar = GetPointerToScriptVariable(SCOPE_GLOBAL);
            LatestCmpFlagResult = (*pLocalVar > *pGlobalVar);
            UpdateCompareFlag(LatestCmpFlagResult);
            return OR_CONTINUE;
        }
        case COMMAND_IS_FLOAT_VAR_GREATER_THAN_NUMBER:
        {
            pGlobalVar = GetPointerToScriptVariable(SCOPE_GLOBAL);
            CollectParameters(1);
            LatestCmpFlagResult = (*(f32*)pGlobalVar > *(f32*)&ScriptParams[0]);
            UpdateCompareFlag(LatestCmpFlagResult);
            return OR_CONTINUE;
        }
        case COMMAND_IS_FLOAT_LVAR_GREATER_THAN_NUMBER:
        {
            pLocalVar = GetPointerToScriptVariable(SCOPE_LOCAL);
            CollectParameters(1);
            LatestCmpFlagResult = (*(f32*)pLocalVar > *(f32*)&ScriptParams[0]);
            UpdateCompareFlag(LatestCmpFlagResult);
            return OR_CONTINUE;
        }
        case COMMAND_IS_NUMBER_GREATER_THAN_FLOAT_VAR:
        {
            CollectParameters(1);
            pGlobalVar = GetPointerToScriptVariable(SCOPE_GLOBAL);
            LatestCmpFlagResult = (*(f32*)&ScriptParams[0] > *(f32*)pGlobalVar);
            UpdateCompareFlag(LatestCmpFlagResult);
            return OR_CONTINUE;
        }
        case COMMAND_IS_NUMBER_GREATER_THAN_FLOAT_LVAR:
        {
            CollectParameters(1);
            pLocalVar = GetPointerToScriptVariable(SCOPE_LOCAL);
            LatestCmpFlagResult = (*(f32*)&ScriptParams[0] > *(f32*)pLocalVar);
            UpdateCompareFlag(LatestCmpFlagResult);
            return OR_CONTINUE;
        }
        case COMMAND_IS_FLOAT_VAR_GREATER_THAN_FLOAT_VAR:
        {
            pGlobalVar = GetPointerToScriptVariable(SCOPE_GLOBAL);
            pGlobalVar2 = GetPointerToScriptVariable(SCOPE_GLOBAL);
            LatestCmpFlagResult = (*(f32*)pGlobalVar > *(f32*)pGlobalVar2);
            UpdateCompareFlag(LatestCmpFlagResult);
            return OR_CONTINUE;
        }
        case COMMAND_IS_FLOAT_LVAR_GREATER_THAN_FLOAT_LVAR:
        {
            pLocalVar = GetPointerToScriptVariable(SCOPE_LOCAL);
            pLocalVar2 = GetPointerToScriptVariable(SCOPE_LOCAL);
            LatestCmpFlagResult = (*(f32*)pLocalVar > *(f32*)pLocalVar2);
            UpdateCompareFlag(LatestCmpFlagResult);
            return OR_CONTINUE;
        }
        case COMMAND_IS_FLOAT_VAR_GREATER_THAN_FLOAT_LVAR:
        {
            pGlobalVar = GetPointerToScriptVariable(SCOPE_GLOBAL);
            pLocalVar = GetPointerToScriptVariable(SCOPE_LOCAL);
            LatestCmpFlagResult = (*(f32*)pGlobalVar > *(f32*)pLocalVar);
            UpdateCompareFlag(LatestCmpFlagResult);
            return OR_CONTINUE;
        }
        case COMMAND_IS_FLOAT_LVAR_GREATER_THAN_FLOAT_VAR:
        {
            pLocalVar = GetPointerToScriptVariable(SCOPE_LOCAL);
            pGlobalVar = GetPointerToScriptVariable(SCOPE_GLOBAL);
            LatestCmpFlagResult = (*(f32*)pLocalVar > *(f32*)pGlobalVar);
            UpdateCompareFlag(LatestCmpFlagResult);
            return OR_CONTINUE;
        }
        case COMMAND_IS_INT_VAR_GREATER_OR_EQUAL_TO_NUMBER:
        {
            pGlobalVar = GetPointerToScriptVariable(SCOPE_GLOBAL);
            CollectParameters(1);
            LatestCmpFlagResult = (*pGlobalVar >= ScriptParams[0]);
            UpdateCompareFlag(LatestCmpFlagResult);
            return OR_CONTINUE;
        }
        case COMMAND_IS_INT_LVAR_GREATER_OR_EQUAL_TO_NUMBER:
        {
            pLocalVar = GetPointerToScriptVariable(SCOPE_LOCAL);
            CollectParameters(1);
            LatestCmpFlagResult = (*pLocalVar >= ScriptParams[0]);
            UpdateCompareFlag(LatestCmpFlagResult);
            return OR_CONTINUE;
        }
        case COMMAND_IS_NUMBER_GREATER_OR_EQUAL_TO_INT_VAR:
        {
            CollectParameters(1);
            pGlobalVar = GetPointerToScriptVariable(SCOPE_GLOBAL);
            LatestCmpFlagResult = (ScriptParams[0] >= *pGlobalVar);
            UpdateCompareFlag(LatestCmpFlagResult);
            return OR_CONTINUE;
        }
        case COMMAND_IS_NUMBER_GREATER_OR_EQUAL_TO_INT_LVAR:
        {
            CollectParameters(1);
            pLocalVar = GetPointerToScriptVariable(SCOPE_LOCAL);
            LatestCmpFlagResult = (ScriptParams[0] >= *pLocalVar);
            UpdateCompareFlag(LatestCmpFlagResult);
            return OR_CONTINUE;
        }
        case COMMAND_IS_INT_VAR_GREATER_OR_EQUAL_TO_INT_VAR:
        {
            pGlobalVar = GetPointerToScriptVariable(SCOPE_GLOBAL);
            pGlobalVar2 = GetPointerToScriptVariable(SCOPE_GLOBAL);
            LatestCmpFlagResult = (*pGlobalVar >= *pGlobalVar2);
            UpdateCompareFlag(LatestCmpFlagResult);
            return OR_CONTINUE;
        }
        case COMMAND_IS_INT_LVAR_GREATER_OR_EQUAL_TO_INT_LVAR:
        {
            pLocalVar = GetPointerToScriptVariable(SCOPE_LOCAL);
            pLocalVar2 = GetPointerToScriptVariable(SCOPE_LOCAL);
            LatestCmpFlagResult = (*pLocalVar >= *pLocalVar2);
            UpdateCompareFlag(LatestCmpFlagResult);
            return OR_CONTINUE;
        }
        case COMMAND_IS_INT_VAR_GREATER_OR_EQUAL_TO_INT_LVAR:
        {
            pGlobalVar = GetPointerToScriptVariable(SCOPE_GLOBAL);
            pLocalVar = GetPointerToScriptVariable(SCOPE_LOCAL);
            LatestCmpFlagResult = (*pGlobalVar >= *pLocalVar);
            UpdateCompareFlag(LatestCmpFlagResult);
            return OR_CONTINUE;
        }
        case COMMAND_IS_INT_LVAR_GREATER_OR_EQUAL_TO_INT_VAR:
        {
            pLocalVar = GetPointerToScriptVariable(SCOPE_LOCAL);
            pGlobalVar = GetPointerToScriptVariable(SCOPE_GLOBAL);
            LatestCmpFlagResult = (*pLocalVar >= *pGlobalVar);
            UpdateCompareFlag(LatestCmpFlagResult);
            return OR_CONTINUE;
        }
        case COMMAND_IS_FLOAT_VAR_GREATER_OR_EQUAL_TO_NUMBER:
        {
            pGlobalVar = GetPointerToScriptVariable(SCOPE_GLOBAL);
            CollectParameters(1);
            LatestCmpFlagResult = (*(f32*)pGlobalVar >= *(f32*)&ScriptParams[0]);
            UpdateCompareFlag(LatestCmpFlagResult);
            return OR_CONTINUE;
        }
        case COMMAND_IS_FLOAT_LVAR_GREATER_OR_EQUAL_TO_NUMBER:
        {
            pLocalVar = GetPointerToScriptVariable(SCOPE_LOCAL);
            CollectParameters(1);
            LatestCmpFlagResult = (*(f32*)pLocalVar >= *(f32*)&ScriptParams[0]);
            UpdateCompareFlag(LatestCmpFlagResult);
            return OR_CONTINUE;
        }
        case COMMAND_IS_NUMBER_GREATER_OR_EQUAL_TO_FLOAT_VAR:
        {
            CollectParameters(1);
            pGlobalVar = GetPointerToScriptVariable(SCOPE_GLOBAL);
            LatestCmpFlagResult = (*(f32*)&ScriptParams[0] >= *(f32*)pGlobalVar);
            UpdateCompareFlag(LatestCmpFlagResult);
            return OR_CONTINUE;
        }
        case COMMAND_IS_NUMBER_GREATER_OR_EQUAL_TO_FLOAT_LVAR:
        {
            CollectParameters(1);
            pLocalVar = GetPointerToScriptVariable(SCOPE_LOCAL);
            LatestCmpFlagResult = (*(f32*)&ScriptParams[0] >= *(f32*)pLocalVar);
            UpdateCompareFlag(LatestCmpFlagResult);
            return OR_CONTINUE;
        }
        case COMMAND_IS_FLOAT_VAR_GREATER_OR_EQUAL_TO_FLOAT_VAR:
        {
            pGlobalVar = GetPointerToScriptVariable(SCOPE_GLOBAL);
            pGlobalVar2 = GetPointerToScriptVariable(SCOPE_GLOBAL);
            LatestCmpFlagResult = (*(f32*)pGlobalVar >= *(f32*)pGlobalVar2);
            UpdateCompareFlag(LatestCmpFlagResult);
            return OR_CONTINUE;
        }
        case COMMAND_IS_FLOAT_LVAR_GREATER_OR_EQUAL_TO_FLOAT_LVAR:
        {
            pLocalVar = GetPointerToScriptVariable(SCOPE_LOCAL);
            pLocalVar2 = GetPointerToScriptVariable(SCOPE_LOCAL);
            LatestCmpFlagResult = (*(f32*)pLocalVar >= *(f32*)pLocalVar2);
            UpdateCompareFlag(LatestCmpFlagResult);
            return OR_CONTINUE;
        }
        case COMMAND_IS_FLOAT_VAR_GREATER_OR_EQUAL_TO_FLOAT_LVAR:
        {
            pGlobalVar = GetPointerToScriptVariable(SCOPE_GLOBAL);
            pLocalVar = GetPointerToScriptVariable(SCOPE_LOCAL);
            LatestCmpFlagResult = (*(f32*)pGlobalVar >= *(f32*)pLocalVar);
            UpdateCompareFlag(LatestCmpFlagResult);
            return OR_CONTINUE;
        }
        case COMMAND_IS_FLOAT_LVAR_GREATER_OR_EQUAL_TO_FLOAT_VAR:
        {
            pLocalVar = GetPointerToScriptVariable(SCOPE_LOCAL);
            pGlobalVar = GetPointerToScriptVariable(SCOPE_GLOBAL);
            LatestCmpFlagResult = (*(f32*)pLocalVar >= *(f32*)pGlobalVar);
            UpdateCompareFlag(LatestCmpFlagResult);
            return OR_CONTINUE;
        }
        case COMMAND_IS_INT_VAR_EQUAL_TO_NUMBER:
        {
            pGlobalVar = GetPointerToScriptVariable(SCOPE_GLOBAL);
            CollectParameters(1);
            LatestCmpFlagResult = (*pGlobalVar == ScriptParams[0]);
            UpdateCompareFlag(LatestCmpFlagResult);
            return OR_CONTINUE;
        }
        case COMMAND_IS_INT_LVAR_EQUAL_TO_NUMBER:
        {
            pLocalVar = GetPointerToScriptVariable(SCOPE_LOCAL);
            CollectParameters(1);
            LatestCmpFlagResult = (*pLocalVar == ScriptParams[0]);
            UpdateCompareFlag(LatestCmpFlagResult);
            return OR_CONTINUE;
        }
        case COMMAND_IS_INT_VAR_EQUAL_TO_INT_VAR:
        {
            pGlobalVar = GetPointerToScriptVariable(SCOPE_GLOBAL);
            pGlobalVar2 = GetPointerToScriptVariable(SCOPE_GLOBAL);
            LatestCmpFlagResult = (*pGlobalVar == *pGlobalVar2);
            UpdateCompareFlag(LatestCmpFlagResult);
            return OR_CONTINUE;
        }
        case COMMAND_IS_INT_LVAR_EQUAL_TO_INT_LVAR:
        {
            pLocalVar = GetPointerToScriptVariable(SCOPE_LOCAL);
            pLocalVar2 = GetPointerToScriptVariable(SCOPE_LOCAL);
            LatestCmpFlagResult = (*pLocalVar == *pLocalVar2);
            UpdateCompareFlag(LatestCmpFlagResult);
            return OR_CONTINUE;
        }
        case COMMAND_IS_INT_VAR_EQUAL_TO_INT_LVAR:
        {
            pGlobalVar = GetPointerToScriptVariable(SCOPE_GLOBAL);
            pLocalVar = GetPointerToScriptVariable(SCOPE_LOCAL);
            LatestCmpFlagResult = (*pGlobalVar == *pLocalVar);
            UpdateCompareFlag(LatestCmpFlagResult);
            return OR_CONTINUE;
        }
        case COMMAND_IS_FLOAT_VAR_EQUAL_TO_NUMBER:
        {
            pGlobalVar = GetPointerToScriptVariable(SCOPE_GLOBAL);
            CollectParameters(1);
            LatestCmpFlagResult = (*(f32*)pGlobalVar == *(f32*)&ScriptParams[0]);
            UpdateCompareFlag(LatestCmpFlagResult);
            return OR_CONTINUE;
        }
        case COMMAND_IS_FLOAT_LVAR_EQUAL_TO_NUMBER:
        {
            pLocalVar = GetPointerToScriptVariable(SCOPE_LOCAL);
            CollectParameters(1);
            LatestCmpFlagResult = (*(f32*)pLocalVar == *(f32*)&ScriptParams[0]);
            UpdateCompareFlag(LatestCmpFlagResult);
            return OR_CONTINUE;
        }
        case COMMAND_IS_FLOAT_VAR_EQUAL_TO_FLOAT_VAR:
        {
            pGlobalVar = GetPointerToScriptVariable(SCOPE_GLOBAL);
            pGlobalVar2 = GetPointerToScriptVariable(SCOPE_GLOBAL);
            LatestCmpFlagResult = (*(f32*)pGlobalVar == *(f32*)pGlobalVar2);
            UpdateCompareFlag(LatestCmpFlagResult);
            return OR_CONTINUE;
        }
        case COMMAND_IS_FLOAT_LVAR_EQUAL_TO_FLOAT_LVAR:
        {
            pLocalVar = GetPointerToScriptVariable(SCOPE_LOCAL);
            pLocalVar2 = GetPointerToScriptVariable(SCOPE_LOCAL);
            LatestCmpFlagResult = (*(f32*)pLocalVar == *(f32*)pLocalVar2);
            UpdateCompareFlag(LatestCmpFlagResult);
            return OR_CONTINUE;
        }
        case COMMAND_IS_FLOAT_VAR_EQUAL_TO_FLOAT_LVAR:
        {
            pGlobalVar = GetPointerToScriptVariable(SCOPE_GLOBAL);
            pLocalVar = GetPointerToScriptVariable(SCOPE_LOCAL);
            LatestCmpFlagResult = (*(f32*)pGlobalVar == *(f32*)pLocalVar);
            UpdateCompareFlag(LatestCmpFlagResult);
            return OR_CONTINUE;
        }
        case COMMAND_GOTO_IF_FALSE:
        {
            CollectParameters(1);
            if (!CmpFlag)
            {
                UpdatePC(ScriptParams[0]);
            }
            return OR_CONTINUE;
        }
        case COMMAND_TERMINATE_THIS_SCRIPT:
        {
            if (ThisMustBeTheOnlyMissionRunning)
            {
                CTheScripts::bAlreadyRunningAMissionScript = false;
            }
            RemoveScriptFromList(&CTheScripts::pActiveScripts);
            AddScriptToList(&CTheScripts::pIdleScripts);
            ShutdownThisScript();
            return OR_WAIT;
        }
        case COMMAND_START_NEW_SCRIPT:
        {
            CRunningScript* pNewOne;
            i32 NewScriptStartIndex;

            CollectParameters(1);
            if (ScriptParams[0] < 0)
            {
                // ASSERTOBJ(0, ScriptName, "START_NEW_SCRIPT - can only start a script which will always be in memory");
            }
            else
            {
                NewScriptStartIndex = ScriptParams[0];
            }
            pNewOne = CTheScripts::StartNewScript(&CTheScripts::ScriptSpace[NewScriptStartIndex]);
            ReadParametersForNewlyStartedScript(pNewOne);
            return OR_CONTINUE;
        }
        case COMMAND_GOSUB:
        {
            CollectParameters(1);
            PCStack[StackDepth++] = PCPointer;
            UpdatePC(ScriptParams[0]);
            return OR_CONTINUE;
        }
        case COMMAND_RETURN:
        {
            PCPointer = PCStack[--StackDepth];
            return OR_CONTINUE;
        }
        case COMMAND_LINE:
        {
            CollectParameters(6);
            return OR_CONTINUE;
        }
        case COMMAND_CREATE_PLAYER:
        {
            CollectParameters(4);
            PlayerIndex = ScriptParams[0];
            if (!CStreaming::HasModelLoaded(MODELID_PLAYER_PED))
            {
                CStreaming::RequestSpecialModel(MODELID_PLAYER_PED, "player", STRFLAG_DONTDELETE | STRFLAG_FORCE_LOAD);
                CStreaming::LoadAllRequestedModels(true);
            }
            CPlayerPed::SetupPlayerPed(PlayerIndex);
            CWorld::Players[PlayerIndex].m_pPed->SetCharCreatedBy(MISSION_CHAR);
            CPlayerPed::DeactivatePlayerPed(PlayerIndex);

            NewX = *(f32*)&ScriptParams[1];
            NewY = *(f32*)&ScriptParams[2];
            NewZ = *(f32*)&ScriptParams[3];
            if (NewZ <= -100.0f)
            {
                NewZ = CWorld::FindGroundZForCoord(NewX, NewY);
            }
            NewZ += CWorld::Players[PlayerIndex].m_pPed->GetDistanceFromCentreOfMassToBaseOfModel();
            CWorld::Players[PlayerIndex].m_pPed->SetPosition(NewX, NewY, NewZ);
            CTheScripts::ClearSpaceForMissionEntity(CVector(NewX, NewY, NewZ), CWorld::Players[PlayerIndex].m_pPed);
            CPlayerPed::ReactivatePlayerPed(PlayerIndex);
            ScriptParams[0] = PlayerIndex;
            StoreParameters(1);
            CWorld::Players[PlayerIndex].m_pPed->GetPedIntelligence()->AddTaskDefault(new CTaskSimplePlayerOnFoot());
            return OR_CONTINUE;
        }
        case COMMAND_ADD_INT_VAR_TO_INT_VAR:
        {
            pGlobalVar = GetPointerToScriptVariable(SCOPE_GLOBAL);
            pGlobalVar2 = GetPointerToScriptVariable(SCOPE_GLOBAL);
            *pGlobalVar += *pGlobalVar2;
            return OR_CONTINUE;
        }
        case COMMAND_ADD_FLOAT_VAR_TO_FLOAT_VAR:
        {
            pGlobalVar = GetPointerToScriptVariable(SCOPE_GLOBAL);
            pGlobalVar2 = GetPointerToScriptVariable(SCOPE_GLOBAL);
            *(f32*)pGlobalVar += *(f32*)pGlobalVar2;
            return OR_CONTINUE;
        }
        case COMMAND_ADD_INT_LVAR_TO_INT_LVAR:
        {
            pLocalVar = GetPointerToScriptVariable(SCOPE_LOCAL);
            pLocalVar2 = GetPointerToScriptVariable(SCOPE_LOCAL);
            *pLocalVar += *pLocalVar2;
            return OR_CONTINUE;
        }
        case COMMAND_ADD_FLOAT_LVAR_TO_FLOAT_LVAR:
        {
            pLocalVar = GetPointerToScriptVariable(SCOPE_LOCAL);
            pLocalVar2 = GetPointerToScriptVariable(SCOPE_LOCAL);
            *(f32*)pLocalVar += *(f32*)pLocalVar2;
            return OR_CONTINUE;
        }
        case COMMAND_ADD_INT_VAR_TO_INT_LVAR:
        {
            pLocalVar = GetPointerToScriptVariable(SCOPE_LOCAL);
            pGlobalVar = GetPointerToScriptVariable(SCOPE_GLOBAL);
            *pLocalVar += *pGlobalVar;
            return OR_CONTINUE;
        }
        case COMMAND_ADD_FLOAT_VAR_TO_FLOAT_LVAR:
        {
            pLocalVar = GetPointerToScriptVariable(SCOPE_LOCAL);
            pGlobalVar = GetPointerToScriptVariable(SCOPE_GLOBAL);
            *(f32*)pLocalVar += *(f32*)pGlobalVar;
            return OR_CONTINUE;
        }
        case COMMAND_ADD_INT_LVAR_TO_INT_VAR:
        {
            pGlobalVar = GetPointerToScriptVariable(SCOPE_GLOBAL);
            pLocalVar = GetPointerToScriptVariable(SCOPE_LOCAL);
            *pGlobalVar += *pLocalVar;
            return OR_CONTINUE;
        }
        case COMMAND_ADD_FLOAT_LVAR_TO_FLOAT_VAR:
        {
            pGlobalVar = GetPointerToScriptVariable(SCOPE_GLOBAL);
            pLocalVar = GetPointerToScriptVariable(SCOPE_LOCAL);
            *(f32*)pGlobalVar += *(f32*)pLocalVar;
            return OR_CONTINUE;
        }
        case COMMAND_SUB_INT_VAR_FROM_INT_VAR:
        {
            pGlobalVar = GetPointerToScriptVariable(SCOPE_GLOBAL);
            pGlobalVar2 = GetPointerToScriptVariable(SCOPE_GLOBAL);
            *pGlobalVar -= *pGlobalVar2;
            return OR_CONTINUE;
        }
        case COMMAND_SUB_FLOAT_VAR_FROM_FLOAT_VAR:
        {
            pGlobalVar = GetPointerToScriptVariable(SCOPE_GLOBAL);
            pGlobalVar2 = GetPointerToScriptVariable(SCOPE_GLOBAL);
            *(f32*)pGlobalVar -= *(f32*)pGlobalVar2;
            return OR_CONTINUE;
        }
        case COMMAND_SUB_INT_LVAR_FROM_INT_LVAR:
        {
            pLocalVar = GetPointerToScriptVariable(SCOPE_LOCAL);
            pLocalVar2 = GetPointerToScriptVariable(SCOPE_LOCAL);
            *pLocalVar -= *pLocalVar2;
            return OR_CONTINUE;
        }
        case COMMAND_SUB_FLOAT_LVAR_FROM_FLOAT_LVAR:
        {
            pLocalVar = GetPointerToScriptVariable(SCOPE_LOCAL);
            pLocalVar2 = GetPointerToScriptVariable(SCOPE_LOCAL);
            *(f32*)pLocalVar -= *(f32*)pLocalVar2;
            return OR_CONTINUE;
        }
    }
    return OR_INTERRUPT;
}

// MARK: 100To199

// stub: sa 0x466DE0
OpcodeResult CRunningScript::ProcessCommands100To199(i32 CurrCommand)
{
    MARKFUNCTION(0x466DE0);

    bool LatestCmpFlagResult;
    CPed* pPed;
    CVehicle* pVehicle;
    CVector TempCoors;
    const GxtChar* pString;
    char TextLabel[10];
    CVehicle* pNewVehicle;
    f32 NewX, NewY, NewZ;
    f32 CentreZ;
    i32 *pGlobalVar, *pGlobalVar2;
    i32 *pLocalVar, *pLocalVar2;

    switch (CurrCommand)
    {
        case COMMAND_SUB_INT_VAR_FROM_INT_LVAR:
        {
            pLocalVar = GetPointerToScriptVariable(SCOPE_LOCAL);
            pGlobalVar = GetPointerToScriptVariable(SCOPE_GLOBAL);
            *pLocalVar -= *pGlobalVar;
            return OR_CONTINUE;
        }
        case COMMAND_SUB_FLOAT_VAR_FROM_FLOAT_LVAR:
        {
            pLocalVar = GetPointerToScriptVariable(SCOPE_LOCAL);
            pGlobalVar = GetPointerToScriptVariable(SCOPE_GLOBAL);
            *(f32*)pLocalVar -= *(f32*)pGlobalVar;
            return OR_CONTINUE;
        }
        case COMMAND_SUB_INT_LVAR_FROM_INT_VAR:
        {
            pGlobalVar = GetPointerToScriptVariable(SCOPE_GLOBAL);
            pLocalVar = GetPointerToScriptVariable(SCOPE_LOCAL);
            *pGlobalVar -= *pLocalVar;
            return OR_CONTINUE;
        }
        case COMMAND_SUB_FLOAT_LVAR_FROM_FLOAT_VAR:
        {
            pGlobalVar = GetPointerToScriptVariable(SCOPE_GLOBAL);
            pLocalVar = GetPointerToScriptVariable(SCOPE_LOCAL);
            *(f32*)pGlobalVar -= *(f32*)pLocalVar;
            return OR_CONTINUE;
        }
        case COMMAND_MULT_INT_VAR_BY_INT_VAR:
        {
            pGlobalVar = GetPointerToScriptVariable(SCOPE_GLOBAL);
            pGlobalVar2 = GetPointerToScriptVariable(SCOPE_GLOBAL);
            *pGlobalVar *= *pGlobalVar2;
            return OR_CONTINUE;
        }
        case COMMAND_MULT_FLOAT_VAR_BY_FLOAT_VAR:
        {
            pGlobalVar = GetPointerToScriptVariable(SCOPE_GLOBAL);
            pGlobalVar2 = GetPointerToScriptVariable(SCOPE_GLOBAL);
            *(f32*)pGlobalVar *= *(f32*)pGlobalVar2;
            return OR_CONTINUE;
        }
        case COMMAND_MULT_INT_LVAR_BY_INT_LVAR:
        {
            pLocalVar = GetPointerToScriptVariable(SCOPE_LOCAL);
            pLocalVar2 = GetPointerToScriptVariable(SCOPE_LOCAL);
            *pLocalVar *= *pLocalVar2;
            return OR_CONTINUE;
        }
        case COMMAND_MULT_FLOAT_LVAR_BY_FLOAT_LVAR:
        {
            pLocalVar = GetPointerToScriptVariable(SCOPE_LOCAL);
            pLocalVar2 = GetPointerToScriptVariable(SCOPE_LOCAL);
            *(f32*)pLocalVar *= *(f32*)pLocalVar2;
            return OR_CONTINUE;
        }
        case COMMAND_MULT_INT_VAR_BY_INT_LVAR:
        {
            pGlobalVar = GetPointerToScriptVariable(SCOPE_GLOBAL);
            pLocalVar = GetPointerToScriptVariable(SCOPE_LOCAL);
            *pGlobalVar *= *pLocalVar;
            return OR_CONTINUE;
        }
        case COMMAND_MULT_FLOAT_VAR_BY_FLOAT_LVAR:
        {
            pGlobalVar = GetPointerToScriptVariable(SCOPE_GLOBAL);
            pLocalVar = GetPointerToScriptVariable(SCOPE_LOCAL);
            *(f32*)pGlobalVar *= *(f32*)pLocalVar;
            return OR_CONTINUE;
        }
        case COMMAND_MULT_INT_LVAR_BY_INT_VAR:
        {
            pLocalVar = GetPointerToScriptVariable(SCOPE_LOCAL);
            pGlobalVar = GetPointerToScriptVariable(SCOPE_GLOBAL);
            *pLocalVar *= *pGlobalVar;
            return OR_CONTINUE;
        }
        case COMMAND_MULT_FLOAT_LVAR_BY_FLOAT_VAR:
        {
            pLocalVar = GetPointerToScriptVariable(SCOPE_LOCAL);
            pGlobalVar = GetPointerToScriptVariable(SCOPE_GLOBAL);
            *(f32*)pLocalVar *= *(f32*)pGlobalVar;
            return OR_CONTINUE;
        }
        case COMMAND_DIV_INT_VAR_BY_INT_VAR:
        {
            pGlobalVar = GetPointerToScriptVariable(SCOPE_GLOBAL);
            pGlobalVar2 = GetPointerToScriptVariable(SCOPE_GLOBAL);
            *pGlobalVar /= *pGlobalVar2;
            return OR_CONTINUE;
        }
        case COMMAND_DIV_FLOAT_VAR_BY_FLOAT_VAR:
        {
            pGlobalVar = GetPointerToScriptVariable(SCOPE_GLOBAL);
            pGlobalVar2 = GetPointerToScriptVariable(SCOPE_GLOBAL);
            *(f32*)pGlobalVar /= *(f32*)pGlobalVar2;
            return OR_CONTINUE;
        }
        case COMMAND_DIV_INT_LVAR_BY_INT_LVAR:
        {
            pLocalVar = GetPointerToScriptVariable(SCOPE_LOCAL);
            pLocalVar2 = GetPointerToScriptVariable(SCOPE_LOCAL);
            *pLocalVar /= *pLocalVar2;
            return OR_CONTINUE;
        }
        case COMMAND_DIV_FLOAT_LVAR_BY_FLOAT_LVAR:
        {
            pLocalVar = GetPointerToScriptVariable(SCOPE_LOCAL);
            pLocalVar2 = GetPointerToScriptVariable(SCOPE_LOCAL);
            *(f32*)pLocalVar /= *(f32*)pLocalVar2;
            return OR_CONTINUE;
        }
        case COMMAND_DIV_INT_VAR_BY_INT_LVAR:
        {
            pGlobalVar = GetPointerToScriptVariable(SCOPE_GLOBAL);
            pLocalVar = GetPointerToScriptVariable(SCOPE_LOCAL);
            *pGlobalVar /= *pLocalVar;
            return OR_CONTINUE;
        }
        case COMMAND_DIV_FLOAT_VAR_BY_FLOAT_LVAR:
        {
            pGlobalVar = GetPointerToScriptVariable(SCOPE_GLOBAL);
            pLocalVar = GetPointerToScriptVariable(SCOPE_LOCAL);
            *(f32*)pGlobalVar /= *(f32*)pLocalVar;
            return OR_CONTINUE;
        }
        case COMMAND_DIV_INT_LVAR_BY_INT_VAR:
        {
            pLocalVar = GetPointerToScriptVariable(SCOPE_LOCAL);
            pGlobalVar = GetPointerToScriptVariable(SCOPE_GLOBAL);
            *pLocalVar /= *pGlobalVar;
            return OR_CONTINUE;
        }
        case COMMAND_DIV_FLOAT_LVAR_BY_FLOAT_VAR:
        {
            pLocalVar = GetPointerToScriptVariable(SCOPE_LOCAL);
            pGlobalVar = GetPointerToScriptVariable(SCOPE_GLOBAL);
            *(f32*)pLocalVar /= *(f32*)pGlobalVar;
            return OR_CONTINUE;
        }
        case COMMAND_ADD_TIMED_VAL_TO_FLOAT_VAR:
        {
            pGlobalVar = GetPointerToScriptVariable(SCOPE_GLOBAL);
            CollectParameters(1);
            *(f32*)pGlobalVar += *(f32*)&ScriptParams[0] * CTimer::GetTimeStep();
            return OR_CONTINUE;
        }
        case COMMAND_ADD_TIMED_VAL_TO_FLOAT_LVAR:
        {
            pLocalVar = GetPointerToScriptVariable(SCOPE_LOCAL);
            CollectParameters(1);
            *(f32*)pLocalVar += *(f32*)&ScriptParams[0] * CTimer::GetTimeStep();
            return OR_CONTINUE;
        }
        case COMMAND_ADD_TIMED_FLOAT_VAR_TO_FLOAT_VAR:
        {
            pGlobalVar = GetPointerToScriptVariable(SCOPE_GLOBAL);
            pGlobalVar2 = GetPointerToScriptVariable(SCOPE_GLOBAL);
            *(f32*)pGlobalVar += *(f32*)pGlobalVar2 * CTimer::GetTimeStep();
            return OR_CONTINUE;
        }
        case COMMAND_ADD_TIMED_FLOAT_LVAR_TO_FLOAT_LVAR:
        {
            pLocalVar = GetPointerToScriptVariable(SCOPE_LOCAL);
            pLocalVar2 = GetPointerToScriptVariable(SCOPE_LOCAL);
            *(f32*)pLocalVar += *(f32*)pLocalVar2 * CTimer::GetTimeStep();
            return OR_CONTINUE;
        }
        case COMMAND_ADD_TIMED_FLOAT_VAR_TO_FLOAT_LVAR:
        {
            pLocalVar = GetPointerToScriptVariable(SCOPE_LOCAL);
            pGlobalVar = GetPointerToScriptVariable(SCOPE_GLOBAL);
            *(f32*)pLocalVar += *(f32*)pGlobalVar * CTimer::GetTimeStep();
            return OR_CONTINUE;
        }
        case COMMAND_ADD_TIMED_FLOAT_LVAR_TO_FLOAT_VAR:
        {
            pGlobalVar = GetPointerToScriptVariable(SCOPE_GLOBAL);
            pLocalVar = GetPointerToScriptVariable(SCOPE_LOCAL);
            *(f32*)pGlobalVar += *(f32*)pLocalVar * CTimer::GetTimeStep();
            return OR_CONTINUE;
        }
        case COMMAND_SUB_TIMED_VAL_FROM_FLOAT_VAR:
        {
            pGlobalVar = GetPointerToScriptVariable(SCOPE_GLOBAL);
            CollectParameters(1);
            *(f32*)pGlobalVar -= *(f32*)&ScriptParams[0] * CTimer::GetTimeStep();
            return OR_CONTINUE;
        }
        case COMMAND_SUB_TIMED_VAL_FROM_FLOAT_LVAR:
        {
            pLocalVar = GetPointerToScriptVariable(SCOPE_LOCAL);
            CollectParameters(1);
            *(f32*)pLocalVar -= *(f32*)&ScriptParams[0] * CTimer::GetTimeStep();
            return OR_CONTINUE;
        }
        case COMMAND_SUB_TIMED_FLOAT_VAR_FROM_FLOAT_VAR:
        {
            pGlobalVar = GetPointerToScriptVariable(SCOPE_GLOBAL);
            pGlobalVar2 = GetPointerToScriptVariable(SCOPE_GLOBAL);
            *(f32*)pGlobalVar -= *(f32*)pGlobalVar2 * CTimer::GetTimeStep();
            return OR_CONTINUE;
        }
        case COMMAND_SUB_TIMED_FLOAT_LVAR_FROM_FLOAT_LVAR:
        {
            pLocalVar = GetPointerToScriptVariable(SCOPE_LOCAL);
            pLocalVar2 = GetPointerToScriptVariable(SCOPE_LOCAL);
            *(f32*)pLocalVar -= *(f32*)pLocalVar2 * CTimer::GetTimeStep();
            return OR_CONTINUE;
        }
        case COMMAND_SUB_TIMED_FLOAT_VAR_FROM_FLOAT_LVAR:
        {
            pLocalVar = GetPointerToScriptVariable(SCOPE_LOCAL);
            pGlobalVar = GetPointerToScriptVariable(SCOPE_GLOBAL);
            *(f32*)pLocalVar -= *(f32*)pGlobalVar * CTimer::GetTimeStep();
            return OR_CONTINUE;
        }
        case COMMAND_SUB_TIMED_FLOAT_LVAR_FROM_FLOAT_VAR:
        {
            pGlobalVar = GetPointerToScriptVariable(SCOPE_GLOBAL);
            pLocalVar = GetPointerToScriptVariable(SCOPE_LOCAL);
            *(f32*)pGlobalVar -= *(f32*)pLocalVar * CTimer::GetTimeStep();
            return OR_CONTINUE;
        }
        case COMMAND_SET_VAR_INT_TO_VAR_INT:
        {
            pGlobalVar = GetPointerToScriptVariable(SCOPE_GLOBAL);
            pGlobalVar2 = GetPointerToScriptVariable(SCOPE_GLOBAL);
            *pGlobalVar = *pGlobalVar2;
            return OR_CONTINUE;
        }
        case COMMAND_SET_LVAR_INT_TO_LVAR_INT:
        {
            pLocalVar = GetPointerToScriptVariable(SCOPE_LOCAL);
            pLocalVar2 = GetPointerToScriptVariable(SCOPE_LOCAL);
            *pLocalVar = *pLocalVar2;
            return OR_CONTINUE;
        }
        case COMMAND_SET_VAR_FLOAT_TO_VAR_FLOAT:
        {
            pGlobalVar = GetPointerToScriptVariable(SCOPE_GLOBAL);
            pGlobalVar2 = GetPointerToScriptVariable(SCOPE_GLOBAL);
            *pGlobalVar = *pGlobalVar2;
            return OR_CONTINUE;
        }
        case COMMAND_SET_LVAR_FLOAT_TO_LVAR_FLOAT:
        {
            pLocalVar = GetPointerToScriptVariable(SCOPE_LOCAL);
            pLocalVar2 = GetPointerToScriptVariable(SCOPE_LOCAL);
            *pLocalVar = *pLocalVar2;
            return OR_CONTINUE;
        }
        case COMMAND_SET_VAR_FLOAT_TO_LVAR_FLOAT:
        {
            pGlobalVar = GetPointerToScriptVariable(SCOPE_GLOBAL);
            pLocalVar = GetPointerToScriptVariable(SCOPE_LOCAL);
            *pGlobalVar = *pLocalVar;
            return OR_CONTINUE;
        }
        case COMMAND_SET_LVAR_FLOAT_TO_VAR_FLOAT:
        {
            pLocalVar = GetPointerToScriptVariable(SCOPE_LOCAL);
            pGlobalVar = GetPointerToScriptVariable(SCOPE_GLOBAL);
            *pLocalVar = *pGlobalVar;
            return OR_CONTINUE;
        }
        case COMMAND_SET_VAR_INT_TO_LVAR_INT:
        {
            pGlobalVar = GetPointerToScriptVariable(SCOPE_GLOBAL);
            pLocalVar = GetPointerToScriptVariable(SCOPE_LOCAL);
            *pGlobalVar = *pLocalVar;
            return OR_CONTINUE;
        }
        case COMMAND_SET_LVAR_INT_TO_VAR_INT:
        {
            pLocalVar = GetPointerToScriptVariable(SCOPE_LOCAL);
            pGlobalVar = GetPointerToScriptVariable(SCOPE_GLOBAL);
            *pLocalVar = *pGlobalVar;
            return OR_CONTINUE;
        }
        case COMMAND_CSET_VAR_INT_TO_VAR_FLOAT:
        {
            pGlobalVar = GetPointerToScriptVariable(SCOPE_GLOBAL);
            pGlobalVar2 = GetPointerToScriptVariable(SCOPE_GLOBAL);
            *pGlobalVar = (i32)(*(f32*)pGlobalVar2);
            return OR_CONTINUE;
        }
        case COMMAND_CSET_VAR_FLOAT_TO_VAR_INT:
        {
            pGlobalVar = GetPointerToScriptVariable(SCOPE_GLOBAL);
            pGlobalVar2 = GetPointerToScriptVariable(SCOPE_GLOBAL);
            *(f32*)pGlobalVar = (f32)(*pGlobalVar2);
            return OR_CONTINUE;
        }
        case COMMAND_CSET_LVAR_INT_TO_VAR_FLOAT:
        {
            pLocalVar = GetPointerToScriptVariable(SCOPE_LOCAL);
            pGlobalVar = GetPointerToScriptVariable(SCOPE_GLOBAL);
            *pLocalVar = (i32)(*(f32*)pGlobalVar);
            return OR_CONTINUE;
        }
        case COMMAND_CSET_LVAR_FLOAT_TO_VAR_INT:
        {
            pLocalVar = GetPointerToScriptVariable(SCOPE_LOCAL);
            pGlobalVar = GetPointerToScriptVariable(SCOPE_GLOBAL);
            *(f32*)pLocalVar = (f32)(*pGlobalVar);
            return OR_CONTINUE;
        }
        case COMMAND_CSET_VAR_INT_TO_LVAR_FLOAT:
        {
            pGlobalVar = GetPointerToScriptVariable(SCOPE_GLOBAL);
            pLocalVar = GetPointerToScriptVariable(SCOPE_LOCAL);
            *pGlobalVar = (i32) * (f32*)pLocalVar;
            return OR_CONTINUE;
        }
        case COMMAND_CSET_VAR_FLOAT_TO_LVAR_INT:
        {
            pGlobalVar = GetPointerToScriptVariable(SCOPE_GLOBAL);
            pLocalVar = GetPointerToScriptVariable(SCOPE_LOCAL);
            *(f32*)pGlobalVar = (f32)*pLocalVar;
            return OR_CONTINUE;
        }
        case COMMAND_CSET_LVAR_INT_TO_LVAR_FLOAT:
        {
            pLocalVar = GetPointerToScriptVariable(SCOPE_LOCAL);
            pLocalVar2 = GetPointerToScriptVariable(SCOPE_LOCAL);
            *pLocalVar = (i32) * (f32*)pLocalVar2;
            return OR_CONTINUE;
        }
        case COMMAND_CSET_LVAR_FLOAT_TO_LVAR_INT:
        {
            pLocalVar = GetPointerToScriptVariable(SCOPE_LOCAL);
            pLocalVar2 = GetPointerToScriptVariable(SCOPE_LOCAL);
            *(f32*)pLocalVar = (f32)*pLocalVar2;
            return OR_CONTINUE;
        }
        case COMMAND_ABS_VAR_INT:
        {
            pGlobalVar = GetPointerToScriptVariable(SCOPE_GLOBAL);
            *pGlobalVar = ABS(*pGlobalVar);
            return OR_CONTINUE;
        }
        case COMMAND_ABS_LVAR_INT:
        {
            pLocalVar = GetPointerToScriptVariable(SCOPE_LOCAL);
            *pLocalVar = ABS(*pLocalVar);
            return OR_CONTINUE;
        }
        case COMMAND_ABS_VAR_FLOAT:
        {
            pGlobalVar = GetPointerToScriptVariable(SCOPE_GLOBAL);
            *(f32*)pGlobalVar = ABS(*(f32*)pGlobalVar);
            return OR_CONTINUE;
        }
        case COMMAND_ABS_LVAR_FLOAT:
        {
            pLocalVar = GetPointerToScriptVariable(SCOPE_LOCAL);
            *(f32*)pLocalVar = ABS(*(f32*)pLocalVar);
            return OR_CONTINUE;
        }
        case COMMAND_GENERATE_RANDOM_FLOAT:
        {
            pGlobalVar = GetPointerToScriptVariable(SCOPE_GLOBAL);
            CGeneral::GetRandomNumber();
            CGeneral::GetRandomNumber();
            CGeneral::GetRandomNumber();
            *(f32*)pGlobalVar = CGeneral::GetRandomNumber() / 65536.0f;
            return OR_CONTINUE;
        }
        case COMMAND_GENERATE_RANDOM_INT:
        {
            pGlobalVar = GetPointerToScriptVariable(SCOPE_GLOBAL);
            *pGlobalVar = CGeneral::GetRandomNumber();
            return OR_CONTINUE;
        }
        case COMMAND_CREATE_CHAR:
        {
            CollectParameters(5);
            GetCorrectPedModelIndexForEmergencyServiceType(ScriptParams[0], &ScriptParams[1]);
            if (ScriptParams[0] == PEDTYPE_COP)
            {
                pPed = new CCopPed((eCopType)ScriptParams[1]);
            }
            else if ((ScriptParams[0] == PEDTYPE_MEDIC) || (ScriptParams[0] == PEDTYPE_FIRE))
            {
                pPed = new CEmergencyPed(ScriptParams[0], ScriptParams[1]);
            }
            else
            {
                pPed = new CCivilianPed((ePedType)ScriptParams[0], ScriptParams[1]);
            }
            pPed->GetPedIntelligence()->AddTaskDefault(new CTaskSimpleStandStill(999999, true));
            pPed->SetCharCreatedBy(MISSION_CHAR);
            pPed->bAllowMedicsToReviveMe = false;
            NewX = *(f32*)&ScriptParams[2];
            NewY = *(f32*)&ScriptParams[3];
            NewZ = *(f32*)&ScriptParams[4];
            if (NewZ <= -100.0f)
            {
                NewZ = CWorld::FindGroundZForCoord(NewX, NewY);
            }
            NewZ += 1.0f;
            TempCoors = CVector(NewX, NewY, NewZ);
            pPed->SetPosition(TempCoors);
            pPed->SetOrientation(DEGTORAD(0.0f), DEGTORAD(0.0f), DEGTORAD(0.0f));
            CTheScripts::ClearSpaceForMissionEntity(TempCoors, pPed);
            if (IsThisAMissionScript)
            {
                pPed->m_bIsStaticWaitingForCollision = true;
            }
            CWorld::Add(pPed);
            CPopulation::ms_nTotalMissionPeds++;
            ScriptParams[0] = CPools::GetPedPool().GetIndex(pPed);
            StoreParameters(1);
            if (IsThisAMissionScript)
            {
                CTheScripts::MissionCleanUp.AddEntityToList(ScriptParams[0], CLEANUP_CHAR);
            }
            return OR_CONTINUE;
        }
        case COMMAND_DELETE_CHAR:
        {
            CollectParameters(1);
            pPed = CPools::GetPedPool().GetAt(ScriptParams[0]);
            CTheScripts::RemoveThisPed(pPed);
            if (IsThisAMissionScript)
            {
                CTheScripts::MissionCleanUp.RemoveEntityFromList(ScriptParams[0], CLEANUP_CHAR);
            }
            return OR_CONTINUE;
        }
        case COMMAND_GET_CHAR_COORDINATES:
        {
            CollectParameters(1);
            pPed = CPools::GetPedPool().GetAt(ScriptParams[0]);
            if (pPed->bInVehicle)
            {
                pVehicle = pPed->m_pMyVehicle;
            }
            else
            {
                pVehicle = nullptr;
            }
            if (pVehicle)
            {
                TempCoors = pVehicle->GetPosition();
            }
            else
            {
                TempCoors = pPed->GetPosition();
            }
            ScriptParams[0] = *((i32*)&(TempCoors.x));
            ScriptParams[1] = *((i32*)&(TempCoors.y));
            ScriptParams[2] = *((i32*)&(TempCoors.z));
            StoreParameters(3);
            return OR_CONTINUE;
        }
        case COMMAND_SET_CHAR_COORDINATES:
        {
            CollectParameters(4);
            pPed = CPools::GetPedPool().GetAt(ScriptParams[0]);
            NewX = *(f32*)&ScriptParams[1];
            NewY = *(f32*)&ScriptParams[2];
            NewZ = *(f32*)&ScriptParams[3];
            SetCharCoordinates(pPed, NewX, NewY, NewZ, true, true);
            return OR_CONTINUE;
        }
        case COMMAND_IS_CHAR_IN_AREA_2D:
        {
            CollectParameters(6);
            pPed = CPools::GetPedPool().GetAt(ScriptParams[0]);
            if (pPed->bInVehicle)
            {
                pVehicle = pPed->m_pMyVehicle;
            }
            else
            {
                pVehicle = nullptr;
            }
            if (pVehicle)
            {
                if (pVehicle->IsWithinArea(*(f32*)&ScriptParams[1], *(f32*)&ScriptParams[2], *(f32*)&ScriptParams[3], *(f32*)&ScriptParams[4]))
                {
                    LatestCmpFlagResult = true;
                }
                else
                {
                    LatestCmpFlagResult = false;
                }
            }
            else
            {
                if (pPed->IsWithinArea(*(f32*)&ScriptParams[1], *(f32*)&ScriptParams[2], *(f32*)&ScriptParams[3], *(f32*)&ScriptParams[4]))
                {
                    LatestCmpFlagResult = true;
                }
                else
                {
                    LatestCmpFlagResult = false;
                }
            }
            UpdateCompareFlag(LatestCmpFlagResult);
            if (ScriptParams[5])
            {
                CTheScripts::HighlightImportantArea(((u32)(this) + (u32)PCPointer), *(f32*)&ScriptParams[1], *(f32*)&ScriptParams[2], *(f32*)&ScriptParams[3], *(f32*)&ScriptParams[4], -100.0f);
            }
            if (CTheScripts::DbgFlag)
            {
                CTheScripts::DrawDebugSquare(*(f32*)&ScriptParams[1], *(f32*)&ScriptParams[2], *(f32*)&ScriptParams[3], *(f32*)&ScriptParams[4]);
            }
            return OR_CONTINUE;
        }
        case COMMAND_IS_CHAR_IN_AREA_3D:
        {
            CollectParameters(8);
            pPed = CPools::GetPedPool().GetAt(ScriptParams[0]);
            if (pPed->bInVehicle)
            {
                pVehicle = pPed->m_pMyVehicle;
            }
            else
            {
                pVehicle = nullptr;
            }
            if (pVehicle)
            {
                if (pVehicle->IsWithinArea(*(f32*)&ScriptParams[1], *(f32*)&ScriptParams[2], *(f32*)&ScriptParams[3], *(f32*)&ScriptParams[4], *(f32*)&ScriptParams[5], *(f32*)&ScriptParams[6]))
                {
                    LatestCmpFlagResult = true;
                }
                else
                {
                    LatestCmpFlagResult = false;
                }
            }
            else
            {
                if (pPed->IsWithinArea(*(f32*)&ScriptParams[1], *(f32*)&ScriptParams[2], *(f32*)&ScriptParams[3], *(f32*)&ScriptParams[4], *(f32*)&ScriptParams[5], *(f32*)&ScriptParams[6]))
                {
                    LatestCmpFlagResult = true;
                }
                else
                {
                    LatestCmpFlagResult = false;
                }
            }
            UpdateCompareFlag(LatestCmpFlagResult);
            if (ScriptParams[7])
            {
                CentreZ = (*(f32*)&ScriptParams[3] + *(f32*)&ScriptParams[6]) / 2.0f;
                CTheScripts::HighlightImportantArea(((u32)(this) + (u32)PCPointer), *(f32*)&ScriptParams[1], *(f32*)&ScriptParams[2], *(f32*)&ScriptParams[4], *(f32*)&ScriptParams[5], CentreZ);
            }
            if (CTheScripts::DbgFlag)
            {
                CTheScripts::DrawDebugCube(*(f32*)&ScriptParams[1], *(f32*)&ScriptParams[2], *(f32*)&ScriptParams[3], *(f32*)&ScriptParams[4], *(f32*)&ScriptParams[5], *(f32*)&ScriptParams[6]);
            }
            return OR_CONTINUE;
        }
        case COMMAND_CREATE_CAR:
        {
            CollectParameters(4);
            NewX = *(f32*)&ScriptParams[1];
            NewY = *(f32*)&ScriptParams[2];
            NewZ = *(f32*)&ScriptParams[3];
            pNewVehicle = CCarCtrl::CreateCarForScript(ScriptParams[0], CVector(NewX, NewY, NewZ), IsThisAMissionScript);
            ScriptParams[0] = CPools::GetVehiclePool().GetIndex(pNewVehicle);
            StoreParameters(1);
            return OR_CONTINUE;
        }
        case COMMAND_DELETE_CAR:
        {
            CollectParameters(1);
            pVehicle = CPools::GetVehiclePool().GetAt(ScriptParams[0]);
            if (pVehicle)
            {
                CWorld::Remove(pVehicle);
                CWorld::RemoveReferencesToDeletedObject(pVehicle);
                delete (pVehicle);
            }
            if (IsThisAMissionScript)
            {
                CTheScripts::MissionCleanUp.RemoveEntityFromList(ScriptParams[0], CLEANUP_CAR);
            }
            return OR_CONTINUE;
        }
        case COMMAND_CAR_GOTO_COORDINATES:
        {
            CollectParameters(4);
            pVehicle = CPools::GetVehiclePool().GetAt(ScriptParams[0]);
            NewX = *((f32*)&ScriptParams[1]);
            NewY = *((f32*)&ScriptParams[2]);
            NewZ = *((f32*)&ScriptParams[3]);
            if (NewZ <= -100.0f)
            {
                NewZ = CWorld::FindGroundZForCoord(NewX, NewY);
            }
            NewZ += pVehicle->GetDistanceFromCentreOfMassToBaseOfModel();
            if (CCarCtrl::JoinCarWithRoadSystemGotoCoors(pVehicle, CVector(NewX, NewY, NewZ), false, false))
            {
                pVehicle->AutoPilot.SetMission(MISSION_GOTOCOORDINATES_STRAIGHTLINE);
            }
            else
            {
                pVehicle->AutoPilot.SetMission(MISSION_GOTOCOORDINATES);
            }
            pVehicle->SetStatus(STATUS_PHYSICS);
            pVehicle->SetEngineOn(true);
            pVehicle->AutoPilot.CruiseSpeed = MAX(pVehicle->AutoPilot.CruiseSpeed, 1);
            pVehicle->AutoPilot.LastTimeNotStuck = CTimer::GetTimeInMilliseconds();
            return OR_CONTINUE;
        }
        case COMMAND_CAR_WANDER_RANDOMLY:
        {
            CollectParameters(1);
            pVehicle = CPools::GetVehiclePool().GetAt(ScriptParams[0]);
            CCarCtrl::JoinCarWithRoadSystem(pVehicle);
            pVehicle->AutoPilot.SetMission(MISSION_CRUISE);
            pVehicle->SetEngineOn(true);
            pVehicle->AutoPilot.CruiseSpeed = MAX(pVehicle->AutoPilot.CruiseSpeed, 1);
            pVehicle->AutoPilot.LastTimeNotStuck = CTimer::GetTimeInMilliseconds();
            return OR_CONTINUE;
        }
        case COMMAND_CAR_SET_IDLE:
        {
            CollectParameters(1);
            pVehicle = CPools::GetVehiclePool().GetAt(ScriptParams[0]);
            pVehicle->AutoPilot.SetMission(MISSION_NONE);
            return OR_CONTINUE;
        }
        case COMMAND_GET_CAR_COORDINATES:
        {
            CollectParameters(1);
            pVehicle = CPools::GetVehiclePool().GetAt(ScriptParams[0]);
            TempCoors = pVehicle->GetPosition();
            ScriptParams[0] = *((i32*)&(TempCoors.x));
            ScriptParams[1] = *((i32*)&(TempCoors.y));
            ScriptParams[2] = *((i32*)&(TempCoors.z));
            StoreParameters(3);
            return OR_CONTINUE;
        }
        case COMMAND_SET_CAR_COORDINATES:
        {
            CollectParameters(4);
            pVehicle = CPools::GetVehiclePool().GetAt(ScriptParams[0]);
            NewX = *((f32*)&ScriptParams[1]);
            NewY = *((f32*)&ScriptParams[2]);
            NewZ = *((f32*)&ScriptParams[3]);
            CCarCtrl::SetCoordsOfScriptCar(pVehicle, NewX, NewY, NewZ, false, true);
            return OR_CONTINUE;
        }
        case COMMAND_SET_CAR_CRUISE_SPEED:
        {
            CollectParameters(2);
            pVehicle = CPools::GetVehiclePool().GetAt(ScriptParams[0]);
            pVehicle->AutoPilot.CruiseSpeed = *(f32*)&ScriptParams[1];
            pVehicle->AutoPilot.CruiseSpeed = MIN(pVehicle->AutoPilot.CruiseSpeed, pVehicle->pHandling->Transmission.m_fMaxFlatVelocity * 60.0f);
            return OR_CONTINUE;
        }
        case COMMAND_SET_CAR_DRIVING_STYLE:
        {
            CollectParameters(2);
            pVehicle = CPools::GetVehiclePool().GetAt(ScriptParams[0]);
            pVehicle->AutoPilot.DrivingMode = static_cast<eCarDrivingStyle>(ScriptParams[1]);
            return OR_CONTINUE;
        }
        case COMMAND_SET_CAR_MISSION:
        {
            CollectParameters(2);
            pVehicle = CPools::GetVehiclePool().GetAt(ScriptParams[0]);
            pVehicle->AutoPilot.SetMission(ScriptParams[1]);
            pVehicle->AutoPilot.LastTimeNotStuck = CTimer::GetTimeInMilliseconds();
            pVehicle->SetEngineOn(true);
            return OR_CONTINUE;
        }
        case COMMAND_IS_CAR_IN_AREA_2D:
        {
            CollectParameters(6);
            pVehicle = CPools::GetVehiclePool().GetAt(ScriptParams[0]);
            if (pVehicle->IsWithinArea(*(f32*)&ScriptParams[1], *(f32*)&ScriptParams[2], *(f32*)&ScriptParams[3], *(f32*)&ScriptParams[4]))
            {
                LatestCmpFlagResult = true;
            }
            else
            {
                LatestCmpFlagResult = false;
            }
            UpdateCompareFlag(LatestCmpFlagResult);
            if (ScriptParams[5])
            {
                CTheScripts::HighlightImportantArea(((u32)(this) + (u32)PCPointer), *(f32*)&ScriptParams[1], *(f32*)&ScriptParams[2], *(f32*)&ScriptParams[3], *(f32*)&ScriptParams[4], -100.0f);
            }
            if (CTheScripts::DbgFlag)
            {
                CTheScripts::DrawDebugSquare(*(f32*)&ScriptParams[1], *(f32*)&ScriptParams[2], *(f32*)&ScriptParams[3], *(f32*)&ScriptParams[4]);
            }
            return OR_CONTINUE;
        }
        case COMMAND_IS_CAR_IN_AREA_3D:
        {
            CollectParameters(8);
            pVehicle = CPools::GetVehiclePool().GetAt(ScriptParams[0]);
            if (pVehicle->IsWithinArea(*(f32*)&ScriptParams[1], *(f32*)&ScriptParams[2], *(f32*)&ScriptParams[3], *(f32*)&ScriptParams[4], *(f32*)&ScriptParams[5], *(f32*)&ScriptParams[6]))
            {
                LatestCmpFlagResult = true;
            }
            else
            {
                LatestCmpFlagResult = false;
            }
            UpdateCompareFlag(LatestCmpFlagResult);
            if (ScriptParams[7])
            {
                CentreZ = (*(f32*)&ScriptParams[3] + *(f32*)&ScriptParams[6]) / 2.0f;
                CTheScripts::HighlightImportantArea(((u32)(this) + (u32)PCPointer), *(f32*)&ScriptParams[1], *(f32*)&ScriptParams[2], *(f32*)&ScriptParams[4], *(f32*)&ScriptParams[5], CentreZ);
            }
            if (CTheScripts::DbgFlag)
            {
                CTheScripts::DrawDebugCube(*(f32*)&ScriptParams[1], *(f32*)&ScriptParams[2], *(f32*)&ScriptParams[3], *(f32*)&ScriptParams[4], *(f32*)&ScriptParams[5], *(f32*)&ScriptParams[6]);
            }
            return OR_CONTINUE;
        }
        case COMMAND_PRINT_BIG:
        {
            ReadTextLabelFromScript(&TextLabel[0], 8);
            pString = TheText.Get(TextLabel);
            CollectParameters(2);
            CMessages::AddBigMessage(pString, ScriptParams[0], (ScriptParams[1] - 1));
            return OR_CONTINUE;
        }
        case COMMAND_PRINT:
        {
            ReadTextLabelFromScript(&TextLabel[0], 8);
            pString = TheText.Get(TextLabel);
            CollectParameters(2);
            if (pString && (pString[0] == FO_CHAR_TOKEN_DELIMITER) && (pString[1] == FO_CHAR_TOKEN_DIALOGUE) && (pString[2] == FO_CHAR_TOKEN_DELIMITER) && (!FrontEndMenuManager.m_PrefsShowSubtitles)) {}
            else
            {
                CMessages::AddMessage(pString, ScriptParams[0], ScriptParams[1], CTheScripts::bAddNextMessageToPreviousBriefs);
            }
            CTheScripts::bAddNextMessageToPreviousBriefs = true;
            return OR_CONTINUE;
        }
        case COMMAND_PRINT_NOW:
        {
            ReadTextLabelFromScript(&TextLabel[0], 8);
            pString = TheText.Get(TextLabel);
            CollectParameters(2);
            if (pString && (pString[0] == FO_CHAR_TOKEN_DELIMITER) && (pString[1] == FO_CHAR_TOKEN_DIALOGUE) && (pString[2] == FO_CHAR_TOKEN_DELIMITER) && (!FrontEndMenuManager.m_PrefsShowSubtitles)) {}
            else
            {
                CMessages::AddMessageJumpQ(pString, ScriptParams[0], ScriptParams[1], CTheScripts::bAddNextMessageToPreviousBriefs);
            }
            CTheScripts::bAddNextMessageToPreviousBriefs = true;
            return OR_CONTINUE;
        }
        case COMMAND_CLEAR_PRINTS:
        {
            CMessages::ClearMessages(false);
            return OR_CONTINUE;
        }
        case COMMAND_GET_TIME_OF_DAY:
        {
            ScriptParams[0] = CClock::GetGameClockHours();
            ScriptParams[1] = CClock::GetGameClockMinutes();
            StoreParameters(2);
            return OR_CONTINUE;
        }
        case COMMAND_SET_TIME_OF_DAY:
        {
            CollectParameters(2);
            CClock::SetGameClock(ScriptParams[0], ScriptParams[1], CClock::GetGameClockDays());
            return OR_CONTINUE;
        }
        case COMMAND_GET_MINUTES_TO_TIME_OF_DAY:
        {
            CollectParameters(2);
            ScriptParams[0] = CClock::GetGameClockMinutesUntil(ScriptParams[0], ScriptParams[1]);
            StoreParameters(1);
            return OR_CONTINUE;
        }
        case COMMAND_IS_POINT_ON_SCREEN:
        {
            CollectParameters(4);
            NewX = *(f32*)&ScriptParams[0];
            NewY = *(f32*)&ScriptParams[1];
            NewZ = *(f32*)&ScriptParams[2];
            if (NewZ <= -100.0f)
            {
                NewZ = CWorld::FindGroundZForCoord(NewX, NewY);
            }
            LatestCmpFlagResult = false;
            if (TheCamera.IsSphereVisible(CVector(NewX, NewY, NewZ), *(f32*)&ScriptParams[3]))
            {
                LatestCmpFlagResult = true;
            }
            UpdateCompareFlag(LatestCmpFlagResult);
            return OR_CONTINUE;
        }
        case COMMAND_DEBUG_ON:
        {
            return OR_CONTINUE;
        }
        case COMMAND_DEBUG_OFF:
        {
            return OR_CONTINUE;
        }
    }
    return OR_INTERRUPT;
}

// MARK: 200To299

// stub: sa 0x469390
OpcodeResult CRunningScript::ProcessCommands200To299(i32 CurrCommand)
{
    MARKFUNCTION(0x469390);

    bool LatestCmpFlagResult;
    CVehicle* pVehicle;
    CVehicle* pVehicle2;
    CPlayerInfo* pPlayer;
    CVector TempCoors;
    CPed* pPed;
    CObject* pObj;
    i16 ButtonState;
    f32 NewX, NewY, NewZ;
    i32 ArrayIndex;
    i32 NewStoreVehicleIndex;
    CVehicle* pOldStoreVehicle;

    switch (CurrCommand)
    {
        case COMMAND_AND_OR:
        {
            CollectParameters(1);
            AndOrState = ScriptParams[0];
            if (AndOrState == NO_ANDS_OR_ORS)
            {
                CmpFlag = false;
            }
            else if ((AndOrState >= NUMBER_OF_ANDS1) && (AndOrState <= NUMBER_OF_ANDS8))
            {
                CmpFlag = true;
                AndOrState += 1;
            }
            else if ((AndOrState >= NUMBER_OF_ORS1) && (AndOrState <= NUMBER_OF_ORS8))
            {
                CmpFlag = false;
                AndOrState += 1;
            }
            else
            {
            }
            return OR_CONTINUE;
        }
        case COMMAND_LAUNCH_MISSION:
        {
            {
                CRunningScript* pNewOne;
                CollectParameters(1);
                pNewOne = CTheScripts::StartNewScript(&CTheScripts::ScriptSpace[ScriptParams[0]]);
            }
            return OR_CONTINUE;
        }
        case COMMAND_MISSION_HAS_FINISHED:
        {
            if (IsThisAMissionScript)
            {
                if (strcmp(ScriptName, "love3") == 0) {}
                CTheScripts::MissionCleanUp.Process();
            }
            return OR_CONTINUE;
        }
        case COMMAND_STORE_CAR_CHAR_IS_IN:
        {
            CollectParameters(1);
            pPed = CPools::GetPedPool().GetAt(ScriptParams[0]);
            if (pPed->bInVehicle)
            {
                pVehicle = pPed->m_pMyVehicle;
            }
            else
            {
                pVehicle = nullptr;
            }
            NewStoreVehicleIndex = CPools::GetVehiclePool().GetIndex((CAutomobile*)pVehicle);
            if (NewStoreVehicleIndex != CTheScripts::StoreVehicleIndex)
            {
                if (IsThisAMissionScript)
                {
                    pOldStoreVehicle = CPools::GetVehiclePool().GetAt(CTheScripts::StoreVehicleIndex);
                    if (pOldStoreVehicle)
                    {
                        CCarCtrl::RemoveFromInterestingVehicleList(pOldStoreVehicle);
                        if (pOldStoreVehicle->GetVehicleCreatedBy() == MISSION_VEHICLE)
                        {
                            if (CTheScripts::StoreVehicleWasRandom)
                            {
                                pOldStoreVehicle->SetVehicleCreatedBy(RANDOM_VEHICLE);
                                pOldStoreVehicle->vehicleFlags.bIsLocked = false;
                                CTheScripts::MissionCleanUp.RemoveEntityFromList(CTheScripts::StoreVehicleIndex, CLEANUP_CAR);
                            }
                        }
                    }
                    CTheScripts::StoreVehicleIndex = NewStoreVehicleIndex;
                    switch (pVehicle->GetVehicleCreatedBy())
                    {
                        case RANDOM_VEHICLE:
                        {
                            pVehicle->SetVehicleCreatedBy(MISSION_VEHICLE);
                            CTheScripts::StoreVehicleWasRandom = true;
                            CTheScripts::MissionCleanUp.AddEntityToList(CTheScripts::StoreVehicleIndex, CLEANUP_CAR);
                            break;
                        }
                        case PARKED_VEHICLE:
                        {
                            pVehicle->SetVehicleCreatedBy(MISSION_VEHICLE);
                            CTheScripts::StoreVehicleWasRandom = true;
                            CTheScripts::MissionCleanUp.AddEntityToList(CTheScripts::StoreVehicleIndex, CLEANUP_CAR);
                            break;
                        }
                        case MISSION_VEHICLE:
                        case PERMANENT_VEHICLE:
                        {
                            CTheScripts::StoreVehicleWasRandom = false;
                            break;
                        }
                        default:
                        {
                            break;
                        }
                    }
                }
                else
                {
                }
            }
            ScriptParams[0] = CTheScripts::StoreVehicleIndex;
            StoreParameters(1);
            return OR_CONTINUE;
        }
        case COMMAND_IS_CHAR_IN_CAR:
        {
            CollectParameters(2);
            pPed = CPools::GetPedPool().GetAt(ScriptParams[0]);
            pVehicle = CPools::GetVehiclePool().GetAt(ScriptParams[1]);
            if (pPed->bInVehicle)
            {
                pVehicle2 = pPed->m_pMyVehicle;
            }
            else
            {
                pVehicle2 = nullptr;
            }
            if (pVehicle2 && (pVehicle2 == pVehicle))
            {
                LatestCmpFlagResult = true;
            }
            else
            {
                LatestCmpFlagResult = false;
            }
            UpdateCompareFlag(LatestCmpFlagResult);
            return OR_CONTINUE;
        }
        case COMMAND_IS_CHAR_IN_MODEL:
        {
            CollectParameters(2);
            pPed = CPools::GetPedPool().GetAt(ScriptParams[0]);
            if (pPed->bInVehicle)
            {
                pVehicle = pPed->m_pMyVehicle;
            }
            else
            {
                pVehicle = nullptr;
            }
            if (pVehicle && (pVehicle->m_nModelIndex == ScriptParams[1]))
            {
                LatestCmpFlagResult = true;
            }
            else
            {
                LatestCmpFlagResult = false;
            }
            UpdateCompareFlag(LatestCmpFlagResult);
            return OR_CONTINUE;
        }
        case COMMAND_IS_CHAR_IN_ANY_CAR:
        {
            CollectParameters(1);
            pPed = CPools::GetPedPool().GetAt(ScriptParams[0]);
            if ((pPed->bInVehicle) && (pPed->m_pMyVehicle))
            {
                LatestCmpFlagResult = true;
            }
            else
            {
                LatestCmpFlagResult = false;
            }
            UpdateCompareFlag(LatestCmpFlagResult);
            return OR_CONTINUE;
        }
        case COMMAND_IS_BUTTON_PRESSED:
        {
            CollectParameters(2);
            if (GetPadState(ScriptParams[0], ScriptParams[1]) && !CPad::GetPad(0)->JustOutOfFrontEnd)
            {
                LatestCmpFlagResult = true;
            }
            else
            {
                LatestCmpFlagResult = false;
            }
            UpdateCompareFlag(LatestCmpFlagResult);
            return OR_CONTINUE;
        }
        case COMMAND_GET_PAD_STATE:
        {
            CollectParameters(2);
            ButtonState = GetPadState(ScriptParams[0], ScriptParams[1]);
            ScriptParams[0] = (i32)ButtonState;
            StoreParameters(1);
            return OR_CONTINUE;
        }
        case COMMAND_LOCATE_CHAR_ANY_MEANS_2D:
        case COMMAND_LOCATE_CHAR_ON_FOOT_2D:
        case COMMAND_LOCATE_CHAR_IN_CAR_2D:
        case COMMAND_LOCATE_STOPPED_CHAR_ANY_MEANS_2D:
        case COMMAND_LOCATE_STOPPED_CHAR_ON_FOOT_2D:
        case COMMAND_LOCATE_STOPPED_CHAR_IN_CAR_2D:
        {
            LocateCharCommand(CurrCommand);
            return OR_CONTINUE;
        }
        case COMMAND_LOCATE_CHAR_ANY_MEANS_CHAR_2D:
        case COMMAND_LOCATE_CHAR_ON_FOOT_CHAR_2D:
        case COMMAND_LOCATE_CHAR_IN_CAR_CHAR_2D:
        {
            LocateCharCharCommand(CurrCommand);
            return OR_CONTINUE;
        }
        case COMMAND_LOCATE_CHAR_ANY_MEANS_3D:
        case COMMAND_LOCATE_CHAR_ON_FOOT_3D:
        case COMMAND_LOCATE_CHAR_IN_CAR_3D:
        case COMMAND_LOCATE_STOPPED_CHAR_ANY_MEANS_3D:
        case COMMAND_LOCATE_STOPPED_CHAR_ON_FOOT_3D:
        case COMMAND_LOCATE_STOPPED_CHAR_IN_CAR_3D:
        {
            LocateCharCommand(CurrCommand);
            return OR_CONTINUE;
        }
        case COMMAND_LOCATE_CHAR_ANY_MEANS_CHAR_3D:
        case COMMAND_LOCATE_CHAR_ON_FOOT_CHAR_3D:
        case COMMAND_LOCATE_CHAR_IN_CAR_CHAR_3D:
        {
            LocateCharCharCommand(CurrCommand);
            return OR_CONTINUE;
        }
        case COMMAND_CREATE_OBJECT:
        {
            {
                i32 ModelIndex;
                CollectParameters(4);
                ModelIndex = *((i32*)&ScriptParams[0]);
                if (ModelIndex < 0)
                {
                    ArrayIndex = -ModelIndex;
                    ModelIndex = CTheScripts::UsedObjectArray[ArrayIndex].Index;
                }
                CBaseModelInfo* pModel = CModelInfo::GetModelInfo(ModelIndex);
                pModel->SetAlpha(255);
                CObject* pObject = CObject::Create(ModelIndex, false);
                if (bIsThisAStreamedScript || ScriptBrainType != -1)
                {
                    pObject->ObjectCreatedBy = MISSION_BRAIN_OBJECT;
                }
                else
                {
                    pObject->ObjectCreatedBy = MISSION_OBJECT;
                }
                NewX = *(f32*)&ScriptParams[1];
                NewY = *(f32*)&ScriptParams[2];
                NewZ = *(f32*)&ScriptParams[3];
                if (NewZ <= -100.0f)
                {
                    NewZ = CWorld::FindGroundZForCoord(NewX, NewY);
                }
                NewZ += pObject->GetDistanceFromCentreOfMassToBaseOfModel();
                pObject->SetPosition(NewX, NewY, NewZ);
                pObject->SetOrientation(DEGTORAD(0.0f), DEGTORAD(0.0f), DEGTORAD(0.0f));
                pObject->UpdateRwMatrix();
                pObject->UpdateRwFrame();
                if (pModel->AsLodAtomicModelInfoPtr())
                {
                    pObject->SetupBigBuilding();
                }
                CTheScripts::ClearSpaceForMissionEntity(CVector(NewX, NewY, NewZ), pObject);
                CWorld::Add(pObject);
                ScriptParams[0] = CPools::GetObjectPool().GetIndex(pObject);
                StoreParameters(1);
                if (IsThisAMissionScript)
                {
                    CTheScripts::MissionCleanUp.AddEntityToList(ScriptParams[0], CLEANUP_OBJECT);
                }
            }
            return OR_CONTINUE;
        }
        case COMMAND_DELETE_OBJECT:
        {
            CollectParameters(1);
            pObj = CPools::GetObjectPool().GetAt(ScriptParams[0]);
            if (pObj)
            {
                CWorld::Remove(pObj);
                CWorld::RemoveReferencesToDeletedObject(pObj);
                delete (pObj);
            }
            if (IsThisAMissionScript)
            {
                CTheScripts::MissionCleanUp.RemoveEntityFromList(ScriptParams[0], CLEANUP_OBJECT);
            }
            return OR_CONTINUE;
        }
        case COMMAND_ADD_SCORE:
        {
            CollectParameters(2);
            pPlayer = &(CWorld::Players[ScriptParams[0]]);
            pPlayer->Score += *((i32*)&ScriptParams[1]);
            return OR_CONTINUE;
        }
        case COMMAND_IS_SCORE_GREATER:
        {
            CollectParameters(2);
            {
                i32 PlayerScore;
                pPlayer = &(CWorld::Players[ScriptParams[0]]);
                PlayerScore = pPlayer->Score;
                if (PlayerScore > *((i32*)&ScriptParams[1]))
                {
                    LatestCmpFlagResult = true;
                }
                else
                {
                    LatestCmpFlagResult = false;
                }
                UpdateCompareFlag(LatestCmpFlagResult);
            }
            return OR_CONTINUE;
        }
        case COMMAND_STORE_SCORE:
        {
            CollectParameters(1);
            pPlayer = &(CWorld::Players[ScriptParams[0]]);
            ScriptParams[0] = pPlayer->Score;
            StoreParameters(1);
            return OR_CONTINUE;
        }
        case COMMAND_GIVE_REMOTE_CONTROLLED_CAR_TO_PLAYER:
        {
            return OR_CONTINUE;
        }
        case COMMAND_ALTER_WANTED_LEVEL:
        {
            CollectParameters(2);
            pPlayer = &(CWorld::Players[ScriptParams[0]]);
            pPlayer->m_pPed->SetWantedLevel(*((i32*)&ScriptParams[1]));
            return OR_CONTINUE;
        }
        case COMMAND_ALTER_WANTED_LEVEL_NO_DROP:
        {
            CollectParameters(2);
            pPlayer = &(CWorld::Players[ScriptParams[0]]);
            pPlayer->m_pPed->SetWantedLevelNoDrop(*((i32*)&ScriptParams[1]));
            return OR_CONTINUE;
        }
        case COMMAND_IS_WANTED_LEVEL_GREATER:
        {
            CollectParameters(2);
            pPlayer = &(CWorld::Players[ScriptParams[0]]);
            if (FindPlayerWanted(ScriptParams[0])->m_WantedLevel > *((i32*)&ScriptParams[1]))
            {
                LatestCmpFlagResult = true;
            }
            else
            {
                LatestCmpFlagResult = false;
            }
            UpdateCompareFlag(LatestCmpFlagResult);
            return OR_CONTINUE;
        }
        case COMMAND_CLEAR_WANTED_LEVEL:
        {
            CollectParameters(1);
            pPlayer = &(CWorld::Players[ScriptParams[0]]);
            pPlayer->m_pPed->SetWantedLevel(WANTED_CLEAN);
            return OR_CONTINUE;
        }
        case COMMAND_SET_DEATHARREST_STATE:
        {
            CollectParameters(1);
            if (ScriptParams[0] == true)
            {
                DeatharrestCheckEnabled = true;
            }
            else
            {
                DeatharrestCheckEnabled = false;
            }
            return OR_CONTINUE;
        }
        case COMMAND_HAS_DEATHARREST_BEEN_EXECUTED:
        {
            if (DoneDeatharrest)
            {
                LatestCmpFlagResult = true;
            }
            else
            {
                LatestCmpFlagResult = false;
            }
            UpdateCompareFlag(LatestCmpFlagResult);
            return OR_CONTINUE;
        }
        case COMMAND_ADD_AMMO_TO_CHAR:
        {
            CollectParameters(3);
            pPed = CPools::GetPedPool().GetAt(ScriptParams[0]);
            pPed->GrantAmmo((eWeaponType)ScriptParams[1], ScriptParams[2]);
            return OR_CONTINUE;
        }
        case COMMAND_IS_PLAYER_DEAD:
        {
            CollectParameters(1);
            if (CWorld::Players[ScriptParams[0]].PlayerState == PLAYERSTATE_HASDIED)
            {
                LatestCmpFlagResult = true;
            }
            else
            {
                LatestCmpFlagResult = false;
            }
            UpdateCompareFlag(LatestCmpFlagResult);
            return OR_CONTINUE;
        }
        case COMMAND_IS_CHAR_DEAD:
        {
            CollectParameters(1);
            pPed = CPools::GetPedPool().GetAt(ScriptParams[0]);
            if (pPed)
            {
                if (IsPedDead(pPed))
                {
                    LatestCmpFlagResult = true;
                }
                else
                {
                    LatestCmpFlagResult = false;
                }
            }
            else
            {
                LatestCmpFlagResult = true;
            }
            UpdateCompareFlag(LatestCmpFlagResult);
            return OR_CONTINUE;
        }
        case COMMAND_IS_CAR_DEAD:
        {
            CollectParameters(1);
            pVehicle = CPools::GetVehiclePool().GetAt(ScriptParams[0]);
            if (pVehicle)
            {
                if (pVehicle->GetStatus() == STATUS_WRECKED)
                {
                    LatestCmpFlagResult = true;
                }
                else if (pVehicle->vehicleFlags.bIsDrowning)
                {
                    LatestCmpFlagResult = true;
                }
                else
                {
                    LatestCmpFlagResult = false;
                }
            }
            else
            {
                LatestCmpFlagResult = true;
            }
            UpdateCompareFlag(LatestCmpFlagResult);
            return OR_CONTINUE;
        }
        case COMMAND_IS_PLAYER_PRESSING_HORN:
        {
            CollectParameters(1);
            if (CWorld::Players[ScriptParams[0]].m_pPed->GetPedState() == PED_DRIVING)
            {
                if (CPad::GetPad(ScriptParams[0])->GetHorn())
                {
                    LatestCmpFlagResult = true;
                }
                else
                {
                    LatestCmpFlagResult = false;
                }
            }
            else
            {
                LatestCmpFlagResult = false;
            }
            UpdateCompareFlag(LatestCmpFlagResult);
            return OR_CONTINUE;
        }
        case COMMAND_CREATE_CHAR_INSIDE_CAR:
        {
            CollectParameters(3);
            pVehicle = CPools::GetVehiclePool().GetAt(ScriptParams[0]);
            GetCorrectPedModelIndexForEmergencyServiceType(ScriptParams[1], &ScriptParams[2]);
            if (ScriptParams[1] == PEDTYPE_COP)
            {
                pPed = new CCopPed((eCopType)ScriptParams[2]);
            }
            else if ((ScriptParams[1] == PEDTYPE_MEDIC) || (ScriptParams[1] == PEDTYPE_FIRE))
            {
                pPed = new CEmergencyPed(ScriptParams[1], ScriptParams[2]);
            }
            else
            {
                pPed = new CCivilianPed((ePedType)ScriptParams[1], ScriptParams[2]);
            }
            pPed->SetCharCreatedBy(MISSION_CHAR);
            pPed->bAllowMedicsToReviveMe = false;

            {
                CTaskSimpleCarSetPedInAsDriver task(pVehicle, true);
                
                task.ProcessPed(pPed);
            }

            CWorld::Add(pPed);
            CPopulation::ms_nTotalMissionPeds++;
            ScriptParams[0] = CPools::GetPedPool().GetIndex(pPed);
            StoreParameters(1);
            if (IsThisAMissionScript)
            {
                CTheScripts::MissionCleanUp.AddEntityToList(ScriptParams[0], CLEANUP_CHAR);
            }
            return OR_CONTINUE;
        }
    }
    return OR_INTERRUPT;
}

// MARK: 300To399

// func: sa 0x47C100
OpcodeResult CRunningScript::ProcessCommands300To399(i32 CurrCommand)
{
    MARKFUNCTION(0x47C100);

    bool LatestCmpFlagResult;
    CVehicle* pVehicle;
    CPed* pPed;
    CVector TempCoors;
    CObject* pObj;
    char ZoneLabel[10];
    u64 FindZoneInt;
    i32 BlipIndex;
    f32 NewZ, NewY, NewX;
    f32 CosOfRot, SinOfRot;
    f32 fHeading;
    f32 NewHeading;

    switch (CurrCommand)
    {
        case COMMAND_IS_CAR_MODEL:
        {
            CollectParameters(2);
            pVehicle = CPools::GetVehiclePool().GetAt(ScriptParams[0]);
            UpdateCompareFlag(pVehicle->m_nModelIndex == ScriptParams[1]);
            return OR_CONTINUE;
        }
        case COMMAND_CREATE_CAR_GENERATOR:
        {
            CollectParameters(12);
            NewX = *reinterpret_cast<f32*>(&ScriptParams[0]);
            NewY = *reinterpret_cast<f32*>(&ScriptParams[1]);
            NewZ = *reinterpret_cast<f32*>(&ScriptParams[2]);
            if (NewZ > -100.0f)
            {
                NewZ += 0.015f;
            }
            ScriptParams[0] = CTheCarGenerators::CreateCarGenerator(NewX, NewY, NewZ, *reinterpret_cast<f32*>(&ScriptParams[3]), ScriptParams[4], ScriptParams[5], ScriptParams[6], ScriptParams[7], ScriptParams[8], ScriptParams[9], ScriptParams[10], ScriptParams[11], 0, true);
            StoreParameters(1);
            return OR_CONTINUE;
        }
        case COMMAND_SWITCH_CAR_GENERATOR:
        {
            CollectParameters(2);
            CCarGenerator* pCarGen = CTheCarGenerators::GetCarGenerator(static_cast<u16>(ScriptParams[0]));
            if (ScriptParams[1] == 0)
            {
                pCarGen->SwitchOff();
            }
            else if (ScriptParams[1] <= 100)
            {
                pCarGen->SwitchOnAndGenerateNumber(static_cast<i16>(ScriptParams[1]));
            }
            else
            {
                pCarGen->SwitchOn();
            }
            return OR_CONTINUE;
        }
        case COMMAND_DISPLAY_ONSCREEN_TIMER:
        {
            const u16 VarIndex = GetIndexOfGlobalVariable();
            CollectParameters(1);
            if (ScriptParams[0])
            {
                CUserDisplay::OnscnTimer.AddClock(VarIndex, nullptr, true);
            }
            else
            {
                CUserDisplay::OnscnTimer.AddClock(VarIndex, nullptr, false);
            }
            return OR_CONTINUE;
        }
        case COMMAND_CLEAR_ONSCREEN_TIMER:
        {
            CUserDisplay::OnscnTimer.ClearClock(GetIndexOfGlobalVariable());
            return OR_CONTINUE;
        }
        case COMMAND_CLEAR_ONSCREEN_COUNTER:
        {
            CUserDisplay::OnscnTimer.ClearCounter(GetIndexOfGlobalVariable());
            return OR_CONTINUE;
        }
        case COMMAND_IS_CHAR_IN_ZONE:
        {
            CollectParameters(1);
            pPed = CPools::GetPedPool().GetAt(ScriptParams[0]);
            ReadTextLabelFromScript(ZoneLabel, 8);
            FindZoneInt = 0;
            for (i32 i = 0; (reinterpret_cast<char*>(&FindZoneInt)[i] = ZoneLabel[i]) != '\0'; i++)
            {
            }
            pPed->IsPlayer();
            if (pPed->bInVehicle && pPed->m_pMyVehicle)
            {
                TempCoors = pPed->m_pMyVehicle->GetPosition();
            }
            else
            {
                TempCoors = pPed->GetPosition();
            }
            LatestCmpFlagResult = CTheZones::DoesPointLieWithinZoneWithGivenName(&TempCoors, FindZoneInt, ZONE_NAVIGATION) != false;
            UpdateCompareFlag(LatestCmpFlagResult);
            return OR_CONTINUE;
        }
        case COMMAND_POINT_CAMERA_AT_CAR:
        {
            CollectParameters(3);
            pVehicle = CPools::GetVehiclePool().GetAt(ScriptParams[0]);
            if (pVehicle)
            {
                TheCamera.TakeControl(pVehicle, ScriptParams[1], ScriptParams[2], 1);
            }
            return OR_CONTINUE;
        }
        case COMMAND_POINT_CAMERA_AT_CHAR:
        {
            CollectParameters(3);
            pPed = CPools::GetPedPool().GetAt(ScriptParams[0]);
            if (pPed)
            {
                if (pPed->IsPlayer())
                {
                    TheCamera.TakeControl(nullptr, ScriptParams[1], ScriptParams[2], 1);
                }
                else
                {
                    TheCamera.TakeControl(pPed, ScriptParams[1], ScriptParams[2], 1);
                }
            }
            return OR_CONTINUE;
        }
        case COMMAND_RESTORE_CAMERA:
        {
            TheCamera.Restore();
            return OR_CONTINUE;
        }
        case COMMAND_SHAKE_PAD:
        {
            CollectParameters(3);
            CPad::GetPad(ScriptParams[0])->StartShake(static_cast<i16>(ScriptParams[1]), static_cast<u8>(ScriptParams[2]), 0);
            return OR_CONTINUE;
        }
        case COMMAND_SET_TIME_SCALE:
        {
            CollectParameters(1);
            CTimer::ms_fTimeScale = *reinterpret_cast<f32*>(&ScriptParams[0]);
            return OR_CONTINUE;
        }
        case COMMAND_SET_FIXED_CAMERA_POSITION:
        {
            CollectParameters(6);
            CVector vecUpOffsets(*reinterpret_cast<f32*>(&ScriptParams[3]), *reinterpret_cast<f32*>(&ScriptParams[4]), *reinterpret_cast<f32*>(&ScriptParams[5]));
            CVector vecPosition(*reinterpret_cast<f32*>(&ScriptParams[0]), *reinterpret_cast<f32*>(&ScriptParams[1]), *reinterpret_cast<f32*>(&ScriptParams[2]));
            TheCamera.SetCamPositionForFixedMode(vecPosition, vecUpOffsets);
            return OR_CONTINUE;
        }
        case COMMAND_POINT_CAMERA_AT_POINT:
        {
            CollectParameters(4);
            TempCoors = CVector(*reinterpret_cast<f32*>(&ScriptParams[0]), *reinterpret_cast<f32*>(&ScriptParams[1]), *reinterpret_cast<f32*>(&ScriptParams[2]));
            if (TempCoors.z <= -100.0f)
            {
                TempCoors.z = CWorld::FindGroundZForCoord(TempCoors.x, TempCoors.y);
            }
            TheCamera.TakeControlNoEntity(TempCoors, ScriptParams[3], 1);
            return OR_CONTINUE;
        }
        case COMMAND_ADD_BLIP_FOR_CAR_OLD:
        {
            CollectParameters(3);
            CRadar::GetActualBlipArrayIndex(CollectNextParameterWithoutIncreasingPC());
            ScriptParams[0] = CRadar::SetEntityBlip(BLIPTYPE_CAR, ScriptParams[0], ScriptParams[1], static_cast<eBlipDisplay>(ScriptParams[2]));
            StoreParameters(1);
            return OR_CONTINUE;
        }
        case COMMAND_ADD_BLIP_FOR_CHAR_OLD:
        {
            return OR_CONTINUE;
        }
        case COMMAND_REMOVE_BLIP:
        {
            CollectParameters(1);
            CRadar::ClearBlip(ScriptParams[0]);
            return OR_CONTINUE;
        }
        case COMMAND_CHANGE_BLIP_COLOUR:
        {
            CollectParameters(2);
            CRadar::ChangeBlipColour(ScriptParams[0], ScriptParams[1]);
            return OR_CONTINUE;
        }
        case COMMAND_DIM_BLIP:
        {
            return OR_CONTINUE;
        }
        case COMMAND_ADD_BLIP_FOR_COORD_OLD:
        {
            CollectParameters(5);
            TempCoors = CVector(*reinterpret_cast<f32*>(&ScriptParams[0]), *reinterpret_cast<f32*>(&ScriptParams[1]), *reinterpret_cast<f32*>(&ScriptParams[2]));
            if (TempCoors.z <= -100.0f)
            {
                TempCoors.z = CWorld::FindGroundZForCoord(TempCoors.x, TempCoors.y);
            }
            CRadar::GetActualBlipArrayIndex(CollectNextParameterWithoutIncreasingPC());
            ScriptParams[0] = CRadar::SetCoordBlip(BLIPTYPE_COORDS, TempCoors, ScriptParams[3], static_cast<eBlipDisplay>(ScriptParams[4]), nullptr);
            StoreParameters(1);
            return OR_CONTINUE;
        }
        case COMMAND_CHANGE_BLIP_SCALE:
        {
            CollectParameters(2);
            CRadar::ChangeBlipScale(ScriptParams[0], ScriptParams[1]);
            return OR_CONTINUE;
        }
        case COMMAND_SET_FADING_COLOUR:
        {
            CollectParameters(3);
            TheCamera.SetFadeColour(static_cast<u8>(ScriptParams[0]), static_cast<u8>(ScriptParams[1]), static_cast<u8>(ScriptParams[2]));
            return OR_CONTINUE;
        }
        case COMMAND_DO_FADE:
        {
            CollectParameters(2);
            TheCamera.Fade(static_cast<f32>(ScriptParams[0]) / 1000.0f, static_cast<i16>(ScriptParams[1]));
            if (ScriptParams[1] == 1)
            {
                if (IsThisAMissionScript)
                {
                    CTheScripts::bScriptHasFadedOut = false;
                }
            }
            else if (IsThisAMissionScript)
            {
                CTheScripts::bScriptHasFadedOut = true;
            }
            return OR_CONTINUE;
        }
        case COMMAND_GET_FADING_STATUS:
        {
            LatestCmpFlagResult = TheCamera.GetFading();
            UpdateCompareFlag(LatestCmpFlagResult);
            return OR_CONTINUE;
        }
        case COMMAND_ADD_HOSPITAL_RESTART:
        {
            CollectParameters(5);
            TempCoors = CVector(*reinterpret_cast<f32*>(&ScriptParams[0]), *reinterpret_cast<f32*>(&ScriptParams[1]), *reinterpret_cast<f32*>(&ScriptParams[2]));
            NewHeading = *reinterpret_cast<f32*>(&ScriptParams[3]);
            if (TempCoors.z <= -100.0f)
            {
                TempCoors.z = CWorld::FindGroundZForCoord(TempCoors.x, TempCoors.y);
            }
            CRestart::AddHospitalRestartPoint(TempCoors, NewHeading, ScriptParams[4]);
            return OR_CONTINUE;
        }
        case COMMAND_ADD_POLICE_RESTART:
        {
            CollectParameters(5);
            TempCoors = CVector(*reinterpret_cast<f32*>(&ScriptParams[0]), *reinterpret_cast<f32*>(&ScriptParams[1]), *reinterpret_cast<f32*>(&ScriptParams[2]));
            NewHeading = *reinterpret_cast<f32*>(&ScriptParams[3]);
            if (TempCoors.z <= -100.0f)
            {
                TempCoors.z = CWorld::FindGroundZForCoord(TempCoors.x, TempCoors.y);
            }
            CRestart::AddPoliceRestartPoint(TempCoors, NewHeading, ScriptParams[4]);
            return OR_CONTINUE;
        }
        case COMMAND_OVERRIDE_NEXT_RESTART:
        {
            CollectParameters(4);
            TempCoors = CVector(*reinterpret_cast<f32*>(&ScriptParams[0]), *reinterpret_cast<f32*>(&ScriptParams[1]), *reinterpret_cast<f32*>(&ScriptParams[2]));
            NewHeading = *reinterpret_cast<f32*>(&ScriptParams[3]);
            if (TempCoors.z <= -100.0f)
            {
                TempCoors.z = CWorld::FindGroundZForCoord(TempCoors.x, TempCoors.y);
            }
            CRestart::OverrideNextRestart(TempCoors, NewHeading);
            return OR_CONTINUE;
        }
        case COMMAND_DRAW_SHADOW:
        {
            CollectParameters(10);
            TempCoors = CVector(*reinterpret_cast<f32*>(&ScriptParams[1]), *reinterpret_cast<f32*>(&ScriptParams[2]), *reinterpret_cast<f32*>(&ScriptParams[3]));
            if (*reinterpret_cast<f32*>(&ScriptParams[4]) == 0.0f)
            {
                CosOfRot = 0.0f;
                SinOfRot = *reinterpret_cast<f32*>(&ScriptParams[5]);
            }
            else
            {
                CosOfRot = cosf(*reinterpret_cast<f32*>(&ScriptParams[4])) * *reinterpret_cast<f32*>(&ScriptParams[5]);
                SinOfRot = sinf(*reinterpret_cast<f32*>(&ScriptParams[4])) * *reinterpret_cast<f32*>(&ScriptParams[5]);
            }
            CShadows::StoreShadowToBeRendered(static_cast<u8>(ScriptParams[0]), &TempCoors, -SinOfRot, CosOfRot, CosOfRot, SinOfRot, static_cast<i16>(ScriptParams[6]), static_cast<u8>(ScriptParams[7]), static_cast<u8>(ScriptParams[8]), static_cast<u8>(ScriptParams[9]));
            return OR_CONTINUE;
        }
        case COMMAND_GET_CHAR_HEADING:
        {
            CollectParameters(1);
            pPed = CPools::GetPedPool().GetAt(ScriptParams[0]);
            if (pPed->bInVehicle && pPed->m_pMyVehicle)
            {
                fHeading = pPed->m_pMyVehicle->GetHeading();
            }
            else
            {
                fHeading = pPed->GetHeading();
            }
            fHeading = RADTODEG(fHeading);
            if (fHeading < 0.0f)
            {
                fHeading += 360.0f;
            }
            if (fHeading > 360.0f)
            {
                fHeading -= 360.0f;
            }
            *reinterpret_cast<f32*>(&ScriptParams[0]) = fHeading;
            StoreParameters(1);
            return OR_CONTINUE;
        }
        case COMMAND_SET_CHAR_HEADING:
        {
            CollectParameters(2);
            pPed = CPools::GetPedPool().GetAt(ScriptParams[0]);
            NewHeading = *reinterpret_cast<f32*>(&ScriptParams[1]);
            if (NewHeading < 0.0f)
            {
                NewHeading += 360.0f;
            }
            if (NewHeading > 360.0f)
            {
                NewHeading -= 360.0f;
            }
            if (!pPed->bInVehicle || !pPed->m_pMyVehicle)
            {
                NewHeading = DEGTORAD(NewHeading);
                pPed->m_fCurrentHeading = NewHeading;
                pPed->m_fDesiredHeading = NewHeading;
                pPed->SetHeading(NewHeading);
                pPed->UpdateRwMatrix();
            }
            return OR_CONTINUE;
        }
        case COMMAND_GET_CAR_HEADING:
        {
            CollectParameters(1);
            pVehicle = CPools::GetVehiclePool().GetAt(ScriptParams[0]);
            fHeading = RADTODEG(pVehicle->GetHeading());
            if (fHeading < 0.0f)
            {
                fHeading += 360.0f;
            }
            if (fHeading > 360.0f)
            {
                fHeading -= 360.0f;
            }
            *reinterpret_cast<f32*>(&ScriptParams[0]) = fHeading;
            StoreParameters(1);
            return OR_CONTINUE;
        }
        case COMMAND_SET_CAR_HEADING:
        {
            CollectParameters(2);
            pVehicle = CPools::GetVehiclePool().GetAt(ScriptParams[0]);
            NewHeading = *reinterpret_cast<f32*>(&ScriptParams[1]);
            if (NewHeading < 0.0f)
            {
                NewHeading += 360.0f;
            }
            if (NewHeading > 360.0f)
            {
                NewHeading -= 360.0f;
            }
            pVehicle->SetHeading(DEGTORAD(NewHeading));
            pVehicle->UpdateRwMatrix();
            return OR_CONTINUE;
        }
        case COMMAND_GET_OBJECT_HEADING:
        {
            CollectParameters(1);
            pObj = CPools::GetObjectPool().GetAt(ScriptParams[0]);
            fHeading = RADTODEG(pObj->GetHeading());
            if (fHeading < 0.0f)
            {
                fHeading += 360.0f;
            }
            if (fHeading > 360.0f)
            {
                fHeading -= 360.0f;
            }
            *reinterpret_cast<f32*>(&ScriptParams[0]) = fHeading;
            StoreParameters(1);
            return OR_CONTINUE;
        }
        case COMMAND_SET_OBJECT_HEADING:
        {
            CollectParameters(2);
            pObj = CPools::GetObjectPool().GetAt(ScriptParams[0]);
            CWorld::Remove(pObj);
            NewHeading = *reinterpret_cast<f32*>(&ScriptParams[1]);
            if (NewHeading < 0.0f)
            {
                NewHeading += 360.0f;
            }
            if (NewHeading > 360.0f)
            {
                NewHeading -= 360.0f;
            }
            pObj->SetHeading(DEGTORAD(NewHeading));
            pObj->UpdateRwMatrix();
            pObj->UpdateRwFrame();
            CWorld::Add(pObj);
            return OR_CONTINUE;
        }
        case COMMAND_IS_CHAR_TOUCHING_OBJECT:
        {
            CollectParameters(2);
            pPed = CPools::GetPedPool().GetAt(ScriptParams[0]);
            pObj = CPools::GetObjectPool().GetAt(ScriptParams[1]);
            LatestCmpFlagResult = false;
            if (pPed->bInVehicle && pPed->m_pMyVehicle)
            {
                if (pPed->m_pMyVehicle->GetHasCollidedWith(pObj))
                {
                    LatestCmpFlagResult = true;
                }
            }
            else if (pPed->GetHasCollidedWith(pObj))
            {
                LatestCmpFlagResult = true;
            }
            UpdateCompareFlag(LatestCmpFlagResult);
            return OR_CONTINUE;
        }
        case COMMAND_SET_CHAR_AMMO:
        {
            CollectParameters(3);
            pPed = CPools::GetPedPool().GetAt(ScriptParams[0]);
            pPed->SetAmmo(static_cast<eWeaponType>(ScriptParams[1]), ScriptParams[2]);
            return OR_CONTINUE;
        }
        case COMMAND_DECLARE_MISSION_FLAG:
        {
            PCPointer++;
            CTheScripts::OnAMissionFlag = *reinterpret_cast<u16*>(PCPointer);
            PCPointer += 2;
            return OR_CONTINUE;
        }
        case COMMAND_DECLARE_MISSION_FLAG_FOR_CONTACT:
        {
            return OR_CONTINUE;
        }
        case COMMAND_ADD_BLIP_FOR_CONTACT_POINT:
        {
            return OR_CONTINUE;
        }
        case COMMAND_IS_CHAR_HEALTH_GREATER:
        {
            CollectParameters(2);
            pPed = CPools::GetPedPool().GetAt(ScriptParams[0]);
            LatestCmpFlagResult = pPed->m_nHealth > static_cast<f32>(ScriptParams[1]);
            UpdateCompareFlag(LatestCmpFlagResult);
            return OR_CONTINUE;
        }
        case COMMAND_IS_CAR_HEALTH_GREATER:
        {
            CollectParameters(2);
            pVehicle = CPools::GetVehiclePool().GetAt(ScriptParams[0]);
            LatestCmpFlagResult = pVehicle->m_nHealth > static_cast<f32>(ScriptParams[1]);
            UpdateCompareFlag(LatestCmpFlagResult);
            return OR_CONTINUE;
        }
        case COMMAND_ADD_BLIP_FOR_CAR:
        {
            CollectParameters(1);
            CRadar::GetActualBlipArrayIndex(CollectNextParameterWithoutIncreasingPC());
            BlipIndex = CRadar::SetEntityBlip(BLIPTYPE_CAR, ScriptParams[0], 0, BLIPDISPLAY_BOTH);
            CRadar::ChangeBlipScale(BlipIndex, 3);
            ScriptParams[0] = BlipIndex;
            StoreParameters(1);
            return OR_CONTINUE;
        }
        case COMMAND_ADD_BLIP_FOR_CHAR:
        {
            CollectParameters(1);
            CRadar::GetActualBlipArrayIndex(CollectNextParameterWithoutIncreasingPC());
            BlipIndex = CRadar::SetEntityBlip(BLIPTYPE_CHAR, ScriptParams[0], 1, BLIPDISPLAY_BOTH);
            CRadar::ChangeBlipScale(BlipIndex, 3);
            ScriptParams[0] = BlipIndex;
            StoreParameters(1);
            return OR_CONTINUE;
        }
        case COMMAND_ADD_BLIP_FOR_OBJECT:
        {
            CollectParameters(1);
            CRadar::GetActualBlipArrayIndex(CollectNextParameterWithoutIncreasingPC());
            BlipIndex = CRadar::SetEntityBlip(BLIPTYPE_OBJECT, ScriptParams[0], 6, BLIPDISPLAY_BOTH);
            CRadar::ChangeBlipScale(BlipIndex, 3);
            ScriptParams[0] = BlipIndex;
            StoreParameters(1);
            return OR_CONTINUE;
        }
        case COMMAND_ADD_BLIP_FOR_COORD:
        {
            CollectParameters(3);
            TempCoors = CVector(*reinterpret_cast<f32*>(&ScriptParams[0]), *reinterpret_cast<f32*>(&ScriptParams[1]), *reinterpret_cast<f32*>(&ScriptParams[2]));
            if (TempCoors.z <= -100.0f)
            {
                TempCoors.z = CWorld::FindGroundZForCoord(TempCoors.x, TempCoors.y);
            }
            CRadar::GetActualBlipArrayIndex(CollectNextParameterWithoutIncreasingPC());
            BlipIndex = CRadar::SetCoordBlip(BLIPTYPE_COORDS, TempCoors, 5, BLIPDISPLAY_BOTH, nullptr);
            CRadar::ChangeBlipScale(BlipIndex, 3);
            ScriptParams[0] = BlipIndex;
            StoreParameters(1);
            return OR_CONTINUE;
        }
        case COMMAND_CHANGE_BLIP_DISPLAY:
        {
            CollectParameters(2);
            CRadar::ChangeBlipDisplay(ScriptParams[0], static_cast<eBlipDisplay>(ScriptParams[1]));
            return OR_CONTINUE;
        }
        case COMMAND_ADD_ONE_OFF_SOUND:
        {
            CollectParameters(4);
            TempCoors = CVector(*reinterpret_cast<f32*>(&ScriptParams[0]), *reinterpret_cast<f32*>(&ScriptParams[1]), *reinterpret_cast<f32*>(&ScriptParams[2]));
            AudioEngine.ReportMissionAudioEvent(static_cast<u16>(ScriptParams[3]), TempCoors);
            return OR_CONTINUE;
        }
        case COMMAND_ADD_CONTINUOUS_SOUND:
        {
            CollectParameters(4);
            StoreParameters(1);
            return OR_CONTINUE;
        }
        case COMMAND_REMOVE_SOUND:
        {
            CollectParameters(1);
            return OR_CONTINUE;
        }
        case COMMAND_IS_CAR_STUCK_ON_ROOF:
        {
            CollectParameters(1);
            LatestCmpFlagResult = false;
            if (CTheScripts::UpsideDownCars.HasCarBeenUpsideDownForAWhile(ScriptParams[0]))
            {
                LatestCmpFlagResult = true;
            }
            UpdateCompareFlag(LatestCmpFlagResult);
            return OR_CONTINUE;
        }
        default:
            return OR_INTERRUPT;
    }
}

// MARK: 400To499

// func: sa 0x47D210
OpcodeResult CRunningScript::ProcessCommands400To499(i32 CurrCommand)
{
    MARKFUNCTION(0x47D210);

    CVehicle* pVehicle;
    CPed* pPed;
    CObject* pObj;
    CPlayerInfo* pPlayer;
    char TextLabel[10];

    switch (CurrCommand)
    {
        case COMMAND_ADD_UPSIDEDOWN_CAR_CHECK:
        {
            CollectParameters(1);
            CTheScripts::UpsideDownCars.AddCarToCheck(ScriptParams[0]);
            return OR_CONTINUE;
        }
        case COMMAND_REMOVE_UPSIDEDOWN_CAR_CHECK:
        {
            CollectParameters(1);
            CTheScripts::UpsideDownCars.RemoveCarFromCheck(ScriptParams[0]);
            return OR_CONTINUE;
        }
        case COMMAND_IS_CHAR_IN_AREA_ON_FOOT_2D:
        case COMMAND_IS_CHAR_IN_AREA_IN_CAR_2D:
        case COMMAND_IS_CHAR_STOPPED_IN_AREA_2D:
        case COMMAND_IS_CHAR_STOPPED_IN_AREA_ON_FOOT_2D:
        case COMMAND_IS_CHAR_STOPPED_IN_AREA_IN_CAR_2D:
        case COMMAND_IS_CHAR_IN_AREA_ON_FOOT_3D:
        case COMMAND_IS_CHAR_IN_AREA_IN_CAR_3D:
        case COMMAND_IS_CHAR_STOPPED_IN_AREA_3D:
        case COMMAND_IS_CHAR_STOPPED_IN_AREA_ON_FOOT_3D:
        case COMMAND_IS_CHAR_STOPPED_IN_AREA_IN_CAR_3D:
        {
            CharInAreaCheckCommand(CurrCommand);
            return OR_CONTINUE;
        }
        case COMMAND_IS_CAR_STOPPED_IN_AREA_2D:
        case COMMAND_IS_CAR_STOPPED_IN_AREA_3D:
        {
            CarInAreaCheckCommand(CurrCommand);
            return OR_CONTINUE;
        }
        case COMMAND_LOCATE_CAR_2D:
        case COMMAND_LOCATE_STOPPED_CAR_2D:
        case COMMAND_LOCATE_CAR_3D:
        case COMMAND_LOCATE_STOPPED_CAR_3D:
        {
            LocateCarCommand(CurrCommand);
            return OR_CONTINUE;
        }
        case COMMAND_GIVE_WEAPON_TO_CHAR:
        {
            CollectParameters(3);
            pPed = CPools::GetPedPool().GetAt(ScriptParams[0]);
            i32 weaponSlot = (i32)pPed->GiveWeapon(static_cast<eWeaponType>(ScriptParams[1]), ScriptParams[2], true);
            if (pPed->IsPlayer())
            {
                pPed->GetPlayerData()->m_nChosenWeapon = weaponSlot;
                return OR_CONTINUE;
            }
            pPed->SetCurrentWeapon(weaponSlot);
            if (pPed->bInVehicle && pPed->m_pMyVehicle)
            {
                CWeaponInfo* pWeaponInfo = CWeaponInfo::GetWeaponInfo(pPed->m_WeaponSlots[pPed->m_nCurrentWeapon].m_eWeaponType, 1);
                pPed->RemoveWeaponModel(pWeaponInfo->GetModelId());
            }
            return OR_CONTINUE;
        }
        case COMMAND_SET_PLAYER_CONTROL:
        {
            CollectParameters(2);
            pPlayer = &CWorld::Players[ScriptParams[0]];
            if (ScriptParams[1])
            {
                pPlayer->MakePlayerSafe(false, 10.0f);
            }
            else
            {
                pPlayer->MakePlayerSafe(true, 10.0f);
            }
            return OR_CONTINUE;
        }
        case COMMAND_FORCE_WEATHER:
        {
            CollectParameters(1);
            CWeather::ForceWeather(ScriptParams[0]);
            return OR_CONTINUE;
        }
        case COMMAND_FORCE_WEATHER_NOW:
        {
            CollectParameters(1);
            CWeather::ForceWeatherNow(ScriptParams[0]);
            return OR_CONTINUE;
        }
        case COMMAND_RELEASE_WEATHER:
        {
            CWeather::ReleaseWeather();
            return OR_CONTINUE;
        }
        case COMMAND_SET_CURRENT_CHAR_WEAPON:
        {
            CollectParameters(2);
            pPed = CPools::GetPedPool().GetAt(ScriptParams[0]);
            if (!ScriptParams[1])
            {
                if (pPed->IsPlayer())
                {
                    pPed->GetPlayerData()->m_nChosenWeapon = 0;
                }
                else
                {
                    pPed->SetCurrentWeapon(0);
                }
                return OR_CONTINUE;
            }
            for (u32 i = 0; i < 13; i++)  // TOTAL_WEAPON_SLOTS
            {
                if (pPed->m_WeaponSlots[i].m_eWeaponType == ScriptParams[1])
                {
                    if (pPed->IsPlayer())
                    {
                        pPed->GetPlayerData()->m_nChosenWeapon = i;
                    }
                    else
                    {
                        pPed->SetCurrentWeapon(i);
                    }
                }
            }
            return OR_CONTINUE;
        }
        case COMMAND_GET_OBJECT_COORDINATES:
        {
            CollectParameters(1);
            pObj = CPools::GetObjectPool().GetAt(ScriptParams[0]);
            ScriptParams[0] = *(i32*)&pObj->GetPosition().x;
            ScriptParams[1] = *(i32*)&pObj->GetPosition().y;
            ScriptParams[2] = *(i32*)&pObj->GetPosition().z;
            StoreParameters(3);
            return OR_CONTINUE;
        }
        case COMMAND_SET_OBJECT_COORDINATES:
        {
            CollectParameters(4);
            pObj = CPools::GetObjectPool().GetAt(ScriptParams[0]);
            f32 z = *(f32*)&ScriptParams[3];
            if (z <= -100.0f)
            {
                z = CWorld::FindGroundZForCoord(*(f32*)&ScriptParams[1], *(f32*)&ScriptParams[2]);
            }
            pObj->Teleport(CVector(*(f32*)&ScriptParams[1], *(f32*)&ScriptParams[2], z), false);
            CTheScripts::ClearSpaceForMissionEntity(CVector(*(f32*)&ScriptParams[1], *(f32*)&ScriptParams[2], z), pObj);
            return OR_CONTINUE;
        }
        case COMMAND_GET_GAME_TIMER:
        {
            ScriptParams[0] = CTimer::m_snTimeInMilliseconds;
            StoreParameters(1);
            return OR_CONTINUE;
        }
        case COMMAND_STORE_WANTED_LEVEL:
        {
            CollectParameters(1);
            ScriptParams[0] = (i32)(CWorld::Players[ScriptParams[0]].m_pPed->GetWantedLevel());
            StoreParameters(1);
            return OR_CONTINUE;
        }
        case COMMAND_IS_CAR_STOPPED:
        {
            CollectParameters(1);
            pVehicle = CPools::GetVehiclePool().GetAt(ScriptParams[0]);
            UpdateCompareFlag(CTheScripts::IsVehicleStopped(pVehicle));
            return OR_CONTINUE;
        }
        case COMMAND_MARK_CHAR_AS_NO_LONGER_NEEDED:
        {
            CollectParameters(1);
            pPed = CPools::GetPedPool().GetAt(ScriptParams[0]);
            CTheScripts::CleanUpThisPed(pPed);
            if (IsThisAMissionScript)
            {
                CTheScripts::MissionCleanUp.RemoveEntityFromList(ScriptParams[0], CLEANUP_CHAR);
            }
            return OR_CONTINUE;
        }
        case COMMAND_MARK_CAR_AS_NO_LONGER_NEEDED:
        {
            CollectParameters(1);
            pVehicle = CPools::GetVehiclePool().GetAt(ScriptParams[0]);
            CTheScripts::CleanUpThisVehicle(pVehicle);
            if (IsThisAMissionScript)
            {
                CTheScripts::MissionCleanUp.RemoveEntityFromList(ScriptParams[0], CLEANUP_CAR);
            }
            return OR_CONTINUE;
        }
        case COMMAND_MARK_OBJECT_AS_NO_LONGER_NEEDED:
        {
            CollectParameters(1);
            pObj = CPools::GetObjectPool().GetAt(ScriptParams[0]);
            CTheScripts::CleanUpThisObject(pObj);
            if (IsThisAMissionScript)
            {
                CTheScripts::MissionCleanUp.RemoveEntityFromList(ScriptParams[0], CLEANUP_OBJECT);
            }
            return OR_CONTINUE;
        }
        case COMMAND_DONT_REMOVE_CHAR:
        {
            CollectParameters(1);
            CTheScripts::MissionCleanUp.RemoveEntityFromList(ScriptParams[0], CLEANUP_CHAR);
            return OR_CONTINUE;
        }
        case COMMAND_DONT_REMOVE_OBJECT:
        {
            CollectParameters(1);
            CTheScripts::MissionCleanUp.RemoveEntityFromList(ScriptParams[0], CLEANUP_OBJECT);
            return OR_CONTINUE;
        }
        case COMMAND_CREATE_CHAR_AS_PASSENGER:
        {
            CollectParameters(4);
            pVehicle = CPools::GetVehiclePool().GetAt(ScriptParams[0]);
            GetCorrectPedModelIndexForEmergencyServiceType(ScriptParams[1], &ScriptParams[2]);
            if (ScriptParams[1] == PEDTYPE_COP)
            {
                pPed = new CCopPed((eCopType)ScriptParams[2]);
            }
            else if ((ScriptParams[1] == PEDTYPE_MEDIC) || (ScriptParams[1] == PEDTYPE_FIRE))
            {
                pPed = new CEmergencyPed(ScriptParams[1], ScriptParams[2]);
            }
            else
            {
                pPed = new CCivilianPed((ePedType)ScriptParams[1], ScriptParams[2]);
            }
            pPed->SetCharCreatedBy(MISSION_CHAR);
            pPed->bAllowMedicsToReviveMe = false;
            if (pVehicle->vehicleFlags.bIsBus)
            {
                pPed->bStayInCarOnJack = false;
            }
            i32 doorId = 0;
            if (ScriptParams[3] >= 0)
            {
                doorId = CCarEnterExit::ComputeTargetDoorToEnterAsPassenger(*pVehicle, ScriptParams[3]);
            }
            {
                CTaskSimpleCarSetPedInAsPassenger task(pVehicle, doorId, nullptr);
                
                task.ProcessPed(pPed);
            }
            CPopulation::ms_nTotalMissionPeds++;
            CWorld::Add(pPed);
            ScriptParams[0] = CPools::GetPedPool().GetIndex(pPed);
            StoreParameters(1);
            if (IsThisAMissionScript)
            {
                CTheScripts::MissionCleanUp.AddEntityToList(ScriptParams[0], CLEANUP_CHAR);
            }
            return OR_CONTINUE;
        }
        case COMMAND_PRINT_WITH_NUMBER_BIG:
        {
            ReadTextLabelFromScript(TextLabel, 8);
            CollectParameters(3);
            CMessages::AddBigMessageWithNumber(TheText.Get(TextLabel), ScriptParams[1], (u16)ScriptParams[2] - 1, ScriptParams[0], -1, -1, -1, -1, -1);
            return OR_CONTINUE;
        }
        case COMMAND_PRINT_WITH_NUMBER:
        {
            ReadTextLabelFromScript(TextLabel, 8);
            CollectParameters(3);
            CMessages::AddMessageWithNumber(TheText.Get(TextLabel), ScriptParams[1], ScriptParams[2], ScriptParams[0], -1, -1, -1, -1, -1, CTheScripts::bAddNextMessageToPreviousBriefs != 0);
            CTheScripts::bAddNextMessageToPreviousBriefs = true;
            return OR_CONTINUE;
        }
        case COMMAND_PRINT_WITH_NUMBER_NOW:
        {
            ReadTextLabelFromScript(TextLabel, 8);
            CollectParameters(3);
            CMessages::AddMessageJumpQWithNumber(TheText.Get(TextLabel), ScriptParams[1], ScriptParams[2], ScriptParams[0], -1, -1, -1, -1, -1, CTheScripts::bAddNextMessageToPreviousBriefs != 0);
            CTheScripts::bAddNextMessageToPreviousBriefs = true;
            return OR_CONTINUE;
        }
        case COMMAND_SWITCH_ROADS_ON:
        {
            CollectParameters(6);
            f32 minX = *(f32*)&ScriptParams[0];
            f32 maxX = *(f32*)&ScriptParams[3];
            f32 minY = *(f32*)&ScriptParams[1];
            f32 maxY = *(f32*)&ScriptParams[4];
            f32 minZ = *(f32*)&ScriptParams[2];
            f32 maxZ = *(f32*)&ScriptParams[5];
            if (*(f32*)&ScriptParams[0] > *(f32*)&ScriptParams[3])
            {
                maxX = *(f32*)&ScriptParams[0];
                minX = *(f32*)&ScriptParams[3];
            }
            if (*(f32*)&ScriptParams[1] > *(f32*)&ScriptParams[4])
            {
                maxY = *(f32*)&ScriptParams[1];
                minY = *(f32*)&ScriptParams[4];
            }
            if (*(f32*)&ScriptParams[2] > *(f32*)&ScriptParams[5])
            {
                maxZ = *(f32*)&ScriptParams[2];
                minZ = *(f32*)&ScriptParams[5];
            }
            ThePaths.SwitchRoadsOffInArea(minX, maxX, minY, maxY, minZ, maxZ, false, 1, 0);
            return OR_CONTINUE;
        }
        case COMMAND_SWITCH_ROADS_OFF:
        {
            CollectParameters(6);
            f32 minX = *(f32*)&ScriptParams[0];
            f32 maxX = *(f32*)&ScriptParams[3];
            f32 minY = *(f32*)&ScriptParams[1];
            f32 maxY = *(f32*)&ScriptParams[4];
            f32 minZ = *(f32*)&ScriptParams[2];
            f32 maxZ = *(f32*)&ScriptParams[5];
            if (*(f32*)&ScriptParams[0] > *(f32*)&ScriptParams[3])
            {
                maxX = *(f32*)&ScriptParams[0];
                minX = *(f32*)&ScriptParams[3];
            }
            if (*(f32*)&ScriptParams[1] > *(f32*)&ScriptParams[4])
            {
                maxY = *(f32*)&ScriptParams[1];
                minY = *(f32*)&ScriptParams[4];
            }
            if (*(f32*)&ScriptParams[2] > *(f32*)&ScriptParams[5])
            {
                maxZ = *(f32*)&ScriptParams[2];
                minZ = *(f32*)&ScriptParams[5];
            }
            ThePaths.SwitchRoadsOffInArea(minX, maxX, minY, maxY, minZ, maxZ, true, 1, 0);
            return OR_CONTINUE;
        }
        case COMMAND_GET_NUMBER_OF_PASSENGERS:
        {
            CollectParameters(1);
            pVehicle = CPools::GetVehiclePool().GetAt(ScriptParams[0]);
            ScriptParams[0] = pVehicle->m_nNumPassengers;
            StoreParameters(1);
            return OR_CONTINUE;
        }
        case COMMAND_GET_MAXIMUM_NUMBER_OF_PASSENGERS:
        {
            CollectParameters(1);
            pVehicle = CPools::GetVehiclePool().GetAt(ScriptParams[0]);
            ScriptParams[0] = pVehicle->m_nMaxPassengers;
            StoreParameters(1);
            return OR_CONTINUE;
        }
        case COMMAND_SET_CAR_DENSITY_MULTIPLIER:
        {
            CollectParameters(1);
            CCarCtrl::CarDensityMultiplier = *(f32*)&ScriptParams[0];
            return OR_CONTINUE;
        }
        case COMMAND_SET_CAR_HEAVY:
        {
            CollectParameters(2);
            pVehicle = CPools::GetVehiclePool().GetAt(ScriptParams[0]);
            if (pVehicle)
            {
                if (ScriptParams[1])
                {
                    pVehicle->physicalFlags.bExtraHeavy = true;
                    pVehicle->m_fMass = pVehicle->pHandling->fMass * 3.0f;
                    pVehicle->m_fTurnMass = pVehicle->pHandling->fTurnMass * 5.0f;
                    pVehicle->m_fBuoyancyConstant = pVehicle->pHandling->fBuoyancyConstant + pVehicle->pHandling->fBuoyancyConstant;
                }
                else
                {
                    pVehicle->physicalFlags.bExtraHeavy = false;
                    pVehicle->m_fMass = pVehicle->pHandling->fMass;
                    pVehicle->m_fTurnMass = pVehicle->pHandling->fTurnMass;
                    pVehicle->m_fBuoyancyConstant = pVehicle->pHandling->fBuoyancyConstant;
                }
            }
            return OR_CONTINUE;
        }
        case COMMAND_SET_MAX_WANTED_LEVEL:
        {
            CollectParameters(1);
            CWanted::SetMaximumWantedLevel(ScriptParams[0]);
            return OR_CONTINUE;
        }
        case COMMAND_IS_CAR_IN_AIR_PROPER:
        {
            CollectParameters(1);
            pVehicle = CPools::GetVehiclePool().GetAt(ScriptParams[0]);
            bool bInAir = true;
            if (pVehicle->m_nNoOfCollisionRecords)
            {
                for (i32 i = 0; i < pVehicle->m_nNoOfCollisionRecords; i++)
                {
                    if (pVehicle->m_aCollisionRecordPtrs[i])
                    {
                        u8 type = pVehicle->m_aCollisionRecordPtrs[i]->GetType();
                        if (type == ENTITY_TYPE_BUILDING || type == ENTITY_TYPE_VEHICLE)
                        {
                            bInAir = false;
                        }
                    }
                }
            }
            UpdateCompareFlag(bInAir);
            return OR_CONTINUE;
        }
        default:
            return OR_INTERRUPT;
    }
}

// MARK: 500To599

// stub: sa 0x47E090
OpcodeResult CRunningScript::ProcessCommands500To599(i32 CurrCommand)
{
    MARKFUNCTION(0x47E090);

    bool LatestCmpFlagResult;

    char TextLabel[12];
    char GarageName[12];
    char SpecialCharName[16];
    char AnimGroupName[16];

    CPlayerPedData* pPlayerData;
    const GxtChar* pText;

    CPed* pPed;
    CVehicle* pVehicle;
    CObject* pObj;

    CTask* pTask;

    i32 ModelIndex;
    i16 GarageIndex;

    f32 X1, X2, Y1, Y2, Z1, Z2;

    CVector TempCoors;

    switch (CurrCommand)
    {
        // sa: 0x0 (opcode 500)
        case COMMAND_IS_CAR_UPSIDEDOWN:
        {
            CollectParameters(1);
            TempCoors = CPools::GetVehiclePool().GetAt(ScriptParams[0])->GetUp();
            LatestCmpFlagResult = false;
            if (TempCoors.z <= 0.30000001f)
            {
                LatestCmpFlagResult = true;
            }
            UpdateCompareFlag(LatestCmpFlagResult);
            return OR_CONTINUE;
        }
        // sa: 0x53 (opcode 501)
        case COMMAND_GET_PLAYER_CHAR:
        {
            CollectParameters(1);
            ScriptParams[0] = CPools::GetPedPool().GetIndex(CWorld::Players[ScriptParams[0]].m_pPed);
            StoreParameters(1);
            return OR_CONTINUE;
        }
        // sa: 0x8e (opcode 502)
        case COMMAND_CANCEL_OVERRIDE_RESTART:
        {
            CRestart::CancelOverrideRestart();
            return OR_CONTINUE;
        }
        // sa: 0x9a (opcode 503)
        case COMMAND_SET_POLICE_IGNORE_PLAYER:
        {
            CollectParameters(2);
            pPed = CWorld::Players[ScriptParams[0]].m_pPed;
            pPlayerData = pPed->GetPlayerData();
            if (ScriptParams[1])
            {
                pPed->GetPlayerWanted()->m_PoliceBackOff = true;
                CWorld::StopAllLawEnforcersInTheirTracks();
                return OR_CONTINUE;
            }
            pPed->GetPlayerWanted()->m_PoliceBackOff = false;
            return OR_CONTINUE;
        }
        // sa: 0xf6 (opcode 504)
        case COMMAND_START_KILL_FRENZY:
        {
            ReadTextLabelFromScript(TextLabel, 8);
            if (!_stricmp(TextLabel, "DUMMY"))
            {
                pText = nullptr;
            }
            else
            {
                pText = TheText.Get(TextLabel);
            }
            CollectParameters(8);
            CDarkel::StartFrenzy((eWeaponType)ScriptParams[0], ScriptParams[1], (u16)ScriptParams[2], ScriptParams[3], pText, ScriptParams[4], ScriptParams[5], ScriptParams[6], ScriptParams[7] != 0, false);
            return OR_CONTINUE;
        }
        // sa: 0x180 (opcode 505)
        case COMMAND_READ_KILL_FRENZY_STATUS:
        {
            ScriptParams[0] = (i32)CDarkel::ReadStatus();
            StoreParameters(1);
            return OR_CONTINUE;
        }
        // sa: 0x19b (opcode 506)
        case COMMAND_SQRT:
        {
            CollectParameters(1);
            *(f32*)&ScriptParams[0] = CMaths::Sqrt(*(f32*)&ScriptParams[0]);
            StoreParameters(1);
            return OR_CONTINUE;
        }
        // sa: 0x1c9 (opcodes 507-512)
        case COMMAND_LOCATE_CHAR_ANY_MEANS_CAR_2D:
        case COMMAND_LOCATE_CHAR_ON_FOOT_CAR_2D:
        case COMMAND_LOCATE_CHAR_IN_CAR_CAR_2D:
        case COMMAND_LOCATE_CHAR_ANY_MEANS_CAR_3D:
        case COMMAND_LOCATE_CHAR_ON_FOOT_CAR_3D:
        case COMMAND_LOCATE_CHAR_IN_CAR_CAR_3D:
        {
            LocateCharCarCommand(CurrCommand);
            return OR_CONTINUE;
        }
        // sa: 0x1d8 (opcode 513)
        case COMMAND_GENERATE_RANDOM_FLOAT_IN_RANGE:
        {
            CollectParameters(2);
            *(f32*)&ScriptParams[0] = CGeneral::GetRandomNumberInRange(*(f32*)&ScriptParams[0], *(f32*)&ScriptParams[1]);
            StoreParameters(1);
            return OR_CONTINUE;
        }
        // sa: 0x21b (opcode 514)
        case COMMAND_GENERATE_RANDOM_INT_IN_RANGE:
        {
            CollectParameters(2);
            ScriptParams[0] = CGeneral::GetRandomNumberInRange(ScriptParams[0], ScriptParams[1]);
            StoreParameters(1);
            return OR_CONTINUE;
        }
        // sa: 0x25d (opcode 515)
        case COMMAND_LOCK_CAR_DOORS:
        {
            CollectParameters(2);
            pVehicle = CPools::GetVehiclePool().GetAt(ScriptParams[0]);
            if (!pVehicle)
            {
                return OR_CONTINUE;
            }
            pVehicle->m_eDoorLockState = (eCarLockState)ScriptParams[1];
            return OR_CONTINUE;
        }
        // sa: 0x28f (opcode 516)
        case COMMAND_EXPLODE_CAR:
        {
            CollectParameters(1);
            pVehicle = CPools::GetVehiclePool().GetAt(ScriptParams[0]);
            pVehicle->BlowUpCar(nullptr, false);
            return OR_CONTINUE;
        }
        // sa: 0x2b9 (opcode 517)
        case COMMAND_ADD_EXPLOSION:
        {
            CollectParameters(4);
            TempCoors = *(CVector*)&ScriptParams[0];
            CExplosion::AddExplosion(nullptr, nullptr, (eExplosionType)ScriptParams[3], TempCoors, 0, true, -1.0f, false);
            return OR_CONTINUE;
        }
        // sa: 0x30a (opcode 518)
        case COMMAND_IS_CAR_UPRIGHT:
        {
            CollectParameters(1);
            TempCoors = CPools::GetVehiclePool().GetAt(ScriptParams[0])->GetUp();
            LatestCmpFlagResult = false;
            if (TempCoors.z >= 0.0f)
            {
                LatestCmpFlagResult = true;
            }
            UpdateCompareFlag(LatestCmpFlagResult);
            return OR_CONTINUE;
        }
        // sa: 0x35d (opcode 519)
        case COMMAND_CREATE_PICKUP:
        {
            CollectParameters(5);
            ModelIndex = ScriptParams[0];
            if (ScriptParams[0] < 0)
            {
                ModelIndex = CTheScripts::UsedObjectArray[-ScriptParams[0]].Index;
            }
            X1 = *(f32*)&ScriptParams[2];
            Y1 = *(f32*)&ScriptParams[3];
            Z1 = *(f32*)&ScriptParams[4];
            if (Z1 <= -100.0f)
            {
                Z1 = CWorld::FindGroundZForCoord(X1, Y1) + 0.5f;
            }
            CPickups::GetActualPickupIndex(CollectNextParameterWithoutIncreasingPC());
            TempCoors.x = X1;
            TempCoors.y = Y1;
            TempCoors.z = Z1;
            ScriptParams[0] = CPickups::GenerateNewOne(TempCoors, ModelIndex, ScriptParams[1], 0, 0, false, nullptr);
            StoreParameters(1);
            return OR_CONTINUE;
        }
        // sa: 0x442 (opcode 520)
        case COMMAND_HAS_PICKUP_BEEN_COLLECTED:
        {
            CollectParameters(1);
            UpdateCompareFlag(CPickups::IsPickUpPickedUp(ScriptParams[0]) != 0);
            return OR_CONTINUE;
        }
        // sa: 0x46e (opcode 521)
        case COMMAND_REMOVE_PICKUP:
        {
            CollectParameters(1);
            CPickups::RemovePickUp(ScriptParams[0]);
            return OR_CONTINUE;
        }
        // sa: 0x491 (opcode 522)
        case COMMAND_SET_TAXI_LIGHTS:
        {
            CollectParameters(2);
            pVehicle = CPools::GetVehiclePool().GetAt(ScriptParams[0]);
            if (ScriptParams[1])
            {
                ((CAutomobile*)pVehicle)->SetTaxiLight(true);
            }
            else
            {
                ((CAutomobile*)pVehicle)->SetTaxiLight(false);
            }
            return OR_CONTINUE;
        }
        // sa: 0x4d3 (opcode 523)
        case COMMAND_PRINT_BIG_Q:
        {
            ReadTextLabelFromScript(TextLabel, 8);
            pText = TheText.Get(TextLabel);
            CollectParameters(2);
            CMessages::AddBigMessageQ(pText, ScriptParams[0], (u16)(ScriptParams[1] - 1));
            return OR_CONTINUE;
        }
        // sa: 0x527 (opcode 524)
        case COMMAND_SET_TARGET_CAR_FOR_MISSION_GARAGE:
        {
            ReadTextLabelFromScript(GarageName, 8);
            GarageIndex = CGarages::FindGarageIndex(GarageName);
            CollectParameters(1);
            if (GarageIndex < 0)
            {
                return OR_CONTINUE;
            }
            if (ScriptParams[0] >= 0)
            {
                pVehicle = CPools::GetVehiclePool().GetAt(ScriptParams[0]);
                CGarages::SetTargetCarForMissionGarage(GarageIndex, (CAutomobile*)pVehicle);
            }
            else
            {
                CGarages::SetTargetCarForMissionGarage(GarageIndex, nullptr);
            }
            return OR_CONTINUE;
        }
        // sa: 0x5a5 (opcodes 525, 526, 527)
        case COMMAND_IS_CAR_IN_MISSION_GARAGE:
        case COMMAND_SET_SCRIPT_POLICE_HELI_TO_CHASE_CHAR:
        case COMMAND_DRAW_LIGHT:
        {
            return OR_CONTINUE;
        }
        // sa: 0x5a9 (opcode 528)
        case COMMAND_APPLY_BRAKES_TO_PLAYERS_CAR:
        {
            CollectParameters(2);
            if (ScriptParams[1])
            {
                CPad::GetPad(ScriptParams[0])->bApplyBrakes = true;
            }
            else
            {
                CPad::GetPad(ScriptParams[0])->bApplyBrakes = false;
            }
            return OR_CONTINUE;
        }
        // sa: 0x5ef (opcode 529)
        case COMMAND_SET_CHAR_HEALTH:
        {
            CollectParameters(2);
            pPed = CPools::GetPedPool().GetAt(ScriptParams[0]);
            if (!ScriptParams[1])
            {
                pTask = new CTaskComplexDie(WEAPONTYPE_UNARMED, ANIM_STD_PED, ANIM_STD_KO_FRONT, 4.0f, 0.0f, false, false, 0, false);
                CEventScriptCommand event(TASK_PRIMARY_PRIMARY, pTask);
                pPed->GetPedIntelligence()->AddEvent(event);
                return OR_CONTINUE;
            }
            if (pPed->IsPlayer())
            {
                i32 MaxHealth = CWorld::Players[CWorld::FindPlayerSlotWithPedPointer(pPed)].m_nMaxHealth;
                if (MaxHealth >= ScriptParams[1])
                {
                    MaxHealth = ScriptParams[1];
                }
                pPed->m_nHealth = (f32)MaxHealth;
                return OR_CONTINUE;
            }
            if (pPed->m_fMaxHealth != 100.0f)
            {
                pPed->m_nHealth = (f32)ScriptParams[1];
                return OR_CONTINUE;
            }
            pPed->m_nHealth = (f32)ScriptParams[1];
            pPed->m_fMaxHealth = (f32)ScriptParams[1];
            return OR_CONTINUE;
        }
        // sa: 0x6d1 (opcode 530)
        case COMMAND_SET_CAR_HEALTH:
        {
            CollectParameters(2);
            CPools::GetVehiclePool().GetAt(ScriptParams[0])->m_nHealth = (f32)ScriptParams[1];
            return OR_CONTINUE;
        }
        // sa: 0x70a (opcode 531)
        case COMMAND_GET_CHAR_HEALTH:
        {
            CollectParameters(1);
            ScriptParams[0] = (i32)CPools::GetPedPool().GetAt(ScriptParams[0])->m_nHealth;
            StoreParameters(1);
            return OR_CONTINUE;
        }
        // sa: 0x746 (opcode 532)
        case COMMAND_GET_CAR_HEALTH:
        {
            CollectParameters(1);
            ScriptParams[0] = (i32)CPools::GetVehiclePool().GetAt(ScriptParams[0])->m_nHealth;
            StoreParameters(1);
            return OR_CONTINUE;
        }
        // sa: 0x77f (opcode 533)
        case COMMAND_CHANGE_CAR_COLOUR:
        {
            CollectParameters(3);
            pVehicle = CPools::GetVehiclePool().GetAt(ScriptParams[0]);
            pVehicle->m_colour1 = ScriptParams[1];
            pVehicle->m_colour2 = ScriptParams[2];
            return OR_CONTINUE;
        }
        // sa: 0x7bb (opcode 534)
        case COMMAND_SWITCH_PED_ROADS_ON:
        {
            CollectParameters(6);
            X1 = *(f32*)&ScriptParams[0];
            X2 = *(f32*)&ScriptParams[3];
            Y1 = *(f32*)&ScriptParams[1];
            Z1 = *(f32*)&ScriptParams[2];
            Y2 = *(f32*)&ScriptParams[4];
            Z2 = *(f32*)&ScriptParams[5];
            if (*(f32*)&ScriptParams[0] > *(f32*)&ScriptParams[3])
            {
                X2 = *(f32*)&ScriptParams[0];
                X1 = *(f32*)&ScriptParams[3];
            }
            if (*(f32*)&ScriptParams[1] > *(f32*)&ScriptParams[4])
            {
                Y2 = *(f32*)&ScriptParams[1];
                Y1 = *(f32*)&ScriptParams[4];
            }
            if (*(f32*)&ScriptParams[2] > *(f32*)&ScriptParams[5])
            {
                Z2 = *(f32*)&ScriptParams[2];
                Z1 = *(f32*)&ScriptParams[5];
            }
            ThePaths.SwitchPedRoadsOffInArea(X1, X2, Y1, Y2, Z1, Z2, false, false);
            return OR_CONTINUE;
        }
        // sa: 0x88a (opcode 535)
        case COMMAND_SWITCH_PED_ROADS_OFF:
        {
            CollectParameters(6);
            X1 = *(f32*)&ScriptParams[0];
            X2 = *(f32*)&ScriptParams[3];
            Y1 = *(f32*)&ScriptParams[1];
            Z1 = *(f32*)&ScriptParams[2];
            Y2 = *(f32*)&ScriptParams[4];
            Z2 = *(f32*)&ScriptParams[5];
            if (*(f32*)&ScriptParams[0] > *(f32*)&ScriptParams[3])
            {
                X2 = *(f32*)&ScriptParams[0];
                X1 = *(f32*)&ScriptParams[3];
            }
            if (*(f32*)&ScriptParams[1] > *(f32*)&ScriptParams[4])
            {
                Y2 = *(f32*)&ScriptParams[1];
                Y1 = *(f32*)&ScriptParams[4];
            }
            if (*(f32*)&ScriptParams[2] > *(f32*)&ScriptParams[5])
            {
                Z2 = *(f32*)&ScriptParams[2];
                Z1 = *(f32*)&ScriptParams[5];
            }
            ThePaths.SwitchPedRoadsOffInArea(X1, X2, Y1, Y2, Z1, Z2, true, false);
            return OR_CONTINUE;
        }
        // sa: 0x959 (opcode 536)
        case COMMAND_SET_GANG_PED_MODELS:
        {
            CollectParameters(3);
            return OR_CONTINUE;
        }
        // sa: 0x968 (opcode 537)
        case COMMAND_SET_GANG_CAR_MODEL:
        {
            CollectParameters(2);
            return OR_CONTINUE;
        }
        // sa: 0x977 (opcode 538)
        case COMMAND_SET_GANG_WEAPONS:
        {
            CollectParameters(4);
            CGangs::SetGangWeapons(ScriptParams[0], ScriptParams[1], ScriptParams[2], ScriptParams[3]);
            return OR_CONTINUE;
        }
        // sa: 0x9b3 (opcode 539)
        case COMMAND_IS_CHAR_TOUCHING_OBJECT_ON_FOOT:
        {
            CollectParameters(2);
            pPed = CPools::GetPedPool().GetAt(ScriptParams[0]);
            pObj = CPools::GetObjectPool().GetAt(ScriptParams[1]);
            LatestCmpFlagResult = false;
            if (pPed->bInVehicle && pPed->m_pMyVehicle)
            {
                UpdateCompareFlag(false);
                return OR_CONTINUE;
            }
            if (pPed->GetHasCollidedWith(pObj))
            {
                LatestCmpFlagResult = true;
            }
            UpdateCompareFlag(LatestCmpFlagResult);
            return OR_CONTINUE;
        }
        // sa: 0xa1c (opcode 540)
        case COMMAND_LOAD_SPECIAL_CHARACTER:
        {
            CollectParameters(1);
            ScriptParams[0]--;
            ReadTextLabelFromScript(SpecialCharName, 8);
            char* pChar = SpecialCharName;
            i32 nCharsLeft = 8;
            do
            {
                u8 Char = *pChar;
                if (Char >= 'A' && Char <= 'Z')
                {
                    *pChar = Char + 32;
                }
                pChar++;
                nCharsLeft--;
            } while (nCharsLeft);
            CStreaming::RequestSpecialChar(ScriptParams[0], SpecialCharName, STRFLAG_DONTDELETE | STRFLAG_MISSION_REQUIRED);
            CTheScripts::ScriptResourceManager.AddToResourceManager(ScriptParams[0] + 290, 2, this);
            return OR_CONTINUE;
        }
        // sa: 0xac8 (opcode 541)
        case COMMAND_HAS_SPECIAL_CHARACTER_LOADED:
        {
            CollectParameters(1);
            ScriptParams[0]--;
            UpdateCompareFlag(CStreaming::HasSpecialCharLoaded(ScriptParams[0]));
            return OR_CONTINUE;
        }
        // sa: 0xb06 (opcode 542)
        case COMMAND_IS_PLAYER_IN_REMOTE_MODE:
        {
            CollectParameters(1);
            LatestCmpFlagResult = false;
            if (CWorld::Players[ScriptParams[0]].IsPlayerInRemoteMode())
            {
                LatestCmpFlagResult = true;
            }
            UpdateCompareFlag(LatestCmpFlagResult);
            return OR_CONTINUE;
        }
        // sa: 0xb54 (opcode 544)
        case COMMAND_SET_CUTSCENE_OFFSET:
        {
            CollectParameters(3);
            TempCoors = *(CVector*)&ScriptParams[0];
            CCutsceneMgr::SetCutsceneOffset(TempCoors);
            return OR_CONTINUE;
        }
        // sa: 0xb96 (opcode 545)
        case COMMAND_SET_ANIM_GROUP_FOR_CHAR:
        {
            CollectParameters(1);
            pPed = CPools::GetPedPool().GetAt(ScriptParams[0]);
            ReadTextLabelFromScript(AnimGroupName, 16);
            u16 GroupId = 0;
            bool bFound = false;
            while (GroupId < CAnimManager::GetNumAnimAssocDefinitions())
            {
                if (bFound)
                {
                    break;
                }
                if (!_stricmp(AnimGroupName, CAnimManager::GetAnimGroupName((AssocGroupId)GroupId)))
                {
                    bFound = true;
                }
                else
                {
                    GroupId++;
                }
            }
            pPed->m_motionAnimGroup = (AssocGroupId)GroupId;
            return OR_CONTINUE;
        }
        // sa: 0xefc (opcode 546)
        case COMMAND_REQUEST_MODEL:
        {
            CollectParameters(1);
            ModelIndex = ScriptParams[0];
            if (ScriptParams[0] < 0)
            {
                ModelIndex = CTheScripts::UsedObjectArray[-ScriptParams[0]].Index;
            }
            CStreaming::RequestModel(ModelIndex, STRFLAG_DONTDELETE | STRFLAG_MISSION_REQUIRED);
            CTheScripts::ScriptResourceManager.AddToResourceManager(ModelIndex, 2, this);
            return OR_CONTINUE;
        }
        // sa: 0xf5b (opcode 547)
        case COMMAND_HAS_MODEL_LOADED:
        {
            CollectParameters(1);
            ModelIndex = ScriptParams[0];
            if (ScriptParams[0] < 0)
            {
                ModelIndex = CTheScripts::UsedObjectArray[-ScriptParams[0]].Index;
            }
            LatestCmpFlagResult = false;
            if (CStreaming::HasModelLoaded(ModelIndex))
            {
                LatestCmpFlagResult = true;
            }
            UpdateCompareFlag(LatestCmpFlagResult);
            return OR_CONTINUE;
        }
        // sa: 0xfc2 (opcode 548)
        case COMMAND_MARK_MODEL_AS_NO_LONGER_NEEDED:
        {
            CollectParameters(1);
            ModelIndex = ScriptParams[0];
            if (ScriptParams[0] < 0)
            {
                ModelIndex = CTheScripts::UsedObjectArray[-ScriptParams[0]].Index;
            }
            if (CTheScripts::ScriptResourceManager.RemoveFromResourceManager(ModelIndex, 2, this))
            {
                CStreaming::SetMissionDoesntRequireModel(ModelIndex);
            }
            return OR_CONTINUE;
        }
        // sa: 0x1032 (opcode 549)
        case COMMAND_DRAW_CORONA:
        {
            CollectParameters(9);
            Z1 = *(f32*)&ScriptParams[2];
            Y1 = *(f32*)&ScriptParams[1];
            X1 = *(f32*)&ScriptParams[0];
            if (Z1 <= -100.0f)
            {
                Z1 = CWorld::FindGroundZForCoord(X1, Y1);
            }
            TempCoors.z = Z1;
            TempCoors.x = X1;
            TempCoors.y = Y1;
            CCoronas::RegisterCorona((u_native)this + (u_native)&PCPointer[(u_native)((Y1 + 3000.0f) * 12001.0f + X1 + 3000.0f)], nullptr, ScriptParams[6], ScriptParams[7], ScriptParams[8], 255, TempCoors, *(f32*)&ScriptParams[3], 450.0f, (u8)ScriptParams[4], ScriptParams[5], 1, 0, 0, 0.0f, false, 1.5f, false, 15.0f, false, false);
            return OR_CONTINUE;
        }
        // sa: 0x1284 (opcode 550)
        case COMMAND_STORE_CLOCK:
        {
            CClock::StoreClock();
            return OR_CONTINUE;
        }
        // sa: 0x1290 (opcode 551)
        case COMMAND_RESTORE_CLOCK:
        {
            CClock::RestoreClock();
            return OR_CONTINUE;
        }
        // sa: 0x129c (opcode 552)
        case COMMAND_IS_PLAYER_PLAYING:
        {
            CollectParameters(1);
            LatestCmpFlagResult = false;
            if (CWorld::Players[ScriptParams[0]].PlayerState == PLAYERSTATE_PLAYING)
            {
                LatestCmpFlagResult = true;
            }
            UpdateCompareFlag(LatestCmpFlagResult);
            return OR_CONTINUE;
        }
    }

    return OR_INTERRUPT;
}

// MARK: 600To699

// stub: sa 0x47F370
OpcodeResult CRunningScript::ProcessCommands600To699(i32 CurrCommand)
{
    MARKFUNCTION(0x47F370);

    bool LatestCmpFlagResult;
    char GarageName[12];
    i32 GarageIndex;

    i32 ModelIndex;
    CBaseModelInfo* pModelInfo;

    CObject* pObject;
    CVehicle* pVehicle;
    CPed* pPed;
    CPhysical* pPhysical;

    i32 BlipIndex;

    f32 NewX, NewY, NewZ;
    CVector BlipCoors;
    CVector TempCoors;

    switch (CurrCommand)
    {
        // sa: 0x29 (opcode 600)
        case COMMAND_GET_CONTROLLER_MODE:
        {
            ScriptParams[0] = 0;
            StoreParameters(1);
            return OR_CONTINUE;
        }
        // sa: 0x48 (opcode 601)
        case COMMAND_SET_CAN_RESPRAY_CAR:
        {
            CollectParameters(2);
            pVehicle = CPools::GetVehiclePool().GetAt(ScriptParams[0]);
            if (ScriptParams[1])
            {
                ((CAutomobile*)pVehicle)->autoFlags.bShouldNotChangeColour = false;
            }
            else
            {
                ((CAutomobile*)pVehicle)->autoFlags.bShouldNotChangeColour = true;
            }
            return OR_CONTINUE;
        }
        // sa: 0x9d (opcode 602)
        case COMMAND_UNLOAD_SPECIAL_CHARACTER:
        {
            CollectParameters(1);
            ScriptParams[0]--;
            if (!CTheScripts::ScriptResourceManager.RemoveFromResourceManager(ScriptParams[0] + 290, 2, this))
            {
                return OR_CONTINUE;
            }
            CStreaming::SetMissionDoesntRequireSpecialChar(ScriptParams[0]);
            return OR_CONTINUE;
        }
        // sa: 0xe7 (opcode 603)
        case COMMAND_RESET_NUM_OF_MODELS_KILLED_BY_PLAYER:
        {
            CollectParameters(1);
            CDarkel::ResetModelsKilledByPlayer(ScriptParams[0]);
            return OR_CONTINUE;
        }
        // sa: 0x10a (opcode 604)
        case COMMAND_GET_NUM_OF_MODELS_KILLED_BY_PLAYER:
        {
            CollectParameters(2);
            ScriptParams[0] = CDarkel::QueryModelsKilledByPlayer(ScriptParams[1], ScriptParams[0]);
            StoreParameters(1);
            return OR_CONTINUE;
        }
        // sa: 0x146 (opcode 605)
        case COMMAND_ACTIVATE_GARAGE:
        {
            ReadTextLabelFromScript(GarageName, 8);
            GarageIndex = CGarages::FindGarageIndex(GarageName);
            if (GarageIndex < 0)
            {
                return OR_CONTINUE;
            }
            CGarages::ActivateGarage(GarageIndex);
            return OR_CONTINUE;
        }
        // sa: 0x181 (opcode 606)
        case COMMAND_CREATE_OBJECT_NO_OFFSET:
        {
            CollectParameters(4);
            ModelIndex = ScriptParams[0];
            if (ModelIndex < 0)
            {
                ModelIndex = CTheScripts::UsedObjectArray[-ScriptParams[0]].Index;
            }
            pModelInfo = CModelInfo::GetModelInfo(ModelIndex);
            pModelInfo->SetAlpha(255);
            pObject = CObject::Create(ModelIndex, false);
            if (bIsThisAStreamedScript || ScriptBrainType != -1)
            {
                pObject->ObjectCreatedBy = MISSION_BRAIN_OBJECT;
            }
            else
            {
                pObject->ObjectCreatedBy = MISSION_OBJECT;
            }
            NewX = *(f32*)&ScriptParams[1];
            NewY = *(f32*)&ScriptParams[2];
            NewZ = *(f32*)&ScriptParams[3];
            if (NewZ <= -100.0f)
            {
                NewZ = CWorld::FindGroundZForCoord(NewX, NewY);
            }
            pObject->SetPosition(NewX, NewY, NewZ);
            pObject->SetOrientation(0.0f, 0.0f, 0.0f);
            pObject->UpdateRwMatrix();
            pObject->UpdateRwFrame();
            if (pModelInfo->AsLodAtomicModelInfoPtr())
            {
                pObject->SetupBigBuilding();
            }
            TempCoors.x = NewX;
            TempCoors.y = NewY;
            TempCoors.z = NewZ;
            CTheScripts::ClearSpaceForMissionEntity(TempCoors, pObject);
            CWorld::Add(pObject);
            ScriptParams[0] = CPools::GetObjectPool().GetIndex(pObject);
            StoreParameters(1);
            if (!IsThisAMissionScript)
            {
                return OR_CONTINUE;
            }
            CTheScripts::MissionCleanUp.AddEntityToList(ScriptParams[0], 3);
            return OR_CONTINUE;
        }
        // sa: 0x2ca (opcode 607)
        case COMMAND_IS_CHAR_STOPPED:
        {
            CollectParameters(1);
            pPed = CPools::GetPedPool().GetAt(ScriptParams[0]);
            LatestCmpFlagResult = false;
            if (CTheScripts::IsPedStopped(pPed))
            {
                LatestCmpFlagResult = true;
            }
            UpdateCompareFlag(LatestCmpFlagResult);
            return OR_CONTINUE;
        }
        // sa: 0x314 (opcode 608)
        case COMMAND_SWITCH_WIDESCREEN:
        {
            CollectParameters(1);
            if (ScriptParams[0])
            {
                TheCamera.SetWideScreenOn();
            }
            else
            {
                TheCamera.SetWideScreenOff();
            }
            return OR_CONTINUE;
        }
        // sa: 0x2a4 (opcodes 609, 610)
        case COMMAND_ADD_SPRITE_BLIP_FOR_CAR:
        case COMMAND_ADD_SPRITE_BLIP_FOR_CHAR:
        {
            return OR_CONTINUE;
        }
        // sa: 0x34d (opcode 611)
        case COMMAND_ADD_SPRITE_BLIP_FOR_CONTACT_POINT:
        {
            CollectParameters(4);
            NewX = *(f32*)&ScriptParams[0];
            NewY = *(f32*)&ScriptParams[1];
            NewZ = *(f32*)&ScriptParams[2];
            if (NewZ <= -100.0f)
            {
                NewZ = CWorld::FindGroundZForCoord(NewX, NewY);
            }
            CRadar::GetActualBlipArrayIndex(CollectNextParameterWithoutIncreasingPC());
            BlipCoors.x = NewX;
            BlipCoors.y = NewY;
            BlipCoors.z = NewZ;
            BlipIndex = CRadar::SetCoordBlip(BLIPTYPE_CONTACT, BlipCoors, 2, BLIPDISPLAY_BOTH, ScriptName);
            CRadar::SetBlipSprite(BlipIndex, ScriptParams[3]);
            ScriptParams[0] = BlipIndex;
            StoreParameters(1);
            return OR_CONTINUE;
        }
        // sa: 0x3ed (opcode 612)
        case COMMAND_ADD_SPRITE_BLIP_FOR_COORD:
        {
            CollectParameters(4);
            NewX = *(f32*)&ScriptParams[0];
            NewY = *(f32*)&ScriptParams[1];
            NewZ = *(f32*)&ScriptParams[2];
            if (NewZ <= -100.0f)
            {
                NewZ = CWorld::FindGroundZForCoord(NewX, NewY);
            }
            CRadar::GetActualBlipArrayIndex(CollectNextParameterWithoutIncreasingPC());
            BlipCoors.x = NewX;
            BlipCoors.y = NewY;
            BlipCoors.z = NewZ;
            BlipIndex = CRadar::SetCoordBlip(BLIPTYPE_COORDS, BlipCoors, 5, BLIPDISPLAY_BOTH, ScriptName);
            CRadar::SetBlipSprite(BlipIndex, ScriptParams[3]);
            ScriptParams[0] = BlipIndex;
            StoreParameters(1);
            return OR_CONTINUE;
        }
        // sa: 0x4b1 (opcode 613)
        case COMMAND_SET_CHAR_ONLY_DAMAGED_BY_PLAYER:
        {
            CollectParameters(2);
            pPhysical = CPools::GetPedPool().GetAt(ScriptParams[0]);
            if (ScriptParams[1])
            {
                pPhysical->physicalFlags.bOnlyDamagedByPlayer = true;
            }
            else
            {
                pPhysical->physicalFlags.bOnlyDamagedByPlayer = false;
            }
            return OR_CONTINUE;
        }
        // sa: 0x4ee (opcode 614)
        case COMMAND_SET_CAR_ONLY_DAMAGED_BY_PLAYER:
        {
            CollectParameters(2);
            pPhysical = CPools::GetVehiclePool().GetAt(ScriptParams[0]);
            if (ScriptParams[1])
            {
                pPhysical->physicalFlags.bOnlyDamagedByPlayer = true;
            }
            else
            {
                pPhysical->physicalFlags.bOnlyDamagedByPlayer = false;
            }
            return OR_CONTINUE;
        }
        // sa: 0x51f (opcode 615)
        case COMMAND_SET_CHAR_PROOFS:
        {
            CollectParameters(6);
            pPhysical = CPools::GetPedPool().GetAt(ScriptParams[0]);
            if (ScriptParams[1])
            {
                pPhysical->physicalFlags.bNotDamagedByBullets = true;
            }
            else
            {
                pPhysical->physicalFlags.bNotDamagedByBullets = false;
            }
            if (ScriptParams[2])
            {
                pPhysical->physicalFlags.bNotDamagedByFlames = true;
            }
            else
            {
                pPhysical->physicalFlags.bNotDamagedByFlames = false;
            }
            if (ScriptParams[3])
            {
                pPhysical->physicalFlags.bIgnoresExplosions = true;
            }
            else
            {
                pPhysical->physicalFlags.bIgnoresExplosions = false;
            }
            if (ScriptParams[4])
            {
                pPhysical->physicalFlags.bNotDamagedByCollisions = true;
            }
            else
            {
                pPhysical->physicalFlags.bNotDamagedByCollisions = false;
            }
            if (ScriptParams[5])
            {
                pPhysical->physicalFlags.bNotDamagedByMelee = true;
            }
            else
            {
                pPhysical->physicalFlags.bNotDamagedByMelee = false;
            }
            return OR_CONTINUE;
        }
        // sa: 0x598 (opcode 616)
        case COMMAND_SET_CAR_PROOFS:
        {
            CollectParameters(6);
            pPhysical = CPools::GetVehiclePool().GetAt(ScriptParams[0]);
            if (ScriptParams[1])
            {
                pPhysical->physicalFlags.bNotDamagedByBullets = true;
            }
            else
            {
                pPhysical->physicalFlags.bNotDamagedByBullets = false;
            }
            if (ScriptParams[2])
            {
                pPhysical->physicalFlags.bNotDamagedByFlames = true;
            }
            else
            {
                pPhysical->physicalFlags.bNotDamagedByFlames = false;
            }
            if (ScriptParams[3])
            {
                pPhysical->physicalFlags.bIgnoresExplosions = true;
            }
            else
            {
                pPhysical->physicalFlags.bIgnoresExplosions = false;
            }
            if (ScriptParams[4])
            {
                pPhysical->physicalFlags.bNotDamagedByCollisions = true;
            }
            else
            {
                pPhysical->physicalFlags.bNotDamagedByCollisions = false;
            }
            if (ScriptParams[5])
            {
                pPhysical->physicalFlags.bNotDamagedByMelee = true;
            }
            else
            {
                pPhysical->physicalFlags.bNotDamagedByMelee = false;
            }
            return OR_CONTINUE;
        }
        // sa: 0x606 (opcode 617)
        case COMMAND_DEACTIVATE_GARAGE:
        {
            ReadTextLabelFromScript(GarageName, 8);
            GarageIndex = CGarages::FindGarageIndex(GarageName);
            if (GarageIndex >= 0)
            {
                CGarages::DeActivateGarage(GarageIndex);
            }
            return OR_CONTINUE;
        }
    }

    return OR_INTERRUPT;
}

// MARK: 700To799

// func: sa 0x47FA30
OpcodeResult CRunningScript::ProcessCommands700To799(i32 CurrCommand)
{
    MARKFUNCTION(0x47FA30);

    CVehicle* pVehicle;
    CPed* pPed;
    CObject* pObject;
    f32 fRadius;
    CVector vecBoundCentre;

    switch (CurrCommand)
    {
        case COMMAND_IS_CAR_IN_WATER:
        {
            CollectParameters(1);
            pVehicle = CPools::GetVehiclePool().GetAt(ScriptParams[0]);
            bool bInWater = false;
            if (pVehicle)
            {
                if (pVehicle->physicalFlags.bIsInWater)
                {
                    bInWater = true;
                }
                else if (pVehicle->m_nModelIndex == MODELID_CAR_VORTEX)
                {
                    CAutomobile* pAutomobile = static_cast<CAutomobile*>(pVehicle);
                    if (pAutomobile->m_aWheelRatios[0] < 1.0f)
                    {
                        if (g_surfaceInfos.IsShallowWater(pAutomobile->m_aWheelColPoints[0].GetSurfaceTypeB()))
                        {
                            bInWater = true;
                        }
                    }
                }
            }
            UpdateCompareFlag(bInWater);
            return OR_CONTINUE;
        }

        case COMMAND_GET_CLOSEST_CHAR_NODE:
        {
            CollectParameters(3);
            f32 z = *(f32*)&ScriptParams[2];
            f32 x = *(f32*)&ScriptParams[0];
            f32 y = *(f32*)&ScriptParams[1];
            if (z <= -100.0f)
            {
                z = CWorld::FindGroundZForCoord(x, y);
            }
            CNodeAddress node = ThePaths.FindNodeClosestToCoors(CVector(x, y, z), 1, 999999.9f, true, false, false, false);
            if (node.IsEmpty())
            {
                *(f32*)&ScriptParams[0] = 0.0f;
                ScriptParams[1] = 0;
                ScriptParams[2] = 0;
                UpdateCompareFlag(false);
            }
            else
            {
                CVector coors = ThePaths.FindNodePointer(node)->GetCoors();
                *(f32*)&ScriptParams[0] = coors.x;
                *(f32*)&ScriptParams[1] = coors.y;
                *(f32*)&ScriptParams[2] = coors.z;
                UpdateCompareFlag(true);
            }
            StoreParameters(3);
            return OR_CONTINUE;
        }

        case COMMAND_GET_CLOSEST_CAR_NODE:
        {
            CollectParameters(3);
            f32 z = *(f32*)&ScriptParams[2];
            f32 x = *(f32*)&ScriptParams[0];
            f32 y = *(f32*)&ScriptParams[1];
            if (z <= -100.0f)
            {
                z = CWorld::FindGroundZForCoord(x, y);
            }
            CNodeAddress node = ThePaths.FindNodeClosestToCoors(CVector(x, y, z), 0, 999999.9f, false, true, false, false);
            bool bSuccess = false;
            CVector coors = ThePaths.FindNodeCoorsForScript(node, &bSuccess);
            if (bSuccess)
            {
                *(f32*)&ScriptParams[0] = coors.x;
                *(f32*)&ScriptParams[1] = coors.y;
                *(f32*)&ScriptParams[2] = coors.z;
                UpdateCompareFlag(true);
            }
            else
            {
                *(f32*)&ScriptParams[0] = 0.0f;
                ScriptParams[1] = 0;
                ScriptParams[2] = 0;
                UpdateCompareFlag(false);
            }
            StoreParameters(3);
            return OR_CONTINUE;
        }

        case COMMAND_CAR_GOTO_COORDINATES_ACCURATE:
        {
            CollectParameters(4);
            pVehicle = CPools::GetVehiclePool().GetAt(ScriptParams[0]);
            f32 NewX = *(f32*)&ScriptParams[1];
            f32 NewY = *(f32*)&ScriptParams[2];
            f32 NewZ = *(f32*)&ScriptParams[3];
            if (NewZ <= -100.0f)
            {
                NewZ = CWorld::FindGroundZForCoord(NewX, NewY);
            }
            NewZ += pVehicle->GetDistanceFromCentreOfMassToBaseOfModel();
            if (CCarCtrl::JoinCarWithRoadSystemGotoCoors(pVehicle, CVector(NewX, NewY, NewZ), false, false))
            {
                pVehicle->AutoPilot.SetMission(MISSION_GOTOCOORDINATES_STRAIGHTLINE_ACCURATE);
            }
            else
            {
                pVehicle->AutoPilot.SetMission(MISSION_GOTOCOORDINATES_ACCURATE);
            }
            pVehicle->SetStatus(STATUS_PHYSICS);
            pVehicle->SetEngineOn(true);
            pVehicle->AutoPilot.CruiseSpeed = MAX(pVehicle->AutoPilot.CruiseSpeed, 1);
            pVehicle->AutoPilot.LastTimeNotStuck = CTimer::GetTimeInMilliseconds();
            return OR_CONTINUE;
        }

        case COMMAND_IS_CAR_ON_SCREEN:
        {
            CollectParameters(1);
            pVehicle = CPools::GetVehiclePool().GetAt(ScriptParams[0]);
            fRadius = CModelInfo::GetColModel(pVehicle->m_nModelIndex).GetBoundRadius();
            pVehicle->GetBoundCentre(vecBoundCentre);
            UpdateCompareFlag(TheCamera.IsSphereVisible(vecBoundCentre, fRadius));
            return OR_CONTINUE;
        }

        case COMMAND_IS_CHAR_ON_SCREEN:
        {
            CollectParameters(1);
            pPed = CPools::GetPedPool().GetAt(ScriptParams[0]);
            fRadius = CModelInfo::GetColModel(pPed->m_nModelIndex).GetBoundRadius();
            pPed->GetBoundCentre(vecBoundCentre);
            UpdateCompareFlag(TheCamera.IsSphereVisible(vecBoundCentre, fRadius));
            return OR_CONTINUE;
        }

        case COMMAND_IS_OBJECT_ON_SCREEN:
        {
            CollectParameters(1);
            pObject = CPools::GetObjectPool().GetAt(ScriptParams[0]);
            fRadius = CModelInfo::GetColModel(pObject->m_nModelIndex).GetBoundRadius();
            pObject->GetBoundCentre(vecBoundCentre);
            UpdateCompareFlag(TheCamera.IsSphereVisible(vecBoundCentre, fRadius));
            return OR_CONTINUE;
        }

        case COMMAND_GET_GROUND_Z_FOR_3D_COORD:
        {
            CollectParameters(3);
            bool bFoundGround;
            *(f32*)&ScriptParams[0] = CWorld::FindGroundZFor3DCoord(*(f32*)&ScriptParams[0], *(f32*)&ScriptParams[1], *(f32*)&ScriptParams[2], &bFoundGround, nullptr);
            StoreParameters(1);
            return OR_CONTINUE;
        }

        case COMMAND_START_SCRIPT_FIRE:
        {
            CollectParameters(5);
            f32 z = *(f32*)&ScriptParams[2];
            if (z <= -100.0f)
            {
                z = CWorld::FindGroundZForCoord(*(f32*)&ScriptParams[0], *(f32*)&ScriptParams[1]);
            }
            CVector pos(*(f32*)&ScriptParams[0], *(f32*)&ScriptParams[1], z);
            ScriptParams[0] = gFireManager.StartScriptFire(pos, nullptr, 0.8f, 1, (u8)ScriptParams[3], (u8)ScriptParams[4]);
            StoreParameters(1);
            return OR_CONTINUE;
        }

        case COMMAND_IS_SCRIPT_FIRE_EXTINGUISHED:
        {
            CollectParameters(1);
            i32 actualIndex = CTheScripts::GetActualScriptThingIndex(ScriptParams[0], 5);
            bool bExtinguished = false;
            if (actualIndex >= 0 && actualIndex < 60)
            {
                bExtinguished = gFireManager.IsScriptFireExtinguished((i16)actualIndex);
            }
            UpdateCompareFlag(bExtinguished);
            return OR_CONTINUE;
        }

        case COMMAND_REMOVE_SCRIPT_FIRE:
        {
            CollectParameters(1);
            i32 actualIndex = CTheScripts::GetActualScriptThingIndex(ScriptParams[0], 5);
            if (actualIndex >= 0 && actualIndex < 60)
            {
                gFireManager.RemoveScriptFire((i16)actualIndex);
            }
            return OR_CONTINUE;
        }

        case COMMAND_BOAT_GOTO_COORDS:
        {
            CollectParameters(4);
            pVehicle = CPools::GetVehiclePool().GetAt(ScriptParams[0]);
            f32 z = *(f32*)&ScriptParams[3];
            f32 waterZ = 0.0f;
            if (z <= -100.0f)
            {
                CWaterLevel::GetWaterLevel(*(f32*)&ScriptParams[1], *(f32*)&ScriptParams[2], z, &waterZ, true, nullptr);
            }
            pVehicle->AutoPilot.SetMission(MISSION_GOTOCOORDINATES_ASTHECROWSWIMS);
            pVehicle->AutoPilot.TargetCoors = CVector(*(f32*)&ScriptParams[1], *(f32*)&ScriptParams[2], waterZ);
            pVehicle->SetStatus(STATUS_PHYSICS);
            pVehicle->SetEngineOn(true);
            pVehicle->AutoPilot.CruiseSpeed = MAX(pVehicle->AutoPilot.CruiseSpeed, 1);
            pVehicle->AutoPilot.LastTimeNotStuck = CTimer::GetTimeInMilliseconds();
            return OR_CONTINUE;
        }

        case COMMAND_BOAT_STOP:
        {
            CollectParameters(1);
            pVehicle = CPools::GetVehiclePool().GetAt(ScriptParams[0]);
            pVehicle->AutoPilot.SetMission(MISSION_NONE);
            pVehicle->vehicleFlags.bEngineOn = false;
            pVehicle->SetStatus(STATUS_PHYSICS);
            pVehicle->AutoPilot.CruiseSpeed = 0;
            return OR_CONTINUE;
        }

        case COMMAND_IS_CHAR_SHOOTING_IN_AREA:
        {
            CollectParameters(6);
            pPed = CPools::GetPedPool().GetAt(ScriptParams[0]);
            bool bResult = false;
            if (pPed->bFiringWeapon)
            {
                bResult = pPed->IsWithinArea(*(f32*)&ScriptParams[1], *(f32*)&ScriptParams[2], *(f32*)&ScriptParams[3], *(f32*)&ScriptParams[4]);
            }
            UpdateCompareFlag(bResult);
            if (ScriptParams[5])
            {
                CTheScripts::HighlightImportantArea(((u32)this + (u32)PCPointer), *(f32*)&ScriptParams[1], *(f32*)&ScriptParams[2], *(f32*)&ScriptParams[3], *(f32*)&ScriptParams[4], -100.0f);
            }
            if (CTheScripts::DbgFlag)
            {
                CTheScripts::DrawDebugSquare(*(f32*)&ScriptParams[1], *(f32*)&ScriptParams[2], *(f32*)&ScriptParams[3], *(f32*)&ScriptParams[4]);
            }
            return OR_CONTINUE;
        }

        case COMMAND_IS_CURRENT_CHAR_WEAPON:
        {
            CollectParameters(2);
            pPed = CPools::GetPedPool().GetAt(ScriptParams[0]);
            bool bResult = false;
            if (ScriptParams[1] == WEAPONTYPE_ANYMELEE && pPed->GetWeapon()->IsTypeMelee())
            {
                bResult = true;
            }
            else if (pPed->GetWeapon()->GetWeaponType() == (eWeaponType)ScriptParams[1])
            {
                bResult = true;
            }
            UpdateCompareFlag(bResult);
            return OR_CONTINUE;
        }

        case COMMAND_SET_BOAT_CRUISE_SPEED:
        {
            CollectParameters(2);
            pVehicle = CPools::GetVehiclePool().GetAt(ScriptParams[0]);
            pVehicle->AutoPilot.CruiseSpeed = (u8)ScriptParams[1];
            return OR_CONTINUE;
        }

        case COMMAND_GET_RANDOM_CHAR_IN_ZONE:
        {
            char ptr_str[12];
            ReadTextLabelFromScript(ptr_str, 8);
            u32 zone_name[2] = {0, 0};
            for (i32 i = 0; ptr_str[i]; ++i)
            {
                ((char*)zone_name)[i] = ptr_str[i];
            }
            CollectParameters(3);
            i32 foundPedId = -1;
            f32 playerZ = FindPlayerCoors(-1).z;
            for (i32 i = CPools::GetPedPool().GetSize() - 1; i >= 0; --i)
            {
                if (foundPedId != -1)
                {
                    break;
                }
                CPed* curr_ped = CPools::GetPedPool().GetSlot(i);
                if (curr_ped)
                {
                    if (CPools::GetPedPool().GetIndex(curr_ped) != CTheScripts::LastRandomPedId && curr_ped->CharCreatedBy == RANDOM_CHAR && !curr_ped->m_bRemoveFromWorld && !curr_ped->bFadeOut)
                    {
                        if (!IsPedDead(curr_ped) && !curr_ped->m_pMyVehicle && !curr_ped->bInVehicle)
                        {
                            if (ThisIsAValidRandomPed(curr_ped->m_nPedType, ScriptParams[0], ScriptParams[1], ScriptParams[2]))
                            {
                                if (!CPedGroups::GetPedsGroup(curr_ped))
                                {
                                    CVector pos = curr_ped->GetPosition();
                                    if (CTheZones::DoesPointLieWithinZoneWithGivenName(&pos, *(u64*)zone_name, ZONE_NAVIGATION))
                                    {
                                        if (playerZ - 5.0f <= pos.z && playerZ + 5.0f >= pos.z)
                                        {
                                            bool bRoofFound = false;
                                            CWorld::FindRoofZFor3DCoord(pos.x, pos.y, pos.z, &bRoofFound);
                                            if (!bRoofFound)
                                            {
                                                foundPedId = CPools::GetPedPool().GetIndex(curr_ped);
                                                CTheScripts::LastRandomPedId = foundPedId;
                                                curr_ped->SetCharCreatedBy(MISSION_CHAR);
                                                ++CPopulation::ms_nTotalMissionPeds;
                                                if (this->IsThisAMissionScript)
                                                {
                                                    CTheScripts::MissionCleanUp.AddEntityToList(foundPedId, CLEANUP_CHAR);
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
            ScriptParams[0] = foundPedId;
            StoreParameters(1);
            return OR_CONTINUE;
        }

        case COMMAND_IS_CHAR_SHOOTING:
        {
            CollectParameters(1);
            pPed = CPools::GetPedPool().GetAt(ScriptParams[0]);
            UpdateCompareFlag(pPed->bFiringWeapon);
            return OR_CONTINUE;
        }

        case COMMAND_CREATE_MONEY_PICKUP:
        {
            CollectParameters(5);
            f32 z = *(f32*)&ScriptParams[2];
            if (z <= -100.0f)
            {
                z = CWorld::FindGroundZForCoord(*(f32*)&ScriptParams[0], *(f32*)&ScriptParams[1]) + 0.5f;
            }
            CollectNextParameterWithoutIncreasingPC();
            ScriptParams[0] = CPickups::GenerateNewOne(CVector(*(f32*)&ScriptParams[0], *(f32*)&ScriptParams[1], z), MI_MONEY, ScriptParams[4] ? PICKUP_MONEY_DOESNTDISAPPEAR : PICKUP_MONEY, ScriptParams[3], 0, false, nullptr);
            StoreParameters(1);
            return OR_CONTINUE;
        }

        case COMMAND_SET_CHAR_ACCURACY:
        {
            CollectParameters(2);
            pPed = CPools::GetPedPool().GetAt(ScriptParams[0]);
            pPed->SetShootingAccuracy((u8)ScriptParams[1]);
            return OR_CONTINUE;
        }

        case COMMAND_GET_CAR_SPEED:
        {
            CollectParameters(1);
            pVehicle = CPools::GetVehiclePool().GetAt(ScriptParams[0]);
            *(f32*)&ScriptParams[0] = pVehicle->m_vecMoveSpeed.Magnitude() * 50.0f;
            StoreParameters(1);
            return OR_CONTINUE;
        }

        case COMMAND_LOAD_CUTSCENE:
        {
            char v109[16];
            ReadTextLabelFromScript(v109, 8);
            CCutsceneMgr::LoadCutsceneData(v109);
            return OR_CONTINUE;
        }

        case COMMAND_CREATE_CUTSCENE_OBJECT:
        case COMMAND_SET_CUTSCENE_ANIM:
        case COMMAND_LOAD_SPECIAL_MODEL:
        case COMMAND_SET_CHAR_RUNNING:
        case COMMAND_CREATE_COLLECTABLE1:
            return OR_CONTINUE;

        case COMMAND_START_CUTSCENE:
        {
            CCutsceneMgr::StartCutscene();
            return OR_CONTINUE;
        }

        case COMMAND_GET_CUTSCENE_TIME:
        {
            ScriptParams[0] = CCutsceneMgr::GetCutsceneTimeInMilleseconds();
            StoreParameters(1);
            return OR_CONTINUE;
        }

        case COMMAND_HAS_CUTSCENE_FINISHED:
        {
            UpdateCompareFlag(CCutsceneMgr::HasCutsceneFinished());
            return OR_CONTINUE;
        }

        case COMMAND_CLEAR_CUTSCENE:
        {
            CCutsceneMgr::DeleteCutsceneData();
            return OR_CONTINUE;
        }

        case COMMAND_RESTORE_CAMERA_JUMPCUT:
        {
            TheCamera.RestoreWithJumpCut();
            return OR_CONTINUE;
        }

        case COMMAND_SET_COLLECTABLE1_TOTAL:
        {
            CollectParameters(1);
            CWorld::Players[CWorld::PlayerInFocus].TotalNumCollectables = ScriptParams[0];
            return OR_CONTINUE;
        }

        case COMMAND_IS_PROJECTILE_IN_AREA:
        {
            CollectParameters(6);
            f32 minX = *(f32*)&ScriptParams[0];
            f32 maxX = *(f32*)&ScriptParams[3];
            f32 minY = *(f32*)&ScriptParams[1];
            f32 maxY = *(f32*)&ScriptParams[4];
            f32 minZ = *(f32*)&ScriptParams[2];
            f32 maxZ = *(f32*)&ScriptParams[5];
            if (minX > maxX)
            {
                f32 temp = minX;
                minX = maxX;
                maxX = temp;
            }
            if (minY > maxY)
            {
                f32 temp = minY;
                minY = maxY;
                maxY = temp;
            }
            if (minZ > maxZ)
            {
                f32 temp = minZ;
                minZ = maxZ;
                maxZ = temp;
            }
            UpdateCompareFlag(CProjectileInfo::IsProjectileInRange(minX, maxX, minY, maxY, minZ, maxZ, false));
            return OR_CONTINUE;
        }

        case COMMAND_IS_CHAR_MODEL:
        {
            CollectParameters(2);
            pPed = CPools::GetPedPool().GetAt(ScriptParams[0]);
            UpdateCompareFlag(pPed->m_nModelIndex == ScriptParams[1]);
            return OR_CONTINUE;
        }

        case COMMAND_SIN:
        {
            CollectParameters(1);
            *(f32*)&ScriptParams[0] = sinf(*(f32*)&ScriptParams[0] * 0.017453292f);
            StoreParameters(1);
            return OR_CONTINUE;
        }

        case COMMAND_COS:
        {
            CollectParameters(1);
            *(f32*)&ScriptParams[0] = cosf(*(f32*)&ScriptParams[0] * 0.017453292f);
            StoreParameters(1);
            return OR_CONTINUE;
        }

        case COMMAND_GET_CAR_FORWARD_X:
        {
            CollectParameters(1);
            pVehicle = CPools::GetVehiclePool().GetAt(ScriptParams[0]);
            f32 fwdX = pVehicle->GetMatrix().GetForward().x;
            f32 fwdY = pVehicle->GetMatrix().GetForward().y;
            *(f32*)&ScriptParams[0] = fwdX / sqrtf(fwdY * fwdY + fwdX * fwdX);
            StoreParameters(1);
            return OR_CONTINUE;
        }

        case COMMAND_GET_CAR_FORWARD_Y:
        {
            CollectParameters(1);
            pVehicle = CPools::GetVehiclePool().GetAt(ScriptParams[0]);
            f32 fwdX = pVehicle->GetMatrix().GetForward().x;
            f32 fwdY = pVehicle->GetMatrix().GetForward().y;
            *(f32*)&ScriptParams[0] = fwdY / sqrtf(fwdY * fwdY + fwdX * fwdX);
            StoreParameters(1);
            return OR_CONTINUE;
        }

        case COMMAND_CHANGE_GARAGE_TYPE:
        {
            char name[12];
            ReadTextLabelFromScript(name, 8);
            i16 garageIndex = CGarages::FindGarageIndex(name);
            CollectParameters(1);
            if (garageIndex >= 0)
            {
                CGarages::ChangeGarageType(garageIndex, (u8)ScriptParams[0], 0);
            }
            return OR_CONTINUE;
        }

        case COMMAND_PRINT_WITH_2_NUMBERS_NOW:
        {
            char key[15];
            ReadTextLabelFromScript(key, 8);
            const GxtChar* text = TheText.Get(key);
            CollectParameters(4);
            CMessages::AddMessageJumpQWithNumber(text, ScriptParams[2], ScriptParams[3], ScriptParams[0], ScriptParams[1], -1, -1, -1, -1, CTheScripts::bAddNextMessageToPreviousBriefs);
            CTheScripts::bAddNextMessageToPreviousBriefs = true;
            return OR_CONTINUE;
        }

        case COMMAND_PRINT_WITH_3_NUMBERS:
        {
            char key[15];
            ReadTextLabelFromScript(key, 8);
            const GxtChar* text = TheText.Get(key);
            CollectParameters(5);
            CMessages::AddMessageWithNumber(text, ScriptParams[3], ScriptParams[4], ScriptParams[0], ScriptParams[1], ScriptParams[2], -1, -1, -1, CTheScripts::bAddNextMessageToPreviousBriefs);
            CTheScripts::bAddNextMessageToPreviousBriefs = true;
            return OR_CONTINUE;
        }

        case COMMAND_PRINT_WITH_4_NUMBERS:
        {
            char key[15];
            ReadTextLabelFromScript(key, 8);
            const GxtChar* text = TheText.Get(key);
            CollectParameters(6);
            CMessages::AddMessageWithNumber(text, ScriptParams[4], ScriptParams[5], ScriptParams[0], ScriptParams[1], ScriptParams[2], ScriptParams[3], -1, -1, CTheScripts::bAddNextMessageToPreviousBriefs);
            CTheScripts::bAddNextMessageToPreviousBriefs = true;
            return OR_CONTINUE;
        }

        case COMMAND_PRINT_WITH_4_NUMBERS_NOW:
        {
            char key[15];
            ReadTextLabelFromScript(key, 8);
            const GxtChar* text = TheText.Get(key);
            CollectParameters(6);
            CMessages::AddMessageJumpQWithNumber(text, ScriptParams[4], ScriptParams[5], ScriptParams[0], ScriptParams[1], ScriptParams[2], ScriptParams[3], -1, -1, CTheScripts::bAddNextMessageToPreviousBriefs);
            CTheScripts::bAddNextMessageToPreviousBriefs = true;
            return OR_CONTINUE;
        }

        case COMMAND_PRINT_WITH_6_NUMBERS:
        {
            char key[15];
            ReadTextLabelFromScript(key, 8);
            const GxtChar* text = TheText.Get(key);
            CollectParameters(8);
            CMessages::AddMessageWithNumber(text, ScriptParams[6], ScriptParams[7], ScriptParams[0], ScriptParams[1], ScriptParams[2], ScriptParams[3], ScriptParams[4], ScriptParams[5], CTheScripts::bAddNextMessageToPreviousBriefs);
            CTheScripts::bAddNextMessageToPreviousBriefs = true;
            return OR_CONTINUE;
        }

        case COMMAND_PLAYER_MADE_PROGRESS:
        {
            CollectParameters(1);
            CStats::IncrementStat(PROGRESS_MADE, (f32)ScriptParams[0]);
            return OR_CONTINUE;
        }

        case COMMAND_SET_PROGRESS_TOTAL:
        {
            CollectParameters(1);
            CStats::SetStatValue(TOTAL_PROGRESS, (f32)ScriptParams[0]);
            return OR_CONTINUE;
        }

        case COMMAND_REGISTER_MISSION_GIVEN:
        {
            CStats::IncrementStat(MISSIONS_ATTEMPTED, 1.0f);
            return OR_CONTINUE;
        }

        case COMMAND_REGISTER_MISSION_PASSED:
        {
            char Source[12];
            ReadTextLabelFromScript(Source, 8);
            (void)TheText.Get(Source);
            strncpy(CStats::LastMissionPassedName, Source, 8);
            CStats::IncrementStat(MISSIONS_PASSED, 1.0f);
            f32 kills = CStats::GetStatValue(KILLS_SINCE_LAST_CHECKPOINT);
            CStats::IncrementStat(TOTAL_LEGITIMATE_KILLS, kills);
            CStats::SetStatValue(KILLS_SINCE_LAST_CHECKPOINT, 0.0f);
            CTheScripts::LastMissionPassedTime = CTimer::GetTimeInMilliseconds();
            return OR_CONTINUE;
        }

        case COMMAND_REMOVE_ALL_SCRIPT_FIRES:
        {
            gFireManager.RemoveAllScriptFires();
            return OR_CONTINUE;
        }

        case COMMAND_HAS_CHAR_BEEN_DAMAGED_BY_WEAPON:
        {
            CollectParameters(2);
            pPed = CPools::GetPedPool().GetAt(ScriptParams[0]);
            bool bDamaged = false;
            if (pPed)
            {
                if (ScriptParams[1] == WEAPONTYPE_ANYMELEE || ScriptParams[1] == WEAPONTYPE_ANYWEAPON)
                {
                    if (CDarkel::CheckDamagedWeaponType((eWeaponType)pPed->m_nLastWeaponDamage, (eWeaponType)ScriptParams[1]))
                    {
                        bDamaged = true;
                    }
                }
                else if (pPed->m_nLastWeaponDamage == ScriptParams[1])
                {
                    bDamaged = true;
                }
            }
            UpdateCompareFlag(bDamaged);
            return OR_CONTINUE;
        }

        case COMMAND_HAS_CAR_BEEN_DAMAGED_BY_WEAPON:
        {
            CollectParameters(2);
            pVehicle = CPools::GetVehiclePool().GetAt(ScriptParams[0]);
            bool bDamaged = false;
            if (pVehicle)
            {
                if (ScriptParams[1] != WEAPONTYPE_ANYMELEE && ScriptParams[1] != WEAPONTYPE_ANYWEAPON)
                {
                    if (pVehicle->m_nLastWeaponDamageType == ScriptParams[1])
                    {
                        bDamaged = true;
                    }
                }
                else if (CDarkel::CheckDamagedWeaponType((eWeaponType)pVehicle->m_nLastWeaponDamageType, (eWeaponType)ScriptParams[1]))
                {
                    bDamaged = true;
                }
            }
            UpdateCompareFlag(bDamaged);
            return OR_CONTINUE;
        }

        default:
            return OR_INTERRUPT;
    }
}

// MARK: 800To899

// func: sa 0x481300
OpcodeResult CRunningScript::ProcessCommands800To899(i32 CurrCommand)
{
    MARKFUNCTION(0x481300);

    bool LatestCmpFlagResult;
    CVehicle* pVehicle;
    CPed *pPed, *pSecondPed;
    CObject* pObj;
    CPlayerInfo* pPlayer;
    i32 PlayerSlot;
    CPedGroup* pPedGroup;
    f32 NewZ, NewY, NewX;
    f32 MinZ, MinY, MinX;
    f32 MaxZ, MaxY, MaxX;
    f32 temp_float;
    i16 NumberOfEnts;
    i32 PickupIndex, ActualIndex;
    i16 GarageIndex;
    char GarageName[10];
    const GxtChar* pString;
    char TextLabel[10];
    CVector TempCoors;
    CVector PlayerPos;
    ePedType NewPedType;
    u32 NewPedOccupation;
    u16 Tries;
    i32 RandomCarIndex, CarPoolIndex;
    CRGBA tempRGBA;
    f32 fHeading;
    f32 TargetRotation, RotationStep;
    f32 RotationDifference2, RotationDifference1;
    f32 ZDest, YDest, XDest;
    f32 XDiff, YDiff, ZDiff;
    f32 ZIncrement, YIncrement, XIncrement;
    f32 ZComponent, YComponent, XComponent;
    i32 ModelIndex, ArrayIndex;
    bool ObjectHasCollided;
    CVector BBoxVMin, BBoxVMax;
    CMatrix temp_mat;
    CVector Point1, Point2, Point3, Point4;

    // NOTE: Keep switch cases in strictly ascending order as in commands.hpp to preserve MSVC jump table layout
    switch (CurrCommand)
    {
        case COMMAND_EXPLODE_CHAR_HEAD:
        {
            CollectParameters(1);
            pPed = CPools::GetPedPool().GetAt(ScriptParams[0]);
            CPedDamageResponseCalculator damageResponseCalculator(nullptr, 1000.0f, WEAPONTYPE_SNIPERRIFLE, PED_SPHERE_HEAD, false);
            CEventDamage event(nullptr, CTimer::GetTimeInMilliseconds(), WEAPONTYPE_SNIPERRIFLE, PED_SPHERE_HEAD, 0, false,
                pPed->bInVehicle);
            if (event.AffectsPed(pPed))
            {
                damageResponseCalculator.ComputeDamageResponse(pPed, event.GetDamageResponseData(), true);
            }
            else
            {
                event.GetDamageResponseData().SetDamageCalculated();
            }
            pPed->GetPedIntelligence()->AddEvent(event, false);
            return OR_CONTINUE;
        }
        case COMMAND_ANCHOR_BOAT:
            CollectParameters(2);
            pVehicle = CPools::GetVehiclePool().GetAt(ScriptParams[0]);
            if (ScriptParams[1])
            {
                static_cast<CBoat*>(pVehicle)->m_nBoatFlags.bLockedToXY = true;
            }
            else
            {
                static_cast<CBoat*>(pVehicle)->m_nBoatFlags.bLockedToXY = false;
            }
            return OR_CONTINUE;
        case COMMAND_START_CAR_FIRE:
            CollectParameters(1);
            pVehicle = CPools::GetVehiclePool().GetAt(ScriptParams[0]);
            TempCoors = pVehicle->GetPosition();
            ScriptParams[0] = gFireManager.StartScriptFire(TempCoors, pVehicle, 0.8f, 1, 0, 1);
            StoreParameters(1);
            return OR_CONTINUE;
        case COMMAND_START_CHAR_FIRE:
            CollectParameters(1);
            pPed = CPools::GetPedPool().GetAt(ScriptParams[0]);
            TempCoors = pPed->GetPosition();
            ScriptParams[0] = gFireManager.StartScriptFire(TempCoors, pPed, 0.8f, 1, 0, 1);
            StoreParameters(1);
            return OR_CONTINUE;
        case COMMAND_GET_RANDOM_CAR_OF_TYPE_IN_AREA:
            CollectParameters(5);
            CarPoolIndex = CPools::GetVehiclePool().GetSize();
            RandomCarIndex = -1;
            while (CarPoolIndex-- && RandomCarIndex == -1)
            {
                pVehicle = CPools::GetVehiclePool().GetSlot(CarPoolIndex);
                if (pVehicle &&
                    (pVehicle->GetVehicleAppearance() == APR_CAR || pVehicle->GetVehicleAppearance() == APR_BIKE) &&
                    !pVehicle->vehicleFlags.bIsLawEnforcer && (pVehicle->GetModelIndex() == ScriptParams[4] || ScriptParams[4] < 0) &&
                    pVehicle->CanBeDeleted() &&
                    pVehicle->IsWithinArea(*reinterpret_cast<f32*>(&ScriptParams[0]), *reinterpret_cast<f32*>(&ScriptParams[1]),
                        *reinterpret_cast<f32*>(&ScriptParams[2]), *reinterpret_cast<f32*>(&ScriptParams[3])))
                {
                    RandomCarIndex = CPools::GetVehiclePool().GetIndex(pVehicle);
                    pVehicle->SetVehicleCreatedBy(MISSION_VEHICLE);
                    if (IsThisAMissionScript)
                    {
                        CTheScripts::MissionCleanUp.AddEntityToList(RandomCarIndex, CLEANUP_CAR);
                    }
                }
            }
            ScriptParams[0] = RandomCarIndex;
            StoreParameters(1);
            return OR_CONTINUE;
        case COMMAND_HAS_RESPRAY_HAPPENED:
            return OR_CONTINUE;
        case COMMAND_SET_CAMERA_ZOOM:
            CollectParameters(1);
            if (TheCamera.Cams[TheCamera.ActiveCam].m_nMode == MODE_FOLLOWPED)
            {
                TheCamera.SetZoomValueFollowPedScript(static_cast<i16>(ScriptParams[0]));
            }
            else if (TheCamera.Cams[TheCamera.ActiveCam].m_nMode == MODE_CAM_ON_A_STRING)
            {
                TheCamera.SetZoomValueCamStringScript(static_cast<i16>(ScriptParams[0]));
            }
            return OR_CONTINUE;
        case COMMAND_CREATE_PICKUP_WITH_AMMO:
            CollectParameters(6);
            ModelIndex = ScriptParams[0];
            if (ModelIndex < 0)
            {
                ArrayIndex = -ModelIndex;
                ModelIndex = CTheScripts::UsedObjectArray[ArrayIndex].Index;
            }
            NewX = *reinterpret_cast<f32*>(&ScriptParams[3]);
            NewY = *reinterpret_cast<f32*>(&ScriptParams[4]);
            NewZ = *reinterpret_cast<f32*>(&ScriptParams[5]);
            if (NewZ <= -100.0f)
            {
                NewZ = CWorld::FindGroundZForCoord(NewX, NewY) + 0.5f;
            }
            CPickups::GetActualPickupIndex(CollectNextParameterWithoutIncreasingPC());
            ScriptParams[0] = CPickups::GenerateNewOne(CVector(NewX, NewY, NewZ), ModelIndex, static_cast<u8>(ScriptParams[1]), ScriptParams[2], 0,
                false, nullptr);
            StoreParameters(1);
            return OR_CONTINUE;
        case COMMAND_SET_CAR_RAM_CAR:
            return OR_CONTINUE;
        case COMMAND_SET_PLAYER_NEVER_GETS_TIRED:
            CollectParameters(2);
            CWorld::Players[ScriptParams[0]].m_bDoesNotGetTired = ScriptParams[1] != 0;
            return OR_CONTINUE;
        case COMMAND_SET_PLAYER_FAST_RELOAD:
            CollectParameters(2);
            CWorld::Players[ScriptParams[0]].m_bFastReload = ScriptParams[1] != 0;
            return OR_CONTINUE;
        case COMMAND_SET_CHAR_BLEEDING:
            CollectParameters(2);
            pPed = CPools::GetPedPool().GetAt(ScriptParams[0]);
            if (ScriptParams[1])
            {
                pPed->bPedIsBleeding = true;
            }
            else
            {
                pPed->bPedIsBleeding = false;
            }
            return OR_CONTINUE;
        case COMMAND_SET_FREE_RESPRAYS:
            CollectParameters(1);
            CGarages::RespraysAreFree = ScriptParams[0] != 0;
            return OR_CONTINUE;
        case COMMAND_SET_CHAR_VISIBLE:
            CollectParameters(2);
            pPed = CPools::GetPedPool().GetAt(ScriptParams[0]);
            if (pPed == FindPlayerPed())
            {
                gPlayerPedVisible = ScriptParams[1] != 0;
            }
            if (ScriptParams[1])
            {
                pPed->m_bIsVisible = true;
            }
            else
            {
                pPed->m_bIsVisible = false;
            }
            return OR_CONTINUE;
        case COMMAND_SET_CAR_VISIBLE:
            CollectParameters(2);
            pVehicle = CPools::GetVehiclePool().GetAt(ScriptParams[0]);
            if (ScriptParams[1])
            {
                pVehicle->m_bIsVisible = true;
            }
            else
            {
                pVehicle->m_bIsVisible = false;
            }
            return OR_CONTINUE;
        case COMMAND_IS_AREA_OCCUPIED:
            CollectParameters(11);
            MinX = *reinterpret_cast<f32*>(&ScriptParams[0]);
            MinY = *reinterpret_cast<f32*>(&ScriptParams[1]);
            MinZ = *reinterpret_cast<f32*>(&ScriptParams[2]);
            MaxX = *reinterpret_cast<f32*>(&ScriptParams[3]);
            MaxY = *reinterpret_cast<f32*>(&ScriptParams[4]);
            MaxZ = *reinterpret_cast<f32*>(&ScriptParams[5]);
            if (MinX > MaxX)
            {
                temp_float = MinX;
                MinX = MaxX;
                MaxX = temp_float;
            }
            if (MinY > MaxY)
            {
                temp_float = MinY;
                MinY = MaxY;
                MaxY = temp_float;
            }
            if (MinZ > MaxZ)
            {
                temp_float = MinZ;
                MinZ = MaxZ;
                MaxZ = temp_float;
            }
            LatestCmpFlagResult = false;
            CWorld::FindObjectsIntersectingCube(CVector(MinX, MinY, MinZ), CVector(MaxX, MaxY, MaxZ), &NumberOfEnts, 2, nullptr,
                ScriptParams[6] != 0, ScriptParams[7] != 0, ScriptParams[8] != 0, ScriptParams[9] != 0, ScriptParams[10] != 0);
            if (NumberOfEnts > 0)
            {
                LatestCmpFlagResult = true;
            }
            UpdateCompareFlag(LatestCmpFlagResult);
            return OR_CONTINUE;
        case COMMAND_DISPLAY_TEXT:
        {
            CollectParameters(2);
            ReadTextLabelFromScript(TextLabel, 8);
            pString = TheText.Get(TextLabel);
            f32 x = *reinterpret_cast<f32*>(&ScriptParams[0]);
            f32 y = *reinterpret_cast<f32*>(&ScriptParams[1]);
            CTheScripts::IntroTextLines[CTheScripts::NumberOfIntroTextLinesThisFrame].ScriptTextAtX = x;
            CTheScripts::IntroTextLines[CTheScripts::NumberOfIntroTextLinesThisFrame].ScriptTextAtY = y;
            strncpy(CTheScripts::IntroTextLines[CTheScripts::NumberOfIntroTextLinesThisFrame].ScriptTextLabel, TextLabel, 8);
            CTheScripts::IntroTextLines[CTheScripts::NumberOfIntroTextLinesThisFrame].NumberToInsert1 = -1;
            CTheScripts::IntroTextLines[CTheScripts::NumberOfIntroTextLinesThisFrame].NumberToInsert2 = -1;
            CTheScripts::NumberOfIntroTextLinesThisFrame++;
            return OR_CONTINUE;
        }
        case COMMAND_SET_TEXT_SCALE:
            CollectParameters(2);
            CTheScripts::IntroTextLines[CTheScripts::NumberOfIntroTextLinesThisFrame].ScriptTextXScale = *reinterpret_cast<f32*>(&ScriptParams[0]);
            CTheScripts::IntroTextLines[CTheScripts::NumberOfIntroTextLinesThisFrame].ScriptTextYScale = *reinterpret_cast<f32*>(&ScriptParams[1]);
            return OR_CONTINUE;
        case COMMAND_SET_TEXT_COLOUR:
            CollectParameters(4);
            tempRGBA = CRGBA(static_cast<u8>(ScriptParams[0]), static_cast<u8>(ScriptParams[1]), static_cast<u8>(ScriptParams[2]),
                static_cast<u8>(ScriptParams[3]));
            CTheScripts::IntroTextLines[CTheScripts::NumberOfIntroTextLinesThisFrame].ScriptTextColor = tempRGBA;
            return OR_CONTINUE;
        case COMMAND_SET_TEXT_JUSTIFY:
            CollectParameters(1);
            if (ScriptParams[0])
            {
                CTheScripts::IntroTextLines[CTheScripts::NumberOfIntroTextLinesThisFrame].ScriptTextJustify = true;
            }
            else
            {
                CTheScripts::IntroTextLines[CTheScripts::NumberOfIntroTextLinesThisFrame].ScriptTextJustify = false;
            }
            return OR_CONTINUE;
        case COMMAND_SET_TEXT_CENTRE:
            CollectParameters(1);
            if (ScriptParams[0])
            {
                CTheScripts::IntroTextLines[CTheScripts::NumberOfIntroTextLinesThisFrame].ScriptTextCentre = true;
            }
            else
            {
                CTheScripts::IntroTextLines[CTheScripts::NumberOfIntroTextLinesThisFrame].ScriptTextCentre = false;
            }
            return OR_CONTINUE;
        case COMMAND_SET_TEXT_WRAPX:
            CollectParameters(1);
            CTheScripts::IntroTextLines[CTheScripts::NumberOfIntroTextLinesThisFrame].ScriptTextWrapX = *reinterpret_cast<f32*>(&ScriptParams[0]);
            return OR_CONTINUE;
        case COMMAND_SET_TEXT_CENTRE_SIZE:
            CollectParameters(1);
            CTheScripts::IntroTextLines[CTheScripts::NumberOfIntroTextLinesThisFrame].ScriptTextCentreSize = *reinterpret_cast<f32*>(&ScriptParams[0]);
            return OR_CONTINUE;
        case COMMAND_SET_TEXT_BACKGROUND:
            CollectParameters(1);
            if (ScriptParams[0])
            {
                CTheScripts::IntroTextLines[CTheScripts::NumberOfIntroTextLinesThisFrame].ScriptTextBackgrnd = true;
            }
            else
            {
                CTheScripts::IntroTextLines[CTheScripts::NumberOfIntroTextLinesThisFrame].ScriptTextBackgrnd = false;
            }
            return OR_CONTINUE;
        case COMMAND_SET_TEXT_PROPORTIONAL:
            CollectParameters(1);
            if (ScriptParams[0])
            {
                CTheScripts::IntroTextLines[CTheScripts::NumberOfIntroTextLinesThisFrame].ScriptTextProportional = true;
            }
            else
            {
                CTheScripts::IntroTextLines[CTheScripts::NumberOfIntroTextLinesThisFrame].ScriptTextProportional = false;
            }
            return OR_CONTINUE;
        case COMMAND_SET_TEXT_FONT:
            CollectParameters(1);
            CTheScripts::IntroTextLines[CTheScripts::NumberOfIntroTextLinesThisFrame].ScriptTextFontStyle = static_cast<eFontStyle>(ScriptParams[0]);
            return OR_CONTINUE;
        case COMMAND_ROTATE_OBJECT:
            CollectParameters(4);
            pObj = CPools::GetObjectPool().GetAt(ScriptParams[0]);
            fHeading = RADTODEG(pObj->GetHeading());
            if (fHeading < 0.0f)
            {
                fHeading += 360.0f;
            }
            TargetRotation = *reinterpret_cast<f32*>(&ScriptParams[1]);
            RotationStep = *reinterpret_cast<f32*>(&ScriptParams[2]);
            LatestCmpFlagResult = false;
            if (fHeading == TargetRotation)
            {
                LatestCmpFlagResult = true;
            }
            else
            {
                RotationDifference1 = TargetRotation - fHeading;
                RotationDifference2 = fHeading - TargetRotation;
                if (RotationDifference1 < 0.0f)
                {
                    RotationDifference1 += 360.0f;
                }
                if (RotationDifference2 < 0.0f)
                {
                    RotationDifference2 += 360.0f;
                }
                if (RotationDifference1 < RotationDifference2)
                {
                    if (RotationDifference1 < RotationStep)
                    {
                        fHeading = TargetRotation;
                    }
                    else
                    {
                        fHeading += RotationStep;
                    }
                }
                else
                {
                    if (RotationDifference2 < RotationStep)
                    {
                        fHeading = TargetRotation;
                    }
                    else
                    {
                        fHeading -= RotationStep;
                    }
                }
                ObjectHasCollided = false;
                if (ScriptParams[3])
                {
                    TempCoors = pObj->GetPosition();
                    temp_mat.SetRotateZ(DEGTORAD(fHeading));
                    temp_mat.Translate(TempCoors);
                    BBoxVMin = CModelInfo::GetColModel(pObj->GetModelIndex()).GetBoundBox().m_vecMin;
                    BBoxVMax = CModelInfo::GetColModel(pObj->GetModelIndex()).GetBoundBox().m_vecMax;
                    Point1 = BBoxVMin;
                    Point2 = CVector(BBoxVMax.x, BBoxVMin.y, BBoxVMax.z);
                    Point3 = CVector(BBoxVMax.x, BBoxVMax.y, BBoxVMin.z);
                    Point4 = CVector(BBoxVMin.x, BBoxVMax.y, BBoxVMax.z);
                    Point1 = temp_mat * Point1;
                    Point2 = temp_mat * Point2;
                    Point3 = temp_mat * Point3;
                    Point4 = temp_mat * Point4;
                    MinX = MIN(Point4.x, MIN(Point3.x, MIN(Point2.x, Point1.x)));
                    MaxX = MAX(Point4.x, MAX(Point3.x, MAX(Point2.x, Point1.x)));
                    MinY = MIN(Point4.y, MIN(Point3.y, MIN(Point2.y, Point1.y)));
                    MaxY = MAX(Point4.y, MAX(Point3.y, MAX(Point2.y, Point1.y)));
                    CWorld::FindObjectsIntersectingAngledCollisionBox(CModelInfo::GetColModel(pObj->GetModelIndex()).GetBoundBox(), temp_mat, TempCoors,
                        MinX, MinY, MaxX, MaxY, &NumberOfEnts, 2, nullptr, false, true, true, false, false);
                    if (NumberOfEnts > 0)
                    {
                        ObjectHasCollided = true;
                    }
                }
                if (ObjectHasCollided)
                {
                    LatestCmpFlagResult = true;
                }
                else
                {
                    pObj->SetHeading(DEGTORAD(fHeading));
                    pObj->UpdateRwMatrix();
                    pObj->UpdateRwFrame();
                    if (fHeading == TargetRotation)
                    {
                        LatestCmpFlagResult = true;
                    }
                }
            }
            UpdateCompareFlag(LatestCmpFlagResult);
            return OR_CONTINUE;
        case COMMAND_SLIDE_OBJECT:
            CollectParameters(8);
            pObj = CPools::GetObjectPool().GetAt(ScriptParams[0]);
            TempCoors = pObj->GetPosition();
            XDest = *reinterpret_cast<f32*>(&ScriptParams[1]);
            YDest = *reinterpret_cast<f32*>(&ScriptParams[2]);
            ZDest = *reinterpret_cast<f32*>(&ScriptParams[3]);
            if (TempCoors.x == XDest && TempCoors.y == YDest && TempCoors.z == ZDest)
            {
                LatestCmpFlagResult = true;
                UpdateCompareFlag(LatestCmpFlagResult);
                return OR_CONTINUE;
            }
            XIncrement = *reinterpret_cast<f32*>(&ScriptParams[4]);
            YIncrement = *reinterpret_cast<f32*>(&ScriptParams[5]);
            ZIncrement = *reinterpret_cast<f32*>(&ScriptParams[6]);
            XDiff = TempCoors.x - XDest;
            YDiff = TempCoors.y - YDest;
            ZDiff = TempCoors.z - ZDest;
            if (XDiff > 0.0f)
            {
                if (XDiff > XIncrement)
                {
                    TempCoors.x -= XIncrement;
                }
                else
                {
                    TempCoors.x = XDest;
                }
            }
            else
            {
                if (-XDiff > XIncrement)
                {
                    TempCoors.x += XIncrement;
                }
                else
                {
                    TempCoors.x = XDest;
                }
            }
            if (YDiff > 0.0f)
            {
                if (YDiff > YIncrement)
                {
                    TempCoors.y -= YIncrement;
                }
                else
                {
                    TempCoors.y = YDest;
                }
            }
            else
            {
                if (-YDiff > YIncrement)
                {
                    TempCoors.y += YIncrement;
                }
                else
                {
                    TempCoors.y = YDest;
                }
            }
            if (ZDiff > 0.0f)
            {
                if (ZDiff > ZIncrement)
                {
                    TempCoors.z -= ZIncrement;
                }
                else
                {
                    TempCoors.z = ZDest;
                }
            }
            else
            {
                if (-ZDiff > ZIncrement)
                {
                    TempCoors.z += ZIncrement;
                }
                else
                {
                    TempCoors.z = ZDest;
                }
            }
            ObjectHasCollided = false;
            if (ScriptParams[7])
            {
                temp_mat = pObj->GetMatrix();
                temp_mat.GetTranslate() = TempCoors;
                BBoxVMin = CModelInfo::GetColModel(pObj->GetModelIndex()).GetBoundBox().m_vecMin;
                BBoxVMax = CModelInfo::GetColModel(pObj->GetModelIndex()).GetBoundBox().m_vecMax;
                Point1 = BBoxVMin;
                Point2 = CVector(BBoxVMax.x, BBoxVMin.y, BBoxVMax.z);
                Point3 = CVector(BBoxVMax.x, BBoxVMax.y, BBoxVMin.z);
                Point4 = CVector(BBoxVMin.x, BBoxVMax.y, BBoxVMax.z);
                Point1 = temp_mat * Point1;
                Point2 = temp_mat * Point2;
                Point3 = temp_mat * Point3;
                Point4 = temp_mat * Point4;
                MinX = MIN(Point4.x, MIN(Point3.x, MIN(Point2.x, Point1.x)));
                MaxX = MAX(Point4.x, MAX(Point3.x, MAX(Point2.x, Point1.x)));
                MinY = MIN(Point4.y, MIN(Point3.y, MIN(Point2.y, Point1.y)));
                MaxY = MAX(Point4.y, MAX(Point3.y, MAX(Point2.y, Point1.y)));
                CWorld::FindObjectsIntersectingAngledCollisionBox(CModelInfo::GetColModel(pObj->GetModelIndex()).GetBoundBox(), temp_mat, TempCoors, MinX,
                    MinY, MaxX, MaxY, &NumberOfEnts, 2, nullptr, false, true, true, false, false);
                if (NumberOfEnts > 0)
                {
                    ObjectHasCollided = true;
                }
            }
            if (ObjectHasCollided)
            {
                LatestCmpFlagResult = true;
                UpdateCompareFlag(LatestCmpFlagResult);
                return OR_CONTINUE;
            }
            pObj->Teleport(TempCoors, false);
            if (TempCoors.x == XDest && TempCoors.y == YDest && TempCoors.z == ZDest)
            {
                LatestCmpFlagResult = true;
            }
            else
            {
                LatestCmpFlagResult = false;
            }
            UpdateCompareFlag(LatestCmpFlagResult);
            return OR_CONTINUE;
        case COMMAND_REMOVE_CHAR_ELEGANTLY:
            CollectParameters(1);
            pPed = CPools::GetPedPool().GetAt(ScriptParams[0]);
            if (pPed && pPed->GetCharCreatedBy() == MISSION_CHAR)
            {
                if (pPed->bInVehicle && pPed->m_pMyVehicle)
                {
                    CTheScripts::RemoveThisPed(pPed);
                }
                else
                {
                    pPed->SetCharCreatedBy(RANDOM_CHAR);
                    pPedGroup = CPedGroups::GetPedsGroup(pPed);
                    if (pPedGroup && pPedGroup->GetGroupMembership()->IsFollower(pPed))
                    {
                        pPedGroup->GetGroupMembership()->RemoveMember(pPed);
                    }
                    CPopulation::ms_nTotalMissionPeds--;
                    pPed->bFadeOut = true;
                    CWorld::RemoveReferencesToDeletedObject(pPed);
                }
            }
            if (IsThisAMissionScript)
            {
                CTheScripts::MissionCleanUp.RemoveEntityFromList(ScriptParams[0], CLEANUP_CHAR);
            }
            return OR_CONTINUE;
        case COMMAND_SET_CHAR_STAY_IN_SAME_PLACE:
            CollectParameters(2);
            pPed = CPools::GetPedPool().GetAt(ScriptParams[0]);
            if (ScriptParams[1])
            {
                pPed->SetStayInSamePlaceFlag(true);
            }
            else
            {
                pPed->SetStayInSamePlaceFlag(false);
            }
            return OR_CONTINUE;
        case COMMAND_IS_EXPLOSION_IN_AREA:
            CollectParameters(7);
            MinX = *reinterpret_cast<f32*>(&ScriptParams[1]);
            MinY = *reinterpret_cast<f32*>(&ScriptParams[2]);
            MinZ = *reinterpret_cast<f32*>(&ScriptParams[3]);
            MaxX = *reinterpret_cast<f32*>(&ScriptParams[4]);
            MaxY = *reinterpret_cast<f32*>(&ScriptParams[5]);
            MaxZ = *reinterpret_cast<f32*>(&ScriptParams[6]);
            if (MinX > MaxX)
            {
                temp_float = MinX;
                MinX = MaxX;
                MaxX = temp_float;
            }
            if (MinY > MaxY)
            {
                temp_float = MinY;
                MinY = MaxY;
                MaxY = temp_float;
            }
            if (MinZ > MaxZ)
            {
                temp_float = MinZ;
                MinZ = MaxZ;
                MaxZ = temp_float;
            }
            LatestCmpFlagResult = false;
            if (CExplosion::TestForExplosionInArea(static_cast<eExplosionType>(ScriptParams[0]), MinX, MaxX, MinY, MaxY, MinZ, MaxZ))
            {
                LatestCmpFlagResult = true;
            }
            UpdateCompareFlag(LatestCmpFlagResult);
            return OR_CONTINUE;
        case COMMAND_PLACE_OBJECT_RELATIVE_TO_CAR:
            CollectParameters(5);
            pObj = CPools::GetObjectPool().GetAt(ScriptParams[0]);
            pVehicle = CPools::GetVehiclePool().GetAt(ScriptParams[1]);
            CPhysical::PlacePhysicalRelativeToOtherPhysical(pVehicle, pObj,
                CVector(*reinterpret_cast<f32*>(&ScriptParams[2]), *reinterpret_cast<f32*>(&ScriptParams[3]), *reinterpret_cast<f32*>(&ScriptParams[4])));
            return OR_CONTINUE;
        case COMMAND_MAKE_OBJECT_TARGETTABLE:
        {
            CollectParameters(2);
            pObj = CPools::GetObjectPool().GetAt(ScriptParams[0]);
            CPlayerPed* pPlayerPed = CWorld::Players[CWorld::PlayerInFocus].m_pPed;
            if (ScriptParams[1])
            {
                pObj->SetObjectTargettable(true);
            }
            else
            {
                pObj->SetObjectTargettable(false);
                if (pPlayerPed->GetWeaponLockOnTarget() == pObj)
                {
                    pPlayerPed->SetWeaponLockOnTarget(nullptr);
                }
            }
            return OR_CONTINUE;
        }
        case COMMAND_ADD_ARMOUR_TO_CHAR:
            CollectParameters(2);
            pPed = CPools::GetPedPool().GetAt(ScriptParams[0]);
            pPed->m_nArmour += ScriptParams[1];
            if (pPed->IsPlayer())
            {
                pPed->m_nArmour = MIN(pPed->m_nArmour, static_cast<f32>(CWorld::Players[CWorld::FindPlayerSlotWithPedPointer(pPed)].MaxArmour));
            }
            else
            {
                pPed->m_nArmour = MIN(pPed->m_nArmour, 100.0f);
            }
            pPed->m_nArmour = MAX(pPed->m_nArmour, 0.0f);
            return OR_CONTINUE;
        case COMMAND_OPEN_GARAGE:
            ReadTextLabelFromScript(GarageName, 8);
            GarageIndex = CGarages::FindGarageIndex(GarageName);
            if (GarageIndex >= 0)
            {
                CGarages::aGarages[GarageIndex].OpenThisGarage();
            }
            return OR_CONTINUE;
        case COMMAND_CLOSE_GARAGE:
            ReadTextLabelFromScript(GarageName, 8);
            GarageIndex = CGarages::FindGarageIndex(GarageName);
            if (GarageIndex >= 0)
            {
                CGarages::aGarages[GarageIndex].CloseThisGarage();
            }
            return OR_CONTINUE;
        case COMMAND_WARP_CHAR_FROM_CAR_TO_COORD:
            CollectParameters(4);
            pPed = CPools::GetPedPool().GetAt(ScriptParams[0]);
            NewX = *reinterpret_cast<f32*>(&ScriptParams[1]);
            NewY = *reinterpret_cast<f32*>(&ScriptParams[2]);
            NewZ = *reinterpret_cast<f32*>(&ScriptParams[3]);
            if (NewZ <= -100.0f)
            {
                NewZ = CWorld::FindGroundZForCoord(NewX, NewY);
            }
            pPed->GetPedIntelligence()->FlushImmediately(true);
            NewZ += pPed->GetDistanceFromCentreOfMassToBaseOfModel();
            pPed->Teleport(CVector(NewX, NewY, NewZ), false);
            CTheScripts::ClearSpaceForMissionEntity(CVector(NewX, NewY, NewZ), pPed);
            return OR_CONTINUE;
        case COMMAND_SET_VISIBILITY_OF_CLOSEST_OBJECT_OF_TYPE:
        {
            i16 NumOfEntities;
            CEntity* pEntityArray[16];
            CEntity* pClosestObj;
            f32 ClosestDist;
            CVector ObjPos, DiffVector;
            f32 Radius;
            u16 loop = 0;
            f32 ObjDistance;

            CollectParameters(6);
            NewX = *reinterpret_cast<f32*>(&ScriptParams[0]);
            NewY = *reinterpret_cast<f32*>(&ScriptParams[1]);
            NewZ = *reinterpret_cast<f32*>(&ScriptParams[2]);
            if (NewZ <= -100.0f)
            {
                NewZ = CWorld::FindGroundZForCoord(NewX, NewY);
            }
            ModelIndex = ScriptParams[4];
            Radius = *reinterpret_cast<f32*>(&ScriptParams[3]);
            pClosestObj = nullptr;
            ClosestDist = Radius * 2.0f;
            if (ModelIndex < 0)
            {
                ArrayIndex = -ModelIndex;
                ModelIndex = CTheScripts::UsedObjectArray[ArrayIndex].Index;
            }
            CWorld::FindObjectsOfTypeInRange(ModelIndex, CVector(NewX, NewY, NewZ), Radius, true, &NumOfEntities, 16, pEntityArray, true, false, false, true, true);
            if (NumOfEntities == 0)
            {
                CWorld::FindLodOfTypeInRange(ModelIndex, CVector(NewX, NewY, NewZ), Radius, true, &NumOfEntities, 16, pEntityArray);
            }
            for (loop = 0; loop < NumOfEntities; loop++)
            {
                ObjPos = pEntityArray[loop]->GetPosition();
                ObjDistance = (ObjPos - CVector(NewX, NewY, NewZ)).Magnitude();
                if (ObjDistance < ClosestDist)
                {
                    ClosestDist = ObjDistance;
                    pClosestObj = pEntityArray[loop];
                }
            }
            if (pClosestObj)
            {
                if (ScriptParams[5])
                {
                    pClosestObj->m_bIsVisible = true;
                    pClosestObj->m_bUsesCollision = true;
                    if (pClosestObj->m_iplIndex == 0)
                    {
                        loop = 0;
                        ObjectHasCollided = false;
                        while (!ObjectHasCollided && loop < MAX_NUM_INVISIBILITY_SETTINGS)
                        {
                            if (CTheScripts::InvisibilitySettingArray[loop] == pClosestObj)
                            {
                                ObjectHasCollided = true;
                            }
                            else
                            {
                                loop++;
                            }
                        }
                        if (ObjectHasCollided)
                        {
                            CTheScripts::InvisibilitySettingArray[loop] = nullptr;
                        }
                    }
                }
                else
                {
                    pClosestObj->m_bIsVisible = false;
                    pClosestObj->m_bUsesCollision = false;
                    CTheScripts::AddToInvisibilitySwapArray(pClosestObj, false);
                }
            }
            return OR_CONTINUE;
        }
        case COMMAND_HAS_CHAR_SPOTTED_CHAR:
            CollectParameters(2);
            pPed = CPools::GetPedPool().GetAt(ScriptParams[0]);
            pSecondPed = CPools::GetPedPool().GetAt(ScriptParams[1]);
            LatestCmpFlagResult = pPed->OurPedCanSeeThisEntity(pSecondPed, true) != false;
            UpdateCompareFlag(LatestCmpFlagResult);
            return OR_CONTINUE;
        case COMMAND_HAS_OBJECT_BEEN_DAMAGED:
            CollectParameters(1);
            pObj = CPools::GetObjectPool().GetAt(ScriptParams[0]);
            LatestCmpFlagResult = pObj->IsObjectDamaged();
            UpdateCompareFlag(LatestCmpFlagResult);
            return OR_CONTINUE;
        case COMMAND_WARP_CHAR_INTO_CAR:
        {
            CollectParameters(2);
            const i32 iPedID = ScriptParams[0];
            const i32 iVehicleID = ScriptParams[1];
            pPed = CPools::GetPedPool().GetAt(iPedID);
            pVehicle = CPools::GetVehiclePool().GetAt(iVehicleID);
            pPed->GetPedIntelligence()->FlushImmediately(false);
            CTaskSimpleCarSetPedInAsDriver task(pVehicle, nullptr);
            
            task.ProcessPed(pPed);
            return OR_CONTINUE;
        }
        case COMMAND_PRINT_WITH_2_NUMBERS_BIG:
            ReadTextLabelFromScript(TextLabel, 8);
            pString = TheText.Get(TextLabel);
            CollectParameters(4);
            CMessages::AddBigMessageWithNumber(pString, ScriptParams[2], static_cast<u16>(ScriptParams[3] - 1), ScriptParams[0], ScriptParams[1], -1, -1, -1,
                -1);
            return OR_CONTINUE;
        case COMMAND_SET_CAMERA_BEHIND_PLAYER:
            TheCamera.SetCameraDirectlyBehindForFollowPed_CamOnAString();
            return OR_CONTINUE;
        case COMMAND_CREATE_RANDOM_CHAR:
            CollectParameters(3);
            NewPedType = PEDTYPE_COP;
            Tries = 0;
            while (NewPedType != PEDTYPE_CIVMALE && NewPedType != PEDTYPE_CIVFEMALE && Tries < 5)
            {
                NewPedOccupation = CPopulation::ChooseCivilianOccupation(0, 0, -1, -1, 0xFFFFFFFF, 0, 0, 0, 0);
                if (CModelInfo::GetModelInfo(NewPedOccupation)->GetRwObject())
                {
                    NewPedType = static_cast<CPedModelInfo*>(CModelInfo::GetModelInfo(NewPedOccupation))->GetDefaultPedType();
                }
                Tries++;
            }
            if (!CModelInfo::GetModelInfo(NewPedOccupation)->GetRwObject())
            {
                NewPedType = static_cast<CPedModelInfo*>(CModelInfo::GetModelInfo(7))->GetDefaultPedType();
                NewPedOccupation = 7;
            }
            pPed = new CCivilianPed(NewPedType, NewPedOccupation);
            pPed->GetPedIntelligence()->AddTaskDefault(new CTaskSimpleStandStill(999999, true, false, 8.0f), false);
            pPed->SetCharCreatedBy(MISSION_CHAR);
            pPed->bAllowMedicsToReviveMe = false;
            NewX = *reinterpret_cast<f32*>(&ScriptParams[0]);
            NewY = *reinterpret_cast<f32*>(&ScriptParams[1]);
            NewZ = *reinterpret_cast<f32*>(&ScriptParams[2]);
            if (NewZ <= -100.0f)
            {
                NewZ = CWorld::FindGroundZForCoord(NewX, NewY);
            }
            TempCoors = CVector(NewX, NewY, NewZ + 1.0f);
            pPed->SetPosition(TempCoors);
            pPed->SetOrientation(0.0f, 0.0f, 0.0f);
            CTheScripts::ClearSpaceForMissionEntity(TempCoors, pPed);
            if (IsThisAMissionScript)
            {
                pPed->m_bIsStaticWaitingForCollision = true;
            }
            CWorld::Add(pPed);
            CPopulation::ms_nTotalMissionPeds++;
            ScriptParams[0] = CPools::GetPedPool().GetIndex(pPed);
            StoreParameters(1);
            if (IsThisAMissionScript)
            {
                CTheScripts::MissionCleanUp.AddEntityToList(ScriptParams[0], CLEANUP_CHAR);
            }
            return OR_CONTINUE;
        case COMMAND_IS_SNIPER_BULLET_IN_AREA:
            return OR_CONTINUE;
        case COMMAND_SET_OBJECT_VELOCITY:
            CollectParameters(4);
            pObj = CPools::GetObjectPool().GetAt(ScriptParams[0]);
            pObj->SetMoveSpeed(*reinterpret_cast<f32*>(&ScriptParams[1]) * 0.02f, *reinterpret_cast<f32*>(&ScriptParams[2]) * 0.02f,
                *reinterpret_cast<f32*>(&ScriptParams[3]) * 0.02f);
            return OR_CONTINUE;
        case COMMAND_SET_OBJECT_COLLISION:
            CollectParameters(2);
            pObj = CPools::GetObjectPool().GetAt(ScriptParams[0]);
            if (ScriptParams[1])
            {
                pObj->m_bUsesCollision = true;
            }
            else
            {
                pObj->m_bUsesCollision = false;
            }
            return OR_CONTINUE;

        case COMMAND_IS_ICECREAM_JINGLE_ON:
            return OR_CONTINUE;
    }
    return OR_INTERRUPT;
}

// MARK: 900To999

// func: sa 0x483BD0
OpcodeResult CRunningScript::ProcessCommands900To999(i32 CurrCommand)
{
    MARKFUNCTION(0x483BD0);

    bool LatestCmpFlagResult;

    CObject* pObj;
    CPed* pPed;
    CVehicle* pVehicle;

    CPlayerInfo* pPlayer;

    CVector TempCoors;
    CVector Diff;

    CAnimBlendAssociation* pAnim;
    char AnimName[24];

    const GxtChar* pString;
    const GxtChar* pString2;
    char TextLabel[10];

    char FileName[50];
    u8 CharLoop;

    f32 NewZ;
    f32 NewY;
    f32 NewX;

    f32 Radius;
    f32 NewHeading;

    i16 GarageIndex;
    char GarageName[10];

    CNodeAddress ResultNode;

    CVector CurrentObjectSpeed;

    f32 CentreZ;
    f32 CentreX;
    f32 CentreY;
    f32 Depth;
    f32 Height;
    f32 Width;

    f32 MinZ;
    f32 MinX;
    f32 MinY;
    f32 MaxZ;
    f32 MaxX;
    f32 MaxY;
    f32 temp_float;

    i16 NumberOfEnts;

    f32 ZComponent;
    f32 YComponent;
    f32 XComponent;

    i32 ModelIndex;
    i32 ArrayIndex;
    i32 NewModelIndex;
    i32 NewArrayIndex;

    i32 SphereIndex;
    i32 ActualIndex;
    i32 BlipIndex;
    i32 txdIndex;

    switch (CurrCommand)
    {
        case COMMAND_PRINT_STRING_IN_STRING_NOW:
        {
            ReadTextLabelFromScript(TextLabel, 8);
            pString = TheText.Get(TextLabel);
            ReadTextLabelFromScript(TextLabel, 8);
            pString2 = TheText.Get(TextLabel);
            CollectParameters(2);
            CMessages::AddMessageJumpQWithString(pString, ScriptParams[0], (u16)ScriptParams[1], pString2, CTheScripts::bAddNextMessageToPreviousBriefs);
            CTheScripts::bAddNextMessageToPreviousBriefs = true;
            return OR_CONTINUE;
        }
        case COMMAND_IS_POINT_OBSCURED_BY_A_MISSION_ENTITY:
        {
            CollectParameters(6);
            MinX = *(f32*)&ScriptParams[0] - *(f32*)&ScriptParams[3];
            MaxX = *(f32*)&ScriptParams[0] + *(f32*)&ScriptParams[3];
            MinY = *(f32*)&ScriptParams[1] - *(f32*)&ScriptParams[4];
            MaxY = *(f32*)&ScriptParams[1] + *(f32*)&ScriptParams[4];
            MinZ = *(f32*)&ScriptParams[2] - *(f32*)&ScriptParams[5];
            MaxZ = *(f32*)&ScriptParams[2] + *(f32*)&ScriptParams[5];
            if (MinX > MaxX)
            {
                temp_float = MinX;
                MinX = MaxX;
                MaxX = temp_float;
            }
            if (MinY > MaxY)
            {
                temp_float = MinY;
                MinY = MaxY;
                MaxY = temp_float;
            }
            if (MinZ > MaxZ)
            {
                temp_float = MinZ;
                MinZ = MaxZ;
                MaxZ = temp_float;
            }
            CWorld::FindMissionEntitiesIntersectingCube(CVector(MinX, MinY, MinZ), CVector(MaxX, MaxY, MaxZ), &NumberOfEnts, 2, nullptr, true, true, true);
            UpdateCompareFlag(NumberOfEnts > 0);
            return OR_CONTINUE;
        }
        case COMMAND_LOAD_ALL_MODELS_NOW:
        {
            CTimer::Suspend();
            CStreaming::LoadAllRequestedModels(false);
            CTimer::Resume();
            return OR_CONTINUE;
        }
        case COMMAND_ADD_TO_OBJECT_VELOCITY:
        {
            CollectParameters(4);
            pObj = CPools::GetObjectPool().GetAt(ScriptParams[0]);
            pObj->m_vecMoveSpeed.x += *(f32*)&ScriptParams[1] * 0.02f;
            pObj->m_vecMoveSpeed.y += *(f32*)&ScriptParams[2] * 0.02f;
            pObj->m_vecMoveSpeed.z += *(f32*)&ScriptParams[3] * 0.02f;
            return OR_CONTINUE;
        }
        case COMMAND_DRAW_SPRITE:
        {
            CollectParameters(9);
            ActualIndex = CTheScripts::NumberOfIntroRectanglesThisFrame;
            CTheScripts::IntroRectangles[ActualIndex].eWindowType = WINDOW_SPRITE_NO_ROTATION;
            CTheScripts::IntroRectangles[ActualIndex].ScriptSpriteIndex = ScriptParams[0] - 1;
            CTheScripts::IntroRectangles[ActualIndex].ScriptRectMinX = (*(f32*)&ScriptParams[1] - *(f32*)&ScriptParams[3] * 0.5f) * (SCREEN_WIDTH / 640.0f);
            CTheScripts::IntroRectangles[ActualIndex].ScriptRectMinY = (*(f32*)&ScriptParams[2] - *(f32*)&ScriptParams[4] * 0.5f) * (SCREEN_HEIGHT / 448.0f);
            CTheScripts::IntroRectangles[ActualIndex].ScriptRectMaxX = (*(f32*)&ScriptParams[1] + *(f32*)&ScriptParams[3] * 0.5f) * (SCREEN_WIDTH / 640.0f);
            CTheScripts::IntroRectangles[ActualIndex].ScriptRectMaxY = (*(f32*)&ScriptParams[2] + *(f32*)&ScriptParams[4] * 0.5f) * (SCREEN_HEIGHT / 448.0f);
            CTheScripts::IntroRectangles[ActualIndex].ScriptRectRotation = 0.0f;
            CTheScripts::IntroRectangles[ActualIndex].ScriptRectColour = CRGBA((u8)ScriptParams[5], (u8)ScriptParams[6], (u8)ScriptParams[7], (u8)ScriptParams[8]);
            CTheScripts::IntroRectangles[ActualIndex].pTitle[0] = 0;
            ++CTheScripts::NumberOfIntroRectanglesThisFrame;
            return OR_CONTINUE;
        }
        case COMMAND_DRAW_RECT:
        {
            CollectParameters(8);
            ActualIndex = CTheScripts::NumberOfIntroRectanglesThisFrame;
            CTheScripts::IntroRectangles[ActualIndex].eWindowType = WINDOW_SOLID_COLOUR;
            CTheScripts::IntroRectangles[ActualIndex].ScriptSpriteIndex = -1;
            MinX = (*(f32*)&ScriptParams[0] - *(f32*)&ScriptParams[2] * 0.5f) * (SCREEN_WIDTH / 640.0f);
            MinY = (*(f32*)&ScriptParams[1] - *(f32*)&ScriptParams[3] * 0.5f) * (SCREEN_HEIGHT / 448.0f);
            MaxX = (*(f32*)&ScriptParams[0] + *(f32*)&ScriptParams[2] * 0.5f) * (SCREEN_WIDTH / 640.0f);
            MaxY = (*(f32*)&ScriptParams[1] + *(f32*)&ScriptParams[3] * 0.5f) * (SCREEN_HEIGHT / 448.0f);
            CTheScripts::IntroRectangles[ActualIndex].ScriptRectMinX = MinX;
            CTheScripts::IntroRectangles[ActualIndex].ScriptRectMinY = MinY;
            CTheScripts::IntroRectangles[ActualIndex].ScriptRectMaxX = MaxX;
            CTheScripts::IntroRectangles[ActualIndex].ScriptRectMaxY = MaxY;
            CTheScripts::IntroRectangles[ActualIndex].ScriptRectRotation = 0.0f;
            CTheScripts::IntroRectangles[ActualIndex].ScriptRectColour = CRGBA((u8)ScriptParams[4], (u8)ScriptParams[5], (u8)ScriptParams[6], (u8)ScriptParams[7]);
            CTheScripts::IntroRectangles[ActualIndex].pTitle[0] = 0;
            ++CTheScripts::NumberOfIntroRectanglesThisFrame;
            return OR_CONTINUE;
        }
        case COMMAND_LOAD_SPRITE:
        {
            CollectParameters(1);
            ReadTextLabelFromScript(FileName, 15);
            for (CharLoop = 0; CharLoop < 15; CharLoop++)
            {
                if (FileName[CharLoop] >= 'A' && FileName[CharLoop] <= 'Z')
                {
                    FileName[CharLoop] += 32;
                }
            }
            txdIndex = CTxdStore::FindTxdSlot("script");
            CTxdStore::PushCurrentTxd();
            CTxdStore::SetCurrentTxd(txdIndex);
            CTheScripts::ScriptSprites[ScriptParams[0]].SetTexture(FileName);
            CTxdStore::PopCurrentTxd();
            return OR_CONTINUE;
        }
        case COMMAND_LOAD_TEXTURE_DICTIONARY:
        {
            ReadTextLabelFromScript(TextLabel, 8);
            strcpy(FileName, "models\\txd\\");
            strncpy(&FileName[11], TextLabel, 8);
            char* pExt = FileName;
            while (*++pExt)
                ;
            strcpy(pExt, ".txd");
            txdIndex = CTxdStore::FindTxdSlot("script");
            if (txdIndex == -1)
            {
                txdIndex = CTxdStore::AddTxdSlot("script");
            }
            CTxdStore::LoadTxd(txdIndex, FileName);
            CTxdStore::AddRef(txdIndex);
            if (IsThisAMissionScript)
            {
                CTheScripts::MissionCleanUp.AddEntityToList(1, CLEANUP_TEXTURE_DICTIONARY);
            }
            return OR_CONTINUE;
        }
        case COMMAND_REMOVE_TEXTURE_DICTIONARY:
        {
            CTheScripts::RemoveScriptTextureDictionary();
            if (IsThisAMissionScript)
            {
                CTheScripts::MissionCleanUp.RemoveEntityFromList(1, CLEANUP_TEXTURE_DICTIONARY);
            }
            return OR_CONTINUE;
        }
        case COMMAND_SET_OBJECT_DYNAMIC:
        {
            CollectParameters(2);
            pObj = CPools::GetObjectPool().GetAt(ScriptParams[0]);
            if (ScriptParams[1])
            {
                if (pObj->m_bIsStatic)
                {
                    pObj->SetIsStatic(false);
                    pObj->AddToMovingList();
                }
            }
            else
            {
                if (!pObj->m_bIsStatic)
                {
                    pObj->SetIsStatic(true);
                    pObj->RemoveFromMovingList();
                }
            }
            return OR_CONTINUE;
        }
        case COMMAND_SET_CHAR_ANIM_SPEED:
        {
            CollectParameters(1);
            pPed = CPools::GetPedPool().GetAt(ScriptParams[0]);
            ReadTextLabelFromScript(AnimName, 24);
            pAnim = RpAnimBlendClumpGetAssociation((RpClump*)pPed->m_pRwObject, AnimName);
            CollectParameters(1);
            if (pAnim)
            {
                pAnim->SetSpeed(*(f32*)&ScriptParams[0]);
            }
            return OR_CONTINUE;
        }
        case COMMAND_PLAY_MISSION_PASSED_TUNE:
        {
            CollectParameters(1);
            if (ScriptParams[0] == 1 || ScriptParams[0] == 2)
            {
                AudioEngine.PreloadBeatTrack(ScriptParams[0] + 10);
                AudioEngine.PlayPreloadedBeatTrack(true);
            }
            return OR_CONTINUE;
        }
        case COMMAND_CLEAR_AREA:
        {
            CollectParameters(5);
            NewX = *(f32*)&ScriptParams[0];
            NewY = *(f32*)&ScriptParams[1];
            NewZ = *(f32*)&ScriptParams[2];
            if (NewZ <= -100.0f)
            {
                NewZ = CWorld::FindGroundZForCoord(NewX, NewY);
            }
            CWorld::ClearExcitingStuffFromArea(CVector(NewX, NewY, NewZ), *(f32*)&ScriptParams[3], ScriptParams[4] != 0);
            return OR_CONTINUE;
        }
        case COMMAND_FREEZE_ONSCREEN_TIMER:
        {
            CollectParameters(1);
            CUserDisplay::OnscnTimer.FreezeTimers = ScriptParams[0] != 0;
            return OR_CONTINUE;
        }
        case COMMAND_SWITCH_CAR_SIREN:
        {
            CollectParameters(2);
            pVehicle = CPools::GetVehiclePool().GetAt(ScriptParams[0]);
            pVehicle->vehicleFlags.bSirenOrAlarm = ScriptParams[1] != 0;
            return OR_CONTINUE;
        }
        case COMMAND_SET_CAR_WATERTIGHT:
        {
            CollectParameters(2);
            pVehicle = CPools::GetVehiclePool().GetAt(ScriptParams[0]);
            if (pVehicle->GetBaseVehicleType() == 0)
            {
                ((CAutomobile*)pVehicle)->autoFlags.bWaterTight = ScriptParams[1] != 0;
            }
            else if (pVehicle->GetBaseVehicleType() == 5)
            {
                ((CBike*)pVehicle)->m_nBikeFlags.bWaterTight = ScriptParams[1] != 0;
            }
            return OR_CONTINUE;
        }
        case COMMAND_SET_CHAR_CANT_BE_DRAGGED_OUT:
        {
            CollectParameters(2);
            pPed = CPools::GetPedPool().GetAt(ScriptParams[0]);
            pPed->bDontDragMeOutCar = ScriptParams[1] != 0;
            return OR_CONTINUE;
        }
        case COMMAND_TURN_CAR_TO_FACE_COORD:
        {
            CollectParameters(3);
            pVehicle = CPools::GetVehiclePool().GetAt(ScriptParams[0]);
            TempCoors = pVehicle->GetPosition();
            Diff.x = TempCoors.x - *(f32*)&ScriptParams[1];
            Diff.y = TempCoors.y - *(f32*)&ScriptParams[2];
            NewHeading = CGeneral::GetATanOfXY(Diff.x, Diff.y) + HALF_PI;
            if (NewHeading > TWO_PI)
            {
                NewHeading -= TWO_PI;
            }
            pVehicle->SetHeading(NewHeading);
            return OR_CONTINUE;
        }
        case COMMAND_DRAW_SPHERE:
        {
            CollectParameters(4);
            NewX = *(f32*)&ScriptParams[0];
            NewY = *(f32*)&ScriptParams[1];
            NewZ = *(f32*)&ScriptParams[2];
            if (NewZ <= -100.0f)
            {
                NewZ = CWorld::FindGroundZForCoord(NewX, NewY);
            }
            C3dMarkers::PlaceMarkerSet(reinterpret_cast<u32>(this) + reinterpret_cast<u32>(PCPointer), 1, CVector(NewX, NewY, NewZ), *(f32*)&ScriptParams[3], 255, 0, 0, 228, 2048, 0.1f, 0);
            return OR_CONTINUE;
        }
        case COMMAND_SET_CAR_STATUS:
        {
            CollectParameters(2);
            pVehicle = CPools::GetVehiclePool().GetAt(ScriptParams[0]);
            if (pVehicle->GetStatus() == STATUS_SIMPLE && ScriptParams[1] != STATUS_SIMPLE)
            {
                CCarCtrl::SwitchVehicleToRealPhysics(pVehicle);
            }
            pVehicle->SetStatus(ScriptParams[1]);
            return OR_CONTINUE;
        }
        case COMMAND_IS_CHAR_MALE:
        {
            CollectParameters(1);
            pPed = CPools::GetPedPool().GetAt(ScriptParams[0]);
            UpdateCompareFlag(!IsPedTypeFemale(static_cast<ePedType>(pPed->m_nPedType)));
            return OR_CONTINUE;
        }
        case COMMAND_SCRIPT_NAME:
        {
            ReadTextLabelFromScript(FileName, 8);
            for (CharLoop = 0; CharLoop < 8; CharLoop++)
            {
                if (FileName[CharLoop] >= 'A' && FileName[CharLoop] <= 'Z')
                {
                    FileName[CharLoop] += 32;
                }
            }
            strncpy(ScriptName, FileName, 8);
            return OR_CONTINUE;
        }
        case COMMAND_SAVE_INT_TO_DEBUG_FILE:
        case COMMAND_SAVE_FLOAT_TO_DEBUG_FILE:
        {
            CollectParameters(1);
            return OR_CONTINUE;
        }
        case COMMAND_SAVE_NEWLINE_TO_DEBUG_FILE:
        case COMMAND_HAS_IMPORT_GARAGE_SLOT_BEEN_FILLED:
        case COMMAND_NO_SPECIAL_CAMERA_FOR_THIS_GARAGE:
        case COMMAND_FORCE_RANDOM_PED_TYPE:
        case COMMAND_GET_COLLECTABLE1S_COLLECTED:
        {
            return OR_CONTINUE;
        }
        case COMMAND_POLICE_RADIO_MESSAGE:
        {
            CollectParameters(3);
            return OR_CONTINUE;
        }
        case COMMAND_SET_CAR_STRONG:
        {
            CollectParameters(2);
            pVehicle = CPools::GetVehiclePool().GetAt(ScriptParams[0]);
            pVehicle->vehicleFlags.bTakeLessDamage = ScriptParams[1] != 0;
            return OR_CONTINUE;
        }
        case COMMAND_SWITCH_RUBBISH:
        {
            CollectParameters(1);
            CRubbish::SetVisibility(ScriptParams[0] != 0);
            return OR_CONTINUE;
        }
        case COMMAND_SWITCH_STREAMING:
        {
            CollectParameters(1);
            if (ScriptParams[0])
            {
                CStreaming::EnableStreaming();
            }
            else
            {
                CStreaming::DisableStreaming();
            }
            return OR_CONTINUE;
        }
        case COMMAND_IS_GARAGE_OPEN:
        {
            ReadTextLabelFromScript(GarageName, 8);
            GarageIndex = CGarages::FindGarageIndex(GarageName);
            if (GarageIndex < 0)
            {
                UpdateCompareFlag(false);
            }
            else
            {
                UpdateCompareFlag(CGarages::IsGarageOpen(GarageIndex));
            }
            return OR_CONTINUE;
        }
        case COMMAND_IS_GARAGE_CLOSED:
        {
            ReadTextLabelFromScript(GarageName, 8);
            GarageIndex = CGarages::FindGarageIndex(GarageName);
            if (GarageIndex < 0)
            {
                UpdateCompareFlag(false);
            }
            else
            {
                UpdateCompareFlag(CGarages::IsGarageClosed(GarageIndex));
            }
            return OR_CONTINUE;
        }
        case COMMAND_SWAP_NEAREST_BUILDING_MODEL:
        {
            CollectParameters(6);
            NewX = *(f32*)&ScriptParams[0];
            NewY = *(f32*)&ScriptParams[1];
            NewZ = *(f32*)&ScriptParams[2];
            if (NewZ <= -100.0f)
            {
                NewZ = CWorld::FindGroundZForCoord(NewX, NewY);
            }
            ModelIndex = ScriptParams[4];
            if (ModelIndex < 0)
            {
                ModelIndex = CTheScripts::UsedObjectArray[-ModelIndex].Index;
            }
            NewModelIndex = ScriptParams[5];
            if (NewModelIndex < 0)
            {
                NewModelIndex = CTheScripts::UsedObjectArray[-NewModelIndex].Index;
            }
            CEntity* pEntityArray[16];
            f32 searchRadius = *(f32*)&ScriptParams[3];
            f32 closestDist = searchRadius + searchRadius;
            CEntity* pClosestObj = nullptr;
            CWorld::FindObjectsOfTypeInRange(ModelIndex, CVector(NewX, NewY, NewZ), searchRadius, true, &NumberOfEnts, 16, pEntityArray, true, false, false, false, false);
            for (i32 i = 0; i < NumberOfEnts; i++)
            {
                CEntity* ent = pEntityArray[i];
                CVector diff = ent->GetPosition() - CVector(NewX, NewY, NewZ);
                f32 dist = diff.Magnitude();
                if (dist < closestDist)
                {
                    closestDist = dist;
                    pClosestObj = ent;
                }
            }
            if (pClosestObj)
            {
                CBuilding* pBuilding = (CBuilding*)pClosestObj;
                pBuilding->ReplaceWithNewModel(NewModelIndex);
                CTheScripts::AddToBuildingSwapArray(pBuilding, ModelIndex, NewModelIndex);
            }
            return OR_CONTINUE;
        }
        case COMMAND_SWITCH_WORLD_PROCESSING:
        {
            CollectParameters(1);
            CWorld::bProcessCutsceneOnly = ScriptParams[0] == 0;
            return OR_CONTINUE;
        }
        case COMMAND_CLEAR_AREA_OF_CARS:
        {
            CollectParameters(6);
            MinX = *(f32*)&ScriptParams[0];
            MaxX = *(f32*)&ScriptParams[3];
            MinY = *(f32*)&ScriptParams[1];
            MaxY = *(f32*)&ScriptParams[4];
            MinZ = *(f32*)&ScriptParams[2];
            MaxZ = *(f32*)&ScriptParams[5];
            if (MinX > MaxX)
            {
                temp_float = MinX;
                MinX = MaxX;
                MaxX = temp_float;
            }
            if (MinY > MaxY)
            {
                temp_float = MinY;
                MinY = MaxY;
                MaxY = temp_float;
            }
            if (MinZ > MaxZ)
            {
                temp_float = MinZ;
                MinZ = MaxZ;
                MaxZ = temp_float;
            }
            CWorld::ClearCarsFromArea(MinX, MinY, MinZ, MaxX, MaxY, MaxZ);
            return OR_CONTINUE;
        }
        case COMMAND_ADD_SPHERE:
        {
            CollectParameters(4);
            NewX = *(f32*)&ScriptParams[0];
            NewY = *(f32*)&ScriptParams[1];
            NewZ = *(f32*)&ScriptParams[2];
            if (NewZ <= -100.0f)
            {
                NewZ = CWorld::FindGroundZForCoord(NewX, NewY);
            }
            ArrayIndex = CollectNextParameterWithoutIncreasingPC();
            ActualIndex = CTheScripts::GetActualScriptThingIndex(ArrayIndex, 0);
            ScriptParams[0] = CTheScripts::AddScriptSphere(reinterpret_cast<u32>(this) + reinterpret_cast<u32>(PCPointer), CVector(NewX, NewY, NewZ), *(f32*)&ScriptParams[3]);
            StoreParameters(1);
            return OR_CONTINUE;
        }
        case COMMAND_REMOVE_SPHERE:
        {
            CollectParameters(1);
            CTheScripts::RemoveScriptSphere(ScriptParams[0]);
            return OR_CONTINUE;
        }
        case COMMAND_SET_EVERYONE_IGNORE_PLAYER:
        {
            CollectParameters(2);
            pPlayer = &CWorld::Players[ScriptParams[0]];
            if (ScriptParams[1])
            {
                if (pPlayer->m_pPed && pPlayer->m_pPed->m_pPlayerData)
                {
                    pPlayer->m_pPed->m_pPlayerData->m_Wanted->m_EverybodyBackOff = true;
                }
                CWorld::StopAllLawEnforcersInTheirTracks();
            }
            else
            {
                if (pPlayer->m_pPed && pPlayer->m_pPed->m_pPlayerData)
                {
                    pPlayer->m_pPed->m_pPlayerData->m_Wanted->m_EverybodyBackOff = false;
                }
            }
            return OR_CONTINUE;
        }
        case COMMAND_STORE_CAR_CHAR_IS_IN_NO_SAVE:
        {
            CollectParameters(1);
            pPed = CPools::GetPedPool().GetAt(ScriptParams[0]);
            if (pPed->bInVehicle)
            {
                pVehicle = pPed->m_pMyVehicle;
            }
            else
            {
                pVehicle = nullptr;
            }
            ScriptParams[0] = CPools::GetVehiclePool().GetIndex(pVehicle);
            StoreParameters(1);
            return OR_CONTINUE;
        }
        case COMMAND_DISPLAY_ONSCREEN_TIMER_WITH_STRING:
        {
            u16 varId = GetIndexOfGlobalVariable();
            CollectParameters(1);
            ReadTextLabelFromScript(TextLabel, 8);
            (void)TheText.Get(TextLabel);
            CUserDisplay::OnscnTimer.AddClock(varId, TextLabel, ScriptParams[0] != 0);
            return OR_CONTINUE;
        }
        case COMMAND_DISPLAY_ONSCREEN_COUNTER_WITH_STRING:
        {
            u16 varId = GetIndexOfGlobalVariable();
            CollectParameters(1);
            ReadTextLabelFromScript(TextLabel, 8);
            (void)TheText.Get(TextLabel);
            CUserDisplay::OnscnTimer.AddCounter(varId, (u16)ScriptParams[0], TextLabel, 0);
            return OR_CONTINUE;
        }
        case COMMAND_CREATE_RANDOM_CAR_FOR_CAR_PARK:
        {
            CollectParameters(4);
            if (CCarCtrl::NumRandomCars < 45)
            {
                i32 randomModel = CPopulation::m_AppropriateLoadedCars.PickRandomCar(false, true);
                if (randomModel != -1)
                {
                    CVehicle* pNewCar = nullptr;
                    switch (((CVehicleModelInfo*)CModelInfo::GetModelInfo(randomModel))->m_vehicleType)
                    {
                        case VEHICLE_TYPE_MONSTERTRUCK:
                            pNewCar = new CMonsterTruck(randomModel, RANDOM_VEHICLE);
                            break;
                        case VEHICLE_TYPE_QUADBIKE:
                            pNewCar = new CQuadBike(randomModel, RANDOM_VEHICLE);
                            break;
                        case VEHICLE_TYPE_HELI:
                            pNewCar = new CHeli(randomModel, RANDOM_VEHICLE);
                            break;
                        case VEHICLE_TYPE_PLANE:
                            pNewCar = new CPlane(randomModel, RANDOM_VEHICLE);
                            break;
                        case VEHICLE_TYPE_BIKE:
                            pNewCar = new CBike(randomModel, RANDOM_VEHICLE);
                            ((CBike*)pNewCar)->m_nBikeFlags.bOnSideStand = true;
                            break;
                        case VEHICLE_TYPE_BMX:
                            pNewCar = new CBmx(randomModel, RANDOM_VEHICLE);
                            ((CBike*)pNewCar)->m_nBikeFlags.bOnSideStand = true;
                            break;
                        case VEHICLE_TYPE_TRAILER:
                            pNewCar = new CTrailer(randomModel, RANDOM_VEHICLE);
                            break;
                        default:
                            pNewCar = new CAutomobile(randomModel, RANDOM_VEHICLE, true);
                            break;
                    }
                    if (pNewCar)
                    {
                        NewX = *(f32*)&ScriptParams[0];
                        NewY = *(f32*)&ScriptParams[1];
                        NewZ = *(f32*)&ScriptParams[2] + pNewCar->GetHeightAboveRoad();
                        pNewCar->SetPosition(CVector(NewX, NewY, NewZ));
                        pNewCar->SetHeading(*(f32*)&ScriptParams[3] * 0.017453292f);
                        CTheScripts::ClearSpaceForMissionEntity(CVector(NewX, NewY, NewZ), pNewCar);
                        pNewCar->vehicleFlags.bIsLocked = false;
                        pNewCar->vehicleFlags.bIsCarParkVehicle = true;
                        pNewCar->SetStatus(STATUS_ABANDONED);
                        CCarCtrl::JoinCarWithRoadSystem(pNewCar);
                        if (pNewCar->AutoPilot.Mission != MISSION_PLANE_CRASH_AND_BURN && pNewCar->AutoPilot.Mission != MISSION_HELI_CRASH_AND_BURN)
                        {
                            pNewCar->AutoPilot.Mission = MISSION_NONE;
                        }
                        pNewCar->AutoPilot.TempAction = static_cast<eAutoPilotTempAction>(0);
                        pNewCar->AutoPilot.DrivingMode = static_cast<eCarDrivingStyle>(CAutoPilot::DRIVINGMODE_STOPFORCARS);
                        pNewCar->AutoPilot.ActualSpeed = 9.0f;
                        pNewCar->AutoPilot.CruiseSpeed = 9;
                        pNewCar->AutoPilot.OldLane = 0;
                        pNewCar->AutoPilot.NewLane = 0;
                        pNewCar->vehicleFlags.bEngineOn = false;
                        CWorld::Add(pNewCar);
                    }
                }
            }
            return OR_CONTINUE;
        }
        case COMMAND_SET_WANTED_MULTIPLIER:
        {
            CollectParameters(1);
            FindPlayerWanted(-1)->m_fMultiplier = *(f32*)&ScriptParams[0];
            return OR_CONTINUE;
        }
        case COMMAND_SET_CAMERA_IN_FRONT_OF_PLAYER:
        {
            TheCamera.SetCameraDirectlyInFrontForFollowPed_CamOnAString();
            return OR_CONTINUE;
        }
        case COMMAND_IS_CAR_VISIBLY_DAMAGED:
        {
            CollectParameters(1);
            pVehicle = CPools::GetVehiclePool().GetAt(ScriptParams[0]);
            UpdateCompareFlag(pVehicle->vehicleFlags.bIsDamaged);
            return OR_CONTINUE;
        }
        case COMMAND_DOES_OBJECT_EXIST:
        {
            CollectParameters(1);
            pObj = CPools::GetObjectPool().GetAt(ScriptParams[0]);
            UpdateCompareFlag(pObj != nullptr);
            return OR_CONTINUE;
        }
        case COMMAND_LOAD_SCENE:
        {
            CollectParameters(3);
            CTimer::Stop();
            CStreaming::LoadScene(CVector(*(f32*)&ScriptParams[0], *(f32*)&ScriptParams[1], *(f32*)&ScriptParams[2]));
            CTimer::Update();
            return OR_CONTINUE;
        }
        case COMMAND_ADD_STUCK_CAR_CHECK:
        {
            CollectParameters(3);
            CTheScripts::StuckCars.AddCarToCheck(ScriptParams[0], *(f32*)&ScriptParams[1], ScriptParams[2], false, false, false, false, 0);
            return OR_CONTINUE;
        }
        case COMMAND_REMOVE_STUCK_CAR_CHECK:
        {
            CollectParameters(1);
            CTheScripts::StuckCars.RemoveCarFromCheck(ScriptParams[0]);
            return OR_CONTINUE;
        }
        case COMMAND_IS_CAR_STUCK:
        {
            CollectParameters(1);
            UpdateCompareFlag(CTheScripts::StuckCars.HasCarBeenStuckForAWhile(ScriptParams[0]));
            return OR_CONTINUE;
        }
        case COMMAND_LOAD_MISSION_AUDIO:
        {
            CollectParameters(2);
            AudioEngine.PreloadMissionAudio((u8)ScriptParams[0] - 1, ScriptParams[1]);
            return OR_CONTINUE;
        }
        case COMMAND_HAS_MISSION_AUDIO_LOADED:
        {
            CollectParameters(1);
            UpdateCompareFlag(AudioEngine.GetMissionAudioLoadingStatus((u8)ScriptParams[0] - 1) != 0);
            return OR_CONTINUE;
        }
        case COMMAND_PLAY_MISSION_AUDIO:
        {
            CollectParameters(1);
            AudioEngine.PlayLoadedMissionAudio((u8)ScriptParams[0] - 1);
            return OR_CONTINUE;
        }
        case COMMAND_HAS_MISSION_AUDIO_FINISHED:
        {
            CollectParameters(1);
            UpdateCompareFlag(AudioEngine.IsMissionAudioSampleFinished((u8)ScriptParams[0] - 1));
            return OR_CONTINUE;
        }
        case COMMAND_GET_CLOSEST_CAR_NODE_WITH_HEADING:
        {
            CollectParameters(3);
            NewX = *(f32*)&ScriptParams[0];
            NewY = *(f32*)&ScriptParams[1];
            NewZ = *(f32*)&ScriptParams[2];
            if (NewZ <= -100.0f)
            {
                NewZ = CWorld::FindGroundZForCoord(NewX, NewY);
            }
            ResultNode = ThePaths.FindNodeClosestToCoors(CVector(NewX, NewY, NewZ), 0, 999999.88f, false, true, false, false, false);
            bool bFound = false;
            TempCoors = ThePaths.FindNodeCoorsForScript(ResultNode, &bFound);
            if (bFound)
            {
                *(CVector*)&ScriptParams[0] = TempCoors;
                *(f32*)&ScriptParams[3] = ThePaths.FindNodeOrientationForCarPlacement(ResultNode);
                StoreParameters(4);
                UpdateCompareFlag(true);
            }
            else
            {
                ScriptParams[0] = 0;
                ScriptParams[1] = 0;
                ScriptParams[2] = 0;
                ScriptParams[3] = 0;
                StoreParameters(4);
                UpdateCompareFlag(false);
            }
            return OR_CONTINUE;
        }
        case COMMAND_CLEAR_THIS_PRINT:
        {
            ReadTextLabelFromScript(TextLabel, 8);
            CMessages::ClearThisPrint(TheText.Get(TextLabel));
            return OR_CONTINUE;
        }
        case COMMAND_CLEAR_THIS_BIG_PRINT:
        {
            ReadTextLabelFromScript(TextLabel, 8);
            CMessages::ClearThisBigPrint(TheText.Get(TextLabel));
            return OR_CONTINUE;
        }
        case COMMAND_SET_MISSION_AUDIO_POSITION:
        {
            CollectParameters(4);
            CVector audioPos(*(f32*)&ScriptParams[1], *(f32*)&ScriptParams[2], *(f32*)&ScriptParams[3]);
            AudioEngine.SetMissionAudioPosition((u8)ScriptParams[0] - 1, audioPos);
            return OR_CONTINUE;
        }
        case COMMAND_ACTIVATE_SAVE_MENU:
        {
            CPed* pPlayerPed = FindPlayerPed(-1);
            if (pPlayerPed && !pPlayerPed->bInVehicle)
            {
                FrontEndMenuManager.m_bIsSaveDone = true;
                pPlayerPed->SetMoveSpeed(0.0f, 0.0f, 0.0f);
                pPlayerPed->SetTurnSpeed(0.0f, 0.0f, 0.0f);
            }
            return OR_CONTINUE;
        }
        case COMMAND_HAS_SAVE_GAME_FINISHED:
        {
            bool bDone = false;
            if (!FrontEndMenuManager.m_MenuActive && !FrontEndMenuManager.m_bIsSaveDone)
            {
                bDone = true;
            }
            UpdateCompareFlag(bDone);
            return OR_CONTINUE;
        }
        case COMMAND_ADD_BLIP_FOR_PICKUP:
        {
            CollectParameters(1);
            CPickups::GetActualPickupIndex(ScriptParams[0]);
            ArrayIndex = CollectNextParameterWithoutIncreasingPC();
            CRadar::GetActualBlipArrayIndex(ArrayIndex);
            BlipIndex = CRadar::SetEntityBlip(BLIPTYPE_PICKUP, ScriptParams[0], 6, BLIPDISPLAY_BOTH);
            CRadar::ChangeBlipScale(BlipIndex, 3);
            ScriptParams[0] = BlipIndex;
            StoreParameters(1);
            return OR_CONTINUE;
        }
        case COMMAND_SET_PED_DENSITY_MULTIPLIER:
        {
            CollectParameters(1);
            CPopulation::PedDensityMultiplier = *(f32*)&ScriptParams[0];
            return OR_CONTINUE;
        }
        case COMMAND_SET_TEXT_DRAW_BEFORE_FADE:
        {
            CollectParameters(1);
            CTheScripts::IntroTextLines[CTheScripts::NumberOfIntroTextLinesThisFrame].ScriptTextBeforeFade = ScriptParams[0] != 0;
            return OR_CONTINUE;
        }
        case COMMAND_SET_SPRITES_DRAW_BEFORE_FADE:
        {
            CollectParameters(1);
            CTheScripts::IntroRectangles[CTheScripts::NumberOfIntroRectanglesThisFrame].ScriptRectBeforeFade = ScriptParams[0] != 0;
            return OR_CONTINUE;
        }
        case COMMAND_SET_TEXT_RIGHT_JUSTIFY:
        {
            CollectParameters(1);
            CTheScripts::IntroTextLines[CTheScripts::NumberOfIntroTextLinesThisFrame].ScriptTextRightJustify = ScriptParams[0] != 0;
            return OR_CONTINUE;
        }
        case COMMAND_PRINT_HELP:
        {
            ReadTextLabelFromScript(TextLabel, 8);
            pString = TheText.Get(TextLabel);
            if (CTheScripts::bDisplayNonMiniGameHelpMessages || bIsThisAMiniGameScript || !CTheScripts::bMiniGameInProgress)
            {
                CHud::SetHelpMessage(pString, false, false, false);
                if (CTheScripts::bAddNextMessageToPreviousBriefs)
                {
                    CMessages::AddToPreviousBriefArray(pString, -1, -1, -1, -1, -1, -1, nullptr);
                }
            }
            CTheScripts::bAddNextMessageToPreviousBriefs = true;
            return OR_CONTINUE;
        }
        case COMMAND_CLEAR_HELP:
        {
            CHud::SetHelpMessage(nullptr, true, false, false);
            return OR_CONTINUE;
        }
        case COMMAND_FLASH_HUD_OBJECT:
        {
            CollectParameters(1);
            CHud::m_ItemToFlash = static_cast<eHudItem>(ScriptParams[0]);
            return OR_CONTINUE;
        }
        default:
            return OR_INTERRUPT;
    }
    return OR_CONTINUE;
}

// MARK: 1000To1099

// func: sa 0x489500
OpcodeResult CRunningScript::ProcessCommands1000To1099(i32 CurrCommand)
{
    MARKFUNCTION(0x489500);

    bool LatestCmpFlagResult;

    CPed* pPed;
    CVehicle* pVehicle;

    CObject* pObj;

    CPlayerInfo* pPlayer;

    CVector TempCoors;
    CVector TempVec;

    f32 MinZ, MinY, MinX;
    f32 MaxZ, MaxY, MaxX;
    f32 temp_float;

    f32 NewX, NewY, NewZ;

    i32 loop;

    f32 NewTraction;
    f32 Distance;
    i32 IntegerDistance;

    switch (CurrCommand)
    {
        case COMMAND_FLASH_RADAR_BLIP:
        {
            return OR_CONTINUE;
        }
        case COMMAND_SET_GENERATE_CARS_AROUND_CAMERA:
        {
            return OR_CONTINUE;
        }
        case COMMAND_CLEAR_SMALL_PRINTS:
        {
            CMessages::ClearSmallMessagesOnly();
            return OR_CONTINUE;
        }
        case COMMAND_SET_UPSIDEDOWN_CAR_NOT_DAMAGED:
        {
            CollectParameters(2);
            pVehicle = CPools::GetVehiclePool().GetAt(ScriptParams[0]);
            if (ScriptParams[1])
            {
                ((CAutomobile*)pVehicle)->autoFlags.bDoesNotGetDamagedUpsideDown = true;
            }
            else
            {
                ((CAutomobile*)pVehicle)->autoFlags.bDoesNotGetDamagedUpsideDown = false;
            }
            return OR_CONTINUE;
        }
        case COMMAND_CAN_PLAYER_START_MISSION:
        {
            CollectParameters(1);
            UpdateCompareFlag(CWorld::Players[ScriptParams[0]].m_pPed->CanPlayerStartMission());
            return OR_CONTINUE;
        }
        case COMMAND_MAKE_PLAYER_SAFE_FOR_CUTSCENE:
        {
            CollectParameters(1);
            pPlayer = &CWorld::Players[ScriptParams[0]];
            CPad::GetPad(ScriptParams[0])->DisablePlayerControls |= DISABLE_CUTSCENES;
            pPlayer->MakePlayerSafe(true, 10000.0f);
            CCutsceneMgr::ms_cutsceneProcessing = true;
            return OR_CONTINUE;
        }
        case COMMAND_USE_TEXT_COMMANDS:
        {
            CollectParameters(1);
            CTheScripts::UseTextCommands = static_cast<CTheScripts::eUseTextCommandState>((ScriptParams[0] != 0) + 1);
            return OR_CONTINUE;
        }
        case COMMAND_GET_CAR_COLOURS:
        {
            CollectParameters(1);
            pVehicle = CPools::GetVehiclePool().GetAt(ScriptParams[0]);
            ScriptParams[0] = pVehicle->m_colour1;
            ScriptParams[1] = pVehicle->m_colour2;
            StoreParameters(2);
            return OR_CONTINUE;
        }
        case COMMAND_SET_ALL_CARS_CAN_BE_DAMAGED:
        {
            CollectParameters(1);
            if (ScriptParams[0])
            {
                CWorld::SetAllCarsCanBeDamaged(true);
            }
            else
            {
                CWorld::SetAllCarsCanBeDamaged(false);
                CWorld::ExtinguishAllCarFiresInArea(FindPlayerCoors(-1), 4000.0f);
            }
            return OR_CONTINUE;
        }
        case COMMAND_SET_CAR_CAN_BE_DAMAGED:
        {
            CollectParameters(2);
            pVehicle = CPools::GetVehiclePool().GetAt(ScriptParams[0]);
            if (ScriptParams[1])
            {
                pVehicle->vehicleFlags.bCanBeDamaged = true;
            }
            else
            {
                pVehicle->ExtinguishCarFire();
                pVehicle->vehicleFlags.bCanBeDamaged = false;
            }
            return OR_CONTINUE;
        }
        case COMMAND_SET_DRUNK_INPUT_DELAY:
        {
            CollectParameters(2);
            CPad::GetPad(ScriptParams[0])->SetDrunkInputDelay(ScriptParams[1]);
            return OR_CONTINUE;
        }
        case COMMAND_SET_CHAR_MONEY:
        {
            CollectParameters(2);
            pPed = CPools::GetPedPool().GetAt(ScriptParams[0]);
            pPed->bMoneyHasBeenGivenByScript = true;
            pPed->m_MoneyCarried = (u16)ScriptParams[1];
            return OR_CONTINUE;
        }
        case COMMAND_GET_OFFSET_FROM_OBJECT_IN_WORLD_COORDS:
        {
            CollectParameters(4);
            pObj = CPools::GetObjectPool().GetAt(ScriptParams[0]);
            TempCoors = Multiply3x3(*pObj->m_pMat, *(CVector*)&ScriptParams[1]);
            TempCoors += pObj->GetPosition();
            *(CVector*)&ScriptParams[0] = TempCoors;
            StoreParameters(3);
            return OR_CONTINUE;
        }
        case COMMAND_GET_OFFSET_FROM_CAR_IN_WORLD_COORDS:
        {
            CollectParameters(4);
            pVehicle = CPools::GetVehiclePool().GetAt(ScriptParams[0]);
            TempCoors = Multiply3x3(*pVehicle->m_pMat, *(CVector*)&ScriptParams[1]);
            TempCoors += pVehicle->GetPosition();
            *(CVector*)&ScriptParams[0] = TempCoors;
            StoreParameters(3);
            return OR_CONTINUE;
        }
        case COMMAND_BLOW_UP_RC_BUGGY:
        {
            return OR_CONTINUE;
        }
        case COMMAND_IS_FRENCH_GAME:
        {
            return OR_CONTINUE;
        }
        case COMMAND_IS_GERMAN_GAME:
        {
            UpdateCompareFlag(CLocalisation::GermanGame());
            return OR_CONTINUE;
        }
        case COMMAND_CLEAR_MISSION_AUDIO:
        {
            CollectParameters(1);
            AudioEngine.ClearMissionAudio(ScriptParams[0] - 1);
            return OR_CONTINUE;
        }
        case COMMAND_SET_FREE_HEALTH_CARE:
        {
            CollectParameters(2);
            CWorld::Players[ScriptParams[0]].bFreeHealthCare = (ScriptParams[1] != 0);
            return OR_CONTINUE;
        }
        case COMMAND_LOAD_AND_LAUNCH_MISSION:
        {
            return OR_CONTINUE;
        }
        case COMMAND_LOAD_AND_LAUNCH_MISSION_INTERNAL:
        {
            CollectParameters(1);
            if (CTheScripts::NumberOfExclusiveMissionScripts > 0)
            {
                if (ScriptParams[0] > 65532)
                {
                    ScriptParams[0] = 0xFFFF - ScriptParams[0];
                }
                else
                {
                    return OR_CONTINUE;
                }
            }
            CTimer::Suspend();
            u32 offsetToMission = CTheScripts::MultiScriptArray[ScriptParams[0]];
            CFileMgr::ChangeDir("\\");
            i32 bytesRead = 0;
            if (CGame::bMissionPackGame)
            {
                while (FrontEndMenuManager.CheckMissionPackValidMenu())
                {
                    CFileMgr::SetDirMyDocuments();
                    sprintf(gString, "MPACK//MPACK%d//SCR.SCM", CGame::bMissionPackGame);
                    FILESTREAM file0 = CFileMgr::OpenFile(gString, "rb");
                    if (file0)
                    {
                        CFileMgr::Seek(file0, offsetToMission, 0);
                        bytesRead = CFileMgr::Read(file0, (char*)&CTheScripts::ScriptSpace[200000], 69000);
                        CFileMgr::CloseFile(file0);
                        if (bytesRead >= 1)
                        {
                            CTheScripts::WipeLocalVariableMemoryForMissionScript();
                            CRunningScript* pScript = CTheScripts::StartNewScript(&CTheScripts::ScriptSpace[200000]);
                            pScript->IsThisAMissionScript = true;
                            pScript->ThisMustBeTheOnlyMissionRunning = true;
                            pScript->BaseAddressOfThisScript = &CTheScripts::ScriptSpace[200000];
                            CTheScripts::bAlreadyRunningAMissionScript = true;
                            CGameLogic::ClearSkip(false);
                            break;
                        }
                    }
                    if (bytesRead >= 1)
                    {
                        break;
                    }
                }
            }
            CFileMgr::SetDir("");
            if (!CGame::bMissionPackGame)
            {
                FILESTREAM file = CFileMgr::OpenFile("data\\script\\main.scm", "rb");
                CFileMgr::Seek(file, offsetToMission, 0);
                CFileMgr::Read(file, (char*)&CTheScripts::ScriptSpace[200000], 69000);
                CFileMgr::CloseFile(file);
                CTheScripts::WipeLocalVariableMemoryForMissionScript();
                CRunningScript* pScript = CTheScripts::StartNewScript(&CTheScripts::ScriptSpace[200000]);
                pScript->IsThisAMissionScript = true;
                pScript->ThisMustBeTheOnlyMissionRunning = true;
                pScript->BaseAddressOfThisScript = &CTheScripts::ScriptSpace[200000];
                CTheScripts::bAlreadyRunningAMissionScript = true;
                CGameLogic::ClearSkip(false);
            }
            CTimer::Resume();
            return OR_CONTINUE;
        }
        case COMMAND_SET_OBJECT_DRAW_LAST:
        {
            CollectParameters(2);
            pObj = CPools::GetObjectPool().GetAt(ScriptParams[0]);
            pObj->m_bDrawLast = (ScriptParams[1] != 0);
            return OR_CONTINUE;
        }
        case COMMAND_GET_AMMO_IN_CHAR_WEAPON:
        {
            CollectParameters(2);
            pPed = CPools::GetPedPool().GetAt(ScriptParams[0]);
            ScriptParams[0] = 0;
            for (i32 i = 0; i < 13; i++)
            {
                if (pPed->m_WeaponSlots[i].m_eWeaponType == ScriptParams[1])
                {
                    ScriptParams[0] = pPed->m_WeaponSlots[i].m_nAmmoTotal;
                }
            }
            StoreParameters(1);
            return OR_CONTINUE;
        }
        case COMMAND_SET_NEAR_CLIP:
        {
            CollectParameters(1);
            TheCamera.SetNearClipScript(*(f32*)&ScriptParams[0]);
            return OR_CONTINUE;
        }
        case COMMAND_SET_RADIO_CHANNEL:
        {
            CollectParameters(1);
            i8 radio = ScriptParams[0];
            if (radio == -1)
            {
                return OR_CONTINUE;
            }
            if (radio == 11)
            {
                radio = 12;
            }
            AudioEngine.RetuneRadio(radio + 1);
            return OR_CONTINUE;
        }
        case COMMAND_SET_CAR_TRACTION:
        {
            CollectParameters(2);
            pVehicle = CPools::GetVehiclePool().GetAt(ScriptParams[0]);
            if (pVehicle->GetBaseVehicleType() == 0)
            {
                ((CAutomobile*)pVehicle)->m_fCarTraction = *(f32*)&ScriptParams[1];
            }
            else
            {
                ((CBike*)pVehicle)->m_fExtraTractionMult = *(f32*)&ScriptParams[1];
            }
            return OR_CONTINUE;
        }
        case COMMAND_ARE_MEASUREMENTS_IN_METRES:
        {
            UpdateCompareFlag(CLocalisation::Metric());
            return OR_CONTINUE;
        }
        case COMMAND_CONVERT_METRES_TO_FEET:
        {
            CollectParameters(1);
            *(f32*)&ScriptParams[0] = *(f32*)&ScriptParams[0] * 3.3333333f;
            StoreParameters(1);
            return OR_CONTINUE;
        }
        case COMMAND_SET_CAR_AVOID_LEVEL_TRANSITIONS:
        {
            CollectParameters(2);
            pVehicle = CPools::GetVehiclePool().GetAt(ScriptParams[0]);
            pVehicle->AutoPilot.AvoidLevelTransitions = (ScriptParams[1] != 0);
            return OR_CONTINUE;
        }
        case COMMAND_CLEAR_AREA_OF_CHARS:
        {
            CollectParameters(6);
            f32 infX = *(f32*)&ScriptParams[0];
            f32 supX = *(f32*)&ScriptParams[3];
            f32 infY = *(f32*)&ScriptParams[1];
            f32 supY = *(f32*)&ScriptParams[4];
            f32 infZ = *(f32*)&ScriptParams[2];
            f32 supZ = *(f32*)&ScriptParams[5];
            if (infX > supX)
            {
                f32 t = infX;
                infX = supX;
                supX = t;
            }
            if (infY > supY)
            {
                f32 t = infY;
                infY = supY;
                supY = t;
            }
            if (infZ > supZ)
            {
                f32 t = infZ;
                infZ = supZ;
                supZ = t;
            }
            CWorld::ClearPedsFromArea(infX, infY, infZ, supX, supY, supZ);
            return OR_CONTINUE;
        }
        case COMMAND_SET_TOTAL_NUMBER_OF_MISSIONS:
        {
            CollectParameters(1);
            CStats::SetStatValue(0x94, (f32)ScriptParams[0]);
            return OR_CONTINUE;
        }
        case COMMAND_CONVERT_METRES_TO_FEET_INT:
        {
            CollectParameters(1);
            ScriptParams[0] = (i32)((f32)ScriptParams[0] * 3.3333333f);
            StoreParameters(1);
            return OR_CONTINUE;
        }
        case COMMAND_REGISTER_FASTEST_TIME:
        {
            CollectParameters(2);
            CStats::RegisterFastestTime(ScriptParams[0], ScriptParams[1]);
            return OR_CONTINUE;
        }
        case COMMAND_REGISTER_HIGHEST_SCORE:
        {
            return OR_CONTINUE;
        }
        case COMMAND_WARP_CHAR_INTO_CAR_AS_PASSENGER:
        {
            CollectParameters(3);
            pPed = CPools::GetPedPool().GetAt(ScriptParams[0]);
            pVehicle = CPools::GetVehiclePool().GetAt(ScriptParams[1]);
            i32 nTargetDoor = 0;
            if (ScriptParams[2] >= 0)
            {
                nTargetDoor = CCarEnterExit::ComputeTargetDoorToEnterAsPassenger(*pVehicle, ScriptParams[2]);
            }
            pPed->m_pPedIntelligence->FlushImmediately(false);
            CTaskSimpleCarSetPedInAsPassenger task(pVehicle, nTargetDoor, nullptr);
            
            task.ProcessPed(pPed);
            return OR_CONTINUE;
        }
        case COMMAND_IS_CAR_PASSENGER_SEAT_FREE:
        {
            CollectParameters(2);
            pVehicle = CPools::GetVehiclePool().GetAt(ScriptParams[0]);
            UpdateCompareFlag(ScriptParams[1] < pVehicle->m_nMaxPassengers && pVehicle->pPassengers[ScriptParams[1]] == nullptr);
            return OR_CONTINUE;
        }
        case COMMAND_GET_CHAR_IN_CAR_PASSENGER_SEAT:
        {
            CollectParameters(2);
            pVehicle = CPools::GetVehiclePool().GetAt(ScriptParams[0]);
            ScriptParams[0] = CPools::GetPedPool().GetIndex(pVehicle->pPassengers[ScriptParams[1]]);
            StoreParameters(1);
            return OR_CONTINUE;
        }
        case COMMAND_SET_CHAR_IS_CHRIS_CRIMINAL:
        {
            CollectParameters(2);
            pPed = CPools::GetPedPool().GetAt(ScriptParams[0]);
            pPed->bChrisCriminal = (ScriptParams[1] != 0);
            return OR_CONTINUE;
        }
        case COMMAND_START_CREDITS:
        {
            CCredits::Start();
            return OR_CONTINUE;
        }
        case COMMAND_STOP_CREDITS:
        {
            CCredits::Stop();
            return OR_CONTINUE;
        }
        case COMMAND_ARE_CREDITS_FINISHED:
        {
            UpdateCompareFlag(!CCredits::bCreditsGoing);
            return OR_CONTINUE;
        }
        case COMMAND_SET_MUSIC_DOES_FADE:
        {
            CollectParameters(1);
            TheCamera.m_bIgnoreFadingStuffForMusic = (ScriptParams[0] == 0);
            return OR_CONTINUE;
        }
        case COMMAND_SET_PLAYER_HOOKER:
        {
            return OR_CONTINUE;
        }
        case COMMAND_PLAY_END_OF_GAME_TUNE:
        {
            return OR_CONTINUE;
        }
        case COMMAND_STOP_END_OF_GAME_TUNE:
        {
            return OR_CONTINUE;
        }
        case COMMAND_GET_CAR_MODEL:
        {
            CollectParameters(1);
            pVehicle = CPools::GetVehiclePool().GetAt(ScriptParams[0]);
            ScriptParams[0] = pVehicle->m_nModelIndex;
            StoreParameters(1);
            return OR_CONTINUE;
        }
        case COMMAND_ARE_ANY_CAR_CHEATS_ACTIVATED:
        {
            UpdateCompareFlag(CCheat::IsCheatActive(STRONGGRIP_CHEAT) || CCheat::IsCheatActive(BACKTOTHEFUTURE_CHEAT) || CCheat::IsCheatActive(FLYINGFISH_CHEAT));
            return OR_CONTINUE;
        }
        case COMMAND_SET_CHAR_SUFFERS_CRITICAL_HITS:
        {
            CollectParameters(2);
            pPed = CPools::GetPedPool().GetAt(ScriptParams[0]);
            pPed->bNoCriticalHits = (ScriptParams[1] == 0);
            return OR_CONTINUE;
        }
        case COMMAND_IS_CHAR_SITTING_IN_CAR:
        {
            CollectParameters(2);
            pPed = CPools::GetPedPool().GetAt(ScriptParams[0]);
            bool bSitting = false;
            if (pPed->bInVehicle && pPed->m_pMyVehicle == CPools::GetVehiclePool().GetAt(ScriptParams[1]))
            {
                CTask* pTask = pPed->m_pPedIntelligence->GetTaskManager().GetSimplestActiveTask();
                if (pTask && (pTask->GetTaskType() == CTaskTypes::TASK_SIMPLE_CAR_DRIVE || pTask->GetTaskType() == CTaskTypes::TASK_SIMPLE_GANG_DRIVEBY))
                {
                    bSitting = true;
                }
            }
            UpdateCompareFlag(bSitting);
            return OR_CONTINUE;
        }
        case COMMAND_IS_CHAR_SITTING_IN_ANY_CAR:
        {
            CollectParameters(1);
            pPed = CPools::GetPedPool().GetAt(ScriptParams[0]);
            bool bSitting = false;
            if (pPed->bInVehicle)
            {
                CTask* pTask = pPed->m_pPedIntelligence->GetTaskManager().GetSimplestActiveTask();
                if (pTask && (pTask->GetTaskType() == CTaskTypes::TASK_SIMPLE_CAR_DRIVE || pTask->GetTaskType() == CTaskTypes::TASK_SIMPLE_GANG_DRIVEBY))
                {
                    bSitting = true;
                }
            }
            UpdateCompareFlag(bSitting);
            return OR_CONTINUE;
        }
        case COMMAND_IS_CHAR_ON_FOOT:
        {
            CollectParameters(1);
            pPed = CPools::GetPedPool().GetAt(ScriptParams[0]);
            bool bOnFoot = true;
            if (pPed->bInVehicle || pPed->m_pPedIntelligence->GetTaskManager().FindActiveTaskByType(CTaskTypes::TASK_COMPLEX_ENTER_CAR_AS_PASSENGER) || pPed->m_pPedIntelligence->GetTaskManager().FindActiveTaskByType(CTaskTypes::TASK_COMPLEX_ENTER_CAR_AS_DRIVER))
            {
                bOnFoot = false;
            }
            UpdateCompareFlag(bOnFoot);
            return OR_CONTINUE;
        }
        default:
        {
            return OR_INTERRUPT;
        }
    }
}

// MARK: 1100To1199

// sa 0x48A320
OpcodeResult CRunningScript::ProcessCommands1100To1199(i32 CurrCommand)
{
    MARKFUNCTION(0x48A320);
    switch (CurrCommand)
    {
        case COMMAND_LOAD_SPLASH_SCREEN:
        case COMMAND_SET_JAMES_CAR_ON_PATH_TO_PLAYER:
        case COMMAND_LOAD_END_OF_GAME_TUNE:
        case COMMAND_GET_CLOSEST_OBJECT_OF_TYPE:
        case COMMAND_GET_RANDOM_COP_IN_AREA:
        case COMMAND_SET_ENTER_CAR_RANGE_MULTIPLIER:
        case COMMAND_SET_THREAT_REACTION_RANGE_MULTIPLIER:
        case COMMAND_SET_TANK_DETONATE_CARS:
        case COMMAND_INITIALISE_OBJECT_PATH:
        case COMMAND_SET_OBJECT_PATH_SPEED:
        case COMMAND_SET_OBJECT_PATH_POSITION:
        case COMMAND_CLEAR_OBJECT_PATH:
            return OR_CONTINUE;

        case COMMAND_SET_OBJECT_ROTATION:
        {
            CollectParameters(4);
            CObject* pObj = CPools::GetObjectPool().GetAt(ScriptParams[0]);
            CWorld::Remove(pObj);
            f32 rotX = DEGTORAD(*(f32*)&ScriptParams[1]);
            f32 rotY = DEGTORAD(*(f32*)&ScriptParams[2]);
            f32 rotZ = DEGTORAD(*(f32*)&ScriptParams[3]);
            pObj->SetOrientation(rotX, rotY, rotZ);
            pObj->UpdateRwMatrix();
            pObj->UpdateRwFrame();
            CWorld::Add(pObj);
            return OR_CONTINUE;
        }

        case COMMAND_GET_DEBUG_CAMERA_COORDINATES:
        {
            ScriptParams[0] = *(i32*)&TheCamera.Cams[2].Source.x;
            ScriptParams[1] = *(i32*)&TheCamera.Cams[2].Source.y;
            ScriptParams[2] = *(i32*)&TheCamera.Cams[2].Source.z;
            StoreParameters(3);
            return OR_CONTINUE;
        }

        case COMMAND_IS_PLAYER_TARGETTING_CHAR:
        {
            CollectParameters(2);
            CPlayerInfo* pPlayer = &CWorld::Players[ScriptParams[0]];
            CPed* pPed = CPools::GetPedPool().GetAt(ScriptParams[1]);
            bool bFound = false;
            if (pPlayer->m_pPed->GetWeaponLockOnTarget())
            {
                if (pPlayer->m_pPed->GetWeaponLockOnTarget()->GetIsTypePed())
                {
                    bFound = (pPlayer->m_pPed->GetWeaponLockOnTarget() == pPed);
                }
            }
            if (CCamera::m_bUseMouse3rdPerson)
            {
                CPad* pPad = pPlayer->m_pPed->GetPadFromPlayer();
                if (pPad->GetTarget())
                {
                    if (pPlayer->m_pPed->m_pMouseLockOnRecruitPed)
                    {
                        if (pPlayer->m_pPed->m_pMouseLockOnRecruitPed == pPed)
                        {
                            bFound = true;
                        }
                    }
                }
            }
            UpdateCompareFlag(bFound);
            return OR_CONTINUE;
        }

        case COMMAND_IS_PLAYER_TARGETTING_OBJECT:
        {
            CollectParameters(2);
            CEntity* pTarget = CWorld::Players[ScriptParams[0]].m_pPed->GetWeaponLockOnTarget();
            bool bFound = false;
            if (pTarget && pTarget->GetIsTypeObject() && pTarget == CPools::GetObjectPool().GetAt(ScriptParams[1]))
            {
                bFound = true;
            }
            UpdateCompareFlag(bFound);
            return OR_CONTINUE;
        }

        case COMMAND_TERMINATE_ALL_SCRIPTS_WITH_THIS_NAME:
        {
            char name[8];
            ReadTextLabelFromScript(name, 8);
            for (i32 i = 0; i < 8; i++)
            {
                if (name[i] >= 'A' && name[i] <= 'Z')
                {
                    name[i] += 'a' - 'A';
                }
            }
            CRunningScript* pScript = CTheScripts::pActiveScripts;
            while (pScript)
            {
                CRunningScript* pNextScript = pScript->pNext;
                if (strcmp(pScript->ScriptName, name) == 0)
                {
                    pScript->RemoveScriptFromList(&CTheScripts::pActiveScripts);
                    pScript->AddScriptToList(&CTheScripts::pIdleScripts);
                    pScript->ShutdownThisScript();
                }
                pScript = pNextScript;
            }
            return OR_CONTINUE;
        }

        case COMMAND_DISPLAY_TEXT_WITH_NUMBER:
        {
            CollectParameters(2);
            char key[8];
            ReadTextLabelFromScript(key, 8);
            (void)TheText.Get(key);
            u16 idx = CTheScripts::NumberOfIntroTextLinesThisFrame;
            CTheScripts::IntroTextLines[idx].ScriptTextAtX = *(f32*)&ScriptParams[0];
            CTheScripts::IntroTextLines[idx].ScriptTextAtY = *(f32*)&ScriptParams[1];
            CollectParameters(1);
            strncpy(CTheScripts::IntroTextLines[CTheScripts::NumberOfIntroTextLinesThisFrame].ScriptTextLabel, key, 8);
            CTheScripts::IntroTextLines[CTheScripts::NumberOfIntroTextLinesThisFrame].NumberToInsert1 = ScriptParams[0];
            CTheScripts::IntroTextLines[CTheScripts::NumberOfIntroTextLinesThisFrame].NumberToInsert2 = -1;
            CTheScripts::NumberOfIntroTextLinesThisFrame++;
            return OR_CONTINUE;
        }

        case COMMAND_DISPLAY_TEXT_WITH_2_NUMBERS:
        {
            CollectParameters(2);
            char key[8];
            ReadTextLabelFromScript(key, 8);
            (void)TheText.Get(key);
            u16 idx = CTheScripts::NumberOfIntroTextLinesThisFrame;
            CTheScripts::IntroTextLines[idx].ScriptTextAtX = *(f32*)&ScriptParams[0];
            CTheScripts::IntroTextLines[idx].ScriptTextAtY = *(f32*)&ScriptParams[1];
            CollectParameters(2);
            strncpy(CTheScripts::IntroTextLines[CTheScripts::NumberOfIntroTextLinesThisFrame].ScriptTextLabel, key, 8);
            CTheScripts::IntroTextLines[CTheScripts::NumberOfIntroTextLinesThisFrame].NumberToInsert1 = ScriptParams[0];
            CTheScripts::IntroTextLines[CTheScripts::NumberOfIntroTextLinesThisFrame].NumberToInsert2 = ScriptParams[1];
            CTheScripts::NumberOfIntroTextLinesThisFrame++;
            return OR_CONTINUE;
        }

        case COMMAND_FAIL_CURRENT_MISSION:
        {
            CTheScripts::FailCurrentMission = 2;
            return OR_CONTINUE;
        }

        case COMMAND_SET_INTERPOLATION_PARAMETERS:
        {
            CollectParameters(2);
            f32 fStop = *(f32*)&ScriptParams[0];
            TheCamera.SetParametersForScriptInterpolation(fStop, 100.0f - fStop, ScriptParams[1]);
            return OR_CONTINUE;
        }

        case COMMAND_GET_DEBUG_CAMERA_POINT_AT:
        {
            *(f32*)&ScriptParams[0] = TheCamera.Cams[2].Front.x + TheCamera.Cams[2].Source.x;
            *(f32*)&ScriptParams[1] = TheCamera.Cams[2].Front.y + TheCamera.Cams[2].Source.y;
            *(f32*)&ScriptParams[2] = TheCamera.Cams[2].Front.z + TheCamera.Cams[2].Source.z;
            StoreParameters(3);
            return OR_CONTINUE;
        }

        case COMMAND_ATTACH_CHAR_TO_CAR:
        {
            CollectParameters(8);
            CPed* pPed = CPools::GetPedPool().GetAt(ScriptParams[0]);
            CVehicle* pVehicle = CPools::GetVehiclePool().GetAt(ScriptParams[1]);
            f32 angleLimit = DEGTORAD(*(f32*)&ScriptParams[6]);
            pPed->AttachPedToEntity(pVehicle, CVector(*(f32*)&ScriptParams[2], *(f32*)&ScriptParams[3], *(f32*)&ScriptParams[4]), (u16)ScriptParams[5], angleLimit, (eWeaponType)ScriptParams[7]);
            return OR_CONTINUE;
        }

        case COMMAND_DETACH_CHAR_FROM_CAR:
        {
            CollectParameters(1);
            CPed* pPed = CPools::GetPedPool().GetAt(ScriptParams[0]);
            if (!pPed || !pPed->m_pAttachToEntity)
            {
                return OR_CONTINUE;
            }
            pPed->DettachPedFromEntity();
            return OR_CONTINUE;
        }

        case COMMAND_SET_CAR_STAY_IN_FAST_LANE:
        {
            CollectParameters(2);
            CVehicle* pVehicle = CPools::GetVehiclePool().GetAt(ScriptParams[0]);
            pVehicle->AutoPilot.bAlwaysInFastLane = (ScriptParams[1] != 0);
            return OR_CONTINUE;
        }

        case COMMAND_CLEAR_CHAR_LAST_WEAPON_DAMAGE:
        {
            CollectParameters(1);
            CPed* pPed = CPools::GetPedPool().GetAt(ScriptParams[0]);
            if (!pPed)
            {
                return OR_CONTINUE;
            }
            pPed->m_nLastWeaponDamage = -1;
            return OR_CONTINUE;
        }

        case COMMAND_CLEAR_CAR_LAST_WEAPON_DAMAGE:
        {
            CollectParameters(1);
            CVehicle* pVehicle = CPools::GetVehiclePool().GetAt(ScriptParams[0]);
            if (!pVehicle)
            {
                return OR_CONTINUE;
            }
            pVehicle->m_nLastWeaponDamageType = -1;
            return OR_CONTINUE;
        }

        case COMMAND_GET_DRIVER_OF_CAR:
        {
            CollectParameters(1);
            CPed* pDriver = CPools::GetVehiclePool().GetAt(ScriptParams[0])->pDriver;
            if (pDriver)
            {
                ScriptParams[0] = CPools::GetPedPool().GetIndex(pDriver);
            }
            else
            {
                ScriptParams[0] = -1;
            }
            StoreParameters(1);
            return OR_CONTINUE;
        }

        case COMMAND_GET_NUMBER_OF_FOLLOWERS:
        {
            CollectParameters(1);
            CPed* pPed = CPools::GetPedPool().GetAt(ScriptParams[0]);
            u8 count = 0;
            CPedGroup* pGroup = CPedGroups::GetPedsGroup(pPed);
            if (pGroup)
            {
                CPedGroupMembership* pMembership = pGroup->GetGroupMembership();
                if (pMembership->IsLeader(pPed))
                {
                    count = pMembership->CountMembersExcludingLeader();
                }
            }
            ScriptParams[0] = count;
            StoreParameters(1);
            return OR_CONTINUE;
        }

        case COMMAND_GIVE_REMOTE_CONTROLLED_MODEL_TO_PLAYER:
        {
            CollectParameters(6);
            f32 x = *(f32*)&ScriptParams[1];
            f32 y = *(f32*)&ScriptParams[2];
            f32 z = *(f32*)&ScriptParams[3];
            if (z <= -100.0f)
            {
                z = CWorld::FindGroundZForCoord(x, y);
            }
            f32 heading = DEGTORAD(*(f32*)&ScriptParams[4]);
            CRemote::GivePlayerRemoteControlledCar(x, y, z, heading, (u16)ScriptParams[5]);
            return OR_CONTINUE;
        }

        case COMMAND_GET_CURRENT_CHAR_WEAPON:
        {
            CollectParameters(1);
            CPed* pPed = CPools::GetPedPool().GetAt(ScriptParams[0]);
            ScriptParams[0] = pPed->m_WeaponSlots[pPed->m_nCurrentWeapon].m_eWeaponType;
            StoreParameters(1);
            return OR_CONTINUE;
        }

        case COMMAND_LOCATE_CHAR_ANY_MEANS_OBJECT_2D:
        case COMMAND_LOCATE_CHAR_ON_FOOT_OBJECT_2D:
        case COMMAND_LOCATE_CHAR_IN_CAR_OBJECT_2D:
        case COMMAND_LOCATE_CHAR_ANY_MEANS_OBJECT_3D:
        case COMMAND_LOCATE_CHAR_ON_FOOT_OBJECT_3D:
        case COMMAND_LOCATE_CHAR_IN_CAR_OBJECT_3D:
        {
            LocateCharObjectCommand(CurrCommand);
            return OR_CONTINUE;
        }

        case COMMAND_SET_CAR_TEMP_ACTION:
        {
            CollectParameters(3);
            CVehicle* pVehicle = CPools::GetVehiclePool().GetAt(ScriptParams[0]);
            pVehicle->AutoPilot.TempAction = static_cast<eAutoPilotTempAction>(ScriptParams[1]);
            pVehicle->AutoPilot.TempActionFinish = ScriptParams[2] + CTimer::m_snTimeInMilliseconds;
            return OR_CONTINUE;
        }

        case COMMAND_IS_CHAR_ON_ANY_BIKE:
        {
            CollectParameters(1);
            CPed* pPed = CPools::GetPedPool().GetAt(ScriptParams[0]);
            bool bResult = false;
            if (pPed->bInVehicle && pPed->m_pMyVehicle)
            {
                bResult = (pPed->m_pMyVehicle->GetVehicleAppearance() == APR_BIKE);
            }
            UpdateCompareFlag(bResult);
            return OR_CONTINUE;
        }

        case COMMAND_CAN_CHAR_SEE_DEAD_CHAR:
        {
            CollectParameters(2);
            CPed* pPed = CPools::GetPedPool().GetAt(ScriptParams[0]);
            bool bResult = false;
            CEntity** apEntities = pPed->GetPedIntelligence()->GetNearbyPeds();
            for (i32 i = 0; i < 16; i++)
            {
                CPed* pOther = (CPed*)apEntities[i];
                if (pOther)
                {
                    if (!pOther->IsAlive() && CPedGeometryAnalyser::CanPedTargetPed(*pPed, *pOther, true))
                    {
                        bResult = true;
                        break;
                    }
                }
            }
            UpdateCompareFlag(bResult);
            return OR_CONTINUE;
        }

        case COMMAND_GET_REMOTE_CONTROLLED_CAR:
        {
            CollectParameters(1);
            if (CWorld::Players[ScriptParams[0]].pRemoteVehicle)
            {
                ScriptParams[0] = CPools::GetVehiclePool().GetIndex(CWorld::Players[ScriptParams[0]].pRemoteVehicle);
            }
            else
            {
                ScriptParams[0] = -1;
            }
            StoreParameters(1);
            return OR_CONTINUE;
        }

        case COMMAND_IS_PC_VERSION:
        {
            UpdateCompareFlag(true);
            return OR_CONTINUE;
        }

        case COMMAND_IS_MODEL_AVAILABLE:
        {
            CollectParameters(1);
            UpdateCompareFlag(CModelInfo::GetModelInfo(ScriptParams[0]) != nullptr);
            return OR_CONTINUE;
        }

        case COMMAND_SHUT_CHAR_UP:
        {
            CollectParameters(2);
            CPed* pPed = CPools::GetPedPool().GetAt(ScriptParams[0]);
            if (ScriptParams[1])
            {
                if (pPed)
                {
                    pPed->DisablePedSpeech(0);
                    return OR_CONTINUE;
                }
            }
            else if (pPed)
            {
                pPed->EnablePedSpeech();
                return OR_CONTINUE;
            }
            return OR_CONTINUE;
        }

        case COMMAND_SET_ENABLE_RC_DETONATE:
        {
            CollectParameters(1);
            CVehicle::bDisableRemoteDetonation = (ScriptParams[0] == 0);
            return OR_CONTINUE;
        }

        case COMMAND_SET_CAR_RANDOM_ROUTE_SEED:
        {
            CollectParameters(2);
            CPools::GetVehiclePool().GetAt(ScriptParams[0])->ForcedRandomSeed = ScriptParams[1];
            return OR_CONTINUE;
        }

        case COMMAND_IS_ANY_PICKUP_AT_COORDS:
        {
            CollectParameters(3);
            f32 x = *(f32*)&ScriptParams[0];
            f32 y = *(f32*)&ScriptParams[1];
            f32 z = *(f32*)&ScriptParams[2];
            bool bFound = false;
            CPickup* pPickup = CPickups::aPickUps.data();
            for (i32 i = 0; i < MAX_NUM_PICKUPS; i++, pPickup++)
            {
                if (pPickup->m_nPickupType != PICKUP_NONE)
                {
                    CVector vecDiff = pPickup->GetCoors() - CVector(x, y, z);
                    if (vecDiff.Magnitude() < 0.5f)
                    {
                        bFound = true;
                        break;
                    }
                }
            }
            UpdateCompareFlag(bFound);
            return OR_CONTINUE;
        }

        case COMMAND_REMOVE_ALL_CHAR_WEAPONS:
        {
            CollectParameters(1);
            CPed* pPed = CPools::GetPedPool().GetAt(ScriptParams[0]);
            pPed->ClearWeapons();
            return OR_CONTINUE;
        }

        case COMMAND_HAS_CHAR_GOT_WEAPON:
        {
            CollectParameters(2);
            CPed* pPed = CPools::GetPedPool().GetAt(ScriptParams[0]);
            bool bFound = false;
            for (i32 i = 0; i < 13; i++)
            {
                if (pPed->m_WeaponSlots[i].m_eWeaponType == ScriptParams[1])
                {
                    bFound = true;
                    break;
                }
            }
            UpdateCompareFlag(bFound);
            return OR_CONTINUE;
        }

        case COMMAND_GET_POSITION_OF_ANALOGUE_STICKS:
        {
            CollectParameters(1);
            CPad* pPad = CPad::GetPad(ScriptParams[0]);
            ScriptParams[0] = pPad->NewState.LeftStickX;
            ScriptParams[1] = pPad->NewState.LeftStickY;
            ScriptParams[2] = pPad->NewState.RightStickX;
            ScriptParams[3] = pPad->NewState.RightStickY;
            StoreParameters(4);
            return OR_CONTINUE;
        }

        case COMMAND_IS_CAR_ON_FIRE:
        {
            CollectParameters(1);
            CVehicle* pVehicle = CPools::GetVehiclePool().GetAt(ScriptParams[0]);
            bool bOnFire = (pVehicle->m_pFire != nullptr);
            if (pVehicle->GetVehicleType() == VEHICLE_TYPE_CAR && ((CAutomobile*)pVehicle)->Damage.GetEngineStatus() >= 225)
            {
                bOnFire = true;
            }
            if (pVehicle->m_nHealth < CAR_ON_FIRE_HEALTH)
            {
                bOnFire = true;
            }
            UpdateCompareFlag(bOnFire);
            return OR_CONTINUE;
        }

        case COMMAND_IS_CAR_TYRE_BURST:
        {
            CollectParameters(2);
            CVehicle* pVehicle = CPools::GetVehiclePool().GetAt(ScriptParams[0]);
            bool bBurst = false;
            if (pVehicle->GetBaseVehicleType() == VEHICLE_TYPE_BIKE)
            {
                CBike* pBike = (CBike*)pVehicle;
                if (ScriptParams[1] == 4)
                {
                    if (pBike->m_nWheelStatus[0] == 1)
                    {
                        bBurst = true;
                    }
                    if (pBike->m_nWheelStatus[1] == 1)
                    {
                        bBurst = true;
                    }
                }
                else
                {
                    if (ScriptParams[1] == 2)
                    {
                        ScriptParams[1] = 0;
                    }
                    else if (ScriptParams[1] == 3)
                    {
                        ScriptParams[1] = 1;
                    }
                    if (pBike->m_nWheelStatus[ScriptParams[1]] == 1)
                    {
                        bBurst = true;
                    }
                }
            }
            else
            {
                CAutomobile* pCar = (CAutomobile*)pVehicle;
                if (ScriptParams[1] == 4)
                {
                    for (i32 i = 0; i < 4; i++)
                    {
                        if (pCar->Damage.GetWheelStatus(i) == 1)
                        {
                            bBurst = true;
                        }
                    }
                }
                else
                {
                    if (pCar->Damage.GetWheelStatus(ScriptParams[1]) == 1)
                    {
                        bBurst = true;
                    }
                }
            }
            UpdateCompareFlag(bBurst);
            return OR_CONTINUE;
        }

        case COMMAND_HELI_GOTO_COORDS:
        {
            CollectParameters(6);
            CVehicle* pVehicle = CPools::GetVehiclePool().GetAt(ScriptParams[0]);
            ((CAutomobile*)pVehicle)->TellHeliToGoToCoors(*(f32*)&ScriptParams[1], *(f32*)&ScriptParams[2], *(f32*)&ScriptParams[3], *(f32*)&ScriptParams[4], *(f32*)&ScriptParams[5]);
            return OR_CONTINUE;
        }

        case COMMAND_IS_INT_VAR_EQUAL_TO_CONSTANT:
        {
            i32* pVar = GetPointerToScriptVariable(SCOPE_GLOBAL);
            CollectParameters(1);
            UpdateCompareFlag(*pVar == ScriptParams[0]);
            return OR_CONTINUE;
        }

        case COMMAND_IS_INT_LVAR_EQUAL_TO_CONSTANT:
        {
            i32* pVar = GetPointerToScriptVariable(SCOPE_LOCAL);
            CollectParameters(1);
            UpdateCompareFlag(*pVar == ScriptParams[0]);
            return OR_CONTINUE;
        }

        case COMMAND_GET_DEAD_CHAR_PICKUP_COORDS:
        {
            CollectParameters(1);
            CPed* pPed = CPools::GetPedPool().GetAt(ScriptParams[0]);
            pPed->CreateDeadPedPickupCoors((f32*)&ScriptParams[0], (f32*)&ScriptParams[1], (f32*)&ScriptParams[2]);
            StoreParameters(3);
            return OR_CONTINUE;
        }

        case COMMAND_CREATE_PROTECTION_PICKUP:
        {
            CollectParameters(5);
            f32 x = *(f32*)&ScriptParams[0];
            f32 y = *(f32*)&ScriptParams[1];
            f32 z = *(f32*)&ScriptParams[2];
            if (z <= -100.0f)
            {
                z = CWorld::FindGroundZForCoord(x, y) + 0.5f;
            }
            i32 param = CollectNextParameterWithoutIncreasingPC();
            CPickups::GetActualPickupIndex(param);
            ScriptParams[0] = CPickups::GenerateNewOne(CVector(x, y, z), MI_PICKUP_REVENUE, PICKUP_ASSET_REVENUE, ScriptParams[3], ScriptParams[4], false, nullptr);
            StoreParameters(1);
            return OR_CONTINUE;
        }

        case COMMAND_IS_CHAR_IN_ANY_BOAT:
        {
            CollectParameters(1);
            CPed* pPed = CPools::GetPedPool().GetAt(ScriptParams[0]);
            bool bFound = false;
            if (pPed->bInVehicle && pPed->m_pMyVehicle->GetVehicleAppearance() == APR_BOAT)
            {
                bFound = true;
            }
            UpdateCompareFlag(bFound);
            return OR_CONTINUE;
        }

        case COMMAND_IS_CHAR_IN_ANY_HELI:
        {
            CollectParameters(1);
            CPed* pPed = CPools::GetPedPool().GetAt(ScriptParams[0]);
            bool bFound = false;
            if (pPed->bInVehicle)
            {
                bFound = (pPed->m_pMyVehicle->GetVehicleAppearance() == APR_HELI);
            }
            UpdateCompareFlag(bFound);
            return OR_CONTINUE;
        }

        case COMMAND_IS_CHAR_IN_ANY_PLANE:
        {
            CollectParameters(1);
            CPed* pPed = CPools::GetPedPool().GetAt(ScriptParams[0]);
            bool bFound = false;
            if (pPed->bInVehicle)
            {
                bFound = (pPed->m_pMyVehicle->GetVehicleAppearance() == APR_PLANE);
            }
            UpdateCompareFlag(bFound);
            return OR_CONTINUE;
        }

        case COMMAND_IS_CHAR_IN_WATER:
        {
            CollectParameters(1);
            CPed* pPed = CPools::GetPedPool().GetAt(ScriptParams[0]);
            bool bFound = false;
            if (pPed && pPed->physicalFlags.bIsInWater)
            {
                bFound = true;
            }
            UpdateCompareFlag(bFound);
            return OR_CONTINUE;
        }

        case COMMAND_SET_VAR_INT_TO_CONSTANT:
        {
            i32* pVar = GetPointerToScriptVariable(SCOPE_GLOBAL);
            CollectParameters(1);
            *pVar = ScriptParams[0];
            return OR_CONTINUE;
        }

        case COMMAND_SET_LVAR_INT_TO_CONSTANT:
        {
            i32* pVar = GetPointerToScriptVariable(SCOPE_LOCAL);
            CollectParameters(1);
            *pVar = ScriptParams[0];
            return OR_CONTINUE;
        }

        default:
            return OR_INTERRUPT;
    }
}

// MARK: 1200To1299

// sa 0x48B590
OpcodeResult CRunningScript::ProcessCommands1200To1299(i32 CurrCommand)
{
    MARKFUNCTION(0x48B590);

    bool LatestCmpFlagResult;

    CPed* pPed;
    CVehicle* pVehicle;

    CObject* pObj;

    CPlayerInfo* pPlayer;

    CVector TempCoors;

    f32 MinX;
    f32 MaxX;
    f32 temp_float;

    f32 NewX = 0.0f;
    f32 NewY;
    f32 NewZ;
    f32 FirstY, FirstX;
    f32 SecondY, SecondX;

    f32 Generate1X;
    f32 Generate2X;

    const GxtChar* pString;
    char TextLabel[16];

    f32 HeadingLimit;

    f32 CarSpeed;
    char AnimBlockName[16];

    CBaseModelInfo* pBaseInfo;
    i32* pGlobalVar;
    i32* pLocalVar;
    i32 BlipIndex;
    CNodeAddress Node1, Node2;
    f32 Orientation;
    i32 AmmoTotal;
    CNodeAddress ResultNode;
    CAnimBlock* pAnimBlock;
    i32 AnimBlockIndex;
    f32 fLowestFlightHeight;
    f32 fMinHeightAboveTerrain;

    switch (CurrCommand)
    {
        case COMMAND_IS_INT_VAR_GREATER_THAN_CONSTANT:
        {
            pGlobalVar = GetPointerToScriptVariable(SCOPE_GLOBAL);
            CollectParameters(1);
            UpdateCompareFlag(*pGlobalVar > ScriptParams[0]);
            return OR_CONTINUE;
        }
        case COMMAND_IS_INT_LVAR_GREATER_THAN_CONSTANT:
        {
            pLocalVar = GetPointerToScriptVariable(SCOPE_LOCAL);
            CollectParameters(1);
            UpdateCompareFlag(*pLocalVar > ScriptParams[0]);
            return OR_CONTINUE;
        }
        case COMMAND_IS_CONSTANT_GREATER_THAN_INT_VAR:
        {
            CollectParameters(1);
            pGlobalVar = GetPointerToScriptVariable(SCOPE_GLOBAL);
            UpdateCompareFlag(ScriptParams[0] > *pGlobalVar);
            return OR_CONTINUE;
        }
        case COMMAND_IS_CONSTANT_GREATER_THAN_INT_LVAR:
        {
            CollectParameters(1);
            pLocalVar = GetPointerToScriptVariable(SCOPE_LOCAL);
            UpdateCompareFlag(ScriptParams[0] > *pLocalVar);
            return OR_CONTINUE;
        }
        case COMMAND_IS_INT_VAR_GREATER_OR_EQUAL_TO_CONSTANT:
        {
            pGlobalVar = GetPointerToScriptVariable(SCOPE_GLOBAL);
            CollectParameters(1);
            UpdateCompareFlag(*pGlobalVar >= ScriptParams[0]);
            return OR_CONTINUE;
        }
        case COMMAND_IS_INT_LVAR_GREATER_OR_EQUAL_TO_CONSTANT:
        {
            pLocalVar = GetPointerToScriptVariable(SCOPE_LOCAL);
            CollectParameters(1);
            UpdateCompareFlag(*pLocalVar >= ScriptParams[0]);
            return OR_CONTINUE;
        }
        case COMMAND_IS_CONSTANT_GREATER_OR_EQUAL_TO_INT_VAR:
        {
            CollectParameters(1);
            pGlobalVar = GetPointerToScriptVariable(SCOPE_GLOBAL);
            UpdateCompareFlag(ScriptParams[0] >= *pGlobalVar);
            return OR_CONTINUE;
        }
        case COMMAND_IS_CONSTANT_GREATER_OR_EQUAL_TO_INT_LVAR:
        {
            CollectParameters(1);
            pLocalVar = GetPointerToScriptVariable(SCOPE_LOCAL);
            UpdateCompareFlag(ScriptParams[0] >= *pLocalVar);
            return OR_CONTINUE;
        }
        case COMMAND_GET_CHAR_WEAPON_IN_SLOT:
        {
            CollectParameters(2);
            pPed = CPools::GetPedPool().GetAt(ScriptParams[0]);
            ScriptParams[0] = pPed->m_WeaponSlots[ScriptParams[1] - 1].m_eWeaponType;
            ScriptParams[1] = pPed->m_WeaponSlots[ScriptParams[1] - 1].m_nAmmoTotal;
            ScriptParams[2] = CPickups::ModelForWeapon((eWeaponType)ScriptParams[0]);
            StoreParameters(3);
            return OR_CONTINUE;
        }
        case COMMAND_GET_CLOSEST_STRAIGHT_ROAD:
        {
            CollectParameters(5);
            ThePaths.FindNodePairClosestToCoors(CVector(*(f32*)&ScriptParams[0], *(f32*)&ScriptParams[1], *(f32*)&ScriptParams[2]), 0, &Node1, &Node2, &Orientation, *(f32*)&ScriptParams[3], *(f32*)&ScriptParams[4], true, true, false);
            if (!Node1.IsEmpty())
            {
                CVector Coors1 = ThePaths.FindNodeCoorsForScript(Node1, nullptr);
                ScriptParams[0] = *(i32*)&Coors1.x;
                ScriptParams[1] = *(i32*)&Coors1.y;
                ScriptParams[2] = *(i32*)&Coors1.z;
                bool bSuccess;
                CVector Coors2 = ThePaths.FindNodeCoorsForScript(Node2, &bSuccess);
                if (bSuccess)
                {
                    ScriptParams[3] = *(i32*)&Coors2.x;
                    ScriptParams[4] = *(i32*)&Coors2.y;
                    ScriptParams[5] = *(i32*)&Coors2.z;
                    ScriptParams[6] = *(i32*)&Orientation;
                    StoreParameters(7);
                    return OR_CONTINUE;
                }
            }
            ScriptParams[0] = 0;
            ScriptParams[1] = 0;
            ScriptParams[2] = 0;
            ScriptParams[3] = 0;
            ScriptParams[4] = 0;
            ScriptParams[5] = 0;
            ScriptParams[6] = 0;
            StoreParameters(7);
            return OR_CONTINUE;
        }
        case COMMAND_SET_CAR_FORWARD_SPEED:
        {
            CollectParameters(2);
            pVehicle = CPools::GetVehiclePool().GetAt(ScriptParams[0]);
            pVehicle->m_vecMoveSpeed = pVehicle->GetMatrix().GetForward() * (*(f32*)&ScriptParams[1] * (1.0f / 60.0f));
            if ((pVehicle->pHandling->mFlags & 0x2000000) != 0 && pVehicle->GetVehicleType() == VEHICLE_TYPE_CAR)
            {
                ((CAutomobile*)pVehicle)->m_aWheelAngularVelocity[1] = 0.22f;
            }
            return OR_CONTINUE;
        }
        case COMMAND_SET_AREA_VISIBLE:
        {
            CollectParameters(1);
            CGame::currArea = (eVisibleArea)ScriptParams[0];
            CStreaming::RemoveBuildingsNotInArea(ScriptParams[0]);
            return OR_CONTINUE;
        }
        case COMMAND_SET_CUTSCENE_ANIM_TO_LOOP:
        {
            return OR_CONTINUE;
        }
        case COMMAND_MARK_CAR_AS_CONVOY_CAR:
        {
            CollectParameters(2);
            pVehicle = CPools::GetVehiclePool().GetAt(ScriptParams[0]);
            pVehicle->vehicleFlags.bPartOfConvoy = (ScriptParams[1] != 0);
            return OR_CONTINUE;
        }
        case COMMAND_RESET_HAVOC_CAUSED_BY_PLAYER:
        {
            return OR_CONTINUE;
        }
        case COMMAND_GET_HAVOC_CAUSED_BY_PLAYER:
        {
            return OR_CONTINUE;
        }
        case COMMAND_CREATE_SCRIPT_ROADBLOCK:
        {
            CollectParameters(7);
            CRoadBlocks::RegisterScriptRoadBlock(*(CVector*)&ScriptParams[0], *(CVector*)&ScriptParams[3], ScriptParams[6] != 0);
            return OR_CONTINUE;
        }
        case COMMAND_CLEAR_ALL_SCRIPT_ROADBLOCKS:
        {
            CRoadBlocks::ClearScriptRoadBlocks();
            return OR_CONTINUE;
        }
        case COMMAND_GET_OFFSET_FROM_CHAR_IN_WORLD_COORDS:
        {
            CollectParameters(4);
            pPed = CPools::GetPedPool().GetAt(ScriptParams[0]);
            CVector in = *(CVector*)&ScriptParams[1];
            TempCoors = Multiply3x3(*pPed->m_pMat, in);
            TempCoors += pPed->GetPosition();
            *(CVector*)&ScriptParams[0] = TempCoors;
            StoreParameters(3);
            return OR_CONTINUE;
        }
        case COMMAND_HAS_CHAR_BEEN_PHOTOGRAPHED:
        {
            CollectParameters(1);
            pPed = CPools::GetPedPool().GetAt(ScriptParams[0]);
            bool bPhotographed = false;
            if (pPed && pPed->bHasBeenPhotographed)
            {
                bPhotographed = true;
                pPed->bHasBeenPhotographed = false;
            }
            UpdateCompareFlag(bPhotographed);
            return OR_CONTINUE;
        }
        case COMMAND_SWITCH_SECURITY_CAMERA:
        {
            return OR_CONTINUE;
        }
        case COMMAND_IS_CHAR_IN_FLYING_VEHICLE:
        {
            CollectParameters(1);
            pPed = CPools::GetPedPool().GetAt(ScriptParams[0]);
            bool bFlying = false;
            if (pPed->bInVehicle && (pPed->m_pMyVehicle->GetVehicleAppearance() == APR_PLANE || pPed->m_pMyVehicle->GetVehicleAppearance() == APR_HELI))
            {
                bFlying = true;
            }
            UpdateCompareFlag(bFlying);
            return OR_CONTINUE;
        }
        case COMMAND_ADD_SHORT_RANGE_SPRITE_BLIP_FOR_COORD:
        {
            CollectParameters(4);
            NewX = *(f32*)&ScriptParams[0];
            NewY = *(f32*)&ScriptParams[1];
            NewZ = *(f32*)&ScriptParams[2];
            if (NewZ <= -100.0f)
            {
                NewZ = CWorld::FindGroundZForCoord(NewX, NewY);
            }
            CRadar::GetActualBlipArrayIndex(CollectNextParameterWithoutIncreasingPC());
            CVector BlipCoors(NewX, NewY, NewZ);
            BlipIndex = CRadar::SetShortRangeCoordBlip(BLIPTYPE_COORDS, BlipCoors, 5, BLIPDISPLAY_BOTH, ScriptName);
            CRadar::SetBlipSprite(BlipIndex, ScriptParams[3]);
            ScriptParams[0] = BlipIndex;
            StoreParameters(1);
            return OR_CONTINUE;
        }
        case COMMAND_SET_HELI_ORIENTATION:
        {
            CollectParameters(2);
            pVehicle = CPools::GetVehiclePool().GetAt(ScriptParams[0]);
            f32 angle = (*(f32*)&ScriptParams[1] + 90.0f) * 0.017453292f;
            while (angle < 0.0f)
            {
                angle += 6.2831855f;
            }
            while (angle > 6.2831855f)
            {
                angle -= 6.2831855f;
            }
            ((CAutomobile*)pVehicle)->SetHeliOrientation(angle);
            return OR_CONTINUE;
        }
        case COMMAND_CLEAR_HELI_ORIENTATION:
        {
            CollectParameters(1);
            pVehicle = CPools::GetVehiclePool().GetAt(ScriptParams[0]);
            ((CAutomobile*)pVehicle)->ClearHeliOrientation();
            return OR_CONTINUE;
        }
        case COMMAND_PLANE_GOTO_COORDS:
        {
            CollectParameters(6);
            pVehicle = CPools::GetVehiclePool().GetAt(ScriptParams[0]);
            ((CAutomobile*)pVehicle)->TellPlaneToGoToCoors(*(f32*)&ScriptParams[1], *(f32*)&ScriptParams[2], *(f32*)&ScriptParams[3], *(f32*)&ScriptParams[4], *(f32*)&ScriptParams[5]);
            return OR_CONTINUE;
        }
        case COMMAND_GET_NTH_CLOSEST_CAR_NODE:
        {
            CollectParameters(4);
            NewX = *(f32*)&ScriptParams[0];
            NewY = *(f32*)&ScriptParams[1];
            NewZ = *(f32*)&ScriptParams[2];
            if (NewZ <= -100.0f)
            {
                NewZ = CWorld::FindGroundZForCoord(NewX, NewY);
            }
            ResultNode = ThePaths.FindNthNodeClosestToCoors(CVector(NewX, NewY, NewZ), 0, 999999.88f, false, true, ScriptParams[3] - 1, false, false, nullptr);
            bool bSuccess;
            CVector NodeCoors = ThePaths.FindNodeCoorsForScript(ResultNode, &bSuccess);
            if (bSuccess)
            {
                *(CVector*)&ScriptParams[0] = NodeCoors;
            }
            else
            {
                ScriptParams[0] = 0;
                ScriptParams[1] = 0;
                ScriptParams[2] = 0;
            }
            StoreParameters(3);
            return OR_CONTINUE;
        }
        case COMMAND_DRAW_WEAPONSHOP_CORONA:
        {
            CollectParameters(9);
            NewX = *(f32*)&ScriptParams[0];
            NewY = *(f32*)&ScriptParams[1];
            NewZ = *(f32*)&ScriptParams[2];
            if (NewZ <= -100.0f)
            {
                NewZ = CWorld::FindGroundZForCoord(NewX, NewY);
            }
            CCoronas::RegisterCorona(reinterpret_cast<u_native>(this) + reinterpret_cast<u_native>(PCPointer), nullptr, ScriptParams[6], ScriptParams[7], ScriptParams[8], 255, CVector(NewX, NewY, NewZ), *(f32*)&ScriptParams[3], 150.0f, (u8)ScriptParams[4], ScriptParams[5], 1, 0, 0, 0.0f, false, 0.2f, false, 15.0f, false, false);
            return OR_CONTINUE;
        }
        case COMMAND_SET_ENABLE_RC_DETONATE_ON_CONTACT:
        {
            CollectParameters(1);
            CVehicle::bDisableRemoteDetonationOnContact = (ScriptParams[0] == 0);
            return OR_CONTINUE;
        }
        case COMMAND_FREEZE_CHAR_POSITION:
        {
            CollectParameters(2);
            pPed = CPools::GetPedPool().GetAt(ScriptParams[0]);
            pPed->physicalFlags.bCoorsFrozenByScript = (ScriptParams[1] != 0);
            return OR_CONTINUE;
        }
        case COMMAND_SET_CHAR_DROWNS_IN_WATER:
        {
            CollectParameters(2);
            pPed = CPools::GetPedPool().GetAt(ScriptParams[0]);
            pPed->bDrownsInWater = (ScriptParams[1] != 0);
            return OR_CONTINUE;
        }
        case COMMAND_SET_OBJECT_RECORDS_COLLISIONS:
        {
            CollectParameters(2);
            pObj = CPools::GetObjectPool().GetAt(ScriptParams[0]);
            pObj->physicalFlags.bUsesCollisionRecords = (ScriptParams[1] != 0);
            return OR_CONTINUE;
        }
        case COMMAND_HAS_OBJECT_COLLIDED_WITH_ANYTHING:
        {
            CollectParameters(1);
            pObj = CPools::GetObjectPool().GetAt(ScriptParams[0]);
            UpdateCompareFlag(pObj->m_nNoOfCollisionRecords != 0);
            return OR_CONTINUE;
        }
        case COMMAND_REMOVE_RC_BUGGY:
        {
            CWorld::Players[CWorld::PlayerInFocus].BlowUpRCBuggy(false);
            return OR_CONTINUE;
        }
        case COMMAND_GET_CHAR_ARMOUR:
        {
            CollectParameters(1);
            pPed = CPools::GetPedPool().GetAt(ScriptParams[0]);
            ScriptParams[0] = (i32)pPed->m_nArmour;
            StoreParameters(1);
            return OR_CONTINUE;
        }
        case COMMAND_SET_HELI_STABILISER:
        {
            CollectParameters(2);
            pVehicle = CPools::GetVehiclePool().GetAt(ScriptParams[0]);
            pVehicle->vehicleFlags.bHeliMinimumTilt = (ScriptParams[1] != 0);
            return OR_CONTINUE;
        }
        case COMMAND_SET_CAR_STRAIGHT_LINE_DISTANCE:
        {
            CollectParameters(2);
            pVehicle = CPools::GetVehiclePool().GetAt(ScriptParams[0]);
            pVehicle->AutoPilot.AISwitchToStraightLineDistance = ScriptParams[1];
            return OR_CONTINUE;
        }
        case COMMAND_POP_CAR_BOOT:
        {
            CollectParameters(1);
            pVehicle = CPools::GetVehiclePool().GetAt(ScriptParams[0]);
            ((CAutomobile*)pVehicle)->PopBoot();
            return OR_CONTINUE;
        }
        case COMMAND_SHUT_PLAYER_UP:
        {
            CollectParameters(2);
            return OR_CONTINUE;
        }
        case COMMAND_SET_PLAYER_MOOD:
        {
            CollectParameters(3);
            CAEPedSpeechAudioEntity::SetCJMood(ScriptParams[1], ScriptParams[2], -1, -1, -1);
            return OR_CONTINUE;
        }
        case COMMAND_REQUEST_COLLISION:
        {
            CollectParameters(2);
            CColStore::RequestCollision(CVector(*(f32*)&ScriptParams[0], *(f32*)&ScriptParams[1], 0.0f), CGame::currArea);
            return OR_CONTINUE;
        }
        case COMMAND_LOCATE_OBJECT_2D:
        {
            LocateObjectCommand(CurrCommand);
            return OR_CONTINUE;
        }
        case COMMAND_LOCATE_OBJECT_3D:
        {
            LocateObjectCommand(CurrCommand);
            return OR_CONTINUE;
        }
        case COMMAND_IS_OBJECT_IN_WATER:
        {
            CollectParameters(1);
            pObj = CPools::GetObjectPool().GetAt(ScriptParams[0]);
            UpdateCompareFlag(pObj && pObj->physicalFlags.bIsInWater);
            return OR_CONTINUE;
        }
        case COMMAND_IS_OBJECT_IN_AREA_2D:
        {
            ObjectInAreaCheckCommand(CurrCommand);
            return OR_CONTINUE;
        }
        case COMMAND_IS_OBJECT_IN_AREA_3D:
        {
            ObjectInAreaCheckCommand(CurrCommand);
            return OR_CONTINUE;
        }
        case COMMAND_TASK_TOGGLE_DUCK:
        {
            CollectParameters(2);
            GivePedScriptedTask(ScriptParams[0], new CTaskSimpleDuckToggle(static_cast<CTaskSimpleDuckToggle::eMode>(ScriptParams[1])), CurrCommand);
            return OR_CONTINUE;
        }
        case COMMAND_REQUEST_ANIMATION:
        {
            ReadTextLabelFromScript(AnimBlockName, 16);
            AnimBlockIndex = CAnimManager::GetAnimationBlockIndex(AnimBlockName);
            CStreaming::RequestModel(AnimBlockIndex + 25575, 2);
            CTheScripts::ScriptResourceManager.AddToResourceManager(AnimBlockIndex, 1, this);
            return OR_CONTINUE;
        }
        case COMMAND_HAS_ANIMATION_LOADED:
        {
            ReadTextLabelFromScript(AnimBlockName, 16);
            UpdateCompareFlag(CAnimManager::GetAnimationBlock(AnimBlockName)->m_loaded);
            return OR_CONTINUE;
        }
        case COMMAND_REMOVE_ANIMATION:
        {
            ReadTextLabelFromScript(AnimBlockName, 16);
            AnimBlockIndex = CAnimManager::GetAnimationBlockIndex(AnimBlockName);
            if (CTheScripts::ScriptResourceManager.RemoveFromResourceManager(AnimBlockIndex, 1, this))
            {
                CStreaming::SetMissionDoesntRequireAnimations(AnimBlockIndex);
            }
            return OR_CONTINUE;
        }
        case COMMAND_IS_CHAR_WAITING_FOR_WORLD_COLLISION:
        {
            CollectParameters(1);
            pPed = CPools::GetPedPool().GetAt(ScriptParams[0]);
            UpdateCompareFlag(pPed->m_bIsStaticWaitingForCollision);
            return OR_CONTINUE;
        }
        case COMMAND_IS_CAR_WAITING_FOR_WORLD_COLLISION:
        {
            CollectParameters(1);
            pVehicle = CPools::GetVehiclePool().GetAt(ScriptParams[0]);
            UpdateCompareFlag(pVehicle->m_bIsStaticWaitingForCollision);
            return OR_CONTINUE;
        }
        case COMMAND_ATTACH_CHAR_TO_OBJECT:
        {
            CollectParameters(8);
            pPed = CPools::GetPedPool().GetAt(ScriptParams[0]);
            pObj = CPools::GetObjectPool().GetAt(ScriptParams[1]);
            pPed->AttachPedToEntity(pObj, *(CVector*)&ScriptParams[2], (u16)ScriptParams[5], *(f32*)&ScriptParams[6] * 0.017453292f, (eWeaponType)ScriptParams[7]);
            return OR_CONTINUE;
        }
        case COMMAND_SET_CHAR_AS_PLAYER_FRIEND:
        {
            return OR_CONTINUE;
        }
        case COMMAND_DISPLAY_NTH_ONSCREEN_COUNTER_WITH_STRING:
        {
            u16 varId = GetIndexOfGlobalVariable();
            CollectParameters(2);
            ReadTextLabelFromScript(TextLabel, 8);
            (void)TheText.Get(TextLabel);
            CUserDisplay::OnscnTimer.AddCounter(varId, (u16)ScriptParams[0], TextLabel, (u16)ScriptParams[1] - 1);
            return OR_CONTINUE;
        }
        case COMMAND_ADD_SET_PIECE:
        {
            CollectParameters(13);
            CSetPieces::AddOne(static_cast<eSetPieceType>(ScriptParams[0]), CVector2D(*(f32*)&ScriptParams[1], *(f32*)&ScriptParams[2]), CVector2D(*(f32*)&ScriptParams[3], *(f32*)&ScriptParams[4]), CVector2D(*(f32*)&ScriptParams[5], *(f32*)&ScriptParams[6]), CVector2D(*(f32*)&ScriptParams[7], *(f32*)&ScriptParams[8]), CVector2D(*(f32*)&ScriptParams[9], *(f32*)&ScriptParams[10]), CVector2D(*(f32*)&ScriptParams[11], *(f32*)&ScriptParams[12]));
            return OR_CONTINUE;
        }
        case COMMAND_SET_EXTRA_COLOURS:
        {
            CollectParameters(2);
            CTimeCycle::StartExtraColour(ScriptParams[0] - 1, ScriptParams[1] != 0);
            return OR_CONTINUE;
        }
        case COMMAND_CLEAR_EXTRA_COLOURS:
        {
            CollectParameters(1);
            CTimeCycle::StopExtraColour(ScriptParams[0] != 0);
            return OR_CONTINUE;
        }
        case COMMAND_GET_WHEELIE_STATS:
        {
            CollectParameters(1);
            pPlayer = &CWorld::Players[ScriptParams[0]];
            ScriptParams[0] = pPlayer->nBestCarTwoWheelsTimeMs;
            *(f32*)&ScriptParams[1] = pPlayer->fBestCarTwoWheelsDistM;
            ScriptParams[2] = pPlayer->nBestBikeWheelieTimeMs;
            *(f32*)&ScriptParams[3] = pPlayer->fBestBikeWheelieDistM;
            ScriptParams[4] = pPlayer->nBestBikeStoppieTimeMs;
            *(f32*)&ScriptParams[5] = pPlayer->fBestBikeStoppieDistM;
            StoreParameters(6);
            pPlayer->nBestCarTwoWheelsTimeMs = 0;
            pPlayer->fBestCarTwoWheelsDistM = 0.0f;
            pPlayer->nBestBikeWheelieTimeMs = 0;
            pPlayer->fBestBikeWheelieDistM = 0.0f;
            pPlayer->nBestBikeStoppieTimeMs = 0;
            pPlayer->fBestBikeStoppieDistM = 0.0f;
            return OR_CONTINUE;
        }
        case COMMAND_BURST_CAR_TYRE:
        {
            CollectParameters(2);
            pVehicle = CPools::GetVehiclePool().GetAt(ScriptParams[0]);
            i32 tyre = ScriptParams[1];
            if (pVehicle->GetVehicleType() == VEHICLE_TYPE_BIKE)
            {
                if (tyre == 2)
                {
                    tyre = 0;
                }
                else if (tyre == 3)
                {
                    tyre = 1;
                }
            }
            pVehicle->BurstTyre(tyre, true);
            return OR_CONTINUE;
        }
        case COMMAND_IS_PLAYER_WEARING:
        {
            CollectParameters(2);
            u32 key = CWorld::Players[ScriptParams[0]].PlayerPedData.m_pClothes->GetTexture(ScriptParams[1]);
            ReadTextLabelFromScript(TextLabel, 16);
            UpdateCompareFlag(key == CKeyGen::GetUppercaseKey(TextLabel));
            return OR_CONTINUE;
        }
        case COMMAND_SET_PLAYER_CAN_DO_DRIVE_BY:
        {
            CollectParameters(2);
            CWorld::Players[ScriptParams[0]].bCanDoDriveBy = (ScriptParams[1] != 0);
            return OR_CONTINUE;
        }
        case COMMAND_CREATE_SWAT_ROPE:
        {
            CollectParameters(5);
            ePedType pedType = (ePedType)ScriptParams[0];
            i32 modelIndex = ScriptParams[1];
            CVector coors = *(CVector*)&ScriptParams[2];
            u32 ropeId = CRopes::CreateRopeForSwatPed(coors);
            CPed* swatPed = CPopulation::AddPed(pedType, modelIndex, coors, true);
            CTaskComplexUseSwatRope* swatRopeTask = new CTaskComplexUseSwatRope(ropeId);
            swatPed->m_pPedIntelligence->GetTaskManager().SetTask(swatRopeTask, 3, false);
            i32 vacantSlot = CPedScriptedTaskRecord::GetVacantSlot();
            CPedScriptedTaskRecord::ms_scriptedTasks[vacantSlot].Set(swatPed, CurrCommand, swatRopeTask);
            swatPed->SetCharCreatedBy(MISSION_CHAR);
            swatPed->bAllowMedicsToReviveMe = false;
            swatPed->m_pPedIntelligence->GetTaskManager().SetTask(new CTaskSimpleStandStill(999999, true, false, 8.0f), 4, false);
            CTheScripts::ClearSpaceForMissionEntity(coors, swatPed);
            if (IsThisAMissionScript)
            {
                swatPed->m_bIsStaticWaitingForCollision = true;
            }
            ++CPopulation::ms_nTotalMissionPeds;
            ScriptParams[0] = CPools::GetPedPool().GetIndex(swatPed);
            StoreParameters(1);
            if (IsThisAMissionScript)
            {
                CTheScripts::MissionCleanUp.AddEntityToList(ScriptParams[0], CLEANUP_CHAR);
            }
            return OR_CONTINUE;
        }
        case COMMAND_SET_CAR_MODEL_COMPONENTS:
        {
            CollectParameters(3);
            CVehicleModelInfo::ms_compsToUse[0] = ScriptParams[1];
            CVehicleModelInfo::ms_compsToUse[1] = ScriptParams[2];
            return OR_CONTINUE;
        }
        case COMMAND_SWITCH_LIFT_CAMERA:
        {
            return OR_CONTINUE;
        }
        case COMMAND_CLOSE_ALL_CAR_DOORS:
        {
            CollectParameters(1);
            pVehicle = CPools::GetVehiclePool().GetAt(ScriptParams[0]);
            ((CAutomobile*)pVehicle)->CloseAllDoors();
            return OR_CONTINUE;
        }
        case COMMAND_GET_DISTANCE_BETWEEN_COORDS_2D:
        {
            CollectParameters(4);
            f32 dx = *(f32*)&ScriptParams[0] - *(f32*)&ScriptParams[2];
            f32 dy = *(f32*)&ScriptParams[1] - *(f32*)&ScriptParams[3];
            *(f32*)&ScriptParams[0] = sqrtf(dx * dx + dy * dy);
            StoreParameters(1);
            return OR_CONTINUE;
        }
        case COMMAND_GET_DISTANCE_BETWEEN_COORDS_3D:
        {
            CollectParameters(6);
            CVector diff(*(f32*)&ScriptParams[0] - *(f32*)&ScriptParams[3], *(f32*)&ScriptParams[1] - *(f32*)&ScriptParams[4], *(f32*)&ScriptParams[2] - *(f32*)&ScriptParams[5]);
            *(f32*)&ScriptParams[0] = diff.Magnitude();
            StoreParameters(1);
            return OR_CONTINUE;
        }
        case COMMAND_POP_CAR_BOOT_USING_PHYSICS:
        {
            return OR_CONTINUE;
        }
        case COMMAND_SORT_OUT_OBJECT_COLLISION_WITH_CAR:
        {
            CollectParameters(2);
            pObj = CPools::GetObjectPool().GetAt(ScriptParams[0]);
            pVehicle = CPools::GetVehiclePool().GetAt(ScriptParams[1]);
            pObj->m_pNOCollisionVehicle = pVehicle;
            return OR_CONTINUE;
        }
        case COMMAND_GET_MAX_WANTED_LEVEL:
        {
            ScriptParams[0] = (i32)CWanted::MaximumWantedLevel;
            StoreParameters(1);
            return OR_CONTINUE;
        }
        case COMMAND_PRINT_HELP_FOREVER:
        {
            ReadTextLabelFromScript(TextLabel, 8);
            pString = TheText.Get(TextLabel);
            if (CTheScripts::bDisplayNonMiniGameHelpMessages || bIsThisAMiniGameScript || !CTheScripts::bMiniGameInProgress)
            {
                CHud::SetHelpMessage(pString, false, true, false);
                if (CTheScripts::bAddNextMessageToPreviousBriefs)
                {
                    CMessages::AddToPreviousBriefArray(pString, -1, -1, -1, -1, -1, -1, nullptr);
                }
            }
            CTheScripts::bAddNextMessageToPreviousBriefs = true;
            return OR_CONTINUE;
        }
        case COMMAND_PRINT_HELP_FOREVER_WITH_NUMBER:
        {
            ReadTextLabelFromScript(TextLabel, 8);
            pString = TheText.Get(TextLabel);
            CollectParameters(1);
            if (CTheScripts::bDisplayNonMiniGameHelpMessages || bIsThisAMiniGameScript || !CTheScripts::bMiniGameInProgress)
            {
                CHud::SetHelpMessageWithNumber(pString, ScriptParams[0], false, true);
            }
            return OR_CONTINUE;
        }
        default:
        {
            return OR_INTERRUPT;
        }
    }
}

// MARK: 1300To1399

// sa 0x48CDD0
OpcodeResult CRunningScript::ProcessCommands1300To1399(i32 CurrCommand)
{
    MARKFUNCTION(0x48CDD0);

    bool LatestCmpFlagResult;

    CPed* pPed;
    CPed* pSecondPed;

    CVehicle* pVehicle;
    CVehicle* pSecondVehicle;

    CObject* pObj;

    CPlayerInfo* pPlayer;

    f32 NewZ, NewY, NewX;

    char TextLabel[16];

    i32 PickupIndex, BlipIndex, ActualIndex;

    CVector TempCoors;

    switch (CurrCommand)
    {
        case COMMAND_LOAD_AND_LAUNCH_MISSION_EXCLUSIVE:
        {
            return OR_CONTINUE;
        }
        case COMMAND_CREATE_LOCKED_PROPERTY_PICKUP:
        {
            CollectParameters(3);
            NewX = *(f32*)&ScriptParams[0];
            NewY = *(f32*)&ScriptParams[1];
            NewZ = *(f32*)&ScriptParams[2];
            if (NewZ <= -100.0f)
            {
                NewZ = CWorld::FindGroundZForCoord(NewX, NewY) + 0.5f;
            }
            ReadTextLabelFromScript(TextLabel, 8);
            (void)TheText.Get(TextLabel);
            CPickups::GetActualPickupIndex(CollectNextParameterWithoutIncreasingPC());
            TempCoors = CVector(NewX, NewY, NewZ);
            PickupIndex = CPickups::GenerateNewOne(TempCoors, MI_PICKUP_PROPERTY, PICKUP_PROPERTY_LOCKED, 0, 0, false, TextLabel);
            ScriptParams[0] = PickupIndex;
            StoreParameters(1);
            return OR_CONTINUE;
        }
        case COMMAND_CREATE_FORSALE_PROPERTY_PICKUP:
        {
            CollectParameters(4);
            NewX = *(f32*)&ScriptParams[0];
            NewY = *(f32*)&ScriptParams[1];
            NewZ = *(f32*)&ScriptParams[2];
            if (NewZ <= -100.0f)
            {
                NewZ = CWorld::FindGroundZForCoord(NewX, NewY) + 0.5f;
            }
            ReadTextLabelFromScript(TextLabel, 8);
            (void)TheText.Get(TextLabel);
            CPickups::GetActualPickupIndex(CollectNextParameterWithoutIncreasingPC());
            TempCoors = CVector(NewX, NewY, NewZ);
            PickupIndex = CPickups::GenerateNewOne(TempCoors, MI_PICKUP_PROPERTY_FORSALE, PICKUP_PROPERTY_FORSALE, ScriptParams[3], 0, false, TextLabel);
            ScriptParams[0] = PickupIndex;
            StoreParameters(1);
            return OR_CONTINUE;
        }
        case COMMAND_FREEZE_CAR_POSITION:
        {
            CollectParameters(2);
            pVehicle = CPools::GetVehiclePool().GetAt(ScriptParams[0]);
            pVehicle->physicalFlags.bCoorsFrozenByScript = (ScriptParams[1] != 0);
            return OR_CONTINUE;
        }
        case COMMAND_HAS_CHAR_BEEN_DAMAGED_BY_CHAR:
        {
            CollectParameters(2);
            pPed = CPools::GetPedPool().GetAt(ScriptParams[0]);
            pSecondPed = nullptr;
            if (ScriptParams[1] != -1)
            {
                pSecondPed = CPools::GetPedPool().GetAt(ScriptParams[1]);
            }
            LatestCmpFlagResult = false;
            if (pPed && pPed->m_pLastEntityDamage)
            {
                if (ScriptParams[1] == -1)
                {
                    LatestCmpFlagResult = pPed->m_pLastEntityDamage->GetIsTypePed();
                }
                else
                {
                    LatestCmpFlagResult = (pPed->m_pLastEntityDamage == pSecondPed);
                    if (!LatestCmpFlagResult && pSecondPed && pSecondPed->bInVehicle)
                    {
                        LatestCmpFlagResult = (pPed->m_pLastEntityDamage == pSecondPed->m_pMyVehicle);
                    }
                }
            }
            UpdateCompareFlag(LatestCmpFlagResult);
            return OR_CONTINUE;
        }
        case COMMAND_HAS_CHAR_BEEN_DAMAGED_BY_CAR:
        {
            CollectParameters(2);
            pPed = CPools::GetPedPool().GetAt(ScriptParams[0]);
            pSecondVehicle = nullptr;
            if (ScriptParams[1] != -1)
            {
                pSecondVehicle = CPools::GetVehiclePool().GetAt(ScriptParams[1]);
            }
            LatestCmpFlagResult = false;
            if (pPed && pPed->m_pLastEntityDamage)
            {
                if (ScriptParams[1] == -1)
                {
                    LatestCmpFlagResult = pPed->m_pLastEntityDamage->GetIsTypeVehicle();
                }
                else
                {
                    LatestCmpFlagResult = (pPed->m_pLastEntityDamage == pSecondVehicle);
                }
            }
            UpdateCompareFlag(LatestCmpFlagResult);
            return OR_CONTINUE;
        }
        case COMMAND_HAS_CAR_BEEN_DAMAGED_BY_CHAR:
        {
            CollectParameters(2);
            pVehicle = CPools::GetVehiclePool().GetAt(ScriptParams[0]);
            pSecondPed = nullptr;
            if (ScriptParams[1] != -1)
            {
                pSecondPed = CPools::GetPedPool().GetAt(ScriptParams[1]);
            }
            LatestCmpFlagResult = false;
            if (pVehicle && pVehicle->pLastDamageEntity)
            {
                if (ScriptParams[1] == -1)
                {
                    LatestCmpFlagResult = pVehicle->pLastDamageEntity->GetIsTypePed();
                }
                else
                {
                    LatestCmpFlagResult = (pVehicle->pLastDamageEntity == pSecondPed);
                    if (!LatestCmpFlagResult && pSecondPed && pSecondPed->bInVehicle)
                    {
                        LatestCmpFlagResult = (pVehicle->pLastDamageEntity == pSecondPed->m_pMyVehicle);
                    }
                }
            }
            UpdateCompareFlag(LatestCmpFlagResult);
            return OR_CONTINUE;
        }
        case COMMAND_HAS_CAR_BEEN_DAMAGED_BY_CAR:
        {
            CollectParameters(2);
            pVehicle = CPools::GetVehiclePool().GetAt(ScriptParams[0]);
            pSecondVehicle = nullptr;
            if (ScriptParams[1] != -1)
            {
                pSecondVehicle = CPools::GetVehiclePool().GetAt(ScriptParams[1]);
            }
            LatestCmpFlagResult = false;
            if (pVehicle && pVehicle->pLastDamageEntity)
            {
                if (ScriptParams[1] == -1)
                {
                    LatestCmpFlagResult = pVehicle->pLastDamageEntity->GetIsTypeVehicle();
                }
                else
                {
                    LatestCmpFlagResult = (pVehicle->pLastDamageEntity == pSecondVehicle);
                }
            }
            UpdateCompareFlag(LatestCmpFlagResult);
            return OR_CONTINUE;
        }
        case COMMAND_GET_RADIO_CHANNEL:
        {
            ScriptParams[0] = AudioEngine.GetCurrentRadioStationID() - 1;
            StoreParameters(1);
            return OR_CONTINUE;
        }
        case COMMAND_IS_CHAR_DROWNING_IN_WATER:
        {
            return OR_CONTINUE;
        }
        case COMMAND_DISABLE_CUTSCENE_SHADOWS:
        {
            return OR_CONTINUE;
        }
        case COMMAND_HAS_GLASS_BEEN_SHATTERED_NEARBY:
        {
            return OR_CONTINUE;
        }
        case COMMAND_ATTACH_CUTSCENE_OBJECT_TO_BONE:
        {
            return OR_CONTINUE;
        }
        case COMMAND_ATTACH_CUTSCENE_OBJECT_TO_COMPONENT:
        {
            return OR_CONTINUE;
        }
        case COMMAND_SET_CHAR_STAY_IN_CAR_WHEN_JACKED:
        {
            CollectParameters(2);
            pPed = CPools::GetPedPool().GetAt(ScriptParams[0]);
            pPed->bStayInCarOnJack = (ScriptParams[1] != 0);
            return OR_CONTINUE;
        }
        case COMMAND_SET_PLAYER_DRUNKENNESS:
        {
            CollectParameters(2);
            pPlayer = &CWorld::Players[ScriptParams[0]];
            pPlayer->PlayerPedData.m_nDrunkenness = (u8)ScriptParams[1];
            pPlayer->PlayerPedData.m_bFadeDrunkenness = false;
            if ((ScriptParams[1] & 0xFF) == 0)
            {
                CMBlur::ClearDrunkBlur();
            }
            return OR_CONTINUE;
        }
        case COMMAND_GET_RANDOM_CAR_OF_TYPE_IN_AREA_NO_SAVE:
        {
            CollectParameters(5);
            ActualIndex = -1;
            i32 poolSize = CPools::GetVehiclePool().GetSize();
            if (poolSize > 0)
            {
                while (poolSize > 0)
                {
                    --poolSize;
                    if (ActualIndex != -1)
                    {
                        break;
                    }
                    pVehicle = CPools::GetVehiclePool().GetSlot(poolSize);
                    if (pVehicle)
                    {
                        if ((pVehicle->GetVehicleAppearance() == APR_CAR || pVehicle->GetVehicleAppearance() == APR_BIKE) && (pVehicle->m_nModelIndex == ScriptParams[4] || ScriptParams[4] < 0))
                        {
                            if (pVehicle->CanBeDeleted())
                            {
                                if (pVehicle->IsWithinArea(*(f32*)&ScriptParams[0], *(f32*)&ScriptParams[1], *(f32*)&ScriptParams[2], *(f32*)&ScriptParams[3]))
                                {
                                    ActualIndex = CPools::GetVehiclePool().GetIndex(pVehicle);
                                    pVehicle->vehicleFlags.bHasBeenOwnedByPlayer = true;
                                }
                            }
                        }
                    }
                }
            }
            ScriptParams[0] = ActualIndex;
            StoreParameters(1);
            return OR_CONTINUE;
        }
        case COMMAND_SET_CAN_BURST_CAR_TYRES:
        {
            CollectParameters(2);
            pVehicle = CPools::GetVehiclePool().GetAt(ScriptParams[0]);
            pVehicle->vehicleFlags.bTyresDontBurst = (ScriptParams[1] == 0);
            return OR_CONTINUE;
        }
        case COMMAND_SET_PLAYER_AUTO_AIM:
        {
            return OR_CONTINUE;
        }
        case COMMAND_FIRE_HUNTER_GUN:
        {
            CollectParameters(1);
            pVehicle = CPools::GetVehiclePool().GetAt(ScriptParams[0]);
            if (CTimer::m_snTimeInMilliseconds <= pVehicle->m_LastTimeGunFired + 150)
            {
                return OR_CONTINUE;
            }
            CWeapon weapon_buf(WEAPONTYPE_MINIGUN, 5000);
            CVector gunPos = CAutomobile::vecHunterGunPos;
            CVector moveOffset = pVehicle->m_vecMoveSpeed * CTimer::ms_fTimeStep;
            CVector firePos = Multiply3x3(pVehicle->GetMatrix(), gunPos) + pVehicle->GetPosition() + moveOffset;
            weapon_buf.FireInstantHit(pVehicle, &firePos, &firePos, nullptr, nullptr, nullptr, false, true);
            CVector2D shellDir(0.0f, 0.1f);
            weapon_buf.AddGunshell(pVehicle, firePos, shellDir, 0.025f);
            AudioEngine.ReportWeaponEvent(static_cast<tAudioEvent>(149), WEAPONTYPE_MINIGUN, pVehicle);
            pVehicle->m_LastTimeGunFired = CTimer::m_snTimeInMilliseconds;
            return OR_CONTINUE;
        }
        case COMMAND_REMOVE_EVERYTHING_FOR_HUGE_CUTSCENE:
        {
            return OR_CONTINUE;
        }
        case COMMAND_IS_CHAR_TOUCHING_VEHICLE:
        {
            CollectParameters(2);
            pPed = CPools::GetPedPool().GetAt(ScriptParams[0]);
            pVehicle = CPools::GetVehiclePool().GetAt(ScriptParams[1]);
            bool bTouching = false;
            if (pPed->bInVehicle)
            {
                bTouching = pPed->m_pMyVehicle->GetHasCollidedWith(pVehicle);
            }
            else
            {
                bTouching = pPed->GetHasCollidedWith(pVehicle);
            }
            UpdateCompareFlag(bTouching);
            return OR_CONTINUE;
        }
        case COMMAND_CHECK_FOR_PED_MODEL_AROUND_PLAYER:
        {
            return OR_CONTINUE;
        }
        case COMMAND_SET_CHAR_CAN_BE_SHOT_IN_VEHICLE:
        {
            CollectParameters(2);
            pPed = CPools::GetPedPool().GetAt(ScriptParams[0]);
            pPed->bCanBeShotInVehicle = (ScriptParams[1] != 0);
            return OR_CONTINUE;
        }
        case COMMAND_ATTACH_CUTSCENE_OBJECT_TO_VEHICLE:
        {
            return OR_CONTINUE;
        }
        case COMMAND_LOAD_MISSION_TEXT:
        {
            ReadTextLabelFromScript(TextLabel, 8);
            TheText.LoadMissionText(TextLabel);
            return OR_CONTINUE;
        }
        case COMMAND_SET_TONIGHTS_EVENT:
        {
            return OR_CONTINUE;
        }
        case COMMAND_CLEAR_CHAR_LAST_DAMAGE_ENTITY:
        {
            CollectParameters(1);
            pPed = CPools::GetPedPool().GetAt(ScriptParams[0]);
            if (pPed)
            {
                pPed->m_pLastEntityDamage = nullptr;
            }
            return OR_CONTINUE;
        }
        case COMMAND_CLEAR_CAR_LAST_DAMAGE_ENTITY:
        {
            CollectParameters(1);
            pVehicle = CPools::GetVehiclePool().GetAt(ScriptParams[0]);
            if (pVehicle)
            {
                pVehicle->pLastDamageEntity = nullptr;
            }
            return OR_CONTINUE;
        }
        case COMMAND_FREEZE_OBJECT_POSITION:
        {
            CollectParameters(2);
            pObj = CPools::GetObjectPool().GetAt(ScriptParams[0]);
            if (ScriptParams[1])
            {
                pObj->physicalFlags.bInfiniteMassFixed = true;
                pObj->physicalFlags.bCoorsFrozenByScript = true;
            }
            else
            {
                pObj->physicalFlags.bInfiniteMassFixed = false;
                pObj->physicalFlags.bCoorsFrozenByScript = false;
            }
            return OR_CONTINUE;
        }
        case COMMAND_SET_PLAYER_HAS_MET_DEBBIE_HARRY:
        {
            return OR_CONTINUE;
        }
        case COMMAND_SET_RIOT_INTENSITY:
        {
            return OR_CONTINUE;
        }
        case COMMAND_REMOVE_WEAPON_FROM_CHAR:
        {
            CollectParameters(2);
            pPed = CPools::GetPedPool().GetAt(ScriptParams[0]);
            pPed->ClearWeapon((eWeaponType)ScriptParams[1]);
            return OR_CONTINUE;
        }
        case COMMAND_SET_UP_TAXI_SHORTCUT:
        {
            return OR_CONTINUE;
        }
        case COMMAND_CLEAR_TAXI_SHORTCUT:
        {
            return OR_CONTINUE;
        }
        case COMMAND_CREATE_CLOTHES_PICKUP:
        {
            return OR_CONTINUE;
        }
        case COMMAND_MAKE_PLAYER_FIRE_PROOF:
        {
            CollectParameters(2);
            CWorld::Players[ScriptParams[0]].FireProof = (ScriptParams[1] != 0);
            return OR_CONTINUE;
        }
        case COMMAND_INCREASE_PLAYER_MAX_HEALTH:
        {
            CollectParameters(2);
            pPlayer = &CWorld::Players[ScriptParams[0]];
            pPlayer->MaxHealth += (u8)ScriptParams[1];
            pPlayer->m_pPed->m_nHealth = (f32)pPlayer->MaxHealth;
            return OR_CONTINUE;
        }
        case COMMAND_INCREASE_PLAYER_MAX_ARMOUR:
        {
            CollectParameters(2);
            pPlayer = &CWorld::Players[ScriptParams[0]];
            pPlayer->MaxArmour += (u8)ScriptParams[1];
            pPlayer->m_pPed->m_nArmour = (f32)pPlayer->MaxArmour;
            return OR_CONTINUE;
        }
        case COMMAND_CREATE_RANDOM_CHAR_AS_DRIVER:
        {
            CollectParameters(1);
            pVehicle = CPools::GetVehiclePool().GetAt(ScriptParams[0]);
            pPed = CPopulation::AddPedInCar(pVehicle, true, -1, 0, false, false);
            pPed->SetCharCreatedBy(MISSION_CHAR);
            pPed->m_pPedIntelligence->GetTaskManager().SetTask(nullptr, TASK_PRIMARY_PRIMARY, false);
            pPed->bAllowMedicsToReviveMe = false;
            ++CPopulation::ms_nTotalMissionPeds;
            ScriptParams[0] = CPools::GetPedPool().GetIndex(pPed);
            StoreParameters(1);
            if (IsThisAMissionScript)
            {
                CTheScripts::MissionCleanUp.AddEntityToList(ScriptParams[0], CLEANUP_CHAR);
            }
            return OR_CONTINUE;
        }
        case COMMAND_CREATE_RANDOM_CHAR_AS_PASSENGER:
        {
            CollectParameters(2);
            pVehicle = CPools::GetVehiclePool().GetAt(ScriptParams[0]);
            pPed = CPopulation::AddPedInCar(pVehicle, false, -1, ScriptParams[1], false, false);
            pPed->SetCharCreatedBy(MISSION_CHAR);
            pPed->bAllowMedicsToReviveMe = false;
            ++CPopulation::ms_nTotalMissionPeds;
            ScriptParams[0] = CPools::GetPedPool().GetIndex(pPed);
            StoreParameters(1);
            if (IsThisAMissionScript)
            {
                CTheScripts::MissionCleanUp.AddEntityToList(ScriptParams[0], CLEANUP_CHAR);
            }
            return OR_CONTINUE;
        }
        case COMMAND_ENSURE_PLAYER_HAS_DRIVE_BY_WEAPON:
        {
            CollectParameters(2);
            pPlayer = &CWorld::Players[ScriptParams[0]];
            pPed = pPlayer->m_pPed;
            if (!pPed->bInVehicle)
            {
                return OR_CONTINUE;
            }
            eWeaponType weaponType = pPed->m_WeaponSlots[4].m_eWeaponType;
            if (weaponType != WEAPONTYPE_UNARMED)
            {
                if (pPed->m_WeaponSlots[4].m_nAmmoTotal < (u32)ScriptParams[1])
                {
                    pPed->SetAmmo(weaponType, ScriptParams[1]);
                }
            }
            else
            {
                pPed->GiveWeapon(WEAPONTYPE_MICRO_UZI, ScriptParams[1], true);
                if (pPed->m_eStoredWeapon == WEAPONTYPE_UNIDENTIFIED)
                {
                    pPed->m_eStoredWeapon = pPed->m_WeaponSlots[pPed->m_nCurrentWeapon].m_eWeaponType;
                }
                pPed->SetCurrentWeapon(WEAPONTYPE_MICRO_UZI);
            }
            return OR_CONTINUE;
        }
        case COMMAND_MAKE_HELI_COME_CRASHING_DOWN:
        {
            CollectParameters(1);
            pVehicle = CPools::GetVehiclePool().GetAt(ScriptParams[0]);
            if (pVehicle->AutoPilot.Mission != MISSION_PLANE_CRASH_AND_BURN && pVehicle->AutoPilot.Mission != MISSION_HELI_CRASH_AND_BURN)
            {
                pVehicle->AutoPilot.Mission = MISSION_HELI_CRASH_AND_BURN;
            }
            return OR_CONTINUE;
        }
        case COMMAND_ADD_EXPLOSION_NO_SOUND:
        {
            CollectParameters(4);
            CExplosion::AddExplosion(nullptr, nullptr, (eExplosionType)ScriptParams[3], *(CVector*)&ScriptParams[0], 0, false, -1.0f, false);
            return OR_CONTINUE;
        }
        case COMMAND_SET_OBJECT_AREA_VISIBLE:
        {
            CollectParameters(2);
            pObj = CPools::GetObjectPool().GetAt(ScriptParams[0]);
            pObj->SetAreaCode(static_cast<eAreaCodes>((u8)ScriptParams[1]));
            return OR_CONTINUE;
        }
        case COMMAND_SET_CHAR_NEVER_TARGETTED:
        {
            CollectParameters(2);
            pPed = CPools::GetPedPool().GetAt(ScriptParams[0]);
            pPed->bNeverEverTargetThisPed = (ScriptParams[1] != 0);
            return OR_CONTINUE;
        }
        case COMMAND_LOAD_UNCOMPRESSED_ANIM:
        {
            return OR_CONTINUE;
        }
        case COMMAND_WAS_CUTSCENE_SKIPPED:
        {
            UpdateCompareFlag(CCutsceneMgr::WasCutsceneSkipped());
            return OR_CONTINUE;
        }
        case COMMAND_IS_CHAR_IN_ANY_POLICE_VEHICLE:
        {
            CollectParameters(1);
            pPed = CPools::GetPedPool().GetAt(ScriptParams[0]);
            LatestCmpFlagResult = false;
            if (pPed->bInVehicle && pPed->m_pMyVehicle && pPed->m_pMyVehicle->IsLawEnforcementVehicle() && pPed->m_pMyVehicle->m_nModelIndex != MODELID_BOAT_PREDATOR)
            {
                LatestCmpFlagResult = true;
            }
            UpdateCompareFlag(LatestCmpFlagResult);
            return OR_CONTINUE;
        }
        case COMMAND_DOES_CHAR_EXIST:
        {
            CollectParameters(1);
            UpdateCompareFlag(CPools::GetPedPool().GetAt(ScriptParams[0]) != nullptr);
            return OR_CONTINUE;
        }
        case COMMAND_DOES_VEHICLE_EXIST:
        {
            CollectParameters(1);
            UpdateCompareFlag(CPools::GetVehiclePool().GetAt(ScriptParams[0]) != nullptr);
            return OR_CONTINUE;
        }
        case COMMAND_ADD_SHORT_RANGE_SPRITE_BLIP_FOR_CONTACT_POINT:
        {
            CollectParameters(4);
            NewX = *(f32*)&ScriptParams[0];
            NewY = *(f32*)&ScriptParams[1];
            NewZ = *(f32*)&ScriptParams[2];
            if (NewZ <= -100.0f)
            {
                NewZ = CWorld::FindGroundZForCoord(NewX, NewY);
            }
            CRadar::GetActualBlipArrayIndex(CollectNextParameterWithoutIncreasingPC());
            CVector BlipCoors(NewX, NewY, NewZ);
            BlipIndex = CRadar::SetShortRangeCoordBlip(BLIPTYPE_CONTACT, BlipCoors, 2, BLIPDISPLAY_BOTH, ScriptName);
            CRadar::SetBlipSprite(BlipIndex, ScriptParams[3]);
            ScriptParams[0] = BlipIndex;
            StoreParameters(1);
            return OR_CONTINUE;
        }
        case COMMAND_SET_ALL_TAXIS_HAVE_NITRO:
        {
            CollectParameters(1);
            if (ScriptParams[0])
            {
                CCheat::EnableLegimateCheat(TAXINITRO_CHEAT);
            }
            else
            {
                CCheat::Disable(static_cast<eCheats>(TAXINITRO_CHEAT));
            }
            return OR_CONTINUE;
        }
        case COMMAND_FREEZE_CAR_POSITION_AND_DONT_LOAD_COLLISION:
        {
            CollectParameters(2);
            pVehicle = CPools::GetVehiclePool().GetAt(ScriptParams[0]);
            if (ScriptParams[1])
            {
                pVehicle->physicalFlags.bInfiniteMassFixed = true;
                pVehicle->physicalFlags.bCoorsFrozenByScript = true;
                if (IsThisAMissionScript)
                {
                    CWorld::Remove(pVehicle);
                    pVehicle->m_bIsStaticWaitingForCollision = true;
                    CWorld::Add(pVehicle);
                }
            }
            else
            {
                pVehicle->physicalFlags.bInfiniteMassFixed = false;
                pVehicle->physicalFlags.bCoorsFrozenByScript = false;
                pVehicle->m_nNoOfStaticFrames = 0;
            }
            return OR_CONTINUE;
        }
        case COMMAND_FREEZE_CHAR_POSITION_AND_DONT_LOAD_COLLISION:
        {
            CollectParameters(2);
            pPed = CPools::GetPedPool().GetAt(ScriptParams[0]);
            if (ScriptParams[1])
            {
                pPed->physicalFlags.bCoorsFrozenByScript = true;
                if (IsThisAMissionScript)
                {
                    CWorld::Remove(pPed);
                    pPed->m_bIsStaticWaitingForCollision = true;
                    CWorld::Add(pPed);
                }
            }
            else
            {
                pPed->physicalFlags.bCoorsFrozenByScript = false;
            }
            return OR_CONTINUE;
        }
        default:
        {
            return OR_INTERRUPT;
        }
    }
}

// MARK: 1400To1499

// func: sa 0x48EAA0
OpcodeResult CRunningScript::ProcessCommands1400To1499(i32 CurrCommand)
{
    MARKFUNCTION(0x48EAA0);

    bool LatestCmpFlagResult;
    CPed* pPed;
    CVehicle* pVehicle;
    CObject* pObj;
    CPlayerInfo* pPlayer;
    CVector TempCoors;
    CVector TempVec;
    CVector NewCoors;
    CVector ObjUpVec, ObjRightVec, ObjForwardVec;
    CVector rightVec, upVec, forwardVec;
    CVector VelocityVector;
    char ZoneLabel[10];
    char debug_string[130];
    u64 FindZoneInt;
    f32 ZComponent, YComponent, XComponent;
    f32 MinX, MinY, MinZ;
    f32 MaxX, MaxZ, MaxY;
    f32 temp_float;
    i16 temp_int;
    i16 temp_int2;
    f32 temp_float4, temp_float3, temp_float2, temp_float1;
    f32 temp_float6, temp_float7, temp_float8, temp_float5;
    f32 temp_float10, temp_float9;
    f32 NewX, NewY;
    i16 CardStackAddresses[312];
    i16 CardStackSpacesAvailable;
    f32 fResult;
    f32 top1, bottom1, right1, left1;
    f32 bottom2, top2, left2, right2;
    CVector CurrentObjectSpeed;
    char TextLabel[10];
    i32* pGlobalVar;
    i32* pLocalVar;
    f32 m2, m1;
    f32 c2, c1;
    f32 d, b, c, a;
    f32 determinant1, determinant2, determinant3;
    CTask* pTask;
    CTask* pTaskSecond;

    switch (CurrCommand)
    {
        case COMMAND_WANTED_STARS_ARE_FLASHING:
        {
            return OR_CONTINUE;
        }
        case COMMAND_PLAY_ANNOUNCEMENT:
        {
            return OR_CONTINUE;
        }
        case COMMAND_SET_PLAYER_IS_IN_STADIUM:
        {
            CollectParameters(1);
            CTheScripts::bPlayerIsOffTheMap = (ScriptParams[0] != 0);
            return OR_CONTINUE;
        }
        case COMMAND_GET_BUS_FARES_COLLECTED_BY_PLAYER:
        {
            return OR_CONTINUE;
        }
        case COMMAND_DISPLAY_RADAR:
        {
            CollectParameters(1);
            CHud::bScriptDontDisplayRadar = (ScriptParams[0] == 0);
            return OR_CONTINUE;
        }
        case COMMAND_REGISTER_BEST_POSITION:
        {
            CollectParameters(2);
            CStats::RegisterBestPosition(ScriptParams[0], ScriptParams[1]);
            return OR_CONTINUE;
        }
        case COMMAND_IS_PLAYER_IN_INFO_ZONE:
            CollectParameters(1);
            pPlayer = &CWorld::Players[ScriptParams[0]];
            ReadTextLabelFromScript(ZoneLabel, 8);
            FindZoneInt = 0;
            strcpy(reinterpret_cast<char*>(&FindZoneInt), ZoneLabel);
            TempCoors = pPlayer->GetPos();
            LatestCmpFlagResult = CTheZones::DoesPointLieWithinZoneWithGivenName(&TempCoors, FindZoneInt, ZONE_INFORMATION);
            UpdateCompareFlag(LatestCmpFlagResult);
            return OR_CONTINUE;
        case COMMAND_IS_IN_CAR_FIRE_BUTTON_PRESSED:
        {
            return OR_CONTINUE;
        }
        case COMMAND_SET_LOAD_COLLISION_FOR_CAR_FLAG:
        {
            CollectParameters(2);
            CPhysical* pPhysical = CPools::GetVehiclePool().GetAt(ScriptParams[0]);
            if (ScriptParams[1])
            {
                pPhysical->physicalFlags.bDontLoadCollision = false;
                if (IsThisAMissionScript)
                {
                    CWorld::Remove(pPhysical);
                    pPhysical->m_bIsStaticWaitingForCollision = true;
                    CWorld::Add(pPhysical);
                }
            }
            else
            {
                pPhysical->physicalFlags.bDontLoadCollision = true;
                if (pPhysical->m_bIsStaticWaitingForCollision)
                {
                    pPhysical->m_bIsStaticWaitingForCollision = false;
                    if (!pPhysical->GetIsStatic())
                    {
                        pPhysical->AddToMovingList();
                    }
                }
            }
            return OR_CONTINUE;
        }
        case COMMAND_SET_LOAD_COLLISION_FOR_CHAR_FLAG:
        {
            CollectParameters(2);
            CPhysical* pPhysical = CPools::GetPedPool().GetAt(ScriptParams[0]);
            if (ScriptParams[1])
            {
                pPhysical->physicalFlags.bDontLoadCollision = false;
                if (IsThisAMissionScript)
                {
                    CWorld::Remove(pPhysical);
                    pPhysical->m_bIsStaticWaitingForCollision = true;
                    CWorld::Add(pPhysical);
                }
            }
            else
            {
                pPhysical->physicalFlags.bDontLoadCollision = true;
                if (pPhysical->m_bIsStaticWaitingForCollision)
                {
                    pPhysical->m_bIsStaticWaitingForCollision = false;
                    if (!pPhysical->GetIsStatic())
                    {
                        pPhysical->AddToMovingList();
                    }
                }
            }
            return OR_CONTINUE;
        }
        case COMMAND_ADD_BIG_GUN_FLASH:
        {
            CollectParameters(6);
            TempCoors = *(CVector*)&ScriptParams[0];
            TempVec.x = *(f32*)&ScriptParams[3] - *(f32*)&ScriptParams[0];
            TempVec.y = *(f32*)&ScriptParams[4] - *(f32*)&ScriptParams[1];
            TempVec.z = *(f32*)&ScriptParams[5] - *(f32*)&ScriptParams[2];
            TempVec.Normalise();
            CPointLights::AddLight(0, TempCoors, CVector(0.0f, 0.0f, 0.0f), 5.0f, 1.0f, 0.8f, 0.0f, 0, false, nullptr);
            g_fx.TriggerGunshot(nullptr, TempCoors, TempVec, true);
            return OR_CONTINUE;
        }
        case COMMAND_GET_PROGRESS_PERCENTAGE:
        {
            *(f32*)&ScriptParams[0] = CStats::GetPercentageProgress();
            StoreParameters(1);
            return OR_CONTINUE;
        }
        case COMMAND_SET_SHORTCUT_PICKUP_POINT:
        {
            return OR_CONTINUE;
        }
        case COMMAND_SET_SHORTCUT_DROPOFF_POINT_FOR_MISSION:
        {
            return OR_CONTINUE;
        }
        case COMMAND_UNLOCK_ALL_CAR_DOORS_IN_AREA:
        {
            return OR_CONTINUE;
        }
        case COMMAND_SET_GANG_ATTACK_PLAYER_WITH_COPS:
        {
            return OR_CONTINUE;
        }
        case COMMAND_SET_VEHICLE_TO_FADE_IN:
        {
            CollectParameters(2);
            pVehicle = CPools::GetVehiclePool().GetAt(ScriptParams[0]);
            CVisibilityPlugins::SetClumpAlpha(reinterpret_cast<RpClump*>(pVehicle->GetRwObject()), ScriptParams[1]);
            return OR_CONTINUE;
        }
        case COMMAND_REGISTER_ODDJOB_MISSION_PASSED:
        {
            CStats::IncrementStat(0x80, 1.0f);
            temp_float = CStats::GetStatValue(0x12B);
            CStats::IncrementStat(0x12A, temp_float);
            CStats::SetStatValue(0x12B, 0.0f);
            CTheScripts::LastMissionPassedTime = CTimer::m_snTimeInMilliseconds;
            return OR_CONTINUE;
        }
        case COMMAND_IS_PLAYER_IN_SHORTCUT_TAXI:
        {
            CollectParameters(1);
            UpdateCompareFlag(false);
            return OR_CONTINUE;
        }
        case COMMAND_IS_CHAR_DUCKING:
        {
            CollectParameters(1);
            pPed = CPools::GetPedPool().GetAt(ScriptParams[0]);
            UpdateCompareFlag(pPed->bIsDucking != 0);
            return OR_CONTINUE;
        }
        case COMMAND_IS_AUSTRALIAN_GAME:
        {
            UpdateCompareFlag(false);
            return OR_CONTINUE;
        }
        case COMMAND_DISARM_CAR_BOMB:
        {
            return OR_CONTINUE;
        }
        case COMMAND_SET_ONSCREEN_COUNTER_FLASH_WHEN_FIRST_DISPLAYED:
        {
            u16 IndexOfGlobalVariable = GetIndexOfGlobalVariable();
            CollectParameters(1);
            if (ScriptParams[0])
            {
                CUserDisplay::OnscnTimer.SetCounterFlashWhenFirstDisplayed(IndexOfGlobalVariable, true);
            }
            else
            {
                CUserDisplay::OnscnTimer.SetCounterFlashWhenFirstDisplayed(IndexOfGlobalVariable, false);
            }
            return OR_CONTINUE;
        }
        case COMMAND_SHUFFLE_CARD_DECKS:
        {
            CollectParameters(1);
            memset(CTheScripts::CardStack.data(), 0, sizeof(CTheScripts::CardStack));
            for (temp_int = 0; temp_int < 312; temp_int++)
            {
                CardStackAddresses[temp_int] = temp_int;
            }
            CardStackSpacesAvailable = (i16)(52 * (i16)ScriptParams[0]);
            for (temp_int2 = 1; temp_int2 < 53; temp_int2++)
            {
                temp_int = 0;
                if (ScriptParams[0] > 0)
                {
                    do
                    {
                        i32 cardIndex = CGeneral::GetRandomNumberInRange(0, (i32)CardStackSpacesAvailable);
                        CTheScripts::CardStack[CardStackAddresses[cardIndex]] = temp_int2;
                        if (cardIndex < CardStackSpacesAvailable)
                        {
                            do
                            {
                                if (cardIndex <= 310)
                                {
                                    CardStackAddresses[cardIndex] = CardStackAddresses[cardIndex + 1];
                                }
                                else
                                {
                                    CardStackAddresses[cardIndex] = 0;
                                }
                                cardIndex++;
                            } while (cardIndex < CardStackSpacesAvailable);
                        }
                        CardStackSpacesAvailable--;
                        temp_int++;
                    } while (temp_int < ScriptParams[0]);
                }
            }
            CTheScripts::CardStackPosition = 0;
            return OR_CONTINUE;
        }
        case COMMAND_FETCH_NEXT_CARD:
        {
            i16 pos = CTheScripts::CardStackPosition;
            if (!CTheScripts::CardStack[CTheScripts::CardStackPosition])
            {
                pos = 0;
            }
            ScriptParams[0] = CTheScripts::CardStack[pos];
            CTheScripts::CardStackPosition = pos + 1;
            if (pos == 311)
            {
                CTheScripts::CardStackPosition = 0;
            }
            StoreParameters(1);
            return OR_CONTINUE;
        }
        case COMMAND_GET_OBJECT_VELOCITY:
        {
            CollectParameters(1);
            pObj = CPools::GetObjectPool().GetAt(ScriptParams[0]);
            *(f32*)&ScriptParams[0] = pObj->m_vecMoveSpeed.x * 50.0f;
            *(f32*)&ScriptParams[1] = pObj->m_vecMoveSpeed.y * 50.0f;
            *(f32*)&ScriptParams[2] = pObj->m_vecMoveSpeed.z * 50.0f;
            StoreParameters(3);
            return OR_CONTINUE;
        }
        case COMMAND_IS_DEBUG_CAMERA_ON:
        {
            UpdateCompareFlag(TheCamera.WorldViewerBeingUsed);
            return OR_CONTINUE;
        }
        case COMMAND_ADD_TO_OBJECT_ROTATION_VELOCITY:
            CollectParameters(4);
            pObj = CPools::GetObjectPool().GetAt(ScriptParams[0]);
            TempVec = pObj->GetTurnSpeed();
            VelocityVector = CVector(*reinterpret_cast<f32*>(&ScriptParams[1]) * 0.02f, *reinterpret_cast<f32*>(&ScriptParams[2]) * 0.02f,
                *reinterpret_cast<f32*>(&ScriptParams[3]) * 0.02f);
            VelocityVector = Multiply3x3(pObj->GetMatrix(), VelocityVector);
            TempVec.x += VelocityVector.x;
            TempVec.y += VelocityVector.y;
            TempVec.z += VelocityVector.z;
            if (pObj->m_bIsStatic)
            {
                pObj->SetIsStatic(false);
                pObj->AddToMovingList();
            }
            pObj->SetTurnSpeed(TempVec);
            return OR_CONTINUE;
        case COMMAND_SET_OBJECT_ROTATION_VELOCITY:
            CollectParameters(4);
            pObj = CPools::GetObjectPool().GetAt(ScriptParams[0]);
            VelocityVector = CVector(CTimer::GetTimeStep() * *reinterpret_cast<f32*>(&ScriptParams[1]),
                CTimer::GetTimeStep() * *reinterpret_cast<f32*>(&ScriptParams[2]), CTimer::GetTimeStep() * *reinterpret_cast<f32*>(&ScriptParams[3]));
            VelocityVector = Multiply3x3(pObj->GetMatrix(), VelocityVector);
            if (pObj->m_bIsStatic)
            {
                pObj->SetIsStatic(false);
                pObj->AddToMovingList();
            }
            pObj->SetTurnSpeed(VelocityVector.x * 0.02f, VelocityVector.y * 0.02f, VelocityVector.z * 0.02f);
            return OR_CONTINUE;
        case COMMAND_IS_OBJECT_STATIC:
        {
            CollectParameters(1);
            pObj = CPools::GetObjectPool().GetAt(ScriptParams[0]);
            UpdateCompareFlag(pObj->GetIsStatic());
            return OR_CONTINUE;
        }
        case COMMAND_GET_ANGLE_BETWEEN_2D_VECTORS:
        {
            CollectParameters(4);
            f32 x1 = *(f32*)&ScriptParams[0];
            f32 y1 = *(f32*)&ScriptParams[1];
            f32 x2 = *(f32*)&ScriptParams[2];
            f32 y2 = *(f32*)&ScriptParams[3];
            *(f32*)&ScriptParams[0] = acos((x2 * x1 + y2 * y1) / (sqrt(x2 * x2 + y2 * y2) * sqrt(x1 * x1 + y1 * y1))) * 57.295776f;
            StoreParameters(1);
            return OR_CONTINUE;
        }
        case COMMAND_DO_2D_RECTANGLES_COLLIDE:
            CollectParameters(8);
            temp_float1 = *reinterpret_cast<f32*>(&ScriptParams[0]);
            temp_float2 = *reinterpret_cast<f32*>(&ScriptParams[1]);
            temp_float3 = *reinterpret_cast<f32*>(&ScriptParams[2]);
            temp_float4 = *reinterpret_cast<f32*>(&ScriptParams[3]);
            temp_float5 = *reinterpret_cast<f32*>(&ScriptParams[4]);
            temp_float6 = *reinterpret_cast<f32*>(&ScriptParams[5]);
            temp_float7 = *reinterpret_cast<f32*>(&ScriptParams[6]);
            temp_float8 = *reinterpret_cast<f32*>(&ScriptParams[7]);
            bottom1 = temp_float2 - temp_float4 * 0.5f;
            top1 = temp_float2 + temp_float4 * 0.5f;
            left1 = temp_float1 - temp_float3 * 0.5f;
            right1 = temp_float1 + temp_float3 * 0.5f;
            bottom2 = temp_float6 - temp_float8 * 0.5f;
            top2 = temp_float6 + temp_float8 * 0.5f;
            left2 = temp_float5 - temp_float7 * 0.5f;
            right2 = temp_float5 + temp_float7 * 0.5f;
            LatestCmpFlagResult = true;
            if (bottom2 > top1)
            {
                LatestCmpFlagResult = false;
            }
            if (bottom1 > top2)
            {
                LatestCmpFlagResult = false;
            }
            if (right1 < left2)
            {
                LatestCmpFlagResult = false;
            }
            if (left1 > right2)
            {
                LatestCmpFlagResult = false;
            }
            UpdateCompareFlag(LatestCmpFlagResult);
            return OR_CONTINUE;
        case COMMAND_GET_OBJECT_ROTATION_VELOCITY:
        {
            CollectParameters(1);
            pObj = CPools::GetObjectPool().GetAt(ScriptParams[0]);
            TempVec = Multiply3x3(pObj->m_vecTurnSpeed, pObj->GetMatrix());
            *(f32*)&ScriptParams[0] = TempVec.x * 50.0f;
            *(f32*)&ScriptParams[1] = TempVec.y * 50.0f;
            *(f32*)&ScriptParams[2] = TempVec.z * 50.0f;
            StoreParameters(3);
            return OR_CONTINUE;
        }
        case COMMAND_ADD_VELOCITY_RELATIVE_TO_OBJECT_VELOCITY:
            CollectParameters(4);
            pObj = CPools::GetObjectPool().GetAt(ScriptParams[0]);
            VelocityVector = CVector(*reinterpret_cast<f32*>(&ScriptParams[1]), *reinterpret_cast<f32*>(&ScriptParams[2]),
                *reinterpret_cast<f32*>(&ScriptParams[3]));
            VelocityVector *= CTimer::GetTimeStep();
            VelocityVector /= 50.0f;
            if (pObj->m_bIsStatic)
            {
                return OR_CONTINUE;
            }
            CurrentObjectSpeed = pObj->GetMoveSpeed();
            CurrentObjectSpeed.Normalise();
            ObjForwardVec = CurrentObjectSpeed;
            if (CurrentObjectSpeed.z == 1.0f)
            {
                return OR_CONTINUE;
            }
            TempVec = CVector(0.0f, 0.0f, 1.0f);
            ObjUpVec = CVector(0.0f, 0.0f, 1.0f);
            ObjRightVec = CrossProduct(CurrentObjectSpeed, ObjUpVec);
            ObjRightVec.Normalise();
            ObjUpVec = CrossProduct(ObjRightVec, ObjForwardVec);
            ObjUpVec.Normalise();
            CurrentObjectSpeed = pObj->GetMoveSpeed();
            CurrentObjectSpeed += ObjRightVec * VelocityVector.x;
            CurrentObjectSpeed += ObjForwardVec * VelocityVector.y;
            CurrentObjectSpeed += ObjUpVec * VelocityVector.z;
            if (pObj->m_bIsStatic)
            {
                pObj->SetIsStatic(false);
                pObj->AddToMovingList();
            }
            pObj->SetMoveSpeed(CurrentObjectSpeed);
            return OR_CONTINUE;
        case COMMAND_GET_OBJECT_SPEED:
        {
            CollectParameters(1);
            pObj = CPools::GetObjectPool().GetAt(ScriptParams[0]);
            *(f32*)&ScriptParams[0] = pObj->m_vecMoveSpeed.Magnitude() * 50.0f;
            StoreParameters(1);
            return OR_CONTINUE;
        }
        case COMMAND_SET_VAR_TEXT_LABEL:
        {
            pGlobalVar = (i32*)GetPointerToScriptVariable(SCOPE_GLOBAL);
            ReadTextLabelFromScript(TextLabel, 8);
            strncpy((char*)pGlobalVar, TextLabel, 8);
            return OR_CONTINUE;
        }
        case COMMAND_SET_LVAR_TEXT_LABEL:
        {
            pLocalVar = (i32*)GetPointerToScriptVariable(SCOPE_LOCAL);
            ReadTextLabelFromScript(TextLabel, 8);
            strncpy((char*)pLocalVar, TextLabel, 8);
            return OR_CONTINUE;
        }
        case COMMAND_IS_VAR_TEXT_LABEL_EQUAL_TO_TEXT_LABEL:
        {
            pGlobalVar = (i32*)GetPointerToScriptVariable(SCOPE_GLOBAL);
            ReadTextLabelFromScript(TextLabel, 8);
            UpdateCompareFlag(strncmp((char*)pGlobalVar, TextLabel, 8) == 0);
            return OR_CONTINUE;
        }
        case COMMAND_IS_LVAR_TEXT_LABEL_EQUAL_TO_TEXT_LABEL:
        {
            pLocalVar = (i32*)GetPointerToScriptVariable(SCOPE_LOCAL);
            ReadTextLabelFromScript(TextLabel, 8);
            UpdateCompareFlag(strncmp((char*)pLocalVar, TextLabel, 8) == 0);
            return OR_CONTINUE;
        }
        case COMMAND_GET_2D_LINES_INTERSECT_POINT:
            CollectParameters(8);
            left1 = *reinterpret_cast<f32*>(&ScriptParams[0]);
            bottom1 = *reinterpret_cast<f32*>(&ScriptParams[1]);
            right1 = *reinterpret_cast<f32*>(&ScriptParams[2]);
            top1 = *reinterpret_cast<f32*>(&ScriptParams[3]);
            left2 = *reinterpret_cast<f32*>(&ScriptParams[4]);
            bottom2 = *reinterpret_cast<f32*>(&ScriptParams[5]);
            right2 = *reinterpret_cast<f32*>(&ScriptParams[6]);
            top2 = *reinterpret_cast<f32*>(&ScriptParams[7]);
            LatestCmpFlagResult = true;
            temp_float1 = left1 - right1;
            if (temp_float1 == 0.0f)
            {
                temp_float2 = 0.000001f;
            }
            else
            {
                temp_float2 = temp_float1;
            }
            temp_float3 = left2 - right2;
            if (temp_float3 == 0.0f)
            {
                temp_float3 = 0.000001f;
            }
            temp_float4 = bottom1 - top1;
            m1 = temp_float4 / temp_float2;
            m2 = (bottom2 - top2) / temp_float3;
            c1 = bottom1 - m1 * left1;
            c2 = bottom2 - m2 * left2;
            a = -m1;
            b = -m2;
            determinant1 = a - b;
            determinant2 = a * c2 - b * c1;
            if (determinant1 == 0.0f)
            {
                NewX = -1000000.0f;
                NewY = -1000000.0f;
                LatestCmpFlagResult = false;
            }
            else
            {
                NewX = temp_float1 == 0.0f ? left1 : (c1 - c2) / determinant1;
                NewY = temp_float4 == 0.0f ? bottom1 : determinant2 / determinant1;
                if (left1 > right1)
                {
                    temp_float = left1;
                    left1 = right1;
                    right1 = temp_float;
                }
                if (bottom1 > top1)
                {
                    temp_float = bottom1;
                    bottom1 = top1;
                    top1 = temp_float;
                }
                if (left2 > right2)
                {
                    temp_float = left2;
                    left2 = right2;
                    right2 = temp_float;
                }
                if (bottom2 > top2)
                {
                    temp_float = bottom2;
                    bottom2 = top2;
                    top2 = temp_float;
                }
                if (left1 - 0.01f > NewX || right1 + 0.01f < NewX || left2 - 0.01f > NewX || right2 + 0.01f < NewX || bottom1 - 0.01f > NewY ||
                    top1 + 0.01f < NewY || bottom2 - 0.01f > NewY || top2 + 0.01f < NewY)
                {
                    NewX = -1000000.0f;
                    NewY = -1000000.0f;
                    LatestCmpFlagResult = false;
                }
            }
            UpdateCompareFlag(LatestCmpFlagResult);
            *reinterpret_cast<f32*>(&ScriptParams[0]) = NewX;
            *reinterpret_cast<f32*>(&ScriptParams[1]) = NewY;
            StoreParameters(2);
            return OR_CONTINUE;
        case COMMAND_IS_2D_POINT_IN_TRIANGLE:
        {
            return OR_CONTINUE;
        }
        case COMMAND_IS_2D_POINT_IN_RECTANGLE_ON_LEFT_SIDE_OF_LINE:
        {
            return OR_CONTINUE;
        }
        case COMMAND_IS_2D_POINT_ON_LEFT_SIDE_OF_2D_LINE:
        {
            return OR_CONTINUE;
        }
        case COMMAND_APPLY_COLLISION_ON_OBJECT:
        {
            return OR_CONTINUE;
        }
        case COMMAND_SAVE_STRING_TO_DEBUG_FILE:
        {
            for (temp_int = 0; temp_int < 128; temp_int++)
            {
                debug_string[temp_int] = *(char*)PCPointer++;
            }
            return OR_CONTINUE;
        }
        case COMMAND_TASK_PLAYER_ON_FOOT:
        {
            return OR_CONTINUE;
        }
        case COMMAND_TASK_PLAYER_IN_CAR:
        {
            return OR_CONTINUE;
        }
        case COMMAND_TASK_PAUSE:
        {
            CollectParameters(2);
            const i32 iPedID = ScriptParams[0];
            const i32 iPauseTime = ScriptParams[1];
            GivePedScriptedTask(iPedID, new CTaskSimplePause(iPauseTime), CurrCommand);
            return OR_CONTINUE;
        }
        case COMMAND_TASK_STAND_STILL:
        {
            CollectParameters(2);
            const i32 iPedID = ScriptParams[0];
            const i32 iStandStillTime = ScriptParams[1];
            if (iStandStillTime == -1)
            {
                pTask = new CTaskSimpleStandStill(0, false, false, 8.0f);
            }
            else if (iStandStillTime == -2)
            {
                pTask = new CTaskSimpleStandStill(999999, true, false, 8.0f);
            }
            else
            {
                pTask = new CTaskSimpleStandStill(iStandStillTime, false, false, 8.0f);
            }
            GivePedScriptedTask(iPedID, pTask, CurrCommand);
            return OR_CONTINUE;
        }
        case COMMAND_TASK_FALL_AND_GET_UP:
        {
            CollectParameters(3);
            const i32 iPedID = ScriptParams[0];
            const i32 iDirection = ScriptParams[1];
            const i32 iPauseTime = ScriptParams[2];
            GivePedScriptedTask(iPedID, new CTaskComplexFallAndGetUp(iDirection, iPauseTime), CurrCommand);
            return OR_CONTINUE;
        }
        case COMMAND_TASK_JUMP:
        {
            CollectParameters(2);
            const i32 iPedID = ScriptParams[0];
            const i32 useplayerlaunchforce = ScriptParams[1];
            CTaskComplexJump* pTaskJump = new CTaskComplexJump(CTaskComplexJump::eForceClimb::OK);
            pTaskJump->SetUsePlayerLaunchForce(useplayerlaunchforce != 0);
            GivePedScriptedTask(iPedID, pTaskJump, CurrCommand);
            return OR_CONTINUE;
        }
        case COMMAND_TASK_TIRED:
        {
            CollectParameters(2);
            const i32 iPedID = ScriptParams[0];
            const i32 iTiredTime = ScriptParams[1];
            GivePedScriptedTask(iPedID, new CTaskSimpleTired(iTiredTime), CurrCommand);
            return OR_CONTINUE;
        }
        case COMMAND_TASK_DIE:
        {
            CollectParameters(1);
            const i32 iPedID = ScriptParams[0];
            GivePedScriptedTask(iPedID, new CTaskComplexDie(WEAPONTYPE_UNARMED, (AssocGroupId)0, (AnimationId)1, 4.0f, 0.0f, false, false, 0, false),
                CurrCommand);
            return OR_CONTINUE;
        }
        case COMMAND_TASK_LOOK_AT_CHAR:
        {
            CollectParameters(3);
            const i32 iPedID = ScriptParams[0];
            const i32 iOtherPedID = ScriptParams[1];
            i32 iTime = ScriptParams[2];
            if (iTime == -1)
            {
                iTime = 20000;
            }
            else if (iTime == -2)
            {
                iTime = -1;
            }
            pPed = CPools::GetPedPool().GetAt(iOtherPedID);
            if (iPedID != -1)
            {
                g_ikChainMan.LookAt("COMMAND_TASK_LOOK_AT_CHAR", CPools::GetPedPool().GetAt(iPedID), pPed, iTime, static_cast<eBoneTag>(5), nullptr, false, 0.25f, 500, 6,
                    true);
            }
            else
            {
                RwV3d offset;
                offset.x = 0.0f;
                offset.y = 0.0f;
                offset.z = 0.0f;
                CTaskSequences::ms_taskSequence[CTaskSequences::ms_iActiveSequence].AddTask(
                    new CTaskSimpleTriggerLookAt(pPed, iTime, 5, offset, true, 0.25f, 1000, 3));
            }
            return OR_CONTINUE;
        }
        case COMMAND_TASK_LOOK_AT_VEHICLE:
        {
            CollectParameters(3);
            const i32 iPedID = ScriptParams[0];
            const i32 iVehicleID = ScriptParams[1];
            i32 iTime = ScriptParams[2];
            if (iTime == -1)
            {
                iTime = 20000;
            }
            else if (iTime == -2)
            {
                iTime = 0x7FFFFFFF;
            }
            pVehicle = CPools::GetVehiclePool().GetAt(iVehicleID);
            if (iPedID != -1)
            {
                g_ikChainMan.LookAt("COMMAND_TASK_LOOK_AT_VEHICLE", CPools::GetPedPool().GetAt(iPedID), pVehicle, iTime, static_cast<eBoneTag>(-1), nullptr, false, 0.25f,
                    500, 6, true);
            }
            else
            {
                RwV3d offset;
                offset.x = 0.0f;
                offset.y = 0.0f;
                offset.z = 0.0f;
                CTaskSequences::ms_taskSequence[CTaskSequences::ms_iActiveSequence].AddTask(
                    new CTaskSimpleTriggerLookAt(pVehicle, iTime, -1, offset, true, 0.25f, 1000, 3));
            }
            return OR_CONTINUE;
        }
        case COMMAND_TASK_SAY:
        {
            CollectParameters(2);
            const i32 iPedID = ScriptParams[0];
            const i32 iSay = ScriptParams[1];
            pTask = new CTaskSimpleSay(static_cast<eGlobalSpeechContext>(iSay), -1);
            if (iPedID == -1)
            {
                CTaskSequences::ms_taskSequence[CTaskSequences::ms_iActiveSequence].AddTask(new CTaskSimpleAffectSecondaryBehaviour(true, static_cast<eSecondaryTask>(2), pTask));
            }
            else
            {
                CPools::GetPedPool().GetAt(iPedID)->GetPedIntelligence()->AddTaskSecondarySay(pTask);
            }
            return OR_CONTINUE;
        }
        case COMMAND_TASK_SHAKE_FIST:
        {
            CollectParameters(1);
            const i32 iPedID = ScriptParams[0];
            pTask = new CTaskSimpleShakeFist();
            if (iPedID == -1)
            {
                CTaskSequences::ms_taskSequence[CTaskSequences::ms_iActiveSequence].AddTask(new CTaskSimpleAffectSecondaryBehaviour(true, static_cast<eSecondaryTask>(4), pTask));
            }
            else
            {
                CPools::GetPedPool().GetAt(iPedID)->GetPedIntelligence()->AddTaskSecondaryPartialAnim(pTask);
            }
            return OR_CONTINUE;
        }
        case COMMAND_TASK_COWER:
        {
            CollectParameters(1);
            const i32 iPedID = ScriptParams[0];
            GivePedScriptedTask(iPedID, new CTaskSimpleCower(), CurrCommand);
            return OR_CONTINUE;
        }
        case COMMAND_TASK_HANDS_UP:
        {
            CollectParameters(2);
            const i32 iPedID = ScriptParams[0];
            i32 iTime = ScriptParams[1];
            if (iTime == -1)
            {
                iTime = 20000;
            }
            else if (iTime == -2)
            {
                iTime = 0x7FFFFFFF;
            }
            GivePedScriptedTask(iPedID, new CTaskSimpleHandsUp(iTime), CurrCommand);
            return OR_CONTINUE;
        }
        case COMMAND_TASK_DUCK:
        {
            CollectParameters(2);
            const i32 iPedID = ScriptParams[0];
            const i32 iTime = ScriptParams[1];
            if (iTime == -2)
            {
                pTask = new CTaskSimpleDuck(static_cast<eDuckControlType>(0), static_cast<uint16>(0), -1);
            }
            else if (iTime == -1)
            {
                pTask = new CTaskSimpleDuck(static_cast<eDuckControlType>(0), static_cast<uint16>(20000), -1);
            }
            else
            {
                pTask = new CTaskSimpleDuck(static_cast<eDuckControlType>(0), static_cast<uint16>(iTime), -1);
            }
            GivePedScriptedTask(iPedID, pTask, CurrCommand);
            return OR_CONTINUE;
        }
        case COMMAND_TASK_DETONATE:
        {
            return OR_CONTINUE;
        }
        case COMMAND_TASK_USE_ATM:
        {
            CollectParameters(1);
            const i32 iPedID = ScriptParams[0];
            GivePedScriptedTask(iPedID, new CTaskSimpleUseAtm(), CurrCommand);
            return OR_CONTINUE;
        }
        case COMMAND_TASK_SCRATCH_HEAD:
        {
            CollectParameters(1);
            const i32 iPedID = ScriptParams[0];
            GivePedScriptedTask(iPedID, new CTaskSimpleScratchHead(), CurrCommand);
            return OR_CONTINUE;
        }
        case COMMAND_TASK_LOOK_ABOUT:
        {
            CollectParameters(2);
            const i32 iPedID = ScriptParams[0];
            i32 iTime = ScriptParams[1];
            if (iTime == -1)
            {
                iTime = 20000;
            }
            else if (iTime == -2)
            {
                iTime = 0x7FFFFFFF;
            }
            CTask* pTask = new CTaskSimpleLookAbout(iTime);
            GivePedScriptedTask(iPedID, pTask, CurrCommand);
            return OR_CONTINUE;
        }
        case COMMAND_TASK_ENTER_CAR_AS_PASSENGER:
        {
            CollectParameters(4);
            const i32 iPedID = ScriptParams[0];
            const i32 iVehicleID = ScriptParams[1];
            const i32 iTime = ScriptParams[2];
            const i32 iSeat = ScriptParams[3];
            pVehicle = CPools::GetVehiclePool().GetAt(iVehicleID);
            i32 iTargetDoor = 0;
            if (iSeat != -1)
            {
                iTargetDoor = CCarEnterExit::ComputeTargetDoorToEnterAsPassenger(*pVehicle, iSeat);
            }
            CTaskComplexEnterCarAsPassengerTimed* pTaskEnter;
            if (iTime == -2)
            {
                pTaskEnter = new CTaskComplexEnterCarAsPassengerTimed(pVehicle, iTargetDoor, -1, true);
            }
            else if (iTime == -1)
            {
                pTaskEnter = new CTaskComplexEnterCarAsPassengerTimed(pVehicle, iTargetDoor, 0, true);
            }
            else
            {
                pTaskEnter = new CTaskComplexEnterCarAsPassengerTimed(pVehicle, iTargetDoor, iTime, true);
            }
            pTaskEnter->SetMoveState(static_cast<eMoveState>(CPedIntelligence::ms_iDesiredMoveState));
            CPedIntelligence::ms_iDesiredMoveState = PEDMOVE_RUN;
            GivePedScriptedTask(iPedID, pTaskEnter, CurrCommand);
            return OR_CONTINUE;
        }
        case COMMAND_TASK_ENTER_CAR_AS_DRIVER:
        {
            CollectParameters(3);
            const i32 iPedID = ScriptParams[0];
            const i32 iVehicleID = ScriptParams[1];
            const i32 iTime = ScriptParams[2];
            pVehicle = CPools::GetVehiclePool().GetAt(iVehicleID);
            CTaskComplexEnterCarAsDriverTimed* pTaskEnter;
            if (iTime == -2)
            {
                pTaskEnter = new CTaskComplexEnterCarAsDriverTimed(pVehicle, -1);
            }
            else if (iTime == -1)
            {
                pTaskEnter = new CTaskComplexEnterCarAsDriverTimed(pVehicle, 0);
            }
            else
            {
                pTaskEnter = new CTaskComplexEnterCarAsDriverTimed(pVehicle, iTime);
            }
            pTaskEnter->SetMoveState(static_cast<eMoveState>(CPedIntelligence::ms_iDesiredMoveState));
            CPedIntelligence::ms_iDesiredMoveState = PEDMOVE_RUN;
            GivePedScriptedTask(iPedID, pTaskEnter, CurrCommand);
            return OR_CONTINUE;
        }
        case COMMAND_TASK_STEAL_CAR:
        {
            return OR_CONTINUE;
        }
        case COMMAND_TASK_LEAVE_CAR:
        {
            CollectParameters(2);
            const i32 iPedID = ScriptParams[0];
            const i32 iVehicleID = ScriptParams[1];
            pVehicle = CPools::GetVehiclePool().GetAt(iVehicleID);
            GivePedScriptedTask(iPedID, new CTaskComplexLeaveCar(pVehicle, 0, 0, true, false), CurrCommand);
            return OR_CONTINUE;
        }
        case COMMAND_TASK_LEAVE_CAR_AND_DIE:
        {
            return OR_CONTINUE;
        }
        case COMMAND_TASK_LEAVE_CAR_AND_FLEE:
        {
            CollectParameters(5);
            const i32 iPedID = ScriptParams[0];
            const i32 iVehicleID = ScriptParams[1];
            pVehicle = CPools::GetVehiclePool().GetAt(iVehicleID);
            const f32 x = *reinterpret_cast<f32*>(&ScriptParams[2]);
            const f32 y = *reinterpret_cast<f32*>(&ScriptParams[3]);
            const f32 z = *reinterpret_cast<f32*>(&ScriptParams[4]);
            GivePedScriptedTask(iPedID, new CTaskComplexLeaveCarAndFlee(pVehicle, CVector(x, y, z), static_cast<eTargetDoor>(0), 0, false), CurrCommand);
            return OR_CONTINUE;
        }
        case COMMAND_TASK_CAR_DRIVE:
        {
            return OR_CONTINUE;
        }
        case COMMAND_TASK_CAR_DRIVE_TO_COORD:
        {
            CollectParameters(9);
            const i32 iPedID = ScriptParams[0];
            const i32 iVehicleID = ScriptParams[1];
            pVehicle = nullptr;
            if (iVehicleID >= 0)
            {
                pVehicle = CPools::GetVehiclePool().GetAt(iVehicleID);
            }
            const f32 x = *reinterpret_cast<f32*>(&ScriptParams[2]);
            const f32 y = *reinterpret_cast<f32*>(&ScriptParams[3]);
            f32 z = *reinterpret_cast<f32*>(&ScriptParams[4]);
            if (z <= -100.0f)
            {
                z = CWorld::FindGroundZForCoord(x, y);
            }
            CVector vTarget(x, y, z);
            const f32 fCruiseSpeed = *reinterpret_cast<f32*>(&ScriptParams[5]);
            const i32 iMode = ScriptParams[6];
            i32 iDesiredCarModel = ScriptParams[7];
            if (iDesiredCarModel == 0)
            {
                iDesiredCarModel = -1;
            }
            else if (iDesiredCarModel == 1)
            {
                iDesiredCarModel = 415;
            }
            const i32 iDrivingStyle = ScriptParams[8];
            GivePedScriptedTask(iPedID, new CTaskComplexDriveToPoint(pVehicle, vTarget, fCruiseSpeed, iMode, static_cast<eModelID>(iDesiredCarModel), -1.0f, static_cast<eCarDrivingStyle>(iDrivingStyle)),
                CurrCommand);
            return OR_CONTINUE;
        }
        case COMMAND_TASK_CAR_DRIVE_WANDER:
        {
            CollectParameters(4);
            const i32 iPedID = ScriptParams[0];
            const i32 iVehicleID = ScriptParams[1];
            const f32 fSpeed = *reinterpret_cast<f32*>(&ScriptParams[2]);
            const i32 iDrivingStyle = ScriptParams[3];
            pVehicle = nullptr;
            if (iVehicleID >= 0)
            {
                pVehicle = CPools::GetVehiclePool().GetAt(iVehicleID);
            }
            GivePedScriptedTask(iPedID, new CTaskComplexCarDriveWander(pVehicle, static_cast<eCarDrivingStyle>(iDrivingStyle), fSpeed), CurrCommand);
            return OR_CONTINUE;
        }
        case COMMAND_TASK_GO_STRAIGHT_TO_COORD:
        {
            CollectParameters(6);
            const i32 iPedID = ScriptParams[0];
            const f32 x = *reinterpret_cast<f32*>(&ScriptParams[1]);
            const f32 y = *reinterpret_cast<f32*>(&ScriptParams[2]);
            const f32 z = *reinterpret_cast<f32*>(&ScriptParams[3]);
            CVector vTarget(x, y, z);
            const i32 iMoveState = ScriptParams[4];
            const i32 iTime = ScriptParams[5];
            const f32 fTargetRadius = CTaskComplexGoToPointAndStandStill::ms_fTargetRadius;
            const f32 fSlowDownDistance = CTaskComplexGoToPointAndStandStill::ms_fSlowDownDistance;
            if (iTime == -2)
            {
                pTask = new CTaskComplexGoToPointAndStandStill(static_cast<eMoveState>(iMoveState), vTarget, fTargetRadius, fSlowDownDistance, false, true);
            }
            else if (iTime == -1)
            {
                pTask = new CTaskComplexGoToPointAndStandStillTimed(static_cast<eMoveState>(iMoveState), vTarget, fTargetRadius, fSlowDownDistance,
                    0);
            }
            else
            {
                pTask = new CTaskComplexGoToPointAndStandStillTimed(static_cast<eMoveState>(iMoveState), vTarget, fTargetRadius, fSlowDownDistance, iTime);
            }
            GivePedScriptedTask(iPedID, pTask, CurrCommand);
            return OR_CONTINUE;
        }
        case COMMAND_TASK_ACHIEVE_HEADING:
        {
            CollectParameters(2);
            const i32 iPedID = ScriptParams[0];
            const f32 fHeadingDegrees = *reinterpret_cast<f32*>(&ScriptParams[1]);
            const f32 fHeadingRadians = DEGTORAD(fHeadingDegrees);
            GivePedScriptedTask(iPedID, new CTaskSimpleAchieveHeading(fHeadingRadians, 0.5f, 0.2f), CurrCommand);
            return OR_CONTINUE;
        }
        case COMMAND_SET_CHAR_IN_DISGUISE:
        {
            return OR_CONTINUE;
        }
        case COMMAND_FLUSH_ROUTE:
            CTaskComplexFollowPointRoute::ms_pointRoute.Clear();
            return OR_CONTINUE;
        case COMMAND_EXTEND_ROUTE:
        {
            CollectParameters(3);
            const f32 x = *reinterpret_cast<f32*>(&ScriptParams[0]);
            const f32 y = *reinterpret_cast<f32*>(&ScriptParams[1]);
            const f32 z = *reinterpret_cast<f32*>(&ScriptParams[2]);
            CVector vTarget(x, y, z);
            CTaskComplexFollowPointRoute::ms_pointRoute.Add(vTarget);
            return OR_CONTINUE;
        }
        case COMMAND_TASK_FOLLOW_POINT_ROUTE:
        {
            CollectParameters(3);
            const i32 iPedID = ScriptParams[0];
            const i32 iMoveState = ScriptParams[1];
            const i32 iMode = ScriptParams[2];
            GivePedScriptedTask(iPedID,
                new CTaskComplexFollowPointRoute(static_cast<eMoveState>(iMoveState), CTaskComplexFollowPointRoute::ms_pointRoute, static_cast<CTaskComplexFollowPointRoute::Mode>(iMode), 0.5f, 5.0f, false, true, true),
                CurrCommand);
            return OR_CONTINUE;
        }
        case COMMAND_TASK_GOTO_CHAR:
        {
            CollectParameters(4);
            const i32 iPedID = ScriptParams[0];
            const i32 iOtherPedID = ScriptParams[1];
            i32 iMaxSeekTime = ScriptParams[2];
            const f32 fSeekRadius = *reinterpret_cast<f32*>(&ScriptParams[3]);
            pPed = CPools::GetPedPool().GetAt(iOtherPedID);
            if (iMaxSeekTime == -2)
            {
                iMaxSeekTime = -1;
            }
            else if (iMaxSeekTime == -1)
            {
                iMaxSeekTime = 50000;
            }
            GivePedScriptedTask(iPedID, new TTaskComplexSeekEntityStandard(pPed, iMaxSeekTime, 1000, fSeekRadius, 2.0f, 2.0f, true, true), CurrCommand);
            return OR_CONTINUE;
        }
        case COMMAND_TASK_FLEE_POINT:
        {
            CollectParameters(6);
            const i32 iPedID = ScriptParams[0];
            const f32 x = *reinterpret_cast<f32*>(&ScriptParams[1]);
            const f32 y = *reinterpret_cast<f32*>(&ScriptParams[2]);
            const f32 z = *reinterpret_cast<f32*>(&ScriptParams[3]);
            CVector vTarget(x, y, z);
            const f32 fSafeDistance = *reinterpret_cast<f32*>(&ScriptParams[4]);
            const i32 iFleeTime = ScriptParams[5];
            GivePedScriptedTask(iPedID, new CTaskComplexFleePoint(vTarget, true, fSafeDistance, iFleeTime), CurrCommand);
            return OR_CONTINUE;
        }
        case COMMAND_TASK_FLEE_CHAR:
        {
            CollectParameters(4);
            const i32 iPedID = ScriptParams[0];
            const i32 iOtherPedID = ScriptParams[1];
            const f32 fSafeDistance = static_cast<f32>(ScriptParams[2]);
            const i32 iFleeTime = ScriptParams[3];
            pPed = CPools::GetPedPool().GetAt(iOtherPedID);
            GivePedScriptedTask(iPedID, new CTaskComplexFleeEntity(pPed, true, fSafeDistance, iFleeTime, 1000, 1.0f), CurrCommand);
            return OR_CONTINUE;
        }
    }
    return OR_INTERRUPT;
}

// MARK: 1500To1599

// func: sa 0x490DB0
OpcodeResult CRunningScript::ProcessCommands1500To1599(i32 CurrCommand)
{
    MARKFUNCTION(0x490DB0);

    bool LatestCmpFlagResult;
    CPed *pPed, *pSecondPed;
    CVehicle *pVehicle, *pSecondVehicle;
    CVector TempCoors;
    CVector TempVec;
    CVector NewCoors;
    CVector ObjRightVec, ObjUpVec, ObjForwardVec;
    CVector rightVec, upVec, forwardVec;
    f32 MinY, MinX;
    f32 MaxY, MaxX;
    f32 temp_float;
    CVector CurrentObjectSpeed;
    CRGBA tempRGBA;
    char AnimName[24];
    CAnimBlendAssociation* pAnim;
    CTask* pTask;
    i32 TempID, SequenceTaskID, UniqueID, ActualIndex;

    // NOTE: Keep switch cases in strictly ascending order as in commands.hpp to preserve MSVC jump table layout
    switch (CurrCommand)
    {
        case COMMAND_TASK_SMART_FLEE_POINT:
        {
            CollectParameters(6);
            const i32 iPedID = ScriptParams[0];
            const f32 x = *reinterpret_cast<f32*>(&ScriptParams[1]);
            const f32 y = *reinterpret_cast<f32*>(&ScriptParams[2]);
            const f32 z = *reinterpret_cast<f32*>(&ScriptParams[3]);
            CVector vTarget(x, y, z);
            const f32 fSafeDistance = *reinterpret_cast<f32*>(&ScriptParams[4]);
            const i32 iFleeTime = ScriptParams[5];
            GivePedScriptedTask(iPedID, new CTaskComplexSmartFleePoint(vTarget, true, fSafeDistance, iFleeTime), CurrCommand);
            return OR_CONTINUE;
        }
        case COMMAND_TASK_SMART_FLEE_CHAR:
        {
            CollectParameters(4);
            const i32 iPedID = ScriptParams[0];
            const i32 iOtherPedID = ScriptParams[1];
            const f32 fSafeDistance = static_cast<f32>(ScriptParams[2]);
            const i32 iFleeTime = ScriptParams[3];
            CPed* pOtherPed = CPools::GetPedPool().GetAt(iOtherPedID);
            GivePedScriptedTask(iPedID,
                new CTaskComplexSmartFleeEntity(pOtherPed, true, fSafeDistance, iFleeTime, CTaskComplexSmartFleeEntity::ms_iEntityPosCheckPeriod,
                    CTaskComplexSmartFleeEntity::ms_fEntityPosChangeThreshold),
                CurrCommand);
            return OR_CONTINUE;
        }
        case COMMAND_TASK_WANDER_STANDARD:
        {
            CollectParameters(1);
            const i32 iPedID = ScriptParams[0];
            GivePedScriptedTask(iPedID, new CTaskComplexWanderStandard(PEDMOVE_WALK, CGeneral::GetRandomNumberInRange(0, 8), true), CurrCommand);
            return OR_CONTINUE;
        }
        case COMMAND_TASK_WANDER_COP:
            return OR_CONTINUE;
        case COMMAND_TASK_WANDER_CRIMINAL:
            return OR_CONTINUE;
        case COMMAND_TASK_FOLLOW_LEADER_IN_FORMATION:
            return OR_CONTINUE;
        case COMMAND_TASK_KILL_CHAR_ON_FOOT:
        {
            CollectParameters(2);
            const i32 iPedID = ScriptParams[0];
            const i32 iOtherPedID = ScriptParams[1];
            pPed = CPools::GetPedPool().GetAt(iOtherPedID);
            GivePedScriptedTask(iPedID, new CTaskComplexKillPedOnFoot(pPed, -1, 0, 0, 0, 1), CurrCommand);
            return OR_CONTINUE;
        }
        case COMMAND_START_ADDING_STUNT_POINTS:
            return OR_CONTINUE;
        case COMMAND_ADD_STUNT_POINT:
            return OR_CONTINUE;
        case COMMAND_START_PLAYING_STUNT:
            return OR_CONTINUE;
        case COMMAND_HAS_STUNT_ENDED:
            return OR_CONTINUE;
        case COMMAND_HAS_STUNT_FAILED:
            return OR_CONTINUE;
        case COMMAND_START_RECORDING_STUNT:
            return OR_CONTINUE;
        case COMMAND_START_RECORDING_CAR:
            CollectParameters(2);
            return OR_CONTINUE;
        case COMMAND_STOP_RECORDING_CARS:
            return OR_CONTINUE;
        case COMMAND_START_PLAYBACK_RECORDED_CAR:
            CollectParameters(2);
            CVehicleRecording::StartPlaybackRecordedCar(CPools::GetVehiclePool().GetAt(ScriptParams[0]), ScriptParams[1], false, false);
            return OR_CONTINUE;
        case COMMAND_STOP_PLAYBACK_RECORDED_CAR:
            CollectParameters(1);
            pVehicle = CPools::GetVehiclePool().GetAt(ScriptParams[0]);
            if (pVehicle)
            {
                CVehicleRecording::StopPlaybackRecordedCar(pVehicle);
            }
            return OR_CONTINUE;
        case COMMAND_PAUSE_PLAYBACK_RECORDED_CAR:
            CollectParameters(1);
            pVehicle = CPools::GetVehiclePool().GetAt(ScriptParams[0]);
            CVehicleRecording::PausePlaybackRecordedCar(pVehicle);
            return OR_CONTINUE;
        case COMMAND_UNPAUSE_PLAYBACK_RECORDED_CAR:
            CollectParameters(1);
            pVehicle = CPools::GetVehiclePool().GetAt(ScriptParams[0]);
            CVehicleRecording::UnpausePlaybackRecordedCar(pVehicle);
            return OR_CONTINUE;
        case COMMAND_SET_CAR_PROTECT_CAR_REAR:
            return OR_CONTINUE;
        case COMMAND_SET_CAR_PROTECT_CAR_FRONT:
            return OR_CONTINUE;
        case COMMAND_SET_CAR_ESCORT_CAR_LEFT:
            CollectParameters(2);
            pVehicle = CPools::GetVehiclePool().GetAt(ScriptParams[0]);
            pSecondVehicle = CPools::GetVehiclePool().GetAt(ScriptParams[1]);
            pVehicle->AutoPilot.pTargetEntity = pSecondVehicle;
            pSecondVehicle->RegisterReference(reinterpret_cast<CEntity**>(&pVehicle->AutoPilot.pTargetEntity));
            pVehicle->AutoPilot.SetMission(MISSION_ESCORT_LEFT);
            return OR_CONTINUE;
        case COMMAND_SET_CAR_ESCORT_CAR_RIGHT:
            CollectParameters(2);
            pVehicle = CPools::GetVehiclePool().GetAt(ScriptParams[0]);
            pSecondVehicle = CPools::GetVehiclePool().GetAt(ScriptParams[1]);
            pVehicle->AutoPilot.pTargetEntity = pSecondVehicle;
            pSecondVehicle->RegisterReference(reinterpret_cast<CEntity**>(&pVehicle->AutoPilot.pTargetEntity));
            pVehicle->AutoPilot.SetMission(MISSION_ESCORT_RIGHT);
            return OR_CONTINUE;
        case COMMAND_SET_CAR_ESCORT_CAR_REAR:
            CollectParameters(2);
            pVehicle = CPools::GetVehiclePool().GetAt(ScriptParams[0]);
            pSecondVehicle = CPools::GetVehiclePool().GetAt(ScriptParams[1]);
            pVehicle->AutoPilot.pTargetEntity = pSecondVehicle;
            pSecondVehicle->RegisterReference(reinterpret_cast<CEntity**>(&pVehicle->AutoPilot.pTargetEntity));
            pVehicle->AutoPilot.SetMission(MISSION_ESCORT_REAR);
            return OR_CONTINUE;
        case COMMAND_SET_CAR_ESCORT_CAR_FRONT:
            CollectParameters(2);
            pVehicle = CPools::GetVehiclePool().GetAt(ScriptParams[0]);
            pSecondVehicle = CPools::GetVehiclePool().GetAt(ScriptParams[1]);
            pVehicle->AutoPilot.pTargetEntity = pSecondVehicle;
            pSecondVehicle->RegisterReference(reinterpret_cast<CEntity**>(&pVehicle->AutoPilot.pTargetEntity));
            pVehicle->AutoPilot.SetMission(MISSION_ESCORT_FRONT);
            return OR_CONTINUE;
        case COMMAND_TASK_FOLLOW_PATH_NODES_TO_COORD:
        {
            CollectParameters(6);
            const i32 iPedID = ScriptParams[0];
            const f32 x = *reinterpret_cast<f32*>(&ScriptParams[1]);
            const f32 y = *reinterpret_cast<f32*>(&ScriptParams[2]);
            const f32 z = *reinterpret_cast<f32*>(&ScriptParams[3]);
            CVector vTarget(x, y, z);
            const i32 iMoveState = ScriptParams[4];
            i32 iTime = ScriptParams[5];
            if (iTime == -1)
            {
                iTime = CTaskComplexFollowNodeRoute::ms_iDefaultTime;
            }
            else if (iTime == -2)
            {
                iTime = -1;
            }
            GivePedScriptedTask(iPedID, new CTaskComplexFollowNodeRoute(static_cast<eMoveState>(iMoveState), vTarget, CTaskComplexFollowNodeRoute::ms_fTargetRadius,
                    CTaskComplexFollowNodeRoute::ms_fSlowDownDistance, CTaskComplexFollowNodeRoute::ms_fFollowNodeThresholdHeightChange, true, iTime, true), CurrCommand);
            return OR_CONTINUE;
        }
        case COMMAND_IS_CHAR_IN_ANGLED_AREA_2D:
        case COMMAND_IS_CHAR_IN_ANGLED_AREA_ON_FOOT_2D:
        case COMMAND_IS_CHAR_IN_ANGLED_AREA_IN_CAR_2D:
        case COMMAND_IS_CHAR_STOPPED_IN_ANGLED_AREA_2D:
        case COMMAND_IS_CHAR_STOPPED_IN_ANGLED_AREA_ON_FOOT_2D:
        case COMMAND_IS_CHAR_STOPPED_IN_ANGLED_AREA_IN_CAR_2D:
        case COMMAND_IS_CHAR_IN_ANGLED_AREA_3D:
        case COMMAND_IS_CHAR_IN_ANGLED_AREA_ON_FOOT_3D:
        case COMMAND_IS_CHAR_IN_ANGLED_AREA_IN_CAR_3D:
        case COMMAND_IS_CHAR_STOPPED_IN_ANGLED_AREA_3D:
        case COMMAND_IS_CHAR_STOPPED_IN_ANGLED_AREA_ON_FOOT_3D:
        case COMMAND_IS_CHAR_STOPPED_IN_ANGLED_AREA_IN_CAR_3D:
            CharInAngledAreaCheckCommand(CurrCommand);
            return OR_CONTINUE;
        case COMMAND_IS_CHAR_IN_TAXI:
            CollectParameters(1);
            pPed = CPools::GetPedPool().GetAt(ScriptParams[0]);
            LatestCmpFlagResult = false;
            if (pPed->bInVehicle && pPed->m_pMyVehicle)
            {
                if (pPed->m_pMyVehicle->GetModelIndex() == MODELID_CAR_TAXI || pPed->m_pMyVehicle->GetModelIndex() == MODELID_CAR_CABBIE)
                {
                    LatestCmpFlagResult = true;
                }
            }
            UpdateCompareFlag(LatestCmpFlagResult);
            return OR_CONTINUE;
        case COMMAND_TASK_GO_TO_COORD_ANY_MEANS:
        {
            CollectParameters(6);
            const i32 iPedID = ScriptParams[0];
            const f32 x = *reinterpret_cast<f32*>(&ScriptParams[1]);
            const f32 y = *reinterpret_cast<f32*>(&ScriptParams[2]);
            const f32 z = *reinterpret_cast<f32*>(&ScriptParams[3]);
            CVector vTarget(x, y, z);
            const i32 iMoveState = ScriptParams[4];
            const i32 iVehicleID = ScriptParams[5];
            pVehicle = nullptr;
            if (iVehicleID >= 0)
            {
                pVehicle = CPools::GetVehiclePool().GetAt(iVehicleID);
            }
            GivePedScriptedTask(iPedID,
                new CTaskComplexGoToPointAnyMeans(iMoveState, vTarget, pVehicle, CTaskComplexGoToPointAnyMeans::ms_fTargetRadius, -1), CurrCommand);
            return OR_CONTINUE;
        }
        case COMMAND_GET_HEADING_FROM_VECTOR_2D:
            CollectParameters(2);
            temp_float = RADTODEG(CGeneral::GetATanOfXY(*reinterpret_cast<f32*>(&ScriptParams[0]), *reinterpret_cast<f32*>(&ScriptParams[1]))) - 90.0f;
            while (temp_float < 0.0f)
            {
                temp_float += 360.0f;
            }
            ScriptParams[0] = *reinterpret_cast<i32*>(&temp_float);
            StoreParameters(1);
            return OR_CONTINUE;
        case COMMAND_TASK_PLAY_ANIM:
            PlayAnimScriptCommand(CurrCommand);
            return OR_CONTINUE;
        case COMMAND_LOAD_PATH_NODES_IN_AREA:
            CollectParameters(4);
            MinX = *reinterpret_cast<f32*>(&ScriptParams[0]);
            MaxX = *reinterpret_cast<f32*>(&ScriptParams[2]);
            MinY = *reinterpret_cast<f32*>(&ScriptParams[1]);
            MaxY = *reinterpret_cast<f32*>(&ScriptParams[3]);
            if (MinX > MaxX)
            {
                temp_float = MinX;
                MinX = MaxX;
                MaxX = temp_float;
            }
            if (MinY > MaxY)
            {
                temp_float = MinY;
                MinY = MaxY;
                MaxY = temp_float;
            }
            ThePaths.MakeRequestForNodesToBeLoaded(MinX, MaxX, MinY, MaxY);
            return OR_CONTINUE;
        case COMMAND_RELEASE_PATH_NODES:
            ThePaths.ReleaseRequestedNodes();
            return OR_CONTINUE;
        case COMMAND_HAVE_PATH_NODES_BEEN_LOADED:
            return OR_CONTINUE;
        case COMMAND_LOAD_ALL_PATH_NODES_FOR_DEBUG:
            return OR_CONTINUE;
        case COMMAND_LOAD_CHAR_DECISION_MAKER:
        {
            CollectParameters(1);
            const i32 index = ScriptParams[0];
            char dmName[256];
            CDecisionMakerTypesFileLoader::GetPedDMName(index, dmName);
            CTheScripts::GetActualScriptThingIndex(CollectNextParameterWithoutIncreasingPC(), UNIQUE_SCRIPT_DECISION_MAKER);
            const i32 dm = CDecisionMakerTypesFileLoader::LoadDecisionMaker(dmName, static_cast<eDecisionTypes>(0), IsThisAMissionScript);
            ScriptParams[0] = CTheScripts::GetNewUniqueScriptThingIndex(dm, UNIQUE_SCRIPT_DECISION_MAKER);
            StoreParameters(1);
            if (IsThisAMissionScript)
            {
                CTheScripts::MissionCleanUp.AddEntityToList(ScriptParams[0], CLEANUP_DECISION_MAKER);
            }
            return OR_CONTINUE;
        }
        case COMMAND_SET_CHAR_DECISION_MAKER:
        {
            CollectParameters(2);
            pPed = CPools::GetPedPool().GetAt(ScriptParams[0]);
            i32 iDecisionMakerIndex = -1;
            if (ScriptParams[1] != -1)
            {
                iDecisionMakerIndex = CTheScripts::GetActualScriptThingIndex(ScriptParams[1], UNIQUE_SCRIPT_DECISION_MAKER);
            }
            pPed->GetPedIntelligence()->SetPedDecisionMakerType(iDecisionMakerIndex);
            return OR_CONTINUE;
        }
        case COMMAND_CLEAR_ALL_DECISION_MAKERS:
            return OR_CONTINUE;
        case COMMAND_SET_TEXT_DROPSHADOW:
            CollectParameters(5);
            tempRGBA = CRGBA(static_cast<u8>(ScriptParams[1]), static_cast<u8>(ScriptParams[2]), static_cast<u8>(ScriptParams[3]),
                static_cast<u8>(ScriptParams[4]));
            CTheScripts::IntroTextLines[CTheScripts::NumberOfIntroTextLinesThisFrame].ScriptTextDropShadowColour = tempRGBA;
            CTheScripts::IntroTextLines[CTheScripts::NumberOfIntroTextLinesThisFrame].ScriptTextDropShadow = static_cast<i8>(ScriptParams[0]);
            return OR_CONTINUE;
        case COMMAND_IS_PLAYBACK_GOING_ON_FOR_CAR:
            CollectParameters(1);
            pVehicle = CPools::GetVehiclePool().GetAt(ScriptParams[0]);
            LatestCmpFlagResult = false;
            if (CVehicleRecording::IsPlaybackGoingOnForCar(pVehicle))
            {
                LatestCmpFlagResult = true;
            }
            UpdateCompareFlag(LatestCmpFlagResult);
            return OR_CONTINUE;
        case COMMAND_SET_SENSE_RANGE:
        {
            CollectParameters(2);
            const i32 iPedID = ScriptParams[0];
            const f32 fRange = *reinterpret_cast<f32*>(&ScriptParams[1]);
            if (iPedID != -1)
            {
                pPed = CPools::GetPedPool().GetAt(iPedID);
                pPed->GetPedIntelligence()->SetSeeingRange(fRange);
                pPed->GetPedIntelligence()->SetHearingRange(fRange);
            }
            else
            {
                for (auto* pEntity : CWorld::ms_listMovingEntityPtrs)
                {
                    if (pEntity->GetType() == ENTITY_TYPE_PED && static_cast<CPed*>(pEntity)->GetCharCreatedBy() == MISSION_CHAR)
                    {
                        static_cast<CPed*>(pEntity)->GetPedIntelligence()->SetSeeingRange(fRange);
                        static_cast<CPed*>(pEntity)->GetPedIntelligence()->SetHearingRange(fRange);
                    }
                }
            }
            return OR_CONTINUE;
        }
        case COMMAND_SET_HEARING_RANGE:
            return OR_CONTINUE;
        case COMMAND_IS_CHAR_PLAYING_ANIM:
            CollectParameters(1);
            pPed = CPools::GetPedPool().GetAt(ScriptParams[0]);
            ReadTextLabelFromScript(AnimName, 24);
            LatestCmpFlagResult = RpAnimBlendClumpGetAssociation(reinterpret_cast<RpClump*>(pPed->m_pRwObject), AnimName) != nullptr;
            UpdateCompareFlag(LatestCmpFlagResult);
            return OR_CONTINUE;
        case COMMAND_SET_CHAR_ANIM_PLAYING_FLAG:
            CollectParameters(1);
            pPed = CPools::GetPedPool().GetAt(ScriptParams[0]);
            ReadTextLabelFromScript(AnimName, 24);
            pAnim = RpAnimBlendClumpGetAssociation(reinterpret_cast<RpClump*>(pPed->m_pRwObject), AnimName);
            CollectParameters(1);
            if (pAnim)
            {
                if (ScriptParams[0])
                {
                    pAnim->SetFlag(static_cast<eAnimationFlags>(1));
                }
                else
                {
                    pAnim->ClearFlag(static_cast<eAnimationFlags>(1));
                }
            }
            return OR_CONTINUE;
        case COMMAND_GET_CHAR_ANIM_CURRENT_TIME:
            CollectParameters(1);
            pPed = CPools::GetPedPool().GetAt(ScriptParams[0]);
            ReadTextLabelFromScript(AnimName, 24);
            pAnim = RpAnimBlendClumpGetAssociation(reinterpret_cast<RpClump*>(pPed->m_pRwObject), AnimName);
            temp_float = 0.0f;
            if (pAnim)
            {
                temp_float = pAnim->GetCurrentTime() / pAnim->GetTotalTime();
            }
            ScriptParams[0] = *reinterpret_cast<i32*>(&temp_float);
            StoreParameters(1);
            return OR_CONTINUE;
        case COMMAND_SET_CHAR_ANIM_CURRENT_TIME:
            CollectParameters(1);
            pPed = CPools::GetPedPool().GetAt(ScriptParams[0]);
            ReadTextLabelFromScript(AnimName, 24);
            pAnim = RpAnimBlendClumpGetAssociation(reinterpret_cast<RpClump*>(pPed->m_pRwObject), AnimName);
            CollectParameters(1);
            if (pAnim)
            {
                pAnim->SetCurrentTime(pAnim->GetTotalTime() * *reinterpret_cast<f32*>(&ScriptParams[0]));
            }
            return OR_CONTINUE;
        case COMMAND_OPEN_SEQUENCE_TASK:
        {
            CTheScripts::GetActualScriptThingIndex(CollectNextParameterWithoutIncreasingPC(), UNIQUE_SCRIPT_SEQUENCE_TASK);
            const i32 iTaskID = CTaskSequences::GetAvailableSlot(IsThisAMissionScript);
            if (iTaskID >= 0 && iTaskID < CTaskSequences::MAX_NUM_SEQUENCE_TASKS)
            {
                CTaskSequences::ms_bIsOpened[iTaskID] = true;
                CTaskSequences::ms_taskSequence[iTaskID].Flush();
                CTaskSequences::ms_iActiveSequence = iTaskID;
                UniqueID = CTheScripts::GetNewUniqueScriptThingIndex(iTaskID, UNIQUE_SCRIPT_SEQUENCE_TASK);
                ScriptParams[0] = UniqueID;
                if (IsThisAMissionScript)
                {
                    CTheScripts::MissionCleanUp.AddEntityToList(UniqueID, CLEANUP_SEQUENCE_TASK);
                }
            }
            else
            {
                ScriptParams[0] = -1;
            }
            StoreParameters(1);
            return OR_CONTINUE;
        }
        case COMMAND_CLOSE_SEQUENCE_TASK:
        {
            CollectParameters(1);
            const i32 iTaskID = CTheScripts::GetActualScriptThingIndex(ScriptParams[0], UNIQUE_SCRIPT_SEQUENCE_TASK);
            if (iTaskID >= 0 && iTaskID < CTaskSequences::MAX_NUM_SEQUENCE_TASKS)
            {
                CTaskSequences::ms_bIsOpened[iTaskID] = false;
                CTaskSequences::ms_iActiveSequence = -1;
            }
            return OR_CONTINUE;
        }
        case COMMAND_SCRIPT_EVENT:
            return OR_CONTINUE;
        case COMMAND_PERFORM_SEQUENCE_TASK:
        {
            CollectParameters(2);
            const i32 iPedID = ScriptParams[0];
            const i32 iTaskID = CTheScripts::GetActualScriptThingIndex(ScriptParams[1], UNIQUE_SCRIPT_SEQUENCE_TASK);
            if (iTaskID >= 0 && iTaskID < CTaskSequences::MAX_NUM_SEQUENCE_TASKS)
            {
                CTask* pTask = new CTaskComplexUseSequence(iTaskID);
                GivePedScriptedTask(iPedID, pTask, CurrCommand);
            }
            return OR_CONTINUE;
        }
        case COMMAND_SET_CHAR_COLLISION:
            CollectParameters(2);
            pPed = CPools::GetPedPool().GetAt(ScriptParams[0]);
            if (ScriptParams[1])
            {
                pPed->SetUsesCollision(true);
            }
            else
            {
                pPed->SetUsesCollision(false);
            }
            return OR_CONTINUE;
        case COMMAND_GET_CHAR_ANIM_TOTAL_TIME:
            CollectParameters(1);
            pPed = CPools::GetPedPool().GetAt(ScriptParams[0]);
            ReadTextLabelFromScript(AnimName, 24);
            pAnim = RpAnimBlendClumpGetAssociation(reinterpret_cast<RpClump*>(pPed->m_pRwObject), AnimName);
            temp_float = 0.0f;
            if (pAnim)
            {
                temp_float = pAnim->GetTotalTime() * 1000.0f;
            }
            ScriptParams[0] = *reinterpret_cast<i32*>(&temp_float);
            StoreParameters(1);
            return OR_CONTINUE;
        case COMMAND_CLEAR_SEQUENCE_TASK:
        {
            CollectParameters(1);
            UniqueID = ScriptParams[0];
            const i32 iTaskID = CTheScripts::GetActualScriptThingIndex(UniqueID, UNIQUE_SCRIPT_SEQUENCE_TASK);
            if (iTaskID >= 0 && iTaskID < CTaskSequences::MAX_NUM_SEQUENCE_TASKS)
            {
                CTaskSequences::ms_bIsOpened[iTaskID] = false;
                CTaskSequences::ms_taskSequence[iTaskID].SetCanBeEmptied(true);
                CTheScripts::ScriptSequenceTaskArray[iTaskID].m_bUsed = false;
            }
            if (IsThisAMissionScript)
            {
                CTheScripts::MissionCleanUp.RemoveEntityFromList(UniqueID, CLEANUP_SEQUENCE_TASK);
            }
            return OR_CONTINUE;
        }
        case COMMAND_CLEAR_ALL_SEQUENCE_TASKS:
            return OR_CONTINUE;
        case COMMAND_ADD_ATTRACTOR:
        {
            CollectParameters(6);
            CTheScripts::GetActualScriptThingIndex(CollectNextParameterWithoutIncreasingPC(), UNIQUE_SCRIPT_ATTRACTOR);
            const i32 iAttractorID = CScripted2dEffects::AddScripted2DEffect(-1.0f);
            const i32 iSequenceTaskID = CTheScripts::GetActualScriptThingIndex(ScriptParams[5], UNIQUE_SCRIPT_SEQUENCE_TASK);
            if (iAttractorID >= 0 && iAttractorID < 64 && iSequenceTaskID >= 0 &&
                iSequenceTaskID < CTaskSequences::MAX_NUM_SEQUENCE_TASKS)
            {
                CVector vPos(*reinterpret_cast<f32*>(&ScriptParams[0]), *reinterpret_cast<f32*>(&ScriptParams[1]),
                    *reinterpret_cast<f32*>(&ScriptParams[2]));
                {
                    f32 fUse = DEGTORAD(*reinterpret_cast<f32*>(&ScriptParams[3]));
                    CVector vUse(-std::sin(fUse), std::cos(fUse), 0.0f);
                    vUse.Normalise();
                    f32 fForward = DEGTORAD(*reinterpret_cast<f32*>(&ScriptParams[4]));
                    CVector vForward(-std::sin(fForward), std::cos(fForward), 0.0f);
                    vForward.Normalise();
                    C2dEffect& effect = CScripted2dEffects::ms_effects[iAttractorID];
                    effect.m_Pos = vPos;
                    effect.pedAttractor.m_vecQueueDir = *reinterpret_cast<RwV3d*>(&vUse);
                    CScripted2dEffects::ms_effectSequenceTaskIDs[iAttractorID] = iSequenceTaskID;
                    effect.pedAttractor.m_vecUseDir = *reinterpret_cast<RwV3d*>(&vUse);
                    effect.pedAttractor.m_vecForwardDir = *reinterpret_cast<RwV3d*>(&vForward);
                    effect.m_Type = EFFECT_ATTRACTOR;
                    effect.pedAttractor.m_nAttractorType = PED_ATTRACTOR_SCRIPTED;
                }
                ScriptParams[0] = CTheScripts::GetNewUniqueScriptThingIndex(iAttractorID, UNIQUE_SCRIPT_ATTRACTOR);
                if (IsThisAMissionScript)
                {
                    CTheScripts::MissionCleanUp.AddEntityToList(ScriptParams[0], CLEANUP_ATTRACTOR);
                }
            }
            else
            {
                ScriptParams[0] = -1;
            }
            StoreParameters(1);
            return OR_CONTINUE;
        }
        case COMMAND_CLEAR_ATTRACTOR:
        {
            CollectParameters(1);
            UniqueID = ScriptParams[0];
            const i32 iAttractorID = CTheScripts::GetActualScriptThingIndex(UniqueID, UNIQUE_SCRIPT_ATTRACTOR);
            if (iAttractorID >= 0 && iAttractorID < 64)
            {
                CScripted2dEffects::ms_activated[iAttractorID] = false;
            }
            if (IsThisAMissionScript)
            {
                CTheScripts::MissionCleanUp.RemoveEntityFromList(UniqueID, CLEANUP_ATTRACTOR);
            }
            return OR_CONTINUE;
        }
        case COMMAND_CLEAR_ALL_ATTRACTORS:
            return OR_CONTINUE;
        case COMMAND_TASK_PLAY_ANIM_FOR_TIME:
            return OR_CONTINUE;
        case COMMAND_CREATE_CHAR_AT_ATTRACTOR:
        {
            CollectParameters(4);
            const i32 iAttractorID = CTheScripts::GetActualScriptThingIndex(ScriptParams[2], UNIQUE_SCRIPT_ATTRACTOR);
            if (iAttractorID < 0 || iAttractorID >= 64)
            {
                ScriptParams[0] = -1;
                StoreParameters(1);
                return OR_CONTINUE;
            }
            GetCorrectPedModelIndexForEmergencyServiceType(ScriptParams[0], &ScriptParams[1]);
            if (ScriptParams[0] == PEDTYPE_COP)
            {
                pPed = new CCopPed(static_cast<eCopType>(ScriptParams[1]));
            }
            else if (ScriptParams[0] == PEDTYPE_MEDIC || ScriptParams[0] == PEDTYPE_FIRE)
            {
                pPed = new CEmergencyPed(ScriptParams[0], ScriptParams[1]);
            }
            else
            {
                pPed = new CCivilianPed(static_cast<ePedType>(ScriptParams[0]), ScriptParams[1]);
            }
            pPed->SetCharCreatedBy(MISSION_CHAR);
            pPed->bAllowMedicsToReviveMe = false;
            const C2dEffect& effect = CScripted2dEffects::ms_effects[iAttractorID];
            CPedAttractorPedPlacer::PlacePedAtEffect(reinterpret_cast<const C2dEffectPedAttractor&>(effect), nullptr, pPed, 0.01f);
            TempCoors = pPed->GetPosition();
            CTheScripts::ClearSpaceForMissionEntity(TempCoors, pPed);
            if (IsThisAMissionScript)
            {
                pPed->m_bIsStaticWaitingForCollision = true;
            }
            CWorld::Add(pPed);
            const i32 iDefaultTaskType = ScriptParams[3];
            switch (iDefaultTaskType)
            {
                case COMMAND_TASK_STAND_STILL:
                    pPed->GetPedIntelligence()->AddTaskDefault(new CTaskSimpleStandStill(0, true, false, 8.0f), false);
                    break;
                case COMMAND_TASK_WANDER_STANDARD:
                    pPed->GetPedIntelligence()->AddTaskDefault(
                        new CTaskComplexWanderStandard(PEDMOVE_WALK, CGeneral::GetRandomNumberInRange(0, 8), true), false);
                    break;
                case COMMAND_TASK_WANDER_COP:
                    break;
                case COMMAND_TASK_WANDER_CRIMINAL:
                    pPed->GetPedIntelligence()->AddTaskDefault(
                        new CTaskComplexWanderCriminal(PEDMOVE_WALK, CGeneral::GetRandomNumberInRange(0, 8), true), false);
                    break;
                default:
                    pPed->GetPedIntelligence()->AddTaskDefault(
                        new CTaskComplexWanderStandard(PEDMOVE_WALK, CGeneral::GetRandomNumberInRange(0, 8), true), false);
                    break;
            }
            {
                CTask* pTask = new CTaskComplexUseEffect(const_cast<C2dEffectPedAttractor*>(reinterpret_cast<const C2dEffectPedAttractor*>(&effect)), nullptr);
                CEventScriptCommand event(TASK_PRIMARY_PRIMARY, pTask, false);
                CEvent* pEvent = pPed->GetPedIntelligence()->AddEvent(event, false);
                const i32 iVacantSlot = CPedScriptedTaskRecord::GetVacantSlot();
                CPedScriptedTaskRecord::ms_scriptedTasks[iVacantSlot].Set(pPed, CurrCommand, static_cast<CEventScriptCommand*>(pEvent));
                CPopulation::ms_nTotalMissionPeds++;
                ScriptParams[0] = CPools::GetPedPool().GetIndex(pPed);
                if (IsThisAMissionScript)
                {
                    CTheScripts::MissionCleanUp.AddEntityToList(ScriptParams[0], CLEANUP_CHAR);
                }
            }
            StoreParameters(1);
            return OR_CONTINUE;
        }
        case COMMAND_TASK_LEAVE_CAR_IMMEDIATELY:
        {
            CollectParameters(2);
            const i32 iPedID = ScriptParams[0];
            const i32 iVehicleID = ScriptParams[1];
            pVehicle = CPools::GetVehiclePool().GetAt(iVehicleID);
            GivePedScriptedTask(iPedID, new CTaskComplexLeaveCar(pVehicle, 0, 0, false, false), CurrCommand);
            return OR_CONTINUE;
        }
        case COMMAND_INCREMENT_INT_STAT:
            CollectParameters(2);
            CStats::IncrementStat(static_cast<eStats>(ScriptParams[0]), static_cast<f32>(ScriptParams[1]));
            CStats::DisplayScriptStatUpdateMessage(eStatUpdateState::STAT_UPDATE_INCREASE, static_cast<eStats>(ScriptParams[0]), static_cast<f32>(ScriptParams[1]));
            return OR_CONTINUE;
        case COMMAND_INCREMENT_FLOAT_STAT:
            CollectParameters(2);
            CStats::IncrementStat(static_cast<eStats>(ScriptParams[0]), *reinterpret_cast<f32*>(&ScriptParams[1]));
            CStats::DisplayScriptStatUpdateMessage(eStatUpdateState::STAT_UPDATE_INCREASE, static_cast<eStats>(ScriptParams[0]), *reinterpret_cast<f32*>(&ScriptParams[1]));
            return OR_CONTINUE;
        case COMMAND_DECREMENT_INT_STAT:
            CollectParameters(2);
            CStats::DecrementStat(static_cast<eStats>(ScriptParams[0]), static_cast<f32>(ScriptParams[1]));
            CStats::DisplayScriptStatUpdateMessage(eStatUpdateState::STAT_UPDATE_DECREASE, static_cast<eStats>(ScriptParams[0]), static_cast<f32>(ScriptParams[1]));
            return OR_CONTINUE;
        case COMMAND_DECREMENT_FLOAT_STAT:
            CollectParameters(2);
            CStats::DecrementStat(static_cast<eStats>(ScriptParams[0]), *reinterpret_cast<f32*>(&ScriptParams[1]));
            CStats::DisplayScriptStatUpdateMessage(eStatUpdateState::STAT_UPDATE_DECREASE, static_cast<eStats>(ScriptParams[0]), *reinterpret_cast<f32*>(&ScriptParams[1]));
            return OR_CONTINUE;
        case COMMAND_REGISTER_INT_STAT:
            CollectParameters(2);
            CStats::SetNewRecordStat(static_cast<eStats>(ScriptParams[0]), static_cast<f32>(ScriptParams[1]));
            CStats::DisplayScriptStatUpdateMessage(eStatUpdateState::STAT_UPDATE_INCREASE, static_cast<eStats>(ScriptParams[0]), static_cast<f32>(ScriptParams[1]));
            return OR_CONTINUE;
        case COMMAND_REGISTER_FLOAT_STAT:
            CollectParameters(2);
            CStats::SetNewRecordStat(static_cast<eStats>(ScriptParams[0]), *reinterpret_cast<f32*>(&ScriptParams[1]));
            CStats::DisplayScriptStatUpdateMessage(eStatUpdateState::STAT_UPDATE_INCREASE, static_cast<eStats>(ScriptParams[0]), *reinterpret_cast<f32*>(&ScriptParams[1]));
            return OR_CONTINUE;
        case COMMAND_SET_INT_STAT:
            CollectParameters(2);
            CStats::SetStatValue(static_cast<eStats>(ScriptParams[0]), static_cast<f32>(ScriptParams[1]));
            CStats::DisplayScriptStatUpdateMessage(eStatUpdateState::STAT_UPDATE_INCREASE, static_cast<eStats>(ScriptParams[0]), static_cast<f32>(ScriptParams[1]));
            return OR_CONTINUE;
        case COMMAND_SET_FLOAT_STAT:
            CollectParameters(2);
            CStats::SetStatValue(static_cast<eStats>(ScriptParams[0]), *reinterpret_cast<f32*>(&ScriptParams[1]));
            CStats::DisplayScriptStatUpdateMessage(eStatUpdateState::STAT_UPDATE_INCREASE, static_cast<eStats>(ScriptParams[0]), *reinterpret_cast<f32*>(&ScriptParams[1]));
            return OR_CONTINUE;
        case COMMAND_GET_ATTEMPTS_FOR_THIS_MISSION:
            return OR_CONTINUE;
        case COMMAND_REGISTER_THIS_MISSION_HAS_BEEN_ATTEMPTED:
            return OR_CONTINUE;
        case COMMAND_REGISTER_THIS_MISSION_HAS_BEEN_PASSED:
            return OR_CONTINUE;
        case COMMAND_GET_SCRIPT_TASK_STATUS:
            CollectParameters(2);
            pPed = CPools::GetPedPool().GetAt(ScriptParams[0]);
            ScriptParams[0] = static_cast<int32>(CPedScriptedTaskRecord::GetStatus(pPed, ScriptParams[1]));
            if (ScriptParams[0] == -1)
            {
                ScriptParams[0] = 7;
            }
            StoreParameters(1);
            return OR_CONTINUE;
        case COMMAND_CREATE_GROUP:
        {
            CollectParameters(1);
            const i32 iDefaultTaskType = ScriptParams[0];
            CTheScripts::GetActualScriptThingIndex(CollectNextParameterWithoutIncreasingPC(), UNIQUE_SCRIPT_PEDGROUP);
            const i32 iGroupID = CPedGroups::AddGroup();
            CPedGroups::ms_groups[iGroupID].GetGroupIntelligence()->SetDefaultTaskAllocatorType(iDefaultTaskType);
            CPedGroups::ms_groups[iGroupID].m_bIsMissionGroup = true;
            ScriptParams[0] = CTheScripts::GetNewUniqueScriptThingIndex(iGroupID, UNIQUE_SCRIPT_PEDGROUP);
            StoreParameters(1);
            if (IsThisAMissionScript)
            {
                CTheScripts::MissionCleanUp.AddEntityToList(ScriptParams[0], CLEANUP_PEDGROUP);
            }
            return OR_CONTINUE;
        }
        case COMMAND_SET_GROUP_LEADER:
        {
            CollectParameters(2);
            const i32 iGroupID = CTheScripts::GetActualScriptThingIndex(ScriptParams[0], UNIQUE_SCRIPT_PEDGROUP);
            const i32 iPedID = ScriptParams[1];
            pPed = CPools::GetPedPool().GetAt(iPedID);
            if (iGroupID >= 0 && iGroupID < 8)
            {
                if (!pPed->IsPlayer())
                {
                    CEventScriptCommand event(TASK_PRIMARY_PRIMARY, new CTaskComplexBeInGroup(iGroupID, true), false);
                    pPed->GetPedIntelligence()->AddEvent(event, false);
                }
                CPedGroups::ms_groups[iGroupID].GetGroupMembership()->SetLeader(pPed);
                CPedGroups::ms_groups[iGroupID].Process();
            }
            return OR_CONTINUE;
        }
        case COMMAND_SET_GROUP_MEMBER:
        {
            CollectParameters(2);
            const i32 iGroupID = CTheScripts::GetActualScriptThingIndex(ScriptParams[0], UNIQUE_SCRIPT_PEDGROUP);
            const i32 iPedID = ScriptParams[1];
            pPed = CPools::GetPedPool().GetAt(iPedID);
            if (iGroupID < 0 || iGroupID >= 8)
            {
                return OR_CONTINUE;
            }
            CEventScriptCommand event(TASK_PRIMARY_PRIMARY, new CTaskComplexBeInGroup(iGroupID, false), false);
            pPed->GetPedIntelligence()->AddEvent(event, false);
            if (CPedGroups::ms_groups[iGroupID].GetGroupMembership()->CountMembersExcludingLeader() >= 7 &&
                CPedGroups::ms_groups[iGroupID].GetGroupMembership()->GetLeader() &&
                CPedGroups::ms_groups[iGroupID].GetGroupMembership()->GetLeader()->IsPlayer())
            {
                CPedGroups::ms_groups[iGroupID].GetGroupMembership()->RemoveNFollowers(1);
            }
            CPedGroups::ms_groups[iGroupID].GetGroupMembership()->AddFollower(pPed);
            CPedGroups::ms_groups[iGroupID].Process();
            CPed* pLeader = CPedGroups::ms_groups[iGroupID].GetGroupMembership()->GetLeader();
            if (pLeader)
            {
                CVehicle* pVeh;
                if (pLeader->bInVehicle && pLeader->m_pMyVehicle)
                {
                    pVeh = pLeader->m_pMyVehicle;
                }
                else
                {
                    CTaskComplexEnterCarAsDriver* pTaskEnterCar = static_cast<CTaskComplexEnterCarAsDriver*>(
                        pLeader->GetPedIntelligence()->FindTaskActiveByType(CTaskTypes::TASK_COMPLEX_ENTER_CAR_AS_DRIVER));
                    if (!pTaskEnterCar)
                    {
                        return OR_CONTINUE;
                    }
                    pVeh = pTaskEnterCar->GetTargetVehicle();
                }
                if (pVeh)
                {
                    i32 i;
                    for (i = 0; i < pVeh->m_nMaxPassengers; i++)
                    {
                        if (!pVeh->pPassengers[i])
                        {
                            break;
                        }
                    }
                    if (i < pVeh->m_nMaxPassengers)
                    {
                        CEventLeaderEnteredCarAsDriver eventLeaderEnteredCar(pVeh);
                        eventLeaderEnteredCar.ComputeResponseTaskType(&CPedGroups::ms_groups[iGroupID]);
                        const i32 iTaskType = eventLeaderEnteredCar.GetResponseTaskType();
                        if (iTaskType == CTaskTypes::TASK_GROUP_ENTER_CAR)
                        {
                            CTaskComplexEnterCarAsPassenger task(pVeh, 0, true);
                            CPedGroups::ms_groups[iGroupID].GetGroupIntelligence()->SetScriptCommandTask(pPed, task);
                            if (!pPed->bHasGroupDriveTask)
                            {
                                pPed->bHasGroupDriveTask = true;
                                CTaskComplexSequence taskSequence;
                                taskSequence.AddTask(new CTaskSimpleCarDrive(pVeh, nullptr, true));
                                const CTask* pTaskDefault = CPedGroups::ms_groups[iGroupID].GetGroupIntelligence()->GetTaskDefault(pPed);
                                if (pTaskDefault)
                                {
                                    taskSequence.AddTask(pTaskDefault->Clone());
                                }
                                CPedGroups::ms_groups[iGroupID].GetGroupIntelligence()->SetDefaultTask(pPed, taskSequence);
                            }
                        }
                        else if (iTaskType == CTaskTypes::TASK_GROUP_ENTER_CAR_AND_PERFORM_SEQUENCE)
                        {
                            CEventLeaderEnteredCarAsDriver* pEvent = new CEventLeaderEnteredCarAsDriver(pVeh);
                            CEventGroupEvent groupEvent(pLeader, pEvent);
                            CPedGroups::ms_groups[iGroupID].GetGroupIntelligence()->AddEvent(groupEvent);
                        }
                    }
                }
            }
            return OR_CONTINUE;
        }
        case COMMAND_REMOVE_GROUP:
        {
            CollectParameters(1);
            const i32 UniqueID = ScriptParams[0];
            const i32 iGroupID = CTheScripts::GetActualScriptThingIndex(UniqueID, UNIQUE_SCRIPT_PEDGROUP);
            if (iGroupID >= 0 && iGroupID < 8)
            {
                if (CPedGroups::ms_activeGroups[iGroupID] && CPedGroups::ms_groups[iGroupID].GetGroupMembership()->GetLeader() &&
                    CPedGroups::ms_groups[iGroupID].GetGroupMembership()->GetLeader()->IsPlayer())
                {
                    CPedGroups::RemoveAllFollowersFromGroup(iGroupID);
                    return OR_CONTINUE;
                }
                CPedGroups::RemoveGroup(iGroupID);
            }
            if (IsThisAMissionScript)
            {
                CTheScripts::MissionCleanUp.RemoveEntityFromList(UniqueID, CLEANUP_PEDGROUP);
            }
            return OR_CONTINUE;
        }
        case COMMAND_TASK_LEAVE_ANY_CAR:
        {
            CollectParameters(1);
            const i32 iPedID = ScriptParams[0];
            GivePedScriptedTask(iPedID, new CTaskComplexLeaveAnyCar(0, true, false), CurrCommand);
            return OR_CONTINUE;
        }
        case COMMAND_TASK_KILL_CHAR_ON_FOOT_WHILE_DUCKING:
        {
            CollectParameters(5);
            const i32 iPedID = ScriptParams[0];
            const i32 iOtherPedID = ScriptParams[1];
            pPed = CPools::GetPedPool().GetAt(iOtherPedID);
            GivePedScriptedTask(iPedID, new CTaskComplexKillPedOnFoot(pPed, -1, ScriptParams[2], ScriptParams[3], ScriptParams[4], 1), CurrCommand);
            return OR_CONTINUE;
        }
        case COMMAND_TASK_AIM_GUN_AT_CHAR:
        {
            CollectParameters(3);
            const i32 iPedID = ScriptParams[0];
            const i32 iTargetPedID = ScriptParams[1];
            pPed = CPools::GetPedPool().GetAt(iTargetPedID);
            GivePedScriptedTask(iPedID, new CTaskSimpleGunControl(pPed, nullptr, nullptr, 0, 1, ScriptParams[2]), CurrCommand);
            return OR_CONTINUE;
        }
        case COMMAND_TASK_SIDE_STEP_AND_SHOOT:
            CollectParameters(9);
            return OR_CONTINUE;
        case COMMAND_TASK_GO_TO_COORD_WHILE_SHOOTING:
        {
            CollectParameters(8);
            const i32 iPedID = ScriptParams[0];
            const f32 TargetRadius = *reinterpret_cast<f32*>(&ScriptParams[5]);
            const f32 SlowDistance = *reinterpret_cast<f32*>(&ScriptParams[6]);
            const i32 iOtherPedID = ScriptParams[7];
            pPed = CPools::GetPedPool().GetAt(iOtherPedID);
            TempCoors = CVector(*reinterpret_cast<f32*>(&ScriptParams[1]), *reinterpret_cast<f32*>(&ScriptParams[2]),
                *reinterpret_cast<f32*>(&ScriptParams[3]));
            GivePedScriptedTask(iPedID,
                new CTaskComplexGoToPointShooting(static_cast<eMoveState>(ScriptParams[4]), TempCoors, pPed, CVector(0.0f, 0.0f, 0.0f), TargetRadius, SlowDistance), CurrCommand);
            return OR_CONTINUE;
        }
        case COMMAND_TASK_STAY_IN_SAME_PLACE:
        {
            CollectParameters(2);
            const i32 iPedID = ScriptParams[0];
            if (iPedID == -1)
            {
                pTask = new CTaskSimpleSetStayInSamePlace(ScriptParams[1] != 0);
                CTaskSequences::ms_taskSequence[CTaskSequences::ms_iActiveSequence].AddTask(pTask);
            }
            else
            {
                pPed = CPools::GetPedPool().GetAt(iPedID);
                {
                    CTaskSimpleSetStayInSamePlace task(ScriptParams[1] != 0);
                    task.ProcessPed(pPed);
                }
            }
            return OR_CONTINUE;
        }
        case COMMAND_TASK_TURN_CHAR_TO_FACE_CHAR:
        {
            CollectParameters(2);
            const i32 iPedID = ScriptParams[0];
            const i32 iOtherPedID = ScriptParams[1];
            GivePedScriptedTask(iPedID, new CTaskComplexTurnToFaceEntityOrCoord(CPools::GetPedPool().GetAt(iOtherPedID), 0.5f, 0.2f), CurrCommand);
            return OR_CONTINUE;
        }
    }
    return OR_INTERRUPT;
}

// MARK: 1600To1699

// sa 0x493FE0
OpcodeResult CRunningScript::ProcessCommands1600To1699(i32 CurrCommand)
{
    MARKFUNCTION(0x493FE0);

    bool LatestCmpFlagResult;

    CPed* pPed;
    CPed* pSecondPed;
    CVehicle* pVehicle;
    CVehicle* pSecondVehicle;

    CObject* pObj;
    CPlayerInfo* pPlayer;

    CVector TempCoors;
    CVector TempVec;
    CVector NewCoors;

    i32 ActualIndex;
    f32 temp_float;

    char TextLabel[40];
    char name[32];
    CTask* pTask;
    i32 loop;
    CVector attachedCamOffset;
    CVector attachedCamLookAt;

    // NOTE: Keep switch cases in strictly ascending order as in commands.hpp to preserve MSVC jump table layout
    switch (CurrCommand)
    {
        case COMMAND_IS_CHAR_AT_SCRIPTED_ATTRACTOR:
        {
            CollectParameters(2);
            i32 pedHandle = ScriptParams[0];
            ActualIndex = CTheScripts::GetActualScriptThingIndex(ScriptParams[1], UNIQUE_SCRIPT_ATTRACTOR);
            LatestCmpFlagResult = false;
            if (ActualIndex >= 0 && ActualIndex < 64)
            {
                C2dEffect* pEffect = &CScripted2dEffects::ms_effects[ActualIndex];
                CPed* pUsingPed = GetPedAttractorManager()->GetPedUsingEffect(reinterpret_cast<const C2dEffectPedAttractor*>(pEffect), nullptr);
                if (pedHandle == -1)
                {
                    LatestCmpFlagResult = (pUsingPed != nullptr);
                }
                else
                {
                    LatestCmpFlagResult = (CPools::GetPedPool().GetAt(pedHandle) == pUsingPed);
                }
            }
            UpdateCompareFlag(LatestCmpFlagResult);
            return OR_CONTINUE;
        }
        case COMMAND_SET_SEQUENCE_TO_REPEAT:
        {
            CollectParameters(2);
            ActualIndex = CTheScripts::GetActualScriptThingIndex(ScriptParams[0], UNIQUE_SCRIPT_SEQUENCE_TASK);
            if (ActualIndex >= 0 && ActualIndex < 64)
            {
                CTaskSequences::ms_taskSequence[CTaskSequences::ms_iActiveSequence].SetRepeatMode(ScriptParams[1]);
            }
            return OR_CONTINUE;
        }
        case COMMAND_GET_SEQUENCE_PROGRESS:
        {
            CollectParameters(1);
            pPed = CPools::GetPedPool().GetAt(ScriptParams[0]);
            if (CPedScriptedTaskRecord::GetStatus(pPed, COMMAND_PERFORM_SEQUENCE_TASK) <= eScriptedTaskStatus::NONE)
            {
                ScriptParams[0] = -1;
                StoreParameters(1);
                return OR_CONTINUE;
            }
            CTaskComplexUseSequence* pTaskSequence = static_cast<CTaskComplexUseSequence*>(pPed->GetPedIntelligence()->GetTaskPrimary());
            ScriptParams[0] = pTaskSequence->GetProgress1();
            StoreParameters(1);
            return OR_CONTINUE;
        }
        case COMMAND_CLEAR_LOOK_AT:
        {
            CollectParameters(1);
            if (ScriptParams[0] == -1)
            {
                pTask = new CTaskSimpleClearLookAt();
                CTaskSequences::ms_taskSequence[CTaskSequences::ms_iActiveSequence].AddTask(pTask);
            }
            else
            {
                pPed = CPools::GetPedPool().GetAt(ScriptParams[0]);
                if (g_ikChainMan.IsLooking(pPed))
                {
                    g_ikChainMan.AbortLookAt(pPed, 500);
                }
            }
            return OR_CONTINUE;
        }
        case COMMAND_SET_FOLLOW_NODE_THRESHOLD_DISTANCE:
        {
            CollectParameters(2);
            pPed = CPools::GetPedPool().GetAt(ScriptParams[0]);
            pPed->GetPedIntelligence()->SetFollowNodeThresholdDistance(*(f32*)&ScriptParams[1]);
            return OR_CONTINUE;
        }
        case COMMAND_ADD_PEDMODEL_AS_ATTRACTOR_USER:
            return OR_CONTINUE;
        case COMMAND_CREATE_FX_SYSTEM:
        {
            ReadTextLabelFromScript(name, 32);
            CollectParameters(4);
            CTheScripts::GetActualScriptThingIndex(CollectNextParameterWithoutIncreasingPC(), UNIQUE_SCRIPT_EFFECT_SYSTEM);
            f32 x = *(f32*)&ScriptParams[0];
            f32 y = *(f32*)&ScriptParams[1];
            f32 z = *(f32*)&ScriptParams[2];
            if (z <= -100.0f)
            {
                z = CWorld::FindGroundZForCoord(x, y);
            }
            CVector pos(x, y, z);
            FxSystem_c* pFxSystem = g_fxMan.CreateFxSystem(name, pos, nullptr, ScriptParams[3] != 0);
            if (pFxSystem)
            {
                ScriptParams[0] = CTheScripts::AddScriptEffectSystem(pFxSystem);
                if (IsThisAMissionScript)
                {
                    CTheScripts::MissionCleanUp.AddEntityToList(ScriptParams[0], CLEANUP_EFFECT_SYSTEM);
                }
            }
            else
            {
                ScriptParams[0] = -1;
            }
            StoreParameters(1);
            return OR_CONTINUE;
        }
        case COMMAND_PLAY_FX_SYSTEM:
        {
            CollectParameters(1);
            ActualIndex = CTheScripts::GetActualScriptThingIndex(ScriptParams[0], UNIQUE_SCRIPT_EFFECT_SYSTEM);
            if (ActualIndex >= 0)
            {
                FxSystem_c* pFxSystem = CTheScripts::ScriptEffectSystemArray[ActualIndex].pFXSystem;
                if (pFxSystem)
                {
                    pFxSystem->Play();
                }
            }
            return OR_CONTINUE;
        }
        case COMMAND_PAUSE_FX_SYSTEM:
            return OR_CONTINUE;
        case COMMAND_STOP_FX_SYSTEM:
        {
            CollectParameters(1);
            ActualIndex = CTheScripts::GetActualScriptThingIndex(ScriptParams[0], UNIQUE_SCRIPT_EFFECT_SYSTEM);
            if (ActualIndex >= 0)
            {
                FxSystem_c* pFxSystem = CTheScripts::ScriptEffectSystemArray[ActualIndex].pFXSystem;
                if (pFxSystem)
                {
                    pFxSystem->Stop();
                }
            }
            return OR_CONTINUE;
        }
        case COMMAND_PLAY_AND_KILL_FX_SYSTEM:
        {
            CollectParameters(1);
            ActualIndex = CTheScripts::GetActualScriptThingIndex(ScriptParams[0], UNIQUE_SCRIPT_EFFECT_SYSTEM);
            if (ActualIndex >= 0)
            {
                FxSystem_c* pFxSystem = CTheScripts::ScriptEffectSystemArray[ActualIndex].pFXSystem;
                if (pFxSystem)
                {
                    pFxSystem->PlayAndKill();
                    CTheScripts::RemoveScriptEffectSystem(ScriptParams[0]);
                    if (IsThisAMissionScript)
                    {
                        CTheScripts::MissionCleanUp.RemoveEntityFromList(ScriptParams[0], CLEANUP_EFFECT_SYSTEM);
                    }
                }
            }
            return OR_CONTINUE;
        }
        case COMMAND_KILL_FX_SYSTEM:
        {
            CollectParameters(1);
            ActualIndex = CTheScripts::GetActualScriptThingIndex(ScriptParams[0], UNIQUE_SCRIPT_EFFECT_SYSTEM);
            if (ActualIndex >= 0)
            {
                FxSystem_c* pFxSystem = CTheScripts::ScriptEffectSystemArray[ActualIndex].pFXSystem;
                if (pFxSystem)
                {
                    pFxSystem->Kill();
                    CTheScripts::RemoveScriptEffectSystem(ScriptParams[0]);
                    if (IsThisAMissionScript)
                    {
                        CTheScripts::MissionCleanUp.RemoveEntityFromList(ScriptParams[0], CLEANUP_EFFECT_SYSTEM);
                    }
                }
            }
            return OR_CONTINUE;
        }
        case COMMAND_CREATE_FX_SYSTEM_WITH_DIRECTION:
            return OR_CONTINUE;
        case COMMAND_GET_INT_STAT:
        {
            CollectParameters(1);
            ScriptParams[0] = static_cast<i32>(CStats::GetStatValue(static_cast<u16>(ScriptParams[0])));
            StoreParameters(1);
            return OR_CONTINUE;
        }
        case COMMAND_GET_FLOAT_STAT:
        {
            CollectParameters(1);
            *(f32*)&ScriptParams[0] = CStats::GetStatValue(static_cast<u16>(ScriptParams[0]));
            StoreParameters(1);
            return OR_CONTINUE;
        }
        case COMMAND_SET_OBJECT_RENDER_SCORCHED:
        {
            CollectParameters(2);
            pObj = CPools::GetObjectPool().GetAt(ScriptParams[0]);
            pObj->physicalFlags.bRenderScorched = (ScriptParams[1] != 0);
            return OR_CONTINUE;
        }
        case COMMAND_TASK_LOOK_AT_OBJECT:
        {
            CollectParameters(3);
            i32 time = ScriptParams[2];
            if (time == -1)
            {
                time = 20000;
            }
            else if (time == -2)
            {
                time = 0x7FFFFFFF;
            }
            pObj = CPools::GetObjectPool().GetAt(ScriptParams[1]);
            if (ScriptParams[0] != -1)
            {
                pPed = CPools::GetPedPool().GetAt(ScriptParams[0]);
                g_ikChainMan.LookAt("COMMAND_TASK_LOOK_AT_OBJECT", pPed, pObj, time, static_cast<eBoneTag>(-1), nullptr, false, 0.25f, 500, 6, true);
            }
            else
            {
                CVector offset(0.0f, 0.0f, 0.0f);
                pTask = new CTaskSimpleTriggerLookAt(pObj, time, -1, offset, true, 0.25f, 1000, 3);
                CTaskSequences::ms_taskSequence[CTaskSequences::ms_iActiveSequence].AddTask(pTask);
            }
            return OR_CONTINUE;
        }
        case COMMAND_LIMIT_ANGLE:
        {
            CollectParameters(1);
            f32 angle = *(f32*)&ScriptParams[0];
            while (angle < 0.0f)
            {
                angle += 360.0f;
            }
            while (angle >= 360.0f)
            {
                angle -= 360.0f;
            }
            *(f32*)&ScriptParams[0] = angle;
            StoreParameters(1);
            return OR_CONTINUE;
        }
        case COMMAND_OPEN_CAR_DOOR:
        {
            CollectParameters(2);
            pVehicle = CPools::GetVehiclePool().GetAt(ScriptParams[0]);
            eDoors door = static_cast<eDoors>(ScriptParams[1]);
            i32 carNodeIndex = CDamageManager::GetCarNodeIndexFromDoor(door);
            if (!pVehicle->IsDoorMissing(door))
            {
                CAutomobile* pCar = static_cast<CAutomobile*>(pVehicle);
                if (pCar->m_aCarNodes[carNodeIndex])
                {
                    pVehicle->OpenDoor(nullptr, carNodeIndex, door, 1.0f, true);
                }
            }
            return OR_CONTINUE;
        }
        case COMMAND_SET_ATTRACTOR_PAIR:
            return OR_CONTINUE;
        case COMMAND_PLACE_CHAR_AT_ATTRACTOR:
            return OR_CONTINUE;
        case COMMAND_GET_PICKUP_COORDINATES:
        {
            CollectParameters(1);
            i32 pickupIndex = CPickups::GetActualPickupIndex(ScriptParams[0]);
            CVector coors(0.0f, 0.0f, -100.0f);
            if (pickupIndex != -1)
            {
                coors = CPickups::aPickUps.data()[pickupIndex].GetCoors();
            }
            *(CVector*)&ScriptParams[0] = coors;
            StoreParameters(3);
            return OR_CONTINUE;
        }
        case COMMAND_REMOVE_DECISION_MAKER:
        {
            CollectParameters(1);
            i32 decisionMaker = ScriptParams[0];
            ActualIndex = CTheScripts::GetActualScriptThingIndex(ScriptParams[0], UNIQUE_SCRIPT_DECISION_MAKER);
            if (ActualIndex < 20 && CTheScripts::ScriptResourceManager.RemoveFromResourceManager(decisionMaker, 3, this))
            {
                CDecisionMakerTypesFileLoader::UnloadDecisionMaker(static_cast<eDecisionTypes>(ActualIndex));
            }
            if (IsThisAMissionScript)
            {
                CTheScripts::MissionCleanUp.RemoveEntityFromList(decisionMaker, CLEANUP_DECISION_MAKER);
            }
            return OR_CONTINUE;
        }
        case COMMAND_VIEW_INTEGER_VARIABLE:
            GetIndexOfGlobalVariable();
            ReadTextLabelFromScript(TextLabel, 40);
            return OR_CONTINUE;
        case COMMAND_VIEW_FLOAT_VARIABLE:
            GetIndexOfGlobalVariable();
            ReadTextLabelFromScript(TextLabel, 40);
            return OR_CONTINUE;
        case COMMAND_WATCH_INTEGER_VARIABLE:
            GetIndexOfGlobalVariable();
            ReadTextLabelFromScript(TextLabel, 40);
            return OR_CONTINUE;
        case COMMAND_WATCH_FLOAT_VARIABLE:
            GetIndexOfGlobalVariable();
            ReadTextLabelFromScript(TextLabel, 40);
            return OR_CONTINUE;
        case COMMAND_BREAKPOINT:
            ReadTextLabelFromScript(TextLabel, 40);
            return OR_CONTINUE;
        case COMMAND_WRITE_DEBUG:
            ReadTextLabelFromScript(TextLabel, 40);
            return OR_CONTINUE;
        case COMMAND_WRITE_DEBUG_WITH_INT:
            ReadTextLabelFromScript(TextLabel, 40);
            CollectParameters(1);
            return OR_CONTINUE;
        case COMMAND_WRITE_DEBUG_WITH_FLOAT:
            ReadTextLabelFromScript(TextLabel, 40);
            CollectParameters(1);
            return OR_CONTINUE;
        case COMMAND_GET_CHAR_MODEL:
        {
            CollectParameters(1);
            pPed = CPools::GetPedPool().GetAt(ScriptParams[0]);
            ScriptParams[0] = pPed->m_nModelIndex;
            StoreParameters(1);
            return OR_CONTINUE;
        }
        case COMMAND_IS_CHAR_TOUCHING_ANY_OBJECT:
            return OR_CONTINUE;
        case COMMAND_TASK_AIM_GUN_AT_COORD:
        {
            CollectParameters(5);
            CVector target(*(f32*)&ScriptParams[1], *(f32*)&ScriptParams[2], *(f32*)&ScriptParams[3]);
            pTask = new CTaskSimpleGunControl(nullptr, &target, nullptr, 0, 500, ScriptParams[4]);
            GivePedScriptedTask(ScriptParams[0], pTask, CurrCommand);
            return OR_CONTINUE;
        }
        case COMMAND_TASK_SHOOT_AT_COORD:
        {
            CollectParameters(5);
            CVector target(*(f32*)&ScriptParams[1], *(f32*)&ScriptParams[2], *(f32*)&ScriptParams[3]);
            pTask = nullptr;
            if (ScriptParams[0] != -1)
            {
                pPed = CPools::GetPedPool().GetAt(ScriptParams[0]);
                CWeaponInfo* pWeaponInfo = CWeaponInfo::GetWeaponInfo(pPed->GetWeapon()->GetWeaponType(), 1);
                if (pWeaponInfo->IsThrowWeapon())
                {
                    pTask = new CTaskSimpleThrowControl(nullptr, &target);
                }
            }
            if (!pTask)
            {
                pTask = new CTaskSimpleGunControl(nullptr, &target, nullptr, 3, 5, ScriptParams[4]);
            }
            GivePedScriptedTask(ScriptParams[0], pTask, CurrCommand);
            return OR_CONTINUE;
        }
        case COMMAND_CREATE_FX_SYSTEM_ON_CHAR:
        {
            ReadTextLabelFromScript(name, 32);
            CollectParameters(5);
            CTheScripts::GetActualScriptThingIndex(CollectNextParameterWithoutIncreasingPC(), UNIQUE_SCRIPT_EFFECT_SYSTEM);
            pPed = CPools::GetPedPool().GetAt(ScriptParams[0]);
            f32 x = *(f32*)&ScriptParams[1];
            f32 y = *(f32*)&ScriptParams[2];
            f32 z = *(f32*)&ScriptParams[3];
            if (z <= -100.0f)
            {
                z = CWorld::FindGroundZForCoord(x, y);
            }
            CVector pos(x, y, z);
            CEntity* pEntity = pPed;
            if (!pEntity->GetRwObject())
            {
                pEntity->CreateRwObject();
            }
            RwMatrix* pParentMat = pEntity->GetRwMatrix();
            FxSystem_c* pFxSystem = nullptr;
            if (pParentMat)
            {
                pFxSystem = g_fxMan.CreateFxSystem(name, pos, pParentMat, ScriptParams[4] != 0);
            }
            if (pFxSystem)
            {
                ScriptParams[0] = CTheScripts::AddScriptEffectSystem(pFxSystem);
                if (IsThisAMissionScript)
                {
                    CTheScripts::MissionCleanUp.AddEntityToList(ScriptParams[0], CLEANUP_EFFECT_SYSTEM);
                }
            }
            else
            {
                ScriptParams[0] = -1;
            }
            StoreParameters(1);
            return OR_CONTINUE;
        }
        case COMMAND_CREATE_FX_SYSTEM_ON_CHAR_WITH_DIRECTION:
        {
            ReadTextLabelFromScript(name, 32);
            CollectParameters(8);
            CTheScripts::GetActualScriptThingIndex(CollectNextParameterWithoutIncreasingPC(), UNIQUE_SCRIPT_EFFECT_SYSTEM);
            pPed = CPools::GetPedPool().GetAt(ScriptParams[0]);
            f32 x = *(f32*)&ScriptParams[1];
            f32 y = *(f32*)&ScriptParams[2];
            f32 z = *(f32*)&ScriptParams[3];
            if (z <= -100.0f)
            {
                z = CWorld::FindGroundZForCoord(x, y);
            }
            CVector pos(x, y, z);
            CVector dir(*(f32*)&ScriptParams[4], *(f32*)&ScriptParams[5], *(f32*)&ScriptParams[6]);
            RwMatrix matrix;
            g_fx.CreateMatFromVec(&matrix, &pos, &dir);
            CEntity* pEntity = pPed;
            if (!pEntity->GetRwObject())
            {
                pEntity->CreateRwObject();
            }
            RwMatrix* pParentMat = pEntity->GetRwMatrix();
            FxSystem_c* pFxSystem = nullptr;
            if (pParentMat)
            {
                pFxSystem = g_fxMan.CreateFxSystem(name, matrix, pParentMat, ScriptParams[7] != 0);
            }
            if (pFxSystem)
            {
                ScriptParams[0] = CTheScripts::AddScriptEffectSystem(pFxSystem);
                if (IsThisAMissionScript)
                {
                    CTheScripts::MissionCleanUp.AddEntityToList(ScriptParams[0], CLEANUP_EFFECT_SYSTEM);
                }
            }
            else
            {
                ScriptParams[0] = -1;
            }
            StoreParameters(1);
            return OR_CONTINUE;
        }
        case COMMAND_CREATE_FX_SYSTEM_ON_CAR:
        {
            ReadTextLabelFromScript(name, 32);
            CollectParameters(5);
            CTheScripts::GetActualScriptThingIndex(CollectNextParameterWithoutIncreasingPC(), UNIQUE_SCRIPT_EFFECT_SYSTEM);
            pVehicle = CPools::GetVehiclePool().GetAt(ScriptParams[0]);
            f32 x = *(f32*)&ScriptParams[1];
            f32 y = *(f32*)&ScriptParams[2];
            f32 z = *(f32*)&ScriptParams[3];
            if (z <= -100.0f)
            {
                z = CWorld::FindGroundZForCoord(x, y);
            }
            CVector pos(x, y, z);
            CEntity* pEntity = pVehicle;
            if (!pEntity->GetRwObject())
            {
                pEntity->CreateRwObject();
            }
            RwMatrix* pParentMat = pEntity->GetRwMatrix();
            FxSystem_c* pFxSystem = nullptr;
            if (pParentMat)
            {
                pFxSystem = g_fxMan.CreateFxSystem(name, pos, pParentMat, ScriptParams[4] != 0);
            }
            if (pFxSystem)
            {
                ScriptParams[0] = CTheScripts::AddScriptEffectSystem(pFxSystem);
                if (IsThisAMissionScript)
                {
                    CTheScripts::MissionCleanUp.AddEntityToList(ScriptParams[0], CLEANUP_EFFECT_SYSTEM);
                }
            }
            else
            {
                ScriptParams[0] = -1;
            }
            StoreParameters(1);
            return OR_CONTINUE;
        }
        case COMMAND_CREATE_FX_SYSTEM_ON_CAR_WITH_DIRECTION:
        {
            ReadTextLabelFromScript(name, 32);
            CollectParameters(8);
            CTheScripts::GetActualScriptThingIndex(CollectNextParameterWithoutIncreasingPC(), UNIQUE_SCRIPT_EFFECT_SYSTEM);
            pVehicle = CPools::GetVehiclePool().GetAt(ScriptParams[0]);
            f32 x = *(f32*)&ScriptParams[1];
            f32 y = *(f32*)&ScriptParams[2];
            f32 z = *(f32*)&ScriptParams[3];
            if (z <= -100.0f)
            {
                z = CWorld::FindGroundZForCoord(x, y);
            }
            CVector pos(x, y, z);
            CVector dir(*(f32*)&ScriptParams[4], *(f32*)&ScriptParams[5], *(f32*)&ScriptParams[6]);
            RwMatrix matrix;
            g_fx.CreateMatFromVec(&matrix, &pos, &dir);
            CEntity* pEntity = pVehicle;
            if (!pEntity->GetRwObject())
            {
                pEntity->CreateRwObject();
            }
            RwMatrix* pParentMat = pEntity->GetRwMatrix();
            FxSystem_c* pFxSystem = nullptr;
            if (pParentMat)
            {
                pFxSystem = g_fxMan.CreateFxSystem(name, matrix, pParentMat, ScriptParams[7] != 0);
            }
            if (pFxSystem)
            {
                ScriptParams[0] = CTheScripts::AddScriptEffectSystem(pFxSystem);
                if (IsThisAMissionScript)
                {
                    CTheScripts::MissionCleanUp.AddEntityToList(ScriptParams[0], CLEANUP_EFFECT_SYSTEM);
                }
            }
            else
            {
                ScriptParams[0] = -1;
            }
            StoreParameters(1);
            return OR_CONTINUE;
        }
        case COMMAND_CREATE_FX_SYSTEM_ON_OBJECT:
        {
            ReadTextLabelFromScript(name, 32);
            CollectParameters(5);
            CTheScripts::GetActualScriptThingIndex(CollectNextParameterWithoutIncreasingPC(), UNIQUE_SCRIPT_EFFECT_SYSTEM);
            pObj = CPools::GetObjectPool().GetAt(ScriptParams[0]);
            f32 x = *(f32*)&ScriptParams[1];
            f32 y = *(f32*)&ScriptParams[2];
            f32 z = *(f32*)&ScriptParams[3];
            if (z <= -100.0f)
            {
                z = CWorld::FindGroundZForCoord(x, y);
            }
            CVector pos(x, y, z);
            CEntity* pEntity = pObj;
            if (!pEntity->GetRwObject())
            {
                pEntity->CreateRwObject();
            }
            RwMatrix* pParentMat = pEntity->GetRwMatrix();
            FxSystem_c* pFxSystem = nullptr;
            if (pParentMat)
            {
                pFxSystem = g_fxMan.CreateFxSystem(name, pos, pParentMat, ScriptParams[4] != 0);
            }
            if (pFxSystem)
            {
                ScriptParams[0] = CTheScripts::AddScriptEffectSystem(pFxSystem);
                if (IsThisAMissionScript)
                {
                    CTheScripts::MissionCleanUp.AddEntityToList(ScriptParams[0], CLEANUP_EFFECT_SYSTEM);
                }
            }
            else
            {
                ScriptParams[0] = -1;
            }
            StoreParameters(1);
            return OR_CONTINUE;
        }
        case COMMAND_CREATE_FX_SYSTEM_ON_OBJECT_WITH_DIRECTION:
        {
            ReadTextLabelFromScript(name, 32);
            CollectParameters(8);
            CTheScripts::GetActualScriptThingIndex(CollectNextParameterWithoutIncreasingPC(), UNIQUE_SCRIPT_EFFECT_SYSTEM);
            pObj = CPools::GetObjectPool().GetAt(ScriptParams[0]);
            f32 x = *(f32*)&ScriptParams[1];
            f32 y = *(f32*)&ScriptParams[2];
            f32 z = *(f32*)&ScriptParams[3];
            if (z <= -100.0f)
            {
                z = CWorld::FindGroundZForCoord(x, y);
            }
            CVector pos(x, y, z);
            CVector dir(*(f32*)&ScriptParams[4], *(f32*)&ScriptParams[5], *(f32*)&ScriptParams[6]);
            RwMatrix matrix;
            g_fx.CreateMatFromVec(&matrix, &pos, &dir);
            CEntity* pEntity = pObj;
            if (!pEntity->GetRwObject())
            {
                pEntity->CreateRwObject();
            }
            RwMatrix* pParentMat = pEntity->GetRwMatrix();
            FxSystem_c* pFxSystem = nullptr;
            if (pParentMat)
            {
                pFxSystem = g_fxMan.CreateFxSystem(name, matrix, pParentMat, ScriptParams[7] != 0);
            }
            if (pFxSystem)
            {
                ScriptParams[0] = CTheScripts::AddScriptEffectSystem(pFxSystem);
                if (IsThisAMissionScript)
                {
                    CTheScripts::MissionCleanUp.AddEntityToList(ScriptParams[0], CLEANUP_EFFECT_SYSTEM);
                }
            }
            else
            {
                ScriptParams[0] = -1;
            }
            StoreParameters(1);
            return OR_CONTINUE;
        }
        case COMMAND_ADD_QUEUED_DIALOGUE:
            return OR_CONTINUE;
        case COMMAND_IS_DIALOGUE_FINISHED:
            return OR_CONTINUE;
        case COMMAND_IS_DIALOGUE_PLAYING:
            return OR_CONTINUE;
        case COMMAND_TASK_DESTROY_CAR:
        {
            CollectParameters(2);
            pVehicle = CPools::GetVehiclePool().GetAt(ScriptParams[1]);
            pTask = new CTaskComplexDestroyCar(pVehicle, 0, 0, 0);
            GivePedScriptedTask(ScriptParams[0], pTask, CurrCommand);
            return OR_CONTINUE;
        }
        case COMMAND_TASK_DIVE_AND_GET_UP:
        {
            CollectParameters(4);
            CVector dir(*(f32*)&ScriptParams[1], *(f32*)&ScriptParams[2], 0.0f);
            dir.Normalise();
            pTask = new CTaskComplexEvasiveDiveAndGetUp(nullptr, ScriptParams[3], dir, true);
            GivePedScriptedTask(ScriptParams[0], pTask, CurrCommand);
            return OR_CONTINUE;
        }
        case COMMAND_CUSTOM_PLATE_FOR_NEXT_CAR:
        {
            CollectParameters(1);
            char plateText[9];
            ReadTextLabelFromScript(plateText, 9);
            for (i32 i = 0; i < 8; i++)
            {
                if (plateText[i] == '_' || plateText[i] == '\0')
                {
                    plateText[i] = ' ';
                }
            }
            plateText[8] = '\0';
            CBaseModelInfo* pModelInfo = CModelInfo::GetModelInfo(ScriptParams[0]);
            if (pModelInfo && pModelInfo->GetModelType() == MI_TYPE_VEHICLE)
            {
                CVehicleModelInfo* pVehModelInfo = static_cast<CVehicleModelInfo*>(pModelInfo);
                if (pVehModelInfo->pCustomPlateMaterial)
                {
                    pVehModelInfo->SetCustomCarPlateText(plateText);
                }
            }
            return OR_CONTINUE;
        }
        case COMMAND_CREATE_PED_GENERATOR_AT_ATTRACTOR:
            return OR_CONTINUE;
        case COMMAND_TASK_SHUFFLE_TO_NEXT_CAR_SEAT:
        {
            CollectParameters(2);
            pVehicle = CPools::GetVehiclePool().GetAt(ScriptParams[1]);
            pTask = new CTaskComplexShuffleSeats(pVehicle);
            GivePedScriptedTask(ScriptParams[0], pTask, CurrCommand);
            return OR_CONTINUE;
        }
        case COMMAND_TASK_CHAT_WITH_CHAR:
        {
            CollectParameters(4);
            pPed = CPools::GetPedPool().GetAt(ScriptParams[1]);
            bool bLeadSpeaker = (ScriptParams[2] != 0);
            bool bUnknown = (ScriptParams[3] != 0);
            i32 pedType = bUnknown ? -1 : 4;
            CVector pos(0.0f, 0.0f, 0.0f);
            pTask = new CTaskComplexPartnerChat("COMMAND_TASK_CHAT_WITH_CHAR", pPed, bLeadSpeaker, 0.5f, pedType, true, true, pos);
            GivePedScriptedTask(ScriptParams[0], pTask, CurrCommand);
            return OR_CONTINUE;
        }
        case COMMAND_GET_CHAR_AT_SCRIPTED_ATTRACTOR:
            return OR_CONTINUE;
        case COMMAND_ATTACH_CAMERA_TO_VEHICLE:
        {
            CollectParameters(9);
            pVehicle = CPools::GetVehiclePool().GetAt(ScriptParams[0]);
            CVector offset(*(f32*)&ScriptParams[1], *(f32*)&ScriptParams[2], *(f32*)&ScriptParams[3]);
            CVector rotation(*(f32*)&ScriptParams[4], *(f32*)&ScriptParams[5], *(f32*)&ScriptParams[6]);
            TheCamera.TakeControlAttachToEntity(nullptr, pVehicle, offset, rotation, *(f32*)&ScriptParams[7], static_cast<i16>(ScriptParams[8]), 1);
            return OR_CONTINUE;
        }
        case COMMAND_ATTACH_CAMERA_TO_VEHICLE_LOOK_AT_VEHICLE:
        {
            CollectParameters(7);
            pVehicle = CPools::GetVehiclePool().GetAt(ScriptParams[0]);
            pSecondVehicle = CPools::GetVehiclePool().GetAt(ScriptParams[4]);
            CVector offset(*(f32*)&ScriptParams[1], *(f32*)&ScriptParams[2], *(f32*)&ScriptParams[3]);
            CVector lookAt(0.0f, 0.0f, 0.0f);
            TheCamera.TakeControlAttachToEntity(pSecondVehicle, pVehicle, offset, lookAt, *(f32*)&ScriptParams[5], static_cast<i16>(ScriptParams[6]), 1);
            return OR_CONTINUE;
        }
        case COMMAND_ATTACH_CAMERA_TO_VEHICLE_LOOK_AT_CHAR:
        {
            CollectParameters(7);
            pVehicle = CPools::GetVehiclePool().GetAt(ScriptParams[0]);
            pPed = CPools::GetPedPool().GetAt(ScriptParams[4]);
            CVector offset(*(f32*)&ScriptParams[1], *(f32*)&ScriptParams[2], *(f32*)&ScriptParams[3]);
            CVector lookAt(0.0f, 0.0f, 0.0f);
            TheCamera.TakeControlAttachToEntity(pPed, pVehicle, offset, lookAt, *(f32*)&ScriptParams[5], static_cast<i16>(ScriptParams[6]), 1);
            return OR_CONTINUE;
        }
        case COMMAND_ATTACH_CAMERA_TO_CHAR:
        {
            CollectParameters(9);
            pPed = CPools::GetPedPool().GetAt(ScriptParams[0]);
            CVector offset(*(f32*)&ScriptParams[1], *(f32*)&ScriptParams[2], *(f32*)&ScriptParams[3]);
            CVector rotation(*(f32*)&ScriptParams[4], *(f32*)&ScriptParams[5], *(f32*)&ScriptParams[6]);
            TheCamera.TakeControlAttachToEntity(nullptr, pPed, offset, rotation, *(f32*)&ScriptParams[7], static_cast<i16>(ScriptParams[8]), 1);
            return OR_CONTINUE;
        }
        case COMMAND_ATTACH_CAMERA_TO_CHAR_LOOK_AT_VEHICLE:
        {
            CollectParameters(7);
            pPed = CPools::GetPedPool().GetAt(ScriptParams[0]);
            pVehicle = CPools::GetVehiclePool().GetAt(ScriptParams[4]);
            CVector offset(*(f32*)&ScriptParams[1], *(f32*)&ScriptParams[2], *(f32*)&ScriptParams[3]);
            CVector lookAt(0.0f, 0.0f, 0.0f);
            TheCamera.TakeControlAttachToEntity(pVehicle, pPed, offset, lookAt, *(f32*)&ScriptParams[5], static_cast<i16>(ScriptParams[6]), 1);
            return OR_CONTINUE;
        }
        case COMMAND_ATTACH_CAMERA_TO_CHAR_LOOK_AT_CHAR:
        {
            CollectParameters(7);
            pPed = CPools::GetPedPool().GetAt(ScriptParams[0]);
            pSecondPed = CPools::GetPedPool().GetAt(ScriptParams[4]);
            CVector offset(*(f32*)&ScriptParams[1], *(f32*)&ScriptParams[2], *(f32*)&ScriptParams[3]);
            CVector lookAt(0.0f, 0.0f, 0.0f);
            TheCamera.TakeControlAttachToEntity(pSecondPed, pPed, offset, lookAt, *(f32*)&ScriptParams[5], static_cast<i16>(ScriptParams[6]), 1);
            return OR_CONTINUE;
        }
        case COMMAND_FORCE_CAR_LIGHTS:
        {
            CollectParameters(2);
            pVehicle = CPools::GetVehiclePool().GetAt(ScriptParams[0]);
            pVehicle->OverrideLights = ScriptParams[1];
            return OR_CONTINUE;
        }
        case COMMAND_ADD_PEDTYPE_AS_ATTRACTOR_USER:
        {
            CollectParameters(2);
            ActualIndex = CTheScripts::GetActualScriptThingIndex(ScriptParams[0], UNIQUE_SCRIPT_ATTRACTOR);
            if (ActualIndex >= 0 && ActualIndex < 64)
            {
                CScripted2dEffects::ms_userLists[ActualIndex].AddUserByType(static_cast<ePedType>(ScriptParams[1]));
            }
            return OR_CONTINUE;
        }
        case COMMAND_ATTACH_OBJECT_TO_CAR:
        {
            CollectParameters(8);
            pObj = CPools::GetObjectPool().GetAt(ScriptParams[0]);
            pVehicle = CPools::GetVehiclePool().GetAt(ScriptParams[1]);
            CVector offset(*(f32*)&ScriptParams[2], *(f32*)&ScriptParams[3], *(f32*)&ScriptParams[4]);
            CVector rotation(*(f32*)&ScriptParams[5] * 0.017453292f, *(f32*)&ScriptParams[6] * 0.017453292f, *(f32*)&ScriptParams[7] * 0.017453292f);
            pObj->AttachEntityToEntity(pVehicle, offset, rotation);
            return OR_CONTINUE;
        }
        case COMMAND_DETACH_OBJECT:
        {
            CollectParameters(5);
            pObj = CPools::GetObjectPool().GetAt(ScriptParams[0]);
            if (pObj && pObj->m_pAttachToEntity)
            {
                pObj->DettachEntityFromEntity(*(f32*)&ScriptParams[1] * 0.017453292f, *(f32*)&ScriptParams[2] * -0.017453292f, *(f32*)&ScriptParams[3], ScriptParams[4] != 0);
            }
            return OR_CONTINUE;
        }
        case COMMAND_ATTACH_CAR_TO_CAR:
        {
            CollectParameters(8);
            pVehicle = CPools::GetVehiclePool().GetAt(ScriptParams[0]);
            pSecondVehicle = CPools::GetVehiclePool().GetAt(ScriptParams[1]);
            CVector offset(*(f32*)&ScriptParams[2], *(f32*)&ScriptParams[3], *(f32*)&ScriptParams[4]);
            CVector rotation(*(f32*)&ScriptParams[5] * 0.017453292f, *(f32*)&ScriptParams[6] * 0.017453292f, *(f32*)&ScriptParams[7] * 0.017453292f);
            if (*(f32*)&ScriptParams[2] > -999.9f)
            {
                pVehicle->AttachEntityToEntity(pSecondVehicle, offset, rotation);
            }
            else
            {
                pVehicle->AttachEntityToEntity(pSecondVehicle, (CVector*)nullptr, (CQuaternion*)nullptr);
            }
            return OR_CONTINUE;
        }
        case COMMAND_DETACH_CAR:
        {
            CollectParameters(5);
            pVehicle = CPools::GetVehiclePool().GetAt(ScriptParams[0]);
            if (pVehicle && pVehicle->m_pAttachToEntity)
            {
                pVehicle->DettachEntityFromEntity(*(f32*)&ScriptParams[1] * 0.017453292f, *(f32*)&ScriptParams[2] * -0.017453292f, *(f32*)&ScriptParams[3], ScriptParams[4] != 0);
            }
            return OR_CONTINUE;
        }
        case COMMAND_IS_OBJECT_ATTACHED:
        {
            CollectParameters(1);
            pObj = CPools::GetObjectPool().GetAt(ScriptParams[0]);
            UpdateCompareFlag(pObj && pObj->m_pAttachToEntity != nullptr);
            return OR_CONTINUE;
        }
        case COMMAND_IS_VEHICLE_ATTACHED:
        {
            CollectParameters(1);
            pVehicle = CPools::GetVehiclePool().GetAt(ScriptParams[0]);
            UpdateCompareFlag(pVehicle && pVehicle->m_pAttachToEntity != nullptr);
            return OR_CONTINUE;
        }
        case COMMAND_CLEAR_CHAR_TASKS:
        {
            CollectParameters(1);
            pPed = CPools::GetPedPool().GetAt(ScriptParams[0]);
            pPed->GetPedIntelligence()->ClearTasks(true, true);
            return OR_CONTINUE;
        }
        case COMMAND_TASK_TOGGLE_PED_THREAT_SCANNER:
        {
            CollectParameters(4);
            bool bScanThreats = (ScriptParams[1] != 0);
            bool bScanScripted = (ScriptParams[2] != 0);
            bool bScanOther = (ScriptParams[3] != 0);
            if (ScriptParams[0] == -1)
            {
                pTask = new CTaskSimpleTogglePedThreatScanner(bScanThreats, bScanScripted, bScanOther);
                CTaskSequences::ms_taskSequence[CTaskSequences::ms_iActiveSequence].AddTask(pTask);
            }
            else
            {
                pPed = CPools::GetPedPool().GetAt(ScriptParams[0]);
                CTaskSimpleTogglePedThreatScanner task(bScanThreats, bScanScripted, bScanOther);
                task.ProcessPed(pPed);
            }
            return OR_CONTINUE;
        }
        case COMMAND_POP_CAR_DOOR:
        {
            CollectParameters(3);
            pVehicle = CPools::GetVehiclePool().GetAt(ScriptParams[0]);
            eDoors door = static_cast<eDoors>(ScriptParams[1]);
            CAutomobile* pCar = static_cast<CAutomobile*>(pVehicle);
            pCar->PopDoor(CDamageManager::GetCarNodeIndexFromDoor(door), door, ScriptParams[2] != 0);
            return OR_CONTINUE;
        }
        case COMMAND_FIX_CAR_DOOR:
        {
            CollectParameters(2);
            pVehicle = CPools::GetVehiclePool().GetAt(ScriptParams[0]);
            eDoors door = static_cast<eDoors>(ScriptParams[1]);
            CAutomobile* pCar = static_cast<CAutomobile*>(pVehicle);
            pCar->FixDoor(CDamageManager::GetCarNodeIndexFromDoor(door), door);
            return OR_CONTINUE;
        }
        case COMMAND_TASK_EVERYONE_LEAVE_CAR:
        {
            CollectParameters(1);
            pVehicle = CPools::GetVehiclePool().GetAt(ScriptParams[0]);
            if (pVehicle->pDriver)
            {
                pTask = new CTaskComplexLeaveAnyCar(0, true, false);
                GivePedScriptedTask(CPools::GetPedPool().GetIndex(pVehicle->pDriver), pTask, CurrCommand);
            }
            loop = 0;
            for (i32 i = 0; i < pVehicle->m_nMaxPassengers; i++)
            {
                if (pVehicle->pPassengers[i])
                {
                    pTask = new CTaskComplexLeaveAnyCar(CGeneral::GetRandomNumberInRange(-250, 250) + loop + 500, true, false);
                    GivePedScriptedTask(CPools::GetPedPool().GetIndex(pVehicle->pPassengers[i]), pTask, CurrCommand);
                }
                loop += 500;
            }
            return OR_CONTINUE;
        }
        case COMMAND_IS_PLAYER_TARGETTING_ANYTHING:
        {
            CollectParameters(1);
            pPed = CWorld::Players[ScriptParams[0]].m_pPed;
            UpdateCompareFlag(pPed && pPed->GetWeaponLockOnTarget() != nullptr);
            return OR_CONTINUE;
        }
        case COMMAND_GET_ACTIVE_CAMERA_COORDINATES:
        {
            CCam& cam = TheCamera.Cams[TheCamera.ActiveCam];
            *(CVector*)&ScriptParams[0] = cam.Source;
            StoreParameters(3);
            return OR_CONTINUE;
        }
        case COMMAND_GET_ACTIVE_CAMERA_POINT_AT:
        {
            CCam& cam = TheCamera.Cams[TheCamera.ActiveCam];
            *(CVector*)&ScriptParams[0] = cam.Front + cam.Source;
            StoreParameters(3);
            return OR_CONTINUE;
        }
        case COMMAND_GET_CLOSEST_BUYABLE_OBJECT_TO_PLAYER:
            return OR_CONTINUE;
        case COMMAND_POP_CAR_PANEL:
        {
            CollectParameters(3);
            pVehicle = CPools::GetVehiclePool().GetAt(ScriptParams[0]);
            ePanels panel = static_cast<ePanels>(ScriptParams[1]);
            CAutomobile* pCar = static_cast<CAutomobile*>(pVehicle);
            pCar->PopPanel(CDamageManager::GetCarNodeIndexFromPanel(panel), panel, ScriptParams[2] != 0);
            return OR_CONTINUE;
        }
        case COMMAND_FIX_CAR_PANEL:
        {
            CollectParameters(2);
            pVehicle = CPools::GetVehiclePool().GetAt(ScriptParams[0]);
            ePanels panel = static_cast<ePanels>(ScriptParams[1]);
            CAutomobile* pCar = static_cast<CAutomobile*>(pVehicle);
            pCar->FixPanel(CDamageManager::GetCarNodeIndexFromPanel(panel), panel);
            return OR_CONTINUE;
        }
        case COMMAND_FIX_CAR_TYRE:
        {
            CollectParameters(2);
            pVehicle = CPools::GetVehiclePool().GetAt(ScriptParams[0]);
            CAutomobile* pCar = static_cast<CAutomobile*>(pVehicle);
            pCar->FixTyre(static_cast<eWheels>(ScriptParams[1]));
            return OR_CONTINUE;
        }
        case COMMAND_ATTACH_OBJECT_TO_OBJECT:
        {
            CollectParameters(8);
            pObj = CPools::GetObjectPool().GetAt(ScriptParams[0]);
            CObject* pSecondObj = CPools::GetObjectPool().GetAt(ScriptParams[1]);
            CVector offset(*(f32*)&ScriptParams[2], *(f32*)&ScriptParams[3], *(f32*)&ScriptParams[4]);
            CVector rotation(*(f32*)&ScriptParams[5] * 0.017453292f, *(f32*)&ScriptParams[6] * 0.017453292f, *(f32*)&ScriptParams[7] * 0.017453292f);
            pObj->AttachEntityToEntity(pSecondObj, offset, rotation);
            return OR_CONTINUE;
        }
        case COMMAND_ATTACH_OBJECT_TO_CHAR:
        {
            CollectParameters(8);
            pObj = CPools::GetObjectPool().GetAt(ScriptParams[0]);
            pPed = CPools::GetPedPool().GetAt(ScriptParams[1]);
            CVector offset(*(f32*)&ScriptParams[2], *(f32*)&ScriptParams[3], *(f32*)&ScriptParams[4]);
            CVector rotation(*(f32*)&ScriptParams[5] * 0.017453292f, *(f32*)&ScriptParams[6] * 0.017453292f, *(f32*)&ScriptParams[7] * 0.017453292f);
            pObj->AttachEntityToEntity(pPed, offset, rotation);
            return OR_CONTINUE;
        }
        case COMMAND_ATTACH_CAMERA_TO_OBJECT:
            return OR_CONTINUE;
        case COMMAND_ATTACH_CAMERA_TO_OBJECT_LOOK_AT_VEHICLE:
            return OR_CONTINUE;
        case COMMAND_ATTACH_CAMERA_TO_OBJECT_LOOK_AT_CHAR:
            return OR_CONTINUE;
        case COMMAND_ATTACH_CAMERA_TO_OBJECT_LOOK_AT_OBJECT:
            return OR_CONTINUE;
        case COMMAND_ATTACH_CAMERA_TO_CHAR_LOOK_AT_OBJECT:
            return OR_CONTINUE;
        case COMMAND_ATTACH_CAMERA_TO_VEHICLE_LOOK_AT_OBJECT:
            return OR_CONTINUE;
        case COMMAND_GET_CAR_SPEED_VECTOR:
        {
            CollectParameters(1);
            pVehicle = CPools::GetVehiclePool().GetAt(ScriptParams[0]);
            CVector speed = pVehicle->m_vecMoveSpeed * 50.0f;
            *(CVector*)&ScriptParams[0] = speed;
            StoreParameters(3);
            return OR_CONTINUE;
        }
        case COMMAND_GET_CAR_MASS:
        {
            CollectParameters(1);
            pVehicle = CPools::GetVehiclePool().GetAt(ScriptParams[0]);
            *(f32*)&ScriptParams[0] = pVehicle->m_fMass;
            StoreParameters(1);
            return OR_CONTINUE;
        }
        default:
            return OR_INTERRUPT;
    }
}

// MARK: 1700To1799

// stub: sa 0x496E00
OpcodeResult CRunningScript::ProcessCommands1700To1799(i32 CurrCommand)
{
    MARKFUNCTION(0x496E00);

    CRect area; // default is the inverted "empty" rect (0x4041C0); the value ctor would assert on it

    // NOTE: Keep switch cases in strictly ascending order as in commands.hpp to preserve MSVC jump table layout
    switch (CurrCommand)
    {
        case COMMAND_TASK_KILL_THREATS_ON_FOOT_WHILE_DUCKING:
            return OR_CONTINUE;

        case COMMAND_TASK_DIVE_FROM_ATTACHMENT_AND_GET_UP:
        {
            CollectParameters(2);
            CTask* pTask = new CTaskComplexDiveFromAttachedEntityAndGetUp(ScriptParams[1]);
            GivePedScriptedTask(ScriptParams[0], pTask, CurrCommand);
            return OR_CONTINUE;
        }

        case COMMAND_TASK_PLAY_ANIM_WITH_VELOCITY_EXTRACTION:
            return OR_CONTINUE;

        case COMMAND_ATTACH_CHAR_TO_BIKE:
        {
            CollectParameters(9);
            CPed* pPed = CPools::GetPedPool().GetAt(ScriptParams[0]);
            CVehicle* pVehicle = CPools::GetVehiclePool().GetAt(ScriptParams[1]);
            pPed->AttachPedToBike(
                pVehicle,
                *reinterpret_cast<CVector*>(&ScriptParams[2]),
                static_cast<u16>(ScriptParams[5]),
                DEGTORAD(*(f32*)&ScriptParams[6]),
                DEGTORAD(*(f32*)&ScriptParams[7]),
                static_cast<eWeaponType>(ScriptParams[8])
            );
            return OR_CONTINUE;
        }

        case COMMAND_TASK_GOTO_CHAR_OFFSET:
        {
            CollectParameters(5);
            CPed* pTargetPed = CPools::GetPedPool().GetAt(ScriptParams[1]);
            f32 fDist = *(f32*)&ScriptParams[3];
            f32 fAngle = DEGTORAD(*(f32*)&ScriptParams[4]);
            i32 time = ScriptParams[2];
            TTaskComplexSeekEntityRadiusAngleOffset* pTask = nullptr;
            if (time < 0)
            {
                pTask = new TTaskComplexSeekEntityRadiusAngleOffset(pTargetPed, 50000, 1000, 2.0f, 2.0f, 2.0f, true, true);
            }
            else
            {
                pTask = new TTaskComplexSeekEntityRadiusAngleOffset(pTargetPed, time, 1000, 2.0f, 2.0f, 2.0f, true, true);
            }
            pTask->SetEntitySeekPosCalculator(CEntitySeekPosCalculatorRadiusAngleOffset(fDist, fAngle));
            GivePedScriptedTask(ScriptParams[0], pTask, CurrCommand);
            return OR_CONTINUE;
        }

        case COMMAND_TASK_LOOK_AT_COORD:
        {
            CollectParameters(5);
            CVector posn(*(f32*)&ScriptParams[1], *(f32*)&ScriptParams[2], *(f32*)&ScriptParams[3]);
            i32 time = ScriptParams[4];
            if (time == -1)
                time = 20000;
            else if (time == -2)
                time = 1000000;
            if (ScriptParams[0] != -1)
            {
                CPed* pPed = CPools::GetPedPool().GetAt(ScriptParams[0]);
                g_ikChainMan.LookAt("COMMAND_TASK_LOOK_AT_COORD", pPed, nullptr, time, static_cast<eBoneTag>(-1), &posn, false, 0.25f, 500, 6, true);
            }
            else
            {
                CTaskSimpleTriggerLookAt* pTask = new CTaskSimpleTriggerLookAt(nullptr, time, -1, *&posn, true, 0.25f, 1000, 3);
                CTaskSequences::ms_taskSequence[CTaskSequences::ms_iActiveSequence].AddTask(pTask);
            }
            return OR_CONTINUE;
        }

        case COMMAND_IS_RECORDING_GOING_ON_FOR_CAR:
        {
            CollectParameters(1);
            UpdateCompareFlag(false);
            return OR_CONTINUE;
        }

        case COMMAND_HIDE_CHAR_WEAPON_FOR_SCRIPTED_CUTSCENE:
        {
            CollectParameters(2);
            CPed* pPed = CPools::GetPedPool().GetAt(ScriptParams[0]);
            if (ScriptParams[1])
            {
                pPed->ReplaceWeaponForScriptedCutscene();
            }
            else
            {
                pPed->RemoveWeaponForScriptedCutscene();
                if (pPed->bInVehicle)
                {
                    pPed->RemoveWeaponWhenEnteringVehicle(0);
                }
            }
            return OR_CONTINUE;
        }

        case COMMAND_GET_CHAR_SPEED:
        {
            CollectParameters(1);
            CPed* pPed = CPools::GetPedPool().GetAt(ScriptParams[0]);
            CPhysical* pEntity = (pPed->bInVehicle && pPed->m_pMyVehicle) ? static_cast<CPhysical*>(pPed->m_pMyVehicle) : static_cast<CPhysical*>(pPed);
            f32 speed = pEntity->m_vecMoveSpeed.Magnitude() * 50.0f;
            *(f32*)&ScriptParams[0] = speed;
            StoreParameters(1);
            return OR_CONTINUE;
        }

        case COMMAND_SET_GROUP_DECISION_MAKER:
        {
            CollectParameters(2);
            i32 groupIndex = CTheScripts::GetActualScriptThingIndex(ScriptParams[0], UNIQUE_SCRIPT_PEDGROUP);
            if (groupIndex >= 0 && groupIndex < 8)
            {
                i32 dmIndex = -1;
                if (ScriptParams[1] != -1)
                {
                    dmIndex = CTheScripts::GetActualScriptThingIndex(ScriptParams[1], UNIQUE_SCRIPT_DECISION_MAKER);
                }
                CPedGroups::ms_groups[groupIndex].GetGroupIntelligence()->SetGroupDecisionMakerType(static_cast<eDecisionMakerType>(dmIndex));
            }
            return OR_CONTINUE;
        }

        case COMMAND_LOAD_GROUP_DECISION_MAKER:
        {
            CollectParameters(1);
            char dmName[256];
            CDecisionMakerTypesFileLoader::GetGrpDMName(ScriptParams[0], dmName);
            u32 nextParam = CollectNextParameterWithoutIncreasingPC();
            CTheScripts::GetActualScriptThingIndex(nextParam, UNIQUE_SCRIPT_DECISION_MAKER);
            i32 dmId = CDecisionMakerTypesFileLoader::LoadDecisionMaker(dmName, static_cast<eDecisionTypes>(1), IsThisAMissionScript);
            ScriptParams[0] = CTheScripts::GetNewUniqueScriptThingIndex(dmId, UNIQUE_SCRIPT_DECISION_MAKER);
            StoreParameters(1);
            if (IsThisAMissionScript)
            {
                CTheScripts::MissionCleanUp.AddEntityToList(ScriptParams[0], CLEANUP_DECISION_MAKER);
            }
            return OR_CONTINUE;
        }

        case COMMAND_DISABLE_PLAYER_SPRINT:
        {
            CollectParameters(2);
            CPlayerInfo* pPlayer = &CWorld::Players[ScriptParams[0]];
            if (ScriptParams[1])
                pPlayer->m_pPed->m_pPlayerData->m_bPlayerSprintDisabled = true;
            else
                pPlayer->m_pPed->m_pPlayerData->m_bPlayerSprintDisabled = false;
            return OR_CONTINUE;
        }

        case COMMAND_TASK_SIT_DOWN:
        {
            CollectParameters(2);
            CTask* pTask = nullptr;
            if (ScriptParams[1] == -1)
                pTask = new CTaskComplexSitDownThenIdleThenStandUp(100000000, 0, 0);
            else
                pTask = new CTaskComplexSitDownThenIdleThenStandUp(ScriptParams[1], 0, 0);
            GivePedScriptedTask(ScriptParams[0], pTask, CurrCommand);
            return OR_CONTINUE;
        }

        case COMMAND_CREATE_SEARCHLIGHT:
        {
            CollectParameters(8);
            u32 nextParam = CollectNextParameterWithoutIncreasingPC();
            CTheScripts::GetActualScriptThingIndex(nextParam, UNIQUE_SCRIPT_SEARCHLIGHT);
            i32 id = CTheScripts::AddScriptSearchLight(
                *(f32*)&ScriptParams[0], *(f32*)&ScriptParams[1], *(f32*)&ScriptParams[2],
                nullptr,
                *(f32*)&ScriptParams[3], *(f32*)&ScriptParams[4], *(f32*)&ScriptParams[5],
                *(f32*)&ScriptParams[6], *(f32*)&ScriptParams[7]
            );
            ScriptParams[0] = CTheScripts::GetNewUniqueScriptThingIndex(id, UNIQUE_SCRIPT_SEARCHLIGHT);
            StoreParameters(1);
            if (IsThisAMissionScript)
            {
                CTheScripts::MissionCleanUp.AddEntityToList(ScriptParams[0], CLEANUP_SEARCHLIGHT);
            }
            return OR_CONTINUE;
        }

        case COMMAND_DELETE_SEARCHLIGHT:
        {
            CollectParameters(1);
            if (CTheScripts::GetActualScriptThingIndex(ScriptParams[0], UNIQUE_SCRIPT_SEARCHLIGHT) >= 0)
            {
                CTheScripts::RemoveScriptSearchLight(ScriptParams[0]);
                if (IsThisAMissionScript)
                {
                    CTheScripts::MissionCleanUp.RemoveEntityFromList(ScriptParams[0], CLEANUP_SEARCHLIGHT);
                }
            }
            return OR_CONTINUE;
        }

        case COMMAND_DOES_SEARCHLIGHT_EXIST:
        {
            CollectParameters(1);
            UpdateCompareFlag(CTheScripts::GetActualScriptThingIndex(ScriptParams[0], UNIQUE_SCRIPT_SEARCHLIGHT) >= 0);
            return OR_CONTINUE;
        }

        case COMMAND_MOVE_SEARCHLIGHT_BETWEEN_COORDS:
        {
            CollectParameters(8);
            CTheScripts::MoveSearchLightBetweenTwoPoints(
                ScriptParams[0],
                *(f32*)&ScriptParams[1], *(f32*)&ScriptParams[2], *(f32*)&ScriptParams[3],
                *(f32*)&ScriptParams[4], *(f32*)&ScriptParams[5], *(f32*)&ScriptParams[6],
                *(f32*)&ScriptParams[7]
            );
            return OR_CONTINUE;
        }

        case COMMAND_POINT_SEARCHLIGHT_AT_COORD:
        {
            CollectParameters(5);
            CTheScripts::MoveSearchLightToPointAndStop(
                ScriptParams[0],
                *(f32*)&ScriptParams[1], *(f32*)&ScriptParams[2], *(f32*)&ScriptParams[3],
                *(f32*)&ScriptParams[4]
            );
            return OR_CONTINUE;
        }

        case COMMAND_POINT_SEARCHLIGHT_AT_CHAR:
        {
            CollectParameters(3);
            CPed* pPed = CPools::GetPedPool().GetAt(ScriptParams[1]);
            CTheScripts::MoveSearchLightToEntity(ScriptParams[0], pPed, *(f32*)&ScriptParams[2]);
            return OR_CONTINUE;
        }

        case COMMAND_IS_CHAR_IN_SEARCHLIGHT:
        {
            CollectParameters(2);
            CPed* pPed = CPools::GetPedPool().GetAt(ScriptParams[1]);
            UpdateCompareFlag(CTheScripts::IsEntityWithinSearchLight(ScriptParams[0], pPed));
            return OR_CONTINUE;
        }

        case COMMAND_SET_GROUP_DEFAULT_TASK:
            return OR_INTERRUPT;

        case COMMAND_HAS_CUTSCENE_LOADED:
        {
            UpdateCompareFlag(CCutsceneMgr::HasLoaded());
            return OR_CONTINUE;
        }

        case COMMAND_TASK_TURN_CHAR_TO_FACE_COORD:
        {
            CollectParameters(4);
            CVector vecTarget(*(f32*)&ScriptParams[1], *(f32*)&ScriptParams[2], *(f32*)&ScriptParams[3]);
            CTask* pTask = new CTaskComplexTurnToFaceEntityOrCoord(vecTarget, 0.5f, 0.2f);
            GivePedScriptedTask(ScriptParams[0], pTask, CurrCommand);
            return OR_CONTINUE;
        }

        case COMMAND_TASK_DRIVE_POINT_ROUTE:
        {
            CollectParameters(3);
            CVehicle* pVehicle = ScriptParams[1] >= 0 ? CPools::GetVehiclePool().GetAt(ScriptParams[1]) : nullptr;
            CTask* pTask = new CTaskComplexDrivePointRoute(pVehicle, CTaskComplexFollowPointRoute::ms_pointRoute, *(f32*)&ScriptParams[2], 2, static_cast<eModelID>(-1), -1.0f, static_cast<eCarDrivingStyle>(0));
            GivePedScriptedTask(ScriptParams[0], pTask, CurrCommand);
            return OR_CONTINUE;
        }

        case COMMAND_FIRE_SINGLE_BULLET:
        {
            CollectParameters(7);
            CVector p1(*(f32*)&ScriptParams[0], *(f32*)&ScriptParams[1], *(f32*)&ScriptParams[2]);
            CVector p2(*(f32*)&ScriptParams[3], *(f32*)&ScriptParams[4], *(f32*)&ScriptParams[5]);
            FireOneInstantHitRound(p1, p2, ScriptParams[6]);
            return OR_CONTINUE;
        }

        case COMMAND_IS_LINE_OF_SIGHT_CLEAR:
        {
            CollectParameters(11);
            CVector p1(*(f32*)&ScriptParams[0], *(f32*)&ScriptParams[1], *(f32*)&ScriptParams[2]);
            CVector p2(*(f32*)&ScriptParams[3], *(f32*)&ScriptParams[4], *(f32*)&ScriptParams[5]);
            UpdateCompareFlag(CWorld::GetIsLineOfSightClear(
                p1, p2,
                ScriptParams[6] != 0,
                ScriptParams[7] != 0,
                ScriptParams[8] != 0,
                ScriptParams[9] != 0,
                ScriptParams[10] != 0,
                false, false
            ));
            return OR_CONTINUE;
        }

        case COMMAND_GET_CAR_ROLL:
        {
            CollectParameters(1);
            CVehicle* pVehicle = CPools::GetVehiclePool().GetAt(ScriptParams[0]);
            *(f32*)&ScriptParams[0] = static_cast<CAutomobile*>(pVehicle)->GetRoll();
            StoreParameters(1);
            return OR_CONTINUE;
        }

        case COMMAND_POINT_SEARCHLIGHT_AT_VEHICLE:
        {
            CollectParameters(3);
            CVehicle* pVehicle = CPools::GetVehiclePool().GetAt(ScriptParams[1]);
            CTheScripts::MoveSearchLightToEntity(ScriptParams[0], pVehicle, *(f32*)&ScriptParams[2]);
            return OR_CONTINUE;
        }

        case COMMAND_IS_VEHICLE_IN_SEARCHLIGHT:
        {
            CollectParameters(2);
            CVehicle* pVehicle = CPools::GetVehiclePool().GetAt(ScriptParams[1]);
            UpdateCompareFlag(CTheScripts::IsEntityWithinSearchLight(ScriptParams[0], pVehicle));
            return OR_CONTINUE;
        }

        case COMMAND_CREATE_SEARCHLIGHT_ON_VEHICLE:
        {
            CollectParameters(9);
            u32 nextParam = CollectNextParameterWithoutIncreasingPC();
            CTheScripts::GetActualScriptThingIndex(nextParam, UNIQUE_SCRIPT_SEARCHLIGHT);
            CVehicle* pVehicle = CPools::GetVehiclePool().GetAt(ScriptParams[0]);
            i32 id = CTheScripts::AddScriptSearchLight(
                *(f32*)&ScriptParams[1], *(f32*)&ScriptParams[2], *(f32*)&ScriptParams[3],
                pVehicle,
                *(f32*)&ScriptParams[4], *(f32*)&ScriptParams[5], *(f32*)&ScriptParams[6],
                *(f32*)&ScriptParams[7], *(f32*)&ScriptParams[8]
            );
            ScriptParams[0] = CTheScripts::GetNewUniqueScriptThingIndex(id, UNIQUE_SCRIPT_SEARCHLIGHT);
            if (IsThisAMissionScript)
            {
                CTheScripts::MissionCleanUp.AddEntityToList(ScriptParams[0], CLEANUP_SEARCHLIGHT);
            }
            StoreParameters(1);
            return OR_CONTINUE;
        }

        case COMMAND_TASK_GO_TO_COORD_WHILE_AIMING:
        {
            CollectParameters(11);
            CVector targetPos(*(f32*)&ScriptParams[1], *(f32*)&ScriptParams[2], *(f32*)&ScriptParams[3]);
            CVector aimPos(*(f32*)&ScriptParams[8], *(f32*)&ScriptParams[9], *(f32*)&ScriptParams[10]);
            CPed* pTargetPed = ScriptParams[7] >= 0 ? CPools::GetPedPool().GetAt(ScriptParams[7]) : nullptr;
            CTask* pTask = new CTaskComplexGoToPointAiming(static_cast<eMoveState>(
                ScriptParams[4]), targetPos, pTargetPed, aimPos, *(f32*)&ScriptParams[5], *(f32*)&ScriptParams[6]
            );
            GivePedScriptedTask(ScriptParams[0], pTask, CurrCommand);
            return OR_CONTINUE;
        }

        case COMMAND_GET_NUMBER_OF_FIRES_IN_RANGE:
        {
            CollectParameters(4);
            CVector pos(*(f32*)&ScriptParams[0], *(f32*)&ScriptParams[1], *(f32*)&ScriptParams[2]);
            ScriptParams[0] = gFireManager.GetNumFiresInRange(pos, *(f32*)&ScriptParams[3]);
            StoreParameters(1);
            return OR_CONTINUE;
        }

        case COMMAND_ADD_BLIP_FOR_SEARCHLIGHT:
        {
            CollectParameters(1);
            CTheScripts::GetActualScriptThingIndex(ScriptParams[0], UNIQUE_SCRIPT_SEARCHLIGHT);
            u32 nextParam = CollectNextParameterWithoutIncreasingPC();
            CRadar::GetActualBlipArrayIndex(nextParam);
            i32 blip = CRadar::SetEntityBlip(BLIPTYPE_SEARCHLIGHT, ScriptParams[0], 0, BLIPDISPLAY_BOTH);
            CRadar::ChangeBlipScale(blip, 3);
            ScriptParams[0] = blip;
            StoreParameters(1);
            return OR_CONTINUE;
        }

        case COMMAND_SKIP_TO_END_AND_STOP_PLAYBACK_RECORDED_CAR:
        {
            CollectParameters(1);
            CVehicle* pVehicle = CPools::GetVehiclePool().GetAt(ScriptParams[0]);
            CVehicleRecording::SkipToEndAndStopPlaybackRecordedCar(pVehicle);
            return OR_CONTINUE;
        }

        case COMMAND_TASK_OPEN_DRIVER_DOOR:
            return OR_CONTINUE;

        case COMMAND_TASK_CAR_TEMP_ACTION:
        {
            CollectParameters(3);
            CVehicle* pVehicle = CPools::GetVehiclePool().GetAt(ScriptParams[1]);
            CTask* pTask = new CTaskSimpleCarSetTempAction(pVehicle, static_cast<eAutoPilotTempAction>(ScriptParams[2]), ScriptParams[0]);
            GivePedScriptedTask(ScriptParams[0], pTask, CurrCommand);
            return OR_CONTINUE;
        }

        case COMMAND_SET_LA_RIOTS:
        {
            CollectParameters(1);
            gbLARiots = (ScriptParams[0] != 0);
            return OR_CONTINUE;
        }

        case COMMAND_REMOVE_CHAR_FROM_GROUP:
        {
            CollectParameters(1);
            CPed* pPed = CPools::GetPedPool().GetAt(ScriptParams[0]);
            CPedGroup* pGroup = CPedGroups::GetPedsGroup(pPed);
            if (pGroup)
            {
                pGroup->GetGroupMembership()->RemoveMember(pPed);
            }
            return OR_CONTINUE;
        }

        case COMMAND_ATTACH_SEARCHLIGHT_TO_SEARCHLIGHT_OBJECT:
        {
            CollectParameters(7);
            CObject* pTower = CPools::GetObjectPool().GetAt(ScriptParams[1]);
            CObject* pHousing = CPools::GetObjectPool().GetAt(ScriptParams[2]);
            CObject* pBulb = CPools::GetObjectPool().GetAt(ScriptParams[3]);
            CTheScripts::AttachSearchlightToSearchlightObject(
                ScriptParams[0], pTower, pHousing, pBulb,
                *(f32*)&ScriptParams[4], *(f32*)&ScriptParams[5], *(f32*)&ScriptParams[6]
            );
            return OR_CONTINUE;
        }

        case COMMAND_SET_VEHICLE_RECORDS_COLLISIONS:
            return OR_CONTINUE;

        case COMMAND_DRAW_CROSS_IN_FRONT_OF_DEBUG_CAMERA:
            return OR_INTERRUPT;

        case COMMAND_DRAW_DEBUG_CUBE:
            return OR_INTERRUPT;

        case COMMAND_GET_CAR_LAST_ROUTE_COORDS:
            return OR_CONTINUE;

        case COMMAND_DISPLAY_TIMER_BARS:
        {
            CollectParameters(1);
            return OR_CONTINUE;
        }

        case COMMAND_SWITCH_EMERGENCY_SERVICES:
        {
            CollectParameters(1);
            CCarCtrl::bAllowEmergencyServicesToBeCreated = (ScriptParams[0] != 0);
            return OR_CONTINUE;
        }

        case COMMAND_SET_VAR_TEXT_LABEL16:
        {
            i32* pVar = GetPointerToScriptVariable(SCOPE_GLOBAL);
            char text[16];
            ReadTextLabelFromScript(text, 16);
            strncpy(reinterpret_cast<char*>(pVar), text, 16);
            return OR_CONTINUE;
        }

        case COMMAND_SET_LVAR_TEXT_LABEL16:
        {
            i32* pVar = GetPointerToScriptVariable(SCOPE_LOCAL);
            char text[16];
            ReadTextLabelFromScript(text, 16);
            strncpy(reinterpret_cast<char*>(pVar), text, 16);
            return OR_CONTINUE;
        }

        case COMMAND_VAR_TEXT_LABEL16:
            return OR_INTERRUPT;

        case COMMAND_LVAR_TEXT_LABEL16:
            return OR_INTERRUPT;

        case COMMAND_CREATE_CHECKPOINT:
        {
            CollectParameters(8);
            u32 nextParam = CollectNextParameterWithoutIncreasingPC();
            CTheScripts::GetActualScriptThingIndex(nextParam, UNIQUE_SCRIPT_CHECKPOINT);
            ScriptParams[0] = CTheScripts::AddScriptCheckpoint(
                *(f32*)&ScriptParams[1], *(f32*)&ScriptParams[2], *(f32*)&ScriptParams[3],
                *(f32*)&ScriptParams[4], *(f32*)&ScriptParams[5], *(f32*)&ScriptParams[6],
                *(f32*)&ScriptParams[7], ScriptParams[0]
            );
            if (IsThisAMissionScript)
            {
                CTheScripts::MissionCleanUp.AddEntityToList(ScriptParams[0], CLEANUP_CHECKPOINT);
            }
            StoreParameters(1);
            return OR_CONTINUE;
        }

        case COMMAND_DELETE_CHECKPOINT:
        {
            CollectParameters(1);
            if (CTheScripts::GetActualScriptThingIndex(ScriptParams[0], UNIQUE_SCRIPT_CHECKPOINT) >= 0)
            {
                CTheScripts::RemoveScriptCheckpoint(ScriptParams[0]);
                if (IsThisAMissionScript)
                {
                    CTheScripts::MissionCleanUp.RemoveEntityFromList(ScriptParams[0], CLEANUP_CHECKPOINT);
                }
            }
            return OR_CONTINUE;
        }

        case COMMAND_SWITCH_RANDOM_TRAINS:
        {
            CollectParameters(1);
            CTrain::DisableRandomTrains(ScriptParams[0] == 0);
            return OR_CONTINUE;
        }

        case COMMAND_CREATE_MISSION_TRAIN:
        {
            CollectParameters(5);
            CTrain* pTrain = nullptr;
            CVector posn(*(f32*)&ScriptParams[1], *(f32*)&ScriptParams[2], *(f32*)&ScriptParams[3]);
            CTrain::CreateMissionTrain(posn, ScriptParams[4] != 0, ScriptParams[0], &pTrain, nullptr, -1, -1, true);
            ScriptParams[0] = CPools::GetVehiclePool().GetIndex(pTrain);
            StoreParameters(1);
            return OR_CONTINUE;
        }

        case COMMAND_DELETE_MISSION_TRAINS:
        {
            CTrain::RemoveMissionTrains();
            return OR_CONTINUE;
        }

        case COMMAND_MARK_MISSION_TRAINS_AS_NO_LONGER_NEEDED:
        {
            CTrain::ReleaseMissionTrains();
            return OR_CONTINUE;
        }

        case COMMAND_DELETE_ALL_TRAINS:
        {
            CTrain::RemoveAllTrains();
            return OR_CONTINUE;
        }

        case COMMAND_SET_TRAIN_SPEED:
        {
            CollectParameters(2);
            CVehicle* pVehicle = CPools::GetVehiclePool().GetAt(ScriptParams[0]);
            CTrain::SetTrainSpeed(static_cast<CTrain*>(pVehicle), *(f32*)&ScriptParams[1]);
            return OR_CONTINUE;
        }

        case COMMAND_SET_TRAIN_CRUISE_SPEED:
        {
            CollectParameters(2);
            CVehicle* pVehicle = CPools::GetVehiclePool().GetAt(ScriptParams[0]);
            CTrain::SetTrainCruiseSpeed(static_cast<CTrain*>(pVehicle), *(f32*)&ScriptParams[1]);
            return OR_CONTINUE;
        }

        case COMMAND_GET_TRAIN_CABOOSE:
        {
            CollectParameters(1);
            CVehicle* pVehicle = CPools::GetVehiclePool().GetAt(ScriptParams[0]);
            CTrain* pCaboose = CTrain::FindCaboose(static_cast<CTrain*>(pVehicle));
            ScriptParams[0] = CPools::GetVehiclePool().GetIndex(pCaboose);
            StoreParameters(1);
            return OR_CONTINUE;
        }

        case COMMAND_DELETE_PLAYER:
        {
            CollectParameters(1);
            CPlayerPed::RemovePlayerPed(ScriptParams[0]);
            return OR_CONTINUE;
        }

        case COMMAND_SET_TWO_PLAYER_CAMERA_MODE:
        {
            CollectParameters(1);
            TheCamera.StartCooperativeCamMode();
            return OR_CONTINUE;
        }

        case COMMAND_TASK_CAR_MISSION:
        {
            CollectParameters(6);
            CVehicle* pVehicle = ScriptParams[1] >= 0 ? CPools::GetVehiclePool().GetAt(ScriptParams[1]) : nullptr;
            CVehicle* pTargetVehicle = ScriptParams[2] >= 0 ? CPools::GetVehiclePool().GetAt(ScriptParams[2]) : nullptr;
            CTask* pTask = new CTaskComplexCarDriveMission(pVehicle, pTargetVehicle, static_cast<eCarMission>(ScriptParams[3]), static_cast<eCarDrivingStyle>(ScriptParams[5]), *(f32*)&ScriptParams[4]);
            GivePedScriptedTask(ScriptParams[0], pTask, CurrCommand);
            return OR_CONTINUE;
        }

        case COMMAND_TASK_GO_TO_OBJECT:
        {
            CollectParameters(4);
            CObject* pObject = CPools::GetObjectPool().GetAt(ScriptParams[1]);
            CTask* pTask = new TTaskComplexSeekEntityStandard(pObject, ScriptParams[2], 1000, *(f32*)&ScriptParams[3], 2.0f, 2.0f, true, true);
            GivePedScriptedTask(ScriptParams[0], pTask, CurrCommand);
            return OR_CONTINUE;
        }

        case COMMAND_TASK_WEAPON_ROLL:
        {
            CollectParameters(2);
            CTask* pTask = nullptr;
            if (ScriptParams[1])
                pTask = new CTaskSimpleRunAnim(static_cast<AssocGroupId>(0), static_cast<AnimationId>(0x3B), 4.0f, false);
            else
                pTask = new CTaskSimpleRunAnim(static_cast<AssocGroupId>(0), static_cast<AnimationId>(0x39), 4.0f, false);
            GivePedScriptedTask(ScriptParams[0], pTask, CurrCommand);
            return OR_CONTINUE;
        }

        case COMMAND_TASK_CHAR_ARREST_CHAR:
        {
            CollectParameters(2);
            CPed* pTargetPed = CPools::GetPedPool().GetAt(ScriptParams[1]);
            CTask* pTask = new CTaskComplexArrestPed(pTargetPed);
            GivePedScriptedTask(ScriptParams[0], pTask, CurrCommand);
            return OR_CONTINUE;
        }

        case COMMAND_GET_AVAILABLE_VEHICLE_MOD:
        {
            CollectParameters(2);
            CVehicle* pVehicle = CPools::GetVehiclePool().GetAt(ScriptParams[0]);
            CVehicleModelInfo* pModelInfo = static_cast<CVehicleModelInfo*>(CModelInfo::GetModelInfo(pVehicle->m_nModelIndex));
            ScriptParams[0] = pModelInfo->m_anUpgrades[ScriptParams[1]];
            StoreParameters(1);
            return OR_CONTINUE;
        }

        case COMMAND_GET_VEHICLE_MOD_TYPE:
        {
            CollectParameters(1);
            i32 index = ScriptParams[0];
            if (index < 0)
            {
                index = CTheScripts::UsedObjectArray[-index].Index;
            }
            u32 flags = CModelInfo::GetModelInfo(index)->m_nFlags;
            u32 subType = (flags >> 10) & 0x1F;
            if (flags & 0x100)
            {
                switch (subType)
                {
                    case 1:  ScriptParams[0] = 11; break;
                    case 2:  ScriptParams[0] = 12; break;
                    case 12: ScriptParams[0] = 14; break;
                    case 13: ScriptParams[0] = 15; break;
                    case 19: ScriptParams[0] = 13; break;
                    case 20:
                    case 21:
                    case 22: ScriptParams[0] = 16; break;
                    default: break;
                }
            }
            else
            {
                switch (subType)
                {
                    case 0:  ScriptParams[0] = 0; break;
                    case 1:
                    case 2:  ScriptParams[0] = 1; break;
                    case 6:  ScriptParams[0] = 2; break;
                    case 8:
                    case 9:  ScriptParams[0] = 3; break;
                    case 10: ScriptParams[0] = 4; break;
                    case 11: ScriptParams[0] = 5; break;
                    case 12: ScriptParams[0] = 6; break;
                    case 14: ScriptParams[0] = 7; break;
                    case 15: ScriptParams[0] = 8; break;
                    case 16: ScriptParams[0] = 9; break;
                    case 17: ScriptParams[0] = 10; break;
                    default: break;
                }
            }
            StoreParameters(1);
            return OR_CONTINUE;
        }

        case COMMAND_ADD_VEHICLE_MOD:
        {
            CollectParameters(2);
            i32 modelId = ScriptParams[1];
            if (modelId < 0)
            {
                modelId = CTheScripts::UsedObjectArray[-modelId].Index;
            }
            CVehicle* pVehicle = CPools::GetVehiclePool().GetAt(ScriptParams[0]);
            pVehicle->AddVehicleUpgrade(modelId);
            return OR_CONTINUE;
        }

        case COMMAND_REMOVE_VEHICLE_MOD:
        {
            CollectParameters(2);
            i32 modelId = ScriptParams[1];
            if (modelId < 0)
            {
                modelId = CTheScripts::UsedObjectArray[-modelId].Index;
            }
            CVehicle* pVehicle = CPools::GetVehiclePool().GetAt(ScriptParams[0]);
            pVehicle->RemoveVehicleUpgrade(modelId);
            return OR_CONTINUE;
        }

        case COMMAND_REQUEST_VEHICLE_MOD:
        {
            CollectParameters(1);
            i32 modelId = ScriptParams[0];
            if (modelId < 0)
            {
                modelId = CTheScripts::UsedObjectArray[-modelId].Index;
            }
            CStreaming::RequestVehicleUpgrade(modelId, 12);
            return OR_CONTINUE;
        }

        case COMMAND_HAS_VEHICLE_MOD_LOADED:
        {
            CollectParameters(1);
            i32 modelId = ScriptParams[0];
            if (modelId < 0)
            {
                modelId = CTheScripts::UsedObjectArray[-modelId].Index;
            }
            UpdateCompareFlag(CStreaming::HasVehicleUpgradeLoaded(modelId));
            return OR_CONTINUE;
        }

        case COMMAND_MARK_VEHICLE_MOD_AS_NO_LONGER_NEEDED:
        {
            CollectParameters(1);
            i32 modelId = ScriptParams[0];
            if (modelId < 0)
            {
                modelId = CTheScripts::UsedObjectArray[-modelId].Index;
            }
            CStreaming::SetMissionDoesntRequireModel(modelId);
            i32 other = CVehicleModelInfo::ms_linkedUpgrades.FindOtherUpgrade(modelId);
            if (other != -1)
            {
                CStreaming::SetMissionDoesntRequireModel(other);
            }
            return OR_CONTINUE;
        }

        case COMMAND_GET_NUM_AVAILABLE_PAINTJOBS:
        {
            CollectParameters(1);
            CVehicle* pVehicle = CPools::GetVehiclePool().GetAt(ScriptParams[0]);
            CVehicleModelInfo* pModelInfo = static_cast<CVehicleModelInfo*>(CModelInfo::GetModelInfo(pVehicle->m_nModelIndex));
            ScriptParams[0] = pModelInfo->GetNumRemaps();
            StoreParameters(1);
            return OR_CONTINUE;
        }

        case COMMAND_GIVE_VEHICLE_PAINTJOB:
        {
            CollectParameters(2);
            CVehicle* pVehicle = CPools::GetVehiclePool().GetAt(ScriptParams[0]);
            pVehicle->SetRemap(ScriptParams[1]);
            return OR_CONTINUE;
        }

        case COMMAND_IS_GROUP_MEMBER:
        {
            CollectParameters(2);
            CPed* pPed = CPools::GetPedPool().GetAt(ScriptParams[0]);
            i32 groupIndex = CTheScripts::GetActualScriptThingIndex(ScriptParams[1], UNIQUE_SCRIPT_PEDGROUP);
            bool bMember = false;
            if (groupIndex >= 0 && groupIndex < 8)
            {
                bMember = CPedGroups::ms_groups[groupIndex].GetGroupMembership()->IsMember(pPed);
            }
            UpdateCompareFlag(bMember);
            return OR_CONTINUE;
        }

        case COMMAND_IS_GROUP_LEADER:
        {
            CollectParameters(2);
            CPed* pPed = CPools::GetPedPool().GetAt(ScriptParams[0]);
            i32 groupIndex = CTheScripts::GetActualScriptThingIndex(ScriptParams[1], UNIQUE_SCRIPT_PEDGROUP);
            bool bLeader = false;
            if (groupIndex >= 0 && groupIndex < 8)
            {
                bLeader = CPedGroups::ms_groups[groupIndex].GetGroupMembership()->IsLeader(pPed);
            }
            UpdateCompareFlag(bLeader);
            return OR_CONTINUE;
        }

        case COMMAND_SET_GROUP_SEPARATION_RANGE:
        {
            CollectParameters(2);
            i32 groupIndex = CTheScripts::GetActualScriptThingIndex(ScriptParams[0], UNIQUE_SCRIPT_PEDGROUP);
            if (groupIndex >= 0 && groupIndex < 8)
            {
                CPedGroups::ms_groups[groupIndex].GetGroupMembership()->SetMaxSeparation(*(f32*)&ScriptParams[1]);
            }
            return OR_CONTINUE;
        }

        case COMMAND_LIMIT_TWO_PLAYER_DISTANCE:
        {
            CollectParameters(1);
            CGameLogic::bLimitPlayerDistance = true;
            CGameLogic::MaxPlayerDistance = *(f32*)&ScriptParams[0];
            return OR_CONTINUE;
        }

        case COMMAND_RELEASE_TWO_PLAYER_DISTANCE:
        {
            CGameLogic::bLimitPlayerDistance = false;
            return OR_CONTINUE;
        }

        case COMMAND_SET_PLAYER_PLAYER_TARGETTING:
        {
            CollectParameters(1);
            CGameLogic::bPlayersCannotTargetEachOther = (ScriptParams[0] == 0);
            return OR_CONTINUE;
        }

        case COMMAND_CREATE_SCRIPT_GANG_ROADBLOCK:
            return OR_CONTINUE;

        case COMMAND_GET_SCRIPT_FIRE_COORDS:
        {
            CollectParameters(1);
            i32 fireIndex = CTheScripts::GetActualScriptThingIndex(ScriptParams[0], UNIQUE_SCRIPT_FIRE);
            if (fireIndex >= 0 && fireIndex < 60)
            {
                CVector coords = gFireManager.GetScriptFireCoords(fireIndex);
                *(CVector*)&ScriptParams[0] = coords;
            }
            else
            {
                ScriptParams[0] = 0;
                ScriptParams[1] = 0;
                ScriptParams[2] = 0;
            }
            StoreParameters(3);
            return OR_CONTINUE;
        }

        case COMMAND_CLEAR_TWO_PLAYER_CAMERA_MODE:
            return OR_CONTINUE;

        case COMMAND_SET_PLAYER_PASSENGER_CAN_SHOOT:
            return OR_CONTINUE;

        case COMMAND_GET_NTH_CLOSEST_CAR_NODE_WITH_HEADING:
        {
            CollectParameters(4);
            f32 targetZ = *(f32*)&ScriptParams[2];
            if (targetZ <= -100.0f)
            {
                targetZ = CWorld::FindGroundZForCoord(*(f32*)&ScriptParams[0], *(f32*)&ScriptParams[1]);
            }
            CVector targetCoors(*(f32*)&ScriptParams[0], *(f32*)&ScriptParams[1], targetZ);
            CNodeAddress nodeAddr;
            ThePaths.FindNthNodeClosestToCoors(
                targetCoors,
                0,
                999999.9f,
                true,
                false,
                ScriptParams[3] - 1,
                false,
                false,
                &nodeAddr
            );
            bool bFound = false;
            CVector nodeCoors;
            ThePaths.FindNodeCoorsForScript(nodeAddr, &bFound);
            if (bFound)
            {
                *(CVector*)&ScriptParams[0] = nodeCoors;
                *(f32*)&ScriptParams[3] = ThePaths.FindNodeOrientationForCarPlacement(nodeAddr);
                StoreParameters(4);
                UpdateCompareFlag(true);
            }
            else
            {
                ScriptParams[0] = 0;
                ScriptParams[1] = 0;
                ScriptParams[2] = 0;
                ScriptParams[3] = 0;
                StoreParameters(4);
                UpdateCompareFlag(false);
            }
            return OR_CONTINUE;
        }

        case COMMAND_GET_HEIGHT_OF_CAR_WHEELS_FROM_GROUND:
            return OR_CONTINUE;

        case COMMAND_SET_PLAYERS_CAN_BE_IN_SEPARATE_CARS:
        {
            CollectParameters(1);
            CGameLogic::bPlayersCanBeInSeparateCars = (ScriptParams[0] != 0);
            return OR_CONTINUE;
        }

        case COMMAND_SWITCH_PLAYER_CROSSHAIR:
            return OR_CONTINUE;

        case COMMAND_DOES_CAR_HAVE_STUCK_CAR_CHECK:
        {
            CollectParameters(1);
            UpdateCompareFlag(CTheScripts::StuckCars.IsCarInStuckCarArray(ScriptParams[0]));
            return OR_CONTINUE;
        }

        case COMMAND_SET_PLAYBACK_SPEED:
        {
            CollectParameters(2);
            CVehicle* pVehicle = CPools::GetVehiclePool().GetAt(ScriptParams[0]);
            CVehicleRecording::SetPlaybackSpeed(pVehicle, *(f32*)&ScriptParams[1]);
            return OR_CONTINUE;
        }

        case COMMAND_GET_CAR_VALUE:
            return OR_CONTINUE;

        case COMMAND_ARE_ANY_CHARS_NEAR_CHAR:
        {
            CollectParameters(2);
            CPed* pPed = CPools::GetPedPool().GetAt(ScriptParams[0]);
            f32 radius = *(f32*)&ScriptParams[1];
            f32 radiusSq = radius * radius;
            CVector pedPos = pPed->GetPosition();
            bool bFound = false;
            for (i32 i = CPools::GetPedPool().GetSize() - 1; i >= 0; --i)
            {
                if (bFound)
                    break;
                CPed* pOther = CPools::GetPedPool().GetSlot(i);
                if (pOther && pOther != pPed)
                {
                    if ((pOther->GetPosition() - pedPos).SquaredMagnitude() < radiusSq)
                    {
                        bFound = true;
                    }
                }
            }
            UpdateCompareFlag(bFound);
            return OR_CONTINUE;
        }

        case COMMAND_SKIP_CUTSCENE_START:
            return OR_CONTINUE;

        case COMMAND_SKIP_CUTSCENE_END:
        {
            EndOfScriptedCutscenePC = 0;
            return OR_CONTINUE;
        }

        case COMMAND_GET_PERCENTAGE_TAGGED_IN_AREA:
        {
            CollectParameters(4);
            f32 x1 = *(f32*)&ScriptParams[0];
            f32 y1 = *(f32*)&ScriptParams[1];
            f32 x2 = *(f32*)&ScriptParams[2];
            f32 y2 = *(f32*)&ScriptParams[3];
            if (x1 > x2)
            {
                f32 temp = x1;
                x1 = x2;
                x2 = temp;
            }
            if (y1 > y2)
            {
                f32 temp = y1;
                y1 = y2;
                y2 = temp;
            }
            area.left = x1;
            area.top = y2;
            area.right = x2;
            area.bottom = y1;
            ScriptParams[0] = CTagManager::GetPercentageTaggedInArea(area);
            StoreParameters(1);
            return OR_CONTINUE;
        }

        case COMMAND_SET_TAG_STATUS_IN_AREA:
        {
            CollectParameters(5);
            f32 x1 = *(f32*)&ScriptParams[0];
            f32 y1 = *(f32*)&ScriptParams[1];
            f32 x2 = *(f32*)&ScriptParams[2];
            f32 y2 = *(f32*)&ScriptParams[3];
            if (x1 > x2)
            {
                f32 temp = x1;
                x1 = x2;
                x2 = temp;
            }
            if (y1 > y2)
            {
                f32 temp = y1;
                y1 = y2;
                y2 = temp;
            }
            area.left = x1;
            area.top = y2;
            area.right = x2;
            area.bottom = y1;
            if (ScriptParams[4])
                CTagManager::SetAlphaInArea(area, 255);
            else
                CTagManager::SetAlphaInArea(area, 0);
            return OR_CONTINUE;
        }

        case COMMAND_CAR_GOTO_COORDINATES_RACING:
        {
            CollectParameters(4);
            CVehicle* pVehicle = CPools::GetVehiclePool().GetAt(ScriptParams[0]);
            f32 targetX = *(f32*)&ScriptParams[1];
            f32 targetY = *(f32*)&ScriptParams[2];
            f32 targetZ = *(f32*)&ScriptParams[3];
            if (targetZ <= -100.0f)
            {
                targetZ = CWorld::FindGroundZForCoord(targetX, targetY);
            }
            targetZ += pVehicle->GetDistanceFromCentreOfMassToBaseOfModel();
            if (CCarCtrl::JoinCarWithRoadSystemGotoCoors(pVehicle, CVector(targetX, targetY, targetZ), false, false))
            {
                if (pVehicle->AutoPilot.Mission != MISSION_PLANE_CRASH_AND_BURN &&
                    pVehicle->AutoPilot.Mission != MISSION_HELI_CRASH_AND_BURN)
                {
                    pVehicle->AutoPilot.SetMission(MISSION_GOTOCOORDINATES_STRAIGHTLINE);
                }
            }
            else
            {
                if (pVehicle->AutoPilot.Mission != MISSION_PLANE_CRASH_AND_BURN &&
                    pVehicle->AutoPilot.Mission != MISSION_HELI_CRASH_AND_BURN)
                {
                    pVehicle->AutoPilot.SetMission(MISSION_GOTOCOORDINATES_RACING);
                }
            }
            pVehicle->SetStatus(STATUS_PHYSICS);
            pVehicle->SetEngineOn(true);
            pVehicle->AutoPilot.CruiseSpeed = MAX(pVehicle->AutoPilot.CruiseSpeed, 1);
            pVehicle->AutoPilot.LastTimeNotStuck = CTimer::GetTimeInMilliseconds();
            return OR_CONTINUE;
        }

        case COMMAND_START_PLAYBACK_RECORDED_CAR_USING_AI:
        {
            CollectParameters(2);
            CVehicle* pVehicle = CPools::GetVehiclePool().GetAt(ScriptParams[0]);
            CVehicleRecording::StartPlaybackRecordedCar(pVehicle, ScriptParams[1], true, false);
            return OR_CONTINUE;
        }

        case COMMAND_SKIP_IN_PLAYBACK_RECORDED_CAR:
        {
            CollectParameters(2);
            CVehicle* pVehicle = CPools::GetVehiclePool().GetAt(ScriptParams[0]);
            CVehicleRecording::SkipForwardInRecording(pVehicle, *(f32*)&ScriptParams[1]);
            return OR_CONTINUE;
        }

        case COMMAND_SKIP_CUTSCENE_START_INTERNAL:
        {
            CollectParameters(1);
            EndOfScriptedCutscenePC = ScriptParams[0];
            return OR_CONTINUE;
        }

        default:
            return OR_INTERRUPT;
    }
}

// MARK: 1800To1899

#ifndef MODEL_HYDRA
#define MODEL_HYDRA 520
#endif

// sa 0x46D050
OpcodeResult CRunningScript::ProcessCommands1800To1899(i32 CurrCommand)
{
    MARKFUNCTION(0x46D050);

    bool LatestCmpFlagResult;
    CPed* pPed;
    CPed* pSecondPed;
    CVehicle* pVehicle;
    CVehicle* pSecondVehicle;
    CEntity* pTargetEntity;
    CEntity* pEntityBeingHeld;
    CObject* pObj;
    CVector TempVec;
    CVector CentreVec;
    CVector VecDiff;
    CPlayerInfo* pPlayer;
    f32 fDirection;
    f32 Radius;
    f32 temp_float;
    i32 ActualIndex;
    f32 NewZ;
    f32 NewY;
    f32 NewX;
    i32 PedPoolIndex;
    i16 ZoneIndex;
    char ZoneLabel[10];
    i32 RandomPedIndex;
    f32 CentreY;
    f32 CentreX;
    f32 Rotation;
    f32 Height;
    f32 Width;
    f32 MinY;
    f32 MinX;
    f32 MaxY;
    f32 MaxX;
    i32 RandomCarIndex;
    i32 CarPoolIndex;
    f32 fHeading;
    i32 loop = 0;
    i32 test_total;
    i32 ModelIndex;
    i32 ArrayIndex;
    CTask* pTask;
    f32 fLowestFlightHeight;
    f32 fMinHeightAboveTerrain;
    f32 ClosestDistance;
    f32 Distance;
    u32 ItemID;
    char AnimName[24];
    char AnimGroupName[16];

    // NOTE: Keep switch cases in strictly ascending order as in commands.hpp to preserve MSVC jump table layout
    switch (CurrCommand)
    {
        case COMMAND_CLEAR_CHAR_DECISION_MAKER_EVENT_RESPONSE:
        {
            CollectParameters(2);
            ActualIndex = CTheScripts::GetActualScriptThingIndex(ScriptParams[0], UNIQUE_SCRIPT_DECISION_MAKER);
            if (ActualIndex >= 0 && ActualIndex < 20)
            {
                CDecisionMakerTypes::GetInstance()->FlushDecisionMakerEventResponse(ActualIndex, static_cast<eEventType>(ScriptParams[1]));
            }
            return OR_CONTINUE;
        }
        case COMMAND_ADD_CHAR_DECISION_MAKER_EVENT_RESPONSE:
        {
            CollectParameters(9);
            ActualIndex = CTheScripts::GetActualScriptThingIndex(ScriptParams[0], UNIQUE_SCRIPT_DECISION_MAKER);
            f32 probs[4];
            probs[3] = *(f32*)&ScriptParams[3];
            probs[2] = *(f32*)&ScriptParams[4];
            probs[1] = *(f32*)&ScriptParams[5];
            probs[0] = *(f32*)&ScriptParams[6];
            i32 bools[2];
            bools[1] = ScriptParams[7];
            bools[0] = ScriptParams[8];
            if (ActualIndex >= 0 && ActualIndex < 20)
            {
                CDecisionMakerTypes::GetInstance()->AddEventResponse(ActualIndex, static_cast<eEventType>(ScriptParams[1]), static_cast<eTaskType>(ScriptParams[2]), probs, bools);
            }
            return OR_CONTINUE;
        }
        case COMMAND_TASK_PICK_UP_OBJECT:
        {
            ScriptTaskPickUpObject(CurrCommand);
            return OR_CONTINUE;
        }
        case COMMAND_DROP_OBJECT:
        {
            CollectParameters(2);
            pPed = CPools::GetPedPool().GetAt(ScriptParams[0]);
            bool bAtSpeed = ScriptParams[1] != 0;
            if (pPed->GetPedIntelligence()->GetTaskHold(false))
            {
                pPed->GetPedIntelligence()->GetTaskHold(false)->DropEntity(pPed, bAtSpeed);
            }
            return OR_CONTINUE;
        }
        case COMMAND_EXPLODE_CAR_IN_CUTSCENE:
        {
            CollectParameters(1);
            pVehicle = CPools::GetVehiclePool().GetAt(ScriptParams[0]);
            pVehicle->vehicleFlags.bCanBeDamaged = false;
            pVehicle->BlowUpCar(nullptr, true);
            return OR_CONTINUE;
        }
        case COMMAND_BUILD_PLAYER_MODEL:
        {
            CollectParameters(1);
            CClothes::RebuildPlayer(CWorld::Players[ScriptParams[0]].m_pPed, false);
            CReplay::Init();
            return OR_CONTINUE;
        }
        case COMMAND_PLANE_ATTACK_PLAYER:
        {
            CollectParameters(3);
            pVehicle = CPools::GetVehiclePool().GetAt(ScriptParams[0]);
            fMinHeightAboveTerrain = *(f32*)&ScriptParams[2];
            if (pVehicle->AutoPilot.Mission != MISSION_PLANE_CRASH_AND_BURN && pVehicle->AutoPilot.Mission != MISSION_HELI_CRASH_AND_BURN)
            {
                pVehicle->AutoPilot.Mission = MISSION_PLANE_ATTACK_PLAYER;
            }
            ((CPlane*)pVehicle)->m_minAltitude = fMinHeightAboveTerrain;
            return OR_CONTINUE;
        }
        case COMMAND_PLANE_FLY_IN_DIRECTION:
        {
            CollectParameters(4);
            pVehicle = CPools::GetVehiclePool().GetAt(ScriptParams[0]);
            fDirection = *(f32*)&ScriptParams[1] * (PI / 180.0f);
            fMinHeightAboveTerrain = *(f32*)&ScriptParams[2];
            fLowestFlightHeight = *(f32*)&ScriptParams[3];
            if (pVehicle->AutoPilot.Mission != MISSION_PLANE_CRASH_AND_BURN && pVehicle->AutoPilot.Mission != MISSION_HELI_CRASH_AND_BURN)
            {
                pVehicle->AutoPilot.Mission = MISSION_PLANE_FLYINDIRECTION;
            }
            ((CPlane*)pVehicle)->m_planeHeading = fDirection;
            ((CPlane*)pVehicle)->m_maxAltitude = fLowestFlightHeight;
            ((CPlane*)pVehicle)->m_planeHeadingPrev = fDirection;
            ((CPlane*)pVehicle)->m_minAltitude = fMinHeightAboveTerrain;
            return OR_CONTINUE;
        }
        case COMMAND_PLANE_FOLLOW_ENTITY:
        {
            CollectParameters(4);
            pVehicle = CPools::GetVehiclePool().GetAt(ScriptParams[0]);
            pTargetEntity = nullptr;
            if (ScriptParams[1] >= 0)
            {
                pTargetEntity = CPools::GetPedPool().GetAt(ScriptParams[1]);
            }
            if (ScriptParams[2] >= 0)
            {
                pTargetEntity = CPools::GetVehiclePool().GetAt(ScriptParams[2]);
            }
            fMinHeightAboveTerrain = *(f32*)&ScriptParams[3];
            if (pVehicle->AutoPilot.Mission != MISSION_PLANE_CRASH_AND_BURN && pVehicle->AutoPilot.Mission != MISSION_HELI_CRASH_AND_BURN)
            {
                pVehicle->AutoPilot.Mission = MISSION_PLANE_FOLLOW_ENTITY;
            }
            if (pVehicle->AutoPilot.pTargetEntity)
            {
                pVehicle->AutoPilot.pTargetEntity->CleanUpOldReference((CEntity**)&pVehicle->AutoPilot.pTargetEntity);
            }
            pVehicle->AutoPilot.pTargetEntity = pTargetEntity;
            if (pTargetEntity)
            {
                pTargetEntity->RegisterReference((CEntity**)&pVehicle->AutoPilot.pTargetEntity);
            }
            pVehicle->SetEngineOn(true);
            ((CPlane*)pVehicle)->m_minAltitude = fMinHeightAboveTerrain;
            return OR_CONTINUE;
        }
        case COMMAND_TASK_DRIVE_BY:
        {
            CollectParameters(10);
            ActualIndex = ScriptParams[0];
            i32 targetPedHandle = ScriptParams[1];
            i32 targetCarHandle = ScriptParams[2];
            CVector targetPos = *(CVector*)&ScriptParams[3];
            f32 abortRange = *(f32*)&ScriptParams[6];
            i32 driveBySeat = ScriptParams[7];
            bool bSteerCar = ScriptParams[8] != 0;
            i32 frequencyPercentage = ScriptParams[9];

            pTargetEntity = nullptr;
            CPed* pTargetPed = nullptr;
            if (targetPedHandle >= 0)
            {
                pTargetPed = CPools::GetPedPool().GetAt(targetPedHandle);
                pTargetEntity = pTargetPed;
            }
            if (targetCarHandle >= 0)
            {
                pTargetEntity = CPools::GetVehiclePool().GetAt(targetCarHandle);
            }
            if (ActualIndex != -1)
            {
                pPed = CPools::GetPedPool().GetAt(ActualIndex);
                CTask* pTaskPrimary = pPed->GetPedIntelligence()->GetTaskManager().GetTask(TASK_PRIMARY_PRIMARY);
                if (pTaskPrimary && pTaskPrimary->GetTaskType() == CTaskTypes::TASK_SIMPLE_GANG_DRIVEBY && ((CTaskSimpleGangDriveBy*)pTaskPrimary)->m_pTargetEntity == pTargetPed)
                {
                    return OR_CONTINUE;
                }
            }
            CTaskSimpleGangDriveBy* pDriveByTask = new CTaskSimpleGangDriveBy(pTargetEntity, &targetPos, abortRange, (int8)frequencyPercentage, static_cast<eDrivebyStyle>(driveBySeat), bSteerCar);
            pDriveByTask->m_bFromScriptCommand = true;
            GivePedScriptedTask(ActualIndex, pDriveByTask, CurrCommand);
            return OR_CONTINUE;
        }
        case COMMAND_SET_CAR_STAY_IN_SLOW_LANE:
        {
            CollectParameters(2);
            pVehicle = CPools::GetVehiclePool().GetAt(ScriptParams[0]);
            pVehicle->AutoPilot.bAlwaysInSlowLane = ScriptParams[1] != 0;
            return OR_CONTINUE;
        }
        case COMMAND_TAKE_REMOTE_CONTROL_OF_CAR:
        {
            CollectParameters(2);
            pVehicle = CPools::GetVehiclePool().GetAt(ScriptParams[1]);
            CRemote::TakeRemoteControlOfCar(pVehicle);
            return OR_CONTINUE;
        }
        case COMMAND_IS_CLOSEST_OBJECT_OF_TYPE_SMASHED_OR_DAMAGED:
        {
            CollectParameters(7);
            NewZ = *(f32*)&ScriptParams[2];
            NewX = *(f32*)&ScriptParams[0];
            NewY = *(f32*)&ScriptParams[1];
            if (NewZ <= -100.0f)
            {
                NewZ = CWorld::FindGroundZForCoord(NewX, NewY);
            }
            ModelIndex = ScriptParams[4];
            if (ModelIndex < 0)
            {
                ModelIndex = CTheScripts::UsedObjectArray[-ModelIndex].Index;
            }
            pObj = (CObject*)CWorld::FindNearestObjectOfType(ModelIndex, CVector(NewX, NewY, NewZ), *(f32*)&ScriptParams[3], false, false, false, false, true, true);
            LatestCmpFlagResult = false;
            if (pObj && pObj->GetIsTypeObject())
            {
                if (pObj->objectFlags.bIsBroken)
                {
                    LatestCmpFlagResult = ScriptParams[5] != 0;
                }
                if (pObj->IsObjectDamaged())
                {
                    LatestCmpFlagResult = ScriptParams[6] != 0;
                }
            }
            UpdateCompareFlag(LatestCmpFlagResult);
            return OR_CONTINUE;
        }
        case COMMAND_START_SETTING_UP_CONVERSATION:
        {
            CollectParameters(1);
            pPed = CPools::GetPedPool().GetAt(ScriptParams[0]);
            CConversations::StartSettingUpConversation(pPed);
            return OR_CONTINUE;
        }
        case COMMAND_FINISH_SETTING_UP_CONVERSATION:
        {
            CConversations::DoneSettingUpConversation(false);
            return OR_CONTINUE;
        }
        case COMMAND_IS_CONVERSATION_AT_NODE:
        {
            CollectParameters(1);
            pPed = CPools::GetPedPool().GetAt(ScriptParams[0]);
            ReadTextLabelFromScript(ZoneLabel, 8);
            LatestCmpFlagResult = CConversations::IsConversationAtNode(ZoneLabel, pPed);
            UpdateCompareFlag(LatestCmpFlagResult);
            return OR_CONTINUE;
        }
        case COMMAND_GET_OBJECT_HEALTH:
        {
            CollectParameters(1);
            pObj = CPools::GetObjectPool().GetAt(ScriptParams[0]);
            *(f32*)&ScriptParams[0] = pObj->m_fHealth;
            StoreParameters(1);
            return OR_CONTINUE;
        }
        case COMMAND_SET_OBJECT_HEALTH:
        {
            CollectParameters(2);
            pObj = CPools::GetObjectPool().GetAt(ScriptParams[0]);
            pObj->m_fHealth = *(f32*)&ScriptParams[1];
            return OR_CONTINUE;
        }
        case COMMAND_BREAK_OBJECT:
        {
            CollectParameters(2);
            pObj = CPools::GetObjectPool().GetAt(ScriptParams[0]);
            if (CGlass::IsObjectGlass(pObj))
            {
                CGlass::WindowRespondsToCollision(pObj, 99999.9f, CVector(0.0f, 0.0f, 0.0f), pObj->GetPosition(), false);
                pObj->objectFlags.bIsBroken = true;
                return OR_CONTINUE;
            }
            else
            {
                g_breakMan.Add(pObj, &pObj->m_pObjectInfo->m_vecBreakVelocity, pObj->m_pObjectInfo->m_fBreakVelocityRand, ScriptParams[1] != 0);
                pObj->m_bUsesCollision = false;
                pObj->m_bIsVisible = false;
                if (!pObj->GetIsStatic())
                {
                    pObj->RemoveFromMovingList();
                }
                pObj->objectFlags.bIsBroken = true;
                pObj->m_bDrawLast = true;
                pObj->physicalFlags.bRenderScorched = true;
                pObj->m_vecMoveSpeed = CVector(0.0f, 0.0f, 0.0f);
                pObj->m_vecTurnSpeed = CVector(0.0f, 0.0f, 0.0f);
                pObj->DeleteRwObject();
                return OR_CONTINUE;
            }
        }
        case COMMAND_HELI_ATTACK_PLAYER:
        {
            CollectParameters(3);
            pVehicle = CPools::GetVehiclePool().GetAt(ScriptParams[0]);
            fMinHeightAboveTerrain = *(f32*)&ScriptParams[2];
            if (pVehicle->AutoPilot.Mission != MISSION_PLANE_CRASH_AND_BURN && pVehicle->AutoPilot.Mission != MISSION_HELI_CRASH_AND_BURN)
            {
                pVehicle->AutoPilot.Mission = MISSION_HELI_ATTACK_PLAYER;
            }
            ((CHeli*)pVehicle)->m_MinHeightAboveTerrain = fMinHeightAboveTerrain;
            pVehicle->AutoPilot.CruiseSpeed = 100;
            return OR_CONTINUE;
        }
        case COMMAND_HELI_FOLLOW_ENTITY:
        {
            CollectParameters(4);
            pVehicle = CPools::GetVehiclePool().GetAt(ScriptParams[0]);
            pTargetEntity = nullptr;
            if (ScriptParams[1] >= 0)
            {
                pTargetEntity = CPools::GetPedPool().GetAt(ScriptParams[1]);
            }
            if (ScriptParams[2] >= 0)
            {
                pTargetEntity = CPools::GetVehiclePool().GetAt(ScriptParams[2]);
            }
            fMinHeightAboveTerrain = *(f32*)&ScriptParams[3];
            if (pVehicle->AutoPilot.Mission != MISSION_PLANE_CRASH_AND_BURN && pVehicle->AutoPilot.Mission != MISSION_HELI_CRASH_AND_BURN)
            {
                pVehicle->AutoPilot.Mission = MISSION_HELI_FOLLOW_ENTITY;
            }
            if (pVehicle->AutoPilot.pTargetEntity)
            {
                pVehicle->AutoPilot.pTargetEntity->CleanUpOldReference((CEntity**)&pVehicle->AutoPilot.pTargetEntity);
            }
            pVehicle->AutoPilot.pTargetEntity = pTargetEntity;
            if (pTargetEntity)
            {
                pTargetEntity->RegisterReference((CEntity**)&pVehicle->AutoPilot.pTargetEntity);
            }
            ((CHeli*)pVehicle)->m_MinHeightAboveTerrain = fMinHeightAboveTerrain;
            pVehicle->AutoPilot.CruiseSpeed = 100;
            return OR_CONTINUE;
        }
        case COMMAND_POLICE_HELI_CHASE_ENTITY:
        {
            CollectParameters(4);
            pVehicle = CPools::GetVehiclePool().GetAt(ScriptParams[0]);
            pTargetEntity = nullptr;
            if (ScriptParams[1] >= 0)
            {
                pTargetEntity = CPools::GetPedPool().GetAt(ScriptParams[1]);
            }
            if (ScriptParams[2] >= 0)
            {
                pTargetEntity = CPools::GetVehiclePool().GetAt(ScriptParams[2]);
            }
            fMinHeightAboveTerrain = *(f32*)&ScriptParams[3];
            if (pVehicle->AutoPilot.Mission != MISSION_PLANE_CRASH_AND_BURN && pVehicle->AutoPilot.Mission != MISSION_HELI_CRASH_AND_BURN)
            {
                pVehicle->AutoPilot.Mission = MISSION_HELI_POLICE_BEHAVIOUR;
            }
            if (pVehicle->AutoPilot.pTargetEntity)
            {
                pVehicle->AutoPilot.pTargetEntity->CleanUpOldReference((CEntity**)&pVehicle->AutoPilot.pTargetEntity);
            }
            pVehicle->AutoPilot.pTargetEntity = pTargetEntity;
            if (pTargetEntity)
            {
                pTargetEntity->RegisterReference((CEntity**)&pVehicle->AutoPilot.pTargetEntity);
            }
            ((CHeli*)pVehicle)->m_MinHeightAboveTerrain = fMinHeightAboveTerrain;
            pVehicle->AutoPilot.CruiseSpeed = 100;
            return OR_CONTINUE;
        }
        case COMMAND_TASK_USE_MOBILE_PHONE:
        {
            CollectParameters(2);
            ActualIndex = ScriptParams[0];
            if (ScriptParams[1] <= 0)
            {
                pPed = CPools::GetPedPool().GetAt(ScriptParams[0]);
                CTask* pTaskPhone = pPed->GetPedIntelligence()->GetTaskManager().FindTaskByType(TASK_PRIMARY_PRIMARY, CTaskTypes::TASK_COMPLEX_USE_MOBILE_PHONE);
                if (pTaskPhone && pTaskPhone->GetTaskType() == CTaskTypes::TASK_COMPLEX_USE_MOBILE_PHONE)
                {
                    ((CTaskComplexUseMobilePhone*)pTaskPhone)->Stop(pPed);
                }
                return OR_CONTINUE;
            }
            else
            {
                CTaskComplexUseMobilePhone* pTaskPhone = CTaskComplexUseMobilePhone::Create(-1);
                GivePedScriptedTask(ActualIndex, pTaskPhone, CurrCommand);
                return OR_CONTINUE;
            }
        }
        case COMMAND_TASK_WARP_CHAR_INTO_CAR_AS_DRIVER:
        {
            CollectParameters(2);
            ActualIndex = ScriptParams[0];
            pVehicle = CPools::GetVehiclePool().GetAt(ScriptParams[1]);
            CTaskSimpleCarSetPedInAsDriver* pTaskDriver = new CTaskSimpleCarSetPedInAsDriver(pVehicle, true, nullptr);
            GivePedScriptedTask(ActualIndex, pTaskDriver, CurrCommand);
            return OR_CONTINUE;
        }
        case COMMAND_TASK_WARP_CHAR_INTO_CAR_AS_PASSENGER:
        {
            CollectParameters(3);
            ActualIndex = ScriptParams[0];
            pVehicle = CPools::GetVehiclePool().GetAt(ScriptParams[1]);
            i32 targetDoor = 0;
            if (ScriptParams[2] >= 0)
            {
                targetDoor = CCarEnterExit::ComputeTargetDoorToEnterAsPassenger(*pVehicle, ScriptParams[2]);
            }
            CTaskSimpleCarSetPedInAsPassenger* pTaskPassenger = new CTaskSimpleCarSetPedInAsPassenger(pVehicle, targetDoor, true, nullptr);
            GivePedScriptedTask(ActualIndex, pTaskPassenger, CurrCommand);
            return OR_CONTINUE;
        }
        case COMMAND_SWITCH_COPS_ON_BIKES:
        {
            CollectParameters(1);
            CStreaming::DisableCopBikes(ScriptParams[0] == 0);
            return OR_CONTINUE;
        }
        case COMMAND_IS_FLAME_IN_ANGLED_AREA_2D:
        case COMMAND_IS_FLAME_IN_ANGLED_AREA_3D:
        {
            FlameInAngledAreaCheckCommand(CurrCommand);
            return OR_CONTINUE;
        }
        case COMMAND_ADD_STUCK_CAR_CHECK_WITH_WARP:
        {
            CollectParameters(7);
            CTheScripts::StuckCars.AddCarToCheck(ScriptParams[0], *(f32*)&ScriptParams[1], ScriptParams[2], *(f32*)&ScriptParams[3], true, ScriptParams[4] != 0, ScriptParams[5] != 0, ScriptParams[6]);
            return OR_CONTINUE;
        }
        case COMMAND_DAMAGE_CAR_PANEL:
        {
            CollectParameters(2);
            pVehicle = CPools::GetVehiclePool().GetAt(ScriptParams[0]);
            ((CAutomobile*)pVehicle)->Damage.ApplyDamage((CAutomobile*)pVehicle, (tComponent)(ScriptParams[1] + 11), 150.0f, 1.0f);
            return OR_CONTINUE;
        }
        case COMMAND_SET_CAR_ROLL:
        {
            CollectParameters(2);
            pVehicle = CPools::GetVehiclePool().GetAt(ScriptParams[0]);
            CVector pos = pVehicle->GetPosition();
            fHeading = pVehicle->GetHeading();
            pVehicle->m_pMat->SetRotateZ(fHeading);
            pVehicle->m_pMat->RotateY(*(f32*)&ScriptParams[1] * (PI / 180.0f));
            pVehicle->m_pMat->Translate(pos.x, pos.y, pos.z);
            return OR_CONTINUE;
        }
        case COMMAND_SUPPRESS_CAR_MODEL:
        {
            CollectParameters(1);
            CTheScripts::AddToSuppressedCarModelArray(ScriptParams[0]);
            return OR_CONTINUE;
        }
        case COMMAND_DONT_SUPPRESS_CAR_MODEL:
        {
            CollectParameters(1);
            CTheScripts::RemoveFromSuppressedCarModelArray(ScriptParams[0]);
            return OR_CONTINUE;
        }
        case COMMAND_DONT_SUPPRESS_ANY_CAR_MODELS:
        {
            CTheScripts::ClearAllSuppressedCarModels();
            return OR_CONTINUE;
        }
        case COMMAND_IS_PS2_KEYBOARD_KEY_PRESSED:
        {
            CollectParameters(1);
            LatestCmpFlagResult = false;
            UpdateCompareFlag(LatestCmpFlagResult);
            return OR_CONTINUE;
        }
        case COMMAND_IS_PS2_KEYBOARD_KEY_JUST_PRESSED:
        {
            CollectParameters(1);
            LatestCmpFlagResult = false;
            UpdateCompareFlag(LatestCmpFlagResult);
            return OR_CONTINUE;
        }
        case COMMAND_IS_CHAR_HOLDING_OBJECT:
        {
            CollectParameters(2);
            pPed = ScriptParams[0] < 0 ? nullptr : CPools::GetPedPool().GetAt(ScriptParams[0]);
            pObj = ScriptParams[1] < 0 ? nullptr : CPools::GetObjectPool().GetAt(ScriptParams[1]);
            pEntityBeingHeld = nullptr;
            LatestCmpFlagResult = false;
            if (pPed)
            {
                CTaskSimpleHoldEntity* pTaskHold = pPed->GetPedIntelligence()->GetTaskHold(false);
                if (pTaskHold && pTaskHold->GetHeldEntity())
                {
                    pEntityBeingHeld = pTaskHold->GetHeldEntity();
                }
                if (pObj)
                {
                    if (pObj == pEntityBeingHeld)
                    {
                        LatestCmpFlagResult = true;
                    }
                }
                else
                {
                    if (pEntityBeingHeld != nullptr)
                    {
                        LatestCmpFlagResult = true;
                    }
                }
            }
            else
            {
                for (PedPoolIndex = CPools::GetPedPool().GetSize(); PedPoolIndex > 0;)
                {
                    --PedPoolIndex;
                    if (LatestCmpFlagResult)
                    {
                        break;
                    }
                    pPed = CPools::GetPedPool().GetSlot(PedPoolIndex);
                    if (pPed)
                    {
                        CEntity* pFound = nullptr;
                        CTaskSimpleHoldEntity* pTaskHold = pPed->GetPedIntelligence()->GetTaskHold(false);
                        if (pTaskHold && pTaskHold->GetHeldEntity())
                        {
                            pFound = pTaskHold->GetHeldEntity();
                        }
                        if (!pObj)
                        {
                            if (pFound)
                            {
                                LatestCmpFlagResult = true;
                            }
                        }
                        else if (pObj == pFound)
                        {
                            LatestCmpFlagResult = true;
                        }
                    }
                }
            }
            UpdateCompareFlag(LatestCmpFlagResult);
            return OR_CONTINUE;
        }
        case COMMAND_SET_CAR_CAN_GO_AGAINST_TRAFFIC:
        {
            CollectParameters(2);
            pVehicle = CPools::GetVehiclePool().GetAt(ScriptParams[0]);
            pVehicle->AutoPilot.bDontGoAgainstTraffic = ScriptParams[1] == 0;
            return OR_CONTINUE;
        }
        case COMMAND_DAMAGE_CAR_DOOR:
        {
            CollectParameters(2);
            pVehicle = CPools::GetVehiclePool().GetAt(ScriptParams[0]);
            ((CAutomobile*)pVehicle)->Damage.ApplyDamage((CAutomobile*)pVehicle, (tComponent)(ScriptParams[1] + 5), 150.0f, 1.0f);
            return OR_CONTINUE;
        }
        case COMMAND_GET_RANDOM_CAR_IN_SPHERE_NO_SAVE:
        {
            CollectParameters(5);
            TempVec = *(CVector*)&ScriptParams[0];
            Radius = *(f32*)&ScriptParams[3];
            ModelIndex = ScriptParams[4];
            ClosestDistance = 9999.9f;
            ActualIndex = -1;
            CarPoolIndex = CPools::GetVehiclePool().GetSize();
            while (CarPoolIndex > 0)
            {
                --CarPoolIndex;
                pVehicle = CPools::GetVehiclePool().GetSlot(CarPoolIndex);
                if (pVehicle)
                {
                    if ((pVehicle->GetVehicleAppearance() == APR_CAR || pVehicle->GetVehicleAppearance() == APR_BIKE) && !pVehicle->vehicleFlags.bIsLawEnforcer && (pVehicle->m_nModelIndex == ModelIndex || ModelIndex < 0))
                    {
                        if (pVehicle->CanBeDeleted())
                        {
                            Distance = (pVehicle->GetPosition() - TempVec).Magnitude();
                            if (Distance < Radius && Distance < ClosestDistance)
                            {
                                ActualIndex = CPools::GetVehiclePool().GetIndex(pVehicle);
                                ClosestDistance = Distance;
                            }
                        }
                    }
                }
            }
            ScriptParams[0] = ActualIndex;
            StoreParameters(1);
            return OR_CONTINUE;
        }
        case COMMAND_GET_RANDOM_CHAR_IN_SPHERE:
        {
            CollectParameters(7);
            TempVec = *(CVector*)&ScriptParams[0];
            Radius = *(f32*)&ScriptParams[3];
            ClosestDistance = 9999.9f;
            ActualIndex = -1;
            PedPoolIndex = CPools::GetPedPool().GetSize();
            while (PedPoolIndex > 0)
            {
                --PedPoolIndex;
                pPed = CPools::GetPedPool().GetSlot(PedPoolIndex);
                if (pPed)
                {
                    if (pPed->CharCreatedBy == RANDOM_CHAR && !pPed->bFadeOut)
                    {
                        if (!IsPedDead(pPed) && ThisIsAValidRandomPed(pPed->m_nPedType, ScriptParams[4], ScriptParams[5], ScriptParams[6]) && !CPedGroups::GetPedsGroup(pPed))
                        {
                            Distance = (pPed->GetPosition() - TempVec).Magnitude();
                            if (Distance < Radius && Distance < ClosestDistance)
                            {
                                ActualIndex = CPools::GetPedPool().GetIndex(pPed);
                                ClosestDistance = Distance;
                            }
                        }
                    }
                }
            }
            if (ActualIndex >= 0)
            {
                pPed = CPools::GetPedPool().GetAt(ActualIndex);
                pPed->SetCharCreatedBy(MISSION_CHAR);
                ++CPopulation::ms_nTotalMissionPeds;
                if (IsThisAMissionScript)
                {
                    CTheScripts::MissionCleanUp.AddEntityToList(ActualIndex, CLEANUP_CHAR);
                }
            }
            ScriptParams[0] = ActualIndex;
            StoreParameters(1);
            return OR_CONTINUE;
        }
        case COMMAND_HAS_CHAR_BEEN_ARRESTED:
        {
            CollectParameters(1);
            pPed = CPools::GetPedPool().GetAt(ScriptParams[0]);
            LatestCmpFlagResult = (pPed->m_nPedState == PED_ARRESTED);
            UpdateCompareFlag(LatestCmpFlagResult);
            return OR_CONTINUE;
        }
        case COMMAND_SET_PLANE_THROTTLE:
        {
            CollectParameters(2);
            pVehicle = CPools::GetVehiclePool().GetAt(ScriptParams[0]);
            ((CPlane*)pVehicle)->m_fScriptThrottleControl = *(f32*)&ScriptParams[1];
            return OR_CONTINUE;
        }
        case COMMAND_HELI_LAND_AT_COORDS:
        {
            CollectParameters(6);
            pVehicle = CPools::GetVehiclePool().GetAt(ScriptParams[0]);
            ((CAutomobile*)pVehicle)->TellHeliToGoToCoors(*(f32*)&ScriptParams[1], *(f32*)&ScriptParams[2], *(f32*)&ScriptParams[3], *(f32*)&ScriptParams[4], *(f32*)&ScriptParams[5]);
            if (pVehicle->AutoPilot.Mission != MISSION_PLANE_CRASH_AND_BURN && pVehicle->AutoPilot.Mission != MISSION_HELI_CRASH_AND_BURN)
            {
                pVehicle->AutoPilot.Mission = MISSION_HELI_LAND;
            }
            return OR_CONTINUE;
        }
        case COMMAND_PLANE_STARTS_IN_AIR:
        {
            CollectParameters(1);
            pVehicle = CPools::GetVehiclePool().GetAt(ScriptParams[0]);
            ((CPlane*)pVehicle)->IsAlreadyFlying();
            if (pVehicle->m_nModelIndex == MODEL_HYDRA)
            {
                ((CPlane*)pVehicle)->SetGearUp();
            }
            return OR_CONTINUE;
        }
        case COMMAND_SET_RELATIONSHIP:
        {
            CollectParameters(3);
            CPedType::SetPedTypeAsAcquaintance(ScriptParams[0], static_cast<ePedType>(ScriptParams[1]), CPedType::GetPedFlag(ScriptParams[2]));
            return OR_CONTINUE;
        }
        case COMMAND_CLEAR_RELATIONSHIP:
        {
            CollectParameters(3);
            CPedType::ClearPedTypeAsAcquaintance(ScriptParams[0], static_cast<ePedType>(ScriptParams[1]), CPedType::GetPedFlag(ScriptParams[2]));
            return OR_CONTINUE;
        }
        case COMMAND_CLEAR_GROUP_DECISION_MAKER_EVENT_RESPONSE:
        {
            CollectParameters(2);
            const i32 iDecisionMakerIndex = ScriptParams[0];
            const i32 iEventType = ScriptParams[1];
            ActualIndex = CTheScripts::GetActualScriptThingIndex(iDecisionMakerIndex, UNIQUE_SCRIPT_DECISION_MAKER);
            if (ActualIndex >= 0 && ActualIndex < 20)
            {
                CDecisionMakerTypes::GetInstance()->FlushDecisionMakerEventResponse(ActualIndex, static_cast<eEventType>(iEventType));
            }
            return OR_CONTINUE;
        }
        case COMMAND_ADD_GROUP_DECISION_MAKER_EVENT_RESPONSE:
        {
            CollectParameters(9);
            const i32 iDecisionMakerIndex = ScriptParams[0];
            const i32 iEventType = ScriptParams[1];
            const i32 iTaskType = ScriptParams[2];
            f32 probs[4];
            probs[3] = *(f32*)&ScriptParams[3];
            probs[2] = *(f32*)&ScriptParams[4];
            probs[1] = *(f32*)&ScriptParams[5];
            probs[0] = *(f32*)&ScriptParams[6];
            i32 bools[2];
            bools[1] = ScriptParams[7];
            bools[0] = ScriptParams[8];
            ActualIndex = CTheScripts::GetActualScriptThingIndex(iDecisionMakerIndex, UNIQUE_SCRIPT_DECISION_MAKER);
            if (ActualIndex >= 0 && ActualIndex < 20)
            {
                CDecisionMakerTypes::GetInstance()->AddEventResponse(ActualIndex, static_cast<eEventType>(iEventType), static_cast<eTaskType>(iTaskType), probs, bools);
            }
            return OR_CONTINUE;
        }
        case COMMAND_DRAW_SPRITE_WITH_ROTATION:
        {
            CollectParameters(10);
            CentreX = *(f32*)&ScriptParams[1];
            CentreY = *(f32*)&ScriptParams[2];
            Width = *(f32*)&ScriptParams[3] * 0.5f;
            Height = *(f32*)&ScriptParams[4] * 0.5f;
            f32 screenXMult = RsGlobal.maximumWidth * (1.0f / 640.0f);
            f32 screenYMult = RsGlobal.maximumHeight * (1.0f / 448.0f);
            ActualIndex = CTheScripts::NumberOfIntroRectanglesThisFrame;
            CTheScripts::IntroRectangles[ActualIndex].eWindowType = WINDOW_SPRITE_WITH_ROTATION;
            CTheScripts::IntroRectangles[ActualIndex].ScriptSpriteIndex = ScriptParams[0] - 1;
            CTheScripts::IntroRectangles[ActualIndex].ScriptRectMinX = (CentreX - Width) * screenXMult;
            CTheScripts::IntroRectangles[ActualIndex].ScriptRectMinY = (CentreY - Height) * screenYMult;
            CTheScripts::IntroRectangles[ActualIndex].ScriptRectMaxX = (CentreX + Width) * screenXMult;
            CTheScripts::IntroRectangles[ActualIndex].ScriptRectMaxY = (CentreY + Height) * screenYMult;
            CTheScripts::IntroRectangles[ActualIndex].ScriptRectRotation = *(f32*)&ScriptParams[5] * (PI / 180.0f);
            CTheScripts::IntroRectangles[ActualIndex].ScriptRectColour = CRGBA(ScriptParams[6], ScriptParams[7], ScriptParams[8], ScriptParams[9]);
            CTheScripts::IntroRectangles[ActualIndex].pTitle[0] = '\0';
            ++CTheScripts::NumberOfIntroRectanglesThisFrame;
            return OR_CONTINUE;
        }
        case COMMAND_TASK_USE_ATTRACTOR:
        {
            CollectParameters(2);
            ActualIndex = ScriptParams[0];
            i32 effectIndex = CTheScripts::GetActualScriptThingIndex(ScriptParams[1], UNIQUE_SCRIPT_ATTRACTOR);
            if (effectIndex >= 64)
            {
                return OR_CONTINUE;
            }
            CTaskComplexUseEffect* pTaskEffect = new CTaskComplexUseEffect(reinterpret_cast<C2dEffectPedAttractor*>(&CScripted2dEffects::ms_effects[effectIndex]), nullptr);
            GivePedScriptedTask(ActualIndex, pTaskEffect, CurrCommand);
            return OR_CONTINUE;
        }
        case COMMAND_TASK_SHOOT_AT_CHAR:
        {
            CollectParameters(3);
            ActualIndex = ScriptParams[0];
            pPed = CPools::GetPedPool().GetAt(ScriptParams[1]);
            CTaskSimpleGunControl* pTaskShoot = new CTaskSimpleGunControl(pPed, nullptr, nullptr, 3, 1, ScriptParams[2]);
            GivePedScriptedTask(ActualIndex, pTaskShoot, CurrCommand);
            return OR_CONTINUE;
        }
        case COMMAND_SET_INFORM_RESPECTED_FRIENDS:
        {
            CollectParameters(3);
            pPed = CPools::GetPedPool().GetAt(ScriptParams[0]);
            *(f32*)((u8*)pPed->GetPedIntelligence() + 372) = *(f32*)&ScriptParams[1];
            *(i32*)((u8*)pPed->GetPedIntelligence() + 368) = ScriptParams[2];
            return OR_CONTINUE;
        }
        case COMMAND_IS_CHAR_RESPONDING_TO_EVENT:
        {
            CollectParameters(2);
            pPed = CPools::GetPedPool().GetAt(ScriptParams[0]);
            LatestCmpFlagResult = pPed->GetPedIntelligence()->IsRespondingToEvent(static_cast<eEventType>(ScriptParams[1]));
            UpdateCompareFlag(LatestCmpFlagResult);
            return OR_CONTINUE;
        }
        case COMMAND_SET_OBJECT_VISIBLE:
        {
            CollectParameters(2);
            pObj = CPools::GetObjectPool().GetAt(ScriptParams[0]);
            pObj->m_bIsVisible = ScriptParams[1] != 0;
            return OR_CONTINUE;
        }
        case COMMAND_TASK_FLEE_CHAR_ANY_MEANS:
        {
            CollectParameters(8);
            ActualIndex = ScriptParams[0];
            pPed = CPools::GetPedPool().GetAt(ScriptParams[1]);
            CTaskComplexFleeAnyMeans* pTaskFlee = new CTaskComplexFleeAnyMeans(pPed, ScriptParams[4] != 0, *(f32*)&ScriptParams[2], ScriptParams[3], ScriptParams[5], *(f32*)&ScriptParams[6], *(f32*)&ScriptParams[7], 1000, 1.0f);
            GivePedScriptedTask(ActualIndex, pTaskFlee, CurrCommand);
            return OR_CONTINUE;
        }
        case COMMAND_STOP_RECORDING_CAR:
        {
            CollectParameters(1);
            return OR_CONTINUE;
        }
        case COMMAND_FLUSH_PATROL_ROUTE:
        {
            CTaskComplexFollowPatrolRoute::ms_patrolRoute.Clear();
            return OR_CONTINUE;
        }
        case COMMAND_EXTEND_PATROL_ROUTE:
        {
            CollectParameters(3);
            TempVec = *(CVector*)&ScriptParams[0];
            ReadTextLabelFromScript(AnimName, 24);
            ReadTextLabelFromScript(AnimGroupName, 16);
            if (!strcmp(AnimName, "NONE"))
            {
                CAnimNameDescriptor animDesc;
                CTaskComplexFollowPatrolRoute::ms_patrolRoute.Add(TempVec, animDesc);
            }
            else
            {
                CAnimNameDescriptor animDesc(AnimName, AnimGroupName);
                CTaskComplexFollowPatrolRoute::ms_patrolRoute.Add(TempVec, animDesc);
            }
            return OR_CONTINUE;
        }
        case COMMAND_PLAY_OBJECT_ANIM:
        {
            CollectParameters(1);
            pObj = CPools::GetObjectPool().GetAt(ScriptParams[0]);
            ReadTextLabelFromScript(AnimName, 24);
            ReadTextLabelFromScript(AnimGroupName, 16);
            CollectParameters(3);
            f32 fBlendDelta = *(f32*)&ScriptParams[0];
            CAnimBlock* pAnimBlock = CAnimManager::GetAnimationBlock(AnimGroupName);
            CAnimBlendHierarchy* pHierarchy = CAnimManager::GetAnimation(AnimName, pAnimBlock);
            i32 flags = 0;
            if (ScriptParams[1])
            {
                flags = 2;
            }
            if (!ScriptParams[2])
            {
                flags |= 8;
            }
            if (pObj->GetRwObject())
            {
                RpClump* pClump = reinterpret_cast<RpClump*>(pObj->GetRwObject());
                if (!RpAnimBlendClumpIsInitialized(pClump))
                {
                    RpAnimBlendClumpInit(pClump);
                }
                CAnimManager::BlendAnimation(pClump, pHierarchy, flags, fBlendDelta);
                UpdateCompareFlag(true);
            }
            else
            {
                UpdateCompareFlag(false);
            }
            return OR_CONTINUE;
        }
        case COMMAND_SET_RADAR_ZOOM:
        {
            CollectParameters(1);
            CTheScripts::RadarZoomValue = ScriptParams[0];
            return OR_CONTINUE;
        }
        case COMMAND_DOES_BLIP_EXIST:
        {
            CollectParameters(1);
            LatestCmpFlagResult = CRadar::GetActualBlipArrayIndex(ScriptParams[0]) != -1;
            UpdateCompareFlag(LatestCmpFlagResult);
            return OR_CONTINUE;
        }
        case COMMAND_LOAD_PRICES:
        {
            ReadTextLabelFromScript(ZoneLabel, 16);
            CShopping::LoadPrices(ZoneLabel);
            return OR_CONTINUE;
        }
        case COMMAND_LOAD_SHOP:
        {
            ReadTextLabelFromScript(ZoneLabel, 16);
            CShopping::LoadShop(ZoneLabel);
            return OR_CONTINUE;
        }
        case COMMAND_GET_NUMBER_OF_ITEMS_IN_SHOP:
        {
            ScriptParams[0] = CShopping::GetNumItemsInShop();
            StoreParameters(1);
            return OR_CONTINUE;
        }
        case COMMAND_GET_ITEM_IN_SHOP:
        {
            CollectParameters(1);
            ScriptParams[0] = CShopping::GetItemInShop(ScriptParams[0]);
            StoreParameters(1);
            return OR_CONTINUE;
        }
        case COMMAND_GET_PRICE_OF_ITEM:
        {
            CollectParameters(1);
            ScriptParams[0] = CShopping::GetPrice(ScriptParams[0]);
            StoreParameters(1);
            return OR_CONTINUE;
        }
        case COMMAND_TASK_DEAD:
        {
            CollectParameters(1);
            CTaskComplexDie* pTaskDie = new CTaskComplexDie(WEAPONTYPE_UNARMED, ANIM_STD_PED, ANIM_STD_KO_FRONT, 4.0f, 0.0f, false, false, false, false);
            GivePedScriptedTask(ScriptParams[0], pTaskDie, CurrCommand);
            return OR_CONTINUE;
        }
        case COMMAND_SET_CAR_AS_MISSION_CAR:
        {
            CollectParameters(1);
            pVehicle = CPools::GetVehiclePool().GetAt(ScriptParams[0]);
            ActualIndex = CPools::GetVehiclePool().GetIndex(pVehicle);
            if (IsThisAMissionScript && (pVehicle->m_nCreatedBy == RANDOM_VEHICLE || pVehicle->m_nCreatedBy == PARKED_VEHICLE))
            {
                pVehicle->SetVehicleCreatedBy(MISSION_VEHICLE);
                CTheScripts::MissionCleanUp.AddEntityToList(ActualIndex, CLEANUP_CAR);
            }
            return OR_CONTINUE;
        }
        case COMMAND_SET_ZONE_POPULATION_TYPE:
        {
            ReadTextLabelFromScript(ZoneLabel, 8);
            CollectParameters(1);
            ZoneIndex = CTheZones::FindZoneByLabelAndReturnIndex(ZoneLabel, ZONE_INFORMATION);
            if (ZoneIndex >= 0)
            {
                CTheZones::ZoneInfoArray[CTheZones::NavigationZoneArray[ZoneIndex].ZoneInfoIndex].PopType = ScriptParams[0];
            }
            return OR_CONTINUE;
        }
        case COMMAND_SET_ZONE_DEALER_STRENGTH:
        {
            ReadTextLabelFromScript(ZoneLabel, 8);
            CollectParameters(1);
            ZoneIndex = CTheZones::FindZoneByLabelAndReturnIndex(ZoneLabel, ZONE_INFORMATION);
            if (ZoneIndex >= 0)
            {
                CTheZones::ZoneInfoArray[CTheZones::NavigationZoneArray[ZoneIndex].ZoneInfoIndex].DealerStrength = ScriptParams[0];
            }
            return OR_CONTINUE;
        }
        case COMMAND_GET_ZONE_DEALER_STRENGTH:
        {
            ReadTextLabelFromScript(ZoneLabel, 8);
            ZoneIndex = CTheZones::FindZoneByLabelAndReturnIndex(ZoneLabel, ZONE_INFORMATION);
            ScriptParams[0] = CTheZones::ZoneInfoArray[CTheZones::NavigationZoneArray[ZoneIndex].ZoneInfoIndex].DealerStrength;
            StoreParameters(1);
            return OR_CONTINUE;
        }
        case COMMAND_ALLOCATE_SCRIPT_TO_PED_GENERATOR:
        {
            return OR_CONTINUE;
        }
        case COMMAND_ALLOCATE_SCRIPT_TO_RANDOM_PED:
        {
            return OR_CONTINUE;
        }
        case COMMAND_SET_UP_CONVERSATION_NODE:
        {
            return OR_CONTINUE;
        }
        case COMMAND_CLEAR_ALL_CONVERSATIONS:
        {
            return OR_CONTINUE;
        }
        case COMMAND_GET_CHAR_LIGHTING:
        {
            return OR_CONTINUE;
        }
        case COMMAND_SET_CLOSEST_OBJECT_OF_TYPE_RENDER_SCORCHED:
        {
            return OR_CONTINUE;
        }
        case COMMAND_GET_VEHICLE_WHEEL_UPGRADE_CLASS:
        {
            return OR_CONTINUE;
        }
        case COMMAND_GET_NUM_WHEELS_IN_UPGRADE_CLASS:
        {
            return OR_CONTINUE;
        }
        case COMMAND_GET_WHEEL_IN_UPGRADE_CLASS:
        {
            return OR_CONTINUE;
        }
        case COMMAND_HELI_FLY_IN_DIRECTION:
        {
            return OR_CONTINUE;
        }
        case COMMAND_SET_UP_CONVERSATION_END_NODE:
        {
            return OR_CONTINUE;
        }
        case COMMAND_SET_ZONE_RADAR_COLOURS:
        {
            return OR_CONTINUE;
        }
        case COMMAND_GIVE_LOWRIDER_SUSPENSION_TO_CAR:
        {
            return OR_CONTINUE;
        }
        case COMMAND_DOES_CAR_HAVE_LOWRIDER_SUSPENSION:
        {
            return OR_CONTINUE;
        }
        case COMMAND_GET_RANDOM_CAR_IN_SPHERE:
        {
            return OR_CONTINUE;
        }
        case COMMAND_GET_COORDS_OF_CLOSEST_COLLECTABLE1:
        {
            return OR_CONTINUE;
        }
        case COMMAND_GET_STAT_CHANGE_AMOUNT:
        {
            return OR_CONTINUE;
        }
        case COMMAND_CLEAR_ALL_RELATIONSHIPS:
        {
            return OR_CONTINUE;
        }
        case COMMAND_SET_CHAR_SPECIAL_EVENT:
        {
            return OR_CONTINUE;
        }
        case COMMAND_SET_ATTRACTOR_AS_COVER_NODE:
        {
            return OR_CONTINUE;
        }
        case COMMAND_IS_SEARCHLIGHT_IN_ANGLED_AREA_2D:
        {
            return OR_CONTINUE;
        }
        case COMMAND_IS_SEARCHLIGHT_IN_ANGLED_AREA_3D:
        {
            return OR_CONTINUE;
        }
        case COMMAND_SWITCH_SEARCHLIGHT_BULB:
        {
            return OR_CONTINUE;
        }
    }
    return OR_INTERRUPT;
}

// MARK: 1900To1999

// stub: sa 0x46B460
OpcodeResult CRunningScript::ProcessCommands1900To1999(i32 CurrCommand)
{
    MARKFUNCTION(0x46B460);

    bool LatestCmpFlagResult;
    i16 ZoneIndex;
    char ZoneLabel[10];

    CPed* pPed;
    CVehicle *pVehicle, *pTrailer, *pSecondVehicle;
    CObject* pObj;

    CEntity* pTargetEntity;
    CEntity* pTagEntity;

    CPhysical* pPhysical;

    CBaseModelInfo* pModelInfo;
    CVehicleModelInfo* pVehicleModelInfo;

    CPlayerInfo* pPlayer;

    CTask* pTask;

    f32 NewX, NewY, NewZ, NewW;
    f32 tempfloat;

    i32 ActualIndex;

    CVector TempCoors;
    i32 IPLSlotIndex;
    char IPLName[18];

    f32 temp_float, fDistance, fMinHeightAboveTerrain;

    u32 ItemID;

    CTrain *pTrainEngine, *pTrainCarriage;

    i32* pGlobalVar;

    f32 fRopeHeight;

    i32 SearchLightIndex;

    tBeatInfo* pBeatInfo;
    i32 TimeTillNextBeat, TimeSinceLastBeat;

    f32 MinZ, MinY, MinX;
    f32 MaxZ, MaxY, MaxX;

    CQuaternion temp_quat(0.0f, 0.0f, 0.0f, 0.0f);
    RwMatrix* pRwMatrix;
    RwMatrix temp_matrix;

    switch (CurrCommand)
    {
        // sa: 0x5e (opcode 1900)
        case COMMAND_SET_ZONE_GANG_STRENGTH:
        {
            ReadTextLabelFromScript(ZoneLabel, 8);
            CollectParameters(2);
            ZoneIndex = CTheZones::FindZoneByLabelAndReturnIndex(ZoneLabel, ZONE_INFORMATION);
            if (ZoneIndex < 0)
            {
                return OR_CONTINUE;
            }
            CTheZones::ZoneInfoArray[CTheZones::NavigationZoneArray[ZoneIndex].ZoneInfoIndex].aGangStrengths[ScriptParams[0]] = ScriptParams[1];
            return OR_CONTINUE;
        }
        // sa: 0xb7 (opcode 1901)
        case COMMAND_GET_ZONE_GANG_STRENGTH:
        {
            ReadTextLabelFromScript(ZoneLabel, 8);
            CollectParameters(1);
            ScriptParams[0] = CTheZones::ZoneInfoArray[CTheZones::NavigationZoneArray[CTheZones::FindZoneByLabelAndReturnIndex(ZoneLabel, ZONE_INFORMATION)].ZoneInfoIndex].aGangStrengths[ScriptParams[0]];
            StoreParameters(1);
            return OR_CONTINUE;
        }
        // sa: 0x111 (opcode 1903)
        case COMMAND_IS_MESSAGE_BEING_DISPLAYED:
        {
            LatestCmpFlagResult = false;
            if (CMessages::BriefMessages[0].pMessage != nullptr)
            {
                LatestCmpFlagResult = true;
            }
            UpdateCompareFlag(LatestCmpFlagResult);
            return OR_CONTINUE;
        }
        // sa: 0x137 (opcode 1904)
        case COMMAND_SET_CHAR_IS_TARGET_PRIORITY:
        {
            CollectParameters(2);
            pPed = CPools::GetPedPool().GetAt(ScriptParams[0]);
            pPed->bThisPedIsATargetPriority = ScriptParams[1] != 0;
            return OR_CONTINUE;
        }
        // sa: 0x188 (opcode 1905)
        case COMMAND_CUSTOM_PLATE_DESIGN_FOR_NEXT_CAR:
        {
            CollectParameters(2);
            pVehicleModelInfo = (CVehicleModelInfo*)CModelInfo::GetModelInfo(ScriptParams[0]);
            if (!pVehicleModelInfo || pVehicleModelInfo->GetModelType() != MI_TYPE_VEHICLE || !pVehicleModelInfo->GetCustomPlateMaterial())
            {
                return OR_CONTINUE;
            }
            pVehicleModelInfo->SetCustomCarPlateDesign(ScriptParams[1]);
            return OR_CONTINUE;
        }
        // sa: 0x1d0 (opcode 1906)
        case COMMAND_TASK_GOTO_CAR:
        {
            CollectParameters(4);
            ActualIndex = ScriptParams[0];
            tempfloat = *(f32*)&ScriptParams[3];
            pVehicle = CPools::GetVehiclePool().GetAt(ScriptParams[1]);
            if (ScriptParams[2] < 0)
            {
                ScriptParams[2] = 50000;
            }
            pTask = new CTaskComplexSeekEntity<CEntitySeekPosCalculatorStandard>(pVehicle, ScriptParams[2], 1000, tempfloat, 2.0f, 2.0f, true, true);
            GivePedScriptedTask(ActualIndex, pTask, CurrCommand);
            return OR_CONTINUE;
        }
        // sa: 0x274 (opcode 1910)
        case COMMAND_REQUEST_IPL:
        {
            ReadTextLabelFromScript(IPLName, 18);
            IPLSlotIndex = CIplStore::FindIplSlot(IPLName);
            CIplStore::RequestIplAndIgnore(IPLSlotIndex);
            return OR_CONTINUE;
        }
        // sa: 0x29c (opcode 1911)
        case COMMAND_REMOVE_IPL:
        {
            ReadTextLabelFromScript(IPLName, 18);
            IPLSlotIndex = CIplStore::FindIplSlot(IPLName);
            CIplStore::RemoveIplAndIgnore(IPLSlotIndex);
            return OR_CONTINUE;
        }
        // sa: 0x2c4 (opcode 1912)
        case COMMAND_REMOVE_IPL_DISCREETLY:
        {
            ReadTextLabelFromScript(IPLName, 18);
            IPLSlotIndex = CIplStore::FindIplSlot(IPLName);
            CIplStore::RemoveIplWhenFarAway(IPLSlotIndex);
            return OR_CONTINUE;
        }
        // sa: 0x2ec (opcode 1914)
        case COMMAND_SET_CHAR_RELATIONSHIP:
        {
            CollectParameters(3);
            pPed = CPools::GetPedPool().GetAt(ScriptParams[0]);
            pPed->m_acquaintances.SetAsAcquaintance(ScriptParams[1], CPedType::GetPedFlag(ScriptParams[2]));
            return OR_CONTINUE;
        }
        // sa: 0x331 (opcode 1915)
        case COMMAND_CLEAR_CHAR_RELATIONSHIP:
        {
            CollectParameters(3);
            pPed = CPools::GetPedPool().GetAt(ScriptParams[0]);
            pPed->m_acquaintances.ClearAsAcquaintance(ScriptParams[1], CPedType::GetPedFlag(ScriptParams[2]));
            return OR_CONTINUE;
        }
        // sa: 0x377 (opcode 1916)
        case COMMAND_CLEAR_ALL_CHAR_RELATIONSHIPS:
        {
            CollectParameters(2);
            pPed = CPools::GetPedPool().GetAt(ScriptParams[0]);
            pPed->m_acquaintances.ClearAcquaintances(ScriptParams[1]);
            return OR_CONTINUE;
        }
        // sa: 0x3ac (opcode 1917)
        case COMMAND_GET_CAR_PITCH:
        {
            CollectParameters(1);
            pVehicle = CPools::GetVehiclePool().GetAt(ScriptParams[0]);
            tempfloat = ((CAutomobile*)pVehicle)->GetCarPitch() * 57.295776f;
            if (tempfloat < 0.0f)
            {
                tempfloat = tempfloat + 360.0f;
            }
            if (tempfloat > 360.0f)
            {
                tempfloat = tempfloat - 360.0f;
            }
            ScriptParams[0] = *(i32*)&tempfloat;
            StoreParameters(1);
            return OR_CONTINUE;
        }
        // sa: 0x42c (opcode 1918)
        case COMMAND_GET_AREA_VISIBLE:
        {
            ScriptParams[0] = CGame::currArea;
            StoreParameters(1);
            return OR_CONTINUE;
        }
        // sa: 0x446 (opcode 1920)
        case COMMAND_HELI_KEEP_ENTITY_IN_VIEW:
        {
            CollectParameters(5);
            pVehicle = CPools::GetVehiclePool().GetAt(ScriptParams[0]);
            pTargetEntity = nullptr;
            if (ScriptParams[1] >= 0)
            {
                pTargetEntity = CPools::GetPedPool().GetAt(ScriptParams[1]);
            }
            if (ScriptParams[2] >= 0)
            {
                pTargetEntity = CPools::GetVehiclePool().GetAt(ScriptParams[2]);
            }
            NewX = *(f32*)&ScriptParams[3];
            NewY = *(f32*)&ScriptParams[4];
            pVehicle->AutoPilot.SetMission(MISSION_HELI_KEEP_ENTITY_IN_VIEW);
            if (pVehicle->AutoPilot.pTargetEntity)
            {
                pVehicle->AutoPilot.pTargetEntity->CleanUpOldReference((CEntity**)&pVehicle->AutoPilot.pTargetEntity);
            }
            pVehicle->AutoPilot.pTargetEntity = pTargetEntity;
            pTargetEntity->RegisterReference((CEntity**)&pVehicle->AutoPilot.pTargetEntity);
            pVehicle->AutoPilot.CruiseSpeed = 100;
            pVehicle->AutoPilot.HooverDistFromTarget = NewX;
            ((CHeli*)pVehicle)->m_MinHeightAboveTerrain = NewY;
            return OR_CONTINUE;
        }
        // sa: 0x500 (opcode 1921)
        case COMMAND_GET_WEAPONTYPE_MODEL:
        {
            CollectParameters(1);
            ScriptParams[0] = CWeaponInfo::GetWeaponInfo((eWeaponType)ScriptParams[0], 1)->GetModelId();
            StoreParameters(1);
            return OR_CONTINUE;
        }
        // sa: 0x532 (opcode 1922)
        case COMMAND_GET_WEAPONTYPE_SLOT:
        {
            CollectParameters(1);
            ScriptParams[0] = CWeaponInfo::GetWeaponInfo((eWeaponType)ScriptParams[0], 1)->m_nWeaponSlot;
            StoreParameters(1);
            return OR_CONTINUE;
        }
        // sa: 0x565 (opcode 1923)
        case COMMAND_GET_SHOPPING_EXTRA_INFO:
        {
            CollectParameters(2);
            ScriptParams[0] = CShopping::GetExtraInfo(ScriptParams[0], ScriptParams[1]);
            StoreParameters(1);
            return OR_CONTINUE;
        }
        // sa: 0x598 (opcode 1924)
        case COMMAND_GIVE_PLAYER_CLOTHES:
        {
            CollectParameters(4);
            CWorld::Players[ScriptParams[0]].m_pPed->m_pPlayerData->m_pClothes->SetTextureAndModel(ScriptParams[1], ScriptParams[2], ScriptParams[3]);
            return OR_CONTINUE;
        }
        // sa: 0x5d3 (opcode 1926)
        case COMMAND_GET_NUMBER_OF_FIRES_IN_AREA:
        {
            CollectParameters(6);
            MinX = *(f32*)&ScriptParams[0];
            MinY = *(f32*)&ScriptParams[1];
            MinZ = *(f32*)&ScriptParams[2];
            MaxX = *(f32*)&ScriptParams[3];
            MaxY = *(f32*)&ScriptParams[4];
            MaxZ = *(f32*)&ScriptParams[5];
            if (MinX > MaxX)
            {
                temp_float = MaxX;
                MaxX = MinX;
                MinX = temp_float;
            }
            if (MinY > MaxY)
            {
                temp_float = MaxY;
                MaxY = MinY;
                MinY = temp_float;
            }
            if (MinZ > MaxZ)
            {
                temp_float = MaxZ;
                MaxZ = MinZ;
                MinZ = temp_float;
            }
            ScriptParams[0] = gFireManager.GetNumFiresInArea(MinX, MinY, MinZ, MaxX, MaxY, MaxZ);
            StoreParameters(1);
            return OR_CONTINUE;
        }
        // sa: 0x6b0 (opcode 1928)
        case COMMAND_ATTACH_WINCH_TO_HELI:
        {
            CollectParameters(2);
            pVehicle = CPools::GetVehiclePool().GetAt(ScriptParams[0]);
            pVehicle->InitWinch(ScriptParams[1]);
            return OR_CONTINUE;
        }
        // sa: 0x6df (opcode 1929)
        case COMMAND_RELEASE_ENTITY_FROM_WINCH:
        {
            CollectParameters(1);
            pVehicle = CPools::GetVehiclePool().GetAt(ScriptParams[0]);
            pVehicle->ReleasePickedUpEntityWithWinch();
            return OR_CONTINUE;
        }
        // sa: 0x708 (opcode 1930)
        case COMMAND_GET_TRAIN_CARRIAGE:
        {
            CollectParameters(2);
            pTrainEngine = (CTrain*)CPools::GetVehiclePool().GetAt(ScriptParams[0]);
            pTrainCarriage = CTrain::FindCarriage(pTrainEngine, ScriptParams[1]);
            if (pTrainCarriage)
            {
                ScriptParams[0] = CPools::GetVehiclePool().GetIndex(pTrainCarriage);
            }
            else
            {
                ScriptParams[0] = -1;
            }
            StoreParameters(1);
            return OR_CONTINUE;
        }
        // sa: 0x773 (opcode 1931)
        case COMMAND_GRAB_ENTITY_ON_WINCH:
        {
            CollectParameters(1);
            pVehicle = CPools::GetVehiclePool().GetAt(ScriptParams[0]);
            ScriptParams[0] = -1;
            ScriptParams[1] = -1;
            ScriptParams[2] = -1;
            pPhysical = static_cast<CPhysical*>(pVehicle->QueryPickedUpEntityWithWinch());
            if (pPhysical)
            {
                switch (pPhysical->GetType() & 7)
                {
                    case ENTITY_TYPE_VEHICLE:
                        ScriptParams[0] = CPools::GetVehiclePool().GetIndex((CVehicle*)pPhysical);
                        break;
                    case ENTITY_TYPE_PED:
                        ScriptParams[1] = CPools::GetPedPool().GetIndex((CPed*)pPhysical);
                        break;
                    case ENTITY_TYPE_OBJECT:
                        ScriptParams[2] = CPools::GetObjectPool().GetIndex((CObject*)pPhysical);
                        break;
                }
            }
            StoreParameters(3);
            return OR_CONTINUE;
        }
        // sa: 0x7d0 (opcode 1932)
        case COMMAND_GET_NAME_OF_ITEM:
        {
            CollectParameters(1);
            const char* pNameTag = CShopping::GetNameTag(ScriptParams[0]);
            char* pScriptVariable = (char*)GetPointerToScriptVariable(SCOPE_GLOBAL);
            strncpy(pScriptVariable, pNameTag, 8);
            return OR_CONTINUE;
        }
        // sa: 0x805 (opcode 1935)
        case COMMAND_TASK_CLIMB:
        {
            CollectParameters(2);
            ActualIndex = ScriptParams[0];
            LatestCmpFlagResult = ScriptParams[1] != 0;
            pTask = new CTaskComplexClimb();
            ((CTaskComplexClimb*)pTask)->SetUsePlayerLaunchForce(LatestCmpFlagResult);
            GivePedScriptedTask(ActualIndex, pTask, CurrCommand);
            return OR_CONTINUE;
        }
        // sa: 0x866 (opcode 1936)
        case COMMAND_BUY_ITEM:
        {
            CollectParameters(1);
            CShopping::Buy(ScriptParams[0], 0);
            return OR_CONTINUE;
        }
        // sa: 0x886 (opcode 1938)
        case COMMAND_CLEAR_CHAR_TASKS_IMMEDIATELY:
        {
            CollectParameters(1);
            pPed = CPools::GetPedPool().GetAt(ScriptParams[0]);
            pPed->GetPedIntelligence()->FlushImmediately(true);
            return OR_CONTINUE;
        }
        // sa: 0x8b5 (opcode 1939)
        case COMMAND_STORE_CLOTHES_STATE:
        {
            CShopping::StoreClothesState();
            return OR_CONTINUE;
        }
        // sa: 0x8c1 (opcode 1940)
        case COMMAND_RESTORE_CLOTHES_STATE:
        {
            CShopping::RestoreClothesState();
            return OR_CONTINUE;
        }
        // sa: 0x8cd (opcode 1942)
        case COMMAND_GET_ROPE_HEIGHT_FOR_OBJECT:
        {
            CollectParameters(1);
            pObj = CPools::GetObjectPool().GetAt(ScriptParams[0]);
            *(f32*)&ScriptParams[0] = pObj->GetRopeHeight();
            StoreParameters(1);
            return OR_CONTINUE;
        }
        // sa: 0x905 (opcode 1943)
        case COMMAND_SET_ROPE_HEIGHT_FOR_OBJECT:
        {
            CollectParameters(2);
            fRopeHeight = *(f32*)&ScriptParams[1];
            pObj = CPools::GetObjectPool().GetAt(ScriptParams[0]);
            pObj->SetRopeHeight(fRopeHeight);
            return OR_CONTINUE;
        }
        // sa: 0x934 (opcode 1944)
        case COMMAND_GRAB_ENTITY_ON_ROPE_FOR_OBJECT:
        {
            CollectParameters(1);
            pObj = CPools::GetObjectPool().GetAt(ScriptParams[0]);
            ScriptParams[0] = -1;
            ScriptParams[1] = -1;
            ScriptParams[2] = -1;
            pPhysical = static_cast<CPhysical*>(pObj->GetObjectCarriedWithRope());
            if (pPhysical)
            {
                switch (pPhysical->GetType() & 7)
                {
                    case ENTITY_TYPE_VEHICLE:
                        ScriptParams[0] = CPools::GetVehiclePool().GetIndex((CVehicle*)pPhysical);
                        break;
                    case ENTITY_TYPE_PED:
                        ScriptParams[1] = CPools::GetPedPool().GetIndex((CPed*)pPhysical);
                        break;
                    case ENTITY_TYPE_OBJECT:
                        ScriptParams[2] = CPools::GetObjectPool().GetIndex((CObject*)pPhysical);
                        break;
                }
            }
            StoreParameters(3);
            return OR_CONTINUE;
        }
        // sa: 0x9d0 (opcode 1945)
        case COMMAND_RELEASE_ENTITY_FROM_ROPE_FOR_OBJECT:
        {
            CollectParameters(1);
            pObj = CPools::GetObjectPool().GetAt(ScriptParams[0]);
            pObj->ReleaseObjectCarriedWithRope();
            return OR_CONTINUE;
        }
        // sa: 0x9f8 (opcode 1949)
        case COMMAND_PLAYER_ENTERED_DOCK_CRANE:
        {
            CRopes::PlayerControlsCrane = static_cast<eControlledCrane>(1);
            CWaterLevel::m_bWaterFogScript = false;
            return OR_CONTINUE;
        }
        // sa: 0xa10 (opcode 1950)
        case COMMAND_PLAYER_ENTERED_BUILDINGSITE_CRANE:
        {
            CRopes::PlayerControlsCrane = static_cast<eControlledCrane>(2);
            CWaterLevel::m_bWaterFogScript = false;
            return OR_CONTINUE;
        }
        // sa: 0xa28 (opcode 1951)
        case COMMAND_PLAYER_LEFT_CRANE:
        {
            CRopes::PlayerControlsCrane = static_cast<eControlledCrane>(0);
            CWaterLevel::m_bWaterFogScript = true;
            return OR_CONTINUE;
        }
        // sa: 0xa40 (opcode 1952)
        case COMMAND_PERFORM_SEQUENCE_TASK_FROM_PROGRESS:
        {
            CollectParameters(4);
            ActualIndex = ScriptParams[0];
            ItemID = CTheScripts::GetActualScriptThingIndex(ScriptParams[1], 4);
            if ((i32)ItemID < 0 || (i32)ItemID >= 64)
            {
                return OR_CONTINUE;
            }
            pTask = new CTaskComplexUseSequence(ItemID);
            ((CTaskComplexUseSequence*)pTask)->SetProgress(ScriptParams[2], ScriptParams[3]);
            GivePedScriptedTask(ActualIndex, pTask, CurrCommand);
            return OR_CONTINUE;
        }
        // sa: 0xad9 (opcode 1953)
        case COMMAND_SET_NEXT_DESIRED_MOVE_STATE:
        {
            CollectParameters(1);
            CPedIntelligence::ms_iDesiredMoveState = ScriptParams[0];
            return OR_CONTINUE;
        }
        // sa: 0xaf3 (opcode 1955)
        case COMMAND_TASK_GOTO_CHAR_AIMING:
        {
            CollectParameters(4);
            ActualIndex = ScriptParams[0];
            NewX = *(f32*)&ScriptParams[2];
            NewY = *(f32*)&ScriptParams[3];
            pPed = CPools::GetPedPool().GetAt(ScriptParams[1]);
            pTask = new CTaskComplexSeekEntityAiming(pPed, NewX, NewY);
            GivePedScriptedTask(ActualIndex, pTask, CurrCommand);
            return OR_CONTINUE;
        }
        // sa: 0xb78 (opcode 1956)
        case COMMAND_GET_SEQUENCE_PROGRESS_RECURSIVE:
        {
            CollectParameters(1);
            pPed = CPools::GetPedPool().GetAt(ScriptParams[0]);
            ActualIndex = -1;
            SearchLightIndex = -1;
            if (CPedScriptedTaskRecord::GetStatus(pPed, COMMAND_PERFORM_SEQUENCE_TASK) != eScriptedTaskStatus::EVENT_ASSOCIATED)
            {
                pTask = pPed->GetPedIntelligence()->GetTaskManager().GetTask(3);
                ActualIndex = ((CTaskComplexUseSequence*)pTask)->GetProgress1();
                pTask = pTask->GetSubTask();
                if (pTask && pTask->GetTaskType() == CTaskTypes::TASK_COMPLEX_USE_SEQUENCE)
                {
                    SearchLightIndex = ((CTaskComplexUseSequence*)pTask)->GetProgress1();
                }
            }
            ScriptParams[0] = ActualIndex;
            ScriptParams[1] = SearchLightIndex;
            StoreParameters(2);
            return OR_CONTINUE;
        }
        // sa: 0xbef (opcode 1957)
        case COMMAND_TASK_KILL_CHAR_ON_FOOT_TIMED:
        {
            CollectParameters(3);
            ActualIndex = ScriptParams[0];
            pPed = CPools::GetPedPool().GetAt(ScriptParams[1]);
            pTask = new CTaskComplexKillPedOnFoot(pPed, ScriptParams[2], 0, 0, 0, 1);
            GivePedScriptedTask(ActualIndex, pTask, CurrCommand);
            return OR_CONTINUE;
        }
        // sa: 0xc4f (opcode 1958)
        case COMMAND_GET_NEAREST_TAG_POSITION:
        {
            CollectParameters(3);
            TempCoors = *(CVector*)&ScriptParams[0];
            pTagEntity = CTagManager::GetNearestTag(TempCoors);
            if (pTagEntity)
            {
                TempCoors = pTagEntity->GetPosition();
                ScriptParams[0] = *(i32*)&TempCoors.x;
                ScriptParams[1] = *(i32*)&TempCoors.y;
                ScriptParams[2] = *(i32*)&TempCoors.z;
            }
            else
            {
                tempfloat = -4000.0f;
                ScriptParams[0] = *(i32*)&tempfloat;
                ScriptParams[1] = *(i32*)&tempfloat;
                ScriptParams[2] = *(i32*)&tempfloat;
            }
            StoreParameters(3);
            return OR_CONTINUE;
        }
        // sa: 0xce9 (opcode 1959)
        case COMMAND_TASK_JETPACK:
        {
            CollectParameters(1);
            ActualIndex = ScriptParams[0];
            pTask = new CTaskSimpleJetPack(nullptr, 10.0f, 0, nullptr);
            GivePedScriptedTask(ActualIndex, pTask, CurrCommand);
            return OR_CONTINUE;
        }
        // sa: 0xd47 (opcode 1960)
        case COMMAND_SET_AREA51_SAM_SITE:
        {
            CollectParameters(1);
            CObject::bArea51SamSiteDisabled = (ScriptParams[0] == 0);
            return OR_CONTINUE;
        }
        // sa: 0xd67 (opcode 1961)
        case COMMAND_IS_CHAR_IN_ANY_SEARCHLIGHT:
        {
            CollectParameters(1);
            pPed = CPools::GetPedPool().GetAt(ScriptParams[0]);
            LatestCmpFlagResult = false;
            ScriptParams[0] = -1;
            if (CTheScripts::IsEntityWithinAnySearchLight(pPed, &SearchLightIndex))
            {
                ScriptParams[0] = SearchLightIndex;
                LatestCmpFlagResult = true;
            }
            StoreParameters(1);
            UpdateCompareFlag(LatestCmpFlagResult);
            return OR_CONTINUE;
        }
        // sa: 0xdce (opcode 1963)
        case COMMAND_IS_TRAILER_ATTACHED_TO_CAB:
        {
            CollectParameters(2);
            pVehicle = (ScriptParams[0] == -1) ? nullptr : CPools::GetVehiclePool().GetAt(ScriptParams[0]);
            pSecondVehicle = (ScriptParams[1] == -1) ? nullptr : CPools::GetVehiclePool().GetAt(ScriptParams[1]);
            LatestCmpFlagResult = false;
            if (pVehicle)
            {
                if (pSecondVehicle)
                {
                    if (pVehicle->m_pTowingVehicle == pSecondVehicle)
                    {
                        LatestCmpFlagResult = true;
                    }
                }
                else if (pVehicle->m_pTowingVehicle)
                {
                    LatestCmpFlagResult = true;
                }
            }
            else if (pSecondVehicle->m_pVehicleBeingTowed)
            {
                LatestCmpFlagResult = true;
            }
            UpdateCompareFlag(LatestCmpFlagResult);
            return OR_CONTINUE;
        }
        // sa: 0xe4f (opcode 1964)
        case COMMAND_DETACH_TRAILER_FROM_CAB:
        {
            CollectParameters(2);
            pVehicle = (ScriptParams[0] == -1) ? nullptr : CPools::GetVehiclePool().GetAt(ScriptParams[0]);
            pSecondVehicle = (ScriptParams[1] == -1) ? nullptr : CPools::GetVehiclePool().GetAt(ScriptParams[1]);
            if (pVehicle)
            {
                pVehicle->BreakTowLink();
                return OR_CONTINUE;
            }
            pTrailer = pSecondVehicle->m_pVehicleBeingTowed;
            if (pTrailer)
            {
                pTrailer->BreakTowLink();
            }
            return OR_CONTINUE;
        }
        // sa: 0xed4 (opcode 1967)
        case COMMAND_GET_PLAYER_GROUP:
        {
            CollectParameters(1);
            ScriptParams[0] = CTheScripts::GetUniqueScriptThingIndex(CWorld::Players[ScriptParams[0]].m_pPed->m_pPlayerData->m_PlayerGroup, 8);
            StoreParameters(1);
            return OR_CONTINUE;
        }
        // sa: 0xf04 (opcode 1968)
        case COMMAND_GET_LOADED_SHOP:
        {
            strncpy(ZoneLabel, CShopping::GetShopLoaded(), 8);
            MakeUpperCase(ZoneLabel);
            pGlobalVar = GetPointerToScriptVariable(SCOPE_GLOBAL);
            strncpy((char*)pGlobalVar, ZoneLabel, 8);
            return OR_CONTINUE;
        }
        // sa: 0xf42 (opcode 1969)
        case COMMAND_GET_BEAT_PROXIMITY:
        {
            CollectParameters(1);
            if (ScriptParams[0] < 0)
            {
                ActualIndex = ScriptParams[0] + 10;
            }
            else if (ScriptParams[0] > 0)
            {
                ActualIndex = ScriptParams[0] + 9;
            }
            else
            {
                pBeatInfo = AudioEngine.GetBeatInfo();
                if (pBeatInfo->bBeatInfoPresent)
                {
                    TimeTillNextBeat = pBeatInfo->BeatWindow[10].Time;
                    if (TimeTillNextBeat && pBeatInfo->BeatNumber && abs(TimeTillNextBeat) > abs(pBeatInfo->BeatWindow[9].Time))
                    {
                        ScriptParams[0] = pBeatInfo->BeatWindow[9].Time;
                        ScriptParams[1] = pBeatInfo->BeatWindow[9].Type;
                        ScriptParams[2] = pBeatInfo->BeatNumber - 1;
                    }
                    else
                    {
                        ScriptParams[0] = pBeatInfo->BeatWindow[10].Time;
                        ScriptParams[1] = pBeatInfo->BeatWindow[10].Type;
                        ScriptParams[2] = pBeatInfo->BeatNumber;
                    }
                    StoreParameters(3);
                    return OR_CONTINUE;
                }
                ScriptParams[0] = -1;
                ScriptParams[1] = -1;
                ScriptParams[2] = -1;
                StoreParameters(3);
                return OR_CONTINUE;
            }
            pBeatInfo = AudioEngine.GetBeatInfo();
            if (!pBeatInfo->bBeatInfoPresent)
            {
                ScriptParams[0] = -1;
                ScriptParams[1] = -1;
                ScriptParams[2] = -1;
                StoreParameters(3);
                return OR_CONTINUE;
            }
            ScriptParams[0] = pBeatInfo->BeatWindow[ActualIndex].Time;
            ScriptParams[1] = pBeatInfo->BeatWindow[ActualIndex].Type;
            ScriptParams[2] = pBeatInfo->BeatNumber;
            StoreParameters(3);
            return OR_CONTINUE;
        }
        // sa: 0x1057 (opcode 1971)
        case COMMAND_SET_GROUP_DEFAULT_TASK_ALLOCATOR:
        {
            CollectParameters(2);
            ActualIndex = CTheScripts::GetActualScriptThingIndex(ScriptParams[0], 8);
            if (ActualIndex < 0 || ActualIndex >= 8)
            {
                return OR_CONTINUE;
            }
            CPedGroups::ms_groups[ActualIndex].GetGroupIntelligence()->SetDefaultTaskAllocatorType(ScriptParams[1]);
            return OR_CONTINUE;
        }
        // sa: 0x10a3 (opcode 1972)
        case COMMAND_SET_PLAYER_GROUP_RECRUITMENT:
        {
            CollectParameters(2);
            pPlayer = &CWorld::Players[ScriptParams[0]];
            if (ScriptParams[1])
            {
                pPlayer->m_pPed->m_pPlayerData->m_GroupStuffDisabled = false;
            }
            else
            {
                pPlayer->m_pPed->m_pPlayerData->m_GroupStuffDisabled = true;
            }
            return OR_CONTINUE;
        }
        // sa: 0x10f2 (opcode 1979)
        case COMMAND_ACTIVATE_HELI_SPEED_CHEAT:
        {
            CollectParameters(2);
            CPools::GetVehiclePool().GetAt(ScriptParams[0])->AutoPilot.SpeedCheat = ScriptParams[1];
            return OR_CONTINUE;
        }
        // sa: 0x1120 (opcode 1980)
        case COMMAND_TASK_SET_CHAR_DECISION_MAKER:
        {
            CollectParameters(2);
            ActualIndex = ScriptParams[0];
            SearchLightIndex = -1;
            if (ScriptParams[1] != -1)
            {
                SearchLightIndex = CTheScripts::GetActualScriptThingIndex(ScriptParams[1], 7);
            }
            if (ActualIndex != -1)
            {
                pPed = CPools::GetPedPool().GetAt(ActualIndex);
                pPed->GetPedIntelligence()->SetPedDecisionMakerType(SearchLightIndex);
                return OR_CONTINUE;
            }
            pTask = new CTaskSimpleSetCharDecisionMaker(SearchLightIndex);
            GivePedScriptedTask(-1, pTask, CurrCommand);
            return OR_CONTINUE;
        }
        // sa: 0x11b5 (opcode 1981)
        case COMMAND_DELETE_MISSION_TRAIN:
        {
            CollectParameters(1);
            pTrainEngine = (CTrain*)CPools::GetVehiclePool().GetAt(ScriptParams[0]);
            if (!pTrainEngine)
            {
                return OR_CONTINUE;
            }
            CTrain::RemoveOneMissionTrain(pTrainEngine);
            return OR_CONTINUE;
        }
        // sa: 0x11e7 (opcode 1982)
        case COMMAND_MARK_MISSION_TRAIN_AS_NO_LONGER_NEEDED:
        {
            CollectParameters(1);
            pTrainEngine = (CTrain*)CPools::GetVehiclePool().GetAt(ScriptParams[0]);
            if (!pTrainEngine)
            {
                return OR_CONTINUE;
            }
            CTrain::ReleaseOneMissionTrain(pTrainEngine);
            return OR_CONTINUE;
        }
        // sa: 0x121a (opcode 1983)
        case COMMAND_SET_BLIP_ALWAYS_DISPLAY_ON_ZOOMED_RADAR:
        {
            CollectParameters(2);
            CRadar::SetBlipAlwaysDisplayInZoom(ScriptParams[0], ScriptParams[1]);
            return OR_CONTINUE;
        }
        // sa: 0x1241 (opcode 1984)
        case COMMAND_REQUEST_CAR_RECORDING:
        {
            CollectParameters(1);
            CVehicleRecording::RequestRecordingFile(ScriptParams[0]);
            return OR_CONTINUE;
        }
        // sa: 0x1260 (opcode 1985)
        case COMMAND_HAS_CAR_RECORDING_BEEN_LOADED:
        {
            CollectParameters(1);
            LatestCmpFlagResult = false;
            if (CVehicleRecording::HasRecordingFileBeenLoaded(ScriptParams[0]))
            {
                LatestCmpFlagResult = true;
            }
            UpdateCompareFlag(LatestCmpFlagResult);
            return OR_CONTINUE;
        }
        // sa: 0x1284 (opcode 1987)
        case COMMAND_GET_OBJECT_QUATERNION:
        {
            CollectParameters(1);
            pObj = CPools::GetObjectPool().GetAt(ScriptParams[0]);
            temp_quat.Set(*pObj->GetRwMatrix());
            *(CQuaternion*)&ScriptParams[0] = temp_quat;
            StoreParameters(4);
            return OR_CONTINUE;
        }
        // sa: 0x12e7 (opcode 1988)
        case COMMAND_SET_OBJECT_QUATERNION:
        {
            CollectParameters(5);
            pObj = CPools::GetObjectPool().GetAt(ScriptParams[0]);
            temp_quat = *(CQuaternion*)&ScriptParams[1];
            temp_quat.Get(&temp_matrix);
            TempCoors = pObj->GetPosition();
            {
                CMatrix NewMatrix(&temp_matrix, false);
                pObj->SetMatrix(NewMatrix);
            }
            pObj->SetPosition(TempCoors);
            return OR_CONTINUE;
        }
        // sa: 0x1397 (opcode 1989)
        case COMMAND_GET_VEHICLE_QUATERNION:
        {
            CollectParameters(1);
            pVehicle = CPools::GetVehiclePool().GetAt(ScriptParams[0]);
            temp_quat.Set(*pVehicle->GetRwMatrix());
            *(CQuaternion*)&ScriptParams[0] = temp_quat;
            StoreParameters(4);
            return OR_CONTINUE;
        }
        // sa: 0x13fa (opcode 1990)
        case COMMAND_SET_VEHICLE_QUATERNION:
        {
            CollectParameters(5);
            pVehicle = CPools::GetVehiclePool().GetAt(ScriptParams[0]);
            temp_quat = *(CQuaternion*)&ScriptParams[1];
            temp_quat.Get(&temp_matrix);
            TempCoors = pVehicle->GetPosition();
            {
                CMatrix NewMatrix(&temp_matrix, false);
                pVehicle->SetMatrix(NewMatrix);
            }
            pVehicle->SetPosition(TempCoors);
            return OR_CONTINUE;
        }
        // sa: 0x14c8 (opcode 1991)
        case COMMAND_SET_MISSION_TRAIN_COORDINATES:
        {
            CollectParameters(4);
            TempCoors.x = *(f32*)&ScriptParams[1];
            TempCoors.y = *(f32*)&ScriptParams[2];
            TempCoors.z = *(f32*)&ScriptParams[3];
            pTrainEngine = (CTrain*)CPools::GetVehiclePool().GetAt(ScriptParams[0]);
            CTrain::SetNewTrainPosition(pTrainEngine, TempCoors);
            return OR_CONTINUE;
        }
        // sa: 0x1528 (opcode 1993)
        case COMMAND_TASK_COMPLEX_PICKUP_OBJECT:
        {
            CollectParameters(2);
            pObj = CPools::GetObjectPool().GetAt(ScriptParams[1]);
            pTask = new CTaskComplexGoPickUpEntity(pObj, ANIM_CARRY_BOX);
            GivePedScriptedTask(ScriptParams[0], pTask, CurrCommand);
            return OR_CONTINUE;
        }
        // sa: 0x1592 (opcode 1995)
        case COMMAND_LISTEN_TO_PLAYER_GROUP_COMMANDS:
        {
            CollectParameters(2);
            pPed = CPools::GetPedPool().GetAt(ScriptParams[0]);
            pPed->bDoesntListenToPlayerGroupCommands = !ScriptParams[1];
            return OR_CONTINUE;
        }
        // sa: 0x15dc (opcode 1996)
        case COMMAND_SET_PLAYER_ENTER_CAR_BUTTON:
        {
            CollectParameters(2);
            if (ScriptParams[1])
            {
                CPad::GetPad(ScriptParams[0])->bDisablePlayerEnterCar = false;
            }
            else
            {
                CPad::GetPad(ScriptParams[0])->bDisablePlayerEnterCar = true;
            }
            return OR_CONTINUE;
        }
        // sa: 0x1628 (opcode 1997)
        case COMMAND_TASK_CHAR_SLIDE_TO_COORD:
        {
            CollectParameters(6);
            ActualIndex = ScriptParams[0];
            tempfloat = DEGTORAD(*(float*)&ScriptParams[4]);
            fDistance = *(f32*)&ScriptParams[5];
            if (fDistance < 0.0f)
            {
                fDistance = 0.1f;
            }
            TempCoors.x = *(f32*)&ScriptParams[1];
            TempCoors.y = *(f32*)&ScriptParams[2];
            TempCoors.z = *(f32*)&ScriptParams[3];
            pTask = new CTaskComplexSequence();
            ((CTaskComplexSequence*)pTask)->AddTask(new CTaskComplexGoToPointAndStandStill(static_cast<eMoveState>(PEDMOVE_WALK), TempCoors, CTaskComplexGoToPointAndStandStill::ms_fTargetRadius, CTaskComplexGoToPointAndStandStill::ms_fSlowDownDistance, false, false));
            ((CTaskComplexSequence*)pTask)->AddTask(new CTaskSimpleSlideToCoord(TempCoors, tempfloat, fDistance));
            GivePedScriptedTask(ActualIndex, pTask, CurrCommand);
            return OR_CONTINUE;
        }
        // sa: 0x176b — MSVC pliega estos cuerpos identicos en un solo bloque
        case COMMAND_SET_NO_POLICE_DURING_LA_RIOTS:
        {
            return OR_CONTINUE;
        }
        case COMMAND_CLEAR_HELP_WITH_THIS_LABEL:
        {
            return OR_CONTINUE;
        }
        case COMMAND_IS_SEARCHLIGHT_BULB_ON:
        {
            return OR_CONTINUE;
        }
        case COMMAND_CREATE_OIL_PUDDLE:
        {
            return OR_CONTINUE;
        }
        case COMMAND_TASK_OPEN_PASSENGER_DOOR:
        {
            return OR_CONTINUE;
        }
        case COMMAND_ADD_INT_TO_VAR_CONSOLE:
        {
            return OR_CONTINUE;
        }
        case COMMAND_GIVE_PLAYER_TATTOO:
        {
            return OR_CONTINUE;
        }
        case COMMAND_SET_CHAR_TYRES_CAN_BE_BURST:
        {
            return OR_CONTINUE;
        }
        case COMMAND_ADD_FLOAT_TO_VAR_CONSOLE:
        {
            return OR_CONTINUE;
        }
        case COMMAND_TASK_DRAG_CHAR_FROM_CAR:
        {
            return OR_CONTINUE;
        }
        case COMMAND_BUY_TATTOO:
        {
            return OR_CONTINUE;
        }
        case COMMAND_DELETE_WINCH_FOR_HELI:
        {
            return OR_CONTINUE;
        }
        case COMMAND_ATTACH_CAR_TO_ROPE_FOR_OBJECT:
        {
            return OR_CONTINUE;
        }
        case COMMAND_ATTACH_CHAR_TO_ROPE_FOR_OBJECT:
        {
            return OR_CONTINUE;
        }
        case COMMAND_ATTACH_OBJECT_TO_ROPE_FOR_OBJECT:
        {
            return OR_CONTINUE;
        }
        case COMMAND_SET_NEXT_EVENT_RESPONSE_SEQUENCE:
        {
            return OR_CONTINUE;
        }
        case COMMAND_GET_SEARCHLIGHT_COORDS:
        {
            return OR_CONTINUE;
        }
        case COMMAND_GET_TRAILER_ATTACHED_TO_CAB:
        {
            return OR_CONTINUE;
        }
        case COMMAND_GET_CAB_ATTACHED_TO_TRAILER:
        {
            return OR_CONTINUE;
        }
        case COMMAND_SET_BEAT_ZONE_SIZE:
        {
            return OR_CONTINUE;
        }
        case COMMAND_DISPLAY_TWO_ONSCREEN_COUNTERS:
        {
            return OR_CONTINUE;
        }
        case COMMAND_DISPLAY_TWO_ONSCREEN_COUNTERS_WITH_STRING:
        {
            return OR_CONTINUE;
        }
        case COMMAND_DISPLAY_NTH_TWO_ONSCREEN_COUNTERS:
        {
            return OR_CONTINUE;
        }
        case COMMAND_DISPLAY_NTH_TWO_ONSCREEN_COUNTERS_WITH_STRING:
        {
            return OR_CONTINUE;
        }
        case COMMAND_HELI_AIM_AHEAD_OF_TARGET_ENTITY:
        {
            return OR_CONTINUE;
        }
        case COMMAND_DISPLAY_PLAYBACK_RECORDED_CAR:
        {
            return OR_CONTINUE;
        }
        case COMMAND_DISPLAY_DEBUG_MESSAGE:
        {
            return OR_CONTINUE;
        }
        case COMMAND_TASK_SIMPLE_PUTDOWN_OBJECT:
        {
            return OR_CONTINUE;
        }
        case COMMAND_SET_BULLET_WHIZZ_BY_DISTANCE:
        {
            return OR_CONTINUE;
        }
        case COMMAND_SET_TWO_PLAYER_CAM_MODE_SEPARATE_CARS:
        {
            return OR_CONTINUE;
        }
    }
    return OR_INTERRUPT;
}

// MARK: 2000To2099

// func: sa 0x472310
OpcodeResult CRunningScript::ProcessCommands2000To2099(i32 CurrCommand)
{
    MARKFUNCTION(0x472310);

    bool LatestCmpFlagResult;
    CPed *pPed, *pSecondPed;
    CVehicle *pVehicle, *pSecondVehicle;
    f32 NewZ, NewY, NewX;
    CVector TempCoors, vecDiff, TempVec;
    CVector VelocityVector;
    f32 fRadius;
    f32 ZComponent, YComponent, XComponent;
    f32 MinY, MaxY, MinX, MaxX, MinZ, MaxZ;
    i32 ModelIndex, ArrayIndex;
    i32 ActualIndex, TempID;
    CObject *pObj, *pSecondObj;
    CPlayerInfo* pPlayer;
    const GxtChar* pString;
    char TextLabel[10];
    char ScriptBrainName[10];
    char LongTextLabel[18];
    i32* pGlobalVar;
    i32* pLocalVar;
    f32 fraction, temp_float;
    i32 temp_int, fraction_int;
    char AnimName[24];
    char AnimGroupName[16];
    CTask *pTask, *pTask2;
    i32 TotalKills;
    CRGBA tempRGBA;

    // NOTE: Keep switch cases in strictly ascending order as in commands.hpp to preserve MSVC jump table layout
    switch (CurrCommand)
    {
        case COMMAND_GET_CURRENT_DAY_OF_WEEK:
            ScriptParams[0] = CClock::CurrentDay;
            StoreParameters(1);
            return OR_CONTINUE;
        case COMMAND_SET_CURRENT_DAY_OF_WEEK:
            return OR_CONTINUE;
        case COMMAND_ACTIVATE_INTERIORS:
            return OR_CONTINUE;
        case COMMAND_REGISTER_SCRIPT_BRAIN_FOR_CODE_USE:
            CollectParameters(1);
            ScriptParams[0] = CTheScripts::StreamedScripts.GetProperIndexFromIndexUsedByScript(static_cast<i16>(ScriptParams[0]));
            ReadTextLabelFromScript(ScriptBrainName, 8);
            CTheScripts::ScriptsForBrains.AddNewStreamedScriptBrainForCodeUse(static_cast<i16>(ScriptParams[0]), ScriptBrainName,
                3 /* CODE_PED */);
            return OR_CONTINUE;
        case COMMAND_REGISTER_OBJECT_SCRIPT_BRAIN_FOR_CODE_USE:
            return OR_CONTINUE;
        case COMMAND_APPLY_FORCE_TO_CAR:
            CollectParameters(7);
            pVehicle = CPools::GetVehiclePool().GetAt(ScriptParams[0]);
            VelocityVector.x = *reinterpret_cast<f32*>(&ScriptParams[1]);
            VelocityVector.y = *reinterpret_cast<f32*>(&ScriptParams[2]);
            VelocityVector.z = *reinterpret_cast<f32*>(&ScriptParams[3]);
            TempCoors.x = *reinterpret_cast<f32*>(&ScriptParams[4]);
            TempCoors.y = *reinterpret_cast<f32*>(&ScriptParams[5]);
            TempCoors.z = *reinterpret_cast<f32*>(&ScriptParams[6]);
            TempCoors += Multiply3x3(pVehicle->GetMatrix(), pVehicle->m_vecCentreOfMass);
            TempVec = VelocityVector;
            TempVec.Normalise();
            VelocityVector *= 1.0f / (CrossProduct(TempCoors - Multiply3x3(pVehicle->GetMatrix(), pVehicle->m_vecCentreOfMass), TempVec).SquaredMagnitude() /
                                            pVehicle->m_fTurnMass +
                                        1.0f / pVehicle->m_fMass);
            pVehicle->ApplyForce(VelocityVector, TempCoors, true);
            return OR_CONTINUE;
        case COMMAND_IS_INT_LVAR_EQUAL_TO_INT_VAR:
            pLocalVar = GetPointerToScriptVariable(1);
            pGlobalVar = GetPointerToScriptVariable(2);
            UpdateCompareFlag(*pLocalVar == *pGlobalVar);
            return OR_CONTINUE;
        case COMMAND_IS_FLOAT_LVAR_EQUAL_TO_FLOAT_VAR:
            pLocalVar = GetPointerToScriptVariable(1);
            pGlobalVar = GetPointerToScriptVariable(2);
            LatestCmpFlagResult = *reinterpret_cast<f32*>(pLocalVar) == *reinterpret_cast<f32*>(pGlobalVar);
            UpdateCompareFlag(LatestCmpFlagResult);
            return OR_CONTINUE;
        case COMMAND_ADD_TO_CAR_ROTATION_VELOCITY:
            CollectParameters(4);
            pVehicle = CPools::GetVehiclePool().GetAt(ScriptParams[0]);
            TempVec = pVehicle->GetTurnSpeed();
            VelocityVector = CVector(*reinterpret_cast<f32*>(&ScriptParams[1]) * 0.02f, *reinterpret_cast<f32*>(&ScriptParams[2]) * 0.02f,
                *reinterpret_cast<f32*>(&ScriptParams[3]) * 0.02f);
            VelocityVector = Multiply3x3(pVehicle->GetMatrix(), VelocityVector);
            TempVec.x += VelocityVector.x;
            TempVec.y += VelocityVector.y;
            TempVec.z += VelocityVector.z;
            if (pVehicle->m_bIsStatic)
            {
                pVehicle->SetIsStatic(false);
                pVehicle->AddToMovingList();
            }
            pVehicle->SetTurnSpeed(TempVec);
            return OR_CONTINUE;
        case COMMAND_SET_CAR_ROTATION_VELOCITY:
            CollectParameters(4);
            pVehicle = CPools::GetVehiclePool().GetAt(ScriptParams[0]);
            VelocityVector = CVector(CTimer::GetTimeStep() * *reinterpret_cast<f32*>(&ScriptParams[1]),
                CTimer::GetTimeStep() * *reinterpret_cast<f32*>(&ScriptParams[2]), CTimer::GetTimeStep() * *reinterpret_cast<f32*>(&ScriptParams[3]));
            VelocityVector = Multiply3x3(pVehicle->GetMatrix(), VelocityVector);
            if (pVehicle->m_bIsStatic)
            {
                pVehicle->SetIsStatic(false);
                pVehicle->AddToMovingList();
            }
            pVehicle->SetTurnSpeed(VelocityVector.x * 0.02f, VelocityVector.y * 0.02f, VelocityVector.z * 0.02f);
            return OR_CONTINUE;
        case COMMAND_GET_CAR_ROTATION_VELOCITY:
            return OR_CONTINUE;
        case COMMAND_SET_CHAR_SHOOT_RATE:
            CollectParameters(2);
            CPools::GetPedPool().GetAt(ScriptParams[0])->m_nWeaponShootingRate = static_cast<u8>(ScriptParams[1]);
            return OR_CONTINUE;
        case COMMAND_IS_MODEL_IN_CDIMAGE:
            CollectParameters(1);
            ModelIndex = ScriptParams[0];
            if (ModelIndex < 0)
            {
                ArrayIndex = -ModelIndex;
                ModelIndex = CTheScripts::UsedObjectArray[ArrayIndex].Index;
            }
            LatestCmpFlagResult = false;
            if (CStreaming::IsObjectInCdImage(ModelIndex))
            {
                LatestCmpFlagResult = true;
            }
            UpdateCompareFlag(LatestCmpFlagResult);
            return OR_CONTINUE;
        case COMMAND_REMOVE_OIL_PUDDLES_IN_AREA:
            CollectParameters(4);
            MinX = *reinterpret_cast<f32*>(&ScriptParams[0]);
            MinY = *reinterpret_cast<f32*>(&ScriptParams[1]);
            MaxX = *reinterpret_cast<f32*>(&ScriptParams[2]);
            MaxY = *reinterpret_cast<f32*>(&ScriptParams[3]);
            if (MinX > MaxX)
            {
                temp_float = MinX;
                MinX = MaxX;
                MaxX = temp_float;
            }
            if (MinY > MaxY)
            {
                temp_float = MinY;
                MinY = MaxY;
                MaxY = temp_float;
            }
            CShadows::RemoveOilInArea(MinX, MaxX, MinY, MaxY);
            return OR_CONTINUE;
        case COMMAND_SET_BLIP_AS_FRIENDLY:
            CollectParameters(2);
            CRadar::SetBlipFriendly(ScriptParams[0], static_cast<u8>(ScriptParams[1]));
            return OR_CONTINUE;
        case COMMAND_TASK_SWIM_TO_COORD:
        {
            CollectParameters(4);
            const i32 iPedID = ScriptParams[0];
            CVector vTarget(*reinterpret_cast<f32*>(&ScriptParams[1]), *reinterpret_cast<f32*>(&ScriptParams[2]),
                *reinterpret_cast<f32*>(&ScriptParams[3]));
            if (iPedID != -1 && (pPed = CPools::GetPedPool().GetAt(iPedID)) != nullptr && pPed->GetPedIntelligence()->GetTaskSwim())
            {
                pPed->GetPedIntelligence()->GetTaskSwim()->m_vecPos = vTarget;
            }
            else
            {
                GivePedScriptedTask(iPedID, new CTaskSimpleSwim(&vTarget, nullptr), CurrCommand);
            }
            return OR_CONTINUE;
        }
        case COMMAND_TASK_GO_STRAIGHT_TO_COORD_WITHOUT_STOPPING:
            return OR_CONTINUE;
        case COMMAND_GET_BEAT_INFO_FOR_CURRENT_TRACK:
            return OR_CONTINUE;
        case COMMAND_GET_MODEL_DIMENSIONS:
        {
            CollectParameters(1);
            ModelIndex = ScriptParams[0];
            if (ModelIndex < 0)
            {
                ArrayIndex = -ModelIndex;
                ModelIndex = CTheScripts::UsedObjectArray[ArrayIndex].Index;
            }
            CColModel& ColMod = *CModelInfo::GetModelInfo(ModelIndex)->GetColModel();
            *reinterpret_cast<f32*>(&ScriptParams[0]) = ColMod.GetBoundBox().m_vecMin.x;
            *reinterpret_cast<f32*>(&ScriptParams[1]) = ColMod.GetBoundBox().m_vecMin.y;
            *reinterpret_cast<f32*>(&ScriptParams[2]) = ColMod.GetBoundBox().m_vecMin.z;
            *reinterpret_cast<f32*>(&ScriptParams[3]) = ColMod.GetBoundBox().m_vecMax.x;
            *reinterpret_cast<f32*>(&ScriptParams[4]) = ColMod.GetBoundBox().m_vecMax.y;
            *reinterpret_cast<f32*>(&ScriptParams[5]) = ColMod.GetBoundBox().m_vecMax.z;
            StoreParameters(6);
            return OR_CONTINUE;
        }
        case COMMAND_COPY_CHAR_DECISION_MAKER:
        {
            CollectParameters(1);
            i32 iDecisionMakerIndex = -1;
            if (ScriptParams[0] != -1)
            {
                iDecisionMakerIndex = CTheScripts::GetActualScriptThingIndex(ScriptParams[0], UNIQUE_SCRIPT_DECISION_MAKER);
            }
            CTheScripts::GetActualScriptThingIndex(CollectNextParameterWithoutIncreasingPC(), UNIQUE_SCRIPT_DECISION_MAKER);
            {
                const i32 iCopy = CDecisionMakerTypes::GetInstance()->CopyDecisionMaker(iDecisionMakerIndex,
                    0, IsThisAMissionScript);
                ScriptParams[0] = CTheScripts::GetNewUniqueScriptThingIndex(iCopy, UNIQUE_SCRIPT_DECISION_MAKER);
                StoreParameters(1);
                if (IsThisAMissionScript)
                {
                    CTheScripts::MissionCleanUp.AddEntityToList(ScriptParams[0], CLEANUP_DECISION_MAKER);
                }
            }
            return OR_CONTINUE;
        }
        case COMMAND_COPY_GROUP_DECISION_MAKER:
        {
            CollectParameters(1);
            i32 iDecisionMakerIndex = -1;
            if (ScriptParams[0] != -1)
            {
                iDecisionMakerIndex = CTheScripts::GetActualScriptThingIndex(ScriptParams[0], UNIQUE_SCRIPT_DECISION_MAKER);
            }
            CTheScripts::GetActualScriptThingIndex(CollectNextParameterWithoutIncreasingPC(), UNIQUE_SCRIPT_DECISION_MAKER);
            {
                const i32 iCopy = CDecisionMakerTypes::GetInstance()->CopyDecisionMaker(iDecisionMakerIndex,
                    1, IsThisAMissionScript);
                ScriptParams[0] = CTheScripts::GetNewUniqueScriptThingIndex(iCopy, UNIQUE_SCRIPT_DECISION_MAKER);
                StoreParameters(1);
                if (IsThisAMissionScript)
                {
                    CTheScripts::MissionCleanUp.AddEntityToList(ScriptParams[0], CLEANUP_DECISION_MAKER);
                }
            }
            return OR_CONTINUE;
        }
        case COMMAND_TASK_DRIVE_POINT_ROUTE_ADVANCED:
        {
            CollectParameters(6);
            const i32 iPedID = ScriptParams[0];
            const i32 iVehicleID = ScriptParams[1];
            CVehicle* pVehicle = nullptr;
            if (iVehicleID >= 0)
            {
                pVehicle = CPools::GetVehiclePool().GetAt(iVehicleID);
            }
            const f32 fCruiseSpeed = *reinterpret_cast<f32*>(&ScriptParams[2]);
            const i32 iMode = ScriptParams[3];
            i32 iDesiredCarModel = ScriptParams[4];
            if (iDesiredCarModel == 0)
            {
                iDesiredCarModel = -1;
            }
            else if (iDesiredCarModel == 1)
            {
                iDesiredCarModel = 415;
            }
            const i32 iDrivingStyle = ScriptParams[5];
            CTask* pTask = new CTaskComplexDrivePointRoute(pVehicle, CTaskComplexFollowPointRoute::ms_pointRoute, fCruiseSpeed, iMode, static_cast<eModelID>(iDesiredCarModel), -1.0f, static_cast<eCarDrivingStyle>(iDrivingStyle));
            GivePedScriptedTask(iPedID, pTask, CurrCommand);
            return OR_CONTINUE;
        }
        case COMMAND_IS_RELATIONSHIP_SET:
        {
            CollectParameters(3);
            const i32 iRelationshipType = ScriptParams[0];
            const i32 iPedType = ScriptParams[1];
            const i32 iAcquaintancePedType = ScriptParams[2];
            const u32 nFlags = CPedType::GetPedTypeAcquaintances(static_cast<AcquaintanceId>(iRelationshipType), static_cast<ePedType>(iPedType));
            LatestCmpFlagResult = (CPedType::GetPedFlag(iAcquaintancePedType) & nFlags) != 0;
            UpdateCompareFlag(LatestCmpFlagResult);
            return OR_CONTINUE;
        }
        case COMMAND_HAS_CHAR_SPOTTED_CAR:
            return OR_CONTINUE;
        case COMMAND_SET_ROPE_HEIGHT_FOR_HELI:
            return OR_CONTINUE;
        case COMMAND_GET_ROPE_HEIGHT_FOR_HELI:
            return OR_CONTINUE;
        case COMMAND_IS_CAR_LOWRIDER:
            return OR_CONTINUE;
        case COMMAND_IS_PERFORMANCE_CAR:
            return OR_CONTINUE;
        case COMMAND_SET_CAR_ALWAYS_CREATE_SKIDS:
            CollectParameters(2);
            pVehicle = CPools::GetVehiclePool().GetAt(ScriptParams[0]);
            if (ScriptParams[1])
            {
                pVehicle->vehicleFlags.bAlwaysSkidMarks = true;
            }
            else
            {
                pVehicle->vehicleFlags.bAlwaysSkidMarks = false;
            }
            return OR_CONTINUE;
        case COMMAND_GET_CITY_FROM_COORDS:
            CollectParameters(3);
            TempCoors.x = *reinterpret_cast<f32*>(&ScriptParams[0]);
            TempCoors.y = *reinterpret_cast<f32*>(&ScriptParams[1]);
            TempCoors.z = *reinterpret_cast<f32*>(&ScriptParams[2]);
            ScriptParams[0] = CTheZones::GetLevelFromPosition(TempCoors);
            StoreParameters(1);
            return OR_CONTINUE;
        case COMMAND_HAS_OBJECT_OF_TYPE_BEEN_SMASHED:
        {
            CollectParameters(5);
            NewX = *reinterpret_cast<f32*>(&ScriptParams[0]);
            NewY = *reinterpret_cast<f32*>(&ScriptParams[1]);
            NewZ = *reinterpret_cast<f32*>(&ScriptParams[2]);
            if (NewZ <= -100.0f)
            {
                NewZ = CWorld::FindGroundZForCoord(NewX, NewY);
            }
            const CVector tempVec(NewX, NewY, NewZ);
            fRadius = *reinterpret_cast<f32*>(&ScriptParams[3]);
            ModelIndex = ScriptParams[4];
            if (ModelIndex < 0)
            {
                ArrayIndex = -ModelIndex;
                ModelIndex = CTheScripts::UsedObjectArray[ArrayIndex].Index;
            }
            auto ObjPool = CPools::GetObjectPool();
            i32 i = ObjPool.GetSize();
            LatestCmpFlagResult = false;
            while (i-- && !LatestCmpFlagResult)
            {
                pObj = ObjPool.GetSlot(i);
                if (pObj && pObj->objectFlags.bIsBroken && pObj->GetModelIndex() == ModelIndex)
                {
                    if ((tempVec - pObj->GetPosition()).Magnitude() <= fRadius)
                    {
                        LatestCmpFlagResult = true;
                    }
                }
            }
            UpdateCompareFlag(LatestCmpFlagResult);
            return OR_CONTINUE;
        }
        case COMMAND_IS_PLAYER_PERFORMING_WHEELIE:
            CollectParameters(1);
            pPlayer = &CWorld::Players[ScriptParams[0]];
            LatestCmpFlagResult = false;
            if (pPlayer->m_pPed->bInVehicle && pPlayer->m_pPed->m_pMyVehicle->GetVehicleAppearance() == APR_BIKE)
            {
                LatestCmpFlagResult = pPlayer->m_nBikeRearWheelCounter != 0;
            }
            UpdateCompareFlag(LatestCmpFlagResult);
            return OR_CONTINUE;
        case COMMAND_IS_PLAYER_PERFORMING_STOPPIE:
            CollectParameters(1);
            pPlayer = &CWorld::Players[ScriptParams[0]];
            LatestCmpFlagResult = false;
            if (pPlayer->m_pPed->bInVehicle && pPlayer->m_pPed->m_pMyVehicle->GetVehicleAppearance() == APR_BIKE &&
                pPlayer->m_nBikeFrontWheelCounter)
            {
                LatestCmpFlagResult = true;
            }
            UpdateCompareFlag(LatestCmpFlagResult);
            return OR_CONTINUE;
        case COMMAND_SET_CHECKPOINT_COORDS:
            CollectParameters(4);
            TempCoors = CVector(*reinterpret_cast<f32*>(&ScriptParams[1]), *reinterpret_cast<f32*>(&ScriptParams[2]),
                *reinterpret_cast<f32*>(&ScriptParams[3]));
            ActualIndex = CTheScripts::GetActualScriptThingIndex(ScriptParams[0], UNIQUE_SCRIPT_CHECKPOINT);
            if (ActualIndex >= 0 && CTheScripts::ScriptCheckpointArray[ActualIndex].m_Checkpoint)
            {
                CCheckpoints::UpdatePos(CTheScripts::ScriptCheckpointArray[ActualIndex].m_Checkpoint->m_ID, TempCoors);
            }
            return OR_CONTINUE;
        case COMMAND_SET_ONSCREEN_TIMER_DISPLAY:
            return OR_CONTINUE;
        case COMMAND_CONTROL_CAR_HYDRAULICS:
            CollectParameters(5);
            pVehicle = CPools::GetVehiclePool().GetAt(ScriptParams[0]);
            if (pVehicle->m_vehicleSpecialColIndex < 0)
            {
                pVehicle->GetSpecialColModel();
            }
            if (pVehicle->m_vehicleSpecialColIndex > -1)
            {
                CVehicle::m_aSpecialHydraulicData[pVehicle->m_vehicleSpecialColIndex].m_aWheelSuspension[0] = *reinterpret_cast<f32*>(&ScriptParams[1]);
                CVehicle::m_aSpecialHydraulicData[pVehicle->m_vehicleSpecialColIndex].m_aWheelSuspension[1] = *reinterpret_cast<f32*>(&ScriptParams[2]);
                CVehicle::m_aSpecialHydraulicData[pVehicle->m_vehicleSpecialColIndex].m_aWheelSuspension[2] = *reinterpret_cast<f32*>(&ScriptParams[3]);
                CVehicle::m_aSpecialHydraulicData[pVehicle->m_vehicleSpecialColIndex].m_aWheelSuspension[3] = *reinterpret_cast<f32*>(&ScriptParams[4]);
            }
            return OR_CONTINUE;
        case COMMAND_GET_GROUP_SIZE:
        {
            CollectParameters(1);
            const i32 iGroupID = CTheScripts::GetActualScriptThingIndex(ScriptParams[0], UNIQUE_SCRIPT_PEDGROUP);
            if (iGroupID >= 0 && iGroupID < 8)
            {
                CPedGroupMembership* pMembership = CPedGroups::ms_groups[iGroupID].GetGroupMembership();
                const i32 iHasLeader = pMembership->GetLeader() != nullptr;
                const i32 iNoOfFollowers = pMembership->CountMembersExcludingLeader();
                ScriptParams[0] = iHasLeader;
                ScriptParams[1] = iNoOfFollowers;
            }
            else
            {
                ScriptParams[0] = 0;
                ScriptParams[1] = 0;
            }
            StoreParameters(2);
            return OR_CONTINUE;
        }
        case COMMAND_SET_OBJECT_COLLISION_DAMAGE_EFFECT:
            CollectParameters(2);
            pObj = CPools::GetObjectPool().GetAt(ScriptParams[0]);
            if (ScriptParams[1])
            {
                if (pObj->m_nSpecialColResponseCase)
                {
                    pObj->m_nColDamageEffect = pObj->m_nSpecialColResponseCase;
                    pObj->m_nSpecialColResponseCase = 0;
                }
            }
            else
            {
                if (pObj->m_nColDamageEffect)
                {
                    pObj->m_nSpecialColResponseCase = pObj->m_nColDamageEffect;
                    pObj->m_nColDamageEffect = 0;
                }
            }
            return OR_CONTINUE;
        case COMMAND_SET_CAR_FOLLOW_CAR:
            CollectParameters(3);
            CCarAI::TellCarToFollowOtherCar(CPools::GetVehiclePool().GetAt(ScriptParams[0]), CPools::GetVehiclePool().GetAt(ScriptParams[1]),
                *reinterpret_cast<f32*>(&ScriptParams[2]));
            return OR_CONTINUE;
        case COMMAND_PLAYER_ENTERED_QUARRY_CRANE:
            CRopes::PlayerControlsCrane = static_cast<eControlledCrane>(4);
            return OR_CONTINUE;
        case COMMAND_PLAYER_ENTERED_LAS_VEGAS_CRANE:
            CRopes::PlayerControlsCrane = static_cast<eControlledCrane>(3);
            CWaterLevel::m_bWaterFogScript = false;
            return OR_CONTINUE;
        case COMMAND_SWITCH_ENTRY_EXIT:
            ReadTextLabelFromScript(TextLabel, 8);
            CollectParameters(1);
            if (ScriptParams[0])
            {
                CEntryExitManager::SetEnabledByName(TextLabel, true);
            }
            else
            {
                CEntryExitManager::SetEnabledByName(TextLabel, false);
            }
            return OR_CONTINUE;
        case COMMAND_DISPLAY_TEXT_WITH_FLOAT:
        {
            CollectParameters(2);
            ReadTextLabelFromScript(TextLabel, 8);
            pString = TheText.Get(TextLabel);
            f32 x = *reinterpret_cast<f32*>(&ScriptParams[0]);
            f32 y = *reinterpret_cast<f32*>(&ScriptParams[1]);
            CTheScripts::IntroTextLines[CTheScripts::NumberOfIntroTextLinesThisFrame].ScriptTextAtX = x;
            CTheScripts::IntroTextLines[CTheScripts::NumberOfIntroTextLinesThisFrame].ScriptTextAtY = y;
            CollectParameters(2);
            temp_float = *reinterpret_cast<f32*>(&ScriptParams[0]);
            fraction = static_cast<f32>(ScriptParams[1]);
            temp_int = static_cast<i32>(temp_float);
            strncpy(CTheScripts::IntroTextLines[CTheScripts::NumberOfIntroTextLinesThisFrame].ScriptTextLabel, TextLabel, 8);
            CTheScripts::IntroTextLines[CTheScripts::NumberOfIntroTextLinesThisFrame].NumberToInsert1 = temp_int;
            CTheScripts::IntroTextLines[CTheScripts::NumberOfIntroTextLinesThisFrame].NumberToInsert2 =
                static_cast<i32>(std::pow(10.0f, fraction) * (temp_float - temp_int));
            CTheScripts::NumberOfIntroTextLinesThisFrame++;
            return OR_CONTINUE;
        }
        case COMMAND_DOES_GROUP_EXIST:
        {
            CollectParameters(1);
            const i32 iGroupID = CTheScripts::GetActualScriptThingIndex(ScriptParams[0], UNIQUE_SCRIPT_PEDGROUP);
            LatestCmpFlagResult = (iGroupID >= 0 && iGroupID < 8);
            UpdateCompareFlag(LatestCmpFlagResult);
            return OR_CONTINUE;
        }
        case COMMAND_GIVE_MELEE_ATTACK_TO_CHAR:
            CollectParameters(3);
            pPed = CPools::GetPedPool().GetAt(ScriptParams[0]);
            if (pPed->m_nFightingStyle != ScriptParams[1])
            {
                pPed->m_nFightingStyle = static_cast<eFightingStyle>(ScriptParams[1]);
                pPed->m_nAllowedAttackMoves = 0;
            }
            switch (ScriptParams[2])
            {
                case 1:
                    pPed->m_nAllowedAttackMoves = 0;
                    break;
                case 2:
                    pPed->m_nAllowedAttackMoves |= 1;
                    break;
                case 3:
                    pPed->m_nAllowedAttackMoves |= 2;
                    break;
                case 4:
                    pPed->m_nAllowedAttackMoves |= 4;
                    break;
                case 5:
                    pPed->m_nAllowedAttackMoves |= 8;
                    break;
                case 6:
                    pPed->m_nAllowedAttackMoves |= 0xF;
                    break;
            }
            return OR_CONTINUE;
        case COMMAND_SET_CAR_HYDRAULICS:
            CollectParameters(2);
            pVehicle = CPools::GetVehiclePool().GetAt(ScriptParams[0]);
            if (ScriptParams[1])
            {
                pVehicle->AddVehicleUpgrade(MI_HYDRAULICS);
            }
            else
            {
                pVehicle->RemoveVehicleUpgrade(MI_HYDRAULICS);
            }
            return OR_CONTINUE;
        case COMMAND_IS_2PLAYER_GAME_GOING_ON:
            LatestCmpFlagResult = false;
            if (CGameLogic::IsCoopGameGoingOn())
            {
                LatestCmpFlagResult = true;
            }
            UpdateCompareFlag(LatestCmpFlagResult);
            return OR_CONTINUE;
        case COMMAND_GET_CAMERA_FOV:
            *reinterpret_cast<f32*>(&ScriptParams[0]) = TheCamera.Cams[TheCamera.ActiveCam].m_fFOV;
            StoreParameters(1);
            return OR_CONTINUE;
        case COMMAND_DOES_CAR_HAVE_HYDRAULICS:
            CollectParameters(1);
            pVehicle = CPools::GetVehiclePool().GetAt(ScriptParams[0]);
            LatestCmpFlagResult = false;
            if (pVehicle->GetVehicleType() == VEHICLE_TYPE_CAR && pVehicle->handlingFlags.bHydraulicInst)
            {
                LatestCmpFlagResult = true;
            }
            UpdateCompareFlag(LatestCmpFlagResult);
            return OR_CONTINUE;
        case COMMAND_TASK_CHAR_SLIDE_TO_COORD_AND_PLAY_ANIM:
        {
            CollectParameters(6);
            const i32 iPedID = ScriptParams[0];
            const f32 x = *reinterpret_cast<f32*>(&ScriptParams[1]);
            const f32 y = *reinterpret_cast<f32*>(&ScriptParams[2]);
            const f32 z = *reinterpret_cast<f32*>(&ScriptParams[3]);
            const f32 heading = DEGTORAD(*reinterpret_cast<f32*>(&ScriptParams[4]));
            f32 speed = *reinterpret_cast<f32*>(&ScriptParams[5]);
            if (speed < 0.0f)
            {
                speed = 0.1f;
            }
            ReadTextLabelFromScript(AnimName, 24);
            ReadTextLabelFromScript(AnimGroupName, 16);
            CollectParameters(6);
            const f32 fBlendDelta = *reinterpret_cast<f32*>(&ScriptParams[0]);
            const i32 iTime = ScriptParams[5];
            i32 flags = 0x10;
            if (ScriptParams[1] || iTime > 0)
            {
                flags = 0x12;
            }
            if (ScriptParams[2])
            {
                flags |= 0x40;
            }
            if (ScriptParams[3])
            {
                flags |= 0x80;
            }
            if (!ScriptParams[4])
            {
                flags |= 0x8;
            }
            bool bRunInSequence = CTaskSequences::ms_iActiveSequence >= 0;
            CVector v(x, y, z);
            CTaskComplexSequence* pSequence = new CTaskComplexSequence();
            pSequence->AddTask(new CTaskComplexGoToPointAndStandStill(static_cast<eMoveState>(PEDMOVE_WALK), v, 0.5f, 2.0f, false, false));
            if (iTime > 0)
            {
                pSequence->AddTask(new CTaskSimpleSlideToCoord(v, heading, speed, AnimName, AnimGroupName, flags, fBlendDelta, bRunInSequence, iTime));
            }
            else
            {
                pSequence->AddTask(new CTaskSimpleSlideToCoord(v, heading, speed, AnimName, AnimGroupName, flags, fBlendDelta, bRunInSequence, -1));
            }
            GivePedScriptedTask(iPedID, pSequence, CurrCommand);
            return OR_CONTINUE;
        }
        case COMMAND_ALLOCATE_SCRIPT_TO_OBJECT:
            return OR_CONTINUE;
        case COMMAND_GET_TOTAL_NUMBER_OF_PEDS_KILLED_BY_PLAYER:
            CollectParameters(1);
            TotalKills = CDarkel::FindTotalPedsKilledByPlayer(ScriptParams[0]);
            ScriptParams[0] = TotalKills;
            StoreParameters(1);
            return OR_CONTINUE;
        case COMMAND_SET_TWO_PLAYER_CAM_MODE_SAME_CAR_SHOOTING:
            return OR_CONTINUE;
        case COMMAND_SET_TWO_PLAYER_CAM_MODE_SAME_CAR_NO_SHOOTING:
            return OR_CONTINUE;
        case COMMAND_SET_TWO_PLAYER_CAM_MODE_NOT_BOTH_IN_CAR:
            return OR_CONTINUE;
        case COMMAND_GET_LEVEL_DESIGN_COORDS_FOR_OBJECT:
            CollectParameters(2);
            pObj = CPools::GetObjectPool().GetAt(ScriptParams[0]);
            pObj->FindTriggerPointCoors(&TempCoors, ScriptParams[1]);
            *reinterpret_cast<f32*>(&ScriptParams[0]) = TempCoors.x;
            *reinterpret_cast<f32*>(&ScriptParams[1]) = TempCoors.y;
            *reinterpret_cast<f32*>(&ScriptParams[2]) = TempCoors.z;
            StoreParameters(3);
            return OR_CONTINUE;
        case COMMAND_SAVE_TEXT_LABEL_TO_DEBUG_FILE:
            ReadTextLabelFromScript(LongTextLabel, 16);
            return OR_CONTINUE;
        case COMMAND_GET_CHAR_BREATH:
            return OR_CONTINUE;
        case COMMAND_SET_CHAR_BREATH:
            return OR_CONTINUE;
        case COMMAND_GET_CHAR_HIGHEST_PRIORITY_EVENT:
        {
            CollectParameters(1);
            pPed = CPools::GetPedPool().GetAt(ScriptParams[0]);
            const i32 iEventType = pPed->GetPedIntelligence()->m_nEventId;
            ScriptParams[0] = iEventType;
            StoreParameters(1);
            return OR_CONTINUE;
        }
        case COMMAND_ARE_PATHS_LOADED_FOR_CAR:
            return OR_CONTINUE;
        case COMMAND_GET_PARKING_NODE_IN_AREA:
            CollectParameters(6);
            MinX = *reinterpret_cast<f32*>(&ScriptParams[0]);
            MinY = *reinterpret_cast<f32*>(&ScriptParams[1]);
            MinZ = *reinterpret_cast<f32*>(&ScriptParams[2]);
            MaxX = *reinterpret_cast<f32*>(&ScriptParams[3]);
            MaxY = *reinterpret_cast<f32*>(&ScriptParams[4]);
            MaxZ = *reinterpret_cast<f32*>(&ScriptParams[5]);
            if (MinX > MaxX)
            {
                temp_float = MinX;
                MinX = MaxX;
                MaxX = temp_float;
            }
            if (MinY > MaxY)
            {
                temp_float = MinY;
                MinY = MaxY;
                MaxY = temp_float;
            }
            if (MinZ > MaxZ)
            {
                temp_float = MinZ;
                MinZ = MaxZ;
                MaxZ = temp_float;
            }
            TempCoors = ThePaths.FindParkingNodeInArea(MinX, MaxX, MinY, MaxY, MinZ, MaxZ);
            *reinterpret_cast<f32*>(&ScriptParams[0]) = TempCoors.x;
            *reinterpret_cast<f32*>(&ScriptParams[1]) = TempCoors.y;
            *reinterpret_cast<f32*>(&ScriptParams[2]) = TempCoors.z;
            StoreParameters(3);
            return OR_CONTINUE;
        case COMMAND_GET_CAR_CHAR_IS_USING:
            CollectParameters(1);
            pPed = CPools::GetPedPool().GetAt(ScriptParams[0]);
            pVehicle = nullptr;
            if (pPed->bInVehicle)
            {
                pVehicle = pPed->m_pMyVehicle;
            }
            else
            {
                pTask = pPed->GetPedIntelligence()->FindTaskActiveByType(CTaskTypes::TASK_COMPLEX_ENTER_CAR_AS_DRIVER);
                if (!pTask)
                {
                    pTask = pPed->GetPedIntelligence()->FindTaskActiveByType(CTaskTypes::TASK_COMPLEX_ENTER_CAR_AS_PASSENGER);
                }
                if (pTask)
                {
                    pVehicle = static_cast<CTaskComplexEnterCar*>(pTask)->GetTargetVehicle();
                }
            }
            if (pVehicle)
            {
                ScriptParams[0] = CPools::GetVehiclePool().GetIndex(pVehicle);
            }
            else
            {
                ScriptParams[0] = -1;
            }
            StoreParameters(1);
            return OR_CONTINUE;
        case COMMAND_TASK_PLAY_ANIM_NON_INTERRUPTABLE:
            PlayAnimScriptCommand(CurrCommand);
            return OR_CONTINUE;
        case COMMAND_FORCE_NEXT_DIE_ANIM:
            return OR_CONTINUE;
        case COMMAND_ADD_STUNT_JUMP:
        {
            CollectParameters(16);
            CVector vStart(*reinterpret_cast<f32*>(&ScriptParams[0]), *reinterpret_cast<f32*>(&ScriptParams[1]), *reinterpret_cast<f32*>(&ScriptParams[2]));
            CVector vStartRadius(*reinterpret_cast<f32*>(&ScriptParams[3]), *reinterpret_cast<f32*>(&ScriptParams[4]), *reinterpret_cast<f32*>(&ScriptParams[5]));
            CBoundingBox bbStartBox(vStart - vStartRadius, vStart + vStartRadius);
            CVector vEnd(*reinterpret_cast<f32*>(&ScriptParams[6]), *reinterpret_cast<f32*>(&ScriptParams[7]), *reinterpret_cast<f32*>(&ScriptParams[8]));
            CVector vEndRadius(*reinterpret_cast<f32*>(&ScriptParams[9]), *reinterpret_cast<f32*>(&ScriptParams[10]), *reinterpret_cast<f32*>(&ScriptParams[11]));
            CBoundingBox bbEndBox(vEnd - vEndRadius, vEnd + vEndRadius);
            CStuntJumpManager::AddOne(bbStartBox, bbEndBox, CVector(*reinterpret_cast<f32*>(&ScriptParams[12]), *reinterpret_cast<f32*>(&ScriptParams[13]), *reinterpret_cast<f32*>(&ScriptParams[14])), ScriptParams[15]);
            return OR_CONTINUE;
        }
        case COMMAND_SET_OBJECT_COORDINATES_AND_VELOCITY:
            CollectParameters(4);
            pObj = CPools::GetObjectPool().GetAt(ScriptParams[0]);
            NewX = *reinterpret_cast<f32*>(&ScriptParams[1]);
            NewY = *reinterpret_cast<f32*>(&ScriptParams[2]);
            NewZ = *reinterpret_cast<f32*>(&ScriptParams[3]);
            if (NewZ <= -100.0f)
            {
                NewZ = CWorld::FindGroundZForCoord(NewX, NewY);
            }
            pObj->SetMoveSpeed((CVector(NewX, NewY, NewZ) - pObj->GetPosition()) / CTimer::GetTimeStep());
            if (pObj->m_vecMoveSpeed.x < -1.0f)
            {
                pObj->m_vecMoveSpeed.x = -1.0f;
            }
            if (pObj->m_vecMoveSpeed.x > 1.0f)
            {
                pObj->m_vecMoveSpeed.x = 1.0f;
            }
            if (pObj->m_vecMoveSpeed.y < -1.0f)
            {
                pObj->m_vecMoveSpeed.y = -1.0f;
            }
            if (pObj->m_vecMoveSpeed.y > 1.0f)
            {
                pObj->m_vecMoveSpeed.y = 1.0f;
            }
            if (pObj->m_vecMoveSpeed.z < -1.0f)
            {
                pObj->m_vecMoveSpeed.z = -1.0f;
            }
            if (pObj->m_vecMoveSpeed.z > 1.0f)
            {
                pObj->m_vecMoveSpeed.z = 1.0f;
            }
            MarkSurroundingEntitiesForCollisionWithTrain(CVector(NewX, NewY, NewZ), 25.0f, pObj, false);
            pObj->Teleport(CVector(NewX, NewY, NewZ), false);
            CTheScripts::ClearSpaceForMissionEntity(CVector(NewX, NewY, NewZ), pObj);
            return OR_CONTINUE;
        case COMMAND_SET_CHAR_KINDA_STAY_IN_SAME_PLACE:
            CollectParameters(2);
            pPed = CPools::GetPedPool().GetAt(ScriptParams[0]);
            if (ScriptParams[1])
            {
                pPed->bKindaStayInSamePlace = true;
            }
            else
            {
                pPed->bKindaStayInSamePlace = false;
            }
            return OR_CONTINUE;
        case COMMAND_TASK_FOLLOW_PATROL_ROUTE:
        {
            CollectParameters(3);
            const i32 iPedID = ScriptParams[0];
            const i32 iMoveState = ScriptParams[1];
            const i32 iMode = ScriptParams[2];
            GivePedScriptedTask(iPedID,
                new CTaskComplexFollowPatrolRoute(iMoveState, CTaskComplexFollowPatrolRoute::ms_patrolRoute, iMode, 0.5f, 5.0f), CurrCommand);
            return OR_CONTINUE;
        }
        case COMMAND_IS_CHAR_IN_AIR:
            CollectParameters(1);
            pPed = CPools::GetPedPool().GetAt(ScriptParams[0]);
            LatestCmpFlagResult = false;
            if (pPed->bIsInTheAir)
            {
                LatestCmpFlagResult = true;
            }
            else if (!pPed->bIsStanding)
            {
                if (pPed->bKnockedUpIntoAir || pPed->bKnockedOffBike)
                {
                    if (pPed->GetPedIntelligence()->GetTaskManager().GetSimplestActiveTask() &&
                        pPed->GetPedIntelligence()->GetTaskManager().GetSimplestActiveTask()->GetTaskType() == CTaskTypes::TASK_SIMPLE_FALL)
                    {
                        LatestCmpFlagResult = true;
                    }
                }
            }
            UpdateCompareFlag(LatestCmpFlagResult);
            return OR_CONTINUE;
        case COMMAND_GET_CHAR_HEIGHT_ABOVE_GROUND:
            CollectParameters(1);
            pPed = CPools::GetPedPool().GetAt(ScriptParams[0]);
            TempCoors = pPed->GetPosition();
            *reinterpret_cast<f32*>(&ScriptParams[0]) = TempCoors.z - CWorld::FindGroundZFor3DCoord(TempCoors.x, TempCoors.y, TempCoors.z, nullptr, nullptr);
            StoreParameters(1);
            return OR_CONTINUE;
        case COMMAND_SET_CHAR_WEAPON_SKILL:
            CollectParameters(2);
            CPools::GetPedPool().GetAt(ScriptParams[0])->m_nWeaponSkill = static_cast<eWeaponSkill>(ScriptParams[1]);
            return OR_CONTINUE;
        case COMMAND_ARE_PATHS_LOADED_IN_AREA:
            return OR_CONTINUE;
        case COMMAND_SET_TEXT_EDGE:
            CollectParameters(5);
            tempRGBA = CRGBA(static_cast<u8>(ScriptParams[1]), static_cast<u8>(ScriptParams[2]), static_cast<u8>(ScriptParams[3]),
                static_cast<u8>(ScriptParams[4]));
            CTheScripts::IntroTextLines[CTheScripts::NumberOfIntroTextLinesThisFrame].ScriptTextDropShadowColour = tempRGBA;
            CTheScripts::IntroTextLines[CTheScripts::NumberOfIntroTextLinesThisFrame].TextEdge = static_cast<i8>(ScriptParams[0]);
            return OR_CONTINUE;
        case COMMAND_SET_CAR_ENGINE_BROKEN:
            CollectParameters(2);
            pVehicle = CPools::GetVehiclePool().GetAt(ScriptParams[0]);
            if (ScriptParams[1])
            {
                pVehicle->vehicleFlags.bEngineBroken = true;
                pVehicle->vehicleFlags.bEngineOn = false;
            }
            else
            {
                pVehicle->vehicleFlags.bEngineBroken = false;
            }
            return OR_CONTINUE;
        case COMMAND_IS_THIS_MODEL_A_BOAT:
            CollectParameters(1);
            LatestCmpFlagResult = CModelInfo::IsBoatModel(ScriptParams[0]);
            UpdateCompareFlag(LatestCmpFlagResult);
            return OR_CONTINUE;
        case COMMAND_IS_THIS_MODEL_A_PLANE:
            CollectParameters(1);
            LatestCmpFlagResult = CModelInfo::IsPlaneModel(ScriptParams[0]);
            UpdateCompareFlag(LatestCmpFlagResult);
            return OR_CONTINUE;
        case COMMAND_IS_THIS_MODEL_A_HELI:
            CollectParameters(1);
            LatestCmpFlagResult = false;
            if (CModelInfo::IsHeliModel(ScriptParams[0]))
            {
                LatestCmpFlagResult = true;
            }
            UpdateCompareFlag(LatestCmpFlagResult);
            return OR_CONTINUE;
        case COMMAND_IS_3D_COORD_IN_ZONE:
            return OR_CONTINUE;
        case COMMAND_SET_FIRST_PERSON_IN_CAR_CAMERA_MODE:
            CollectParameters(1);
            if (ScriptParams[0])
            {
                TheCamera.m_bDisableFirstPersonInCar = false;
            }
            else
            {
                TheCamera.m_bDisableFirstPersonInCar = true;
                TheCamera.m_nCarZoom = 2;
            }
            return OR_CONTINUE;
        case COMMAND_TASK_GREET_PARTNER:
            CollectParameters(4);
            pPed = CPools::GetPedPool().GetAt(ScriptParams[0]);
            pSecondPed = CPools::GetPedPool().GetAt(ScriptParams[1]);
            temp_float = *reinterpret_cast<f32*>(&ScriptParams[2]);
            pTask = new CTaskComplexPartnerGreet("COMMAND_TASK_GREET_PARTNER", pSecondPed, true, temp_float, ScriptParams[3], CVector(0.0f, 0.0f, 0.0f));
            GivePedScriptedTask(ScriptParams[0], pTask, CurrCommand);
            pTask2 = new CTaskComplexPartnerGreet("COMMAND_TASK_GREET_PARTNER", pPed, false, temp_float, ScriptParams[3], CVector(0.0f, 0.0f, 0.0f));
            GivePedScriptedTask(ScriptParams[1], pTask2, CurrCommand);
            return OR_CONTINUE;
        case COMMAND_GET_CLOSEST_PICKUP_COORDS_TO_COORD:
            return OR_CONTINUE;
        case COMMAND_SET_HELI_BLADES_FULL_SPEED:
            CollectParameters(1);
            pVehicle = CPools::GetVehiclePool().GetAt(ScriptParams[0]);
            if (pVehicle->GetVehicleType() == VEHICLE_TYPE_HELI)
            {
                static_cast<CAutomobile*>(pVehicle)->m_aWheelAngularVelocity[1] = 0.22f;
            }
            else if (pVehicle->GetVehicleType() == VEHICLE_TYPE_PLANE)
            {
                static_cast<CPlane*>(pVehicle)->m_fPropSpeed = 0.18f;
            }
            return OR_CONTINUE;
        case COMMAND_DISPLAY_HUD:
            CollectParameters(1);
            CTheScripts::bDisplayHud = ScriptParams[0] != 0;
            return OR_CONTINUE;
        case COMMAND_CONNECT_LODS:
            CollectParameters(2);
            pObj = CPools::GetObjectPool().GetAt(ScriptParams[0]);
            pSecondObj = CPools::GetObjectPool().GetAt(ScriptParams[1]);
            CTheScripts::ScriptConnectLodsFunction(ScriptParams[0], ScriptParams[1]);
            CTheScripts::AddToListOfConnectedLodObjects(pObj, pSecondObj);
            return OR_CONTINUE;
        case COMMAND_SET_MAX_FIRE_GENERATIONS:
            CollectParameters(1);
            gFireManager.m_nMaxFireGenerationsAllowed = ScriptParams[0];
            return OR_CONTINUE;
        case COMMAND_TASK_DIE_NAMED_ANIM:
        {
            CollectParameters(1);
            const i32 iPedID = ScriptParams[0];
            ReadTextLabelFromScript(AnimName, 24);
            ReadTextLabelFromScript(AnimGroupName, 16);
            CollectParameters(2);
            const f32 fBlendDelta = *reinterpret_cast<f32*>(&ScriptParams[0]);
            i32 flags = 0x10;
            if (ScriptParams[1])
            {
                flags = 0xD0;
            }
            GivePedScriptedTask(iPedID, new CTaskSimpleDie(AnimName, AnimGroupName, static_cast<eAnimationFlags>(flags), fBlendDelta, 1.0f), CurrCommand);
            return OR_CONTINUE;
        }
        case COMMAND_SET_PLAYER_DUCK_BUTTON:
            CollectParameters(2);
            if (ScriptParams[1])
            {
                CPad::GetPad(ScriptParams[0])->bDisablePlayerDuck = false;
            }
            else
            {
                CPad::GetPad(ScriptParams[0])->bDisablePlayerDuck = true;
            }
            return OR_CONTINUE;
        case COMMAND_SET_POOL_TABLE_COORDS:
            CollectParameters(6);
            CWorld::SnookerTableMax = CVector(*reinterpret_cast<f32*>(&ScriptParams[3]), *reinterpret_cast<f32*>(&ScriptParams[4]),
                *reinterpret_cast<f32*>(&ScriptParams[5]));
            CWorld::SnookerTableMin = CVector(*reinterpret_cast<f32*>(&ScriptParams[0]), *reinterpret_cast<f32*>(&ScriptParams[1]),
                *reinterpret_cast<f32*>(&ScriptParams[2]));
            return OR_CONTINUE;
        case COMMAND_IS_AUDIO_BUILD:
            UpdateCompareFlag(false);
            return OR_CONTINUE;
        case COMMAND_CLEAR_QUEUED_DIALOGUE:
            return OR_CONTINUE;
        case COMMAND_HAS_OBJECT_BEEN_PHOTOGRAPHED:
            CollectParameters(1);
            pObj = CPools::GetObjectPool().GetAt(ScriptParams[0]);
            LatestCmpFlagResult = false;
            if (pObj && pObj->objectFlags.bIsPhotographed)
            {
                LatestCmpFlagResult = true;
                pObj->objectFlags.bIsPhotographed = false;
            }
            UpdateCompareFlag(LatestCmpFlagResult);
            return OR_CONTINUE;
    }
    return OR_INTERRUPT;
}

// MARK: 2100To2199

// func: sa 0x470A90
OpcodeResult CRunningScript::ProcessCommands2100To2199(i32 CurrCommand)
{
    MARKFUNCTION(0x470A90);

    i16 ZoneIndex;
    char ZoneLabel[10];
    CPed *pPed, *pSecondPed;
    CVehicle *pVehicle, *pTrailer;
    CObject* pObj;
    CPlayerInfo* pPlayer;
    f32 NewZ, NewY, NewX;
    bool LatestCmpFlagResult;
    CVector TempVec, vCarUp;
    i32* pGlobalVar;
    i32* pLocalVar;
    CTask* pTask;
    f32 FloatDamage;
    u8 MaxPossibleParametersForThisCommand;
    u32 loop;
    i32 JumpAddress;
    i32 ModelIndex;
    i32 EntryExitID;
    CEntryExit* pEntryExit;
    u16 GlobalVarIndex;
    CVector TempCoors;
    char ScriptBrainName[10];
    char AnimName[24];
    CAnimBlendAssociation* pAnim;
    char ClothesTextureName[16];
    char ClothesModelName[16];
    f32 temp_float;
    i32 FXSystemIndex, ActualIndex;

    // NOTE: Keep switch cases in strictly ascending order as in commands.hpp to preserve MSVC jump table layout
    switch (CurrCommand)
    {
        case COMMAND_DO_CAMERA_BUMP:
            CollectParameters(2);
            TheCamera.Cams[TheCamera.ActiveCam].DoCamBump(*reinterpret_cast<f32*>(&ScriptParams[0]), *reinterpret_cast<f32*>(&ScriptParams[1]));
            return OR_CONTINUE;
        case COMMAND_GET_CURRENT_DATE:
            ScriptParams[0] = CClock::ms_nGameClockDays;
            ScriptParams[1] = CClock::ms_nGameClockMonth;
            StoreParameters(2);
            return OR_CONTINUE;
        case COMMAND_SET_OBJECT_ANIM_SPEED:
            CollectParameters(1);
            pObj = CPools::GetObjectPool().GetAt(ScriptParams[0]);
            ReadTextLabelFromScript(AnimName, 24);
            pAnim = RpAnimBlendClumpGetAssociation(reinterpret_cast<RpClump*>(pObj->m_pRwObject), AnimName);
            CollectParameters(1);
            if (pAnim)
            {
                pAnim->SetSpeed(*reinterpret_cast<f32*>(&ScriptParams[0]));
            }
            return OR_CONTINUE;
        case COMMAND_IS_OBJECT_PLAYING_ANIM:
            CollectParameters(1);
            pObj = CPools::GetObjectPool().GetAt(ScriptParams[0]);
            ReadTextLabelFromScript(AnimName, 24);
            LatestCmpFlagResult = false;
            if (pObj->m_pRwObject && RwObjectGetType(pObj->m_pRwObject) == rpCLUMP &&
                RpAnimBlendClumpIsInitialized(reinterpret_cast<RpClump*>(pObj->m_pRwObject)))
            {
                if (RpAnimBlendClumpGetAssociation(reinterpret_cast<RpClump*>(pObj->m_pRwObject), AnimName))
                {
                    LatestCmpFlagResult = true;
                }
            }
            UpdateCompareFlag(LatestCmpFlagResult);
            return OR_CONTINUE;
        case COMMAND_SET_OBJECT_ANIM_PLAYING_FLAG:
            return OR_CONTINUE;
        case COMMAND_GET_OBJECT_ANIM_CURRENT_TIME:
            CollectParameters(1);
            pObj = CPools::GetObjectPool().GetAt(ScriptParams[0]);
            ReadTextLabelFromScript(AnimName, 24);
            pAnim = RpAnimBlendClumpGetAssociation(reinterpret_cast<RpClump*>(pObj->m_pRwObject), AnimName);
            temp_float = 0.0f;
            if (pAnim)
            {
                temp_float = pAnim->GetCurrentTime() / pAnim->GetTotalTime();
            }
            ScriptParams[0] = *reinterpret_cast<i32*>(&temp_float);
            StoreParameters(1);
            return OR_CONTINUE;
        case COMMAND_SET_OBJECT_ANIM_CURRENT_TIME:
            CollectParameters(1);
            pObj = CPools::GetObjectPool().GetAt(ScriptParams[0]);
            ReadTextLabelFromScript(AnimName, 24);
            pAnim = RpAnimBlendClumpGetAssociation(reinterpret_cast<RpClump*>(pObj->m_pRwObject), AnimName);
            CollectParameters(1);
            if (pAnim)
            {
                pAnim->SetCurrentTime(pAnim->GetTotalTime() * *reinterpret_cast<f32*>(&ScriptParams[0]));
            }
            return OR_CONTINUE;
        case COMMAND_GET_OBJECT_ANIM_TOTAL_TIME:
            return OR_CONTINUE;
        case COMMAND_SET_CHAR_VELOCITY:
            CollectParameters(4);
            pPed = CPools::GetPedPool().GetAt(ScriptParams[0]);
            pPed->SetMoveSpeed(*reinterpret_cast<f32*>(&ScriptParams[1]) * 0.02f, *reinterpret_cast<f32*>(&ScriptParams[2]) * 0.02f,
                *reinterpret_cast<f32*>(&ScriptParams[3]) * 0.02f);
            return OR_CONTINUE;
        case COMMAND_GET_CHAR_VELOCITY:
        {
            CollectParameters(1);
            const CVector& TempVel = CPools::GetPedPool().GetAt(ScriptParams[0])->GetMoveSpeed();
            *reinterpret_cast<f32*>(&ScriptParams[0]) = TempVel.x * 50.0f;
            *reinterpret_cast<f32*>(&ScriptParams[1]) = TempVel.y * 50.0f;
            *reinterpret_cast<f32*>(&ScriptParams[2]) = TempVel.z * 50.0f;
            StoreParameters(3);
            return OR_CONTINUE;
        }
        case COMMAND_SET_CHAR_ROTATION:
            CollectParameters(4);
            pPed = CPools::GetPedPool().GetAt(ScriptParams[0]);
            CWorld::Remove(pPed);
            pPed->SetOrientation(DEGTORAD(*reinterpret_cast<f32*>(&ScriptParams[1])), DEGTORAD(*reinterpret_cast<f32*>(&ScriptParams[2])),
                DEGTORAD(*reinterpret_cast<f32*>(&ScriptParams[3])));
            CWorld::Add(pPed);
            return OR_CONTINUE;
        case COMMAND_GET_CAR_UPRIGHT_VALUE:
            CollectParameters(1);
            pVehicle = CPools::GetVehiclePool().GetAt(ScriptParams[0]);
            vCarUp = pVehicle->GetMatrix().GetUp();
            *reinterpret_cast<f32*>(&ScriptParams[0]) = vCarUp.z;
            StoreParameters(1);
            return OR_CONTINUE;
        case COMMAND_SET_VEHICLE_AREA_VISIBLE:
            CollectParameters(2);
            pVehicle = CPools::GetVehiclePool().GetAt(ScriptParams[0]);
            pVehicle->SetAreaCode(static_cast<eAreaCodes>(ScriptParams[1]));
            return OR_CONTINUE;
        case COMMAND_SELECT_WEAPONS_FOR_VEHICLE:
            CollectParameters(2);
            pVehicle = CPools::GetVehiclePool().GetAt(ScriptParams[0]);
            pVehicle->m_nVehicleWeaponInUse = static_cast<eCarWeapon>(ScriptParams[1]);
            return OR_CONTINUE;
        case COMMAND_GET_CITY_PLAYER_IS_IN:
            CollectParameters(1);
            ScriptParams[0] = CTheZones::m_CurrLevel;
            StoreParameters(1);
            return OR_CONTINUE;
        case COMMAND_GET_NAME_OF_ZONE:
        {
            CollectParameters(3);
            TempCoors = CVector(*reinterpret_cast<f32*>(&ScriptParams[0]), *reinterpret_cast<f32*>(&ScriptParams[1]),
                *reinterpret_cast<f32*>(&ScriptParams[2]));
            CZone* pZone = CTheZones::FindSmallestZoneForPosition(TempCoors, true);
            pGlobalVar = GetPointerToScriptVariable(2);
            strncpy(reinterpret_cast<char*>(pGlobalVar), pZone->m_TextLabel, 8);
            return OR_CONTINUE;
        }
        case COMMAND_IS_VAR_TEXT_LABEL_EMPTY:
        case COMMAND_IS_VAR_TEXT_LABEL16_EMPTY:
            pGlobalVar = GetPointerToScriptVariable(2);
            LatestCmpFlagResult = false;
            if (strcmp(reinterpret_cast<char*>(pGlobalVar), "") == 0)
            {
                LatestCmpFlagResult = true;
            }
            UpdateCompareFlag(LatestCmpFlagResult);
            return OR_CONTINUE;
        case COMMAND_IS_LVAR_TEXT_LABEL_EMPTY:
        case COMMAND_IS_LVAR_TEXT_LABEL16_EMPTY:
            pLocalVar = GetPointerToScriptVariable(1);
            LatestCmpFlagResult = false;
            if (strcmp(reinterpret_cast<char*>(pLocalVar), "") == 0)
            {
                LatestCmpFlagResult = true;
            }
            UpdateCompareFlag(LatestCmpFlagResult);
            return OR_CONTINUE;
        case COMMAND_SWITCH:
            return OR_CONTINUE;
        case COMMAND_ENDSWITCH:
            return OR_CONTINUE;
        case COMMAND_CASE:
            return OR_CONTINUE;
        case COMMAND_DEFAULT:
            return OR_CONTINUE;
        case COMMAND_BREAK:
            return OR_CONTINUE;
        case COMMAND_ACTIVATE_INTERIOR_PEDS:
            CollectParameters(1);
            if (ScriptParams[0])
            {
                g_interiorMan.ActivatePeds(true);
            }
            else
            {
                g_interiorMan.ActivatePeds(false);
            }
            return OR_CONTINUE;
        case COMMAND_SET_VEHICLE_CAN_BE_TARGETTED:
            CollectParameters(2);
            pVehicle = CPools::GetVehiclePool().GetAt(ScriptParams[0]);
            if (ScriptParams[1])
            {
                pVehicle->vehicleFlags.bVehicleCanBeTargetted = true;
            }
            else
            {
                pVehicle->vehicleFlags.bVehicleCanBeTargetted = false;
            }
            return OR_CONTINUE;
        case COMMAND_GET_GROUP_LEADER:
            return OR_CONTINUE;
        case COMMAND_TASK_FOLLOW_FOOTSTEPS:
            CollectParameters(2);
            pSecondPed = CPools::GetPedPool().GetAt(ScriptParams[1]);
            pTask = new CTaskComplexFollowPedFootsteps(pSecondPed);
            GivePedScriptedTask(ScriptParams[0], pTask, CurrCommand);
            return OR_CONTINUE;
        case COMMAND_DAMAGE_CHAR:
            CollectParameters(3);
            pPed = CPools::GetPedPool().GetAt(ScriptParams[0]);
            FloatDamage = static_cast<f32>(ScriptParams[1]);
            if (ScriptParams[2])
            {
                if (FloatDamage <= pPed->m_nArmour)
                {
                    pPed->m_nArmour -= FloatDamage;
                    FloatDamage = 0.0f;
                }
                else
                {
                    FloatDamage -= pPed->m_nArmour;
                    pPed->m_nArmour = 0.0f;
                }
            }
            pPed->m_nHealth -= FloatDamage;
            if (pPed->m_nHealth <= 0.0f)
            {
                pPed->m_nHealth = 0.0f;
                {
                    CTask* pTask = new CTaskComplexDie(WEAPONTYPE_UNARMED, (AssocGroupId)0, (AnimationId)15, 4.0f, 0.0f, false, false, 0, false);
                    CEventScriptCommand event(TASK_PRIMARY_PRIMARY, pTask);
                    pPed->GetPedIntelligence()->AddEvent(event, false);
                }
            }
            return OR_CONTINUE;
        case COMMAND_SET_CAR_CAN_BE_VISIBLY_DAMAGED:
            CollectParameters(2);
            pVehicle = CPools::GetVehiclePool().GetAt(ScriptParams[0]);
            if (ScriptParams[1])
            {
                static_cast<CAutomobile*>(pVehicle)->autoFlags.bCanBeVisiblyDamaged = true;
            }
            else
            {
                static_cast<CAutomobile*>(pVehicle)->autoFlags.bCanBeVisiblyDamaged = false;
            }
            return OR_CONTINUE;
        case COMMAND_SET_HELI_REACHED_TARGET_DISTANCE:
            CollectParameters(2);
            pVehicle = CPools::GetVehiclePool().GetAt(ScriptParams[0]);
            pVehicle->AutoPilot.TargetReachedDist = static_cast<u8>(ScriptParams[1]);
            return OR_CONTINUE;
        case COMMAND_GET_SOUND_LEVEL_AT_COORDS:
            CollectParameters(4);
            if (ScriptParams[0] == -1)
            {
                pPed = nullptr;
            }
            else
            {
                pPed = CPools::GetPedPool().GetAt(ScriptParams[0]);
            }
            TempCoors = CVector(*reinterpret_cast<f32*>(&ScriptParams[1]), *reinterpret_cast<f32*>(&ScriptParams[2]),
                *reinterpret_cast<f32*>(&ScriptParams[3]));
            *reinterpret_cast<f32*>(&ScriptParams[0]) = GetEventGlobalGroup()->GetSoundLevel(pPed, TempCoors);
            StoreParameters(1);
            return OR_CONTINUE;
        case COMMAND_SET_CHAR_ALLOWED_TO_DUCK:
            CollectParameters(2);
            pPed = CPools::GetPedPool().GetAt(ScriptParams[0]);
            if (ScriptParams[1])
            {
                pPed->bNotAllowedToDuck = false;
            }
            else
            {
                pPed->bNotAllowedToDuck = true;
            }
            return OR_CONTINUE;
        case COMMAND_SET_WATER_CONFIGURATION:
            return OR_CONTINUE;
        case COMMAND_SET_HEADING_FOR_ATTACHED_PLAYER:
            CollectParameters(3);
            CTheScripts::fCameraHeadingWhenPlayerIsAttached = DEGTORAD(*reinterpret_cast<f32*>(&ScriptParams[1]));
            CTheScripts::fCameraHeadingStepWhenPlayerIsAttached = DEGTORAD(*reinterpret_cast<f32*>(&ScriptParams[2]));
            return OR_CONTINUE;
        case COMMAND_TASK_WALK_ALONGSIDE_CHAR:
            CollectParameters(2);
            pSecondPed = CPools::GetPedPool().GetAt(ScriptParams[1]);
            pTask = new CTaskComplexWalkAlongsidePed(pSecondPed, 10.0f);
            GivePedScriptedTask(ScriptParams[0], pTask, CurrCommand);
            return OR_CONTINUE;
        case COMMAND_CREATE_EMERGENCY_SERVICES_CAR:
            CollectParameters(4);
            TempCoors = CVector(*reinterpret_cast<f32*>(&ScriptParams[1]), *reinterpret_cast<f32*>(&ScriptParams[2]),
                *reinterpret_cast<f32*>(&ScriptParams[3]));
            LatestCmpFlagResult = false;
            if (CCarCtrl::ScriptGenerateOneEmergencyServicesCar(ScriptParams[0], TempCoors))
            {
                LatestCmpFlagResult = true;
            }
            UpdateCompareFlag(LatestCmpFlagResult);
            return OR_CONTINUE;
        case COMMAND_TASK_KINDA_STAY_IN_SAME_PLACE:
        {
            CollectParameters(2);
            const i32 iPedID = ScriptParams[0];
            if (iPedID != -1)
            {
                pPed = CPools::GetPedPool().GetAt(iPedID);
                CTaskSimpleSetKindaStayInSamePlace task(ScriptParams[1] != 0);
                task.ProcessPed(pPed);
            }
            else
            {
                pTask = new CTaskSimpleSetKindaStayInSamePlace(ScriptParams[1] != 0);
                CTaskSequences::ms_taskSequence[CTaskSequences::ms_iActiveSequence].AddTask(pTask);
            }
            return OR_CONTINUE;
        }
        case COMMAND_TASK_USE_ATTRACTOR_ADVANCED:
            return OR_CONTINUE;
        case COMMAND_TASK_FOLLOW_PATH_NODES_TO_COORD_SHOOTING:
            return OR_CONTINUE;
        case COMMAND_START_PLAYBACK_RECORDED_CAR_LOOPED:
            CollectParameters(2);
            pVehicle = CPools::GetVehiclePool().GetAt(ScriptParams[0]);
            CVehicleRecording::StartPlaybackRecordedCar(pVehicle, ScriptParams[1], false, true);
            return OR_CONTINUE;
        case COMMAND_START_PLAYBACK_RECORDED_CAR_USING_AI_LOOPED:
            return OR_CONTINUE;
        case COMMAND_SET_CHAR_AREA_VISIBLE:
            CollectParameters(2);
            pPed = CPools::GetPedPool().GetAt(ScriptParams[0]);
            pPed->SetAreaCode(static_cast<eAreaCodes>(ScriptParams[1]));
            if (ScriptParams[1] == 0)
            {
                pPed->m_pEnex = nullptr;
                if (pPed->IsPlayer())
                {
                    CEntryExitManager::ClearEntryExitStack();
                    CTimeCycle::StopExtraColour(false);
                }
            }
            return OR_CONTINUE;
        case COMMAND_IS_ATTACHED_PLAYER_HEADING_ACHIEVED:
            CollectParameters(1);
            LatestCmpFlagResult = false;
            if (CTheScripts::fCameraHeadingStepWhenPlayerIsAttached == 0.0f)
            {
                LatestCmpFlagResult = true;
            }
            UpdateCompareFlag(LatestCmpFlagResult);
            return OR_CONTINUE;
        case COMMAND_GET_MODEL_NAME_FOR_DEBUG_ONLY:
            return OR_CONTINUE;
        case COMMAND_TASK_USE_NEARBY_ENTRY_EXIT:
            return OR_CONTINUE;
        case COMMAND_ENABLE_ENTRY_EXIT_PLAYER_GROUP_WARPING:
            CollectParameters(4);
            EntryExitID = CEntryExitManager::FindNearestEntryExit(
                CVector2D(*reinterpret_cast<f32*>(&ScriptParams[0]), *reinterpret_cast<f32*>(&ScriptParams[1])),
                *reinterpret_cast<f32*>(&ScriptParams[2]), -1);
            pEntryExit = CEntryExitManager::GetEntryExit(EntryExitID);
            if (ScriptParams[3])
            {
                pEntryExit->m_nFlags |= CEntryExit::ACCEPT_NPC_GROUP;
                if (pEntryExit->m_pLink)
                {
                    pEntryExit->m_pLink->m_nFlags |= CEntryExit::ACCEPT_NPC_GROUP;
                }
            }
            else
            {
                pEntryExit->m_nFlags &= ~CEntryExit::ACCEPT_NPC_GROUP;
                if (pEntryExit->m_pLink)
                {
                    pEntryExit->m_pLink->m_nFlags &= ~CEntryExit::ACCEPT_NPC_GROUP;
                }
            }
            return OR_CONTINUE;
        case COMMAND_GET_CLOSEST_STEALABLE_OBJECT:
        {
            i16 NumOfEntities;
            CEntity* pEntityArray[16];
            CEntity* pClosestObj;
            f32 ClosestDist;
            CVector ObjPos, DiffVector;
            f32 Radius;
            f32 ObjDistance;

            CollectParameters(4);
            NewX = *reinterpret_cast<f32*>(&ScriptParams[0]);
            NewY = *reinterpret_cast<f32*>(&ScriptParams[1]);
            NewZ = *reinterpret_cast<f32*>(&ScriptParams[2]);
            Radius = *reinterpret_cast<f32*>(&ScriptParams[3]);
            ClosestDist = Radius * 2.0f;
            pClosestObj = nullptr;
            CWorld::FindObjectsInRange(CVector(NewX, NewY, NewZ), Radius, true, &NumOfEntities, 16, pEntityArray,
                false, false, false, true, false);
            for (loop = 0; loop < static_cast<u32>(NumOfEntities); loop++)
            {
                if (pEntityArray[loop]->GetIsTypeObject() &&
                    static_cast<CObject*>(pEntityArray[loop])->objectFlags.bIsLiftable)
                {
                    ObjPos = pEntityArray[loop]->GetPosition();
                    DiffVector = ObjPos - CVector(NewX, NewY, NewZ);
                    ObjDistance = DiffVector.Magnitude();
                    if (ObjDistance < ClosestDist)
                    {
                        ClosestDist = ObjDistance;
                        pClosestObj = pEntityArray[loop];
                    }
                }
            }
            if (pClosestObj)
            {
                ScriptParams[0] = CPools::GetObjectPool().GetIndex(static_cast<CObject*>(pClosestObj));
            }
            else
            {
                ScriptParams[0] = -1;
            }
            StoreParameters(1);
            return OR_CONTINUE;
        }
        case COMMAND_IS_PROCEDURAL_INTERIOR_ACTIVE:
            CollectParameters(1);
            LatestCmpFlagResult = false;
            if (g_interiorMan.IsGroupActive(ScriptParams[0]))
            {
                LatestCmpFlagResult = true;
            }
            UpdateCompareFlag(LatestCmpFlagResult);
            return OR_CONTINUE;
        case COMMAND_CLEAR_THIS_VIEW_INTEGER_VARIABLE:
            GetIndexOfGlobalVariable();
            return OR_CONTINUE;
        case COMMAND_CLEAR_THIS_VIEW_FLOAT_VARIABLE:
            GetIndexOfGlobalVariable();
            return OR_CONTINUE;
        case COMMAND_CLEAR_ALL_VIEW_VARIABLES:
            return OR_CONTINUE;
        case COMMAND_CLEAR_THIS_INTEGER_WATCHPOINT:
            GetIndexOfGlobalVariable();
            return OR_CONTINUE;
        case COMMAND_CLEAR_THIS_FLOAT_WATCHPOINT:
            GetIndexOfGlobalVariable();
            return OR_CONTINUE;
        case COMMAND_CLEAR_ALL_BREAKPOINTS:
            return OR_CONTINUE;
        case COMMAND_CLEAR_ALL_WATCHPOINTS:
            return OR_CONTINUE;
        case COMMAND_IS_THIS_MODEL_A_TRAIN:
            return OR_CONTINUE;
        case COMMAND_GET_VEHICLE_CHAR_IS_STANDING_ON:
            return OR_CONTINUE;
        case COMMAND_SWITCH_START:
        case COMMAND_SWITCH_CONTINUED:
            if (CurrCommand == COMMAND_SWITCH_START)
            {
                CollectParameters(4);
                CTheScripts::ValueToCheckInSwitchStatement = ScriptParams[0];
                CTheScripts::NumberOfEntriesStillToReadForSwitch = static_cast<u16>(ScriptParams[1] * 2);
                CTheScripts::SwitchDefaultExists = ScriptParams[2] != 0;
                CTheScripts::SwitchDefaultAddress = ScriptParams[3];
                MaxPossibleParametersForThisCommand = 14;
            }
            else if (CurrCommand == COMMAND_SWITCH_CONTINUED)
            {
                MaxPossibleParametersForThisCommand = 18;
            }
            CollectParameters(MaxPossibleParametersForThisCommand);
            if (CTheScripts::NumberOfEntriesStillToReadForSwitch > MaxPossibleParametersForThisCommand)
            {
                for (loop = 0; loop < MaxPossibleParametersForThisCommand; loop += 2)
                {
                    CTheScripts::AddToSwitchJumpTable(ScriptParams[loop], ScriptParams[loop + 1]);
                }
                CTheScripts::NumberOfEntriesStillToReadForSwitch -= MaxPossibleParametersForThisCommand;
            }
            else
            {
                for (loop = 0; loop < CTheScripts::NumberOfEntriesStillToReadForSwitch; loop += 2)
                {
                    CTheScripts::AddToSwitchJumpTable(ScriptParams[loop], ScriptParams[loop + 1]);
                }
                CTheScripts::NumberOfEntriesStillToReadForSwitch = 0;
            }
            if (CTheScripts::NumberOfEntriesStillToReadForSwitch == 0)
            {
                CTheScripts::UseSwitchJumpTable(JumpAddress);
                UpdatePC(JumpAddress);
            }
            return OR_CONTINUE;
        case COMMAND_REMOVE_CAR_RECORDING:
            CollectParameters(1);
            CVehicleRecording::RemoveRecordingFile(ScriptParams[0]);
            return OR_CONTINUE;
        case COMMAND_SET_ZONE_POPULATION_RACE:
            ReadTextLabelFromScript(ZoneLabel, 8);
            CollectParameters(1);
            ZoneIndex = CTheZones::FindZoneByLabelAndReturnIndex(ZoneLabel, ZONE_INFORMATION);
            if (ZoneIndex >= 0)
            {
                CTheZones::ZoneInfoArray[CTheZones::NavigationZoneArray[ZoneIndex].ZoneInfoIndex].PopRaces = ScriptParams[0];
            }
            return OR_CONTINUE;
        case COMMAND_SET_OBJECT_ONLY_DAMAGED_BY_PLAYER:
            CollectParameters(2);
            pObj = CPools::GetObjectPool().GetAt(ScriptParams[0]);
            if (ScriptParams[1])
            {
                pObj->physicalFlags.bOnlyDamagedByPlayer = true;
            }
            else
            {
                pObj->physicalFlags.bOnlyDamagedByPlayer = false;
            }
            return OR_CONTINUE;
        case COMMAND_CREATE_BIRDS:
            CollectParameters(8);
            TempCoors = CVector(static_cast<f32>(ScriptParams[0]), static_cast<f32>(ScriptParams[1]), static_cast<f32>(ScriptParams[2]));
            CBirds::CreateNumberOfBirds(TempCoors,
                CVector(static_cast<f32>(ScriptParams[3]), static_cast<f32>(ScriptParams[4]), static_cast<f32>(ScriptParams[5])),
                ScriptParams[6], static_cast<eBirdsBiome>(ScriptParams[7]), false);
            return OR_CONTINUE;
        case COMMAND_GET_VEHICLE_DIRT_LEVEL:
            return OR_CONTINUE;
        case COMMAND_SET_VEHICLE_DIRT_LEVEL:
            CollectParameters(2);
            pVehicle = CPools::GetVehiclePool().GetAt(ScriptParams[0]);
            pVehicle->m_fDirtLevel = (*reinterpret_cast<f32*>(&ScriptParams[1]));
            return OR_CONTINUE;
        case COMMAND_SET_GANG_WARS_ACTIVE:
            CollectParameters(1);
            if (ScriptParams[0])
            {
                CGangWars::SetGangWarsActive(true);
            }
            else
            {
                CGangWars::SetGangWarsActive(false);
            }
            return OR_CONTINUE;
        case COMMAND_IS_GANG_WAR_GOING_ON:
            LatestCmpFlagResult = false;
            if (CGangWars::GangWarGoingOn())
            {
                LatestCmpFlagResult = true;
            }
            UpdateCompareFlag(LatestCmpFlagResult);
            return OR_CONTINUE;
        case COMMAND_GIVE_PLAYER_CLOTHES_OUTSIDE_SHOP:
            CollectParameters(1);
            pPlayer = &CWorld::Players[ScriptParams[0]];
            ReadTextLabelFromScript(ClothesTextureName, 16);
            ReadTextLabelFromScript(ClothesModelName, 16);
            CollectParameters(1);
            pPlayer->PlayerPedData.m_pClothes->SetTextureAndModel(ClothesTextureName, ClothesModelName, ScriptParams[0]);
            CShopping::SetPlayerHasBought(CKeyGen::GetUppercaseKey(ClothesTextureName));
            return OR_CONTINUE;
        case COMMAND_CLEAR_LOADED_SHOP:
            CShopping::RemoveLoadedShop();
            return OR_CONTINUE;
        case COMMAND_SET_GROUP_SEQUENCE:
        {
            CollectParameters(2);
            const i32 iGroupID = CTheScripts::GetActualScriptThingIndex(ScriptParams[0], UNIQUE_SCRIPT_PEDGROUP);
            if (iGroupID >= 0 && iGroupID < 8)
            {
                i32 iSequenceID = -1;
                if (ScriptParams[1] != -1)
                {
                    iSequenceID = CTheScripts::GetActualScriptThingIndex(ScriptParams[1], UNIQUE_SCRIPT_SEQUENCE_TASK);
                }
                CPedGroups::ms_groups[iGroupID].GetGroupIntelligence()->SetGroupSequenceTaskID(iSequenceID);
            }
            return OR_CONTINUE;
        }
        case COMMAND_SET_CHAR_DROPS_WEAPONS_WHEN_DEAD:
            CollectParameters(2);
            pPed = CPools::GetPedPool().GetAt(ScriptParams[0]);
            if (ScriptParams[1])
            {
                pPed->bDoesntDropWeaponsWhenDead = false;
            }
            else
            {
                pPed->bDoesntDropWeaponsWhenDead = true;
            }
            return OR_CONTINUE;
        case COMMAND_SET_CHAR_NEVER_LEAVES_GROUP:
            CollectParameters(2);
            pPed = CPools::GetPedPool().GetAt(ScriptParams[0]);
            if (ScriptParams[1])
            {
                pPed->bNeverLeavesGroup = true;
            }
            else
            {
                pPed->bNeverLeavesGroup = false;
            }
            return OR_CONTINUE;
        case COMMAND_DRAW_RECT_WITH_TITLE:
            return OR_CONTINUE;
        case COMMAND_SET_PLAYER_FIRE_BUTTON:
            CollectParameters(2);
            if (ScriptParams[1])
            {
                CPad::GetPad(ScriptParams[0])->bDisablePlayerFireWeapon = false;
            }
            else
            {
                CPad::GetPad(ScriptParams[0])->bDisablePlayerFireWeapon = true;
            }
            return OR_CONTINUE;
        case COMMAND_SET_ATTRACTOR_RADIUS:
            return OR_CONTINUE;
        case COMMAND_ATTACH_FX_SYSTEM_TO_CHAR_BONE:
            CollectParameters(3);
            FXSystemIndex = ScriptParams[0];
            pPed = CPools::GetPedPool().GetAt(ScriptParams[1]);
            ActualIndex = CTheScripts::GetActualScriptThingIndex(FXSystemIndex, UNIQUE_SCRIPT_EFFECT_SYSTEM);
            if (ActualIndex >= 0 && CTheScripts::ScriptEffectSystemArray[ActualIndex].pFXSystem)
            {
                CTheScripts::ScriptEffectSystemArray[ActualIndex].pFXSystem->AttachToBone(pPed, static_cast<eBoneTag>(ScriptParams[2]));
            }
            return OR_CONTINUE;
        case COMMAND_REGISTER_ATTRACTOR_SCRIPT_BRAIN_FOR_CODE_USE:
            CollectParameters(1);
            ScriptParams[0] = CTheScripts::StreamedScripts.GetProperIndexFromIndexUsedByScript(static_cast<i16>(ScriptParams[0]));
            ReadTextLabelFromScript(ScriptBrainName, 8);
            CTheScripts::ScriptsForBrains.AddNewStreamedScriptBrainForCodeUse(static_cast<i16>(ScriptParams[0]), ScriptBrainName,
                5 /* CODE_ATTRACTOR_PED */);
            return OR_CONTINUE;
        case COMMAND_CONST_INT:
            return OR_CONTINUE;
        case COMMAND_CONST_FLOAT:
            return OR_CONTINUE;
        case COMMAND_SET_HEADING_LIMIT_FOR_ATTACHED_CHAR:
            CollectParameters(3);
            pPed = CPools::GetPedPool().GetAt(ScriptParams[0]);
            pPed->m_fTurretAngleA = static_cast<u16>(ScriptParams[1]);
            pPed->m_fTurretAngleB = DEGTORAD(*reinterpret_cast<f32*>(&ScriptParams[2]));
            return OR_CONTINUE;
        case COMMAND_ADD_BLIP_FOR_DEAD_CHAR:
        {
            CollectParameters(1);
            CRadar::GetActualBlipArrayIndex(CollectNextParameterWithoutIncreasingPC());
            i32 BlipIndex = CRadar::SetEntityBlip(BLIPTYPE_CHAR, ScriptParams[0], 1, BLIPDISPLAY_BOTH);
            CRadar::ChangeBlipScale(BlipIndex, 3);
            ScriptParams[0] = BlipIndex;
            StoreParameters(1);
            return OR_CONTINUE;
        }
        case COMMAND_GET_DEAD_CHAR_COORDINATES:
            CollectParameters(1);
            pPed = CPools::GetPedPool().GetAt(ScriptParams[0]);
            if (pPed->bInVehicle && pPed->m_pMyVehicle)
            {
                TempCoors = pPed->m_pMyVehicle->GetPosition();
            }
            else
            {
                pPed->GetBonePosition(&TempCoors, static_cast<eBoneTag>(1), false);
            }
            *reinterpret_cast<f32*>(&ScriptParams[0]) = TempCoors.x;
            *reinterpret_cast<f32*>(&ScriptParams[1]) = TempCoors.y;
            *reinterpret_cast<f32*>(&ScriptParams[2]) = TempCoors.z;
            StoreParameters(3);
            return OR_CONTINUE;
        case COMMAND_TASK_PLAY_ANIM_WITH_FLAGS:
            PlayAnimScriptCommand(CurrCommand);
            return OR_CONTINUE;
        case COMMAND_SET_VEHICLE_AIR_RESISTANCE_MULTIPLIER:
            CollectParameters(2);
            pVehicle = CPools::GetVehiclePool().GetAt(ScriptParams[0]);
            temp_float = *reinterpret_cast<f32*>(&ScriptParams[1]);
            if (pVehicle->pHandling)
            {
                if (pVehicle->pHandling->m_fDragMult > 0.01f)
                {
                    pVehicle->m_fAirResistance = pVehicle->pHandling->m_fDragMult / 1000.0f /* GAME_AIR_RESISTANCE_MASS */ * 0.5f;
                }
                else
                {
                    pVehicle->m_fAirResistance = pVehicle->pHandling->m_fDragMult;
                }
                pVehicle->m_fAirResistance *= temp_float;
            }
            return OR_CONTINUE;
        case COMMAND_SET_CAR_COORDINATES_NO_OFFSET:
            CollectParameters(4);
            pVehicle = CPools::GetVehiclePool().GetAt(ScriptParams[0]);
            CCarCtrl::SetCoordsOfScriptCar(pVehicle, *reinterpret_cast<f32*>(&ScriptParams[1]), *reinterpret_cast<f32*>(&ScriptParams[2]),
                *reinterpret_cast<f32*>(&ScriptParams[3]), false, false);
            return OR_CONTINUE;
        case COMMAND_SET_USES_COLLISION_OF_CLOSEST_OBJECT_OF_TYPE:
        {
            i16 NumOfEntities;
            CEntity* pEntityArray[16];
            CEntity* pClosestObj;
            f32 ClosestDist;
            CVector ObjPos, DiffVector;
            f32 Radius;
            u16 loop = 0;
            f32 ObjDistance;

            CollectParameters(6);
            NewX = *reinterpret_cast<f32*>(&ScriptParams[0]);
            NewY = *reinterpret_cast<f32*>(&ScriptParams[1]);
            NewZ = *reinterpret_cast<f32*>(&ScriptParams[2]);
            if (NewZ <= -100.0f)
            {
                NewZ = CWorld::FindGroundZForCoord(NewX, NewY);
            }
            Radius = *reinterpret_cast<f32*>(&ScriptParams[3]);
            ModelIndex = ScriptParams[4];
            pClosestObj = nullptr;
            ClosestDist = Radius * 2.0f;
            if (ModelIndex < 0)
            {
                ModelIndex = CTheScripts::UsedObjectArray[-ModelIndex].Index;
            }
            CWorld::FindObjectsOfTypeInRange(ModelIndex, CVector(NewX, NewY, NewZ), Radius, true, &NumOfEntities, 16, pEntityArray, true, false, false,
                true, true);
            for (loop = 0; loop < NumOfEntities; loop++)
            {
                ObjPos = pEntityArray[loop]->GetPosition();
                DiffVector = ObjPos - CVector(NewX, NewY, NewZ);
                ObjDistance = DiffVector.Magnitude();
                if (ObjDistance < ClosestDist)
                {
                    ClosestDist = ObjDistance;
                    pClosestObj = pEntityArray[loop];
                }
            }
            if (pClosestObj)
            {
                if (ScriptParams[5])
                {
                    pClosestObj->m_bUsesCollision = true;
                }
                else
                {
                    pClosestObj->m_bUsesCollision = false;
                }
            }
            return OR_CONTINUE;
        }
        case COMMAND_SET_TIME_ONE_DAY_FORWARD:
            CClock::OffsetClockByADay(true);
            return OR_CONTINUE;
        case COMMAND_SET_TIME_ONE_DAY_BACK:
            return OR_CONTINUE;
        case COMMAND_SET_TIMER_BEEP_COUNTDOWN_TIME:
            GlobalVarIndex = GetIndexOfGlobalVariable();
            CollectParameters(1);
            CUserDisplay::OnscnTimer.SetClockBeepCountdownSecs(GlobalVarIndex, ScriptParams[0]);
            return OR_CONTINUE;
        case COMMAND_TASK_SIT_IN_RESTAURANT:
            return OR_CONTINUE;
        case COMMAND_GET_RANDOM_ATTRACTOR_ON_CLOSEST_OBJECT_OF_TYPE:
            return OR_CONTINUE;
        case COMMAND_ATTACH_TRAILER_TO_CAB:
            CollectParameters(2);
            pTrailer = CPools::GetVehiclePool().GetAt(ScriptParams[0]);
            pVehicle = CPools::GetVehiclePool().GetAt(ScriptParams[1]);
            if (pTrailer && pVehicle)
            {
                pTrailer->SetTowLink(pVehicle, true);
            }
            return OR_CONTINUE;
        case COMMAND_ADD_INTERESTING_ENTITY_FOR_CHAR:
            return OR_CONTINUE;
        case COMMAND_CLEAR_INTERESTING_ENTITIES_FOR_CHAR:
            return OR_CONTINUE;
        case COMMAND_GET_CLOSEST_ATTRACTOR:
            return OR_CONTINUE;
        case COMMAND_IS_VEHICLE_TOUCHING_OBJECT:
            CollectParameters(2);
            pVehicle = CPools::GetVehiclePool().GetAt(ScriptParams[0]);
            pObj = CPools::GetObjectPool().GetAt(ScriptParams[1]);
            LatestCmpFlagResult = false;
            if (pVehicle && pVehicle->GetHasCollidedWith(pObj))
            {
                LatestCmpFlagResult = true;
            }
            UpdateCompareFlag(LatestCmpFlagResult);
            return OR_CONTINUE;
    }
    return OR_INTERRUPT;
}

// MARK: 2200To2299

// func: sa 0x474900
OpcodeResult CRunningScript::ProcessCommands2200To2299(i32 CurrCommand)
{
    MARKFUNCTION(0x474900);

    CPed* pPed;
    bool LatestCmpFlagResult;
    f32 NewY, NewX, NewZ;
    f32 fRadius, fHeading;
    CVector CentreVec, TempVec, VecDiff;
    CVehicle* pVehicle;
    CPlayerInfo* pPlayer;
    f32 temp_float;
    char TextLabel[10];
    char TextLabel1[10];
    char TextLabel2[10];
    char LongTextLabel[16];
    i32 iPedID;
    i32 ModelIndex, ArrayIndex;
    i32* pGlobalVar;
    i32* pLocalVar;
    char ScriptBrainName[10];
    CTask* pTask;
    i32 RowIndex, ColumnIndex, MenuIndex;
    char AnimName[24];
    char ModelName[10];

    // NOTE: Keep switch cases in strictly ascending order as in commands.hpp to preserve MSVC jump table layout
    switch (CurrCommand)
    {
        case COMMAND_ENABLE_CRANE_CONTROLS:
            CollectParameters(3);
            CTheScripts::bEnableCraneRaise = ScriptParams[0] != 0;
            CTheScripts::bEnableCraneLower = ScriptParams[1] != 0;
            CTheScripts::bEnableCraneRelease = ScriptParams[2] != 0;
            return OR_CONTINUE;
        case COMMAND_ALLOCATE_SCRIPT_TO_ATTRACTOR:
            return OR_CONTINUE;
        case COMMAND_GET_CLOSEST_ATTRACTOR_WITH_THIS_SCRIPT:
            return OR_CONTINUE;
        case COMMAND_IS_PLAYER_IN_POSITION_FOR_CONVERSATION:
            CollectParameters(1);
            pPed = CPools::GetPedPool().GetAt(ScriptParams[0]);
            LatestCmpFlagResult = CConversations::IsPlayerInPositionForConversation(pPed, false);
            UpdateCompareFlag(LatestCmpFlagResult);
            return OR_CONTINUE;
        case COMMAND_ENABLE_CONVERSATION:
            CollectParameters(2);
            pPed = CPools::GetPedPool().GetAt(ScriptParams[0]);
            if (ScriptParams[1])
            {
                CConversations::EnableConversation(pPed, true);
            }
            else
            {
                CConversations::EnableConversation(pPed, false);
            }
            return OR_CONTINUE;
        case COMMAND_GET_CONVERSATION_STATUS:
            return OR_CONTINUE;
        case COMMAND_GET_RANDOM_CHAR_IN_SPHERE_ONLY_DRUGS_BUYERS:
        {
            CollectParameters(4);
            CentreVec = CVector(*reinterpret_cast<f32*>(&ScriptParams[0]), *reinterpret_cast<f32*>(&ScriptParams[1]),
                *reinterpret_cast<f32*>(&ScriptParams[2]));
            fRadius = *reinterpret_cast<f32*>(&ScriptParams[3]);
            i32 PedPoolIndex = CPools::GetPedPool().GetSize();
            i32 RandomPedIndex = -1;
            i32 ClosestDistance = 9999;
            while (PedPoolIndex--)
            {
                pPed = CPools::GetPedPool().GetSlot(PedPoolIndex);
                if (pPed && pPed->GetCharCreatedBy() == RANDOM_CHAR && !pPed->m_bRemoveFromWorld && !pPed->bFadeOut &&
                    !IsPedDead(pPed) &&
                    static_cast<CPedModelInfo*>(CModelInfo::GetModelInfo(pPed->GetModelIndex()))->m_nPedFlags & 1 /* PED_MINFO_FLAG_BUYSDRUGS */ &&
                    !CPedGroups::GetPedsGroup(pPed))
                {
                    TempVec = pPed->GetPosition();
                    VecDiff = CVector(TempVec.x - CentreVec.x, TempVec.y - CentreVec.y, TempVec.z - CentreVec.z);
                    f32 Distance = VecDiff.Magnitude();
                    if (Distance < fRadius && Distance < ClosestDistance)
                    {
                        RandomPedIndex = CPools::GetPedPool().GetIndex(pPed);
                        ClosestDistance = static_cast<i32>(Distance);
                    }
                }
            }
            if (RandomPedIndex >= 0)
            {
                pPed = CPools::GetPedPool().GetAt(RandomPedIndex);
                pPed->SetCharCreatedBy(MISSION_CHAR);
                CPopulation::ms_nTotalMissionPeds++;
                if (IsThisAMissionScript)
                {
                    CTheScripts::MissionCleanUp.AddEntityToList(RandomPedIndex, CLEANUP_CHAR);
                }
            }
            ScriptParams[0] = RandomPedIndex;
            StoreParameters(1);
            return OR_CONTINUE;
        }
        case COMMAND_GET_PED_TYPE:
            CollectParameters(1);
            ScriptParams[0] = CPools::GetPedPool().GetAt(ScriptParams[0])->m_nPedType;
            StoreParameters(1);
            return OR_CONTINUE;
        case COMMAND_TASK_USE_CLOSEST_MAP_ATTRACTOR:
        {
            CollectParameters(6);
            iPedID = ScriptParams[0];
            fRadius = *reinterpret_cast<f32*>(&ScriptParams[1]);
            ModelIndex = ScriptParams[2];
            NewX = *reinterpret_cast<f32*>(&ScriptParams[3]);
            NewY = *reinterpret_cast<f32*>(&ScriptParams[4]);
            NewZ = *reinterpret_cast<f32*>(&ScriptParams[5]);
            ReadTextLabelFromScript(TextLabel, 8);
            CVector vPos;
            if (iPedID == -1)
            {
                vPos = CVector(NewX, NewY, NewZ);
                pPed = nullptr;
            }
            else
            {
                pPed = CPools::GetPedPool().GetAt(iPedID);
                vPos = pPed->GetPosition();
            }
            C2dEffect* pEffectToAvoid = nullptr;
            if (pPed)
            {
                pEffectToAvoid = pPed->GetPedIntelligence()->GetEffectInUse();
            }
            if (ModelIndex == 0)
            {
                ModelIndex = -1;
            }
            else if (ModelIndex < 0)
            {
                ModelIndex = CTheScripts::UsedObjectArray[-ModelIndex].Index;
            }
            {
                C2dEffect* pNearestEffect = nullptr;
                CEntity* pNearestEntity = nullptr;
                CAttractorScanner::GetClosestAttractorOfType(vPos, fRadius, ModelIndex, 5, TextLabel, true, pEffectToAvoid,
                    pNearestEffect, pNearestEntity);
                if (pNearestEffect)
                {
                    if (ScriptBrainType != -1)
                    {
                        RemoveScriptFromList(&CTheScripts::pActiveScripts);
                        AddScriptToList(&CTheScripts::pIdleScripts);
                        ShutdownThisScript();
                        {
                            CEventAttractor event(reinterpret_cast<C2dEffectPedAttractor*>(pNearestEffect), pNearestEntity, true, TASK_COMPLEX_USE_EFFECT);
                            pPed->GetPedIntelligence()->AddEvent(event, true);
                            pPed->GetPedIntelligence()->SetEffectInUse(pNearestEffect);
                        }
                        return OR_WAIT;
                    }
                    GivePedScriptedTask(iPedID, new CTaskComplexUseEffect(reinterpret_cast<C2dEffectPedAttractor*>(pNearestEffect), pNearestEntity), CurrCommand);
                    UpdateCompareFlag(true);
                }
                else
                {
                    UpdateCompareFlag(false);
                }
            }
            return OR_CONTINUE;
        }
        case COMMAND_GET_CHAR_MAP_ATTRACTOR_STATUS:
            return OR_CONTINUE;
        case COMMAND_PLANE_ATTACK_PLAYER_USING_DOG_FIGHT:
            CollectParameters(3);
            pVehicle = CPools::GetVehiclePool().GetAt(ScriptParams[0]);
            pVehicle->AutoPilot.SetMission(MISSION_PLANE_DOG_FIGHT_PLAYER);
            static_cast<CPlane*>(pVehicle)->m_minAltitude = *reinterpret_cast<f32*>(&ScriptParams[2]);
            return OR_CONTINUE;
        case COMMAND_CAN_TRIGGER_GANG_WAR_WHEN_ON_A_MISSION:
            CollectParameters(1);
            CGangWars::bCanTriggerGangWarWhenOnAMission = ScriptParams[0] != 0;
            return OR_CONTINUE;
        case COMMAND_CONTROL_MOVABLE_VEHICLE_PART:
            CollectParameters(2);
            static_cast<CAutomobile*>(CPools::GetVehiclePool().GetAt(ScriptParams[0]))->UpdateMovingCollision(*reinterpret_cast<f32*>(&ScriptParams[1]));
            return OR_CONTINUE;
        case COMMAND_WINCH_CAN_PICK_VEHICLE_UP:
            CollectParameters(2);
            pVehicle = CPools::GetVehiclePool().GetAt(ScriptParams[0]);
            if (ScriptParams[1])
            {
                pVehicle->vehicleFlags.bWinchCanPickMeUp = true;
            }
            else
            {
                pVehicle->vehicleFlags.bWinchCanPickMeUp = false;
            }
            return OR_CONTINUE;
        case COMMAND_OPEN_CAR_DOOR_A_BIT:
        {
            CollectParameters(3);
            pVehicle = CPools::GetVehiclePool().GetAt(ScriptParams[0]);
            const eDoors DoorId = static_cast<eDoors>(ScriptParams[1]);
            i32 PanelId = CDamageManager::GetCarNodeIndexFromDoor(DoorId);
            if (!pVehicle->IsDoorMissing(DoorId) && static_cast<CAutomobile*>(pVehicle)->m_aCarNodes[PanelId])
            {
                pVehicle->OpenDoor(nullptr, PanelId, DoorId, *reinterpret_cast<f32*>(&ScriptParams[2]), true);
            }
            return OR_CONTINUE;
        }
        case COMMAND_IS_CAR_DOOR_FULLY_OPEN:
        {
            CollectParameters(2);
            pVehicle = CPools::GetVehiclePool().GetAt(ScriptParams[0]);
            const eDoors DoorId = static_cast<eDoors>(ScriptParams[1]);
            i32 PanelId = CDamageManager::GetCarNodeIndexFromDoor(DoorId);
            LatestCmpFlagResult = pVehicle->IsDoorFullyOpen(static_cast<eDoors>(PanelId));
            UpdateCompareFlag(LatestCmpFlagResult);
            return OR_CONTINUE;
        }
        case COMMAND_SET_ALWAYS_DRAW_3D_MARKERS:
            CollectParameters(1);
            if (ScriptParams[0])
            {
                C3dMarkers::ForceRender(true);
            }
            else
            {
                C3dMarkers::ForceRender(false);
            }
            return OR_CONTINUE;
        case COMMAND_STREAM_SCRIPT:
            CollectParameters(1);
            ScriptParams[0] = CTheScripts::StreamedScripts.GetProperIndexFromIndexUsedByScript(static_cast<i16>(ScriptParams[0]));
            CStreaming::RequestModel(SCMToModelId(ScriptParams[0]), STRFLAG_MISSION_REQUIRED);
            return OR_CONTINUE;
        case COMMAND_STREAM_SCRIPT_INTERNAL:
            return OR_CONTINUE;
        case COMMAND_HAS_STREAMED_SCRIPT_LOADED:
            CollectParameters(1);
            ScriptParams[0] = CTheScripts::StreamedScripts.GetProperIndexFromIndexUsedByScript(static_cast<i16>(ScriptParams[0]));
            LatestCmpFlagResult = false;
            if (CStreaming::IsModelLoaded(SCMToModelId(ScriptParams[0])))
            {
                LatestCmpFlagResult = true;
            }
            UpdateCompareFlag(LatestCmpFlagResult);
            return OR_CONTINUE;
        case COMMAND_SET_GANG_WARS_TRAINING_MISSION:
            CollectParameters(1);
            CGangWars::bTrainingMission = ScriptParams[0] != 0;
            return OR_CONTINUE;
        case COMMAND_SET_CHAR_HAS_USED_ENTRY_EXIT:
        {
            CollectParameters(4);
            pPed = CPools::GetPedPool().GetAt(ScriptParams[0]);
            i32 EntryExitID = CEntryExitManager::FindNearestEntryExit(
                CVector2D(*reinterpret_cast<f32*>(&ScriptParams[1]), *reinterpret_cast<f32*>(&ScriptParams[2])),
                *reinterpret_cast<f32*>(&ScriptParams[3]), -1);
            CEntryExit* pEntryExit = CEntryExitManager::GetEntryExit(EntryExitID);
            CEntryExit* pSpawnEE = pEntryExit;
            if (pEntryExit->m_pLink)
            {
                pSpawnEE = pEntryExit->m_pLink;
                pSpawnEE->m_pLink = pEntryExit;
            }
            pPed->SetAreaCode(static_cast<eAreaCodes>(pSpawnEE->m_nArea));
            pPed->m_pEnex = pSpawnEE->m_nArea != 0 ? pEntryExit : nullptr;
            if (pPed->IsPlayer())
            {
                CEntryExitManager::ClearEntryExitStack();
                CEntryExitManager::AddEntryExitToStack(pEntryExit);
                if (pSpawnEE->m_nSkyColor)
                {
                    CTimeCycle::StartExtraColour(pSpawnEE->m_nSkyColor - 1, false);
                }
                else
                {
                    CTimeCycle::StopExtraColour(false);
                }
            }
            return OR_CONTINUE;
        }
        case COMMAND_DRAW_WINDOW_TEXT:
            return OR_CONTINUE;
        case COMMAND_SET_CHAR_MAX_HEALTH:
            CollectParameters(2);
            CPools::GetPedPool().GetAt(ScriptParams[0])->m_fMaxHealth = static_cast<f32>(ScriptParams[1]);
            return OR_CONTINUE;
        case COMMAND_SET_CAR_PITCH:
            return OR_CONTINUE;
        case COMMAND_SET_NIGHT_VISION:
            CollectParameters(1);
            if (ScriptParams[0])
            {
                CPostEffects::ScriptNightVisionSwitch(true);
            }
            else
            {
                CPostEffects::ScriptNightVisionSwitch(false);
            }
            return OR_CONTINUE;
        case COMMAND_SET_INFRARED_VISION:
            CollectParameters(1);
            if (ScriptParams[0])
            {
                CPostEffects::ScriptInfraredVisionSwitch(true);
            }
            else
            {
                CPostEffects::ScriptInfraredVisionSwitch(false);
            }
            return OR_CONTINUE;
        case COMMAND_SET_ZONE_FOR_GANG_WARS_TRAINING:
        {
            ReadTextLabelFromScript(TextLabel, 8);
            i32 index = CTheZones::FindZoneByLabelAndReturnIndex(TextLabel, ZONE_INFORMATION);
            CGangWars::ZoneInfoForTraining = CTheZones::NavigationZoneArray[index].ZoneInfoIndex;
            return OR_CONTINUE;
        }
        case COMMAND_IS_GLOBAL_VAR_BIT_SET_CONST:
        case COMMAND_IS_GLOBAL_VAR_BIT_SET_VAR:
        case COMMAND_IS_GLOBAL_VAR_BIT_SET_LVAR:
        case COMMAND_IS_LOCAL_VAR_BIT_SET_CONST:
        case COMMAND_IS_LOCAL_VAR_BIT_SET_VAR:
        case COMMAND_IS_LOCAL_VAR_BIT_SET_LVAR:
        {
            CollectParameters(2);
            i32 nInt = ScriptParams[0];
            i32 nBit = ScriptParams[1];
            bool bResult = (nInt & (1 << nBit)) != 0;
            UpdateCompareFlag(bResult);
            return OR_CONTINUE;
        }
        case COMMAND_SET_GLOBAL_VAR_BIT_CONST:
        case COMMAND_SET_GLOBAL_VAR_BIT_VAR:
        case COMMAND_SET_GLOBAL_VAR_BIT_LVAR:
        {
            i32* pGlobalVar = GetPointerToScriptVariable(2);
            CollectParameters(1);
            i32 nBit = ScriptParams[0];
            *pGlobalVar |= (1 << nBit);
            return OR_CONTINUE;
        }
        case COMMAND_SET_LOCAL_VAR_BIT_CONST:
        case COMMAND_SET_LOCAL_VAR_BIT_VAR:
        case COMMAND_SET_LOCAL_VAR_BIT_LVAR:
        {
            i32* pLocalVar = GetPointerToScriptVariable(1);
            CollectParameters(1);
            i32 nBit = ScriptParams[0];
            *pLocalVar |= (1 << nBit);
            return OR_CONTINUE;
        }
        case COMMAND_CLEAR_GLOBAL_VAR_BIT_CONST:
        case COMMAND_CLEAR_GLOBAL_VAR_BIT_VAR:
        case COMMAND_CLEAR_GLOBAL_VAR_BIT_LVAR:
        {
            i32* pGlobalVar = GetPointerToScriptVariable(2);
            CollectParameters(1);
            i32 nBit = ScriptParams[0];
            *pGlobalVar &= ~(1 << nBit);
            return OR_CONTINUE;
        }
        case COMMAND_CLEAR_LOCAL_VAR_BIT_CONST:
        case COMMAND_CLEAR_LOCAL_VAR_BIT_VAR:
        case COMMAND_CLEAR_LOCAL_VAR_BIT_LVAR:
        {
            i32* pLocalVar = GetPointerToScriptVariable(1);
            CollectParameters(1);
            i32 nBit = ScriptParams[0];
            *pLocalVar &= ~(1 << nBit);
            return OR_CONTINUE;
        }
        case COMMAND_SET_CHAR_CAN_BE_KNOCKED_OFF_BIKE:
            CollectParameters(2);
            pPed = CPools::GetPedPool().GetAt(ScriptParams[0]);
            pPed->CantBeKnockedOffBike = ScriptParams[1];
            return OR_CONTINUE;
        case COMMAND_SET_CHAR_COORDINATES_DONT_WARP_GANG:
            CollectParameters(4);
            SetCharCoordinates(CPools::GetPedPool().GetAt(ScriptParams[0]), *reinterpret_cast<f32*>(&ScriptParams[1]),
                *reinterpret_cast<f32*>(&ScriptParams[2]), *reinterpret_cast<f32*>(&ScriptParams[3]), false, true);
            return OR_CONTINUE;
        case COMMAND_ADD_PRICE_MODIFIER:
        {
            CollectParameters(2);
            u32 nKey = ScriptParams[0];
            i32 nPrice = ScriptParams[1];
            CShopping::AddPriceModifier(nKey, nPrice);
            return OR_CONTINUE;
        }
        case COMMAND_REMOVE_PRICE_MODIFIER:
        {
            CollectParameters(1);
            u32 nKey = ScriptParams[0];
            CShopping::RemovePriceModifier(nKey);
            return OR_CONTINUE;
        }
        case COMMAND_INIT_ZONE_POPULATION_SETTINGS:
            CTheZones::InitZonesPopulationSettings();
            return OR_CONTINUE;
        case COMMAND_EXPLODE_CAR_IN_CUTSCENE_SHAKE_AND_BIT:
            CollectParameters(4);
            pVehicle = CPools::GetVehiclePool().GetAt(ScriptParams[0]);
            pVehicle->vehicleFlags.bCanBeDamaged = true;
            pVehicle->BlowUpCarCutSceneNoExtras(ScriptParams[1] == 0, ScriptParams[2] == 0, false, ScriptParams[3] != 0);
            return OR_CONTINUE;
        case COMMAND_PICK_UP_OBJECT_WITH_WINCH:
            return OR_CONTINUE;
        case COMMAND_PICK_UP_VEHICLE_WITH_WINCH:
            return OR_CONTINUE;
        case COMMAND_PICK_UP_CHAR_WITH_WINCH:
            return OR_CONTINUE;
        case COMMAND_STORE_CAR_IN_NEAREST_IMPOUNDING_GARAGE:
            return OR_CONTINUE;
        case COMMAND_IS_SKIP_CUTSCENE_BUTTON_PRESSED:
            LatestCmpFlagResult = CCutsceneMgr::IsCutsceneSkipButtonBeingPressed();
            UpdateCompareFlag(LatestCmpFlagResult);
            return OR_CONTINUE;
        case COMMAND_GET_CUTSCENE_OFFSET:
            TempVec = CCutsceneMgr::ms_cutsceneOffset;
            *reinterpret_cast<f32*>(&ScriptParams[0]) = TempVec.x;
            *reinterpret_cast<f32*>(&ScriptParams[1]) = TempVec.y;
            *reinterpret_cast<f32*>(&ScriptParams[2]) = TempVec.z;
            StoreParameters(3);
            return OR_CONTINUE;
        case COMMAND_SET_OBJECT_SCALE:
            CollectParameters(2);
            CPools::GetObjectPool().GetAt(ScriptParams[0])->m_fScale = *reinterpret_cast<f32*>(&ScriptParams[1]);
            return OR_CONTINUE;
        case COMMAND_GET_CURRENT_POPULATION_ZONE_TYPE:
            ScriptParams[0] = CPopCycle::m_nCurrentZoneType;
            StoreParameters(1);
            return OR_CONTINUE;
        case COMMAND_CREATE_MENU:
        {
            char pTitle[10];
            ReadTextLabelFromScript(TextLabel, 8);
            pTitle[0] = 0;
            if (_stricmp(TextLabel, "DUMMY"))
            {
                strcpy(pTitle, TextLabel);
            }
            CollectParameters(7);
            f32 column_size = static_cast<f32>(RsGlobal.maximumWidth) * (1.0f / 640.0f) * *reinterpret_cast<f32*>(&ScriptParams[2]);
            ScriptParams[0] = CMenuSystem::CreateNewMenu(CMenuSystem::MENU_TYPE_DEFAULT, pTitle,
                *reinterpret_cast<f32*>(&ScriptParams[0]) * (static_cast<f32>(RsGlobal.maximumWidth) * (1.0f / 640.0f)),
                static_cast<f32>(RsGlobal.maximumHeight) * (1.0f / 448.0f) * *reinterpret_cast<f32*>(&ScriptParams[1]), column_size,
                static_cast<u8>(ScriptParams[3]), static_cast<u8>(ScriptParams[4]), static_cast<u8>(ScriptParams[5]),
                static_cast<eFontAlignment>(ScriptParams[6]));
            StoreParameters(1);
            return OR_CONTINUE;
        }
        case COMMAND_CONSTANT_INT:
            return OR_CONTINUE;
        case COMMAND_SET_MENU_COLUMN_ORIENTATION:
            CollectParameters(3);
            CMenuSystem::SetColumnOrientation(static_cast<u8>(ScriptParams[0]), static_cast<u8>(ScriptParams[1]), static_cast<u8>(ScriptParams[2]));
            return OR_CONTINUE;
        case COMMAND_GET_MENU_ITEM_SELECTED:
            CollectParameters(1);
            ScriptParams[0] = CMenuSystem::CheckForSelected(static_cast<u8>(ScriptParams[0]));
            StoreParameters(1);
            return OR_CONTINUE;
        case COMMAND_GET_MENU_ITEM_ACCEPTED:
            CollectParameters(1);
            ScriptParams[0] = CMenuSystem::CheckForAccept(static_cast<u8>(ScriptParams[0]));
            StoreParameters(1);
            return OR_CONTINUE;
        case COMMAND_ACTIVATE_MENU_ITEM:
            CollectParameters(3);
            CMenuSystem::ActivateOneItem(static_cast<u8>(ScriptParams[0]), static_cast<u8>(ScriptParams[1]), static_cast<u8>(ScriptParams[2]));
            return OR_CONTINUE;
        case COMMAND_DELETE_MENU:
            CollectParameters(1);
            CMenuSystem::SwitchOffMenu(static_cast<u8>(ScriptParams[0]));
            return OR_CONTINUE;
        case COMMAND_SET_MENU_COLUMN:
        {
            CollectParameters(2);
            i32 nMenuID = ScriptParams[0];
            i32 nColumn = ScriptParams[1];
            char pText[10];
            pText[0] = 0;
            ReadTextLabelFromScript(TextLabel, 8);
            if (_stricmp(TextLabel, "DUMMY"))
            {
                strcpy(pText, TextLabel);
            }
            char pText2[10];
            pText2[0] = 0;
            ReadTextLabelFromScript(TextLabel1, 8);
            if (_stricmp(TextLabel1, "DUMMY"))
            {
                strcpy(pText2, TextLabel1);
            }
            char aTextLabels[11][10];
            u32 i;
            for (i = 0; i < 11; i++)
            {
                aTextLabels[i][0] = 0;
            }
            for (i = 0; i < 11; i++)
            {
                ReadTextLabelFromScript(TextLabel2, 8);
                if (_stricmp(TextLabel2, "DUMMY"))
                {
                    strcpy(aTextLabels[i], TextLabel2);
                }
            }
            CMenuSystem::InsertMenu(static_cast<u8>(nMenuID), static_cast<u8>(nColumn), pText, pText2, aTextLabels[0], aTextLabels[1],
                aTextLabels[2], aTextLabels[3], aTextLabels[4], aTextLabels[5], aTextLabels[6], aTextLabels[7], aTextLabels[8],
                aTextLabels[9], aTextLabels[10]);
            return OR_CONTINUE;
        }
        case COMMAND_SET_BLIP_ENTRY_EXIT:
        {
            CollectParameters(4);
            i32 blipId = ScriptParams[0];
            i32 EntryExitID = CEntryExitManager::FindNearestEntryExit(
                CVector2D(*reinterpret_cast<f32*>(&ScriptParams[1]), *reinterpret_cast<f32*>(&ScriptParams[2])),
                *reinterpret_cast<f32*>(&ScriptParams[3]), 0);
            CEntryExit* pEntryExit = CEntryExitManager::GetEntryExit(EntryExitID);
            CRadar::SetBlipEntryExit(blipId, pEntryExit);
            return OR_CONTINUE;
        }
        case COMMAND_SWITCH_DEATH_PENALTIES:
            CollectParameters(1);
            CGameLogic::bPenaltyForDeathApplies = ScriptParams[0] != 0;
            return OR_CONTINUE;
        case COMMAND_SWITCH_ARREST_PENALTIES:
            CollectParameters(1);
            CGameLogic::bPenaltyForArrestApplies = ScriptParams[0] != 0;
            return OR_CONTINUE;
        case COMMAND_SET_EXTRA_HOSPITAL_RESTART_POINT:
            CollectParameters(5);
            CRestart::ExtraHospitalRestartCoors = CVector(*reinterpret_cast<f32*>(&ScriptParams[0]), *reinterpret_cast<f32*>(&ScriptParams[1]),
                *reinterpret_cast<f32*>(&ScriptParams[2]));
            CRestart::ExtraHospitalRestartRadius = *reinterpret_cast<f32*>(&ScriptParams[3]);
            CRestart::ExtraHospitalRestartHeading = *reinterpret_cast<f32*>(&ScriptParams[4]);
            return OR_CONTINUE;
        case COMMAND_SET_EXTRA_POLICE_STATION_RESTART_POINT:
            CollectParameters(5);
            CRestart::ExtraPoliceStationRestartCoors = CVector(*reinterpret_cast<f32*>(&ScriptParams[0]),
                *reinterpret_cast<f32*>(&ScriptParams[1]), *reinterpret_cast<f32*>(&ScriptParams[2]));
            CRestart::ExtraPoliceStationRestartRadius = *reinterpret_cast<f32*>(&ScriptParams[3]);
            CRestart::ExtraPoliceStationRestartHeading = *reinterpret_cast<f32*>(&ScriptParams[4]);
            return OR_CONTINUE;
        case COMMAND_FIND_NUMBER_TAGS_TAGGED:
            ScriptParams[0] = CTagManager::ms_numTagged;
            StoreParameters(1);
            return OR_CONTINUE;
        case COMMAND_GET_TERRITORY_UNDER_CONTROL_PERCENTAGE:
            ScriptParams[0] = static_cast<i32>(CGangWars::TerritoryUnderControlPercentage * 100.0f);
            StoreParameters(1);
            return OR_CONTINUE;
        case COMMAND_IS_OBJECT_IN_ANGLED_AREA_2D:
        case COMMAND_IS_OBJECT_IN_ANGLED_AREA_3D:
            ObjectInAngledAreaCheckCommand(CurrCommand);
            return OR_CONTINUE;
        case COMMAND_GET_RANDOM_CHAR_IN_SPHERE_NO_BRAIN:
        {
            CollectParameters(4);
            CentreVec = CVector(*reinterpret_cast<f32*>(&ScriptParams[0]), *reinterpret_cast<f32*>(&ScriptParams[1]),
                *reinterpret_cast<f32*>(&ScriptParams[2]));
            fRadius = *reinterpret_cast<f32*>(&ScriptParams[3]);
            i32 PedPoolIndex = CPools::GetPedPool().GetSize();
            i32 RandomPedIndex = -1;
            i32 ClosestDistance = 9999;
            while (PedPoolIndex--)
            {
                pPed = CPools::GetPedPool().GetSlot(PedPoolIndex);
                if (pPed && pPed->GetCharCreatedBy() == RANDOM_CHAR && !pPed->m_bRemoveFromWorld && !pPed->bFadeOut &&
                    !IsPedDead(pPed) && !CPedGroups::GetPedsGroup(pPed) && !pPed->bHasAScriptBrain)
                {
                    TempVec = pPed->GetPosition();
                    VecDiff = CVector(TempVec.x - CentreVec.x, TempVec.y - CentreVec.y, TempVec.z - CentreVec.z);
                    f32 Distance = VecDiff.Magnitude();
                    if (Distance < fRadius && Distance < ClosestDistance)
                    {
                        RandomPedIndex = CPools::GetPedPool().GetIndex(pPed);
                        ClosestDistance = static_cast<i32>(Distance);
                    }
                }
            }
            if (RandomPedIndex >= 0)
            {
                pPed = CPools::GetPedPool().GetAt(RandomPedIndex);
                pPed->SetCharCreatedBy(MISSION_CHAR);
                CPopulation::ms_nTotalMissionPeds++;
                if (IsThisAMissionScript)
                {
                    CTheScripts::MissionCleanUp.AddEntityToList(RandomPedIndex, CLEANUP_CHAR);
                }
            }
            ScriptParams[0] = RandomPedIndex;
            StoreParameters(1);
            return OR_CONTINUE;
        }
        case COMMAND_SET_PLANE_UNDERCARRIAGE_UP:
            CollectParameters(2);
            pVehicle = CPools::GetVehiclePool().GetAt(ScriptParams[0]);
            if (ScriptParams[1])
            {
                static_cast<CPlane*>(pVehicle)->SetGearUp();
            }
            else
            {
                static_cast<CPlane*>(pVehicle)->SetGearDown();
            }
            return OR_CONTINUE;
        case COMMAND_DISABLE_ALL_ENTRY_EXITS:
            CollectParameters(1);
            CEntryExitManager::DisableAllEntryExits(ScriptParams[0] != 0);
            return OR_CONTINUE;
        case COMMAND_ATTACH_ANIMS_TO_MODEL:
        {
            CollectParameters(1);
            i32 modelId = ScriptParams[0];
            ReadTextLabelFromScript(TextLabel1, 8);
            if (modelId < 0)
            {
                modelId = CTheScripts::UsedObjectArray[-modelId].Index;
            }
            CTheScripts::ScriptAttachAnimGroupToCharModel(modelId, TextLabel1);
            CTheScripts::AddToListOfSpecialAnimGroupsAttachedToCharModels(modelId, TextLabel1);
            return OR_CONTINUE;
        }
        case COMMAND_SET_OBJECT_AS_STEALABLE:
        {
            CollectParameters(2);
            CObject* pObject = CPools::GetObjectPool().GetAt(ScriptParams[0]);
            if (ScriptParams[1])
            {
                pObject->objectFlags.bIsLiftable = true;
            }
            else
            {
                pObject->objectFlags.bIsLiftable = false;
            }
            return OR_CONTINUE;
        }
        case COMMAND_SET_CREATE_RANDOM_GANG_MEMBERS:
            CollectParameters(1);
            CPopulation::m_bDontCreateRandomGangMembers = ScriptParams[0] == 0;
            return OR_CONTINUE;
        case COMMAND_ADD_SPARKS:
        {
            CollectParameters(7);
            TempVec.x = *reinterpret_cast<f32*>(&ScriptParams[0]);
            TempVec.y = *reinterpret_cast<f32*>(&ScriptParams[1]);
            TempVec.z = *reinterpret_cast<f32*>(&ScriptParams[2]);
            i32 nSparks = ScriptParams[6];
            CVector vecMoveSpeed(0.0f, 0.0f, 0.0f);
            CVector sparkDir;
            sparkDir = CVector(*reinterpret_cast<f32*>(&ScriptParams[3]), *reinterpret_cast<f32*>(&ScriptParams[4]),
                *reinterpret_cast<f32*>(&ScriptParams[5]));
            f32 sparkSpeed = sparkDir.NormaliseAndMag();
            g_fx.AddSparks(TempVec, sparkDir, sparkSpeed, nSparks, vecMoveSpeed, SPARK_PARTICLE_SPARK, 0.4f, 1.0f);
            return OR_CONTINUE;
        }
        case COMMAND_GET_VEHICLE_CLASS:
            CollectParameters(1);
            pVehicle = CPools::GetVehiclePool().GetAt(ScriptParams[0]);
            ScriptParams[0] = static_cast<CVehicleModelInfo*>(CModelInfo::GetModelInfo(pVehicle->GetModelIndex()))->GetVehicleList();
            StoreParameters(1);
            return OR_CONTINUE;
        case COMMAND_CLEAR_CONVERSATION_FOR_CHAR:
            CollectParameters(1);
            pPed = CPools::GetPedPool().GetAt(ScriptParams[0]);
            if (pPed)
            {
                CConversations::RemoveConversationForPed(pPed);
            }
            return OR_CONTINUE;
        case COMMAND_SET_MENU_ITEM_WITH_NUMBER:
        case COMMAND_SET_MENU_ITEM_WITH_2_NUMBERS:
        {
            CollectParameters(3);
            MenuIndex = ScriptParams[0];
            ColumnIndex = ScriptParams[1];
            RowIndex = ScriptParams[2];
            ReadTextLabelFromScript(TextLabel, 8);
            char pLabel[10];
            strcpy(pLabel, TextLabel);
            if (CurrCommand == COMMAND_SET_MENU_ITEM_WITH_NUMBER)
            {
                CollectParameters(1);
                CMenuSystem::InsertOneMenuItemWithNumber(static_cast<u8>(MenuIndex), static_cast<u8>(ColumnIndex), static_cast<u8>(RowIndex), pLabel,
                    ScriptParams[0], -1);
            }
            else if (CurrCommand == COMMAND_SET_MENU_ITEM_WITH_2_NUMBERS)
            {
                CollectParameters(2);
                CMenuSystem::InsertOneMenuItemWithNumber(static_cast<u8>(MenuIndex), static_cast<u8>(ColumnIndex), static_cast<u8>(RowIndex), pLabel,
                    ScriptParams[0], ScriptParams[1]);
            }
            return OR_CONTINUE;
        }
        case COMMAND_APPEND_TO_NEXT_CUTSCENE:
            ReadTextLabelFromScript(ModelName, 8);
            ReadTextLabelFromScript(AnimName, 8);
            CCutsceneMgr::AppendToNextCutscene(ModelName, AnimName);
            return OR_CONTINUE;
        case COMMAND_GET_NAME_OF_INFO_ZONE:
        {
            CollectParameters(3);
            TempVec.x = *reinterpret_cast<f32*>(&ScriptParams[0]);
            TempVec.y = *reinterpret_cast<f32*>(&ScriptParams[1]);
            TempVec.z = *reinterpret_cast<f32*>(&ScriptParams[2]);
            CZone* pZone = CTheZones::FindSmallestZoneForPosition(TempVec, true);
            pGlobalVar = GetPointerToScriptVariable(2);
            strncpy(reinterpret_cast<char*>(pGlobalVar), pZone->m_InfoLabel, 8);
            return OR_CONTINUE;
        }
        case COMMAND_VEHICLE_CAN_BE_TARGETTED_BY_HS_MISSILE:
            CollectParameters(2);
            pVehicle = CPools::GetVehiclePool().GetAt(ScriptParams[0]);
            if (ScriptParams[1])
            {
                pVehicle->vehicleFlags.bVehicleCanBeTargettedByHS = true;
            }
            else
            {
                pVehicle->vehicleFlags.bVehicleCanBeTargettedByHS = false;
            }
            return OR_CONTINUE;
        case COMMAND_SET_FREEBIES_IN_VEHICLE:
            CollectParameters(2);
            pVehicle = CPools::GetVehiclePool().GetAt(ScriptParams[0]);
            if (ScriptParams[1])
            {
                pVehicle->vehicleFlags.bFreebies = true;
            }
            else
            {
                pVehicle->vehicleFlags.bFreebies = false;
            }
            return OR_CONTINUE;
        case COMMAND_SET_SCRIPT_LIMIT_TO_GANG_SIZE:
        {
            CollectParameters(1);
            FindPlayerPed()->GetPlayerData()->m_nScriptLimitToGangSize = static_cast<u8>(ScriptParams[0]);
            i32 PlayerGroup = FindPlayerPed()->GetPlayerData()->m_PlayerGroup;
            i32 ToBeRemoved = CPedGroups::ms_groups[PlayerGroup].GetGroupMembership()->CountMembersExcludingLeader() -
                              FindPlayerPed()->GetPlayerData()->m_nScriptLimitToGangSize;
            if (ToBeRemoved > 0)
            {
                CPedGroups::ms_groups[PlayerGroup].GetGroupMembership()->RemoveNFollowers(ToBeRemoved);
            }
            return OR_CONTINUE;
        }
        case COMMAND_MAKE_PLAYER_GANG_DISAPPEAR:
            FindPlayerPed()->MakePlayerGroupDisappear();
            return OR_CONTINUE;
        case COMMAND_MAKE_PLAYER_GANG_REAPPEAR:
            FindPlayerPed()->MakePlayerGroupReappear();
            return OR_CONTINUE;
        case COMMAND_GET_CLOTHES_ITEM:
        {
            CollectParameters(2);
            i32 clothesId = ScriptParams[1];
            pPlayer = &CWorld::Players[ScriptParams[0]];
            i32 modelId = +CClothes::GetTextureDependency(static_cast<eClothesTexturePart>(clothesId));
            ScriptParams[0] = pPlayer->PlayerPedData.m_pClothes->GetTexture(clothesId);
            ScriptParams[1] = pPlayer->PlayerPedData.m_pClothes->m_anModelKeys[modelId];
            StoreParameters(2);
            return OR_CONTINUE;
        }
        case COMMAND_SHOW_UPDATE_STATS:
            CollectParameters(1);
            CStats::bShowUpdateStats = ScriptParams[0] != 0;
            return OR_CONTINUE;
        case COMMAND_IS_VAR_TEXT_LABEL16_EQUAL_TO_TEXT_LABEL:
            pGlobalVar = GetPointerToScriptVariable(2);
            ReadTextLabelFromScript(LongTextLabel, 16);
            LatestCmpFlagResult = false;
            if (strncmp(reinterpret_cast<char*>(pGlobalVar), LongTextLabel, 16) == 0)
            {
                LatestCmpFlagResult = true;
            }
            UpdateCompareFlag(LatestCmpFlagResult);
            return OR_CONTINUE;
        case COMMAND_IS_LVAR_TEXT_LABEL16_EQUAL_TO_TEXT_LABEL:
            pLocalVar = GetPointerToScriptVariable(1);
            ReadTextLabelFromScript(LongTextLabel, 16);
            LatestCmpFlagResult = false;
            if (strncmp(reinterpret_cast<char*>(pLocalVar), LongTextLabel, 16) == 0)
            {
                LatestCmpFlagResult = true;
            }
            UpdateCompareFlag(LatestCmpFlagResult);
            return OR_CONTINUE;
        case COMMAND_SET_COORD_BLIP_APPEARANCE:
            CollectParameters(2);
            CRadar::SetCoordBlipAppearance(ScriptParams[0], static_cast<eBlipAppearance>(ScriptParams[1]));
            return OR_CONTINUE;
    }
    return OR_INTERRUPT;
}

// MARK: 2300To2399

// func: sa 0x4762D0
OpcodeResult CRunningScript::ProcessCommands2300To2399(i32 CurrCommand)
{
    MARKFUNCTION(0x4762D0);

    bool LatestCmpFlagResult;
    CPed* pPed;
    CVehicle* pVehicle;
    CObject* pObject;
    CVector TempCoors;
    char TextLabel[12];
    f32 NewX, NewY, NewZ;

    // NOTE: Keep switch cases in strictly ascending order as in commands.hpp to preserve MSVC jump table layout
    switch (CurrCommand)
    {
        case COMMAND_GET_MENU_POSITION:
        {
            return OR_CONTINUE;
        }
        case COMMAND_SET_HEATHAZE_EFFECT:
        {
            CollectParameters(1);
            CPostEffects::ScriptHeatHazeFXSwitch(ScriptParams[0] != 0);
            return OR_CONTINUE;
        }
        case COMMAND_IS_HELP_MESSAGE_BEING_DISPLAYED:
        {
            LatestCmpFlagResult = CHud::HelpMessageDisplayed();
            UpdateCompareFlag(LatestCmpFlagResult);
            return OR_CONTINUE;
        }
        case COMMAND_HAS_OBJECT_BEEN_DAMAGED_BY_WEAPON:
        {
            CollectParameters(2);
            pObject = CPools::GetObjectPool().GetAt(ScriptParams[0]);
            LatestCmpFlagResult = false;
            if (pObject)
            {
                if (ScriptParams[1] == WEAPONTYPE_ANYMELEE || ScriptParams[1] == WEAPONTYPE_ANYWEAPON)
                {
                    if (CDarkel::CheckDamagedWeaponType((eWeaponType)pObject->m_nLastWeaponDamage, (eWeaponType)ScriptParams[1]))
                    {
                        LatestCmpFlagResult = true;
                    }
                }
                else if (pObject->m_nLastWeaponDamage == ScriptParams[1])
                {
                    LatestCmpFlagResult = true;
                }
            }
            UpdateCompareFlag(LatestCmpFlagResult);
            return OR_CONTINUE;
        }
        case COMMAND_CLEAR_OBJECT_LAST_WEAPON_DAMAGE:
        {
            CollectParameters(1);
            pObject = CPools::GetObjectPool().GetAt(ScriptParams[0]);
            if (pObject)
            {
                pObject->m_nLastWeaponDamage = -1;
            }
            return OR_CONTINUE;
        }
        case COMMAND_SET_PLAYER_JUMP_BUTTON:
        {
            CollectParameters(2);
            if (ScriptParams[1])
            {
                CPad::GetPad(ScriptParams[0])->bDisablePlayerJump = false;
            }
            else
            {
                CPad::GetPad(ScriptParams[0])->bDisablePlayerJump = true;
            }
            return OR_CONTINUE;
        }
        case COMMAND_SET_OBJECT_BEEN_PHOTOGRAPHED_FLAG:
        case COMMAND_SET_CHAR_BEEN_PHOTOGRAPHED_FLAG:
        {
            return OR_CONTINUE;
        }
        case COMMAND_GET_HUD_COLOUR:
        {
            CollectParameters(1);
            CRGBA Colour = HudColour.GetRGBA(static_cast<eHudColours>(ScriptParams[0]), 255);
            ScriptParams[1] = Colour.g;
            ScriptParams[0] = Colour.r;
            ScriptParams[2] = Colour.b;
            ScriptParams[3] = Colour.a;
            StoreParameters(4);
            return OR_CONTINUE;
        }
        case COMMAND_LOCK_DOOR:
        {
            CollectParameters(2);
            pObject = CPools::GetObjectPool().GetAt(ScriptParams[0]);
            if (ScriptParams[1])
            {
                pObject->LockDoor();
            }
            else
            {
                pObject->physicalFlags.bInfiniteMass = false;
                pObject->physicalFlags.bInfiniteMassFixed = false;
                pObject->SetTurnSpeed(CVector(0.0f, 0.0f, 0.0f));
                pObject->SetIsStatic(true);
            }
            return OR_CONTINUE;
        }
        case COMMAND_SET_OBJECT_MASS:
        {
            CollectParameters(2);
            CPools::GetObjectPool().GetAt(ScriptParams[0])->m_fMass = *reinterpret_cast<f32*>(&ScriptParams[1]);
            return OR_CONTINUE;
        }
        case COMMAND_GET_OBJECT_MASS:
        {
            CollectParameters(1);
            *reinterpret_cast<f32*>(&ScriptParams[0]) = CPools::GetObjectPool().GetAt(ScriptParams[0])->m_fMass;
            StoreParameters(1);
            return OR_CONTINUE;
        }
        case COMMAND_SET_OBJECT_TURN_MASS:
        {
            CollectParameters(2);
            CPools::GetObjectPool().GetAt(ScriptParams[0])->m_fTurnMass = *reinterpret_cast<f32*>(&ScriptParams[1]);
            return OR_CONTINUE;
        }
        case COMMAND_GET_OBJECT_TURN_MASS:
        {
            CollectParameters(1);
            *reinterpret_cast<f32*>(&ScriptParams[0]) = CPools::GetObjectPool().GetAt(ScriptParams[0])->m_fTurnMass;
            StoreParameters(1);
            return OR_CONTINUE;
        }
        case COMMAND_IS_PLAYBACK_FOR_CAR_PAUSED:
        case COMMAND_TRIGGER_PED_BOUNCE:
        {
            return OR_CONTINUE;
        }
        case COMMAND_SET_SPECIFIC_ZONE_TO_TRIGGER_GANG_WAR:
        {
            ReadTextLabelFromScript(TextLabel, 8);
            CGangWars::SetSpecificZoneToTriggerGangWar(CTheZones::FindZoneByLabelAndReturnIndex(TextLabel, ZONE_INFORMATION));
            return OR_CONTINUE;
        }
        case COMMAND_CLEAR_SPECIFIC_ZONES_TO_TRIGGER_GANG_WAR:
        {
            CGangWars::ClearSpecificZonesToTriggerGangWar();
            return OR_CONTINUE;
        }
        case COMMAND_SET_ACTIVE_MENU_ITEM:
        {
            CollectParameters(2);
            CMenuSystem::SetActiveMenuItem(static_cast<u8>(ScriptParams[0]), static_cast<i8>(ScriptParams[1]));
            return OR_CONTINUE;
        }
        case COMMAND_MARK_STREAMED_SCRIPT_AS_NO_LONGER_NEEDED:
        {
            CollectParameters(1);
            ScriptParams[0] = CTheScripts::StreamedScripts.GetProperIndexFromIndexUsedByScript(static_cast<i16>(ScriptParams[0]));
            CStreaming::SetMissionDoesntRequireModel(SCMToModelId(ScriptParams[0]));
            return OR_CONTINUE;
        }
        case COMMAND_REMOVE_STREAMED_SCRIPT:
        {
            CollectParameters(1);
            ScriptParams[0] = CTheScripts::StreamedScripts.GetProperIndexFromIndexUsedByScript(static_cast<i16>(ScriptParams[0]));
            CStreaming::RemoveModel(SCMToModelId(ScriptParams[0]));
            return OR_CONTINUE;
        }
        case COMMAND_REGISTER_STREAMED_SCRIPT:
        {
            return OR_CONTINUE;
        }
        case COMMAND_SET_MESSAGE_FORMATTING:
        {
            CollectParameters(3);
            CTheScripts::MessageCentre = static_cast<i16>(ScriptParams[1]);
            CTheScripts::bUseMessageFormatting = ScriptParams[0] != 0;
            CTheScripts::MessageWidth = static_cast<i16>(ScriptParams[2]);
            return OR_CONTINUE;
        }
        case COMMAND_START_NEW_STREAMED_SCRIPT:
        {
            CollectParameters(1);
            ScriptParams[0] = CTheScripts::StreamedScripts.GetProperIndexFromIndexUsedByScript(static_cast<i16>(ScriptParams[0]));
            ReadParametersForNewlyStartedScript(CTheScripts::StreamedScripts.StartNewStreamedScript(ScriptParams[0]));
            return OR_CONTINUE;
        }
        case COMMAND_REGISTER_STREAMED_SCRIPT_INTERNAL:
        {
            CollectParameters(1);
            return OR_CONTINUE;
        }
        case COMMAND_SET_WEATHER_TO_APPROPRIATE_TYPE_NOW:
        {
            CWeather::SetWeatherToAppropriateTypeNow();
            return OR_CONTINUE;
        }
        case COMMAND_WINCH_CAN_PICK_OBJECT_UP:
        {
            CollectParameters(2);
            pObject = CPools::GetObjectPool().GetAt(ScriptParams[0]);
            if (ScriptParams[1])
            {
                pObject->objectFlags.bCanBeAttachedToMagnet = true;
            }
            else
            {
                pObject->objectFlags.bCanBeAttachedToMagnet = false;
            }
            return OR_CONTINUE;
        }
        case COMMAND_SWITCH_AUDIO_ZONE:
        {
            ReadTextLabelFromScript(TextLabel, 8);
            CollectParameters(1);
            CAudioZones::SwitchAudioZone(TextLabel, ScriptParams[0] != 0);
            return OR_CONTINUE;
        }
        case COMMAND_SET_CAR_ENGINE_ON:
        {
            CollectParameters(2);
            CPools::GetVehiclePool().GetAt(ScriptParams[0])->SetEngineOn(ScriptParams[1] != 0);
            return OR_CONTINUE;
        }
        case COMMAND_SET_CAR_LIGHTS_ON:
        {
            CollectParameters(2);
            pVehicle = CPools::GetVehiclePool().GetAt(ScriptParams[0]);
            pVehicle->vehicleFlags.bLightsOn = ScriptParams[1] != 0;
            return OR_CONTINUE;
        }
        case COMMAND_GET_LATEST_CONSOLE_COMMAND:
        {
            GetPointerToScriptVariable(2);
            return OR_CONTINUE;
        }
        case COMMAND_RESET_LATEST_CONSOLE_COMMAND:
        {
            return OR_CONTINUE;
        }
        case COMMAND_GET_USER_OF_CLOSEST_MAP_ATTRACTOR:
        {
            CollectParameters(5);
            i32 ModelIndex = ScriptParams[4];
            TempCoors = CVector(*reinterpret_cast<f32*>(&ScriptParams[0]), *reinterpret_cast<f32*>(&ScriptParams[1]), *reinterpret_cast<f32*>(&ScriptParams[2]));
            const f32 fRadius = *reinterpret_cast<f32*>(&ScriptParams[3]);
            ReadTextLabelFromScript(TextLabel, 8);
            CVector vecPos = TempCoors;
            if (ModelIndex == 0)
            {
                ModelIndex = -1;
            }
            else if (ModelIndex < 0)
            {
                ModelIndex = CTheScripts::UsedObjectArray[-ModelIndex].Index;
            }
            C2dEffect* pEffect = nullptr;
            CEntity* pEntity = nullptr;
            CAttractorScanner::GetClosestAttractorOfType(vecPos, fRadius, ModelIndex, 5, TextLabel, false, nullptr, pEffect, pEntity);
            ScriptParams[0] = -1;
            if (pEffect)
            {
                CPed* pUser = GetPedAttractorManager()->GetPedUsingEffect(reinterpret_cast<const C2dEffectPedAttractor*>(pEffect), pEntity);
                if (pUser)
                {
                    ScriptParams[0] = CPools::GetPedPool().GetIndex(pUser);
                }
            }
            StoreParameters(1);
            return OR_CONTINUE;
        }
        case COMMAND_SWITCH_ROADS_BACK_TO_ORIGINAL:
        {
            CollectParameters(6);
            f32 X1 = *reinterpret_cast<f32*>(&ScriptParams[0]);
            f32 Y1 = *reinterpret_cast<f32*>(&ScriptParams[1]);
            f32 Z1 = *reinterpret_cast<f32*>(&ScriptParams[2]);
            f32 X2 = *reinterpret_cast<f32*>(&ScriptParams[3]);
            f32 Y2 = *reinterpret_cast<f32*>(&ScriptParams[4]);
            f32 Z2 = *reinterpret_cast<f32*>(&ScriptParams[5]);
            f32 temp;
            if (X1 > X2)
            {
                temp = X1;
                X1 = X2;
                X2 = temp;
            }
            if (Y1 > Y2)
            {
                temp = Y2;
                Y2 = Y1;
                Y1 = temp;
            }
            if (Z1 > Z2)
            {
                temp = Z2;
                Z2 = Z1;
                Z1 = temp;
            }
            ThePaths.SwitchRoadsOffInArea(X1, X2, Y1, Y2, Z1, Z2, false, true, true);
            return OR_CONTINUE;
        }
        case COMMAND_SWITCH_PED_ROADS_BACK_TO_ORIGINAL:
        {
            CollectParameters(6);
            f32 X1 = *reinterpret_cast<f32*>(&ScriptParams[0]);
            f32 Y1 = *reinterpret_cast<f32*>(&ScriptParams[1]);
            f32 Z1 = *reinterpret_cast<f32*>(&ScriptParams[2]);
            f32 X2 = *reinterpret_cast<f32*>(&ScriptParams[3]);
            f32 Y2 = *reinterpret_cast<f32*>(&ScriptParams[4]);
            f32 Z2 = *reinterpret_cast<f32*>(&ScriptParams[5]);
            f32 temp;
            if (X1 > X2)
            {
                temp = X1;
                X1 = X2;
                X2 = temp;
            }
            if (Y1 > Y2)
            {
                temp = Y2;
                Y2 = Y1;
                Y1 = temp;
            }
            if (Z1 > Z2)
            {
                temp = Z2;
                Z2 = Z1;
                Z1 = temp;
            }
            ThePaths.SwitchPedRoadsOffInArea(X1, X2, Y1, Y2, Z1, Z2, false, true);
            return OR_CONTINUE;
        }
        case COMMAND_GET_PLANE_UNDERCARRIAGE_POSITION:
        {
            CollectParameters(1);
            CPlane* pPlane = static_cast<CPlane*>(CPools::GetVehiclePool().GetAt(ScriptParams[0]));
            *reinterpret_cast<f32*>(&ScriptParams[0]) = pPlane->m_fLandingGearStatus;
            StoreParameters(1);
            return OR_CONTINUE;
        }
        case COMMAND_CAMERA_SET_VECTOR_TRACK:
        {
            CollectParameters(8);
            CVector vecFrom(*reinterpret_cast<f32*>(&ScriptParams[0]), *reinterpret_cast<f32*>(&ScriptParams[1]), *reinterpret_cast<f32*>(&ScriptParams[2]));
            CVector vecTo(*reinterpret_cast<f32*>(&ScriptParams[3]), *reinterpret_cast<f32*>(&ScriptParams[4]), *reinterpret_cast<f32*>(&ScriptParams[5]));
            TheCamera.VectorTrackLinear(&vecTo, &vecFrom, static_cast<f32>(ScriptParams[6]), ScriptParams[7] != 0);
            return OR_CONTINUE;
        }
        case COMMAND_CAMERA_SET_SHAKE_SIMULATION:
        {
            return OR_CONTINUE;
        }
        case COMMAND_CAMERA_SET_LERP_FOV:
        {
            CollectParameters(4);
            TheCamera.LerpFOV(*reinterpret_cast<f32*>(&ScriptParams[0]), *reinterpret_cast<f32*>(&ScriptParams[1]), static_cast<f32>(ScriptParams[2]), ScriptParams[3] != 0);
            return OR_CONTINUE;
        }
        case COMMAND_SWITCH_AMBIENT_PLANES:
        {
            CollectParameters(1);
            CPlane::SwitchAmbientPlanes(ScriptParams[0] != 0);
            return OR_CONTINUE;
        }
        case COMMAND_SET_DARKNESS_EFFECT:
        {
            CollectParameters(2);
            CPostEffects::ScriptDarknessFilterSwitch(ScriptParams[0] != 0, ScriptParams[1]);
            return OR_CONTINUE;
        }
        case COMMAND_CAMERA_RESET_NEW_SCRIPTABLES:
        {
            TheCamera.InitialiseScriptableComponents();
            return OR_CONTINUE;
        }
        case COMMAND_GET_NUMBER_OF_INSTANCES_OF_STREAMED_SCRIPT:
        {
            CollectParameters(1);
            ScriptParams[0] = CTheScripts::StreamedScripts.GetNumUsersOfStreamedScript(CTheScripts::StreamedScripts.GetProperIndexFromIndexUsedByScript(static_cast<i16>(ScriptParams[0])));
            StoreParameters(1);
            return OR_CONTINUE;
        }
        case COMMAND_ALLOCATE_STREAMED_SCRIPT_TO_PED_GENERATOR:
        {
            return OR_CONTINUE;
        }
        case COMMAND_ALLOCATE_STREAMED_SCRIPT_TO_RANDOM_PED:
        {
            CollectParameters(3);
            const i16 ScriptIndex = CTheScripts::StreamedScripts.GetProperIndexFromIndexUsedByScript(static_cast<i16>(ScriptParams[0]));
            ScriptParams[0] = ScriptIndex;
            CTheScripts::ScriptsForBrains.AddNewScriptBrain(ScriptIndex, static_cast<i16>(ScriptParams[1]), static_cast<u16>(ScriptParams[2]), 0, -1, -1.0f);
            return OR_CONTINUE;
        }
        case COMMAND_ALLOCATE_STREAMED_SCRIPT_TO_OBJECT:
        {
            CollectParameters(5);
            const i16 ScriptIndex = CTheScripts::StreamedScripts.GetProperIndexFromIndexUsedByScript(static_cast<i16>(ScriptParams[0]));
            i16 ModelIndex = static_cast<i16>(ScriptParams[1]);
            ScriptParams[0] = ScriptIndex;
            if (ScriptParams[1] < 0)
            {
                ModelIndex = static_cast<i16>(CTheScripts::UsedObjectArray[-ScriptParams[1]].Index);
            }
            CTheScripts::ScriptsForBrains.AddNewScriptBrain(ScriptIndex, ModelIndex, static_cast<u16>(ScriptParams[2]), 1, static_cast<i8>(ScriptParams[4]), *reinterpret_cast<f32*>(&ScriptParams[3]));
            return OR_CONTINUE;
        }
        case COMMAND_SET_PLAYER_CAN_BE_DAMAGED:
        {
            return OR_CONTINUE;
        }
        case COMMAND_GET_GROUP_MEMBER:
        {
            CollectParameters(2);
            const i32 GroupIndex = CTheScripts::GetActualScriptThingIndex(ScriptParams[0], UNIQUE_SCRIPT_PEDGROUP);
            CPed* pMember = CPedGroups::ms_groups[GroupIndex].GetGroupMembership()->GetMember(ScriptParams[1]);
            i32 MemberIndex = -1;
            if (pMember)
            {
                MemberIndex = CPools::GetPedPool().GetIndex(pMember);
            }
            ScriptParams[0] = MemberIndex;
            StoreParameters(1);
            return OR_CONTINUE;
        }
        case COMMAND_GET_PLAYERS_GANG_IN_CAR_ACTIVE:
        case COMMAND_SET_PLAYERS_GANG_IN_CAR_ACTIVE:
        {
            return OR_CONTINUE;
        }
        case COMMAND_GET_WATER_HEIGHT_AT_COORDS:
        {
            CollectParameters(3);
            f32 fWaterLevel;
            bool bFound;
            if (ScriptParams[2])
            {
                bFound = CWaterLevel::GetWaterLevel(*reinterpret_cast<f32*>(&ScriptParams[0]), *reinterpret_cast<f32*>(&ScriptParams[1]), 0.0f, &fWaterLevel, true, nullptr);
            }
            else
            {
                bFound = CWaterLevel::GetWaterLevelNoWaves(CVector(*reinterpret_cast<f32*>(&ScriptParams[0]), *reinterpret_cast<f32*>(&ScriptParams[1]), 0.0f), &fWaterLevel, nullptr, nullptr);
            }
            if (!bFound)
            {
                fWaterLevel = -1000.0f;
            }
            *reinterpret_cast<f32*>(&ScriptParams[0]) = fWaterLevel;
            StoreParameters(1);
            return OR_CONTINUE;
        }
        case COMMAND_CAMERA_PERSIST_TRACK:
        {
            CollectParameters(1);
            TheCamera.m_bCameraPersistTrack = ScriptParams[0] != 0;
            return OR_CONTINUE;
        }
        case COMMAND_CAMERA_PERSIST_POS:
        {
            CollectParameters(1);
            TheCamera.m_bCameraPersistPosition = ScriptParams[0] != 0;
            return OR_CONTINUE;
        }
        case COMMAND_CAMERA_PERSIST_FOV:
        {
            CollectParameters(1);
            TheCamera.m_bBlockZoom = ScriptParams[0] != 0;
            return OR_CONTINUE;
        }
        case COMMAND_CAMERA_IS_FOV_RUNNING:
        {
            return OR_CONTINUE;
        }
        case COMMAND_CAMERA_IS_VECTOR_MOVE_RUNNING:
        {
            UpdateCompareFlag(TheCamera.VectorMoveRunning());
            return OR_CONTINUE;
        }
        case COMMAND_CAMERA_IS_VECTOR_TRACK_RUNNING:
        {
            UpdateCompareFlag(TheCamera.VectorTrackRunning());
            return OR_CONTINUE;
        }
        case COMMAND_CAMERA_IS_SHAKE_RUNNING:
        {
            return OR_CONTINUE;
        }
        case COMMAND_CAMERA_SET_VECTOR_MOVE:
        {
            CollectParameters(8);
            CVector vecFrom(*reinterpret_cast<f32*>(&ScriptParams[0]), *reinterpret_cast<f32*>(&ScriptParams[1]), *reinterpret_cast<f32*>(&ScriptParams[2]));
            CVector vecTo(*reinterpret_cast<f32*>(&ScriptParams[3]), *reinterpret_cast<f32*>(&ScriptParams[4]), *reinterpret_cast<f32*>(&ScriptParams[5]));
            TheCamera.VectorMoveLinear(&vecTo, &vecFrom, static_cast<f32>(ScriptParams[6]), ScriptParams[7] != 0);
            return OR_CONTINUE;
        }
        case COMMAND_DRAW_WINDOW:
        {
            CollectParameters(4);
            const f32 X1 = *reinterpret_cast<f32*>(&ScriptParams[0]);
            const f32 Y1 = *reinterpret_cast<f32*>(&ScriptParams[1]);
            const f32 X2 = *reinterpret_cast<f32*>(&ScriptParams[2]);
            const f32 Y2 = *reinterpret_cast<f32*>(&ScriptParams[3]);
            char WindowTitle[12];
            ReadTextLabelFromScript(TextLabel, 8);
            WindowTitle[0] = '\0';
            if (_stricmp(TextLabel, "DUMMY") != 0)
            {
                strcpy(WindowTitle, TextLabel);
            }
            CollectParameters(1);
            const u16 RectIndex = CTheScripts::NumberOfIntroRectanglesThisFrame;
            const f32 fScaleX = RsGlobal.maximumWidth / 640.0f;
            const f32 fScaleY = RsGlobal.maximumHeight / 448.0f;
            tScriptRectangle& Rect = CTheScripts::IntroRectangles[RectIndex];
            Rect.eWindowType = WINDOW_HEADER_NO_TEXT;
            Rect.ScriptSpriteIndex = -1;
            Rect.ScriptRectMinX = X1 * fScaleX;
            Rect.ScriptRectMinY = Y1 * fScaleY;
            Rect.ScriptRectMaxX = fScaleX * X2;
            Rect.ScriptRectMaxY = fScaleY * Y2;
            Rect.ScriptRectRotation = 0.0f;
            Rect.ScriptRectColour = CRGBA(0, 0, 0, 190);
            strcpy(Rect.pTitle, WindowTitle);
            CTheScripts::NumberOfIntroRectanglesThisFrame++;
            Rect.gxt2[0] = 0;
            Rect.m_Alignment = static_cast<eFontAlignment>(1);
            Rect.m_nTextboxStyle = ScriptParams[0];
            return OR_CONTINUE;
        }
        case COMMAND_CLEAR_ALL_QUEUED_DIALOGUE:
        {
            return OR_CONTINUE;
        }
        case COMMAND_ATTACH_CAR_TO_OBJECT:
        {
            CollectParameters(8);
            pVehicle = CPools::GetVehiclePool().GetAt(ScriptParams[0]);
            pObject = CPools::GetObjectPool().GetAt(ScriptParams[1]);
            CVector vecOffset(*reinterpret_cast<f32*>(&ScriptParams[2]), *reinterpret_cast<f32*>(&ScriptParams[3]), *reinterpret_cast<f32*>(&ScriptParams[4]));
            CVector vecRotation(DEGTORAD(*reinterpret_cast<f32*>(&ScriptParams[5])), DEGTORAD(*reinterpret_cast<f32*>(&ScriptParams[6])), DEGTORAD(*reinterpret_cast<f32*>(&ScriptParams[7])));
            pVehicle->AttachEntityToEntity(pObject, vecOffset, vecRotation);
            return OR_CONTINUE;
        }
        case COMMAND_SET_GARAGE_RESPRAY_FREE:
        {
            ReadTextLabelFromScript(TextLabel, 8);
            CollectParameters(1);
            const i16 GarageIndex = CGarages::FindGarageIndex(TextLabel);
            if (GarageIndex >= 0)
            {
                CGarages::aGarages[GarageIndex].m_bRespraysAlwaysFree = ScriptParams[0] != 0;
            }
            return OR_CONTINUE;
        }
        case COMMAND_SET_CHAR_BULLETPROOF_VEST:
        {
            CollectParameters(2);
            pPed = CPools::GetPedPool().GetAt(ScriptParams[0]);
            pPed->bHasBulletProofVest = ScriptParams[1] != 0;
            return OR_CONTINUE;
        }
        case COMMAND_SET_ONSCREEN_COUNTER_COLOUR:
        {
            return OR_CONTINUE;
        }
        case COMMAND_SET_CINEMA_CAMERA:
        {
            CollectParameters(1);
            TheCamera.m_bCinemaCamera = ScriptParams[0] != 0;
            return OR_CONTINUE;
        }
        case COMMAND_SET_CHAR_FIRE_DAMAGE_MULTIPLIER:
        {
            CollectParameters(2);
            return OR_CONTINUE;
        }
        case COMMAND_IS_FIRE_BUTTON_PRESSED:
        {
            return OR_CONTINUE;
        }
        case COMMAND_SET_GROUP_FOLLOW_STATUS:
        {
            CollectParameters(2);
            const u32 GroupIndex = CTheScripts::GetActualScriptThingIndex(ScriptParams[0], UNIQUE_SCRIPT_PEDGROUP);
            if (GroupIndex < 8)
            {
                CPedGroups::ms_groups[GroupIndex].m_bMembersEnterLeadersVehicle = ScriptParams[1] != 0;
            }
            return OR_CONTINUE;
        }
        case COMMAND_SET_SEARCHLIGHT_CLIP_IF_COLLIDING:
        {
            CollectParameters(2);
            const i32 SearchLightIndex = CTheScripts::GetActualScriptThingIndex(ScriptParams[0], UNIQUE_SCRIPT_SEARCHLIGHT);
            if (SearchLightIndex >= 0)
            {
                if (ScriptParams[1])
                {
                    CTheScripts::ScriptSearchLightArray[SearchLightIndex].m_bClipIfColliding = true;
                }
                else
                {
                    CTheScripts::ScriptSearchLightArray[SearchLightIndex].m_bClipIfColliding = false;
                }
            }
            return OR_CONTINUE;
        }
        case COMMAND_HAS_PLAYER_BOUGHT_ITEM:
        {
            CollectParameters(1);
            UpdateCompareFlag(CShopping::HasPlayerBought(ScriptParams[0]));
            return OR_CONTINUE;
        }
        case COMMAND_SET_CAMERA_BEHIND_CHAR:
        {
            return OR_CONTINUE;
        }
        case COMMAND_SET_CAMERA_IN_FRONT_OF_CHAR:
        {
            CollectParameters(1);
            pPed = CPools::GetPedPool().GetAt(ScriptParams[0]);
            TheCamera.SetCameraDirectlyInFrontForFollowPed_ForAPed_CamOnAString(pPed);
            return OR_CONTINUE;
        }
        case COMMAND_GET_PLAYER_MAX_ARMOUR:
        {
            CollectParameters(1);
            ScriptParams[0] = CWorld::Players[ScriptParams[0]].MaxArmour;
            StoreParameters(1);
            return OR_CONTINUE;
        }
        case COMMAND_SET_CHAR_USES_UPPERBODY_DAMAGE_ANIMS_ONLY:
        {
            CollectParameters(2);
            pPed = CPools::GetPedPool().GetAt(ScriptParams[0]);
            pPed->bUpperBodyDamageAnimsOnly = ScriptParams[1] != 0;
            return OR_CONTINUE;
        }
        case COMMAND_SET_CHAR_SAY_CONTEXT:
        {
            CollectParameters(2);
            pPed = CPools::GetPedPool().GetAt(ScriptParams[0]);
            ScriptParams[0] = pPed->Say(static_cast<eGlobalSpeechContext>(ScriptParams[1]), 0, 1.0f, false, false, false);
            StoreParameters(1);
            return OR_CONTINUE;
        }
        case COMMAND_ADD_EXPLOSION_VARIABLE_SHAKE:
        {
            CollectParameters(5);
            TempCoors = CVector(*reinterpret_cast<f32*>(&ScriptParams[0]), *reinterpret_cast<f32*>(&ScriptParams[1]), *reinterpret_cast<f32*>(&ScriptParams[2]));
            CExplosion::AddExplosion(nullptr, nullptr, static_cast<eExplosionType>(ScriptParams[3]), TempCoors, 0, true, *reinterpret_cast<f32*>(&ScriptParams[4]), false);
            return OR_CONTINUE;
        }
        case COMMAND_ATTACH_MISSION_AUDIO_TO_CHAR:
        {
            CollectParameters(2);
            pPed = CPools::GetPedPool().GetAt(ScriptParams[1]);
            AudioEngine.AttachMissionAudioToPed(static_cast<u8>(ScriptParams[0] - 1), pPed);
            return OR_CONTINUE;
        }
        case COMMAND_UPDATE_PICKUP_MONEY_PER_DAY:
        {
            CollectParameters(2);
            CPickups::UpdateMoneyPerDay(ScriptParams[0], static_cast<u16>(ScriptParams[1]));
            return OR_CONTINUE;
        }
        case COMMAND_GET_NAME_OF_ENTRY_EXIT_CHAR_USED:
        {
            CollectParameters(1);
            pPed = CPools::GetPedPool().GetAt(ScriptParams[0]);
            char* pString = reinterpret_cast<char*>(GetPointerToScriptVariable(2));
            if (pPed->m_pEnex->m_szName)
            {
                strcpy(pString, pPed->m_pEnex->m_szName);
            }
            else
            {
                pString[0] = '\0';
            }
            return OR_CONTINUE;
        }
        case COMMAND_GET_POSITION_OF_ENTRY_EXIT_CHAR_USED:
        {
            CollectParameters(1);
            CEntryExit* pEntryExit = CPools::GetPedPool().GetAt(ScriptParams[0])->m_pEnex;
            TempCoors = CVector(0.0f, 0.0f, 0.0f);
            f32 fHeading = 0.0f;
            if (pEntryExit)
            {
                if (pEntryExit->m_pLink)
                {
                    TempCoors = pEntryExit->m_vecExitPos;
                    fHeading = DEGTORAD(pEntryExit->m_fExitAngle);
                }
                else
                {
                    TempCoors = CVector(pEntryExit->m_recEntrance.GetCenter(), pEntryExit->m_fEntranceZ);
                }
                *reinterpret_cast<f32*>(&ScriptParams[0]) = TempCoors.x;
                *reinterpret_cast<f32*>(&ScriptParams[1]) = TempCoors.y;
                *reinterpret_cast<f32*>(&ScriptParams[2]) = TempCoors.z;
                *reinterpret_cast<f32*>(&ScriptParams[3]) = fHeading;
            }
            StoreParameters(4);
            return OR_CONTINUE;
        }
        case COMMAND_IS_CHAR_TALKING:
        {
            CollectParameters(1);
            pPed = CPools::GetPedPool().GetAt(ScriptParams[0]);
            LatestCmpFlagResult = pPed->GetPedTalking();
            UpdateCompareFlag(LatestCmpFlagResult);
            return OR_CONTINUE;
        }
        case COMMAND_DISABLE_CHAR_SPEECH:
        {
            CollectParameters(2);
            pPed = CPools::GetPedPool().GetAt(ScriptParams[0]);
            pPed->DisablePedSpeech(static_cast<i16>(ScriptParams[1]));
            return OR_CONTINUE;
        }
        case COMMAND_ENABLE_CHAR_SPEECH:
        {
            CollectParameters(1);
            pPed = CPools::GetPedPool().GetAt(ScriptParams[0]);
            pPed->EnablePedSpeech();
            return OR_CONTINUE;
        }
        case COMMAND_SET_UP_SKIP:
        {
            CollectParameters(4);
            TempCoors = CVector(*reinterpret_cast<f32*>(&ScriptParams[0]), *reinterpret_cast<f32*>(&ScriptParams[1]), *reinterpret_cast<f32*>(&ScriptParams[2]));
            CGameLogic::SetUpSkip(TempCoors, *reinterpret_cast<f32*>(&ScriptParams[3]), false, nullptr, false);
            return OR_CONTINUE;
        }
        case COMMAND_CLEAR_SKIP:
        {
            CGameLogic::ClearSkip(false);
            return OR_CONTINUE;
        }
        case COMMAND_PRELOAD_BEAT_TRACK:
        {
            CollectParameters(1);
            AudioEngine.PreloadBeatTrack(static_cast<i16>(ScriptParams[0]));
            return OR_CONTINUE;
        }
        case COMMAND_GET_BEAT_TRACK_STATUS:
        {
            ScriptParams[0] = AudioEngine.GetBeatTrackStatus();
            StoreParameters(1);
            return OR_CONTINUE;
        }
        case COMMAND_PLAY_BEAT_TRACK:
        {
            AudioEngine.PlayPreloadedBeatTrack(false);
            return OR_CONTINUE;
        }
        case COMMAND_STOP_BEAT_TRACK:
        {
            AudioEngine.StopBeatTrack();
            return OR_CONTINUE;
        }
        case COMMAND_FIND_MAX_NUMBER_OF_GROUP_MEMBERS:
        {
            ScriptParams[0] = CStats::FindMaxNumberOfGroupMembers();
            StoreParameters(1);
            return OR_CONTINUE;
        }
        case COMMAND_VEHICLE_DOES_PROVIDE_COVER:
        {
            CollectParameters(2);
            pVehicle = CPools::GetVehiclePool().GetAt(ScriptParams[0]);
            pVehicle->vehicleFlags.bDoesProvideCover = ScriptParams[1] != 0;
            return OR_CONTINUE;
        }
        case COMMAND_CREATE_SNAPSHOT_PICKUP:
        case COMMAND_CREATE_HORSESHOE_PICKUP:
        case COMMAND_CREATE_OYSTER_PICKUP:
        {
            CollectParameters(3);
            NewX = *reinterpret_cast<f32*>(&ScriptParams[0]);
            NewY = *reinterpret_cast<f32*>(&ScriptParams[1]);
            NewZ = *reinterpret_cast<f32*>(&ScriptParams[2]);
            u32 ModelIndex;
            u8 PickupType;
            if (CurrCommand == COMMAND_CREATE_SNAPSHOT_PICKUP)
            {
                ModelIndex = MI_PICKUP_CAMERA;
                PickupType = PICKUP_SNAPSHOT;
                CStats::IncrementStat(TOTAL_SNAPSHOTS, 1.0f);
            }
            else
            {
                PickupType = PICKUP_ONCE;
                if (CurrCommand == COMMAND_CREATE_HORSESHOE_PICKUP)
                {
                    ModelIndex = MI_HORSESHOE;
                    CStats::IncrementStat(TOTAL_HORSESHOES, 1.0f);
                }
                else
                {
                    ModelIndex = MI_OYSTER;
                    CStats::IncrementStat(TOTAL_OYSTERS, 1.0f);
                }
            }
            if (NewZ <= -100.0f)
            {
                NewZ = CWorld::FindGroundZForCoord(NewX, NewY) + 0.5f;
            }
            ScriptParams[0] = CPickups::GenerateNewOne(CVector(NewX, NewY, NewZ), ModelIndex, PickupType, 0, 0, false, nullptr);
            StoreParameters(1);
            return OR_CONTINUE;
        }
        case COMMAND_HAS_OBJECT_BEEN_UPROOTED:
        {
            CollectParameters(1);
            pObject = CPools::GetObjectPool().GetAt(ScriptParams[0]);
            LatestCmpFlagResult = !pObject->m_bIsStatic;
            UpdateCompareFlag(LatestCmpFlagResult);
            return OR_CONTINUE;
        }
        case COMMAND_ADD_SMOKE_PARTICLE:
        {
            CollectParameters(12);
            RwV3d vecPos = {*reinterpret_cast<f32*>(&ScriptParams[0]), *reinterpret_cast<f32*>(&ScriptParams[1]), *reinterpret_cast<f32*>(&ScriptParams[2])};
            RwV3d vecVel = {*reinterpret_cast<f32*>(&ScriptParams[3]), *reinterpret_cast<f32*>(&ScriptParams[4]), *reinterpret_cast<f32*>(&ScriptParams[5])};
            FxPrtMult_c PrtMult(*reinterpret_cast<f32*>(&ScriptParams[6]), *reinterpret_cast<f32*>(&ScriptParams[7]), *reinterpret_cast<f32*>(&ScriptParams[8]), *reinterpret_cast<f32*>(&ScriptParams[9]), *reinterpret_cast<f32*>(&ScriptParams[10]), 1.0f, *reinterpret_cast<f32*>(&ScriptParams[11]));
            g_fx.m_SmokeHuge->AddParticle(vecPos, vecVel, 0.0f, PrtMult, -1.0f, 1.2f, 0.6f, 0);
            return OR_CONTINUE;
        }
        case COMMAND_IS_CHAR_STUCK_UNDER_CAR:
        {
            CollectParameters(1);
            pPed = CPools::GetPedPool().GetAt(ScriptParams[0]);
            UpdateCompareFlag(pPed->bStuckUnderCar);
            return OR_CONTINUE;
        }
        case COMMAND_CONTROL_CAR_DOOR:
        {
            CollectParameters(4);
            CAutomobile* pAutomobile = static_cast<CAutomobile*>(CPools::GetVehiclePool().GetAt(ScriptParams[0]));
            const f32 fOpenRatio = *reinterpret_cast<f32*>(&ScriptParams[3]);
            const eDoors Door = static_cast<eDoors>(ScriptParams[1]);
            const u32 DoorStatus = ScriptParams[2];
            if (ScriptParams[1] >= MAX_DOORS)
            {
                if (fOpenRatio >= 0.0f)
                {
                    pAutomobile->m_swingingChassis.Open(fOpenRatio);
                }
                pAutomobile->m_swingingChassis.m_doorState = static_cast<eDoorState>(DoorStatus);
            }
            else
            {
                if (fOpenRatio >= 0.0f)
                {
                    pAutomobile->m_doors[Door].Open(fOpenRatio);
                }
                pAutomobile->Damage.SetDoorStatus(Door, static_cast<eDoorStatus>(DoorStatus));
                pAutomobile->SetDoorDamage(Door, false);
            }
            return OR_CONTINUE;
        }
        case COMMAND_GET_DOOR_ANGLE_RATIO:  // 0x95F
        {
            // vehicle handle, doorID
            CollectParameters(2);
            auto* pAutomobile = static_cast<CAutomobile*>(CPools::GetVehiclePool().GetAt(ScriptParams[0]));
            const eDoors iDoor = static_cast<eDoors>(ScriptParams[1]);
            if (iDoor < MAX_DOORS)
            {
                *reinterpret_cast<f32*>(&ScriptParams[0]) = pAutomobile->m_doors[iDoor].GetDoorAngle();
            }
            else
            {
                *reinterpret_cast<f32*>(&ScriptParams[0]) = pAutomobile->m_swingingChassis.GetDoorAngle();
            }
            StoreParameters(1);
            return OR_CONTINUE;
        }
        default:
            return OR_INTERRUPT;
    }
}

// func: sa 0x477DA0
bool IsVecIn2dAngledArea(CVector& vecPos, f32 TargetX1, f32 TargetY1, f32 TargetX2, f32 TargetY2, f32 DistanceFrom1To4)
{
    MARKFUNCTION(0x477DA0);

    f32 RadiansBetweenFirstTwoPoints = CGeneral::GetRadianAngleBetweenPoints(TargetX1, TargetY1, TargetX2, TargetY2);
    f32 RadiansBetweenPoints1and4 = RadiansBetweenFirstTwoPoints + HALF_PI;
    while (RadiansBetweenPoints1and4 < 0.0f)
    {
        RadiansBetweenPoints1and4 += TWO_PI;
    }
    while (RadiansBetweenPoints1and4 > TWO_PI)
    {
        RadiansBetweenPoints1and4 -= TWO_PI;
    }

    f32 TargetX3 = TargetX2 + std::sin(RadiansBetweenPoints1and4) * DistanceFrom1To4;
    f32 TargetY3 = TargetY2 - std::cos(RadiansBetweenPoints1and4) * DistanceFrom1To4;
    f32 TargetX4 = TargetX1 + std::sin(RadiansBetweenPoints1and4) * DistanceFrom1To4;
    f32 TargetY4 = TargetY1 - std::cos(RadiansBetweenPoints1and4) * DistanceFrom1To4;

    CVector2D vec1To2(TargetX2 - TargetX1, TargetY2 - TargetY1);
    CVector2D vec1To4(TargetX4 - TargetX1, TargetY4 - TargetY1);
    f32 DistanceFrom1To2 = vec1To2.Magnitude();
    f32 DistanceFrom1To4Test = vec1To4.Magnitude();
    CVector2D vec1ToPos(vecPos.x - TargetX1, vecPos.y - TargetY1);

    vec1To2.Normalise();
    f32 TestDistance = DotProduct2D(vec1ToPos, vec1To2);
    bool IsWithinRange = false;
    if (TestDistance >= 0.0f && TestDistance <= DistanceFrom1To2)
    {
        vec1To4.Normalise();
        TestDistance = DotProduct2D(vec1ToPos, vec1To4);
        if (TestDistance >= 0.0f)
        {
            IsWithinRange = TestDistance <= DistanceFrom1To4Test;
        }
    }

    if (CTheScripts::DbgFlag)
    {
        CTheScripts::DrawDebugAngledSquare(CVector2D(TargetX1, TargetY1), CVector2D(TargetX2, TargetY2), CVector2D(TargetX3, TargetY3), CVector2D(TargetX4, TargetY4));
    }

    return IsWithinRange;
}

// MARK: 2400To2499

// func: sa 0x478000
OpcodeResult CRunningScript::ProcessCommands2400To2499(i32 CurrCommand)
{
    MARKFUNCTION(0x478000);

    bool LatestCmpFlagResult;
    CPed* pPed;
    CVehicle* pVehicle;
    CObject* pObject;
    CVector TempCoors;
    char TextLabel[12];
    i32 ScriptThingIndex;
    f32 NewX, NewY, NewZ;

    switch (CurrCommand)
    {
        case COMMAND_SET_PLAYER_DISPLAY_VITAL_STATS_BUTTON:
        {
            CollectParameters(2);
            if (ScriptParams[1])
            {
                CPad::GetPad(ScriptParams[0])->bDisablePlayerDisplayVitalStats = false;
            }
            else
            {
                CPad::GetPad(ScriptParams[0])->bDisablePlayerDisplayVitalStats = true;
            }
            return OR_CONTINUE;
        }
        case COMMAND_SET_CHAR_KEEP_TASK:
        {
            CollectParameters(2);
            pPed = CPools::GetPedPool().GetAt(ScriptParams[0]);
            pPed->bKeepTasksAfterCleanUp = ScriptParams[1] != 0;
            return OR_CONTINUE;
        }
        case COMMAND_DOES_CAR_HAVE_ROOF:
        case COMMAND_SET_BLIP_FADE:
        {
            return OR_CONTINUE;
        }
        case COMMAND_CREATE_MENU_GRID:
        {
            char MenuTitle[12];
            ReadTextLabelFromScript(TextLabel, 8);
            MenuTitle[0] = '\0';
            if (_stricmp(TextLabel, "DUMMY") != 0)
            {
                strcpy(MenuTitle, TextLabel);
            }
            CollectParameters(7);
            const f32 fScaleX = RsGlobal.maximumWidth / 640.0f;
            NewX = *reinterpret_cast<f32*>(&ScriptParams[0]) * fScaleX;
            NewZ = fScaleX * *reinterpret_cast<f32*>(&ScriptParams[2]);
            NewY = RsGlobal.maximumHeight / 448.0f * *reinterpret_cast<f32*>(&ScriptParams[1]);
            ScriptParams[0] = CMenuSystem::CreateNewMenu(CMenuSystem::MENU_TYPE_GRID, MenuTitle, NewX, NewY, NewZ, static_cast<u8>(ScriptParams[3]), static_cast<u8>(ScriptParams[4]), static_cast<u8>(ScriptParams[5]), static_cast<eFontAlignment>(ScriptParams[6]));
            StoreParameters(1);
            return OR_CONTINUE;
        }
        case COMMAND_IS_CHAR_SWIMMING:
        {
            CollectParameters(1);
            pPed = CPools::GetPedPool().GetAt(ScriptParams[0]);
            LatestCmpFlagResult = pPed->GetPedIntelligence()->GetTaskSwim() != nullptr;
            UpdateCompareFlag(LatestCmpFlagResult);
            return OR_CONTINUE;
        }
        case COMMAND_GET_CHAR_SWIM_STATE:
        {
            CollectParameters(1);
            pPed = CPools::GetPedPool().GetAt(ScriptParams[0]);
            ScriptParams[0] = pPed->GetPedIntelligence()->GetTaskSwim()->m_nSwimState;
            StoreParameters(1);
            return OR_CONTINUE;
        }
        case COMMAND_START_CHAR_FACIAL_TALK:
        {
            CollectParameters(2);
            pPed = CPools::GetPedPool().GetAt(ScriptParams[0]);
            static_cast<CTaskComplexFacial*>(pPed->GetPedIntelligence()->GetTaskManager().GetTaskSecondary(TASK_SECONDARY_FACIAL_COMPLEX))->SetRequest(eFacialExpression::TALKING, ScriptParams[1], eFacialExpression::NONE, 0);
            return OR_CONTINUE;
        }
        case COMMAND_STOP_CHAR_FACIAL_TALK:
        {
            CollectParameters(1);
            pPed = CPools::GetPedPool().GetAt(ScriptParams[0]);
            static_cast<CTaskComplexFacial*>(pPed->GetPedIntelligence()->GetTaskManager().GetTaskSecondary(TASK_SECONDARY_FACIAL_COMPLEX))->StopAll();
            return OR_CONTINUE;
        }
        case COMMAND_IS_BIG_VEHICLE:
        {
            CollectParameters(1);
            pVehicle = CPools::GetVehiclePool().GetAt(ScriptParams[0]);
            UpdateCompareFlag(pVehicle->vehicleFlags.bIsBig);
            return OR_CONTINUE;
        }
        case COMMAND_SWITCH_POLICE_HELIS:
        {
            CollectParameters(1);
            CHeli::SwitchPoliceHelis(ScriptParams[0] != 0);
            return OR_CONTINUE;
        }
        case COMMAND_STORE_CAR_MOD_STATE:
        {
            CShopping::StoreVehicleMods();
            return OR_CONTINUE;
        }
        case COMMAND_RESTORE_CAR_MOD_STATE:
        {
            CShopping::RestoreVehicleMods();
            return OR_CONTINUE;
        }
        case COMMAND_GET_CURRENT_CAR_MOD:
        {
            CollectParameters(2);
            pVehicle = CPools::GetVehiclePool().GetAt(ScriptParams[0]);
            i32 Upgrade = -1;
            switch (ScriptParams[1])
            {
                case 0:
                    Upgrade = pVehicle->GetUpgrade(0);
                    break;
                case 1:
                    Upgrade = pVehicle->GetUpgrade(1);
                    if (Upgrade == -1)
                    {
                        Upgrade = pVehicle->GetUpgrade(2);
                    }
                    break;
                case 2:
                    Upgrade = pVehicle->GetUpgrade(6);
                    break;
                case 3:
                    Upgrade = pVehicle->GetUpgrade(8);
                    if (Upgrade == -1)
                    {
                        Upgrade = pVehicle->GetUpgrade(9);
                    }
                    break;
                case 4:
                    Upgrade = pVehicle->GetUpgrade(10);
                    break;
                case 5:
                    Upgrade = pVehicle->GetUpgrade(11);
                    break;
                case 6:
                    Upgrade = pVehicle->GetUpgrade(12);
                    break;
                case 7:
                    Upgrade = pVehicle->GetUpgrade(14);
                    break;
                case 8:
                    Upgrade = pVehicle->GetUpgrade(15);
                    break;
                case 9:
                    Upgrade = pVehicle->GetUpgrade(16);
                    break;
                case 10:
                    Upgrade = pVehicle->GetUpgrade(17);
                    break;
                case 12:
                    Upgrade = pVehicle->GetReplacementUpgrade(2);
                    break;
                case 13:
                    Upgrade = pVehicle->GetReplacementUpgrade(19);
                    break;
                case 14:
                    Upgrade = pVehicle->GetReplacementUpgrade(12);
                    break;
                case 15:
                    Upgrade = pVehicle->GetReplacementUpgrade(13);
                    break;
                case 16:
                    Upgrade = pVehicle->GetReplacementUpgrade(20);
                    break;
            }
            ScriptParams[0] = Upgrade;
            StoreParameters(1);
            return OR_CONTINUE;
        }
        case COMMAND_IS_CAR_LOW_RIDER:
        {
            CollectParameters(1);
            pVehicle = CPools::GetVehiclePool().GetAt(ScriptParams[0]);
            UpdateCompareFlag(gHandlingDataMgr.GetVehiclePointer(static_cast<CVehicleModelInfo*>(CModelInfo::GetModelInfo(pVehicle->m_nModelIndex))->m_nHandlingId)->m_bLowRider);
            return OR_CONTINUE;
        }
        case COMMAND_IS_CAR_STREET_RACER:
        {
            CollectParameters(1);
            pVehicle = CPools::GetVehiclePool().GetAt(ScriptParams[0]);
            UpdateCompareFlag(gHandlingDataMgr.GetVehiclePointer(static_cast<CVehicleModelInfo*>(CModelInfo::GetModelInfo(pVehicle->m_nModelIndex))->m_nHandlingId)->m_bStreetRacer);
            return OR_CONTINUE;
        }
        case COMMAND_FORCE_DEATH_RESTART:
        {
            CGameLogic::ForceDeathRestart();
            return OR_CONTINUE;
        }
        case COMMAND_SYNC_WATER:
        {
            CWaterLevel::SyncWater();
            return OR_CONTINUE;
        }
        case COMMAND_SET_CHAR_COORDINATES_NO_OFFSET:
        {
            CollectParameters(4);
            pPed = CPools::GetPedPool().GetAt(ScriptParams[0]);
            SetCharCoordinates(pPed, *reinterpret_cast<f32*>(&ScriptParams[1]), *reinterpret_cast<f32*>(&ScriptParams[2]), *reinterpret_cast<f32*>(&ScriptParams[3]), true, false);
            return OR_CONTINUE;
        }
        case COMMAND_DOES_SCRIPT_FIRE_EXIST:
        {
            CollectParameters(1);
            ScriptThingIndex = CTheScripts::GetActualScriptThingIndex(ScriptParams[0], UNIQUE_SCRIPT_FIRE);
            LatestCmpFlagResult = false;
            if (ScriptThingIndex >= 0)
            {
                LatestCmpFlagResult = ScriptThingIndex < 60;
            }
            UpdateCompareFlag(LatestCmpFlagResult);
            return OR_CONTINUE;
        }
        case COMMAND_RESET_STUFF_UPON_RESURRECTION:
        {
            CGameLogic::ResetStuffUponResurrection();
            return OR_CONTINUE;
        }
        case COMMAND_IS_EMERGENCY_SERVICES_VEHICLE:
        {
            CollectParameters(1);
            pVehicle = CPools::GetVehiclePool().GetAt(ScriptParams[0]);
            LatestCmpFlagResult = false;
            if (pVehicle->IsLawEnforcementVehicle() || pVehicle->m_nModelIndex == MODEL_AMBULAN || pVehicle->m_nModelIndex == MODEL_FIRETRUK || pVehicle->m_nModelIndex == MODEL_FIRELA)
            {
                LatestCmpFlagResult = true;
            }
            UpdateCompareFlag(LatestCmpFlagResult);
            return OR_CONTINUE;
        }
        case COMMAND_KILL_FX_SYSTEM_NOW:
        {
            CollectParameters(1);
            const i32 UniqueIndex = ScriptParams[0];
            ScriptThingIndex = CTheScripts::GetActualScriptThingIndex(UniqueIndex, UNIQUE_SCRIPT_EFFECT_SYSTEM);
            if (ScriptThingIndex >= 0 && CTheScripts::ScriptEffectSystemArray[ScriptThingIndex].pFXSystem)
            {
                g_fxMan.DestroyFxSystem(CTheScripts::ScriptEffectSystemArray[ScriptThingIndex].pFXSystem);
                CTheScripts::RemoveScriptEffectSystem(UniqueIndex);
                if (IsThisAMissionScript)
                {
                    CTheScripts::MissionCleanUp.RemoveEntityFromList(UniqueIndex, CLEANUP_EFFECT_SYSTEM);
                }
            }
            return OR_CONTINUE;
        }
        case COMMAND_IS_OBJECT_WITHIN_BRAIN_ACTIVATION_RANGE:
        {
            CollectParameters(1);
            pObject = CPools::GetObjectPool().GetAt(ScriptParams[0]);
            LatestCmpFlagResult = false;
            if (CWorld::Players[CWorld::PlayerInFocus].m_pPed && pObject)
            {
                TempCoors = FindPlayerCentreOfWorld(CWorld::PlayerInFocus);
                LatestCmpFlagResult = CTheScripts::ScriptsForBrains.IsObjectWithinBrainActivationRange(pObject, TempCoors) != false;
            }
            UpdateCompareFlag(LatestCmpFlagResult);
            return OR_CONTINUE;
        }
        case COMMAND_COPY_SHARED_CHAR_DECISION_MAKER:
        {
            CollectParameters(1);
            i32 SourceIndex = -1;
            if (ScriptParams[0] != -1)
            {
                SourceIndex = CTheScripts::GetActualScriptThingIndex(ScriptParams[0], UNIQUE_SCRIPT_DECISION_MAKER);
            }
            const i32 DestUniqueIndex = CollectNextParameterWithoutIncreasingPC();
            if (CTheScripts::GetActualScriptThingIndex(DestUniqueIndex, UNIQUE_SCRIPT_DECISION_MAKER) == -1)
            {
                ScriptParams[0] = CTheScripts::GetNewUniqueScriptThingIndex(CDecisionMakerTypes::GetInstance()->CopyDecisionMaker(SourceIndex, 0, IsThisAMissionScript), UNIQUE_SCRIPT_DECISION_MAKER);
                if (IsThisAMissionScript)
                {
                    CTheScripts::MissionCleanUp.AddEntityToList(ScriptParams[0], CLEANUP_DECISION_MAKER);
                }
            }
            else
            {
                ScriptParams[0] = DestUniqueIndex;
            }
            CTheScripts::ScriptResourceManager.AddToResourceManager(ScriptParams[0], 3, this);
            StoreParameters(1);
            return OR_CONTINUE;
        }
        case COMMAND_LOAD_SHARED_CHAR_DECISION_MAKER:
        {
            return OR_CONTINUE;
        }
        case COMMAND_REPORT_MISSION_AUDIO_EVENT_AT_POSITION:
        {
            CollectParameters(4);
            TempCoors = CVector(*reinterpret_cast<f32*>(&ScriptParams[0]), *reinterpret_cast<f32*>(&ScriptParams[1]), *reinterpret_cast<f32*>(&ScriptParams[2]));
            AudioEngine.ReportMissionAudioEvent(static_cast<u16>(ScriptParams[3]), TempCoors);
            return OR_CONTINUE;
        }
        case COMMAND_REPORT_MISSION_AUDIO_EVENT_AT_OBJECT:
        {
            CollectParameters(2);
            pObject = CPools::GetObjectPool().GetAt(ScriptParams[0]);
            AudioEngine.ReportMissionAudioEvent(static_cast<u16>(ScriptParams[1]), pObject);
            return OR_CONTINUE;
        }
        case COMMAND_ATTACH_MISSION_AUDIO_TO_OBJECT:
        {
            CollectParameters(2);
            pObject = CPools::GetObjectPool().GetAt(ScriptParams[1]);
            AudioEngine.AttachMissionAudioToObject(static_cast<u8>(ScriptParams[0] - 1), pObject);
            return OR_CONTINUE;
        }
        case COMMAND_GET_NUM_CAR_COLOURS:
        {
            CollectParameters(1);
            pVehicle = CPools::GetVehiclePool().GetAt(ScriptParams[0]);
            ScriptParams[0] = static_cast<CVehicleModelInfo*>(CModelInfo::GetModelInfo(pVehicle->m_nModelIndex))->m_nNumColorVariations;
            StoreParameters(1);
            return OR_CONTINUE;
        }
        case COMMAND_IS_POLICE_VEHICLE_IN_PURSUIT:
        case COMMAND_GET_CAR_COLOUR_FROM_MENU_INDEX:
        {
            return OR_CONTINUE;
        }
        case COMMAND_EXTINGUISH_FIRE_AT_POINT:
        {
            CollectParameters(4);
            TempCoors = CVector(*reinterpret_cast<f32*>(&ScriptParams[0]), *reinterpret_cast<f32*>(&ScriptParams[1]), *reinterpret_cast<f32*>(&ScriptParams[2]));
            gFireManager.ExtinguishPoint(TempCoors, *reinterpret_cast<f32*>(&ScriptParams[3]));
            return OR_CONTINUE;
        }
        case COMMAND_HAS_TRAIN_DERAILED:
        {
            CollectParameters(1);
            CTrain* pTrain = static_cast<CTrain*>(CPools::GetVehiclePool().GetAt(ScriptParams[0]));
            UpdateCompareFlag(pTrain->trainFlags.bNotOnARailRoad);
            return OR_CONTINUE;
        }
        case COMMAND_SET_CHAR_FORCE_DIE_IN_CAR:
        {
            CollectParameters(2);
            pPed = CPools::GetPedPool().GetAt(ScriptParams[0]);
            pPed->bForceDieInCar = ScriptParams[1];
            return OR_CONTINUE;
        }
        case COMMAND_SET_ONLY_CREATE_GANG_MEMBERS:
        {
            CollectParameters(1);
            CPopulation::m_bOnlyCreateRandomGangMembers = ScriptParams[0] == 0;
            return OR_CONTINUE;
        }
        case COMMAND_GET_OBJECT_MODEL:
        {
            CollectParameters(1);
            ScriptParams[0] = CPools::GetObjectPool().GetAt(ScriptParams[0])->m_nModelIndex;
            StoreParameters(1);
            return OR_CONTINUE;
        }
        case COMMAND_SET_CHAR_USES_COLLISION_CLOSEST_OBJECT_OF_TYPE:
        {
            CollectParameters(7);
            NewX = *reinterpret_cast<f32*>(&ScriptParams[0]);
            NewY = *reinterpret_cast<f32*>(&ScriptParams[1]);
            NewZ = *reinterpret_cast<f32*>(&ScriptParams[2]);
            if (NewZ <= -100.0f)
            {
                NewZ = CWorld::FindGroundZForCoord(NewX, NewY);
            }
            i32 ModelIndex = ScriptParams[4];
            const f32 fRadius = *reinterpret_cast<f32*>(&ScriptParams[3]);
            CEntity* pClosestEntity = nullptr;
            f32 fClosestDist = fRadius + fRadius;
            if (ModelIndex < 0)
            {
                ModelIndex = CTheScripts::UsedObjectArray[-ModelIndex].Index;
            }
            CVector vecCentre(NewX, NewY, NewZ);
            i16 NumObjects;
            CEntity* apObjects[16];
            CWorld::FindObjectsOfTypeInRange(ModelIndex, vecCentre, fRadius, true, &NumObjects, 16, apObjects, true, false, false, true, true);
            for (i32 i = 0; i < NumObjects; i++)
            {
                const f32 fDist = (apObjects[i]->GetPosition() - vecCentre).Magnitude();
                if (fDist < fClosestDist)
                {
                    fClosestDist = fDist;
                    pClosestEntity = apObjects[i];
                }
            }
            pPed = CPools::GetPedPool().GetAt(ScriptParams[6]);
            if (pClosestEntity)
            {
                pPed->m_pNOCollisionVehicle = ScriptParams[5] ? nullptr : pClosestEntity;
            }
            return OR_CONTINUE;
        }
        case COMMAND_CLEAR_ALL_SCRIPT_FIRE_FLAGS:
        {
            gFireManager.ClearAllScriptFireFlags();
            return OR_CONTINUE;
        }
        case COMMAND_GET_CAR_BLOCKING_CAR:
        {
            CollectParameters(1);
            i32 BlockingCarIndex = -1;
            CEntity* pObstructingEntity = CPools::GetVehiclePool().GetAt(ScriptParams[0])->AutoPilot.pObstructingEntity;
            if (pObstructingEntity && pObstructingEntity->GetIsTypeVehicle())
            {
                BlockingCarIndex = CPools::GetVehiclePool().GetIndex(static_cast<CVehicle*>(pObstructingEntity));
            }
            ScriptParams[0] = BlockingCarIndex;
            StoreParameters(1);
            return OR_CONTINUE;
        }
        case COMMAND_GET_CURRENT_VEHICLE_PAINTJOB:
        {
            CollectParameters(1);
            pVehicle = CPools::GetVehiclePool().GetAt(ScriptParams[0]);
            ScriptParams[0] = pVehicle->GetRemapIndex();
            StoreParameters(1);
            return OR_CONTINUE;
        }
        case COMMAND_SET_HELP_MESSAGE_BOX_SIZE:
        {
            CollectParameters(1);
            CHud::m_fHelpMessageBoxWidth = static_cast<f32>(ScriptParams[0]);
            return OR_CONTINUE;
        }
        case COMMAND_SET_GUNSHOT_SENSE_RANGE_FOR_RIOT2:
        {
            CollectParameters(1);
            CEventGunShot::ms_fGunShotSenseRangeForRiot2 = *reinterpret_cast<f32*>(&ScriptParams[0]);
            return OR_CONTINUE;
        }
        case COMMAND_STRING_CAT16:
        case COMMAND_STRING_CAT8:
        {
            char* pString1 = reinterpret_cast<char*>(GetPointerToScriptVariable(2));
            char* pString2 = reinterpret_cast<char*>(GetPointerToScriptVariable(2));
            const u32 TotalLength = strlen(pString1) + strlen(pString2);
            if (CurrCommand == COMMAND_STRING_CAT16 ? TotalLength < 16 : TotalLength < 8)
            {
                char* pDest = reinterpret_cast<char*>(GetPointerToScriptVariable(2));
                strcpy(pDest, pString1);
                strcat(pDest, pString2);
            }
            return OR_CONTINUE;
        }
        case COMMAND_GET_CAR_MOVING_COMPONENT_OFFSET:
        {
            CollectParameters(1);
            CAutomobile* pAutomobile = static_cast<CAutomobile*>(CPools::GetVehiclePool().GetAt(ScriptParams[0]));
            *reinterpret_cast<f32*>(&ScriptParams[0]) = pAutomobile->GetMovingCollisionOffset();
            StoreParameters(1);
            return OR_CONTINUE;
        }
        case COMMAND_SET_NAMED_ENTRY_EXIT_FLAG:
        {
            ReadTextLabelFromScript(TextLabel, 8);
            CollectParameters(2);
            CEntryExitManager::SetEntryExitFlag(TextLabel, ScriptParams[0], ScriptParams[1] != 0);
            return OR_CONTINUE;
        }
        case COMMAND_RADIANS_TO_DEGREES:
        case COMMAND_DEGREES_TO_RADIANS:
        {
            return OR_CONTINUE;
        }
        case COMMAND_PAUSE_CURRENT_BEAT_TRACK:
        {
            CollectParameters(1);
            AudioEngine.PauseBeatTrack(ScriptParams[0] != 0);
            return OR_CONTINUE;
        }
        case COMMAND_SET_PLAYER_CYCLE_WEAPON_BUTTON:
        {
            CollectParameters(2);
            if (ScriptParams[1])
            {
                CPad::GetPad(ScriptParams[0])->bDisablePlayerCycleWeapon = false;
            }
            else
            {
                CPad::GetPad(ScriptParams[0])->bDisablePlayerCycleWeapon = true;
            }
            return OR_CONTINUE;
        }
        case COMMAND_SET_CHAR_AIR_RESISTANCE_MULTIPLIER:
        {
            return OR_CONTINUE;
        }
        case COMMAND_MARK_ROAD_NODE_AS_DONT_WANDER:
        {
            CollectParameters(3);
            ThePaths.MarkRoadNodeAsDontWander(*reinterpret_cast<f32*>(&ScriptParams[0]), *reinterpret_cast<f32*>(&ScriptParams[1]), *reinterpret_cast<f32*>(&ScriptParams[2]));
            return OR_CONTINUE;
        }
        case COMMAND_UNMARK_ALL_ROAD_NODES_AS_DONT_WANDER:
        {
            ThePaths.UnMarkAllRoadNodesAsDontWander();
            return OR_CONTINUE;
        }
        case COMMAND_SET_CHECKPOINT_HEADING:
        {
            CollectParameters(2);
            const f32 fHeading = *reinterpret_cast<f32*>(&ScriptParams[1]);
            ScriptThingIndex = CTheScripts::GetActualScriptThingIndex(ScriptParams[0], UNIQUE_SCRIPT_CHECKPOINT);
            if (ScriptThingIndex >= 0 && CTheScripts::ScriptCheckpointArray[ScriptThingIndex].m_Checkpoint)
            {
                CCheckpoints::SetHeading(CTheScripts::ScriptCheckpointArray[ScriptThingIndex].m_Checkpoint->m_ID, fHeading);
            }
            return OR_CONTINUE;
        }
        case COMMAND_SET_MISSION_RESPECT_TOTAL:
        {
            CollectParameters(1);
            CStats::SetStatValue(RESPECT_MISSION_TOTAL, static_cast<f32>(ScriptParams[0]));
            return OR_CONTINUE;
        }
        case COMMAND_AWARD_PLAYER_MISSION_RESPECT:
        {
            CollectParameters(1);
            CStats::IncrementStat(RESPECT_MISSION, static_cast<f32>(ScriptParams[0]));
            return OR_CONTINUE;
        }
        case COMMAND_SET_PLAYER_FIRE_WITH_SHOULDER_BUTTON:
        {
            return OR_CONTINUE;
        }
        case COMMAND_SET_CAR_COLLISION:
        {
            CollectParameters(2);
            pVehicle = CPools::GetVehiclePool().GetAt(ScriptParams[0]);
            if (ScriptParams[1])
            {
                pVehicle->m_bUsesCollision = true;
                pVehicle->physicalFlags.bApplyGravity = true;
            }
            else
            {
                pVehicle->m_bUsesCollision = false;
                pVehicle->physicalFlags.bApplyGravity = false;
            }
            return OR_CONTINUE;
        }
        case COMMAND_CHANGE_PLAYBACK_TO_USE_AI:
        {
            CollectParameters(1);
            pVehicle = CPools::GetVehiclePool().GetAt(ScriptParams[0]);
            CVehicleRecording::ChangeCarPlaybackToUseAI(pVehicle);
            return OR_CONTINUE;
        }
        case COMMAND_CAMERA_SET_SHAKE_SIMULATION_SIMPLE:
        {
            CollectParameters(3);
            TheCamera.AddShakeSimple(static_cast<f32>(ScriptParams[1]), ScriptParams[0], *reinterpret_cast<f32*>(&ScriptParams[2]));
            return OR_CONTINUE;
        }
        case COMMAND_IS_NIGHT_VISION_ACTIVE:
        {
            UpdateCompareFlag(CPostEffects::m_bNightVision);
            return OR_CONTINUE;
        }
        case COMMAND_SET_CREATE_RANDOM_COPS:
        {
            CollectParameters(1);
            CPopulation::m_bDontCreateRandomCops = ScriptParams[0] == 0;
            return OR_CONTINUE;
        }
        case COMMAND_TASK_SET_IGNORE_WEAPON_RANGE_FLAG:
        {
            CollectParameters(2);
            if (ScriptParams[0] == -1)
            {
                CTask* pTask = new CTaskSimpleSetCharIgnoreWeaponRangeFlag(ScriptParams[1] != 0);
                GivePedScriptedTask(-1, pTask, CurrCommand);
            }
            else
            {
                pPed = CPools::GetPedPool().GetAt(ScriptParams[0]);
                pPed->bIgnoreWeaponRange = ScriptParams[1];
            }
            return OR_CONTINUE;
        }
        case COMMAND_TASK_PICK_UP_SECOND_OBJECT:
        {
            ScriptTaskPickUpObject(CurrCommand);
            return OR_CONTINUE;
        }
        case COMMAND_DROP_SECOND_OBJECT:
        {
            CollectParameters(2);
            pPed = CPools::GetPedPool().GetAt(ScriptParams[0]);
            const bool bAddImpulse = ScriptParams[1] != 0;
            CTaskManager* pTaskManager = &pPed->GetPedIntelligence()->GetTaskManager();
            if (pTaskManager->GetTaskSecondary(TASK_SECONDARY_ATTACK) && pTaskManager->GetTaskSecondary(TASK_SECONDARY_ATTACK)->GetTaskType() == CTaskTypes::TASK_SIMPLE_HOLD_ENTITY)
            {
                CTaskSimpleHoldEntity* pTaskHold = static_cast<CTaskSimpleHoldEntity*>(pTaskManager->GetTaskSecondary(TASK_SECONDARY_ATTACK));
                pTaskHold->DropEntity(pPed, bAddImpulse);
                pTaskHold->MakeAbortable(pPed, ABORT_PRIORITY_URGENT, nullptr);
            }
            return OR_CONTINUE;
        }
        case COMMAND_REMOVE_OBJECT_ELEGANTLY:
        {
            CollectParameters(1);
            pObject = CPools::GetObjectPool().GetAt(ScriptParams[0]);
            CTheScripts::CleanUpThisObject(pObject);
            if (pObject)
            {
                if (reinterpret_cast<RpClump*>(pObject->m_pRwObject))
                {
                    RpClumpForAllAtomics(reinterpret_cast<RpClump*>(pObject->m_pRwObject), CClumpModelInfo::SetAtomicRendererCB, reinterpret_cast<void*>(CVisibilityPlugins::RenderFadingClumpCB));
                }
                pObject->objectFlags.bFadingIn = true;
            }
            if (IsThisAMissionScript)
            {
                CTheScripts::MissionCleanUp.RemoveEntityFromList(ScriptParams[0], CLEANUP_OBJECT);
            }
            return OR_CONTINUE;
        }
        case COMMAND_DRAW_CROSSHAIR:
        {
            CollectParameters(1);
            CTheScripts::bDrawCrossHair = ScriptParams[0] ? eCrossHairType::FIXED_DRAW_CIRCLE : eCrossHairType::NONE;
            return OR_CONTINUE;
        }
        case COMMAND_SET_UP_CONVERSATION_NODE_WITH_SPEECH:
        {
            char NodeName[8];
            char LinkYes[12];
            char LinkNo[12];
            ReadTextLabelFromScript(NodeName, 8);
            ReadTextLabelFromScript(LinkYes, 8);
            ReadTextLabelFromScript(LinkNo, 8);
            CollectParameters(3);
            CConversations::SetUpConversationNode(NodeName, LinkYes, LinkNo, ScriptParams[0], ScriptParams[1], ScriptParams[2]);
            return OR_CONTINUE;
        }
        case COMMAND_SET_CCTV_EFFECT:
        {
            return OR_CONTINUE;
        }
        case COMMAND_SHOW_BLIPS_ON_ALL_LEVELS:
        {
            CollectParameters(1);
            CTheScripts::RadarShowBlipOnAllLevels = ScriptParams[0] != 0;
            return OR_CONTINUE;
        }
        case COMMAND_SET_CHAR_DRUGGED_UP:
        {
            CollectParameters(2);
            pPed = CPools::GetPedPool().GetAt(ScriptParams[0]);
            pPed->bDruggedUp = ScriptParams[1] != 0;
            return OR_CONTINUE;
        }
        case COMMAND_IS_CHAR_HEAD_MISSING:
        {
            CollectParameters(1);
            pPed = CPools::GetPedPool().GetAt(ScriptParams[0]);
            LatestCmpFlagResult = false;
            if (pPed && pPed->bRemoveHead)
            {
                LatestCmpFlagResult = pPed->m_nBodypartToRemove == PED_NODE_HEAD;
            }
            UpdateCompareFlag(LatestCmpFlagResult);
            return OR_CONTINUE;
        }
        case COMMAND_GET_HASH_KEY:
        {
            char LongTextLabel[16];
            ReadTextLabelFromScript(LongTextLabel, 15);
            ScriptParams[0] = CKeyGen::GetUppercaseKey(LongTextLabel);
            StoreParameters(1);
            return OR_CONTINUE;
        }
        case COMMAND_SET_UP_CONVERSATION_END_NODE_WITH_SPEECH:
        {
            ReadTextLabelFromScript(TextLabel, 8);
            CollectParameters(1);
            CConversations::SetUpConversationNode(TextLabel, nullptr, nullptr, ScriptParams[0], 0, 0);
            return OR_CONTINUE;
        }
        case COMMAND_RANDOM_PASSENGER_SAY:
        {
            CollectParameters(2);
            pVehicle = CPools::GetVehiclePool().GetAt(ScriptParams[0]);
            pPed = pVehicle->PickRandomPassenger();
            if (pPed)
            {
                pPed->Say(static_cast<eGlobalSpeechContext>(ScriptParams[1]), 0, 1.0f, false, false, false);
            }
            return OR_CONTINUE;
        }
        case COMMAND_HIDE_ALL_FRONTEND_BLIPS:
        {
            CollectParameters(1);
            CTheScripts::HideAllFrontEndMapBlips = ScriptParams[0] != 0;
            return OR_CONTINUE;
        }
        case COMMAND_SET_PLAYER_IN_CAR_CAMERA_MODE:
        {
            CollectParameters(1);
            if (ScriptParams[0] != 4)
            {
                TheCamera.m_nCarZoom = ScriptParams[0];
            }
            return OR_CONTINUE;
        }
        case COMMAND_IS_CHAR_IN_ANY_TRAIN:
        {
            CollectParameters(1);
            pPed = CPools::GetPedPool().GetAt(ScriptParams[0]);
            LatestCmpFlagResult = false;
            if (pPed->bInVehicle)
            {
                LatestCmpFlagResult = pPed->m_pMyVehicle->GetBaseVehicleType() == 6;
            }
            UpdateCompareFlag(LatestCmpFlagResult);
            return OR_CONTINUE;
        }
        case COMMAND_SET_UP_SKIP_AFTER_MISSION:
        {
            CollectParameters(4);
            TempCoors = CVector(*reinterpret_cast<f32*>(&ScriptParams[0]), *reinterpret_cast<f32*>(&ScriptParams[1]), *reinterpret_cast<f32*>(&ScriptParams[2]));
            CGameLogic::SetUpSkip(TempCoors, *reinterpret_cast<f32*>(&ScriptParams[3]), true, nullptr, false);
            return OR_CONTINUE;
        }
        case COMMAND_SET_VEHICLE_IS_CONSIDERED_BY_PLAYER:
        {
            CollectParameters(2);
            pVehicle = CPools::GetVehiclePool().GetAt(ScriptParams[0]);
            pVehicle->vehicleFlags.bConsideredByPlayer = ScriptParams[1] != 0;
            return OR_CONTINUE;
        }
        case COMMAND_GET_CPU_LEVEL:
        {
            return OR_CONTINUE;
        }
        case COMMAND_GET_RANDOM_CAR_MODEL_IN_MEMORY:
        {
            CollectParameters(1);
            const i32 ModelIndex = CPopulation::m_AppropriateLoadedCars.PickRandomCar(false, ScriptParams[0] != 0);
            ScriptParams[0] = ModelIndex;
            if (ModelIndex == -1)
            {
                ScriptParams[1] = -1;
            }
            else
            {
                ScriptParams[1] = static_cast<CVehicleModelInfo*>(CModelInfo::GetModelInfo(ModelIndex))->GetVehicleList();
            }
            StoreParameters(2);
            return OR_CONTINUE;
        }
        case COMMAND_GET_CAR_DOOR_LOCK_STATUS:
        {
            CollectParameters(1);
            ScriptParams[0] = CPools::GetVehiclePool().GetAt(ScriptParams[0])->m_eDoorLockState;
            StoreParameters(1);
            return OR_CONTINUE;
        }
        case COMMAND_SET_CLOSEST_ENTRY_EXIT_FLAG:
        {
            CollectParameters(5);
            CVector2D vecPos(*reinterpret_cast<f32*>(&ScriptParams[0]), *reinterpret_cast<f32*>(&ScriptParams[1]));
            const i32 EntryExitIndex = CEntryExitManager::FindNearestEntryExit(vecPos, *reinterpret_cast<f32*>(&ScriptParams[2]), -1);
            CEntryExitManager::SetEntryExitFlagWithIndex(EntryExitIndex, ScriptParams[3], ScriptParams[4] != 0);
            return OR_CONTINUE;
        }
        case COMMAND_SET_CHAR_SIGNAL_AFTER_KILL:
        {
            CollectParameters(2);
            pPed = CPools::GetPedPool().GetAt(ScriptParams[0]);
            pPed->bSignalAfterKill = ScriptParams[1] != 0;
            return OR_CONTINUE;
        }
        case COMMAND_SET_CHAR_WANTED_BY_POLICE:
        {
            CollectParameters(2);
            pPed = CPools::GetPedPool().GetAt(ScriptParams[0]);
            pPed->bWantedByPolice = ScriptParams[1] != 0;
            return OR_CONTINUE;
        }
        case COMMAND_SET_ZONE_NO_COPS:
        {
            ReadTextLabelFromScript(TextLabel, 8);
            CollectParameters(1);
            const i16 ZoneIndex = CTheZones::FindZoneByLabelAndReturnIndex(TextLabel, ZONE_INFORMATION);
            if (ZoneIndex >= 0)
            {
                CTheZones::ZoneInfoArray[CTheZones::NavigationZoneArray[ZoneIndex].ZoneInfoIndex].IsNoCops = ScriptParams[0];
            }
            return OR_CONTINUE;
        }
        case COMMAND_ADD_BLOOD:
        {
            CollectParameters(8);
            CVector vecPos(*reinterpret_cast<f32*>(&ScriptParams[0]), *reinterpret_cast<f32*>(&ScriptParams[1]), *reinterpret_cast<f32*>(&ScriptParams[2]));
            CVector vecDir(*reinterpret_cast<f32*>(&ScriptParams[3]), *reinterpret_cast<f32*>(&ScriptParams[4]), *reinterpret_cast<f32*>(&ScriptParams[5]));
            pPed = CPools::GetPedPool().GetAt(ScriptParams[7]);
            g_fx.AddBlood(vecPos, vecDir, ScriptParams[6], pPed->m_fContactSurfaceBrightness);
            return OR_CONTINUE;
        }
        case COMMAND_DISPLAY_CAR_NAMES:
        {
            CollectParameters(1);
            CHud::bScriptDontDisplayVehicleName = ScriptParams[0] == 0;
            return OR_CONTINUE;
        }
        case COMMAND_DISPLAY_ZONE_NAMES:
        {
            CollectParameters(1);
            CHud::bScriptDontDisplayAreaName = ScriptParams[0] == 0;
            return OR_CONTINUE;
        }
        case COMMAND_IS_CAR_DOOR_DAMAGED:
        {
            CollectParameters(2);
            CAutomobile* pAutomobile = static_cast<CAutomobile*>(CPools::GetVehiclePool().GetAt(ScriptParams[0]));
            LatestCmpFlagResult = pAutomobile->Damage.GetDoorStatus(static_cast<eDoors>(ScriptParams[1])) != 0;
            UpdateCompareFlag(LatestCmpFlagResult);
            return OR_CONTINUE;
        }
        case COMMAND_SET_CHAR_COORDINATES_DONT_WARP_GANG_NO_OFFSET:
        {
            CollectParameters(4);
            pPed = CPools::GetPedPool().GetAt(ScriptParams[0]);
            SetCharCoordinates(pPed, *reinterpret_cast<f32*>(&ScriptParams[1]), *reinterpret_cast<f32*>(&ScriptParams[2]), *reinterpret_cast<f32*>(&ScriptParams[3]), false, false);
            return OR_CONTINUE;
        }
        case COMMAND_SET_MINIGAME_IN_PROGRESS:
        {
            CollectParameters(1);
            if (ScriptParams[0])
            {
                bIsThisAMiniGameScript = true;
                CTheScripts::bMiniGameInProgress = true;
                CTheScripts::bDisplayNonMiniGameHelpMessages = false;
            }
            else
            {
                bIsThisAMiniGameScript = false;
                CTheScripts::bMiniGameInProgress = false;
                CTheScripts::bDisplayNonMiniGameHelpMessages = true;
            }
            return OR_CONTINUE;
        }
        case COMMAND_IS_MINIGAME_IN_PROGRESS:
        {
            UpdateCompareFlag(CTheScripts::bMiniGameInProgress);
            return OR_CONTINUE;
        }
        case COMMAND_SET_FORCE_RANDOM_CAR_MODEL:
        {
            CollectParameters(1);
            CTheScripts::ForceRandomCarModel = ScriptParams[0];
            return OR_CONTINUE;
        }
        case COMMAND_GET_RANDOM_CAR_OF_TYPE_IN_ANGLED_AREA_NO_SAVE:
        {
            CollectParameters(6);
            const f32 X1 = *reinterpret_cast<f32*>(&ScriptParams[0]);
            const f32 Y1 = *reinterpret_cast<f32*>(&ScriptParams[1]);
            const f32 X2 = *reinterpret_cast<f32*>(&ScriptParams[2]);
            const f32 Y2 = *reinterpret_cast<f32*>(&ScriptParams[3]);
            const f32 Distance = *reinterpret_cast<f32*>(&ScriptParams[4]);
            i32 FoundCarIndex = -1;
            i32 i = CPools::GetVehiclePool().GetSize();
            while (i--)
            {
                if (FoundCarIndex != -1)
                {
                    break;
                }
                pVehicle = CPools::GetVehiclePool().GetSlot(i);
                if (pVehicle && (pVehicle->GetVehicleAppearance() == APR_CAR || pVehicle->GetVehicleAppearance() == APR_BIKE) && (pVehicle->m_nModelIndex == ScriptParams[5] || ScriptParams[5] < 0) && pVehicle->CanBeDeleted())
                {
                    if (IsVecIn2dAngledArea(const_cast<CVector&>(pVehicle->GetPosition()), X1, Y1, X2, Y2, Distance))
                    {
                        FoundCarIndex = CPools::GetVehiclePool().GetIndex(pVehicle);
                    }
                }
            }
            ScriptParams[0] = FoundCarIndex;
            StoreParameters(1);
            return OR_CONTINUE;
        }
        case COMMAND_ADD_NEXT_MESSAGE_TO_PREVIOUS_BRIEFS:
        {
            CollectParameters(1);
            CTheScripts::bAddNextMessageToPreviousBriefs = ScriptParams[0];
            return OR_CONTINUE;
        }
        case COMMAND_FAIL_KILL_FRENZY:
        {
            CDarkel::FailKillFrenzy();
            return OR_CONTINUE;
        }
        case COMMAND_IS_COP_VEHICLE_IN_AREA_3D_NO_SAVE:
        {
            CollectParameters(6);
            f32 X1 = *reinterpret_cast<f32*>(&ScriptParams[0]);
            f32 Y1 = *reinterpret_cast<f32*>(&ScriptParams[1]);
            f32 Z1 = *reinterpret_cast<f32*>(&ScriptParams[2]);
            f32 X2 = *reinterpret_cast<f32*>(&ScriptParams[3]);
            f32 Y2 = *reinterpret_cast<f32*>(&ScriptParams[4]);
            f32 Z2 = *reinterpret_cast<f32*>(&ScriptParams[5]);
            f32 temp;
            if (X1 > X2)
            {
                temp = X1;
                X1 = X2;
                X2 = temp;
            }
            if (Y1 > Y2)
            {
                temp = Y2;
                Y2 = Y1;
                Y1 = temp;
            }
            if (Z1 > Z2)
            {
                temp = Z2;
                Z2 = Z1;
                Z1 = temp;
            }
            LatestCmpFlagResult = false;
            const i32 PoolSize = CPools::GetVehiclePool().GetSize();
            for (i32 i = 0; i < PoolSize; i++)
            {
                pVehicle = CPools::GetVehiclePool().GetSlot(i);
                if (pVehicle && pVehicle->IsLawEnforcementVehicle() && pVehicle->m_nModelIndex != MODELID_BOAT_PREDATOR)
                {
                    TempCoors = pVehicle->GetPosition();
                    if (TempCoors.x >= X1 && TempCoors.x <= X2 && TempCoors.y >= Y1 && TempCoors.y <= Y2 && TempCoors.z >= Z1 && TempCoors.z <= Z2)
                    {
                        LatestCmpFlagResult = true;
                    }
                }
            }
            UpdateCompareFlag(LatestCmpFlagResult);
            return OR_CONTINUE;
        }
        default:
            return OR_INTERRUPT;
    }
}

// MARK: 2500To2599

// func: sa 0x47A760
OpcodeResult CRunningScript::ProcessCommands2500To2599(i32 CurrCommand)
{
    MARKFUNCTION(0x47A760);

    bool LatestCmpFlagResult;
    CVehicle* pVehicle = nullptr;
    CPed* pPed;
    CObject* pObject;
    CVector vec;
    u8 new_car_colour;
    i32 iDecisionMakerIndex;

    // NOTE: Keep switch cases in strictly ascending order as in commands.hpp to preserve MSVC jump table layout
    switch (CurrCommand)
    {
        case COMMAND_SET_PETROL_TANK_WEAKPOINT:
            CollectParameters(2);
            pVehicle = CPools::GetVehiclePool().GetAt(ScriptParams[0]);
            pVehicle->vehicleFlags.bPetrolTankIsWeakPoint = ScriptParams[1];
            return OR_CONTINUE;
        case COMMAND_IS_CHAR_USING_MAP_ATTRACTOR:
            CollectParameters(1);
            pPed = CPools::GetPedPool().GetAt(ScriptParams[0]);
            LatestCmpFlagResult = GetPedAttractorManager()->IsPedRegisteredWithEffect(pPed);
            UpdateCompareFlag(LatestCmpFlagResult);
            return OR_CONTINUE;
        case COMMAND_SET_ALL_CARS_IN_AREA_VISIBLE:
            return OR_CONTINUE;
        case COMMAND_SET_PLAYER_MODEL:
        {
            CollectParameters(2);
            pPed = CWorld::Players[ScriptParams[0]].m_pPed;
            i32 modelID = ScriptParams[1];
            i32 animGrp = pPed->m_motionAnimGroup;
            pPed->DeleteRwObject();
            pPed->m_nModelIndex = -1;
            pPed->SetModelIndex(modelID);
            pPed->m_motionAnimGroup = static_cast<AssocGroupId>(animGrp);
            return OR_CONTINUE;
        }
        case COMMAND_ARE_SUBTITLES_SWITCHED_ON:
            UpdateCompareFlag(FrontEndMenuManager.m_PrefsShowSubtitles);
            return OR_CONTINUE;
        case COMMAND_REMOVE_CHAR_FROM_CAR_MAINTAIN_POSITION:
        {
            CollectParameters(2);
            pPed = CPools::GetPedPool().GetAt(ScriptParams[0]);
            pVehicle = CPools::GetVehiclePool().GetAt(ScriptParams[1]);
            const CVector vStartPos = pPed->GetPosition();
            const i32 iTargetDoor = CCarEnterExit::ComputeTargetDoorToExit(pVehicle, pPed);
            {
                CTaskSimpleCarSetPedOut task(pVehicle, static_cast<eTargetDoor>(iTargetDoor), false);
                task.ProcessPed(pPed);
                pPed->SetPosition(vStartPos);
            }
            return OR_CONTINUE;
        }
        case COMMAND_SET_OBJECT_PROOFS:
            CollectParameters(6);
            pObject = CPools::GetObjectPool().GetAt(ScriptParams[0]);
            pObject->physicalFlags.bNotDamagedByBullets = ScriptParams[1];
            pObject->physicalFlags.bNotDamagedByFlames = ScriptParams[2];
            pObject->physicalFlags.bIgnoresExplosions = ScriptParams[3];
            pObject->physicalFlags.bNotDamagedByCollisions = ScriptParams[4];
            pObject->physicalFlags.bNotDamagedByMelee = ScriptParams[5];
            return OR_CONTINUE;
        case COMMAND_IS_CAR_TOUCHING_CAR:
        {
            CollectParameters(2);
            pVehicle = CPools::GetVehiclePool().GetAt(ScriptParams[0]);
            CVehicle* pVehicle2 = CPools::GetVehiclePool().GetAt(ScriptParams[1]);
            LatestCmpFlagResult = pVehicle->GetHasCollidedWith(pVehicle2);
            UpdateCompareFlag(LatestCmpFlagResult);
            return OR_CONTINUE;
        }
        case COMMAND_DOES_OBJECT_HAVE_THIS_MODEL:
        {
            CollectParameters(2);
            pObject = CPools::GetObjectPool().GetAt(ScriptParams[0]);
            i32 ModelIndex = ScriptParams[1];
            if (ModelIndex < 0)
            {
                i32 ArrayIndex = -ModelIndex;
                ModelIndex = CTheScripts::UsedObjectArray[ArrayIndex].Index;
            }
            UpdateCompareFlag(pObject->GetModelIndex() == ModelIndex);
            return OR_CONTINUE;
        }
        case COMMAND_SET_TRAIN_FORCED_TO_SLOW_DOWN:
            CollectParameters(2);
            pVehicle = CPools::GetVehiclePool().GetAt(ScriptParams[0]);
            static_cast<CTrain*>(pVehicle)->trainFlags.bForceSlowDown = (ScriptParams[1] != 0);
            return OR_CONTINUE;
        case COMMAND_IS_VEHICLE_ON_ALL_WHEELS:
            CollectParameters(1);
            pVehicle = CPools::GetVehiclePool().GetAt(ScriptParams[0]);
            LatestCmpFlagResult = false;
            if (pVehicle->GetBaseVehicleType() == VEHICLE_TYPE_BIKE)
            {
                if (static_cast<CBike*>(pVehicle)->m_nNoOfContactWheels == 4)
                {
                    LatestCmpFlagResult = true;
                }
            }
            if (pVehicle->GetBaseVehicleType() == VEHICLE_TYPE_CAR && static_cast<CAutomobile*>(pVehicle)->m_nNumContactWheels == 4)
            {
                LatestCmpFlagResult = true;
            }
            UpdateCompareFlag(LatestCmpFlagResult);
            return OR_CONTINUE;
        case COMMAND_DOES_PICKUP_EXIST:
            CollectParameters(1);
            LatestCmpFlagResult = (CPickups::GetActualPickupIndex(ScriptParams[0]) != -1);
            UpdateCompareFlag(LatestCmpFlagResult);
            return OR_CONTINUE;
        case COMMAND_ENABLE_AMBIENT_CRIME:
            CollectParameters(1);
            if (ScriptParams[0])
            {
                g_LoadMonitor.EnableAmbientCrime();
            }
            else
            {
                g_LoadMonitor.DisableAmbientCrime();
            }
            return OR_CONTINUE;
        case COMMAND_IS_AMBIENT_CRIME_ENABLED:
            return OR_CONTINUE;
        case COMMAND_CLEAR_WANTED_LEVEL_IN_GARAGE:
            FindPlayerWanted()->ClearWantedLevelAndGoOnParole();
            return OR_CONTINUE;
        case COMMAND_SET_CHAR_SAY_CONTEXT_IMPORTANT:
            CollectParameters(5);
            pPed = CPools::GetPedPool().GetAt(ScriptParams[0]);
            ScriptParams[0] = pPed->Say(static_cast<eGlobalSpeechContext>(ScriptParams[1]), 0, 1.0f, static_cast<bool>(ScriptParams[2] != 0),
                static_cast<bool>(ScriptParams[3] != 0), static_cast<bool>(ScriptParams[4] != 0));
            StoreParameters(1);
            return OR_CONTINUE;
        case COMMAND_SET_CHAR_SAY_SCRIPT:
            CollectParameters(5);
            pPed = CPools::GetPedPool().GetAt(ScriptParams[0]);
            pPed->SayScript(static_cast<tAudioEvent>(ScriptParams[1]), static_cast<bool>(ScriptParams[2] != 0), static_cast<bool>(ScriptParams[3] != 0),
                static_cast<bool>(ScriptParams[4] != 0));
            return OR_CONTINUE;
        case COMMAND_FORCE_INTERIOR_LIGHTING_FOR_PLAYER:
            CollectParameters(2);
            CWorld::Players[ScriptParams[0]].m_pPed->GetPlayerData()->m_bForceInteriorLighting = ScriptParams[1] != 0;
            return OR_CONTINUE;
        case COMMAND_DISABLE_2ND_PAD_FOR_DEBUG:
            CollectParameters(1);
            return OR_CONTINUE;
        case COMMAND_USE_DETONATOR:
            CWorld::UseDetonator(FindPlayerPed());
            return OR_CONTINUE;
        case COMMAND_IS_MONEY_PICKUP_AT_COORDS:
        {
            CollectParameters(3);
            vec.x = *reinterpret_cast<f32*>(&ScriptParams[0]);
            vec.y = *reinterpret_cast<f32*>(&ScriptParams[1]);
            vec.z = *reinterpret_cast<f32*>(&ScriptParams[2]);
            LatestCmpFlagResult = false;
            for (u32 PickupIndex = 0; PickupIndex < MAX_NUM_PICKUPS && !LatestCmpFlagResult; PickupIndex++)
            {
                if (CPickups::aPickUps.data()[PickupIndex].m_nPickupType == PICKUP_MONEY)
                {
                    CVector vecDiff = CPickups::aPickUps.data()[PickupIndex].GetPosn() - vec;
                    if (vecDiff.Magnitude() < 0.5f)
                    {
                        LatestCmpFlagResult = true;
                    }
                }
            }
            UpdateCompareFlag(LatestCmpFlagResult);
            return OR_CONTINUE;
        }
        case COMMAND_SET_MENU_COLUMN_WIDTH:
        {
            CollectParameters(3);
            u16 width = static_cast<u16>(ScriptParams[2]);
            CMenuSystem::SetColumnWidth(static_cast<u8>(ScriptParams[0]), static_cast<u8>(ScriptParams[1]),
                static_cast<u16>(SCREEN_WIDTH * (1.0f / 640.0f) * width));
            return OR_CONTINUE;
        }
        case COMMAND_SET_CHAR_CAN_CLIMB_OUT_WATER:
            return OR_CONTINUE;
        case COMMAND_MAKE_ROOM_IN_PLAYER_GANG_FOR_MISSION_PEDS:
        {
            CollectParameters(1);
            i32 PlayerGroup = FindPlayerPed()->GetPlayerData()->m_PlayerGroup;
            i32 ToBeRemoved = CPedGroups::ms_groups[PlayerGroup].GetGroupMembership()->CountMembersExcludingLeader() -
                              FindPlayerPed()->GetPlayerData()->m_nScriptLimitToGangSize - ScriptParams[0];
            if (ToBeRemoved > 0)
            {
                CPedGroups::ms_groups[PlayerGroup].GetGroupMembership()->RemoveNFollowers(ToBeRemoved);
            }
            return OR_CONTINUE;
        }
        case COMMAND_IS_CHAR_GETTING_IN_TO_A_CAR:
            CollectParameters(1);
            pPed = CPools::GetPedPool().GetAt(ScriptParams[0]);
            LatestCmpFlagResult = false;
            if ((pPed->GetPedIntelligence()->FindTaskActiveByType(CTaskTypes::TASK_COMPLEX_ENTER_CAR_AS_DRIVER) ||
                    pPed->GetPedIntelligence()->FindTaskActiveByType(CTaskTypes::TASK_COMPLEX_ENTER_CAR_AS_PASSENGER)) &&
                !pPed->GetPedIntelligence()->FindTaskActiveByType(CTaskTypes::TASK_COMPLEX_GO_TO_CAR_DOOR_AND_STAND_STILL))
            {
                LatestCmpFlagResult = true;
            }
            UpdateCompareFlag(LatestCmpFlagResult);
            return OR_CONTINUE;
        case COMMAND_SET_UP_SKIP_FOR_SPECIFIC_VEHICLE:
            CollectParameters(5);
            vec.x = *reinterpret_cast<f32*>(&ScriptParams[0]);
            vec.y = *reinterpret_cast<f32*>(&ScriptParams[1]);
            vec.z = *reinterpret_cast<f32*>(&ScriptParams[2]);
            CGameLogic::SetUpSkip(vec, *reinterpret_cast<f32*>(&ScriptParams[3]), false, CPools::GetVehiclePool().GetAt(ScriptParams[4]), false);
            return OR_CONTINUE;
        case COMMAND_GET_CAR_MODEL_VALUE:
        {
            CollectParameters(1);
            CVehicleModelInfo* pModelInfo = static_cast<CVehicleModelInfo*>(CModelInfo::GetModelInfo(ScriptParams[0]));
            ScriptParams[0] = gHandlingDataMgr.GetVehiclePointer(pModelInfo->m_nHandlingId)->m_nMonetaryValue;
            StoreParameters(1);
            return OR_CONTINUE;
        }
        case COMMAND_CREATE_CAR_GENERATOR_WITH_PLATE:
        {
            i32 CarGeneratorIndex;
            f32 NewX, NewY, NewZ;
            char CustomPlateString[9];
            i32 loop;

            CollectParameters(12);
            NewX = *reinterpret_cast<f32*>(&ScriptParams[0]);
            NewY = *reinterpret_cast<f32*>(&ScriptParams[1]);
            NewZ = *reinterpret_cast<f32*>(&ScriptParams[2]);
            if (NewZ > -100.0f)
            {
                NewZ += 0.015f;
            }
            CarGeneratorIndex = CTheCarGenerators::CreateCarGenerator(NewX, NewY, NewZ, *reinterpret_cast<f32*>(&ScriptParams[3]), ScriptParams[4],
                static_cast<i16>(ScriptParams[5]), static_cast<i16>(ScriptParams[6]), static_cast<u8>(ScriptParams[7]),
                static_cast<u8>(ScriptParams[8]), static_cast<u8>(ScriptParams[9]), static_cast<u16>(ScriptParams[10]),
                static_cast<u16>(ScriptParams[11]), 0, true);
            ReadTextLabelFromScript(CustomPlateString, 9);
            for (loop = 0; loop < 8; loop++)
            {
                if (CustomPlateString[loop] == '_' || CustomPlateString[loop] == 0)
                {
                    CustomPlateString[loop] = ' ';
                }
            }
            CustomPlateString[8] = 0;
            CTheCarGenerators::m_SpecialPlateHandler.Add(CarGeneratorIndex, CustomPlateString);
            ScriptParams[0] = CarGeneratorIndex;
            StoreParameters(1);
            return OR_CONTINUE;
        }
        case COMMAND_FIND_TRAIN_DIRECTION:
            CollectParameters(1);
            pVehicle = CPools::GetVehiclePool().GetAt(ScriptParams[0]);
            UpdateCompareFlag(static_cast<CTrain*>(pVehicle)->trainFlags.bClockwiseDirection);
            return OR_CONTINUE;
        case COMMAND_SET_AIRCRAFT_CARRIER_SAM_SITE:
            CollectParameters(1);
            CObject::bAircraftCarrierSamSiteDisabled = ScriptParams[0] == 0;
            return OR_CONTINUE;
        case COMMAND_DRAW_LIGHT_WITH_RANGE:
        {
            f32 RedFloat, GreenFloat, BlueFloat, fRange;

            CollectParameters(7);
            vec.x = *reinterpret_cast<f32*>(&ScriptParams[0]);
            vec.y = *reinterpret_cast<f32*>(&ScriptParams[1]);
            vec.z = *reinterpret_cast<f32*>(&ScriptParams[2]);
            RedFloat = ScriptParams[3] * (1.0f / 255.0f);
            GreenFloat = ScriptParams[4] * (1.0f / 255.0f);
            BlueFloat = ScriptParams[5] * (1.0f / 255.0f);
            fRange = *reinterpret_cast<f32*>(&ScriptParams[6]);
            CPointLights::AddLight(PLTYPE_POINTLIGHT, vec, CVector(0.0f, 0.0f, 0.0f), fRange, RedFloat, GreenFloat, BlueFloat,
                0 /* FOGEFF_OFF */, true, nullptr);
            return OR_CONTINUE;
        }
        case COMMAND_ENABLE_BURGLARY_HOUSES:
            CollectParameters(1);
            CEntryExitManager::EnableBurglaryHouses(ScriptParams[0] != 0);
            return OR_CONTINUE;
        case COMMAND_IS_PLAYER_CONTROL_ON:
        {
            CollectParameters(1);
            CPad* pPad = CPad::GetPad(ScriptParams[0]);
            UpdateCompareFlag((pPad->DisablePlayerControls & (1 << 5) /* DISABLE_SCRIPT */) == 0);
            return OR_CONTINUE;
        }
        case COMMAND_GET_CHAR_AREA_VISIBLE:
            CollectParameters(1);
            pPed = CPools::GetPedPool().GetAt(ScriptParams[0]);
            ScriptParams[0] = pPed->GetAreaCode();
            StoreParameters(1);
            return OR_CONTINUE;
        case COMMAND_GIVE_NON_PLAYER_CAR_NITRO:
            CollectParameters(1);
            pVehicle = CPools::GetVehiclePool().GetAt(ScriptParams[0]);
            static_cast<CAutomobile*>(pVehicle)->NitrousControl(1);
            return OR_CONTINUE;
        case COMMAND_PLAYER_PUT_ON_GOGGLES:
            return OR_CONTINUE;
        case COMMAND_PLAYER_TAKE_OFF_GOGGLES:
            CollectParameters(2);
            pPed = CWorld::Players[ScriptParams[0]].m_pPed;
            if (ScriptParams[1])
            {
                if (pPed->IsWearingGoggles() && !pPed->GetPedIntelligence()->GetTaskPrimary())
                {
                    pPed->GetPedIntelligence()->AddTaskPrimary(new CTaskComplexUseGoggles());
                    if (pPed->GetPlayerData())
                    {
                        pPed->GetPlayerData()->m_bDontAllowWeaponChange = true;
                    }
                }
            }
            else
            {
                pPed->TakeOffGoggles();
            }
            return OR_CONTINUE;
        case COMMAND_ALLOW_FIXED_CAMERA_COLLISION:
            CollectParameters(1);
            gAllowScriptedFixedCameraCollision = ScriptParams[0] != 0;
            return OR_CONTINUE;
        case COMMAND_HAS_CHAR_SPOTTED_CHAR_IN_FRONT:
        {
            CollectParameters(2);
            pPed = CPools::GetPedPool().GetAt(ScriptParams[0]);
            CPed* pSecondPed = CPools::GetPedPool().GetAt(ScriptParams[1]);
            LatestCmpFlagResult = pPed->OurPedCanSeeThisEntity(pSecondPed, false);
            UpdateCompareFlag(LatestCmpFlagResult);
            return OR_CONTINUE;
        }
        case COMMAND_FORCE_BIG_MESSAGE_AND_COUNTER:
            CollectParameters(1);
            CHud::bScriptForceDisplayWithCounters = ScriptParams[0];
            return OR_CONTINUE;
        case COMMAND_SET_VEHICLE_CAMERA_TWEAK:
            CollectParameters(4);
            {
                auto& tweak = TheCamera.m_aCamTweak[std::size(TheCamera.m_aCamTweak) - 1];
                tweak.ModelID = ScriptParams[0];
                tweak.Dist = *reinterpret_cast<f32*>(&ScriptParams[1]);
                tweak.Alt = *reinterpret_cast<f32*>(&ScriptParams[2]);
                tweak.Angle = *reinterpret_cast<f32*>(&ScriptParams[3]);
            }
            return OR_CONTINUE;
        case COMMAND_RESET_VEHICLE_CAMERA_TWEAK:
            TheCamera.m_aCamTweak[std::size(TheCamera.m_aCamTweak) - 1] = CamTweak{ -1, 1.0f, 1.0f, 0.0f };
            return OR_CONTINUE;
        case COMMAND_REPORT_MISSION_AUDIO_EVENT_AT_CHAR:
            CollectParameters(2);
            pPed = CPools::GetPedPool().GetAt(ScriptParams[0]);
            AudioEngine.ReportMissionAudioEvent(static_cast<u16>(ScriptParams[1]), pPed);
            return OR_CONTINUE;
        case COMMAND_DOES_DECISION_MAKER_EXIST:
            CollectParameters(1);
            LatestCmpFlagResult = false;
            iDecisionMakerIndex = CTheScripts::GetActualScriptThingIndex(ScriptParams[0], UNIQUE_SCRIPT_DECISION_MAKER);
            if (iDecisionMakerIndex >= 0 && iDecisionMakerIndex < (i32)CDecisionMakerTypes::NUM_TYPES)
            {
                if (CDecisionMakerTypes::m_IsActive[iDecisionMakerIndex])
                {
                    LatestCmpFlagResult = true;
                }
            }
            UpdateCompareFlag(LatestCmpFlagResult);
            return OR_CONTINUE;
        case COMMAND_GET_RANDOM_TRAIN_IN_SPHERE_NO_SAVE:
            return OR_CONTINUE;
        case COMMAND_IGNORE_HEIGHT_DIFFERENCE_FOLLOWING_NODES:
            CollectParameters(2);
            pPed = CPools::GetPedPool().GetAt(ScriptParams[0]);
            pPed->bIgnoreHeightDifferenceFollowingNodes = ScriptParams[1] != 0;
            return OR_CONTINUE;
        case COMMAND_SHUT_ALL_CHARS_UP:
            CollectParameters(1);
            if (ScriptParams[0])
            {
                CAEPedSpeechAudioEntity::DisableAllPedSpeech();
            }
            else
            {
                CAEPedSpeechAudioEntity::EnableAllPedSpeech();
            }
            return OR_CONTINUE;
        case COMMAND_SET_CHAR_GET_OUT_UPSIDE_DOWN_CAR:
            CollectParameters(2);
            pPed = CPools::GetPedPool().GetAt(ScriptParams[0]);
            pPed->bGetOutUpsideDownCar = ScriptParams[1] != 0;
            return OR_CONTINUE;
        case COMMAND_REPORT_MISSION_AUDIO_EVENT_AT_CAR:
            CollectParameters(2);
            pVehicle = CPools::GetVehiclePool().GetAt(ScriptParams[0]);
            AudioEngine.ReportMissionAudioEvent(static_cast<u16>(ScriptParams[1]), pVehicle);
            return OR_CONTINUE;
        case COMMAND_DO_WEAPON_STUFF_AT_START_OF_2P_GAME:
            CGameLogic::DoWeaponStuffAtStartOf2PlayerGame(true);
            return OR_CONTINUE;
        case COMMAND_SET_MENU_HEADER_ORIENTATION:
            return OR_CONTINUE;
        case COMMAND_HAS_GAME_JUST_RETURNED_FROM_FRONTEND:
            UpdateCompareFlag(CPad::GetPad(0)->JustOutOfFrontEnd == 1);
            return OR_CONTINUE;
        case COMMAND_GET_CURRENT_LANGUAGE:
            ScriptParams[0] = static_cast<i32>(FrontEndMenuManager.m_nPrefsLanguage);
            StoreParameters(1);
            return OR_CONTINUE;
        case COMMAND_IS_OBJECT_INTERSECTING_WORLD:
            CollectParameters(1);
            pObject = CPools::GetObjectPool().GetAt(ScriptParams[0]);
            LatestCmpFlagResult = pObject->TestCollision(false);
            UpdateCompareFlag(LatestCmpFlagResult);
            return OR_CONTINUE;
        case COMMAND_GET_STRING_WIDTH:
        {
            char TextLabel[10];
            ReadTextLabelFromScript(TextLabel, 8);
            {
                const GxtChar* pString = TheText.Get(TextLabel);
                ScriptParams[0] = CFont::GetStringWidth(pString, true, true);
            }
            StoreParameters(1);
            return OR_CONTINUE;
        }
        case COMMAND_RESET_VEHICLE_HYDRAULICS:
            CollectParameters(1);
            pVehicle = CPools::GetVehiclePool().GetAt(ScriptParams[0]);
            if (pVehicle->handlingFlags.bHydraulicInst && pVehicle->GetBaseVehicleType() == VEHICLE_TYPE_CAR)
            {
                static_cast<CAutomobile*>(pVehicle)->m_wMiscComponentAngle = 0;
            }
            return OR_CONTINUE;
        case COMMAND_SET_RESPAWN_POINT_FOR_DURATION_OF_MISSION:
            CollectParameters(3);
            vec.x = *reinterpret_cast<f32*>(&ScriptParams[0]);
            vec.y = *reinterpret_cast<f32*>(&ScriptParams[1]);
            vec.z = *reinterpret_cast<f32*>(&ScriptParams[2]);
            CRestart::SetRespawnPointForDurationOfMission(vec);
            return OR_CONTINUE;
        case COMMAND_IS_THIS_MODEL_A_BIKE:
            return OR_CONTINUE;
        case COMMAND_IS_THIS_MODEL_A_CAR:
            CollectParameters(1);
            LatestCmpFlagResult = false;
            if (CModelInfo::IsCarModel(ScriptParams[0]))
            {
                LatestCmpFlagResult = true;
            }
            UpdateCompareFlag(LatestCmpFlagResult);
            return OR_CONTINUE;
        case COMMAND_SWITCH_ON_GROUND_SEARCHLIGHT:
        {
            CollectParameters(2);
            i32 ActualSearchlightIndex = CTheScripts::GetActualScriptThingIndex(ScriptParams[0], UNIQUE_SCRIPT_SEARCHLIGHT);
            if (ActualSearchlightIndex >= 0)
            {
                CTheScripts::ScriptSearchLightArray[ActualSearchlightIndex].m_bEnableShadow = ScriptParams[1];
            }
            return OR_CONTINUE;
        }
        case COMMAND_IS_GANG_WAR_FIGHTING_GOING_ON:
            LatestCmpFlagResult = CGangWars::GangWarFightingGoingOn();
            UpdateCompareFlag(LatestCmpFlagResult);
            return OR_CONTINUE;
        case COMMAND_SET_VEHICLE_FIRING_RATE_MULTIPLIER:
            return OR_CONTINUE;
        case COMMAND_GET_VEHICLE_FIRING_RATE_MULTIPLIER:
            return OR_CONTINUE;
        case COMMAND_IS_NEXT_STATION_ALLOWED:
            CollectParameters(1);
            pVehicle = CPools::GetVehiclePool().GetAt(ScriptParams[0]);
            LatestCmpFlagResult = CTrain::IsNextStationAllowed(static_cast<CTrain*>(pVehicle));
            UpdateCompareFlag(LatestCmpFlagResult);
            return OR_CONTINUE;
        case COMMAND_SKIP_TO_NEXT_ALLOWED_STATION:
            CollectParameters(1);
            pVehicle = CPools::GetVehiclePool().GetAt(ScriptParams[0]);
            CTrain::SkipToNextAllowedStation(static_cast<CTrain*>(pVehicle));
            return OR_CONTINUE;
        case COMMAND_GET_STRING_WIDTH_WITH_NUMBER:
        {
            char TextLabel[10];
            ReadTextLabelFromScript(TextLabel, 8);
            {
                const GxtChar* pString = TheText.Get(TextLabel);
                CollectParameters(1);
                i32 nNum = ScriptParams[0];
                GxtChar GxtString[256];
                CMessages::InsertNumberInString(pString, nNum, -1, -1, -1, -1, -1, GxtString);
                ScriptParams[0] = CFont::GetStringWidth(GxtString, true, true);
            }
            StoreParameters(1);
            return OR_CONTINUE;
        }
        case COMMAND_SHUT_CHAR_UP_FOR_SCRIPTED_SPEECH:
            CollectParameters(2);
            pPed = CPools::GetPedPool().GetAt(ScriptParams[0]);
            if (ScriptParams[1])
            {
                if (pPed)
                {
                    pPed->DisablePedSpeechForScriptSpeech(false);
                }
            }
            else
            {
                if (pPed)
                {
                    pPed->EnablePedSpeechForScriptSpeech();
                }
            }
            return OR_CONTINUE;
        case COMMAND_ENABLE_DISABLED_ATTACTORS_ON_OBJECT:
            CollectParameters(2);
            pObject = CPools::GetObjectPool().GetAt(ScriptParams[0]);
            pObject->objectFlags.bEnableDisabledAttractors = ScriptParams[1];
            return OR_CONTINUE;
        case COMMAND_LOAD_SCENE_IN_DIRECTION:
        {
            CollectParameters(4);
            vec.x = *reinterpret_cast<f32*>(&ScriptParams[0]);
            vec.y = *reinterpret_cast<f32*>(&ScriptParams[1]);
            vec.z = *reinterpret_cast<f32*>(&ScriptParams[2]);
            f32 fHeading = *reinterpret_cast<f32*>(&ScriptParams[3]);
            CTimer::Stop();
            CRenderer::RequestObjectsInDirection(vec, DEGTORAD(fHeading), STRFLAG_LOADSCENE);
            CStreaming::LoadScene(vec);
            CTimer::Update();
            return OR_CONTINUE;
        }
        case COMMAND_IS_PLAYER_USING_JETPACK:
            CollectParameters(1);
            LatestCmpFlagResult = (CWorld::Players[ScriptParams[0]].m_pPed->GetPedIntelligence()->GetTaskJetPack() != nullptr);
            UpdateCompareFlag(LatestCmpFlagResult);
            return OR_CONTINUE;
        case COMMAND_BLOCK_VEHICLE_MODEL:
            return OR_CONTINUE;

        case COMMAND_CLEAR_THIS_PRINT_BIG_NOW:
            CollectParameters(1);
            CMessages::ClearThisPrintBigNow(static_cast<u16>(ScriptParams[0] - 1));
            return OR_CONTINUE;
        case COMMAND_HAS_LANGUAGE_CHANGED:
            LatestCmpFlagResult = FrontEndMenuManager.HasLanguageChanged();
            UpdateCompareFlag(LatestCmpFlagResult);
            return OR_CONTINUE;
        case COMMAND_INCREMENT_INT_STAT_NO_MESSAGE:
            CollectParameters(2);
            CStats::IncrementStat(static_cast<u16>(ScriptParams[0]), static_cast<f32>(ScriptParams[1]));
            return OR_CONTINUE;
        case COMMAND_SET_EXTRA_CAR_COLOURS:
            CollectParameters(3);
            pVehicle = CPools::GetVehiclePool().GetAt(ScriptParams[0]);
            pVehicle->m_nTertiaryColor = ScriptParams[1];
            pVehicle->m_nQuaternaryColor = ScriptParams[2];
            return OR_CONTINUE;
        case COMMAND_GET_EXTRA_CAR_COLOURS:
            CollectParameters(1);
            pVehicle = CPools::GetVehiclePool().GetAt(ScriptParams[0]);
            ScriptParams[0] = pVehicle->m_nTertiaryColor;
            ScriptParams[1] = pVehicle->m_nQuaternaryColor;
            StoreParameters(2);
            return OR_CONTINUE;
        case COMMAND_MANAGE_ALL_POPULATION:
            CPopulation::ManageAllPopulation();
            return OR_CONTINUE;
        case COMMAND_SET_NO_RESPRAYS:
            CollectParameters(1);
            CGarages::NoResprays = ScriptParams[0] != 0;
            CGarages::AllRespraysCloseOrOpen(ScriptParams[0] == 0);
            return OR_CONTINUE;
        case COMMAND_HAS_CAR_BEEN_RESPRAYED:
            CollectParameters(1);
            pVehicle = CPools::GetVehiclePool().GetAt(ScriptParams[0]);
            LatestCmpFlagResult = false;
            if (pVehicle->vehicleFlags.bHasBeenResprayed)
            {
                LatestCmpFlagResult = true;
                pVehicle->vehicleFlags.bHasBeenResprayed = false;
            }
            UpdateCompareFlag(LatestCmpFlagResult);
            return OR_CONTINUE;
        case COMMAND_ATTACH_MISSION_AUDIO_TO_CAR:
            CollectParameters(2);
            pVehicle = CPools::GetVehiclePool().GetAt(ScriptParams[1]);
            AudioEngine.AttachMissionAudioToPhysical(static_cast<u8>(ScriptParams[0] - 1), pVehicle);
            return OR_CONTINUE;
        case COMMAND_SET_HAS_BEEN_OWNED_FOR_CAR_GENERATOR:
            CollectParameters(2);
            CTheCarGenerators::CarGeneratorArray[static_cast<u16>(ScriptParams[0])].bPlayerHasAlreadyOwnedCar = (ScriptParams[1] != 0);
            return OR_CONTINUE;
        case COMMAND_SET_UP_CONVERSATION_NODE_WITH_SCRIPTED_SPEECH:
        {
            char ThisNode[8], YesNode[8], NoNode[8];
            ReadTextLabelFromScript(ThisNode, 8);
            ReadTextLabelFromScript(YesNode, 8);
            ReadTextLabelFromScript(NoNode, 8);
            CollectParameters(3);
            CConversations::SetUpConversationNode(ThisNode, YesNode, NoNode, -ScriptParams[0], -ScriptParams[1], -ScriptParams[2]);
            return OR_CONTINUE;
        }
        case COMMAND_SET_AREA_NAME:
        {
            char ZoneLabel[8];
            ReadTextLabelFromScript(ZoneLabel, 8);
            const GxtChar* pZoneName = TheText.Get(ZoneLabel);
            CHud::SetZoneName(pZoneName, true);
            return OR_CONTINUE;
        }
        case COMMAND_TASK_PLAY_ANIM_SECONDARY:
            PlayAnimScriptCommand(CurrCommand);
            return OR_CONTINUE;
        case COMMAND_IS_CHAR_TOUCHING_CHAR:
        {
            CollectParameters(2);
            pPed = CPools::GetPedPool().GetAt(ScriptParams[0]);
            CPed* pPed2 = CPools::GetPedPool().GetAt(ScriptParams[1]);
            CVehicle* pVehicle2 = nullptr;
            LatestCmpFlagResult = false;
            if (pPed->bInVehicle)
            {
                pVehicle = pPed->m_pMyVehicle;
            }
            if (pPed2->bInVehicle)
            {
                pVehicle2 = pPed2->m_pMyVehicle;
            }
            if (pVehicle)
            {
                if (pVehicle2)
                {
                    if (pVehicle->GetHasCollidedWith(pVehicle2) || pVehicle2->GetHasCollidedWith(pVehicle))
                    {
                        LatestCmpFlagResult = true;
                    }
                }
                else
                {
                    if (pPed2->GetHasCollidedWith(pVehicle))
                    {
                        LatestCmpFlagResult = true;
                    }
                }
            }
            else if (pVehicle2)
            {
                if (pPed->GetHasCollidedWith(pVehicle2))
                {
                    LatestCmpFlagResult = true;
                }
            }
            else
            {
                if (pPed->GetHasCollidedWith(pPed2))
                {
                    LatestCmpFlagResult = true;
                }
            }
            UpdateCompareFlag(LatestCmpFlagResult);
            return OR_CONTINUE;
        }
        case COMMAND_DISABLE_HELI_AUDIO:
            CollectParameters(2);
            pVehicle = CPools::GetVehiclePool().GetAt(ScriptParams[0]);
            if (ScriptParams[1])
            {
                pVehicle->m_vehicleAudio.EnableHelicoptor();
            }
            else
            {
                pVehicle->m_vehicleAudio.DisableHelicoptor();
            }
            return OR_CONTINUE;
        case COMMAND_TASK_HAND_GESTURE:
        {
            CollectParameters(2);
            const i32 nPedID = ScriptParams[0];
            pPed = CPools::GetPedPool().GetAt(ScriptParams[1]);
            GivePedScriptedTask(nPedID, new CTaskComplexSignalAtPed(pPed, -1, false), CurrCommand);
            return OR_CONTINUE;
        }
        case COMMAND_TAKE_PHOTO:
            CollectParameters(1);
            CWeapon::ms_bTakePhoto = true;
            CPostEffects::m_bSavePhotoFromScript = ScriptParams[0] != 0;
            return OR_CONTINUE;
        case COMMAND_INCREMENT_FLOAT_STAT_NO_MESSAGE:
            CollectParameters(2);
            CStats::IncrementStat(static_cast<u16>(ScriptParams[0]), *reinterpret_cast<f32*>(&ScriptParams[1]));
            return OR_CONTINUE;
        case COMMAND_SET_PLAYER_GROUP_TO_FOLLOW_ALWAYS:
            CollectParameters(2);
            FindPlayerPed(ScriptParams[0])->ForceGroupToAlwaysFollow(ScriptParams[1] != 0);
            return OR_CONTINUE;
        case COMMAND_IMPROVE_CAR_BY_CHEATING:
            CollectParameters(2);
            pVehicle = CPools::GetVehiclePool().GetAt(ScriptParams[0]);
            pVehicle->vehicleFlags.bUseCarCheats = ScriptParams[1];
            return OR_CONTINUE;
        case COMMAND_CHANGE_CAR_COLOUR_FROM_MENU:
            CollectParameters(4);
            pVehicle = CPools::GetVehiclePool().GetAt(ScriptParams[1]);
            new_car_colour = CMenuSystem::GetCarColourFromGrid(static_cast<u8>(ScriptParams[0]), static_cast<u8>(ScriptParams[3]));
            if (ScriptParams[2] == 1)
            {
                pVehicle->m_colour1 = new_car_colour;
            }
            else
            {
                pVehicle->m_colour2 = new_car_colour;
            }
            return OR_CONTINUE;
        case COMMAND_HIGHLIGHT_MENU_ITEM:
            CollectParameters(3);
            CMenuSystem::HighlightOneItem(static_cast<u8>(ScriptParams[0]), static_cast<u8>(ScriptParams[1]), static_cast<u8>(ScriptParams[2]));
            return OR_CONTINUE;
        case COMMAND_SET_DISABLE_MILITARY_ZONES:
            CollectParameters(1);
            CCullZones::bMilitaryZonesDisabled = ScriptParams[0] != 0;
            return OR_CONTINUE;
        case COMMAND_SET_CAMERA_POSITION_UNFIXED:
            CollectParameters(2);
            TheCamera.Cams[TheCamera.ActiveCam].m_fVerticalAngle = *reinterpret_cast<f32*>(&ScriptParams[0]);
            TheCamera.Cams[TheCamera.ActiveCam].m_fHorizontalAngle = *reinterpret_cast<f32*>(&ScriptParams[1]);
            if (TheCamera.Cams[TheCamera.ActiveCam].m_nMode == MODE_FOLLOWPED)
            {
                TheCamera.Cams[TheCamera.ActiveCam].Process_FollowPed_SA(FindPlayerPed()->GetPosition(), 0.0f, 0.0f, 0.0f, true);
            }
            else if (FindPlayerVehicle())
            {
                if (TheCamera.Cams[TheCamera.ActiveCam].m_nMode == MODE_CAM_ON_A_STRING ||
                    TheCamera.Cams[TheCamera.ActiveCam].m_nMode == MODE_BEHINDBOAT ||
                    TheCamera.Cams[TheCamera.ActiveCam].m_nMode == MODE_BEHINDCAR)
                {
                    TheCamera.Cams[TheCamera.ActiveCam].Process_FollowCar_SA(FindPlayerVehicle()->GetPosition(), 0.0f, 0.0f, 0.0f, true);
                }
            }
            return OR_CONTINUE;
        case COMMAND_SET_RADIO_TO_PLAYERS_FAVOURITE_STATION:
            AudioEngine.RetuneRadio(CStats::FindMostFavoriteRadioStation());
            return OR_CONTINUE;
        case COMMAND_SET_DEATH_WEAPONS_PERSIST:
            CollectParameters(2);
            pPed = CPools::GetPedPool().GetAt(ScriptParams[0]);
            pPed->bDeathPickupsPersist = ScriptParams[1] != 0;
            return OR_CONTINUE;
    }
    return OR_INTERRUPT;
}

// MARK: 2600To2699

// Only ever incremented, never read (0xA95190).
static i32 s_nXboxPlayer2StartPresses;

// func: sa 0x479DA0
OpcodeResult CRunningScript::ProcessCommands2600To2699(i32 CurrCommand)
{
    MARKFUNCTION(0x479DA0);

    CPed* pPed;
    CVehicle* pVehicle;
    CVector vec;
    char TextLabel[10];
    i32 ModelIndex;
    f32 temp_float;

    // NOTE: Keep switch cases in strictly ascending order as in commands.hpp to preserve MSVC jump table layout
    switch (CurrCommand)
    {
        case COMMAND_SET_SWIM_SPEED:
        {
            CollectParameters(2);
            pPed = CPools::GetPedPool().GetAt(ScriptParams[0]);
            temp_float = *reinterpret_cast<f32*>(&ScriptParams[1]);
            if (pPed && pPed->GetPedIntelligence()->GetTaskSwim())
            {
                pPed->GetPedIntelligence()->GetTaskSwim()->m_fAnimSpeed = temp_float;
            }
            return OR_CONTINUE;
        }
        case COMMAND_IS_PLAYER_CLIMBING:
        {
            CollectParameters(1);
            UpdateCompareFlag(CWorld::Players[ScriptParams[0]].m_pPed->GetPedIntelligence()->GetTaskClimb() != nullptr);
            return OR_CONTINUE;
        }
        case COMMAND_IS_THIS_HELP_MESSAGE_BEING_DISPLAYED:
        {
            GxtChar TestMessage[400];
            ReadTextLabelFromScript(TextLabel, 8);
            const GxtChar* pString = TheText.Get(TextLabel);
            bool bResult = false;
            if (CHud::HelpMessageDisplayed())
            {
                CMessages::StringCopy(TestMessage, pString, 400);
                CMessages::InsertPlayerControlKeysInString(TestMessage);
                bResult = CMessages::StringCompare(TestMessage, CHud::m_pHelpMessageToPrint, CMessages::GetStringLength(TestMessage));
            }
            UpdateCompareFlag(bResult);
            return OR_CONTINUE;
        }
        case COMMAND_IS_WIDESCREEN_ON_IN_OPTIONS:
        {
            UpdateCompareFlag(FrontEndMenuManager.m_PrefsUseWideScreen);
            return OR_CONTINUE;
        }
        case COMMAND_DRAW_SUBTITLES_BEFORE_FADE:
        {
            CollectParameters(1);
            CTheScripts::bDrawSubtitlesBeforeFade = ScriptParams[0];
            return OR_CONTINUE;
        }
        case COMMAND_DRAW_ODDJOB_TITLE_BEFORE_FADE:
        {
            CollectParameters(1);
            CTheScripts::bDrawOddJobTitleBeforeFade = ScriptParams[0];
            return OR_CONTINUE;
        }
        case COMMAND_TASK_FOLLOW_PATH_NODES_TO_COORD_WITH_RADIUS:
        {
            CollectParameters(7);
            const i32 iPedID = ScriptParams[0];
            CVector vTarget(*reinterpret_cast<f32*>(&ScriptParams[1]), *reinterpret_cast<f32*>(&ScriptParams[2]), *reinterpret_cast<f32*>(&ScriptParams[3]));
            const i32 iMoveState = ScriptParams[4];
            i32 iTime = ScriptParams[5];
            const f32 fRadius = *reinterpret_cast<f32*>(&ScriptParams[6]);
            if (iTime == -1)
            {
                iTime = 50000;
            }
            else if (iTime == -2)
            {
                iTime = -1;
            }
            CTask* pTask = new CTaskComplexFollowNodeRoute(static_cast<eMoveState>(iMoveState), vTarget, fRadius, 3.0f, 2.0f, true, iTime, true);
            GivePedScriptedTask(iPedID, pTask, CurrCommand);
            return OR_CONTINUE;
        }
        case COMMAND_SET_PHOTO_CAMERA_EFFECT:
        {
            CollectParameters(1);
            CTheScripts::bDrawCrossHair = ScriptParams[0] ? eCrossHairType::FIXED_DRAW_1STPERSON_WEAPON : eCrossHairType::NONE;
            return OR_CONTINUE;
        }
        case COMMAND_FIX_CAR:
        {
            CollectParameters(1);
            pVehicle = CPools::GetVehiclePool().GetAt(ScriptParams[0]);
            pVehicle->Fix();
            pVehicle->m_nHealth = 1000.0f;
            return OR_CONTINUE;
        }
        case COMMAND_SET_PLAYER_GROUP_TO_FOLLOW_NEVER:
        {
            CollectParameters(2);
            FindPlayerPed(ScriptParams[0])->ForceGroupToNeverFollow(ScriptParams[1] != 0);
            return OR_CONTINUE;
        }
        case COMMAND_IS_CHAR_ATTACHED_TO_ANY_CAR:
        {
            CollectParameters(1);
            CEntity* pAttachedTo = CPools::GetPedPool().GetAt(ScriptParams[0])->m_pAttachToEntity;
            bool bResult = false;
            if (pAttachedTo)
            {
                bResult = pAttachedTo->GetIsTypeVehicle();
            }
            UpdateCompareFlag(bResult);
            return OR_CONTINUE;
        }
        case COMMAND_STORE_CAR_CHAR_IS_ATTACHED_TO_NO_SAVE:
        {
            CollectParameters(1);
            CEntity* pAttachedTo = CPools::GetPedPool().GetAt(ScriptParams[0])->m_pAttachToEntity;
            i32 nIndex = -1;
            if (pAttachedTo && pAttachedTo->GetIsTypeVehicle())
            {
                nIndex = CPools::GetVehiclePool().GetIndex(static_cast<CVehicle*>(pAttachedTo));
            }
            ScriptParams[0] = nIndex;
            StoreParameters(1);
            return OR_CONTINUE;
        }
        case COMMAND_SET_UP_SKIP_TO_BE_FINISHED_BY_SCRIPT:
        {
            return OR_CONTINUE;
        }
        case COMMAND_SET_UP_SKIP_FOR_VEHICLE_FINISHED_BY_SCRIPT:
        {
            CollectParameters(5);
            vec = CVector(*reinterpret_cast<f32*>(&ScriptParams[0]), *reinterpret_cast<f32*>(&ScriptParams[1]), *reinterpret_cast<f32*>(&ScriptParams[2]));
            pVehicle = CPools::GetVehiclePool().GetAt(ScriptParams[4]);
            CGameLogic::SetUpSkip(vec, *reinterpret_cast<f32*>(&ScriptParams[3]), false, pVehicle, true);
            return OR_CONTINUE;
        }
        case COMMAND_IS_SKIP_WAITING_FOR_SCRIPT_TO_FADE_IN:
        {
            UpdateCompareFlag(CGameLogic::IsSkipWaitingForScriptToFadeIn());
            return OR_CONTINUE;
        }
        case COMMAND_FORCE_ALL_VEHICLE_LIGHTS_OFF:
        {
            CollectParameters(1);
            CVehicle::ms_forceVehicleLightsOff = ScriptParams[0];
            return OR_CONTINUE;
        }
        case COMMAND_SET_RENDER_PLAYER_WEAPON:
        {
            return OR_CONTINUE;
        }
        case COMMAND_GET_PLAYER_IN_CAR_CAMERA_MODE:
        {
            ScriptParams[0] = TheCamera.m_nCarZoom;
            StoreParameters(1);
            return OR_CONTINUE;
        }
        case COMMAND_IS_LAST_BUILDING_MODEL_SHOT_BY_PLAYER:
        {
            CollectParameters(2);
            ModelIndex = ScriptParams[1];
            if (ModelIndex < 0)
            {
                ModelIndex = CTheScripts::UsedObjectArray[-ModelIndex].Index;
            }
            UpdateCompareFlag(CWorld::Players[ScriptParams[0]].PlayerPedData.m_nModelIndexOfLastBuildingShot == ModelIndex);
            return OR_CONTINUE;
        }
        case COMMAND_CLEAR_LAST_BUILDING_MODEL_SHOT_BY_PLAYER:
        {
            CollectParameters(1);
            CWorld::Players[ScriptParams[0]].PlayerPedData.m_nModelIndexOfLastBuildingShot = -1;
            return OR_CONTINUE;
        }
        case COMMAND_SET_UP_CONVERSATION_END_NODE_WITH_SCRIPTED_SPEECH:
        {
            ReadTextLabelFromScript(TextLabel, 8);
            CollectParameters(1);
            CConversations::SetUpConversationNode(TextLabel, nullptr, nullptr, -ScriptParams[0], 0, 0);
            return OR_CONTINUE;
        }
        case COMMAND_ACTIVATE_PIMP_CHEAT:
        {
            CollectParameters(1);
            if (ScriptParams[0])
            {
                CCheat::ApplyCheat(PIMP_CHEAT);
            }
            else
            {
                CCheat::Disable(static_cast<eCheats>(PIMP_CHEAT));
            }
            return OR_CONTINUE;
        }
        case COMMAND_GET_RANDOM_CHAR_IN_AREA_OFFSET_NO_SAVE:
        {
            CollectParameters(6);
            vec = CVector(*reinterpret_cast<f32*>(&ScriptParams[0]), *reinterpret_cast<f32*>(&ScriptParams[1]), *reinterpret_cast<f32*>(&ScriptParams[2]));
            const CVector vecMin = vec - CVector(*reinterpret_cast<f32*>(&ScriptParams[3]), *reinterpret_cast<f32*>(&ScriptParams[4]), *reinterpret_cast<f32*>(&ScriptParams[5]));
            const CVector vecMax = vec + CVector(*reinterpret_cast<f32*>(&ScriptParams[3]), *reinterpret_cast<f32*>(&ScriptParams[4]), *reinterpret_cast<f32*>(&ScriptParams[5]));
            i32 iPedID = -1;
            for (i32 i = CPools::GetPedPool().GetSize(); i;)
            {
                pPed = CPools::GetPedPool().GetSlot(--i);
                if (pPed && pPed->CharCreatedBy == RANDOM_CHAR && !pPed->m_bRemoveFromWorld && !pPed->bFadeOut && !IsPedDead(pPed) && !pPed->bInVehicle && !CPedGroups::GetPedsGroup(pPed))
                {
                    const CVector& vecPedPos = pPed->GetPosition();
                    if (vecPedPos.x >= vecMin.x && vecPedPos.x <= vecMax.x && vecPedPos.y >= vecMin.y && vecPedPos.y <= vecMax.y && vecPedPos.z >= vecMin.z && vecPedPos.z <= vecMax.z)
                    {
                        iPedID = CPools::GetPedPool().GetIndex(pPed);
                        break;
                    }
                }
            }
            ScriptParams[0] = iPedID;
            StoreParameters(1);
            return OR_CONTINUE;
        }
        case COMMAND_SET_SCRIPT_COOP_GAME:
        {
            CollectParameters(1);
            CGameLogic::bScriptCoopGameGoingOn = ScriptParams[0] != 0;
            return OR_CONTINUE;
        }
        case COMMAND_CREATE_USER_3D_MARKER:
        {
            CollectParameters(4);
            vec = CVector(*reinterpret_cast<f32*>(&ScriptParams[0]), *reinterpret_cast<f32*>(&ScriptParams[1]), *reinterpret_cast<f32*>(&ScriptParams[2]));
            ScriptParams[0] = C3dMarkers::User3dMarkerSet(vec.x, vec.y, vec.z, static_cast<eHudColours>(ScriptParams[3]));
            StoreParameters(1);
            return OR_CONTINUE;
        }
        case COMMAND_REMOVE_USER_3D_MARKER:
        {
            CollectParameters(1);
            C3dMarkers::User3dMarkerDelete(ScriptParams[0]);
            return OR_CONTINUE;
        }
        case COMMAND_REMOVE_ALL_USER_3D_MARKERS:
        {
            return OR_CONTINUE;
        }
        case COMMAND_GET_RID_OF_PLAYER_PROSTITUTE:
        {
            CTaskComplexProstituteSolicit::GetRidOfPlayerProstitute();
            return OR_CONTINUE;
        }
        case COMMAND_DISPLAY_NON_MINIGAME_HELP_MESSAGES:
        {
            CollectParameters(1);
            CTheScripts::bDisplayNonMiniGameHelpMessages = ScriptParams[0] != 0;
            return OR_CONTINUE;
        }
        case COMMAND_SET_RAILTRACK_RESISTANCE_MULT:
        {
            CollectParameters(1);
            if (*reinterpret_cast<f32*>(&ScriptParams[0]) > 0.0f)
            {
                CVehicle::ms_fRailTrackResistance = CVehicle::ms_fRailTrackResistanceDefault * *reinterpret_cast<f32*>(&ScriptParams[0]);
            }
            else
            {
                CVehicle::ms_fRailTrackResistance = CVehicle::ms_fRailTrackResistanceDefault;
            }
            return OR_CONTINUE;
        }
        case COMMAND_SWITCH_OBJECT_BRAINS:
        {
            CollectParameters(2);
            if (ScriptParams[1])
            {
                CTheScripts::ScriptsForBrains.SwitchAllObjectBrainsWithThisID(ScriptParams[0], true);
            }
            else
            {
                CTheScripts::ScriptsForBrains.SwitchAllObjectBrainsWithThisID(ScriptParams[0], false);
            }
            return OR_CONTINUE;
        }
        case COMMAND_FINISH_SETTING_UP_CONVERSATION_NO_SUBTITLES:
        {
            CConversations::DoneSettingUpConversation(true);
            return OR_CONTINUE;
        }
        case COMMAND_ALLOW_PAUSE_IN_WIDESCREEN:
        {
            CollectParameters(1);
            FrontEndMenuManager.m_bMenuAccessWidescreen = ScriptParams[0] != 0;
            return OR_CONTINUE;
        }
        case COMMAND_IS_XBOX_VERSION:
        {
            UpdateCompareFlag(false);
            return OR_CONTINUE;
        }
        case COMMAND_GET_PC_MOUSE_MOVEMENT:
        {
            CPad::GetPad(0);
            *reinterpret_cast<f32*>(&ScriptParams[0]) = CPad::NewMouseControllerState.m_AmountMoved.x;
            CPad::GetPad(0);
            *reinterpret_cast<f32*>(&ScriptParams[1]) = CPad::NewMouseControllerState.m_AmountMoved.y;
            StoreParameters(2);
            return OR_CONTINUE;
        }
        case COMMAND_IS_PC_USING_JOYPAD:
        {
            UpdateCompareFlag(static_cast<i32>(FrontEndMenuManager.m_ControlMethod) != 0);
            return OR_CONTINUE;
        }
        case COMMAND_IS_MOUSE_USING_VERTICAL_INVERSION:
        {
            UpdateCompareFlag(CMenuManager::bInvertMouseY != 0);
            return OR_CONTINUE;
        }
        case COMMAND_IS_JAPANESE_VERSION:
        {
            UpdateCompareFlag(false);
            return OR_CONTINUE;
        }
        case COMMAND_IS_XBOX_PLAYER2_PRESSING_START:
        {
            ++s_nXboxPlayer2StartPresses;
            return OR_CONTINUE;
        }
        default:
            return OR_INTERRUPT;
    }
}

// The compatibility macros above are file-local: undo them so they do not leak into the
// next translation unit of a unity build.
#undef PCPointer
#undef PCStack
#undef StackDepth
#undef Locals
#undef bActive
#undef CmpFlag
#undef IsThisAMissionScript
#undef bIsThisAStreamedScript
#undef bIsThisAMiniGameScript
#undef ScriptBrainType
#undef ActivateTime
#undef AndOrState
#undef NotForLatestExpression
#undef DeatharrestCheckEnabled
#undef DoneDeatharrest
#undef EndOfScriptedCutscenePC
#undef ThisMustBeTheOnlyMissionRunning
#undef BaseAddressOfThisScript
#undef ScriptName
#undef DoDeatharrestCheck
#undef SCOPE_GLOBAL
#undef SCOPE_LOCAL
#undef FO_CHAR_TOKEN_DELIMITER
#undef FO_CHAR_TOKEN_DIALOGUE
#undef MARKFUNCTION
#undef ScriptParams
#undef GetPointerToScriptVariable
#undef GetPointerToLocalVariable
#undef GetPointerToLocalArrayElement
#undef HasModelLoaded
#undef UNUSED_CHAR
#undef RANDOM_CHAR
#undef MISSION_CHAR
#undef REPLAY_CHAR
#undef PEDTYPE_COP
#undef PEDTYPE_MEDIC
#undef PEDTYPE_FIRE
#undef MAX
#undef MIN
#undef ABS
#undef DEGTORAD
#undef m_PrefsShowSubtitles
#undef COMMAND_AND_OR
#undef NO_ANDS_OR_ORS
#undef NUMBER_OF_ANDS1
#undef NUMBER_OF_ANDS8
#undef NUMBER_OF_ORS1
#undef NUMBER_OF_ORS8
#undef COMMAND_EXPLODE_CAR_IN_CUTSCENE_SHAKE_AND_BIT
#undef COMMAND_REMOVE_ALL_USER_3D_MARKERS
#undef COMMAND_ENABLE_DISABLED_ATTACTORS_ON_OBJECT
#undef COMMAND_IS_XBOX_PLAYER2_PRESSING_START
#undef m_pMyVehicle
#undef AutoPilot
#undef pHandling
#undef CPools
#undef CLEANUP_CAR
#undef CLEANUP_CHAR
#undef CLEANUP_OBJECT
#undef Score
#undef PlayerState
#undef PLAYERSTATE_HASDIED
#undef ObjectCreatedBy
#undef MISSION_OBJECT
#undef MISSION_BRAIN_OBJECT
#undef GetVehicleCreatedBy
#undef m_nVehicleFlags
#undef Transmission
#undef m_fMaxFlatVelocity
#undef m_fEngineAcceleration
#undef m_fMaxVelocity
#undef GetTimeInMilliseconds
#undef PED_DRIVING
#undef WANTED_CLEAN
#undef m_PoliceBackOff
#undef m_eDoorLockState
#undef WEAPONTYPE_UNARMED
#undef ANIM_STD_PED
#undef ANIM_STD_KO_FRONT
#undef TASK_PRIORITY_PRIMARY
#undef m_nHealth
#undef m_colour1
#undef m_colour2
#undef m_motionAnimGroup
#undef m_nAutomobileFlags
#undef BLIPTYPE_CONTACT
#undef BLIPTYPE_COORDS
#undef BLIPDISPLAY_BOTH
#undef ZONE_INFORMATION
#undef aGangStrengths
#undef ZoneInfoIndex
#undef FindZoneByLabelAndReturnIndex
#undef pMessage
#undef MI_TYPE_VEHICLE
#undef m_acquaintances
#undef m_nWeaponSlot
#undef m_PlayerGroup
#undef m_GroupStuffDisabled
#undef bBeatInfoPresent
#undef ANIM_CARRY_BOX
#undef m_PrefsUseWideScreen
#undef SetWantedLevel
#undef SetWantedLevelNoDrop
#undef GetPadState
#undef GetCorrectPedModelIndexForEmergencyServiceType
#undef SetCharCoordinates
#undef AddBigMessage
#undef AddBigMessageQ
#undef AddMessage
#undef AddMessageJumpQ
#undef GenerateNewOne
#undef SetGangWeapons
#undef SetBlipSprite
#undef SetCoordBlip
#undef AddEntityToList
#undef GetActualScriptThingIndex
#undef GetUniqueScriptThingIndex
#undef GetNewUniqueScriptThingIndex
#undef SetDefaultTaskAllocatorType
#undef GetPedFlag
#undef QueryModelsKilledByPlayer
#undef SetTextureAndModel
#undef RADTODEG
#undef BLIPTYPE_CAR
#undef BLIPTYPE_CHAR
#undef BLIPTYPE_OBJECT
#undef BLIPTYPE_PICKUP
#undef CLEANUP_PARTICLE
#undef CLEANUP_GROUP
#undef CLEANUP_ATTRACTOR
#undef CLEANUP_SEQUENCE_TASK
#undef CLEANUP_DECISION_MAKER
#undef CLEANUP_SEARCHLIGHT
#undef CLEANUP_CHECKPOINT
#undef CLEANUP_TEXTURE_DICTIONARY
#undef CLEANUP_PEDGROUP
#undef pPassengers
#undef CLEANUP_EFFECT_SYSTEM
#undef BLIPTYPE_SEARCHLIGHT
#undef pFXSystem
#undef pCustomPlateMaterial
#undef OverrideLights
#undef TakeControlAttachToEntity
#undef m_fDesiredHeading
#undef m_fCurrentHeading
#undef m_WeaponSlots
#undef m_nCurrentWeapon
#undef fMass
#undef fTurnMass
#undef fBuoyancyConstant
#undef PED_ARRESTED
#undef m_fScriptThrottleControl
#undef ZONE_NAVIGATION
#undef DoesPointLieWithinZoneWithGivenName
#undef AddMessageWithNumber
#undef AddMessageWithNumberJumpQ
#undef ForceWeather
#undef ForceWeatherNow
#undef ChangeBlipColour
#undef Fade
#undef TakeControl
#undef TakeControlNoEntity
#undef StoreShadowToBeRendered
#undef SetShootingAccuracy
#undef TotalNumCollectables
#undef CollectablesPickedUp
#undef PED_SPHERE_HEAD
#undef ChangeGarageType
#undef CreateCarGenerator
#undef GetCarGenerator
#undef AddClock
#undef AddCounter
#undef m_eWeaponType
#undef GetWeaponType
#undef bExtraHeavy
#undef bIsInWater
#undef bNotDamagedByBullets
#undef bNotDamagedByFlames
#undef bNotDamagedByCollisions
#undef bNotDamagedByMelee
#undef bOnlyDamagedByPlayer
#undef bIgnoresExplosions
#undef bUsesCollisionRecords
#undef m_nNoOfCollisionRecords
#undef m_aCollisionRecordPtrs
#undef bTaxiLightOn
#undef bDontDamageOnRoof
#undef bTakePanelDamage
#undef bLostTraction
#undef bSoftSuspension
#undef m_aWheelColPoints
#undef m_aWheelRatios
#undef APR_NOTHING
#undef APR_CAR
#undef APR_BIKE
#undef APR_HELI
#undef APR_BOAT
#undef APR_PLANE
#undef Cams
#undef ActiveCam
#undef SetMaximumWantedLevel
#undef m_Wanted
#undef m_EverybodyBackOff
#undef m_fMultiplier
#undef AddToResourceManager
#undef RemoveFromResourceManager
#undef RegisterCorona
#undef FindNodeClosestToCoors
#undef FindNodePointer
#undef MODELID_CAR_VORTEX
#undef MISSIONS_ATTEMPTED
#undef AddBigMessageWithNumber
#undef AddBigMessageWithNumberQ
#undef ClearThisPrintBigNow
#undef ScriptTextAtX
#undef ScriptTextAtY
#undef ScriptTextLabel
#undef ScriptTextXScale
#undef ScriptTextYScale
#undef ScriptTextColor
#undef ScriptTextJustify
#undef ScriptTextCentre
#undef ScriptTextWrapX
#undef ScriptTextCentreSize
#undef ScriptTextBackgrnd
#undef ScriptTextProportional
#undef ScriptTextFontStyle
#undef ScriptTextBeforeFade
#undef ScriptTextRightJustify
#undef ScriptTextDropShadowColour
#undef ScriptTextDropShadow
#undef CharCreatedBy
#undef SetStayInSamePlaceFlag
#undef m_nArmour
#undef MaxArmour
#undef m_iplIndex
#undef m_MenuActive
#undef ScriptRectBeforeFade
#undef PEDTYPE_CIVMALE
#undef PEDTYPE_CIVFEMALE
#undef VEHICLE_TYPE_MONSTERTRUCK
#undef VEHICLE_TYPE_QUADBIKE
#undef m_nBikeFlags
#undef eWindowType
#undef ScriptSpriteIndex
#undef ScriptRectMinX
#undef ScriptRectMinY
#undef ScriptRectMaxX
#undef ScriptRectMaxY
#undef ScriptRectRotation
#undef ScriptRectColour
#undef pTitle
#undef pText
#undef WINDOW_NONE
#undef WINDOW_HEADER_AND_TEXT
#undef WINDOW_HEADER_NO_TEXT
#undef WINDOW_SOLID_COLOUR
#undef WINDOW_SPRITE_NO_ROTATION
#undef WINDOW_SPRITE_WITH_ROTATION
#undef m_pMat
#undef FreezeTimers
#undef DISABLE_CUTSCENES
#undef PlaceMarkerSet
#undef SetStatus
#undef m_MoneyCarried
#undef bFreeHealthCare
#undef pRemoteVehicle
#undef m_nAmmoTotal
#undef m_pPedIntelligence
#undef pDriver
#undef ForcedRandomSeed
#undef m_pMouseLockOnRecruitPed
#undef pNext
#undef m_pAttachToEntity
#undef GetNearbyPeds
#undef RetuneRadio
#undef SetStatValue
#undef RegisterFastestTime
#undef IsCheatActive
#undef VEHICLE_TYPE_CAR
#undef Damage
#undef CAR_ON_FIRE_HEALTH
#undef GivePlayerRemoteControlledCar
#undef TellHeliToGoToCoors
#undef CreateDeadPedPickupCoors
#undef mFlags
#undef m_aWheelAngularVelocity
#undef eVisibleArea
#undef RemoveBuildingsNotInArea
#undef SetMissionDoesntRequireAnimations
#undef SetShortRangeCoordBlip
#undef bCoorsFrozenByScript
#undef bInfiniteMassFixed
#undef m_pNOCollisionVehicle
#undef m_areaCode
#undef SetCJMood
#undef tAudioEvent
#undef m_loaded
#undef nBestCarTwoWheelsTimeMs
#undef fBestCarTwoWheelsDistM
#undef nBestBikeWheelieTimeMs
#undef fBestBikeWheelieDistM
#undef nBestBikeStoppieTimeMs
#undef fBestBikeStoppieDistM
#undef PlayerPedData
#undef bCanDoDriveBy
#undef FireProof
#undef AddPed
#undef SetTask
#undef pLastDamageEntity
#undef m_LastTimeGunFired
#undef m_eStoredWeapon
#undef WasCutsceneSkipped
#undef MODELID_BOAT_PREDATOR
#undef EnableLegimateCheat
#undef Source
#undef Front
#undef Up
#undef WorldViewerBeingUsed
#undef GetWheelStatus
#undef m_pClothes
#undef m_bFadeDrunkenness
#undef MaxHealth
#undef m_nNoOfStaticFrames
#undef RegisterBestPosition
#undef IncrementStat
#undef GetStatValue
#undef TTaskComplexSeekEntityStandard
#undef MODELID_CAR_TAXI
#undef MODELID_CAR_CABBIE
#undef UNIQUE_SCRIPT_SPHERE
#undef UNIQUE_SCRIPT_EFFECT_SYSTEM
#undef UNIQUE_SCRIPT_SEARCHLIGHT
#undef UNIQUE_SCRIPT_CHECKPOINT
#undef UNIQUE_SCRIPT_SEQUENCE_TASK
#undef UNIQUE_SCRIPT_FIRE
#undef UNIQUE_SCRIPT_ATTRACTOR
#undef UNIQUE_SCRIPT_DECISION_MAKER
#undef UNIQUE_SCRIPT_PEDGROUP
#undef PROGRESS_MADE
#undef TOTAL_PROGRESS
#undef FURTHEST_HOOP
#undef DISTANCE_TRAVELLED_ON_FOOT
#undef DISTANCE_TRAVELLED_BY_CAR
#undef DISTANCE_TRAVELLED_BY_MOTORBIKE
#undef DISTANCE_TRAVELLED_BY_BOAT
#undef DISTANCE_TRAVELLED_BY_GOLF_CART
#undef DISTANCE_TRAVELLED_BY_HELICOPTER
#undef DISTANCE_TRAVELLED_BY_PLANE
#undef LONGEST_WHEELIE_DISTANCE
#undef LONGEST_STOPPIE_DISTANCE
#undef LONGEST_2_WHEELS_DISTANCE
#undef WEAPON_BUDGET
#undef FASHION_BUDGET
#undef PROPERTY_BUDGET
#undef AUTO_REPAIR_AND_PAINTING_BUDGET
#undef LONGEST_WHEELIE_TIME
#undef LONGEST_STOPPIE_TIME
#undef LONGEST_2_WHEELS_TIME
#undef FOOD_BUDGET
#undef FAT
#undef STAMINA
#undef MUSCLE
#undef MAX_HEALTH
#undef SEX_APPEAL
#undef DISTANCE_TRAVELLED_BY_SWIMMING
#undef DISTANCE_TRAVELLED_BY_BICYCLE
#undef DISTANCE_TRAVELLED_ON_TREADMILL
#undef DISTANCE_TRAVELLED_ON_EXERCISE_BIKE
#undef TATTOO_BUDGET
#undef HAIRDRESSING_BUDGET
#undef GIRLFRIEND_BUDGET
#undef PROSTITUTE_BUDGET
#undef FURNITURE_BUDGET
#undef MONEY_SPENT_GAMBLING
#undef MONEY_MADE_FROM_PIMPING
#undef MONEY_WON_GAMBLING
#undef BIGGEST_GAMBLING_WIN
#undef BIGGEST_GAMBLING_LOSS
#undef LARGEST_BURGLARY_SWAG
#undef MONEY_MADE_FROM_BURGLARY
#undef MONEY_SPENT_BUILDING_PROPERTY
#undef LONGEST_TREADMILL_TIME
#undef LONGEST_EXERCISE_BIKE_TIME
#undef HEAVIEST_WEIGHT_ON_BENCH_PRESS
#undef HEAVIEST_WEIGHT_ON_DUMBBELLS
#undef BEST_TIME_IN_8_TRACK
#undef BMX_BEST_TIME
#undef LIGHTEST_WEIGHT
#undef LONGEST_CHASE_TIME_WITH_5_OR_MORE_STARS
#undef LAST_CHASE_TIME_WITH_5_OR_MORE_STARS
#undef WAGE_BILL
#undef STRIP_CLUB_BUDGET
#undef CAR_MODIFICATION_BUDGET
#undef TIME_SPENT_SHOPPING
#undef TIME_SPENT_GAMBLING
#undef TIME_SPENT_ON_LONGEST_MISSION
#undef TIME_SPENT_ON_QUICKEST_MISSION
#undef AVERAGE_MISSION_TIME
#undef DRUGS_BUDGET
#undef TOTAL_SHOPPING_BUDGET
#undef TIME_SPENT_UNDERWATER
#undef TOTAL_RESPECT
#undef GIRLFRIEND_RESPECT
#undef CLOTHES_RESPECT
#undef FITNESS_RESPECT
#undef RESPECT
#undef PISTOL_SKILL
#undef SILENCED_PISTOL_SKILL
#undef DESERT_EAGLE_SKILL
#undef SHOTGUN_SKILL
#undef SAWN_OFF_SHOTGUN_SKILL
#undef COMBAT_SHOTGUN_SKILL
#undef MACHINE_PISTOL_SKILL
#undef SMG_SKILL
#undef AK_47_SKILL
#undef M4_SKILL
#undef RIFLE_SKILL
#undef APPEARANCE
#undef GAMBLING
#undef PEOPLE_WASTED_BY_OTHERS
#undef PEOPLE_YOUVE_WASTED
#undef ROAD_VEHICLES_DESTROYED
#undef BOATS_DESTROYED
#undef PLANES_HELICOPTERS_DESTROYED
#undef COST_OF_PROPERTY_DAMAGED
#undef BULLETS_FIRED
#undef KGS_OF_EXPLOSIVES_USED
#undef BULLETS_THAT_HIT
#undef TIRES_POPPED_WITH_GUNFIRE
#undef NUMBER_OF_HEADSHOTS
#undef TOTAL_NUMBER_OF_WANTED_STARS_ATTAINED
#undef TOTAL_NUMBER_OF_WANTED_STARS_EVADED
#undef TIMES_BUSTED
#undef DAYS_PASSED_IN_GAME
#undef NUMBER_OF_HOSPITAL_VISITS
#undef SAFEHOUSE_VISITS
#undef TIMES_CHEATED
#undef VEHICLE_RESPRAYS
#undef MAXIMUM_INSANE_JUMP_DISTANCE
#undef MAXIMUM_INSANE_JUMP_HEIGHT
#undef MAXIMUM_INSANE_JUMP_FLIPS
#undef MAXIMUM_INSANE_JUMP_ROTATION
#undef BEST_INSANE_STUNT_AWARDED
#undef UNIQUE_JUMPS_FOUND
#undef UNIQUE_JUMPS_DONE
#undef MISSION_ATTEMPTS
#undef MISSIONS_PASSED
#undef TOTAL_NUMBER_OF_MISSIONS_IN_GAME
#undef CASH_MADE_IN_A_TAXI
#undef PASSENGERS_DROPPED_OFF
#undef PEOPLE_SAVED_IN_AN_AMBULANCE
#undef CRIMINALS_KILLED_ON_VIGILANTE_MISSION
#undef TOTAL_FIRES_EXTINGUISHED
#undef PACKAGES_DELIVERED
#undef ASSASSINATIONS
#undef LAST_DANCE_SCORE
#undef HIGHEST_VIGILANTE_MISSION_LEVEL
#undef HIGHEST_PARAMEDIC_MISSION_LEVEL
#undef HIGHEST_FIREFIGHTER_MISSION_LEVEL
#undef DRIVING_SKILL
#undef NUMBER_OF_TRUCK_MISSIONS_PASSED
#undef MONEY_MADE_IN_TRUCK
#undef RECRUITED_GANG_MEMBERS_KILLED
#undef ARMOR
#undef ENERGY
#undef PHOTOGRAPHS_TAKEN
#undef RAMPAGES_ATTEMPTED
#undef RAMPAGES_PASSED
#undef FLIGHT_TIME
#undef TIMES_DROWNED
#undef NUMBER_OF_GIRLS_PIMPED
#undef BEST_POSITION_IN_8_TRACK
#undef TIME_ON_JETPACK
#undef SHOOTING_RANGE_LEVELS_PASSED
#undef MOST_CARS_PARKED_ON_VALET_PARKING
#undef KILLS_SINCE_LAST_CHECKPOINT
#undef TOTAL_LEGITIMATE_KILLS
#undef NUMBER_OF_BLOODRING_KILLS
#undef TOTAL_TIME_IN_BLOODRING
#undef NO_MORE_HURRICANES_FLAG
#undef CITY_UNLOCKED
#undef NUMBER_OF_POLICE_BRIBES
#undef NUMBER_OF_CARS_STOLEN
#undef CURRENT_NUMBER_OF_GIRLFRIENDS
#undef NUMBER_OF_DISASTROUS_DATES
#undef NUMBER_OF_GIRLS_DATED
#undef NUMBER_OF_TIMES_SCORED_WITH_A_GIRL
#undef NUMBER_OF_SUCCESSFUL_DATES
#undef NUMBER_OF_GIRLS_DUMPED
#undef NUMBER_OF_PROSTITUTES_VISITED
#undef NUMBER_OF_HOUSES_BURGLED
#undef NUMBER_OF_SAFES_CRACKED
#undef BURGULAR_STATUS
#undef NUMBER_OF_STOLEN_ITEMS_SOLD
#undef NUMBER_OF_EIGHT_BALLS_IN_POOL
#undef TOTAL_NUMBER_OF_WINS_PLAYING_POOL
#undef TOTAL_NUMBER_OF_LOSSES_PLAYING_POOL
#undef TIMES_VISITED_THE_GYM
#undef BODY_TYPE
#undef NUMBER_OF_MEALS_EATEN
#undef MONEY_MADE_IN_QUARRY
#undef TIME_TAKEN_TO_COMPLETE_QUARRY
#undef NUMBER_OF_FURNITURE_PURCHASED
#undef FAVORITE_CONSOLE_GAME
#undef HIGHEST_CIVILIAN_PEDS_KILLED_ON_RAMPAGE
#undef HIGHEST_POLICE_PEDS_KILLED_ON_RAMPAGE
#undef HIGHEST_CIVILIAN_VEHICLES_DESTROYED_ON_RAMPAGE
#undef HIGHEST_POLICE_VEHICLES_DESTROYED_ON_RAMPAGE
#undef HIGHEST_NUMBER_OF_TANKS_DESTROYED_ON_RAMPAGE
#undef PIMPING_LEVEL
#undef KICKSTART_BEST_SCORE
#undef BEST_LAP_TIME_IN_8_TRACK
#undef NUMBER_OF_VEHICLES_EXPORTED
#undef NUMBER_OF_VEHICLES_IMPORTED
#undef HIGHEST_BASKETBALL_SCORE
#undef FIRES_STARTED
#undef AMOUNT_OF_DRUGS_SOLD
#undef AMOUNT_OF_DRUGS_BOUGHT
#undef BEST_LAP_TIME_IN_DIRT_TRACK
#undef BEST_TIME_IN_DIRT_TRACK
#undef BEST_POSITION_IN_DIRT_TRACK
#undef NRG_500_BEST_TIME
#undef FLYING_SKILL
#undef RESPECT_MISSION
#undef LUNG_CAPACITY
#undef GAME_GORE_RATING
#undef GAME_SEX_RATING
#undef RESPECT_MISSION_TOTAL
#undef BIKE_SKILL
#undef CYCLING_SKILL
#undef SNAPSHOTS_TAKEN
#undef TOTAL_SNAPSHOTS
#undef LUCK
#undef TERRITORIES_TAKEN_OVER
#undef TERRITORIES_LOST
#undef TERRITORIES_HELD
#undef HIGHEST_NUMBER_OF_TERRITORIES_HELD
#undef GANG_MEMBERS_RECRUITED
#undef ENEMY_GANG_MEMBERS_KILLED
#undef FRIENDLY_GANG_MEMBERS_KILLED
#undef HORSESHOES_COLLECTED
#undef TOTAL_HORSESHOES
#undef OYSTERS_COLLECTED
#undef TOTAL_OYSTERS
#undef CALORIES
#undef KEEPIE_UPPY_BEST_TIME
#undef PROGRESS_WITH_DENISE
#undef PROGRESS_WITH_MICHELLE
#undef PROGRESS_WITH_HELENA
#undef PROGRESS_WITH_BARBARA
#undef PROGRESS_WITH_KATIE
#undef PROGRESS_WITH_MILLIE
#undef BEST_POSITION_IN_LOWRIDER_RACE
#undef BEST_TIME_IN_LOWRIDER_RACE
#undef BEST_POSITION_IN_LITTLE_LOOP
#undef BEST_TIME_IN_LITTLE_LOOP
#undef BEST_POSITION_IN_BACKROAD_WANDERER
#undef BEST_TIME_IN_BACKROAD_WANDERER
#undef BEST_POSITION_IN_CITY_CIRCUIT
#undef BEST_TIME_IN_CITY_CIRCUIT
#undef BEST_POSITION_IN_VINEWOOD
#undef BEST_TIME_IN_VINEWOOD
#undef BEST_POSITION_IN_FREEWAY
#undef BEST_TIME_IN_FREEWAY
#undef BEST_POSITION_IN_INTO_THE_COUNTRY
#undef BEST_TIME_IN_INTO_THE_COUNTRY
#undef BEST_POSITION_IN_BADLANDS_A
#undef BEST_TIME_IN_BADLANDS_A
#undef BEST_POSITION_IN_BADLANDS_B
#undef BEST_TIME_IN_BADLANDS_B
#undef BEST_POSITION_IN_DIRTBIKE_DANGER
#undef BEST_TIME_IN_DIRTBIKE_DANGER
#undef BEST_POSITION_IN_BANDITO_COUNTY
#undef BEST_TIME_IN_BANDITO_COUNTY
#undef BEST_POSITION_IN_GO_GO_KART
#undef BEST_TIME_IN_GO_GO_KART
#undef BEST_POSITION_IN_SAN_FIERRO_FASTLANE
#undef BEST_TIME_IN_SAN_FIERRO_FASTLANE
#undef BEST_POSITION_IN_SAN_FIERRO_HILLS
#undef BEST_TIME_IN_SAN_FIERRO_HILLS
#undef BEST_POSITION_IN_COUNTRY_ENDURANCE
#undef BEST_TIME_IN_COUNTRY_ENDURANCE
#undef BEST_POSITION_IN_SF_TO_LV
#undef BEST_TIME_IN_SF_TO_LV
#undef BEST_POSITION_IN_DAM_RIDER
#undef BEST_TIME_IN_DAM_RIDER
#undef BEST_POSITION_IN_DESERT_TRICKS
#undef BEST_TIME_IN_DESERT_TRICKS
#undef BEST_POSITION_IN_LV_RINGROAD
#undef BEST_TIME_IN_LV_RINGROAD
#undef BEST_TIME_IN_WORLD_WAR_ACES
#undef BEST_TIME_IN_BARNSTORMING
#undef BEST_TIME_IN_MILITARY_SERVICE
#undef BEST_TIME_IN_CHOPPER_CHECKPOINT
#undef BEST_TIME_IN_WHIRLY_BIRD_WAYPOINT
#undef BEST_TIME_IN_HELI_HELL
#undef DRIVE_THRU_MISSION_ACCOMPLISHED
#undef MANAGEMENT_ISSUES_MISSION_ACCOMPLISHED
#undef YAY_KA_BOOM_BOOM_MISSION_ACCOMPLISHED
#undef FISH_IN_A_BARREL_MISSION_ACCOMPLISHED
#undef BREAKING_THE_BANK_AT_CALIGULAS_MISSION_ACCOMPLISHED
#undef A_HOME_IN_THE_HILLS_MISSION_ACCOMPLISHED
#undef MAYBE_SET_RIOT_MODE
#undef RYDERS_MISSION_ROBBING_UNCLE_SAM_ACCOMPLISHED
#undef MIKE_TORENO_MISSION_ACCOMPLISHED
#undef ARCHITECTURAL_ESPIONAGE_MISSION_ACCOMPLISHED
#undef JIZZY_MISSION_ACCOMPLISHED
#undef REUNITING_THE_FAMILIES_MISSION_ACCOMPLISHED
#undef SMALL_TOWN_BANK_MISSION_ACCOMPLISHED
#undef PHOTO_OPPORTUNITY_MISSION_ACCOMPLISHED
#undef DON_PEYOTE_MISSION_ACCOMPLISHED
#undef LOCAL_LIQUOR_STORE_MISSION_ACCOMPLISHED
#undef BADLANDS_MISSION_ACCOMPLISHED
#undef PLAYING_TIME
#undef HIDDEN_PACKAGES_FOUND
#undef TAGS_SPRAYED
#undef LEAST_FAVORITE_GANG
#undef GANG_MEMBERS_WASTED
#undef CRIMINALS_WASTED
#undef MOST_FAVORITE_RADIO_STATION
#undef LEAST_FAVORITE_RADIO_STATION
#undef CURRENT_WEAPON_SKILL
#undef WEAPON_SKILL_LEVELS
#undef PILOT_RANKING
#undef STRONGEST_GANG
#undef MONEY_LOST_GAMBLING
#undef RIOT_MISSION_ACCOMPLISHED
#undef GANG_STRENGTH
#undef TERRITORY_UNDER_CONTROL
#undef ARE_YOU_GOING_TO_SAN_FIERRO_MISSION_ACCOMPLISHED
#undef HIGH_NOON_MISSION_ACCOMPLISHED
#undef THE_GREEN_SABRE_MISSION_ACCOMPLISHED
#undef MAYBE_CATALINA_MEETING
#undef MAYBE_WU_ZI_MEETING
#undef WEAPONTYPE_BRASSKNUCKLE
#undef WEAPONTYPE_GOLFCLUB
#undef WEAPONTYPE_NIGHTSTICK
#undef WEAPONTYPE_KNIFE
#undef WEAPONTYPE_BASEBALLBAT
#undef WEAPONTYPE_SHOVEL
#undef WEAPONTYPE_POOL_CUE
#undef WEAPONTYPE_KATANA
#undef WEAPONTYPE_CHAINSAW
#undef WEAPONTYPE_DILDO1
#undef WEAPONTYPE_DILDO2
#undef WEAPONTYPE_VIBE1
#undef WEAPONTYPE_VIBE2
#undef WEAPONTYPE_FLOWERS
#undef WEAPONTYPE_CANE
#undef WEAPONTYPE_GRENADE
#undef WEAPONTYPE_TEARGAS
#undef WEAPONTYPE_MOLOTOV
#undef WEAPONTYPE_ROCKET
#undef WEAPONTYPE_ROCKET_HS
#undef WEAPONTYPE_FREEFALL_BOMB
#undef WEAPONTYPE_PISTOL
#undef WEAPONTYPE_PISTOL_SILENCED
#undef WEAPONTYPE_DESERT_EAGLE
#undef WEAPONTYPE_SHOTGUN
#undef WEAPONTYPE_SAWNOFF_SHOTGUN
#undef WEAPONTYPE_SPAS12_SHOTGUN
#undef WEAPONTYPE_MICRO_UZI
#undef WEAPONTYPE_MP5
#undef WEAPONTYPE_AK47
#undef WEAPONTYPE_M4
#undef WEAPONTYPE_TEC9
#undef WEAPONTYPE_COUNTRYRIFLE
#undef WEAPONTYPE_SNIPERRIFLE
#undef WEAPONTYPE_RLAUNCHER
#undef WEAPONTYPE_RLAUNCHER_HS
#undef WEAPONTYPE_FLAMETHROWER
#undef WEAPONTYPE_MINIGUN
#undef WEAPONTYPE_REMOTE_SATCHEL_CHARGE
#undef WEAPONTYPE_DETONATOR
#undef WEAPONTYPE_SPRAYCAN
#undef WEAPONTYPE_EXTINGUISHER
#undef WEAPONTYPE_CAMERA
#undef WEAPONTYPE_NIGHTVISION
#undef WEAPONTYPE_INFRARED
#undef WEAPONTYPE_PARACHUTE
#undef WEAPONTYPE_LAST_WEAPON
#undef WEAPONTYPE_ARMOUR
#undef WEAPONTYPE_RAMMEDBYCAR
#undef WEAPONTYPE_RUNOVERBYCAR
#undef WEAPONTYPE_EXPLOSION
#undef WEAPONTYPE_UZI_DRIVEBY
#undef WEAPONTYPE_DROWNING
#undef WEAPONTYPE_FALL
#undef WEAPONTYPE_UNIDENTIFIED
#undef WEAPONTYPE_ANYMELEE
#undef WEAPONTYPE_ANYWEAPON
#undef WEAPONTYPE_FLARE
#undef MODEL_HYDRA
