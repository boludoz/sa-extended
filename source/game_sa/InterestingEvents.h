#pragma once

#include <Timer.h>

constexpr int8 MAX_INTERESTING_EVENTS = 8;
constexpr int8 MAX_NUM_INTERESTING_EVENTS = 8;

struct TInterestingEvent {
    uint32   m_eType;      // ex: type
    uint32   m_iStartTime; // ex: time
    CEntity* m_pEntity;    // ex: entity
};
VALIDATE_SIZE(TInterestingEvent, 0xC);

class CInterestingEvents {
public:
    enum EType {
        ENone,                       // ex: INTERESTING_EVENT_0
        EPedsChatting,               // ex: PEDS_CHATTING - CTaskComplexChat::CreateFirstSubTask
        EPedSunbathing,              // ex: INTERESTING_EVENT_2 - CTaskComplexSunbathe::CreateNextSubTask
        EPedUsingAttractor,          // ex: INTERESTING_EVENT_3 - CTaskComplexUseAttractor::CreateFirstSubTask CTaskComplexUseClosestFreeScriptedAttractor::CreateFirstSubTask
        EProzzyNearby,               // ex: INTERESTING_EVENT_4 - CInterestingEvents::ScanForNearbyEntities
        ECopNearby,                  // ex: INTERESTING_EVENT_5 - CInterestingEvents::ScanForNearbyEntities
        ECriminalNearby,             // ex: INTERESTING_EVENT_6 - CInterestingEvents::ScanForNearbyEntities
        EGangMemberNearby,           // ex: INTERESTING_EVENT_7 - CInterestingEvents::ScanForNearbyEntities
        ESexyCar,                    // ex: INTERESTING_EVENT_8 - CEventHandler::ComputeSexyVehicleResponse
        ESexyPed,                    // ex: INTERESTING_EVENT_9 - CSexyPedScanner::ScanForSexyPedEvents
        EPlaneFlyby,                 // ex: INTERESTING_EVENT_10
        EPedRevived,                 // ex: INTERESTING_EVENT_11 - CTaskSimpleGiveCPR::ReviveDeadPed
        EEmergencyServicesArrived,   // ex: INTERESTING_EVENT_12 - CTaskComplexMedicTreatInjuredPed::CreateNextSubTask
        EPanickedPed,                // ex: INTERESTING_EVENT_13 - CTaskComplexFleeEntity::CreateFirstSubTask CTaskComplexSmartFleeEntity::CreateFirstSubTask
        EMadDriver,                  // ex: INTERESTING_EVENT_14 - CInterestingEvents::ScanForNearbyEntities
        EPedRunOver,                 // ex: INTERESTING_EVENT_15 - CEventHandler::ComputeDamageResponse
        EPedKnockedOffBike,          // ex: INTERESTING_EVENT_16 - CEventHandler::ComputeKnockOffBikeResponse
        ECarCrash,                   // ex: VEHICLE_DAMAGE
        ERoadRage,                   // ex: INTERESTING_EVENT_18 - CTaskComplexRoadRage::CreateFirstSubTask
        EFistFight,                  // ex: INTERESTING_EVENT_19 - CTaskComplexKillPedOnFootMelee::CreateFirstSubTask
        ECarJacking,                 // ex: INTERESTING_EVENT_20 - CEventHandler::ComputePedEnteredVehicleResponse
        EHelicopterOverhead,         // ex: INTERESTING_EVENT_21 - CHeli::ProcessControl
        EGunshotFired,               // ex: INTERESTING_EVENT_22 - CWeapon::FireSniper CWeapon::FireInstantHitFromCar2 CWeapon::FireInstantHit CWeapon::FireProjectile
        EGangAttackingPed,           // ex: GANG_ATTACKING_PED - CTaskAllocatorKillThreatsDriveby::AllocateTasks CTaskAllocatorKillThreatsBasic::AllocateTasks ComputeKillThreatsBasicResponse
        EGangFight,                  // ex: GANG_FIGHT
        ECopKillingCriminal,         // ex: INTERESTING_EVENT_25 - CTaskComplexKillCriminal::CreateFirstSubTask
        ESwatTeamAbseiling,          // ex: ZELDICK_OCCUPATION - CHeli::ProcessControl
        EExplosion,                  // ex: EVENT_ATTRACTOR
        EPedGotKilled,               // ex: INTERESTING_EVENT_28 - CEventHandler::ComputeDamageResponse

        ENumCategories,              // ex: MAX_INTERESTING_EVENT_TYPES

