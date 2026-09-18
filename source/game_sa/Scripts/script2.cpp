#pragma warning(disable: 4101 4244 4805 4060 4189 4505 4702)

#include "StdInc.h"

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
            pVehicle->m_autoPilot.SetDrivingStyle(static_cast<eCarDrivingStyle>(ScriptParams[1]));
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

// stub: sa 0x47C100
OpcodeResult CRunningScript::ProcessCommands300To399(i32 CurrCommand)
{
    MARKFUNCTION(0x47C100);
    switch (CurrCommand)
    {
        case COMMAND_SET_CHAR_INVINCIBLE:
        {
            return OR_CONTINUE;
        }
        case COMMAND_SET_PLAYER_INVINCIBLE:
        {
            return OR_CONTINUE;
        }
        case COMMAND_SET_CHAR_GRAPHIC_TYPE:
        {
            return OR_CONTINUE;
        }
        case COMMAND_SET_PLAYER_GRAPHIC_TYPE:
        {
            return OR_CONTINUE;
        }
        case COMMAND_HAS_PLAYER_BEEN_ARRESTED:
        {
            return OR_CONTINUE;
        }
        case COMMAND_STOP_CHAR_DRIVING:
        {
            return OR_CONTINUE;
        }
        case COMMAND_KILL_CHAR:
        {
            return OR_CONTINUE;
        }
        case COMMAND_SET_FAVOURITE_CAR_MODEL_FOR_CHAR:
        {
            return OR_CONTINUE;
        }
        case COMMAND_SET_CHAR_OCCUPATION:
        {
            return OR_CONTINUE;
        }
        case COMMAND_CHANGE_CAR_LOCK:
        {
            return OR_CONTINUE;
        }
        case COMMAND_SHAKE_CAM_WITH_POINT:
        {
            return OR_CONTINUE;
        }
        case COMMAND_IS_CAR_MODEL:
        {
            return OR_CONTINUE;
        }
        case COMMAND_IS_CAR_REMAP:
        {
            return OR_CONTINUE;
        }
        case COMMAND_HAS_CAR_JUST_SUNK:
        {
            return OR_CONTINUE;
        }
        case COMMAND_SET_CAR_NO_COLLIDE:
        {
            return OR_CONTINUE;
        }
        case COMMAND_IS_CAR_DEAD_IN_AREA_2D:
        {
            return OR_CONTINUE;
        }
        case COMMAND_IS_CAR_DEAD_IN_AREA_3D:
        {
            return OR_CONTINUE;
        }
        case COMMAND_IS_TRAILER_ATTACHED:
        {
            return OR_CONTINUE;
        }
        case COMMAND_IS_CAR_ON_TRAILER:
        {
            return OR_CONTINUE;
        }
        case COMMAND_HAS_CAR_GOT_WEAPON:
        {
            return OR_CONTINUE;
        }
        case COMMAND_PARK:
        {
            return OR_CONTINUE;
        }
        case COMMAND_HAS_PARK_FINISHED:
        {
            return OR_CONTINUE;
        }
        case COMMAND_KILL_ALL_PASSENGERS:
        {
            return OR_CONTINUE;
        }
        case COMMAND_SET_CAR_BULLETPROOF:
        {
            return OR_CONTINUE;
        }
        case COMMAND_SET_CAR_FLAMEPROOF:
        {
            return OR_CONTINUE;
        }
        case COMMAND_SET_CAR_ROCKETPROOF:
        {
            return OR_CONTINUE;
        }
        case COMMAND_IS_CARBOMB_ACTIVE:
        {
            return OR_CONTINUE;
        }
        case COMMAND_GIVE_CAR_ALARM:
        {
            return OR_CONTINUE;
        }
        case COMMAND_PUT_CAR_ON_TRAILER:
        {
            return OR_CONTINUE;
        }
        case COMMAND_IS_CAR_CRUSHED:
        {
            return OR_CONTINUE;
        }
        case COMMAND_CREATE_GANG_CAR:
        {
            return OR_CONTINUE;
        }
        case COMMAND_CREATE_CAR_GENERATOR:
        {
            return OR_CONTINUE;
        }
        case COMMAND_SWITCH_CAR_GENERATOR:
        {
            return OR_CONTINUE;
        }
        case COMMAND_ADD_PAGER_MESSAGE:
        {
            return OR_CONTINUE;
        }
        case COMMAND_DISPLAY_ONSCREEN_TIMER:
        {
            return OR_CONTINUE;
        }
        case COMMAND_CLEAR_ONSCREEN_TIMER:
        {
            return OR_CONTINUE;
        }
        case COMMAND_DISPLAY_ONSCREEN_COUNTER:
        {
            return OR_CONTINUE;
        }
        case COMMAND_CLEAR_ONSCREEN_COUNTER:
        {
            return OR_CONTINUE;
        }
        case COMMAND_SET_ZONE_CAR_INFO:
        {
            return OR_CONTINUE;
        }
        case COMMAND_IS_CHAR_IN_GANG_ZONE:
        {
            return OR_CONTINUE;
        }
        case COMMAND_IS_CHAR_IN_ZONE:
        {
            return OR_CONTINUE;
        }
        case COMMAND_SET_CAR_DENSITY:
        {
            return OR_CONTINUE;
        }
        case COMMAND_SET_PED_DENSITY:
        {
            return OR_CONTINUE;
        }
        case COMMAND_POINT_CAMERA_AT_PLAYER:
        {
            return OR_CONTINUE;
        }
        case COMMAND_POINT_CAMERA_AT_CAR:
        {
            return OR_CONTINUE;
        }
        case COMMAND_POINT_CAMERA_AT_CHAR:
        {
            return OR_CONTINUE;
        }
        case COMMAND_RESTORE_CAMERA:
        {
            return OR_CONTINUE;
        }
        case COMMAND_SHAKE_PAD:
        {
            return OR_CONTINUE;
        }
        case COMMAND_SET_ZONE_PED_INFO:
        {
            return OR_CONTINUE;
        }
        case COMMAND_SET_TIME_SCALE:
        {
            return OR_CONTINUE;
        }
        case COMMAND_IS_CAR_IN_AIR:
        {
            return OR_CONTINUE;
        }
        case COMMAND_SET_FIXED_CAMERA_POSITION:
        {
            return OR_CONTINUE;
        }
        case COMMAND_POINT_CAMERA_AT_POINT:
        {
            return OR_CONTINUE;
        }
        case COMMAND_ADD_BLIP_FOR_CAR_OLD:
        {
            return OR_CONTINUE;
        }
        case COMMAND_ADD_BLIP_FOR_CHAR_OLD:
        {
            return OR_CONTINUE;
        }
        case COMMAND_ADD_BLIP_FOR_OBJECT_OLD:
        {
            return OR_CONTINUE;
        }
        case COMMAND_REMOVE_BLIP:
        {
            return OR_CONTINUE;
        }
        case COMMAND_CHANGE_BLIP_COLOUR:
        {
            return OR_CONTINUE;
        }
        case COMMAND_DIM_BLIP:
        {
            return OR_CONTINUE;
        }
        case COMMAND_ADD_BLIP_FOR_COORD_OLD:
        {
            return OR_CONTINUE;
        }
        case COMMAND_CHANGE_BLIP_SCALE:
        {
            return OR_CONTINUE;
        }
        case COMMAND_SET_FADING_COLOUR:
        {
            return OR_CONTINUE;
        }
        case COMMAND_DO_FADE:
        {
            return OR_CONTINUE;
        }
        case COMMAND_GET_FADING_STATUS:
        {
            return OR_CONTINUE;
        }
        case COMMAND_ADD_HOSPITAL_RESTART:
        {
            return OR_CONTINUE;
        }
        case COMMAND_ADD_POLICE_RESTART:
        {
            return OR_CONTINUE;
        }
        case COMMAND_OVERRIDE_NEXT_RESTART:
        {
            return OR_CONTINUE;
        }
        case COMMAND_DRAW_SHADOW:
        {
            return OR_CONTINUE;
        }
        case COMMAND_GET_PLAYER_HEADING:
        {
            return OR_CONTINUE;
        }
        case COMMAND_SET_PLAYER_HEADING:
        {
            return OR_CONTINUE;
        }
        case COMMAND_GET_CHAR_HEADING:
        {
            return OR_CONTINUE;
        }
        case COMMAND_SET_CHAR_HEADING:
        {
            return OR_CONTINUE;
        }
        case COMMAND_GET_CAR_HEADING:
        {
            return OR_CONTINUE;
        }
        case COMMAND_SET_CAR_HEADING:
        {
            return OR_CONTINUE;
        }
        case COMMAND_GET_OBJECT_HEADING:
        {
            return OR_CONTINUE;
        }
        case COMMAND_SET_OBJECT_HEADING:
        {
            return OR_CONTINUE;
        }
        case COMMAND_IS_PLAYER_TOUCHING_OBJECT:
        {
            return OR_CONTINUE;
        }
        case COMMAND_IS_CHAR_TOUCHING_OBJECT:
        {
            return OR_CONTINUE;
        }
        case COMMAND_SET_PLAYER_AMMO:
        {
            return OR_CONTINUE;
        }
        case COMMAND_SET_CHAR_AMMO:
        {
            return OR_CONTINUE;
        }
        case COMMAND_SET_CAR_AMMO:
        {
            return OR_CONTINUE;
        }
        case COMMAND_LOAD_CAMERA_SPLINE:
        {
            return OR_CONTINUE;
        }
        case COMMAND_MOVE_CAMERA_ALONG_SPLINE:
        {
            return OR_CONTINUE;
        }
        case COMMAND_GET_CAMERA_POSITION_ALONG_SPLINE:
        {
            return OR_CONTINUE;
        }
        case COMMAND_DECLARE_MISSION_FLAG:
        {
            return OR_CONTINUE;
        }
        case COMMAND_DECLARE_MISSION_FLAG_FOR_CONTACT:
        {
            return OR_CONTINUE;
        }
        case COMMAND_DECLARE_BASE_BRIEF_ID_FOR_CONTACT:
        {
            return OR_CONTINUE;
        }
        case COMMAND_IS_PLAYER_HEALTH_GREATER:
        {
            return OR_CONTINUE;
        }
        case COMMAND_IS_CHAR_HEALTH_GREATER:
        {
            return OR_CONTINUE;
        }
        case COMMAND_IS_CAR_HEALTH_GREATER:
        {
            return OR_CONTINUE;
        }
        case COMMAND_ADD_BLIP_FOR_CAR:
        {
            return OR_CONTINUE;
        }
        case COMMAND_ADD_BLIP_FOR_CHAR:
        {
            return OR_CONTINUE;
        }
        case COMMAND_ADD_BLIP_FOR_OBJECT:
        {
            return OR_CONTINUE;
        }
        case COMMAND_ADD_BLIP_FOR_CONTACT_POINT:
        {
            return OR_CONTINUE;
        }
        case COMMAND_ADD_BLIP_FOR_COORD:
        {
            return OR_CONTINUE;
        }
        case COMMAND_CHANGE_BLIP_DISPLAY:
        {
            return OR_CONTINUE;
        }
        case COMMAND_ADD_ONE_OFF_SOUND:
        {
            return OR_CONTINUE;
        }
        case COMMAND_ADD_CONTINUOUS_SOUND:
        {
            return OR_CONTINUE;
        }
        case COMMAND_REMOVE_SOUND:
        {
            return OR_CONTINUE;
        }
        case COMMAND_IS_CAR_STUCK_ON_ROOF:
        {
            return OR_CONTINUE;
        }
    }
    return OR_INTERRUPT;
}

