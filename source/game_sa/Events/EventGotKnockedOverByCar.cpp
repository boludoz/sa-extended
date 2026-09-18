#include "StdInc.h"

#include "EventGotKnockedOverByCar.h"

void CEventGotKnockedOverByCar::InjectHooks() {
    RH_ScopedVirtualClass(CEventGotKnockedOverByCar, 0x85B618, 17);
    RH_ScopedCategory("Events");

    RH_ScopedInstall(Constructor, 0x4B1B60);
    RH_ScopedVMTInstall(AffectsPed, 0x4B1C70);
    RH_ScopedVMTInstall(CloneEditable, 0x4B7960);
}

CEventGotKnockedOverByCar* CEventGotKnockedOverByCar::Constructor(CVehicle* vehicle) {
    this->CEventGotKnockedOverByCar::CEventGotKnockedOverByCar(vehicle);
    return this;
}

// 0x4B1B60
CEventGotKnockedOverByCar::CEventGotKnockedOverByCar(CVehicle* vehicle) {
    m_vehicle = vehicle;
    CEntity::SafeRegisterRef(m_vehicle);
}

// 0x4B1C10
CEventGotKnockedOverByCar::~CEventGotKnockedOverByCar() {
    CEntity::SafeCleanUpRef(m_vehicle);
}

// 0x4B1C70
bool CEventGotKnockedOverByCar::AffectsPed(CPed* ped) {
    if (m_vehicle && !ped->IsPlayer())
        return ped->IsAlive();
    return false;
}

// 0x4B7960
CEventEditableResponse* CEventGotKnockedOverByCar::CloneEditable() const noexcept {
    return new CEventGotKnockedOverByCar(m_vehicle);
}
