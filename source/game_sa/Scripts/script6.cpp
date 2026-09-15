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



// 0x489490
bool CRunningScript::ThisIsAValidRandomPed(ePedType pedType, bool civilian, bool gang, bool criminal) {
    switch (pedType) {
    case PED_TYPE_CIVMALE:
    case PED_TYPE_CIVFEMALE:
        return civilian;
    case PED_TYPE_GANG1:
    case PED_TYPE_GANG2:
    case PED_TYPE_GANG3:
    case PED_TYPE_GANG4:
    case PED_TYPE_GANG5:
    case PED_TYPE_GANG6:
    case PED_TYPE_GANG7:
    case PED_TYPE_GANG8:
    case PED_TYPE_GANG9:
        return gang;
    case PED_TYPE_CRIMINAL:
    case PED_TYPE_PROSTITUTE:
        return criminal;
    default:
        return false;
    }
}

void InjectHooks_Script6() {
    RH_ScopedClass(CRunningScript);
    RH_ScopedCategory("Scripts");

    RH_ScopedInstall(ThisIsAValidRandomPed, 0x489490);
}
