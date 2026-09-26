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
#include "ShotInfo.h"


inline constexpr uint32 DbgLineColour = 0x0000FFFF; // r = 0, g = 0, b = 255, a = 255
static uint16 NumScriptDebugLines;
static tStoredLine aStoredLines[MAX_NUM_STORED_LINES];



// Updates comparement flag, used in conditional commands
// 0x4859D0
void CRunningScript::UpdateCompareFlag(bool state) {
    if (m_NotFlag)
        state = !state;

    if (m_AndOrState == ANDOR_NONE) {
        m_CondResult = state;
        return;
    }

    if (m_AndOrState >= ANDS_1 && m_AndOrState <= ANDS_8) {
        m_CondResult &= state;
        if (m_AndOrState == ANDS_1)
            m_AndOrState = ANDOR_NONE;
        else
            m_AndOrState--;

        return;
    }

    if (m_AndOrState >= ORS_1 && m_AndOrState <= ORS_8) {
        m_CondResult |= state;
        if (m_AndOrState == ORS_1)
            m_AndOrState = ANDOR_NONE;
        else
            m_AndOrState--;

        return;
    }
}


// 0x485A50
void CRunningScript::DoDeathArrestCheck() {
    if (!m_IsDeathArrestCheckEnabled) {
        return;
    }

    if (!CTheScripts::IsPlayerOnAMission()) {
        return;
    }

    if (const auto& pi = FindPlayerInfo(); !pi.IsRestartingAfterDeath() && !pi.IsRestartingAfterArrest()) {
        return;
    }

    CMessages::ClearSmallMessagesOnly();
    memset(&CTheScripts::ScriptSpace[CTheScripts::OnAMissionFlag], 0, sizeof(uint32));
    ResetIP();

    m_DoneDeathArrest = true;
    m_WakeTime        = 0;
}


// 0x486D80
void CRunningScript::LocateCharCommand(int32 commandId) {
    bool LatestCmpFlagResult = false;
    bool Do3dCheck;
    bool IsWithinRange = true;
    bool SkipRestOfCheck = false;
    int32 HighlightArea;

    if (commandId >= COMMAND_LOCATE_CHAR_ANY_MEANS_3D && commandId <= COMMAND_LOCATE_STOPPED_CHAR_IN_CAR_3D) {
        Do3dCheck = true;
        CollectParameters(8);
    } else {
        Do3dCheck = false;
        CollectParameters(6);
    }

    CPed* pPed = CPools::GetPed(ScriptParams[0].iParam);

    CVector CharPos;
    if (pPed->bInVehicle && pPed->m_pMyVehicle)
        CharPos = pPed->m_pMyVehicle->GetPosition();
    else
        CharPos = pPed->GetPosition();

    switch (commandId) {
    case COMMAND_LOCATE_STOPPED_CHAR_ANY_MEANS_2D:
    case COMMAND_LOCATE_STOPPED_CHAR_ON_FOOT_2D:
    case COMMAND_LOCATE_STOPPED_CHAR_IN_CAR_2D:
    case COMMAND_LOCATE_STOPPED_CHAR_ANY_MEANS_3D:
    case COMMAND_LOCATE_STOPPED_CHAR_ON_FOOT_3D:
    case COMMAND_LOCATE_STOPPED_CHAR_IN_CAR_3D:
        if (!CTheScripts::IsPedStopped(pPed)) {
            LatestCmpFlagResult = false;
            SkipRestOfCheck = true;
        }
        break;
    default:
        break;
    }

    float TargetX = ScriptParams[1].fParam;
    float TargetY = ScriptParams[2].fParam;
    float TargetZ = 0.0f;
    float TargetWidth;
    float TargetDepth;
    float TargetHeight = 0.0f;

    if (Do3dCheck) {
        TargetZ = ScriptParams[3].fParam;
        TargetWidth = ScriptParams[4].fParam;
        TargetDepth = ScriptParams[5].fParam;
        TargetHeight = ScriptParams[6].fParam;
        HighlightArea = ScriptParams[7].iParam;
    } else {
        TargetWidth = ScriptParams[3].fParam;
        TargetDepth = ScriptParams[4].fParam;
        HighlightArea = ScriptParams[5].iParam;
    }

    if (!SkipRestOfCheck) {
        LatestCmpFlagResult = false;
        if (Do3dCheck) {
            if (CharPos.x >= TargetX - TargetWidth &&
                CharPos.x <= TargetX + TargetWidth &&
                CharPos.y >= TargetY - TargetDepth &&
                CharPos.y <= TargetY + TargetDepth &&
                CharPos.z >= TargetZ - TargetHeight &&
                CharPos.z <= TargetZ + TargetHeight) {
                IsWithinRange = true;
            } else {
                IsWithinRange = false;
            }
        } else {
            if (CharPos.x >= TargetX - TargetWidth &&
                CharPos.x <= TargetX + TargetWidth &&
                CharPos.y >= TargetY - TargetDepth &&
                CharPos.y <= TargetY + TargetDepth) {
                IsWithinRange = true;
            } else {
                IsWithinRange = false;
            }
        }

        if (IsWithinRange) {
            switch (commandId) {
            case COMMAND_LOCATE_CHAR_ANY_MEANS_2D:
            case COMMAND_LOCATE_STOPPED_CHAR_ANY_MEANS_2D:
            case COMMAND_LOCATE_CHAR_ANY_MEANS_3D:
            case COMMAND_LOCATE_STOPPED_CHAR_ANY_MEANS_3D:
                LatestCmpFlagResult = true;
                break;
            case COMMAND_LOCATE_CHAR_ON_FOOT_2D:
            case COMMAND_LOCATE_STOPPED_CHAR_ON_FOOT_2D:
            case COMMAND_LOCATE_CHAR_ON_FOOT_3D:
            case COMMAND_LOCATE_STOPPED_CHAR_ON_FOOT_3D:
                if (!pPed->bInVehicle)
                    LatestCmpFlagResult = true;
                break;
            case COMMAND_LOCATE_CHAR_IN_CAR_2D:
            case COMMAND_LOCATE_STOPPED_CHAR_IN_CAR_2D:
            case COMMAND_LOCATE_CHAR_IN_CAR_3D:
            case COMMAND_LOCATE_STOPPED_CHAR_IN_CAR_3D:
                if (pPed->bInVehicle)
                    LatestCmpFlagResult = true;
                break;
            default:
                break;
            }
        }
    }

    UpdateCompareFlag(LatestCmpFlagResult);

    if (HighlightArea) {
        if (Do3dCheck) {
            CTheScripts::HighlightImportantArea(reinterpret_cast<uint32>(this) + reinterpret_cast<uint32>(m_IP),
                TargetX - TargetWidth,
                TargetY - TargetDepth,
                TargetX + TargetWidth,
                TargetY + TargetDepth,
                TargetZ);
        } else {
            CTheScripts::HighlightImportantArea(reinterpret_cast<uint32>(this) + reinterpret_cast<uint32>(m_IP),
                TargetX - TargetWidth,
                TargetY - TargetDepth,
                TargetX + TargetWidth,
                TargetY + TargetDepth,
                -100.0f);
        }
    }

    if (CTheScripts::DbgFlag) {
        if (!Do3dCheck) {
            CTheScripts::DrawDebugSquare(
                TargetX - TargetWidth,
                TargetY - TargetDepth,
                TargetX + TargetWidth,
                TargetY + TargetDepth);
        }
    }
}


// 0x4870F0
void CRunningScript::LocateCharCharCommand(int32 commandId) {
    bool LatestCmpFlagResult;
    bool Do3dCheck;
    bool IsWithinRange;
    int32 HighlightArea;

    if (commandId < COMMAND_LOCATE_CHAR_ANY_MEANS_CHAR_3D || commandId > COMMAND_LOCATE_CHAR_IN_CAR_CHAR_3D) {
        Do3dCheck = false;
        CollectParameters(5);
    } else {
        Do3dCheck = true;
        CollectParameters(6);
    }

    CPed* pFirstPed = CPools::GetPed(ScriptParams[0].iParam);
    CPed* pTargetPed = CPools::GetPed(ScriptParams[1].iParam);

    CVector FirstCharPos;
    if (pFirstPed->bInVehicle && pFirstPed->m_pMyVehicle)
        FirstCharPos = pFirstPed->m_pMyVehicle->GetPosition();
    else
        FirstCharPos = pFirstPed->GetPosition();

    CVector TargetCharPos;
    if (pTargetPed->bInVehicle && pTargetPed->m_pMyVehicle)
        TargetCharPos = pTargetPed->m_pMyVehicle->GetPosition();
    else
        TargetCharPos = pTargetPed->GetPosition();

    float WidthFromTarget = ScriptParams[2].fParam;
    float DepthFromTarget = ScriptParams[3].fParam;
    float HeightFromTarget = 0.0f;
    if (Do3dCheck) {
        HeightFromTarget = ScriptParams[4].fParam;
        HighlightArea = ScriptParams[5].iParam;
    } else {
        HighlightArea = ScriptParams[4].iParam;
    }

    LatestCmpFlagResult = false;

    if (Do3dCheck) {
        if (FirstCharPos.x >= TargetCharPos.x - WidthFromTarget &&
            FirstCharPos.x <= TargetCharPos.x + WidthFromTarget &&
            FirstCharPos.y >= TargetCharPos.y - DepthFromTarget &&
            FirstCharPos.y <= TargetCharPos.y + DepthFromTarget &&
            FirstCharPos.z >= TargetCharPos.z - HeightFromTarget &&
            FirstCharPos.z <= TargetCharPos.z + HeightFromTarget) {
            IsWithinRange = true;
        } else {
            IsWithinRange = false;
        }
    } else {
        if (FirstCharPos.x >= TargetCharPos.x - WidthFromTarget &&
            FirstCharPos.x <= TargetCharPos.x + WidthFromTarget &&
            FirstCharPos.y >= TargetCharPos.y - DepthFromTarget &&
            FirstCharPos.y <= TargetCharPos.y + DepthFromTarget) {
            IsWithinRange = true;
        } else {
            IsWithinRange = false;
        }
    }

    if (IsWithinRange) {
        switch (commandId) {
        case COMMAND_LOCATE_CHAR_ANY_MEANS_CHAR_2D:
        case COMMAND_LOCATE_CHAR_ANY_MEANS_CHAR_3D:
            LatestCmpFlagResult = true;
            break;
        case COMMAND_LOCATE_CHAR_ON_FOOT_CHAR_2D:
        case COMMAND_LOCATE_CHAR_ON_FOOT_CHAR_3D:
            if (!pFirstPed->bInVehicle)
                LatestCmpFlagResult = true;
            break;
        case COMMAND_LOCATE_CHAR_IN_CAR_CHAR_2D:
        case COMMAND_LOCATE_CHAR_IN_CAR_CHAR_3D:
            if (pFirstPed->bInVehicle)
                LatestCmpFlagResult = true;
            break;
        default:
            break;
        }
    }

    UpdateCompareFlag(LatestCmpFlagResult);

    if (HighlightArea) {
        if (Do3dCheck) {
            CTheScripts::HighlightImportantArea(reinterpret_cast<uint32>(this) + reinterpret_cast<uint32>(m_IP),
                TargetCharPos.x - WidthFromTarget,
                TargetCharPos.y - DepthFromTarget,
                TargetCharPos.x + WidthFromTarget,
                TargetCharPos.y + DepthFromTarget,
                TargetCharPos.z);
        } else {
            CTheScripts::HighlightImportantArea(reinterpret_cast<uint32>(this) + reinterpret_cast<uint32>(m_IP),
                TargetCharPos.x - WidthFromTarget,
                TargetCharPos.y - DepthFromTarget,
                TargetCharPos.x + WidthFromTarget,
                TargetCharPos.y + DepthFromTarget,
                -100.0f);
        }
    }

    if (CTheScripts::DbgFlag) {
        if (!Do3dCheck) {
            CTheScripts::DrawDebugSquare(
                TargetCharPos.x - WidthFromTarget,
                TargetCharPos.y - DepthFromTarget,
                TargetCharPos.x + WidthFromTarget,
                TargetCharPos.y + DepthFromTarget);
        }
    }
}


