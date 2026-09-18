#include "StdInc.h"
#include "EventFireNearby.h"

void CEventFireNearby::InjectHooks() {
    RH_ScopedVirtualClass(CEventFireNearby, 0x85B6E8, 17);
    RH_ScopedCategory("Events");
    RH_ScopedInstall(Destructor, 0x4B1F80);

    RH_ScopedInstall(Constructor, 0x4B1F10);
    RH_ScopedVMTInstall(AffectsPed, 0x4B1F90);
    RH_ScopedVMTInstall(TakesPriorityOver, 0x4B1FC0);
    RH_ScopedVMTInstall(CloneEditable, 0x4B7A70);
}

CEventFireNearby* CEventFireNearby::Constructor(const CVector& position) {
    this->CEventFireNearby::CEventFireNearby(position);
    return this;
}

// 0x4B1F10
CEventFireNearby::CEventFireNearby(const CVector& position) : CEventEditableResponse() {
    m_position = position;
}

// 0x4B1F90
bool CEventFireNearby::AffectsPed(CPed* ped) {
    return !ped->GetTaskManager().Has<TASK_COMPLEX_EXTINGUISH_FIRES>() && ped->IsAlive();
}

// 0x4B1FC0
bool CEventFireNearby::TakesPriorityOver(const CEvent& refevent) {
    return true;
}

// 0x4B7A70
CEventFireNearby* CEventFireNearby::CloneEditable() const noexcept {
    return new CEventFireNearby(m_position);
}
