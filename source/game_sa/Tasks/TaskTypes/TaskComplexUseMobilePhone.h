/*
    Plugin-SDK file
    Authors: GTA Community. See more here
    https://github.com/DK22Pac/plugin-sdk
    Do not delete this comment block. Respect others' work!
*/
#pragma once

#include "TaskComplex.h"
#include "TaskTimer.h"

class NOTSA_EXPORT_VTABLE CTaskComplexUseMobilePhone : public CTaskComplex {
public:
    int32      m_nDuration;
    CTaskTimer m_timer;
    bool       m_bIsAborting;
    bool       m_bQuit;

public:
    static constexpr auto Type = TASK_COMPLEX_USE_MOBILE_PHONE;

    CTaskComplexUseMobilePhone(int32 nDuration);

    // 0x634A40
    void Stop(CPed* ped) { plugin::CallMethod<0x634A40, CTaskComplexUseMobilePhone*, CPed*>(this, ped); }

    //! NOTSA: not reversed yet (abstract here), so build it through the original ctor (0x6348A0)
    static CTaskComplexUseMobilePhone* Create(int32 duration) {
        auto* task = static_cast<CTaskComplexUseMobilePhone*>(CTask::operator new(sizeof(CTaskComplexUseMobilePhone)));
        plugin::CallMethod<0x6348A0, CTaskComplexUseMobilePhone*, int32>(task, duration);
        return task;
    }
};

VALIDATE_SIZE(CTaskComplexUseMobilePhone, 0x20);