// 0x487420
void CRunningScript::LocateCharCarCommand(int32 commandId) {
    bool LatestCmpFlagResult;
    bool Do3dCheck;
    bool IsWithinRange;
    int32 HighlightArea;

    if (commandId < COMMAND_LOCATE_CHAR_ANY_MEANS_CAR_3D || commandId > COMMAND_LOCATE_CHAR_IN_CAR_CAR_3D) {
        Do3dCheck = false;
        CollectParameters(5);
    } else {
        Do3dCheck = true;
        CollectParameters(6);
    }

    CPed* pFirstPed = CPools::GetPed(ScriptParams[0].iParam);
    CVehicle* pTargetVehicle = CPools::GetVehicle(ScriptParams[1].iParam);

    CVector FirstCharPos;
    if (pFirstPed->bInVehicle && pFirstPed->m_pMyVehicle)
        FirstCharPos = pFirstPed->m_pMyVehicle->GetPosition();
    else
        FirstCharPos = pFirstPed->GetPosition();

    CVector TargetVehiclePos = pTargetVehicle->GetPosition();

    float WidthFromTarget = ScriptParams[2].fParam;
    float DepthFromTarget = ScriptParams[3].fParam;
    float HeightFromTarget = 0.0f;
    if (Do3dCheck) {
        HeightFromTarget = ScriptParams[4].fParam;
        HighlightArea = ScriptParams[5].iParam;
    } else {
        HighlightArea = ScriptParams[4].iParam;
    }

    LatestCmpFlagResult = false;

    if (Do3dCheck) {
        if (FirstCharPos.x >= TargetVehiclePos.x - WidthFromTarget &&
            FirstCharPos.x <= TargetVehiclePos.x + WidthFromTarget &&
            FirstCharPos.y >= TargetVehiclePos.y - DepthFromTarget &&
            FirstCharPos.y <= TargetVehiclePos.y + DepthFromTarget &&
            FirstCharPos.z >= TargetVehiclePos.z - HeightFromTarget &&
            FirstCharPos.z <= TargetVehiclePos.z + HeightFromTarget) {
            IsWithinRange = true;
        } else {
            IsWithinRange = false;
        }
    } else {
        if (FirstCharPos.x >= TargetVehiclePos.x - WidthFromTarget &&
            FirstCharPos.x <= TargetVehiclePos.x + WidthFromTarget &&
            FirstCharPos.y >= TargetVehiclePos.y - DepthFromTarget &&
            FirstCharPos.y <= TargetVehiclePos.y + DepthFromTarget) {
            IsWithinRange = true;
        } else {
            IsWithinRange = false;
        }
    }

    if (IsWithinRange) {
        switch (commandId) {
        case COMMAND_LOCATE_CHAR_ANY_MEANS_CAR_2D:
        case COMMAND_LOCATE_CHAR_ANY_MEANS_CAR_3D:
            LatestCmpFlagResult = true;
            break;
        case COMMAND_LOCATE_CHAR_ON_FOOT_CAR_2D:
        case COMMAND_LOCATE_CHAR_ON_FOOT_CAR_3D:
            if (!pFirstPed->bInVehicle)
                LatestCmpFlagResult = true;
            break;
        case COMMAND_LOCATE_CHAR_IN_CAR_CAR_2D:
        case COMMAND_LOCATE_CHAR_IN_CAR_CAR_3D:
            if (pFirstPed->bInVehicle)
                LatestCmpFlagResult = true;
            break;
        default:
            break;
        }
    }

    UpdateCompareFlag(LatestCmpFlagResult);

    if (HighlightArea) {
        if (Do3dCheck) {
            CTheScripts::HighlightImportantArea(reinterpret_cast<uint32>(this) + reinterpret_cast<uint32>(m_IP),
                TargetVehiclePos.x - WidthFromTarget,
                TargetVehiclePos.y - DepthFromTarget,
                TargetVehiclePos.x + WidthFromTarget,
                TargetVehiclePos.y + DepthFromTarget,
                TargetVehiclePos.z);
        } else {
            CTheScripts::HighlightImportantArea(reinterpret_cast<uint32>(this) + reinterpret_cast<uint32>(m_IP),
                TargetVehiclePos.x - WidthFromTarget,
                TargetVehiclePos.y - DepthFromTarget,
                TargetVehiclePos.x + WidthFromTarget,
                TargetVehiclePos.y + DepthFromTarget,
                -100.0f);
        }
    }

    if (CTheScripts::DbgFlag) {
        if (!Do3dCheck) {
            CTheScripts::DrawDebugSquare(
                TargetVehiclePos.x - WidthFromTarget,
                TargetVehiclePos.y - DepthFromTarget,
                TargetVehiclePos.x + WidthFromTarget,
                TargetVehiclePos.y + DepthFromTarget);
        }
    }
}


// 0x487720
void CRunningScript::LocateCharObjectCommand(int32 commandId) {
    bool LatestCmpFlagResult;
    bool Do3dCheck;
    bool IsWithinRange;
    int32 HighlightArea;

    if (commandId < COMMAND_LOCATE_CHAR_ANY_MEANS_OBJECT_3D || commandId > COMMAND_LOCATE_CHAR_IN_CAR_OBJECT_3D) {
        Do3dCheck = false;
        CollectParameters(5);
    } else {
        Do3dCheck = true;
        CollectParameters(6);
    }

    CPed* pFirstPed = CPools::GetPed(ScriptParams[0].iParam);
    CObject* pTargetObject = CPools::GetObject(ScriptParams[1].iParam);

    CVector FirstCharPos;
    if (pFirstPed->bInVehicle && pFirstPed->m_pMyVehicle)
        FirstCharPos = pFirstPed->m_pMyVehicle->GetPosition();
    else
        FirstCharPos = pFirstPed->GetPosition();

    CVector TargetObjectPos = pTargetObject->GetPosition();

    float WidthFromTarget = ScriptParams[2].fParam;
    float DepthFromTarget = ScriptParams[3].fParam;
    float HeightFromTarget = 0.0f;
    if (Do3dCheck) {
        HeightFromTarget = ScriptParams[4].fParam;
        HighlightArea = ScriptParams[5].iParam;
    } else {
        HighlightArea = ScriptParams[4].iParam;
    }

    LatestCmpFlagResult = false;

    if (Do3dCheck) {
        if (FirstCharPos.x >= TargetObjectPos.x - WidthFromTarget &&
            FirstCharPos.x <= TargetObjectPos.x + WidthFromTarget &&
            FirstCharPos.y >= TargetObjectPos.y - DepthFromTarget &&
            FirstCharPos.y <= TargetObjectPos.y + DepthFromTarget &&
            FirstCharPos.z >= TargetObjectPos.z - HeightFromTarget &&
            FirstCharPos.z <= TargetObjectPos.z + HeightFromTarget) {
            IsWithinRange = true;
        } else {
            IsWithinRange = false;
        }
    } else {
        if (FirstCharPos.x >= TargetObjectPos.x - WidthFromTarget &&
            FirstCharPos.x <= TargetObjectPos.x + WidthFromTarget &&
            FirstCharPos.y >= TargetObjectPos.y - DepthFromTarget &&
            FirstCharPos.y <= TargetObjectPos.y + DepthFromTarget) {
            IsWithinRange = true;
        } else {
            IsWithinRange = false;
        }
    }

    if (IsWithinRange) {
        switch (commandId) {
        case COMMAND_LOCATE_CHAR_ANY_MEANS_OBJECT_2D:
        case COMMAND_LOCATE_CHAR_ANY_MEANS_OBJECT_3D:
            LatestCmpFlagResult = true;
            break;
        case COMMAND_LOCATE_CHAR_ON_FOOT_OBJECT_2D:
        case COMMAND_LOCATE_CHAR_ON_FOOT_OBJECT_3D:
            if (!pFirstPed->bInVehicle)
                LatestCmpFlagResult = true;
            break;
        case COMMAND_LOCATE_CHAR_IN_CAR_OBJECT_2D:
        case COMMAND_LOCATE_CHAR_IN_CAR_OBJECT_3D:
            if (pFirstPed->bInVehicle)
                LatestCmpFlagResult = true;
            break;
        default:
            break;
        }
    }

    UpdateCompareFlag(LatestCmpFlagResult);

    if (HighlightArea) {
        if (Do3dCheck) {
            CTheScripts::HighlightImportantArea(reinterpret_cast<uint32>(this) + reinterpret_cast<uint32>(m_IP),
                TargetObjectPos.x - WidthFromTarget,
                TargetObjectPos.y - DepthFromTarget,
                TargetObjectPos.x + WidthFromTarget,
                TargetObjectPos.y + DepthFromTarget,
                TargetObjectPos.z);
        } else {
            CTheScripts::HighlightImportantArea(reinterpret_cast<uint32>(this) + reinterpret_cast<uint32>(m_IP),
                TargetObjectPos.x - WidthFromTarget,
                TargetObjectPos.y - DepthFromTarget,
                TargetObjectPos.x + WidthFromTarget,
                TargetObjectPos.y + DepthFromTarget,
                -100.0f);
        }
    }

    if (CTheScripts::DbgFlag) {
        if (!Do3dCheck) {
            CTheScripts::DrawDebugSquare(
                TargetObjectPos.x - WidthFromTarget,
                TargetObjectPos.y - DepthFromTarget,
                TargetObjectPos.x + WidthFromTarget,
                TargetObjectPos.y + DepthFromTarget);
        }
    }
}


