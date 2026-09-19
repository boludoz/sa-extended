/*
    Plugin-SDK file
    Authors: GTA Community. See more here
    https://github.com/DK22Pac/plugin-sdk
    Do not delete this comment block. Respect others' work!
*/
#include "StdInc.h"
#include "WindModifiers.h"
#include "Entity.h"
#include "Shadows.h"
#include "InterestingEvents.h"
#include "Ropes.h"

void CHeli::InjectHooks() {
    RH_ScopedVirtualClass(CHeli, 0x871680, 71);
    RH_ScopedCategory("Vehicle");

    RH_ScopedInstall(InitHelis, 0x6C4560);
    RH_ScopedInstall(AddHeliSearchLight, 0x6C45B0);
    RH_ScopedInstall(Pre_SearchLightCone, 0x6C4650);
    RH_ScopedInstall(Post_SearchLightCone, 0x6C46E0);
    RH_ScopedInstall(SwitchPoliceHelis, 0x6C4800);
    RH_ScopedInstall(RenderAllHeliSearchLights, 0x6C7C50);
    RH_ScopedInstall(TestSniperCollision, 0x6C6890);
    RH_ScopedInstall(UpdateHelis, 0x6C79A0);
    RH_ScopedVMTInstall(ProcessControlInputs, 0x6C4830);
    RH_ScopedVMTInstall(ProcessFlyingCarStuff, 0x6C4E60);
    RH_ScopedVMTInstall(Render, 0x6C4400);
    RH_ScopedVMTInstall(Fix, 0x6C4530);
    RH_ScopedVMTInstall(BurstTyre, 0x6C4330);
    RH_ScopedVMTInstall(SetUpWheelColModel, 0x6C4320);
    RH_ScopedVMTInstall(PreRender, 0x6C5420);
    RH_ScopedVMTInstall(BlowUpCar, 0x6C6D30);
    RH_ScopedVMTInstall(ProcessControl, 0x6C7050);
}

// 0x6C4190
CHeli::CHeli(int32 modelIndex, eVehicleCreatedBy createdBy) : CAutomobile(modelIndex, createdBy, true) {
    m_vehicleType                        = VEHICLE_TYPE_HELI;

    m_fYawControl                        = 0.0;
    m_fPitchControl                      = 0.0;
    m_fRollControl                       = 0.0;
    m_fThrottleControl                   = 0.0;
    m_fEngineSpeed                       = 0.0;
    m_fMainRotorAngle                    = 0.0;
    m_fRearRotorAngle                    = 0.0;
    m_MinHeightAboveTerrain              = 10.0;
    m_LowestFlightHeight                 = 10.0;
    m_DesiredHeight                      = 10.0;
    m_FlightDirection                    = 0.0;
    m_nHeliFlags.bStopFlyingForAWhile    = 0;
    m_nHeliFlags.bUseSearchLightOnTarget = 0;
    m_nHeliFlags.bWarnTarger             = 0; // fix
    m_LightBrightness                    = 0.0;

    physicalFlags.bDontCollideWithFlyers = true;

    if (modelIndex == MODEL_HUNTER) {
        m_damageManager.SetDoorStatus(DOOR_LEFT_FRONT, DAMSTATE_OK);
        m_doors[DOOR_LEFT_FRONT].Init((3.0f * PI) / 10.0f, 0.0f, DOOR_AXIS_NEG_X, DOOR_AXIS_Y, DOOR_EXTRA_BASED);
    }

    m_nSwatOnBoard = 4;
    m_SwatRopeActive.fill(0);

    m_LastSearchLightSample = CTimer::GetTimeInMS();

    m_OldSearchLightX.fill(0.0f);
    m_OldSearchLightY.fill(0.0f);

    m_LastTimeSearchLightWasTooFarAwayToShoot = 0;
    m_nNextTalkTimer = CTimer::GetTimeInMS();

    vehicleFlags.bNeverUseSmallerRemovalRange = true; // 0x6C42BD
    m_autoPilot.TargetReachedDist = 10;

    m_GunflashFxPtrs = nullptr;
    m_FiringRateMultiplier = 16;

    m_bStopFlyingForAWhile = 0;
    m_bSearchLightOn = false;
    m_crashAndBurnTurnSpeed = CGeneral::GetRandomNumberInRange(2.f, 8.f);
}

// 0x6C4340
CHeli::~CHeli() {
    if (m_GunflashFxPtrs) {
        for (auto i = 0; i < CVehicle::GetPlaneNumGuns(); i++) {
            if (auto& fx = m_GunflashFxPtrs[i]) {
                fx->Kill();
                g_fxMan.DestroyFxSystem(fx);
            }
        }
        delete[] m_GunflashFxPtrs;
        m_GunflashFxPtrs = nullptr;
    }

    m_vehicleAudio.Terminate();
}

// 0x6C4560
void CHeli::InitHelis() {
    std::ranges::fill(pHelis, nullptr);
    for (auto& light : HeliSearchLights) {
        light.Init();
    }
    NumberOfSearchLights = 0;
    bPoliceHelisAllowed = true;
}

// 0x6C45B0
void CHeli::AddHeliSearchLight(const CVector& origin, const CVector& target, float targetRadius, float power, uint32 coronaIndex, uint8 unknownFlag, uint8 drawShadow) {
    auto& light = HeliSearchLights[NumberOfSearchLights];

    light.m_vecOrigin     = origin;
    light.m_vecTarget     = target;
    light.m_fTargetRadius = targetRadius;
    light.m_fPower        = power;
    light.m_nCoronaIndex  = coronaIndex;
    light.field_24        = unknownFlag;
    light.m_bDrawShadow   = drawShadow;

    NumberOfSearchLights += 1;
}

// 0x6C4640
void CHeli::PreRenderAlways() {
    // NOP
}

// 0x6C4650
void CHeli::Pre_SearchLightCone() {
    ZoneScoped;

    RwRenderStateSet(rwRENDERSTATEZWRITEENABLE,         RWRSTATE(FALSE));
    RwRenderStateSet(rwRENDERSTATEZTESTENABLE,          RWRSTATE(TRUE));
    RwRenderStateSet(rwRENDERSTATESRCBLEND,             RWRSTATE(rwBLENDONE));
    RwRenderStateSet(rwRENDERSTATEDESTBLEND,            RWRSTATE(rwBLENDONE));
    RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE,    RWRSTATE(TRUE));
    RwRenderStateSet(rwRENDERSTATETEXTURERASTER,        RWRSTATE(NULL));
    RwRenderStateSet(rwRENDERSTATEFOGENABLE,            RWRSTATE(FALSE));
    RwRenderStateSet(rwRENDERSTATESHADEMODE,            RWRSTATE(rwSHADEMODEGOURAUD));
    RwRenderStateSet(rwRENDERSTATEALPHATESTFUNCTION,    RWRSTATE(rwALPHATESTFUNCTIONGREATEREQUAL));
    RwRenderStateSet(rwRENDERSTATEALPHATESTFUNCTIONREF, RWRSTATE(0));
}

