#pragma once

#include "Event.h"

#include "Event.h"


class NOTSA_EXPORT_VTABLE CEventRevived : public CEvent {
public:
    CEventRevived() = default;
    ~CEventRevived() override = default;

    eEventType GetEventType() const override { return EVENT_REVIVE; }
    int32 GetEventPriority() const override { return 74; }
    int32 GetLifeTime() override { return 0; }
    CEventRevived* Clone() const noexcept override;
    bool AffectsPed(CPed* ped) override;

private:
    friend void InjectHooksMain();
    static void InjectHooks();

    CEventRevived* Constructor();
    CEventRevived* Destructor() {
        this->CEventRevived::~CEventRevived();
        return this;
    }
};
VALIDATE_SIZE(CEventRevived, 0xC);
