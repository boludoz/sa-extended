#pragma once

#include "TaskSimple.h"

class NOTSA_EXPORT_VTABLE CTaskSimpleNone : public CTaskSimple {
public:
    static constexpr auto Type = TASK_NONE;

    CTaskSimpleNone() = default;
    ~CTaskSimpleNone() override = default;

    eTaskType GetTaskType() const override;
    CTask* Clone() const override;
    bool MakeAbortable(CPed* ped, eAbortPriority priority = ABORT_PRIORITY_URGENT, const CEvent* event = nullptr) override;
    bool ProcessPed(CPed* ped) override;

private:
    friend void InjectHooksMain();
    static void InjectHooks();
};
VALIDATE_SIZE(CTaskSimpleNone, 0x8);
