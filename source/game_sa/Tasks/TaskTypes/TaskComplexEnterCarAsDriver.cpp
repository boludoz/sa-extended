#include "StdInc.h"

#include "TaskComplexEnterCarAsDriver.h"

void CTaskComplexEnterCarAsDriver::InjectHooks() {
    RH_ScopedVirtualClass(CTaskComplexEnterCarAsDriver, 0x86EAAC, 11);
    RH_ScopedCategory("Tasks/TaskTypes");

    RH_ScopedInstall(Constructor, 0x6402F0);
    RH_ScopedVMTInstall(GetTaskType, 0x640320);
    RH_ScopedVMTInstall(Clone, 0x643780);
}

// 0x6402F0
CTaskComplexEnterCarAsDriver* CTaskComplexEnterCarAsDriver::Constructor(CVehicle* targetVehicle) {
    this->CTaskComplexEnterCarAsDriver::CTaskComplexEnterCarAsDriver(targetVehicle);
    return this;
}

// 0x6402F0
CTaskComplexEnterCarAsDriver::CTaskComplexEnterCarAsDriver(CVehicle* targetVehicle) :
    CTaskComplexEnterCar(targetVehicle, true, false, false, false)
{
}

// NOTSA
CTaskComplexEnterCarAsDriver::CTaskComplexEnterCarAsDriver(CVehicle* targetVehicle, eMoveState moveState) :
    CTaskComplexEnterCarAsDriver{ targetVehicle }
{
    m_MoveState = moveState;
}

// For 0x643780
CTaskComplexEnterCarAsDriver::CTaskComplexEnterCarAsDriver(const CTaskComplexEnterCarAsDriver& o) :
    CTaskComplexEnterCarAsDriver{ o.m_Car }
{
    m_MoveState = o.m_MoveState;
}

// 0x640320
eTaskType CTaskComplexEnterCarAsDriver::GetTaskType() const {
    return Type;
}

// 0x643780
CTask* CTaskComplexEnterCarAsDriver::Clone() const {
    return new CTaskComplexEnterCarAsDriver{*this};
}
