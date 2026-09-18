#pragma once

#include "Event.h"

#include "Event.h"

class NOTSA_EXPORT_VTABLE CEventInWater : public CEvent {
public:
    float m_acceleration;

public:
    CEventInWater(float acceleration);
    ~CEventInWater() override = default;

    eEventType GetEventType() const override { return EVENT_IN_WATER; }
    int32 GetEventPriority() const override { return 62; }
    int32 GetLifeTime() override { return 0; }
    CEvent* Clone() const noexcept override;
    bool AffectsPed(CPed* ped) override;
    bool TakesPriorityOver(const CEvent& refEvent) override;

private:
    friend void InjectHooksMain();
    static void InjectHooks();

    CEventInWater* Constructor(float acceleration);
    CEventInWater* Destructor() {
        this->CEventInWater::~CEventInWater();
        return this;
    }
};
VALIDATE_SIZE(CEventInWater, 0x10);