// MARK: 400To499

// stub: sa 0x47D210
OpcodeResult CRunningScript::ProcessCommands400To499(i32 CurrCommand)
{
    MARKFUNCTION(0x47D210);
    switch (CurrCommand) {}
    return OR_INTERRUPT;
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
                pTask = new CTaskComplexDie(WEAPONTYPE_UNARMED, ANIM_STD_PED, ANIM_STD_KO_FRONT, 4.0f, 0.0f, false, false, eDirection::FORWARD, false);
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
            CTheScripts::ScriptResourceManager.AddToResourceManager(MODEL_SPECIAL01 + ScriptParams[0], RESOURCE_TYPE_MODEL_OR_SPECIAL_CHAR, this);
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
            CTheScripts::ScriptResourceManager.AddToResourceManager(ModelIndex, RESOURCE_TYPE_MODEL_OR_SPECIAL_CHAR, this);
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
            if (CTheScripts::ScriptResourceManager.RemoveFromResourceManager(ModelIndex, RESOURCE_TYPE_MODEL_OR_SPECIAL_CHAR, this))
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
            CCoronas::RegisterCorona((u_native)this + (u_native)&PCPointer[(u_native)((Y1 + 3000.0f) * 12001.0f + X1 + 3000.0f)], nullptr, ScriptParams[6], ScriptParams[7], ScriptParams[8], 255, TempCoors, *(f32*)&ScriptParams[3], 450.0f, static_cast<eCoronaType>(ScriptParams[4]), static_cast<eCoronaFlareType>(ScriptParams[5]), eCoronaReflType::CORREFL_SIMPLE, eCoronaLOSCheck::LOSCHECK_OFF, eCoronaTrail::TRAIL_OFF, 0.0f, false, 1.5f, false, 15.0f, false, false);
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
            if (!CTheScripts::ScriptResourceManager.RemoveFromResourceManager(MODEL_SPECIAL01 + ScriptParams[0], RESOURCE_TYPE_MODEL_OR_SPECIAL_CHAR, this))
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
                pPhysical->physicalFlags.bInvulnerable = true;
            }
            else
            {
                pPhysical->physicalFlags.bInvulnerable = false;
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
                pPhysical->physicalFlags.bInvulnerable = true;
            }
            else
            {
                pPhysical->physicalFlags.bInvulnerable = false;
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
                pPhysical->physicalFlags.bBulletProof = true;
            }
            else
            {
                pPhysical->physicalFlags.bBulletProof = false;
            }
            if (ScriptParams[2])
            {
                pPhysical->physicalFlags.bFireProof = true;
            }
            else
            {
                pPhysical->physicalFlags.bFireProof = false;
            }
            if (ScriptParams[3])
            {
                pPhysical->physicalFlags.bExplosionProof = true;
            }
            else
            {
                pPhysical->physicalFlags.bExplosionProof = false;
            }
            if (ScriptParams[4])
            {
                pPhysical->physicalFlags.bCollisionProof = true;
            }
            else
            {
                pPhysical->physicalFlags.bCollisionProof = false;
            }
            if (ScriptParams[5])
            {
                pPhysical->physicalFlags.bMeleeProof = true;
            }
            else
            {
                pPhysical->physicalFlags.bMeleeProof = false;
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
                pPhysical->physicalFlags.bBulletProof = true;
            }
            else
            {
                pPhysical->physicalFlags.bBulletProof = false;
            }
            if (ScriptParams[2])
            {
                pPhysical->physicalFlags.bFireProof = true;
            }
            else
            {
                pPhysical->physicalFlags.bFireProof = false;
            }
            if (ScriptParams[3])
            {
                pPhysical->physicalFlags.bExplosionProof = true;
            }
            else
            {
                pPhysical->physicalFlags.bExplosionProof = false;
            }
            if (ScriptParams[4])
            {
                pPhysical->physicalFlags.bCollisionProof = true;
            }
            else
            {
                pPhysical->physicalFlags.bCollisionProof = false;
            }
            if (ScriptParams[5])
            {
                pPhysical->physicalFlags.bMeleeProof = true;
            }
            else
            {
                pPhysical->physicalFlags.bMeleeProof = false;
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

// stub: sa 0x47FA30
OpcodeResult CRunningScript::ProcessCommands700To799(i32 CurrCommand)
{
    MARKFUNCTION(0x47FA30);
    switch (CurrCommand) {}
    return OR_INTERRUPT;
}

// MARK: 800To899

// stub: sa 0x481300
OpcodeResult CRunningScript::ProcessCommands800To899(i32 CurrCommand)
{
    MARKFUNCTION(0x481300);
    switch (CurrCommand) {}
    return OR_INTERRUPT;
}

// MARK: 900To999

// stub: sa 0x483BD0
OpcodeResult CRunningScript::ProcessCommands900To999(i32 CurrCommand)
{
    MARKFUNCTION(0x483BD0);
    switch (CurrCommand) {}
    return OR_INTERRUPT;
}

// MARK: 1000To1099

// stub: sa 0x489500
OpcodeResult CRunningScript::ProcessCommands1000To1099(i32 CurrCommand)
{
    MARKFUNCTION(0x489500);
    switch (CurrCommand) {}
    return OR_INTERRUPT;
}

// MARK: 1100To1199

// stub: sa 0x48A320
OpcodeResult CRunningScript::ProcessCommands1100To1199(i32 CurrCommand)
{
    MARKFUNCTION(0x48A320);
    switch (CurrCommand) {}
    return OR_INTERRUPT;
}

// MARK: 1200To1299

// stub: sa 0x48B590
OpcodeResult CRunningScript::ProcessCommands1200To1299(i32 CurrCommand)
{
    MARKFUNCTION(0x48B590);
    switch (CurrCommand) {}
    return OR_INTERRUPT;
}

// MARK: 1300To1399

// stub: sa 0x48CDD0
OpcodeResult CRunningScript::ProcessCommands1300To1399(i32 CurrCommand)
{
    MARKFUNCTION(0x48CDD0);
    switch (CurrCommand) {}
    return OR_INTERRUPT;
}

// MARK: 1400To1499

// stub: sa 0x48EAA0
OpcodeResult CRunningScript::ProcessCommands1400To1499(i32 CurrCommand)
{
    MARKFUNCTION(0x48EAA0);
    switch (CurrCommand) {}
    return OR_INTERRUPT;
}

// MARK: 1500To1599

// stub: sa 0x490DB0
OpcodeResult CRunningScript::ProcessCommands1500To1599(i32 CurrCommand)
{
    MARKFUNCTION(0x490DB0);
    switch (CurrCommand) {}
    return OR_INTERRUPT;
}

// MARK: 1600To1699

// stub: sa 0x493FE0
OpcodeResult CRunningScript::ProcessCommands1600To1699(i32 CurrCommand)
{
    MARKFUNCTION(0x493FE0);
    switch (CurrCommand) {}
    return OR_INTERRUPT;
}

// MARK: 1700To1799

// stub: sa 0x496E00
OpcodeResult CRunningScript::ProcessCommands1700To1799(i32 CurrCommand)
{
    MARKFUNCTION(0x496E00);
    switch (CurrCommand) {}
    return OR_INTERRUPT;
}

// MARK: 1800To1899

// stub: sa 0x46D050
OpcodeResult CRunningScript::ProcessCommands1800To1899(i32 CurrCommand)
{
    MARKFUNCTION(0x46D050);
    switch (CurrCommand) {}
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
            ScriptParams[0] = CWeaponInfo::GetWeaponInfo((eWeaponType)ScriptParams[0], eWeaponSkill::STD)->GetModelId();
            StoreParameters(1);
            return OR_CONTINUE;
        }
        // sa: 0x532 (opcode 1922)
        case COMMAND_GET_WEAPONTYPE_SLOT:
        {
            CollectParameters(1);
            ScriptParams[0] = CWeaponInfo::GetWeaponInfo((eWeaponType)ScriptParams[0], eWeaponSkill::STD)->m_nWeaponSlot;
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
            CWorld::Players[ScriptParams[0]].m_pPed->GetPlayerData()->m_pPedClothesDesc->SetTextureAndModel(ScriptParams[1], ScriptParams[2], ScriptParams[3]);
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
            CRopes::PlayerControlsCrane = eControlledCrane::MAGNO_CRANE;
            CWaterLevel::m_bWaterFogScript = false;
            return OR_CONTINUE;
        }
        // sa: 0xa10 (opcode 1950)
        case COMMAND_PLAYER_ENTERED_BUILDINGSITE_CRANE:
        {
            CRopes::PlayerControlsCrane = eControlledCrane::WRECKING_BALL;
            CWaterLevel::m_bWaterFogScript = false;
            return OR_CONTINUE;
        }
        // sa: 0xa28 (opcode 1951)
        case COMMAND_PLAYER_LEFT_CRANE:
        {
            CRopes::PlayerControlsCrane = eControlledCrane::NONE;
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
            if (CPedScriptedTaskRecord::GetStatus(pPed, COMMAND_PERFORM_SEQUENCE_TASK) != eScriptedTaskStatus::NONE)
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
            ScriptParams[0] = CTheScripts::GetUniqueScriptThingIndex(CWorld::Players[ScriptParams[0]].m_pPed->GetPlayerData()->m_nPlayerGroup, 8);
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
                pPlayer->m_pPed->GetPlayerData()->m_bGroupStuffDisabled = false;
            }
            else
            {
                pPlayer->m_pPed->GetPlayerData()->m_bGroupStuffDisabled = true;
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
            pPed->bDoesntListenToPlayerGroupCommands = (ScriptParams[1] == 0);
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
            tempfloat = *(f32*)&ScriptParams[4] * 0.017453292f;
            fDistance = *(f32*)&ScriptParams[5];
            if (fDistance < 0.0f)
            {
                fDistance = 0.1f;
            }
            TempCoors.x = *(f32*)&ScriptParams[1];
            TempCoors.y = *(f32*)&ScriptParams[2];
            TempCoors.z = *(f32*)&ScriptParams[3];
            pTask = new CTaskComplexSequence();
            ((CTaskComplexSequence*)pTask)->AddTask(new CTaskComplexGoToPointAndStandStill(PEDMOVE_WALK, TempCoors, CTaskComplexGoToPointAndStandStill::ms_fTargetRadius, CTaskComplexGoToPointAndStandStill::ms_fSlowDownDistance, false, false));
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

// stub: sa 0x472310
OpcodeResult CRunningScript::ProcessCommands2000To2099(i32 CurrCommand)
{
    MARKFUNCTION(0x472310);
    switch (CurrCommand)
    {
        case COMMAND_GET_CURRENT_DAY_OF_WEEK:
        {
            return OR_CONTINUE;
        }
        case COMMAND_SET_CURRENT_DAY_OF_WEEK:
        {
            return OR_CONTINUE;
        }
        case COMMAND_ACTIVATE_INTERIORS:
        {
            return OR_CONTINUE;
        }
        case COMMAND_REGISTER_SCRIPT_BRAIN_FOR_CODE_USE:
        {
            return OR_CONTINUE;
        }
        case COMMAND_REGISTER_OBJECT_SCRIPT_BRAIN_FOR_CODE_USE:
        {
            return OR_CONTINUE;
        }
        case COMMAND_APPLY_FORCE_TO_CAR:
        {
            return OR_CONTINUE;
        }
        case COMMAND_IS_INT_LVAR_EQUAL_TO_INT_VAR:
        {
            return OR_CONTINUE;
        }
        case COMMAND_IS_FLOAT_LVAR_EQUAL_TO_FLOAT_VAR:
        {
            return OR_CONTINUE;
        }
        case COMMAND_IS_INT_LVAR_NOT_EQUAL_TO_INT_VAR:
        {
            return OR_CONTINUE;
        }
        case COMMAND_IS_FLOAT_LVAR_NOT_EQUAL_TO_FLOAT_VAR:
        {
            return OR_CONTINUE;
        }
        case COMMAND_ADD_TO_CAR_ROTATION_VELOCITY:
        {
            return OR_CONTINUE;
        }
        case COMMAND_SET_CAR_ROTATION_VELOCITY:
        {
            return OR_CONTINUE;
        }
        case COMMAND_GET_CAR_ROTATION_VELOCITY:
        {
            return OR_CONTINUE;
        }
        case COMMAND_SET_CHAR_SHOOT_RATE:
        {
            return OR_CONTINUE;
        }
        case COMMAND_IS_MODEL_IN_CDIMAGE:
        {
            return OR_CONTINUE;
        }
        case COMMAND_REMOVE_OIL_PUDDLES_IN_AREA:
        {
            return OR_CONTINUE;
        }
        case COMMAND_SET_BLIP_AS_FRIENDLY:
        {
            return OR_CONTINUE;
        }
        case COMMAND_TASK_SWIM_TO_COORD:
        {
            return OR_CONTINUE;
        }
        case COMMAND_TASK_GO_STRAIGHT_TO_COORD_WITHOUT_STOPPING:
        {
            return OR_CONTINUE;
        }
        case COMMAND_GET_BEAT_INFO_FOR_CURRENT_TRACK:
        {
            return OR_CONTINUE;
        }
        case COMMAND_GET_MODEL_DIMENSIONS:
        {
            return OR_CONTINUE;
        }
        case COMMAND_COPY_CHAR_DECISION_MAKER:
        {
            return OR_CONTINUE;
        }
        case COMMAND_COPY_GROUP_DECISION_MAKER:
        {
            return OR_CONTINUE;
        }
        case COMMAND_TASK_DRIVE_POINT_ROUTE_ADVANCED:
        {
            return OR_CONTINUE;
        }
        case COMMAND_IS_RELATIONSHIP_SET:
        {
            return OR_CONTINUE;
        }
        case COMMAND_HAS_CHAR_SPOTTED_CAR:
        {
            return OR_CONTINUE;
        }
        case COMMAND_SET_ROPE_HEIGHT_FOR_HELI:
        {
            return OR_CONTINUE;
        }
        case COMMAND_GET_ROPE_HEIGHT_FOR_HELI:
        {
            return OR_CONTINUE;
        }
        case COMMAND_IS_CAR_LOWRIDER:
        {
            return OR_CONTINUE;
        }
        case COMMAND_IS_PERFORMANCE_CAR:
        {
            return OR_CONTINUE;
        }
        case COMMAND_SET_CAR_ALWAYS_CREATE_SKIDS:
        {
            return OR_CONTINUE;
        }
        case COMMAND_GET_CITY_FROM_COORDS:
        {
            return OR_CONTINUE;
        }
        case COMMAND_HAS_OBJECT_OF_TYPE_BEEN_SMASHED:
        {
            return OR_CONTINUE;
        }
        case COMMAND_IS_PLAYER_PERFORMING_WHEELIE:
        {
            return OR_CONTINUE;
        }
        case COMMAND_IS_PLAYER_PERFORMING_STOPPIE:
        {
            return OR_CONTINUE;
        }
        case COMMAND_SET_CHECKPOINT_COORDS:
        {
            return OR_CONTINUE;
        }
        case COMMAND_SET_ONSCREEN_TIMER_DISPLAY:
        {
            return OR_CONTINUE;
        }
        case COMMAND_CONTROL_CAR_HYDRAULICS:
        {
            return OR_CONTINUE;
        }
        case COMMAND_GET_GROUP_SIZE:
        {
            return OR_CONTINUE;
        }
        case COMMAND_SET_OBJECT_COLLISION_DAMAGE_EFFECT:
        {
            return OR_CONTINUE;
        }
        case COMMAND_SET_CAR_FOLLOW_CAR:
        {
            return OR_CONTINUE;
        }
        case COMMAND_PLAYER_ENTERED_QUARRY_CRANE:
        {
            return OR_CONTINUE;
        }
        case COMMAND_PLAYER_ENTERED_LAS_VEGAS_CRANE:
        {
            return OR_CONTINUE;
        }
        case COMMAND_SWITCH_ENTRY_EXIT:
        {
            return OR_CONTINUE;
        }
        case COMMAND_DISPLAY_TEXT_WITH_FLOAT:
        {
            return OR_CONTINUE;
        }
        case COMMAND_DOES_GROUP_EXIST:
        {
            return OR_CONTINUE;
        }
        case COMMAND_GIVE_MELEE_ATTACK_TO_CHAR:
        {
            return OR_CONTINUE;
        }
        case COMMAND_SET_CAR_HYDRAULICS:
        {
            return OR_CONTINUE;
        }
        case COMMAND_IS_2PLAYER_GAME_GOING_ON:
        {
            return OR_CONTINUE;
        }
        case COMMAND_GET_CAMERA_FOV:
        {
            return OR_CONTINUE;
        }
        case COMMAND_SET_GLOBAL_PED_SEARCH_PARAMS:
        {
            return OR_CONTINUE;
        }
        case COMMAND_DOES_CAR_HAVE_HYDRAULICS:
        {
            return OR_CONTINUE;
        }
        case COMMAND_TASK_CHAR_SLIDE_TO_COORD_AND_PLAY_ANIM:
        {
            return OR_CONTINUE;
        }
        case COMMAND_ALLOCATE_SCRIPT_TO_OBJECT:
        {
            return OR_CONTINUE;
        }
        case COMMAND_GET_TOTAL_NUMBER_OF_PEDS_KILLED_BY_PLAYER:
        {
            return OR_CONTINUE;
        }
        case COMMAND_SET_TWO_PLAYER_CAM_MODE_SAME_CAR_SHOOTING:
        {
            return OR_CONTINUE;
        }
        case COMMAND_SET_TWO_PLAYER_CAM_MODE_SAME_CAR_NO_SHOOTING:
        {
            return OR_CONTINUE;
        }
        case COMMAND_SET_TWO_PLAYER_CAM_MODE_NOT_BOTH_IN_CAR:
        {
            return OR_CONTINUE;
        }
        case COMMAND_GET_LEVEL_DESIGN_COORDS_FOR_OBJECT:
        {
            return OR_CONTINUE;
        }
        case COMMAND_SAVE_TEXT_LABEL_TO_DEBUG_FILE:
        {
            return OR_CONTINUE;
        }
        case COMMAND_GET_CHAR_BREATH:
        {
            return OR_CONTINUE;
        }
        case COMMAND_SET_CHAR_BREATH:
        {
            return OR_CONTINUE;
        }
        case COMMAND_GET_CHAR_HIGHEST_PRIORITY_EVENT:
        {
            return OR_CONTINUE;
        }
        case COMMAND_ARE_PATHS_LOADED_FOR_CAR:
        {
            return OR_CONTINUE;
        }
        case COMMAND_GET_PARKING_NODE_IN_AREA:
        {
            return OR_CONTINUE;
        }
        case COMMAND_GET_CAR_CHAR_IS_USING:
        {
            return OR_CONTINUE;
        }
        case COMMAND_TASK_PLAY_ANIM_NON_INTERRUPTABLE:
        {
            return OR_CONTINUE;
        }
        case COMMAND_FORCE_NEXT_DIE_ANIM:
        {
            return OR_CONTINUE;
        }
        case COMMAND_ADD_STUNT_JUMP:
        {
            return OR_CONTINUE;
        }
        case COMMAND_SET_OBJECT_COORDINATES_AND_VELOCITY:
        {
            return OR_CONTINUE;
        }
        case COMMAND_SET_CHAR_KINDA_STAY_IN_SAME_PLACE:
        {
            return OR_CONTINUE;
        }
        case COMMAND_TASK_FOLLOW_PATROL_ROUTE:
        {
            return OR_CONTINUE;
        }
        case COMMAND_IS_CHAR_IN_AIR:
        {
            return OR_CONTINUE;
        }
        case COMMAND_GET_CHAR_HEIGHT_ABOVE_GROUND:
        {
            return OR_CONTINUE;
        }
        case COMMAND_SET_CHAR_WEAPON_SKILL:
        {
            return OR_CONTINUE;
        }
        case COMMAND_ARE_PATHS_LOADED_IN_AREA:
        {
            return OR_CONTINUE;
        }
        case COMMAND_SET_TEXT_EDGE:
        {
            return OR_CONTINUE;
        }
        case COMMAND_SET_CAR_ENGINE_BROKEN:
        {
            return OR_CONTINUE;
        }
        case COMMAND_IS_THIS_MODEL_A_BOAT:
        {
            return OR_CONTINUE;
        }
        case COMMAND_IS_THIS_MODEL_A_PLANE:
        {
            return OR_CONTINUE;
        }
        case COMMAND_IS_THIS_MODEL_A_HELI:
        {
            return OR_CONTINUE;
        }
        case COMMAND_IS_3D_COORD_IN_ZONE:
        {
            return OR_CONTINUE;
        }
        case COMMAND_SET_FIRST_PERSON_IN_CAR_CAMERA_MODE:
        {
            return OR_CONTINUE;
        }
        case COMMAND_TASK_GREET_PARTNER:
        {
            return OR_CONTINUE;
        }
        case COMMAND_GET_CLOSEST_PICKUP_COORDS_TO_COORD:
        {
            return OR_CONTINUE;
        }
        case COMMAND_SET_HELI_BLADES_FULL_SPEED:
        {
            return OR_CONTINUE;
        }
        case COMMAND_DISPLAY_HUD:
        {
            return OR_CONTINUE;
        }
        case COMMAND_CONNECT_LODS:
        {
            return OR_CONTINUE;
        }
        case COMMAND_SET_MAX_FIRE_GENERATIONS:
        {
            return OR_CONTINUE;
        }
        case COMMAND_TASK_DIE_NAMED_ANIM:
        {
            return OR_CONTINUE;
        }
        case COMMAND_SET_PLAYER_DUCK_BUTTON:
        {
            return OR_CONTINUE;
        }
        case COMMAND_FIND_NEAREST_MULTIBUILDING:
        {
            return OR_CONTINUE;
        }
        case COMMAND_SET_MULTIBUILDING_MODEL:
        {
            return OR_CONTINUE;
        }
        case COMMAND_GET_NUMBER_MULTIBUILDING_MODELS:
        {
            return OR_CONTINUE;
        }
        case COMMAND_GET_MULTIBUILDING_MODEL_INDEX:
        {
            return OR_CONTINUE;
        }
        case COMMAND_SET_CURRENT_BUYABLE_PROPERTY:
        {
            return OR_CONTINUE;
        }
        case COMMAND_SET_POOL_TABLE_COORDS:
        {
            return OR_CONTINUE;
        }
        case COMMAND_IS_AUDIO_BUILD:
        {
            return OR_CONTINUE;
        }
        case COMMAND_CLEAR_QUEUED_DIALOGUE:
        {
            return OR_CONTINUE;
        }
        case COMMAND_HAS_OBJECT_BEEN_PHOTOGRAPHED:
        {
            return OR_CONTINUE;
        }
    }
    return OR_INTERRUPT;
}

// MARK: 2100To2199

// stub: sa 0x470A90
OpcodeResult CRunningScript::ProcessCommands2100To2199(i32 CurrCommand)
{
    MARKFUNCTION(0x470A90);
    switch (CurrCommand)
    {
        case COMMAND_DO_CAMERA_BUMP:
        {
            return OR_CONTINUE;
        }
        case COMMAND_GET_CURRENT_DATE:
        {
            return OR_CONTINUE;
        }
        case COMMAND_SET_OBJECT_ANIM_SPEED:
        {
            return OR_CONTINUE;
        }
        case COMMAND_IS_OBJECT_PLAYING_ANIM:
        {
            return OR_CONTINUE;
        }
        case COMMAND_SET_OBJECT_ANIM_PLAYING_FLAG:
        {
            return OR_CONTINUE;
        }
        case COMMAND_GET_OBJECT_ANIM_CURRENT_TIME:
        {
            return OR_CONTINUE;
        }
        case COMMAND_SET_OBJECT_ANIM_CURRENT_TIME:
        {
            return OR_CONTINUE;
        }
        case COMMAND_GET_OBJECT_ANIM_TOTAL_TIME:
        {
            return OR_CONTINUE;
        }
        case COMMAND_SET_CHAR_VELOCITY:
        {
            return OR_CONTINUE;
        }
        case COMMAND_GET_CHAR_VELOCITY:
        {
            return OR_CONTINUE;
        }
        case COMMAND_SET_CHAR_ROTATION:
        {
            return OR_CONTINUE;
        }
        case COMMAND_GET_CAR_UPRIGHT_VALUE:
        {
            return OR_CONTINUE;
        }
        case COMMAND_SET_VEHICLE_AREA_VISIBLE:
        {
            return OR_CONTINUE;
        }
        case COMMAND_SELECT_WEAPONS_FOR_VEHICLE:
        {
            return OR_CONTINUE;
        }
        case COMMAND_GET_CITY_PLAYER_IS_IN:
        {
            return OR_CONTINUE;
        }
        case COMMAND_GET_NAME_OF_ZONE:
        {
            return OR_CONTINUE;
        }
        case COMMAND_IS_VAR_TEXT_LABEL_EMPTY:
        {
            return OR_CONTINUE;
        }
        case COMMAND_IS_LVAR_TEXT_LABEL_EMPTY:
        {
            return OR_CONTINUE;
        }
        case COMMAND_IS_VAR_TEXT_LABEL16_EMPTY:
        {
            return OR_CONTINUE;
        }
        case COMMAND_IS_LVAR_TEXT_LABEL16_EMPTY:
        {
            return OR_CONTINUE;
        }
        case COMMAND_SWITCH:
        {
            return OR_CONTINUE;
        }
        case COMMAND_ENDSWITCH:
        {
            return OR_CONTINUE;
        }
        case COMMAND_CASE:
        {
            return OR_CONTINUE;
        }
        case COMMAND_DEFAULT:
        {
            return OR_CONTINUE;
        }
        case COMMAND_BREAK:
        {
            return OR_CONTINUE;
        }
        case COMMAND_ACTIVATE_INTERIOR_PEDS:
        {
            return OR_CONTINUE;
        }
        case COMMAND_SET_VEHICLE_CAN_BE_TARGETTED:
        {
            return OR_CONTINUE;
        }
        case COMMAND_GET_GROUP_LEADER:
        {
            return OR_CONTINUE;
        }
        case COMMAND_TASK_FOLLOW_FOOTSTEPS:
        {
            return OR_CONTINUE;
        }
        case COMMAND_DAMAGE_CHAR:
        {
            return OR_CONTINUE;
        }
        case COMMAND_SET_CAR_CAN_BE_VISIBLY_DAMAGED:
        {
            return OR_CONTINUE;
        }
        case COMMAND_SET_HELI_REACHED_TARGET_DISTANCE:
        {
            return OR_CONTINUE;
        }
        case COMMAND_BLOCK_NODES_IN_AREA:
        {
            return OR_CONTINUE;
        }
        case COMMAND_GET_SOUND_LEVEL_AT_COORDS:
        {
            return OR_CONTINUE;
        }
        case COMMAND_SET_CHAR_ALLOWED_TO_DUCK:
        {
            return OR_CONTINUE;
        }
        case COMMAND_SET_WATER_CONFIGURATION:
        {
            return OR_CONTINUE;
        }
        case COMMAND_SET_HEADING_FOR_ATTACHED_PLAYER:
        {
            return OR_CONTINUE;
        }
        case COMMAND_TASK_WALK_ALONGSIDE_CHAR:
        {
            return OR_CONTINUE;
        }
        case COMMAND_CREATE_EMERGENCY_SERVICES_CAR:
        {
            return OR_CONTINUE;
        }
        case COMMAND_TASK_KINDA_STAY_IN_SAME_PLACE:
        {
            return OR_CONTINUE;
        }
        case COMMAND_TASK_USE_ATTRACTOR_ADVANCED:
        {
            return OR_CONTINUE;
        }
        case COMMAND_TASK_FOLLOW_PATH_NODES_TO_COORD_SHOOTING:
        {
            return OR_CONTINUE;
        }
        case COMMAND_START_PLAYBACK_RECORDED_CAR_LOOPED:
        {
            return OR_CONTINUE;
        }
        case COMMAND_START_PLAYBACK_RECORDED_CAR_USING_AI_LOOPED:
        {
            return OR_CONTINUE;
        }
        case COMMAND_SET_CHAR_AREA_VISIBLE:
        {
            return OR_CONTINUE;
        }
        case COMMAND_IS_ATTACHED_PLAYER_HEADING_ACHIEVED:
        {
            return OR_CONTINUE;
        }
        case COMMAND_GET_MODEL_NAME_FOR_DEBUG_ONLY:
        {
            return OR_CONTINUE;
        }
        case COMMAND_TASK_USE_NEARBY_ENTRY_EXIT:
        {
            return OR_CONTINUE;
        }
        case COMMAND_ENABLE_ENTRY_EXIT_PLAYER_GROUP_WARPING:
        {
            return OR_CONTINUE;
        }
        case COMMAND_FREEZE_STATE_OF_INTERIORS:
        {
            return OR_CONTINUE;
        }
        case COMMAND_GET_CLOSEST_STEALABLE_OBJECT:
        {
            return OR_CONTINUE;
        }
        case COMMAND_IS_PROCEDURAL_INTERIOR_ACTIVE:
        {
            return OR_CONTINUE;
        }
        case COMMAND_CLEAR_THIS_VIEW_INTEGER_VARIABLE:
        {
            return OR_CONTINUE;
        }
        case COMMAND_CLEAR_THIS_VIEW_FLOAT_VARIABLE:
        {
            return OR_CONTINUE;
        }
        case COMMAND_CLEAR_ALL_VIEW_VARIABLES:
        {
            return OR_CONTINUE;
        }
        case COMMAND_CLEAR_THIS_INTEGER_WATCHPOINT:
        {
            return OR_CONTINUE;
        }
        case COMMAND_CLEAR_THIS_FLOAT_WATCHPOINT:
        {
            return OR_CONTINUE;
        }
        case COMMAND_CLEAR_ALL_BREAKPOINTS:
        {
            return OR_CONTINUE;
        }
        case COMMAND_CLEAR_ALL_WATCHPOINTS:
        {
            return OR_CONTINUE;
        }
        case COMMAND_IS_THIS_MODEL_A_TRAIN:
        {
            return OR_CONTINUE;
        }
        case COMMAND_GET_VEHICLE_CHAR_IS_STANDING_ON:
        {
            return OR_CONTINUE;
        }
        case COMMAND_SWITCH_START:
        {
            return OR_CONTINUE;
        }
        case COMMAND_SWITCH_CONTINUED:
        {
            return OR_CONTINUE;
        }
        case COMMAND_REMOVE_CAR_RECORDING:
        {
            return OR_CONTINUE;
        }
        case COMMAND_SET_ZONE_POPULATION_RACE:
        {
            return OR_CONTINUE;
        }
        case COMMAND_SET_OBJECT_ONLY_DAMAGED_BY_PLAYER:
        {
            return OR_CONTINUE;
        }
        case COMMAND_CREATE_BIRDS:
        {
            return OR_CONTINUE;
        }
        case COMMAND_GET_VEHICLE_DIRT_LEVEL:
        {
            return OR_CONTINUE;
        }
        case COMMAND_SET_VEHICLE_DIRT_LEVEL:
        {
            return OR_CONTINUE;
        }
        case COMMAND_SET_GANG_WARS_ACTIVE:
        {
            return OR_CONTINUE;
        }
        case COMMAND_IS_GANG_WAR_GOING_ON:
        {
            return OR_CONTINUE;
        }
        case COMMAND_GIVE_PLAYER_CLOTHES_OUTSIDE_SHOP:
        {
            return OR_CONTINUE;
        }
        case COMMAND_CLEAR_LOADED_SHOP:
        {
            return OR_CONTINUE;
        }
        case COMMAND_SET_GROUP_SEQUENCE:
        {
            return OR_CONTINUE;
        }
        case COMMAND_SET_CHAR_DROPS_WEAPONS_WHEN_DEAD:
        {
            return OR_CONTINUE;
        }
        case COMMAND_SET_CHAR_NEVER_LEAVES_GROUP:
        {
            return OR_CONTINUE;
        }
        case COMMAND_DRAW_RECT_WITH_TITLE:
        {
            return OR_CONTINUE;
        }
        case COMMAND_SET_PLAYER_FIRE_BUTTON:
        {
            return OR_CONTINUE;
        }
        case COMMAND_SET_ATTRACTOR_RADIUS:
        {
            return OR_CONTINUE;
        }
        case COMMAND_ATTACH_FX_SYSTEM_TO_CHAR_BONE:
        {
            return OR_CONTINUE;
        }
        case COMMAND_REGISTER_ATTRACTOR_SCRIPT_BRAIN_FOR_CODE_USE:
        {
            return OR_CONTINUE;
        }
        case COMMAND_CONST_INT:
        {
            return OR_CONTINUE;
        }
        case COMMAND_CONST_FLOAT:
        {
            return OR_CONTINUE;
        }
        case COMMAND_SET_HEADING_LIMIT_FOR_ATTACHED_CHAR:
        {
            return OR_CONTINUE;
        }
        case COMMAND_ADD_BLIP_FOR_DEAD_CHAR:
        {
            return OR_CONTINUE;
        }
        case COMMAND_GET_DEAD_CHAR_COORDINATES:
        {
            return OR_CONTINUE;
        }
        case COMMAND_TASK_PLAY_ANIM_WITH_FLAGS:
        {
            return OR_CONTINUE;
        }
        case COMMAND_SET_VEHICLE_AIR_RESISTANCE_MULTIPLIER:
        {
            return OR_CONTINUE;
        }
        case COMMAND_SET_CAR_COORDINATES_NO_OFFSET:
        {
            return OR_CONTINUE;
        }
        case COMMAND_SET_USES_COLLISION_OF_CLOSEST_OBJECT_OF_TYPE:
        {
            return OR_CONTINUE;
        }
        case COMMAND_SET_TIME_ONE_DAY_FORWARD:
        {
            return OR_CONTINUE;
        }
        case COMMAND_SET_TIME_ONE_DAY_BACK:
        {
            return OR_CONTINUE;
        }
        case COMMAND_SET_TIMER_BEEP_COUNTDOWN_TIME:
        {
            return OR_CONTINUE;
        }
        case COMMAND_TASK_SIT_IN_RESTAURANT:
        {
            return OR_CONTINUE;
        }
        case COMMAND_GET_RANDOM_ATTRACTOR_ON_CLOSEST_OBJECT_OF_TYPE:
        {
            return OR_CONTINUE;
        }
        case COMMAND_ATTACH_TRAILER_TO_CAB:
        {
            return OR_CONTINUE;
        }
        case COMMAND_ADD_INTERESTING_ENTITY_FOR_CHAR:
        {
            return OR_CONTINUE;
        }
        case COMMAND_CLEAR_INTERESTING_ENTITIES_FOR_CHAR:
        {
            return OR_CONTINUE;
        }
        case COMMAND_GET_CLOSEST_ATTRACTOR:
        {
            return OR_CONTINUE;
        }
        case COMMAND_IS_VEHICLE_TOUCHING_OBJECT:
        {
            return OR_CONTINUE;
        }
    }
    return OR_INTERRUPT;
}

// MARK: 2200To2299

// stub: sa 0x474900
OpcodeResult CRunningScript::ProcessCommands2200To2299(i32 CurrCommand)
{
    MARKFUNCTION(0x474900);
    switch (CurrCommand)
    {
        case COMMAND_ENABLE_CRANE_CONTROLS:
        {
            return OR_CONTINUE;
        }
        case COMMAND_ALLOCATE_SCRIPT_TO_ATTRACTOR:
        {
            return OR_CONTINUE;
        }
        case COMMAND_GET_CLOSEST_ATTRACTOR_WITH_THIS_SCRIPT:
        {
            return OR_CONTINUE;
        }
        case COMMAND_IS_PLAYER_IN_POSITION_FOR_CONVERSATION:
        {
            return OR_CONTINUE;
        }
        case COMMAND_ENABLE_CONVERSATION:
        {
            return OR_CONTINUE;
        }
        case COMMAND_GET_CONVERSATION_STATUS:
        {
            return OR_CONTINUE;
        }
        case COMMAND_GET_RANDOM_CHAR_IN_SPHERE_ONLY_DRUGS_BUYERS:
        {
            return OR_CONTINUE;
        }
        case COMMAND_GET_PED_TYPE:
        {
            return OR_CONTINUE;
        }
        case COMMAND_TASK_USE_CLOSEST_MAP_ATTRACTOR:
        {
            return OR_CONTINUE;
        }
        case COMMAND_GET_CHAR_MAP_ATTRACTOR_STATUS:
        {
            return OR_CONTINUE;
        }
        case COMMAND_PLANE_ATTACK_PLAYER_USING_DOG_FIGHT:
        {
            return OR_CONTINUE;
        }
        case COMMAND_CAN_TRIGGER_GANG_WAR_WHEN_ON_A_MISSION:
        {
            return OR_CONTINUE;
        }
        case COMMAND_CONTROL_MOVABLE_VEHICLE_PART:
        {
            return OR_CONTINUE;
        }
        case COMMAND_WINCH_CAN_PICK_VEHICLE_UP:
        {
            return OR_CONTINUE;
        }
        case COMMAND_OPEN_CAR_DOOR_A_BIT:
        {
            return OR_CONTINUE;
        }
        case COMMAND_IS_CAR_DOOR_FULLY_OPEN:
        {
            return OR_CONTINUE;
        }
        case COMMAND_SET_ALWAYS_DRAW_3D_MARKERS:
        {
            return OR_CONTINUE;
        }
        case COMMAND_STREAM_SCRIPT:
        {
            return OR_CONTINUE;
        }
        case COMMAND_STREAM_SCRIPT_INTERNAL:
        {
            return OR_CONTINUE;
        }
        case COMMAND_HAS_STREAMED_SCRIPT_LOADED:
        {
            return OR_CONTINUE;
        }
        case COMMAND_SET_GANG_WARS_TRAINING_MISSION:
        {
            return OR_CONTINUE;
        }
        case COMMAND_SET_CHAR_HAS_USED_ENTRY_EXIT:
        {
            return OR_CONTINUE;
        }
        case COMMAND_DRAW_WINDOW_TEXT:
        {
            return OR_CONTINUE;
        }
        case COMMAND_SET_CHAR_MAX_HEALTH:
        {
            return OR_CONTINUE;
        }
        case COMMAND_SET_CAR_PITCH:
        {
            return OR_CONTINUE;
        }
        case COMMAND_SET_NIGHT_VISION:
        {
            return OR_CONTINUE;
        }
        case COMMAND_SET_INFRARED_VISION:
        {
            return OR_CONTINUE;
        }
        case COMMAND_SET_ZONE_FOR_GANG_WARS_TRAINING:
        {
            return OR_CONTINUE;
        }
        case COMMAND_IS_GLOBAL_VAR_BIT_SET_CONST:
        {
            return OR_CONTINUE;
        }
        case COMMAND_IS_GLOBAL_VAR_BIT_SET_VAR:
        {
            return OR_CONTINUE;
        }
        case COMMAND_IS_GLOBAL_VAR_BIT_SET_LVAR:
        {
            return OR_CONTINUE;
        }
        case COMMAND_IS_LOCAL_VAR_BIT_SET_CONST:
        {
            return OR_CONTINUE;
        }
        case COMMAND_IS_LOCAL_VAR_BIT_SET_VAR:
        {
            return OR_CONTINUE;
        }
        case COMMAND_IS_LOCAL_VAR_BIT_SET_LVAR:
        {
            return OR_CONTINUE;
        }
        case COMMAND_SET_GLOBAL_VAR_BIT_CONST:
        {
            return OR_CONTINUE;
        }
        case COMMAND_SET_GLOBAL_VAR_BIT_VAR:
        {
            return OR_CONTINUE;
        }
        case COMMAND_SET_GLOBAL_VAR_BIT_LVAR:
        {
            return OR_CONTINUE;
        }
        case COMMAND_SET_LOCAL_VAR_BIT_CONST:
        {
            return OR_CONTINUE;
        }
        case COMMAND_SET_LOCAL_VAR_BIT_VAR:
        {
            return OR_CONTINUE;
        }
        case COMMAND_SET_LOCAL_VAR_BIT_LVAR:
        {
            return OR_CONTINUE;
        }
        case COMMAND_CLEAR_GLOBAL_VAR_BIT_CONST:
        {
            return OR_CONTINUE;
        }
        case COMMAND_CLEAR_GLOBAL_VAR_BIT_VAR:
        {
            return OR_CONTINUE;
        }
        case COMMAND_CLEAR_GLOBAL_VAR_BIT_LVAR:
        {
            return OR_CONTINUE;
        }
        case COMMAND_CLEAR_LOCAL_VAR_BIT_CONST:
        {
            return OR_CONTINUE;
        }
        case COMMAND_CLEAR_LOCAL_VAR_BIT_VAR:
        {
            return OR_CONTINUE;
        }
        case COMMAND_CLEAR_LOCAL_VAR_BIT_LVAR:
        {
            return OR_CONTINUE;
        }
        case COMMAND_SET_CHAR_CAN_BE_KNOCKED_OFF_BIKE:
        {
            return OR_CONTINUE;
        }
        case COMMAND_SET_CHAR_COORDINATES_DONT_WARP_GANG:
        {
            return OR_CONTINUE;
        }
        case COMMAND_ADD_PRICE_MODIFIER:
        {
            return OR_CONTINUE;
        }
        case COMMAND_REMOVE_PRICE_MODIFIER:
        {
            return OR_CONTINUE;
        }
        case COMMAND_INIT_ZONE_POPULATION_SETTINGS:
        {
            return OR_CONTINUE;
        }
        case COMMAND_EXPLODE_CAR_IN_CUTSCENE_SHAKE_AND_BIT:
        {
            return OR_CONTINUE;
        }
        case COMMAND_PICK_UP_OBJECT_WITH_WINCH:
        {
            return OR_CONTINUE;
        }
        case COMMAND_PICK_UP_VEHICLE_WITH_WINCH:
        {
            return OR_CONTINUE;
        }
        case COMMAND_PICK_UP_CHAR_WITH_WINCH:
        {
            return OR_CONTINUE;
        }
        case COMMAND_STORE_CAR_IN_NEAREST_IMPOUNDING_GARAGE:
        {
            return OR_CONTINUE;
        }
        case COMMAND_IS_SKIP_CUTSCENE_BUTTON_PRESSED:
        {
            return OR_CONTINUE;
        }
        case COMMAND_GET_CUTSCENE_OFFSET:
        {
            return OR_CONTINUE;
        }
        case COMMAND_SET_OBJECT_SCALE:
        {
            return OR_CONTINUE;
        }
        case COMMAND_GET_CURRENT_POPULATION_ZONE_TYPE:
        {
            return OR_CONTINUE;
        }
        case COMMAND_CREATE_MENU:
        {
            return OR_CONTINUE;
        }
        case COMMAND_CONSTANT_INT:
        {
            return OR_CONTINUE;
        }
        case COMMAND_SET_MENU_COLUMN_ORIENTATION:
        {
            return OR_CONTINUE;
        }
        case COMMAND_GET_MENU_ITEM_SELECTED:
        {
            return OR_CONTINUE;
        }
        case COMMAND_GET_MENU_ITEM_ACCEPTED:
        {
            return OR_CONTINUE;
        }
        case COMMAND_ACTIVATE_MENU_ITEM:
        {
            return OR_CONTINUE;
        }
        case COMMAND_DELETE_MENU:
        {
            return OR_CONTINUE;
        }
        case COMMAND_SET_MENU_COLUMN:
        {
            return OR_CONTINUE;
        }
        case COMMAND_SET_BLIP_ENTRY_EXIT:
        {
            return OR_CONTINUE;
        }
        case COMMAND_SWITCH_DEATH_PENALTIES:
        {
            return OR_CONTINUE;
        }
        case COMMAND_SWITCH_ARREST_PENALTIES:
        {
            return OR_CONTINUE;
        }
        case COMMAND_SET_EXTRA_HOSPITAL_RESTART_POINT:
        {
            return OR_CONTINUE;
        }
        case COMMAND_SET_EXTRA_POLICE_STATION_RESTART_POINT:
        {
            return OR_CONTINUE;
        }
        case COMMAND_FIND_NUMBER_TAGS_TAGGED:
        {
            return OR_CONTINUE;
        }
        case COMMAND_GET_TERRITORY_UNDER_CONTROL_PERCENTAGE:
        {
            return OR_CONTINUE;
        }
        case COMMAND_IS_OBJECT_IN_ANGLED_AREA_2D:
        {
            return OR_CONTINUE;
        }
        case COMMAND_IS_OBJECT_IN_ANGLED_AREA_3D:
        {
            return OR_CONTINUE;
        }
        case COMMAND_GET_RANDOM_CHAR_IN_SPHERE_NO_BRAIN:
        {
            return OR_CONTINUE;
        }
        case COMMAND_SET_PLANE_UNDERCARRIAGE_UP:
        {
            return OR_CONTINUE;
        }
        case COMMAND_DISABLE_ALL_ENTRY_EXITS:
        {
            return OR_CONTINUE;
        }
        case COMMAND_ATTACH_ANIMS_TO_MODEL:
        {
            return OR_CONTINUE;
        }
        case COMMAND_SET_OBJECT_AS_STEALABLE:
        {
            return OR_CONTINUE;
        }
        case COMMAND_SET_CREATE_RANDOM_GANG_MEMBERS:
        {
            return OR_CONTINUE;
        }
        case COMMAND_ADD_SPARKS:
        {
            return OR_CONTINUE;
        }
        case COMMAND_GET_VEHICLE_CLASS:
        {
            CollectParameters(1);
            CVehicle* pVehicle = CPools::GetVehiclePool().GetAt(ScriptParams[0]);
            ScriptParams[0] = static_cast<CVehicleModelInfo*>(CModelInfo::GetModelInfo(static_cast<i32>(pVehicle->GetModelIndex())))->GetVehicleList();

            StoreParameters(1);
            return OR_CONTINUE;
        }
        case COMMAND_CLEAR_CONVERSATION_FOR_CHAR:
        {
            return OR_CONTINUE;
        }
        case COMMAND_SET_MENU_ITEM_WITH_NUMBER:
        {
            return OR_CONTINUE;
        }
        case COMMAND_SET_MENU_ITEM_WITH_2_NUMBERS:
        {
            return OR_CONTINUE;
        }
        case COMMAND_APPEND_TO_NEXT_CUTSCENE:
        {
            return OR_CONTINUE;
        }
        case COMMAND_GET_NAME_OF_INFO_ZONE:
        {
            return OR_CONTINUE;
        }
        case COMMAND_VEHICLE_CAN_BE_TARGETTED_BY_HS_MISSILE:
        {
            return OR_CONTINUE;
        }
        case COMMAND_SET_FREEBIES_IN_VEHICLE:
        {
            return OR_CONTINUE;
        }
        case COMMAND_SET_SCRIPT_LIMIT_TO_GANG_SIZE:
        {
            return OR_CONTINUE;
        }
        case COMMAND_MAKE_PLAYER_GANG_DISAPPEAR:
        {
            return OR_CONTINUE;
        }
        case COMMAND_MAKE_PLAYER_GANG_REAPPEAR:
        {
            return OR_CONTINUE;
        }
        case COMMAND_GET_CLOTHES_ITEM:
        {
            return OR_CONTINUE;
        }
        case COMMAND_SHOW_UPDATE_STATS:
        {
            return OR_CONTINUE;
        }
        case COMMAND_IS_VAR_TEXT_LABEL16_EQUAL_TO_TEXT_LABEL:
        {
            return OR_CONTINUE;
        }
        case COMMAND_IS_LVAR_TEXT_LABEL16_EQUAL_TO_TEXT_LABEL:
        {
            return OR_CONTINUE;
        }
        case COMMAND_SET_COORD_BLIP_APPEARANCE:
        {
            return OR_CONTINUE;
        }
    }
    return OR_INTERRUPT;
}

// MARK: 2300To2399

// stub: sa 0x4762D0
OpcodeResult CRunningScript::ProcessCommands2300To2399(i32 CurrCommand)
{
    MARKFUNCTION(0x4762D0);
    switch (CurrCommand)
    {
        case COMMAND_GET_DOOR_ANGLE_RATIO:  // 0x95F
        {
            CollectParameters(2);
            CVehicle* pVehicle = CPools::GetVehiclePool().GetAt(ScriptParams[0]);
            float ratio = 0.0f;
            if (pVehicle && pVehicle->IsAutomobile()) {
                auto* autoMob = pVehicle->AsAutomobile();
                eDoors iDoor = static_cast<eDoors>(ScriptParams[1]);
                if (iDoor < MAX_DOORS) {
                    ratio = autoMob->m_doors[iDoor].GetAngleOpenRatio();
                }
            }
            *(float*)&ScriptParams[0] = ratio;
            StoreParameters(1);
            break;
        }
    }
    return OR_CONTINUE;
}

// MARK: 2400To2499

// stub: sa 0x478000
OpcodeResult CRunningScript::ProcessCommands2400To2499(i32 CurrCommand)
{
    MARKFUNCTION(0x478000);
    switch (CurrCommand) {}
    return OR_INTERRUPT;
}

// MARK: 2500To2599

// stub: sa 0x47A760
OpcodeResult CRunningScript::ProcessCommands2500To2599(i32 CurrCommand)
{
    MARKFUNCTION(0x47A760);
    bool LatestCmpFlagResult = false;
    CVehicle* pVehicle = nullptr;
    CPed* pPed = nullptr;
    CObject* pObject = nullptr;
    CVector vec;

    u8 new_car_colour = 0;

    i32 iDecisionMakerIndex = 0;

    switch (CurrCommand)
    {
        case COMMAND_SET_PETROL_TANK_WEAKPOINT:
        {
            return OR_CONTINUE;
        }
        case COMMAND_IS_CHAR_USING_MAP_ATTRACTOR:
        {
            return OR_CONTINUE;
        }
        case COMMAND_SET_ALL_CARS_IN_AREA_VISIBLE:
        {
            return OR_CONTINUE;
        }
        case COMMAND_SET_PLAYER_MODEL:  // 0x9C7
        {
            CollectParameters(2);
            CPed* pPed = reinterpret_cast<CPed*>(CWorld::Players[ScriptParams[0]].m_pPed);
            if (pPed) {
                i32 modelID = ScriptParams[1];
                auto animGrp = pPed->m_nAnimGroup;
                pPed->DeleteRwObject();
                pPed->m_nModelIndex = -1;
                pPed->SetModelIndex(modelID);
                pPed->m_nAnimGroup = animGrp;
            }
            break;
        }
        case COMMAND_ARE_SUBTITLES_SWITCHED_ON:
        {
            return OR_CONTINUE;
        }
        case COMMAND_REMOVE_CHAR_FROM_CAR_MAINTAIN_POSITION:
        {
            return OR_CONTINUE;
        }
        case COMMAND_SET_OBJECT_PROOFS:
        {
            return OR_CONTINUE;
        }
        case COMMAND_IS_CAR_TOUCHING_CAR:
        {
            return OR_CONTINUE;
        }
        case COMMAND_DOES_OBJECT_HAVE_THIS_MODEL:
        {
            return OR_CONTINUE;
        }
        case COMMAND_IS_ITALIAN_GAME:
        {
            return OR_CONTINUE;
        }
        case COMMAND_IS_SPANISH_GAME:
        {
            return OR_CONTINUE;
        }
        case COMMAND_SET_TRAIN_FORCED_TO_SLOW_DOWN:
        {
            return OR_CONTINUE;
        }
        case COMMAND_IS_VEHICLE_ON_ALL_WHEELS:
        {
            return OR_CONTINUE;
        }
        case COMMAND_DOES_PICKUP_EXIST:
        {
            return OR_CONTINUE;
        }
        case COMMAND_ENABLE_AMBIENT_CRIME:
        {
            return OR_CONTINUE;
        }
        case COMMAND_IS_AMBIENT_CRIME_ENABLED:
        {
            return OR_CONTINUE;
        }
        case COMMAND_CLEAR_WANTED_LEVEL_IN_GARAGE:
        {
            return OR_CONTINUE;
        }
        case COMMAND_SET_CHAR_SAY_CONTEXT_IMPORTANT:
        {
            return OR_CONTINUE;
        }
        case COMMAND_SET_CHAR_SAY_SCRIPT:
        {
            return OR_CONTINUE;
        }
        case COMMAND_FORCE_INTERIOR_LIGHTING_FOR_PLAYER:
        {
            return OR_CONTINUE;
        }
        case COMMAND_DISABLE_2ND_PAD_FOR_DEBUG:
        {
            return OR_CONTINUE;
        }
        case COMMAND_USE_DETONATOR:
        {
            return OR_CONTINUE;
        }
        case COMMAND_IS_MONEY_PICKUP_AT_COORDS:
        {
            return OR_CONTINUE;
        }
        case COMMAND_SET_MENU_COLUMN_WIDTH:
        {
            return OR_CONTINUE;
        }
        case COMMAND_SET_CHAR_CAN_CLIMB_OUT_WATER:
        {
            return OR_CONTINUE;
        }
        case COMMAND_MAKE_ROOM_IN_PLAYER_GANG_FOR_MISSION_PEDS:
        {
            return OR_CONTINUE;
        }
        case COMMAND_IS_CHAR_GETTING_IN_TO_A_CAR:
        {
            return OR_CONTINUE;
        }
        case COMMAND_RESTORE_PLAYER_AFTER_2P_GAME:
        {
            return OR_CONTINUE;
        }
        case COMMAND_SET_UP_SKIP_FOR_SPECIFIC_VEHICLE:
        {
            return OR_CONTINUE;
        }
        case COMMAND_GET_CAR_MODEL_VALUE:
        {
            return OR_CONTINUE;
        }
        case COMMAND_CREATE_CAR_GENERATOR_WITH_PLATE:
        {
            return OR_CONTINUE;
        }
        case COMMAND_FIND_TRAIN_DIRECTION:
        {
            return OR_CONTINUE;
        }
        case COMMAND_SET_AIRCRAFT_CARRIER_SAM_SITE:
        {
            return OR_CONTINUE;
        }
        case COMMAND_DRAW_LIGHT_WITH_RANGE:
        {
            return OR_CONTINUE;
        }
        case COMMAND_ENABLE_BURGLARY_HOUSES:
        {
            return OR_CONTINUE;
        }
        case COMMAND_IS_PLAYER_CONTROL_ON:
        {
            return OR_CONTINUE;
        }
        case COMMAND_GET_CHAR_AREA_VISIBLE:
        {
            return OR_CONTINUE;
        }
        case COMMAND_GIVE_NON_PLAYER_CAR_NITRO:
        {
            return OR_CONTINUE;
        }
        case COMMAND_PLAYER_PUT_ON_GOGGLES:
        {
            return OR_CONTINUE;
        }
        case COMMAND_PLAYER_TAKE_OFF_GOGGLES:
        {
            return OR_CONTINUE;
        }
        case COMMAND_ALLOW_FIXED_CAMERA_COLLISION:
        {
            return OR_CONTINUE;
        }
        case COMMAND_HAS_CHAR_SPOTTED_CHAR_IN_FRONT:
        {
            return OR_CONTINUE;
        }
        case COMMAND_FORCE_BIG_MESSAGE_AND_COUNTER:
        {
            return OR_CONTINUE;
        }
        case COMMAND_SET_VEHICLE_CAMERA_TWEAK:
        {
            return OR_CONTINUE;
        }
        case COMMAND_RESET_VEHICLE_CAMERA_TWEAK:
        {
            return OR_CONTINUE;
        }
        case COMMAND_REPORT_MISSION_AUDIO_EVENT_AT_CHAR:
        {
            return OR_CONTINUE;
        }
        case COMMAND_DOES_DECISION_MAKER_EXIST:
        {
            return OR_CONTINUE;
        }
        case COMMAND_GET_RANDOM_TRAIN_IN_SPHERE_NO_SAVE:
        {
            return OR_CONTINUE;
        }
        case COMMAND_IGNORE_HEIGHT_DIFFERENCE_FOLLOWING_NODES:
        {
            return OR_CONTINUE;
        }
        case COMMAND_SHUT_ALL_CHARS_UP:
        {
            return OR_CONTINUE;
        }
        case COMMAND_SET_CHAR_GET_OUT_UPSIDE_DOWN_CAR:
        {
            return OR_CONTINUE;
        }
        case COMMAND_REPORT_MISSION_AUDIO_EVENT_AT_CAR:
        {
            return OR_CONTINUE;
        }
        case COMMAND_DO_WEAPON_STUFF_AT_START_OF_2P_GAME:
        {
            return OR_CONTINUE;
        }
        case COMMAND_SET_MENU_HEADER_ORIENTATION:
        {
            return OR_CONTINUE;
        }
        case COMMAND_HAS_GAME_JUST_RETURNED_FROM_FRONTEND:
        {
            return OR_CONTINUE;
        }
        case COMMAND_GET_CURRENT_LANGUAGE:
        {
            return OR_CONTINUE;
        }
        case COMMAND_IS_OBJECT_INTERSECTING_WORLD:
        {
            return OR_CONTINUE;
        }
        case COMMAND_GET_STRING_WIDTH:
        {
            return OR_CONTINUE;
        }
        case COMMAND_RESET_VEHICLE_HYDRAULICS:
        {
            return OR_CONTINUE;
        }
        case COMMAND_SET_RESPAWN_POINT_FOR_DURATION_OF_MISSION:
        {
            return OR_CONTINUE;
        }
        case COMMAND_IS_THIS_MODEL_A_BIKE:
        {
            return OR_CONTINUE;
        }
        case COMMAND_IS_THIS_MODEL_A_CAR:
        {
            return OR_CONTINUE;
        }
        case COMMAND_SWITCH_ON_GROUND_SEARCHLIGHT:
        {
            return OR_CONTINUE;
        }
        case COMMAND_IS_GANG_WAR_FIGHTING_GOING_ON:
        {
            return OR_CONTINUE;
        }
        case COMMAND_SET_VEHICLE_FIRING_RATE_MULTIPLIER:
        {
            return OR_CONTINUE;
        }
        case COMMAND_GET_VEHICLE_FIRING_RATE_MULTIPLIER:
        {
            return OR_CONTINUE;
        }
        case COMMAND_IS_NEXT_STATION_ALLOWED:
        {
            return OR_CONTINUE;
        }
        case COMMAND_SKIP_TO_NEXT_ALLOWED_STATION:
        {
            return OR_CONTINUE;
        }
        case COMMAND_GET_STRING_WIDTH_WITH_NUMBER:
        {
            return OR_CONTINUE;
        }
        case COMMAND_SHUT_CHAR_UP_FOR_SCRIPTED_SPEECH:
        {
            return OR_CONTINUE;
        }
        case COMMAND_ENABLE_DISABLED_ATTACTORS_ON_OBJECT:
        {
            return OR_CONTINUE;
        }
        case COMMAND_LOAD_SCENE_IN_DIRECTION:
        {
            return OR_CONTINUE;
        }
        case COMMAND_IS_PLAYER_USING_JETPACK:
        {
            return OR_CONTINUE;
        }
        case COMMAND_BLOCK_VEHICLE_MODEL:
        {
            return OR_CONTINUE;
        }
        case COMMAND_CLEAR_THIS_PRINT_BIG_NOW:
        {
            return OR_CONTINUE;
        }
        case COMMAND_HAS_LANGUAGE_CHANGED:
        {
            return OR_CONTINUE;
        }
        case COMMAND_INCREMENT_INT_STAT_NO_MESSAGE:
        {
            return OR_CONTINUE;
        }
        case COMMAND_SET_EXTRA_CAR_COLOURS:
        {
            return OR_CONTINUE;
        }
        case COMMAND_GET_EXTRA_CAR_COLOURS:
        {
            return OR_CONTINUE;
        }
        case COMMAND_MANAGE_ALL_POPULATION:
        {
            return OR_CONTINUE;
        }
        case COMMAND_SET_NO_RESPRAYS:
        {
            return OR_CONTINUE;
        }
        case COMMAND_HAS_CAR_BEEN_RESPRAYED:
        {
            return OR_CONTINUE;
        }
        case COMMAND_ATTACH_MISSION_AUDIO_TO_CAR:
        {
            return OR_CONTINUE;
        }
        case COMMAND_SET_HAS_BEEN_OWNED_FOR_CAR_GENERATOR:
        {
            return OR_CONTINUE;
        }
        case COMMAND_SET_UP_CONVERSATION_NODE_WITH_SCRIPTED_SPEECH:
        {
            return OR_CONTINUE;
        }
        case COMMAND_SET_AREA_NAME:
        {
            return OR_CONTINUE;
        }
        case COMMAND_TASK_PLAY_ANIM_SECONDARY:
        {
            return OR_CONTINUE;
        }
        case COMMAND_IS_CHAR_TOUCHING_CHAR:
        {
            return OR_CONTINUE;
        }
        case COMMAND_DISABLE_HELI_AUDIO:
        {
            return OR_CONTINUE;
        }
        case COMMAND_TASK_HAND_GESTURE:
        {
            return OR_CONTINUE;
        }
        case COMMAND_TAKE_PHOTO:
        {
            return OR_CONTINUE;
        }
        case COMMAND_INCREMENT_FLOAT_STAT_NO_MESSAGE:
        {
            return OR_CONTINUE;
        }
        case COMMAND_SET_PLAYER_GROUP_TO_FOLLOW_ALWAYS:
        {
            return OR_CONTINUE;
        }
        case COMMAND_IMPROVE_CAR_BY_CHEATING:
        {
            return OR_CONTINUE;
        }
        case COMMAND_CHANGE_CAR_COLOUR_FROM_MENU:
        {
            return OR_CONTINUE;
        }
        case COMMAND_HIGHLIGHT_MENU_ITEM:
        {
            return OR_CONTINUE;
        }
        case COMMAND_SET_DISABLE_MILITARY_ZONES:
        {
            return OR_CONTINUE;
        }
        case COMMAND_SET_CAMERA_POSITION_UNFIXED:
        {
            return OR_CONTINUE;
        }
        case COMMAND_SET_RADIO_TO_PLAYERS_FAVOURITE_STATION:
        {
            return OR_CONTINUE;
        }
        case COMMAND_SET_DEATH_WEAPONS_PERSIST:
        {
            return OR_CONTINUE;
        }
    }
    return OR_INTERRUPT;
}

// MARK: 2600To2699

// stub: sa 0x479DA0
OpcodeResult CRunningScript::ProcessCommands2600To2699(i32 CurrCommand)
{
    MARKFUNCTION(0x479DA0);
    bool LatestCmpFlagResult = false;

    switch (CurrCommand)
    {
        case COMMAND_SET_SWIM_SPEED:
        {
            return OR_CONTINUE;
        }
        case COMMAND_IS_PLAYER_CLIMBING:
        {
            return OR_CONTINUE;
        }
        case COMMAND_IS_THIS_HELP_MESSAGE_BEING_DISPLAYED:
        {
            return OR_CONTINUE;
        }
        case COMMAND_IS_WIDESCREEN_ON_IN_OPTIONS:
        {
            LatestCmpFlagResult = FrontEndMenuManager.m_PrefsUseWideScreen;
            UpdateCompareFlag(LatestCmpFlagResult);
            return OR_CONTINUE;
        }
        case COMMAND_DRAW_SUBTITLES_BEFORE_FADE:
        {
            return OR_CONTINUE;
        }
        case COMMAND_DRAW_ODDJOB_TITLE_BEFORE_FADE:
        {
            return OR_CONTINUE;
        }
        case COMMAND_TASK_FOLLOW_PATH_NODES_TO_COORD_WITH_RADIUS:
        {
            return OR_CONTINUE;
        }
        case COMMAND_SET_PHOTO_CAMERA_EFFECT:
        {
            return OR_CONTINUE;
        }
        case COMMAND_FIX_CAR:
        {
            return OR_CONTINUE;
        }
        case COMMAND_SET_PLAYER_GROUP_TO_FOLLOW_NEVER:
        {
            return OR_CONTINUE;
        }
        case COMMAND_IS_CHAR_ATTACHED_TO_ANY_CAR:
        {
            return OR_CONTINUE;
        }
        case COMMAND_STORE_CAR_CHAR_IS_ATTACHED_TO_NO_SAVE:
        {
            return OR_CONTINUE;
        }
        case COMMAND_SET_UP_SKIP_TO_BE_FINISHED_BY_SCRIPT:
        {
            return OR_CONTINUE;
        }
        case COMMAND_SET_UP_SKIP_FOR_VEHICLE_FINISHED_BY_SCRIPT:
        {
            return OR_CONTINUE;
        }
        case COMMAND_IS_SKIP_WAITING_FOR_SCRIPT_TO_FADE_IN:
        {
            return OR_CONTINUE;
        }
        case COMMAND_FORCE_ALL_VEHICLE_LIGHTS_OFF:
        {
            return OR_CONTINUE;
        }
        case COMMAND_SET_RENDER_PLAYER_WEAPON:
        {
            return OR_CONTINUE;
        }
        case COMMAND_GET_PLAYER_IN_CAR_CAMERA_MODE:
        {
            return OR_CONTINUE;
        }
        case COMMAND_IS_LAST_BUILDING_MODEL_SHOT_BY_PLAYER:
        {
            return OR_CONTINUE;
        }
        case COMMAND_CLEAR_LAST_BUILDING_MODEL_SHOT_BY_PLAYER:
        {
            return OR_CONTINUE;
        }
        case COMMAND_SET_UP_CONVERSATION_END_NODE_WITH_SCRIPTED_SPEECH:
        {
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
                CCheat::Disable(PIMP_CHEAT);
            }
            return OR_CONTINUE;
        }
        case COMMAND_GET_RANDOM_CHAR_IN_AREA_OFFSET_NO_SAVE:
        {
            return OR_CONTINUE;
        }
        case COMMAND_SET_SCRIPT_COOP_GAME:
        {
            return OR_CONTINUE;
        }
        case COMMAND_CREATE_USER_3D_MARKER:
        {
            return OR_CONTINUE;
        }
        case COMMAND_REMOVE_USER_3D_MARKER:
        {
            return OR_CONTINUE;
        }
        case COMMAND_REMOVE_ALL_USER_3D_MARKERS:
        {
            return OR_CONTINUE;
        }
        case COMMAND_GET_RID_OF_PLAYER_PROSTITUTE:
        {
            return OR_CONTINUE;
        }
        case COMMAND_DISPLAY_NON_MINIGAME_HELP_MESSAGES:
        {
            return OR_CONTINUE;
        }
        case COMMAND_SET_RAILTRACK_RESISTANCE_MULT:
        {
            return OR_CONTINUE;
        }
        case COMMAND_SWITCH_OBJECT_BRAINS:
        {
            return OR_CONTINUE;
        }
        case COMMAND_FINISH_SETTING_UP_CONVERSATION_NO_SUBTITLES:
        {
            return OR_CONTINUE;
        }
        case COMMAND_ALLOW_PAUSE_IN_WIDESCREEN:
        {
            return OR_CONTINUE;
        }
        case COMMAND_IS_XBOX_VERSION:
        {
            return OR_CONTINUE;
        }
        case COMMAND_GET_PC_MOUSE_MOVEMENT:
        {
            return OR_CONTINUE;
        }
        case COMMAND_IS_PC_USING_JOYPAD:
        {
            return OR_CONTINUE;
        }
        case COMMAND_IS_MOUSE_USING_VERTICAL_INVERSION:
        {
            LatestCmpFlagResult = CMenuManager::bInvertMouseY;
            UpdateCompareFlag(LatestCmpFlagResult);
            return OR_CONTINUE;
        }
        case COMMAND_IS_JAPANESE_VERSION:
        {
            return OR_CONTINUE;
        }
        case COMMAND_IS_XBOX_PLAYER2_PRESSING_START:
        {
            return OR_CONTINUE;
        }
    }
    return OR_INTERRUPT;
}

#undef AddEntityToList
#undef GetActualScriptThingIndex
#undef GetUniqueScriptThingIndex
#undef GetNewUniqueScriptThingIndex
#undef SetDefaultTaskAllocatorType
#undef GetPedFlag
#undef QueryModelsKilledByPlayer
#undef SetTextureAndModel
