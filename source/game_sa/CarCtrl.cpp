/*
    Plugin-SDK file
    Authors: GTA Community. See more here
    https://github.com/DK22Pac/plugin-sdk
    Do not delete this comment block. Respect others' work!
*/

#include "StdInc.h"

#include "CarCtrl.h"
#include "TrafficLights.h"
#include "TheScripts.h"
#include "GangWars.h"
#include "Game.h"
#include "General.h"
#include "GameLogic.h"
#include "CutsceneMgr.h"
#include "TheCarGenerators.h"
#include "eAreaCodes.h"
#include "CarAI.h"
#include "Cheat.h"
#include "PopCycle.h"
#include "Population.h"
#include "CullZones.h"
#include "Curves.h"
#include "VisibilityPlugins.h"
#include "WaterLevel.h"
#include "Streaming.h"
#include "Camera.h"
#include "VehicleModelInfo.h"

#include <reversiblebugfixes/Bugs.hpp>

// uint32& CCarCtrl::NumLawEnforcerCars                 = *(uint32*)0x969098;
// uint32& CCarCtrl::NumParkedCars                      = *(uint32*)0x9690A0;
// uint32& CCarCtrl::NumAmbulancesOnDuty                = *(uint32*)0x9690A8;
// uint32& CCarCtrl::NumFireTrucksOnDuty                = *(uint32*)0x9690AC;
// uint32& CCarCtrl::MaxNumberOfCarsInUse               = *(uint32*)0x8A5B24;
// float&  CCarCtrl::CarDensityMultiplier               = *(float*)0x8A5B20;
// int32&  CCarCtrl::NumRandomCars                      = *(int32*)0x969094;
// int32&  CCarCtrl::NumMissionCars                     = *(int32*)0x96909C;
// int32&  CCarCtrl::NumPermanentVehicles               = *(int32*)0x9690A4;
// int32&  CCarCtrl::LastTimeAmbulanceCreated           = *(int32*)0x9690B0;
// int32&  CCarCtrl::LastTimeFireTruckCreated           = *(int32*)0x9690B4;
// bool&   CCarCtrl::bAllowEmergencyServicesToBeCreated = *(bool*)0x8A5B28;
// bool&   CCarCtrl::bCarsGeneratedAroundCamera         = *(bool*)0x9690C1;
// int8&   CCarCtrl::CountDownToCarsAtStart             = *(int8*)0x9690C0;
// float&  CCarCtrl::TimeNextMadDriverChaseCreated      = *(float*)0x9690BC;
// int32&  CCarCtrl::SequenceElements                   = *(int32*)0x969078;
// int32&  CCarCtrl::SequenceRandomOffset               = *(int32*)0x969074;
// bool&   CCarCtrl::bSequenceOtherWay                  = *(bool*)0x969070;
// int32&  CCarCtrl::LastTimeLawEnforcerCreated         = *(int32*)0x9690B8;

CVehicle* (&apCarsToKeep)[2]                         = *(CVehicle * (*)[2])0x969084;
uint32 (&aCarsToKeepTime)[2]                         = *(uint32 (*)[2])0x96907C;

void CCarCtrl::InjectHooks() {
    RH_ScopedClass(CCarCtrl);
    RH_ScopedCategoryGlobal();

    using namespace ReversibleHooks;
    RH_ScopedInstall(Init, 0x4212E0);
    RH_ScopedInstall(ReInit, 0x4213B0);
    RH_ScopedInstall(InitSequence, 0x421740);
    RH_ScopedInstall(ChooseGangCarModel, 0x421A40, { .jmpCodeSize = 7 });
    RH_ScopedInstall(ChoosePoliceCarModel, 0x421980, { .jmpCodeSize = 7 });
    RH_ScopedInstall(CreateCarForScript, 0x431F80);
    RH_ScopedInstall(ChooseBoatModel, 0x421970);
    RH_ScopedInstall(ChooseCarModelToLoad, 0x421900);
    RH_ScopedInstall(GetNewVehicleDependingOnCarModel, 0x421440);
    RH_ScopedInstall(IsAnyoneParking, 0x42C250);
    RH_ScopedInstall(IsThisVehicleInteresting, 0x423EA0);
    RH_ScopedInstall(JoinCarWithRoadAccordingToMission, 0x432CB0);
    RH_ScopedInstall(PossiblyFireHSMissile, 0x429600);
    RH_ScopedInstall(PruneVehiclesOfInterest, 0x423F10);
    RH_ScopedInstall(RemoveCarsIfThePoolGetsFull, 0x4322B0);
    RH_ScopedInstall(RemoveDistantCars, 0x42CD10);
    RH_ScopedInstall(RemoveFromInterestingVehicleList, 0x423ED0);
    RH_ScopedInstall(ScriptGenerateOneEmergencyServicesCar, 0x42FBC0);
    RH_ScopedInstall(SlowCarDownForObject, 0x426220);
    RH_ScopedInstall(SlowCarOnRailsDownForTrafficAndLights, 0x434790);
    RH_ScopedInstall(FindMaxSteerAngle, 0x427FE0);
    RH_ScopedInstall(GenerateRandomCars, 0x4341C0);
    RH_ScopedInstall(GenerateOneRandomCar, 0x430050);
}

// 0x4212E0
void CCarCtrl::Init() {
    ZoneScoped;

    CarDensityMultiplier               = 1.0f;
    NumRandomCars                      = 0;
    NumLawEnforcerCars                 = 0;
    NumMissionCars                     = 0;
    NumParkedCars                      = 0;
    NumPermanentVehicles               = 0;
    NumAmbulancesOnDuty                = 0;
    NumFireTrucksOnDuty                = 0;

    LastTimeAmbulanceCreated           = 0;
    LastTimeFireTruckCreated           = 0;
    bAllowEmergencyServicesToBeCreated = true;
    bCarsGeneratedAroundCamera         = false;
    CountDownToCarsAtStart             = 2;

    TimeNextMadDriverChaseCreated      = CGeneral::GetRandomNumberInRange(600.0f, 1200.0f);

    std::ranges::fill(apCarsToKeep, nullptr);
    for (auto& group : CPopulation::m_LoadedGangCars) {
        group.Clear();
    }
    CPopulation::m_AppropriateLoadedCars.Clear();
    CPopulation::m_InAppropriateLoadedCars.Clear();
    CPopulation::m_LoadedBoats.Clear();
}

// 0x4213B0
void CCarCtrl::ReInit() {
    CarDensityMultiplier               = 1.0f;
    NumRandomCars                      = 0;
    NumLawEnforcerCars                 = 0;
    NumMissionCars                     = 0;
    NumParkedCars                      = 0;
    NumPermanentVehicles               = 0;
    NumAmbulancesOnDuty                = 0;
    NumFireTrucksOnDuty                = 0;

    LastTimeLawEnforcerCreated         = 0;

    bAllowEmergencyServicesToBeCreated = true;
    CountDownToCarsAtStart             = 2;

    std::ranges::fill(apCarsToKeep, nullptr);
    for (auto& group : CPopulation::m_LoadedGangCars) {
        group.Clear();
    }
    CPopulation::m_AppropriateLoadedCars.Clear();
    CPopulation::m_InAppropriateLoadedCars.Clear();
    CPopulation::m_LoadedBoats.Clear();
}

// 0x421970
int32 CCarCtrl::ChooseBoatModel() {
    return CPopulation::m_LoadedBoats.PickLeastUsedModel(1);
}

// 0x421900
int32 CCarCtrl::ChooseCarModelToLoad(int32 groupID) {
    const auto numCarsInGroup = CPopulation::m_nNumCarsInGroup[groupID];
    if (numCarsInGroup > 0) {
        for (auto i = 0; i < 16; i++) { // 16 tries
            const auto model = CPopulation::m_CarGroups[groupID][CGeneral::GetRandomNumberInRange(numCarsInGroup)];
            if (!CStreaming::IsModelLoaded(model)) {
                return model;
            }
        }
    }
    return -1;
}

eModelID CCarCtrl::ChooseGangCarModel(eGangID loadedCarGroupId) {
    return CPopulation::PickGangCar(loadedCarGroupId);
}

// 0x424CE0
int32 CCarCtrl::ChooseModel(int32* arg1) {
    return plugin::CallAndReturn<int32, 0x424CE0, int32*>(arg1);
}

int32 CCarCtrl::ChoosePoliceCarModel(uint32 ignoreLvpd1Model) {
    CWanted* playerWanted = FindPlayerWanted();
    if (playerWanted->AreSwatRequired()
        && CStreaming::IsModelLoaded(MODEL_ENFORCER)
        && CStreaming::IsModelLoaded(MODEL_SWAT)) {
        if (CGeneral::GetRandomNumberInRange(0, 3) == 2) {
            return MODEL_ENFORCER;
        }
    } else {
        if (playerWanted->AreFbiRequired()
            && CStreaming::IsModelLoaded(MODEL_FBIRANCH)
            && CStreaming::IsModelLoaded(MODEL_FBI)) {
            return MODEL_FBIRANCH;
        }

        if (playerWanted->AreArmyRequired()
            && CStreaming::IsModelLoaded(MODEL_RHINO)
            && CStreaming::IsModelLoaded(MODEL_BARRACKS)
            && CStreaming::IsModelLoaded(MODEL_ARMY)) {
            return (CGeneral::GetRandomNumber() < 0x3FFF) + MODEL_RHINO;
        }
    }
    return CStreaming::GetDefaultCopCarModel(ignoreLvpd1Model);
}

// 0x423F00
void CCarCtrl::ClearInterestingVehicleList() {
    plugin::Call<0x423F00>();
}

// 0x422760
void CCarCtrl::ClitargetOrientationToLink(CVehicle* vehicle, CCarPathLinkAddress arg2, int8 arg3, float* arg4, float arg5, float arg6) {
    plugin::Call<0x422760, CVehicle*, CCarPathLinkAddress, int8, float*, float, float>(vehicle, arg2, arg3, arg4, arg5, arg6);
}

