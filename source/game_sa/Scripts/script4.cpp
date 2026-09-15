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



/*!
 * @param type always SCRIPT_THING_PED_GROUP
 * @addr 0x4810C0
 */
int32 CTheScripts::GetUniqueScriptThingIndex(int32 playerGroup, eScriptThingType type) {
    switch (type) {
    /* Android
    case SCRIPT_THING_DECISION_MAKER:
        CDecisionMakerTypes::GetInstance();
        return playerGroup | (CDecisionMakerTypes::ScriptReferenceIndex[playerGroup] << 16);
    */
    case SCRIPT_THING_PED_GROUP:
        return playerGroup | (CPedGroups::ScriptReferenceIndex[playerGroup] << 16);
    default:
        return OR_INTERRUPT;
    }
}


// 0x483720
// TODO: TEST REFACTOR!
int32 CTheScripts::GetNewUniqueScriptThingIndex(int32 index, eScriptThingType type) {
    const auto NewUniqueId = [index](auto& id) -> int32 {
        if (id == -1 || id == -2) {
            id = 1;
        } else {
            ++id;
        }

        return (uint32)id << 16 | index;
    };

    switch (type) {
    case eScriptThingType::SCRIPT_THING_SPHERE:
        return NewUniqueId(ScriptSphereArray[index].m_nUniqueId);
    case eScriptThingType::SCRIPT_THING_EFFECT_SYSTEM:
        return NewUniqueId(ScriptEffectSystemArray[index].m_nId);
    case eScriptThingType::SCRIPT_THING_SEARCH_LIGHT:
        return NewUniqueId(ScriptSearchLightArray[index].m_nId);
    case eScriptThingType::SCRIPT_THING_CHECKPOINT:
        return NewUniqueId(ScriptCheckpointArray[index].m_nId);
    case eScriptThingType::SCRIPT_THING_SEQUENCE_TASK:
        ScriptSequenceTaskArray[index].m_bUsed = true;
        return NewUniqueId(ScriptSequenceTaskArray[index].m_nId);
    case eScriptThingType::SCRIPT_THING_FIRE:
        return NewUniqueId(gFireManager.m_aFires[index].GetId());
    case eScriptThingType::SCRIPT_THING_2D_EFFECT:
        return NewUniqueId(CScripted2dEffects::ScriptReferenceIndex[index]);
    case eScriptThingType::SCRIPT_THING_DECISION_MAKER:
        CDecisionMakerTypes::GetInstance(); // ? TODO check if we really need this
        return NewUniqueId(CDecisionMakerTypes::ScriptReferenceIndex[index]);
    case eScriptThingType::SCRIPT_THING_PED_GROUP:
        return NewUniqueId(CPedGroups::ScriptReferenceIndex[index]);
    default:
        break;
    }

    return -1;
}


// 0x4839A0
int32 CTheScripts::GetActualScriptThingIndex(int32 ref, eScriptThingType type) {
    if (ref == -1) {
        return -1;
    }

    const auto idx = LOWORD(ref), id = HIWORD(ref);

    switch (type) {
    case SCRIPT_THING_SPHERE:
        if (const auto& s = ScriptSphereArray[idx]; s.IsActive() && s.m_nUniqueId == id) {
            return idx;
        }
        break;
    case SCRIPT_THING_EFFECT_SYSTEM:
        if (const auto& fx = ScriptEffectSystemArray[idx]; fx.IsActive() && fx.m_nId == id) {
            return idx;
        }
        break;
    case SCRIPT_THING_SEARCH_LIGHT:
        if (const auto& sl = ScriptSearchLightArray[idx]; sl.IsActive() && sl.m_nId == id) {
            return idx;
        }
        break;
    case SCRIPT_THING_CHECKPOINT:
        if (const auto& cp = ScriptCheckpointArray[idx]; cp.IsActive() && cp.m_nId == id) {
            return idx;
        }
        break;
    case SCRIPT_THING_SEQUENCE_TASK:
        if (const auto& sqt = ScriptSequenceTaskArray[idx]; sqt.IsActive() && sqt.m_nId == id) {
            return idx;
        }
        break;
    case SCRIPT_THING_FIRE:
        if (const auto& f = gFireManager.Get(idx); f.IsScript() && f.GetId() == id) {
            return idx;
        }
        break;
    case SCRIPT_THING_2D_EFFECT:
        if (CScripted2dEffects::ms_activated[idx] && CScripted2dEffects::ScriptReferenceIndex[idx] == id) {
            return idx;
        }
        break;
    case SCRIPT_THING_DECISION_MAKER:
        CDecisionMakerTypes::GetInstance();
        if (CDecisionMakerTypes::m_IsActive[idx] && CDecisionMakerTypes::ScriptReferenceIndex[idx] == id) {
            return idx;
        }
        break;
    case SCRIPT_THING_PED_GROUP:
        if (CPedGroups::ms_activeGroups[idx] && CPedGroups::ScriptReferenceIndex[idx] == id) {
            return idx;
        }
        break;
    default:
        break;
    }
    return -1;
}


