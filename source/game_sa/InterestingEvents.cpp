#include "StdInc.h"

#include "InterestingEvents.h"

auto& g_InterestingEvents = StaticRef<CInterestingEvents>(0xC0B058);

/*
 * Commented hooks aren't tested.
 * */
void CInterestingEvents::InjectHooks() {
    RH_ScopedClass(CInterestingEvents);
    RH_ScopedCategoryGlobal();

    RH_ScopedInstall(Constructor, 0x6023A0); // ASM Checked
    RH_ScopedInstall(Destructor, 0x856880); // ASM Checked
    RH_ScopedInstall(Add, 0x602590); // ASM Checked
    RH_ScopedInstall(ScanForNearbyEntities, 0x605A30); // ASM Checked
    RH_ScopedInstall(GetInterestingEvent, 0x6028A0); // ASM Checked
    RH_ScopedInstall(InvalidateEvent, 0x602960);
    RH_ScopedInstall(InvalidateNonVisibleEvents, 0x6029C0);
}

// 0x6023A0
CInterestingEvents::CInterestingEvents() {
    m_bIsActive = false;

    m_bIgnoreEventsBehindPlayer = true;
    m_bWaitForEventDurationToComplete = true;
    m_bUseTimeDelayBeforeAddingSimilarEvent = true;
    m_iLookingAtEvent = -1;
    m_iLastScanTime = 0;
    m_fEventRadius = 30.0f;
    m_iCurrentFrameCounter = CTimer::GetFrameCounter() - 1;

    for (auto& event : m_Events) {
        event.m_eType = ENone;
        event.m_iStartTime = 0;
        event.m_pEntity = nullptr;
    }

    for (int32 i = 0; i < ENumCategories; i++) {
        m_NextTimeToAcceptEvents[i] = 0;
        m_EventDurations[i] = 2000;
        m_EventPriorities[i] = 5;
    }

    m_EventPriorities[EPedGotKilled] = 10;
    m_EventPriorities[EExplosion] = 10;
    m_EventPriorities[ESwatTeamAbseiling] = 9;
    m_EventPriorities[ECopKillingCriminal] = 9;
    m_EventPriorities[EGangFight] = 9;
    m_EventPriorities[EGangAttackingPed] = 9;
    m_EventPriorities[EGunshotFired] = 9;
    m_EventPriorities[EHelicopterOverhead] = 8;
    m_EventPriorities[ECarJacking] = 7;
    m_EventPriorities[ERoadRage] = 7;
    m_EventPriorities[EFistFight] = 6;
    m_EventPriorities[ECarCrash] = 8;
    m_EventPriorities[EPedKnockedOffBike] = 9;
    m_EventPriorities[EPedRunOver] = 9;
    m_EventPriorities[EMadDriver] = 5;
    m_EventPriorities[EEmergencyServicesArrived] = 6;
    m_EventPriorities[EPanickedPed] = 6;
    m_EventPriorities[EPedRevived] = 6;
    m_EventPriorities[EPlaneFlyby] = 5;
    m_EventPriorities[ESexyPed] = 4;
    m_EventPriorities[ESexyCar] = 4;
    m_EventPriorities[EGangMemberNearby] = 2;
    m_EventPriorities[ECriminalNearby] = 2;
    m_EventPriorities[ECopNearby] = 2;
    m_EventPriorities[EProzzyNearby] = 2;
    m_EventPriorities[EPedUsingAttractor] = 1;
    m_EventPriorities[EPedSunbathing] = 1;
    m_EventPriorities[EPedsChatting] = 1;

    m_EventDurations[EPedGotKilled] = 4000;
    m_EventDurations[EExplosion] = 4000;
    m_EventDurations[ESwatTeamAbseiling] = 8000;
    m_EventDurations[ECopKillingCriminal] = 6000;
    m_EventDurations[EGangFight] = 6000;
    m_EventDurations[EGangAttackingPed] = 6000;
    m_EventDurations[EGunshotFired] = 5000;
    m_EventDurations[EHelicopterOverhead] = 8000;
    m_EventDurations[ECarJacking] = 6000;
    m_EventDurations[ERoadRage] = 6000;
    m_EventDurations[EFistFight] = 5000;
    m_EventDurations[ECarCrash] = 6000;
    m_EventDurations[EPedKnockedOffBike] = 6000;
    m_EventDurations[EPedRunOver] = 6000;
    m_EventDurations[EMadDriver] = 6000;
    m_EventDurations[EEmergencyServicesArrived] = 8000;
    m_EventDurations[EPanickedPed] = 5000;
    m_EventDurations[EPedRevived] = 6000;
    m_EventDurations[EPlaneFlyby] = 6000;
    m_EventDurations[ESexyPed] = 3000;
    m_EventDurations[ESexyCar] = 3000;
    m_EventDurations[EGangMemberNearby] = 3000;
    m_EventDurations[ECriminalNearby] = 3000;
    m_EventDurations[ECopNearby] = 3000;
    m_EventDurations[EProzzyNearby] = 3000;
    m_EventDurations[EPedUsingAttractor] = 5000;
    m_EventDurations[EPedSunbathing] = 5000;
    m_EventDurations[EPedsChatting] = 5000;
}

