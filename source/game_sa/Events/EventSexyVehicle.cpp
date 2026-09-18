#include "StdInc.h"
#include "EventSexyVehicle.h"

void CEventSexyVehicle::InjectHooks() {
    RH_ScopedVirtualClass(CEventSexyVehicle, 0x85B0F8, 16);
    RH_ScopedCategory("Events");

    RH_ScopedInstall(Constructor, 0x4AF010);
    RH_ScopedVMTInstall(AffectsPed, 0x4AF100);
    RH_ScopedVMTInstall(Clone, 0x4B72F0);
}

CEventSexyVehicle* CEventSexyVehicle::Constructor(CVehicle* vehicle) {
    this->CEventSexyVehicle::CEventSexyVehicle(vehicle);
    return this;
}

// 0x4AF010
CEventSexyVehicle::CEventSexyVehicle(CVehicle* vehicle) : CEvent() {
    m_vehicle = vehicle;
    CEntity::SafeRegisterRef(m_vehicle);
}

// 0x4AF0A0
CEventSexyVehicle::~CEventSexyVehicle() {
    CEntity::SafeCleanUpRef(m_vehicle);
}

// 0x4AF100
bool CEventSexyVehicle::AffectsPed(CPed* ped) {
    return ped->IsAlive() && m_vehicle;
}

// 0x4B72F0
CEventSexyVehicle* CEventSexyVehicle::Clone() const noexcept {
    return new CEventSexyVehicle(m_vehicle);
}