// 0x4810E0
void CTheScripts::DrawScriptSpheres() {
    ZoneScoped;

    for (auto& ss : ScriptSphereArray) {
        if (!ss.m_bUsed) {
            continue;
        }

        C3dMarkers::PlaceMarkerSet(
            ss.m_nId,
            e3dMarkerType::MARKER3D_CYLINDER,
            ss.m_vCoords,
            ss.m_fRadius,
            255,
            0,
            0,
            228,
            2048,
            0.1f,
            0
        );
    }
}


// 0x483B30
uint32 CTheScripts::AddScriptSphere(uint32 id, CVector posn, float radius) {
    const auto sphere = rng::find_if(ScriptSphereArray, [](auto& sphere) {
        return !sphere.IsActive();
    });
    assert(sphere != ScriptSphereArray.end()); // In vanilla game does OOB access.

    const auto idx = std::distance(ScriptSphereArray.begin(), sphere);
    sphere->m_nId     = idx + id;
    sphere->m_vCoords = posn;
    sphere->m_bUsed   = true;
    sphere->m_fRadius = radius;
    return GetNewUniqueScriptThingIndex(idx, SCRIPT_THING_EFFECT_SYSTEM);
}


// 0x483BA0
void CTheScripts::RemoveScriptSphere(int32 scriptIndex) {
    const auto i = GetActualScriptThingIndex(scriptIndex, eScriptThingType::SCRIPT_THING_SPHERE);
    if (i == -1) {
        return;
    }

    auto& ss = ScriptSphereArray[i];
    ss.m_bUsed = false;
    ss.m_nId   = 0;
}


// 0x481140
void CTheScripts::AddToBuildingSwapArray(CBuilding* building, int32 oldModelId, int32 newModelId) {
    if (building->GetIplIndex())
        return;

    for (auto& swap : BuildingSwapArray) {
        if (swap.m_pCBuilding == building) {
            if (newModelId == swap.m_nOldModelIndex) {
                swap.Clear();
            } else
                swap.m_nNewModelIndex = newModelId;

            return;
        }
    }

    for (auto& swap : BuildingSwapArray) {
        if (!swap.m_pCBuilding) {
            swap = tBuildingSwap(building, newModelId, oldModelId);
            return;
        }
    }
}


// 0x481200
void CTheScripts::AddToInvisibilitySwapArray(CEntity* entity, bool visible) {
    if (entity->GetIplIndex())
        return;

    const auto is = rng::find(InvisibilitySettingArray, entity);
    if (is != InvisibilitySettingArray.end()) {
        // Already exists.
        if (visible) {
            *is = nullptr;
        }
        return;
    }

    const auto free = rng::find(InvisibilitySettingArray, nullptr);
    if (free == InvisibilitySettingArray.end()) {
        return;
    }

    *free = entity;
}


// 0x481290
void CTheScripts::UndoBuildingSwaps() {
    for (auto& swap : BuildingSwapArray) {
        if (swap.m_pCBuilding) {
            swap.m_pCBuilding->ReplaceWithNewModel(swap.m_nOldModelIndex);
            swap.Clear();
        }
    }
}


// 0x4812D0
void CTheScripts::UndoEntityInvisibilitySettings() {
    for (auto& is : InvisibilitySettingArray) {
        if (!is) {
            continue;
        }

        is->SetIsVisible(true);
        is->SetUsesCollision(true);

        is = nullptr; // Remove from the array.
    }
}

void InjectHooks_Script4() {
    RH_ScopedClass(CTheScripts);
    RH_ScopedCategory("Scripts");

    RH_ScopedInstall(GetActualScriptThingIndex, 0x4839A0);
    RH_ScopedInstall(GetNewUniqueScriptThingIndex, 0x483720);
    RH_ScopedInstall(GetUniqueScriptThingIndex, 0x4810C0);
    RH_ScopedInstall(DrawScriptSpheres, 0x4810E0);
    RH_ScopedInstall(AddScriptSphere, 0x483B30);
    RH_ScopedInstall(RemoveScriptSphere, 0x483BA0);
    RH_ScopedInstall(AddToBuildingSwapArray, 0x481140);
    RH_ScopedInstall(AddToInvisibilitySwapArray, 0x481200);
    RH_ScopedInstall(UndoBuildingSwaps, 0x481290);
    RH_ScopedInstall(UndoEntityInvisibilitySettings, 0x4812D0);
}
