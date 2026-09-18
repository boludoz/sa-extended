#pragma once

#include "Event.h"
#include "Ped.h"

class NOTSA_EXPORT_VTABLE CEventPedToFlee : public CEvent {
public:
    CPed* m_ped;

public:
    explicit CEventPedToFlee(CPed* ped);
    ~CEventPedToFlee() override;

    eEventType GetEventType() const override { return EVENT_PED_TO_FLEE; }
    int32 GetEventPriority() const override { return 2; }
    int32 GetLifeTime() override { return 0; }
    CEvent* Clone() const noexcept override;
    bool AffectsPed(CPed* ped) override;

private:
    friend void InjectHooksMain();
    static void InjectHooks();

    CEventPedToFlee* Constructor(CPed* ped);
};
VALIDATE_SIZE(CEventPedToFlee, 0x10);
