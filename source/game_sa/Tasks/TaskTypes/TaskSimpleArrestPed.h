#pragma once

#include "TaskSimple.h"

class CPed;
class CAnimBlendAssociation;

class NOTSA_EXPORT_VTABLE CTaskSimpleArrestPed : public CTaskSimple {
public:
    CPed*                  m_Ped;
    bool                   m_bFinished;
    CAnimBlendAssociation* m_Assoc;

public:
    static constexpr auto Type = TASK_SIMPLE_ARREST_PED;

    explicit CTaskSimpleArrestPed(CPed* ped);
    ~CTaskSimpleArrestPed() override;

    eTaskType GetTaskType() const override; // 0x68B680
    CTask* Clone() const override;          // 0x68CD10
    bool MakeAbortable(CPed* ped, eAbortPriority priority = ABORT_PRIORITY_URGENT, const CEvent* event = nullptr) override;
    bool ProcessPed(CPed* ped) override;
    void StartAnim(CPed* ped);

    static void FinishAnimArrestPedCB(CAnimBlendAssociation* assoc, void* data);

private:
    friend void CTaskSimpleArrestPed__InjectHooks();
    CTaskSimpleArrestPed* Constructor(CPed* ped) { this->CTaskSimpleArrestPed::CTaskSimpleArrestPed(ped); return this; }
    CTaskSimpleArrestPed* Destructor() { this->CTaskSimpleArrestPed::~CTaskSimpleArrestPed(); return this; }
};
VALIDATE_SIZE(CTaskSimpleArrestPed, 0x14);
extern void CTaskSimpleArrestPed__InjectHooks();
