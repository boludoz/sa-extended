#include "StdInc.h"

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

// 0x690220
CTask* CTaskComplexArrestPed::CreateNextSubTask(CPed* ped) {
    if (!m_PedToArrest) {
        return CreateSubTask(TASK_FINISHED, ped);
    }

    const eTaskType subTaskType = m_pSubTask->GetTaskType();

    // 0x690249
    if (m_PedToArrest->bIsBeingArrested && subTaskType != TASK_SIMPLE_ARREST_PED) {
        if (subTaskType == TASK_COMPLEX_SEEK_ENTITY
            && static_cast<CTaskComplexSeekEntity<>*>(m_pSubTask)->HasAchievedSeekEntity())
        {
            return CreateSubTask(TASK_SIMPLE_ARREST_PED, ped);
        }
        return CreateSubTask(TASK_COMPLEX_SEEK_ENTITY, ped);
    }

    if (subTaskType > TASK_COMPLEX_SEEK_ENTITY) {
        const int32 v27 = subTaskType - TASK_COMPLEX_KILL_PED_ON_FOOT;
        if (v27 == 0) {
            if (m_PedToArrest->m_fHealth <= 0.0f) {
                return CreateSubTask(TASK_SIMPLE_ARREST_PED, ped);
            }

            auto* const fall = static_cast<CTaskComplexFallAndGetUp*>(
                m_PedToArrest->GetTaskManager().FindActiveTaskByType(TASK_COMPLEX_FALL_AND_GET_UP)
            );
            if (fall && fall->IsFalling()) {
                CVector diff = m_PedToArrest->GetPosition() - ped->GetPosition();
                const float diffZ = std::abs(diff.z);
                diff.z = 0.0f;
                if (diffZ > 2.0f || diff.SquaredMagnitude() > 9.0f) {
                    return CreateSubTask(TASK_COMPLEX_SEEK_ENTITY, ped);
                }
                fall->SetDownTime(100'000);
                return CreateSubTask(TASK_SIMPLE_ARREST_PED, ped);
            }

            if (ped->m_nPedType == PED_TYPE_COP) {
                return CreateSubTask(TASK_COMPLEX_KILL_PED_ON_FOOT, ped);
            }

            if (!m_PedToArrest->IsPlayer()) {
                return CreateSubTask(TASK_COMPLEX_KILL_PED_ON_FOOT, ped);
            }

            const CPlayerPedData* playerData = m_PedToArrest->GetPlayerData();
            const CWanted* wanted = playerData ? playerData->m_pWanted : nullptr;
            if (!wanted || wanted->m_NumCopsInPursuit == 0) {
                return CreateSubTask(TASK_COMPLEX_KILL_PED_ON_FOOT, ped);
            }
        } else if (v27 != 3 && v27 != 100) { // TASK_COMPLEX_DESTROY_CAR, TASK_SIMPLE_ARREST_PED
            return nullptr;
        }

        return CreateSubTask(TASK_FINISHED, ped);
    }

    if (subTaskType == TASK_COMPLEX_SEEK_ENTITY) {
        auto* const fall = static_cast<CTaskComplexFallAndGetUp*>(
            m_PedToArrest->GetTaskManager().FindActiveTaskByType(TASK_COMPLEX_FALL_AND_GET_UP)
        );
        if (!fall || !fall->IsFalling() || !static_cast<CTaskComplexSeekEntity<>*>(m_pSubTask)->HasAchievedSeekEntity()) {
            return CreateSubTask(TASK_COMPLEX_KILL_PED_ON_FOOT, ped);
        }

        CVector diff = m_PedToArrest->GetPosition() - ped->GetPosition();
        const float diffZ = std::abs(diff.z);
        diff.z = 0.0f;
        if (diffZ <= 2.0f && diff.SquaredMagnitude() <= 9.0f) {
            fall->SetDownTime(100'000);
            return CreateSubTask(TASK_SIMPLE_ARREST_PED, ped);
        }

        return CreateSubTask(TASK_COMPLEX_KILL_PED_ON_FOOT, ped);
    }

    switch (subTaskType) {
    case TASK_COMPLEX_DRAG_PED_FROM_CAR: {
        auto* const fall = static_cast<CTaskComplexFallAndGetUp*>(
            m_PedToArrest->GetTaskManager().FindActiveTaskByType(TASK_COMPLEX_FALL_AND_GET_UP)
        );
        if (!fall || !fall->IsFalling() || static_cast<CTaskComplexDragPedFromCar*>(m_pSubTask)->IsQuitAfterDraggingPedOut()) {
            return CreateSubTask(TASK_COMPLEX_KILL_PED_ON_FOOT, ped);
        }

        CVector diff = m_PedToArrest->GetPosition() - ped->GetPosition();
        const float diffZ = std::abs(diff.z);
        diff.z = 0.0f;
        if (diffZ <= 2.0f && diff.SquaredMagnitude() <= 9.0f) {
            fall->SetDownTime(100'000);
            return CreateSubTask(TASK_SIMPLE_ARREST_PED, ped);
        }

        return CreateSubTask(TASK_COMPLEX_KILL_PED_ON_FOOT, ped);
    }

    case TASK_COMPLEX_CAR_OPEN_DRIVER_DOOR: {
        if (static_cast<CTaskComplexEnterCar*>(m_pSubTask)->IsQuitAfterOpeningDoor()) {
            CVehicle* const veh = m_PedToArrest->m_pVehicle;
            if (veh && !veh->CanPedOpenLocks(ped)) {
                m_Vehicle = veh;
            }
        }

        if (!m_PedToArrest->IsAlive()) {
            return CreateSubTask(TASK_SIMPLE_ARREST_PED, ped);
        }

        if (!m_PedToArrest->bInVehicle || static_cast<CTaskComplexEnterCar*>(m_pSubTask)->IsQuitAfterOpeningDoor()) {
            return CreateSubTask(TASK_COMPLEX_KILL_PED_ON_FOOT, ped);
        }

        if (m_PedToArrest->GetTaskManager().FindActiveTaskByType(TASK_COMPLEX_LEAVE_CAR)) {
            return CreateSubTask(TASK_COMPLEX_KILL_PED_ON_FOOT, ped);
        }

        return CreateSubTask(TASK_SIMPLE_ARREST_PED, ped);
    }

    case TASK_COMPLEX_CAR_OPEN_PASSENGER_DOOR: {
        if (static_cast<CTaskComplexEnterCar*>(m_pSubTask)->IsQuitAfterOpeningDoor()) {
            CVehicle* const veh = m_PedToArrest->m_pVehicle;
            if (veh && !veh->CanPedOpenLocks(ped)) {
                m_Vehicle = veh;
            }
        }

        if (!m_PedToArrest->IsAlive()) {
            return CreateSubTask(TASK_SIMPLE_ARREST_PED, ped);
        }

        if (!m_PedToArrest->bInVehicle || static_cast<CTaskComplexEnterCar*>(m_pSubTask)->IsQuitAfterOpeningDoor()) {
            return CreateSubTask(TASK_COMPLEX_KILL_PED_ON_FOOT, ped);
        }

        if (m_PedToArrest->GetTaskManager().FindActiveTaskByType(TASK_COMPLEX_LEAVE_CAR)) {
            return CreateSubTask(TASK_COMPLEX_KILL_PED_ON_FOOT, ped);
        }

        return CreateSubTask(TASK_SIMPLE_ARREST_PED, ped);
    }

    case TASK_SIMPLE_WAIT_UNTIL_PED_OUT_CAR:
        return CreateSubTask(TASK_COMPLEX_KILL_PED_ON_FOOT, ped);

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

    if (m_PedToArrest->m_pVehicle->IsBike() || m_PedToArrest->m_pVehicle->IsSubQuad()) {
        return CreateSubTask(TASK_COMPLEX_DRAG_PED_FROM_CAR, ped);
    }

    if (m_PedToArrest->m_pVehicle->IsSubBoat()) {
        if (ped->GetActiveWeapon().IsTypeMelee()) {
            if (ped->DoWeHaveWeaponAvailable(WEAPON_SHOTGUN)) {
                ped->SetCurrentWeapon(WEAPON_SHOTGUN);
            } else {
                if (!ped->DoWeHaveWeaponAvailable(WEAPON_PISTOL)) {
                    ped->GiveWeapon(WEAPON_PISTOL, 10, false);
                }
                ped->SetCurrentWeapon(WEAPON_PISTOL);
            }
        }
        return CreateSubTask(TASK_COMPLEX_DESTROY_CAR, ped);
    }

    if (m_PedToArrest->m_pVehicle->IsUpsideDown() || m_PedToArrest->m_pVehicle->IsOnItsSide()) {
        return CreateSubTask(TASK_COMPLEX_DESTROY_CAR, ped);
    }

    return CreateSubTask(TASK_COMPLEX_CAR_OPEN_DRIVER_DOOR, ped);
}

// 0x68D350
CTask* CTaskComplexArrestPed::ControlSubTask(CPed* ped) {
    CTask* nextSubTask = m_pSubTask;
    eTaskType taskId = TASK_NONE;

    if (!m_PedToArrest || m_PedToArrest->m_fHealth <= 0.0f) {
        taskId = TASK_FINISHED;
    } else if (m_bSubTaskNeedsToBeCreated) {
        if (m_pSubTask->MakeAbortable(ped, ABORT_PRIORITY_URGENT, nullptr)) {
            nextSubTask = CreateFirstSubTask(ped);
        }
    } else if (m_PedToArrest->bIsBeingArrested
        && m_pSubTask->GetTaskType() != TASK_SIMPLE_ARREST_PED
        && m_pSubTask->GetTaskType() != TASK_COMPLEX_SEEK_ENTITY)
    {
        taskId = TASK_COMPLEX_SEEK_ENTITY;
    } else {
        const eTaskType subTaskType = m_pSubTask->GetTaskType();
        if (subTaskType > TASK_COMPLEX_CAR_OPEN_PASSENGER_DOOR) {
            const int32 v24 = subTaskType - TASK_COMPLEX_KILL_PED_ON_FOOT;
            if (v24 == 0) {
                auto* const fall = static_cast<CTaskComplexFallAndGetUp*>(
                    m_PedToArrest->GetTaskManager().FindActiveTaskByType(TASK_COMPLEX_FALL_AND_GET_UP)
                );
                if (fall && fall->IsFalling()) {
                    CVector a1 = ped->GetPosition() - m_PedToArrest->GetPosition();
                    const float v32 = std::abs(a1.z);
                    a1.z = 0.0f;
                    if (v32 <= 2.0f && a1.SquaredMagnitude() <= 9.0f) {
                        fall->SetDownTime(100'000);
                        taskId = TASK_SIMPLE_ARREST_PED;
                    } else {
                        taskId = TASK_COMPLEX_SEEK_ENTITY;
                    }
                } else if (m_PedToArrest->bInVehicle && m_PedToArrest->m_pVehicle) {
                    CVehicle* const veh = m_PedToArrest->m_pVehicle;
                    if (veh->GetBaseVehicleType() == VEHICLE_TYPE_BOAT || veh->GetVehicleType() == VEHICLE_TYPE_PLANE || veh->GetVehicleType() == VEHICLE_TYPE_HELI) {
                        if (ped->GetActiveWeapon().IsTypeMelee()) {
                            if (ped->DoWeHaveWeaponAvailable(WEAPON_SHOTGUN)) {
                                ped->SetCurrentWeapon(WEAPON_SHOTGUN);
                            } else {
                                ped->SetCurrentWeapon(WEAPON_PISTOL);
                            }
                        }
                        taskId = TASK_COMPLEX_DESTROY_CAR;
                    } else {
                        if (!ped->GetActiveWeapon().IsTypeMelee()) {
                            CWanted* const wanted = FindPlayerWanted(-1);
                            if (!wanted->IsClosestCop(ped, 2)) {
                                taskId = TASK_COMPLEX_DESTROY_CAR;
                            }
                        }

                        if (taskId != TASK_COMPLEX_DESTROY_CAR) {
                            if (this->m_Vehicle != veh && !m_PedToArrest->GetTaskManager().FindActiveTaskByType(TASK_COMPLEX_LEAVE_CAR)) {
                                if (veh->GetBaseVehicleType() == VEHICLE_TYPE_BIKE || veh->GetVehicleType() == VEHICLE_TYPE_QUAD) {
                                    taskId = TASK_COMPLEX_DRAG_PED_FROM_CAR;
                                } else if (!veh->IsUpsideDown() && !veh->IsOnItsSide()) {
                                    taskId = TASK_COMPLEX_CAR_OPEN_DRIVER_DOOR;
                                } else {
                                    taskId = TASK_COMPLEX_DESTROY_CAR;
                                }
                            }
                        }
                    }
                }
            } else if (v24 == 3) { // TASK_COMPLEX_DESTROY_CAR
                if (!m_PedToArrest->bInVehicle) {
                    taskId = TASK_COMPLEX_KILL_PED_ON_FOOT;
                }
            }
        } else if (subTaskType == TASK_COMPLEX_CAR_OPEN_PASSENGER_DOOR) {
            const bool hasLeaveCar = m_PedToArrest->GetTaskManager().FindActiveTaskByType(TASK_COMPLEX_LEAVE_CAR) != nullptr;
            const float distSq = (m_PedToArrest->GetPosition() - ped->GetPosition()).SquaredMagnitude();

            if (hasLeaveCar && m_PedToArrest->bInVehicle && distSq < 25.0f) {
                taskId = TASK_SIMPLE_WAIT_UNTIL_PED_OUT_CAR;
            } else if (!m_PedToArrest->bInVehicle) {
                taskId = TASK_COMPLEX_KILL_PED_ON_FOOT;
            } else if (!CCarEnterExit::IsRoomForPedToLeaveCar(m_PedToArrest->m_pVehicle, TARGET_DOOR_FRONT_RIGHT)) {
                if (CCarEnterExit::IsRoomForPedToLeaveCar(m_PedToArrest->m_pVehicle, TARGET_DOOR_DRIVER)) {
                    taskId = TASK_COMPLEX_CAR_OPEN_DRIVER_DOOR;
                } else {
                    if (ped->GetActiveWeapon().IsTypeMelee()) {
                        if (ped->DoWeHaveWeaponAvailable(WEAPON_SHOTGUN)) {
                            ped->SetCurrentWeapon(WEAPON_SHOTGUN);
                        } else {
                            ped->SetCurrentWeapon(WEAPON_PISTOL);
                        }
                    }
                    taskId = TASK_COMPLEX_DESTROY_CAR;
                }
            }
        } else {
            const int32 v6 = subTaskType - TASK_COMPLEX_DRAG_PED_FROM_CAR;
            if (v6 == 0) {
                if (!m_PedToArrest->bInVehicle) {
                    taskId = TASK_COMPLEX_KILL_PED_ON_FOOT;
                }
            } else if (v6 == 19) { // TASK_COMPLEX_CAR_OPEN_DRIVER_DOOR
                const bool hasLeaveCar = m_PedToArrest->GetTaskManager().FindActiveTaskByType(TASK_COMPLEX_LEAVE_CAR) != nullptr;
                const float distSq = (m_PedToArrest->GetPosition() - ped->GetPosition()).SquaredMagnitude();

                if (hasLeaveCar && m_PedToArrest->bInVehicle && distSq < 25.0f) {
                    taskId = TASK_SIMPLE_WAIT_UNTIL_PED_OUT_CAR;
                } else if (m_PedToArrest->bInVehicle) {
                    if (!CCarEnterExit::IsRoomForPedToLeaveCar(m_PedToArrest->m_pVehicle, TARGET_DOOR_DRIVER)
                        && CCarEnterExit::IsRoomForPedToLeaveCar(m_PedToArrest->m_pVehicle, TARGET_DOOR_FRONT_RIGHT))
                    {
                        taskId = TASK_COMPLEX_CAR_OPEN_PASSENGER_DOOR;
                    }
                } else {
                    taskId = TASK_COMPLEX_KILL_PED_ON_FOOT;
                }
            }
        }
    }

    // 0x68D7F9
    if (m_PedToArrest && m_PedToArrest->IsPlayer()) {
        CWanted* const wanted = FindPlayerWanted(-1);
        if (wanted && wanted->m_NumCopsInPursuit == 1) {
            ped->Say(CTX_GLOBAL_SOLO);
        }
    }

    if (taskId == TASK_NONE || !m_pSubTask->MakeAbortable(ped, ABORT_PRIORITY_URGENT, nullptr)) {
        return nextSubTask;
    }

    return CreateSubTask(taskId, ped);
}

// 0x68CF80
CTask* CTaskComplexArrestPed::CreateSubTask(eTaskType taskType, CPed* ped) {
    switch (taskType) {
    case TASK_SIMPLE_ARREST_PED: {
        CVehicle* const veh = m_PedToArrest->m_pVehicle;
        if (veh && veh->IsDriver(m_PedToArrest)) {
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
            m_PedToArrest->bIsBeingArrested ? 4.0f : 3.0f,
            2.0f,
            2.0f,
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