// 0x487A20
void CRunningScript::LocateCarCommand(int32 commandId) {
    bool LatestCmpFlagResult;
    bool Do3dCheck;
    bool SkipRestOfCheck = false;
    int32 HighlightArea;

    if (commandId < COMMAND_LOCATE_CAR_3D || commandId > COMMAND_LOCATE_STOPPED_CAR_3D) {
        Do3dCheck = false;
        CollectParameters(6);
    } else {
        Do3dCheck = true;
        CollectParameters(8);
    }

    CVehicle* pVehicle = CPools::GetVehicle(ScriptParams[0].iParam);

    if (commandId == COMMAND_LOCATE_STOPPED_CAR_2D || commandId == COMMAND_LOCATE_STOPPED_CAR_3D) {
        if (!CTheScripts::IsVehicleStopped(pVehicle)) {
            LatestCmpFlagResult = false;
            SkipRestOfCheck = true;
        }
    }

    float TargetX = ScriptParams[1].fParam;
    float TargetY = ScriptParams[2].fParam;
    float TargetZ = 0.0f;
    float TargetWidth;
    float TargetDepth;
    float TargetHeight = 0.0f;

    if (Do3dCheck) {
        TargetZ = ScriptParams[3].fParam;
        TargetWidth = ScriptParams[4].fParam;
        TargetDepth = ScriptParams[5].fParam;
        TargetHeight = ScriptParams[6].fParam;
        HighlightArea = ScriptParams[7].iParam;
    } else {
        TargetWidth = ScriptParams[3].fParam;
        TargetDepth = ScriptParams[4].fParam;
        HighlightArea = ScriptParams[5].iParam;
    }

    if (!SkipRestOfCheck) {
        CVector VehiclePos = pVehicle->GetPosition();
        LatestCmpFlagResult = false;

        if (Do3dCheck) {
            if (VehiclePos.x >= TargetX - TargetWidth &&
                VehiclePos.x <= TargetX + TargetWidth &&
                VehiclePos.y >= TargetY - TargetDepth &&
                VehiclePos.y <= TargetY + TargetDepth &&
                VehiclePos.z >= TargetZ - TargetHeight &&
                VehiclePos.z <= TargetZ + TargetHeight) {
                LatestCmpFlagResult = true;
            }
        } else {
            if (VehiclePos.x >= TargetX - TargetWidth &&
                VehiclePos.x <= TargetX + TargetWidth &&
                VehiclePos.y >= TargetY - TargetDepth &&
                VehiclePos.y <= TargetY + TargetDepth) {
                LatestCmpFlagResult = true;
            }
        }
    }

    UpdateCompareFlag(LatestCmpFlagResult);

    if (HighlightArea) {
        if (Do3dCheck) {
            CTheScripts::HighlightImportantArea(reinterpret_cast<uint32>(this) + reinterpret_cast<uint32>(m_IP),
                TargetX - TargetWidth,
                TargetY - TargetDepth,
                TargetX + TargetWidth,
                TargetY + TargetDepth,
                TargetZ);
        } else {
            CTheScripts::HighlightImportantArea(reinterpret_cast<uint32>(this) + reinterpret_cast<uint32>(m_IP),
                TargetX - TargetWidth,
                TargetY - TargetDepth,
                TargetX + TargetWidth,
                TargetY + TargetDepth,
                -100.0f);
        }
    }

    if (CTheScripts::DbgFlag) {
        if (!Do3dCheck) {
            CTheScripts::DrawDebugSquare(
                TargetX - TargetWidth,
                TargetY - TargetDepth,
                TargetX + TargetWidth,
                TargetY + TargetDepth);
        }
    }
}


// 0x487D10
void CRunningScript::LocateObjectCommand(int32 commandId) {
    bool LatestCmpFlagResult;
    bool Do3dCheck;
    int32 HighlightArea;

    if (commandId == COMMAND_LOCATE_OBJECT_3D) {
        Do3dCheck = true;
        CollectParameters(8);
    } else {
        Do3dCheck = false;
        CollectParameters(6);
    }

    CObject* pObject = CPools::GetObject(ScriptParams[0].iParam);

    float TargetX = ScriptParams[1].fParam;
    float TargetY = ScriptParams[2].fParam;
    float TargetZ = 0.0f;
    float TargetWidth;
    float TargetDepth;
    float TargetHeight = 0.0f;

    if (Do3dCheck) {
        TargetZ = ScriptParams[3].fParam;
        TargetWidth = ScriptParams[4].fParam;
        TargetDepth = ScriptParams[5].fParam;
        TargetHeight = ScriptParams[6].fParam;
        HighlightArea = ScriptParams[7].iParam;
    } else {
        TargetWidth = ScriptParams[3].fParam;
        TargetDepth = ScriptParams[4].fParam;
        HighlightArea = ScriptParams[5].iParam;
    }

    CVector ObjectPos = pObject->GetPosition();
    LatestCmpFlagResult = false;

    if (Do3dCheck) {
        if (ObjectPos.x >= TargetX - TargetWidth &&
            ObjectPos.x <= TargetX + TargetWidth &&
            ObjectPos.y >= TargetY - TargetDepth &&
            ObjectPos.y <= TargetY + TargetDepth &&
            ObjectPos.z >= TargetZ - TargetHeight &&
            ObjectPos.z <= TargetZ + TargetHeight) {
            LatestCmpFlagResult = true;
        }
    } else {
        if (ObjectPos.x >= TargetX - TargetWidth &&
            ObjectPos.x <= TargetX + TargetWidth &&
            ObjectPos.y >= TargetY - TargetDepth &&
            ObjectPos.y <= TargetY + TargetDepth) {
            LatestCmpFlagResult = true;
        }
    }

    UpdateCompareFlag(LatestCmpFlagResult);

    if (HighlightArea) {
        if (Do3dCheck) {
            CTheScripts::HighlightImportantArea(reinterpret_cast<uint32>(this) + reinterpret_cast<uint32>(m_IP),
                TargetX - TargetWidth,
                TargetY - TargetDepth,
                TargetX + TargetWidth,
                TargetY + TargetDepth,
                TargetZ);
        } else {
            CTheScripts::HighlightImportantArea(reinterpret_cast<uint32>(this) + reinterpret_cast<uint32>(m_IP),
                TargetX - TargetWidth,
                TargetY - TargetDepth,
                TargetX + TargetWidth,
                TargetY + TargetDepth,
                -100.0f);
        }
    }

    if (CTheScripts::DbgFlag) {
        if (!Do3dCheck) {
            CTheScripts::DrawDebugSquare(
                TargetX - TargetWidth,
                TargetY - TargetDepth,
                TargetX + TargetWidth,
                TargetY + TargetDepth);
        }
    }
}


// 0x487F60
void CRunningScript::CharInAngledAreaCheckCommand(int32 commandId) {
    bool SkipRestOfCheck = false;
    bool Do3dCheck;
    bool LatestCmpFlagResult = false;
    int32 HighlightArea;

    if (commandId >= COMMAND_IS_CHAR_IN_ANGLED_AREA_3D && commandId <= COMMAND_IS_CHAR_STOPPED_IN_ANGLED_AREA_IN_CAR_3D) {
        Do3dCheck = true;
        CollectParameters(9);
    } else {
        Do3dCheck = false;
        CollectParameters(7);
    }

    CPed* pPed = CPools::GetPed(ScriptParams[0].iParam);

    switch (commandId) {
    case COMMAND_IS_CHAR_STOPPED_IN_ANGLED_AREA_2D:
    case COMMAND_IS_CHAR_STOPPED_IN_ANGLED_AREA_ON_FOOT_2D:
    case COMMAND_IS_CHAR_STOPPED_IN_ANGLED_AREA_IN_CAR_2D:
    case COMMAND_IS_CHAR_STOPPED_IN_ANGLED_AREA_3D:
    case COMMAND_IS_CHAR_STOPPED_IN_ANGLED_AREA_ON_FOOT_3D:
    case COMMAND_IS_CHAR_STOPPED_IN_ANGLED_AREA_IN_CAR_3D:
        if (!CTheScripts::IsPedStopped(pPed)) {
            LatestCmpFlagResult = false;
            SkipRestOfCheck = true;
        }
        break;
    default:
        break;
    }

    float TargetX1 = ScriptParams[1].fParam;
    float TargetY1 = ScriptParams[2].fParam;
    float TargetZ1 = 0.0f;
    float TargetX2;
    float TargetY2;
    float TargetZ2 = 0.0f;
    float DistanceFrom1To4;

    if (Do3dCheck) {
        TargetZ1 = ScriptParams[3].fParam;
        TargetX2 = ScriptParams[4].fParam;
        TargetY2 = ScriptParams[5].fParam;
        TargetZ2 = ScriptParams[6].fParam;
        if (TargetZ1 > TargetZ2) {
            std::swap(TargetZ1, TargetZ2);
        }
        DistanceFrom1To4 = ScriptParams[7].fParam;
        HighlightArea = ScriptParams[8].iParam;
    } else {
        TargetX2 = ScriptParams[3].fParam;
        TargetY2 = ScriptParams[4].fParam;
        DistanceFrom1To4 = ScriptParams[5].fParam;
        HighlightArea = ScriptParams[6].iParam;
    }

    float RadiansBetweenPoints1and4 = CGeneral::GetRadianAngleBetweenPoints(TargetX1, TargetY1, TargetX2, TargetY2) + HALF_PI;
    while (RadiansBetweenPoints1and4 < 0.0f) {
        RadiansBetweenPoints1and4 += TWO_PI;
    }
    while (RadiansBetweenPoints1and4 > TWO_PI) {
        RadiansBetweenPoints1and4 -= TWO_PI;
    }

    float TargetX3 = TargetX2 + std::sin(RadiansBetweenPoints1and4) * DistanceFrom1To4;
    float TargetY3 = TargetY2 - std::cos(RadiansBetweenPoints1and4) * DistanceFrom1To4;
    float TargetX4 = TargetX1 + std::sin(RadiansBetweenPoints1and4) * DistanceFrom1To4;
    float TargetY4 = TargetY1 - std::cos(RadiansBetweenPoints1and4) * DistanceFrom1To4;

    CVector2D vec1To2(TargetX2 - TargetX1, TargetY2 - TargetY1);
    CVector2D vec1To4(TargetX4 - TargetX1, TargetY4 - TargetY1);

    float DistanceFrom1To2 = vec1To2.Magnitude();
    float DistanceFrom1To4Test = vec1To4.Magnitude();

    if (!SkipRestOfCheck) {
        CVector CharPos;
        if (pPed->bInVehicle && pPed->m_pMyVehicle) {
            CharPos = pPed->m_pMyVehicle->GetPosition();
        } else {
            CharPos = pPed->GetPosition();
        }

        LatestCmpFlagResult = false;
        CVector2D vec1ToPed(CharPos.x - TargetX1, CharPos.y - TargetY1);

        vec1To2.Normalise();
        float dot1 = DotProduct2D(vec1ToPed, vec1To2);
        if (dot1 >= 0.0f && dot1 <= DistanceFrom1To2) {
            vec1To4.Normalise();
            float dot2 = DotProduct2D(vec1ToPed, vec1To4);
            if (dot2 >= 0.0f && dot2 <= DistanceFrom1To4Test) {
                if (!Do3dCheck || (CharPos.z >= TargetZ1 && CharPos.z <= TargetZ2)) {
                    switch (commandId) {
                    case COMMAND_IS_CHAR_IN_ANGLED_AREA_2D:
                    case COMMAND_IS_CHAR_STOPPED_IN_ANGLED_AREA_2D:
                    case COMMAND_IS_CHAR_IN_ANGLED_AREA_3D:
                    case COMMAND_IS_CHAR_STOPPED_IN_ANGLED_AREA_3D:
                        LatestCmpFlagResult = true;
                        break;
                    case COMMAND_IS_CHAR_IN_ANGLED_AREA_ON_FOOT_2D:
                    case COMMAND_IS_CHAR_STOPPED_IN_ANGLED_AREA_ON_FOOT_2D:
                    case COMMAND_IS_CHAR_IN_ANGLED_AREA_ON_FOOT_3D:
                    case COMMAND_IS_CHAR_STOPPED_IN_ANGLED_AREA_ON_FOOT_3D:
                        if (!pPed->bInVehicle) {
                            LatestCmpFlagResult = true;
                        }
                        break;
                    case COMMAND_IS_CHAR_IN_ANGLED_AREA_IN_CAR_2D:
                    case COMMAND_IS_CHAR_STOPPED_IN_ANGLED_AREA_IN_CAR_2D:
                    case COMMAND_IS_CHAR_IN_ANGLED_AREA_IN_CAR_3D:
                    case COMMAND_IS_CHAR_STOPPED_IN_ANGLED_AREA_IN_CAR_3D:
                        if (pPed->bInVehicle) {
                            LatestCmpFlagResult = true;
                        }
                        break;
                    default:
                        break;
                    }
                }
            }
        }
    }

    UpdateCompareFlag(LatestCmpFlagResult);

    if (HighlightArea) {
        float CentreZ = Do3dCheck ? (TargetZ1 + TargetZ2) * 0.5f : -100.0f;
        CTheScripts::HighlightImportantAngledArea(reinterpret_cast<uint32>(this) + reinterpret_cast<uint32>(m_IP),
            TargetX1,
            TargetY1,
            TargetX2,
            TargetY2,
            TargetX3,
            TargetY3,
            TargetX4,
            TargetY4,
            CentreZ);
    }

    if (CTheScripts::DbgFlag) {
        if (!Do3dCheck) {
            CTheScripts::DrawDebugAngledSquare(
                CVector2D(TargetX1, TargetY1),
                CVector2D(TargetX2, TargetY2),
                CVector2D(TargetX3, TargetY3),
                CVector2D(TargetX4, TargetY4));
        }
    }
}


