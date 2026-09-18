#pragma once

#include "TaskSimple.h"

class NOTSA_EXPORT_VTABLE CTaskSimpleSetStayInSamePlace : public CTaskSimple {
public:
    bool m_bStayInSamePlace;

public:
    static constexpr auto Type = TASK_SIMPLE_SET_STAY_IN_SAME_PLACE;

    explicit CTaskSimpleSetStayInSamePlace(bool bStayInSamePlace);
    ~CTaskSimpleSetStayInSamePlace() override = default;

    eTaskType GetTaskType() const override;
    CTask* Clone() const override;
    bool MakeAbortable(class CPed* ped, eAbortPriority priority = ABORT_PRIORITY_URGENT, const CEvent* event = nullptr) override;
    bool ProcessPed(CPed* ped) override;

private:
    friend void InjectHooksMain();
    static void InjectHooks();

    CTaskSimpleSetStayInSamePlace* Constructor(bool bStayInSamePlace);
    CTaskSimpleSetStayInSamePlace* Destructor() {
        this->CTaskSimpleSetStayInSamePlace::~CTaskSimpleSetStayInSamePlace();
        return this;
    }
};
VALIDATE_SIZE(CTaskSimpleSetStayInSamePlace, 0xC);
