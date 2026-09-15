#pragma once

#include "TaskComplex.h"

class CEntity;

class CTaskComplexGoPickUpEntity : public CTaskComplex {
public:
    static constexpr auto Type = TASK_COMPLEX_GO_PICKUP_ENTITY;

    CEntity* m_pEntity{};
    CVector  m_vecPosition{};
    CVector  m_vecPickupPosition{};
    uint32   m_nTimePassedSinceLastSubTaskCreatedInMs{};
    int32    m_nAnimGroupId{};
    bool     m_bAnimBlockReferenced{};
    char     _pad[3]{};

    CTaskComplexGoPickUpEntity(CEntity* entity = nullptr, int32 animGroupId = 0)
        : m_pEntity(entity), m_nAnimGroupId(animGroupId) {}
    ~CTaskComplexGoPickUpEntity() override = default;

    eTaskType GetTaskType() const override { return Type; }
    CTask* Clone() const override { return new CTaskComplexGoPickUpEntity(*this); }
    bool MakeAbortable(CPed* ped, eAbortPriority priority = ABORT_PRIORITY_URGENT, const CEvent* event = nullptr) override { return false; }
    CTask* CreateNextSubTask(CPed* ped) override { return nullptr; }
    CTask* CreateFirstSubTask(CPed* ped) override { return nullptr; }
    CTask* ControlSubTask(CPed* ped) override { return nullptr; }
};

VALIDATE_SIZE(CTaskComplexGoPickUpEntity, 0x34);