#include "StdInc.h"
#include "EventCreatePartnerTask.h"

void CEventCreatePartnerTask::InjectHooks() {
    RH_ScopedVirtualClass(CEventCreatePartnerTask, 0x86C6D0, 16);
    RH_ScopedCategory("Events");
    RH_ScopedInstall(Destructor, 0x5F6200);

    RH_ScopedInstall(Constructor, 0x5F6190);
    RH_ScopedVMTInstall(AffectsPed, 0x5F62F0);
    RH_ScopedVMTInstall(Clone, 0x5F6280);
}

CEventCreatePartnerTask* CEventCreatePartnerTask::Constructor(int32 type, CPed* partner, bool isLeadSpeaker, float meetDist) {
    this->CEventCreatePartnerTask::CEventCreatePartnerTask(type, partner, isLeadSpeaker, meetDist);
    return this;
}

// 0x5F6190
CEventCreatePartnerTask::CEventCreatePartnerTask(int32 type, CPed* partner, bool isLeadSpeaker, float meetDist) :
    m_partnerType{type},
    m_partner{partner},
    m_isLeadSpeaker{isLeadSpeaker},
    m_meetDist{meetDist}
{
    CEntity::SafeRegisterRef(m_partner);
}

// 0x5F6200
CEventCreatePartnerTask::~CEventCreatePartnerTask() {
    CEntity::SafeCleanUpRef(m_partner);
}

// 0x5F62F0
bool CEventCreatePartnerTask::AffectsPed(CPed* ped) {
    return true;
}

// 0x5F6280
CEvent* CEventCreatePartnerTask::Clone() const noexcept {
    return new CEventCreatePartnerTask(m_partnerType, m_partner, m_isLeadSpeaker, m_meetDist);
}