        // Backward compatibility aliases
        INTERESTING_EVENT_0         = ENone,
        PEDS_CHATTING               = EPedsChatting,
        INTERESTING_EVENT_2         = EPedSunbathing,
        INTERESTING_EVENT_3         = EPedUsingAttractor,
        INTERESTING_EVENT_4         = EProzzyNearby,
        INTERESTING_EVENT_5         = ECopNearby,
        INTERESTING_EVENT_6         = ECriminalNearby,
        INTERESTING_EVENT_7         = EGangMemberNearby,
        INTERESTING_EVENT_8         = ESexyCar,
        INTERESTING_EVENT_9         = ESexyPed,
        INTERESTING_EVENT_10        = EPlaneFlyby,
        INTERESTING_EVENT_11        = EPedRevived,
        INTERESTING_EVENT_12        = EEmergencyServicesArrived,
        INTERESTING_EVENT_13        = EPanickedPed,
        INTERESTING_EVENT_14        = EMadDriver,
        INTERESTING_EVENT_15        = EPedRunOver,
        INTERESTING_EVENT_16        = EPedKnockedOffBike,
        VEHICLE_DAMAGE              = ECarCrash,
        INTERESTING_EVENT_18        = ERoadRage,
        INTERESTING_EVENT_19        = EFistFight,
        INTERESTING_EVENT_20        = ECarJacking,
        INTERESTING_EVENT_21        = EHelicopterOverhead,
        INTERESTING_EVENT_22        = EGunshotFired,
        GANG_ATTACKING_PED          = EGangAttackingPed,
        GANG_FIGHT                  = EGangFight,
        INTERESTING_EVENT_25        = ECopKillingCriminal,
        ZELDICK_OCCUPATION          = ESwatTeamAbseiling,
        EVENT_ATTRACTOR             = EExplosion,
        INTERESTING_EVENT_28        = EPedGotKilled,
        MAX_INTERESTING_EVENT_TYPES = ENumCategories,
    };

    TInterestingEvent m_Events[MAX_INTERESTING_EVENTS];
    uint8             m_EventPriorities[ENumCategories];        // ex: m_nPriorities
    uint16            m_EventDurations[ENumCategories];         // ex: m_nDelays
    uint32            m_NextTimeToAcceptEvents[ENumCategories]; // ex: m_nEndsOfTime
    union {
        struct {
            uint8 m_bIsActive : 1;                             // ex: m_b1 : 1;
            uint8 m_bIgnoreEventsBehindPlayer : 1;             // ex: m_b2 : 1;
            uint8 m_bWaitForEventDurationToComplete : 1;       // ex: m_b4 : 1;
            uint8 m_bUseTimeDelayBeforeAddingSimilarEvent : 1; // ex: m_b8 : 1;
        };
        uint8 m_nFlags{};
    };
    uint32            m_iCurrentFrameCounter{CTimer::GetFrameCounter() - 1}; // ex: m_CurrentFrameCounter
    uint32            m_iLastScanTime{};                                     // ex: m_nLastScanTimeUpdate
    float             m_fEventRadius{30.f};                                  // ex: m_fRadius
    CVector           m_ScanOrigin{};                                        // ex: m_vecCenter
    CVector           m_ViewVec{};                                           // ex: vec148
    int8              m_iLookingAtEvent{-1};                                 // ex: m_nInterestingEvent

public:
    static void InjectHooks();

    CInterestingEvents();
    CInterestingEvents* Constructor();

    ~CInterestingEvents();
    CInterestingEvents* Destructor();

    void Add(CInterestingEvents::EType type, CEntity* entity);
    void ScanForNearbyEntities();
    TInterestingEvent* GetInterestingEvent();
    void InvalidateEvent(const TInterestingEvent* event);
    void InvalidateNonVisibleEvents();

    bool IsActive() const { return m_bIsActive; }
    void SetActive(bool bActive) { m_bIsActive = bActive; }

    bool GetIgnoreEventsBehindPlayer() const { return m_bIgnoreEventsBehindPlayer; }
    void SetIgnoreEventsBehindPlayer(bool bIgnore) { m_bIgnoreEventsBehindPlayer = bIgnore; }

    float GetEventRadius() const { return m_fEventRadius; }
    void SetEventRadius(float fRadius) { m_fEventRadius = fRadius; }
};
VALIDATE_SIZE(CInterestingEvents, 0x158);

extern CInterestingEvents& g_InterestingEvents;