// 0x6C46E0
void CHeli::Post_SearchLightCone() {
    ZoneScoped;

    RwRenderStateSet(rwRENDERSTATEZWRITEENABLE,         RWRSTATE(TRUE));
    RwRenderStateSet(rwRENDERSTATEZTESTENABLE,          RWRSTATE(TRUE));
    RwRenderStateSet(rwRENDERSTATESRCBLEND,             RWRSTATE(rwBLENDSRCALPHA));
    RwRenderStateSet(rwRENDERSTATEDESTBLEND,            RWRSTATE(rwBLENDINVSRCALPHA));
    RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE,    RWRSTATE(FALSE));
    RwRenderStateSet(rwRENDERSTATECULLMODE,             RWRSTATE(rwCULLMODECULLBACK));
    RwRenderStateSet(rwRENDERSTATEALPHATESTFUNCTION,    RWRSTATE(rwALPHATESTFUNCTIONGREATER));
    RwRenderStateSet(rwRENDERSTATEALPHATESTFUNCTIONREF, RWRSTATE(2u));
}

// 0x6C4750
void CHeli::SpecialHeliPreRender() {
    // NOP
}

// 0x6C4760
CVector CHeli::FindSwatPositionRelativeToHeli(int32 swatNumber) {
    CVector result;
    ((void(__thiscall*)(CHeli*, CVector*, int32))0x6C4760)(this, &result, swatNumber);
    return result;

    switch ( swatNumber ) {
    case 0:
        return { -1.2f, -1.0f, -0.5f };
    case 1:
        return { 1.2f,  -1.0f, -0.5f };
    case 2:
        return { -1.2f, 1.0f,  -0.5f };
    case 3:
        return { 1.2f,  1.0f,  -0.5f };
    default:
        return { 0.0f,  0.0f,  0.0f  };
    }
}

// 0x6C4800
void CHeli::SwitchPoliceHelis(bool enable) {
    bPoliceHelisAllowed = enable;
}

// 0x6C58E0
void CHeli::SearchLightCone(int32 coronaIndex,
                            CVector origin,
                            CVector target,
                            float targetRadius,
                            float power,
                            uint8 unknownFlag,
                            uint8 drawShadow,
                            CVector& useless0,
                            CVector& useless1,
                            CVector& useless2,
                            bool a11,
                            float baseRadius,
                            float a13,
                            float a14,
                            float a15
) {
    ((void(__cdecl*)(int32, CVector, CVector, float, float, uint8, uint8, CVector&, CVector&, CVector&, bool, float, float, float, float))0x6C58E0)(coronaIndex, origin, target, targetRadius, power, unknownFlag, drawShadow, useless0, useless1, useless2, a11, baseRadius, a13, a14, a15);
}

// 0x6C6520
CHeli* CHeli::GenerateHeli(CPed* target, bool newsHeli) {
    return ((CHeli * (__cdecl*)(CPed*, bool))0x6C6520)(target, newsHeli);
}

// 0x6C6890
void CHeli::TestSniperCollision(CVector* origin, CVector* target) {
    CVector point = *target - *origin;

    if (point.z >= point.Magnitude() / 2.0f)
        return;

    for (auto& heli : pHelis) {
        if (!heli || heli->physicalFlags.bBulletProof)
            continue;

        const auto mat = (CMatrix*)heli->m_matrix;
        if (CCollision::DistToLine(*origin, *target, mat->TransformPoint({ -0.43f, 1.49f, 1.5f })) < 0.8f) {
            heli->m_fRotationBalance = (float)(CGeneral::GetRandomNumber() < pow(2, 14) - 1) * 0.1f - 0.05f; // 2^14 - 1 = 16383 [-0.05, 0.05]
            heli->BlowUpCar(FindPlayerPed(), false);
            heli->m_nSwatOnBoard = 0;
        };
    }
}

// 0x6C69C0
bool CHeli::SendDownSwat() {
    return ((bool(__thiscall*)(CHeli*))0x6C69C0)(this);
}

