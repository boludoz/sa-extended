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
    plugin::CallMethod<0x486D80, CRunningScript*, int32>(this, commandId);
}


// 0x4870F0
void CRunningScript::LocateCharCharCommand(int32 commandId) {
    plugin::CallMethod<0x4870F0, CRunningScript*, int32>(this, commandId);
}


// 0x487420
void CRunningScript::LocateCharCarCommand(int32 commandId) {
    plugin::CallMethod<0x487420, CRunningScript*, int32>(this, commandId);
}


// 0x487720
void CRunningScript::LocateCharObjectCommand(int32 commandId) {
    plugin::CallMethod<0x487720, CRunningScript*, int32>(this, commandId);
}


// 0x487A20
void CRunningScript::LocateCarCommand(int32 commandId) {
    plugin::CallMethod<0x487A20, CRunningScript*, int32>(this, commandId);
}


// 0x487D10
void CRunningScript::LocateObjectCommand(int32 commandId) {
    plugin::CallMethod<0x487D10, CRunningScript*, int32>(this, commandId);
}


// 0x487F60
void CRunningScript::CharInAngledAreaCheckCommand(int32 commandId) {
    plugin::CallMethod<0x487F60, CRunningScript*, int32>(this, commandId);
}


// 0x4883F0
void CRunningScript::ObjectInAngledAreaCheckCommand(int32 commandId) {
    plugin::CallMethod<0x4883F0, CRunningScript*, int32>(this, commandId);
}


// 0x488780
void CRunningScript::FlameInAngledAreaCheckCommand(int32 commandId) {
    plugin::CallMethod<0x488780, CRunningScript*, int32>(this, commandId);
}


// 0x488B50
void CRunningScript::CharInAreaCheckCommand(int32 commandId) {
    plugin::CallMethod<0x488B50, CRunningScript*, int32>(this, commandId);
}


// 0x488EC0
void CRunningScript::CarInAreaCheckCommand(int32 commandId) {
    plugin::CallMethod<0x488EC0, CRunningScript*, int32>(this, commandId);
}