// 0x4883F0
void CRunningScript::ObjectInAngledAreaCheckCommand(int32 commandId) {
    bool Do3dCheck;
    bool LatestCmpFlagResult;
    int32 HighlightArea;

    if (commandId == COMMAND_IS_OBJECT_IN_ANGLED_AREA_3D) {
        Do3dCheck = true;
        CollectParameters(9);
    } else {
        Do3dCheck = false;
        CollectParameters(7);
    }

    CObject* pObject = CPools::GetObject(ScriptParams[0].iParam);

    float TargetX1 = ScriptParams[1].fParam;
    float TargetY1 = ScriptParams[2].fParam;
    float TargetZ1 = 0.0f;
    float TargetX2;
    float TargetY2;
    float TargetZ2 = 0.0f;
    float DistanceFrom1To4;

    if (Do3dCheck) {
        TargetZ1 = ScriptParams[3].fParam;
        TargetX2 = ScriptParams[4].fParam;
        TargetY2 = ScriptParams[5].fParam;
        TargetZ2 = ScriptParams[6].fParam;
        if (TargetZ1 > TargetZ2) {
            std::swap(TargetZ1, TargetZ2);
        }
        DistanceFrom1To4 = ScriptParams[7].fParam;
        HighlightArea = ScriptParams[8].iParam;
    } else {
        TargetX2 = ScriptParams[3].fParam;
        TargetY2 = ScriptParams[4].fParam;
        DistanceFrom1To4 = ScriptParams[5].fParam;
        HighlightArea = ScriptParams[6].iParam;
    }

    float RadiansBetweenPoints1and4 = CGeneral::GetRadianAngleBetweenPoints(TargetX1, TargetY1, TargetX2, TargetY2) + HALF_PI;
    while (RadiansBetweenPoints1and4 < 0.0f) {
        RadiansBetweenPoints1and4 += TWO_PI;
    }
    while (RadiansBetweenPoints1and4 > TWO_PI) {
        RadiansBetweenPoints1and4 -= TWO_PI;
    }

    float TargetX3 = TargetX2 + std::sin(RadiansBetweenPoints1and4) * DistanceFrom1To4;
    float TargetY3 = TargetY2 - std::cos(RadiansBetweenPoints1and4) * DistanceFrom1To4;
    float TargetX4 = TargetX1 + std::sin(RadiansBetweenPoints1and4) * DistanceFrom1To4;
    float TargetY4 = TargetY1 - std::cos(RadiansBetweenPoints1and4) * DistanceFrom1To4;

    CVector2D vec1To2(TargetX2 - TargetX1, TargetY2 - TargetY1);
    CVector2D vec1To4(TargetX4 - TargetX1, TargetY4 - TargetY1);

    float DistanceFrom1To2 = vec1To2.Magnitude();
    float DistanceFrom1To4Test = vec1To4.Magnitude();

    CVector ObjectPos = pObject->GetPosition();

    LatestCmpFlagResult = false;
    CVector2D vec1ToObject(ObjectPos.x - TargetX1, ObjectPos.y - TargetY1);

    vec1To2.Normalise();
    float dot1 = DotProduct2D(vec1ToObject, vec1To2);
    if (dot1 >= 0.0f && dot1 <= DistanceFrom1To2) {
        vec1To4.Normalise();
        float dot2 = DotProduct2D(vec1ToObject, vec1To4);
        if (dot2 >= 0.0f && dot2 <= DistanceFrom1To4Test) {
            if (!Do3dCheck || (ObjectPos.z >= TargetZ1 && ObjectPos.z <= TargetZ2)) {
                LatestCmpFlagResult = true;
            }
        }
    }

    UpdateCompareFlag(LatestCmpFlagResult);

    if (HighlightArea) {
        float CentreZ = Do3dCheck ? (TargetZ1 + TargetZ2) * 0.5f : -100.0f;
        CTheScripts::HighlightImportantAngledArea(reinterpret_cast<uint32>(this) + reinterpret_cast<uint32>(m_IP),
            TargetX1,
            TargetY1,
            TargetX2,
            TargetY2,
            TargetX3,
            TargetY3,
            TargetX4,
            TargetY4,
            CentreZ);
    }

    if (CTheScripts::DbgFlag) {
        if (!Do3dCheck) {
            CTheScripts::DrawDebugAngledSquare(
                CVector2D(TargetX1, TargetY1),
                CVector2D(TargetX2, TargetY2),
                CVector2D(TargetX3, TargetY3),
                CVector2D(TargetX4, TargetY4));
        }
    }
}


// 0x488780
void CRunningScript::FlameInAngledAreaCheckCommand(int32 commandId) {
    bool Do3dCheck;
    bool LatestCmpFlagResult = false;
    int32 HighlightArea;

    if (commandId == COMMAND_IS_FLAME_IN_ANGLED_AREA_3D) {
        Do3dCheck = true;
        CollectParameters(8);
    } else {
        Do3dCheck = false;
        CollectParameters(6);
    }

    float TargetX1 = ScriptParams[0].fParam;
    float TargetY1 = ScriptParams[1].fParam;
    float TargetZ1 = 0.0f;
    float TargetX2;
    float TargetY2;
    float TargetZ2 = 0.0f;
    float DistanceFrom1To4;

    if (Do3dCheck) {
        TargetZ1 = ScriptParams[2].fParam;
        TargetX2 = ScriptParams[3].fParam;
        TargetY2 = ScriptParams[4].fParam;
        TargetZ2 = ScriptParams[5].fParam;
        if (TargetZ1 > TargetZ2) {
            std::swap(TargetZ1, TargetZ2);
        }
        DistanceFrom1To4 = ScriptParams[6].fParam;
        HighlightArea = ScriptParams[7].iParam;
    } else {
        TargetX2 = ScriptParams[2].fParam;
        TargetY2 = ScriptParams[3].fParam;
        DistanceFrom1To4 = ScriptParams[4].fParam;
        HighlightArea = ScriptParams[5].iParam;
    }

    float RadiansBetweenPoints1and4 = CGeneral::GetRadianAngleBetweenPoints(TargetX1, TargetY1, TargetX2, TargetY2) + HALF_PI;
    while (RadiansBetweenPoints1and4 < 0.0f) {
        RadiansBetweenPoints1and4 += TWO_PI;
    }
    while (RadiansBetweenPoints1and4 > TWO_PI) {
        RadiansBetweenPoints1and4 -= TWO_PI;
    }

    float TargetX3 = TargetX2 + std::sin(RadiansBetweenPoints1and4) * DistanceFrom1To4;
    float TargetY3 = TargetY2 - std::cos(RadiansBetweenPoints1and4) * DistanceFrom1To4;
    float TargetX4 = TargetX1 + std::sin(RadiansBetweenPoints1and4) * DistanceFrom1To4;
    float TargetY4 = TargetY1 - std::cos(RadiansBetweenPoints1and4) * DistanceFrom1To4;

    CVector2D vec1To2(TargetX2 - TargetX1, TargetY2 - TargetY1);
    CVector2D vec1To4(TargetX4 - TargetX1, TargetY4 - TargetY1);

    float DistanceFrom1To2 = vec1To2.Magnitude();
    float DistanceFrom1To4Test = vec1To4.Magnitude();

    uint16 FlameLoop = 0;
    do {
        if (LatestCmpFlagResult) {
            break;
        }

        CVector FlamePos;
        if (CShotInfo::GetFlameThrowerShotPosn(static_cast<uint8>(FlameLoop), FlamePos)) {
            CVector2D vec1ToFlame(FlamePos.x - TargetX1, FlamePos.y - TargetY1);

            vec1To2.Normalise();
            float dot1 = DotProduct2D(vec1ToFlame, vec1To2);
            if (dot1 >= 0.0f && dot1 <= DistanceFrom1To2) {
                vec1To4.Normalise();
                float dot2 = DotProduct2D(vec1ToFlame, vec1To4);
                if (dot2 >= 0.0f && dot2 <= DistanceFrom1To4Test) {
                    if (!Do3dCheck || (FlamePos.z >= TargetZ1 && FlamePos.z <= TargetZ2)) {
                        LatestCmpFlagResult = true;
                    }
                }
            }
        }

        ++FlameLoop;
    } while (FlameLoop < 100);

    UpdateCompareFlag(LatestCmpFlagResult);

    if (HighlightArea) {
        float CentreZ = Do3dCheck ? (TargetZ1 + TargetZ2) * 0.5f : -100.0f;
        CTheScripts::HighlightImportantAngledArea(reinterpret_cast<uint32>(this) + reinterpret_cast<uint32>(m_IP),
            TargetX1,
            TargetY1,
            TargetX2,
            TargetY2,
            TargetX3,
            TargetY3,
            TargetX4,
            TargetY4,
            CentreZ);
    }

    if (CTheScripts::DbgFlag) {
        if (!Do3dCheck) {
            CTheScripts::DrawDebugAngledSquare(
                CVector2D(TargetX1, TargetY1),
                CVector2D(TargetX2, TargetY2),
                CVector2D(TargetX3, TargetY3),
                CVector2D(TargetX4, TargetY4));
        }
    }
}


