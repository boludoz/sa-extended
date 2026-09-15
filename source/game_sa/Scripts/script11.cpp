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



// 0x470150
void CRunningScript::PlayAnimScriptCommand(int32 commandId) {
    plugin::CallMethod<0x470150, CRunningScript*, int32>(this, commandId);
}


// 0x470370
void CTheScripts::ReinitialiseSwitchStatementData() {
    NumberOfEntriesStillToReadForSwitch = 0;
    ValueToCheckInSwitchStatement       = 0;
    SwitchDefaultExists                 = false;
    SwitchDefaultAddress                = 0;
    NumberOfEntriesInSwitchTable        = 0;
}


// 0x470390
void CTheScripts::AddToSwitchJumpTable(int32 switchValue, int32 switchLabelLocalAddress) {
    SwitchJumpTable[NumberOfEntriesInSwitchTable].m_nSwitchValue        = switchValue;
    SwitchJumpTable[NumberOfEntriesInSwitchTable].m_nSwitchLabelAddress = switchLabelLocalAddress;
    NumberOfEntriesInSwitchTable++;
}


// 0x4703C0
void CTheScripts::UseSwitchJumpTable(int32& switchLabelAddress) {
    switchLabelAddress = 0x0;

    const auto CheckEntryAndJump = [&](tScriptSwitchCase* swtch) {
        if (swtch && ValueToCheckInSwitchStatement != swtch->m_nSwitchValue) {
            return false;
        }

        switchLabelAddress = swtch ? swtch->m_nSwitchLabelAddress : SwitchDefaultAddress;
        ReinitialiseSwitchStatementData();
        return true;
    };

    auto ptr1 = 0u;
    auto ptr2 = NumberOfEntriesInSwitchTable - 1u;
    while (ptr2 - ptr1 > 1) {
        const auto idx = (ptr1 + ptr2) / 2;
        if (CheckEntryAndJump(&SwitchJumpTable[idx])) {
            return;
        }

        if (ValueToCheckInSwitchStatement <= SwitchJumpTable[idx].m_nSwitchValue) {
            ptr2 = idx;
        } else {
            ptr1 = idx;
        }
    }

    if (CheckEntryAndJump(&SwitchJumpTable[ptr2]) || CheckEntryAndJump(&SwitchJumpTable[ptr1])) {
        return;
    }

    CheckEntryAndJump(nullptr); // Jump to the default case
}


// 0x470940
void CTheScripts::InitialiseConnectLodObjects(uint16 index) {
    ScriptConnectLodsObjects[index] = tScriptConnectLodsObject();
}


// 0x470960
void CTheScripts::InitialiseAllConnectLodObjects() {
    for (auto& pair : ScriptConnectLodsObjects) {
        pair = tScriptConnectLodsObject(); // InitialiseConnectLodObjects();
    }
}


// 0x470980
void CTheScripts::AddToListOfConnectedLodObjects(CObject* obj1, CObject* obj2) {
    const auto idx1 = GetObjectPool()->GetRef(obj1), idx2 = GetObjectPool()->GetRef(obj2);

    const auto lod = rng::find_if(ScriptConnectLodsObjects, [idx1, idx2](auto& lod) {
        return lod.a == idx1 && lod.b == idx2;
    });
    if (lod != ScriptConnectLodsObjects.end()) {
        // Already exists.
        return;
    }

    const auto free = rng::find_if(ScriptConnectLodsObjects, [](auto& lod) { return lod.a == -1; });
    assert(free != ScriptConnectLodsObjects.end()); // In vanilla game does OOB access.

    free->a = idx1;
    free->b = idx2;
}


// 0x470A20
void CTheScripts::ScriptConnectLodsFunction(int32 lodRef1, int32 lodRef2) {
    auto obj1 = GetObjectPool()->GetAtRef(lodRef1), obj2 = GetObjectPool()->GetAtRef(lodRef2);

    obj1->SetLod(obj2);
    obj2->AddLodChildren();
    CWorld::Remove(obj2);
    obj2->SetupBigBuilding();
    CWorld::Add(obj2);
}

void InjectHooks_Script11() {
    {
        RH_ScopedClass(CRunningScript);
        RH_ScopedCategory("Scripts");

        RH_ScopedInstall(PlayAnimScriptCommand, 0x470150, { .reversed = false });
    }
    {
        RH_ScopedClass(CTheScripts);
        RH_ScopedCategory("Scripts");

        RH_ScopedInstall(InitialiseAllConnectLodObjects, 0x470960);
        RH_ScopedInstall(InitialiseConnectLodObjects, 0x470940);
        RH_ScopedInstall(AddToListOfConnectedLodObjects, 0x470980);
        RH_ScopedInstall(AddToSwitchJumpTable, 0x470390);
        RH_ScopedInstall(ReinitialiseSwitchStatementData, 0x470370);
        RH_ScopedInstall(ScriptConnectLodsFunction, 0x470A20);
        RH_ScopedInstall(UseSwitchJumpTable, 0x4703C0);
    }
}
