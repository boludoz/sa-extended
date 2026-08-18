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
    RH_ScopedInstall(SetUpDriverAndPassengersForVehicle, 0x4217C0);
    RH_ScopedInstall(ClearInterestingVehicleList, 0x423F00);
    RH_ScopedInstall(FindSpeedMultiplierWithSpeedFromNodes, 0x424130);
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
    apCarsToKeep[0] = nullptr;
    apCarsToKeep[1] = nullptr;
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
        boat->m_autoPilot.TempAction = TEMPACT_NONE;
        boat->m_autoPilot.ActualSpeed       = 20.0F;
        boat->m_autoPilot.SetCruiseSpeed(20);

        if (doMissionCleanup) {
            boat->m_bIsStaticWaitingForCollision = true;
        }

        boat->m_autoPilot.bWaitForValidNodes = true;
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
    vehicle->m_autoPilot.TempAction      = TEMPACT_NONE;
    vehicle->m_autoPilot.DrivingMode = DRIVING_STYLE_STOP_FOR_CARS;
    vehicle->m_autoPilot.ActualSpeed            = 13.0F;
    vehicle->m_autoPilot.SetCruiseSpeed(13);
    vehicle->m_autoPilot.OldLane = 0;
    vehicle->m_autoPilot.NewLane    = 0;

    if (doMissionCleanup) {
        vehicle->m_bIsStaticWaitingForCollision = true;
    }

    CWorld::Add(vehicle);
    if (doMissionCleanup) {
        CTheScripts::MissionCleanUp.AddEntityToList(GetVehiclePool()->GetRef(vehicle), MISSION_CLEANUP_ENTITY_TYPE_VEHICLE);
    }

    if (vehicle->IsSubRoadVehicle()) {
        vehicle->m_autoPilot.bWaitForValidNodes = true;
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
float CCarCtrl::FindSpeedMultiplierWithSpeedFromNodes(int8 speedFromNodes) {
    switch (speedFromNodes) {
    case -1:
        return 0.5f;
    case 0:
        return 0.8f;
    case 2:
        return 1.2f;
    default:
        return 1.0f;
    }
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
void CCarCtrl::GenerateOneRandomCar()
{
	//float Fraction, Length, Interp, Length2; 
    float OldDirX, OldDirY, NewDirX, NewDirY;//, ClosestZ;
    CNodeAddress FromNode = CNodeAddress(), ToNode = CNodeAddress();
    CPlayerInfo* pPlayer;
    int32 CarRating, CarModel;
    CVehicle* pNewVehicle;
    CVector Result,
        DirF,
        ResultSpeed ,
        SetCoors,
        Dir, Diff
        ;
    CVector      RelSpeed; //, RelCoors, TempVec;
    int16 Link, RandomLinkIndex;
    //int16 TempLanesOurWay, TempLanesOtherWay;
    CCarPathLinkAddress RandomLink;
    CColPoint TestColPoint;
    CEntity* TestEntity;
    CVector PlayerPos, PlayerSpeed;
    float NewLaneOffset, OldLaneOffset;
    float DirX;
    float DirY;
    float RequiredDistance,
        ///RequiredDotProduct,
        Dist;
    CVehicle* pPlayerVehicle;
    //int32 FractionMadDrivers;
    //bool bClearAreaTest, bRequiredInside;
    int32 PlayerForGeneration;
    bool bTopDown = false;
    bool bMadDriver = false;
    bool bNoPoliceChasing = false;

    PlayerForGeneration = CWorld::PlayerInFocus;
    pPlayer = &CWorld::Players[PlayerForGeneration];
    PlayerPos = FindPlayerCentreOfWorld(PlayerForGeneration);
    PlayerSpeed = FindPlayerSpeed();

    {
        int32 TotalCarsOnMap = NumRandomCars + NumLawEnforcerCars + NumMissionCars + NumAmbulancesOnDuty + NumFireTrucksOnDuty;
        float LocalCarDensityMultiplier = CarDensityMultiplier;
        if (CCullZones::FewerCars())
        {
            LocalCarDensityMultiplier *= 0.6f;
        }

        if (CPopulation::FindCarMultiplierMotorway() * MaxNumberOfCarsInUse * LocalCarDensityMultiplier <= (float)TotalCarsOnMap
            || CPopulation::FindCarMultiplierMotorway() * (float)(CPopCycle::m_NumDealers_Cars + CPopCycle::m_NumGangs_Cars + CPopCycle::m_NumCops_Cars + CPopCycle::m_NumOther_Cars) * LocalCarDensityMultiplier <= (float)TotalCarsOnMap)
        {
            return;
        }
    }

    if ((uint32)FindPlayerWanted()->GetWantedLevel() > 1
        && NumLawEnforcerCars < FindPlayerWanted()->m_MaxCopCarsInPursuit
        && FindPlayerWanted()->m_NumCopsInPursuit < FindPlayerWanted()->m_MaxCopsInPursuit
        && CGame::currArea == AREA_CODE_NORMAL_WORLD
        && !CGangWars::GangWarFightingGoingOn()
        && ((uint32)FindPlayerWanted()->GetWantedLevel() > 3
            || ((uint32)FindPlayerWanted()->GetWantedLevel() > 2 && CTimer::GetTimeInMS() > (uint32)LastTimeLawEnforcerCreated + 5'000)
            || CTimer::GetTimeInMS() > (uint32)LastTimeLawEnforcerCreated + 8'000))
    {
        CarModel = ChoosePoliceCarModel(0);
        CarRating = 13;
    }
    else
    {
        CarModel = ChooseModel(&CarRating);
        if (CarModel == -1)
        {
            return;
        }
        if ((CarRating == 13 || CarRating == 24) && (uint32)FindPlayerWanted()->GetWantedLevel() >= 1)
        {
            return;
        }
    }

    if (CGameLogic::LaRiotsActiveHere() && !gbLARiots_NoPoliceCars && (CGeneral::GetRandomNumber() & 127) < 55)
    {
        CarModel = ChoosePoliceCarModel(0);
        CarRating = 13;
    }

    float DirectionX, DirectionY, PreferredDistance;
    bool bGenerateBehind;

    if (TheCamera.m_mCameraMatrix.GetForward().z < -0.9f)
    {
        bTopDown = true;
        DirectionY = 0.707f;
        DirectionX = 0.707f;
        PreferredDistance = -1.0f;
        bGenerateBehind = true;
    }
    else
    {
        pPlayerVehicle = FindPlayerVehicle();
        if (pPlayerVehicle != nullptr)
        {
            float SpeedX = pPlayerVehicle->m_vecMoveSpeed.x;
            float SpeedY = pPlayerVehicle->m_vecMoveSpeed.y;
            float Speed = std::sqrt(SpeedY * SpeedY + SpeedX * SpeedX);

            if (Speed > 0.4f)
            {
                DirectionX = SpeedX * (1.0f / Speed);
                DirectionY = SpeedY * (1.0f / Speed);

                switch (CTimer::m_FrameCounter & 3)
                {
                    case 0:
                    case 1:
                        PreferredDistance = 0.85f;
                        bGenerateBehind = true;
                        break;
                    case 2:
                        PreferredDistance = 0.707f;
                        bGenerateBehind = true;
                        break;
                    case 3:
                        PreferredDistance = 0.707f;
                        bGenerateBehind = false;
                        break;
                }
            }
            else if (Speed > 0.1f)
            {
                DirectionX = SpeedX * (1.0f / Speed);
                DirectionY = SpeedY * (1.0f / Speed);

                switch (CTimer::m_FrameCounter & 3)
                {
                    case 0:
                        PreferredDistance = 0.85f;
                        bGenerateBehind = true;
                        break;
                    case 1:
                        PreferredDistance = 0.707f;
                        bGenerateBehind = true;
                        break;
                    case 2:
                    case 3:
                        PreferredDistance = 0.707f;
                        bGenerateBehind = false;
                        break;
                }
            }
            else
            {
                DirectionX = TheCamera.m_fCamFrontXNorm;
                DirectionY = TheCamera.m_fCamFrontYNorm;

                switch (CTimer::m_FrameCounter & 1)
                {
                    case 0:
                        PreferredDistance = 0.707f;
                        bGenerateBehind = true;
                        break;
                    case 1:
                        PreferredDistance = 0.707f;
                        bGenerateBehind = false;
                        break;
                }
            }
        }
        else
        {
            DirectionX = TheCamera.m_fCamFrontXNorm;
            DirectionY = TheCamera.m_fCamFrontYNorm;

            switch (CTimer::m_FrameCounter & 1)
            {
                case 0:
                    PreferredDistance = 0.707f;
                    bGenerateBehind = true;
                    break;
                case 1:
                    PreferredDistance = 0.707f;
                    bGenerateBehind = false;
                    break;
            }
        }
    }

    float FractionOfLinkBetweenNodes;
    if (!GenerateCarCreationCoors2(PlayerPos, DirectionX, DirectionY, PreferredDistance, bGenerateBehind, TheCamera.m_fGenerationDistMultiplier * 160.0f, 38.0f, &Result, &FromNode, &ToNode, &FractionOfLinkBetweenNodes, CarRating != 13 || (uint32)FindPlayerWanted()->GetWantedLevel() < 1, false))
    {
        return;
    }

    bool bIsBoat = false;
    uint32 Density;

    CPathNode* pToNode = ThePaths.GetPathNode(ToNode);
    CPathNode* pFromNode = ThePaths.GetPathNode(FromNode);

    Density = pFromNode->m_nSpawnProbability >= pToNode->m_nSpawnProbability ? pToNode->m_nSpawnProbability : pFromNode->m_nSpawnProbability;

    if ((uint32)(CGeneral::GetRandomNumber() & 15) > Density)
    {
        return;
    }

    float radius = 8.0f;
    if (pFromNode->m_bWaterNode)
    {
        bIsBoat = true;

        if (CarRating == 13)
        {
            CarModel = MODEL_PREDATOR;
            CarRating = 24;
            if (!CStreaming::IsModelLoaded(MODEL_PREDATOR))
            {
                CStreaming::RequestModel(MODEL_PREDATOR, STREAMING_KEEP_IN_MEMORY);
                return;
            }
        }
        else
        {
            CarModel = CPopulation::m_LoadedBoats.PickLeastUsedModel(1);
            if (CarModel == -1 || !CStreaming::IsModelLoaded(CarModel))
            {
                return;
            }
        }
        radius = 40.0f;
    }

    int16 Num = 0;
    CWorld::FindObjectsKindaColliding(Result, radius, true, &Num, 2, nullptr, false, true, true, false, false);
    if (Num != 0)
    {
        return;
    }

    for (Link = 0; Link < (int16)pFromNode->m_nNumLinks; Link++)
    {
        if (ThePaths.m_pNodeLinks[FromNode.m_wAreaId][Link + pFromNode->m_wBaseLinkId] != ToNode)
        {
            continue;
        }
        break;
    }

    RandomLink = ThePaths.m_pNaviLinks[FromNode.m_wAreaId][Link + pFromNode->m_wBaseLinkId];
    CCarPathLink* pNewLink = &ThePaths.GetCarPathLink(RandomLink);

    int16 NumLanes = 0;
    if (pNewLink->m_attachedTo == ToNode)
    {
        NumLanes = pNewLink->m_numOppositeDirLanes;
    }
    else
    {
        NumLanes = pNewLink->m_numSameDirLanes;
    }

    if (NumLanes > 1)
    {
        if (CModelInfo::GetModelInfo(CarModel)->AsVehicleModelInfoPtr()->m_nVehicleType == VEHICLE_TYPE_BMX)
        {
            return;
        }
    }
    else
    {
        if (CarModel == MODEL_COACH || CarModel == MODEL_BUS)
        {
            return;
        }
    }

    if (NumLanes == 0)
    {
        return;
    }

    if (CPopCycle::m_pCurrZone != nullptr)
    {
        CZoneInfo* pZoneInfo = CTheZones::GetZoneInfo(Result, nullptr);
        int32 ZoneType = pZoneInfo->PopType;
        if (ZoneType >= 17 && ZoneType <= 19)
        {
            if (ZoneType != CPopCycle::m_nCurrentZoneType)
            {
                return;
            }
            bNoPoliceChasing = true;
        }
    }

    pNewVehicle = GetNewVehicleDependingOnCarModel(CarModel, eVehicleCreatedBy::RANDOM_VEHICLE);
    if (pNewVehicle == nullptr)
    {
        return;
    }

    pNewVehicle->m_autoPilot.VeryOldNode.m_wAreaId = (uint16)-1;
    pNewVehicle->m_autoPilot.OldNode = FromNode;
    pNewVehicle->m_autoPilot.NewNode = ToNode;

    switch (CarRating)
    {
        case 13:
        {
            pNewVehicle->m_autoPilot.TempAction = TEMPACT_NONE;
            if ((uint32)FindPlayerWanted()->GetWantedLevel() == 0)
            {
                pNewVehicle->m_autoPilot.CruiseSpeed = (uint8)CGeneral::GetRandomNumberInRange(18.0f, 24.0f);
                pNewVehicle->m_autoPilot.DrivingMode = DRIVING_STYLE_STOP_FOR_CARS;
                pNewVehicle->m_autoPilot.Mission = MISSION_CRUISE;
            }
            else
            {
                pNewVehicle->m_autoPilot.CruiseSpeed = (uint8)CCarAI::FindPoliceCarSpeedForWantedLevel(pNewVehicle);
                if (pNewVehicle->GetVehicleAppearance() == VEHICLE_APPEARANCE_BIKE)
                {
                    pNewVehicle->m_autoPilot.Mission = CCarAI::FindPoliceBikeMissionForWantedLevel();
                }
                else
                {
                    pNewVehicle->m_autoPilot.Mission = CCarAI::FindPoliceCarMissionForWantedLevel();
                }
                pNewVehicle->m_autoPilot.DrivingMode = DRIVING_STYLE_AVOID_CARS;
            }
            if (CarModel == MODEL_FBIRANCH)
            {
                pNewVehicle->m_nPrimaryColor = 0;
                pNewVehicle->m_nSecondaryColor = 0;
            }
            pNewVehicle->vehicleFlags.bCreatedAsPoliceVehicle = true;
        }
        break;
        case 24:
        {
            pNewVehicle->m_autoPilot.TempAction = TEMPACT_NONE;
            pNewVehicle->m_autoPilot.CruiseSpeed = (uint8)CGeneral::GetRandomNumberInRange(14.0f, 18.0f);
            pNewVehicle->m_autoPilot.DrivingMode = DRIVING_STYLE_AVOID_CARS;
            pNewVehicle->m_autoPilot.Mission = CCarAI::FindPoliceBoatMissionForWantedLevel();
            pNewVehicle->vehicleFlags.bCreatedAsPoliceVehicle = true;
        }
        break;
        default:
        {
            pNewVehicle->m_autoPilot.CruiseSpeed = (uint8)CGeneral::GetRandomNumberInRange(13.0f, 21.0f);

            if (CarRating == 3)
            {
                pNewVehicle->m_autoPilot.CruiseSpeed = (uint8)CGeneral::GetRandomNumberInRange(18.0f, 27.0f);
            }
            else if (CarRating == 1)
            {
                pNewVehicle->m_autoPilot.CruiseSpeed = (uint8)CGeneral::GetRandomNumberInRange(10.0f, 15.0f);
            }

            auto* colModel = CModelInfo::GetModelInfo(pNewVehicle->m_nModelIndex)->GetColModel();
            if (colModel->m_boundBox.m_vecMax.y - colModel->m_boundBox.m_vecMin.y > 10.0f || CarRating == 5)
            {
                pNewVehicle->m_autoPilot.CruiseSpeed = (uint8)(pNewVehicle->m_autoPilot.CruiseSpeed * 3 / 4);
            }

            if (bIsBoat)
            {
                if (pNewVehicle->m_nModelIndex == MODEL_SQUALO || pNewVehicle->m_nModelIndex == MODEL_SPEEDER || pNewVehicle->m_nModelIndex == MODEL_JETMAX)
                {
                    pNewVehicle->m_autoPilot.CruiseSpeed = (uint8)CGeneral::GetRandomNumberInRange(25.0f, 35.0f);
                }
                else
                {
                    pNewVehicle->m_autoPilot.CruiseSpeed = (uint8)CGeneral::GetRandomNumberInRange(15.0f, 24.0f);
                }
            }

            pNewVehicle->m_autoPilot.Mission = MISSION_CRUISE;
            pNewVehicle->m_autoPilot.TempAction = TEMPACT_NONE;
            pNewVehicle->m_autoPilot.DrivingMode = DRIVING_STYLE_STOP_FOR_CARS;
        }
        break;
    }

    if (pNewVehicle->m_nModelIndex == MODEL_MRWHOOP)
    {
        pNewVehicle->vehicleFlags.bSirenOrAlarm = true;
    }

    pNewVehicle->m_autoPilot.NewLink = RandomLink;
    int8 Lane = (int8)(CGeneral::GetRandomNumber() % NumLanes);
    pNewVehicle->m_autoPilot.OldLane = Lane;
    pNewVehicle->m_autoPilot.NewLane = Lane;

    int32 MadDriverChance;
    if (CGameLogic::LaRiotsActiveHere())
    {
        MadDriverChance = 80;
    }
    else
    {
        switch (pNewVehicle->GetVehicleAppearance())
        {
            case VEHICLE_APPEARANCE_BIKE:
                MadDriverChance = 50;
                break;
            case VEHICLE_APPEARANCE_BOAT:
                MadDriverChance = 10;
                break;
            default:
                MadDriverChance = 200;
                break;
        }
    }

    if (!bIsBoat && CarRating != 13 && !bNoPoliceChasing)
    {
        if (CGeneral::GetRandomNumberInRange(0, MadDriverChance) == 0 || CCheat::m_aCheatsActive[CHEAT_AGGRESSIVE_DRIVERS])
        {
            bMadDriver = true;
            FractionOfLinkBetweenNodes = 1.0f;
        }
    }

    auto* colModel = CModelInfo::GetModelInfo(pNewVehicle->m_nModelIndex)->GetColModel();
    float MinDistAwayFromNode = (colModel->m_boundBox.m_vecMax.y - colModel->m_boundBox.m_vecMin.y) * 0.5f + 1.0f;

    CVector fromPos = pFromNode->GetPosition();
    CVector toPos = pToNode->GetPosition();
    float NodesDist = std::sqrt((fromPos.x - toPos.x) * (fromPos.x - toPos.x) + (fromPos.y - toPos.y) * (fromPos.y - toPos.y));

    if (0.5f * NodesDist >= MinDistAwayFromNode)
    {
        float MinFraction = MinDistAwayFromNode / NodesDist;
        if (FractionOfLinkBetweenNodes <= MinFraction)
        {
            FractionOfLinkBetweenNodes = MinFraction;
        }
        if (FractionOfLinkBetweenNodes >= 1.0f - MinFraction)
        {
            FractionOfLinkBetweenNodes = 1.0f - MinFraction;
        }
    }
    else
    {
        FractionOfLinkBetweenNodes = 0.5f;
    }

    if (FromNode < ToNode)
    {
        pNewVehicle->m_autoPilot.InvertDirNewLink = -1;
    }
    else
    {
        pNewVehicle->m_autoPilot.InvertDirNewLink = 1;
    }

    if (pFromNode->m_nNumLinks == 1)
    {
        delete pNewVehicle;
        return;
    }

    do
    {
        RandomLinkIndex = (int16)(CGeneral::GetRandomNumber() % pFromNode->m_nNumLinks);
        pNewVehicle->m_autoPilot.OldLink = ThePaths.m_pNaviLinks[FromNode.m_wAreaId][pFromNode->m_wBaseLinkId + RandomLinkIndex];
    } while (pNewVehicle->m_autoPilot.OldLink == pNewVehicle->m_autoPilot.NewLink);

    if (!ThePaths.m_pPathNodes[pNewVehicle->m_autoPilot.OldLink.m_wAreaId])
    {
        delete pNewVehicle;
        return;
    }

    if (ThePaths.m_pNodeLinks[FromNode.m_wAreaId][pFromNode->m_wBaseLinkId + RandomLinkIndex] < FromNode)
    {
        pNewVehicle->m_autoPilot.InvertDirOldLink = -1;
    }
    else
    {
        pNewVehicle->m_autoPilot.InvertDirOldLink = 1;
    }

    DirF = toPos - fromPos;

    DirX = DirF.x;
    DirY = DirF.y;
    float Forward2DMag = std::sqrt(DirX * DirX + DirY * DirY);
    if (Forward2DMag == 0.0f)
    {
        DirX = 1.0f;
    }
    else
    {
        DirX = DirX * (1.0f / Forward2DMag);
        DirY = DirY * (1.0f / Forward2DMag);
    }

    DirF.Normalise();
    pNewVehicle->GetMatrix().GetForward() = DirF;
    pNewVehicle->GetMatrix().GetRight() = CVector(DirY, -DirX, 0.0f);
    pNewVehicle->GetMatrix().GetUp() = CVector(0.0f, 0.0f, 1.0f);

    const auto GetLinkCoors = [](CCarPathLinkAddress link) -> CVector2D {
        return ThePaths.GetCarPathLink(link).GetNodeCoors();
    };
    const auto GetNodeCoors = [](CNodeAddress node) -> CVector2D {
        return CVector2D{ ThePaths.GetPathNode(node)->GetPosition() };
    };

    float DistNewLinkToOldNode = (GetLinkCoors(pNewVehicle->m_autoPilot.NewLink) - GetNodeCoors(pNewVehicle->m_autoPilot.OldNode)).Magnitude();
    float DistNewLinkToNewNode = (GetLinkCoors(pNewVehicle->m_autoPilot.NewLink) - GetNodeCoors(pNewVehicle->m_autoPilot.NewNode)).Magnitude();
    float OurDistToOldNode;
    float DistOldLinkToOldNode;
    float FractionBetweenLinks;

    if (DistNewLinkToOldNode / (DistNewLinkToNewNode + DistNewLinkToOldNode) > FractionOfLinkBetweenNodes)
    {
        DistOldLinkToOldNode = (GetLinkCoors(pNewVehicle->m_autoPilot.OldLink) - GetNodeCoors(pNewVehicle->m_autoPilot.OldNode)).Magnitude();
        OurDistToOldNode = (CVector2D{ Result } - GetNodeCoors(pNewVehicle->m_autoPilot.OldNode)).Magnitude();

        FractionBetweenLinks = (OurDistToOldNode + DistOldLinkToOldNode) / (DistOldLinkToOldNode + DistNewLinkToOldNode);
    }
    else
    {
        PickNextNodeRandomly(pNewVehicle);

        DistNewLinkToOldNode = (GetLinkCoors(pNewVehicle->m_autoPilot.NewLink) - GetNodeCoors(pNewVehicle->m_autoPilot.OldNode)).Magnitude();
        DistOldLinkToOldNode = (GetLinkCoors(pNewVehicle->m_autoPilot.OldLink) - GetNodeCoors(pNewVehicle->m_autoPilot.OldNode)).Magnitude();
        OurDistToOldNode = (CVector2D{ Result } - GetNodeCoors(pNewVehicle->m_autoPilot.OldNode)).Magnitude();

        FractionBetweenLinks = (DistOldLinkToOldNode - OurDistToOldNode) / (DistNewLinkToOldNode + DistOldLinkToOldNode);
    }

    if (FractionBetweenLinks < 0.0f)
    {
        FractionBetweenLinks = 0.0f;
    }
    else if (FractionBetweenLinks > 1.0f)
    {
        FractionBetweenLinks = 1.0f;
    }

    const auto& oldLinkRef = ThePaths.GetCarPathLink(pNewVehicle->m_autoPilot.OldLink);
    const auto& newLinkRef = ThePaths.GetCarPathLink(pNewVehicle->m_autoPilot.NewLink);

    OldDirX = (float)(oldLinkRef.m_dir.x) * (float)pNewVehicle->m_autoPilot.InvertDirOldLink * 0.01f;
    OldDirY = (float)(oldLinkRef.m_dir.y) * (float)pNewVehicle->m_autoPilot.InvertDirOldLink * 0.01f;
    NewDirX = (float)(newLinkRef.m_dir.x) * (float)pNewVehicle->m_autoPilot.InvertDirNewLink * 0.01f;
    NewDirY = (float)(newLinkRef.m_dir.y) * (float)pNewVehicle->m_autoPilot.InvertDirNewLink * 0.01f;

    OldLaneOffset = (oldLinkRef.OneWayLaneOffset() + (float)pNewVehicle->m_autoPilot.OldLane) * 5.4f;
    NewLaneOffset = (newLinkRef.OneWayLaneOffset() + (float)pNewVehicle->m_autoPilot.NewLane) * 5.4f;
    if (pNewVehicle->m_nVehicleType == VEHICLE_TYPE_BMX)
    {
        OldLaneOffset += 1.458f;
        NewLaneOffset += 1.458f;
    }

    CPathNode* pSpeedNode = ThePaths.GetPathNode(pNewVehicle->m_autoPilot.NewNode);
    pNewVehicle->m_autoPilot.SpeedFromNodes = (int8)(pSpeedNode->m_bNotHighway | (pSpeedNode->m_bHighway << 1));
    pNewVehicle->m_autoPilot.SpeedMultiplier = FindSpeedMultiplierWithSpeedFromNodes(pNewVehicle->m_autoPilot.SpeedFromNodes);
    pNewVehicle->m_autoPilot.ActualSpeed = (float)pNewVehicle->m_autoPilot.CruiseSpeed * pNewVehicle->m_autoPilot.SpeedMultiplier;

    CVector oldLanePos(GetLinkCoors(pNewVehicle->m_autoPilot.OldLink).x + OldLaneOffset * OldDirY, GetLinkCoors(pNewVehicle->m_autoPilot.OldLink).y - OldLaneOffset * OldDirX, 0.0f);
    CVector newLanePos(GetLinkCoors(pNewVehicle->m_autoPilot.NewLink).x + NewLaneOffset * NewDirY, GetLinkCoors(pNewVehicle->m_autoPilot.NewLink).y - NewLaneOffset * NewDirX, 0.0f);

    pNewVehicle->m_autoPilot.TimeToGetToNextLink = (int32)(CCurves::CalcSpeedScaleFactor(
        oldLanePos,
        newLanePos,
        OldDirX, OldDirY, NewDirX, NewDirY) * (1000.0f / pNewVehicle->m_autoPilot.ActualSpeed));

    pNewVehicle->m_autoPilot.TimeToLeaveLink = (int32)(CTimer::GetTimeInMS() - FractionBetweenLinks * (float)pNewVehicle->m_autoPilot.TimeToGetToNextLink);

    CCurves::CalcCurvePoint(
        oldLanePos,
        newLanePos,
        CVector(OldDirX, OldDirY, 0.0f), CVector(NewDirX, NewDirY, 0.0f),
        (float)(CTimer::GetTimeInMS() - (uint32)pNewVehicle->m_autoPilot.TimeToLeaveLink) / (float)pNewVehicle->m_autoPilot.TimeToGetToNextLink,
        pNewVehicle->m_autoPilot.TimeToGetToNextLink, SetCoors, RelSpeed);

    float PullBackDistance = 2.0f;
    Dir = fromPos - toPos;
    CVector Posn = SetCoors + Dir * (PullBackDistance / Dir.Magnitude());
    Posn.z = (1.0f - FractionOfLinkBetweenNodes) * fromPos.z + FractionOfLinkBetweenNodes * toPos.z;

    float GroundZ = 1000000000.0f;

    if (bIsBoat)
    {
        float WaterZ;
        if (!CWaterLevel::GetWaterLevel(Posn.x, Posn.y, Posn.z, WaterZ, true, nullptr))
        {
            delete pNewVehicle;
            return;
        }
        GroundZ = WaterZ;
    }
    else
    {
        if (CWorld::ProcessVerticalLine(Posn, 1000.0f, TestColPoint, TestEntity, true, false, false, false, false, true, nullptr))
        {
            GroundZ = TestColPoint.m_vecPoint.z;
        }
        if (CWorld::ProcessVerticalLine(Posn, -1000.0f, TestColPoint, TestEntity, true, false, false, false, false, true, nullptr))
        {
            if (std::abs(TestColPoint.m_vecPoint.z - Posn.z) < std::abs(GroundZ - Posn.z))
            {
                GroundZ = TestColPoint.m_vecPoint.z;
            }
        }
    }

    if (GroundZ == 1000000000.0f)
    {
        delete pNewVehicle;
        return;
    }

    if (std::abs(GroundZ - Posn.z) > 7.0f)
    {
        delete pNewVehicle;
        return;
    }

    if (CModelInfo::IsBoatModel(pNewVehicle->m_nModelIndex))
    {
        Posn.z = GroundZ;
        pNewVehicle->m_nExtendedRemovalRange = 255;
    }
    else
    {
        Posn.z = GroundZ + pNewVehicle->GetHeightAboveRoad();
    }

    pNewVehicle->SetPosn(Posn);
    pNewVehicle->m_vecMoveSpeed = CVector(0.0f, 0.0f, 0.0f);

    ResultSpeed = RelSpeed * (1.0f / 60.0f) - PlayerSpeed;
    Diff.x = SetCoors.x - PlayerPos.x;
    Diff.y = SetCoors.y - PlayerPos.y;

    if (CarRating == 13)
    {
        if (pNewVehicle->m_autoPilot.Mission == MISSION_CRUISE)
        {
            pNewVehicle->SetStatus(STATUS_PHYSICS);
        }
        else
        {
            pNewVehicle->SetStatus(STATUS_SIMPLE);
        }
    }
    else if (CarRating == 24)
    {
        pNewVehicle->SetStatus(STATUS_SIMPLE);
    }
    else if (bIsBoat)
    {
        pNewVehicle->SetStatus(STATUS_SIMPLE);
    }
    else if (pNewVehicle->GetStatus() != STATUS_SIMPLE)
    {
        pNewVehicle->SetStatus(STATUS_PHYSICS);
    }

    CVisibilityPlugins::SetClumpAlpha(pNewVehicle->GetRpClump(), 0);

    if (CCheat::m_aCheatsActive[CHEAT_FUNHOUSE_THEME] && pNewVehicle->m_nVehicleType == VEHICLE_TYPE_AUTOMOBILE)
    {
        pNewVehicle->AddVehicleUpgrade(ModelIndices::MI_HYDRAULICS);
    }
    if (pNewVehicle->GetIsOnScreen())
    {
        Dist = (PlayerPos - pNewVehicle->GetPosition()).Magnitude2D();
        if (std::max(170.0f, (float)pNewVehicle->m_nExtendedRemovalRange) * TheCamera.m_fGenerationDistMultiplier < Dist || TheCamera.m_fGenerationDistMultiplier * 150.0f > Dist)
        {
            delete pNewVehicle;
            return;
        }

        if (TheCamera.m_fGenerationDistMultiplier * 120.0f > (TheCamera.GetPosition() - pNewVehicle->GetPosition()).Magnitude2D() || bTopDown)
        {
            delete pNewVehicle;
            return;
        }

        if (pNewVehicle->m_nModelIndex == MODEL_MARQUIS)
        {
            delete pNewVehicle;
            return;
        }
    }
    else
    {
        Dist = (PlayerPos - pNewVehicle->GetPosition()).Magnitude2D();
        RequiredDistance = std::max(170.0f, (float)pNewVehicle->m_nExtendedRemovalRange) * (1.0f / 170.0f) * 45.0f;
        if (Dist > RequiredDistance && !bTopDown)
        {
            delete pNewVehicle;
            return;
        }
    }
    int16 NumObjectsFound = 0;
    CWorld::FindObjectsKindaColliding(pNewVehicle->GetPosition(), CModelInfo::GetModelInfo(pNewVehicle->m_nModelIndex)->GetColModel()->m_boundSphere.m_fRadius, true, &NumObjectsFound, 2, nullptr, false, true, true, false, false);

    if (NumObjectsFound == 0 && ResultSpeed.x * Diff.x + ResultSpeed.y * Diff.y < 0.0f)
    {
        CModelInfo::GetModelInfo(pNewVehicle->m_nModelIndex)->AsVehicleModelInfoPtr()->ChooseVehicleColour(pNewVehicle->m_nPrimaryColor, pNewVehicle->m_nSecondaryColor, pNewVehicle->m_nTertiaryColor, pNewVehicle->m_nQuaternaryColor, 1);

        CWorld::Add(pNewVehicle);

        if (pNewVehicle->m_nModelIndex == MODEL_TRACTOR || pNewVehicle->m_nModelIndex == MODEL_COMBINE || pNewVehicle->m_nVehicleType == VEHICLE_TYPE_BMX)
        {
            pNewVehicle->m_autoPilot.CruiseSpeed = (uint8)(pNewVehicle->m_autoPilot.CruiseSpeed / 3);
        }

        if (CGameLogic::LaRiotsActiveHere())
        {
            pNewVehicle->m_fHealth = (float)(CGeneral::GetRandomNumber() % 1000);
        }

        if (CarRating == 13)
        {
            LastTimeLawEnforcerCreated = CTimer::GetTimeInMS();
        }

        if (pNewVehicle->m_nModelIndex == MODEL_CADDY)
        {
            pNewVehicle->SetStatus(STATUS_SIMPLE);
            pNewVehicle->m_autoPilot.DrivingMode = DRIVING_STYLE_AVOID_CARS;
        }

        if (pNewVehicle->m_nVehicleType == VEHICLE_TYPE_AUTOMOBILE)
        {
            switch (CarRating)
            {
                case 0:
                case 4:
                case 5:
                case 6:
                case 13:
                    if (CGeneral::GetRandomNumberInRange(0, 20) == 0)
                    {
                        pNewVehicle->AsAutomobile()->SetRandomDamage(false);
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
                    if (CGeneral::GetRandomNumberInRange(0, 8) == 0)
                    {
                        pNewVehicle->AsAutomobile()->SetRandomDamage(true);
                    }
                    break;
                default:
                    break;
            }
        }

        if (pNewVehicle->m_nVehicleType == VEHICLE_TYPE_BIKE && pNewVehicle->m_autoPilot.DrivingMode == DRIVING_STYLE_STOP_FOR_CARS)
        {
            pNewVehicle->SetStatus(STATUS_SIMPLE);
            pNewVehicle->m_autoPilot.DrivingMode = DRIVING_STYLE_DRIVINGMODE_AVOIDCARS_STOPFORPEDS_OBEYLIGHTS;
        }

        if (!bIsBoat && CarRating != 13 && (uint32)FindPlayerWanted()->GetWantedLevel() == 0 && (CCheat::m_aCheatsActive[CHEAT_AGGRESSIVE_DRIVERS] || TimeNextMadDriverChaseCreated <= 0.0f) && !bNoPoliceChasing && CreatePoliceChase(pNewVehicle, CarRating, FromNode))
        {
            if (CGameLogic::LaRiotsActiveHere())
            {
                TimeNextMadDriverChaseCreated = CGeneral::GetRandomNumberInRange(240.0f, 480.0f);
            }
            else
            {
                TimeNextMadDriverChaseCreated = CGeneral::GetRandomNumberInRange(600.0f, 1200.0f);
            }
        }
        else if (bMadDriver)
        {
            if ((pNewVehicle->m_nModelIndex == MODEL_FREEWAY || pNewVehicle->m_nModelIndex == MODEL_PCJ600 || pNewVehicle->m_nModelIndex == MODEL_FCR900 || pNewVehicle->m_nModelIndex == MODEL_NRG500 || pNewVehicle->m_nModelIndex == MODEL_BF400 || pNewVehicle->m_nModelIndex == MODEL_WAYFARER) && !gbLARiots && CGeneral::GetRandomNumberInRange(0, 7) == 0 && CreateConvoy(pNewVehicle, CarRating))
            {
                SetUpDriverAndPassengersForVehicle(pNewVehicle, CarRating, 1, true, false, 99);
            }
            else
            {
                SetUpDriverAndPassengersForVehicle(pNewVehicle, CarRating, 1, true, false, 99);
                pNewVehicle->SetStatus(STATUS_SIMPLE);
                pNewVehicle->m_autoPilot.DrivingMode = DRIVING_STYLE_AVOID_CARS;

                float NewSpeed = (float)pNewVehicle->m_autoPilot.CruiseSpeed + 10.0f;
                pNewVehicle->m_autoPilot.CruiseSpeed = (uint8)NewSpeed;
                pNewVehicle->m_vecMoveSpeed = pNewVehicle->GetForward() * NewSpeed * 0.02f;

                if (CGameLogic::LaRiotsActiveHere() || CCheat::m_aCheatsActive[CHEAT_AGGRESSIVE_DRIVERS])
                {
                    if (pNewVehicle->m_pDriver != nullptr)
                    {
                        pNewVehicle->m_pDriver->bNeverEverTargetThisPed = true;
                    }
                }

                pNewVehicle->vehicleFlags.bMadDriver = true;
            }
        }
        else if (CarRating == 13 || CarRating == 24)
        {
            CCarAI::AddPoliceCarOccupants(pNewVehicle, false);
        }
        else
        {
            bCarIsBeingCreated = true;
            SetUpDriverAndPassengersForVehicle(pNewVehicle, CarRating, 0, false, false, 99);
            bCarIsBeingCreated = false;
        }

        if (CarRating == 13 || CarRating == 24)
        {
            pNewVehicle->ChangeLawEnforcerState(true);
        }

        CStreaming::PossiblyStreamCarOutAfterCreation(pNewVehicle->m_nModelIndex);
        CModelInfo::GetModelInfo(pNewVehicle->m_nModelIndex)->AddRef();
        return;
    }
    delete pNewVehicle;
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
        switch (veh.m_autoPilot.Mission) {
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
    switch (vehicle->m_autoPilot.Mission) {
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
        JoinCarWithRoadSystemGotoCoors(vehicle, vehicle->m_autoPilot.TargetCoors, true, vehicle->IsSubBoat());
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
        JoinCarWithRoadSystemGotoCoors(vehicle, vehicle->m_autoPilot.pTargetEntity->GetPosition(), true, vehicle->IsSubBoat());
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
            pAuto->m_autoPilot.TargetCoors = posn;
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
void CCarCtrl::SetUpDriverAndPassengersForVehicle(CVehicle* pNewVehicle, int CarRating, int MinPassengers, bool bMustBeMale, bool bCriminal, int MaxPassengers)
{

    pNewVehicle->SetUpDriver(CarRating, bMustBeMale, bCriminal);

    if (CarRating >= 14 && CarRating <= 23 && CGeneral::GetRandomNumber() < 16383)
    {
        pNewVehicle->m_pDriver->GiveObjectToPedToHold(ModelIndices::MI_GANG_SMOKE, true);
    }

    int MaxToCreate = MaxPassengers;
    if (pNewVehicle->m_nMaxPassengers < MaxPassengers)
    {
        MaxToCreate = pNewVehicle->m_nMaxPassengers;
    }

    int NumPassengers = MinPassengers;

    if (MinPassengers < MaxToCreate)
    {
        for (int C = MaxToCreate - MinPassengers; C != 0; --C)
        {
            NumPassengers += CGeneral::GetRandomNumberInRange(0.0f, 1.0f) < 0.125f;
        }

        if (NumPassengers >= MaxToCreate)
        {
            NumPassengers = MaxToCreate;
        }
    }
    else
    {
        NumPassengers = MaxToCreate;
    }

    if (CModelInfo::IsCarModel(pNewVehicle->GetModelIndex()))
    {
        int AnimationBlockIndex = CAnimManager::GetAnimationBlockIndex("van");

        if (CModelInfo::GetModelInfo(pNewVehicle->GetModelIndex())->GetAnimFileIndex() == AnimationBlockIndex && NumPassengers >= 1)
        {
            NumPassengers = 1;
        }
    }

    for (int C = 0; C < NumPassengers; ++C)
    {
        CPed* pPassenger = pNewVehicle->SetupPassenger(C, CarRating, bMustBeMale, bCriminal);

        if (pPassenger != nullptr)
        {
            // pPassenger.UpdateStatEnteringVehicle(); NOP

            if (CarRating >= 14 && CarRating <= 23 && CGeneral::GetRandomNumber() < 16383)
            {
                pPassenger->GiveObjectToPedToHold(ModelIndices::MI_GANG_SMOKE, true);
            }
        }
    }
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
            autoPilot.MaxSpeedBuffer = 0.0f;
        } else {
            autoPilot.MaxSpeedBuffer = FindMaximumSpeedForThisCarInTraffic(vehicle);
        }
    }

    if (autoPilot.MaxSpeedBuffer >= autoPilot.ActualSpeed) {
        autoPilot.ModifySpeed(std::min(autoPilot.MaxSpeedBuffer, CTimer::GetTimeStep() * 0.05f + autoPilot.ActualSpeed));
    } else if (autoPilot.ActualSpeed >= 0.1f) {
        autoPilot.ModifySpeed(std::max(autoPilot.MaxSpeedBuffer, autoPilot.ActualSpeed - CTimer::GetTimeStep() * 0.7f));
    } else if (autoPilot.ActualSpeed != 0.0f) {
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
