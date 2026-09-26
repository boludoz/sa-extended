#include "StdInc.h"

#include "TaskComplexDriveToPoint.h"
#include "TaskComplexGoToPointAnyMeans.h"
#include "CarAI.h"

void CTaskComplexDriveToPoint::InjectHooks() {
    RH_ScopedVirtualClass(CTaskComplexDriveToPoint, 0x86E9DC, 14);
    RH_ScopedCategory("Tasks/TaskTypes");

    RH_ScopedInstall(Constructor, 0x63CE00);
    RH_ScopedVMTInstall(Clone, 0x63DDE0);
    RH_ScopedVMTInstall(GetTaskType, 0x63CE60);
    RH_ScopedVMTInstall(SetUpCar, 0x63CF00);
    RH_ScopedVMTInstall(CreateSubTaskCannotGetInCar, 0x63CE80);
    RH_ScopedVMTInstall(Drive, 0x645420);
    RH_ScopedOverloadedInstall(IsTargetBlocked, "Ped", 0x6452C0, bool(CTaskComplexDriveToPoint::*)(CPed*) const);
    RH_ScopedOverloadedInstall(IsTargetBlocked, "Entities", 0x6432A0, bool(CTaskComplexDriveToPoint::*)(CPed*, CEntity**, int32) const);
}

// 0x63CE00
CTaskComplexDriveToPoint* CTaskComplexDriveToPoint::Constructor(CVehicle* vehicle, const CVector& target, float speed, int32 mode, eModelID carModelIndexToCreate, float targetRadius, eCarDrivingStyle drivingStyle) {
    this->CTaskComplexDriveToPoint::CTaskComplexDriveToPoint(vehicle, target, speed, mode, carModelIndexToCreate, targetRadius, drivingStyle);
    return this;
}

// 0x63CE00
CTaskComplexDriveToPoint::CTaskComplexDriveToPoint(CVehicle* vehicle, const CVector& target, float speed, int32 mode, eModelID carModelIndexToCreate, float targetRadius, eCarDrivingStyle drivingStyle) :
    CTaskComplexCarDrive(vehicle, speed, carModelIndexToCreate, drivingStyle),
    m_vTarget{ target },
    m_iMode{ mode },
    m_fTargetRadius{ targetRadius },
    m_bAchievedTarget{ false }
{
}

// 0x63CE60
eTaskType CTaskComplexDriveToPoint::GetTaskType() const {
    return Type;
}

// 0x63DDE0
CTask* CTaskComplexDriveToPoint::Clone() const {
    return new CTaskComplexDriveToPoint(m_Veh, m_vTarget, m_CruiseSpeed, m_iMode, m_DesiredCarModel, m_fTargetRadius, static_cast<eCarDrivingStyle>(m_CarDrivingStyle));
}

// 0x63CE80
CTask* CTaskComplexDriveToPoint::CreateSubTaskCannotGetInCar(CPed* ped) {
    return new CTaskComplexGoToPointAnyMeans(PEDMOVE_RUN, m_vTarget, 0.5f, m_DesiredCarModel);
}

// 0x63CF00
void CTaskComplexDriveToPoint::SetUpCar() {
    m_OriginalDrivingStyle = m_Veh->m_autoPilot.DrivingMode;
    m_OriginalMission      = m_Veh->m_autoPilot.Mission;
    m_OriginalSpeed        = m_Veh->m_autoPilot.CruiseSpeed;

    m_bIsCarSetUp = true;

    if (m_CruiseSpeed > 0.0f) {
        assert(m_CruiseSpeed < 255.0f);
        m_Veh->m_autoPilot.SetCruiseSpeed((uint8)m_CruiseSpeed);
    }
    m_Veh->m_autoPilot.DrivingMode      = static_cast<eCarDrivingStyle>(m_CarDrivingStyle);
    m_Veh->m_autoPilot.LastTimeNotStuck = CTimer::GetTimeInMS();
}

// 0x645420
CTask* CTaskComplexDriveToPoint::Drive(CPed* ped) {
    const float dist = DistanceBetweenPoints(m_Veh->GetPosition(), m_vTarget);
    if (dist < m_fTargetRadius) {
        m_Veh->m_autoPilot.SetCarMission(MISSION_NONE);
        m_bAchievedTarget = true;
        return CTaskComplexCarDrive::CreateSubTask(TASK_FINISHED, ped);
    }

    if (dist >= 3.0f || m_Veh->m_autoPilot.Mission) {
        if (!m_Veh->m_autoPilot.CruiseSpeed) {
            assert(m_CruiseSpeed < 255.0f);
            m_Veh->m_autoPilot.SetCruiseSpeed((uint8)m_CruiseSpeed);
        }

        if (IsTargetBlocked(ped)) {
            m_bAchievedTarget = true;
            return CTaskComplexCarDrive::CreateSubTask(TASK_FINISHED, ped);
        }

        switch (m_iMode) {
        case DRIVE_TO_POINT_DEFAULT:       CCarAI::GetCarToGoToCoors(m_Veh, m_vTarget, m_CarDrivingStyle, false); break;
        case DRIVE_TO_POINT_ACCURATE:      CCarAI::GetCarToGoToCoorsAccurate(m_Veh, m_vTarget, m_CarDrivingStyle, false); break;
        case DRIVE_TO_POINT_STRAIGHT_LINE: CCarAI::GetCarToGoToCoorsStraightLine(m_Veh, m_vTarget, m_CarDrivingStyle, false); break;
        case DRIVE_TO_POINT_RACING:        CCarAI::GetCarToGoToCoorsRacing(m_Veh, m_vTarget, m_CarDrivingStyle, false); break;
        default:                           break;
        }
        return m_pSubTask;
    }

    m_bAchievedTarget = true;
    return CTaskComplexCarDrive::CreateSubTask(TASK_FINISHED, ped);
}

// 0x6452C0
bool CTaskComplexDriveToPoint::IsTargetBlocked(CPed* ped) const {
    if (DistanceBetweenPointsSquared(ped->GetPosition(), m_vTarget) > sq(6.0f)) {
        return false;
    }

    auto intel = ped->GetIntelligence();
    return IsTargetBlocked(ped, intel->GetPedEntities(), 16) || IsTargetBlocked(ped, intel->GetVehicleEntities(), 16);
}

// 0x6432A0
bool CTaskComplexDriveToPoint::IsTargetBlocked(CPed* ped, CEntity** entities, int32 numEntities) const {
    if (!ped->m_pMyVehicle)
        return false;

    const auto vehicleRadius = ped->m_pMyVehicle->GetColModel()->GetBoundRadius();
    const auto& vehPos = ped->m_pMyVehicle->GetPosition();
    const float vehToTargetDistSq = (vehPos - m_vTarget).SquaredMagnitude();

    for (auto i = 0; i < numEntities; ++i) {
        CEntity* entity = entities[i];
        if (!entity || entity == ped->m_pMyVehicle) {
            continue;
        }

        const auto& entityPos = entity->GetPosition();
        const auto entityRadius = entity->GetColModel()->GetBoundRadius();

        if (DistanceBetweenPointsSquared(entityPos, m_vTarget) > sq(entityRadius)) {
            continue;
        }

        if (sq(entityRadius + vehicleRadius) * 1.5f > vehToTargetDistSq) {
            return true;
        }
    }
    return false;
}