// 0x489150
void CRunningScript::ObjectInAreaCheckCommand(int32 commandId) {
    plugin::CallMethod<0x489150, CRunningScript*, int32>(this, commandId);
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
        return CTimer::GetTimeStep() / 100.f >= ped->m_pVehicle->m_fMovingSpeed;
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
// TODO: test
void CTheScripts::CleanUpThisPed(CPed* ped) {
    if (!ped || ped->IsCreatedByMission()) {
        return;
    }

    ped->SetCharCreatedBy(ePedCreatedBy::PED_GAME);
    if (ped->bKeepTasksAfterCleanUp) {
        return;
    }

    notsa::ScopeGuard _([]() {
        --CPopulation::ms_nTotalMissionPeds;
    });

    if (auto* veh = ped->GetVehicleIfInOne(); veh && veh->IsDriver(ped)) {
        const auto FixMission = [veh](eCarMission fix) {
            auto& mis = veh->m_autoPilot.Mission;
            if (mis != MISSION_PLANE_CRASH_AND_BURN && mis != MISSION_HELI_CRASH_AND_BURN) {
                mis = fix;
            }
        };

        switch (veh->GetType()) {
        case eVehicleType::VEHICLE_TYPE_HELI: {
            FixMission(MISSION_HELI_FLYTOCOORS);

            veh->m_autoPilot.TargetCoors = CVector{ 10'000.0f, -10'000.0f, 1'000.0f };
            veh->AsHeli()->m_MinHeightAboveTerrain = 1000.0f;
            veh->AsHeli()->m_LowestFlightHeight    = 1000.0f;
            break;
        }
        case eVehicleType::VEHICLE_TYPE_PLANE: {
            FixMission(MISSION_PLANE_FLYTOCOORS);

            veh->m_autoPilot.TargetCoors = CVector{ 10'000.0f, 10'000.0f, 1'000.0f };
            veh->AsPlane()->m_minAltitude          = 1000.0f;
            veh->AsPlane()->m_maxAltitude          = 1000.0f;
            break;
        }
        default:
            if (veh->IsSubAutomobile() || veh->IsSubBike()) {
                CCarCtrl::JoinCarWithRoadSystem(veh);
                FixMission(MISSION_CRUISE);
            }
        }

        // Quick return: The captain goes down with the ship.
        ped->bStayInSamePlace = false; // ???

        if (auto* group = CPedGroups::GetPedsGroup(ped)) {
            if (auto& member = group->GetMembership(); member.IsFollower(ped)) {
                member.RemoveMember(ped);
            }
        }
        return;
    }
    ped->bStayInSamePlace = false; // ???

    if (auto* group = CPedGroups::GetPedsGroup(ped)) {
        if (auto& member = group->GetMembership(); member.IsFollower(ped)) {
            member.RemoveMember(ped);
        }
    }

    const auto CheckTaskExists = [ped](eTaskType type) {
        if (auto* event = ped->GetEventGroup().GetEventOfType(EVENT_SCRIPT_COMMAND)) {
            if (auto* esc = notsa::dyn_cast<CEventScriptCommand>(event); esc && esc->m_task->GetTaskType() == type) {
                return true;
            }
        }

        if (auto* task = ped->GetTaskManager().GetTaskPrimary(TASK_PRIMARY_PRIMARY); task && task->GetTaskType() == type) {
            return true;
        }

        return false;
    };

    if (ped->IsInVehicle()) {
        if (CheckTaskExists(TASK_COMPLEX_SEQUENCE)) {
            return;
        }

        // Get them out of the car then make them wander.
        ped->GetEventGroup().Add(CEventScriptCommand(TASK_PRIMARY_PRIMARY, new CTaskComplexSequence(
            new CTaskComplexLeaveAnyCar(0, true, false),
            CTaskComplexWander::GetWanderTaskByPedType(ped)
        )));
    } else {
        if (CheckTaskExists(TASK_COMPLEX_WANDER)) {
            return;
        }

        // Make them wander.
        ped->GetEventGroup().Add(CEventScriptCommand(TASK_PRIMARY_PRIMARY, CTaskComplexWander::GetWanderTaskByPedType(ped)));
    }
}


// 0x486670
void CTheScripts::CleanUpThisVehicle(CVehicle* vehicle) {
    if (!vehicle || vehicle->IsCreatedBy(eVehicleCreatedBy::MISSION_VEHICLE)) {
        return;
    }

    vehicle->physicalFlags.bDontApplySpeed        = false;
    vehicle->physicalFlags.bDisableCollisionForce = false;
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
        obj->objectFlags.bChangesVehColor = false;
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
        RH_ScopedInstall(LocateCarCommand, 0x487A20, { .reversed = false });
        RH_ScopedInstall(LocateCharCommand, 0x486D80, { .reversed = false });
        RH_ScopedInstall(LocateObjectCommand, 0x487D10, { .reversed = false });
        RH_ScopedInstall(LocateCharCarCommand, 0x487420, { .reversed = false });
        RH_ScopedInstall(LocateCharCharCommand, 0x4870F0, { .reversed = false });
        RH_ScopedInstall(LocateCharObjectCommand, 0x487720, { .reversed = false });
        RH_ScopedInstall(CarInAreaCheckCommand, 0x488EC0, { .reversed = false });
        RH_ScopedInstall(CharInAreaCheckCommand, 0x488B50, { .reversed = false });
        RH_ScopedInstall(ObjectInAreaCheckCommand, 0x489150, { .reversed = false });
        RH_ScopedInstall(CharInAngledAreaCheckCommand, 0x487F60, { .reversed = false });
        RH_ScopedInstall(FlameInAngledAreaCheckCommand, 0x488780, { .reversed = false });
        RH_ScopedInstall(ObjectInAngledAreaCheckCommand, 0x4883F0, { .reversed = false });
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