// 0x431F80
CVehicle* CCarCtrl::CreateCarForScript(int32 modelid, CVector posn, bool doMissionCleanup) {
    if (CModelInfo::IsBoatModel(modelid)) {
        auto* boat = new CBoat(modelid, eVehicleCreatedBy::MISSION_VEHICLE);
        if (posn.z <= MAP_Z_LOW_LIMIT) {
            posn.z = CWorld::FindGroundZForCoord(posn.x, posn.y);
        }

        posn.z += boat->GetDistanceFromCentreOfMassToBaseOfModel();
        boat->SetPosn(posn);

        CTheScripts::ClearSpaceForMissionEntity(posn, boat);
        boat->vehicleFlags.bEngineOn = false;
        boat->vehicleFlags.bIsLocked = true;
        boat->SetStatus(STATUS_ABANDONED);
        JoinCarWithRoadSystem(boat);

        boat->m_autoPilot.SetCarMission(eCarMission::MISSION_NONE);
        boat->m_autoPilot.m_nTempAction = TEMPACT_NONE;
        boat->m_autoPilot.m_speed       = 20.0F;
        boat->m_autoPilot.SetCruiseSpeed(20);

        if (doMissionCleanup) {
            boat->m_bIsStaticWaitingForCollision = true;
        }

        boat->m_autoPilot.movementFlags.bIsStopped = true;
        CWorld::Add(boat);

        if (doMissionCleanup) {
            CTheScripts::MissionCleanUp.AddEntityToList(GetVehiclePool()->GetRef(boat), MISSION_CLEANUP_ENTITY_TYPE_VEHICLE);
        }

        return boat;
    }

    auto* vehicle = GetNewVehicleDependingOnCarModel(modelid, eVehicleCreatedBy::MISSION_VEHICLE);
    if (posn.z <= MAP_Z_LOW_LIMIT) {
        posn.z = CWorld::FindGroundZForCoord(posn.x, posn.y);
    }

    posn.z += vehicle->GetDistanceFromCentreOfMassToBaseOfModel();
    vehicle->SetPosn(posn);

    if (!doMissionCleanup) {
        if (vehicle->IsAutomobile()) {
            vehicle->AsAutomobile()->PlaceOnRoadProperly();
        } else if (vehicle->IsBike()) {
            vehicle->AsBike()->PlaceOnRoadProperly();
        }
    }

    if (vehicle->IsTrain()) {
        vehicle->AsTrain()->trainFlags.bNotOnARailRoad = true;
    }

    CTheScripts::ClearSpaceForMissionEntity(posn, vehicle);
    vehicle->vehicleFlags.bIsLocked = true;
    vehicle->SetStatus(STATUS_ABANDONED);
    JoinCarWithRoadSystem(vehicle);
    vehicle->vehicleFlags.bEngineOn             = false;
    vehicle->vehicleFlags.bHasBeenOwnedByPlayer = true;

    vehicle->m_autoPilot.SetCarMission(eCarMission::MISSION_NONE);
    vehicle->m_autoPilot.m_nTempAction      = TEMPACT_NONE;
    vehicle->m_autoPilot.m_nCarDrivingStyle = DRIVING_STYLE_STOP_FOR_CARS;
    vehicle->m_autoPilot.m_speed            = 13.0F;
    vehicle->m_autoPilot.SetCruiseSpeed(13);
    vehicle->m_autoPilot.m_nCurrentLane = 0;
    vehicle->m_autoPilot.m_nNextLane    = 0;

    if (doMissionCleanup) {
        vehicle->m_bIsStaticWaitingForCollision = true;
    }

    CWorld::Add(vehicle);
    if (doMissionCleanup) {
        CTheScripts::MissionCleanUp.AddEntityToList(GetVehiclePool()->GetRef(vehicle), MISSION_CLEANUP_ENTITY_TYPE_VEHICLE);
    }

    if (vehicle->IsSubRoadVehicle()) {
        vehicle->m_autoPilot.movementFlags.bIsStopped = true;
    }

    return vehicle;
}

// 0x42C740
bool CCarCtrl::CreateConvoy(CVehicle* vehicle, int32 arg2) {
    return plugin::CallAndReturn<bool, 0x42C740, CVehicle*, int32>(vehicle, arg2);
}

// 0x42C2B0
bool CCarCtrl::CreatePoliceChase(CVehicle* vehicle, int32 arg2, CNodeAddress NodeAddress) {
    return plugin::CallAndReturn<bool, 0x42C2B0, CVehicle*, int32, CNodeAddress>(vehicle, arg2, NodeAddress);
}

// 0x428040
bool CCarCtrl::DealWithBend_Racing(CVehicle* vehicle, CCarPathLinkAddress LinkAddress1, CCarPathLinkAddress LinkAddress2, CCarPathLinkAddress LinkAddress3, CCarPathLinkAddress LinkAddress4, char arg6, char arg7, char arg8, char arg9, float arg10, float* arg11, float* arg12, float* arg13, float* arg14, CVector* pos) {
    return plugin::CallAndReturn<bool, 0x428040, CVehicle*, CCarPathLinkAddress, CCarPathLinkAddress, CCarPathLinkAddress, CCarPathLinkAddress, int8, int8, int8, int8, float, float*, float*, float*, float*, CVector*>(vehicle, LinkAddress1, LinkAddress2, LinkAddress3, LinkAddress4, arg6, arg7, arg8, arg9, arg10, arg11, arg12, arg13, arg14, pos);
}

// 0x42EC90
void CCarCtrl::DragCarToPoint(CVehicle* vehicle, CVector* pos) {
    plugin::Call<0x42EC90, CVehicle*, CVector*>(vehicle, pos);
}

// 0x4325C0
float CCarCtrl::FindAngleToWeaveThroughTraffic(CVehicle* vehicle, CPhysical* physical, float arg3, float arg4, float arg5) {
    return plugin::CallAndReturn<float, 0x4325C0, CVehicle*, CPhysical*, float, float, float>(vehicle, physical, arg3, arg4, arg5);
}

// 0x4226F0
void CCarCtrl::FindIntersection2Lines(float arg1, float arg2, float arg3, float arg4, float arg5, float arg6, float arg7, float arg8, float* arg9, float* arg10) {
    plugin::Call<0x4226F0, float, float, float, float, float, float, float, float, float*, float*>(arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10);
}

// 0x42B470
void CCarCtrl::FindLinksToGoWithTheseNodes(CVehicle* vehicle) {
    plugin::Call<0x42B470, CVehicle*>(vehicle);
}

// 0x434400
float CCarCtrl::FindMaximumSpeedForThisCarInTraffic(CVehicle* vehicle) {
    return plugin::CallAndReturn<float, 0x434400, CVehicle*>(vehicle);
}

// 0x42BD20
void CCarCtrl::FindNodesThisCarIsNearestTo(CVehicle* vehicle, CNodeAddress& nodeAddress1, CNodeAddress& nodeAddress2) {
    plugin::Call<0x42BD20, CVehicle*, CNodeAddress&, CNodeAddress&>(vehicle, nodeAddress1, nodeAddress2);
}

// 0x422090
int8 CCarCtrl::FindPathDirection(CNodeAddress nodeAddress1, CNodeAddress nodeAddress2, CNodeAddress nodeAddress3, bool* arg4) {
    return plugin::CallAndReturn<int8, 0x422090, CNodeAddress, CNodeAddress, CNodeAddress, bool*>(nodeAddress1, nodeAddress2, nodeAddress3, arg4);
}

// 0x422620
float CCarCtrl::FindPercDependingOnDistToLink(CVehicle* vehicle, CCarPathLinkAddress linkAddress) {
    return plugin::CallAndReturn<float, 0x422620, CVehicle*, CCarPathLinkAddress>(vehicle, linkAddress);
}

// 0x421770
int32 CCarCtrl::FindSequenceElement(int32 arg1) {
    return plugin::CallAndReturn<int32, 0x421770, int32>(arg1);
}

// 0x4224E0
float CCarCtrl::FindSpeedMultiplier(float arg1, float arg2, float arg3, float arg4) {
    return plugin::CallAndReturn<float, 0x4224E0, float, float, float, float>(arg1, arg2, arg3, arg4);
}

// 0x424130
float CCarCtrl::FindSpeedMultiplierWithSpeedFromNodes(int8 arg1) {
    return plugin::CallAndReturn<float, 0x424130, int8>(arg1);
}

float CCarCtrl::FindGhostRoadHeight(CVehicle* vehicle) {
    return plugin::CallAndReturn<float, 0x422370, CVehicle*>(vehicle);
}

// 0x42B270
void CCarCtrl::FireHeliRocketsAtTarget(CAutomobile* entityLauncher, CEntity* entity) {
    plugin::Call<0x42B270, CAutomobile*, CEntity*>(entityLauncher, entity);
}

// 0x429A70
void CCarCtrl::FlyAIHeliInCertainDirection(CHeli* heli, float arg2, float arg3, bool arg4) {
    plugin::Call<0x429A70, CHeli*, float, float, bool>(heli, arg2, arg3, arg4);
}

// 0x423940
void CCarCtrl::FlyAIHeliToTarget_FixedOrientation(CHeli* heli, float Orientation, CVector posn) {
    plugin::Call<0x423940, CHeli*, float, CVector>(heli, Orientation, posn);
}

// 0x423000
void CCarCtrl::FlyAIPlaneInCertainDirection(CPlane* pPlane) {
    plugin::Call<0x423000, CPlane*>(pPlane);
}

// 0x424210
bool CCarCtrl::GenerateCarCreationCoors2(CVector posn, float dirX, float dirY, float cosLimit, bool invertTest, float preferredDist, float fallbackDist, CVector* pOrigin, CNodeAddress* pNodeAddress1, CNodeAddress* pNodeAddress12, float* outFraction, bool arg12, bool arg13) {
    return plugin::CallAndReturn<bool, 0x424210, CVector, float, float, float, bool, float, float, CVector*, CNodeAddress*, CNodeAddress*, float*, bool, bool>(posn, dirX, dirY, cosLimit, invertTest, preferredDist, fallbackDist, pOrigin, pNodeAddress1, pNodeAddress12, outFraction, arg12, arg13);
}

// 0x42F9C0
void CCarCtrl::GenerateEmergencyServicesCar() {
    plugin::Call<0x42F9C0>();
}

// 0x42B7D0
CAutomobile* CCarCtrl::GenerateOneEmergencyServicesCar(uint32 modelId, CVector posn) {
    return plugin::CallAndReturn<CAutomobile*, 0x42B7D0, uint32, CVector>(modelId, posn);
}

