#include "StdInc.h"

#include "extensions/utility.hpp"

#include "TaskComplexArrestPed.h"
#include "TaskComplexFallAndGetUp.h"
#include "TaskSimpleWaitUntilPedIsOutCar.h"
#include "TaskSimpleArrestPed.h"
#include "TaskComplexKillPedOnFoot.h"
#include "TaskComplexDestroyCar.h"
#include "SeekEntity/TaskComplexSeekEntity.h"
#include "TaskComplexDragPedFromCar.h"
#include "TaskComplexOpenDriverDoor.h"
#include "TaskComplexOpenPassengerDoor.h"

#include "eTargetDoor.h"

void CTaskComplexArrestPed::InjectHooks() {
    RH_ScopedVirtualClass(CTaskComplexArrestPed, 0x8709A8, 11);
    RH_ScopedCategory("Tasks/TaskTypes");

    RH_ScopedInstall(Constructor, 0x68B990);
    RH_ScopedInstall(Destructor, 0x68BA00);
    RH_ScopedVMTInstall(MakeAbortable, 0x68BA60);
    RH_ScopedVMTInstall(CreateNextSubTask, 0x690220);
    RH_ScopedVMTInstall(CreateFirstSubTask, 0x6907A0);
    RH_ScopedVMTInstall(ControlSubTask, 0x68D350);
    RH_ScopedInstall(CreateSubTask, 0x68CF80);
}

// 0x68B990
CTaskComplexArrestPed::CTaskComplexArrestPed(CPed* ped) : CTaskComplex() {
    m_PedToArrest = ped;
    m_Vehicle = nullptr;
    CEntity::SafeRegisterRef(m_PedToArrest);
}

// 0x68BA00
CTaskComplexArrestPed::~CTaskComplexArrestPed() {
    CEntity::SafeCleanUpRef(m_PedToArrest);
}

// 0x68BA60
bool CTaskComplexArrestPed::MakeAbortable(CPed* ped, eAbortPriority priority, const CEvent* event) {
    return m_pSubTask->MakeAbortable(ped, priority, event);
}

// NOTSA - Inlined into `CreateFirstSubTask`
void MakeSurePedHasWeaponInHand(CPed* ped) {
    // Make sure ped has an actual weapon in their hand
    if (!ped->GetActiveWeapon().IsTypeMelee())
        return;

    if (ped->DoWeHaveWeaponAvailable(WEAPON_SHOTGUN)) { // Use shotgun (if available)
        ped->SetCurrentWeapon(WEAPON_SHOTGUN);
        return;
    }

    // Otherwise a pistol
    if (!ped->DoWeHaveWeaponAvailable(WEAPON_PISTOL)) { // Make sure they have one
        ped->GiveWeapon(WEAPON_PISTOL, 10, false);
    }
    ped->SetCurrentWeapon(WEAPON_PISTOL);
}

// NOTSA - Inlined into `ControlSubTask`. Same as above, except it won't hand out a pistol
static void DrawGunToShootUpCar(CPed* ped) {
    if (ped->GetActiveWeapon().IsTypeMelee()) {
        ped->SetCurrentWeapon(ped->DoWeHaveWeaponAvailable(WEAPON_SHOTGUN) ? WEAPON_SHOTGUN : WEAPON_PISTOL);
    }
}

