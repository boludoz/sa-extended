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



// signature changed (CVector)
// 0x4935A0
uint32 CTheScripts::AddScriptCheckpoint(CVector at, CVector pointTo, float radius, eCheckpointType type) {
    const auto cp = rng::find_if(ScriptCheckpointArray, [](auto& cp) { return !cp.IsActive(); });
    assert(cp != ScriptCheckpointArray.end()); // In vanilla game does OOB access.

    cp->m_bUsed = true;
    const auto color = [&type]() -> CRGBA {
        switch (type) {
        case eCheckpointType::TUBE:
        case eCheckpointType::ENDTUBE:
        case eCheckpointType::EMPTYTUBE:
            return { 255, 0, 0, 32 };
        case eCheckpointType::TORUS:
        case eCheckpointType::TORUS_NOFADE:
        case eCheckpointType::TORUSROT:
        case eCheckpointType::TORUSTHROUGH:
        case eCheckpointType::TORUS_UPDOWN:
        case eCheckpointType::TORUS_DOWN:
            return { 255, 0, 0, 96 };
        default:
            NOTSA_UNREACHABLE();
        }
    }();
    const auto index = GetNewUniqueScriptThingIndex(std::distance(ScriptCheckpointArray.begin(), cp), SCRIPT_THING_CHECKPOINT);
    cp->m_Checkpoint = CCheckpoints::PlaceMarker(
        index,
        type,
        at,
        pointTo,
        radius,
        color,
        1024,
        0.075f,
        0
    );

    ++NumberOfScriptCheckpoints;
    return index;
}


// 0x4936C0
void CTheScripts::RemoveScriptCheckpoint(int32 scriptIndex) {
    const auto i = GetActualScriptThingIndex(scriptIndex, eScriptThingType::SCRIPT_THING_CHECKPOINT);
    if (i == -1) {
        return;
    }

    auto& scp = ScriptCheckpointArray[i];
    if (const auto* cp = scp.m_Checkpoint) {
        CCheckpoints::DeleteCP(cp->m_ID, cp->m_Type.get_underlying());
    }
    scp.m_bUsed = false;
    scp.m_nId = 0;

    --NumberOfScriptCheckpoints;
}


// 0x492F90
uint32 CTheScripts::AddScriptEffectSystem(FxSystem_c* system) {
    const auto fx = rng::find_if(ScriptEffectSystemArray, [](auto& fx) {
        return !fx.IsActive();
    });
    assert(fx != ScriptEffectSystemArray.end()); // In vanilla game does OOB access.

    fx->m_bUsed = true;
    fx->m_pFxSystem = system;
    return GetNewUniqueScriptThingIndex(std::distance(ScriptEffectSystemArray.begin(), fx), SCRIPT_THING_EFFECT_SYSTEM);
}


// 0x492FD0
void CTheScripts::RemoveScriptEffectSystem(int32 scriptIndex) {
    const auto i = GetActualScriptThingIndex(scriptIndex, eScriptThingType::SCRIPT_THING_EFFECT_SYSTEM);
    if (i == -1) {
        return;
    }

    auto& sef = ScriptEffectSystemArray[i];
    sef.m_bUsed = false;
    sef.m_pFxSystem = nullptr;
}


// signature changed (CVector)
// 0x493000
uint32 CTheScripts::AddScriptSearchLight(CVector start, CEntity* entity, CVector target, float targetRadius, float baseRadius) {
    const auto it = rng::find_if(ScriptSearchLightArray, [](auto& ssl) {
        return !ssl.IsActive();
    });
    assert(it != ScriptSearchLightArray.end()); // In vanilla game does OOB access.

    const auto idx = std::distance(ScriptSearchLightArray.begin(), it);
    RemoveScriptSearchLight(idx);

    new (&ScriptSearchLightArray[idx]) tScriptSearchlight{
        .m_bUsed            = true,
        .m_bEnableShadow    = true,
        .m_SomethingFlag    = true,
        .m_Origin           = start,
        .m_Target           = target,
        .m_fTargetRadius    = targetRadius,
        .m_fBaseRadius      = baseRadius,
        .m_AttachedEntity   = entity,
    };
    ++NumberOfScriptSearchLights;

    return CTheScripts::GetNewUniqueScriptThingIndex(idx, SCRIPT_THING_SEARCH_LIGHT);
}


// 0x493160
void CTheScripts::RemoveScriptSearchLight(int32 scriptIndex) {
    const auto i = GetActualScriptThingIndex(scriptIndex, eScriptThingType::SCRIPT_THING_SEARCH_LIGHT);
    if (i != -1) {
        ScriptSearchLightArray[i] = tScriptSearchlight{};
        --NumberOfScriptSearchLights;
    }
}

bool CTheScripts::IsPointWithinSearchLight(const CVector& pointPosn, int32 index) {
    return false;
}

bool CTheScripts::IsEntityWithinSearchLight(uint32 index, CEntity* entity) {
    return false;
}

bool CTheScripts::IsEntityWithinAnySearchLight(CEntity* entity, int32* pIndex) {
    return false;
}


