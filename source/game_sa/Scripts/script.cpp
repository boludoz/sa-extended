#include "StdInc.h"
#include "TheScripts.h"
#include "RunningScript.h"
#include "UpsideDownCarCheck.h"
#include "Lines.h"
#include "FireManager.h"
#include "PedGroups.h"
#include "Checkpoint.h"
#include "Checkpoints.h"
#include "LoadingScreen.h"
#include "Scripted2dEffects.h"
#include "Shadows.h"
#include "VehicleRecording.h"
#include "TaskComplexLeaveAnyCar.h"
#include "TaskComplexWander.h"
#include "CarCtrl.h"
#include "EntryExitManager.h"
#include "CutsceneMgr.h"
#include "FileLoader.h"
#include "DecisionMakers/DecisionMakerTypes.h"
#include "PedType.h"
#include "PedPlacement.h"
#include "Garages.h"
#include "PostEffects.h"
#include "C_PcSave.h"
#include "Streaming.h"
#include "GenericGameStorage.h"
#include "Radar.h"
#include "SpecialFX.h"
#include "PedScriptedTaskRecord.h"
#include "TaskSimpleCarSetPedInAsPassenger.h"
#include "TaskSimpleCarSetPedInAsDriver.h"
#include "TaskComplexEnterCarAsPassengerTimed.h"
#include "TaskComplexEnterCarAsDriverTimed.h"
#include "TaskComplexWanderStandard.h"
#include "TaskSimpleStandStill.h"
#include "TaskComplexSequence.h"
#include "TaskComplexLeaveCar.h"
#include "CarGenerator.h"
#include "Hud.h"
#include "extensions/File.hpp"
#include "TaskSimpleFinishBrain.h"

#include "spdlog/sinks/stdout_color_sinks.h"
#include <fstream>
#include "CommandParser/Parser.hpp"
#include "CommandParser/LUTGenerator.hpp"
#include "reversiblehooks/ReversibleHook/ScriptCommand.h"
#include "Commands/Commands.hpp"
#ifdef NOTSA_WITH_CLEO_SCRIPT_COMMANDS
#include "Commands/CLEO/CLEOCommands.hpp"
#endif


static notsa::log_ptr logger;

static inline auto& ScriptsArray = StaticRef<std::array<CRunningScript, MAX_NUM_SCRIPTS>>(0xA8B430);

static inline std::array<notsa::script::CommandHandlerFunction, (size_t)(COMMAND_HIGHEST_ID_TO_HOOK) + 1> s_CustomCommandHandlerTable{};

std::array<std::array<char, COMMANDS_CHAR_BUFFER_SIZE>, COMMANDS_CHAR_BUFFERS_COUNT> CRunningScript::ScriptArgCharBuffers        = {};
uint8                                                                                CRunningScript::ScriptArgCharNextFreeBuffer = 0;

void InjectHooks_Script4();
void InjectHooks_Script5();
void InjectHooks_Script6();
void InjectHooks_Script9();
void InjectHooks_Script10();
void InjectHooks_Script11();
void InjectHooks_Script12();

void CRunningScript::InjectHooks() {
    logger = NOTSA_MAKE_LOGGER("script");

    RH_ScopedClass(CRunningScript);
    RH_ScopedCategory("Scripts");

    RH_ScopedInstall(ProcessCommands0To99, 0x465E60);
    RH_ScopedInstall(ProcessCommands100To199, 0x466DE0);
    RH_ScopedInstall(ProcessCommands200To299, 0x469390);
    RH_ScopedInstall(ProcessCommands300To399, 0x47C100);
    RH_ScopedInstall(ProcessCommands400To499, 0x47D210);
    RH_ScopedInstall(ProcessCommands500To599, 0x47E090);
    RH_ScopedInstall(ProcessCommands600To699, 0x47F370);
    RH_ScopedInstall(ProcessCommands700To799, 0x47FA30);
    RH_ScopedInstall(ProcessCommands800To899, 0x481300);
    RH_ScopedInstall(ProcessCommands900To999, 0x483BD0);
    RH_ScopedInstall(ProcessCommands1000To1099, 0x489500);
    RH_ScopedInstall(ProcessCommands1100To1199, 0x48A320);
    RH_ScopedInstall(ProcessCommands1200To1299, 0x48B590);
    RH_ScopedInstall(ProcessCommands1300To1399, 0x48CDD0);
    RH_ScopedInstall(ProcessCommands1400To1499, 0x48EAA0);
    RH_ScopedInstall(ProcessCommands1500To1599, 0x490DB0);
    RH_ScopedInstall(ProcessCommands1600To1699, 0x493FE0);
    RH_ScopedInstall(ProcessCommands1700To1799, 0x496E00);
    RH_ScopedInstall(ProcessCommands1800To1899, 0x46D050);
    RH_ScopedInstall(ProcessCommands1900To1999, 0x46B460);
    RH_ScopedInstall(ProcessCommands2000To2099, 0x472310);
    RH_ScopedInstall(ProcessCommands2100To2199, 0x470A90);
    RH_ScopedInstall(ProcessCommands2200To2299, 0x474900);
    RH_ScopedInstall(ProcessCommands2300To2399, 0x4762D0);
    RH_ScopedInstall(ProcessCommands2400To2499, 0x478000);
    RH_ScopedInstall(ProcessCommands2500To2599, 0x47A760);
    RH_ScopedInstall(ProcessCommands2600To2699, 0x479DA0);

    RH_ScopedInstall(Init, 0x4648E0);
    RH_ScopedInstall(GetCorrectPedModelIndexForEmergencyServiceType, 0x464F50);
    RH_ScopedInstall(CollectParameters, 0x464080, { .stackArguments = 1 });
    RH_ScopedInstall(CollectNextParameterWithoutIncreasingPC, 0x464250, { .stackArguments = 0 });
    RH_ScopedInstall(StoreParameters, 0x464370, { .stackArguments = 1 });
    RH_ScopedInstall(ReadArrayInformation, 0x463CF0, { .stackArguments = 3 });
    RH_ScopedInstall(ReadParametersForNewlyStartedScript, 0x464500, { .stackArguments = 1 });
    RH_ScopedInstall(ReadTextLabelFromScript, 0x463D50, { .stackArguments = 2 });
    RH_ScopedInstall(GetIndexOfGlobalVariable, 0x464700, { .stackArguments = 0 });
    RH_ScopedInstall(GetPadState, 0x485B10);
    RH_ScopedInstall(GetPointerToLocalVariable, 0x463CA0, { .stackArguments = 1 });
    RH_ScopedInstall(GetPointerToLocalArrayElement, 0x463CC0, { .stackArguments = 3 });
    RH_ScopedInstall(GetPointerToScriptVariable, 0x464790, { .stackArguments = 1 });
    RH_ScopedInstall(SetCharCoordinates, 0x464DC0);
    RH_ScopedInstall(AddScriptToList, 0x464C00, { .stackArguments = 1 });
    RH_ScopedInstall(RemoveScriptFromList, 0x464BD0, { .stackArguments = 1 });
    RH_ScopedInstall(ShutdownThisScript, 0x465AA0);
    RH_ScopedInstall(IsPedDead, 0x464D70);
    RH_ScopedInstall(UpdatePC, 0x464DA0, { .stackArguments = 1 });
    RH_ScopedInstall(ProcessOneCommand, 0x469EB0);
    RH_ScopedInstall(Process, 0x469F00);
    RH_ScopedOverloadedInstall(GivePedScriptedTask, "OG", 0x465C20, void(CRunningScript::*)(int32, CTask*, int32));
}

