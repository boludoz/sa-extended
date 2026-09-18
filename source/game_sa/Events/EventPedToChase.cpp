#include "StdInc.h"

#include "EventPedToChase.h"

void CEventPedToChase::InjectHooks() {
    RH_ScopedVirtualClass(CEventPedToChase, 0x85B138, 16);
    RH_ScopedCategory("Events");

    RH_ScopedInstall(Constructor, 0x4AF130);
    RH_ScopedVMTInstall(AffectsPed, 0x4AF220);
    RH_ScopedVMTInstall(Clone, 0x4B7360);
}

CEventPedToChase* CEventPedToChase::Constructor(CPed* ped) {
    this->CEventPedToChase::CEventPedToChase(ped);
    return this;
}

// 0x4AF130
CEventPedToChase::CEventPedToChase(CPed* ped) {
    m_ped = ped;
    CEntity::SafeRegisterRef(m_ped);
}

// 0x4AF1C0
CEventPedToChase::~CEventPedToChase() {
    CEntity::SafeCleanUpRef(m_ped);
}

// 0x4AF220
bool CEventPedToChase::AffectsPed(CPed* ped) {
    return ped->IsAlive();
}

// 0x4B7360
CEvent* CEventPedToChase::Clone() const noexcept {
    return new CEventPedToChase(m_ped);
}