// 0x488B50
void CRunningScript::CharInAreaCheckCommand(int32 commandId) {
    bool SkipRestOfCheck = false;
    bool Do3dCheck;
    bool LatestCmpFlagResult;
    int32 HighlightArea;

    if (commandId == COMMAND_IS_CHAR_IN_AREA_3D ||
        (commandId > COMMAND_IS_CHAR_STOPPED_IN_AREA_2D && commandId <= COMMAND_IS_CHAR_STOPPED_IN_AREA_IN_CAR_3D)) {
        Do3dCheck = true;
        CollectParameters(8);
    } else {
        Do3dCheck = false;
        CollectParameters(6);
    }

    CPed* pPed = CPools::GetPed(ScriptParams[0].iParam);

    CVector CharPos;
    if (pPed->bInVehicle && pPed->m_pMyVehicle) {
        CharPos = pPed->m_pMyVehicle->GetPosition();
    } else {
        CharPos = pPed->GetPosition();
    }

    switch (commandId) {
    case COMMAND_IS_CHAR_STOPPED_IN_AREA_2D:
    case COMMAND_IS_CHAR_STOPPED_IN_AREA_ON_FOOT_2D:
    case COMMAND_IS_CHAR_STOPPED_IN_AREA_IN_CAR_2D:
    case COMMAND_IS_CHAR_STOPPED_IN_AREA_3D:
    case COMMAND_IS_CHAR_STOPPED_IN_AREA_ON_FOOT_3D:
    case COMMAND_IS_CHAR_STOPPED_IN_AREA_IN_CAR_3D:
        if (!CTheScripts::IsPedStopped(pPed)) {
            LatestCmpFlagResult = false;
            SkipRestOfCheck = true;
        }
        break;
    default:
        break;
    }

    float TargetX1 = ScriptParams[1].fParam;
    float TargetY1 = ScriptParams[2].fParam;
    float TargetZ1 = 0.0f;
    float TargetX2;
    float TargetY2;
    float TargetZ2 = 0.0f;

    if (Do3dCheck) {
        TargetZ1 = ScriptParams[3].fParam;
        TargetX2 = ScriptParams[4].fParam;
        TargetY2 = ScriptParams[5].fParam;
        TargetZ2 = ScriptParams[6].fParam;
        if (TargetZ1 > TargetZ2) {
            std::swap(TargetZ1, TargetZ2);
        }
        HighlightArea = ScriptParams[7].iParam;
    } else {
        TargetX2 = ScriptParams[3].fParam;
        TargetY2 = ScriptParams[4].fParam;
        HighlightArea = ScriptParams[5].iParam;
    }

    if (TargetX1 > TargetX2) {
        std::swap(TargetX1, TargetX2);
    }

    if (TargetY1 > TargetY2) {
        std::swap(TargetY1, TargetY2);
    }

    if (!SkipRestOfCheck) {
        LatestCmpFlagResult = false;
        bool bWithinArea = false;
        if (Do3dCheck) {
            if (CharPos.x >= TargetX1 &&
                CharPos.x <= TargetX2 &&
                CharPos.y >= TargetY1 &&
                CharPos.y <= TargetY2 &&
                CharPos.z >= TargetZ1 &&
                CharPos.z <= TargetZ2) {
                bWithinArea = true;
            }
        } else {
            if (CharPos.x >= TargetX1 &&
                CharPos.x <= TargetX2 &&
                CharPos.y >= TargetY1 &&
                CharPos.y <= TargetY2) {
                bWithinArea = true;
            }
        }

        if (bWithinArea) {
            switch (commandId) {
            case COMMAND_IS_CHAR_IN_AREA_2D:
            case COMMAND_IS_CHAR_IN_AREA_3D:
            case COMMAND_IS_CHAR_STOPPED_IN_AREA_2D:
            case COMMAND_IS_CHAR_STOPPED_IN_AREA_3D:
                LatestCmpFlagResult = true;
                break;
            case COMMAND_IS_CHAR_IN_AREA_ON_FOOT_2D:
            case COMMAND_IS_CHAR_STOPPED_IN_AREA_ON_FOOT_2D:
            case COMMAND_IS_CHAR_IN_AREA_ON_FOOT_3D:
            case COMMAND_IS_CHAR_STOPPED_IN_AREA_ON_FOOT_3D:
                if (!pPed->bInVehicle) {
                    LatestCmpFlagResult = true;
                }
                break;
            case COMMAND_IS_CHAR_IN_AREA_IN_CAR_2D:
            case COMMAND_IS_CHAR_STOPPED_IN_AREA_IN_CAR_2D:
            case COMMAND_IS_CHAR_IN_AREA_IN_CAR_3D:
            case COMMAND_IS_CHAR_STOPPED_IN_AREA_IN_CAR_3D:
                if (pPed->bInVehicle) {
                    LatestCmpFlagResult = true;
                }
                break;
            default:
                break;
            }
        }
    }

    UpdateCompareFlag(LatestCmpFlagResult);

    if (HighlightArea) {
        if (Do3dCheck) {
            CTheScripts::HighlightImportantArea(reinterpret_cast<uint32>(this) + reinterpret_cast<uint32>(m_IP),
                TargetX1,
                TargetY1,
                TargetX2,
                TargetY2,
                (TargetZ1 + TargetZ2) * 0.5f);
        } else {
            CTheScripts::HighlightImportantArea(reinterpret_cast<uint32>(this) + reinterpret_cast<uint32>(m_IP),
                TargetX1,
                TargetY1,
                TargetX2,
                TargetY2,
                -100.0f);
        }
    }

    if (CTheScripts::DbgFlag) {
        if (!Do3dCheck) {
            CTheScripts::DrawDebugSquare(
                TargetX1,
                TargetY1,
                TargetX2,
                TargetY2);
        }
    }
}


// 0x488EC0
void CRunningScript::CarInAreaCheckCommand(int32 commandId) {
    bool SkipRestOfCheck = false;
    bool Do3dCheck;
    bool LatestCmpFlagResult;
    int32 HighlightArea;

    if (commandId == COMMAND_IS_CAR_IN_AREA_3D || commandId == COMMAND_IS_CAR_STOPPED_IN_AREA_3D) {
        Do3dCheck = true;
        CollectParameters(8);
    } else {
        Do3dCheck = false;
        CollectParameters(6);
    }

    CVehicle* pVehicle = CPools::GetVehicle(ScriptParams[0].iParam);

    if (commandId >= COMMAND_IS_CAR_STOPPED_IN_AREA_2D && commandId <= COMMAND_IS_CAR_STOPPED_IN_AREA_3D &&
        !CTheScripts::IsVehicleStopped(pVehicle)) {
        LatestCmpFlagResult = false;
        SkipRestOfCheck = true;
    }

    float TargetX1 = ScriptParams[1].fParam;
    float TargetY1 = ScriptParams[2].fParam;
    float TargetZ1 = 0.0f;
    float TargetX2;
    float TargetY2;
    float TargetZ2 = 0.0f;

    if (Do3dCheck) {
        TargetZ1 = ScriptParams[3].fParam;
        TargetX2 = ScriptParams[4].fParam;
        TargetY2 = ScriptParams[5].fParam;
        TargetZ2 = ScriptParams[6].fParam;
        if (TargetZ1 > TargetZ2) {
            std::swap(TargetZ1, TargetZ2);
        }
        HighlightArea = ScriptParams[7].iParam;
    } else {
        TargetX2 = ScriptParams[3].fParam;
        TargetY2 = ScriptParams[4].fParam;
        HighlightArea = ScriptParams[5].iParam;
    }

    if (TargetX1 > TargetX2) {
        std::swap(TargetX1, TargetX2);
    }

    if (TargetY1 > TargetY2) {
        std::swap(TargetY1, TargetY2);
    }

    if (!SkipRestOfCheck) {
        CVector VehiclePos = pVehicle->GetPosition();
        LatestCmpFlagResult = false;
        if (Do3dCheck) {
            if (VehiclePos.x >= TargetX1 &&
                VehiclePos.x <= TargetX2 &&
                VehiclePos.y >= TargetY1 &&
                VehiclePos.y <= TargetY2 &&
                VehiclePos.z >= TargetZ1 &&
                VehiclePos.z <= TargetZ2) {
                LatestCmpFlagResult = true;
            }
        } else {
            if (VehiclePos.x >= TargetX1 &&
                VehiclePos.x <= TargetX2 &&
                VehiclePos.y >= TargetY1 &&
                VehiclePos.y <= TargetY2) {
                LatestCmpFlagResult = true;
            }
        }
    }

    UpdateCompareFlag(LatestCmpFlagResult);

    if (HighlightArea) {
        if (Do3dCheck) {
            CTheScripts::HighlightImportantArea(reinterpret_cast<uint32>(this) + reinterpret_cast<uint32>(m_IP),
                TargetX1,
                TargetY1,
                TargetX2,
                TargetY2,
                (TargetZ1 + TargetZ2) * 0.5f);
        } else {
            CTheScripts::HighlightImportantArea(reinterpret_cast<uint32>(this) + reinterpret_cast<uint32>(m_IP),
                TargetX1,
                TargetY1,
                TargetX2,
                TargetY2,
                -100.0f);
        }
    }

    if (CTheScripts::DbgFlag) {
        if (!Do3dCheck) {
            CTheScripts::DrawDebugSquare(
                TargetX1,
                TargetY1,
                TargetX2,
                TargetY2);
        }
    }
}