void CTheScripts::InjectHooks() {
#ifndef NOTSA_STANDALONE
    assert((void*)0xA49960 == (void*)&MainSCMBlock[0]);
    assert((void*)(0xA7A6A0) == (void*)&MissionBlock[0]);
#endif

    RH_ScopedClass(CTheScripts);
    RH_ScopedCategory("Scripts");

    RH_ScopedInstall(Init, 0x468D50);
    RH_ScopedInstall(DoScriptSetupAfterPoolsHaveLoaded, 0x5D3390);
    RH_ScopedInstall(GetScriptIndexFromPointer, 0x464D20);
    RH_ScopedInstall(RemoveScriptTextureDictionary, 0x465A40);
    RH_ScopedOverloadedInstall(StartNewScript, "last-idle", 0x464C20, CRunningScript* (*)(uint8*));
    RH_ScopedOverloadedInstall(StartNewScript, "indexed", 0x464C90, CRunningScript* (*)(uint8*, uint16));
    RH_ScopedInstall(IsPlayerOnAMission, 0x464D50);
    RH_ScopedInstall(Load, 0x5D4FD0);
    RH_ScopedInstall(Save, 0x5D4C40);
    RH_ScopedInstall(WipeLocalVariableMemoryForMissionScript, 0x464BB0);
    RH_ScopedInstall(StartTestScript, 0x464D40);
    RH_ScopedInstall(Process, 0x46A000);
    RH_ScopedInstall(PrintListSizes, 0x4646D0);
    RH_ScopedInstall(DrawScriptSpritesAndRectangles, 0x464980);
    RH_ScopedInstall(CheckStreamedScriptVersion, 0x464FF0);

    InjectHooks_Script4();
    InjectHooks_Script5();
    InjectHooks_Script6();
    InjectHooks_Script9();
    InjectHooks_Script10();
    InjectHooks_Script11();
    InjectHooks_Script12();
}


void CRunningScript::InjectCustomCommandHooks() {
    // Custom handlers from Commands/ are disabled in favor of original modular opcode tables
}


// 0x4648E0
void CRunningScript::Init() {
    SetName("noname");
    rng::fill(m_IPStack, nullptr);
    rng::fill(m_LocalVars, tScriptParam{ 0 });
    m_BaseIP                          = nullptr;
    m_pPrev                           = nullptr;
    m_pNext                           = nullptr;
    m_IP                              = nullptr;
    m_StackDepth                      = 0;
    m_WakeTime                        = 0;
    m_IsActive                        = false;
    m_CondResult                      = false;
    m_UsesMissionCleanup              = false;
    m_IsExternal                      = false;
    m_IsTextBlockOverride             = false;
    m_ExternalType                    = -1;
    m_AndOrState                      = 0;
    m_NotFlag                         = false;
    m_DoneDeathArrest                 = false;
    m_SceneSkipIP                     = 0;
    m_ThisMustBeTheOnlyMissionRunning = false;
    m_IsDeathArrestCheckEnabled       = true;
}


/*!
 * Adds script to list
 * @addr 0x464C00
 */
void CRunningScript::AddScriptToList(CRunningScript** queueList) {
    m_pNext = *queueList;
    m_pPrev = nullptr;
    if (*queueList)
        (*queueList)->m_pPrev = this;
    *queueList = this;
}


/*!
 * Removes script from list
 * @addr 0x464BD0
 */
void CRunningScript::RemoveScriptFromList(CRunningScript** queueList) {
    if (m_pPrev)
        m_pPrev->m_pNext = m_pNext;
    else
        *queueList = m_pNext;

    if (m_pNext)
        m_pNext->m_pPrev = m_pPrev;
}

// 0x465AA0
void CRunningScript::ShutdownThisScript() {
    m_IsActive = false;
    if (m_IsExternal) {
        const auto idx = CTheScripts::StreamedScripts.GetStreamedScriptWithThisStartAddress(m_BaseIP);
        const auto currentNumUsers = CTheScripts::StreamedScripts.GetNumUsersOfStreamedScript(idx);
        CTheScripts::StreamedScripts.SetNumUsersOfStreamedScript(idx, currentNumUsers - 1);
    }

    switch (m_ExternalType) {
    case 0: // PED_STREAMED
    case 2: // PED_GENERATOR_STREAMED
    case 3: // CODE_PED
    case 5: { // CODE_ATTRACTOR_PED
        const auto pLocalVariable = GetPointerToLocalVariable(0);
        if (const auto ped = GetPedPool()->GetAtRef(pLocalVariable->iParam)) {
            ped->bHasAScriptBrain = false;
            if (m_ExternalType == 5) {
                CScriptedBrainTaskStore::SetTask(ped, new CTaskSimpleFinishBrain{});
            }
        }
        break;
    }
    case 1: // OBJECT_STREAMED
    case 4: { // CODE_OBJECT
        const auto pLocalVariable = GetPointerToLocalVariable(0);
        if (const auto obj = GetObjectPool()->GetAtRef(pLocalVariable->iParam)) {
            obj->m_nObjectFlags.ScriptBrainStatus = 0;
        }
        break;
    }
    default:
        break;
    }
}


// 0x465C20
void CRunningScript::GivePedScriptedTask(int32 pedHandle, CTask* task, int32 opcode) {
    if (pedHandle == -1) {
        CTaskSequences::AddTaskToActiveSequence(task);
        return;
    }

    CPed* ped = GetPedPool()->GetAtRef(pedHandle);
    assert(ped);
    CPedGroup* pedGroup = CPedGroups::GetPedsGroup(ped);

    CPed* otherPed = nullptr;
    if (m_ExternalType == 5 || m_ExternalType == 2 || !m_ExternalType || m_ExternalType == 3) {
        auto* localVariable = reinterpret_cast<int32*>(GetPointerToLocalVariable(0));
        otherPed = GetPedPool()->GetAtRef(*localVariable);
    }

    if (ped->bHasAScriptBrain && otherPed != ped) {
        delete task;
    } else if (otherPed && m_ExternalType == 5) {
        if (CScriptedBrainTaskStore::SetTask(ped, task)) {
            const int32 slot = CPedScriptedTaskRecord::GetVacantSlot();
            CPedScriptedTaskRecord::ms_scriptedTasks[slot].SetAsAttractorScriptTask(ped, opcode, task);
        }
    } else if (!pedGroup || ped->IsPlayer()) {
        CEventScriptCommand eventScriptCommand(TASK_PRIMARY_PRIMARY, task, false);
        auto* event = static_cast<CEventScriptCommand*>(ped->GetEventGroup().Add(&eventScriptCommand, false));
        if (event) {
            const int32 slot = CPedScriptedTaskRecord::GetVacantSlot();
            CPedScriptedTaskRecord::ms_scriptedTasks[slot].Set(ped, opcode, event);
        }
    } else {
        pedGroup->GetIntelligence().SetScriptCommandTask(ped, *task);
        CTask* scriptedTask = pedGroup->GetIntelligence().GetTaskScriptCommand(ped);
        const int32 slot = CPedScriptedTaskRecord::GetVacantSlot();
        CPedScriptedTaskRecord::ms_scriptedTasks[slot].SetAsGroupTask(ped, opcode, scriptedTask);
        delete task;
    }
}



void CRunningScript::GivePedScriptedTask(CPed* ped, CTask* task, int32 opcode) {
    GivePedScriptedTask(GetPedPool()->GetRef(ped), task, opcode); // Must do it like this, otherwise unhooking of the original `GivePedScriptedTask` will do nothing
}


// 0x464D70
bool CRunningScript::IsPedDead(CPed* ped) const {
    ePedState pedState = ped->m_nPedState;
    return pedState == PEDSTATE_DEAD || pedState == PEDSTATE_DIE || pedState == PEDSTATE_DIE_BY_STEALTH;
}


// 0x464F50
void CRunningScript::GetCorrectPedModelIndexForEmergencyServiceType(ePedType pedType, uint32* typeSpecificModelId) {
    switch (*typeSpecificModelId) {
    case MODEL_LAPD1:
    case MODEL_SFPD1:
    case MODEL_LVPD1:
    case MODEL_LAPDM1:
        if (pedType == PED_TYPE_COP) {
            *typeSpecificModelId = COP_TYPE_CITYCOP;
        }
        break;
    case MODEL_CSHER:
        if (pedType == PED_TYPE_COP) {
            *typeSpecificModelId = COP_TYPE_CSHER;
        }
        break;
    case MODEL_SWAT:
        if (pedType == PED_TYPE_COP) {
            *typeSpecificModelId = COP_TYPE_SWAT1;
        }
        break;
    case MODEL_FBI:
        if (pedType == PED_TYPE_COP) {
            *typeSpecificModelId = COP_TYPE_FBI;
        }
        break;
    case MODEL_ARMY:
        if (pedType == PED_TYPE_COP) {
            *typeSpecificModelId = COP_TYPE_ARMY;
        }
        break;
    default:
        return;
    }
}