// 0x430050
void CCarCtrl::GenerateOneRandomCar() {
    bool isMadDriver      = false;
    bool isRestrictedZone = false;

    // Get player position and speed
    const CVector& playerCentreOfWorld = FindPlayerCentreOfWorld(CWorld::PlayerInFocus);
    float          x                   = playerCentreOfWorld.x;
    float          y                   = playerCentreOfWorld.y;
    float          z                   = playerCentreOfWorld.z;
    CVector        playerPos           = playerCentreOfWorld;

    //! Subtracted from the spawned vehicle's curve velocity further down, so the "is it driving
    //! towards us" test is done in the player's frame of reference.
    const CVector playerSpeed          = FindPlayerSpeed(-1);

    // Count total cars currently in use
    int32 numCarsInUse = NumRandomCars + NumLawEnforcerCars + NumMissionCars + NumAmbulancesOnDuty + NumFireTrucksOnDuty;

    // Apply density multiplier
    float carDensity = CarDensityMultiplier;
    if (CCullZones::FewerCars()) {
        carDensity *= 0.6f;
    }

    // Check density limits
    if (CPopulation::FindCarMultiplierMotorway() * (float)MaxNumberOfCarsInUse * carDensity <= (float)numCarsInUse
        || CPopulation::FindCarMultiplierMotorway()
                * (CPopCycle::m_NumOther_Cars + CPopCycle::m_NumCops_Cars + CPopCycle::m_NumGangs_Cars + CPopCycle::m_NumDealers_Cars)
                * carDensity
            <= (float)numCarsInUse) {
        return;
    }

    // Choose model: police or civilian
    int32 vehicleModel;
    int32 carRating;

    const auto wanted = FindPlayerWanted(-1);
    if (wanted->GetWantedLevel() <= eWantedLevel::WANTED_LEVEL_1
       || NumLawEnforcerCars >= wanted->m_MaxCopCarsInPursuit
       || wanted->m_NumCopsInPursuit >= wanted->m_MaxCopsInPursuit
       || CGame::currArea
       || CGangWars::GangWarFightingGoingOn()
       || (wanted->GetWantedLevel() <= eWantedLevel::WANTED_LEVEL_3
           && (wanted->GetWantedLevel() <= eWantedLevel::WANTED_LEVEL_2
               || CTimer::m_snTimeInMilliseconds <= (uint32)(LastTimeLawEnforcerCreated + 5'000))
           && CTimer::m_snTimeInMilliseconds <= (uint32)(LastTimeLawEnforcerCreated + 8'000))) {
       // Choose civilian model
       vehicleModel = ChooseModel(&carRating);
       if (vehicleModel == -1) {
           return;
       }
       if ((carRating == 13 || carRating == 24) && wanted->GetWantedLevel() >= eWantedLevel::WANTED_LEVEL_1) {
           return;
       }
    } else
    {
        // Choose police model
        carRating    = 13;
        vehicleModel = ChoosePoliceCarModel(0);
    }

    // LA Riots police car override
    if (CGameLogic::LaRiotsActiveHere() && !gbLARiots_NoPoliceCars && (rand() & 0x7F) < 0x37) {
        carRating    = 13;
        vehicleModel = ChoosePoliceCarModel(0);
    }

    // Determine spawn direction based on camera/player vehicle
    float dirX, dirY;
    float preferredDistance;
    bool  generateBehind = false;
    bool  lookingDown    = false;

    if (TheCamera.m_mCameraMatrix.GetForward().z < -0.9f) {
        lookingDown       = true;
        dirY              = 0.707f;
        dirX              = 0.707f;
        preferredDistance = -1.0f;
        generateBehind    = true;
    } else {
        auto* playerVeh = FindPlayerVehicle(-1, false);
        float speed2D   = 0.0f;
        float speedX    = 0.0f;
        float speedY    = 0.0f;

        if (playerVeh) {
            speedX  = playerVeh->m_vecMoveSpeed.x;
            speedY  = playerVeh->m_vecMoveSpeed.y;
            speed2D = std::sqrt(speedY * speedY + speedX * speedX);
        }

        if (playerVeh && speed2D > 0.4f) {
            float invSpeed = 1.0f / speed2D;
            dirX           = speedX * invSpeed;
            dirY           = invSpeed * speedY;

            switch (CTimer::m_FrameCounter & 3) {
            case 0:
            case 1:
                preferredDistance = 0.85f;
                generateBehind    = true;
                break;
            case 2:
                preferredDistance = 0.707f;
                generateBehind    = true;
                break;
            case 3:
                preferredDistance = 0.707f;
                generateBehind    = false;
                break;
            }
        } else if (playerVeh && speed2D > 0.1f) {
            float invSpeed = 1.0f / speed2D;
            dirX           = speedX * invSpeed;
            dirY           = invSpeed * speedY;

            switch (CTimer::m_FrameCounter & 3) {
            case 0:
                preferredDistance = 0.85f;
                generateBehind    = true;
                break;
            case 1:
                preferredDistance = 0.707f;
                generateBehind    = true;
                break;
            case 2:
            case 3:
                preferredDistance = 0.707f;
                generateBehind    = false;
                break;
            }
        } else {
            // Se ejecuta si no hay vehículo o si la velocidad es <= 0.1f
            dirX = TheCamera.m_fCamFrontXNorm;
            dirY = TheCamera.m_fCamFrontYNorm;

            if ((CTimer::m_FrameCounter & 1) == 0) {
                preferredDistance = 0.707f;
                generateBehind    = true;
            } else {
                preferredDistance = 0.707f;
                generateBehind    = false;
            }
        }
    }

    // Try to generate car creation coordinates
    bool  bNotPoliceChasing = carRating != 13 || FindPlayerWanted(-1)->GetWantedLevel() < eWantedLevel::WANTED_LEVEL_1;
    float genDist           = TheCamera.m_fGenerationDistMultiplier * 160.0f;

    CVector      spawnOrigin;
    CNodeAddress fromNode, toNode;
    float        linkFraction; //!< Out param: where along the picked link the spawn point landed, in `[0; 1]`

    if (!GenerateCarCreationCoors2(
            { x, y, z },
            dirX,
            dirY,
            preferredDistance,
            generateBehind,
            genDist,
            38.0f,
            &spawnOrigin,
            &fromNode,
            &toNode,
            &linkFraction,
            bNotPoliceChasing,
            false
        )) {
        return;
    }

    // Get path node data
    CPathNode* toPathNode   = &ThePaths.m_pPathNodes[toNode.m_wAreaId][toNode.m_wNodeId];
    CPathNode* fromPathNode = &ThePaths.m_pPathNodes[fromNode.m_wAreaId][fromNode.m_wNodeId];

    // Check spawn probability - compare spawn chances of both nodes, use smaller
    uint8 toSpawnProb   = toPathNode->m_nSpawnProbability;
    uint8 fromSpawnProb = fromPathNode->m_nSpawnProbability;
    uint8 spawnProb     = (fromSpawnProb >= toSpawnProb) ? toSpawnProb : fromSpawnProb;

    bool isBoatNode     = false;

    if ((rand() & 0xF) > spawnProb) {
        return;
    }

    // Check if water node (boat spawn)
    if (fromPathNode->m_bWaterNode) {
        isBoatNode = true;

        if (carRating == 13) {
            // Police boat
            vehicleModel = MODEL_PREDATOR;
            carRating    = 24;
            if (CStreaming::ms_aInfoForModel[MODEL_PREDATOR].IsLoaded()) {
                // Model loaded, proceed
            } else {
                CStreaming::RequestModel(MODEL_PREDATOR, STREAMING_KEEP_IN_MEMORY);
                return;
            }
        } else {
            // Civilian boat
            vehicleModel = CPopulation::m_LoadedBoats.PickLeastUsedModel(1);
            if (vehicleModel == -1 || !CStreaming::ms_aInfoForModel[vehicleModel].IsLoaded()) {
                return;
            }
        }

        // Larger collision check for boats
        int16 numObjects = 0;
        CWorld::FindObjectsKindaColliding(spawnOrigin, 40.0f, true, &numObjects, 2, nullptr, false, true, true, false, false);
        if (numObjects) {
            return;
        }
    } else {
        // Land vehicle - smaller collision check
        int16 numObjects = 0;
        CWorld::FindObjectsKindaColliding(spawnOrigin, 8.0f, true, &numObjects, 2, nullptr, false, true, true, false, false);
        if (numObjects) {
            return;
        }
    }

    // Find the navi link between the from and to nodes
    //! Left at `m_nNumLinks` when no link matches, exactly as the original does
    int32 linkIdx  = 0;
    int32 numLinks = fromPathNode->m_nNumLinks;
    for (; linkIdx < numLinks; linkIdx++) {
        if (ThePaths.m_pNodeLinks[fromNode.m_wAreaId][linkIdx + fromPathNode->m_wBaseLinkId] == toNode) {
            break;
        }
    }

    CCarPathLinkAddress naviLink = ThePaths.m_pNaviLinks[fromNode.m_wAreaId][linkIdx + fromPathNode->m_wBaseLinkId];
    CCarPathLink&       naviNode = ThePaths.GetCarPathLink(naviLink);

    // Get number of lanes in the appropriate direction. A navi link's lane counts are relative to
    // its own direction, so which of the two fields applies depends on whether the link points at
    // the node we're heading to - same test as in `CPathFind::DoPathSearch` (0x451814).
    int16 numLanes;
    if (naviNode.m_attachedTo == toNode) {
        numLanes = naviNode.m_numOppositeDirLanes;
    } else {
        numLanes = naviNode.m_numSameDirLanes;
    }

    // Check if model is appropriate for the number of lanes
    if (numLanes <= 1) {
        if (vehicleModel == MODEL_COACH) {
            return; // Coach doesn't fit in single lane
        }
        if (vehicleModel == MODEL_BUS) {
            return; // Bus doesn't fit in single lane (checked via BMX type check in decompilation)
        }
    } else {
        // Check for BMX on multi-lane road
        if (CModelInfo::GetModelInfo(vehicleModel)->AsVehicleModelInfoPtr()->m_nVehicleType == VEHICLE_TYPE_BMX) {
            return;
        }
    }

    if (numLanes == 0) {
        return; // No lanes available
    }

    // Zone type restriction check
    if (CPopCycle::m_pCurrZone) {
        auto* zoneInfo = CTheZones::GetZoneInfo(spawnOrigin, nullptr);
        int32 zoneType = zoneInfo->PopType;
        if (zoneType >= 17 && zoneType <= 19) {
            if (zoneType != CPopCycle::m_nCurrentZoneType) {
                return;
            }
            isRestrictedZone = true;
        }
    }

    // Create the vehicle
    auto* generatedVehicle = GetNewVehicleDependingOnCarModel(vehicleModel, eVehicleCreatedBy::RANDOM_VEHICLE);
    if (!generatedVehicle) {
        return;
    }

    // Set up autopilot node addresses
    generatedVehicle->m_autoPilot.m_endingRouteNode.m_wAreaId = (uint16)-1;
    generatedVehicle->m_autoPilot.m_currentAddress            = fromNode; //!< The node the vehicle is driving away from
    generatedVehicle->m_autoPilot.m_startingRouteNode         = toNode;   //!< The node the vehicle is driving towards

    if (carRating == 13) {
       // Police vehicle autopilot setup
       generatedVehicle->m_autoPilot.m_nTempAction = TEMPACT_NONE;
       if (FindPlayerWanted(-1)->GetWantedLevel() > eWantedLevel::WANTED_CLEAN) {
           generatedVehicle->m_autoPilot.SetCruiseSpeed(CCarAI::FindPoliceCarSpeedForWantedLevel(generatedVehicle));
           eCarMission mission;
           if (generatedVehicle->GetVehicleAppearance() == VEHICLE_APPEARANCE_BIKE) {
               mission = CCarAI::FindPoliceBikeMissionForWantedLevel();
           } else {
               mission = CCarAI::FindPoliceCarMissionForWantedLevel();
           }
           generatedVehicle->m_autoPilot.m_nCarMission      = mission;
           generatedVehicle->m_autoPilot.m_nCarDrivingStyle = DRIVING_STYLE_AVOID_CARS;
       } else {
           generatedVehicle->m_autoPilot.SetCruiseSpeed((uint32)CGeneral::GetRandomNumberInRange(18.0f, 24.0f));
           generatedVehicle->m_autoPilot.m_nCarDrivingStyle = DRIVING_STYLE_STOP_FOR_CARS;
           generatedVehicle->m_autoPilot.m_nCarMission      = MISSION_CRUISE;
       }
       if (vehicleModel == MODEL_FBIRANCH) {
           generatedVehicle->m_nPrimaryColor   = 0;
           generatedVehicle->m_nSecondaryColor = 0;
       }
       generatedVehicle->vehicleFlags.bCreatedAsPoliceVehicle = true;
    } else if (carRating == 24) {
        // Police boat autopilot
        generatedVehicle->m_autoPilot.m_nTempAction = TEMPACT_NONE;
        generatedVehicle->m_autoPilot.SetCruiseSpeed((uint32)CGeneral::GetRandomNumberInRange(14.0f, 18.0f));
        generatedVehicle->m_autoPilot.m_nCarDrivingStyle = DRIVING_STYLE_AVOID_CARS;
        generatedVehicle->m_autoPilot.m_nCarMission      = CCarAI::FindPoliceBoatMissionForWantedLevel();
        generatedVehicle->vehicleFlags.bCreatedAsPoliceVehicle = true;
    } else {
        // Civilian vehicle autopilot
        generatedVehicle->m_autoPilot.SetCruiseSpeed((uint32)CGeneral::GetRandomNumberInRange(13.0f, 21.0f));

        if (carRating == 3) {
            // Fast car group
            generatedVehicle->m_autoPilot.SetCruiseSpeed((uint32)CGeneral::GetRandomNumberInRange(18.0f, 27.0f));
        } else if (carRating == 1) {
            // Slow/gang car group
            generatedVehicle->m_autoPilot.SetCruiseSpeed((uint32)CGeneral::GetRandomNumberInRange(10.0f, 15.0f));
        }

        // Slow down big vehicles
        auto* colModel = CModelInfo::GetModelInfo(generatedVehicle->m_nModelIndex)->GetColModel();
        if (colModel->m_boundBox.m_vecMax.y - colModel->m_boundBox.m_vecMin.y > 10.0f || carRating == 5) {
            generatedVehicle->m_autoPilot.m_nCruiseSpeed = generatedVehicle->m_autoPilot.m_nCruiseSpeed * 3 / 4;
        }

        // Boat speed override
        if (isBoatNode) {
            if (generatedVehicle->m_nModelIndex == MODEL_SQUALO
                || generatedVehicle->m_nModelIndex == MODEL_SPEEDER
                || generatedVehicle->m_nModelIndex == MODEL_JETMAX) {
                generatedVehicle->m_autoPilot.SetCruiseSpeed((uint32)CGeneral::GetRandomNumberInRange(25.0f, 35.0f));
            } else {
                generatedVehicle->m_autoPilot.SetCruiseSpeed((uint32)CGeneral::GetRandomNumberInRange(15.0f, 24.0f));
            }
        }

        generatedVehicle->m_autoPilot.m_nCarMission      = MISSION_CRUISE;
        generatedVehicle->m_autoPilot.m_nTempAction      = TEMPACT_NONE;
        generatedVehicle->m_autoPilot.m_nCarDrivingStyle = DRIVING_STYLE_STOP_FOR_CARS;
    }

    // Common post-setup
    if (generatedVehicle->m_nModelIndex == MODEL_MRWHOOP) {
        generatedVehicle->vehicleFlags.bSirenOrAlarm = true; // Let the ice cream van play its jingle
    }

    //! The `fromNode -> toNode` link is the one the vehicle is heading *into*; the link it is
    //! currently on is picked randomly further down.
    generatedVehicle->m_autoPilot.m_nNextPathNodeInfo = naviLink;
    int8 lane                                         = (int8)(rand() % numLanes);
    generatedVehicle->m_autoPilot.m_nCurrentLane      = lane;
    generatedVehicle->m_autoPilot.m_nNextLane         = lane;

    // Mad driver chance
    if (!isBoatNode && carRating != 13 && !isRestrictedZone) {
        int32 madDriverChance;
        if (CGameLogic::LaRiotsActiveHere()) {
            madDriverChance = 80;
        } else {
            eVehicleAppearance appearance = generatedVehicle->GetVehicleAppearance();
            if (appearance == VEHICLE_APPEARANCE_BIKE) {
                madDriverChance = 50;
            } else if (appearance == VEHICLE_APPEARANCE_BOAT) {
                madDriverChance = 10;
            } else {
                madDriverChance = 200;
            }
        }

        if (!CGeneral::GetRandomNumberInRange(0, madDriverChance) || CCheat::m_aCheatsActive[CHEAT_AGGRESSIVE_DRIVERS]) {
            isMadDriver  = true;
            linkFraction = 1.0f;
        }
    }

    // Keep the vehicle's body between the two nodes: clamp how far along the link it may sit
    const auto*   colModel          = CModelInfo::GetModelInfo(generatedVehicle->m_nModelIndex)->GetColModel();
    const float   vehicleHalfLength = (colModel->m_boundBox.m_vecMax.y - colModel->m_boundBox.m_vecMin.y) * 0.5f + 1.0f;
    const CVector fromNodePos       = fromPathNode->GetPosition();
    const CVector toNodePos         = toPathNode->GetPosition();
    const float   linkLength        = (fromNodePos - toNodePos).Magnitude2D();

    if (0.5f * linkLength >= vehicleHalfLength) {
        const float minFraction = vehicleHalfLength / linkLength;
        if (linkFraction <= minFraction) {
            linkFraction = minFraction;
        }
        if (linkFraction >= 1.0f - minFraction) {
            linkFraction = 1.0f - minFraction;
        }
    } else {
        linkFraction = 0.5f; // Link shorter than the vehicle - just sit in the middle of it
    }

    //! A navi link's direction always runs from the lower to the higher node address, so an ordering
    //! comparison of the two ends tells us whether we travel along it or against it. (0x420980)
    generatedVehicle->m_autoPilot._smthNext = (fromNode < toNode) ? -1 : 1;

    // Pick, at random, another link out of fromNode to act as the link the vehicle is currently on
    if (fromPathNode->m_nNumLinks == 1) { // Dead end, nothing to come from
        delete generatedVehicle;
        return;
    }

    int32               currLinkIdx;
    CCarPathLinkAddress currLink;
    do {
        currLinkIdx = rand() % (int32)fromPathNode->m_nNumLinks;
        currLink    = ThePaths.m_pNaviLinks[fromNode.m_wAreaId][fromPathNode->m_wBaseLinkId + currLinkIdx];
    } while (currLink == generatedVehicle->m_autoPilot.m_nNextPathNodeInfo);
    generatedVehicle->m_autoPilot.m_nCurrentPathNodeInfo = currLink;

    if (!ThePaths.m_pPathNodes[currLink.m_wAreaId]) {
        delete generatedVehicle;
        return;
    }
    generatedVehicle->m_autoPilot._smthCurr =
        (ThePaths.m_pNodeLinks[fromNode.m_wAreaId][fromPathNode->m_wBaseLinkId + currLinkIdx] < fromNode) ? -1 : 1;

    // Point the vehicle down the link
    {
        CVector forward = toNodePos - fromNodePos;
        auto&   matrix  = generatedVehicle->GetMatrix();

        CVector2D   forward2D = { forward.x, forward.y };
        const float mag2D     = forward2D.Magnitude();
        if (mag2D == 0.0f) {
            forward2D.x = 1.0f;
        } else {
            forward2D *= 1.0f / mag2D;
        }

        forward.Normalise();
        matrix.GetForward() = forward;
        matrix.GetRight()   = CVector{ forward2D.y, -forward2D.x, 0.0f };
        matrix.GetUp()      = CVector{ 0.0f, 0.0f, 1.0f };
    }

    //! 2D distance between a path node and a navi link's position - all of the blending below only
    //! looks at X/Y, the navi link Z is never touched.
    const auto DistToNaviLink = [](CNodeAddress node, CCarPathLinkAddress link) {
        const CVector2D nodePos = ThePaths.m_pPathNodes[node.m_wAreaId][node.m_wNodeId].GetPosition();
        return (CVector2D{ ThePaths.GetCarPathLink(link).GetNodeCoors() } - nodePos).Magnitude();
    };
    const auto DistToPoint = [](CNodeAddress node, const CVector& point) {
        const CVector2D nodePos = ThePaths.m_pPathNodes[node.m_wAreaId][node.m_wNodeId].GetPosition();
        return (CVector2D{ point.x, point.y } - nodePos).Magnitude();
    };

    // Turn the fraction along the link into a parameter along the curve the vehicle will drive
    const auto& ap = generatedVehicle->m_autoPilot;

    const float distToNextLink = DistToNaviLink(ap.m_currentAddress, ap.m_nNextPathNodeInfo);
    float       curveParam;

    if (distToNextLink / (DistToNaviLink(ap.m_startingRouteNode, ap.m_nNextPathNodeInfo) + distToNextLink) <= linkFraction) {
        // Already past the next link's position - move the whole route one link forward
        PickNextNodeRandomly(generatedVehicle);

        const float distToCurrLink = DistToNaviLink(ap.m_currentAddress, ap.m_nCurrentPathNodeInfo);
        curveParam                 = (distToCurrLink - DistToPoint(ap.m_currentAddress, spawnOrigin))
                   / (DistToNaviLink(ap.m_currentAddress, ap.m_nNextPathNodeInfo) + distToCurrLink);
    } else {
        const float distToCurrLink = DistToNaviLink(ap.m_currentAddress, ap.m_nCurrentPathNodeInfo);
        curveParam                 = (DistToPoint(ap.m_currentAddress, spawnOrigin) + distToCurrLink)
                   / (distToCurrLink + distToNextLink);
    }
    curveParam = std::clamp(curveParam, 0.0f, 1.0f);

    // Offset both ends of the curve into the vehicle's lane
    const CCarPathLink& currNaviNode = ThePaths.GetCarPathLink(ap.m_nCurrentPathNodeInfo);
    const CCarPathLink& nextNaviNode = ThePaths.GetCarPathLink(ap.m_nNextPathNodeInfo);

    const CVector2D currDir = CVector2D{ currNaviNode.m_dir } * (float)ap._smthCurr;
    const CVector2D nextDir = CVector2D{ nextNaviNode.m_dir } * (float)ap._smthNext;

    float currLaneOffset = (currNaviNode.OneWayLaneOffset() + (float)ap.m_nCurrentLane) * 5.4f;
    float nextLaneOffset = (nextNaviNode.OneWayLaneOffset() + (float)ap.m_nNextLane) * 5.4f;
    if (generatedVehicle->m_nVehicleType == VEHICLE_TYPE_BMX) {
        currLaneOffset += 1.458f;
        nextLaneOffset += 1.458f;
    }

    // Cruise speed scaled by how fast traffic is allowed to go on the node we're driving towards
    const auto& speedNode                    = ThePaths.m_pPathNodes[ap.m_startingRouteNode.m_wAreaId][ap.m_startingRouteNode.m_wNodeId];
    generatedVehicle->m_autoPilot.field_41    = (int8)(speedNode.m_bNotHighway | (speedNode.m_bHighway << 1));
    generatedVehicle->m_autoPilot.m_SpeedMult = FindSpeedMultiplierWithSpeedFromNodes(generatedVehicle->m_autoPilot.field_41);
    generatedVehicle->m_autoPilot.m_speed     = (float)ap.m_nCruiseSpeed * ap.m_SpeedMult;

    const CVector2D currNaviPos = currNaviNode.GetNodeCoors();
    const CVector2D nextNaviPos = nextNaviNode.GetNodeCoors();

    const CVector currLaneCoors{ currNaviPos.x + currLaneOffset * currDir.y, currNaviPos.y - currLaneOffset * currDir.x, 0.0f };
    const CVector nextLaneCoors{ nextNaviPos.x + nextLaneOffset * nextDir.y, nextNaviPos.y - nextLaneOffset * nextDir.x, 0.0f };

    // How long the vehicle needs to drive from one end of the curve to the other
    const int32 timeToNextLink =
        (int32)(CCurves::CalcSpeedScaleFactor(currLaneCoors, nextLaneCoors, currDir.x, currDir.y, nextDir.x, nextDir.y)
                * (1000.0f / ap.m_speed));
    generatedVehicle->m_autoPilot.m_nSpeedScaleFactor = timeToNextLink; // Really the traversal time in ms

    // Back-date when it entered the curve so that it is `curveParam` along it right now
    const int32 timeToLeaveLink                = (int32)((float)CTimer::m_snTimeInMilliseconds - curveParam * (float)timeToNextLink);
    generatedVehicle->m_autoPilot.field_C      = timeToLeaveLink;

    CVector resultPos, resultSpeed;
    CCurves::CalcCurvePoint(
        currLaneCoors,
        nextLaneCoors,
        CVector{ currDir.x, currDir.y, 0.0f },
        CVector{ nextDir.x, nextDir.y, 0.0f },
        (float)(CTimer::m_snTimeInMilliseconds - (uint32)timeToLeaveLink) / (float)timeToNextLink,
        timeToNextLink,
        resultPos,
        resultSpeed
    );

    // Nudge the spawn point 2m back along the link, and interpolate its height between the two nodes
    const CVector backAlongLink = fromNodePos - toNodePos;

    CVector posn = resultPos + backAlongLink * (2.0f / backAlongLink.Magnitude());
    posn.z       = linkFraction * toNodePos.z + (1.0f - linkFraction) * fromNodePos.z;

    // Find ground height
    float groundZ = 1000000000.0f;

    if (isBoatNode) {
        float waterLevel;
        if (!CWaterLevel::GetWaterLevel(posn, waterLevel, true, nullptr)) {
            delete generatedVehicle;
            return;
        }
        groundZ = waterLevel;
    } else {
        CColPoint colPoint;
        CEntity*  colEntity;
        if (CWorld::ProcessVerticalLine(posn, 1000.0f, colPoint, colEntity, true, false, false, false, true, false, nullptr)) {
            groundZ = colPoint.m_vecPoint.z;
        }
        if (CWorld::ProcessVerticalLine(posn, -1000.0f, colPoint, colEntity, true, false, false, false, true, false, nullptr)) {
            if (std::fabs(colPoint.m_vecPoint.z - posn.z) < std::fabs(groundZ - posn.z)) {
                groundZ = colPoint.m_vecPoint.z;
            }
        }
    }

    if (groundZ == 1000000000.0f) {
        delete generatedVehicle;
        return;
    }

    if (std::fabs(groundZ - posn.z) > 7.0f) {
        delete generatedVehicle;
        return;
    }

    // Set vehicle height
    if (CModelInfo::IsBoatModel(generatedVehicle->m_nModelIndex)) {
        posn.z                                    = groundZ;
        generatedVehicle->m_nExtendedRemovalRange = 255;
    } else {
        posn.z = groundZ + generatedVehicle->GetHeightAboveRoad();
    }

    generatedVehicle->SetPosn(posn);
    generatedVehicle->m_vecMoveSpeed = CVector(0.0f, 0.0f, 0.0f);

    // Velocity relative to the player, and where the vehicle sits relative to them - used at the
    // very end to reject vehicles that would drive away from the player instead of towards them.
    const CVector dirVec = resultSpeed / 60.0f - playerSpeed;
    CVector       movementDir;
    movementDir.x = resultPos.x - playerPos.x;
    movementDir.y = resultPos.y - playerPos.y;

    // Set entity status
    if (carRating == 13) {
        if (generatedVehicle->m_autoPilot.m_nCarMission == MISSION_CRUISE) {
            generatedVehicle->SetStatus(STATUS_SIMPLE);
        } else {
            generatedVehicle->SetStatus(STATUS_PHYSICS);
        }
    } else if (carRating == 24) {
        generatedVehicle->SetStatus(STATUS_PHYSICS);
    } else {
        if (isBoatNode) {
            generatedVehicle->SetStatus(STATUS_PHYSICS);
        } else if (generatedVehicle->GetStatus() != STATUS_PHYSICS) {
            generatedVehicle->SetStatus(STATUS_SIMPLE);
        }
    }

    // Set initial alpha to 0 for fade-in
    CVisibilityPlugins::SetClumpAlpha(generatedVehicle->GetRpClump(), 0);

    // Funhouse cheat - add hydraulics
    if (CCheat::m_aCheatsActive[CHEAT_FUNHOUSE_THEME]
        && generatedVehicle->m_nVehicleType == VEHICLE_TYPE_AUTOMOBILE) {
        generatedVehicle->AddVehicleUpgrade(ModelIndices::MI_HYDRAULICS);
    }

    // Distance / visibility checks
    bool    isOnScreen = generatedVehicle->GetIsOnScreen();
    CVector vehPos     = generatedVehicle->GetPosition();

    if (!isOnScreen) {
        // Off-screen: check max removal distance
        float dist2D  = (playerPos - vehPos).Magnitude2D();
        float maxDist = std::max((float)generatedVehicle->m_nExtendedRemovalRange, 170.0f) * (1.0f / 170.0f) * 45.0f;
        if (maxDist < dist2D && !lookingDown) {
            delete generatedVehicle;
            return;
        }
    } else {
        // On-screen: must be within valid generation distance range
        float dist2DFromPlayer = (playerPos - vehPos).Magnitude2D();
        float maxRemoval       = std::max(170.0f, (float)generatedVehicle->m_nExtendedRemovalRange);
        if (maxRemoval * TheCamera.m_fGenerationDistMultiplier < dist2DFromPlayer
            || TheCamera.m_fGenerationDistMultiplier * 150.0f > dist2DFromPlayer) {
            delete generatedVehicle;
            return;
        }

        // Check distance from camera
        CVector camPos        = TheCamera.GetPosition();
        float   dist2DFromCam = (camPos - vehPos).Magnitude2D();
        if (TheCamera.m_fGenerationDistMultiplier * 120.0f > dist2DFromCam || lookingDown) {
            // too close to camera but keep going to LABEL_253 (delete)
            delete generatedVehicle;
            return;
        }

        // Don't spawn MARQUIS on screen
        if (generatedVehicle->m_nModelIndex == MODEL_MARQUIS) {
            delete generatedVehicle;
            return;
        }
    }

    // Final collision check at spawn position
    float colRadius       = CModelInfo::GetModelInfo(generatedVehicle->m_nModelIndex)->GetColModel()->m_boundSphere.m_fRadius;
    int16 numObjectsFound = 0;
    CWorld::FindObjectsKindaColliding(vehPos, colRadius, true, &numObjectsFound, 2, nullptr, false, true, true, false, false);

    // Check vehicle is facing away from player
    if (numObjectsFound || (dirVec.x * movementDir.x + dirVec.y * movementDir.y) >= 0.0f) {
        delete generatedVehicle;
        return;
    }

    // Choose vehicle colour
    CModelInfo::GetModelInfo(generatedVehicle->m_nModelIndex)->AsVehicleModelInfoPtr()->ChooseVehicleColour(generatedVehicle->m_nPrimaryColor, generatedVehicle->m_nSecondaryColor, generatedVehicle->m_nTertiaryColor, generatedVehicle->m_nQuaternaryColor, 1);

    // Add to world
    CWorld::Add(generatedVehicle);

    // Slow down tractor and combine
    if (generatedVehicle->m_nModelIndex == MODEL_TRACTOR
        || generatedVehicle->m_nModelIndex == MODEL_COMBINE
        || generatedVehicle->m_nVehicleType == VEHICLE_TYPE_BMX) {
        generatedVehicle->m_autoPilot.m_nCruiseSpeed /= 3;
    }

    // Beaten up cars during the riots
    if (CGameLogic::LaRiotsActiveHere()) {
        generatedVehicle->m_fHealth = (float)(rand() % 1'000);
    }

    if (carRating == 13) {
        LastTimeLawEnforcerCreated = CTimer::m_snTimeInMilliseconds;
    }

    if (generatedVehicle->m_nModelIndex == MODEL_CADDY) {
        generatedVehicle->SetStatus(STATUS_PHYSICS);
        generatedVehicle->m_autoPilot.m_nCarDrivingStyle = DRIVING_STYLE_AVOID_CARS;
    }

    // Some car groups spawn pre-damaged
    if (generatedVehicle->IsAutomobile()) {
        switch (carRating) {
        case 0:
        case 4:
        case 5:
        case 6:
        case 13:
            if (!CGeneral::GetRandomNumberInRange(0, 20)) {
                generatedVehicle->AsAutomobile()->SetRandomDamage(false);
            }
            break;
        case 1:
        case 14:
        case 15:
        case 16:
        case 17:
        case 18:
        case 19:
        case 20:
        case 21:
        case 22:
        case 23:
            if (!CGeneral::GetRandomNumberInRange(0, 8)) {
                generatedVehicle->AsAutomobile()->SetRandomDamage(true);
            }
            break;
        default:
            break;
        }
    }

    // Bikes in stop-for-cars mode should use physics
    if (generatedVehicle->m_nVehicleType == VEHICLE_TYPE_BIKE
        && generatedVehicle->m_autoPilot.m_nCarDrivingStyle == DRIVING_STYLE_STOP_FOR_CARS) {
        generatedVehicle->SetStatus(STATUS_PHYSICS);
        generatedVehicle->m_autoPilot.m_nCarDrivingStyle = DRIVING_STYLE_DRIVINGMODE_AVOIDCARS_STOPFORPEDS_OBEYLIGHTS;
    }

    // Handle police chase / mad driver / normal driver setup
    if (!isBoatNode
        && carRating != 13
        && FindPlayerPed(-1)->GetWantedLevel() == eWantedLevel::WANTED_CLEAN
        && (CCheat::m_aCheatsActive[CHEAT_AGGRESSIVE_DRIVERS] || TimeNextMadDriverChaseCreated <= 0.0f)
        && !isRestrictedZone
        && CreatePoliceChase(generatedVehicle, carRating, fromNode)) {
        // Police chase was created
        if (CGameLogic::LaRiotsActiveHere()) {
            TimeNextMadDriverChaseCreated = CGeneral::GetRandomNumberInRange(240.0f, 480.0f);
        } else {
            TimeNextMadDriverChaseCreated = CGeneral::GetRandomNumberInRange(600.0f, 1200.0f);
        }
    } else if (isMadDriver) {
        // Mad driver setup
        auto modelId = generatedVehicle->m_nModelIndex;
        if ((modelId == MODEL_FREEWAY || modelId == MODEL_PCJ600 || modelId == MODEL_FCR900
             || modelId == MODEL_NRG500 || modelId == MODEL_BF400 || modelId == MODEL_WAYFARER)
            && !gbLARiots
            && !CGeneral::GetRandomNumberInRange(0, 7)
            && CreateConvoy(generatedVehicle, carRating)) {
            SetUpDriverAndPassengersForVehicle(generatedVehicle, carRating, 1, true, false, 99);
        } else {
            SetUpDriverAndPassengersForVehicle(generatedVehicle, carRating, 1, true, false, 99);
            generatedVehicle->SetStatus(STATUS_PHYSICS);
            generatedVehicle->m_autoPilot.m_nCarDrivingStyle = DRIVING_STYLE_AVOID_CARS;
            float newSpeed                                   = (float)generatedVehicle->m_autoPilot.m_nCruiseSpeed + 10.0f;
            generatedVehicle->m_autoPilot.SetCruiseSpeed((uint32)newSpeed);

            CVector forward                  = generatedVehicle->GetForward();
            generatedVehicle->m_vecMoveSpeed = forward * newSpeed * 0.02f;

            if (CGameLogic::LaRiotsActiveHere() || CCheat::m_aCheatsActive[CHEAT_AGGRESSIVE_DRIVERS]) {
                if (generatedVehicle->m_pDriver) {
                    generatedVehicle->m_pDriver->bNeverEverTargetThisPed = true;
                }
            }
            generatedVehicle->vehicleFlags.bMadDriver = true;
        }
    } else if (carRating == 13 || carRating == 24) {
        // Police occupants
        CCarAI::AddPoliceCarOccupants(generatedVehicle, false);
    } else {
        // Normal civilian occupants
        bCarIsBeingCreated = true;
        SetUpDriverAndPassengersForVehicle(generatedVehicle, carRating, 0, false, false, 99);
        bCarIsBeingCreated = false;
    }

    // Mark law enforcer vehicles
    if (carRating == 13 || carRating == 24) {
        generatedVehicle->ChangeLawEnforcerState(true);
    }

    CStreaming::PossiblyStreamCarOutAfterCreation(generatedVehicle->m_nModelIndex);
    CModelInfo::GetModelInfo(generatedVehicle->m_nModelIndex)->AddRef();
}

void CCarCtrl::GenerateRandomCars() {
    if (CCutsceneMgr::ms_running) {
        CountDownToCarsAtStart = 2;
        return;
    }
    if (CGangWars::DontCreateCivilians() || !CGame::CanSeeOutSideFromCurrArea()) {
        return;
    }

    if (CGameLogic::LaRiotsActiveHere() && TimeNextMadDriverChaseCreated > 480.0f) {
        TimeNextMadDriverChaseCreated = CGeneral::GetRandomNumberInRange(240.0f, 480.0f);
    }
    TimeNextMadDriverChaseCreated -= (CTimer::GetTimeStep() * 0.02f);

    if (NumRandomCars < 45) {
        if (CountDownToCarsAtStart) {
            CountDownToCarsAtStart--;
            for (auto i = 100; i-- > 0;) {
                GenerateOneRandomCar();
            }
            CTheCarGenerators::GenerateEvenIfPlayerIsCloseCounter = 20;
        } else {
            GenerateOneRandomCar();
            GenerateOneRandomCar();
        }
    }
}

// 0x42F3C0
void CCarCtrl::GetAIHeliToAttackPlayer(CAutomobile* automobile) {
    plugin::Call<0x42F3C0, CAutomobile*>(automobile);
}

// 0x42A730
void CCarCtrl::GetAIHeliToFlyInDirection(CAutomobile* automobile) {
    plugin::Call<0x42A730, CAutomobile*>(automobile);
}

// 0x429780
void CCarCtrl::GetAIPlaneToAttackPlayer(CAutomobile* automobile) {
    plugin::Call<0x429780, CAutomobile*>(automobile);
}

// 0x429890
void CCarCtrl::GetAIPlaneToDoDogFight(CAutomobile* automobile) {
    plugin::Call<0x429890, CAutomobile*>(automobile);
}

// 0x42F370
void CCarCtrl::GetAIPlaneToDoDogFightAgainstPlayer(CAutomobile* automobile) {
    plugin::Call<0x42F370, CAutomobile*>(automobile);
}

// 0x421440
CVehicle* CCarCtrl::GetNewVehicleDependingOnCarModel(int32 modelId, eVehicleCreatedBy createdBy) {
    switch (CModelInfo::GetModelInfo(modelId)->AsVehicleModelInfoPtr()->m_nVehicleType) {
    case VEHICLE_TYPE_MTRUCK:
        return new CMonsterTruck(modelId, createdBy);
    case VEHICLE_TYPE_QUAD:
        return new CQuadBike(modelId, createdBy);
    case VEHICLE_TYPE_HELI:
        return new CHeli(modelId, createdBy);
    case VEHICLE_TYPE_PLANE:
        return new CPlane(modelId, createdBy);
    case VEHICLE_TYPE_BOAT:
        return new CBoat(modelId, createdBy);
    case VEHICLE_TYPE_TRAIN:
        return new CTrain(modelId, createdBy);
    case VEHICLE_TYPE_BIKE:
        return new CBike(modelId, createdBy);
    case VEHICLE_TYPE_BMX:
        return new CBmx(modelId, createdBy);
    case VEHICLE_TYPE_TRAILER:
        return new CTrailer(modelId, createdBy);
    case VEHICLE_TYPE_AUTOMOBILE:
        return new CAutomobile(modelId, createdBy, 1);
    }
    return nullptr;
}

// 0x42C250
bool CCarCtrl::IsAnyoneParking() {
    for (auto& veh : GetVehiclePool()->GetAllValid()) {
        switch (veh.m_autoPilot.m_nCarMission) {
        case eCarMission::MISSION_PARK_PARALLEL:
        case eCarMission::MISSION_PARK_PARALLEL_2:
        case eCarMission::MISSION_PARK_PERPENDICULAR:
        case eCarMission::MISSION_PARK_PERPENDICULAR_2:
            return true;
        }
    }
    return false;
}

// 0x42DAB0
bool CCarCtrl::IsThisAnAppropriateNode(CVehicle* vehicle, CNodeAddress nodeAddress1, CNodeAddress nodeAddress2, CNodeAddress nodeAddress3, bool arg5) {
    return plugin::CallAndReturn<bool, 0x42DAB0, CVehicle*, CNodeAddress, CNodeAddress, CNodeAddress, bool>(vehicle, nodeAddress1, nodeAddress2, nodeAddress3, arg5);
}

// 0x423EA0
bool CCarCtrl::IsThisVehicleInteresting(CVehicle* vehicle) {
    for (auto& car : apCarsToKeep) {
        if (car == vehicle) {
            return true;
        }
    }
    return false;
}

// 0x432CB0
void CCarCtrl::JoinCarWithRoadAccordingToMission(CVehicle* vehicle) {
    switch (vehicle->m_autoPilot.m_nCarMission) {
    case MISSION_NONE:
    case MISSION_CRUISE:
    case MISSION_WAITFORDELETION:
    case MISSION_EMERGENCYVEHICLE_STOP:
    case MISSION_STOP_FOREVER:
    case MISSION_FOLLOW_RECORDED_PATH:
    case MISSION_PARK_PERPENDICULAR:
    case MISSION_PARK_PARALLEL:
    case MISSION_PARK_PERPENDICULAR_2:
    case MISSION_PARK_PARALLEL_2:
        return JoinCarWithRoadSystem(vehicle);
    case MISSION_RAMPLAYER_FARAWAY:
    case MISSION_RAMPLAYER_CLOSE:
    case MISSION_BLOCKPLAYER_FARAWAY:
    case MISSION_BLOCKPLAYER_CLOSE:
    case MISSION_BLOCKPLAYER_HANDBRAKESTOP:
    case MISSION_BOAT_ATTACKPLAYER:
    case MISSION_SLOWLY_DRIVE_TOWARDS_PLAYER_1:
    case MISSION_SLOWLY_DRIVE_TOWARDS_PLAYER_2:
    case MISSION_BLOCKPLAYER_FORWARDANDBACK:
    case MISSION_APPROACHPLAYER_FARAWAY:
    case MISSION_APPROACHPLAYER_CLOSE:
    case MISSION_BOAT_CIRCLEPLAYER:             {
        JoinCarWithRoadSystemGotoCoors(vehicle, FindPlayerCoors(-1), true, vehicle->IsSubBoat());
        break;
    }
    case MISSION_GOTOCOORDINATES:
    case MISSION_GOTOCOORDINATES_STRAIGHTLINE:
    case MISSION_GOTOCOORDINATES_ACCURATE:
    case MISSION_GOTOCOORDINATES_STRAIGHTLINE_ACCURATE:
    case MISSION_GOTOCOORDINATES_ASTHECROWSWIMS:
    case MISSION_GOTOCOORDINATES_RACING:                {
        JoinCarWithRoadSystemGotoCoors(vehicle, vehicle->m_autoPilot.m_vecDestinationCoors, true, vehicle->IsSubBoat());
        break;
    }
    case MISSION_RAMCAR_FARAWAY:
    case MISSION_RAMCAR_CLOSE:
    case MISSION_BLOCKCAR_FARAWAY:
    case MISSION_BLOCKCAR_CLOSE:
    case MISSION_BLOCKCAR_HANDBRAKESTOP:
    case MISSION_PROTECTION_REAR:
    case MISSION_PROTECTION_FRONT:
    case MISSION_ESCORT_LEFT:
    case MISSION_ESCORT_RIGHT:
    case MISSION_ESCORT_REAR:
    case MISSION_ESCORT_FRONT:
    case MISSION_FOLLOWCAR_FARAWAY:
    case MISSION_FOLLOWCAR_CLOSE:
    case MISSION_KILLPED_FARAWAY:
    case MISSION_KILLPED_CLOSE:
    case MISSION_DO_DRIVEBY_CLOSE:
    case MISSION_DO_DRIVEBY_FARAWAY:
    case MISSION_ESCORT_LEFT_FARAWAY:
    case MISSION_ESCORT_RIGHT_FARAWAY:
    case MISSION_ESCORT_REAR_FARAWAY:
    case MISSION_ESCORT_FRONT_FARAWAY:   {
        JoinCarWithRoadSystemGotoCoors(vehicle, vehicle->m_autoPilot.m_TargetEntity->GetPosition(), true, vehicle->IsSubBoat());
        break;
    }
    }
}

// 0x42F5A0
void CCarCtrl::JoinCarWithRoadSystem(CVehicle* vehicle) {
    plugin::Call<0x42F5A0, CVehicle*>(vehicle);
}

// 0x42F870
bool CCarCtrl::JoinCarWithRoadSystemGotoCoors(CVehicle* vehicle, const CVector& posn, bool unused, bool bIsBoat) {
    return plugin::CallAndReturn<bool, 0x42F870, CVehicle*, const CVector&, bool, bool>(vehicle, posn, unused, bIsBoat);
}

// 0x432B10
bool CCarCtrl::PickNextNodeAccordingStrategy(CVehicle* vehicle) {
    return plugin::CallAndReturn<bool, 0x432B10, CVehicle*>(vehicle);
}

// 0x421740
void CCarCtrl::InitSequence(int32 numSequenceElements) {
    SequenceElements     = numSequenceElements;
    SequenceRandomOffset = CGeneral::GetRandomNumber() % numSequenceElements;
    bSequenceOtherWay    = (CGeneral::GetRandomNumber() / 4) % 2;
}

// 0x42DE80
void CCarCtrl::PickNextNodeRandomly(CVehicle* vehicle) {
    plugin::Call<0x42DE80, CVehicle*>(vehicle);
}

// 0x426EF0
bool CCarCtrl::PickNextNodeToChaseCar(CVehicle* vehicle, float destX, float destY, float destZ) {
    return plugin::CallAndReturn<bool, 0x426EF0, CVehicle*, float, float, float>(vehicle, destX, destY, destZ);
}

// 0x427740
bool CCarCtrl::PickNextNodeToFollowPath(CVehicle* vehicle) {
    return plugin::CallAndReturn<bool, 0x427740, CVehicle*>(vehicle);
}

// 0x429600
void CCarCtrl::PossiblyFireHSMissile(CVehicle* entityLauncher, CEntity* targetEntity) {
    if (!targetEntity) {
        return;
    }

    if (CTimer::GetTimeInMS() / 2'000u == CTimer::GetPreviousTimeInMS() / 2'000u) {
        return;
    }

    const CVector launcherPos = entityLauncher->GetPosition();
    const CVector targetPos   = targetEntity->GetPosition();
    CVector       dir         = targetPos - launcherPos;
    const float   dist        = dir.Magnitude();
    if (dist < 160.0f && dist > 30.0f) {
        CMatrix launcherMat   = entityLauncher->GetMatrix();
        CVector dirNormalized = dir;
        dir.Normalise();
        if (DotProduct(launcherMat.GetForward(), dirNormalized) > 0.8f) {
            CProjectileInfo::AddProjectile(
                entityLauncher,
                eWeaponType::WEAPON_ROCKET_HS,
                launcherPos + launcherMat.GetForward() * 4.0f - launcherMat.GetUp() * 3.0f,
                1.0f,
                &entityLauncher->GetMatrix().GetForward(),
                targetEntity
            );
        }
    }
}

// 0x424F80
void CCarCtrl::PossiblyRemoveVehicle(CVehicle* vehicle) {
    plugin::Call<0x424F80, CVehicle*>(vehicle);
}

// 0x423F10
void CCarCtrl::PruneVehiclesOfInterest() {
    ZoneScoped;

    if ((CTimer::GetFrameCounter() % 64) == 19 && FindPlayerCoors(-1).z < 950.0f) {
        for (size_t i = 0; i < std::size(apCarsToKeep); i++) {
            if (apCarsToKeep[i]) {
                if (CTimer::GetTimeInMS() > aCarsToKeepTime[i] + 180'000) {
                    apCarsToKeep[i] = nullptr;
                }
            }
        }
    }
}

// 0x42FC40
void CCarCtrl::ReconsiderRoute(CVehicle* vehicle) {
    plugin::Call<0x42FC40, CVehicle*>(vehicle);
}

// 0x423DE0
void CCarCtrl::RegisterVehicleOfInterest(CVehicle* vehicle) {
    plugin::Call<0x423DE0, CVehicle*>(vehicle);
}

// 0x4322B0
void CCarCtrl::RemoveCarsIfThePoolGetsFull() {
    ZoneScoped;

    if (CTimer::GetFrameCounter() % 8 != 3) {
        return;
    }

    if (GetVehiclePool()->GetNoOfFreeSpaces() >= 8) {
        return;
    }

    // Find closest deletable vehicle
    const CVector camPos       = TheCamera.GetPosition();
    float         fClosestDist = std::numeric_limits<float>::max();
    CVehicle*     closestVeh   = nullptr;
    for (auto& veh : GetVehiclePool()->GetAllValid()) {
        if (IsThisVehicleInteresting(&veh)) {
            continue;
        }
        if (veh.vehicleFlags.bIsLocked) {
            continue;
        }
        if (!veh.CanBeDeleted()) {
            continue;
        }
        if (CCranes::IsThisCarBeingTargettedByAnyCrane(&veh)) {
            continue;
        }

        const float fCamVehDist = (camPos - veh.GetPosition()).Magnitude();
        if (fClosestDist > fCamVehDist) {
            fClosestDist = fCamVehDist;
            closestVeh   = &veh;
        }
    }
    if (closestVeh) {
        CWorld::Remove(closestVeh);
        delete closestVeh;
    }
}

// 0x42CD10
void CCarCtrl::RemoveDistantCars() {
    ZoneScoped;

    // FIXBUGS: First remove vehicles that can be removed
    if (notsa::bugfixes::CCarCtrl_RemoveDistantCars_UseAfterFree) {
        for (auto& veh : GetVehiclePool()->GetAllValid()) {
            PossiblyRemoveVehicle(&veh);
        }
    }

    //... only then process them, this way we don't do use-after-free
    // only other solution would be `PossiblyRemoveVehicle` returning a `bool`
    // to indicate whenever the vehicle was deleted or not.
    for (auto& veh : GetVehiclePool()->GetAllValid()) {
        if (!notsa::bugfixes::CCarCtrl_RemoveDistantCars_UseAfterFree) {
            PossiblyRemoveVehicle(&veh); // This may or may not invalidate `veh`
        }
        if (!veh.vehicleFlags.bCreateRoadBlockPeds) {
            continue;
        }
        if (DistanceBetweenPoints(FindPlayerCentreOfWorld(), veh.GetPosition()) >= 54.5f) {
            continue;
        }
        CRoadBlocks::GenerateRoadBlockPedsForCar(
            &veh,
            veh.m_nPedsPositionForRoadBlock,
            veh.IsLawEnforcementVehicle() ? PED_TYPE_COP : PED_TYPE_GANG1
        );
        veh.vehicleFlags.bCreateRoadBlockPeds = false;
    }
}

// 0x423ED0
void CCarCtrl::RemoveFromInterestingVehicleList(CVehicle* vehicle) {
    for (auto& car : apCarsToKeep) {
        if (car == vehicle) {
            car = nullptr;
            break;
        }
    }
}

// 0x42CE40
void CCarCtrl::ScanForPedDanger(CVehicle* vehicle) {
    plugin::Call<0x42CE40, CVehicle*>(vehicle);
}

// 0x42FBC0
bool CCarCtrl::ScriptGenerateOneEmergencyServicesCar(uint32 modelId, CVector posn) {
    if (CStreaming::IsModelLoaded(modelId)) {
        if (auto pAuto = GenerateOneEmergencyServicesCar(modelId, posn)) {
            pAuto->m_autoPilot.m_vecDestinationCoors = posn;
            pAuto->m_autoPilot.SetCarMission(JoinCarWithRoadSystemGotoCoors(pAuto, posn, false, false) ? MISSION_GOTOCOORDINATES_STRAIGHTLINE : MISSION_GOTOCOORDINATES);
            return true;
        }
    }
    return false;
}

// 0x4342A0
void CCarCtrl::SetCoordsOfScriptCar(CVehicle* vehicle, float x, float y, float z, uint8 arg5, uint8 arg6) {
    plugin::Call<0x4342A0, CVehicle*, float, float, float, uint8, uint8>(vehicle, x, y, z, arg5, arg6);
}

// 0x4217C0
void CCarCtrl::SetUpDriverAndPassengersForVehicle(CVehicle* vehicle, int32 arg2, int32 arg3, bool arg4, bool arg5, int32 passengersNum) {
    plugin::Call<0x4217C0, CVehicle*, int32, int32, bool, bool, int32>(vehicle, arg2, arg3, arg4, arg5, passengersNum);
}

// 0x432420
template<typename PtrListType>
void CCarCtrl::SlowCarDownForCarsSectorList(PtrListType& ptrList, CVehicle* vehicle, float arg3, float arg4, float arg5, float arg6, float* arg7, float arg8) {
    plugin::Call<0x432420, PtrListType&, CVehicle*, float, float, float, float, float*, float>(ptrList, vehicle, arg3, arg4, arg5, arg6, arg7, arg8);
}

// 0x426220
void CCarCtrl::SlowCarDownForObject(CEntity* entity, CVehicle* vehicle, float* arg3, float arg4) {
    const CVector entityDir     = entity->GetPosition() - vehicle->GetPosition();
    const float   entityHeading = DotProduct(entityDir, vehicle->GetMatrix().GetForward());
    if (entityHeading > 0.0f && entityHeading < 20.0f) {
        if (entity->GetColModel()->GetBoundRadius() + vehicle->GetColModel()->GetBoundingBox().m_vecMax.x > fabs(DotProduct(entityDir, vehicle->GetMatrix().GetRight()))) {
            if (entityHeading >= 7.0f) {
                *arg3 = std::min(*arg3, (1.0f - (entityHeading - 7.0f) / 13.0f)) * arg4; // Original code multiplies by 0.07692308, which is the recp. of 13
            } else {
                *arg3 = 0.0f;
            }
        }
    }
}

// 0x42D4F0
template<typename PtrListType>
void CCarCtrl::SlowCarDownForObjectsSectorList(PtrListType& ptrList, CVehicle* vehicle, float arg3, float arg4, float arg5, float arg6, float* arg7, float arg8) {
    plugin::Call<0x42D4F0, PtrListType&, CVehicle*, float, float, float, float, float*, float>(ptrList, vehicle, arg3, arg4, arg5, arg6, arg7, arg8);
}

// 0x42D0E0
void CCarCtrl::SlowCarDownForOtherCar(CEntity* car1, CVehicle* car2, float* arg3, float arg4) {
    plugin::Call<0x42D0E0, CEntity*, CVehicle*, float*, float>(car1, car2, arg3, arg4);
}

// 0x425440
template<typename PtrListType>
void CCarCtrl::SlowCarDownForPedsSectorList(PtrListType& PtrList, CVehicle* vehicle, float arg3, float arg4, float arg5, float arg6, float* arg7, float arg8) {
    plugin::Call<0x425440, PtrListType&, CVehicle*, float, float, float, float, float*, float>(PtrList, vehicle, arg3, arg4, arg5, arg6, arg7, arg8);
}

// 0x434790
void CCarCtrl::SlowCarOnRailsDownForTrafficAndLights(CVehicle* vehicle) {
    auto& autoPilot = vehicle->m_autoPilot;

    if ((((int8)CTimer::GetFrameCounter() + (int8)(vehicle->m_nRandomSeed)) & 3) == 0) {
        if (CTrafficLights::ShouldCarStopForLight(vehicle, false) || CTrafficLights::ShouldCarStopForBridge(vehicle)) {
            CCarAI::CarHasReasonToStop(vehicle);
            autoPilot.m_fMaxTrafficSpeed = 0.0f;
        } else {
            autoPilot.m_fMaxTrafficSpeed = FindMaximumSpeedForThisCarInTraffic(vehicle);
        }
    }

    if (autoPilot.m_fMaxTrafficSpeed >= autoPilot.m_speed) {
        autoPilot.ModifySpeed(std::min(autoPilot.m_fMaxTrafficSpeed, CTimer::GetTimeStep() * 0.05f + autoPilot.m_speed));
    } else if (autoPilot.m_speed >= 0.1f) {
        autoPilot.ModifySpeed(std::max(autoPilot.m_fMaxTrafficSpeed, autoPilot.m_speed - CTimer::GetTimeStep() * 0.7f));
    } else if (autoPilot.m_speed != 0.0f) {
        autoPilot.ModifySpeed(0.0f);
    }
}

// 0x428DE0
void CCarCtrl::SteerAIBoatWithPhysicsAttackingPlayer(CVehicle* vehicle, float* arg2, float* arg3, float* arg4, bool* arg5) {
    plugin::Call<0x428DE0, CVehicle*, float*, float*, float*, bool*>(vehicle, arg2, arg3, arg4, arg5);
}

// 0x429090
void CCarCtrl::SteerAIBoatWithPhysicsCirclingPlayer(CVehicle* vehicle, float* arg2, float* arg3, float* arg4, bool* arg5) {
    plugin::Call<0x429090, CVehicle*, float*, float*, float*, bool*>(vehicle, arg2, arg3, arg4, arg5);
}

// 0x428BE0
void CCarCtrl::SteerAIBoatWithPhysicsHeadingForTarget(CVehicle* vehicle, float arg2, float arg3, float* arg4, float* arg5, float* arg6) {
    plugin::Call<0x428BE0, CVehicle*, float, float, float*, float*, float*>(vehicle, arg2, arg3, arg4, arg5, arg6);
}

// 0x422B20
void CCarCtrl::SteerAICarBlockingPlayerForwardAndBack(CVehicle* vehicle, float* arg2, float* arg3, float* arg4, bool* arg5) {
    plugin::Call<0x422B20, CVehicle*, float*, float*, float*, bool*>(vehicle, arg2, arg3, arg4, arg5);
}

// 0x433BA0
void CCarCtrl::SteerAICarParkParallel(CVehicle* vehicle, float* arg2, float* arg3, float* arg4, bool* arg5) {
    plugin::Call<0x433BA0, CVehicle*, float*, float*, float*, bool*>(vehicle, arg2, arg3, arg4, arg5);
}

// 0x433EA0
void CCarCtrl::SteerAICarParkPerpendicular(CVehicle* vehicle, float* arg2, float* arg3, float* arg4, bool* arg5) {
    plugin::Call<0x433EA0, CVehicle*, float*, float*, float*, bool*>(vehicle, arg2, arg3, arg4, arg5);
}

// 0x4336D0
void CCarCtrl::SteerAICarTowardsPointInEscort(CVehicle* vehicle1, CVehicle* vehicle2, float arg3, float arg4, float* arg5, float* arg6, float* arg7, bool* arg8) {
    plugin::Call<0x4336D0, CVehicle*, CVehicle*, float, float, float*, float*, float*, bool*>(vehicle1, vehicle2, arg3, arg4, arg5, arg6, arg7, arg8);
}

// 0x437C20
void CCarCtrl::SteerAICarWithPhysics(CVehicle* vehicle) {
    plugin::Call<0x437C20, CVehicle*>(vehicle);
}

// 0x434900
void CCarCtrl::SteerAICarWithPhysicsFollowPath(CVehicle* vehicle, float* arg2, float* arg3, float* arg4, bool* arg5) {
    plugin::Call<0x434900, CVehicle*, float*, float*, float*, bool*>(vehicle, arg2, arg3, arg4, arg5);
}

// 0x435830
void CCarCtrl::SteerAICarWithPhysicsFollowPath_Racing(CVehicle* vehicle, float* arg2, float* arg3, float* arg4, bool* arg5) {
    plugin::Call<0x435830, CVehicle*, float*, float*, float*, bool*>(vehicle, arg2, arg3, arg4, arg5);
}

// 0x432DD0
void CCarCtrl::SteerAICarWithPhysicsFollowPreRecordedPath(CVehicle* vehicle, float* arg2, float* arg3, float* arg4, bool* arg5) {
    plugin::Call<0x432DD0, CVehicle*, float*, float*, float*, bool*>(vehicle, arg2, arg3, arg4, arg5);
}

// 0x433280
void CCarCtrl::SteerAICarWithPhysicsHeadingForTarget(CVehicle* vehicle, CPhysical* target, float arg3, float arg4, float* arg5, float* arg6, float* arg7, bool* arg8) {
    plugin::Call<0x433280, CVehicle*, CPhysical*, float, float, float*, float*, float*, bool*>(vehicle, target, arg3, arg4, arg5, arg6, arg7, arg8);
}

// 0x4335E0
void CCarCtrl::SteerAICarWithPhysicsTryingToBlockTarget(CVehicle* vehicle, CEntity* Unusued, float arg3, float arg4, float arg5, float arg6, float* arg7, float* arg8, float* arg9, bool* arg10) {
    plugin::Call<0x4335E0, CVehicle*, CEntity*, float, float, float, float, float*, float*, float*, bool*>(vehicle, Unusued, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10);
}

// 0x428990
void CCarCtrl::SteerAICarWithPhysicsTryingToBlockTarget_Stop(CVehicle* vehicle, float x, float y, float arg4, float arg5, float* arg6, float* arg7, float* arg8, bool* arg9) {
    plugin::Call<0x428990, CVehicle*, float, float, float, float, float*, float*, float*, bool*>(vehicle, x, y, arg4, arg5, arg6, arg7, arg8, arg9);
}

// 0x436A90
void CCarCtrl::SteerAICarWithPhysics_OnlyMission(CVehicle* vehicle, float* arg2, float* arg3, float* arg4, bool* arg5) {
    plugin::Call<0x436A90, CVehicle*, float*, float*, float*, bool*>(vehicle, arg2, arg3, arg4, arg5);
}

// 0x42AAD0
void CCarCtrl::SteerAIHeliAsPoliceHeli(CAutomobile* automobile) {
    plugin::Call<0x42AAD0, CAutomobile*>(automobile);
}

// 0x42ACB0
void CCarCtrl::SteerAIHeliFlyingAwayFromPlayer(CAutomobile* automobile) {
    plugin::Call<0x42ACB0, CAutomobile*>(automobile);
}

// 0x4238E0
void CCarCtrl::SteerAIHeliToCrashAndBurn(CAutomobile* automobile) {
    plugin::Call<0x4238E0, CAutomobile*>(automobile);
}

// 0x42A750
void CCarCtrl::SteerAIHeliToFollowEntity(CAutomobile* automobile) {
    plugin::Call<0x42A750, CAutomobile*>(automobile);
}

// 0x42AEB0
void CCarCtrl::SteerAIHeliToKeepEntityInView(CAutomobile* automobile) {
    plugin::Call<0x42AEB0, CAutomobile*>(automobile);
}

// 0x42AD30
void CCarCtrl::SteerAIHeliToLand(CAutomobile* automobile) {
    plugin::Call<0x42AD30, CAutomobile*>(automobile);
}

// 0x42A630
void CCarCtrl::SteerAIHeliTowardsTargetCoors(CAutomobile* automobile) {
    plugin::Call<0x42A630, CAutomobile*>(automobile);
}

// 0x423880
void CCarCtrl::SteerAIPlaneToCrashAndBurn(CAutomobile* automobile) {
    plugin::Call<0x423880, CAutomobile*>(automobile);
}

// 0x4237F0
void CCarCtrl::SteerAIPlaneToFollowEntity(CAutomobile* automobile) {
    plugin::Call<0x4237F0, CAutomobile*>(automobile);
}

// 0x423790
void CCarCtrl::SteerAIPlaneTowardsTargetCoors(CAutomobile* automobile) {
    plugin::Call<0x423790, CAutomobile*>(automobile);
}

// 0x422590
bool CCarCtrl::StopCarIfNodesAreInvalid(CVehicle* vehicle) {
    return plugin::CallAndReturn<bool, 0x422590, CVehicle*>(vehicle);
}

// 0x4222A0
void CCarCtrl::SwitchBetweenPhysicsAndGhost(CVehicle* vehicle) {
    plugin::Call<0x4222A0, CVehicle*>(vehicle);
}

// 0x423FC0
void CCarCtrl::SwitchVehicleToRealPhysics(CVehicle* vehicle) {
    plugin::Call<0x423FC0, CVehicle*>(vehicle);
}

// 0x425B30
float CCarCtrl::TestCollisionBetween2MovingRects(CVehicle* vehicle1, CVehicle* vehicle2, float arg3, float arg4, CVector* pos1, CVector* pos2) {
    return plugin::CallAndReturn<float, 0x425B30, CVehicle*, CVehicle*, float, float, CVector*, CVector*>(vehicle1, vehicle2, arg3, arg4, pos1, pos2);
}

// 0x425F70
float CCarCtrl::TestCollisionBetween2MovingRects_OnlyFrontBumper(CVehicle* vehicle1, CVehicle* vehicle2, float arg3, float arg4, CVector* pos1, CVector* pos2) {
    return plugin::CallAndReturn<float, 0x425F70, CVehicle*, CVehicle*, float, float, CVector*, CVector*>(vehicle1, vehicle2, arg3, arg4, pos1, pos2);
}

// 0x429520
void CCarCtrl::TestWhetherToFirePlaneGuns(CVehicle* vehicle, CEntity* target) {
    plugin::Call<0x429520, CVehicle*, CEntity*>(vehicle, target);
}

// 0x421FE0
bool CCarCtrl::ThisVehicleShouldTryNotToTurn(CVehicle* vehicle) {
    return plugin::CallAndReturn<bool, 0x421FE0, CVehicle*>(vehicle);
}

// 0x429300
void CCarCtrl::TriggerDogFightMoves(CVehicle* vehicle1, CVehicle* vehicle2) {
    plugin::Call<0x429300, CVehicle*, CVehicle*>(vehicle1, vehicle2);
}

// 0x424000
void CCarCtrl::UpdateCarCount(CVehicle* vehicle, uint8 bDecrease) {
    plugin::Call<0x424000, CVehicle*, uint8>(vehicle, bDecrease);
}

// 0x436540
void CCarCtrl::UpdateCarOnRails(CVehicle* vehicle) {
    plugin::Call<0x436540, CVehicle*>(vehicle);
}

// 0x426BC0
void CCarCtrl::WeaveForObject(CEntity* entity, CVehicle* vehicle, float* arg3, float* arg4) {
    plugin::Call<0x426BC0, CEntity*, CVehicle*, float*, float*>(entity, vehicle, arg3, arg4);
}

// 0x426350
void CCarCtrl::WeaveForOtherCar(CEntity* entity, CVehicle* vehicle, float* arg3, float* arg4) {
    plugin::Call<0x426350, CEntity*, CVehicle*, float*, float*>(entity, vehicle, arg3, arg4);
}

// 0x42D680
template<typename PtrListType>
void CCarCtrl::WeaveThroughCarsSectorList(PtrListType& ptrList, CVehicle* vehicle, CPhysical* physical, float arg4, float arg5, float arg6, float arg7, float* arg8, float* arg9) {
    plugin::Call<0x42D680, PtrListType&, CVehicle*, CPhysical*, float, float, float, float, float*, float*>(ptrList, vehicle, physical, arg4, arg5, arg6, arg7, arg8, arg9);
}

// 0x42D950
template<typename PtrListType>
void CCarCtrl::WeaveThroughObjectsSectorList(PtrListType& ptrList, CVehicle* vehicle, float arg3, float arg4, float arg5, float arg6, float* arg7, float* arg8) {
    plugin::Call<0x42D950, PtrListType&, CVehicle*, float, float, float, float, float*, float*>(ptrList, vehicle, arg3, arg4, arg5, arg6, arg7, arg8);
}

// 0x42D7E0
template<typename PtrListType>
void CCarCtrl::WeaveThroughPedsSectorList(PtrListType& ptrList, CVehicle* vehicle, CPhysical* physical, float arg4, float arg5, float arg6, float arg7, float* arg8, float* arg9) {
    plugin::Call<0x42D7E0, PtrListType&, CVehicle*, CPhysical*, float, float, float, float, float*, float*>(ptrList, vehicle, physical, arg4, arg5, arg6, arg7, arg8, arg9);
}

// 0x427FE0
float CCarCtrl::FindMaxSteerAngle(CVehicle* veh) {
    return std::clamp(0.9f - veh->GetMoveSpeed().Magnitude(), 0.2f, 0.7f);
}
