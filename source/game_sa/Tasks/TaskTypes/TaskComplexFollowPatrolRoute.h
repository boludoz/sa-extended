#pragma once

#include "TaskComplex.h"
#include "PatrolRoute.h"

class NOTSA_EXPORT_VTABLE CTaskComplexFollowPatrolRoute : public CTaskComplex {
public:
    static inline auto& ms_patrolRoute = StaticRef<CPatrolRoute>(0xC17CB8);
    static constexpr float ms_fTargetRadius = 0.5f;
    static constexpr float ms_fSlowDownDistance = 3.0f;

    CTaskComplexFollowPatrolRoute(int32 moveState, const CPatrolRoute& route, int32 mode, float targetRadius = ms_fTargetRadius, float slowDownDistance = ms_fSlowDownDistance) {}
    ~CTaskComplexFollowPatrolRoute() override = default;
    eTaskType GetTaskType() const override { return TASK_COMPLEX_FOLLOW_PATROL_ROUTE; }
    CTask* Clone() const override { return nullptr; }
    CTask* CreateNextSubTask(CPed* ped) override { return nullptr; }
    CTask* CreateFirstSubTask(CPed* ped) override { return nullptr; }
    CTask* ControlSubTask(CPed* ped) override { return nullptr; }
};