// Returns state of pad button
// 0x485B10
int16 CRunningScript::GetPadState(uint16 playerIndex, eButtonId buttonId) {
    const auto* pad = CPad::GetPad(playerIndex);
    switch (buttonId) {
    case BUTTON_LEFT_STICK_X:    return pad->NewState.LeftStickX;
    case BUTTON_LEFT_STICK_Y:    return pad->NewState.LeftStickY;
    case BUTTON_RIGHT_STICK_X:   return pad->NewState.RightStickX;
    case BUTTON_RIGHT_STICK_Y:   return pad->NewState.RightStickY;
    case BUTTON_LEFT_SHOULDER1:  return pad->NewState.LeftShoulder1;
    case BUTTON_LEFT_SHOULDER2:  return pad->NewState.LeftShoulder2;
    case BUTTON_RIGHT_SHOULDER1: return pad->NewState.RightShoulder1;
    case BUTTON_RIGHT_SHOULDER2: return pad->NewState.RightShoulder2;
    case BUTTON_DPAD_UP:         return pad->NewState.DPadUp;
    case BUTTON_DPAD_DOWN:       return pad->NewState.DPadDown;
    case BUTTON_DPAD_LEFT:       return pad->NewState.DPadLeft;
    case BUTTON_DPAD_RIGHT:      return pad->NewState.DPadRight;
    case BUTTON_START:           return pad->NewState.Start;
    case BUTTON_SELECT:          return pad->NewState.Select;
    case BUTTON_SQUARE:          return pad->NewState.ButtonSquare;
    case BUTTON_TRIANGLE:        return pad->NewState.ButtonTriangle;
    case BUTTON_CROSS:           return pad->NewState.ButtonCross;
    case BUTTON_CIRCLE:          return pad->NewState.ButtonCircle;
    case BUTTON_LEFTSHOCK:       return pad->NewState.ShockButtonL;
    case BUTTON_RIGHTSHOCK:      return pad->NewState.ShockButtonR;
    default:                     return OR_CONTINUE;
    }
}


// 0x464DC0
void CRunningScript::SetCharCoordinates(CPed& ped, CVector posn, bool warpGang, bool offset) {
    CWorld::PutToGroundIfTooLow(posn);

    CVehicle* vehicle = ped.GetVehicleIfInOne();
    if (vehicle) {
        posn.z += vehicle->GetDistanceFromCentreOfMassToBaseOfModel();
        vehicle->Teleport(posn, false);
        CTheScripts::ClearSpaceForMissionEntity(posn, vehicle);
    } else {
        posn.z += offset ? ped.GetDistanceFromCentreOfMassToBaseOfModel() : 0.0f;
        CTheScripts::ClearSpaceForMissionEntity(posn, &ped);
        auto* group = CPedGroups::GetPedsGroup(&ped);
        if (group && group->GetMembership().IsLeader(&ped) && warpGang) {
            group->Teleport(posn);
        } else {
            ped.Teleport(posn, false);
        }
    }
}


// 0x463CA0
tScriptParam* CRunningScript::GetPointerToLocalVariable(int32 loc) {
    return &GetLocal<tScriptParam>(loc);
}


/*!
 * @addr 0x463CC0
 * @brief Returns pointer to a local script variable.
 *
 * @param arrayBaseOffset          The offset of the array
 * @param index                    Index of the variable inside the array
 * @param arrayEntriesSizeAsParams Size of 1 variable in the array (In terms of `tScriptParam`'s - So for a regular `int` (or float, etc) variable this will be `1`, for long strings it's `4` and for short one's it's `2`)
 */
tScriptParam* CRunningScript::GetPointerToLocalArrayElement(int32 arrayBaseOffset, uint16 index, uint8 arrayEntriesSizeAsParams) {
    return &GetArrayLocal<tScriptParam>(arrayBaseOffset, index, arrayEntriesSizeAsParams);
}


/*!
 * Returns pointer to script variable of any type.
 * @addr 0x464790
 */
tScriptParam* CRunningScript::GetPointerToScriptVariable(eScriptVariableType) {
    uint8  arrElemSize;
    uint16 arrVarOffset;
    int32  arrElemIdx;

    int8 type = CTheScripts::Read1ByteFromScript(m_IP);
    switch (type) {
    case SCRIPT_PARAM_GLOBAL_NUMBER_VARIABLE:
    case SCRIPT_PARAM_GLOBAL_SHORT_STRING_VARIABLE:
    case SCRIPT_PARAM_GLOBAL_LONG_STRING_VARIABLE:
    {
        uint16 index = CTheScripts::Read2BytesFromScript(m_IP);
        return reinterpret_cast<tScriptParam*>(&CTheScripts::ScriptSpace[index]);
    }
    case SCRIPT_PARAM_LOCAL_NUMBER_VARIABLE:
    case SCRIPT_PARAM_LOCAL_SHORT_STRING_VARIABLE:
    case SCRIPT_PARAM_LOCAL_LONG_STRING_VARIABLE:
    {
        uint16 index = CTheScripts::Read2BytesFromScript(m_IP);
        return GetPointerToLocalVariable(index);
    }

    case SCRIPT_PARAM_GLOBAL_NUMBER_ARRAY:
    case SCRIPT_PARAM_GLOBAL_SHORT_STRING_ARRAY:
    case SCRIPT_PARAM_GLOBAL_LONG_STRING_ARRAY:
        ReadArrayInformation(true, &arrVarOffset, &arrElemIdx);
        if (type == SCRIPT_PARAM_GLOBAL_LONG_STRING_ARRAY)
            return reinterpret_cast<tScriptParam*>(&CTheScripts::ScriptSpace[LONG_STRING_SIZE * arrElemIdx + arrVarOffset]);
        else if (type == SCRIPT_PARAM_GLOBAL_SHORT_STRING_ARRAY)
            return reinterpret_cast<tScriptParam*>(&CTheScripts::ScriptSpace[SHORT_STRING_SIZE * arrElemIdx + arrVarOffset]);
        else // SCRIPT_PARAM_GLOBAL_NUMBER_ARRAY
            return reinterpret_cast<tScriptParam*>(&CTheScripts::ScriptSpace[4 * arrElemIdx + arrVarOffset]);

    case SCRIPT_PARAM_LOCAL_NUMBER_ARRAY:
    case SCRIPT_PARAM_LOCAL_SHORT_STRING_ARRAY:
    case SCRIPT_PARAM_LOCAL_LONG_STRING_ARRAY:
        ReadArrayInformation(true, &arrVarOffset, &arrElemIdx);
        if (type == SCRIPT_PARAM_LOCAL_LONG_STRING_ARRAY)
            arrElemSize = 4;
        else if (type == SCRIPT_PARAM_LOCAL_SHORT_STRING_ARRAY)
            arrElemSize = 2;
        else // SCRIPT_PARAM_LOCAL_NUMBER_ARRAY
            arrElemSize = 1;
        return GetPointerToLocalArrayElement(arrVarOffset, arrElemIdx, arrElemSize);

    default:
        NOTSA_UNREACHABLE();
    }
}


/*!
 * Returns offset of global variable
 * @addr 0x464700
 */
uint16 CRunningScript::GetIndexOfGlobalVariable() {
    switch (const auto t = GetAtIPAs<uint8>()) {
    case SCRIPT_PARAM_GLOBAL_NUMBER_VARIABLE:
        return GetAtIPAs<uint16>();
    case SCRIPT_PARAM_GLOBAL_NUMBER_ARRAY: {
        uint16 base;
        int32  idx;
        ReadArrayInformation(true, &base, &idx);
        return base + sizeof(tScriptParam) * idx;
    }
    default:
        NOTSA_UNREACHABLE();
    }
}


