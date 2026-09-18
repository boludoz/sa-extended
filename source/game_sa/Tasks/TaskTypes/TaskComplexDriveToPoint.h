#pragma once

#include "TaskComplexCarDrive.h"

class CVehicle;

enum eDriveToPointMode : int32 {
    DRIVE_TO_POINT_DEFAULT       = 0,
    DRIVE_TO_POINT_ACCURATE      = 1,
    DRIVE_TO_POINT_STRAIGHT_LINE = 2,
    DRIVE_TO_POINT_RACING        = 3,
};

class NOTSA_EXPORT_VTABLE CTaskComplexDriveToPoint : public CTaskComplexCarDrive {
public:
    static constexpr auto Type = TASK_COMPLEX_CAR_DRIVE_TO_POINT;

    CVector m_vTarget;
    int32   m_iMode;
    float   m_fTargetRadius;
    bool    m_bAchievedTarget;

public:
    CTaskComplexDriveToPoint(CVehicle* vehicle, const CVector& target, float speed, int32 mode, eModelID carModelIndexToCreate, float targetRadius, eCarDrivingStyle drivingStyle);
    ~CTaskComplexDriveToPoint() override = default;

    eTaskType GetTaskType() const override;
    CTask* Clone() const override;

    void SetUpCar() override;
    CTask* CreateSubTaskCannotGetInCar(CPed* ped) override;
    CTask* Drive(CPed* ped) override;

    bool IsTargetBlocked(CPed* ped) const;
    bool IsTargetBlocked(CPed* ped, CEntity** entities, int32 numEntities) const;

    bool HasAchievedTarget() const { return m_bAchievedTarget; }
    void SetTarget(const CVector& target) { m_vTarget = target; }

private:
    friend void InjectHooksMain();
    static void InjectHooks();

    CTaskComplexDriveToPoint* Constructor(CVehicle* vehicle, const CVector& target, float speed, int32 mode, eModelID carModelIndexToCreate, float targetRadius, eCarDrivingStyle drivingStyle);
};
VALIDATE_SIZE(CTaskComplexDriveToPoint, 0x3C);