// 0x6C79A0
void CHeli::UpdateHelis() {
    bool hasValidPolmav;
    int currentHeliCount;
    int maxAllowedHelis;
    CVector playerPosition;
    
    // Reset search light count
    CHeli::NumberOfSearchLights = 0;
    
    // Check existing helicopters status
    currentHeliCount = 0;
    hasValidPolmav = false;
    
    // Create a lambda for checking valid helicopters
    auto IsHeliValid = [](CHeli* heli) -> bool {
        return heli != nullptr;
    };
    
    // Check first helicopter
    if (IsHeliValid(CHeli::pHelis[0])) {
        hasValidPolmav = CHeli::pHelis[0]->m_nModelIndex == MODEL_POLMAV && 
                            !CHeli::pHelis[0]->physicalFlags.bRenderScorched && 
                            !CHeli::pHelis[0]->vehicleFlags.bIsDrowning;
        currentHeliCount = 1;
    }
    
    // Check second helicopter
    if (IsHeliValid(CHeli::pHelis[1])) {
        currentHeliCount++;
        if (CHeli::pHelis[1]->m_nModelIndex == MODEL_POLMAV && !CHeli::pHelis[1]->physicalFlags.bRenderScorched) {
            hasValidPolmav = hasValidPolmav || !CHeli::pHelis[1]->vehicleFlags.bIsDrowning;
        }
    }
    
    // Check game conditions for helicopters
    if (IsHeliValid(CHeli::pHelis[0])) {
        hasValidPolmav &= CHeli::pHelis[0]->m_nModelIndex != MODEL_VCNMAV;
    }
    
    if (IsHeliValid(CHeli::pHelis[1])) {
        hasValidPolmav &= CHeli::pHelis[1]->m_nModelIndex != MODEL_VCNMAV;
    }

    // Determine max allowed helicopters based on game conditions
    maxAllowedHelis = (CHeli::bPoliceHelisAllowed
                       && !CCullZones::PlayerNoRain()
                       && !CGame::IsInInterior()
                       && CWeather::OldWeatherType != WEATHER_SANDSTORM_DESERT
                       && CWeather::NewWeatherType != WEATHER_SANDSTORM_DESERT)
        ? FindPlayerWanted()->NumOfHelisRequired()
        : 0;

    // Check if news helicopter is allowed
    const bool useNewsHeliAllowed = hasValidPolmav && CWanted::UseNewsHeliInAdditionToPolice;
    const int modelIndexOffset = useNewsHeliAllowed ? MODEL_VCNMAV : MODEL_POLMAV;
    
    // Generate new helicopter if needed
    if (CStreaming::ms_aInfoForModel[modelIndexOffset].m_LoadState == 1 && 
        CTimer::GetTimeInMS() > CHeli::TestForNewRandomHelisTimer) {
        
        CHeli::TestForNewRandomHelisTimer = CTimer::GetTimeInMS() + 15000;
        
        if (currentHeliCount < maxAllowedHelis) {
            CPlayerPed* playerPed = FindPlayerPed();
            CHeli* newHeli = CHeli::GenerateHeli(playerPed, useNewsHeliAllowed);
            
            // Register new helicopter - corregido según definición de plantilla
            if (CHeli::pHelis[0]) {
                if (!CHeli::pHelis[1]) {
                    CHeli::pHelis[1] = newHeli;
                    CEntity::RegisterReference(CHeli::pHelis[1]);
                }
            } else {
                CHeli::pHelis[0] = newHeli;
                CEntity::RegisterReference(CHeli::pHelis[0]);
            }
        }
    }
    
    // Process first helicopter
    if (IsHeliValid(CHeli::pHelis[0])) {
        // Check if destroyed
        if (CHeli::pHelis[0]->physicalFlags.bRenderScorched || CHeli::pHelis[0]->vehicleFlags.bIsDrowning) {
            CHeli::pHelis[0]->m_autoPilot.Mission = MISSION_HELI_FLY_AWAY_FROM_PLAYER;
            CHeli::pHelis[0] = nullptr;
        } 
        else if (CHeli::pHelis[0]->m_autoPilot.Mission == MISSION_HELI_FLY_AWAY_FROM_PLAYER) {
            // Check distance to player for removal
            playerPosition = FindPlayerCoors();
            CVector camDist = playerPosition - CHeli::pHelis[0]->GetPosition();
            float distanceToPlayer = camDist.Magnitude();

            if (distanceToPlayer > 170.0f) {
                CWorld::Remove(CHeli::pHelis[0]);
                if (CHeli::pHelis[0]) {
                    delete std::exchange(CHeli::pHelis[0], nullptr);
                }
                CHeli::pHelis[0] = nullptr;
            }
        }
    }
    
    // Process second helicopter
    if (IsHeliValid(CHeli::pHelis[1])) {
        // Check if destroyed
        if (CHeli::pHelis[1]->physicalFlags.bRenderScorched || CHeli::pHelis[1]->vehicleFlags.bIsDrowning) {
            CHeli::pHelis[1]->m_autoPilot.Mission = MISSION_HELI_FLY_AWAY_FROM_PLAYER;
            CHeli::pHelis[1] = nullptr;
        } else if (CHeli::pHelis[1]->m_autoPilot.Mission == MISSION_HELI_FLY_AWAY_FROM_PLAYER) {
            // Check distance to player for removal
            playerPosition = FindPlayerCoors();
            CVector camDist = playerPosition - CHeli::pHelis[1]->GetPosition();
            float distanceToPlayer = camDist.Magnitude();

            if (distanceToPlayer > 170.0f) {
                CWorld::Remove(CHeli::pHelis[1]);
                if (CHeli::pHelis[1]) {
                    delete std::exchange(CHeli::pHelis[1], nullptr);
                }
                CHeli::pHelis[1] = nullptr;
            }
        }
    }
    
    // Make helicopters fly away if too many
    if (IsHeliValid(CHeli::pHelis[0]) && CHeli::pHelis[0]->m_autoPilot.Mission != MISSION_HELI_FLY_AWAY_FROM_PLAYER) {
        if (maxAllowedHelis < 1) {
            CHeli::pHelis[0]->m_autoPilot.Mission = MISSION_HELI_FLY_AWAY_FROM_PLAYER;
            CHeli::pHelis[0]->m_MinHeightAboveTerrain = 100.0f;
            CHeli::pHelis[0]->m_LowestFlightHeight = 100.0f;
        } else {
            maxAllowedHelis--;
        }
    }
    
    // Process second helicopter flying away if too many
    if (IsHeliValid(CHeli::pHelis[1]) && CHeli::pHelis[1]->m_autoPilot.Mission != MISSION_HELI_FLY_AWAY_FROM_PLAYER && maxAllowedHelis <= 0) {
        CHeli::pHelis[1]->m_autoPilot.Mission = MISSION_HELI_FLY_AWAY_FROM_PLAYER;
        CHeli::pHelis[1]->m_MinHeightAboveTerrain = 100.0f;
        CHeli::pHelis[1]->m_LowestFlightHeight = 100.0f;
    }
}

// 0x6C7C50
void CHeli::RenderAllHeliSearchLights() {
    ZoneScoped;

    for (auto& light : HeliSearchLights) {
        SearchLightCone(
            light.m_nCoronaIndex,
            light.m_vecOrigin,
            light.m_vecTarget,
            light.m_fTargetRadius,
            light.m_fPower,
            light.field_24,
            light.m_bDrawShadow,
            light.m_vecUseless[0],
            light.m_vecUseless[1],
            light.m_vecUseless[2],
            false,
            0.05f,
            0.0f,
            0.0f,
            1.0f
        );
    }
}