// 0x489150
void CRunningScript::ObjectInAreaCheckCommand(int32 commandId) {
    bool is3D;
    int32 HighlightArea;

    if (commandId == COMMAND_IS_OBJECT_IN_AREA_3D) {
        is3D = true;
        CollectParameters(8);
    } else {
        is3D = false;
        CollectParameters(6);
    }

    CObject* pObj = CPools::GetObject(ScriptParams[0].iParam);

    float TargetX1 = ScriptParams[1].fParam;
    float TargetY1 = ScriptParams[2].fParam;
    float TargetZ1 = 0.0f;
    float TargetX2;
    float TargetY2;
    float TargetZ2 = 0.0f;

    if (is3D) {
        TargetZ1 = ScriptParams[3].fParam;
        TargetX2 = ScriptParams[4].fParam;
        TargetY2 = ScriptParams[5].fParam;
        TargetZ2 = ScriptParams[6].fParam;
        if (TargetZ1 > TargetZ2) {
            std::swap(TargetZ1, TargetZ2);
        }
        HighlightArea = ScriptParams[7].iParam;
    } else {
        TargetX2 = ScriptParams[3].fParam;
        TargetY2 = ScriptParams[4].fParam;
        HighlightArea = ScriptParams[5].iParam;
    }

    if (TargetX1 > TargetX2) {
        std::swap(TargetX1, TargetX2);
    }

    if (TargetY1 > TargetY2) {
        std::swap(TargetY1, TargetY2);
    }

    CVector ObjectPos = pObj->GetPosition();
    bool bResult = false;
    if (is3D) {
        if (ObjectPos.x >= TargetX1 &&
            ObjectPos.x <= TargetX2 &&
            ObjectPos.y >= TargetY1 &&
            ObjectPos.y <= TargetY2 &&
            ObjectPos.z >= TargetZ1 &&
            ObjectPos.z <= TargetZ2) {
            bResult = true;
        }
    } else {
        if (ObjectPos.x >= TargetX1 &&
            ObjectPos.x <= TargetX2 &&
            ObjectPos.y >= TargetY1 &&
            ObjectPos.y <= TargetY2) {
            bResult = true;
        }
    }

    UpdateCompareFlag(bResult);

    if (HighlightArea) {
        if (is3D) {
            CTheScripts::HighlightImportantArea(reinterpret_cast<uint32>(this) + reinterpret_cast<uint32>(m_IP),
                TargetX1,
                TargetY1,
                TargetX2,
                TargetY2,
                (TargetZ1 + TargetZ2) * 0.5f);
        } else {
            CTheScripts::HighlightImportantArea(reinterpret_cast<uint32>(this) + reinterpret_cast<uint32>(m_IP),
                TargetX1,
                TargetY1,
                TargetX2,
                TargetY2,
                -100.0f);
        }
    }

    if (CTheScripts::DbgFlag) {
        if (!is3D) {
            CTheScripts::DrawDebugSquare(
                TargetX1,
                TargetY1,
                TargetX2,
                TargetY2);
        }
    }
}



void CRunningScript::HighlightImportantArea(CVector2D from, CVector2D to, float z) {
    CTheScripts::HighlightImportantArea(reinterpret_cast<int32>(this) + reinterpret_cast<int32>(m_IP), from.x, from.y, to.x, to.y, z);
}



void CRunningScript::HighlightImportantArea(CRect area, float z) {
    HighlightImportantArea(area.GetTopLeft(), area.GetBottomRight(), z);
}



void CRunningScript::HighlightImportantArea(CVector from, CVector to) {
    HighlightImportantArea(CVector2D{ from }, CVector2D{ to }, (from.z + to.z) / 2.f);
}



void CRunningScript::HighlightImportantAngledArea(uint32 id, CVector2D a, CVector2D b, CVector2D c, CVector2D d) {
    NOTSA_UNREACHABLE(); // Fuck this, we dont need it!
}


// Usage:
//  const auto pos = FindPlayerPed()->GetPosition() + CVector{ 0.0f, 0.f, 1.0f };
//  CTheScripts::DrawDebugSquare(pos.x + 5.f, pos.y, pos.x, pos.y + 5.f);
// 0x486840
void CTheScripts::DrawDebugSquare(float x1, float y1, float x2, float y2) {
    CColPoint colPoint{};
    CEntity*  colEntity;

    CVector p1 { x1, y1, -1000.0f };
    CWorld::ProcessVerticalLine(p1, 1000.0f, colPoint, colEntity, true, false, false, false, true, false, nullptr);
    p1.z = colPoint.m_vecPoint.z + 2.0f;

    CVector p2 { x2, y1, -1000.0f };
    CWorld::ProcessVerticalLine(p2, 1000.0f, colPoint, colEntity, true, false, false, false, true, false, nullptr);
    p2.z = colPoint.m_vecPoint.z + 2.0f;

    CVector p3 { x2, y2, -1000.0f };
    CWorld::ProcessVerticalLine(p3, 1000.0f, colPoint, colEntity, true, false, false, false, true, false, nullptr);
    p3.z = colPoint.m_vecPoint.z + 2.0f;

    CVector p4 { x1, y2, -1000.0f };
    CWorld::ProcessVerticalLine(p4, 1000.0f, colPoint, colEntity, true, false, false, false, true, false, nullptr);
    p4.z = colPoint.m_vecPoint.z + 2.0f; // FIX_BUGS: missing in original code 🤷

    ScriptDebugLine3D(p1, p2, DbgLineColour, DbgLineColour);
    ScriptDebugLine3D(p2, p3, DbgLineColour, DbgLineColour);
    ScriptDebugLine3D(p3, p4, DbgLineColour, DbgLineColour);
    ScriptDebugLine3D(p4, p1, DbgLineColour, DbgLineColour);
}


// float infX, float infY, float sup.x, float supY, float rotSupX, float rotSupY, float rotInfX, float rotInfY
// 0x486990
void CTheScripts::DrawDebugAngledSquare(const CVector2D& inf, const CVector2D& sup, const CVector2D& rotSup, const CVector2D& rotInf) {
    CColPoint colPoint{};
    CEntity*  colEntity;

    CVector p1 { inf.x, inf.y, -1000.0f };
    CWorld::ProcessVerticalLine(p1, 1000.0f, colPoint, colEntity, true, false, false, false, true, false, nullptr);
    p1.z = colPoint.m_vecPoint.z + 2.0f;

    CVector p2 { sup.x, sup.y, -1000.0f };
    CWorld::ProcessVerticalLine(p2, 1000.0f, colPoint, colEntity, true, false, false, false, true, false, nullptr);
    p2.z = colPoint.m_vecPoint.z + 2.0f;

    CVector p3 { rotSup.x, rotSup.y, -1000.0f };
    CWorld::ProcessVerticalLine(p3, 1000.0f, colPoint, colEntity, true, false, false, false, true, false, nullptr);
    p3.z = colPoint.m_vecPoint.z + 2.0f;

    CVector p4 { rotInf.x, rotInf.y, -1000.0f };
    CWorld::ProcessVerticalLine(p4, 1000.0f, colPoint, colEntity, true, false, false, false, true, false, nullptr);
    p4.z = colPoint.m_vecPoint.z + 2.0f;

    ScriptDebugLine3D(p1, p2, DbgLineColour, DbgLineColour);
    ScriptDebugLine3D(p2, p3, DbgLineColour, DbgLineColour);
    ScriptDebugLine3D(p3, p4, DbgLineColour, DbgLineColour);
    ScriptDebugLine3D(p4, p1, DbgLineColour, DbgLineColour);
}


// (float infX, float infY, float infZ, float supX, float supY, float supZ)
void CTheScripts::DrawDebugCube(const CVector& inf, const CVector& sup) {
    ScriptDebugLine3D({ inf.x, inf.y, inf.z }, { sup.x, inf.y, inf.z }, DbgLineColour, DbgLineColour);
    ScriptDebugLine3D({ sup.x, inf.y, inf.z }, { sup.x, sup.y, inf.z }, DbgLineColour, DbgLineColour);
    ScriptDebugLine3D({ sup.x, sup.y, inf.z }, { inf.x, sup.y, inf.z }, DbgLineColour, DbgLineColour);
    ScriptDebugLine3D({ inf.x, sup.y, inf.z }, { inf.x, inf.y, inf.z }, DbgLineColour, DbgLineColour);
    ScriptDebugLine3D({ inf.x, inf.y, sup.z }, { sup.x, inf.y, sup.z }, DbgLineColour, DbgLineColour);
    ScriptDebugLine3D({ sup.x, inf.y, sup.z }, { sup.x, sup.y, sup.z }, DbgLineColour, DbgLineColour);
    ScriptDebugLine3D({ sup.x, sup.y, sup.z }, { inf.x, sup.y, sup.z }, DbgLineColour, DbgLineColour);
    ScriptDebugLine3D({ inf.x, sup.y, sup.z }, { inf.x, inf.y, sup.z }, DbgLineColour, DbgLineColour);
    ScriptDebugLine3D({ inf.x, inf.y, sup.z }, { inf.x, inf.y, inf.z }, DbgLineColour, DbgLineColour);
    ScriptDebugLine3D({ sup.x, inf.y, sup.z }, { sup.x, inf.y, inf.z }, DbgLineColour, DbgLineColour);
    ScriptDebugLine3D({ sup.x, sup.y, sup.z }, { sup.x, sup.y, inf.z }, DbgLineColour, DbgLineColour);
    ScriptDebugLine3D({ inf.x, sup.y, sup.z }, { inf.x, sup.y, inf.z }, DbgLineColour, DbgLineColour);
}


// (float infX, float infY, float infZ, float supX, float supY, float supZ, float rotSup, float rotSup, float rotInf, float rotInf)
void CTheScripts::DrawDebugAngledCube(const CVector& inf, const CVector& sup, const CVector2D& rotSup, const CVector2D& rotInf) {
    ScriptDebugLine3D({ inf.x,    inf.y,    inf.z    }, { sup.x,    inf.y,    inf.z }, DbgLineColour, DbgLineColour);
    ScriptDebugLine3D({ sup.x,    inf.y,    inf.z    }, { rotSup.x, rotSup.y, inf.z }, DbgLineColour, DbgLineColour);
    ScriptDebugLine3D({ rotSup.x, rotSup.y, inf.z    }, { rotInf.x, rotInf.y, inf.z }, DbgLineColour, DbgLineColour);
    ScriptDebugLine3D({ rotInf.x, rotInf.y, inf.z    }, { inf.x,    inf.y,    inf.z }, DbgLineColour, DbgLineColour);
    ScriptDebugLine3D({ inf.x,    inf.y,    sup.z    }, { sup.x,    inf.y,    sup.z }, DbgLineColour, DbgLineColour);
    ScriptDebugLine3D({ sup.x,    inf.y,    sup.z    }, { rotSup.x, rotSup.y, sup.z }, DbgLineColour, DbgLineColour);
    ScriptDebugLine3D({ rotSup.x, rotSup.y, rotInf.x }, { rotInf.y, sup.y,    sup.z }, DbgLineColour, DbgLineColour);
    ScriptDebugLine3D({ rotInf.x, rotInf.y, sup.z    }, { inf.x,    inf.y,    sup.z }, DbgLineColour, DbgLineColour);
    ScriptDebugLine3D({ inf.x,    inf.y,    sup.z    }, { inf.x,    inf.y,    inf.z }, DbgLineColour, DbgLineColour);
    ScriptDebugLine3D({ sup.x,    inf.y,    sup.z    }, { sup.x,    inf.y,    inf.z }, DbgLineColour, DbgLineColour);
    ScriptDebugLine3D({ rotSup.x, rotSup.y, sup.z    }, { rotSup.x, rotSup.y, inf.z }, DbgLineColour, DbgLineColour);
    ScriptDebugLine3D({ rotInf.x, rotInf.y, sup.z    }, { rotInf.x, rotInf.y, inf.z }, DbgLineColour, DbgLineColour);
}


