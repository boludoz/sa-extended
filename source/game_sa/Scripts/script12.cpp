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



// 0x474710
void CTheScripts::InitialiseSpecialAnimGroup(uint16 index) {
    ScriptAttachedAnimGroups[index] = tScriptAttachedAnimGroup();
}


// 0x474730
void CTheScripts::InitialiseSpecialAnimGroupsAttachedToCharModels() {
    for (auto& group : ScriptAttachedAnimGroups) {
        group = tScriptAttachedAnimGroup(); // InitialiseSpecialAnimGroup();
    }
}


// 0x474750
void CTheScripts::AddToListOfSpecialAnimGroupsAttachedToCharModels(int32 modelId, Const char* ifpName) {
    const auto aag = rng::find_if(ScriptAttachedAnimGroups, [modelId, ifpName](auto& aag) {
        return aag.m_nModelID == modelId && !std::strcmp(aag.m_IfpName, ifpName);
    });
    if (aag != ScriptAttachedAnimGroups.end()) {
        // Already exists.
        return;
    }

    const auto free = rng::find_if(ScriptAttachedAnimGroups, [](auto& aag) {
        return aag.m_nModelID == MODEL_INVALID;
    });
    if (free == ScriptAttachedAnimGroups.end()) {
        return;
    }

    free->m_nModelID = modelId;
    assert(std::strlen(ifpName) < sizeof(free->m_IfpName));
    std::strcpy(free->m_IfpName, ifpName);
}


// 0x474800
bool CTheScripts::ScriptAttachAnimGroupToCharModel(int32 modelId, const char* ifpName) {
    auto* mi = CModelInfo::GetModelInfo(modelId);
    if (mi->GetAnimFileIndex() == CAnimManager::GetAnimationBlockIndex(ifpName)) {
        // Already attached?
        return false;
    }
    mi->SetAnimFile(ifpName);
    mi->ConvertAnimFileIndex();
    return true;
}

void InjectHooks_Script12() {
    RH_ScopedClass(CTheScripts);
    RH_ScopedCategory("Scripts");

    RH_ScopedInstall(InitialiseSpecialAnimGroupsAttachedToCharModels, 0x474730);
    RH_ScopedInstall(InitialiseSpecialAnimGroup, 0x474710);
    RH_ScopedInstall(AddToListOfSpecialAnimGroupsAttachedToCharModels, 0x474750);
    RH_ScopedInstall(ScriptAttachAnimGroupToCharModel, 0x474800);
}
