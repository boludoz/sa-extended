#include "StdInc.h"

#include "TaskSimpleSetStayInSamePlace.h"

void CTaskSimpleSetStayInSamePlace::InjectHooks() {
    RH_ScopedVirtualClass(CTaskSimpleSetStayInSamePlace, 0x86DD50, 9);
    RH_ScopedCategory("Tasks/TaskTypes");

    RH_ScopedInstall(Constructor, 0x62F590);
    RH_ScopedInstall(Destructor, 0x62F5D0);
    RH_ScopedVMTInstall(GetTaskType, 0x62F5B0);
    RH_ScopedVMTInstall(MakeAbortable, 0x62F5C0);
    RH_ScopedVMTInstall(ProcessPed, 0x62F5E0);
    RH_ScopedVMTInstall(Clone, 0x635BD0);
}

// 0x62F590
CTaskSimpleSetStayInSamePlace* CTaskSimpleSetStayInSamePlace::Constructor(bool bStayInSamePlace) {
    this->CTaskSimpleSetStayInSamePlace::CTaskSimpleSetStayInSamePlace(bStayInSamePlace);
    return this;
}

// 0x62F590
CTaskSimpleSetStayInSamePlace::CTaskSimpleSetStayInSamePlace(bool bStayInSamePlace) : CTaskSimple() {
    m_bStayInSamePlace = bStayInSamePlace;
}

// 0x62F5B0
eTaskType CTaskSimpleSetStayInSamePlace::GetTaskType() const {
    return Type;
}

// 0x62F5C0
bool CTaskSimpleSetStayInSamePlace::MakeAbortable(CPed* ped, eAbortPriority priority, const CEvent* event) {
    return true;
}

// 0x62F5E0
bool CTaskSimpleSetStayInSamePlace::ProcessPed(CPed* ped) {
    ped->bStayInSamePlace = m_bStayInSamePlace;
    return true;
}

// 0x635BD0
CTask* CTaskSimpleSetStayInSamePlace::Clone() const {
    return new CTaskSimpleSetStayInSamePlace(m_bStayInSamePlace);
}
