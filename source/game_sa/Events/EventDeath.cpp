#include "StdInc.h"

#include "EventDeath.h"

void CEventDeath::InjectHooks() {
    RH_ScopedVirtualClass(CEventDeath, 0x85ADC0, 16);
    RH_ScopedCategory("Events");

    RH_ScopedOverloadedInstall(Constructor1, "1", 0x4ADDF0, CEventDeath*(CEventDeath::*)(bool));
    RH_ScopedOverloadedInstall(Constructor2, "2", 0x4ADE50, CEventDeath*(CEventDeath::*)(bool, uint32));
    RH_ScopedVMTInstall(AffectsPed, 0x4ADE80);
    RH_ScopedVMTInstall(Clone, 0x4B6E30);
}

// 0x4ADDF0
CEventDeath* CEventDeath::Constructor1(bool bDrowning) {
    this->CEventDeath::CEventDeath(bDrowning);
    return this;
}

// 0x4ADE50
CEventDeath* CEventDeath::Constructor2(bool bDrowning, uint32 deathTimeInMs) {
    this->CEventDeath::CEventDeath(bDrowning, deathTimeInMs);
    return this;
}

// 0x4ADDF0
CEventDeath::CEventDeath(bool bDrowning) : CEvent() {
    m_bDrowning = bDrowning;
    m_deathTimeInMs = CTimer::GetTimeInMS();
}

// 0x4ADE50
CEventDeath::CEventDeath(bool bDrowning, uint32 deathTimeInMs) : CEvent() {
    m_bDrowning = bDrowning;
    m_deathTimeInMs = deathTimeInMs;
}

// 0x4ADE80
bool CEventDeath::AffectsPed(CPed* ped) {
    return true;
}

// 0x4B6E30
CEvent* CEventDeath::Clone() const noexcept {
    return new CEventDeath(m_bDrowning, m_deathTimeInMs);
}
