#include "StdInc.h"

#include "InterestingEvents.h"

auto& g_InterestingEvents = StaticRef<CInterestingEvents>(0xC0B058);

/*
 * Commented hooks aren't tested.
 * */
void CInterestingEvents::InjectHooks() {
    RH_ScopedClass(CInterestingEvents);
    RH_ScopedCategoryGlobal();

    RH_ScopedInstall(Constructor, 0x6023A0, { .reversed = false });
    RH_ScopedInstall(Destructor, 0x856880, { .reversed = false });
    RH_ScopedInstall(Add, 0x602590, { .reversed = false });
    RH_ScopedInstall(ScanForNearbyEntities, 0x605A30, { .reversed = false });
    RH_ScopedInstall(GetInterestingEvent, 0x6028A0);
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
    for (auto& event : m_Events) {
        CEntity::ClearReference(event.m_pEntity);
    }
}

CInterestingEvents* CInterestingEvents::Destructor() {
    this->CInterestingEvents::~CInterestingEvents();
    return this;
}

// 0x602590
void CInterestingEvents::Add(CInterestingEvents::EType type, CEntity* entity) {
    return plugin::CallMethod<0x602590, CInterestingEvents*, CInterestingEvents::EType, CEntity*>(this, type, entity);

    if (!m_bIsActive || !entity)
        return;

    NOTSA_LOG_DEBUG("type={}, model={}", (int32)(type), entity->m_nModelIndex);

    const auto& camPos = CCamera::GetActiveCamera().m_vecSource;
    if (m_iCurrentFrameCounter != CTimer::GetFrameCounter()) {
        m_iCurrentFrameCounter = CTimer::GetFrameCounter();

        CPlayerPed* player = FindPlayerPed();
        const auto& playerPos = player->GetPosition();
        m_ViewVec = playerPos - camPos;
        m_ViewVec.z = 0.f;
        if (m_ViewVec.NormaliseAndMag() == 0.f) {
            m_ViewVec = player->GetMatrix().GetForward();
        }
        m_ScanOrigin = (m_ViewVec * m_fEventRadius) + playerPos;
    }

    CVector2D distance = m_ScanOrigin - entity->GetPosition();
    if (distance.SquaredMagnitude() > m_fEventRadius * m_fEventRadius)
        return;

    CVector vec0 = m_ViewVec * entity->GetPosition();
    CVector vec1 = m_ViewVec * camPos;
    if (!m_bIgnoreEventsBehindPlayer && vec0.ComponentwiseSum() - vec1.ComponentwiseSum() < 0.f)
        return;

    if (!CWorld::GetIsLineOfSightClear(camPos, entity->GetPosition(), true, false, false, false, false, true, false))
        return;

    uint32 time = CTimer::GetTimeInMS();
    for (auto index = 0; index < MAX_INTERESTING_EVENTS; index++) {
        TInterestingEvent& event = g_InterestingEvents.m_Events[index];
        if (event.m_pEntity) {
            if (m_EventPriorities[type] < m_EventPriorities[event.m_eType] && CTimer::GetTimeInMS() <= event.m_iStartTime + static_cast<uint32>(m_EventDurations[event.m_eType]))
                continue;
            if (CTimer::GetTimeInMS() <= m_NextTimeToAcceptEvents[type] || m_iLookingAtEvent == index)
                continue;
        } else {
            event.m_eType = ENone;
        }

        CEntity::SafeCleanUpRef(event.m_pEntity);
        event.m_eType = type;
        event.m_pEntity = entity;
        event.m_iStartTime = time;
        entity->RegisterReference(&event.m_pEntity);
        if (m_bUseTimeDelayBeforeAddingSimilarEvent)
            m_NextTimeToAcceptEvents[type] = time;
        else
            m_NextTimeToAcceptEvents[type] = time + (m_EventDurations[type] >> 1);
        break;
    }
}

