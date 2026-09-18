#include "StdInc.h"

#include "EventGroupEvent.h"

void CEventGroupEvent::InjectHooks() {
    RH_ScopedVirtualClass(CEventGroupEvent, 0x85AE48, 16);
    RH_ScopedCategory("Events");

    RH_ScopedInstall(Constructor, 0x4ADFD0);
    RH_ScopedVMTInstall(AffectsPed, 0x4AE0D0);
    RH_ScopedVMTInstall(AffectsPedGroup, 0x4AE0E0);
    RH_ScopedInstall(BaseEventTakesPriorityOverBaseEvent, 0x4AE100);
    RH_ScopedVMTInstall(Clone, 0x4B6EE0);
}

CEventGroupEvent* CEventGroupEvent::Constructor(CPed* ped, CEvent* event) {
    this->CEventGroupEvent::CEventGroupEvent(ped, event);
    return this;
}

// 0x4ADFD0
CEventGroupEvent::CEventGroupEvent(CPed* ped, CEvent* event) : CEvent() {
    m_ped = ped;
    m_event = event;
    CEntity::SafeRegisterRef(m_ped);
}

// 0x4AE070
CEventGroupEvent::~CEventGroupEvent() {
    CEntity::SafeCleanUpRef(m_ped);
    delete m_event;
}

// 0x4AE0D0
bool CEventGroupEvent::AffectsPed(CPed* ped) {
    return false;
}

// 0x4AE0E0
bool CEventGroupEvent::AffectsPedGroup(CPedGroup* pedGroup) {
    return pedGroup->GetMembership().IsMember(m_ped);
}

// 0x4B6EE0
CEvent* CEventGroupEvent::Clone() const noexcept {
    return new CEventGroupEvent(m_ped, m_event->Clone());
}

// NOTSA, inlined
bool CEventGroupEvent::IsPriorityEvent() const {
    switch (m_event->GetEventType()) {
    case EVENT_LEADER_ENTERED_CAR_AS_DRIVER:
    case EVENT_LEADER_EXITED_CAR_AS_DRIVER:
    case EVENT_LEADER_QUIT_ENTERING_CAR_AS_DRIVER:
    case EVENT_PLAYER_COMMAND_TO_GROUP:
    case EVENT_PLAYER_COMMAND_TO_GROUP_GATHER:
    case EVENT_DRAGGED_OUT_CAR:
    case EVENT_LEADER_ENTRY_EXIT:
        return true;
    }
    return false;
}

// 0x4AE100
bool CEventGroupEvent::BaseEventTakesPriorityOverBaseEvent(const CEventGroupEvent& other) {
    return IsPriorityEvent()
        || !other.IsPriorityEvent() && GetEvent().TakesPriorityOver(other.GetEvent());
}