// 0x464080
void CRunningScript::CollectParameters(int16 count) {
    uint16 arrVarOffset;
    int32  arrElemIdx;

    for (auto i = 0; i < count; i++) {
        switch (CTheScripts::Read1ByteFromScript(m_IP)) {
        case SCRIPT_PARAM_STATIC_INT_32BITS:
            ScriptParams[i].iParam = CTheScripts::Read4BytesFromScript(m_IP);
            break;
        case SCRIPT_PARAM_GLOBAL_NUMBER_VARIABLE:
        {
            uint16 index = CTheScripts::Read2BytesFromScript(m_IP);
            ScriptParams[i].iParam = *reinterpret_cast<int32*>(&CTheScripts::ScriptSpace[index]);
            break;
        }
        case SCRIPT_PARAM_LOCAL_NUMBER_VARIABLE:
        {
            uint16 index = CTheScripts::Read2BytesFromScript(m_IP);
            ScriptParams[i] = *GetPointerToLocalVariable(index);
            break;
        }
        case SCRIPT_PARAM_STATIC_INT_8BITS:
            ScriptParams[i].iParam = CTheScripts::Read1ByteFromScript(m_IP);
            break;
        case SCRIPT_PARAM_STATIC_INT_16BITS:
            ScriptParams[i].iParam = CTheScripts::Read2BytesFromScript(m_IP);
            break;
        case SCRIPT_PARAM_STATIC_FLOAT:
            ScriptParams[i].fParam = CTheScripts::ReadFloatFromScript(m_IP);
            break;
        case SCRIPT_PARAM_GLOBAL_NUMBER_ARRAY:
            ReadArrayInformation(true, &arrVarOffset, &arrElemIdx);
            ScriptParams[i].iParam = *reinterpret_cast<int32*>(&CTheScripts::ScriptSpace[arrVarOffset + 4 * arrElemIdx]);
            break;
        case SCRIPT_PARAM_LOCAL_NUMBER_ARRAY:
            ReadArrayInformation(true, &arrVarOffset, &arrElemIdx);
            ScriptParams[i] = *GetPointerToLocalArrayElement(arrVarOffset, arrElemIdx, 1);
            break;
        }
    }
}


/*!
 * Collects parameter and returns it.
 * @addr 0x464250
 */
int32 CRunningScript::CollectNextParameterWithoutIncreasingPC() {
    uint16 arrVarOffset;
    int32  arrElemIdx;
    uint8* ip = m_IP;
    int32  result = -1;

    switch (CTheScripts::Read1ByteFromScript(m_IP)) {
    case SCRIPT_PARAM_STATIC_INT_32BITS:
    case SCRIPT_PARAM_STATIC_FLOAT:
        result = CTheScripts::Read4BytesFromScript(m_IP);
        break;
    case SCRIPT_PARAM_GLOBAL_NUMBER_VARIABLE:
    {
        uint16 index = CTheScripts::Read2BytesFromScript(m_IP);
        result = *reinterpret_cast<int32*>(&CTheScripts::ScriptSpace[index]);
        break;
    }
    case SCRIPT_PARAM_LOCAL_NUMBER_VARIABLE:
    {
        uint16 index = CTheScripts::Read2BytesFromScript(m_IP);
        result = GetPointerToLocalVariable(index)->iParam;
        break;
    }
    case SCRIPT_PARAM_STATIC_INT_8BITS:
        result = CTheScripts::Read1ByteFromScript(m_IP);
        break;
    case SCRIPT_PARAM_STATIC_INT_16BITS:
        result = CTheScripts::Read2BytesFromScript(m_IP);
        break;
    case SCRIPT_PARAM_GLOBAL_NUMBER_ARRAY:
        ReadArrayInformation(false, &arrVarOffset, &arrElemIdx);
        result = *reinterpret_cast<int32*>(&CTheScripts::ScriptSpace[arrVarOffset + 4 * arrElemIdx]);
        break;
    case SCRIPT_PARAM_LOCAL_NUMBER_ARRAY:
        ReadArrayInformation(false, &arrVarOffset, &arrElemIdx);
        result = GetPointerToLocalArrayElement(arrVarOffset, arrElemIdx, 1)->iParam;
        break;
    }

    m_IP = ip;
    return result;
}


/*!
 * @addr 0x464370
 */
void CRunningScript::StoreParameters(int16 count) {
    uint16 arrVarOffset;
    int32  arrElemIdx;

    for (auto i = 0; i < count; i++) {
        switch (CTheScripts::Read1ByteFromScript(m_IP)) {
        case SCRIPT_PARAM_GLOBAL_NUMBER_VARIABLE:
        {
            uint16 index = CTheScripts::Read2BytesFromScript(m_IP);
            *reinterpret_cast<int32*>(&CTheScripts::ScriptSpace[index]) = ScriptParams[i].iParam;
            break;
        }
        case SCRIPT_PARAM_LOCAL_NUMBER_VARIABLE:
        {
            uint16 index = CTheScripts::Read2BytesFromScript(m_IP);
            *GetPointerToLocalVariable(index) = ScriptParams[i];
            break;
        }
        case SCRIPT_PARAM_GLOBAL_NUMBER_ARRAY:
            ReadArrayInformation(true, &arrVarOffset, &arrElemIdx);
            *reinterpret_cast<int32*>(&CTheScripts::ScriptSpace[arrVarOffset + 4 * arrElemIdx]) = ScriptParams[i].iParam;
            break;
        case SCRIPT_PARAM_LOCAL_NUMBER_ARRAY:
            ReadArrayInformation(true, &arrVarOffset, &arrElemIdx);
            *GetPointerToLocalArrayElement(arrVarOffset, arrElemIdx, 1) = ScriptParams[i];
            break;
        }
    }
}


// Reads array var base offset and element index from index variable.
// 0x463CF0
void CRunningScript::ReadArrayInformation(int32 updateIP, uint16* outArrayBase, int32* outArrayIndex) {
    const auto op = GetAtIPAs<scm::ArrayAccess>(updateIP);
    *outArrayIndex = op.IdxVarIsGlobal
        ? GetGlobal<int32>(op.IdxVarLoc)
        : GetLocal<int32>(op.IdxVarLoc);
    *outArrayBase = op.ArrayBase;
}


// Collects parameters and puts them to local variables of new script
// 0x464500
void CRunningScript::ReadParametersForNewlyStartedScript(CRunningScript* newScript) {
    uint16 arrVarOffset;
    int32  arrElemIdx;
    int8   type = CTheScripts::Read1ByteFromScript(m_IP);

    for (int i = 0; type != SCRIPT_PARAM_END_OF_ARGUMENTS; type = CTheScripts::Read1ByteFromScript(m_IP), i++) {
        switch (type) {
        case SCRIPT_PARAM_STATIC_INT_32BITS:
            newScript->m_LocalVars[i].iParam = CTheScripts::Read4BytesFromScript(m_IP);
            break;
        case SCRIPT_PARAM_GLOBAL_NUMBER_VARIABLE:
        {
            uint16 index = CTheScripts::Read2BytesFromScript(m_IP);
            newScript->m_LocalVars[i].iParam = *reinterpret_cast<int32*>(&CTheScripts::ScriptSpace[index]);
            break;
        }
        case SCRIPT_PARAM_LOCAL_NUMBER_VARIABLE:
        {
            uint16 index = CTheScripts::Read2BytesFromScript(m_IP);
            newScript->m_LocalVars[i] = *GetPointerToLocalVariable(index);
            break;
        }
        case SCRIPT_PARAM_STATIC_INT_8BITS:
            newScript->m_LocalVars[i].iParam = CTheScripts::Read1ByteFromScript(m_IP);
            break;
        case SCRIPT_PARAM_STATIC_INT_16BITS:
            newScript->m_LocalVars[i].iParam = CTheScripts::Read2BytesFromScript(m_IP);
            break;
        case SCRIPT_PARAM_STATIC_FLOAT:
            newScript->m_LocalVars[i].fParam = CTheScripts::ReadFloatFromScript(m_IP);
            break;
        case SCRIPT_PARAM_GLOBAL_NUMBER_ARRAY:
            ReadArrayInformation(true, &arrVarOffset, &arrElemIdx);
            newScript->m_LocalVars[i].iParam = *reinterpret_cast<int32*>(&CTheScripts::ScriptSpace[arrVarOffset + 4 * arrElemIdx]);
            break;
        case SCRIPT_PARAM_LOCAL_NUMBER_ARRAY:
            ReadArrayInformation(true, &arrVarOffset, &arrElemIdx);
            newScript->m_LocalVars[i] = *GetPointerToLocalArrayElement(arrVarOffset, arrElemIdx, 1);
            break;
        default:
            break;
        }
    }
}