// Usage:
//   const auto pos = FindPlayerPed()->GetPosition() + CVector{ 0.0f, 0.f, 1.0f };
//   CTheScripts::ScriptDebugCircle2D(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 - 100, 50.f, 50.f, HudColour.GetRGB(HUD_COLOUR_RED).ToInt());
// 0x485C20
void CTheScripts::ScriptDebugCircle2D(float x, float y, float width, float height, CRGBA color) {
    return plugin::Call<0x485C20, float, float, float, float, CRGBA>(x, y, width, height, color);

    // untested
    RwRenderStateSet(rwRENDERSTATEZWRITEENABLE,      RWRSTATE(TRUE));
    RwRenderStateSet(rwRENDERSTATEZTESTENABLE,       RWRSTATE(FALSE));
    RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, RWRSTATE(TRUE));
    RwRenderStateSet(rwRENDERSTATESRCBLEND,          RWRSTATE(rwBLENDSRCALPHA));
    RwRenderStateSet(rwRENDERSTATEDESTBLEND,         RWRSTATE(rwBLENDINVSRCALPHA));
    RwRenderStateSet(rwRENDERSTATETEXTUREFILTER,     RWRSTATE(rwFILTERLINEAR));
    RwRenderStateSet(rwRENDERSTATETEXTURERASTER,     RWRSTATE(NULL));

    const auto iters = 16;
    auto part = (CTimer::GetTimeInMS() >> 6) % iters;
    for (auto i = 0; i < iters; i++) {
        RwIm2DVertex vertex{
            .x = std::sin((float)i * PI / 8.0f) * width + x,
            .y = std::cos((float)i * PI / 8.0f) * height + y
        };
        if (part == i)
            vertex.emissiveColor = (3 * color.g / 4) | (((3 * color.b / 4) | (((3 * color.a / 4) | ((3 * color.r / 4) << 8)) << 8)) << 8); // todo: (3 * color / 4).ToIntARGB();
        else
            vertex.emissiveColor = color.ToIntARGB();
        RwIm2DVertex vertices[2] = { vertex, vertex };
        RwIm2DRenderLine(vertices, std::size(vertices), 0, 1); // todo: RwIm2DRenderLine_BUGFIX
    }

    RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, RWRSTATE(FALSE));
    RwRenderStateSet(rwRENDERSTATEZTESTENABLE,       RWRSTATE(TRUE));
    RwRenderStateSet(rwRENDERSTATEZWRITEENABLE,      RWRSTATE(TRUE));
}


// 0x485DE0
void CTheScripts::ScriptDebugLine3D(const CVector& start, const CVector& end, uint32 color1, uint32 color2) {
#ifndef FINAL
    if (NumScriptDebugLines >= MAX_NUM_STORED_LINES)
        return;

    auto& line = aStoredLines[NumScriptDebugLines];
    line.vecInf = start;
    line.vecSup = end;
    line.color1 = color1;
    line.color2 = color2;
    NumScriptDebugLines++;
#endif
}


// 0x0 NOP
void CTheScripts::RenderTheScriptDebugLines() {
#ifndef FINAL
    RwRenderStateSet(rwRENDERSTATEZWRITEENABLE,      RWRSTATE(TRUE));
    RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, RWRSTATE(TRUE));
    for (auto& line : std::span{ aStoredLines, NumScriptDebugLines }) {
        CLines::RenderLineWithClipping(line.vecInf, line.vecSup, line.color1, line.color2);
    }
    NumScriptDebugLines = 0;
    RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, RWRSTATE(FALSE));
#endif
}


// 0x486B00
void CTheScripts::ClearSpaceForMissionEntity(const CVector& pos, CEntity* ourEntity) {
    std::array<CEntity*, 16> colEntities{};
    int16                    numColliding{};

    CWorld::FindObjectsKindaColliding(
        pos,
        ourEntity->GetModelInfo()->GetColModel()->GetBoundRadius(),
        false,
        &numColliding,
        (int16)colEntities.max_size(),
        colEntities.data(),
        false,
        true,
        true,
        false,
        false
    );

    auto* ourColData = ourEntity->GetColData();
    if (!ourColData) {
        return;
    }

    // Disable suspension lines of vehicles.
    const auto cdNumLines = std::exchange(ourColData->m_nNumLines, 0);

    for (auto& entity : std::span{ colEntities.data(), (size_t)numColliding }) {
        if (!entity || entity == ourEntity || (entity->GetIsTypePed() && entity->AsPed()->IsInVehicle())) {
            continue;
        }

        std::array<CColPoint, 32> colPoints{};
        const auto                numCollisions = CCollision::ProcessColModels(
            ourEntity->GetMatrix(),
            *ourEntity->GetColModel(),
            entity->GetMatrix(),
            *entity->GetColModel(),
            colPoints,
            nullptr,
            nullptr,
            false
        );

        if (numCollisions <= 0) {
            continue;
        }

        if (entity->GetIsTypeVehicle()) {
            auto* vehicle = entity->AsVehicle();
            if (vehicle->vehicleFlags.bIsLocked || !vehicle->CanBeDeleted()) {
                continue;
            }

            if (auto& driver = vehicle->m_pDriver) {
                CPopulation::RemovePed(driver);
                CEntity::SafeCleanUpRef(driver);
            }

            // Need to use raw pointer here instead of a reference - the m_aPassengers array is modifed in RemovePassenger() and we would crash in RemovePed afterwards
            for (const auto passenger : vehicle->GetPassengers()) {
                if (passenger) {
                    vehicle->RemovePassenger(passenger);
                    CPopulation::RemovePed(passenger);
                }
            }

            CCarCtrl::RemoveFromInterestingVehicleList(vehicle);
            CWorld::Remove(vehicle);
            delete vehicle;
        }

        if (entity->GetIsTypePed() && !entity->AsPed()->IsPlayer() && entity->AsPed()->CanBeDeleted()) {
            CPopulation::RemovePed(entity->AsPed());
        }
    }
    ourColData->m_nNumLines = cdNumLines;
}


// 0x485E00
void CTheScripts::HighlightImportantArea(uint32 id, float x1, float y1, float x2, float y2, float z) {
    CVector2D inf, sup;
    if (x1 < x2) {
        inf.x = x1;
        sup.x = x2;
    } else {
        inf.x = x2;
        sup.x = x1;
    }

    if (y1 < y2) {
        inf.y = y1;
        sup.y = y2;
    } else {
        inf.y = y2;
        sup.y = y1;
    }

    CVector center;
    center.x = (inf.x + sup.x) / 2;
    center.y = (inf.y + sup.y) / 2;
    center.z = (z <= MAP_Z_LOW_LIMIT) ? CWorld::FindGroundZForCoord(center.x, center.y) : z;
    CShadows::RenderIndicatorShadow(id, SHADOW_ADDITIVE, nullptr, center, sup.x - center.x, 0.0f, 0.0f, center.y - sup.y, 0);
}


// 0x485EF0
void CTheScripts::HighlightImportantAngledArea(uint32 id, float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4, float z) {
    CVector2D inf, sup;
    float x, y;

    x = (x1 + x2) / 2;
    y = (y1 + y2) / 2;
    sup.x = inf.x = x;
    sup.y = inf.y = y;

    x = (x2 + x3) / 2;
    y = (y2 + y3) / 2;
    inf.x = std::min(inf.x, x);
    sup.x = std::max(sup.x, x);
    inf.y = std::min(inf.y, y);
    sup.y = std::max(sup.y, y);

    x = (x3 + x4) / 2;
    y = (y3 + y4) / 2;
    inf.x = std::min(inf.x, x);
    sup.x = std::max(sup.x, x);
    inf.y = std::min(inf.y, y);
    sup.y = std::max(sup.y, y);

    x = (x4 + x1) / 2;
    y = (y4 + y1) / 2;
    inf.x = std::min(inf.x, x);
    sup.x = std::max(sup.x, x);
    inf.y = std::min(inf.y, y);
    sup.y = std::max(sup.y, y);

    CVector center;
    center.x = (inf.x + sup.x) / 2;
    center.y = (inf.y + sup.y) / 2;
    center.z = (z <= MAP_Z_LOW_LIMIT) ? CWorld::FindGroundZForCoord(center.x, center.y) : z;
    CShadows::RenderIndicatorShadow(id, SHADOW_ADDITIVE, nullptr, center, sup.x - center.x, 0.0f, 0.0f, center.y - sup.y, 0);
}


// 0x486110
bool CTheScripts::IsPedStopped(CPed* ped) {
    if (ped->IsInVehicle()) {
        return CTimer::GetTimeStep() / 100.f >= ped->m_pMyVehicle->m_fMovingSpeed;
    }
    if (!ped->IsPedStandingInPlace()) {
        return false;
    }
    if (ped->IsPlayer()) {
        if (RpAnimBlendClumpGetAssociation(ped->GetRpClump(), { ANIM_ID_RUN_STOP, ANIM_ID_RUN_STOPR, ANIM_ID_JUMP_LAUNCH, ANIM_ID_JUMP_GLIDE })) {
            return false;
        }
    }
    if (ped->bIsLanding || ped->bIsInTheAir || !ped->bIsStanding) {
        return false;
    }
    if (ped->m_vecAnimMovingShiftLocal.IsZero()) {
        return true;
    }
    return false;
}


// 0x4861F0
bool CTheScripts::IsVehicleStopped(CVehicle* veh) {
    return std::max(CTimer::GetTimeStep(), CTimer::ms_fOldTimeStep) / 100.0f >= veh->m_fMovingSpeed;
}


// 0x486240
void CTheScripts::RemoveThisPed(CPed* ped) {
    if (!ped) {
        return;
    }

    if (auto* veh = ped->GetVehicleIfInOne()) {
        if (veh->IsDriver(ped)) {
            veh->RemoveDriver(false);

            if (veh->m_nDoorLock == eCarLock::CARLOCK_COP_CAR) {
                veh->m_nDoorLock = eCarLock::CARLOCK_UNLOCKED;
            }

            if (ped->IsCop() && veh->IsLawEnforcementVehicle()) {
                veh->ChangeLawEnforcerState(false);
            }
        } else {
            veh->RemovePassenger(ped);
        }
    }

    const auto isMissionChar = ped->GetCreatedBy() == ePedCreatedBy::PED_MISSION;

    CWorld::RemoveReferencesToDeletedObject(ped);
    delete ped;

    if (isMissionChar) {
        --CPopulation::ms_nTotalMissionPeds;
    }
}