CInterestingEvents* CInterestingEvents::Constructor() {
    this->CInterestingEvents::CInterestingEvents();
    return this;
}

// 0x856880
CInterestingEvents::~CInterestingEvents() {
    for (auto& event : g_InterestingEvents.m_Events) {
        CEntity::ClearReference(event.m_pEntity);
    }
}

CInterestingEvents* CInterestingEvents::Destructor() {
    g_InterestingEvents.~CInterestingEvents();
    return &g_InterestingEvents;
}

// 0x602590
void CInterestingEvents::Add(CInterestingEvents::EType type, CEntity* entity) {
    if (!m_bIsActive || !entity) {
        return;
    }

    CVector vCameraOrigin = CCamera::GetActiveCamera().m_vecSource;

    if (m_iCurrentFrameCounter != CTimer::GetFrameCounter()) {
        m_iCurrentFrameCounter = CTimer::GetFrameCounter();

        CPlayerPed* pPlayerPed = FindPlayerPed();

        m_ViewVec = pPlayerPed->GetPosition() - vCameraOrigin;
        m_ViewVec.z = 0.0f;
        if (m_ViewVec.NormaliseAndMag() == 0.0f) {
            m_ViewVec = pPlayerPed->GetMatrix().GetForward();
        }
        m_ScanOrigin = pPlayerPed->GetPosition() + m_ViewVec * m_fEventRadius;
    }

    const float fRadiusSqr = m_fEventRadius * m_fEventRadius;
    CVector vDiff = m_ScanOrigin - entity->GetPosition();
    float fRadialDistSqr = vDiff.x * vDiff.x + vDiff.y * vDiff.y;
    if (fRadialDistSqr > fRadiusSqr) {
        return;
    }

    if (m_bIgnoreEventsBehindPlayer) {
        float fPlaneDist = -DotProduct(vCameraOrigin, m_ViewVec);
        float fPlanarDist = DotProduct(m_ViewVec, entity->GetPosition()) + fPlaneDist;
        if (fPlanarDist < 0.0f) {
            return;
        }
    }

    bool bLOS = CWorld::GetIsLineOfSightClear(vCameraOrigin, entity->GetPosition(), true, false, false, false, false, true, false);
    if (!bLOS) {
        return;
    }

    uint32 iTimeMs = CTimer::GetTimeInMS();
    int32 iPriority = m_EventPriorities[type];
    bool bCanAddNewEventOfType = (iTimeMs > m_NextTimeToAcceptEvents[type]);

    for (int32 e = 0; e < 8; e++) {
        TInterestingEvent& pEvent = m_Events[e];
        bool bLookingAtThisEvent = (m_iLookingAtEvent == e);
        bool bDurationExpired = (iTimeMs > pEvent.m_iStartTime + m_EventDurations[pEvent.m_eType]);
        EType eExistingType = static_cast<EType>(pEvent.m_eType);

        if (!pEvent.m_pEntity) {
            pEvent.m_eType = ENone;
            pEvent.m_pEntity = nullptr;
        } else if (eExistingType != ENone) {
            int32 iExistingPriority = m_EventPriorities[eExistingType];
            if (iPriority >= iExistingPriority) {
                if (!bCanAddNewEventOfType || bLookingAtThisEvent)
                    continue;
            } else if (bDurationExpired) {
                if (!bCanAddNewEventOfType || bLookingAtThisEvent)
                    continue;
            } else {
                continue;
            }
        }

        if (pEvent.m_pEntity) {
            CEntity::ClearReference(pEvent.m_pEntity);
        }
        pEvent.m_eType = type;
        pEvent.m_pEntity = entity;
        pEvent.m_iStartTime = iTimeMs;
        entity->RegisterReference(&pEvent.m_pEntity);

        if (m_bUseTimeDelayBeforeAddingSimilarEvent) {
            m_NextTimeToAcceptEvents[type] = iTimeMs + (m_EventDurations[type] / 2);
        } else {
            m_NextTimeToAcceptEvents[type] = iTimeMs;
        }
        return;
    }
}

