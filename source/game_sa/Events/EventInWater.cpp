#include "StdInc.h"
#include "EventInWater.h"

void CEventInWater::InjectHooks() {
    RH_ScopedVirtualClass(CEventInWater, 0x85B4C8, 16);
    RH_ScopedCategory("Events");
    RH_ScopedInstall(Destructor, 0x4B13C0);

    RH_ScopedInstall(Constructor, 0x4B1370);
    RH_ScopedVMTInstall(AffectsPed, 0x4B13D0);
    RH_ScopedVMTInstall(TakesPriorityOver, 0x4B1420);
    RH_ScopedVMTInstall(Clone, 0x4B7810);
}

CEventInWater* CEventInWater::Constructor(float acceleration) {
    this->CEventInWater::CEventInWater(acceleration);
    return this;
}

// 0x4B1370
CEventInWater::CEventInWater(float acceleration) {
    m_acceleration = acceleration;
}

// 0x4B13D0
bool CEventInWater::AffectsPed(CPed* ped) {
    CTask* task = ped->GetTaskManager().GetActiveTask();
    if (!ped->IsPlayer() && task && task->GetTaskType() == TASK_COMPLEX_IN_WATER)
        return false;
    return ped->IsAlive();
}

// 0x4B1420
bool CEventInWater::TakesPriorityOver(const CEvent& refEvent) {
    switch (refEvent.GetEventType()) {
    case EVENT_KNOCK_OFF_BIKE:
    case EVENT_DAMAGE:
    case EVENT_STUCK_IN_AIR:
        if (m_acceleration > 1.0f) {
            return true;
        }
        break;
    }
    return CEvent::TakesPriorityOver(refEvent);
}

// 0x4B7810
CEvent* CEventInWater::Clone() const noexcept {
    return new CEventInWater(m_acceleration);
}