// 0x690220
CTask* CTaskComplexArrestPed::CreateNextSubTask(CPed* ped) {
    if (!m_PedToArrest) {
        return CreateSubTask(TASK_FINISHED, ped);
    }

    const auto GetFallTask = [this] {
        return static_cast<CTaskComplexFallAndGetUp*>(m_PedToArrest->GetTaskManager().FindActiveTaskByType(TASK_COMPLEX_FALL_AND_GET_UP));
    };

    /// Whenever the downed ped is within cuffing range - keeps them down for a while if so
    const auto TryPinDownPed = [this, ped](CTaskComplexFallAndGetUp* fall) {
        const auto dir = m_PedToArrest->GetPosition() - ped->GetPosition();
        if (std::abs(dir.z) > 2.f || dir.SquaredMagnitude2D() > sq(3.f)) {
            return false;
        }
        fall->SetDownTime(100'000);
        return true;
    };

    const auto subTaskType = m_pSubTask->GetTaskType();

    // 0x690249 - Already cuffing them, so all we may do is walk up to them
    if (m_PedToArrest->bIsBeingArrested && subTaskType != TASK_SIMPLE_ARREST_PED) {
        const auto arrived = subTaskType == TASK_COMPLEX_SEEK_ENTITY
                          && static_cast<CTaskComplexSeekEntity<>*>(m_pSubTask)->HasAchievedSeekEntity();
        return CreateSubTask(arrived ? TASK_SIMPLE_ARREST_PED : TASK_COMPLEX_SEEK_ENTITY, ped);
    }

    switch (subTaskType) {
    case TASK_COMPLEX_KILL_PED_ON_FOOT: { // 0x6905B7
        if (m_PedToArrest->m_fHealth <= 0.f) {
            return CreateSubTask(TASK_SIMPLE_ARREST_PED, ped);
        }

        if (const auto fall = GetFallTask(); fall && fall->IsFalling()) {
            return CreateSubTask(TryPinDownPed(fall) ? TASK_SIMPLE_ARREST_PED : TASK_COMPLEX_SEEK_ENTITY, ped);
        }

        // Cops never give up, and neither do we as long as someone else is still on the player's tail
        if (   ped->IsCop()
            || !m_PedToArrest->IsPlayer()
            || !m_PedToArrest->GetPlayerWanted()->m_NumCopsInPursuit
        ) {
            return CreateSubTask(TASK_COMPLEX_KILL_PED_ON_FOOT, ped);
        }
        return CreateSubTask(TASK_FINISHED, ped);
    }
    case TASK_COMPLEX_SEEK_ENTITY: { // 0x690491
        const auto fall = GetFallTask();
        if (   !fall
            || !fall->IsFalling()
            || !static_cast<CTaskComplexSeekEntity<>*>(m_pSubTask)->HasAchievedSeekEntity()
            || !TryPinDownPed(fall)
        ) {
            return CreateSubTask(TASK_COMPLEX_KILL_PED_ON_FOOT, ped);
        }
        return CreateSubTask(TASK_SIMPLE_ARREST_PED, ped);
    }
    case TASK_COMPLEX_DRAG_PED_FROM_CAR: { // 0x690437
        const auto fall = GetFallTask();
        if (   !fall
            || !fall->IsFalling()
            || static_cast<CTaskComplexDragPedFromCar*>(m_pSubTask)->IsQuitAfterDraggingPedOut()
            || !TryPinDownPed(fall)
        ) {
            return CreateSubTask(TASK_COMPLEX_KILL_PED_ON_FOOT, ped);
        }
        return CreateSubTask(TASK_SIMPLE_ARREST_PED, ped);
    }
    case TASK_COMPLEX_CAR_OPEN_DRIVER_DOOR: // 0x690302
    case TASK_COMPLEX_CAR_OPEN_PASSENGER_DOOR: { // 0x690392
        // Set when the door task is constructed, but `CTaskComplexEnterCar` clears it once the door is open
        // (0x63EE52, 0x63FA72) - so by the time we get here it's false and we do reach the arrest below
        const auto quitAfterOpeningDoor = static_cast<CTaskComplexEnterCar*>(m_pSubTask)->IsQuitAfterOpeningDoor();

        if (quitAfterOpeningDoor) { // Remember the car we couldn't unlock, `ControlSubTask` won't try opening it again
            if (const auto veh = m_PedToArrest->m_pVehicle; veh && !veh->CanPedOpenLocks(ped)) {
                m_Vehicle = veh;
            }
        }

        if (!m_PedToArrest->IsAlive()) {
            return CreateSubTask(TASK_SIMPLE_ARREST_PED, ped);
        }

        if (   !m_PedToArrest->bInVehicle
            || quitAfterOpeningDoor
            || m_PedToArrest->GetTaskManager().FindActiveTaskByType(TASK_COMPLEX_LEAVE_CAR)
        ) {
            return CreateSubTask(TASK_COMPLEX_KILL_PED_ON_FOOT, ped);
        }
        return CreateSubTask(TASK_SIMPLE_ARREST_PED, ped);
    }
    case TASK_SIMPLE_WAIT_UNTIL_PED_OUT_CAR: // 0x690560
        return CreateSubTask(TASK_COMPLEX_KILL_PED_ON_FOOT, ped);

    case TASK_COMPLEX_DESTROY_CAR:
    case TASK_SIMPLE_ARREST_PED: // 0x6906E3
        return CreateSubTask(TASK_FINISHED, ped);

    default:
        return nullptr;
    }
}

// 0x6907A0
CTask* CTaskComplexArrestPed::CreateFirstSubTask(CPed* ped) {
    if (!m_PedToArrest) {
        return nullptr;
    }

    m_bSubTaskNeedsToBeCreated = false;

    if (!m_PedToArrest->bInVehicle) {
        return CreateSubTask(TASK_COMPLEX_KILL_PED_ON_FOOT, ped);
    }

    if (m_PedToArrest->m_pVehicle->IsBike() || m_PedToArrest->m_pVehicle->IsSubQuad()) { // Just drag ped from a bike/quad
        return CreateSubTask(TASK_COMPLEX_DRAG_PED_FROM_CAR, ped);
    }

    if (m_PedToArrest->m_pVehicle->IsSubBoat()) { // If they're in a boat, just destroy it
        MakeSurePedHasWeaponInHand(ped);
        return CreateSubTask(TASK_COMPLEX_DESTROY_CAR, ped);
    } else {
        if (m_PedToArrest->m_pVehicle->IsUpsideDown() || m_PedToArrest->m_pVehicle->IsOnItsSide()) {
            return CreateSubTask(TASK_COMPLEX_DESTROY_CAR, ped);
        }
        return CreateSubTask(TASK_COMPLEX_CAR_OPEN_DRIVER_DOOR, ped);
    }
}


// 0x68D350
CTask* CTaskComplexArrestPed::ControlSubTask(CPed* ped) {
    auto* nextSubTask = m_pSubTask;

    const auto taskType = [&]() -> eTaskType {
        // 0x68D370, 0x68D37F
        if (!m_PedToArrest || m_PedToArrest->m_fHealth <= 0.f) {
            return TASK_FINISHED;
        }

        // 0x68D39F
        if (m_bSubTaskNeedsToBeCreated) {
            if (m_pSubTask->MakeAbortable(ped)) {
                nextSubTask = CreateFirstSubTask(ped);
            }
            return TASK_NONE;
        }

        const auto subTaskType = m_pSubTask->GetTaskType();

        // 0x68D3CB - Already cuffing them, so all we may do is walk up to them
        if (   m_PedToArrest->bIsBeingArrested
            && subTaskType != TASK_SIMPLE_ARREST_PED
            && subTaskType != TASK_COMPLEX_SEEK_ENTITY
        ) {
            return TASK_COMPLEX_SEEK_ENTITY;
        }

        switch (subTaskType) {
        case TASK_COMPLEX_DRAG_PED_FROM_CAR: // 0x68D49D
        case TASK_COMPLEX_DESTROY_CAR: {     // 0x68D5F6
            return m_PedToArrest->bInVehicle ? TASK_NONE : TASK_COMPLEX_KILL_PED_ON_FOOT;
        }
        case TASK_COMPLEX_KILL_PED_ON_FOOT: { // 0x68D626
            // Ped is on the ground - cuff them if we're close enough, otherwise walk over first
            if (const auto fall = static_cast<CTaskComplexFallAndGetUp*>(m_PedToArrest->GetTaskManager().FindActiveTaskByType(TASK_COMPLEX_FALL_AND_GET_UP));
                fall && fall->IsFalling()
            ) {
                const auto dir = ped->GetPosition() - m_PedToArrest->GetPosition();
                if (std::abs(dir.z) > 2.f || dir.SquaredMagnitude2D() > sq(3.f)) {
                    return TASK_COMPLEX_SEEK_ENTITY;
                }
                fall->SetDownTime(100'000);
                return TASK_SIMPLE_ARREST_PED;
            }

            const auto veh = m_PedToArrest->bInVehicle ? m_PedToArrest->m_pVehicle : nullptr;
            if (!veh) { // Still on foot, carry on
                return TASK_NONE;
            }

            // They've gotten into a vehicle, so we need a different task
            if (veh->IsBoat() || veh->IsSubPlane() || veh->IsSubHeli()) { // Can't chase those, shoot them down
                DrawGunToShootUpCar(ped);
                return TASK_COMPLEX_DESTROY_CAR;
            }

            // Leave dragging them out to whoever is closest, the rest of us shoot the car up
            if (!ped->GetActiveWeapon().IsTypeMelee() && !FindPlayerWanted()->IsClosestCop(ped, 2)) {
                return TASK_COMPLEX_DESTROY_CAR;
            }

            if (veh == m_Vehicle || m_PedToArrest->GetTaskManager().FindActiveTaskByType(TASK_COMPLEX_LEAVE_CAR)) {
                return TASK_NONE; // Locked car we've already failed to open, or they're getting out on their own
            }

            if (veh->IsBike() || veh->IsSubQuad()) {
                return TASK_COMPLEX_DRAG_PED_FROM_CAR;
            }

            if (!veh->IsUpsideDown() && !veh->IsOnItsSide()) {
                return TASK_COMPLEX_CAR_OPEN_DRIVER_DOOR;
            }

            return TASK_COMPLEX_DESTROY_CAR; // 0x68D7A6 jumps past the weapon swap, unlike the cases above
        }
        case TASK_COMPLEX_CAR_OPEN_DRIVER_DOOR: { // 0x68D424
            const auto isLeavingCar = m_PedToArrest->GetTaskManager().FindActiveTaskByType(TASK_COMPLEX_LEAVE_CAR) != nullptr;
            const auto distSq      = (m_PedToArrest->GetPosition() - ped->GetPosition()).SquaredMagnitude();

            if (isLeavingCar && m_PedToArrest->bInVehicle && distSq < sq(5.f)) { // They're getting out by themselves, just wait for it
                return TASK_SIMPLE_WAIT_UNTIL_PED_OUT_CAR;
            }

            if (!m_PedToArrest->bInVehicle) { // Out of the vehicle already, just kill 'em
                return TASK_COMPLEX_KILL_PED_ON_FOOT;
            }

            // No room on the driver's side, but there is on the front right?
            if (   !CCarEnterExit::IsRoomForPedToLeaveCar(m_PedToArrest->m_pVehicle, TARGET_DOOR_DRIVER)
                && CCarEnterExit::IsRoomForPedToLeaveCar(m_PedToArrest->m_pVehicle, TARGET_DOOR_FRONT_RIGHT)
            ) {
                return TASK_COMPLEX_CAR_OPEN_PASSENGER_DOOR;
            }
            return TASK_NONE;
        }
        case TASK_COMPLEX_CAR_OPEN_PASSENGER_DOOR: { // 0x68D510
            // Mirror of the above, except there's nowhere left to try once both doors are blocked
            const auto isLeavingCar = m_PedToArrest->GetTaskManager().FindActiveTaskByType(TASK_COMPLEX_LEAVE_CAR) != nullptr;
            const auto distSq      = (m_PedToArrest->GetPosition() - ped->GetPosition()).SquaredMagnitude();

            if (isLeavingCar && m_PedToArrest->bInVehicle && distSq < sq(5.f)) {
                return TASK_SIMPLE_WAIT_UNTIL_PED_OUT_CAR;
            }

            if (!m_PedToArrest->bInVehicle) {
                return TASK_COMPLEX_KILL_PED_ON_FOOT;
            }

            if (CCarEnterExit::IsRoomForPedToLeaveCar(m_PedToArrest->m_pVehicle, TARGET_DOOR_FRONT_RIGHT)) {
                return TASK_NONE;
            }

            if (CCarEnterExit::IsRoomForPedToLeaveCar(m_PedToArrest->m_pVehicle, TARGET_DOOR_DRIVER)) {
                return TASK_COMPLEX_CAR_OPEN_DRIVER_DOOR;
            }

            DrawGunToShootUpCar(ped);
            return TASK_COMPLEX_DESTROY_CAR;
        }
        default:
            return TASK_NONE;
        }
    }();

    // 0x68D7F9 - Let them know they're on their own out here
    if (m_PedToArrest && m_PedToArrest->IsPlayer() && FindPlayerWanted()->m_NumCopsInPursuit == 1) {
        ped->Say(CTX_GLOBAL_SOLO);
    }

    if (taskType == TASK_NONE || !m_pSubTask->MakeAbortable(ped)) {
        return nextSubTask;
    }
    return CreateSubTask(taskType, ped);
}

// 0x68CF80
CTask* CTaskComplexArrestPed::CreateSubTask(eTaskType taskType, CPed* ped) {
    switch (taskType) {
    case TASK_SIMPLE_ARREST_PED: {
        if (const auto veh = m_PedToArrest->m_pVehicle; veh && veh->IsDriver(m_PedToArrest)) {
            veh->vehicleFlags.bIsHandbrakeOn = true;
            veh->SetStatus(STATUS_FORCED_STOP);
        }
        return new CTaskSimpleArrestPed{m_PedToArrest};
    }
    case TASK_COMPLEX_KILL_PED_ON_FOOT:
        return new CTaskComplexKillPedOnFoot{m_PedToArrest, -1, 0, 0, 0, true};

    case TASK_COMPLEX_DESTROY_CAR:
        return new CTaskComplexDestroyCar{m_PedToArrest->m_pVehicle, 0, 0, 0};

    case TASK_COMPLEX_SEEK_ENTITY:
        return new CTaskComplexSeekEntity<>{
            m_PedToArrest,
            50'000,
            1'000,
            m_PedToArrest->bIsBeingArrested ? 4.f : 3.f,
            2.f,
            2.f,
            true,
            true
        };

    case TASK_COMPLEX_DRAG_PED_FROM_CAR:
        return new CTaskComplexDragPedFromCar{m_PedToArrest, 100'000};

    case TASK_COMPLEX_CAR_OPEN_DRIVER_DOOR:
        return new CTaskComplexOpenDriverDoor{m_PedToArrest->m_pVehicle};

    case TASK_COMPLEX_CAR_OPEN_PASSENGER_DOOR:
        return new CTaskComplexOpenPassengerDoor{m_PedToArrest->m_pVehicle, TARGET_DOOR_FRONT_RIGHT};

    case TASK_SIMPLE_WAIT_UNTIL_PED_OUT_CAR:
        return new CTaskSimpleWaitUntilPedIsOutCar{m_PedToArrest, m_PedToArrest->GetPosition() - ped->GetPosition()};

    default:
        return nullptr;
    }
}