// 0x605A30
void CInterestingEvents::ScanForNearbyEntities() {
    ZoneScoped;

    if (!m_bIsActive)
        return;

    const auto UPDATE_INTERVAL = 500;
    if (CTimer::GetTimeInMS() - m_iLastScanTime < UPDATE_INTERVAL) {
        return;
    }
    m_iLastScanTime = CTimer::GetTimeInMS();

    CPlayerPed* pPlayerPed = FindPlayerPed();
    if (m_iCurrentFrameCounter != CTimer::GetFrameCounter()) {
        m_iCurrentFrameCounter = CTimer::GetFrameCounter();
        const CVector vCameraOrigin = CCamera::GetActiveCamera().m_vecSource;
        m_ViewVec = pPlayerPed->GetPosition() - vCameraOrigin;
        m_ViewVec.z = 0.f;
        if (m_ViewVec.NormaliseAndMag() == 0.f)
            m_ViewVec = pPlayerPed->GetMatrix().GetForward();
        const float fRadiusSqr = m_fEventRadius * m_fEventRadius;
        m_ScanOrigin = pPlayerPed->GetPosition() + (m_ViewVec * m_fEventRadius);
    }

    const int32 iLeft   = std::max(static_cast<int32>(std::floor((m_ScanOrigin.x - m_fEventRadius) / 50.0f + 60.0f)), 0);
    const int32 iBottom = std::max(static_cast<int32>(std::floor((m_ScanOrigin.y - m_fEventRadius) / 50.0f + 60.0f)), 0);
    const int32 iRight  = std::min(static_cast<int32>(std::floor((m_ScanOrigin.x + m_fEventRadius) / 50.0f + 60.0f)), 119);
    const int32 iTop    = std::min(static_cast<int32>(std::floor((m_ScanOrigin.y + m_fEventRadius) / 50.0f + 60.0f)), 119);

    CWorld::AdvanceCurrentScanCode();
    pPlayerPed->SetCurrentScanCode();

    for (int32 y = iBottom; y <= iTop; ++y) {
        for (int32 x = iLeft; x <= iRight; ++x) {
            auto& sector = CWorld::GetRepeatSector(x, y);

            for (auto* const pPed : sector.Peds) {
                if (pPed->IsScanCodeCurrent())
                    continue;

                pPed->SetCurrentScanCode();

                if (pPed->m_nPedState == PEDSTATE_DEAD)
                    continue;

                CEntity* pInterstingEntity = (pPed->bInVehicle && pPed->m_pVehicle) ? static_cast<CEntity*>(pPed->m_pVehicle) : static_cast<CEntity*>(pPed);
                int32 iPedType = pPed->m_nPedType;

                switch (iPedType) {
                case PED_TYPE_COP:
                    Add(ECopNearby, pInterstingEntity);
                    break;
                case PED_TYPE_CRIMINAL:
                    Add(ECriminalNearby, pInterstingEntity);
                    break;
                case PED_TYPE_PROSTITUTE:
                    Add(EProzzyNearby, pInterstingEntity);
                    break;
                }

                if (IsPedTypeGang(static_cast<ePedType>(iPedType))) {
                    Add(EGangMemberNearby, pInterstingEntity);
                }
            }

            for (auto* const pVehicle : sector.Vehicles) {
                if (pVehicle->IsScanCodeCurrent())
                    continue;

                pVehicle->SetCurrentScanCode();
                if (pVehicle->physicalFlags.bRenderScorched != 0)
                    continue;

                if (!pVehicle->m_pDriver)
                    continue;

                bool bMadDriver = pVehicle->m_autoPilot.DrivingMode != DRIVING_STYLE_STOP_FOR_CARS && pVehicle->m_autoPilot.DrivingMode != DRIVING_STYLE_DRIVINGMODE_AVOIDCARS_STOPFORPEDS_OBEYLIGHTS;
                if (bMadDriver) {
                    Add(EMadDriver, pVehicle);
                }
            }
        }
    }
}

