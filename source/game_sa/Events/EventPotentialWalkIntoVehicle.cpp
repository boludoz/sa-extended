#include "StdInc.h"
#include "EventPotentialWalkIntoVehicle.h"


void CEventPotentialWalkIntoVehicle::InjectHooks()
{
    RH_ScopedVirtualClass(CEventPotentialWalkIntoVehicle, 0x85AED0, 18); // or 17.
    RH_ScopedCategory("Events");

    RH_ScopedInstall(Constructor, 0x4AE320);
    RH_ScopedInstall(Destructor, 0x4AE3C0);
    RH_ScopedVMTInstall(AffectsPed, 0x4AE420);
    RH_ScopedVMTInstall(TakesPriorityOver, 0x4AE5C0);
    RH_ScopedVMTInstall(CloneEditable, 0x4B6FB0);
}

// 0x4AE320
// ASM Match
CEventPotentialWalkIntoVehicle::CEventPotentialWalkIntoVehicle(CVehicle* vehicle, int32 moveState) {
    m_vehicle = vehicle;
    m_moveState = moveState;
    CEntity::SafeRegisterRef(m_vehicle);
}

// 0x4AE3C0
// ASM Match
CEventPotentialWalkIntoVehicle::~CEventPotentialWalkIntoVehicle() {
    CEntity::SafeCleanUpRef(m_vehicle);
}

// 0x4AE320
CEventPotentialWalkIntoVehicle* CEventPotentialWalkIntoVehicle::Constructor(CVehicle* vehicle, int32 moveState) {
    this->CEventPotentialWalkIntoVehicle::CEventPotentialWalkIntoVehicle(vehicle, moveState);
    return this;
}

// 0x4AE420
// ASM Match
bool CEventPotentialWalkIntoVehicle::AffectsPed(CPed* ped) {
    auto taskEnterCarAsDriver = ped->GetTaskManager().Find<CTaskComplexEnterCarAsDriver>();
    auto simplestTask = ped->GetTaskManager().GetSimplestActiveTask();
    if (ped->IsPlayer() && !taskEnterCarAsDriver && !CTask::IsGoToTask(simplestTask))
        return false;

    if (!ped->IsAlive() || ped->bInVehicle || m_moveState == PEDMOVE_STILL)
        return false;

    if (const auto task = ped->GetTaskManager().Find<CTaskComplexWalkRoundCar>()) {
        if (task->m_Veh != m_vehicle) {
            if ((m_vehicle->m_pVehicleBeingTowed && m_vehicle->m_pVehicleBeingTowed == task->m_Veh) ||
                (m_vehicle->m_pTowingVehicle && m_vehicle->m_pTowingVehicle == task->m_Veh)
            ) {
                task->SetNewVehicle(m_vehicle, task->m_DirectionGoingRoundCar);
            }
        }
        return false;
    }

    if (ped->m_pAttachedTo)
        return false;

    if (!m_vehicle || m_vehicle->m_vehicleType == -1 || m_vehicle->m_vehicleType == VEHICLE_TYPE_FHELI || m_vehicle->m_vehicleType == VEHICLE_TYPE_FPLANE)
        return false;

    bool bTestTargetSide = false;
    CVector vTarget;
    if (simplestTask->GetTaskType() == TASK_SIMPLE_GO_TO_POINT) {
        vTarget = static_cast<CTaskSimpleGoToPoint*>(simplestTask)->m_vecTargetPoint;
        bTestTargetSide = true;
    }

    if (taskEnterCarAsDriver) {
        if (m_vehicle == taskEnterCarAsDriver->GetTargetCar() && m_vehicle->m_vehicleType == VEHICLE_TYPE_PLANE)
            return false;

        vTarget = taskEnterCarAsDriver->GetTargetPos();
    }

    if (taskEnterCarAsDriver || bTestTargetSide) {
        CVector vClosestSurfacePoint;
        CPedGeometryAnalyser::ComputeClosestSurfacePoint(*ped, *m_vehicle, vClosestSurfacePoint);
        const int32 iCurrentSide = CPedGeometryAnalyser::ComputeEntityHitSide(vClosestSurfacePoint, *m_vehicle);
        const int32 iTargetSide = CPedGeometryAnalyser::ComputeEntityHitSide(vTarget, *m_vehicle);
        if (iCurrentSide == iTargetSide)
            return false;
    }

    return true;
}

// 0x4AE5C0
// ASM Match
bool CEventPotentialWalkIntoVehicle::TakesPriorityOver(const CEvent& refEvent) {
    return true;
}

// 0x4B6FB0
// ASM Match
CEventPotentialWalkIntoVehicle* CEventPotentialWalkIntoVehicle::CloneEditable() const noexcept {
    return new CEventPotentialWalkIntoVehicle(m_vehicle, m_moveState);
}