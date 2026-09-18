#include "StdInc.h"

#include "EventDraggedOutCar.h"

void CEventDraggedOutCar::InjectHooks() {
    RH_ScopedVirtualClass(CEventDraggedOutCar, 0x85AD30, 17);
    RH_ScopedCategory("Events");

    RH_ScopedInstall(Constructor, 0x4AD250);
    RH_ScopedVMTInstall(AffectsPed, 0x4AD3A0);
    RH_ScopedVMTInstall(AffectsPedGroup, 0x4AD3C0);
    RH_ScopedVMTInstall(CloneEditable, 0x4B6DC0);
}

CEventDraggedOutCar* CEventDraggedOutCar::Constructor(CVehicle* vehicle, CPed* carjacker, bool IsDriverSeat) {
    this->CEventDraggedOutCar::CEventDraggedOutCar(vehicle, carjacker, IsDriverSeat);
    return this;
}

// 0x4AD250
CEventDraggedOutCar::CEventDraggedOutCar(CVehicle* vehicle, CPed* carjacker, bool IsDriverSeat) : CEventEditableResponse() {
    m_CarJacker = carjacker;
    m_Vehicle   = vehicle;
    m_IsDriverSeat = IsDriverSeat;
    CEntity::SafeRegisterRef(m_Vehicle);
    CEntity::SafeRegisterRef(m_CarJacker);
}

// 0x4AD330
CEventDraggedOutCar::~CEventDraggedOutCar() {
    CEntity::SafeCleanUpRef(m_Vehicle);
    CEntity::SafeCleanUpRef(m_CarJacker);
}

// 0x4AD3A0
bool CEventDraggedOutCar::AffectsPed(CPed* ped) {
    return ped->IsAlive() && m_CarJacker;
}

// 0x4AD3C0
bool CEventDraggedOutCar::AffectsPedGroup(CPedGroup* pedGroup) {
    return FindPlayerPed() == pedGroup->GetMembership().GetLeader();
}

// 0x4B6DC0
CEventEditableResponse* CEventDraggedOutCar::CloneEditable() const noexcept {
    return new CEventDraggedOutCar(m_Vehicle, m_CarJacker, m_IsDriverSeat);
}