// 0x605A30
void CInterestingEvents::ScanForNearbyEntities() {
    ZoneScoped;

    return plugin::CallMethod<0x605A30, CInterestingEvents*>(this);

    if (!m_bIsActive)
        return;

    const auto UPDATE_INTERVAL = 500;
    if (CTimer::GetTimeInMS() - m_iLastScanTime < UPDATE_INTERVAL) {
        return;
    }
    m_iLastScanTime = CTimer::GetTimeInMS();

    CPlayerPed* player = FindPlayerPed();
    if (m_iCurrentFrameCounter != CTimer::GetFrameCounter()) {
        m_iCurrentFrameCounter = CTimer::GetFrameCounter();
        const auto& camPos = CCamera::GetActiveCamera().m_vecSource, playerPos = player->GetPosition();
        m_ViewVec = playerPos - camPos;
        m_ViewVec.z = 0.f;
        if (m_ViewVec.NormaliseAndMag() == 0.f)
            m_ViewVec = player->GetMatrix().GetForward();
        m_ScanOrigin = (m_ViewVec * m_fEventRadius) + playerPos;
    }

    auto v0 = std::max(static_cast<int>(std::floor((m_ScanOrigin.x - m_fEventRadius) * 50.0f + 60.0f)), 0);
    auto v1 = std::max(static_cast<int>(std::floor((m_ScanOrigin.y - m_fEventRadius) * 50.0f + 60.0f)), 0);
    auto v2 = std::min(static_cast<int>(std::floor((m_ScanOrigin.x + m_fEventRadius) * 50.0f + 60.0f)), 119);
    auto v3 = std::min(static_cast<int>(std::floor((m_ScanOrigin.y + m_fEventRadius) * 50.0f + 60.0f)), 119);

    int32 startSectorX = CWorld::GetSectorX(m_ScanOrigin.x - m_fEventRadius);
    int32 startSectorY = CWorld::GetSectorY(m_ScanOrigin.y - m_fEventRadius);
    int32 endSectorX   = CWorld::GetSectorX(m_ScanOrigin.x + m_fEventRadius);
    int32 endSectorY   = CWorld::GetSectorY(m_ScanOrigin.y + m_fEventRadius);

    assert(v0 == startSectorX);
    assert(v1 == startSectorY);
    assert(v2 == endSectorX);
    assert(v3 == endSectorY);

    CWorld::AdvanceCurrentScanCode();
    player->SetCurrentScanCode();

    for (int32 sectorY = startSectorY; sectorY <= endSectorY; ++sectorY) {
        for (int32 sectorX = startSectorX; sectorX <= endSectorX; ++sectorX) {
            auto& rs = CWorld::GetRepeatSector(sectorX, sectorY);

            for (auto* const ped : rs.Peds) {
                if (ped->IsScanCodeCurrent())
                    continue;

                ped->SetCurrentScanCode();

                if (ped->m_nPedState == PEDSTATE_DEAD)
                    continue;

                CEntity* entity;
                if (ped->bInVehicle) {
                    entity = ped->m_pVehicle;
                } else {
                    entity = ped;
                }

                switch (ped->m_nPedType) {
                case PED_TYPE_COP:
                    Add(ECopNearby, entity);
                    break;
                case PED_TYPE_CRIMINAL:
                    Add(ECriminalNearby, entity);
                    break;
                case PED_TYPE_PROSTITUTE:
                    Add(EProzzyNearby, entity);
                    break;
                default:
                    if (IsPedTypeGang(ped->m_nPedType)) {
                        Add(EGangMemberNearby, entity);
                    }
                    break;
                }
            }

            for (auto* const vehicle : rs.Vehicles) {
                if (vehicle->IsScanCodeCurrent())
                    continue;

                vehicle->SetCurrentScanCode();
                if (vehicle->physicalFlags.bRenderScorched != 0)
                    continue;

                if (!vehicle->m_pDriver)
                    continue;

                auto style = vehicle->m_autoPilot.DrivingMode;
                if (!style)
                    continue;

                if (style == DRIVING_STYLE_DRIVINGMODE_AVOIDCARS_STOPFORPEDS_OBEYLIGHTS)
                    continue;

                Add(EMadDriver, vehicle);
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