// 0x486300
// ASM Match
void CTheScripts::CleanUpThisPed(CPed* pPed) {
    if (!pPed) {
        return;
    }

    if (pPed->GetCharCreatedBy() == ePedCreatedBy::PED_MISSION) {
        pPed->SetCharCreatedBy(ePedCreatedBy::PED_GAME);
        if (pPed->bKeepTasksAfterCleanUp) {
            --CPopulation::ms_nTotalMissionPeds;
            return;
        }

        bool bTellPedToWander = true;
        if (pPed->bInVehicle && pPed->m_pMyVehicle && pPed == pPed->m_pMyVehicle->m_pDriver) {
            bTellPedToWander = false;
            if (pPed->m_pMyVehicle->GetVehicleType() == VEHICLE_TYPE_HELI) {
                auto* pHeli = static_cast<CHeli*>(pPed->m_pMyVehicle);
                pHeli->m_autoPilot.SetCarMissionUnlessCrashing(MISSION_HELI_FLYTOCOORS);
                pHeli->m_autoPilot.TargetCoors = CVector(10000.0f, -10000.0f, 1000.0f);
                pHeli->m_MinHeightAboveTerrain = 1000.0f;
                pHeli->m_LowestFlightHeight = 1000.0f;
            } else if (pPed->m_pMyVehicle->GetVehicleType() == VEHICLE_TYPE_PLANE) {
                auto* pPlane = static_cast<CPlane*>(pPed->m_pMyVehicle);
                pPlane->m_autoPilot.SetCarMissionUnlessCrashing(MISSION_PLANE_FLYTOCOORS);
                pPlane->m_autoPilot.TargetCoors = CVector(10000.0f, 10000.0f, 1000.0f);
                pPlane->m_MinHeightAboveTerrain = 1000.0f;
                pPlane->m_LowestFlightHeight = 1000.0f;
            } else if (pPed->m_pMyVehicle->GetBaseVehicleType() == VEHICLE_TYPE_AUTOMOBILE || pPed->m_pMyVehicle->GetBaseVehicleType() == VEHICLE_TYPE_BIKE) {
                CCarCtrl::JoinCarWithRoadSystem(pPed->m_pMyVehicle);
                pPed->m_pMyVehicle->m_autoPilot.SetCarMissionUnlessCrashing(MISSION_CRUISE);
            }
        }

        pPed->bStayInSamePlace = false;
        if (auto* pPedGroup = CPedGroups::GetPedsGroup(pPed)) {
            if (pPedGroup->GetMembership().IsFollower(pPed)) {
                pPedGroup->GetMembership().RemoveMember(pPed);
            }
        }

        if (!bTellPedToWander) {
            --CPopulation::ms_nTotalMissionPeds;
            return;
        }

        if (pPed->bInVehicle && pPed->m_pMyVehicle) {
            if (auto* pEvent = pPed->GetEventGroup().GetEventOfType(EVENT_SCRIPT_COMMAND)) {
                auto* pTask = static_cast<CEventScriptCommand*>(pEvent)->m_task;
                if (pTask && pTask->GetTaskType() == TASK_COMPLEX_SEQUENCE) {
                    --CPopulation::ms_nTotalMissionPeds;
                    return;
                }
            }

            if (auto* pTaskPrimary = pPed->GetTaskManager().GetTaskPrimary(TASK_PRIMARY_PRIMARY)) {
                if (pTaskPrimary->GetTaskType() == TASK_COMPLEX_SEQUENCE) {
                    --CPopulation::ms_nTotalMissionPeds;
                    return;
                }
            }

            auto* pTaskSequence = new CTaskComplexSequence();
            auto* pTaskLeaveAnyCar = new CTaskComplexLeaveAnyCar(0, true, false);
            auto* pTaskWander = CTaskComplexWander::GetWanderTaskByPedType(pPed);
            pTaskSequence->AddTask(pTaskLeaveAnyCar);
            pTaskSequence->AddTask(pTaskWander);
            CEventScriptCommand event(TASK_PRIMARY_PRIMARY, pTaskSequence, false);
            pPed->GetEventGroup().Add(&event, false);
        } else {
            if (auto* pEvent = pPed->GetEventGroup().GetEventOfType(EVENT_SCRIPT_COMMAND)) {
                auto* pTask = static_cast<CEventScriptCommand*>(pEvent)->m_task;
                if (pTask && pTask->GetTaskType() == TASK_COMPLEX_WANDER) {
                    --CPopulation::ms_nTotalMissionPeds;
                    return;
                }
            }

            if (auto* pTaskPrimary = pPed->GetTaskManager().GetTaskPrimary(TASK_PRIMARY_PRIMARY)) {
                if (pTaskPrimary->GetTaskType() == TASK_COMPLEX_WANDER) {
                    --CPopulation::ms_nTotalMissionPeds;
                    return;
                }
            }

            auto* pTaskWander = CTaskComplexWander::GetWanderTaskByPedType(pPed);
            CEventScriptCommand event(TASK_PRIMARY_PRIMARY, pTaskWander, false);
            pPed->GetEventGroup().Add(&event, false);
        }

        --CPopulation::ms_nTotalMissionPeds;
    }
}

// 0x486670
void CTheScripts::CleanUpThisVehicle(CVehicle* vehicle) {
    if (!vehicle || !vehicle->IsCreatedBy(eVehicleCreatedBy::MISSION_VEHICLE)) {
        return;
    }

    vehicle->m_nPhysicalFlags.bCoorsFrozenByScript        = false;
    vehicle->m_nPhysicalFlags.bInfiniteMass = false;
    vehicle->vehicleFlags.bIsLocked               = false;

    CCarCtrl::RemoveFromInterestingVehicleList(vehicle);
    CVehicleRecording::StopPlaybackRecordedCar(vehicle);
    vehicle->SetVehicleCreatedBy(eVehicleCreatedBy::RANDOM_VEHICLE);
}


// 0x4866C0
void CTheScripts::CleanUpThisObject(CObject* obj) {
    if (!obj) {
        return;
    }

    if (obj->IsMissionObject()) {
        obj->m_nObjectType                = OBJECT_TEMPORARY;
        obj->m_nRemovalTime               = CTimer::GetTimeInMS() + 20'000'000;
        obj->m_nRefModelIndex             = -1;
        obj->m_nObjectFlags.bParentIsACar = false;
        CObject::nNoTempObjects++;
    }
}


// 0x486720
void CTheScripts::ReadObjectNamesFromScript() {
    auto* usedObjs = GetSCMChunk<tSCMUsedObjectsChunk>();

    NOTSA_LOG_TRACE("Number of used objects: {}", usedObjs->m_NumberOfUsedObjects);
    NumberOfUsedObjects = usedObjs->m_NumberOfUsedObjects;
    assert(NumberOfUsedObjects < std::size(UsedObjectArray));

    for (auto&& [i, name] : rngv::enumerate(usedObjs->GetObjectNames())) {
        UsedObjectArray[i].nModelIndex = 0; // To be updated via UpdateObjectIndices.
        std::memcpy(UsedObjectArray[i].szModelName, name, sizeof(name));

        NOTSA_LOG_TRACE("Script object #{}: \"{}\"", i, usedObjs->m_UsedObjectNames[i]);
    }
}


// 0x486780
void CTheScripts::UpdateObjectIndices() {
    // First one is ignored because it's empty.
    for (auto& obj : UsedObjectArray | std::views::drop(1)) {
        CModelInfo::GetModelInfo(obj.szModelName, &obj.nModelIndex);
    }
}


// 0x4867C0
void CTheScripts::ReadMultiScriptFileOffsetsFromScript() {
    auto* sfi = GetSCMChunk<tSCMScriptFileInfoChunk>();

    NOTSA_LOG_TRACE("Main script size: {}", sfi->m_MainScriptSize);
    NOTSA_LOG_TRACE("Largest mission size: {}", sfi->m_LargestMissionScriptSize);
    NOTSA_LOG_TRACE("Number of mission scripts: {}", sfi->m_NumberOfMissionScripts);
    NOTSA_LOG_TRACE("Number of exclusive mission script: {}", sfi->m_NumberOfExclusiveMissionScripts);
    NOTSA_LOG_TRACE("Largest num of mission script local vars: {}", sfi->m_LargestNumberOfMissionScriptLocalVars);

    MainScriptSize                             = sfi->m_MainScriptSize;
    LargestMissionScriptSize                   = sfi->m_LargestMissionScriptSize;
    NumberOfExclusiveMissionScripts            = sfi->m_NumberOfExclusiveMissionScripts;
    NumberOfMissionScripts                     = sfi->m_NumberOfMissionScripts;
    LargestNumberOfMissionScriptLocalVariables = sfi->m_LargestNumberOfMissionScriptLocalVars;

    for (const auto&& [i, missionOffset] : rngv::enumerate(sfi->GetMissionOffsets())) {
        MultiScriptArray[i] = missionOffset;
    }
}

void InjectHooks_Script5() {
    {
        RH_ScopedClass(CRunningScript);
        RH_ScopedCategory("Scripts");

        RH_ScopedInstall(UpdateCompareFlag, 0x4859D0, { .stackArguments = 1 });
        RH_ScopedInstall(DoDeathArrestCheck, 0x485A50);
        RH_ScopedInstall(LocateCarCommand, 0x487A20);
        RH_ScopedInstall(LocateCharCommand, 0x486D80);
        RH_ScopedInstall(LocateObjectCommand, 0x487D10);
        RH_ScopedInstall(LocateCharCarCommand, 0x487420);
        RH_ScopedInstall(LocateCharCharCommand, 0x4870F0);
        RH_ScopedInstall(LocateCharObjectCommand, 0x487720);
        RH_ScopedInstall(CarInAreaCheckCommand, 0x488EC0);
        RH_ScopedInstall(CharInAreaCheckCommand, 0x488B50);
        RH_ScopedInstall(ObjectInAreaCheckCommand, 0x489150);
        RH_ScopedInstall(CharInAngledAreaCheckCommand, 0x487F60);
        RH_ScopedInstall(FlameInAngledAreaCheckCommand, 0x488780);
        RH_ScopedInstall(ObjectInAngledAreaCheckCommand, 0x4883F0);
    }
    {
        RH_ScopedClass(CTheScripts);
        RH_ScopedCategory("Scripts");

        RH_ScopedInstall(ReadObjectNamesFromScript, 0x486720);
        RH_ScopedInstall(UpdateObjectIndices, 0x486780);
        RH_ScopedInstall(ReadMultiScriptFileOffsetsFromScript, 0x4867C0);
        RH_ScopedInstall(CleanUpThisObject, 0x4866C0);
        RH_ScopedInstall(CleanUpThisPed, 0x486300);
        RH_ScopedInstall(CleanUpThisVehicle, 0x486670);
        RH_ScopedInstall(ClearSpaceForMissionEntity, 0x486B00);
        RH_ScopedInstall(RemoveThisPed, 0x486240);
        RH_ScopedInstall(IsPedStopped, 0x486110);
        RH_ScopedInstall(IsVehicleStopped, 0x4861F0);
        RH_ScopedOverloadedInstall(DrawDebugSquare, "", 0x486840, void(*)(float,float,float,float));
        RH_ScopedInstall(DrawDebugAngledSquare, 0x486990);
        RH_ScopedInstall(ScriptDebugCircle2D, 0x485C20);
        RH_ScopedOverloadedInstall(HighlightImportantArea, "", 0x485E00, void(*)(uint32,float,float,float,float,float));
        RH_ScopedInstall(HighlightImportantAngledArea, 0x485EF0);
        RH_ScopedInstall(ScriptDebugLine3D, 0x485DE0);
    }
}