// Collects string parameter
// 0x463D50
void CRunningScript::ReadTextLabelFromScript(char* buffer, uint8 nBufferLength) {
    uint16 arrVarOffset;
    int32  arrElemIdx;

    int8 type = CTheScripts::Read1ByteFromScript(m_IP);
    switch (type) {
    case SCRIPT_PARAM_STATIC_SHORT_STRING:
        for (auto i = 0; i < SHORT_STRING_SIZE; i++)
            buffer[i] = CTheScripts::Read1ByteFromScript(m_IP);
        break;

    case SCRIPT_PARAM_GLOBAL_SHORT_STRING_VARIABLE:
    {
        uint16 index = CTheScripts::Read2BytesFromScript(m_IP);
        strncpy_s(buffer, SHORT_STRING_SIZE, (char*)&CTheScripts::ScriptSpace[index], SHORT_STRING_SIZE);
        break;
    }

    case SCRIPT_PARAM_LOCAL_SHORT_STRING_VARIABLE:
    {
        uint16 index = CTheScripts::Read2BytesFromScript(m_IP);
        strncpy_s(buffer, SHORT_STRING_SIZE, (char*) GetPointerToLocalVariable(index), SHORT_STRING_SIZE);
        break;
    }

    case SCRIPT_PARAM_GLOBAL_SHORT_STRING_ARRAY:
    case SCRIPT_PARAM_GLOBAL_LONG_STRING_ARRAY:
        ReadArrayInformation(true, &arrVarOffset, &arrElemIdx);
        if (type == SCRIPT_PARAM_GLOBAL_SHORT_STRING_ARRAY)
            strncpy_s(buffer, SHORT_STRING_SIZE, (char*) & CTheScripts::ScriptSpace[SHORT_STRING_SIZE * arrElemIdx + arrVarOffset], SHORT_STRING_SIZE);
        else
            strncpy_s(buffer, SHORT_STRING_SIZE, (char*) & CTheScripts::ScriptSpace[LONG_STRING_SIZE * arrElemIdx + arrVarOffset], std::min<uint8>(nBufferLength, LONG_STRING_SIZE));
        break;

    case SCRIPT_PARAM_LOCAL_SHORT_STRING_ARRAY:
    case SCRIPT_PARAM_LOCAL_LONG_STRING_ARRAY:
        ReadArrayInformation(true, &arrVarOffset, &arrElemIdx);
        if (type == SCRIPT_PARAM_LOCAL_SHORT_STRING_ARRAY)
            strncpy_s(buffer, SHORT_STRING_SIZE, (char*) GetPointerToLocalArrayElement(arrVarOffset, arrElemIdx, 2), SHORT_STRING_SIZE);
        else {
            const auto bufferLength = std::min<uint8>(nBufferLength, LONG_STRING_SIZE);
            strncpy_s(buffer, bufferLength, (char*)GetPointerToLocalArrayElement(arrVarOffset, arrElemIdx, 4), bufferLength);
        }
        break;

    case SCRIPT_PARAM_STATIC_PASCAL_STRING:
    {
        int16 nStringLen = CTheScripts::Read1ByteFromScript(m_IP); // sign extension. max size = 127, not 255
        for (auto i = 0; i < nStringLen; i++)
            buffer[i] = CTheScripts::Read1ByteFromScript(m_IP);

        if (nStringLen < nBufferLength)
            memset(&buffer[(uint8)nStringLen], 0, (uint8)(nBufferLength - nStringLen));
        break;
    }

    case SCRIPT_PARAM_STATIC_LONG_STRING:
        // slightly changed code: original code is a bit messy and calls Read1ByteFromScript
        // in a loop and does some additional checks to ensure that buffer can hold the data
        strncpy_s(buffer, LONG_STRING_SIZE, (char*)m_IP, std::min<uint8>(nBufferLength, LONG_STRING_SIZE));
        m_IP += LONG_STRING_SIZE;
        break;

    case SCRIPT_PARAM_GLOBAL_LONG_STRING_VARIABLE:
    {
        uint16 index = CTheScripts::Read2BytesFromScript(m_IP);
        strncpy_s(buffer, LONG_STRING_SIZE, (char*) & CTheScripts::ScriptSpace[index], std::min<uint8>(nBufferLength, LONG_STRING_SIZE));
        break;
    }

    case SCRIPT_PARAM_LOCAL_LONG_STRING_VARIABLE:
    {
        uint16 index = CTheScripts::Read2BytesFromScript(m_IP);
        strncpy_s(buffer, LONG_STRING_SIZE, (char*) GetPointerToLocalVariable(index), std::min<uint8>(nBufferLength, LONG_STRING_SIZE));
        break;
    }

    default:
        break;
    }
}


// Sets instruction pointer, used in GOTO-like commands
// 0x464DA0
void CRunningScript::UpdatePC(int32 newIP) {
    m_IP = newIP >= 0
        ? &CTheScripts::ScriptSpace[newIP]
        : m_BaseIP + std::abs(newIP);
}


// 0x469EB0, inlined
OpcodeResult CRunningScript::ProcessOneCommand() {
    ++CTheScripts::CommandsExecuted;

    const auto op = GetAtIPAs<scm::Instruction>();

    // Check if IP is valid pre-return
    notsa::ScopeGuard guardIP{[this]() {
        const auto next{ GetAtIPAs<scm::Instruction>(false) };
        VERIFY(next.Command <= COMMAND_HIGHEST_VANILLA_ID);
    }};

#ifdef NOTSA_SCRIPT_TRACING
    // snprintf is faster (in debug at least) - Gotta stick to it for now
    char msg[4096];
    sprintf_s(msg, "[%s][IP: 0x%X + 0x%X]: %s [0x%X]", m_szName, LOG_PTR(m_pBaseIP), LOG_PTR(m_IP - m_pBaseIP), notsa::script::GetScriptCommandName((eScriptCommands)op.Command).data(), (size_t)op.Command);
    SPDLOG_LOGGER_TRACE(logger, msg);
    //SPDLOG_LOGGER_TRACE(logger, "[{}][IP: {:#x} + {:#x}]: {} [{:#x}]", BaseFilename, LOG_PTR(m_pBaseIP), LOG_PTR(m_IP - m_pBaseIP), notsa::script::GetScriptCommandName((eScriptCommands)op.command), (size_t)op.command);
#endif
    
    m_NotFlag = op.NotFlag;

    if (const auto handler = CustomCommandHandlerOf((eScriptCommands)(op.Command))) {
        return std::invoke(handler, this);
    } else {
        return std::invoke(s_OriginalCommandHandlerTable[(size_t)op.Command / 100], this, (eScriptCommands)(op.Command));
    }
}


// 0x469F00
OpcodeResult CRunningScript::Process() {
    if (m_SceneSkipIP && CCutsceneMgr::IsCutsceneSkipButtonBeingPressed()) {
        CHud::m_BigMessage[1][0] = 0;
        UpdatePC(std::exchange(m_SceneSkipIP, 0));
        m_WakeTime = 0;
    }

    if (m_UsesMissionCleanup) {
        DoDeathArrestCheck();
    }

    if (m_ThisMustBeTheOnlyMissionRunning && CTheScripts::FailCurrentMission == 1) {
        if (m_StackDepth > 0) {
            ResetIP();
        }
    }

    CTheScripts::ReinitialiseSwitchStatementData();

    if (CTimer::GetTimeInMS() >= (uint32)m_WakeTime) {
        while (ProcessOneCommand() == OR_CONTINUE)
            ; // Process commands
    }

    return OR_CONTINUE;
}



notsa::script::CommandHandlerFunction& CRunningScript::CustomCommandHandlerOf(eScriptCommands command) {
    return s_CustomCommandHandlerTable[(size_t)(command)];
}



void CRunningScript::ResetIP() {
    assert(m_StackDepth > 0); // Original bug...
    do {
        m_IP = std::exchange(m_IPStack[m_StackDepth--], nullptr); // NOTSA: Also clear the stack, we don't need it anymore
    } while (m_StackDepth);
}


