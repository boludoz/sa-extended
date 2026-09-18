#pragma once

#include "EventEditableResponse.h"


class NOTSA_EXPORT_VTABLE CEventSpecial : public CEventEditableResponse {
public:
    CEventSpecial() = default; // 0x4B1AE0
    ~CEventSpecial() override = default; // 0x4B6750 ?

    eEventType GetEventType() const override { return EVENT_SPECIAL; }
    int32 GetEventPriority() const override { return 52; }
    int32 GetLifeTime() override { return 0; }
    bool AffectsPed(CPed* ped) override;
    CEventSpecial* CloneEditable() const noexcept override;

private:
    friend void InjectHooksMain();
    static void InjectHooks();

    CEventSpecial* Constructor();
};
VALIDATE_SIZE(CEventSpecial, 0x14);
