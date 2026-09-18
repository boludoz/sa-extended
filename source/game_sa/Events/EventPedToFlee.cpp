#include "StdInc.h"

#include "EventPedToFlee.h"

void CEventPedToFlee::InjectHooks() {
    RH_ScopedVirtualClass(CEventPedToFlee, 0x85B178, 16);
    RH_ScopedCategory("Events");
    RH_ScopedInstall(Destructor, 0x4AF2D0);

    RH_ScopedInstall(Constructor, 0x4AF240);
    RH_ScopedVMTInstall(AffectsPed, 0x4AF330);
    RH_ScopedVMTInstall(Clone, 0x4B73D0);
}

CEventPedToFlee* CEventPedToFlee::Constructor(CPed* ped) {
    this->CEventPedToFlee::CEventPedToFlee(ped);
    return this;
}

// 0x4AF240
CEventPedToFlee::CEventPedToFlee(CPed* ped) {
    m_ped = ped;
    CEntity::SafeRegisterRef(m_ped);
}

// 0x4AF2D0
CEventPedToFlee::~CEventPedToFlee() {
    CEntity::SafeCleanUpRef(m_ped);
}

// 0x4AF330
bool CEventPedToFlee::AffectsPed(CPed* ped) {
    return ped->IsAlive();
}

// 0x4B73D0
CEvent* CEventPedToFlee::Clone() const noexcept {
    return new CEventPedToFlee(m_ped);
}