// 0x468D50
void CTheScripts::Init() {
    rng::fill(ScriptSpace, 0u);
    rng::fill(LocalVariablesForCurrentMission, tScriptParam{});

    CRunningScript* nextScript = nullptr;
    for (auto& script : ScriptsArray) {
        script.Init();
        script.m_pPrev = nullptr;
        script.m_pNext = nextScript;

        if (nextScript) {
            nextScript->m_pPrev = &script;
        }
        nextScript = &script;
    }
    pActiveScripts = nullptr;
    pIdleScripts   = nextScript;

    MissionCleanUp.Init();
    UpsideDownCars.Init();
    StuckCars.Init();
    ScriptsForBrains.Init();
    ScriptResourceManager.Initialise();
    rng::for_each(EntitiesWaitingForScriptBrain, &tScriptBrainWaitEntity::Clear);

    if (CGame::bMissionPackGame) {
        char scrFile[MAX_PATH]{};

        while (FrontEndMenuManager.CheckMissionPackValidMenu()) {
            CFileMgr::SetDirMyDocuments();
            notsa::format_to_sz(scrFile, "MPACK//MPACK{:d}//SCR.SCM", CGame::bMissionPackGame);

            if (auto f = notsa::File(scrFile, "rb"); f && f.Read(ScriptSpace.data(), MAIN_SCRIPT_SIZE) >= 1) {
                TheText.Load(false);
                break;
            }
        }
    } else {
        CFileMgr::SetDir("data\\script");

        VERIFY(notsa::File("main.scm", "rb").Read(ScriptSpace.data(), MAIN_SCRIPT_SIZE) >= 1);
    }
    CFileMgr::SetDir("");

    StoreVehicleIndex     = -1;
    StoreVehicleWasRandom = true;
    OnAMissionFlag        = false;
    LastMissionPassedTime = -1;
    LastRandomPedId       = -1;

    rng::fill(UsedObjectArray, tUsedObject{});
    ReadObjectNamesFromScript();
    UpdateObjectIndices();
    rng::fill(MultiScriptArray, 0u);
    NumberOfUsedObjects                        = 0;
    bAlreadyRunningAMissionScript              = false;
    bUsingAMultiScriptFile                     = true;
    MainScriptSize                             = 0;
    LargestMissionScriptSize                   = 0;
    NumberOfMissionScripts                     = 0;
    NumberOfExclusiveMissionScripts            = 0;
    LargestNumberOfMissionScriptLocalVariables = 0;

    ReadMultiScriptFileOffsetsFromScript();
    if (!CGame::bMissionPackGame) {
        StreamedScripts.ReadStreamedScriptData();
    }

    ForceRandomCarModel                    = -1;
    FailCurrentMission                     = 0;
    ScriptPickupCycleIndex                 = 0;
    bMiniGameInProgress                    = false;
    bDisplayNonMiniGameHelpMessages        = true;
    bPlayerHasMetDebbieHarry               = false;
    RiotIntensity                          = 0;
    bPlayerIsOffTheMap                     = false;
    RadarZoomValue                         = 0;
    RadarShowBlipOnAllLevels               = false;
    HideAllFrontEndMapBlips                = false;
    bDisplayHud                            = true;
    fCameraHeadingWhenPlayerIsAttached     = 0.0f;
    fCameraHeadingStepWhenPlayerIsAttached = 0.0f;
    bEnableCraneRaise                      = true;
    bEnableCraneLower                      = true;
    bEnableCraneRelease                    = true;
    bDrawCrossHair                         = eCrossHairType::NONE;
    gAllowScriptedFixedCameraCollision     = false;
    bAddNextMessageToPreviousBriefs        = true;
    bScriptHasFadedOut                     = false;
    bDrawOddJobTitleBeforeFade             = true;
    bDrawSubtitlesBeforeFade               = true;

    rng::fill(ScriptSphereArray, tScriptSphere{});
    rng::fill(IntroTextLines, tScriptText{});

    NumberOfIntroTextLinesThisFrame = 0;
    UseTextCommands                 = eUseTextCommandState::DISABLED;
    bUseMessageFormatting           = false;
    MessageCentre                   = 0;
    MessageWidth                    = 0;

    rng::fill(IntroRectangles, tScriptRectangle{});
    NumberOfIntroRectanglesThisFrame = 0;

    RemoveScriptTextureDictionary();

    rng::fill(BuildingSwapArray, tBuildingSwap{});
    rng::fill(InvisibilitySettingArray, nullptr);
    ClearAllSuppressedCarModels();
    ClearAllVehicleModelsBlockedByScript();
    InitialiseAllConnectLodObjects();
    InitialiseSpecialAnimGroupsAttachedToCharModels();
    ScriptEffectSystemArray.fill(tScriptEffectSystem{});

    ScriptSearchLightArray.fill(tScriptSearchlight{});
    NumberOfScriptSearchLights = 0;

    ScriptCheckpointArray.fill(tScriptCheckpoint{});
    NumberOfScriptCheckpoints = 0;

    ScriptSequenceTaskArray.fill(tScriptSequence{});

    CScripted2dEffects::Init();
    CTaskSequences::Init();
    CPedGroups::Init();
    CInformFriendsEventQueue::Init();
    CInformGroupEventQueue::Init();
    CDecisionMakerTypes::GetInstance();

}


// 0x464FF0
bool CTheScripts::CheckStreamedScriptVersion(RwStream* stream, const char* filename) {
    return true;
}


// 0x5D3390
void CTheScripts::DoScriptSetupAfterPoolsHaveLoaded() {
    for (const auto& lod : ScriptConnectLodsObjects) {
        if (lod.a != -1 && lod.b != -1) {
            ScriptConnectLodsFunction(lod.a, lod.b);
        }
    }
}


// 0x464D20
int32 CTheScripts::GetScriptIndexFromPointer(CRunningScript* thread) {
    assert(ScriptsArray.data() <= thread && thread < ScriptsArray.data() + ScriptsArray.size());
    return std::distance(ScriptsArray.data(), thread);
}


// 0x465A40
void CTheScripts::RemoveScriptTextureDictionary() {
    rng::for_each(ScriptSprites, &CSprite2d::Delete);
    if (const auto slot = CTxdStore::FindTxdSlot("script"); slot != -1) {
        if (const auto* txd = CTxdStore::ms_pTxdPool->GetAt(slot); txd) {
            CTxdStore::RemoveTxd(slot);
        }
    }
}


// 0x464C20
CRunningScript* CTheScripts::StartNewScript(uint8* startIP) {
    CRunningScript* script = pIdleScripts;

    script->RemoveScriptFromList(&pIdleScripts);
    script->Init();
    script->SetCurrentIp(startIP);
    script->AddScriptToList(&pActiveScripts);
    script->SetActive(true);

    return script;
}


// 0x464C90
CRunningScript* CTheScripts::StartNewScript(uint8* startIP, uint16 index) {
    if (!pIdleScripts) {
        return nullptr;
    }

    auto* script = pIdleScripts;
    while (script && script != &ScriptsArray[index]) {
        script = script->m_pNext;
    }

    if (!script) {
        return nullptr;
    }

    script->RemoveScriptFromList(&pIdleScripts);
    script->Init();
    script->SetCurrentIp(startIP);
    script->AddScriptToList(&pActiveScripts);
    script->SetActive(true);

    return script;
}


// 0x464D50
bool CTheScripts::IsPlayerOnAMission() {
    return OnAMissionFlag && notsa::ReadAs<uint32_t>(&ScriptSpace[OnAMissionFlag]) == 1;
}


