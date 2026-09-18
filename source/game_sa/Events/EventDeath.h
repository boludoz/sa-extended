#pragma once

#include "Event.h"

class NOTSA_EXPORT_VTABLE CEventDeath : public CEvent {
    bool   m_bDrowning;
    uint32 m_deathTimeInMs;

public:
    CEventDeath(bool bDrowning);
    CEventDeath(bool bDrowning, uint32 deathTimeInMs);
    ~CEventDeath() override = default;

    eEventType GetEventType() const override { return EVENT_DEATH; }
    int32 GetEventPriority() const override { return 73; }
    int32 GetLifeTime() override { return 0; }
    CEvent* Clone() const noexcept override;
    bool AffectsPed(CPed* ped) override;

    auto GetDeathTime() const { return m_deathTimeInMs; }
    auto HasDrowned()   const { return m_bDrowning; }

private:
    friend void InjectHooksMain();
    static void InjectHooks();

    CEventDeath* Constructor1(bool bDrowning);
    CEventDeath* Constructor2(bool bDrowning, uint32 deathTimeInMs);
    CEventDeath* Destructor() {
        this->CEventDeath::~CEventDeath();
        return this;
    }
};
VALIDATE_SIZE(CEventDeath, 0x14);
