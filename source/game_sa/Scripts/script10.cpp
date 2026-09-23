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



// 0x46AF50
void CRunningScript::ScriptTaskPickUpObject(int32 commandId) {
    plugin::CallMethod<0x46AF50, CRunningScript*, int32>(this, commandId);
}


// 0x46A7C0
void CTheScripts::ClearAllSuppressedCarModels() {
    rng::fill(SuppressedVehicleModels, MODEL_INVALID);
}

// 0x46A7E0
void CTheScripts::RemoveFromSuppressedCarModelArray(int32 modelIndex) {
    for (auto& model : SuppressedVehicleModels) {
        if (model == static_cast<eModelID>(modelIndex)) {
            model = MODEL_INVALID;
        }
    }
}

// 0x46A810
bool CTheScripts::HasCarModelBeenSuppressed(eModelID carModelId) {
    return notsa::contains(SuppressedVehicleModels, carModelId);
}

// 0x46B1A0
void CTheScripts::AddToSuppressedCarModelArray(int32 modelIndex) {
    if (HasCarModelBeenSuppressed(static_cast<eModelID>(modelIndex))) {
        return;
    }
    const auto free = rng::find(SuppressedVehicleModels, MODEL_INVALID);
    if (free != SuppressedVehicleModels.end()) {
        *free = static_cast<eModelID>(modelIndex);
    }
}


// 0x46A840
void CTheScripts::ClearAllVehicleModelsBlockedByScript() {
    rng::fill(VehicleModelsBlockedByScript, MODEL_INVALID);
}


// 0x46B200, unused | inlined?
void CTheScripts::AddToVehicleModelsBlockedByScript(eModelID modelIndex) {
    if (notsa::contains(VehicleModelsBlockedByScript, modelIndex)) {
        return;
    }

    const auto free = rng::find(VehicleModelsBlockedByScript, MODEL_INVALID);
    assert(free != VehicleModelsBlockedByScript.end()); // In vanilla game does OOB access.

    *free = modelIndex;
}


// unused
// 0x?
void CTheScripts::RemoveFromVehicleModelsBlockedByScript(int32 modelIndex) {
    for (auto& model : VehicleModelsBlockedByScript) {
        if (model == modelIndex) {
            model = MODEL_INVALID;
        }
    }
}


// 0x46A890
bool CTheScripts::HasVehicleModelBeenBlockedByScript(eModelID carModelId) {
    return notsa::contains(VehicleModelsBlockedByScript, carModelId);
}


// 0x46AB60
void CTheScripts::AddToWaitingForScriptBrainArray(CEntity* entity, int16 specialModelIndex) {
    const auto wfsb = rng::find_if(EntitiesWaitingForScriptBrain, [entity](auto& wfsb) {
        return wfsb.m_pEntity == entity;
    });
    if (wfsb != EntitiesWaitingForScriptBrain.end()) {
        // Already exists.
        return;
    }

    const auto free = rng::find_if(EntitiesWaitingForScriptBrain, [entity](auto& wfsb) { return !wfsb.m_pEntity; });
    if (free == EntitiesWaitingForScriptBrain.end()) {
        return;
    }

    free->m_pEntity = entity;
    free->m_ScriptBrainIndex = specialModelIndex;
}


// 0x46ABC0
void CTheScripts::RemoveFromWaitingForScriptBrainArray(CEntity* entity, int16 modelIndex) {
    for (auto& bwe : EntitiesWaitingForScriptBrain) {
        if (bwe.m_pEntity != entity || bwe.m_ScriptBrainIndex != modelIndex) {
            continue;
        }

        bwe.m_pEntity = nullptr;
        bwe.m_ScriptBrainIndex = MODEL_INVALID;
    }
}


// 0x46CF00
void CTheScripts::ProcessWaitingForScriptBrainArray() {
    ZoneScoped;

    if (!FindPlayerPed())
        return;

    for (auto& e : EntitiesWaitingForScriptBrain) {
        if (!e.m_pEntity) {
            continue;
        }

        switch (const auto t = ScriptsForBrains.m_aScriptForBrains[e.m_ScriptBrainIndex].m_TypeOfBrain) {
        case 0: // TODO: enum
        case 3: // for peds?
        {
            auto*      ped = e.m_pEntity->AsPed();
            const auto idx = ScriptsForBrains.m_aScriptForBrains[ped->m_StreamedScriptBrainToLoad].m_StreamedScriptIndex;

            if (CStreaming::IsModelLoaded(SCMToModelId(idx))) {
                ScriptsForBrains.StartNewStreamedScriptBrain(
                    static_cast<uint8>(ped->m_StreamedScriptBrainToLoad), // cast?
                    ped,
                    false
                );
            } else {
                CStreaming::RequestModel(SCMToModelId(idx), STREAMING_MISSION_REQUIRED);
            }
            break;
        }
        case 1:
        case 4: // for objects?
        {
            auto* obj = e.m_pEntity->AsObject();

            switch (obj->objectFlags.b0x100000_0x200000) {
            case 1:
                if (!ScriptsForBrains.IsObjectWithinBrainActivationRange(obj, FindPlayerCentreOfWorld()))
                    break;

                [[fallthrough]];
            case 2:
                ScriptsForBrains.StartOrRequestNewStreamedScriptBrain(
                    static_cast<uint8>(obj->m_nStreamedScriptBrainToLoad), // cast?
                    obj,
                    t,
                    false
                );
                break;
            default:
                break;
            }
            break;
        }
        default:
            break;
        }
    }
}

void InjectHooks_Script10() {
    {
        RH_ScopedClass(CRunningScript);
        RH_ScopedCategory("Scripts");

        RH_ScopedInstall(ScriptTaskPickUpObject, 0x46AF50, { .reversed = false });
    }
    {
        RH_ScopedClass(CTheScripts);
        RH_ScopedCategory("Scripts");

        RH_ScopedInstall(AddToVehicleModelsBlockedByScript, 0x46B200);
        RH_ScopedInstall(AddToWaitingForScriptBrainArray, 0x46AB60);
        RH_ScopedInstall(ClearAllVehicleModelsBlockedByScript, 0x46A840);
        RH_ScopedInstall(ClearAllSuppressedCarModels, 0x46A7C0);
        RH_ScopedInstall(RemoveFromWaitingForScriptBrainArray, 0x46ABC0);
        RH_ScopedInstall(HasCarModelBeenSuppressed, 0x46A810);
        RH_ScopedInstall(HasVehicleModelBeenBlockedByScript, 0x46A890);
        RH_ScopedInstall(ProcessWaitingForScriptBrainArray, 0x46CF00);
    }
}