// 0x4939F0
void CTheScripts::ProcessAllSearchLights() {
    ZoneScoped;

    for (auto& sl : ScriptSearchLightArray) {
        if (!sl.IsActive() || !sl.m_SomethingFlag) {
            continue;
        }

        const auto dir = sl.m_PathCoord2 - sl.m_Target;
        if (dir.SquaredMagnitude() > sq(sl.m_fPathSpeed)) { // Originally part of every case, moved it out here...
            sl.m_Target *= dir.Normalized() * sl.m_fPathSpeed;
        } else {
            switch (sl.m_nCurrentState) {
            case eScriptSearchLightState::STATE_1: {
                sl.m_Target        = sl.m_PathCoord1;
                sl.m_SomethingFlag = true;
                sl.m_nCurrentState = eScriptSearchLightState::STATE_2;
                break;
            }
            case eScriptSearchLightState::STATE_2: {
                sl.m_Target        = sl.m_PathCoord2;
                sl.m_SomethingFlag = true;
                sl.m_nCurrentState = eScriptSearchLightState::STATE_1;
                break;
            }
            case eScriptSearchLightState::STATE_3: {
                sl.m_Target = sl.m_FollowingEntity->GetPosition();
                /* flag is not altered */
                break;
            }
            case eScriptSearchLightState::STATE_4: {
                sl.m_Target        = sl.m_PathCoord1;
                sl.m_fPathSpeed    = 0.0f;
                sl.m_PathCoord1    = CVector{};
                sl.m_SomethingFlag = true;
                sl.m_nCurrentState = eScriptSearchLightState::STATE_0;
                break;
            }
            default:
                NOTSA_UNREACHABLE("Invalid SearchLight State ({})", (int)sl.m_nCurrentState);
            }
        }

        CEntity* bulb = sl.m_Bulb;
        if (!bulb) {
            continue;
        }

        const auto prevPos    = bulb->GetPosition();
        const auto tgtBulbDir = (sl.m_Target - bulb->GetPosition()).Normalized();

        const auto Transform = [&](CEntity* entity) {
            const auto rotX = std::atan2(tgtBulbDir.z, tgtBulbDir.Magnitude2D());
            const auto rotZ = tgtBulbDir.Heading();

            entity->m_matrix->RotateX(rotX);
            entity->m_matrix->RotateZ(rotZ);
            entity->GetPosition() += prevPos;
            entity->UpdateRwMatrix();
            entity->UpdateRwFrame();
        };

        Transform(bulb);
        Transform(sl.m_Housing);
    }
}


// 0x493E30
void CTheScripts::RenderAllSearchLights() {
    ZoneScoped;

    for (const auto&& [i, light] : rngv::enumerate(ScriptSearchLightArray)) {
        if (!light.IsActive()) {
            continue;
        }

        const auto origin = [&] {
            if (auto e = notsa::coalesce(light.m_AttachedEntity.Get(), light.m_Bulb.Get())) {
                return e->GetMatrix().TransformVector(light.m_Origin) + e->GetPosition();
            }

            return light.m_Origin;
        }();

        CHeli::SearchLightCone(
            i,
            origin,
            light.m_Target,
            light.m_fTargetRadius,
            1.0f,
            light.m_bClipIfColliding,
            light.m_bEnableShadow,
            light.m_TargetSpot,
            light.vf64,
            light.vf70,
            true,
            light.m_fBaseRadius,
            0.0f,
            0.0f,
            1.0f
        );
    }
}


// 0x4934F0
void CTheScripts::AttachSearchlightToSearchlightObject(int32 searchLightId, CObject* tower, CObject* housing, CObject* bulb, CVector offset) {
    const auto idx = GetActualScriptThingIndex(searchLightId, SCRIPT_THING_SEARCH_LIGHT);
    if (idx < 0) {
        return;
    }
    
    assert(IsEntityPointerValid(bulb));
    assert(IsEntityPointerValid(tower));
    assert(IsEntityPointerValid(housing));

    auto& sl = ScriptSearchLightArray[idx];
    sl.m_Origin = offset;
    sl.m_AttachedEntity = nullptr;
    sl.m_Tower = tower;
    sl.m_Housing = housing;
    sl.m_Bulb = bulb;
}

void CTheScripts::MoveSearchLightBetweenTwoPoints(int32 index, float x1, float y1, float z1, float x2, float y2, float z2, float pathSpeed) {
}

void CTheScripts::MoveSearchLightToEntity(int32 index, CEntity* entity, float pathSpeed) {
}

void CTheScripts::MoveSearchLightToPointAndStop(int32 index, float x, float y, float z, float pathSpeed) {
}

void InjectHooks_Script9() {
    RH_ScopedClass(CTheScripts);
    RH_ScopedCategory("Scripts");

    RH_ScopedInstall(AddScriptCheckpoint, 0x4935A0);
    RH_ScopedInstall(AddScriptEffectSystem, 0x492F90);
    RH_ScopedInstall(AddScriptSearchLight, 0x493000);
    RH_ScopedInstall(RemoveScriptCheckpoint, 0x4936C0);
    RH_ScopedInstall(RemoveScriptEffectSystem, 0x492FD0);
    RH_ScopedInstall(RemoveScriptSearchLight, 0x493160);
    RH_ScopedInstall(ProcessAllSearchLights, 0x4939F0);
    RH_ScopedInstall(RenderAllSearchLights, 0x493E30);
    RH_ScopedInstall(AttachSearchlightToSearchlightObject, 0x4934F0);
}