// 0x5D4FD0
void CTheScripts::Load() {
    Init();

    const auto totalSize = CGenericGameStorage::LoadDataFromWorkBuffer<uint32>();
    auto       p         = ScriptSpace.data();

    // Load chunks
    auto nParts = totalSize / MAX_SAVED_GVAR_PART_SIZE;
    for (nParts; nParts-- > 0; p += MAX_SAVED_GVAR_PART_SIZE) {
        CGenericGameStorage::LoadDataFromWorkBuffer(p, MAX_SAVED_GVAR_PART_SIZE);
    }

    // Load remainder
    const auto remainder = totalSize % MAX_SAVED_GVAR_PART_SIZE;
    CGenericGameStorage::LoadDataFromWorkBuffer(p, remainder);

    for (auto& sfb : ScriptsForBrains.ScriptBrainArray) {
        CGenericGameStorage::LoadDataFromWorkBuffer(sfb);
    }

    CGenericGameStorage::LoadDataFromWorkBuffer(OnAMissionFlag);
    CGenericGameStorage::LoadDataFromWorkBuffer(LastMissionPassedTime);

    for (auto& bswap : BuildingSwapArray) {
        const auto type    = CGenericGameStorage::LoadDataFromWorkBuffer<ScriptSavedObjectType>();
        const auto poolRef = CGenericGameStorage::LoadDataFromWorkBuffer<uint32>() - 1;
        CGenericGameStorage::LoadDataFromWorkBuffer(bswap.m_nNewModelIndex);
        CGenericGameStorage::LoadDataFromWorkBuffer(bswap.m_nOldModelIndex);

        bswap.m_pCBuilding = nullptr;
        switch (type) {
        case ScriptSavedObjectType::NONE:
        case ScriptSavedObjectType::INVISIBLE:
            break;
        case ScriptSavedObjectType::BUILDING:
            bswap.m_pCBuilding = GetBuildingPool()->GetAt(poolRef);
            break;
        default:
            NOTSA_UNREACHABLE();
        }

        if (auto* b = bswap.m_pCBuilding) {
            CWorld::Remove(b);
            b->ReplaceWithNewModel(bswap.m_nNewModelIndex);
            CWorld::Add(b);
        }
    }

    for (auto& is : InvisibilitySettingArray) {
        const auto type    = CGenericGameStorage::LoadDataFromWorkBuffer<ScriptSavedObjectType>();
        const auto poolRef = CGenericGameStorage::LoadDataFromWorkBuffer<uint32>() - 1;

        switch (type) {
        case ScriptSavedObjectType::NONE:
            is = nullptr;
            break;
        case ScriptSavedObjectType::INVISIBLE:
            // Not saved by the game, but the logic is still there
            if (is) {
                is->SetUsesCollision(false);
                is->SetIsVisible(false);
            }
            break;
        case ScriptSavedObjectType::BUILDING:
            is = nullptr;
            if (auto* obj = GetBuildingPool()->GetAt(poolRef)) {
                is                   = obj;
                is->SetUsesCollision(false);
                is->SetIsVisible(false);
            }
            break;
        case ScriptSavedObjectType::OBJECT:
            is = nullptr;
            if (auto* obj = GetObjectPool()->GetAt(poolRef)) {
                is                   = obj;
                is->SetUsesCollision(false);
                is->SetIsVisible(false);
            }
            break;
        case ScriptSavedObjectType::DUMMY:
            is = nullptr;
            if (auto* obj = GetDummyPool()->GetAt(poolRef)) {
                is                   = obj;
                is->SetUsesCollision(false);
                is->SetIsVisible(false);
            }
            break;
        default:
            NOTSA_UNREACHABLE();
        }
    }

    for (auto& veh : VehicleModelsBlockedByScript) {
        CGenericGameStorage::LoadDataFromWorkBuffer(veh);
    }

    for (auto& lod : ScriptConnectLodsObjects) {
        CGenericGameStorage::LoadDataFromWorkBuffer(lod);
    }

    for (auto& ag : ScriptAttachedAnimGroups) {
        CGenericGameStorage::LoadDataFromWorkBuffer(ag);

        if (ag.m_nModelID != MODEL_INVALID) {
            ScriptAttachAnimGroupToCharModel(ag.m_nModelID, ag.m_IfpName);
        }
    }

    CGenericGameStorage::LoadDataFromWorkBuffer(bUsingAMultiScriptFile);
    CGenericGameStorage::LoadDataFromWorkBuffer(bPlayerHasMetDebbieHarry);

    {
        // Ignored
        CGenericGameStorage::LoadDataFromWorkBuffer<uint32>(); // MainScriptSize
        CGenericGameStorage::LoadDataFromWorkBuffer<uint32>(); // LargestMissionScriptSize
        CGenericGameStorage::LoadDataFromWorkBuffer<uint16>(); // NumberOfMissionScripts
        CGenericGameStorage::LoadDataFromWorkBuffer<uint16>(); // NumberOfExclusiveMissionScripts
        CGenericGameStorage::LoadDataFromWorkBuffer<uint32>(); // LargestNumberOfMissionScriptLocalVariables
    }

    // Unused
    // auto j = 0u;
    // for (auto* s = pActiveScripts; s; s->m_pNext)
    //     j++;

    auto numScripts = CGenericGameStorage::LoadDataFromWorkBuffer<uint32>();
    for (auto i = 0u; i < numScripts; i++) {
        auto* script = StartNewScript(nullptr, CGenericGameStorage::LoadDataFromWorkBuffer<uint16>());
        {
            const auto prev = script->m_pPrev, next = script->m_pNext;
            CGenericGameStorage::LoadDataFromWorkBuffer(*script);
            script->m_pPrev = prev;
            script->m_pNext = next;
        }
        script->SetCurrentIp(&ScriptSpace[CGenericGameStorage::LoadDataFromWorkBuffer<uint32>()]);

        for (auto& stk : script->m_IPStack) {
            if (const auto ip = CGenericGameStorage::LoadDataFromWorkBuffer<uint32>()) {
                stk = &ScriptSpace[ip];
            } else {
                stk = nullptr;
            }
        }
    }
}


// 0x5D4C40
void CTheScripts::Save() {
    const auto totalSize = GetSCMChunk<tSCMGlobalVarChunk>()->m_NextChunkOffset;
    auto       p         = ScriptSpace.data();
    CGenericGameStorage::SaveDataToWorkBuffer(totalSize);

    // Load chunks
    auto nParts = totalSize / MAX_SAVED_GVAR_PART_SIZE;
    for (nParts; nParts-- > 0; p += MAX_SAVED_GVAR_PART_SIZE) {
        CGenericGameStorage::SaveDataToWorkBuffer(p, MAX_SAVED_GVAR_PART_SIZE);
    }

    // Load remainder
    const auto remainder = totalSize % MAX_SAVED_GVAR_PART_SIZE;
    CGenericGameStorage::SaveDataToWorkBuffer(p, remainder);


    for (auto& sfb : ScriptsForBrains.ScriptBrainArray) {
        CGenericGameStorage::SaveDataToWorkBuffer(sfb);
    }

    CGenericGameStorage::SaveDataToWorkBuffer(OnAMissionFlag);
    CGenericGameStorage::SaveDataToWorkBuffer(LastMissionPassedTime);

    for (auto& bswap : BuildingSwapArray) {
        if (auto* b = bswap.m_pCBuilding) {
            CGenericGameStorage::SaveDataToWorkBuffer(ScriptSavedObjectType::BUILDING);

            // Add 1 to the index because 0 is considered invalid and index can be 0.
            CGenericGameStorage::SaveDataToWorkBuffer(GetBuildingPool()->GetIndex(b) + 1);
        } else {
            CGenericGameStorage::SaveDataToWorkBuffer(ScriptSavedObjectType::NONE);
            CGenericGameStorage::SaveDataToWorkBuffer(0);
        }

        CGenericGameStorage::SaveDataToWorkBuffer(bswap.m_nNewModelIndex);
        CGenericGameStorage::SaveDataToWorkBuffer(bswap.m_nOldModelIndex);
    }

    for (auto& is : InvisibilitySettingArray) {
        if (!is) {
            CGenericGameStorage::SaveDataToWorkBuffer(ScriptSavedObjectType::NONE);
            CGenericGameStorage::SaveDataToWorkBuffer(0);
            continue;
        }

        switch (is->GetType()) {
        case ENTITY_TYPE_NOTHING:
            CGenericGameStorage::SaveDataToWorkBuffer(ScriptSavedObjectType::NONE);
            CGenericGameStorage::SaveDataToWorkBuffer(0);
            break;
        case ENTITY_TYPE_BUILDING:
            CGenericGameStorage::SaveDataToWorkBuffer(ScriptSavedObjectType::BUILDING);
            CGenericGameStorage::SaveDataToWorkBuffer(GetBuildingPool()->GetIndex(is->AsBuilding()) + 1);
            break;
        case ENTITY_TYPE_OBJECT:
            CGenericGameStorage::SaveDataToWorkBuffer(ScriptSavedObjectType::OBJECT);
            CGenericGameStorage::SaveDataToWorkBuffer(GetObjectPool()->GetIndex(is->AsObject()) + 1);
            break;
        case ENTITY_TYPE_DUMMY:
            CGenericGameStorage::SaveDataToWorkBuffer(ScriptSavedObjectType::DUMMY);
            CGenericGameStorage::SaveDataToWorkBuffer(GetDummyPool()->GetIndex(is->AsDummy()) + 1);
            break;
        default:
            NOTSA_UNREACHABLE();
        }
    }

    for (auto& veh : VehicleModelsBlockedByScript) {
        CGenericGameStorage::SaveDataToWorkBuffer(veh);
    }

    for (auto& lod : ScriptConnectLodsObjects) {
        CGenericGameStorage::SaveDataToWorkBuffer(lod);
    }

    for (auto& ag : ScriptAttachedAnimGroups) {
        CGenericGameStorage::SaveDataToWorkBuffer(ag);
    }

    CGenericGameStorage::SaveDataToWorkBuffer(bUsingAMultiScriptFile);
    CGenericGameStorage::SaveDataToWorkBuffer(bPlayerHasMetDebbieHarry); // ?

    CGenericGameStorage::SaveDataToWorkBuffer(MainScriptSize);
    CGenericGameStorage::SaveDataToWorkBuffer(LargestMissionScriptSize);
    CGenericGameStorage::SaveDataToWorkBuffer(NumberOfMissionScripts);
    CGenericGameStorage::SaveDataToWorkBuffer(NumberOfExclusiveMissionScripts);
    CGenericGameStorage::SaveDataToWorkBuffer(LargestNumberOfMissionScriptLocalVariables);

    auto numNonExternalScripts = 0u;
    auto* lastScript           = pActiveScripts;
    for (auto* s = pActiveScripts; s; s = s->m_pNext) {
        lastScript = s;
        if (!s->m_IsExternal && s->m_ExternalType == -1) {
            numNonExternalScripts++;
        }
    }
    CGenericGameStorage::SaveDataToWorkBuffer(numNonExternalScripts);

    for (auto* s = lastScript; s; s = s->m_pPrev) {
        if (s->m_IsExternal || s->m_ExternalType != -1) {
            continue;
        }

        CGenericGameStorage::SaveDataToWorkBuffer((int16)GetScriptIndexFromPointer(s));
        CGenericGameStorage::SaveDataToWorkBuffer(*s);
        CGenericGameStorage::SaveDataToWorkBuffer((uint32)(s->m_IP - ScriptSpace.data()));

        for (auto& stk : s->m_IPStack) {
            CGenericGameStorage::SaveDataToWorkBuffer((uint32)(stk ? stk - ScriptSpace.data() : 0));
        }
    }
}


