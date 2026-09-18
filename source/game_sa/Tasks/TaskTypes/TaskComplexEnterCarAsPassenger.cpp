#include "StdInc.h"
#include "TaskComplexEnterCarAsPassenger.h"

void CTaskComplexEnterCarAsPassenger::InjectHooks() {
    RH_ScopedVirtualClass(CTaskComplexEnterCarAsPassenger, 0x86EADC, 11);
    RH_ScopedCategory("Tasks/TaskTypes");

    RH_ScopedInstall(Constructor, 0x640340);
    RH_ScopedInstall(Destructor, 0x640380);
    RH_ScopedVMTInstall(GetTaskType, 0x640370);
    RH_ScopedVMTInstall(Clone, 0x6437F0);
}

// 0x640340
CTaskComplexEnterCarAsPassenger* CTaskComplexEnterCarAsPassenger::Constructor(CVehicle* targetVehicle, int32 nTargetSeat, bool bCarryOnAfterFallingOff) {
    this->CTaskComplexEnterCarAsPassenger::CTaskComplexEnterCarAsPassenger(targetVehicle, nTargetSeat, bCarryOnAfterFallingOff);
    return this;
}

// 0x640340
CTaskComplexEnterCarAsPassenger::CTaskComplexEnterCarAsPassenger(CVehicle* targetVehicle, int32 nTargetSeat, bool bCarryOnAfterFallingOff)
    : CTaskComplexEnterCar(targetVehicle, false, false, false, bCarryOnAfterFallingOff)
{
    m_TargetSeat = nTargetSeat;
}

// For 0x6437F0
CTaskComplexEnterCarAsPassenger::CTaskComplexEnterCarAsPassenger(const CTaskComplexEnterCarAsPassenger& o) :
    CTaskComplexEnterCarAsPassenger{
        o.m_Car,
        o.m_TargetSeat,
        o.m_bCarryOnAfterFallingOff
    }
{
    m_MoveState = o.m_MoveState;
}

// 0x640370
eTaskType CTaskComplexEnterCarAsPassenger::GetTaskType() const {
    return Type;
}

// 0x6437F0
CTask* CTaskComplexEnterCarAsPassenger::Clone() const {
    return new CTaskComplexEnterCarAsPassenger{ *this };
}
