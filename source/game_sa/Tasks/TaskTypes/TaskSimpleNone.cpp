#include "StdInc.h"
#include "TaskSimpleNone.h"

void CTaskSimpleNone::InjectHooks() {
    RH_ScopedVirtualClass(CTaskSimpleNone, 0x859CFC, 9);
    RH_ScopedCategory("Tasks/TaskTypes");

    RH_ScopedVMTInstall(Clone, 0x463570);
    RH_ScopedVMTInstall(GetTaskType, 0x4635C0);
    RH_ScopedVMTInstall(MakeAbortable, 0x4635D0);
    RH_ScopedVMTInstall(ProcessPed, 0x4635E0);
}

// 0x463570
CTask* CTaskSimpleNone::Clone() const {
    return new CTaskSimpleNone();
}

// 0x4635C0
eTaskType CTaskSimpleNone::GetTaskType() const {
    return Type;
}

// 0x4635D0
bool CTaskSimpleNone::MakeAbortable(CPed* ped, eAbortPriority priority, const CEvent* event) {
    return true;
}

// 0x4635E0
bool CTaskSimpleNone::ProcessPed(CPed* ped) {
    return true;
}