// 0x464BB0
void CTheScripts::WipeLocalVariableMemoryForMissionScript() {
    rng::fill(LocalVariablesForCurrentMission, tScriptParam{});
}


// 0x464D40
void CTheScripts::StartTestScript() {
    ZoneScoped;

    StartNewScript(MainSCMBlock.data());
}


// 0x46A000
void CTheScripts::Process() {
    ZoneScoped;

    if (CReplay::Mode == MODE_PLAYBACK) {
        return;
    }

    CommandsExecuted = 0;

    UpsideDownCars.UpdateTimers();
    StuckCars.Process();
    MissionCleanUp.CheckIfCollisionHasLoadedForMissionObjects();
    DrawScriptSpheres();
    ProcessAllSearchLights();
    ProcessWaitingForScriptBrainArray();

    if (FailCurrentMission) {
        --FailCurrentMission;
    }

    if (UseTextCommands != eUseTextCommandState::DISABLED) {
        rng::fill(IntroTextLines, tScriptText{});
        NumberOfIntroTextLinesThisFrame = 0;

        rng::fill(IntroRectangles, tScriptRectangle{});
        NumberOfIntroRectanglesThisFrame = 0;

        if (UseTextCommands == eUseTextCommandState::DISABLE_NEXT_FRAME) {
            UseTextCommands = eUseTextCommandState::DISABLED;
        }
    } else { // Check these, just in case something modifies them: If so, you can place a 'Break when data changes' breakpoint on the variable
        assert(NumberOfIntroTextLinesThisFrame == 0);
        assert(NumberOfIntroRectanglesThisFrame == 0);
    }

    const auto timeStepMS = (int32)CTimer::GetTimeStepInMS();
    LocalVariablesForCurrentMission[SCRIPT_VAR_TIMERA].iParam += timeStepMS;
    LocalVariablesForCurrentMission[SCRIPT_VAR_TIMERB].iParam += timeStepMS;

    CLoadingScreen::NewChunkLoaded();

    for (CRunningScript* it = pActiveScripts, *next{}; it; it = next) {
        next = it->m_pNext;

        it->m_LocalVars[SCRIPT_VAR_TIMERA].iParam += timeStepMS;
        it->m_LocalVars[SCRIPT_VAR_TIMERB].iParam += timeStepMS;
        it->Process();
    }

    CLoadingScreen::NewChunkLoaded();

    for (auto& ped : GetPedPool()->GetAllValid()) {
        if (ped.IsCreatedByMission()) {
            ped.GetIntelligence()->RecordEventForScript(0, 0);
        }
    }
}


// 0x4646D0
void CTheScripts::PrintListSizes() {
    auto active{ 0u }, idle{ 0u };

    for (const auto* s = pActiveScripts; s; s = s->m_pNext) active++;
    for (const auto* s = pIdleScripts; s; s = s->m_pNext) idle++;

    NOTSA_LOG_DEBUG("Scripts Active: {}, Idle: {}", active, idle);
}


// 0x464980
void CTheScripts::DrawScriptSpritesAndRectangles(bool drawBeforeFade) {
    for (const auto& rt : IntroRectangles) {
        if (rt.m_bDrawBeforeFade != drawBeforeFade) {
            continue;
        }
        switch (rt.m_nType) {
        case eScriptRectangleType::INACTIVE:
            break;
        case eScriptRectangleType::TITLE_AND_MESSAGE:
            FrontEndMenuManager.DrawWindowedText(
                rt.cornerA.x, rt.cornerA.y,
                rt.cornerB.x, // ?
                rt.gxt1,
                rt.gxt2,
                rt.m_Alignment
            );
            break;
        case eScriptRectangleType::TEXT:
            FrontEndMenuManager.DrawWindow(
                CRect{ rt.cornerA, rt.cornerB },
                rt.gxt1,
                0,
                CRGBA{ 0, 0, 0, 190 },
                rt.m_nTextboxStyle, // ?
                true
            );
            break;
        case eScriptRectangleType::MONOCOLOR:
            CSprite2d::DrawRect(
                CRect{ rt.cornerA, rt.cornerB },
                rt.m_nTransparentColor
            );
            break;
        case eScriptRectangleType::TEXTURED:
            ScriptSprites[rt.m_nTextureId].Draw(
                CRect{ rt.cornerA, rt.cornerB },
                rt.m_nTransparentColor
            );
            break;
        case eScriptRectangleType::MONOCOLOR_ANGLED: {
            // mid: Vector that points to the middle of line A-B from A.
            // vAM: A to mid.
            const auto mid = (rt.cornerA + rt.cornerB) / 2.0f;
            const auto vAM = mid - rt.cornerA;
            const auto cos = std::cos(rt.m_nAngle);
            const auto sin = std::sin(rt.m_nAngle);

            // This is 2D rotation, couldn't find a better function aside from
            // using matricies or quaternions.
            ScriptSprites[rt.m_nTextureId].Draw(
                -cos * vAM.x + sin * vAM.y + mid.x,
                -sin * vAM.x - cos * vAM.y + mid.y,
                +sin * vAM.y + cos * vAM.x + mid.x,
                +sin * vAM.x - cos * vAM.y + mid.y,
                -cos * vAM.x - sin * vAM.y + mid.x,
                +cos * vAM.y - sin * vAM.x + mid.y,
                +cos * vAM.x - sin * vAM.y + mid.x,
                +sin * vAM.x + cos * vAM.y + mid.y,
                rt.m_nTransparentColor
            );
            break;
        }
        default:
            NOTSA_UNREACHABLE("Unknown script-rect type ({})", (int32)(rt.m_nType));
        }
    }
}
