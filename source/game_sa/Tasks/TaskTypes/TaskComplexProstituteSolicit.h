/*
    Plugin-SDK file
    Authors: GTA Community. See more here
    https://github.com/DK22Pac/plugin-sdk
    Do not delete this comment block. Respect others' work!
*/
#pragma once

#include "TaskComplex.h"
#include "Ped.h"

class NOTSA_EXPORT_VTABLE CTaskComplexProstituteSolicit : public CTaskComplex {
public:
    CPed*   m_pPunterPed; // player only
    CVector m_InitialVehiclePos;
    int32   m_iLastTimeCarMoving;
    int32   m_iNextTimeToScanForPeds; // when we will scan nearby peds again
    int32   m_iSecondsCounter;                 // when CJ will pay $2 again
    int16   m_iShaggingFreq;            // wait some time and push vehicle again
    int16   m_iShagTimeLeft;

    uint16 m_bWaitingToStopInSecludedSpot : 1;
    uint16 m_bHavingSex : 1;
    uint16 m_bHadEnough : 1;
    uint16 m_bPedsNearby : 1;
    uint16 m_bPedsReallyNear : 1;
    uint16 m_bCopNearby : 1;
    uint16 m_bFirstTime : 1;
    uint16 m_bWaitAfterEnteringCar : 1;

    uint16 m_bSexCamModeEnabled : 1;
    uint16 m_bDoSexAudio : 1;
    uint16 m_bAgreedToSex : 1;
    uint16 m_bNoMoreHealth : 1;
    uint16 m_bHavePrintedSecludedMessage : 1;

public:
    static constexpr auto Type = TASK_COMPLEX_PROSTITUTE_SOLICIT;

    explicit CTaskComplexProstituteSolicit(CPed* client);
    ~CTaskComplexProstituteSolicit() override;

    eTaskType GetTaskType() const override { return Type; } // 0x661AE0
    CTask* Clone() const override { return new CTaskComplexProstituteSolicit(m_pPunterPed); } // 0x6622F0
    bool MakeAbortable(CPed* ped, eAbortPriority priority = ABORT_PRIORITY_URGENT, const CEvent* event = nullptr) override;
    CTask* CreateFirstSubTask(CPed* ped) override;
    CTask* CreateNextSubTask(CPed* ped) override;
    CTask* ControlSubTask(CPed* ped) override;

    CTask* CreateSubTask(eTaskType taskType, CPed* prostitute);
    static void GetRidOfPlayerProstitute();
    static bool IsTaskValid(CPed* prostitute, CPed* ped);
};

VALIDATE_SIZE(CTaskComplexProstituteSolicit, 0x30);