// 0x6C6D30
void CHeli::BlowUpCar(CEntity* damager, bool bHideExplosion) {
    if (!vehicleFlags.bCanBeDamaged) {
        m_autoPilot.Mission = eCarMission::MISSION_HELI_CRASH_AND_BURN;
        m_fHealth = 0.0f;
        return;
    }

    if (GetStatus() <= STATUS_PHYSICS || m_autoPilot.Mission == eCarMission::MISSION_HELI_CRASH_AND_BURN || m_nModelIndex == MODEL_RCGOBLIN || m_nModelIndex == MODEL_RCRAIDER) {
        
        // Update player stats if player caused explosion
        if (damager == FindPlayerPed() || damager == FindPlayerVehicle()) {
            auto& player = CWorld::Players[CWorld::PlayerInFocus]; // find
            player.m_fCurrentChaseValue += 10.0f;
            player.m_nHavocCaused += 20;
            CStats::IncrementStat(STAT_COST_OF_PROPERTY_DAMAGED, (float)CGeneral::GetRandomNumberInRange(4000, 10000));
        }

        // Special case for news helicopter
        if (m_nModelIndex == MODEL_VCNMAV) {
            CWanted::UseNewsHeliInAdditionToPolice = false;
        }

        // Reset vehicle movement if it's not physics
        if (GetStatus() <= STATUS_PHYSICS) { // check
            m_vecMoveSpeed = CVector(0.0f, 0.0f, 0.0f);
            m_vecTurnSpeed = CVector(0.0f, 0.0f, 0.0f);
            physicalFlags.bDisableZ = false;
            physicalFlags.bDisableMoveForce = false;
        }

        // Set appropriate status flags
        SetStatus(STATUS_WRECKED);
        physicalFlags.bRenderScorched = true;
        
        // Mark time of death and set proper visibility
        m_nTimeWhenBlowedUp = CTimer::GetTimeInMS();

        CVisibilityPlugins::SetClumpAlpha(GetRpClump(), 0xFF);
        
        // Damage all parts
        m_damageManager.FuckCarCompletely(false);

        // Destroy components for normal helicopters (not RC models)
        if (m_nModelIndex != MODEL_RCGOBLIN && m_nModelIndex != MODEL_RCRAIDER) {
            // Set all body parts to damaged state
            auto& automobile = *AsAutomobile();
            
            // Damage all body panels
            automobile.SetBumperDamage(FRONT_BUMPER, false);
            automobile.SetBumperDamage(REAR_BUMPER, false);
            
            // Damage all doors
            for (auto door : {DOOR_BONNET, DOOR_BOOT, DOOR_LEFT_FRONT, DOOR_RIGHT_FRONT, DOOR_LEFT_REAR, DOOR_RIGHT_REAR}) {
                automobile.SetDoorDamage(door, false);
            }
            
            // Spawn flying rotor component
            automobile.SpawnFlyingComponent((eCarNodes)PLANE_STATIC_PROP, FLIGHT_MODEL_HELI);
            
            // Hide top rotor
            auto pCurrentAtomic = (void *)nullptr;
            if (RwFrameForAllObjects(m_aCarNodes[PLANE_STATIC_PROP], GetCurrentAtomicObjectCB, &pCurrentAtomic); pCurrentAtomic) {
                RpAtomicSetFlags(pCurrentAtomic, 0);
            }
        }

        // Set remaining state
        m_fHealth = 0.0f;
        m_DelayedExplosion = 0;
        vehicleFlags.bDriverLastFrame = false;
        vehicleFlags.bSirenOrAlarm = false;
        vehicleFlags.bCreatedAsPoliceVehicle = false;
        
        // Camera shake effect
        TheCamera.CamShake(0.4f, GetPosition());
        
        // Kill passengers
        KillPedsInVehicle();
        KillPedsGettingInVehicle(); // NOTSA
        
        // Reset vehicle flags // todo: check if this is correct
        vehicleFlags.bIsHandbrakeOn = false;
        vehicleFlags.bLightsOn = false;
        vehicleFlags.bFreebies = false;
        vehicleFlags.bIsBig = false;
        vehicleFlags.bEngineOn = false;
        
        // Handle emergency vehicle counts
        if (vehicleFlags.bIsAmbulanceOnDuty) {
            vehicleFlags.bIsAmbulanceOnDuty = false;
            --CCarCtrl::NumAmbulancesOnDuty;
        }
        
        if (vehicleFlags.bIsFireTruckOnDuty) {
            vehicleFlags.bIsFireTruckOnDuty = false;
            --CCarCtrl::NumFireTrucksOnDuty;
        }
        
        // Reset law enforcer state
        ChangeLawEnforcerState(false);
        
        // Start fire
        gFireManager.StartFire((CEntity*)this, damager, 0.8f, true, 7'000, 0);
        
        // Register car blown up for Darkel missions
        CDarkel::RegisterCarBlownUpByPlayer(*AsVehicle(), false);
        
        // Create explosion
        eExplosionType explosionType = (m_nModelIndex == MODEL_RCRAIDER || m_nModelIndex == MODEL_RCGOBLIN) ? eExplosionType::EXPLOSION_TINY : eExplosionType::EXPLOSION_HELI;
        
        CExplosion::AddExplosion((CEntity*)this, (CEntity*)damager, explosionType, GetPosition(), false, true, -1.0f, false);
    }
}

// 0x6C4530
void CHeli::Fix() {
    m_damageManager.ResetDamageStatus();
    SetupDamageAfterLoad();
}

// 0x6C4330
bool CHeli::BurstTyre(uint8 tyreComponentId, bool bPhysicalEffect) {
    return false;
}

// 0x6C4320
bool CHeli::SetUpWheelColModel(CColModel* wheelCol) {
    return false;
}

// 0x6C4830
void CHeli::ProcessControlInputs(uint8 playerNum) {
    CPad* pad = CPad::GetPad(playerNum);

    // Throttle
    const int32 accelerate = pad->GetAccelerate();
    m_fThrottleControl = static_cast<float>(accelerate - pad->GetBrake()) * (1.0f / 255.0f);

    // Flight controls (Mouse / Analog)
    if (!CCamera::m_bUseMouse3rdPerson || !CVehicle::m_bEnableMouseFlying) {
        CVehicle::m_nLastControlInput = eControllerType::KEYBOARD;
        m_fPitchControl = static_cast<float>(pad->GetSteeringUpDown()) * (1.0f / 128.0f);
        m_fRollControl  = static_cast<float>(-pad->GetSteeringLeftRight()) * (1.0f / 128.0f);
    } else if (CPad::NewMouseControllerState.m_AmountMoved.x == 0.0f
               && CPad::NewMouseControllerState.m_AmountMoved.y == 0.0f
               && (!(std::fabs(m_fRollControl) > 0.0f || std::fabs(m_fPitchControl) > 0.0f)
                   || CVehicle::m_nLastControlInput != eControllerType::MOUSE
                   || pad->GetSteeringLeftRight()
                   || pad->GetSteeringUpDown())) {
        if (pad->GetSteeringLeftRight() || pad->GetSteeringUpDown() || CVehicle::m_nLastControlInput != eControllerType::MOUSE) {
            CVehicle::m_nLastControlInput = eControllerType::KEYBOARD;
            m_fPitchControl = static_cast<float>(pad->GetSteeringUpDown()) * (1.0f / 128.0f);
            m_fRollControl  = static_cast<float>(-pad->GetSteeringLeftRight()) * (1.0f / 128.0f);
        }
    } else {
        CVehicle::m_nLastControlInput = eControllerType::MOUSE;

        if (!pad->NewState.m_bVehicleMouseLook) {
            m_fRollControl  -= CPad::NewMouseControllerState.m_AmountMoved.x * 0.0025f;
            m_fPitchControl += CPad::NewMouseControllerState.m_AmountMoved.y * 0.0025f;
        }

        if (std::fabs(m_fRollControl) < 0.5f) {
            m_fRollControl *= std::pow(0.98f, CTimer::GetTimeStep());
        }

        if (std::fabs(m_fPitchControl) < 0.5f) {
            m_fPitchControl *= std::pow(0.98f, CTimer::GetTimeStep());
        }
    }

    m_fPitchControl = std::clamp(m_fPitchControl, -1.0f, 1.0f);
    m_fRollControl  = std::clamp(m_fRollControl, -1.0f, 1.0f);

    m_fYawControl = static_cast<float>(pad->GetLookRight());
    if (pad->GetLookLeft()) {
        m_fYawControl = -1.0f;
    }

    if (pad->GetHorn() && GetMatrix().GetUp().z > 0.0f) {
        m_fYawControl = 0.0f;

        CVector vecTemp = CrossProduct(CVector(0.0f, 0.0f, 1.0f), GetMatrix().GetRight());
        vecTemp.Normalise();
        m_fPitchControl = std::clamp(DotProduct(vecTemp, m_vecMoveSpeed) * m_pFlyingHandlingData->m_fPitchStab, -2.0f, 2.0f);

        vecTemp = CrossProduct(GetMatrix().GetForward(), CVector(0.0f, 0.0f, 1.0f));
        vecTemp.Normalise();
        m_fRollControl = std::clamp(DotProduct(vecTemp, m_vecMoveSpeed) * m_pFlyingHandlingData->m_fRollStab, -2.0f, 2.0f);
    }

    // Reset vehicle controls
    SetSteerAngle(0.0f);
    SetBrakePedal(1.0f);
    SetGasPedal(0.0f);
    SetIsHandbrakeOn(false);

    // Locked controls (cutscenes)
    if (pad->DisablePlayerControls) {
        if (auto* player = FindPlayerPed()) {
            player->KeepAreaAroundPlayerClear();
        }

        const float speed = m_vecMoveSpeed.Magnitude();
        if (speed > 0.28f) {
            m_vecMoveSpeed *= (0.28f / speed);
        }
    }

    // Critical damage
    if (m_fHealth < 250.0f) {
        m_fThrottleControl = -0.1f;
        m_fYawControl += 0.5f;
    }
}

// 0x6C4400
void CHeli::Render() {
    auto* mi = GetVehicleModelInfo();
    m_nTimeTillWeNeedThisCar = CTimer::GetTimeInMS() + 3000;
    mi->SetVehicleColour(m_nPrimaryColor, m_nSecondaryColor, m_nTertiaryColor, m_nQuaternaryColor);

    auto staticRotor = m_aCarNodes[HELI_STATIC_ROTOR];
    RpAtomic* data = nullptr;
    if (staticRotor) {
        RwFrameForAllObjects(staticRotor, GetCurrentAtomicObjectCB, &data);
        if (data)
            CVehicle::SetComponentAtomicAlpha(data, 255);
    }

    auto staticRotor2 = m_aCarNodes[HELI_STATIC_ROTOR2];
    data = nullptr;
    if (staticRotor2) {
        RwFrameForAllObjects(staticRotor2, GetCurrentAtomicObjectCB, &data);
        if (data)
            CVehicle::SetComponentAtomicAlpha(data, 255);
    }

    auto movingRotor = m_aCarNodes[HELI_MOVING_ROTOR];
    data = nullptr;
    if (movingRotor) {
        RwFrameForAllObjects(movingRotor, GetCurrentAtomicObjectCB, &data);
        if (data)
            CVehicle::SetComponentAtomicAlpha(data, 0);
    }

    auto movingRotor2 = m_aCarNodes[HELI_MOVING_ROTOR2];
    data = nullptr;
    if (movingRotor2) {
        RwFrameForAllObjects(movingRotor2, GetCurrentAtomicObjectCB, &data);
        if (data)
            CVehicle::SetComponentAtomicAlpha(data, 0);
    }

    CEntity::Render(); // exactly CEntity
}

// 0x6C4550
void CHeli::SetupDamageAfterLoad() {
    vehicleFlags.bIsDamaged = false;
}

// 0x6C4E60
//Vanilla BUG: There is a small bug related in this case, when it comes to a Seasparrow or a LEVIATHN the blades should not stop when touching the water but there is a small reset.
void CHeli::ProcessFlyingCarStuff() {
    const auto vehicleType = AsAutomobile();

    // Please do not confuse with type of automobile!
    if (!GetStatus() || GetStatus() == STATUS_REMOTE_CONTROLLED || GetStatus() == STATUS_PHYSICS) {
        // Handle wheel angular velocity
        if (vehicleType->m_fHeliRotorSpeed < 0.22f && !physicalFlags.bTouchingWater) {
            vehicleType->m_fHeliRotorSpeed += (m_nModelIndex == MODEL_RCGOBLIN || m_nModelIndex == MODEL_RCRAIDER) ? 0.003f : 0.001f;
        }

        // Process flying controls if needed
        if (m_fHeliRotorSpeed > 0.15) {
            if (vehicleFlags.bIsRCVehicle ||
                (m_NumDriveWheelsOnGround < 4 && !(IsAmphibiousHeli() && physicalFlags.bTouchingWater) 
                || (m_fThrottleControl > 0.0f) 
                ||std::abs(m_vecMoveSpeed.x) > 0.02f 
                ||std::abs(m_vecMoveSpeed.y) > 0.02f 
                ||std::abs(m_vecMoveSpeed.z) > 0.02f))
            {
                FlyingControl(
                    vehicleFlags.bIsRCVehicle ? FLIGHT_MODEL_RC : FLIGHT_MODEL_HELI,
                    m_fYawControl,
                    m_fPitchControl,
                    m_fRollControl,
                    m_fThrottleControl
                );
            }
        }

        // Process blade collision if needed
        if (vehicleType->m_fHeliRotorSpeed > 0.015f) {
            auto bladeFrame = m_aCarNodes[PLANE_STATIC_PROP];
            if (bladeFrame) {
                auto bladeMatrix = CMatrix();
                bladeMatrix.Attach(RwFrameGetMatrix(bladeFrame), false);

                RpAtomic* atomic = nullptr;
                RwFrameForAllObjects(bladeFrame, GetCurrentAtomicObjectCB, &atomic);

                if (atomic) {
                    if ((atomic->interpolator.flags & rpINTERPOLATORDIRTYSPHERE) != 0) {
                        _rpAtomicResyncInterpolatedSphere(atomic);
                    }

                    float bladeRadius = atomic->boundingSphere.radius;
                    if (bladeRadius > 0.1f) {
                        float collisionFactor = 1.0f;
                        switch (m_nModelIndex) {
                        case MODEL_RCRAIDER:
                        case MODEL_RCGOBLIN:
                            collisionFactor = 0.9f;
                            break;
                        case MODEL_SPARROW:
                        case MODEL_SEASPAR:
                            collisionFactor = 0.8f;
                            break;
                        case MODEL_HUNTER:
                            collisionFactor = 0.5f;
                            break;
                        }

                        if (!GetStatus() || GetStatus() == STATUS_REMOTE_CONTROLLED) {
                            // Probably for optimization reasons only the top rotor was chosen.
                            CVehicle::DoBladeCollision(bladeMatrix.GetPosition(), GetMatrix(), ROTOR_TOP, bladeRadius, collisionFactor);
                        }
                    }
                }

                if (GetStatus() && GetStatus() != STATUS_PHYSICS || vehicleType->m_fHeliRotorSpeed <= 0.0075f) {
                    if (GetStatus() == STATUS_SIMPLE) {
                        CVector windPosition = GetPosition();
                        CWindModifiers::RegisterOne(windPosition, 1, 1.0f);
                    }
                } else {
                    CVector windPosition = GetPosition();
                    CWindModifiers::RegisterOne(windPosition, 1, std::min(vehicleType->m_fHeliRotorSpeed * 6.6666665f, 1.0f));
                }
            }
        }
    } else {
        if (!vehicleType->IsRealHeli()) {
            return;
        }

        vehicleType->vehicleFlags.bEngineOn = false;
        float speedReduction = CTimer::GetTimeStep() * 0.00055f;
        if (speedReduction >= vehicleType->m_fHeliRotorSpeed) {
            vehicleType->m_fHeliRotorSpeed = 0.0f;
        } else {
            m_nFakePhysics = 0;
            vehicleType->m_fHeliRotorSpeed -= speedReduction;
        }
    }

    // Process audio events for specific models
    if (m_nModelIndex != MODEL_RCRAIDER && m_nModelIndex != MODEL_RCGOBLIN && vehicleType->m_fHeliRotorSpeed < 0.154f && vehicleType->m_fHeliRotorSpeed > 0.0044f) {
        RwFrame* bladeFrame = m_aCarNodes[PLANE_STATIC_PROP];
        if (bladeFrame) {
            CVector camDist = TheCamera.GetPosition() - GetPosition();
            float   distSq  = camDist.SquaredMagnitude();

            if (distSq < sq(20.0f) && std::abs(vehicleType->m_fPropRotate - m_wheelRotation[1]) > DegreesToRadians(30.0f)) { // SQ(20) mts = 400 e. dis.
                CMatrix mat;
                mat.Attach(RwFrameGetMatrix(bladeFrame), false);
                CVector bladeDirection = mat.GetRight();
                m_matrix->InverseTransformVector(bladeDirection);

                float distance = 1.0f / std::max(std::sqrt(distSq), 0.01f);
                if (std::abs(DotProduct(camDist, bladeDirection)) > HELI_ROTOR_DOTPROD_LIMIT) {
                    vehicleType->m_vehicleAudio.AddAudioEvent(AE_HELI_BLADE, 0.0f);
                    vehicleType->m_fPropRotate = m_wheelRotation[1];
                }
            }
        }
    }
}

// 0x6C5420
void CHeli::PreRender() {
    CVehicle::PreRender();

    auto* const mi = GetVehicleModelInfo();

    CMatrix matrix;
    CVector posn;

    if (m_bSearchLightOn && m_LightBrightness > 0.0f && CClock::GetIsTimeInRange(19, 6)) {
        AddHeliSearchLight(
            m_matrix->TransformPoint({ 0.0f, 3.5f, -0.3f }),
            { m_SearchLightX, m_SearchLightY, m_SearchLightZ },
            20.0f,
            m_LightBrightness,
            (uint32)((uintptr_t)this + 0xB),
            true,
            true
        );
    }

    CEntity::GetColModel();

    if (vehicleFlags.bVehicleColProcessed) {
        DoBurstAndSoftGroundRatios();

        for (auto i = 0; i < 4; i++) {
            const auto r     = 1.0f - m_fSuspensionLength[i] / m_fLineLength[i];
            const auto ratio = (m_fWheelsSuspensionCompression[i] - r) / (1.0f - r);

            mi->GetWheelPosn(i, posn, true);

            auto height = posn.z + m_pHandlingData->m_fSuspensionUpperLimit;
            if (ratio > 0.0f) {
                height -= ratio * m_fSuspensionLength[i];
            }

            if (height > m_wheelPosition[i] || (physicalFlags.bDisableCollisionForce && handlingFlags.bHydraulicInst)) {
                m_wheelPosition[i] = height;
            } else {
                m_wheelPosition[i] += (height - m_wheelPosition[i]) * 0.3f;
            }
        }
    }

    UpdateWheelMatrix(CAR_WHEEL_RB, 1);
    UpdateWheelMatrix(CAR_WHEEL_LB, 1);
    UpdateWheelMatrix(CAR_WHEEL_RF, 1);
    UpdateWheelMatrix(CAR_WHEEL_LF, 1);

    if (m_nModelIndex != MODEL_RCRAIDER && m_nModelIndex != MODEL_RCGOBLIN) {
        DoHeliDustEffect(1.0f, 1.0f);
    }

    // The rotor speed lives in the wheel-speed slot, `m_fEngineSpeed` is never written
    const auto rotorSpeed = m_wheelSpeed[1];

    const auto isFastRotor = notsa::contains({ MODEL_SPARROW, MODEL_SEASPAR, MODEL_MAVERICK, MODEL_VCNMAV, MODEL_POLMAV }, (eModelID)m_nModelIndex);
    m_fMainRotorAngle -= CTimer::GetTimeStep() * rotorSpeed * (isFastRotor ? EXTRA_HELI_ROTOR_SPIN_SPEED_MULT : 1.0f);
    while (m_fMainRotorAngle < -TWO_PI) {
        m_fMainRotorAngle += TWO_PI;
    }

    m_fRearRotorAngle -= CTimer::GetTimeStep() * rotorSpeed * (m_nModelIndex == MODEL_LEVIATHN ? 2.0f : REAR_ROTOR_SPIN_SPEED_MULT);
    while (m_fRearRotorAngle > TWO_PI) {
        m_fRearRotorAngle -= TWO_PI;
    }

    const auto SpinRotor = [&](eHeliNodes node, bool aroundZ, float angle) {
        if (auto* const frame = m_aCarNodes[node]) {
            matrix.Attach(RwFrameGetMatrix(frame), false);
            const auto offset = matrix.GetPosition();
            if (aroundZ) {
                matrix.SetRotateZ(angle);
            } else {
                matrix.SetRotateX(angle);
            }
            matrix.GetPosition() += offset;
            matrix.UpdateRW();
        }
    };

    SpinRotor(HELI_STATIC_ROTOR,  true,  m_fMainRotorAngle);
    SpinRotor(HELI_MOVING_ROTOR,  true,  m_fMainRotorAngle);
    SpinRotor(HELI_STATIC_ROTOR2, false, m_fRearRotorAngle);
    SpinRotor(HELI_MOVING_ROTOR2, false, m_fRearRotorAngle);

    CShadows::StoreShadowForVehicle(this, VEH_SHD_HELI);
}

// 0x6C7050
void CHeli::ProcessControl() {
    CAutomobile::ProcessControl();

    if (!vehicleFlags.bEngineOn) {
        if (m_pDustParticle) {
            m_pDustParticle->Kill();
            m_pDustParticle = nullptr;
            m_heliDustFxTimeConst = 0.0f;
        }
    }

    int32 playerId = 0;
    if (m_pDriver && m_pDriver->m_nPedType == PED_TYPE_PLAYER2) {
        playerId = 1;
    }
    CPad* pPad = CPad::GetPad(playerId);
    if (pPad->HornJustDown()) {
        m_bSearchLightOn = !m_bSearchLightOn;
    }

    bool bDoSearchLight = false;
    bool bFireSearchLightGun = false;
    CPhysical* pSearchLightTarget = nullptr;

    if (physicalFlags.bRenderScorched || CCullZones::PlayerNoRain()) {
        m_LightBrightness = 0.0f;
    } else {
        if (m_autoPilot.Mission == MISSION_HELI_POLICE_BEHAVIOUR && (!FindPlayerVehicle(-1, false) || (FindPlayerVehicle(-1, false)->GetVehicleType() != VEHICLE_TYPE_HELI && FindPlayerVehicle(-1, false)->GetVehicleType() != VEHICLE_TYPE_PLANE))) {
            bDoSearchLight = true;
            bFireSearchLightGun = true;
            pSearchLightTarget = static_cast<CPhysical*>(FindPlayerEntity(-1));
        } else if (m_autoPilot.Mission == MISSION_HELI_FOLLOW_ENTITY && m_autoPilot.pTargetEntity && m_nHeliFlags.bUseSearchLightOnTarget) {
            bDoSearchLight = true;
            bFireSearchLightGun = false;
            pSearchLightTarget = static_cast<CPhysical*>(m_autoPilot.pTargetEntity);
        } else if (GetStatus() == STATUS_PLAYER && m_nModelIndex == MODEL_POLMAV && m_bSearchLightOn) {
            bDoSearchLight = true;
            bFireSearchLightGun = false;
            pSearchLightTarget = nullptr;
        }

        if (physicalFlags.bSubmergedInWater) {
            bDoSearchLight = false;
            bFireSearchLightGun = false;
        }

        m_bSearchLightOn = bDoSearchLight;

        if (bDoSearchLight) {
            int32 TimePassed;
            int32 WaitBeforeOpeningFire = 0;
            float Interp;
            float LightDist;
            float XDiff;
            float YDiff;
            float DistSqr;
            CVector TargetPos;
            CVector TargetSpeed;

            if (pSearchLightTarget) {
                TargetPos = pSearchLightTarget->GetPosition();
                TargetSpeed = pSearchLightTarget->m_vecMoveSpeed;
            } else {
                TargetPos = GetPosition() + GetForward() * 10.0f + GetUp() * -30.0f;
                TargetSpeed = m_vecMoveSpeed;
            }

            TimePassed = static_cast<int32>(CTimer::m_snTimeInMilliseconds - m_LastSearchLightSample);
            while (TimePassed > 1000) {
                for (int32 i = 5; i > 0; --i) {
                    m_OldSearchLightX[i] = m_OldSearchLightX[i - 1];
                    m_OldSearchLightY[i] = m_OldSearchLightY[i - 1];
                }
                m_LastSearchLightSample += 1000;
                m_OldSearchLightX[0] = TargetPos.x + TargetSpeed.x * 100.0f;
                m_OldSearchLightY[0] = TargetPos.y + TargetSpeed.y * 100.0f;
                TimePassed -= 1000;
            }

            Interp = static_cast<float>(TimePassed) * 0.001f;
            m_SearchLightZ = TargetPos.z;
            m_SearchLightX = (1.0f - Interp) * m_OldSearchLightX[2] + Interp * m_OldSearchLightX[1];
            m_SearchLightY = (1.0f - Interp) * m_OldSearchLightY[2] + Interp * m_OldSearchLightY[1];

            DistSqr = (m_SearchLightY - GetPosition().y) * (m_SearchLightY - GetPosition().y)
                    + (m_SearchLightX - GetPosition().x) * (m_SearchLightX - GetPosition().x);
            LightDist = std::sqrt(DistSqr);

            if (LightDist > 60.0f) {
                m_LightBrightness = 0.0f;
            } else if (LightDist < 40.0f) {
                m_LightBrightness = 1.0f;
            } else {
                m_LightBrightness = 1.0f - (LightDist - 40.0f) * 0.05f;
            }

            XDiff = TargetPos.x - m_SearchLightX;
            YDiff = TargetPos.y - m_SearchLightY;
            DistSqr = XDiff * XDiff + YDiff * YDiff;
            if (m_LightBrightness < 0.9f || DistSqr > 49.0f) {
                m_LastTimeSearchLightWasTooFarAwayToShoot = CTimer::m_snTimeInMilliseconds;
                m_nTimeForMinigunFiring = m_LastTimeSearchLightWasTooFarAwayToShoot;
            } else {
                uint32 timer = CTimer::m_snTimeInMilliseconds;
                if (static_cast<int32>(timer) > m_nNextTalkTimer) {
                    m_nNextTalkTimer = timer + (CGeneral::GetRandomNumber() & 0xFFF) + 4500;
                }
            }

            if (bFireSearchLightGun) {
                switch (FindPlayerPed(-1)->GetWantedLevel()) {
                case eWantedLevel::WANTED_CLEAN:
                case eWantedLevel::WANTED_LEVEL_1:
                case eWantedLevel::WANTED_LEVEL_2:
                    WaitBeforeOpeningFire = 999999;
                    break;
                case eWantedLevel::WANTED_LEVEL_3:
                    WaitBeforeOpeningFire = 10000;
                    break;
                case eWantedLevel::WANTED_LEVEL_4:
                    WaitBeforeOpeningFire = 5000;
                    break;
                case eWantedLevel::WANTED_LEVEL_5:
                    WaitBeforeOpeningFire = 3500;
                    break;
                case eWantedLevel::WANTED_LEVEL_6:
                    WaitBeforeOpeningFire = 2000;
                    break;
                }

                if (FindPlayerPed(-1)->GetWantedLevel() != eWantedLevel::WANTED_CLEAN) {
                    AudioEngine.SayPedless(AE_SPEECH_PED, CTX_GLOBAL_POLICE_HELICOPTER, this, 0, 1.0f, false, false, false);
                }

                if (CCullZones::NoPolice()) {
                    WaitBeforeOpeningFire /= 2;
                }

                if (pSearchLightTarget != FindPlayerPed(-1)) {
                    WaitBeforeOpeningFire = 5000;
                }

                if (!FindPlayerWanted(-1)->PoliceBackOff()) {
                    CVector ShotOrigin = GetMatrix().TransformPoint(CVector(0.0f, 3.5f, -1.0f));

                    if (CTimer::m_snTimeInMilliseconds > m_LastTimeSearchLightWasTooFarAwayToShoot + WaitBeforeOpeningFire && CTimer::m_snPreviousTimeInMilliseconds <= m_LastTimeSearchLightWasTooFarAwayToShoot + WaitBeforeOpeningFire) {
                        if (!CWorld::GetIsLineOfSightClear(ShotOrigin, TargetPos, true, false, false, false, false, false, false)) {
                            m_LastTimeSearchLightWasTooFarAwayToShoot = CTimer::m_snTimeInMilliseconds;
                            m_nTimeForMinigunFiring = m_LastTimeSearchLightWasTooFarAwayToShoot;
                        }
                    }

                    if (CTimer::m_snTimeInMilliseconds > m_LastTimeSearchLightWasTooFarAwayToShoot + WaitBeforeOpeningFire && CTimer::m_snTimeInMilliseconds > m_nTimeForMinigunFiring) {
                        CVector ToExtendAWeeBit;
                        CVector TargetCoors;
                        CVector Temp;

                        TargetCoors = TargetPos;
                        TargetCoors.x += ((CGeneral::GetRandomNumber() & 0xFF) - 128) * 0.02f;
                        TargetCoors.y += ((CGeneral::GetRandomNumber() & 0xFF) - 128) * 0.02f;

                        ToExtendAWeeBit = TargetPos - ShotOrigin;
                        ToExtendAWeeBit.Normalise();

                        Temp = ShotOrigin;
                        TargetCoors += ToExtendAWeeBit * 3.0f;
                        Temp += ToExtendAWeeBit * 3.0f;

                        FireOneInstantHitRound(Temp, TargetCoors, 20);
                        AudioEngine.ReportWeaponEvent(AE_WEAPON_FIRE, WEAPON_M4, this);

                        if (CGeneral::GetRandomNumberInRange(0.0f, 1.0f) < gHeliFiringTimeStepRandomizer) {
                            m_nTimeForMinigunFiring = CTimer::m_snTimeInMilliseconds + 400;
                        } else {
                            m_nTimeForMinigunFiring = CTimer::m_snTimeInMilliseconds + 150;
                        }
                    }
                } else {
                    m_LastTimeSearchLightWasTooFarAwayToShoot = CTimer::m_snTimeInMilliseconds;
                    m_nTimeForMinigunFiring = m_LastTimeSearchLightWasTooFarAwayToShoot;
                }
            }
        }
    }

    if (m_autoPilot.Mission == MISSION_HELI_POLICE_BEHAVIOUR && m_nSwatOnBoard > 0) {
        SendDownSwat();
        g_InterestingEvents.Add(CInterestingEvents::ESwatTeamAbseiling, this);
    }

    for (int32 Rope = 0; Rope < 4; ++Rope) {
        if (m_SwatRopeActive[Rope]) {
            --m_SwatRopeActive[Rope];
            CRopes::RegisterRope(static_cast<uint32>(reinterpret_cast<uintptr_t>(this) + Rope), static_cast<uint32>(eRopeType::SWAT), GetMatrix().TransformPoint(FindSwatPositionRelativeToHeli(Rope)), false, 0, false, nullptr, 20000);

            if (!m_SwatRopeActive[Rope]) {
                CVector Temp = GetMatrix().TransformVector(FindSwatPositionRelativeToHeli(Rope) * 0.05f);
                Temp.z = 0.0f;
                CRopes::SetSpeedOfTopNode(static_cast<uint32>(reinterpret_cast<uintptr_t>(this) + Rope), Temp);
            }
        }
    }

    UpdateWinch();
    ProcessWeapons();

    if (g_InterestingEvents.m_bIsActive) {
        float fTimeStep = CTimer::GetTimeStepInSeconds();
        float fChance = fTimeStep * 0.1f;
        if (bFireSearchLightGun) {
            fChance *= 2.0f;
        }

        float fRandVal = CGeneral::GetRandomNumberInRange(0.0f, 1.0f);
        if (fRandVal < fChance) {
            g_InterestingEvents.Add(CInterestingEvents::EHelicopterOverhead, this);
        }
    }
}

