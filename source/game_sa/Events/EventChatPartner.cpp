#include "StdInc.h"
#include "EventChatPartner.h"

void CEventChatPartner::InjectHooks() {
    RH_ScopedVirtualClass(CEventChatPartner, 0x85B070, 16);
    RH_ScopedCategory("Events");

    RH_ScopedInstall(Constructor, 0x4AECD0);
    RH_ScopedVMTInstall(AffectsPed, 0x4AEDC0);
    RH_ScopedVMTInstall(Clone, 0x4B7210);
}

CEventChatPartner* CEventChatPartner::Constructor(bool leadSpeaker, CPed* partner) {
    this->CEventChatPartner::CEventChatPartner(leadSpeaker, partner);
    return this;
}

// 0x4AECD0
CEventChatPartner::CEventChatPartner(bool leadSpeaker, CPed* partner) : CEvent() {
    m_leadSpeaker = leadSpeaker;
    m_partner = partner;
    CEntity::SafeRegisterRef(m_partner);
}

// 0x4AED60
CEventChatPartner::~CEventChatPartner() {
    CEntity::SafeCleanUpRef(m_partner);
}

// 0x4AEDC0
bool CEventChatPartner::AffectsPed(CPed* ped) {
    return ped->IsAlive() && m_partner;
}

// 0x4B7210
CEventChatPartner* CEventChatPartner::Clone() const noexcept {
    return new CEventChatPartner(m_leadSpeaker, m_partner);
}
