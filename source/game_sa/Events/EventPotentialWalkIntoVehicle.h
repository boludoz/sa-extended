#pragma once

#include "EventEditableResponse.h"
#include "Vector.h"
#include "Enums/eMoveState.h"
#include "TaskComplexEnterCarAsDriver.h"
#include "TaskSimpleGoToPoint.h"
#include "TaskComplexWalkRoundCar.h"

class NOTSA_EXPORT_VTABLE CEventPotentialWalkIntoVehicle : public CEventEditableResponse {
public:
    CVehicle* m_vehicle;
    int32     m_moveState;

public:
    CEventPotentialWalkIntoVehicle(CVehicle* vehicle, int32 moveState);
    ~CEventPotentialWalkIntoVehicle() override;

    eEventType GetEventType() const override { return EVENT_POTENTIAL_WALK_INTO_VEHICLE; }
    int32 GetEventPriority() const override { return 29; }
    int32 GetLifeTime() override { return 0; }
    CEventPotentialWalkIntoVehicle* CloneEditable() const noexcept override;
    bool AffectsPed(CPed* ped) override;
    bool TakesPriorityOver(const CEvent& refEvent) override;

private:
    friend void InjectHooksMain();
    static void InjectHooks();

    CEventPotentialWalkIntoVehicle* Constructor(CVehicle* vehicle, int32 moveState);
    CEventPotentialWalkIntoVehicle* Destructor() {
        this->CEventPotentialWalkIntoVehicle::~CEventPotentialWalkIntoVehicle();
        return this;
    }
};
VALIDATE_SIZE(CEventPotentialWalkIntoVehicle, 0x1C);