// 0x6028A0
TInterestingEvent* CInterestingEvents::GetInterestingEvent() {
    uint32 iTimeMs = CTimer::GetTimeInMS();

    if (m_bWaitForEventDurationToComplete && m_iLookingAtEvent != -1) {
        TInterestingEvent* pLookingAtEvent = &m_Events[m_iLookingAtEvent];
        if (pLookingAtEvent->m_pEntity && iTimeMs < pLookingAtEvent->m_iStartTime + m_EventDurations[pLookingAtEvent->m_eType]) {
            return pLookingAtEvent;
        }
    }

    int32 iBestEventPriority = 0;
    int32 iBestEventIndex = -1;

    for (int32 e = 0; e < MAX_NUM_INTERESTING_EVENTS; e++) {
        TInterestingEvent* pEvent = &m_Events[e];

        if (pEvent->m_pEntity && iTimeMs < pEvent->m_iStartTime + m_EventDurations[pEvent->m_eType]) {
            if (m_EventPriorities[pEvent->m_eType] > iBestEventPriority || (CGeneral::GetRandomNumber() & 0xFFFF) < 128) {
                iBestEventPriority = m_EventPriorities[pEvent->m_eType];
                iBestEventIndex = e;
            }
        }
    }

    m_iLookingAtEvent = static_cast<int8>(iBestEventIndex);

    if (iBestEventIndex != -1) {
        return &m_Events[iBestEventIndex];
    }

    return nullptr;
}

// 0x602960
void CInterestingEvents::InvalidateEvent(const TInterestingEvent* event) {
    for (auto index = 0; index < MAX_INTERESTING_EVENTS; index++) {
        TInterestingEvent* tevent = &m_Events[index];
        if (tevent != event)
            continue;

        tevent->m_iStartTime = 0;
        CEntity::ClearReference(tevent->m_pEntity);
        if (m_iLookingAtEvent == index) {
            m_iLookingAtEvent = -1;
        }
    }
}

// 0x6029C0
void CInterestingEvents::InvalidateNonVisibleEvents() {
    const auto& camPos = CCamera::GetActiveCamera().m_vecSource;
    for (auto i = 0; i < MAX_INTERESTING_EVENTS; i++) {
        TInterestingEvent& event = m_Events[i];
        if (!event.m_pEntity)
            continue;

        CVector pos = event.m_pEntity->GetPosition();
        if (CWorld::GetIsLineOfSightClear(camPos, pos, true, false, false, false, false, true, false))
            continue;

        event.m_iStartTime = 0;
        CEntity::SafeCleanUpRef(event.m_pEntity);
        if (m_iLookingAtEvent == i) {
            m_iLookingAtEvent = -1;
        }
    }
}
