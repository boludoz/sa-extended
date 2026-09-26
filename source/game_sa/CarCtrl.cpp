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
#include "TaskComplexDriveWander.h"
#include "TaskComplexLeaveAnyCar.h"
#include "TaskComplexWander.h"
#include "EventAcquaintancePedHate.h"
#include "EventPotentialGetRunOver.h"

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

static bool TestForThisAngle(float Angle, CVector* pLine1Start, CVector* pLine1Delta, CVector* pLine2Start, CVector* pLine2Delta, CVector* pLine1Start_Other, CVector* pLine1Delta_Other, CVector* pLine2Start_Other, CVector* pLine2Delta_Other, float OtherCarMoveSpeedX, float OtherCarMoveSpeedY, float OurMoveSpeed, bool bSwapRound);
static bool FindHeightForVerticalAngle(CPlane* pPlane, float Angle, float Orientation, float* pResult);
static float FindFlightHeight(CPlane* pPlane, float Orientation);

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
    RH_ScopedInstall(ChooseModel, 0x424CE0);
    RH_ScopedInstall(FindIntersection2Lines, 0x4226F0);
    RH_ScopedInstall(FindPathDirection, 0x422090);
    RH_ScopedInstall(FindPercDependingOnDistToLink, 0x422620);
    RH_ScopedInstall(FindSequenceElement, 0x421770);
    RH_ScopedInstall(FindSpeedMultiplier, 0x4224E0);
    RH_ScopedInstall(FindGhostRoadHeight, 0x422370);
    RH_ScopedInstall(RegisterVehicleOfInterest, 0x423DE0);
    RH_ScopedInstall(StopCarIfNodesAreInvalid, 0x422590);
    RH_ScopedInstall(SwitchBetweenPhysicsAndGhost, 0x4222A0);
    RH_ScopedInstall(SwitchVehicleToRealPhysics, 0x423FC0);
    RH_ScopedInstall(ThisVehicleShouldTryNotToTurn, 0x421FE0);
    RH_ScopedInstall(UpdateCarCount, 0x424000);
    RH_ScopedInstall(CreateConvoy, 0x42C740);
    RH_ScopedInstall(CreatePoliceChase, 0x42C2B0);
    RH_ScopedInstall(DragCarToPoint, 0x42EC90);
    RH_ScopedInstall(FindAngleToWeaveThroughTraffic, 0x4325C0);
    RH_ScopedInstall(FindLinksToGoWithTheseNodes, 0x42B470);
    RH_ScopedInstall(FindMaximumSpeedForThisCarInTraffic, 0x434400);
    RH_ScopedInstall(FireHeliRocketsAtTarget, 0x42B270);
    RH_ScopedInstall(FlyAIHeliInCertainDirection, 0x429A70);
    RH_ScopedInstall(FlyAIHeliToTarget_FixedOrientation, 0x423940);
    RH_ScopedInstall(FlyAIPlaneInCertainDirection, 0x423000);
    RH_ScopedInstall(GenerateCarCreationCoors2, 0x424210);
    RH_ScopedInstall(GenerateEmergencyServicesCar, 0x42F9C0);
    RH_ScopedInstall(GenerateOneEmergencyServicesCar, 0x42B7D0);
    RH_ScopedInstall(GetAIHeliToAttackPlayer, 0x42F3C0);
    RH_ScopedInstall(GetAIHeliToFlyInDirection, 0x42A730);
    RH_ScopedInstall(GetAIPlaneToAttackPlayer, 0x429780);
    RH_ScopedInstall(GetAIPlaneToDoDogFight, 0x429890);
    RH_ScopedInstall(GetAIPlaneToDoDogFightAgainstPlayer, 0x42F370);
    RH_ScopedInstall(IsThisAnAppropriateNode, 0x42DAB0);
    RH_ScopedInstall(JoinCarWithRoadSystem, 0x42F5A0);
    RH_ScopedInstall(JoinCarWithRoadSystemGotoCoors, 0x42F870);
    RH_ScopedInstall(PickNextNodeAccordingStrategy, 0x432B10);
    RH_ScopedInstall(PickNextNodeRandomly, 0x42DE80);
    RH_ScopedInstall(ScanForPedDanger, 0x42CE40);
    RH_ScopedInstall(SetCoordsOfScriptCar, 0x4342A0);
    RH_ScopedInstall(SlowCarDownForOtherCar, 0x42D0E0);
    RH_ScopedInstall(SteerAIBoatWithPhysicsAttackingPlayer, 0x428DE0);
    RH_ScopedInstall(SteerAIBoatWithPhysicsCirclingPlayer, 0x429090);
    RH_ScopedInstall(SteerAIBoatWithPhysicsHeadingForTarget, 0x428BE0);
    RH_ScopedInstall(SteerAICarBlockingPlayerForwardAndBack, 0x422B20);
    RH_ScopedInstall(SteerAICarParkParallel, 0x433BA0);
    RH_ScopedInstall(SteerAICarParkPerpendicular, 0x433EA0);
    RH_ScopedInstall(SteerAICarWithPhysicsHeadingForTarget, 0x433280);
    RH_ScopedInstall(SteerAICarWithPhysicsTryingToBlockTarget, 0x4335E0);
    RH_ScopedInstall(SteerAICarWithPhysicsTryingToBlockTarget_Stop, 0x428990);
    RH_ScopedInstall(SteerAIHeliAsPoliceHeli, 0x42AAD0);
    RH_ScopedInstall(SteerAIHeliFlyingAwayFromPlayer, 0x42ACB0);
    RH_ScopedInstall(SteerAIHeliToCrashAndBurn, 0x4238E0);
    RH_ScopedInstall(SteerAIHeliToFollowEntity, 0x42A750);
    RH_ScopedInstall(SteerAIHeliToKeepEntityInView, 0x42AEB0);
    RH_ScopedInstall(SteerAIHeliToLand, 0x42AD30);
    RH_ScopedInstall(SteerAIHeliTowardsTargetCoors, 0x42A630);
    RH_ScopedInstall(SteerAIPlaneToCrashAndBurn, 0x423880);
    RH_ScopedInstall(SteerAIPlaneToFollowEntity, 0x4237F0);
    RH_ScopedInstall(SteerAIPlaneTowardsTargetCoors, 0x423790);
    RH_ScopedInstall(TestCollisionBetween2MovingRects, 0x425B30);
    RH_ScopedInstall(TestCollisionBetween2MovingRects_OnlyFrontBumper, 0x425F70);
    RH_ScopedInstall(TestWhetherToFirePlaneGuns, 0x429520);
    RH_ScopedInstall(TriggerDogFightMoves, 0x429300);
    RH_ScopedInstall(WeaveForObject, 0x426BC0);
    RH_ScopedInstall(WeaveForOtherCar, 0x426350);
    RH_ScopedInstall(WeaveForPed, 0x426970);
    RH_ScopedInstall(SlowCarDownForCarsSectorList<CPtrListDoubleLink<CVehicle*>>, 0x432420);
    RH_ScopedInstall(SlowCarDownForObjectsSectorList<CPtrListDoubleLink<CObject*>>, 0x42D4F0);
    RH_ScopedInstall(SlowCarDownForPedsSectorList<CPtrListDoubleLink<CPed*>>, 0x425440);
    RH_ScopedInstall(WeaveThroughCarsSectorList<CPtrListDoubleLink<CVehicle*>>, 0x42D680);
    RH_ScopedInstall(WeaveThroughObjectsSectorList<CPtrListDoubleLink<CObject*>>, 0x42D950);
    RH_ScopedInstall(WeaveThroughPedsSectorList<CPtrListDoubleLink<CPed*>>, 0x42D7E0);
    RH_ScopedGlobalInstall(TestForThisAngle, 0x421A50);
    RH_ScopedGlobalInstall(FindHeightForVerticalAngle, 0x422F00);
    RH_ScopedGlobalInstall(FindFlightHeight, 0x422F80);
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
// ASM Match: 86.6%
int32 CCarCtrl::ChooseModel(int32* carRating) {
    const auto total = CPopCycle::m_NumOther_Cars + CPopCycle::m_NumCops_Cars + CPopCycle::m_NumGangs_Cars + CPopCycle::m_NumDealers_Cars;
    if (total <= 0.0f || CCheat::IsActive(WASTELAND_CHEAT) && CGeneral::GetRandomNumberInRange(0, 100) != 0) {
        return -1;
    }

    const auto random = CGeneral::GetRandomNumberInRange(0.0f, 1.0f);
    int32 modelId = 0;

    if (CPopCycle::m_NumDealers_Cars / total > random && !CCheat::IsActive(GANGLAND_CHEAT)) {
        *carRating = 25;
        modelId = CPopulation::m_CarGroups[28][0];
        return CStreaming::IsModelLoaded(modelId) ? modelId : -1;
    }

    if ((CPopCycle::m_NumGangs_Cars + CPopCycle::m_NumDealers_Cars) / total > random || CCheat::IsActive(GANGLAND_CHEAT)) {
        if (CPopulation::m_bDontCreateRandomGangMembers) {
            return -1;
        }

        int32 gang = 0;
        int32 totalGangStrength = 0;
        for (auto i = 0; i < TOTAL_GANGS; ++i) {
            totalGangStrength += CPopCycle::m_pCurrZoneInfo->GangStrength[i];
        }
        if (totalGangStrength <= 0) {
            return -1;
        }

        auto randomG = CGeneral::GetRandomNumberInRange(0, totalGangStrength);
        int32 c = 0;
        while (CPopCycle::m_pCurrZoneInfo->GangStrength[c] < randomG) {
            randomG -= CPopCycle::m_pCurrZoneInfo->GangStrength[c++];
        }

        if (CCheat::IsActive(GANGLAND_CHEAT)) {
            gang = CGeneral::GetRandomNumberInRange(0, 9);
        }
        *carRating = gang + 14;
        if (CPopulation::PickGangCar((eGangID)gang) < 0) {
            return -1;
        }

        for (c = 0; c < TOTAL_GANGS; ++c) {
            int32 r;
            do {
                r = CGeneral::GetRandomNumberInRange(0, 23);
            } while (CPopulation::m_CarGroups[gang + 18][r] == 2000);

            modelId = CPopulation::m_CarGroups[gang + 18][r];
            if (CStreaming::IsModelLoaded(modelId)) {
                return modelId;
            }
        }
        return -1;
    }

    if ((CPopCycle::m_NumCops_Cars + CPopCycle::m_NumGangs_Cars + CPopCycle::m_NumDealers_Cars) / total > random) {
        if (!CGangWars::GangWarFightingGoingOn() && !CPopulation::m_bDontCreateRandomCops) {
            *carRating = 13;
            return ChoosePoliceCarModel(0);
        }
        return -1;
    }

    *carRating = 0;
    if (CTheScripts::ForceRandomCarModel == -1) {
        return CPopulation::m_AppropriateLoadedCars.PickRandomCar(true, false);
    }
    return CTheScripts::ForceRandomCarModel;
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
        vehicle->AsTrain()->m_nTrainFlags.bDerailed = true;
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
// ASM Match: 89.3%
bool CCarCtrl::CreateConvoy(CVehicle* pLeader, int32 CarRating)
{
    int32 numVehiclesInConvoy;
    int32 veh;
    CVehicle* pNewConvoyMember;
    CVector newPos;
    float ClosestZ;
    CColPoint TestColPoint;
    CEntity* pTestEntity;
    CEntity* pLastVehicleInConvoy;
    int16 Num;
    bool bManagedToCreateConvoy;

    pLastVehicleInConvoy = pLeader;
    bManagedToCreateConvoy = false;

    int32 rand = CGeneral::GetRandomNumberInRange(0, 100);

    float radiusOfLeader = CModelInfo::GetModelInfo(pLeader->GetModelIndex())->GetColModel()->GetBoundRadius();

    pLeader->ForcedRandomSeed = pLeader->RandomSeed;

    numVehiclesInConvoy = (rand > 50) + 2;

    for (veh = 0; veh < numVehiclesInConvoy; ++veh)
    {
        pNewConvoyMember = GetNewVehicleDependingOnCarModel(pLeader->GetModelIndex(), RANDOM_VEHICLE);
        if (pNewConvoyMember == nullptr)
        {
            continue;
        }

        newPos = pLeader->GetPosition() - pLeader->GetMatrix().GetForward() * static_cast<float>(veh + 1) * (radiusOfLeader + radiusOfLeader + 2.5f);

        ClosestZ = 1000000000.0f;

        if (CWorld::ProcessVerticalLine(newPos, 1000.0f, TestColPoint, pTestEntity, true, false, false, false, true, false, nullptr))
        {
            ClosestZ = TestColPoint.GetPosition().z;
        }

        if (CWorld::ProcessVerticalLine(newPos, -1000.0f, TestColPoint, pTestEntity, true, false, false, false, true, false, nullptr))
        {
            if (VABS(TestColPoint.GetPosition().z - newPos.z) < VABS(ClosestZ - newPos.z))
            {
                ClosestZ = TestColPoint.GetPosition().z;
            }
        }

        if (ClosestZ > 1000000000.0f)
        {
            delete pNewConvoyMember;
            continue;
        }

        newPos.z = pNewConvoyMember->GetHeightAboveRoad() + ClosestZ;

        CWorld::FindObjectsKindaColliding(newPos, radiusOfLeader, true, &Num, 2, nullptr, false, true, true, false, false);

        if (Num != 0)
        {
            delete pNewConvoyMember;
            continue;
        }

        if (!bManagedToCreateConvoy)
        {
            pLeader->SetStatus(STATUS_PHYSICS);
            pLeader->m_autoPilot.DrivingMode = DRIVING_STYLE_AVOID_CARS;
            pLeader->m_vecMoveSpeed = pLeader->GetForward() * pLeader->m_autoPilot.CruiseSpeed * 0.02f;
        }

        pNewConvoyMember->GetMatrix() = pLeader->GetMatrix();
        pNewConvoyMember->SetPosition(newPos);
        pNewConvoyMember->m_vecMoveSpeed = pLeader->m_vecMoveSpeed;
        pNewConvoyMember->SetStatus(pLeader->GetStatus());
        pNewConvoyMember->vehicleFlags.bPartOfAttackWave = true;

        CWorld::Add(pNewConvoyMember);

        if (pNewConvoyMember->GetBaseVehicleType() == VEHICLE_TYPE_BIKE)
        {
            static_cast<CBike*>(pNewConvoyMember)->PlaceOnRoadProperly();
        }
        else if (pNewConvoyMember->GetBaseVehicleType() == VEHICLE_TYPE_AUTOMOBILE)
        {
            static_cast<CAutomobile*>(pNewConvoyMember)->PlaceOnRoadProperly();
        }

        SetUpDriverAndPassengersForVehicle(pNewConvoyMember, CarRating, 0, true, false, 99);

        pNewConvoyMember->m_pDriver->GetPedIntelligence()->GetTaskManager().SetTask(new CTaskComplexCarDriveWander(pNewConvoyMember, DRIVING_STYLE_STOP_FOR_CARS, 10.0f), TASK_PRIMARY_PRIMARY, false);

        pNewConvoyMember->m_autoPilot.CruiseSpeed = pLeader->m_autoPilot.CruiseSpeed;

        pNewConvoyMember->m_autoPilot.ActualSpeed = pLeader->m_autoPilot.ActualSpeed - static_cast<float>(veh);
        if (pNewConvoyMember->m_autoPilot.ActualSpeed < 0.0f)
        {
            pNewConvoyMember->m_autoPilot.ActualSpeed = 0.0f;
        }

        pNewConvoyMember->m_autoPilot.DrivingMode = pLeader->m_autoPilot.DrivingMode;
        pNewConvoyMember->m_autoPilot.Mission = pLeader->m_autoPilot.Mission;
        pNewConvoyMember->RandomSeed = pLeader->RandomSeed;
        pNewConvoyMember->ForcedRandomSeed = pLeader->ForcedRandomSeed;

        pNewConvoyMember->m_autoPilot.pTargetEntity = pLastVehicleInConvoy;
        pLastVehicleInConvoy->RegisterReference(&pNewConvoyMember->m_autoPilot.pTargetEntity);

        pLastVehicleInConvoy = pNewConvoyMember;
        bManagedToCreateConvoy = true;
    }

    return bManagedToCreateConvoy;
}

// 0x42C2B0
// ASM Match: 80.4%
bool CCarCtrl::CreatePoliceChase(CVehicle* pLeader, int32 CarRating, CNodeAddress FromNode)
{
    int32 CarModel;
    CVehicle* pNewCopCar;
    CVector copPos;
    float ClosestZ;
    CColPoint TestColPoint;
    CEntity* pTestEntity;
    int16 Num;
    float boundRadius;

    if (CPopCycle::m_NumGangs_Cars != 0.0f && (CarRating < 14 || CarRating > 23))
    {
        if (CGeneral::GetRandomNumberInRange(0, 4) != 0)
        {
            return false;
        }
    }

    CarModel = ChoosePoliceCarModel(1);
    if (CarModel <= -1)
    {
        return false;
    }

    if (!CStreaming::IsModelLoaded(CarModel))
    {
        return false;
    }

    pNewCopCar = GetNewVehicleDependingOnCarModel(CarModel, RANDOM_VEHICLE);
    if (pNewCopCar == nullptr)
    {
        return false;
    }

    copPos = ThePaths.FindNodePointer(FromNode)->GetCoors();

    ClosestZ = 1000000000.0f;

    if (CWorld::ProcessVerticalLine(copPos, 1000.0f, TestColPoint, pTestEntity, true, false, false, false, true, false, nullptr))
    {
        ClosestZ = TestColPoint.GetPosition().z;
    }

    if (CWorld::ProcessVerticalLine(copPos, -1000.0f, TestColPoint, pTestEntity, true, false, false, false, true, false, nullptr))
    {
        if (VABS(TestColPoint.GetPosition().z - copPos.z) < VABS(ClosestZ - copPos.z))
        {
            ClosestZ = TestColPoint.GetPosition().z;
        }
    }

    if (ClosestZ > 1000000000.0f)
    {
        delete pNewCopCar;
        return false;
    }

    copPos.z = pNewCopCar->GetHeightAboveRoad() + ClosestZ;

    boundRadius = CModelInfo::GetModelInfo(pNewCopCar->GetModelIndex())->GetColModel()->GetBoundRadius();

    if (!pLeader->GetIsOnScreen())
    {
        if (TheCamera.IsSphereVisible(copPos, boundRadius))
        {
            delete pNewCopCar;
            return false;
        }
    }

    Num = 0;
    CWorld::FindObjectsKindaColliding(copPos, boundRadius, true, &Num, 2, nullptr, false, true, true, false, false);

    if (Num != 0)
    {
        delete pNewCopCar;
        return false;
    }

    pLeader->SetStatus(STATUS_PHYSICS);
    pLeader->m_autoPilot.DrivingMode = DRIVING_STYLE_AVOID_CARS;

    float NewSpeed = pLeader->m_autoPilot.CruiseSpeed + 10.0f;
    pLeader->vehicleFlags.bMadDriver = true;
    pLeader->m_autoPilot.CruiseSpeed = static_cast<uint8>(NewSpeed);
    pLeader->m_vecMoveSpeed = pLeader->GetForward() * NewSpeed * 0.02f;

    SetUpDriverAndPassengersForVehicle(pLeader, CarRating, 2, true, true, 99);

    pNewCopCar->GetMatrix() = pLeader->GetMatrix();
    pNewCopCar->SetPosition(copPos);
    pNewCopCar->m_vecMoveSpeed = pLeader->m_vecMoveSpeed;
    pNewCopCar->SetStatus(STATUS_PHYSICS);
    pNewCopCar->vehicleFlags.bIsLawEnforcer = true;

    pNewCopCar->ChangeLawEnforcerState(true);

    CWorld::Add(pNewCopCar);

    if (pNewCopCar->GetBaseVehicleType() == VEHICLE_TYPE_BIKE)
    {
        static_cast<CBike*>(pNewCopCar)->PlaceOnRoadProperly();
    }
    else
    {
        static_cast<CAutomobile*>(pNewCopCar)->PlaceOnRoadProperly();
    }

    CCarAI::AddPoliceCarOccupants(pNewCopCar, true);

    pLeader->m_pDriver->bNeverEverTargetThisPed = true;

    for (int32 C = 0; C < pLeader->m_nNumPassengers; ++C)
    {
        if (pLeader->m_apPassengers[C] != nullptr)
        {
            pLeader->m_apPassengers[C]->bNeverEverTargetThisPed = true;
        }
    }

    pNewCopCar->m_pDriver->GetPedIntelligence()->GetTaskManager().SetTask(new CTaskComplexCarDriveWander(pNewCopCar, DRIVING_STYLE_STOP_FOR_CARS, 10.0f), TASK_PRIMARY_PRIMARY, false);

    CEventAcquaintancePedHate event(pLeader->m_pDriver);
    pNewCopCar->m_pDriver->GetPedIntelligence()->AddEvent(event, false);

    pNewCopCar->vehicleFlags.bNeverUseSmallerRemovalRange = true;
    pNewCopCar->m_nExtendedRemovalRange = 255;

    return true;
}

// 0x428040
bool CCarCtrl::DealWithBend_Racing(CVehicle* vehicle, CCarPathLinkAddress LinkAddress1, CCarPathLinkAddress LinkAddress2, CCarPathLinkAddress LinkAddress3, CCarPathLinkAddress LinkAddress4, char arg6, char arg7, char arg8, char arg9, float arg10, float* arg11, float* arg12, float* arg13, float* arg14, CVector* pos) {
    return plugin::CallAndReturn<bool, 0x428040, CVehicle*, CCarPathLinkAddress, CCarPathLinkAddress, CCarPathLinkAddress, CCarPathLinkAddress, int8, int8, int8, int8, float, float*, float*, float*, float*, CVector*>(vehicle, LinkAddress1, LinkAddress2, LinkAddress3, LinkAddress4, arg6, arg7, arg8, arg9, arg10, arg11, arg12, arg13, arg14, pos);
}

// 0x42EC90
// ASM Match: 45.0%
void CCarCtrl::DragCarToPoint(CVehicle* pVehicle, CVector* pCoors)
{
    CColModel* pColModel = &CModelInfo::GetColModel(pVehicle->GetModelIndex());
    float vehicleLength = (pColModel->GetBoundBox().m_vecMax.y - pColModel->GetBoundBox().m_vecMin.y) * 0.95f;
    float searchRadius = (pVehicle->m_autoPilot.FramesFloating <= 16) ? 3.0f : 100.0f;

    CMatrix& matrix = pVehicle->GetMatrix();
    CVector oldForward = matrix.GetForward();
    CVector position = matrix.GetTranslate();

    float halfLength = vehicleLength * 0.5f;
    float rearX = position.x - (halfLength * oldForward.x);
    float rearY = position.y - (halfLength * oldForward.y);

    CColLine line(CVector(rearX, rearY, 0.0f), CVector(rearX + oldForward.x * 1.5f, rearY + oldForward.y * 1.5f, 0.0f));

    CColSphere targetSphere;
    targetSphere.m_fRadius = vehicleLength;
    targetSphere.m_vecCenter = CVector(pCoors->x, pCoors->y, 0.0f);

    CColPoint colPoint;
    float ratio = 1.0f;
    float rearIntersectionX, rearIntersectionY;

    if (CCollision::ProcessLineSphere(line, targetSphere, colPoint, ratio))
    {
        rearIntersectionX = colPoint.GetPosition().x;
        rearIntersectionY = colPoint.GetPosition().y;
    }
    else
    {
        float deltaX = pCoors->x - rearX;
        float deltaY = pCoors->y - rearY;
        float dist = std::sqrt(deltaY * deltaY + deltaX * deltaX);
        float r = vehicleLength / dist;
        rearIntersectionX = pCoors->x + (rearX - pCoors->x) * r;
        rearIntersectionY = pCoors->y + (rearY - pCoors->y) * r;
    }

    float halfHeightOffset = halfLength * oldForward.z;
    float frontCenterZ = position.z + halfHeightOffset;
    float frontLowZ = frontCenterZ - searchRadius;
    float frontHighZ = frontCenterZ + searchRadius;

    pVehicle->m_autoPilot.FramesFloating++;

    CVector testPos(pCoors->x, pCoors->y, frontHighZ);
    float frontGroundZ;

    if (CCollision::IsStoredPolyStillValidVerticalLine(testPos, frontLowZ, colPoint, pVehicle->m_StoredCollPolys[0]))
    {
        frontGroundZ = colPoint.GetPosition().z;
    }
    else
    {
        testPos.z = frontCenterZ + 1.5f;
        CEntity* colEntity = nullptr;
        bool bHit = CWorld::ProcessVerticalLine(testPos, frontCenterZ - 2.0f, colPoint, colEntity, true, false, false, false, false, false, &pVehicle->m_StoredCollPolys[0]);
        if (!bHit)
        {
            testPos.z = frontHighZ;
            bHit = CWorld::ProcessVerticalLine(testPos, frontLowZ, colPoint, colEntity, true, false, false, false, false, false, &pVehicle->m_StoredCollPolys[0]);
        }

        if (bHit)
        {
            frontGroundZ = colPoint.GetPosition().z;
            pVehicle->m_pEntityWeAreOn = colEntity;

            if (colEntity)
            {
                uint32& vehicleFlags = *reinterpret_cast<uint32*>(&pVehicle->m_nFlags);
                uint32& entityFlags = *reinterpret_cast<uint32*>(&colEntity->m_nFlags);
                vehicleFlags = (entityFlags ^ vehicleFlags) & 0x40000000 ^ vehicleFlags;
                vehicleFlags = entityFlags ^ (vehicleFlags ^ entityFlags) & 0x7FFFFFFF;
            }

            pVehicle->m_autoPilot.FramesFloating = 0;
        }
        else
        {
            frontGroundZ = pVehicle->LastFrontHeight;
        }
    }
    pVehicle->LastFrontHeight = frontGroundZ;

    // Rear logic
    float rearCenterZ = position.z - halfHeightOffset;
    float rearLowZ = rearCenterZ - searchRadius;
    float rearHighZ = rearCenterZ + searchRadius;

    testPos.x = rearIntersectionX;
    testPos.y = rearIntersectionY;
    testPos.z = rearHighZ;

    float rearGroundZ;
    if (CCollision::IsStoredPolyStillValidVerticalLine(testPos, rearLowZ, colPoint, pVehicle->m_StoredCollPolys[1]))
    {
        rearGroundZ = colPoint.GetPosition().z;
    }
    else
    {
        testPos.z = rearCenterZ + 1.5f;
        CEntity* colEntity = nullptr;
        bool bHit = CWorld::ProcessVerticalLine(testPos, rearCenterZ - 2.0f, colPoint, colEntity, true, false, false, false, false, false, &pVehicle->m_StoredCollPolys[1]);
        if (!bHit)
        {
            testPos.z = rearHighZ;
            bHit = CWorld::ProcessVerticalLine(testPos, rearLowZ, colPoint, colEntity, true, false, false, false, false, false, &pVehicle->m_StoredCollPolys[1]);
        }

        if (bHit)
        {
            rearGroundZ = colPoint.GetPosition().z;
            pVehicle->m_pEntityWeAreOn = colEntity;

            if (colEntity)
            {
                uint32& vehicleFlags = *reinterpret_cast<uint32*>(&pVehicle->m_nFlags);
                uint32& entityFlags = *reinterpret_cast<uint32*>(&colEntity->m_nFlags);
                vehicleFlags = (entityFlags ^ vehicleFlags) & 0x40000000 ^ vehicleFlags;
                vehicleFlags = entityFlags ^ (vehicleFlags ^ entityFlags) & 0x7FFFFFFF;
            }

            pVehicle->m_autoPilot.FramesFloating = 0;
        }
        else
        {
            rearGroundZ = pVehicle->LastRearHeight;
        }
    }
    pVehicle->LastRearHeight = rearGroundZ;

    float invLength = 1.0f / vehicleLength;
    float pitch = std::atan2((frontGroundZ - rearGroundZ) * invLength, 1.0f);
    float cosPitch = std::cos(pitch);
    float sinPitch = std::sin(pitch);

    matrix.GetRight().x = (pCoors->y - rearIntersectionY) * invLength;
    matrix.GetRight().y = -(pCoors->x - rearIntersectionX) * invLength;
    matrix.GetRight().z = 0.0f;

    matrix.GetForward().x = -cosPitch * matrix.GetRight().y;
    matrix.GetForward().y = cosPitch * matrix.GetRight().x;
    matrix.GetForward().z = sinPitch;

    matrix.GetUp() = CrossProduct(matrix.GetRight(), matrix.GetForward());

    float avgX = (pCoors->x + rearIntersectionX) * 0.5f;
    float avgY = (pCoors->y + rearIntersectionY) * 0.5f;
    float avgZ = (frontGroundZ + rearGroundZ) * 0.5f + pVehicle->GetHeightAboveRoad();

    matrix.SetTranslateOnly(CVector(avgX, avgY, avgZ));
    matrix.UpdateRW();

    pVehicle->UpdateLightingFromStoredPolys();

    CVector cross = CrossProduct(matrix.GetForward(), oldForward);
    float steer = -cross.z * 10.0f;
    pVehicle->m_fSteerAngle = std::clamp(steer, -0.5f, 0.5f);
}

// 0x4325C0
// ASM Match: 99.4%
float CCarCtrl::FindAngleToWeaveThroughTraffic(CVehicle* pVehicle, CPhysical* pException, float Direction, float CarOrientation, float CheckDistMult)
{
    float MinX, MinY, MaxX, MaxY;
    int32 nTop, nRight, nBottom, nLeft;
    int32 LoopY, LoopX;
    float RightAngle, LeftAngle, OrientationDiff, Diff1, Diff2;
    float LeftAngleOld, RightAngleOld;
    float Speed, SpeedMult;

    Speed = CMaths::Sqrt(pVehicle->m_vecMoveSpeed.x * pVehicle->m_vecMoveSpeed.x + pVehicle->m_vecMoveSpeed.y * pVehicle->m_vecMoveSpeed.y);
    SpeedMult = VMIN(Speed * 2.5f + 1.0f, 2.0f);

    MinX = pVehicle->GetPosition().x - SpeedMult * CheckDistMult * 12.0f;
    MaxX = pVehicle->GetPosition().x + SpeedMult * CheckDistMult * 12.0f;
    MinY = pVehicle->GetPosition().y - SpeedMult * CheckDistMult * 12.0f;
    MaxY = pVehicle->GetPosition().y + SpeedMult * CheckDistMult * 12.0f;

    nLeft = VMAX(static_cast<int32>(CMaths::Floor(MinX * 0.02f + 60.0f)), 0);
    nBottom = VMAX(static_cast<int32>(CMaths::Floor(MinY * 0.02f + 60.0f)), 0);
    nRight = VMIN(static_cast<int32>(CMaths::Floor(MaxX * 0.02f + 60.0f)), 119);
    nTop = VMIN(static_cast<int32>(CMaths::Floor(MaxY * 0.02f + 60.0f)), 119);

    RightAngle = Direction;
    LeftAngle = Direction;

    CWorld::AdvanceCurrentScanCode();

    LeftAngleOld = -9999.9f;
    RightAngleOld = -9999.9f;

    while (LeftAngleOld != LeftAngle || RightAngleOld != RightAngle)
    {
        RightAngleOld = RightAngle;
        LeftAngleOld = LeftAngle;

        for (LoopY = nBottom; LoopY <= nTop; LoopY++)
        {
            for (LoopX = nLeft; LoopX <= nRight; LoopX++)
            {
                CRepeatSector& rsector = CWorld::GetRepeatSector(LoopX, LoopY);

                WeaveThroughCarsSectorList(rsector.Vehicles, pVehicle, pException, MinX, MinY, MaxX, MaxY, &LeftAngle, &RightAngle);
                if (pVehicle->m_autoPilot.DrivingMode != DRIVING_STYLE_DRIVINGMODE_AVOIDCARS_STOPFORPEDS_OBEYLIGHTS)
                {
                    WeaveThroughPedsSectorList(rsector.Peds, pVehicle, pException, MinX, MinY, MaxX, MaxY, &LeftAngle, &RightAngle);
                }
                WeaveThroughObjectsSectorList(rsector.Objects, pVehicle, MinX, MinY, MaxX, MaxY, &LeftAngle, &RightAngle);
                WeaveThroughCarsSectorList(rsector.Vehicles, pVehicle, pException, MinX, MinY, MaxX, MaxY, &LeftAngle, &RightAngle);
            }
        }
    }

    OrientationDiff = CarOrientation - Direction;
    while (OrientationDiff < -PI)
    {
        OrientationDiff += TWO_PI;
    }
    while (OrientationDiff > PI)
    {
        OrientationDiff -= TWO_PI;
    }

    Direction += OrientationDiff * 0.5f;
    while (Direction < -PI)
    {
        Direction += TWO_PI;
    }
    while (Direction > PI)
    {
        Direction -= TWO_PI;
    }

    Diff1 = LeftAngle - Direction;
    Diff2 = RightAngle - Direction;
    while (Diff1 < -PI)
    {
        Diff1 += TWO_PI;
    }
    while (Diff1 > PI)
    {
        Diff1 -= TWO_PI;
    }
    Diff1 = VABS(Diff1);
    while (Diff2 < -PI)
    {
        Diff2 += TWO_PI;
    }
    while (Diff2 > PI)
    {
        Diff2 -= TWO_PI;
    }
    Diff2 = VABS(Diff2);

    if (Diff1 > HALF_PI && Diff2 > HALF_PI)
    {
        return Direction;
    }
    if (VABS(Diff1 - Diff2) < 0.08f)
    {
        return RightAngle;
    }
    if (Diff1 < Diff2)
    {
        return LeftAngle;
    }
    return RightAngle;
}

// 0x4226F0
// ASM Match: not measured
void CCarCtrl::FindIntersection2Lines(float center1X, float center1Y, float dir1X, float dir1Y, float center2X, float center2Y, float dir2X, float dir2Y, float* outX, float* outY) {
    const auto dist   = dir1X * dir2Y - dir1Y * dir2X;
    const auto change = dist != 0.0f ? ((center2X - center1X) * dir2Y - (center2Y - center1Y) * dir2X) / dist : 0.0f;

    *outX = dir1X * change + center1X;
    *outY = dir1Y * change + center1Y;
}

// 0x42B470
// ASM Match: 60.4%
void CCarCtrl::FindLinksToGoWithTheseNodes(CVehicle* pVehicle)
{
    int32 RandomLinkIndex;
    CCarPathLinkAddress RandomLink;
    int16 Link;

    if (pVehicle->ForcedRandomSeed)
    {
        CGeneral::SetRandomSeed(pVehicle->ForcedRandomSeed);
    }

    CNodeAddress OldNode = pVehicle->m_autoPilot.OldNode;
    for (Link = 0; Link < 12; Link++)
    {
        if (!(ThePaths.pAdjacentNodes[OldNode.GetRegion()][ThePaths.FindNodePointer(OldNode)->IndexAdjacentNodes + Link] != pVehicle->m_autoPilot.NewNode))
        {
            break;
        }
    }
    pVehicle->m_autoPilot.NewLink = ThePaths.pAdjacentLinks[OldNode.GetRegion()][ThePaths.FindNodePointer(OldNode)->IndexAdjacentNodes + Link];

    if (pVehicle->m_autoPilot.OldNode < pVehicle->m_autoPilot.NewNode)
    {
        pVehicle->m_autoPilot.InvertDirNewLink = -1;
    }
    else
    {
        pVehicle->m_autoPilot.InvertDirNewLink = 1;
    }

    if (ThePaths.FindNodePointer(OldNode)->NumberAdjNodes == 1)
    {
        RandomLinkIndex = 0;
    }
    else
    {
        RandomLinkIndex = -1;
        float BestDist = 999999.88f;
        for (int32 C = 0; C < ThePaths.FindNodePointer(pVehicle->m_autoPilot.OldNode)->NumberAdjNodes; C++)
        {
            CNodeAddress TestNode = ThePaths.pAdjacentNodes[pVehicle->m_autoPilot.OldNode.GetRegion()][ThePaths.FindNodePointer(pVehicle->m_autoPilot.OldNode)->IndexAdjacentNodes + C];
            if (TestNode != pVehicle->m_autoPilot.NewNode && ThePaths.IsRegionLoaded(TestNode))
            {
                CVector T;
                ThePaths.FindNodePointer(pVehicle->m_autoPilot.OldNode)->GetCoors(T);
                CVector T2;
                ThePaths.FindNodePointer(TestNode)->GetCoors(T2);
                float TestDist = CCollision::DistToLine(T, T2, pVehicle->GetPosition());
                if (TestDist < BestDist)
                {
                    BestDist = TestDist;
                    RandomLinkIndex = C;
                }
            }
        }
        if (RandomLinkIndex < 0)
        {
            RandomLinkIndex = 0;
        }
        OldNode = pVehicle->m_autoPilot.OldNode;
    }

    pVehicle->m_autoPilot.OldLink = ThePaths.pAdjacentLinks[OldNode.GetRegion()][ThePaths.FindNodePointer(OldNode)->IndexAdjacentNodes + RandomLinkIndex];
    if (ThePaths.pAdjacentNodes[OldNode.GetRegion()][ThePaths.FindNodePointer(OldNode)->IndexAdjacentNodes + RandomLinkIndex] < pVehicle->m_autoPilot.OldNode)
    {
        pVehicle->m_autoPilot.InvertDirOldLink = -1;
    }
    else
    {
        pVehicle->m_autoPilot.InvertDirOldLink = 1;
    }
}

// 0x434400
// ASM Match: 99.7%
float CCarCtrl::FindMaximumSpeedForThisCarInTraffic(CVehicle* pVehicle)
{
    float MinX, MinY, MaxX, MaxY, MaxSpeed;
    int32 nTop, nRight, nBottom, nLeft;
    int32 LoopY, LoopX;

    if (pVehicle->m_autoPilot.DrivingMode == DRIVING_STYLE_AVOID_CARS || pVehicle->m_autoPilot.DrivingMode == DRIVING_STYLE_PLOUGH_THROUGH ||
        pVehicle->m_autoPilot.DrivingMode == DRIVING_STYLE_DRIVINGMODE_AVOIDCARS_OBEYLIGHTS)
    {
        return pVehicle->m_autoPilot.CruiseSpeed * pVehicle->m_autoPilot.SpeedMultiplier;
    }

    MinX = pVehicle->GetPosition().x - 14.0f;
    MaxX = pVehicle->GetPosition().x + 14.0f;
    MinY = pVehicle->GetPosition().y - 14.0f;
    MaxY = pVehicle->GetPosition().y + 14.0f;

    nLeft = VMAX(static_cast<int32>(CMaths::Floor(MinX * 0.02f + 60.0f)), 0);
    nBottom = VMAX(static_cast<int32>(CMaths::Floor(MinY * 0.02f + 60.0f)), 0);
    nRight = VMIN(static_cast<int32>(CMaths::Floor(MaxX * 0.02f + 60.0f)), 119);
    nTop = VMIN(static_cast<int32>(CMaths::Floor(MaxY * 0.02f + 60.0f)), 119);

    CWorld::AdvanceCurrentScanCode();

    MaxSpeed = pVehicle->m_autoPilot.CruiseSpeed * pVehicle->m_autoPilot.SpeedMultiplier;

    for (LoopY = nBottom; LoopY <= nTop; LoopY++)
    {
        for (LoopX = nLeft; LoopX <= nRight; LoopX++)
        {
            CRepeatSector& rsector = CWorld::GetRepeatSector(LoopX, LoopY);

            if (pVehicle->m_autoPilot.DrivingMode != DRIVING_STYLE_DRIVINGMODE_AVOIDCARS_STOPFORPEDS_OBEYLIGHTS)
            {
                SlowCarDownForCarsSectorList(rsector.Vehicles, pVehicle, MinX, MinY, MaxX, MaxY, &MaxSpeed, pVehicle->m_autoPilot.CruiseSpeed * pVehicle->m_autoPilot.SpeedMultiplier);
            }
            SlowCarDownForPedsSectorList(rsector.Peds, pVehicle, MinX, MinY, MaxX, MaxY, &MaxSpeed, pVehicle->m_autoPilot.CruiseSpeed * pVehicle->m_autoPilot.SpeedMultiplier);
            SlowCarDownForObjectsSectorList(rsector.Objects, pVehicle, MinX, MinY, MaxX, MaxY, &MaxSpeed, pVehicle->m_autoPilot.CruiseSpeed * pVehicle->m_autoPilot.SpeedMultiplier);
        }
    }

    pVehicle->vehicleFlags.bWarnedPeds = true;

    if (pVehicle->m_autoPilot.DrivingMode == DRIVING_STYLE_STOP_FOR_CARS || pVehicle->m_autoPilot.DrivingMode == DRIVING_STYLE_STOP_FOR_CARS_IGNORE_LIGHTS)
    {
        return MaxSpeed;
    }
    return (pVehicle->m_autoPilot.CruiseSpeed * pVehicle->m_autoPilot.SpeedMultiplier + MaxSpeed) * 0.5f;
}

// 0x42BD20
void CCarCtrl::FindNodesThisCarIsNearestTo(CVehicle* vehicle, CNodeAddress& nodeAddress1, CNodeAddress& nodeAddress2) {
    plugin::Call<0x42BD20, CVehicle*, CNodeAddress&, CNodeAddress&>(vehicle, nodeAddress1, nodeAddress2);
}

// 0x422090
// ASM Match: 61.3%
int8 CCarCtrl::FindPathDirection(CNodeAddress oldNode, CNodeAddress currNode, CNodeAddress newNode, bool* outSharpTurn) {
    *outSharpTurn = false;

    if (oldNode.IsEmpty() || currNode.IsEmpty() || newNode.IsEmpty()) {
        return 0;
    }
    if (!ThePaths.IsAreaNodesAvailable(oldNode) || !ThePaths.IsAreaNodesAvailable(currNode) || !ThePaths.IsAreaNodesAvailable(newNode)) {
        return 0;
    }

    const auto currPos = ThePaths.GetPathNode(currNode)->GetPosition();
    const auto oldPos  = ThePaths.GetPathNode(oldNode)->GetPosition();
    const auto newPos  = ThePaths.GetPathNode(newNode)->GetPosition();

    auto deltaOldX = currPos.x - oldPos.x;
    auto deltaOldY = currPos.y - oldPos.y;
    auto deltaNewX = currPos.x - newPos.x;
    auto deltaNewY = currPos.y - newPos.y;

    auto length = std::sqrt(deltaOldX * deltaOldX + deltaOldY * deltaOldY);
    if (length <= 0.0f) {
        return 0;
    }
    deltaOldX /= length;
    deltaOldY /= length;

    length = std::sqrt(deltaNewX * deltaNewX + deltaNewY * deltaNewY);
    if (length <= 0.0f) {
        return 0;
    }
    deltaNewX /= length;
    deltaNewY /= length;

    const auto cross = deltaNewX * deltaOldY - deltaOldX * deltaNewY;
    const auto dot   = deltaOldX * deltaNewX + deltaOldY * deltaNewY;

    if (dot < -0.4f) {
        return 1;
    }
    if (dot > 0.4f) {
        *outSharpTurn = true;
    }
    return cross > 0.0f ? 4 : 2;
}

// 0x422620
// ASM Match: 99.3%
float CCarCtrl::FindPercDependingOnDistToLink(CVehicle* vehicle, CCarPathLinkAddress linkAddress) {
    const auto* link = &ThePaths.GetCarPathLink(linkAddress);
    const auto  pos  = vehicle->GetPosition();
    const auto  dist = std::sqrt(sq(link->GetNodeCoors().x - pos.x) + sq(link->GetNodeCoors().y - pos.y));

    if (dist < 5.0f) {
        return 0.5f;
    }
    if (dist < 15.0f) {
        return 0.5f + (dist - 5.0f) / 20.0f;
    }
    return 1.0f;
}

// 0x421770
// ASM Match: 98.7%
int32 CCarCtrl::FindSequenceElement(int32 element) {
    if (bSequenceOtherWay) {
        return (SequenceRandomOffset + element) % SequenceElements;
    }
    return (SequenceRandomOffset + SequenceElements - element) % SequenceElements;
}

// 0x4224E0
// ASM Match: 52.1%
float CCarCtrl::FindSpeedMultiplier(float angle, float maxAngle, float minAngle, float minSpeed) {
    while (angle < -PI) {
        angle += TWO_PI;
    }
    while (angle > PI) {
        angle -= TWO_PI;
    }
    if (angle < 0.0f) {
        angle = -angle;
    }

    const auto excess = std::max(angle - maxAngle, 0.0f);
    if (excess > minAngle - maxAngle) {
        return minSpeed;
    }
    return 1.0f - excess / (minAngle - maxAngle) * (1.0f - minSpeed);
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

// 0x422370
// ASM Match: 56.8%
float CCarCtrl::FindGhostRoadHeight(CVehicle* vehicle) {
    const auto& ap = vehicle->m_autoPilot;
    if (ap.OldNode.IsEmpty() || ap.NewNode.IsEmpty()) {
        return 0.0f;
    }
    if (!ThePaths.IsAreaNodesAvailable(ap.OldNode) || !ThePaths.IsAreaNodesAvailable(ap.NewNode)) {
        return 0.0f;
    }

    const auto oldPos = ThePaths.GetPathNode(ap.OldNode)->GetPosition();
    const auto newPos = ThePaths.GetPathNode(ap.NewNode)->GetPosition();

    const auto oldDist = (oldPos - vehicle->GetPosition()).Magnitude2D();
    const auto newDist = (newPos - vehicle->GetPosition()).Magnitude2D();

    return (oldDist * newPos.z + newDist * oldPos.z) / (oldDist + newDist);
}

// 0x42B270
// ASM Match: 99.7%
void CCarCtrl::FireHeliRocketsAtTarget(CAutomobile* pVeh, CEntity* pTarget)
{
    if ((pVeh->m_SelectedWeapon == 4 || pVeh->m_SelectedWeapon == 0) && CTimer::GetPreviousTimeInMS() / 250 != CTimer::GetTimeInMS() / 250)
    {
        if ((pVeh->GetPosition() - pTarget->GetPosition()).Magnitude() < 80.0f)
        {
            CVector T = pVeh->GetPosition() + pVeh->GetMatrix().GetForward();
            if (CCollision::DistToMathematicalLine(&pVeh->GetPosition(), &T, &pTarget->GetPosition()) < 7.0f)
            {
                bool bLeftRight = (CTimer::GetTimeInMS() / 250) & 1;
                CVector Origin = pVeh->GetPosition() + pVeh->GetMatrix().GetForward() * 4.0f;
                CVector Offset = pVeh->GetMatrix().GetRight() * 1.5f;
                if (bLeftRight)
                {
                    Offset = -Offset;
                }
                Origin += Offset;
                CProjectileInfo::AddProjectile(pVeh, WEAPON_ROCKET, Origin, 1.0f, &pVeh->GetMatrix().GetForward(), nullptr);
            }
        }
    }
}

// 0x429A70
// ASM Match: 90.2%
void CCarCtrl::FlyAIHeliInCertainDirection(CHeli* pHeli, float Orientation, float Distance, bool bSlowDownAtTarget)
{
    bool bNewsHeliIsClose = false;
    if (pHeli->m_autoPilot.Mission == MISSION_HELI_NEWS_BEHAVIOUR && pHeli->m_autoPilot.TargetReachedDist > Distance && pHeli->GetMoveSpeed().Magnitude2D() < 0.01f)
    {
        bNewsHeliIsClose = true;
        Orientation += HALF_PI;
    }

    uint32 TimeThingNew = (pHeli->RandomSeed + CTimer::GetTimeInMS()) % 500;
    uint32 TimeThingOld = (pHeli->RandomSeed + CTimer::GetPreviousTimeInMS()) % 500;
    if (TimeThingNew < TimeThingOld)
    {
        CColPoint colPoint;
        CEntity* pHitEntity;

        pHeli->m_DesiredHeight = pHeli->m_LowestFlightHeight;

        CVector Pos = pHeli->GetPosition() + pHeli->GetMoveSpeed() * 50.0f;
        CVector Forward(CMaths::Cos(Orientation), CMaths::Sin(Orientation), -1.0f);
        Forward.Normalise();
        CVector TargetPos = Pos + 60.0f * Forward;

        if (CWorld::ProcessLineOfSight(Pos, TargetPos, colPoint, pHitEntity, true, false, false, false, false, false, false, true))
        {
            if (bNewsHeliIsClose)
            {
                pHeli->m_DesiredHeight = CMaths::Max(pHeli->m_DesiredHeight, colPoint.GetPosition().z + pHeli->m_MinHeightAboveTerrain * 0.5f);
            }
            else
            {
                pHeli->m_DesiredHeight = CMaths::Max(pHeli->m_DesiredHeight, colPoint.GetPosition().z + pHeli->m_MinHeightAboveTerrain);
            }
        }

        Forward = CVector(CMaths::Cos(Orientation), CMaths::Sin(Orientation), 0.0f);

        CVector MoveVec = pHeli->GetMoveSpeed();
        float MSpeed = MoveVec.Magnitude2D();
        float ScanLength;
        if (pHeli->vehicleFlags.bIsRCVehicle)
        {
            ScanLength = MSpeed * 100.0f + 5.0f;
        }
        else
        {
            ScanLength = MSpeed * 100.0f + 30.0f;
        }

        pHeli->m_bStopFlyingForAWhile = true;
        TargetPos = Pos + ScanLength * Forward;

        if (CWorld::ProcessLineOfSight(Pos, TargetPos, colPoint, pHitEntity, true, false, false, false, false, false, false, true))
        {
            CVector HeliRight = pHeli->GetMatrix().GetRight();
            float RightDist, LeftDist;
            HeliRight.z = 0.0f;
            HeliRight.Normalise();
            RightDist = LeftDist = 1000.0f;

            Pos = pHeli->GetPosition() + 10.0f * HeliRight;
            TargetPos = Pos + ScanLength * Forward;
            if (CWorld::ProcessLineOfSight(pHeli->GetPosition(), Pos, colPoint, pHitEntity, true, false, false, false, false, false, false, true))
            {
                RightDist = 0.0f;
            }
            else if (CWorld::ProcessLineOfSight(Pos, TargetPos, colPoint, pHitEntity, true, false, false, false, false, false, false, true))
            {
                RightDist = (colPoint.GetPosition() - Pos).Magnitude();
            }

            Pos = pHeli->GetPosition() - 10.0f * HeliRight;
            TargetPos = Pos + ScanLength * Forward;
            if (CWorld::ProcessLineOfSight(pHeli->GetPosition(), Pos, colPoint, pHitEntity, true, false, false, false, false, false, false, true))
            {
                LeftDist = 0.0f;
            }
            else if (CWorld::ProcessLineOfSight(Pos, TargetPos, colPoint, pHitEntity, true, false, false, false, false, false, false, true))
            {
                LeftDist = (colPoint.GetPosition() - Pos).Magnitude();
            }

            if (LeftDist > RightDist)
            {
                pHeli->m_fRollControl = 0.5f;
            }
            else
            {
                pHeli->m_fRollControl = -0.5f;
            }
        }
        else
        {
            pHeli->m_bStopFlyingForAWhile = false;
            pHeli->m_fRollControl = 0.0f;
        }
    }

    if (pHeli->m_autoPilot.Mission == MISSION_HELI_LAND_TOUCHING_DOWN)
    {
        pHeli->m_bStopFlyingForAWhile = false;
        pHeli->m_fRollControl = 0.0f;
    }

    float HeliOrientation = CGeneral::GetATanOfXY(pHeli->GetMatrix().GetForward().x, pHeli->GetMatrix().GetForward().y);
    float PredictedHeight = pHeli->GetPosition().z + pHeli->GetMoveSpeed().z * 100.0f;
    pHeli->m_fThrottleControl = 0.0f;
    float HeightDiff = pHeli->m_DesiredHeight - PredictedHeight;
    if (HeightDiff > 0.0f)
    {
        pHeli->m_fThrottleControl = HeightDiff * 0.1f;
    }
    else
    {
        pHeli->m_fThrottleControl = HeightDiff * 0.2f;
    }
    pHeli->m_fThrottleControl += ((CGeneral::GetRandomNumber() & 15) - 7.0f) * 0.002f;
    float ThrottleExtra = pHeli->m_fThrottleControl - 0.5f;
    pHeli->m_fThrottleControl = CMaths::Max(-0.3f, CMaths::Min(1.0f, pHeli->m_fThrottleControl));

    float OrientationDiff = Orientation - HeliOrientation;
    while (OrientationDiff > PI)
    {
        OrientationDiff -= TWO_PI;
    }
    while (OrientationDiff < -PI)
    {
        OrientationDiff += TWO_PI;
    }
    pHeli->m_fYawControl = OrientationDiff * -2.0f;
    pHeli->m_fYawControl = CMaths::Min(1.0f, CMaths::Max(-1.0f, pHeli->m_fYawControl));

    if (Distance <= 60.0f && bSlowDownAtTarget)
    {
        CVector FutureCoors = pHeli->GetPosition() + 50.0f * pHeli->GetMoveSpeed();
        CVector FutureDiff = FutureCoors - pHeli->m_autoPilot.TargetCoors;
        float FutureDist = FutureDiff.Magnitude2D();
        float Dist2 = FutureDist - pHeli->m_autoPilot.TargetReachedDist;
        if (Dist2 >= 0.0f)
        {
            pHeli->m_fPitchControl = Dist2 * -0.8f / (30.0f - pHeli->m_autoPilot.TargetReachedDist);
        }
    }
    else
    {
        pHeli->m_fPitchControl = -0.8f;
    }

    if (pHeli->m_fPitchControl < 0.0f)
    {
        float AngleDiff = HeliOrientation - Orientation;
        while (AngleDiff < -PI)
        {
            AngleDiff += TWO_PI;
        }
        while (AngleDiff > PI)
        {
            AngleDiff -= TWO_PI;
        }
        if (AngleDiff < 0.0f)
        {
            AngleDiff = -AngleDiff;
        }
        pHeli->m_fPitchControl *= CMaths::Max(0.0f, 1.0f - 2.1220658f * AngleDiff);

        if (AngleDiff > HALF_PI && DotProduct(pHeli->GetMoveSpeed(), pHeli->GetMatrix().GetForward()) > 0.0f)
        {
            pHeli->m_fPitchControl = 0.3f;
        }
    }

    if (ThrottleExtra > 0.0f)
    {
        pHeli->m_fPitchControl *= 1.0f - CMaths::Min(1.0f, ThrottleExtra);
    }

    if (pHeli->m_fPitchControl < 0.0f)
    {
        float OurCruiseSpeed = DotProduct(pHeli->GetMoveSpeed(), pHeli->GetMatrix().GetForward()) * 60.0f;
        float CruiseSpeedDiff = (pHeli->m_autoPilot.CruiseSpeed - OurCruiseSpeed) * 0.1f;
        CruiseSpeedDiff = CMaths::Min(1.0f, CMaths::Max(0.0f, CruiseSpeedDiff));
        float MaxPitch = -CruiseSpeedDiff;
        pHeli->m_fPitchControl = CMaths::Max(MaxPitch, pHeli->m_fPitchControl);

        if (pHeli->m_autoPilot.SpeedCheat && pHeli->m_fPitchControl < -0.2f && !pHeli->m_bStopFlyingForAWhile)
        {
            CVector SpeedCheat(pHeli->GetMatrix().GetForward().x, pHeli->GetMatrix().GetForward().y, 0.0f);
            SpeedCheat *= pHeli->m_autoPilot.SpeedCheat * CTimer::GetTimeStep() * 0.001f;
            pHeli->m_vecMoveSpeed = pHeli->GetMoveSpeed() + SpeedCheat;
        }
    }

    if (pHeli->m_bStopFlyingForAWhile)
    {
        CVector Forward = pHeli->GetMatrix().GetForward();
        Forward.z = 0.0f;
        Forward.Normalise();
        float Vel = DotProduct(Forward, pHeli->GetMoveSpeed());
        pHeli->m_fPitchControl = Vel + Vel;
    }

    if (pHeli->m_fPitchControl < -1.0f)
    {
        pHeli->m_fPitchControl = -1.0f;
    }
    else if (pHeli->m_fPitchControl >= 1.0f)
    {
        pHeli->m_fPitchControl = 1.0f;
    }
}

// 0x423940
// ASM Match: 99.3%
void CCarCtrl::FlyAIHeliToTarget_FixedOrientation(CHeli* pHeli, float Orientation, CVector Target, bool bSlowDownAtTarget)
{
    uint32 TimeThingNew = (pHeli->RandomSeed + CTimer::GetTimeInMS()) % 500;
    uint32 TimeThingOld = (pHeli->RandomSeed + CTimer::GetPreviousTimeInMS()) % 500;
    if (TimeThingNew < TimeThingOld)
    {
        CColPoint colPoint;
        CEntity* pHitEntity;

        pHeli->m_DesiredHeight = pHeli->m_LowestFlightHeight;
        {
            CVector Pos = pHeli->GetPosition() + pHeli->GetMoveSpeed() * 50.0f;
            CVector Forward(CMaths::Cos(Orientation), CMaths::Sin(Orientation), -1.0f);
            Forward.Normalise();
            CVector TargetPos = Pos + 60.0f * Forward;

            if (CWorld::ProcessLineOfSight(Pos, TargetPos, colPoint, pHitEntity, true, false, false, false, false, false, false, true))
            {
                pHeli->m_DesiredHeight = VMAX(pHeli->m_DesiredHeight, colPoint.GetPosition().z + pHeli->m_MinHeightAboveTerrain);
            }
        }
    }

    float HeliOrientation = CGeneral::GetATanOfXY(pHeli->GetMatrix().GetForward().x, pHeli->GetMatrix().GetForward().y);
    float PredictedHeight = pHeli->GetPosition().z + pHeli->GetMoveSpeed().z * 100.0f;
    float HeightDiff;

    pHeli->m_fThrottleControl = 0.0f;
    HeightDiff = pHeli->m_DesiredHeight - PredictedHeight;
    if (HeightDiff > 0.0f)
    {
        pHeli->m_fThrottleControl = HeightDiff * 0.1f;
    }
    else
    {
        pHeli->m_fThrottleControl = HeightDiff * 0.2f;
    }
    pHeli->m_fThrottleControl += ((CGeneral::GetRandomNumber() & 15) - 7.0f) * 0.002f;

    float ThrottleExtra = pHeli->m_fThrottleControl - 0.5f;
    pHeli->m_fThrottleControl = VMAX(-0.3f, VMIN(pHeli->m_fThrottleControl, 1.0f));

    float OrientationDiff = Orientation - HeliOrientation;
    while (OrientationDiff > PI)
    {
        OrientationDiff -= TWO_PI;
    }
    while (OrientationDiff < -PI)
    {
        OrientationDiff += TWO_PI;
    }
    pHeli->m_fYawControl = OrientationDiff * -0.5f;
    pHeli->m_fYawControl = VMAX(-1.0f, VMIN(pHeli->m_fYawControl, 1.0f));

    CVector VecToTarget = Target - pHeli->GetPosition();
    VecToTarget.z = 0.0f;


    float ToTargetRight = DotProduct(VecToTarget, pHeli->GetMatrix().GetRight());
    float ToTargetForward = DotProduct(pHeli->GetMatrix().GetForward(), VecToTarget);
    float SideWaysSpeed = DotProduct(pHeli->GetMatrix().GetRight(), pHeli->GetMoveSpeed());
    float ForwardSpeed = DotProduct(pHeli->GetMoveSpeed(), pHeli->GetMatrix().GetForward());

    ToTargetRight += SideWaysSpeed * 80.0f;
    ToTargetForward += ForwardSpeed * 80.0f;

    if (VABS(ToTargetRight) < 5.0f)
    {
        pHeli->m_fRollControl = SideWaysSpeed;
    }
    else
    {
        pHeli->m_fRollControl = ToTargetRight * -0.02;
    }
    pHeli->m_fRollControl = VMAX(-0.75f, VMIN(pHeli->m_fRollControl, 0.75f));

    if (VABS(ToTargetForward) < 5.0f)
    {
        pHeli->m_fPitchControl = ForwardSpeed;
    }
    else
    {
        pHeli->m_fPitchControl = ToTargetForward * -0.015;
    }
    pHeli->m_fPitchControl = VMAX(-0.5f, VMIN(pHeli->m_fPitchControl, 0.5f));
}

// 0x422F00
// ASM Match: not measured
static bool FindHeightForVerticalAngle(CPlane* pPlane, float Angle, float Orientation, float* pResult)
{
    CColPoint colPoint;
    CEntity* pHitEntity;

    CVector Pos = pPlane->GetPosition() + pPlane->m_vecMoveSpeed * 50.0f;
    CVector Forward = CVector(CMaths::Cos(Orientation), CMaths::Sin(Orientation), 0.0f);
    Forward.Normalise();

    CVector Dir = Forward * CMaths::Cos(Angle) + CVector(0.0f, 0.0f, 1.0f) * CMaths::Sin(Angle);
    CVector TargetPos = Pos + Dir * 200.0f;

    if (CWorld::ProcessLineOfSight(Pos, TargetPos, colPoint, pHitEntity, true, false, false, false, false, false, false, true))
    {
        *pResult = colPoint.GetPosition().z;
        return true;
    }

    return false;
}

static auto& AnglesToTest = StaticRef<std::array<float, 6>>(0x8A5B2C); // {6, 3, 0, -20, -40, -60} degrees

// 0x422F80
// ASM Match: not measured
static float FindFlightHeight(CPlane* pPlane, float Orientation)
{
    float HighestHeight = 0.0f;
    float Result;

    for (int32 i = 0; i < 6; i++)
    {
        if (FindHeightForVerticalAngle(pPlane, AnglesToTest[i], Orientation, &Result))
        {
            if (i == 0)
            {
                return 100000.0f;
            }

            if (Result > HighestHeight)
            {
                HighestHeight = Result;
            }
        }
    }
    return HighestHeight;
}

// 0x423000
// ASM Match: 61.4%
void CCarCtrl::FlyAIPlaneInCertainDirection(CPlane* pPlane)
{
    float ROLL_TO_PITCH_COUPLING = 0.23f;  // 0x8A5B44
    float TAKEOFFSPEED_RCBARON = 7.0f;

    uint32 TimeThingNew = CTimer::GetTimeInMS() + pPlane->RandomSeed;
    uint32 TimeThingOld = CTimer::GetPreviousTimeInMS() + pPlane->RandomSeed;

    // Terrain avoidance calculation (every ~1 second based on random seed)
    if ((TimeThingNew % 1000) < (TimeThingOld % 1000))
    {
        float PlaneOrientation = CGeneral::GetATanOfXY(pPlane->GetMatrix().GetForward().x, pPlane->GetMatrix().GetForward().y);
        float DesiredOrientation = pPlane->m_FlightDirection;
        float FlightHeightFound;

        switch (pPlane->m_autoPilot.TempAction)
        {
            case TEMPACT_PLANE_SHARP_RIGHT:
                DesiredOrientation = PlaneOrientation + 2.0f;
                break;
            case TEMPACT_PLANE_SHARP_LEFT:
                DesiredOrientation = PlaneOrientation - 2.0f;
                break;
            case TEMPACT_PLANE_FLY_STRAIGHT:
                DesiredOrientation = PlaneOrientation;
                break;
        }

        pPlane->m_DesiredHeight = 500.0f;
        pPlane->m_FlightDirectionAvoidingTerrain = pPlane->m_FlightDirection + PI;

        float AngleDiff = PlaneOrientation - DesiredOrientation;
        while (AngleDiff > PI)
        {
            AngleDiff -= TWO_PI;
        }
        while (AngleDiff < -PI)
        {
            AngleDiff += TWO_PI;
        }

        float AngleDiffAbs = CMaths::Abs(AngleDiff);
        float TurnAngle = (AngleDiffAbs >= DegreesToRadians(30.0f)) ? (AngleDiff * 1.5f) : 0.0f;

        // Two passes to find safe flight height
        for (int32 Pass = 0; Pass < 2; ++Pass)
        {
            for (int32 C = 1; C < 20; ++C)
            {
                float OrientationOffset = ((C & 1) == 0) ? (-DegreesToRadians(15.0f) * (C >> 1)) : (DegreesToRadians(15.0f) * (C >> 1));
                float OrientationToTry = DesiredOrientation + OrientationOffset;

                bool bWithinAngles;
                if (Pass == 0)
                {
                    bWithinAngles = (OrientationOffset < 0.0f && OrientationOffset > TurnAngle) || (OrientationOffset > 0.0f && OrientationOffset < TurnAngle);
                }
                else
                {
                    bWithinAngles = (OrientationOffset >= 0.0f || OrientationOffset <= TurnAngle) && (OrientationOffset <= 0.0f || OrientationOffset >= TurnAngle);
                }

                if (bWithinAngles)
                {
                    FlightHeightFound = FindFlightHeight(pPlane, OrientationToTry);
                    if (FlightHeightFound < 150.0f)
                    {
                        pPlane->m_FlightDirectionAvoidingTerrain = pPlane->m_FlightDirection + (OrientationOffset * 1.1f);
                        pPlane->m_DesiredHeight = CMaths::Max(FlightHeightFound + pPlane->m_MinHeightAboveTerrain, pPlane->m_LowestFlightHeight);
                        break;
                    }
                }
            }
        }
    }

    // Calculate speed thresholds
    float TakeOffSpeed = pPlane->vehicleFlags.bIsRCVehicle ? TAKEOFFSPEED_RCBARON : 32.0f;

    float Orientation = CGeneral::GetATanOfXY(pPlane->GetMatrix().GetForward().x, pPlane->GetMatrix().GetForward().y);
    float Speed = (pPlane->GetMatrix().GetForward().y * pPlane->m_vecMoveSpeed.y) + (pPlane->GetMatrix().GetForward().x * pPlane->m_vecMoveSpeed.x);
    Speed *= 60.0f;

    // Check if on ground
    if (pPlane->m_aWheelRatios[0] < 1.0f || pPlane->m_aWheelRatios[1] < 1.0f || pPlane->m_aWheelRatios[2] < 1.0f || pPlane->m_aWheelRatios[3] < 1.0f)
    {
        pPlane->m_OnGroundTimer = CTimer::GetTimeInMS();
    }

    bool bTakingOff = (CTimer::GetTimeInMS() - pPlane->m_OnGroundTimer < 4001);

    if (bTakingOff)
    {
        pPlane->m_fPitchControl = (Speed >= TakeOffSpeed) ? 0.4f : 0.0f;
    }
    else
    {
        // In air - retract gear and calculate pitch
        if (pPlane->m_fLGearAngle != 1.0f)
        {
            pPlane->SetGearUp();
        }

        float HeightDiff = pPlane->GetPosition().z + (pPlane->m_vecMoveSpeed.z * 100.0f);
        float CurrentTilt = CMaths::ASin(pPlane->GetMatrix().GetForward().z);
        float TiltDiff = CurrentTilt - pPlane->m_OldTilt;
        pPlane->m_OldTilt = CurrentTilt;

        float FutureTilt = CurrentTilt + (TiltDiff * (100.0f / CTimer::GetTimeStep()));
        float DesiredTilt = CMaths::Clamp((pPlane->m_DesiredHeight - HeightDiff) / 30.0f, -0.4f, 0.4f);

        if (Speed < TakeOffSpeed)
        {
            DesiredTilt = CMaths::Min(DesiredTilt, 0.25f);
        }

        pPlane->m_fPitchControl = (DesiredTilt - FutureTilt) * 0.5f;
    }

    // Calculate yaw based on desired direction
    float OrientationDiff = pPlane->m_FlightDirectionAvoidingTerrain - Orientation;
    while (OrientationDiff < -PI)
    {
        OrientationDiff += TWO_PI;
    }
    while (OrientationDiff > PI)
    {
        OrientationDiff -= TWO_PI;
    }

    if (bTakingOff)
    {
        // On ground - steer towards takeoff direction
        float Yaw = (pPlane->m_TakeOffDirection - Orientation) * -10.0f;
        pPlane->m_fYawControl = CMaths::Clamp(Yaw, -1.0f, 1.0f);
        pPlane->m_fRollControl = 0.0f;
    }
    else
    {
        // In air - calculate roll and yaw
        float Turn = CMaths::Clamp(OrientationDiff * -1.5f, -0.9f, 0.9f);
        if (pPlane->vehicleFlags.bIsRCVehicle)
        {
            Turn = CMaths::Clamp(Turn, -0.7f, 0.7f);
        }

        float DesiredRoll;
        if (CMaths::Abs(Turn) >= 0.1f)
        {
            DesiredRoll = -Turn;
        }
        else
        {
            Turn *= 4.0f;
            DesiredRoll = 0.0f;
        }

        pPlane->m_fYawControl = Turn;

        // Reduce roll when slow
        float TurnAllowed = TakeOffSpeed * 1.2f;
        if (Speed < TurnAllowed)
        {
            DesiredRoll *= CMaths::Max(1.0f + ((TurnAllowed - Speed) / (TakeOffSpeed * -0.5f)), 0.0f);
        }

        // Calculate current roll angle
        float CurrentRoll = 0.0f;
        {
            float Tweak = CMaths::Sqrt(pPlane->GetMatrix().GetForward().y * pPlane->GetMatrix().GetForward().y + pPlane->GetMatrix().GetForward().x * pPlane->GetMatrix().GetForward().x);
            if (pPlane->GetMatrix().GetUp().z < 0.0f)
            {
                Tweak = -Tweak;
            }
            CurrentRoll = CMaths::ATan2(pPlane->GetMatrix().GetUp().x, Tweak);
        }

        // Calculate roll control
        float PredictedRoll = (CurrentRoll - pPlane->m_fPreviousRoll) * (30.0f / CMaths::Max(CTimer::GetTimeStep(), 1.0f));
        float RollChange = DesiredRoll - (CurrentRoll + PredictedRoll);
        while (RollChange > PI)
        {
            RollChange -= TWO_PI;
        }
        while (RollChange < -PI)
        {
            RollChange += TWO_PI;
        }

        float Mult = -RollChange;
        pPlane->m_fPreviousRoll = CurrentRoll;
        pPlane->m_fRollControl = CMaths::Clamp(Mult, -1.0f, 1.0f);

        // Adjust pitch based on roll
        float Tweak3 = CMaths::Abs(CurrentRoll * ROLL_TO_PITCH_COUPLING) + pPlane->m_fPitchControl;
        pPlane->m_fPitchControl = Tweak3;

        float OldPitchControl = pPlane->m_fPitchControl;
        if (OldPitchControl < 0.0f)
        {
            pPlane->m_fPitchControl = CMaths::Min(Tweak3, OldPitchControl * 0.5f);
        }
    }

    pPlane->m_fPitchControl = CMaths::Clamp(pPlane->m_fPitchControl, -1.0f, 1.0f);
    pPlane->m_fThrottleControl = pPlane->m_fScriptThrottleControl;

    // Handle fly up temp action
    if (pPlane->m_autoPilot.TempAction == TEMPACT_PLANE_FLY_UP)
    {
        pPlane->m_fPitchControl = 1.0f;
        if (Speed < 20.0f)
        {
            pPlane->m_autoPilot.TempAction = TEMPACT_NONE;
        }
    }
}

// 0x424210
// ASM Match: 60.6%
bool CCarCtrl::GenerateCarCreationCoors2(CVector Center, float DirectionX, float DirectionY, float RequiredDotProduct, bool bRequiredInside, float CreationDistOnScreen, float CreationDistOffScreen, CVector* pResult, CNodeAddress* pFromNode, CNodeAddress* pToNode, float* pFraction, bool bIgnoreSwitchedOff, bool bNoWater)
{
    static CNodeAddress StoredNearestNode, StoredSecondNearestNode;
    static CNodeAddress StoredNearestNode_IncludingSwitchedOff, StoredSecondNearestNode_IncludingSwitchedOff;
    static CVector UpdateCoors;
    static uint32 UpdateTime;

    if ((Center - UpdateCoors).Magnitude2D() > 10.0f || CTimer::GetTimeInMS() > UpdateTime)
    {
        ThePaths.Find2NodesForCarCreation(Center, &StoredNearestNode, &StoredSecondNearestNode, true);
        ThePaths.Find2NodesForCarCreation(Center, &StoredNearestNode_IncludingSwitchedOff, &StoredSecondNearestNode_IncludingSwitchedOff, false);

        UpdateCoors = Center;
        UpdateTime = CTimer::GetTimeInMS() + 5000;

        if (!StoredNearestNode.IsEmpty())
        {
            if (ThePaths.IsRegionLoaded(StoredNearestNode))
            {
                CPathNode* pNNode = ThePaths.FindNodePointer(StoredNearestNode);

                CCarPathLinkAddress FirstLink = ThePaths.pAdjacentLinks[pNNode->GetAddress().GetRegion()][pNNode->IndexAdjacentNodes];
                if (ThePaths.IsRegionLoaded(FirstLink))
                {
                    CCarPathLink* pLink = ThePaths.FindLinkPointer(FirstLink);

                    if ((pLink->LanesTo >= 2 || pLink->LanesFro >= 2) && (Center - pLink->GetCoors()).Magnitude2D() < 40.0f)
                    {
                        CPopulation::m_bMoreCarsAndFewerPeds = pNNode->Speed >= 2;
                    }
                    else
                    {
                        CPopulation::m_bMoreCarsAndFewerPeds = false;
                    }
                }
            }
        }
    }

    CNodeAddress CurrentNode;

    if ((CGeneral::GetRandomNumber() & 15) == 4 && !bNoWater)
    {
        CurrentNode = ThePaths.FindNodeClosestToCoors(Center, (ePathType)0, 50.0f, false, false, false, true, true);
        CreationDistOnScreen = CreationDistOnScreen * 1.5f;
    }
    else if ((CGeneral::GetRandomNumber() & 3) != 0)
    {
        CurrentNode = bIgnoreSwitchedOff ? StoredNearestNode : StoredNearestNode_IncludingSwitchedOff;
    }
    else
    {
        CurrentNode = bIgnoreSwitchedOff ? StoredSecondNearestNode : StoredSecondNearestNode_IncludingSwitchedOff;
    }

    if (CurrentNode.IsEmpty() || !ThePaths.IsRegionLoaded(CurrentNode))
    {
        return false;
    }

    CNodeAddress aNodesFoundSoFar[30];
    int32 NodesFoundSoFar = 1;
    aNodesFoundSoFar[0] = CurrentNode;

    float DistanceAlongRoad = 0.0f;

    while (true)
    {
        if (DistanceAlongRoad >= 230.0f)
        {
            return false;
        }

        if (!ThePaths.IsRegionLoaded(CurrentNode))
        {
            return false;
        }

        CPathNode* pCurrentNode = ThePaths.FindNodePointer(CurrentNode);

        int32 NumNeighbours = pCurrentNode->NumberAdjNodes;
        InitSequence(NumNeighbours);

        if (NumNeighbours == 0)
        {
            return false;
        }

        CNodeAddress NewNode;
        int32 NeighbourI = 0;

        while (true)
        {
            if (NeighbourI >= NumNeighbours)
            {
                return false;
            }

            int32 NeighbourToTest = FindSequenceElement(NeighbourI);
            int32 AdjNodeIndex = pCurrentNode->IndexAdjacentNodes + NeighbourToTest;

            NewNode = ThePaths.pAdjacentNodes[CurrentNode.GetRegion()][AdjNodeIndex];

            if (ThePaths.IsRegionLoaded(NewNode))
            {
                CCarPathLinkAddress ConnectingLink = ThePaths.pAdjacentLinks[CurrentNode.GetRegion()][AdjNodeIndex];
                if (ThePaths.IsRegionLoaded(ConnectingLink))
                {
                    bool bInList = false;
                    for (int32 C = 0; C < NodesFoundSoFar; ++C)
                    {
                        if (aNodesFoundSoFar[C] == NewNode)
                        {
                            bInList = true;
                        }
                    }

                    if (!bInList)
                    {
                        break;
                    }
                }
            }

            ++NeighbourI;
        }

        CPathNode* pOldNode = pCurrentNode;
        CPathNode* pNewNode = ThePaths.FindNodePointer(NewNode);

        if (!((pNewNode->SwitchedOff || pOldNode->SwitchedOff) && bIgnoreSwitchedOff))
        {
            float Dist1 = (Center - pOldNode->GetCoors()).Magnitude2D();
            float Dist2 = (Center - pNewNode->GetCoors()).Magnitude2D();

            bool bThisPointWillDo = false;

            float Diff1 = Dist1 - CreationDistOnScreen;
            float Diff2 = Dist2 - CreationDistOnScreen;

            if (Diff1 * Diff2 < 0.0f)
            {
                float Abs1 = VABS(Diff1);
                float Abs2 = VABS(Diff2);

                *pResult = (Abs2 * pOldNode->GetCoors() + Abs1 * pNewNode->GetCoors()) / (Abs1 + Abs2);

                if (TheCamera.IsSphereVisible(*pResult, 5.0f))
                {
                    *pFraction = Abs1 / (Abs1 + Abs2);
                    bThisPointWillDo = true;
                }
            }

            if (!bThisPointWillDo)
            {
                Diff1 = Dist1 - CreationDistOffScreen;
                Diff2 = Dist2 - CreationDistOffScreen;

                if (Diff1 * Diff2 < 0.0f)
                {
                    float Abs1 = VABS(Diff1);
                    float Abs2 = VABS(Diff2);

                    *pResult = (Abs2 * pOldNode->GetCoors() + Abs1 * pNewNode->GetCoors()) / (Abs1 + Abs2);

                    if (!TheCamera.IsSphereVisible(*pResult, 5.0f))
                    {
                        *pFraction = Abs1 / (Abs1 + Abs2);
                        bThisPointWillDo = true;
                    }
                }
            }

            if (bThisPointWillDo)
            {
                CVector NewNodeCoors = pNewNode->GetCoors();
                CVector OldNodeCoors = pOldNode->GetCoors();

                float Dist = (NewNodeCoors - OldNodeCoors).Magnitude2D();
                float HeightDiff = VABS(NewNodeCoors.z - OldNodeCoors.z);

                if (HeightDiff <= Dist * 0.5f)
                {
                    if ((CGeneral::GetRandomNumber() & 8) != 0)
                    {
                        *pFromNode = CurrentNode;
                        *pToNode = NewNode;
                    }
                    else
                    {
                        *pFromNode = NewNode;
                        *pToNode = CurrentNode;
                        *pFraction = 1.0f - *pFraction;
                    }

                    CPathNode* pToNodePtr = ThePaths.FindNodePointer(*pToNode);
                    CPathNode* pFromNodePtr = ThePaths.FindNodePointer(*pFromNode);

                    if (pToNodePtr->OnDeadEnd && ThePaths.ThisNodeWillLeadIntoADeadEnd(pToNodePtr, pFromNodePtr) && bIgnoreSwitchedOff)
                    {
                        return false;
                    }

                    float VecToPointX = pResult->x - Center.x;
                    float VecToPointY = pResult->y - Center.y;
                    float Length = CMaths::Sqrt(VecToPointX * VecToPointX + VecToPointY * VecToPointY);

                    float DotPr = VecToPointX / Length * DirectionX + VecToPointY / Length * DirectionY;

                    if ((DotPr > RequiredDotProduct) != bRequiredInside)
                    {
                        return false;
                    }

                    int32 Link;
                    for (Link = 0; Link < ThePaths.FindNodePointer(*pFromNode)->NumberAdjNodes; ++Link)
                    {
                        CNodeAddress TempNewLink = ThePaths.pAdjacentNodes[pFromNode->GetRegion()][ThePaths.FindNodePointer(*pFromNode)->IndexAdjacentNodes + Link];
                        if (TempNewLink == *pToNode)
                        {
                            break;
                        }
                    }

                    sprintf(gString, "tell Obbe it happened again %d/%d %d/%d", pFromNode->GetRegion(), pFromNode->GetIndex(), pToNode->GetRegion(), pToNode->GetIndex());

                    return true;
                }
            }
        }

        aNodesFoundSoFar[NodesFoundSoFar] = NewNode;

        if (NodesFoundSoFar > 28)
        {
            return false;
        }

        DistanceAlongRoad += (pNewNode->GetCoors() - pOldNode->GetCoors()).Magnitude2D();
        ++NodesFoundSoFar;

        CurrentNode = NewNode;
    }
}

// 0x42F9C0
// ASM Match: 85.7%
void CCarCtrl::GenerateEmergencyServicesCar()
{
    float SearchDist;
    CAccident* pAccident;
    CFire* pFire;

    // Preconditions: allow creation, not during gang war, wanted <= 3, main map
    if (bAllowEmergencyServicesToBeCreated && !CGangWars::GangWarFightingGoingOn() && FindPlayerPed()->GetWantedLevel() <= eWantedLevel::WANTED_LEVEL_3 && CGame::currArea == AREA_CODE_NORMAL_WORLD && CTheZones::GetLevelPlayerIsIn() != LEVEL_NAME_COUNTRY_SIDE && (NumFireTrucksOnDuty + NumAmbulancesOnDuty + NumParkedCars + NumMissionCars + NumRandomCars + NumLawEnforcerCars) <= MaxNumberOfCarsInUse)
    {
        // Ambulance generation
        if (NumAmbulancesOnDuty == 0)
        {
            if (GetAccidentManager()->GetNumberOfFreeAccidents() < 2)
            {
                // Not enough free accidents yet, ensure streaming request for models is cleared/queued
                CStreaming::StreamAmbulanceAndMedic(false);
            }
            else
            {
                // Find nearest free accident to player
                pAccident = GetAccidentManager()->GetNearestFreeAccident(FindPlayerCoors(), false);
                if (pAccident != nullptr && CStreaming::StreamAmbulanceAndMedic(true) && CTimer::GetTimeInMS() > LastTimeAmbulanceCreated + 30000)
                {
                    if (GenerateOneEmergencyServicesCar(CStreaming::GetDefaultAmbulanceModel(), pAccident->m_pPed->GetPosition()) != nullptr)
                    {
                        LastTimeAmbulanceCreated = CTimer::GetTimeInMS();
                    }
                }
            }
        }

        // Fire truck generation
        if (NumFireTrucksOnDuty == 0)
        {
            if (gFireManager.GetNumOfNonScriptFires() < 3u)
            {
                // Not enough fires yet, ensure streaming request for models is cleared/queued
                CStreaming::StreamFireEngineAndFireman(false);
            }
            else
            {
                pFire = gFireManager.FindNearestFire(FindPlayerCoors(), true, true);
                if (pFire != nullptr && CStreaming::StreamFireEngineAndFireman(true) && CTimer::GetTimeInMS() > LastTimeFireTruckCreated + 35000)
                {
                    if (GenerateOneEmergencyServicesCar(CStreaming::GetDefaultFireEngineModel(), pFire->GetPosition()) != nullptr)
                    {
                        LastTimeFireTruckCreated = CTimer::GetTimeInMS();
                    }
                }
            }
        }
    }
}

// 0x42B7D0
// ASM Match: 85.7%
CAutomobile* CCarCtrl::GenerateOneEmergencyServicesCar(uint32 ModelIndex, CVector TargetCoors)
{
    CEntity* TestEntity;
    CColPoint TestColPoint;
    float Fraction;
    float Length;
    float ClosestZ;
    CVector SetCoors;
    CVector Dir;
    CVector Diff;
    CAutomobile* pNewCar;
    CNodeAddress FromNode;
    CNodeAddress ToNode;
    CVector PlayerPos;
    int16 Tries;
    int16 Num;
    bool bFoundOne;

    PlayerPos = FindPlayerCentreOfWorld(CWorld::PlayerInFocus);

    Tries = 0;
    bFoundOne = false;

    for (Tries = 0; Tries < 5; ++Tries)
    {
        if (GenerateCarCreationCoors2(PlayerPos, 0.70710677f, 0.70710677f, -1.0f, true, 160.0f, 160.0f, &SetCoors, &FromNode, &ToNode, &Fraction, false, false))
        {
            if (!ThePaths.FindNodePointer(FromNode)->WaterNode)
            {
                CWorld::FindObjectsKindaColliding(SetCoors, 10.0f, true, &Num, 2, nullptr, false, true, true, false, false);

                if (Num == 0)
                {
                    bFoundOne = true;
                }
            }
        }

        if (bFoundOne)
        {
            break;
        }
    }

    if (Tries >= 5)
    {
        return nullptr;
    }

    pNewCar = new CAutomobile(static_cast<int32>(ModelIndex), RANDOM_VEHICLE, true);

    pNewCar->SetPosition(SetCoors);

    Dir = TargetCoors - SetCoors;
    Diff = Dir;

    Length = CMaths::Sqrt(Dir.x * Dir.x + Dir.y * Dir.y);

    if (Length == 0.0f)
    {
        Dir.x = 1.0f;
    }
    else
    {
        Dir.x = Dir.x / Length;
        Dir.y = Diff.y / Length;
    }

    pNewCar->GetMatrix().GetForward().x = Dir.x;
    pNewCar->GetMatrix().GetForward().y = Dir.y;
    pNewCar->GetMatrix().GetForward().z = 0.0f;
    pNewCar->GetMatrix().GetRight().x = Dir.y;
    pNewCar->GetMatrix().GetRight().y = -Dir.x;
    pNewCar->GetMatrix().GetRight().z = 0.0f;
    pNewCar->GetMatrix().GetUp().x = 0.0f;
    pNewCar->GetMatrix().GetUp().y = 0.0f;
    pNewCar->GetMatrix().GetUp().z = 1.0f;

    ClosestZ = 1.0e9f;

    SetCoors.z = ThePaths.FindNodePointer(FromNode)->GetCoors().z * (1.0f - Fraction) + ThePaths.FindNodePointer(ToNode)->GetCoors().z * Fraction;

    if (CWorld::ProcessVerticalLine(SetCoors, 1000.0f, TestColPoint, TestEntity, true, false, false, false))
    {
        ClosestZ = TestColPoint.GetPosition().z;
    }

    if (CWorld::ProcessVerticalLine(SetCoors, -1000.0f, TestColPoint, TestEntity, true, false, false, false, true, false, nullptr))
    {
        if (VABS(TestColPoint.GetPosition().z - SetCoors.z) < VABS(ClosestZ - SetCoors.z))
        {
            ClosestZ = TestColPoint.GetPosition().z;
        }
    }

    if (ClosestZ == 1.0e9f)
    {
        delete pNewCar;
        return nullptr;
    }

    SetCoors.z = pNewCar->GetDistanceFromCentreOfMassToBaseOfModel() + ClosestZ;

    pNewCar->SetPosition(SetCoors);
    pNewCar->SetMoveSpeed(CVector(0.0f, 0.0f, 0.0f));

    pNewCar->PlaceOnRoadProperly();

    CWorld::Add(pNewCar);

    switch (ModelIndex)
    {
        case MODEL_AMBULAN:
            pNewCar->vehicleFlags.bIsAmbulanceOnDuty = true;
            ++NumAmbulancesOnDuty;
            CCarAI::AddAmbulanceOccupants(pNewCar);
            break;
        case MODEL_FIRETRUK:
            pNewCar->vehicleFlags.bIsFireTruckOnDuty = true;
            ++NumFireTrucksOnDuty;
            CCarAI::AddFiretruckOccupants(pNewCar);
            break;
        case MODEL_ENFORCER:
        case MODEL_COPBIKE:
        case MODEL_COPCARLA:
        case MODEL_COPCARSF:
        case MODEL_COPCARVG:
        case MODEL_COPCARRU:
            CCarAI::AddPoliceCarOccupants(pNewCar, false);
            break;
        default:
            return pNewCar;
    }

    pNewCar->vehicleFlags.bSirenOrAlarm = true;

    return pNewCar;
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

    Density = pFromNode->Density >= pToNode->Density ? pToNode->Density : pFromNode->Density;

    if ((uint32)(CGeneral::GetRandomNumber() & 15) > Density)
    {
        return;
    }

    float radius = 8.0f;
    if (pFromNode->WaterNode)
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

    for (Link = 0; Link < (int16)pFromNode->NumberAdjNodes; Link++)
    {
        if (ThePaths.pAdjacentNodes[FromNode.Region][Link + pFromNode->IndexAdjacentNodes] != ToNode)
        {
            continue;
        }
        break;
    }

    RandomLink = ThePaths.pAdjacentLinks[FromNode.Region][Link + pFromNode->IndexAdjacentNodes];
    CCarPathLink* pNewLink = &ThePaths.GetCarPathLink(RandomLink);

    int16 NumLanes = 0;
    if (pNewLink->Node1 == ToNode)
    {
        NumLanes = pNewLink->LanesTo;
    }
    else
    {
        NumLanes = pNewLink->LanesFro;
    }

    if (NumLanes > 1)
    {
        if (CModelInfo::GetModelInfo(CarModel)->AsVehicleModelInfoPtr()->GetVehicleClass() == VEHICLE_TYPE_BMX)
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

    pNewVehicle->m_autoPilot.VeryOldNode.Region = (uint16)-1;
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
        if (CGeneral::GetRandomNumberInRange(0, MadDriverChance) == 0 || CCheat::m_aCheatsActive[MADDRIVERS_CHEAT])
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

    if (pFromNode->NumberAdjNodes == 1)
    {
        delete pNewVehicle;
        return;
    }

    do
    {
        RandomLinkIndex = (int16)(CGeneral::GetRandomNumber() % pFromNode->NumberAdjNodes);
        pNewVehicle->m_autoPilot.OldLink = ThePaths.pAdjacentLinks[FromNode.Region][pFromNode->IndexAdjacentNodes + RandomLinkIndex];
    } while (pNewVehicle->m_autoPilot.OldLink == pNewVehicle->m_autoPilot.NewLink);

    if (!ThePaths.m_pPathNodes[pNewVehicle->m_autoPilot.OldLink.Region])
    {
        delete pNewVehicle;
        return;
    }

    if (ThePaths.pAdjacentNodes[FromNode.Region][pFromNode->IndexAdjacentNodes + RandomLinkIndex] < FromNode)
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

    OldDirX = (float)(oldLinkRef.m_dir.x) * (float)pNewVehicle->m_autoPilot.InvertDirOldLink;
    OldDirY = (float)(oldLinkRef.m_dir.y) * (float)pNewVehicle->m_autoPilot.InvertDirOldLink;
    NewDirX = (float)(newLinkRef.m_dir.x) * (float)pNewVehicle->m_autoPilot.InvertDirNewLink;
    NewDirY = (float)(newLinkRef.m_dir.y) * (float)pNewVehicle->m_autoPilot.InvertDirNewLink;

    OldLaneOffset = (oldLinkRef.OneWayLaneOffset() + (float)pNewVehicle->m_autoPilot.OldLane) * 5.4f;
    NewLaneOffset = (newLinkRef.OneWayLaneOffset() + (float)pNewVehicle->m_autoPilot.NewLane) * 5.4f;
    if (pNewVehicle->GetBaseVehicleType() == VEHICLE_TYPE_BMX)
    {
        OldLaneOffset += 1.458f;
        NewLaneOffset += 1.458f;
    }

    CPathNode* pSpeedNode = ThePaths.GetPathNode(pNewVehicle->m_autoPilot.NewNode);
    pNewVehicle->m_autoPilot.SpeedFromNodes = (int8)pSpeedNode->Speed;
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

    if (CCheat::m_aCheatsActive[FUNHOUSE_CHEAT] && pNewVehicle->GetBaseVehicleType() == VEHICLE_TYPE_AUTOMOBILE)
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

        if (pNewVehicle->m_nModelIndex == MODEL_TRACTOR || pNewVehicle->m_nModelIndex == MODEL_COMBINE || pNewVehicle->GetBaseVehicleType() == VEHICLE_TYPE_BMX)
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

        if (pNewVehicle->GetBaseVehicleType() == VEHICLE_TYPE_AUTOMOBILE)
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

        if (pNewVehicle->GetBaseVehicleType() == VEHICLE_TYPE_BIKE && pNewVehicle->m_autoPilot.DrivingMode == DRIVING_STYLE_STOP_FOR_CARS)
        {
            pNewVehicle->SetStatus(STATUS_SIMPLE);
            pNewVehicle->m_autoPilot.DrivingMode = DRIVING_STYLE_DRIVINGMODE_AVOIDCARS_STOPFORPEDS_OBEYLIGHTS;
        }

        if (!bIsBoat && CarRating != 13 && (uint32)FindPlayerWanted()->GetWantedLevel() == 0 && (CCheat::m_aCheatsActive[MADDRIVERS_CHEAT] || TimeNextMadDriverChaseCreated <= 0.0f) && !bNoPoliceChasing && CreatePoliceChase(pNewVehicle, CarRating, FromNode))
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

                if (CGameLogic::LaRiotsActiveHere() || CCheat::m_aCheatsActive[MADDRIVERS_CHEAT])
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
// ASM Match: 99.9%
void CCarCtrl::GetAIHeliToAttackPlayer(CAutomobile* pVeh)
{
    CHeli* pHeli = (CHeli*)pVeh;

    float FlightDirection = CGeneral::GetATanOfXY(FindPlayerCoors().x - pHeli->GetPosition().x, FindPlayerCoors().y - pHeli->GetPosition().y);
    float Distance = CMaths::Sqrt((FindPlayerCoors().x - pHeli->GetPosition().x) * (FindPlayerCoors().x - pHeli->GetPosition().x) +
                                (FindPlayerCoors().y - pHeli->GetPosition().y) * (FindPlayerCoors().y - pHeli->GetPosition().y));
    pHeli->m_LowestFlightHeight = FindPlayerCoors().z;
    pHeli->m_autoPilot.TargetCoors = FindPlayerCoors();

    switch (pHeli->m_autoPilot.Mission)
    {
        case MISSION_HELI_ATTACK_PLAYER:
            if (Distance < 15.0f)
            {
                pHeli->m_autoPilot.Mission = MISSION_HELI_ATTACK_PLAYER_FLY_AWAY;
            }
            Distance += 50.0f;
            break;
        case MISSION_HELI_ATTACK_PLAYER_FLY_AWAY:
            if (Distance > 18.0f)
            {
                pHeli->m_autoPilot.Mission = MISSION_HELI_ATTACK_PLAYER;
            }
            FlightDirection += PI;
            break;
    }

    FlyAIHeliInCertainDirection(pHeli, FlightDirection, Distance, false);
    TestWhetherToFirePlaneGuns(pHeli, FindPlayerEntity());
    FireHeliRocketsAtTarget(pHeli, FindPlayerEntity());
}

// 0x42A730
// ASM Match
void CCarCtrl::GetAIHeliToFlyInDirection(CAutomobile* pVeh)
{
    CHeli* pHeli = (CHeli*)pVeh;
    FlyAIHeliInCertainDirection(pHeli, pHeli->m_FlightDirection, 1000.0f, false);
}

// 0x429780
// ASM Match: 99.8%
void CCarCtrl::GetAIPlaneToAttackPlayer(CAutomobile* pVeh)
{
    CPlane* pPlane = static_cast<CPlane*>(pVeh);
    CVector AimForCoors;

    AimForCoors = FindPlayerSpeed() * 50.0f + FindPlayerCoors();
    pPlane->m_FlightDirection = CGeneral::GetATanOfXY(AimForCoors.x - pPlane->GetPosition().x, AimForCoors.y - pPlane->GetPosition().y);

    pPlane->m_LowestFlightHeight = AimForCoors.z;
    FlyAIPlaneInCertainDirection(pPlane);
    if (FindPlayerVehicle() != nullptr)
    {
        if (FindPlayerVehicle()->GetVehicleAppearance() == VEHICLE_APPEARANCE_PLANE)
        {
            TriggerDogFightMoves(pPlane, FindPlayerVehicle());
        }
    }

    TestWhetherToFirePlaneGuns(pPlane, FindPlayerVehicle());
    PossiblyFireHSMissile(pPlane, FindPlayerVehicle());
}

// 0x429890
// ASM Match: 99.6%
void CCarCtrl::GetAIPlaneToDoDogFight(CAutomobile* pVeh)
{
    CPlane* pPlane = static_cast<CPlane*>(pVeh);
    CPhysical* pTargetPhys = static_cast<CPhysical*>(pPlane->m_autoPilot.pTargetEntity);
    CVector AimForCoors = pTargetPhys->GetPosition() + pTargetPhys->GetMoveSpeed() * 50.0f;

    switch (pPlane->m_autoPilot.Diversion)
    {
        case 1:
        {
            pPlane->m_FlightDirection = CGeneral::GetATanOfXY(pPlane->m_autoPilot.TargetCoors.x - pPlane->GetPosition().x, pPlane->m_autoPilot.TargetCoors.y - pPlane->GetPosition().y);
            pPlane->m_LowestFlightHeight = pPlane->m_autoPilot.TargetCoors.z;
            if ((pPlane->m_autoPilot.TargetCoors - pPlane->GetPosition()).Magnitude2D() < 50.0f)
            {
                pPlane->m_autoPilot.Diversion = 0;
            }
            break;
        }
        default:
        {
            pPlane->m_FlightDirection = CGeneral::GetATanOfXY(AimForCoors.x - pPlane->GetPosition().x, AimForCoors.y - pPlane->GetPosition().y);
            pPlane->m_LowestFlightHeight = AimForCoors.z;
            if ((CGeneral::GetRandomNumber() & 1023) == 500)
            {
                pPlane->m_autoPilot.Diversion = 1;
                pPlane->m_autoPilot.TargetCoors.x = CGeneral::GetRandomNumberInRange(0.0f, 600.0f) + AimForCoors.x - 300.0f;
                pPlane->m_autoPilot.TargetCoors.y = CGeneral::GetRandomNumberInRange(0.0f, 600.0f) + AimForCoors.y - 300.0f;
                pPlane->m_autoPilot.TargetCoors.z = AimForCoors.z + 50.0f;
            }
            break;
        }
    }

    CCarCtrl::FlyAIPlaneInCertainDirection(pPlane);
    CCarCtrl::TestWhetherToFirePlaneGuns(pPlane, pTargetPhys);
    CCarCtrl::PossiblyFireHSMissile(pPlane, pTargetPhys);
}

// 0x42F370
// ASM Match
void CCarCtrl::GetAIPlaneToDoDogFightAgainstPlayer(CAutomobile* pVeh)
{
    if (FindPlayerVehicle() != nullptr)
    {
        pVeh->m_autoPilot.pTargetEntity = FindPlayerVehicle();
    }
    else
    {
        pVeh->m_autoPilot.pTargetEntity = FindPlayerPed();
    }

    CCarCtrl::GetAIPlaneToDoDogFight(pVeh);
}

// 0x421440
CVehicle* CCarCtrl::GetNewVehicleDependingOnCarModel(int32 modelId, eVehicleCreatedBy createdBy) {
    switch (CModelInfo::GetModelInfo(modelId)->AsVehicleModelInfoPtr()->GetVehicleClass()) {
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
// ASM Match: 71.7%
bool CCarCtrl::IsThisAnAppropriateNode(CVehicle* pVehicle, CNodeAddress VeryOldNode, CNodeAddress OldNode, CNodeAddress CandidateNode, bool bGoingAgainstTraffic, bool bGoingDownOneWayStreet)
{
    int16 Num;

    if (!ThePaths.IsRegionLoaded(CandidateNode))
    {
        return false;
    }

    if (VeryOldNode == CandidateNode)
    {
        return false;
    }

    CPathNode* pOldNode = ThePaths.FindNodePointer(OldNode);
    CPathNode* pCandidateNode = ThePaths.FindNodePointer(CandidateNode);

    if ((pOldNode->WaterNode ^ pCandidateNode->WaterNode) && pVehicle->GetModelIndex() != MODEL_VORTEX)
    {
        return false;
    }

    switch (pCandidateNode->SpecialFunction)
    {
        case 1:
        {
            if (pVehicle->GetCreatedBy() == 2)
            {
                return false;
            }
            if (pVehicle->m_pDriver && pVehicle->m_pDriver->GetCharCreatedBy() == 2)
            {
                return false;
            }
            if (static_cast<CVehicleModelInfo*>(CModelInfo::GetModelInfo(pVehicle->GetModelIndex()))->GetVehicleList() == 5)
            {
                return false;
            }
            if (!bGoingDownOneWayStreet)
            {
                if (CrossProduct(pOldNode->GetCoors() - pCandidateNode->GetCoors(), pVehicle->GetPosition() - pCandidateNode->GetCoors()).z <= 0.0f)
                {
                    return false;
                }
            }
            if (pVehicle->IsLawEnforcementVehicle())
            {
                return false;
            }
            if (static_cast<CVehicleModelInfo*>(CModelInfo::GetModelInfo(pVehicle->GetModelIndex()))->GetVehicleList() == 5)
            {
                return false;
            }
            if (pVehicle->m_pDriver && pVehicle->m_pDriver->m_nPedType == 20)
            {
                return false;
            }
            if (CCarCtrl::IsAnyoneParking())
            {
                return false;
            }
            CWorld::FindObjectsKindaColliding(pCandidateNode->GetCoors(), 5.0f, true, &Num, 2, nullptr, false, true, false, false, false);
            return Num == 0;
        }

        case 2:
        {
            if (pVehicle->GetCreatedBy() == 2)
            {
                return false;
            }
            if (pVehicle->m_pDriver && pVehicle->m_pDriver->GetCharCreatedBy() == 2)
            {
                return false;
            }
            if (!bGoingDownOneWayStreet)
            {
                if (DotProduct(pVehicle->GetRight(), pVehicle->GetPosition() - pCandidateNode->GetCoors()) <= 0.0f)
                {
                    return false;
                }
            }
            if (pVehicle->IsLawEnforcementVehicle())
            {
                return false;
            }
            if (static_cast<CVehicleModelInfo*>(CModelInfo::GetModelInfo(pVehicle->GetModelIndex()))->GetVehicleList() == 5)
            {
                return false;
            }
            if (pVehicle->m_pDriver && pVehicle->m_pDriver->m_nPedType == 20)
            {
                return false;
            }
            if (CCarCtrl::IsAnyoneParking())
            {
                return false;
            }
            CWorld::FindObjectsKindaColliding(pCandidateNode->GetCoors(), 5.0f, true, &Num, 2, nullptr, false, true, false, false, false);
            return Num == 0;
        }

        case 8:
        case 9:
        {
            if (bGoingDownOneWayStreet)
            {
                return false;
            }
            CColModel& col = CModelInfo::GetColModel(pVehicle->GetModelIndex());
            if (col.GetBoundBox().m_vecMax.z > 1.5f || col.GetBoundBox().m_vecMax.x > 2.0f || col.GetBoundBox().m_vecMax.y > 4.0f)
            {
                return false;
            }
        }
        break;

        case 5:
        case 10:
        {
            if (bGoingDownOneWayStreet)
            {
                return false;
            }
            CColModel& col = CModelInfo::GetColModel(pVehicle->GetModelIndex());
            if (col.GetBoundBox().m_vecMax.z < 2.0f)
            {
                return false;
            }
        }
        break;

        default:
        case 3:
        case 4:
        case 6:
        case 7:
        {
            if (pVehicle->GetCreatedBy() == 2 && pVehicle->m_autoPilot.Mission == MISSION_CRUISE)
            {
                if (pCandidateNode->DontWanderHere && !pOldNode->DontWanderHere)
                {
                    return false;
                }
            }
            if (pCandidateNode->RoadBlock && !pOldNode->RoadBlock)
            {
                return false;
            }
            if (pCandidateNode->WaterNode && !pOldNode->WaterNode)
            {
                return false;
            }
            return !bGoingDownOneWayStreet;
        }
    }

    if (pVehicle->GetCreatedBy() == 2 && pVehicle->m_autoPilot.Mission == MISSION_CRUISE)
    {
        if (pCandidateNode->DontWanderHere && !pOldNode->DontWanderHere)
        {
            return false;
        }
    }
    if (pCandidateNode->RoadBlock && !pOldNode->RoadBlock)
    {
        return false;
    }
    if (pCandidateNode->WaterNode && !pOldNode->WaterNode)
    {
        return false;
    }
    return !bGoingDownOneWayStreet;
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
// ASM Match: 48.4%
void CCarCtrl::JoinCarWithRoadSystem(CVehicle* pVehicle)
{
    pVehicle->m_autoPilot.VeryOldNode.SetEmpty();
    pVehicle->m_autoPilot.OldNode.SetEmpty();
    pVehicle->m_autoPilot.NewNode.SetEmpty();
    pVehicle->m_autoPilot.OldLink.SetEmpty();
    pVehicle->m_autoPilot.VeryOldLink.SetEmpty();
    pVehicle->m_autoPilot.NewLink.SetEmpty();
    pVehicle->m_autoPilot.NumPathNodes = 0;

    CNodeAddress NodeFound = ThePaths.FindNodeClosestToCoorsFavourDirection(pVehicle->GetPosition(), (ePathType)0, CVector2D(pVehicle->GetMatrix().GetForward().x, pVehicle->GetMatrix().GetForward().y));

    if (NodeFound.IsEmpty())
    {
        return;
    }

    if (!ThePaths.IsRegionLoaded(NodeFound))
    {
        return;
    }

    CPathNode* pNodeFound = ThePaths.FindNodePointer(NodeFound);

    int32 NumLinks = pNodeFound->NumberAdjNodes;

    float ClosestDist = 999999.9f;
    CNodeAddress ClosestNode;

    if (NumLinks == 0)
    {
        return;
    }

    CNodeAddress* pAdjacent = &ThePaths.pAdjacentNodes[NodeFound.GetRegion()][pNodeFound->IndexAdjacentNodes];

    for (int32 C = NumLinks; C != 0; --C)
    {
        CNodeAddress NewNode = *pAdjacent;

        if (ThePaths.IsRegionLoaded(NewNode))
        {
            float Dist = (ThePaths.FindNodePointer(NewNode)->GetCoors() - pNodeFound->GetCoors()).Magnitude2D();

            if (Dist < ClosestDist)
            {
                ClosestDist = Dist;
                ClosestNode = NewNode;
            }
        }

        ++pAdjacent;
    }

    if (ClosestNode.IsEmpty())
    {
        return;
    }

    CNodeAddress OldNode;
    CNodeAddress NewNode;

    float DirX = pVehicle->GetMatrix().GetForward().x;
    float DirY = pVehicle->GetMatrix().GetForward().y;

    if (DirX == 0.0f && DirY == 0.0f)
    {
        DirX = 1.0f;
    }

    if ((ThePaths.FindNodePointer(ClosestNode)->GetCoors().x - pNodeFound->GetCoors().x) * DirX + (ThePaths.FindNodePointer(ClosestNode)->GetCoors().y - pNodeFound->GetCoors().y) * DirY < 0.0f)
    {
        OldNode = ClosestNode;
        NewNode = NodeFound;
    }
    else
    {
        OldNode = NodeFound;
        NewNode = ClosestNode;
    }

    pVehicle->m_autoPilot.VeryOldNode.SetEmpty();
    pVehicle->m_autoPilot.OldNode = OldNode;
    pVehicle->m_autoPilot.NewNode = NewNode;

    FindLinksToGoWithTheseNodes(pVehicle);

    pVehicle->m_autoPilot.OldLane = 0;
    pVehicle->m_autoPilot.NewLane = 0;
}

// 0x42F870
// ASM Match: 74.7%
bool CCarCtrl::JoinCarWithRoadSystemGotoCoors(CVehicle* pVehicle, const CVector& TargetCoors, bool bUseClosestLane, bool bBoat)
{
    pVehicle->m_autoPilot.TargetCoors = TargetCoors;

    ThePaths.DoPathSearch((ePathType)0, pVehicle->GetPosition(), CNodeAddress(), TargetCoors, pVehicle->m_autoPilot.aPathNodeList, pVehicle->m_autoPilot.NumPathNodes, 8, nullptr, 999999.9f, nullptr, 999999.9f, pVehicle->m_autoPilot.bDontGoAgainstTraffic != 0, CNodeAddress(), pVehicle->GetModelIndex() == MODEL_VORTEX, bBoat);

    ThePaths.RemoveBadStartNode(pVehicle->GetPosition(), pVehicle->m_autoPilot.aPathNodeList, &pVehicle->m_autoPilot.NumPathNodes);

    if (pVehicle->m_autoPilot.NumPathNodes < 2)
    {
        JoinCarWithRoadSystem(pVehicle);
        pVehicle->m_autoPilot.NumPathNodes = 0;
        return true;
    }

    pVehicle->m_autoPilot.OldNode = pVehicle->m_autoPilot.aPathNodeList[0];
    pVehicle->m_autoPilot.VeryOldNode.SetEmpty();
    pVehicle->m_autoPilot.RemoveOnePathNode();

    pVehicle->m_autoPilot.NewNode = pVehicle->m_autoPilot.aPathNodeList[0];
    pVehicle->m_autoPilot.RemoveOnePathNode();

    FindLinksToGoWithTheseNodes(pVehicle);

    pVehicle->m_autoPilot.OldLane = 0;
    pVehicle->m_autoPilot.NewLane = 0;

    return false;
}

// 0x432B10
// ASM Match: 55.6%
bool CCarCtrl::PickNextNodeAccordingStrategy(CVehicle* pVehicle)
{
    pVehicle->m_autoPilot.SpeedFromNodes = static_cast<int8>(ThePaths.FindNodePointer(pVehicle->m_autoPilot.NewNode)->Speed);
    if (ThePaths.FindNodePointer(pVehicle->m_autoPilot.NewNode)->SpecialFunction >= 8 && ThePaths.FindNodePointer(pVehicle->m_autoPilot.NewNode)->SpecialFunction <= 9)
    {
        pVehicle->m_autoPilot.SpeedFromNodes = -1;
    }

    switch (pVehicle->m_autoPilot.Mission)
    {
        case MISSION_RAMPLAYER_FARAWAY:
        case MISSION_BLOCKPLAYER_FARAWAY:
        case MISSION_APPROACHPLAYER_FARAWAY:
        {
            CVector Coors = FindPlayerCoors();
            return PickNextNodeToChaseCar(pVehicle, Coors.x, FindPlayerCoors().y, FindPlayerCoors().z, FindPlayerVehicle(-1, false));
        }

        case MISSION_GOTOCOORDINATES:
        case MISSION_GOTOCOORDINATES_ACCURATE:
            return PickNextNodeToFollowPath(pVehicle);

        case MISSION_RAMCAR_FARAWAY:
        case MISSION_BLOCKCAR_FARAWAY:
        case MISSION_FOLLOWCAR_FARAWAY:
        case MISSION_KILLPED_FARAWAY:
        case MISSION_DO_DRIVEBY_FARAWAY:
        case MISSION_ESCORT_LEFT_FARAWAY:
        case MISSION_ESCORT_RIGHT_FARAWAY:
        case MISSION_ESCORT_REAR_FARAWAY:
        case MISSION_ESCORT_FRONT_FARAWAY:
            return PickNextNodeToChaseCar(pVehicle, pVehicle->m_autoPilot.pTargetEntity->GetPosition().x, pVehicle->m_autoPilot.pTargetEntity->GetPosition().y, pVehicle->m_autoPilot.pTargetEntity->GetPosition().z, static_cast<CVehicle*>(pVehicle->m_autoPilot.pTargetEntity));

        default:
            PickNextNodeRandomly(pVehicle);
            return false;
    }
}

// 0x421740
void CCarCtrl::InitSequence(int32 numSequenceElements) {
    SequenceElements     = numSequenceElements;
    SequenceRandomOffset = CGeneral::GetRandomNumber() % numSequenceElements;
    bSequenceOtherWay    = (CGeneral::GetRandomNumber() / 4) % 2;
}

// 0x42DE80
// ASM Match: 65.2%
void CCarCtrl::PickNextNodeRandomly(CVehicle* pVehicle)
{
    int32 Loop;
    CNodeAddress FromNode, CurrNode;
    CCarPathLinkAddress TempLink, FoundNewLink;
    int32 Neighbours, LanesToChooseFrom, RandomNeighbour;
    float NewDirX, NewDirY, OldDirY, OldDirX;
    uint8 AllowedDirs, PathDirection;
    bool bComingOutOfOneWayStreet, bSharpTurn, bGoingIntoOneWayStreet, GoingAgainstTraffic;
    float OldLaneOffset, NewLaneOffset;
    bool bHappyWithNode;

    if (pVehicle->ForcedRandomSeed != 0)
    {
        CGeneral::SetRandomSeed(pVehicle->ForcedRandomSeed);
    }

    if (!ThePaths.IsRegionLoaded(pVehicle->m_autoPilot.OldNode))
    {
        return;
    }

    if (!ThePaths.IsRegionLoaded(pVehicle->m_autoPilot.NewNode))
    {
        return;
    }

    if (!ThePaths.IsRegionLoaded(pVehicle->m_autoPilot.NewLink))
    {
        return;
    }

    FromNode = pVehicle->m_autoPilot.OldNode;
    CurrNode = pVehicle->m_autoPilot.NewNode;

    Neighbours = ThePaths.FindNodePointer(CurrNode)->NumberAdjNodes;

    GoingAgainstTraffic = false;

    if (ThePaths.FindLinkPointer(pVehicle->m_autoPilot.NewLink)->Node1 == CurrNode)
    {
        LanesToChooseFrom = ThePaths.FindLinkPointer(pVehicle->m_autoPilot.NewLink)->LanesTo;
        if (ThePaths.FindLinkPointer(pVehicle->m_autoPilot.NewLink)->LanesFro == 0)
        {
            GoingAgainstTraffic = true;
        }
    }
    else
    {
        LanesToChooseFrom = ThePaths.FindLinkPointer(pVehicle->m_autoPilot.NewLink)->LanesFro;
        if (ThePaths.FindLinkPointer(pVehicle->m_autoPilot.NewLink)->LanesTo == 0)
        {
            GoingAgainstTraffic = true;
        }
    }

    AllowedDirs = 0;
    if (pVehicle->m_autoPilot.NewLane == 0)
    {
        AllowedDirs = 4;
    }
    if (pVehicle->m_autoPilot.NewLane == LanesToChooseFrom - 1)
    {
        AllowedDirs |= 2;
    }
    if (LanesToChooseFrom < 3 || AllowedDirs == 0)
    {
        AllowedDirs |= 1;
    }

    pVehicle->m_autoPilot.VeryOldNode = FromNode;
    pVehicle->m_autoPilot.OldNode = CurrNode;

    if (CCarCtrl::ThisVehicleShouldTryNotToTurn(pVehicle))
    {
        AllowedDirs = 1;
    }

    bHappyWithNode = false;

    CCarCtrl::InitSequence(Neighbours);

    for (Loop = 0; Loop < Neighbours; Loop++)
    {
        RandomNeighbour = CCarCtrl::FindSequenceElement(Loop);

        CNodeAddress NextNode = ThePaths.pAdjacentNodes[CurrNode.Region][ThePaths.FindNodePointer(CurrNode)->IndexAdjacentNodes + RandomNeighbour];
        pVehicle->m_autoPilot.NewNode = NextNode;

        if (!ThePaths.IsRegionLoaded(NextNode))
        {
            continue;
        }

        bSharpTurn = false;
        PathDirection = CCarCtrl::FindPathDirection(FromNode, CurrNode, NextNode, &bSharpTurn);
        FoundNewLink = ThePaths.pAdjacentLinks[CurrNode.Region][ThePaths.FindNodePointer(CurrNode)->IndexAdjacentNodes + RandomNeighbour];

        if (!ThePaths.IsRegionLoaded(FoundNewLink))
        {
            continue;
        }

        if (pVehicle->GetStatus() == STATUS_SIMPLE && bSharpTurn)
        {
            continue;
        }

        bComingOutOfOneWayStreet = false;
        bGoingIntoOneWayStreet = false;

        if (ThePaths.FindLinkPointer(FoundNewLink)->Node1 == CurrNode)
        {
            if (ThePaths.FindLinkPointer(FoundNewLink)->LanesFro == 0)
            {
                bComingOutOfOneWayStreet = true;
            }
            if (ThePaths.FindLinkPointer(FoundNewLink)->LanesTo == 0)
            {
                bGoingIntoOneWayStreet = true;
            }
        }
        else
        {
            if (ThePaths.FindLinkPointer(FoundNewLink)->LanesTo == 0)
            {
                bComingOutOfOneWayStreet = true;
            }
            if (ThePaths.FindLinkPointer(FoundNewLink)->LanesFro == 0)
            {
                bGoingIntoOneWayStreet = true;
            }
        }

        if (!CCarCtrl::IsThisAnAppropriateNode(pVehicle, FromNode, CurrNode, NextNode, bComingOutOfOneWayStreet, bGoingIntoOneWayStreet))
        {
            continue;
        }

        if ((PathDirection & AllowedDirs) == 0)
        {
            continue;
        }

        if (!GoingAgainstTraffic || !bGoingIntoOneWayStreet)
        {
            bHappyWithNode = true;
            break;
        }
    }

    if (!bHappyWithNode)
    {
        for (Loop = 0; Loop < Neighbours; Loop++)
        {
            RandomNeighbour = CCarCtrl::FindSequenceElement(Loop);

            CNodeAddress NextNode = ThePaths.pAdjacentNodes[CurrNode.Region][ThePaths.FindNodePointer(CurrNode)->IndexAdjacentNodes + RandomNeighbour];
            pVehicle->m_autoPilot.NewNode = NextNode;

            if (!ThePaths.IsRegionLoaded(NextNode))
            {
                continue;
            }

            FoundNewLink = ThePaths.pAdjacentLinks[CurrNode.Region][ThePaths.FindNodePointer(CurrNode)->IndexAdjacentNodes + RandomNeighbour];
            if (!ThePaths.IsRegionLoaded(FoundNewLink))
            {
                continue;
            }

            bGoingIntoOneWayStreet = false;

            if (ThePaths.FindLinkPointer(FoundNewLink)->Node1 == CurrNode)
            {
                if (ThePaths.FindLinkPointer(FoundNewLink)->LanesFro == 0)
                {
                    bGoingIntoOneWayStreet = true;
                }
            }
            else
            {
                if (ThePaths.FindLinkPointer(FoundNewLink)->LanesTo == 0)
                {
                    bGoingIntoOneWayStreet = true;
                }
            }

            if (NextNode == FromNode)
            {
                continue;
            }

            if (bGoingIntoOneWayStreet)
            {
                continue;
            }

            if (!ThePaths.FindNodePointer(NextNode)->SwitchedOff || ThePaths.FindNodePointer(FromNode)->SwitchedOff)
            {
                bHappyWithNode = true;
                break;
            }
        }
    }

    if (!bHappyWithNode)
    {
        for (Loop = 0; Loop < Neighbours; Loop++)
        {
            RandomNeighbour = CCarCtrl::FindSequenceElement(Loop);

            CNodeAddress NextNode = ThePaths.pAdjacentNodes[CurrNode.Region][ThePaths.FindNodePointer(CurrNode)->IndexAdjacentNodes + RandomNeighbour];
            pVehicle->m_autoPilot.NewNode = NextNode;

            if (!ThePaths.IsRegionLoaded(NextNode))
            {
                continue;
            }

            FoundNewLink = ThePaths.pAdjacentLinks[CurrNode.Region][ThePaths.FindNodePointer(CurrNode)->IndexAdjacentNodes + RandomNeighbour];
            if (!ThePaths.IsRegionLoaded(FoundNewLink))
            {
                continue;
            }

            bGoingIntoOneWayStreet = false;

            if (ThePaths.FindLinkPointer(FoundNewLink)->Node1 == CurrNode)
            {
                if (ThePaths.FindLinkPointer(FoundNewLink)->LanesFro == 0)
                {
                    bGoingIntoOneWayStreet = true;
                }
            }
            else
            {
                if (ThePaths.FindLinkPointer(FoundNewLink)->LanesTo == 0)
                {
                    bGoingIntoOneWayStreet = true;
                }
            }

            if (NextNode == FromNode)
            {
                continue;
            }

            if (!bGoingIntoOneWayStreet)
            {
                bHappyWithNode = true;
                break;
            }
        }
    }

    if (!bHappyWithNode)
    {
        pVehicle->m_autoPilot.NewNode = FromNode;
        FoundNewLink = pVehicle->m_autoPilot.NewLink;
    }

    if (pVehicle->m_autoPilot.NewNode == FromNode)
    {
        if (pVehicle->GetStatus() != STATUS_PHYSICS)
        {
            CCarCtrl::SwitchVehicleToRealPhysics(pVehicle);
        }
    }

    switch (ThePaths.FindNodePointer(pVehicle->m_autoPilot.NewNode)->SpecialFunction)
    {
        case 10:
        {
            pVehicle->m_autoPilot.TempAction = TEMPACT_WAIT;
            pVehicle->m_autoPilot.TempActionFinish = CTimer::GetTimeInMS() + 10000;
            if (pVehicle->GetStatus() == STATUS_SIMPLE)
            {
                pVehicle->m_autoPilot.ModifySpeed(0.0f);
            }
        }
        break;

        case 2:
        {
            CCarCtrl::SwitchVehicleToRealPhysics(pVehicle);
            pVehicle->m_autoPilot.DrivingMode = DRIVING_STYLE_STOP_FOR_CARS;
            pVehicle->m_autoPilot.Mission = (eCarMission)45;
        }
        break;

        case 1:
        {
            CCarCtrl::SwitchVehicleToRealPhysics(pVehicle);
            pVehicle->m_autoPilot.DrivingMode = DRIVING_STYLE_STOP_FOR_CARS;
            pVehicle->m_autoPilot.Mission = (eCarMission)46;
        }
        break;
    }

    if (ThePaths.FindNodePointer(pVehicle->m_autoPilot.OldNode)->SpecialFunction == 9 && pVehicle->GetCreatedBy() != 2)
    {
        pVehicle->m_autoPilot.TempAction = TEMPACT_WAIT;
        pVehicle->m_autoPilot.TempActionFinish = CTimer::GetTimeInMS() + 4500;
        if (pVehicle->GetStatus() == STATUS_SIMPLE)
        {
            pVehicle->m_autoPilot.ModifySpeed(0.0f);
        }
    }

    pVehicle->m_autoPilot.VeryOldLink = pVehicle->m_autoPilot.OldLink;
    pVehicle->m_autoPilot.OldLink = pVehicle->m_autoPilot.NewLink;
    pVehicle->m_autoPilot.InvertDirVeryOldLink = pVehicle->m_autoPilot.InvertDirOldLink;
    pVehicle->m_autoPilot.InvertDirOldLink = pVehicle->m_autoPilot.InvertDirNewLink;
    pVehicle->m_autoPilot.TimeToLeaveLink += pVehicle->m_autoPilot.TimeToGetToNextLink;
    pVehicle->m_autoPilot.OldLane = pVehicle->m_autoPilot.NewLane;
    pVehicle->m_autoPilot.NewLink = FoundNewLink;

    if (pVehicle->m_autoPilot.OldNode < pVehicle->m_autoPilot.NewNode)
    {
        pVehicle->m_autoPilot.InvertDirNewLink = -1;
        if (ThePaths.IsRegionLoaded(FoundNewLink))
        {
            LanesToChooseFrom = ThePaths.FindLinkPointer(FoundNewLink)->LanesFro;
        }
        else
        {
            LanesToChooseFrom = 1;
        }
    }
    else
    {
        pVehicle->m_autoPilot.InvertDirNewLink = 1;
        if (ThePaths.IsRegionLoaded(FoundNewLink))
        {
            LanesToChooseFrom = ThePaths.FindLinkPointer(FoundNewLink)->LanesTo;
        }
        else
        {
            LanesToChooseFrom = 1;
        }
    }

    OldDirX = ThePaths.FindLinkPointer(pVehicle->m_autoPilot.OldLink)->GetDirX() * pVehicle->m_autoPilot.InvertDirOldLink;
    OldDirY = ThePaths.FindLinkPointer(pVehicle->m_autoPilot.OldLink)->GetDirY() * pVehicle->m_autoPilot.InvertDirOldLink;
    NewDirX = ThePaths.FindLinkPointer(pVehicle->m_autoPilot.NewLink)->GetDirX() * pVehicle->m_autoPilot.InvertDirNewLink;
    NewDirY = ThePaths.FindLinkPointer(pVehicle->m_autoPilot.NewLink)->GetDirY() * pVehicle->m_autoPilot.InvertDirNewLink;

    if (LanesToChooseFrom > 0)
    {
        CVector NodeDiff = ThePaths.FindNodePointer(pVehicle->m_autoPilot.NewNode)->GetCoors() - ThePaths.FindNodePointer(pVehicle->m_autoPilot.OldNode)->GetCoors();
        if (NodeDiff.x * NodeDiff.x + NodeDiff.y * NodeDiff.y > 256.0f)
        {
            pVehicle->m_autoPilot.LaneChangeCounter--;
            if (pVehicle->m_autoPilot.LaneChangeCounter == 0)
            {
                pVehicle->m_autoPilot.LaneChangeCounter = (CGeneral::GetRandomNumber() & 3) + 4;
                if (CGeneral::GetRandomTrueFalse())
                {
                    pVehicle->m_autoPilot.NewLane++;
                }
                else
                {
                    pVehicle->m_autoPilot.NewLane--;
                }
            }
        }

        if (pVehicle->m_autoPilot.NewLane >= LanesToChooseFrom - 1)
        {
            pVehicle->m_autoPilot.NewLane = LanesToChooseFrom - 1;
        }
        if (pVehicle->m_autoPilot.NewLane < 0)
        {
            pVehicle->m_autoPilot.NewLane = 0;
        }
    }

    if (pVehicle->m_autoPilot.bAlwaysInFastLane)
    {
        pVehicle->m_autoPilot.NewLane = 0;
    }
    else if (pVehicle->m_autoPilot.bAlwaysInSlowLane)
    {
        pVehicle->m_autoPilot.NewLane = VMAX(0, LanesToChooseFrom - 1);
    }

    if (pVehicle->GetStatus() == STATUS_SIMPLE)
    {
        OldLaneOffset = (ThePaths.FindLinkPointer(pVehicle->m_autoPilot.OldLink)->OneWayLaneOffset() + pVehicle->m_autoPilot.OldLane) * 5.4f;
        NewLaneOffset = (ThePaths.FindLinkPointer(pVehicle->m_autoPilot.NewLink)->OneWayLaneOffset() + pVehicle->m_autoPilot.NewLane) * 5.4f;

        CVector endCoors(ThePaths.FindLinkPointer(pVehicle->m_autoPilot.NewLink)->GetCoorsX() + NewLaneOffset * NewDirY, ThePaths.FindLinkPointer(pVehicle->m_autoPilot.NewLink)->GetCoorsY() - NewLaneOffset * NewDirX, 0.0f);
        CVector startCoors(ThePaths.FindLinkPointer(pVehicle->m_autoPilot.OldLink)->GetCoorsX() + OldLaneOffset * OldDirY, ThePaths.FindLinkPointer(pVehicle->m_autoPilot.OldLink)->GetCoorsY() - OldLaneOffset * OldDirX, 0.0f);

        int32 TimeToSpend = static_cast<int32>(CCurves::CalcSpeedScaleFactor(startCoors, endCoors, OldDirX, OldDirY, NewDirX, NewDirY) * (1000.0f / pVehicle->m_autoPilot.ActualSpeed));
        if (TimeToSpend <= 10)
        {
            TimeToSpend = 10;
        }
        pVehicle->m_autoPilot.TimeToGetToNextLink = TimeToSpend;
    }
}

// 0x426EF0
bool CCarCtrl::PickNextNodeToChaseCar(CVehicle* vehicle, float destX, float destY, float destZ, CVehicle* carWeChase) {
    return plugin::CallAndReturn<bool, 0x426EF0, CVehicle*, float, float, float, CVehicle*>(vehicle, destX, destY, destZ, carWeChase);
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
// ASM Match: 24.3%
void CCarCtrl::RegisterVehicleOfInterest(CVehicle* vehicle) {
    for (auto c = 0; c < 2; c++) {
        if (vehicle == apCarsToKeep[c]) {
            aCarsToKeepTime[c] = CTimer::GetTimeInMS();
            return;
        }
    }

    for (auto c = 0; c < 2; c++) {
        if (!apCarsToKeep[c]) {
            apCarsToKeep[c]    = vehicle;
            aCarsToKeepTime[c] = CTimer::GetTimeInMS();
            return;
        }
    }

    int16  oldestC    = 0;
    uint32 oldestTime = UINT32_MAX;
    for (int16 c = 0; c < 2; c++) {
        if (apCarsToKeep[c] && aCarsToKeepTime[c] < oldestTime) {
            oldestC    = c;
            oldestTime = aCarsToKeepTime[c];
        }
    }
    apCarsToKeep[oldestC]    = vehicle;
    aCarsToKeepTime[oldestC] = CTimer::GetTimeInMS();
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

    for (auto& veh : GetVehiclePool()->GetAllValid()) {
        PossiblyRemoveVehicle(&veh);

        // FIXBUGS: Prevenir Use-After-Free si el vehículo fue eliminado en PossiblyRemoveVehicle
        if (notsa::bugfixes::CCarCtrl_RemoveDistantCars_UseAfterFree) {
            if (!GetVehiclePool()->IsObjectValid(&veh)) {
                continue;
            }
        }

        if (!veh.vehicleFlags.bCreateRoadBlockPeds) {
            continue;
        }

        constexpr float maxDistSq = 54.5f * 54.5f;
        if (DistanceBetweenPointsSquared2D(FindPlayerCentreOfWorld(), veh.GetPosition()) >= maxDistSq) {
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
// ASM Match: 99.6%
void CCarCtrl::ScanForPedDanger(CVehicle* pVehicle)
{
    float MinX, MinY, MaxX, MaxY, MaxSpeed;
    int32 nTop, nRight, nBottom, nLeft;
    int32 LoopY, LoopX;

    float fAvoidBlockSize;
    if (pVehicle == FindPlayerVehicle())
    {
        fAvoidBlockSize = 44.0f;
    }
    else
    {
        fAvoidBlockSize = 11.0f;
    }
    bool bSlowFlag = pVehicle->m_autoPilot.SlowingDownForPed;

    MinX = pVehicle->GetPosition().x - fAvoidBlockSize;
    MaxX = pVehicle->GetPosition().x + fAvoidBlockSize;
    MinY = pVehicle->GetPosition().y - fAvoidBlockSize;
    MaxY = pVehicle->GetPosition().y + fAvoidBlockSize;

    nLeft = VMAX(static_cast<int32>(CMaths::Floor(MinX * 0.02f + 60.0f)), 0);
    nBottom = VMAX(static_cast<int32>(CMaths::Floor(MinY * 0.02f + 60.0f)), 0);
    nRight = VMIN(static_cast<int32>(CMaths::Floor(MaxX * 0.02f + 60.0f)), 119);
    nTop = VMIN(static_cast<int32>(CMaths::Floor(MaxY * 0.02f + 60.0f)), 119);

    CWorld::AdvanceCurrentScanCode();

    MaxSpeed = pVehicle->m_autoPilot.CruiseSpeed;

    for (LoopY = nBottom; LoopY <= nTop; LoopY++)
    {
        for (LoopX = nLeft; LoopX <= nRight; LoopX++)
        {
            CRepeatSector& rsector = CWorld::GetRepeatSector(LoopX, LoopY);
            SlowCarDownForPedsSectorList(rsector.Peds, pVehicle, MinX, MinY, MaxX, MaxY, &MaxSpeed, pVehicle->m_autoPilot.CruiseSpeed);
        }
    }

    pVehicle->vehicleFlags.bWarnedPeds = true;
    pVehicle->m_autoPilot.SlowingDownForPed = bSlowFlag;
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
// ASM Match: 99.9%
void CCarCtrl::SetCoordsOfScriptCar(CVehicle* pVehicle, float NewX, float NewY, float NewZ, uint8 bClearCarOrientation, uint8 bAddOffset)
{
    int32 CarRef = (*GetVehiclePool()).GetIndex(pVehicle);

    if (NewZ <= -100.0f)
    {
        NewZ = CWorld::FindGroundZForCoord(NewX, NewY);
    }
    if (bAddOffset)
    {
        NewZ += pVehicle->GetDistanceFromCentreOfMassToBaseOfModel();
    }

    pVehicle->SetIsStatic(false);
    CTheScripts::StuckCars.ClearStuckFlagForCar(CarRef);

    if (pVehicle->GetBaseVehicleType() == VEHICLE_TYPE_BOAT)
    {
        pVehicle->Teleport(CVector(NewX, NewY, NewZ), bClearCarOrientation);
        CTheScripts::ClearSpaceForMissionEntity(CVector(NewX, NewY, NewZ), pVehicle);
        pVehicle->m_autoPilot.TempAction = TEMPACT_NONE;
        return;
    }

    pVehicle->Teleport(CVector(NewX, NewY, NewZ), bClearCarOrientation);
    switch (pVehicle->GetBaseVehicleType())
    {
        case VEHICLE_TYPE_AUTOMOBILE:
        case VEHICLE_TYPE_TRAILER:
            static_cast<CAutomobile*>(pVehicle)->PlaceOnRoadProperly();
            break;
        case VEHICLE_TYPE_BIKE:
            static_cast<CBike*>(pVehicle)->PlaceOnRoadProperly();
            break;
    }
    CTheScripts::ClearSpaceForMissionEntity(CVector(NewX, NewY, NewZ), pVehicle);
    JoinCarWithRoadAccordingToMission(pVehicle);
    pVehicle->m_autoPilot.TempAction = TEMPACT_NONE;
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
// ASM Match: 99.7%
template<typename PtrListType> void CCarCtrl::SlowCarDownForCarsSectorList(PtrListType& list, CVehicle* pVehicle, float MinX, float MinY, float MaxX, float MaxY, float* pMaxSpeed, float OriginalMaxSpeed)
{
    auto* pNode = list.GetHeadPtr();
    while (pNode != nullptr)
    {
        CVehicle* pEntity = static_cast<CVehicle*>(pNode->GetPtr());
        pNode = pNode->GetNextPtr();

        if (pEntity != pVehicle && pEntity->GetScanCode() != CWorld::GetCurrentScanCode() && pEntity->GetUsesCollision())
        {
            pEntity->SetScanCode(CWorld::GetCurrentScanCode());

            CVector centre;
            pEntity->GetBoundCentre(centre);
            if (centre.x > MinX && centre.x < MaxX && centre.y > MinY && centre.y < MaxY)
            {
                if (VABS(centre.z - pVehicle->GetPosition().z) < 10.0f)
                {
                    float DistAhead = CCollision::DistAlongLine2D(pVehicle->GetPosition().x, pVehicle->GetPosition().y, pVehicle->m_matrix->GetForward().x, pVehicle->m_matrix->GetForward().y, centre.x, centre.y);
                    float OurHeight = pVehicle->GetPosition().z + DistAhead * pVehicle->GetForward().z;
                    if (VABS(centre.z - OurHeight) < 3.0f)
                    {
                        SlowCarDownForOtherCar(pEntity, pVehicle, pMaxSpeed, OriginalMaxSpeed);
                    }
                }
            }
        }
    }
}

// 0x426220
void CCarCtrl::SlowCarDownForObject(CEntity* entity, CVehicle* vehicle, float* arg3, float arg4) {
    const CVector entityDir     = entity->GetPosition() - vehicle->GetPosition();
    const float   entityHeading = DotProduct(entityDir, vehicle->GetMatrix().GetForward());
    if (entityHeading > 0.0f && entityHeading < 20.0f) {
        if (entity->GetColModel()->GetBoundRadius() + vehicle->GetColModel()->GetBoundingBox().m_vecMax.x > fabs(DotProduct(entityDir, vehicle->GetMatrix().GetRight()))) {
            if (entityHeading >= 7.0f) {
                *arg3 = std::min(*arg3, (1.0f - (entityHeading - 7.0f) * (1.0f / 13.0f)) * arg4);
            } else {
                *arg3 = 0.0f;
            }
        }
    }
}

// 0x42D4F0
// ASM Match: 99.6%
template<typename PtrListType> void CCarCtrl::SlowCarDownForObjectsSectorList(PtrListType& list, CVehicle* pVehicle, float MinX, float MinY, float MaxX, float MaxY, float* pMaxSpeed, float OriginalMaxSpeed)
{
    auto* pNode = list.GetHeadPtr();
    while (pNode != nullptr)
    {
        CEntity* pEntity = static_cast<CEntity*>(pNode->GetPtr());
        pNode = pNode->GetNextPtr();

        if (pEntity->GetScanCode() != CWorld::GetCurrentScanCode())
        {
            pEntity->SetScanCode(CWorld::GetCurrentScanCode());

            if (pEntity->GetModelIndex() == ModelIndices::MI_ROADWORKBARRIER1 || pEntity->GetModelIndex() == ModelIndices::MI_ROADBLOCKFUCKEDCAR1 || pEntity->GetModelIndex() == ModelIndices::MI_ROADBLOCKFUCKEDCAR2)
            {
                CVector centre;
                pEntity->GetBoundCentre(centre);
                if (centre.x > MinX && centre.x < MaxX && centre.y > MinY && centre.y < MaxY)
                {
                    if (CMaths::Abs(centre.z - pVehicle->GetPosition().z) < 10.0f)
                    {
                        float DistAhead = CCollision::DistAlongLine2D(pVehicle->GetPosition().x, pVehicle->GetPosition().y, pVehicle->m_matrix->GetForward().x, pVehicle->m_matrix->GetForward().y, centre.x, centre.y);
                        float OurHeight = pVehicle->GetPosition().z + DistAhead * pVehicle->GetForward().z;
                        if (VABS(centre.z - OurHeight) < 3.0f)
                        {
                            SlowCarDownForObject(pEntity, pVehicle, pMaxSpeed, OriginalMaxSpeed);
                        }
                    }
                }
            }
        }
    }
}

// 0x42D0E0
// ASM Match: 97.6%
void CCarCtrl::SlowCarDownForOtherCar(CEntity* pEntity, CVehicle* pVehicle, float* pMaxSpeed, float OriginalMaxSpeed)
{
    float Length, DotPr;
    float CollisionT, CollisionT2;
    CVector HisFront, OurFront;

    OurFront.x = pVehicle->GetMatrix().GetForward().x;
    OurFront.y = pVehicle->GetMatrix().GetForward().y;
    Length = CMaths::Sqrt(OurFront.y * OurFront.y + OurFront.x * OurFront.x);
    if (Length != 0.0f)
    {
        Length = 1.0f / Length;
        OurFront.x *= Length;
        OurFront.y *= Length;
    }
    else
    {
        OurFront.x = 1.0f;
    }

    DotPr = (pEntity->GetPosition().x - pVehicle->GetPosition().x) * OurFront.x + (pEntity->GetPosition().y - pVehicle->GetPosition().y) * OurFront.y;
    if (DotPr < 0.0f)
    {
        return;
    }

    OurFront.z = 0.0f;
    CVehicle* pHisVehicle = static_cast<CVehicle*>(pEntity);
    float HisSpeedX = pHisVehicle->m_vecMoveSpeed.x * 60.0f;
    float HisSpeedY = pHisVehicle->m_vecMoveSpeed.y * 60.0f;
    float OurSpeedX = OurFront.x * OriginalMaxSpeed;
    float OurSpeedY = OurFront.y * OriginalMaxSpeed;

    HisFront = CVector(pEntity->GetMatrix().GetForward().x, pEntity->GetMatrix().GetForward().y, 0.0f);
    Length = CMaths::Sqrt(HisFront.y * HisFront.y + HisFront.x * HisFront.x);
    if (Length != 0.0f)
    {
        Length = 1.0f / Length;
        HisFront.x *= Length;
        HisFront.y = Length * HisFront.y;
    }
    else
    {
        HisFront.x = 1.0f;
    }

    float DeltaSpeedX = HisSpeedX - OurSpeedX;
    float DeltaSpeedY = HisSpeedY - OurSpeedY;

    CollisionT = TestCollisionBetween2MovingRects_OnlyFrontBumper(pHisVehicle, pVehicle, DeltaSpeedX, DeltaSpeedY, &OurFront, &HisFront);
    CollisionT2 = TestCollisionBetween2MovingRects(pVehicle, pHisVehicle, -DeltaSpeedX, -DeltaSpeedY, &HisFront, &OurFront);
    CollisionT = VMIN(CollisionT, CollisionT2);

    if (CollisionT >= 0.0f)
    {
        if (CollisionT < 1.5f)
        {
            pVehicle->m_autoPilot.SlowingDownForCar = true;
            pVehicle->m_autoPilot.pObstructingEntity = pEntity;
            pEntity->RegisterReference(&pVehicle->m_autoPilot.pObstructingEntity);

            float OneFrame = 1.0f / OriginalMaxSpeed;
            if (CollisionT < OneFrame)
            {
                *pMaxSpeed = 0.0f;
            }
            else if (CollisionT < 3.0f * OneFrame)
            {
                *pMaxSpeed = VMIN(*pMaxSpeed, 1.0f);
            }
            else
            {
                CollisionT = VMAX(0.0f, (CollisionT - 0.2f) * (1.0f / 1.3f));
                *pMaxSpeed = VMIN(*pMaxSpeed, CollisionT * OriginalMaxSpeed);
            }
        }

        if (CollisionT >= 0.0f && CollisionT < 0.5f && pEntity->GetIsTypeVehicle() && CTimer::GetTimeInMS() - pVehicle->m_autoPilot.LastTimeMoving > 15000 &&
            CTimer::GetTimeInMS() - pHisVehicle->m_autoPilot.LastTimeMoving > 15000)
        {
            DotPr = pVehicle->GetMatrix().GetForward().x * pEntity->GetMatrix().GetForward().x + pVehicle->GetMatrix().GetForward().y * pEntity->GetMatrix().GetForward().y;
            if (pEntity != FindPlayerVehicle() && DotPr < -0.5f && pVehicle < pEntity)
            {
                *pMaxSpeed = VMAX(*pMaxSpeed, OriginalMaxSpeed * 0.2f);
                if (pVehicle->GetStatus() == STATUS_SIMPLE)
                {
                    SwitchVehicleToRealPhysics(pVehicle);
                }
                pVehicle->m_autoPilot.DrivingMode = DRIVING_STYLE_AVOID_CARS;
                pVehicle->m_autoPilot.TempActionFinish = CTimer::GetTimeInMS() + 1000;
            }
        }
    }
}

// 0x425440
// ASM Match: 98.3%
template<typename PtrListType> void CCarCtrl::SlowCarDownForPedsSectorList(PtrListType& list, CVehicle* pVehicle, float MinX, float MinY, float MaxX, float MaxY, float* pMaxSpeed, float OriginalMaxSpeed)
{
    float CarRightSize, VelFront, CarFrontSize, DotFront, DotSide;
    CVector Delta;

    CarRightSize = CModelInfo::GetColModel(pVehicle->GetModelIndex()).GetBoundBoxMax().x;
    VelFront = DotProduct(pVehicle->GetMatrix().GetForward(), pVehicle->m_vecMoveSpeed);
    float fBrakingMinDist = VelFront * 200.0f;
    float fWarnPedMinDist = CMaths::Abs(VelFront) * 50.0f;

    bool bShouldBeAvoided;
    if (pVehicle == FindPlayerVehicle() || pVehicle->GetBaseVehicleType() == VEHICLE_TYPE_TRAIN || pVehicle->GetBaseVehicleType() == VEHICLE_TYPE_PLANE ||
        pVehicle->GetBaseVehicleType() == VEHICLE_TYPE_HELI ||
        (pVehicle->m_autoPilot.DrivingMode != DRIVING_STYLE_STOP_FOR_CARS && pVehicle->m_autoPilot.DrivingMode != DRIVING_STYLE_DRIVINGMODE_AVOIDCARS_STOPFORPEDS_OBEYLIGHTS) ||
        pVehicle->GetStatus() == STATUS_PHYSICS)
    {
        bShouldBeAvoided = true;
    }
    else
    {
        bShouldBeAvoided = false;
    }

    auto* pNode = list.GetHeadPtr();

    CarFrontSize = CModelInfo::GetColModel(pVehicle->GetModelIndex()).GetBoundBoxMax().y;

    while (pNode != nullptr)
    {
        CEntity* pEntity = static_cast<CEntity*>(pNode->GetPtr());
        pNode = pNode->GetNextPtr();

        if (pEntity->GetScanCode() != CWorld::GetCurrentScanCode() && pEntity->GetUsesCollision())
        {
            pEntity->SetScanCode(CWorld::GetCurrentScanCode());
            CVector centre = pEntity->GetPosition();

            if (centre.x > MinX && centre.x < MaxX && centre.y > MinY && centre.y < MaxY && VABS(centre.z - pVehicle->GetPosition().z) < 6.0f)
            {
                float DistAhead = CCollision::DistAlongLine2D(pVehicle->GetPosition().x, pVehicle->GetPosition().y, pVehicle->m_matrix->GetForward().x, pVehicle->m_matrix->GetForward().y, centre.x, centre.y);
                float OurHeight = pVehicle->GetPosition().z + DistAhead * pVehicle->GetForward().z;

                if (VABS(centre.z - OurHeight) < 3.0f)
                {
                    Delta = centre - pVehicle->GetPosition();
                    DotFront = DotProduct(Delta, pVehicle->GetMatrix().GetForward());

                    if ((pVehicle->m_autoPilot.DrivingMode == DRIVING_STYLE_STOP_FOR_CARS || pVehicle->m_autoPilot.DrivingMode == DRIVING_STYLE_STOP_FOR_CARS_IGNORE_LIGHTS ||
                            pVehicle->m_autoPilot.DrivingMode == DRIVING_STYLE_SLOW_DOWN_FOR_CARS || pVehicle->m_autoPilot.DrivingMode == DRIVING_STYLE_DRIVINGMODE_AVOIDCARS_STOPFORPEDS_OBEYLIGHTS) &&
                        (pEntity != FindPlayerPed() || CWorld::Players[CWorld::PlayerInFocus].GetLastTargetVehicle() != pVehicle))
                    {
                        if (DotFront > CarFrontSize && DotFront - CarFrontSize < fBrakingMinDist)
                        {
                            DotSide = CMaths::Abs(DotProduct(Delta, pVehicle->GetMatrix().GetRight()));
                            if (pVehicle->GetBaseVehicleType() == VEHICLE_TYPE_BIKE)
                            {
                                CarRightSize *= 1.6f;
                            }
                            if (DotSide <= CarRightSize + 0.5f && DotFront - CarFrontSize < 13.0f)
                            {
                                float SlowDownDist = DotFront - CarFrontSize - 1.0f;
                                *pMaxSpeed = VMIN(*pMaxSpeed, VMAX(1.0f, OriginalMaxSpeed * (VMAX(0.0f, SlowDownDist) * (1.0f / 13.0f))));
                                pVehicle->m_autoPilot.SlowingDownForPed = true;

                                if (DotFront - CarFrontSize < 4.0f)
                                {
                                    pVehicle->m_autoPilot.TempAction = TEMPACT_WAIT;
                                    pVehicle->m_autoPilot.TempActionFinish = CTimer::GetTimeInMS() + 4000;
                                }
                                if (DotFront - CarFrontSize < 2.5f)
                                {
                                    pVehicle->m_autoPilot.TempAction = TEMPACT_BRAKE;
                                    pVehicle->m_autoPilot.TempActionFinish = CTimer::GetTimeInMS() + 4000;
                                }
                            }
                        }
                    }

                    if (pEntity->GetIsTypePed())
                    {
                        CPed* pPed = static_cast<CPed*>(pEntity);

                        if (pVehicle == FindPlayerVehicle() && pVehicle->m_cHorn != 0)
                        {
                            if ((pPed->GetPosition() - pVehicle->GetPosition()).SquaredMagnitude() < 49.0f)
                            {
                                CEventPotentialGetRunOver new_event(pVehicle);
                                pPed->GetPedIntelligence()->AddEvent(new_event, false);
                            }
                        }

                        if (bShouldBeAvoided && VelFront != 0.0f && (DotFront < 0.0f ? -1 : 1) == (VelFront < 0.0f ? -1 : 1))
                        {
                            DotFront = CMaths::Abs(DotFront);
                            if (DotFront > CarFrontSize && CMaths::Abs(VelFront) > 0.05f && DotFront - CarFrontSize < fWarnPedMinDist)
                            {
                                DotSide = CMaths::Abs(DotProduct(Delta, pVehicle->GetMatrix().GetRight()));
                                if (DotSide <= CarRightSize + 0.35f)
                                {
                                    CEventPotentialGetRunOver new_event(pVehicle);
                                    pPed->GetPedIntelligence()->AddEvent(new_event, false);

                                    if (pVehicle->m_pDriver != nullptr && pVehicle->m_pDriver->IsPlayer())
                                    {
                                        pPed->GetPedIntelligence()->IncrementAngerAtPlayer(2);
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

// 0x434790
void CCarCtrl::SlowCarOnRailsDownForTrafficAndLights(CVehicle* vehicle) {
    auto& autoPilot = vehicle->m_autoPilot;

    if ((((int8)CTimer::GetFrameCounter() + (int8)(vehicle->RandomSeed)) & 3) == 0) {
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
// ASM Match: 88.9%
void CCarCtrl::SteerAIBoatWithPhysicsAttackingPlayer(CVehicle* pVehicle, float* pSteerAngle, float* pGas, float* pBrake, bool* pHandBrake)
{
    float Distance = (FindPlayerCoors() - pVehicle->GetPosition()).Magnitude();
    float AimAhead = VMIN(Distance * 0.05f, 2.0f);

    CVector Forward;
    float Length;
    Forward.x = pVehicle->GetMatrix().GetForward().x;
    Forward.y = pVehicle->GetMatrix().GetForward().y;
    Length = CMaths::Sqrt(Forward.y * Forward.y + Forward.x * Forward.x);
    if (Length != 0.0f)
    {
        Length = 1.0f / Length;
        Forward.x *= Length;
        Forward.y *= Length;
    }
    else
    {
        Forward.x = 1.0f;
    }

    float TargetX = AimAhead * FindPlayerSpeed().x * 60.0f + FindPlayerCoors().x;
    float TargetY = AimAhead * FindPlayerSpeed().y * 60.0f + FindPlayerCoors().y;

    float AngleDiff = CGeneral::GetATanOfXY(TargetX - pVehicle->GetPosition().x, TargetY - pVehicle->GetPosition().y) - CGeneral::GetATanOfXY(Forward.x, Forward.y);
    while (AngleDiff < -PI)
    {
        AngleDiff += TWO_PI;
    }
    while (AngleDiff > PI)
    {
        AngleDiff -= TWO_PI;
    }

    CVector MoveSpeed = pVehicle->GetMoveSpeed();
    float CruiseSpeed = pVehicle->m_autoPilot.CruiseSpeed;
    float SpeedDiff = CruiseSpeed - MoveSpeed.Magnitude2D() * 60.0f;
    if (SpeedDiff > 0.0f)
    {
        float SpeedRatio = SpeedDiff / CruiseSpeed;
        if (SpeedRatio > 0.25f)
        {
            *pGas = 1.0f;
        }
        else
        {
            *pGas = 1.0f - (0.25f - SpeedRatio) * 4.0f;
        }
    }
    else
    {
        *pGas = -0.1f;
        if (SpeedDiff < -5.0f)
        {
            *pGas = -0.2f;
        }
    }
    *pBrake = 0.0f;
    *pSteerAngle = AngleDiff;
    *pHandBrake = false;

    if (pVehicle->GetModelIndex() == MODEL_PREDATOR && Distance < 40.0f && AngleDiff < 0.15f)
    {
        pVehicle->FireFixedMachineGuns();
    }
}

// 0x429090
// ASM Match: 75.6%
void CCarCtrl::SteerAIBoatWithPhysicsCirclingPlayer(CVehicle* pVehicle, float* pSteerAngle, float* pGas, float* pBrake, bool* pHandBrake)
{
    float TargetOrientation;
    float BoatOrientation;
    float AngleDiff;
    float BoatDirY;
    float BoatDirX;
    float Length;
    float Speed;
    float SpeedDiff;
    float Ratio;
    CVector SpeedVector;

    CVector VecTo = FindPlayerCoors() - pVehicle->GetPosition();
    VecTo.z = 0.0f;
    VecTo.Normalise();
    float Temp = -VecTo.x;
    if ((pVehicle->RandomSeed & 1) != 0)
    {
        VecTo.x = VecTo.y * -12.0f;
        VecTo.y = Temp * -12.0f;
        VecTo.z *= -12.0f;
    }
    else
    {
        VecTo.x = VecTo.y * 26.0f;
        VecTo.y = Temp * 26.0f;
        VecTo.z *= 26.0f;
    }

    BoatDirX = pVehicle->GetMatrix().GetForward().x;
    BoatDirY = pVehicle->GetMatrix().GetForward().y;
    Length = CMaths::Sqrt(BoatDirX * BoatDirX + BoatDirY * BoatDirY);
    if (Length != 0.0f)
    {
        BoatDirX /= Length;
        BoatDirY /= Length;
    }
    else
    {
        BoatDirX = 1.0f;
    }

    TargetOrientation = CGeneral::GetATanOfXY(VecTo.x + FindPlayerCoors().x - pVehicle->GetPosition().x, VecTo.y + FindPlayerCoors().y - pVehicle->GetPosition().y);

    BoatOrientation = TargetOrientation - CGeneral::GetATanOfXY(BoatDirX, BoatDirY);

    while (BoatOrientation < -PI)
    {
        BoatOrientation += TWO_PI;
    }

    while (BoatOrientation >= PI)
    {
        BoatOrientation -= TWO_PI;
    }

    SpeedVector = pVehicle->GetMoveSpeed();
    Speed = pVehicle->m_autoPilot.CruiseSpeed;
    SpeedDiff = Speed - SpeedVector.Magnitude2D() * 60.0f;
    if (SpeedDiff <= 0.0f)
    {
        *pGas = -0.1f;
        if (SpeedDiff < -5.0f)
        {
            *pGas = -0.2f;
        }
    }
    else
    {
        Ratio = SpeedDiff / Speed;
        if (Ratio > 0.25f)
        {
            *pGas = 1.0f;
        }
        else
        {
            *pGas = 1.0f - (0.25f - Ratio) * 4.0f;
        }
    }
    *pBrake = 0.0f;
    *pSteerAngle = BoatOrientation;
    *pHandBrake = false;
}

// 0x428BE0
// ASM Match: 68.0%
void CCarCtrl::SteerAIBoatWithPhysicsHeadingForTarget(CVehicle* pBoat, float TargetX, float TargetY, float* pSteerAngle, float* pGas, float* pBrake)
{
    CVector Forward;
    float Length;

    Forward.x = pBoat->GetMatrix().GetForward().x;
    Forward.y = pBoat->GetMatrix().GetForward().y;
    Length = CMaths::Sqrt(Forward.y * Forward.y + Forward.x * Forward.x);
    if (Length != 0.0f)
    {
        Length = 1.0f / Length;
        Forward.x *= Length;
        Forward.y *= Length;
    }
    else
    {
        Forward.x = 1.0f;
    }

    float AngleDiff = CGeneral::GetATanOfXY(TargetX - pBoat->GetPosition().x, TargetY - pBoat->GetPosition().y) - CGeneral::GetATanOfXY(Forward.x, Forward.y);
    while (AngleDiff < -PI)
    {
        AngleDiff += TWO_PI;
    }
    while (AngleDiff > PI)
    {
        AngleDiff -= TWO_PI;
    }
    if (AngleDiff < -0.5f)
    {
        AngleDiff = -0.5f;
    }
    else if (AngleDiff > 0.5f)
    {
        AngleDiff = 0.5f;
    }

    CVector MoveSpeed = pBoat->GetMoveSpeed();
    float CruiseSpeed = pBoat->m_autoPilot.CruiseSpeed;
    float SpeedDiff = CruiseSpeed - MoveSpeed.Magnitude2D() * 60.0f;
    if (SpeedDiff > 0.0f)
    {
        float SpeedRatio = SpeedDiff / CruiseSpeed;
        if (SpeedRatio > 0.25f)
        {
            *pGas = 1.0f;
        }
        else
        {
            *pGas = 1.0f - (0.25f - SpeedRatio) * 4.0f;
        }
        *pBrake = 0.0f;
        *pSteerAngle = AngleDiff;
    }
    else
    {
        *pGas = -0.1f;
        if (SpeedDiff < -5.0f)
        {
            *pGas = -0.2f;
        }
        *pBrake = 0.0f;
        *pSteerAngle = AngleDiff * -1.0f;
    }
}

// 0x422B20
// ASM Match: 55.4%
void CCarCtrl::SteerAICarBlockingPlayerForwardAndBack(CVehicle* pCar, float* pSteerAngle, float* pGas, float* pBrake, bool* pHandBrake)
{
    *pSteerAngle = 0.0f;
    *pHandBrake = false;

    CVector PlayerSpeed = FindPlayerSpeed();
    PlayerSpeed.x += FindPlayerEntity()->GetMatrix().GetForward().x * 0.1f;
    PlayerSpeed.y += FindPlayerEntity()->GetMatrix().GetForward().y * 0.1f;
    PlayerSpeed.z = 0.0f;

    CVector Forward = pCar->GetMatrix().GetForward();
    Forward.z = 0.0f;
    Forward.Normalise();
    CVector Right = pCar->GetMatrix().GetRight();
    Right.z = 0.0f;
    Right.Normalise();

    float ClosingSpeed = DotProduct(Forward, PlayerSpeed);
    if (ClosingSpeed == 0.0f)
    {
        ClosingSpeed = 0.01f;
    }
    float Time = -(DotProduct(FindPlayerCoors() - pCar->GetPosition(), Forward) / ClosingSpeed);
    if (Time < 0.0f)
    {
        *pGas = 0.0f;
        *pBrake = 0.0f;
        return;
    }

    float OurSideSpeed = DotProduct(Right, pCar->GetMoveSpeed());
    float SideDist = DotProduct(Right, PlayerSpeed) * Time + DotProduct(FindPlayerCoors() - pCar->GetPosition(), Right) - OurSideSpeed * Time;
    if (SideDist > 0.0f)
    {
        *pGas = VMIN(SideDist * 0.1f, 1.0f);
        *pBrake = 0.0f;
    }
    else if (OurSideSpeed > 0.0f)
    {
        float Brake = VMIN(SideDist * -0.1f, 1.0f);
        *pGas = 0.0f;
        *pBrake = Brake;
        if (Brake > 0.95f)
        {
            *pHandBrake = true;
        }
    }
    else
    {
        *pGas = VMAX(SideDist * 0.1f, -1.0f);
        *pBrake = 0.0f;
    }
}

// 0x433BA0
// ASM Match: 94.4%
void CCarCtrl::SteerAICarParkParallel(CVehicle* pCar, float* pSteerAngle, float* pGasPedal, float* pBrakePedal, bool* pHandBrake)
{
    float Distance;
    CVector TargetCoors;

    if (!ThePaths.IsRegionLoaded(pCar->m_autoPilot.NewNode) || !ThePaths.IsRegionLoaded(pCar->m_autoPilot.OldNode))
    {
        pCar->m_autoPilot.Mission = MISSION_STOP_FOREVER;
        *pBrakePedal = 0.0f;
        *pGasPedal = 0.0f;
        *pSteerAngle = 0.0f;
        return;
    }

    CPathNode* pPathNodeNew = ThePaths.FindNodePointer(pCar->m_autoPilot.NewNode);
    CPathNode* pPathNodeOld = ThePaths.FindNodePointer(pCar->m_autoPilot.OldNode);

    if (pCar->m_autoPilot.Mission == MISSION_PARK_PARALLEL)
    {
        CVector TempVec = pPathNodeNew->GetCoors() - pPathNodeOld->GetCoors();
        TempVec.Normalise();
        TargetCoors = pPathNodeNew->GetCoors() + TempVec;
    }
    else
    {
        TargetCoors = pPathNodeNew->GetCoors();
    }

    SteerAICarWithPhysicsHeadingForTarget(pCar, nullptr, TargetCoors.x, TargetCoors.y, pSteerAngle, pGasPedal, pBrakePedal, pHandBrake);
    pCar->m_autoPilot.CruiseSpeed = VMIN(pCar->m_autoPilot.CruiseSpeed, 8);

    Distance = (TargetCoors - pCar->GetPosition()).Magnitude2D();
    if (pCar->m_autoPilot.Mission == MISSION_PARK_PARALLEL)
    {
        if (Distance < 4.0f)
        {
            pCar->m_autoPilot.Mission = MISSION_PARK_PARALLEL_2;
        }
    }
    else if (Distance < 2.0f)
    {
        pCar->m_autoPilot.Mission = MISSION_STOP_FOREVER;
        pCar->vehicleFlags.bEngineOn = false;
        pCar->vehicleFlags.bLightsOn = false;
        if (pCar->m_pDriver)
        {
            pCar->m_pDriver->GetPedIntelligence()->AddTaskPrimary(new CTaskComplexLeaveAnyCar(0, true, false), false);
        }
        for (int32 Passenger = 0; Passenger < 8; Passenger++)
        {
            if (pCar->m_apPassengers[Passenger])
            {
                pCar->m_apPassengers[Passenger]->GetPedIntelligence()->AddTaskPrimary(new CTaskComplexLeaveAnyCar(0, true, false), false);
            }
        }
    }
}

// 0x433EA0
// ASM Match: 90.0%
void CCarCtrl::SteerAICarParkPerpendicular(CVehicle* pCar, float* pSteerAngle, float* pGasPedal, float* pBrakePedal, bool* pHandBrake)
{
    float Distance;

    if (!ThePaths.IsRegionLoaded(pCar->m_autoPilot.NewNode) || !ThePaths.IsRegionLoaded(pCar->m_autoPilot.OldNode))
    {
        pCar->m_autoPilot.Mission = MISSION_STOP_FOREVER;
        *pBrakePedal = 0.0f;
        *pGasPedal = 0.0f;
        *pSteerAngle = 0.0f;
        return;
    }

    CPathNode* pPathNodeNew = ThePaths.FindNodePointer(pCar->m_autoPilot.NewNode);
    CPathNode* pPathNodeOld = ThePaths.FindNodePointer(pCar->m_autoPilot.OldNode);

    if (pCar->m_autoPilot.Mission == MISSION_PARK_PERPENDICULAR)
    {
        CVector CoorsNew(pPathNodeNew->GetCoors().x, pPathNodeNew->GetCoors().y, 0.0f);
        CVector CoorsOld(pPathNodeOld->GetCoors().x, pPathNodeOld->GetCoors().y, 0.0f);
        CVector CoorsCar(pCar->GetPosition().x, pCar->GetPosition().y, 0.0f);
        float DistToLine = CCollision::DistToMathematicalLine(&CoorsNew, &CoorsOld, &CoorsCar);
        if (DistToLine < 6.0f)
        {
            pCar->m_autoPilot.Mission = MISSION_PARK_PERPENDICULAR_2;
        }
    }

    CVector TargetCoors;
    if (pCar->m_autoPilot.Mission == MISSION_PARK_PERPENDICULAR)
    {
        TargetCoors = pPathNodeOld->GetCoors();
    }
    else
    {
        TargetCoors = pPathNodeNew->GetCoors();
    }

    SteerAICarWithPhysicsHeadingForTarget(pCar, nullptr, TargetCoors.x, TargetCoors.y, pSteerAngle, pGasPedal, pBrakePedal, pHandBrake);
    pCar->m_autoPilot.CruiseSpeed = VMIN(pCar->m_autoPilot.CruiseSpeed, 8);

    Distance = (pPathNodeNew->GetCoors() - pCar->GetPosition()).Magnitude2D();
    if (Distance < 2.0f)
    {
        pCar->m_autoPilot.Mission = MISSION_STOP_FOREVER;
        pCar->m_autoPilot.bCarHasToReverseFirst = true;
        pCar->vehicleFlags.bEngineOn = false;
        pCar->vehicleFlags.bLightsOn = false;
        if (pCar->m_pDriver)
        {
            pCar->m_pDriver->GetPedIntelligence()->AddTaskPrimary(new CTaskComplexLeaveAnyCar(0, true, false), false);
        }
        for (int32 Passenger = 0; Passenger < 8; Passenger++)
        {
            if (pCar->m_apPassengers[Passenger])
            {
                pCar->m_apPassengers[Passenger]->GetPedIntelligence()->AddTaskPrimary(new CTaskComplexLeaveAnyCar(0, true, false), false);
            }
        }
    }
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
// ASM Match: 77.9%
void CCarCtrl::SteerAICarWithPhysicsHeadingForTarget(CVehicle* pVehicle, CPhysical* pException, float TargetX, float TargetY, float* pSteerAngle, float* pGasPedal, float* pBrakePedal, bool* pHandBrake)
{
    *pHandBrake = false;

    CVector Forward;
    float Length;
    Forward.x = pVehicle->GetMatrix().GetForward().x;
    Forward.y = pVehicle->GetMatrix().GetForward().y;
    Length = CMaths::Sqrt(Forward.y * Forward.y + Forward.x * Forward.x);
    if (Length != 0.0f)
    {
        Length = 1.0f / Length;
        Forward.x *= Length;
        Forward.y *= Length;
    }
    else
    {
        Forward.x = 1.0f;
    }

    float AngleToTarget = CGeneral::GetATanOfXY(TargetX - pVehicle->GetPosition().x, TargetY - pVehicle->GetPosition().y);
    float CarAngle = CGeneral::GetATanOfXY(Forward.x, Forward.y);
    float SteerDirection;
    if (pVehicle->m_autoPilot.DrivingMode == DRIVING_STYLE_AVOID_CARS || pVehicle->m_autoPilot.DrivingMode == DRIVING_STYLE_DRIVINGMODE_AVOIDCARS_OBEYLIGHTS ||
        pVehicle->m_autoPilot.DrivingMode == DRIVING_STYLE_DRIVINGMODE_AVOIDCARS_STOPFORPEDS_OBEYLIGHTS)
    {
        SteerDirection = FindAngleToWeaveThroughTraffic(pVehicle, pException, AngleToTarget, CarAngle, 1.0f);
    }
    else
    {
        SteerDirection = AngleToTarget;
    }

    float SteerAngle = SteerDirection - CarAngle;
    while (SteerAngle < -PI)
    {
        SteerAngle += TWO_PI;
    }
    while (SteerAngle > PI)
    {
        SteerAngle -= TWO_PI;
    }

    if (pVehicle->GetMoveSpeed().Magnitude() > 0.3f && VABS(SteerAngle) > 0.7f)
    {
        *pHandBrake = true;
    }

    float MaxSteer = FindMaxSteerAngle(pVehicle);
    if (SteerAngle < -MaxSteer)
    {
        SteerAngle = -MaxSteer;
    }
    if (SteerAngle > MaxSteer)
    {
        SteerAngle = MaxSteer;
    }

    float SpeedMult = FindSpeedMultiplier(CGeneral::GetATanOfXY(TargetX - pVehicle->GetPosition().x, TargetY - pVehicle->GetPosition().y) - CarAngle, 0.4f, 1.2f, 0.4f);
    float CruiseSpeed = pVehicle->m_autoPilot.CruiseSpeed;
    *pBrakePedal = 0.0f;
    float CurrentSpeed = pVehicle->GetMoveSpeed().Magnitude() * 60.0f;
    float SpeedDiff = SpeedMult * CruiseSpeed - CurrentSpeed;
    if (SpeedDiff > 0.0f)
    {
        if (CurrentSpeed < 25.0f)
        {
            *pGasPedal = VMIN(0.1f * SpeedDiff, 1.0f);
        }
        else
        {
            *pGasPedal = 1.0f;
        }
        if (pVehicle->GetVehicleType() == VEHICLE_TYPE_BMX && SpeedDiff > 3.0f && static_cast<CBmx*>(pVehicle)->m_fControlJump <= 0.0f)
        {
            static_cast<CBmx*>(pVehicle)->m_fControlJump = 10.0f;
        }
        *pSteerAngle = SteerAngle;
    }
    else
    {
        *pGasPedal = 0.0f;
        *pBrakePedal = VMIN(SpeedDiff * -0.05f, 0.5f);
        *pSteerAngle = SteerAngle;
    }
}

// 0x4335E0
// ASM Match: 99.5%
void CCarCtrl::SteerAICarWithPhysicsTryingToBlockTarget(CVehicle* pVehicle, CEntity* pTargetEntity, float TargetX, float TargetY, float TargetVelX, float TargetVelY, float* pSteerAngle, float* pGasPedal, float* pBrakePedal, bool* pHandBrake)
{
    float Speed = CMaths::Sqrt(TargetVelX * TargetVelX + TargetVelY * TargetVelY);
    if (Speed > 0.13f)
    {
        TargetVelX *= 0.13f / Speed;
        TargetVelY *= 0.13f / Speed;
    }

    TargetX += TargetVelX * 60.0f;
    TargetY += TargetVelY * 60.0f;

    pVehicle->m_autoPilot.DrivingMode = DRIVING_STYLE_AVOID_CARS;
    SteerAICarWithPhysicsHeadingForTarget(pVehicle, nullptr, TargetX, TargetY, pSteerAngle, pGasPedal, pBrakePedal, pHandBrake);

    if ((TargetX - pVehicle->GetPosition().x) * (TargetX - pVehicle->GetPosition().x) + (TargetY - pVehicle->GetPosition().y) * (TargetY - pVehicle->GetPosition().y) < 25.0f)
    {
        if (pVehicle->m_autoPilot.Mission == MISSION_BLOCKCAR_CLOSE)
        {
            pVehicle->m_autoPilot.Mission = MISSION_BLOCKCAR_HANDBRAKESTOP;
        }
        else
        {
            pVehicle->m_autoPilot.Mission = MISSION_BLOCKPLAYER_HANDBRAKESTOP;
        }
    }
}

// 0x428990
// ASM Match: 91.5%
void CCarCtrl::SteerAICarWithPhysicsTryingToBlockTarget_Stop(CVehicle* pVehicle, float TargetX, float TargetY, float TargetVelX, float TargetVelY, float* pSteerAngle, float* pGasPedal, float* pBrakePedal, bool* pHandBrake)
{
    float DistSqr, SqrSpeedUs, SqrSpeedPlayer;

    *pSteerAngle = 0.0f;
    *pGasPedal = 0.0f;
    *pBrakePedal = 1.0f;
    *pHandBrake = true;

    DistSqr = (pVehicle->GetPosition().x - TargetX) * (pVehicle->GetPosition().x - TargetX) + (pVehicle->GetPosition().y - TargetY) * (pVehicle->GetPosition().y - TargetY);
    if (DistSqr > 100.0f)
    {
        pVehicle->m_autoPilot.Mission = pVehicle->m_autoPilot.Mission == MISSION_BLOCKCAR_HANDBRAKESTOP ? MISSION_BLOCKCAR_CLOSE : MISSION_BLOCKPLAYER_CLOSE;
    }

    if (pVehicle->m_autoPilot.Mission == MISSION_BLOCKCAR_HANDBRAKESTOP)
    {
        CTask* pActiveTask;
        if (pVehicle->m_pDriver == nullptr || (pActiveTask = pVehicle->m_pDriver->GetPedIntelligence()->GetTaskManager().GetActiveTask()) == nullptr ||
            pActiveTask->GetTaskType() != eTaskType::TASK_COMPLEX_KILL_CRIMINAL)
        {
            SqrSpeedUs = pVehicle->GetMoveSpeed().x * pVehicle->GetMoveSpeed().x + pVehicle->GetMoveSpeed().y * pVehicle->GetMoveSpeed().y;
            SqrSpeedPlayer = TargetVelX * TargetVelX + TargetVelY * TargetVelY;
            if (SqrSpeedUs < 0.0001f && SqrSpeedPlayer < 0.0004f && pVehicle->GetIsLawEnforcer())
            {
                CCarAI::TellOccupantsToLeaveCar(pVehicle);
                pVehicle->m_autoPilot.CruiseSpeed = 0;
                pVehicle->m_autoPilot.Mission = MISSION_NONE;
            }
        }
    }
    else
    {
        if (FindPlayerVehicle() && FindPlayerVehicle()->GetMoveSpeed().Magnitude() < 0.05f)
        {
            pVehicle->GetOutOfCarTimer += CTimer::GetTimeStep() * (1000.0f / 60.0f);
        }
        else
        {
            pVehicle->GetOutOfCarTimer = 0;
        }

        if ((!FindPlayerVehicle() || FindPlayerVehicle()->IsUpsideDown() || (FindPlayerVehicle()->GetMoveSpeed().Magnitude() < 0.05f && pVehicle->GetOutOfCarTimer > 2500)) &&
            pVehicle->GetIsLawEnforcer() && DistSqr < 100.0f)
        {
            CCarAI::TellOccupantsToLeaveCar(pVehicle);
            pVehicle->m_autoPilot.CruiseSpeed = 0;
            pVehicle->m_autoPilot.Mission = MISSION_NONE;
        }
    }
}

// 0x436A90
void CCarCtrl::SteerAICarWithPhysics_OnlyMission(CVehicle* vehicle, float* arg2, float* arg3, float* arg4, bool* arg5) {
    plugin::Call<0x436A90, CVehicle*, float*, float*, float*, bool*>(vehicle, arg2, arg3, arg4, arg5);
}

// 0x42AAD0
// ASM Match: 99.8%
void CCarCtrl::SteerAIHeliAsPoliceHeli(CAutomobile* pVeh)
{
    CHeli* pHeli = (CHeli*)pVeh;

    float FlightDirection = CGeneral::GetATanOfXY(pHeli->m_autoPilot.pTargetEntity->GetPosition().x - pHeli->GetPosition().x, pHeli->m_autoPilot.pTargetEntity->GetPosition().y - pHeli->GetPosition().y);
    float Distance = CMaths::Sqrt((pHeli->m_autoPilot.pTargetEntity->GetPosition().x - pHeli->GetPosition().x) * (pHeli->m_autoPilot.pTargetEntity->GetPosition().x - pHeli->GetPosition().x) +
                                (pHeli->m_autoPilot.pTargetEntity->GetPosition().y - pHeli->GetPosition().y) * (pHeli->m_autoPilot.pTargetEntity->GetPosition().y - pHeli->GetPosition().y));

    pHeli->m_LowestFlightHeight = CMaths::Max(pHeli->m_autoPilot.pTargetEntity->GetPosition().z, 6.0f);
    if (Distance > 50.0f)
    {
        pHeli->m_LowestFlightHeight = CMaths::Max(pHeli->m_LowestFlightHeight, 25.0f);
    }

    pHeli->m_autoPilot.TargetCoors = pHeli->m_autoPilot.pTargetEntity->GetPosition();
    FlyAIHeliInCertainDirection(pHeli, FlightDirection, Distance, true);

    if (pHeli->m_fHealth < 230.0f)
    {
        pHeli->m_autoPilot.Mission = MISSION_HELI_FLY_AWAY_FROM_PLAYER;
    }
}

// 0x42ACB0
// ASM Match: 82.0%
void CCarCtrl::SteerAIHeliFlyingAwayFromPlayer(CAutomobile* pVeh)
{
    FlyAIHeliInCertainDirection((CHeli*)pVeh, CGeneral::GetATanOfXY(FindPlayerCoors().x - pVeh->GetPosition().x, FindPlayerCoors().y - pVeh->GetPosition().y) + PI, 1000.0f, false);
}

// 0x4238E0
// ASM Match
void CCarCtrl::SteerAIHeliToCrashAndBurn(CAutomobile* pVeh)
{
    CHeli* pHeli = (CHeli*)pVeh;
    pHeli->m_fPitchControl = -0.3f;
    if (pHeli->RandomSeed & 1)
    {
        pHeli->m_fYawControl = pHeli->m_crashAndBurnTurnSpeed;
    }
    else
    {
        pHeli->m_fYawControl = -pHeli->m_crashAndBurnTurnSpeed;
    }
    pHeli->m_fThrottleControl = -0.5f;
    if (pHeli->RandomSeed & 1)
    {
        pHeli->m_fRollControl = 1.0f;
    }
    else
    {
        pHeli->m_fRollControl = -1.0f;
    }
}

// 0x42A750
// ASM Match: 88.1%
void CCarCtrl::SteerAIHeliToFollowEntity(CAutomobile* pVeh)
{
    CHeli* pHeli = (CHeli*)pVeh;

    pHeli->m_autoPilot.TargetCoors = pHeli->m_autoPilot.pTargetEntity->GetPosition();
    if (pHeli->m_autoPilot.AimAheadOfTarget)
    {
        CVector Forw = pHeli->m_autoPilot.pTargetEntity->GetMatrix().GetForward();
        Forw.z = 0.0f;
        pHeli->m_autoPilot.TargetCoors += Forw * pHeli->m_autoPilot.AimAheadOfTarget;
    }

    float FlightDirection = CGeneral::GetATanOfXY(pHeli->m_autoPilot.TargetCoors.x - pHeli->GetPosition().x, pHeli->m_autoPilot.TargetCoors.y - pHeli->GetPosition().y);
    float Distance = CMaths::Sqrt((pHeli->m_autoPilot.pTargetEntity->GetPosition().x - pHeli->GetPosition().x) * (pHeli->m_autoPilot.pTargetEntity->GetPosition().x - pHeli->GetPosition().x) +
                                (pHeli->m_autoPilot.pTargetEntity->GetPosition().y - pHeli->GetPosition().y) * (pHeli->m_autoPilot.pTargetEntity->GetPosition().y - pHeli->GetPosition().y));

    pHeli->m_LowestFlightHeight = CMaths::Max(pHeli->m_autoPilot.pTargetEntity->GetPosition().z, 6.0f);
    if (Distance > 50.0f)
    {
        pHeli->m_LowestFlightHeight = CMaths::Max(pHeli->m_LowestFlightHeight, 25.0f);
    }

    if (pVeh->HeliRequestedOrientation < 0.0f)
    {
        FlyAIHeliInCertainDirection(pHeli, FlightDirection, Distance, true);
    }
    else
    {
        FlyAIHeliToTarget_FixedOrientation(pHeli, pVeh->HeliRequestedOrientation, pHeli->m_autoPilot.pTargetEntity->GetPosition(), true);
    }

    if (pHeli->m_autoPilot.bWarnTargetEntity)
    {
        float Dist = (pHeli->GetPosition() - pHeli->m_autoPilot.pTargetEntity->GetPosition()).Magnitude2D();
        if (Dist < 25.0f)
        {
            if (pHeli->m_autoPilot.pTargetEntity->GetIsTypeVehicle())
            {
                CVehicle* pTargetVeh = static_cast<CVehicle*>(pHeli->m_autoPilot.pTargetEntity);
                if (pTargetVeh->GetStatus() == STATUS_SIMPLE || pTargetVeh->GetStatus() == STATUS_PHYSICS)
                {
                    pTargetVeh->m_autoPilot.Mission = MISSION_CRUISE;
                    pTargetVeh->m_autoPilot.CruiseSpeed = 100;
                    pTargetVeh->m_autoPilot.DrivingMode = DRIVING_STYLE_AVOID_CARS;
                    pTargetVeh->SetStatus(STATUS_PHYSICS);
                }
            }
            else if (pHeli->m_autoPilot.pTargetEntity->GetIsTypePed())
            {
                CPed* pPed = static_cast<CPed*>(pHeli->m_autoPilot.pTargetEntity);
                CTask* pTaskActive = pPed->GetPedIntelligence()->GetTaskActive();
                if (pTaskActive && pTaskActive->GetTaskType() == eTaskType::TASK_COMPLEX_WANDER)
                {
                    CTaskComplexWander* pTaskWander = static_cast<CTaskComplexWander*>(pTaskActive);
                    pTaskWander->SetMoveState(PEDMOVE_SPRINT);
                }
            }
            pHeli->m_autoPilot.bWarnTargetEntity = false;
        }
    }

    if (pHeli->m_autoPilot.bLeaveAfterAWhile && CTimer::GetTimeInMS() > pHeli->m_TimeOfCreation + 50000)
    {
        pHeli->m_autoPilot.bLeaveAfterAWhile = false;
        pHeli->m_autoPilot.Mission = MISSION_HELI_FLY_AWAY_FROM_PLAYER;
    }

    if (pHeli->m_autoPilot.Mission == MISSION_HELI_NEWS_BEHAVIOUR && pHeli->m_fHealth < 300.0f)
    {
        pHeli->m_autoPilot.Mission = MISSION_HELI_FLY_AWAY_FROM_PLAYER;
    }
}

// 0x42AEB0
// ASM Match: 96.8%
void CCarCtrl::SteerAIHeliToKeepEntityInView(CAutomobile* pVeh)
{
    CHeli* pHeli = (CHeli*)pVeh;

    float FlightDirection = CGeneral::GetATanOfXY(pHeli->m_autoPilot.pTargetEntity->GetPosition().x - pHeli->GetPosition().x, pHeli->m_autoPilot.pTargetEntity->GetPosition().y - pHeli->GetPosition().y);
    float Distance = CMaths::Sqrt((pHeli->m_autoPilot.pTargetEntity->GetPosition().x - pHeli->GetPosition().x) * (pHeli->m_autoPilot.pTargetEntity->GetPosition().x - pHeli->GetPosition().x) +
                                (pHeli->m_autoPilot.pTargetEntity->GetPosition().y - pHeli->GetPosition().y) * (pHeli->m_autoPilot.pTargetEntity->GetPosition().y - pHeli->GetPosition().y));

    if (Distance > pHeli->m_autoPilot.HooverDistFromTarget + pHeli->m_autoPilot.HooverDistFromTarget)
    {
        SteerAIHeliToFollowEntity(pVeh);
        return;
    }

    float OrientationDiff = FlightDirection + HALF_PI - CGeneral::GetATanOfXY(pHeli->GetMatrix().GetForward().x, pHeli->GetMatrix().GetForward().y);
    while (OrientationDiff > PI)
    {
        OrientationDiff -= TWO_PI;
    }
    while (OrientationDiff < -PI)
    {
        OrientationDiff += TWO_PI;
    }
    pHeli->m_fYawControl = OrientationDiff * -1.0f;
    pHeli->m_fYawControl = VMAX(-1.0f, VMIN(pHeli->m_fYawControl, 1.0f));

    pHeli->m_LowestFlightHeight = pHeli->m_autoPilot.pTargetEntity->GetPosition().z + 15.0f;
    pHeli->m_autoPilot.TargetCoors = pHeli->m_autoPilot.pTargetEntity->GetPosition();
    pHeli->m_DesiredHeight = pHeli->m_LowestFlightHeight;

    float PredictedHeight = pHeli->GetPosition().z + pHeli->GetMoveSpeed().z * 100.0f;
    pHeli->m_fThrottleControl = 0.3f;
    float HeightDiff = pHeli->m_DesiredHeight - PredictedHeight;
    if (HeightDiff > 0.0f)
    {
        pHeli->m_fThrottleControl += HeightDiff * 0.1f;
    }
    else
    {
        pHeli->m_fThrottleControl += HeightDiff * 0.2f;
    }
    pHeli->m_fThrottleControl += ((CGeneral::GetRandomNumber() & 15) - 7.0f) * 0.002f;
    pHeli->m_fThrottleControl = VMAX(0.0f, VMIN(pHeli->m_fThrottleControl, 1.0f));

    if (Distance < 0.5f * pHeli->m_autoPilot.HooverDistFromTarget)
    {
        pHeli->m_fRollControl = 0.5f;
    }
    else if (Distance > pHeli->m_autoPilot.HooverDistFromTarget)
    {
        pHeli->m_fRollControl = -0.5f;
    }
    else
    {
        pHeli->m_fRollControl = DotProduct(pHeli->GetMoveSpeed(), pHeli->GetMatrix().GetRight());
    }

    pHeli->m_fPitchControl = 0.0f;
    if (Distance < 1.5f * pHeli->m_autoPilot.HooverDistFromTarget)
    {
        pHeli->m_fPitchControl = DotProduct(pHeli->GetMoveSpeed(), pHeli->GetMatrix().GetForward());
    }
}

// 0x42AD30
// ASM Match: 99.7%
void CCarCtrl::SteerAIHeliToLand(CAutomobile* pVeh)
{
    CHeli* pHeli = (CHeli*)pVeh;

    float FlightDirection = CGeneral::GetATanOfXY(pHeli->m_autoPilot.TargetCoors.x - pHeli->GetPosition().x, pHeli->m_autoPilot.TargetCoors.y - pHeli->GetPosition().y);
    float Distance = CMaths::Sqrt((pHeli->m_autoPilot.TargetCoors.x - pHeli->GetPosition().x) * (pHeli->m_autoPilot.TargetCoors.x - pHeli->GetPosition().x) +
                                (pHeli->m_autoPilot.TargetCoors.y - pHeli->GetPosition().y) * (pHeli->m_autoPilot.TargetCoors.y - pHeli->GetPosition().y));
    FlyAIHeliInCertainDirection(pHeli, FlightDirection, Distance, true);

    if (Distance < 10.0f && pHeli->GetMoveSpeed().Magnitude2D() < 0.05f)
    {
        pHeli->m_MinHeightAboveTerrain = 0.0f;
        pHeli->m_LowestFlightHeight = 0.0f;
        if (pHeli->m_aWheelRatios[0] < 1.0f || pHeli->m_aWheelRatios[1] < 1.0f || pHeli->m_aWheelRatios[2] < 1.0f || pHeli->m_aWheelRatios[3] < 1.0f)
        {
            pHeli->m_fThrottleControl = 0.0f;
            pHeli->m_fYawControl = 0.0f;
            pHeli->m_fPitchControl = 0.0f;
            pHeli->m_fRollControl = 0.0f;
        }
    }
}

// 0x42A630
// ASM Match: 97.4%
void CCarCtrl::SteerAIHeliTowardsTargetCoors(CAutomobile* pVeh)
{
    CHeli* pHeli = (CHeli*)pVeh;

    if (pVeh->HeliRequestedOrientation >= 0.0f)
    {
        FlyAIHeliToTarget_FixedOrientation(pHeli, pVeh->HeliRequestedOrientation, pVeh->m_autoPilot.TargetCoors, true);
    }
    else
    {
        float FlightDirection = CGeneral::GetATanOfXY(pVeh->m_autoPilot.TargetCoors.x - pVeh->GetPosition().x, pVeh->m_autoPilot.TargetCoors.y - pVeh->GetPosition().y);
        float Distance = CMaths::Sqrt((pVeh->m_autoPilot.TargetCoors.x - pVeh->GetPosition().x) * (pVeh->m_autoPilot.TargetCoors.x - pVeh->GetPosition().x) + (pVeh->m_autoPilot.TargetCoors.y - pVeh->GetPosition().y) * (pVeh->m_autoPilot.TargetCoors.y - pVeh->GetPosition().y));
        FlyAIHeliInCertainDirection(pHeli, FlightDirection, Distance, true);
    }
}

// 0x423880
// ASM Match: 72.5%
void CCarCtrl::SteerAIPlaneToCrashAndBurn(CAutomobile* pVeh)
{
    CPlane* pPlane = (CPlane*)pVeh;
    pPlane->m_fPitchControl = -0.3f;
    if (pPlane->RandomSeed & 1)
    {
        pPlane->m_fYawControl = 1.0f;
    }
    else
    {
        pPlane->m_fYawControl = -1.0f;
    }
    pPlane->m_fThrottleControl = 0.0f;
    if (pPlane->RandomSeed & 1)
    {
        pPlane->m_fRollControl = 1.0f;
    }
    else
    {
        pPlane->m_fRollControl = -1.0f;
    }
}

// 0x4237F0
// ASM Match
void CCarCtrl::SteerAIPlaneToFollowEntity(CAutomobile* pVeh)
{
    CPlane* pPlane = static_cast<CPlane*>(pVeh);

    pPlane->m_FlightDirection = CGeneral::GetATanOfXY(pPlane->m_autoPilot.pTargetEntity->GetPosition().x - pPlane->GetPosition().x, pPlane->m_autoPilot.pTargetEntity->GetPosition().y - pPlane->GetPosition().y);
    pPlane->m_LowestFlightHeight = pPlane->m_autoPilot.pTargetEntity->GetPosition().z;

    FlyAIPlaneInCertainDirection(pPlane);
}

// 0x423790
// ASM Match
void CCarCtrl::SteerAIPlaneTowardsTargetCoors(CAutomobile* pVeh)
{
    reinterpret_cast<CPlane*>(pVeh)->m_FlightDirection = CGeneral::GetATanOfXY(pVeh->m_autoPilot.TargetCoors.x - pVeh->GetPosition().x, pVeh->m_autoPilot.TargetCoors.y - pVeh->GetPosition().y);

    FlyAIPlaneInCertainDirection(reinterpret_cast<CPlane*>(pVeh));
}

// 0x422590
// ASM Match: 99.5%
bool CCarCtrl::StopCarIfNodesAreInvalid(CVehicle* vehicle) {
    auto& ap = vehicle->m_autoPilot;
    if (ap.OldLink.IsValid() && ThePaths.IsAreaNodesAvailable(ap.OldLink)
        && ap.NewLink.IsValid() && ThePaths.IsAreaNodesAvailable(ap.NewLink)
        && !ap.OldNode.IsEmpty() && ThePaths.IsAreaNodesAvailable(ap.OldNode)
        && !ap.NewNode.IsEmpty() && ThePaths.IsAreaNodesAvailable(ap.NewNode)) {
        return false;
    }
    ap.bWaitForValidNodes = true;
    return true;
}

// 0x4222A0
// ASM Match
void CCarCtrl::SwitchBetweenPhysicsAndGhost(CVehicle* vehicle) {
    if (!vehicle->m_nPhysicalFlags.bFlyer || vehicle->m_nCreatedBy != MISSION_VEHICLE) {
        return;
    }
    if (vehicle->IsSubHeli() || vehicle->IsSubPlane() || vehicle->IsSubTrain()) {
        return;
    }

    if (vehicle->GetStatus() == STATUS_PHYSICS) {
        if (!CColStore::HasCollisionLoaded(vehicle->GetPosition(), AREA_CODE_NORMAL_WORLD)) {
            vehicle->SetStatus(STATUS_GHOST);
            if (vehicle->IsSubAutomobile()) {
                for (auto c = 0; c < 4; c++) {
                    vehicle->AsAutomobile()->m_damageManager.SetWheelStatus((eCarWheel)c, WHEEL_STATUS_OK);
                }
            }
        }
    } else if (vehicle->GetStatus() == STATUS_GHOST) {
        if (CColStore::HasCollisionLoaded(vehicle->GetPosition(), AREA_CODE_NORMAL_WORLD)) {
            vehicle->SetStatus(STATUS_PHYSICS);
            if (vehicle->IsAutomobile()) {
                vehicle->AsAutomobile()->PlaceOnRoadProperly();
            } else if (vehicle->IsBike()) {
                vehicle->AsBike()->PlaceOnRoadProperly();
            }
        }
    }
}

// 0x423FC0
// ASM Match: 99.3%
void CCarCtrl::SwitchVehicleToRealPhysics(CVehicle* vehicle) {
    vehicle->SetStatus(STATUS_PHYSICS);

    auto& ap = vehicle->m_autoPilot;
    ap.TempAction       = TEMPACT_NONE;
    ap.LastTimeNotStuck = CTimer::GetTimeInMS() + 2000;
    ap.LastTimeMoving   = CTimer::GetTimeInMS();
    vehicle->m_nFakePhysics = 0;
}

// 0x425B30
// ASM Match: 99.6%
float CCarCtrl::TestCollisionBetween2MovingRects(CVehicle* pHis, CVehicle* pOurs, float HisSpeedX, float HisSpeedY, CVector* pOurFront, CVector* pHisFront)
{
    float ReturnVal, ChangeInDist, DistFromCentralLine;
    int16 HisPoints;
    CVector LineStart;
    float MinVal1, MaxVal1, MinVal2, MaxVal2;
    float MinVal;

    float HisCarX = pHis->GetPosition().x - pOurs->GetPosition().x;
    float HisCarY = pHis->GetPosition().y - pOurs->GetPosition().y;

    CColModel& OurColModel = CModelInfo::GetColModel(pOurs->GetModelIndex());
    float OurBBHalfLengthPos = OurColModel.GetBoundBoxMax().y;
    float OurBBHalfWidth = OurColModel.GetBoundBoxMax().x;
    float OurBBHalfLengthNeg = -OurColModel.GetBoundBoxMin().y;
    CColModel& HisColModel = CModelInfo::GetColModel(pHis->GetModelIndex());
    float HisBBHalfLengthPos = HisColModel.GetBoundBoxMax().y;
    float HisBBHalfWidth = HisColModel.GetBoundBoxMax().x;

    ReturnVal = 1.0f;

    for (HisPoints = 0; HisPoints < 2; HisPoints++)
    {
        switch (HisPoints)
        {
        case 0:
            LineStart.x = HisCarX + HisBBHalfLengthPos * pHisFront->x + HisBBHalfWidth * pHisFront->y;
            LineStart.y = HisCarY + HisBBHalfLengthPos * pHisFront->y - HisBBHalfWidth * pHisFront->x;
            break;
        case 1:
            LineStart.x = HisCarX + HisBBHalfLengthPos * pHisFront->x - HisBBHalfWidth * pHisFront->y;
            LineStart.y = HisCarY + HisBBHalfLengthPos * pHisFront->y + HisBBHalfWidth * pHisFront->x;
            break;
        }

        MinVal1 = 0.0f;
        MaxVal1 = 1.0f;
        DistFromCentralLine = LineStart.x * pOurFront->y - LineStart.y * pOurFront->x;
        ChangeInDist = HisSpeedX * pOurFront->y - HisSpeedY * pOurFront->x;
        if (DistFromCentralLine > OurBBHalfWidth)
        {
            if (ChangeInDist < 0.0f)
            {
                float Temp = -(DistFromCentralLine - OurBBHalfWidth) / ChangeInDist;
                if (Temp < 1.0f)
                {
                    MinVal1 = Temp;
                    MaxVal1 = CMaths::Min(Temp - (OurBBHalfWidth + OurBBHalfWidth) / ChangeInDist, MaxVal1);
                }
                else
                {
                    MinVal1 = 1.0f;
                }
            }
            else
            {
                MinVal1 = 1.0f;
                MaxVal1 = 1.0f;
            }
        }
        else if (-OurBBHalfWidth > DistFromCentralLine)
        {
            if (ChangeInDist > 0.0f)
            {
                float Temp = -(DistFromCentralLine + OurBBHalfWidth) / ChangeInDist;
                if (Temp < 1.0f)
                {
                    MinVal1 = Temp;
                    MaxVal1 = CMaths::Min(Temp + (OurBBHalfWidth + OurBBHalfWidth) / ChangeInDist, MaxVal1);
                }
                else
                {
                    MinVal1 = 1.0f;
                }
            }
            else
            {
                MinVal1 = 1.0f;
                MaxVal1 = 1.0f;
            }
        }
        else if (ChangeInDist > 0.0f)
        {
            MaxVal1 = (OurBBHalfWidth - DistFromCentralLine) / ChangeInDist;
        }
        else if (ChangeInDist < 0.0f)
        {
            MaxVal1 = -(DistFromCentralLine + OurBBHalfWidth) / ChangeInDist;
        }

        MinVal2 = 0.0f;
        MaxVal2 = 1.0f;
        DistFromCentralLine = LineStart.x * pOurFront->x + LineStart.y * pOurFront->y;
        ChangeInDist = HisSpeedX * pOurFront->x + HisSpeedY * pOurFront->y;
        if (DistFromCentralLine > OurBBHalfLengthPos)
        {
            if (ChangeInDist < 0.0f)
            {
                float Temp = -(DistFromCentralLine - OurBBHalfLengthPos) / ChangeInDist;
                if (Temp < 1.0f)
                {
                    MinVal2 = Temp;
                    MaxVal2 = CMaths::Min(Temp - (OurBBHalfLengthNeg + OurBBHalfLengthPos) / ChangeInDist, MaxVal2);
                }
                else
                {
                    MinVal2 = 1.0f;
                }
            }
            else
            {
                MinVal2 = 1.0f;
                MaxVal2 = 1.0f;
            }
        }
        else if (-OurBBHalfLengthNeg > DistFromCentralLine)
        {
            if (ChangeInDist > 0.0f)
            {
                float Temp = -(DistFromCentralLine + OurBBHalfLengthNeg) / ChangeInDist;
                if (Temp < 1.0f)
                {
                    MinVal2 = Temp;
                    MaxVal2 = CMaths::Min(Temp + (OurBBHalfLengthNeg + OurBBHalfLengthPos) / ChangeInDist, MaxVal2);
                }
                else
                {
                    MinVal2 = 1.0f;
                }
            }
            else
            {
                MinVal2 = 1.0f;
                MaxVal2 = 1.0f;
            }
        }
        else if (ChangeInDist > 0.0f)
        {
            MaxVal2 = (OurBBHalfLengthPos - DistFromCentralLine) / ChangeInDist;
        }
        else if (ChangeInDist < 0.0f)
        {
            MaxVal2 = -(DistFromCentralLine + OurBBHalfLengthNeg) / ChangeInDist;
        }

        MinVal = CMaths::Max(MinVal1, MinVal2);
        if (MinVal < MaxVal1 && MinVal < MaxVal2)
        {
            ReturnVal = CMaths::Min(ReturnVal, MinVal);
        }
    }

    return ReturnVal;
}

// 0x425F70
// ASM Match: 99.8%
float CCarCtrl::TestCollisionBetween2MovingRects_OnlyFrontBumper(CVehicle* pHis, CVehicle* pOurs, float HisSpeedX, float HisSpeedY, CVector* pOurFront, CVector* pHisFront)
{
    float ReturnVal;
    int16 HisPoints;
    CVector LineStart;

    float HisCarX = pHis->GetPosition().x;
    float HisCarY = pHis->GetPosition().y;

    CColModel& OurColModel = CModelInfo::GetColModel(pOurs->GetModelIndex());
    float OurBBHalfLengthPos = OurColModel.GetBoundBoxMax().y;
    float OurBBHalfWidth = OurColModel.GetBoundBoxMax().x;
    float OurBBHalfLengthNeg = -OurColModel.GetBoundBoxMin().y;
    CColModel& HisColModel = CModelInfo::GetColModel(pHis->GetModelIndex());
    float HisBBHalfLengthPos = HisColModel.GetBoundBoxMax().y;
    float HisBBHalfWidth = HisColModel.GetBoundBoxMax().x;
    float HisBBHalfLengthNeg = -HisColModel.GetBoundBoxMin().y;
    float OurCarX = pOurs->GetPosition().x;
    float OurCarY = pOurs->GetPosition().y;

    ReturnVal = 1.0f;

    CVector Bumper1(OurCarX + OurBBHalfLengthPos * pOurFront->x + OurBBHalfWidth * pOurFront->y, OurCarY + OurBBHalfLengthPos * pOurFront->y - OurBBHalfWidth * pOurFront->x, 0.0f);
    CVector Bumper2(OurCarX + OurBBHalfLengthPos * pOurFront->x - OurBBHalfWidth * pOurFront->y, OurCarY + OurBBHalfLengthPos * pOurFront->y + OurBBHalfWidth * pOurFront->x, 0.0f);
    CVector BumperNormal(pOurFront->x, pOurFront->y, 0.0f);

    for (HisPoints = 0; HisPoints < 4; HisPoints++)
    {
        switch (HisPoints)
        {
        case 0:
            LineStart.x = HisCarX + HisBBHalfLengthPos * pHisFront->x + HisBBHalfWidth * pHisFront->y;
            LineStart.y = HisCarY + HisBBHalfLengthPos * pHisFront->y - HisBBHalfWidth * pHisFront->x;
            break;
        case 1:
            LineStart.x = HisCarX + HisBBHalfLengthPos * pHisFront->x - HisBBHalfWidth * pHisFront->y;
            LineStart.y = HisCarY + HisBBHalfLengthPos * pHisFront->y + HisBBHalfWidth * pHisFront->x;
            break;
        case 2:
            LineStart.x = HisCarX - HisBBHalfLengthNeg * pHisFront->x + HisBBHalfWidth * pHisFront->y;
            LineStart.y = HisCarY - HisBBHalfLengthNeg * pHisFront->y - HisBBHalfWidth * pHisFront->x;
            break;
        case 3:
            LineStart.x = HisCarX - HisBBHalfLengthNeg * pHisFront->x - HisBBHalfWidth * pHisFront->y;
            LineStart.y = HisCarY - HisBBHalfLengthNeg * pHisFront->y + HisBBHalfWidth * pHisFront->x;
            break;
        }

        CVector LineEnd = LineStart + CVector(HisSpeedX, HisSpeedY, 0.0f);

        float DotProduct1 = (LineStart.x - Bumper1.x) * BumperNormal.x + (LineStart.y - Bumper1.y) * BumperNormal.y;
        float DotProduct2 = (LineEnd.x - Bumper1.x) * BumperNormal.x + (LineEnd.y - Bumper1.y) * BumperNormal.y;

        if (DotProduct1 > 0.0f && DotProduct2 < 0.0f)
        {
            float TempDot1 = (Bumper1.x - LineStart.x) * HisSpeedY - (Bumper1.y - LineStart.y) * HisSpeedX;
            float TempDot2 = (Bumper2.x - LineStart.x) * HisSpeedY - (Bumper2.y - LineStart.y) * HisSpeedX;
            if (TempDot1 * TempDot2 < 0.0f)
            {
                float CollTime = DotProduct1 / (DotProduct1 - DotProduct2);
                ReturnVal = CMaths::Min(ReturnVal, CollTime);
            }
        }
    }

    return ReturnVal;
}

// 0x429520
// ASM Match: 99.9%
void CCarCtrl::TestWhetherToFirePlaneGuns(CVehicle* pPlane, CEntity* pTargetEntity)
{
    pPlane->vehicleFlags.bFireGun = false;

    if (pPlane->m_SelectedWeapon != CAR_WEAPON_NOT_USED && pPlane->m_SelectedWeapon != CAR_WEAPON_HEAVY_GUN)
    {
        return;
    }

    if (pTargetEntity == nullptr)
    {
        return;
    }

    CVector VecToTarget = pTargetEntity->GetPosition() - pPlane->GetPosition();
    if (VecToTarget.Magnitude() < 150.0f)
    {
        VecToTarget.Normalise();
        if (DotProduct(VecToTarget, pPlane->GetMatrix().GetForward()) > 0.8f)
        {
            pPlane->vehicleFlags.bFireGun = true;
        }
    }
}

// 0x421FE0
// ASM Match
bool CCarCtrl::ThisVehicleShouldTryNotToTurn(CVehicle* vehicle) {
    switch (vehicle->GetModelIndex()) {
    case MODEL_LINERUN:
    case MODEL_DUMPER:
    case MODEL_BUS:
    case MODEL_COACH:
    case MODEL_PACKER:
    case MODEL_FLATBED:
    case MODEL_PETRO:
    case MODEL_RDTRAIN:
    case MODEL_CEMENT:
        return true;
    default:
        return false;
    }
}

// 0x429300
// ASM Match: 99.7%
void CCarCtrl::TriggerDogFightMoves(CVehicle* pPlane, CVehicle* pTargetPlane)
{
    if (pPlane->m_autoPilot.TempAction == TEMPACT_NONE)
    {
        CVector Diff = pPlane->GetPosition() - pTargetPlane->GetPosition();
        float Dist = Diff.Magnitude();

        if (Dist < 70.0f)
        {
            Diff.Normalise();
            float DotPr = DotProduct(Diff, pTargetPlane->GetMatrix().GetForward());

            if (pPlane->GetPosition().z - pTargetPlane->GetPosition().z < 0.0f)
            {
                Dist = -(pPlane->GetPosition().z - pTargetPlane->GetPosition().z);
            }
            else
            {
                Dist = pPlane->GetPosition().z - pTargetPlane->GetPosition().z;
            }

            if (Dist < 15.0f)
            {
                switch (CGeneral::GetRandomNumber() & 0xFF)
                {
                    case 12:
                    {
                        if (DotPr > 0.0f)
                        {
                            pPlane->m_autoPilot.TempAction = TEMPACT_PLANE_FLY_STRAIGHT;
                            pPlane->m_autoPilot.TempActionFinish = CTimer::GetTimeInMS() + 1500 + (CGeneral::GetRandomNumber() & 1023);
                        }
                        break;
                    }
                    case 13:
                    {
                        if (DotPr > 0.0f)
                        {
                            pPlane->m_autoPilot.TempAction = TEMPACT_PLANE_SHARP_LEFT;
                            pPlane->m_autoPilot.TempActionFinish = CTimer::GetTimeInMS() + 700 + (CGeneral::GetRandomNumber() & 511);
                        }
                        break;
                    }
                    case 14:
                    {
                        if (DotPr > 0.0f)
                        {
                            pPlane->m_autoPilot.TempAction = TEMPACT_PLANE_SHARP_RIGHT;
                            pPlane->m_autoPilot.TempActionFinish = CTimer::GetTimeInMS() + 700 + (CGeneral::GetRandomNumber() & 511);
                        }
                        break;
                    }
                    case 15:
                    {
                        if (DotPr > 0.7f)
                        {
                            pPlane->m_autoPilot.TempAction = TEMPACT_PLANE_FLY_UP;
                            pPlane->m_autoPilot.TempActionFinish = CTimer::GetTimeInMS() + 3000 + (CGeneral::GetRandomNumber() & 2047);
                        }
                        break;
                    }
                }
            }
        }
    }
}

// 0x424000
// ASM Match: 98.7%
void CCarCtrl::UpdateCarCount(CVehicle* vehicle, uint8 bDecrease) {
    if (!bDecrease) {
        switch (vehicle->m_nCreatedBy) {
        case RANDOM_VEHICLE:
            if (vehicle->IsLawEnforcementVehicle()) {
                ++NumLawEnforcerCars;
            }
            ++NumRandomCars;
            break;
        case MISSION_VEHICLE:
            if (vehicle->IsLawEnforcementVehicle()) {
                vehicle->vehicleFlags.bIsLawEnforcer = false;
                --NumLawEnforcerCars;
            }
            ++NumMissionCars;
            break;
        case PARKED_VEHICLE:
            ++NumParkedCars;
            break;
        case PERMANENT_VEHICLE:
            ++NumPermanentVehicles;
            break;
        }
        return;
    }

    const auto Dec = [](auto& counter) {
        if (--counter < 0) {
            counter = 0;
        }
    };
    switch (vehicle->m_nCreatedBy) {
    case RANDOM_VEHICLE:
        if (vehicle->IsLawEnforcementVehicle() && (int32)--NumLawEnforcerCars < 0) {
            NumLawEnforcerCars = 0;
        }
        Dec(NumRandomCars);
        break;
    case MISSION_VEHICLE:
        Dec(NumMissionCars);
        break;
    case PARKED_VEHICLE:
        if ((int32)--NumParkedCars < 0) {
            NumParkedCars = 0;
        }
        break;
    case PERMANENT_VEHICLE:
        Dec(NumPermanentVehicles);
        break;
    }
}

// 0x436540
void CCarCtrl::UpdateCarOnRails(CVehicle* vehicle) {
    plugin::Call<0x436540, CVehicle*>(vehicle);
}

// 0x426BC0
// ASM Match: 99.3%
void CCarCtrl::WeaveForObject(CEntity* pEntity, CVehicle* pVehicle, float* pLeftAngle, float* pRightAngle)
{
    float DiffX, DiffY, Distance, Orientation;
    float BlockedRange;
    float AngleDiff;
    float ProjectedSize;
    CVector CenterPoint;
    CVector CollPoint;

    if (pEntity->GetModelIndex() == ModelIndices::MI_TRAFFICLIGHTS)
    {
        CollPoint.x = 2.957f;
        CollPoint.y = 0.147f;
    }
    else if (pEntity->GetModelIndex() == ModelIndices::MI_SINGLESTREETLIGHTS1)
    {
        CollPoint.x = 0.744f;
        CollPoint.y = 0.0f;
    }
    else if (pEntity->GetModelIndex() == ModelIndices::MI_SINGLESTREETLIGHTS2)
    {
        CollPoint.x = 0.043f;
        CollPoint.y = 0.0f;
    }
    else if (pEntity->GetModelIndex() == ModelIndices::MI_SINGLESTREETLIGHTS3)
    {
        CollPoint.x = 1.143f;
        CollPoint.y = 0.145f;
    }
    else if (pEntity->GetModelIndex() == ModelIndices::MI_DOUBLESTREETLIGHTS)
    {
        CollPoint.x = 0.0f;
        CollPoint.y = -0.048f;
    }
    else if (static_cast<CAtomicModelInfo*>(CModelInfo::GetModelInfo(pEntity->GetModelIndex()))->GetIsAnyTree() != false)
    {
        CollPoint.x = 0.0f;
        CollPoint.y = 0.0f;
    }
    else
    {
        return;
    }

    CenterPoint = pEntity->GetPosition();
    CenterPoint.x += CollPoint.y * pEntity->GetMatrix().GetForward().x + CollPoint.x * pEntity->GetMatrix().GetRight().x;
    CenterPoint.y += CollPoint.y * pEntity->GetMatrix().GetForward().y + CollPoint.x * pEntity->GetMatrix().GetRight().y;

    DiffX = CenterPoint.x - pVehicle->GetPosition().x;
    DiffY = CenterPoint.y - pVehicle->GetPosition().y;
    Orientation = CGeneral::GetATanOfXY(DiffX, DiffY);
    Distance = CMaths::Sqrt(DiffY * DiffY + DiffX * DiffX);

    BlockedRange = (CModelInfo::GetColModel(pVehicle->GetModelIndex()).GetBoundBoxMax().x * 2.4f + 0.3f) / Distance;

    AngleDiff = Orientation - *pLeftAngle;
    while (AngleDiff < -PI)
    {
        AngleDiff += TWO_PI;
    }
    while (AngleDiff > PI)
    {
        AngleDiff -= TWO_PI;
    }
    AngleDiff = VABS(AngleDiff);
    ProjectedSize = BlockedRange * 0.5f;
    if (AngleDiff < ProjectedSize)
    {
        *pLeftAngle = Orientation - ProjectedSize;
        while (*pLeftAngle < -PI)
        {
            *pLeftAngle += TWO_PI;
        }
    }

    AngleDiff = Orientation - *pRightAngle;
    while (AngleDiff < -PI)
    {
        AngleDiff += TWO_PI;
    }
    while (AngleDiff > PI)
    {
        AngleDiff -= TWO_PI;
    }
    AngleDiff = VABS(AngleDiff);
    if (AngleDiff < ProjectedSize)
    {
        *pRightAngle = Orientation + ProjectedSize;
        while (*pRightAngle > PI)
        {
            *pRightAngle -= TWO_PI;
        }
    }
}

// 0x421A50
// ASM Match: not measured
static bool TestForThisAngle(float Angle, CVector* pLine1Start, CVector* pLine1Delta, CVector* pLine2Start, CVector* pLine2Delta, CVector* pLine1Start_Other, CVector* pLine1Delta_Other, CVector* pLine2Start_Other, CVector* pLine2Delta_Other, float OtherCarMoveSpeedX, float OtherCarMoveSpeedY, float OurMoveSpeed, bool bSwapRound)
{
    CVector Coors1, Coors2, Coors3, Coors4;

    float OurSpeedX = CMaths::Cos(Angle) * OurMoveSpeed;
    float OurSpeedY = CMaths::Sin(Angle) * OurMoveSpeed;

    float SpeedX_Combined = OtherCarMoveSpeedX - OurSpeedX;
    float SpeedY_Combined = OtherCarMoveSpeedY - OurSpeedY;

    CVector DeltaVec = CVector(SpeedX_Combined, SpeedY_Combined, 0.0f) * 100.0f;

    if (bSwapRound)
    {
        CVector* pTemp;

        pTemp = pLine1Start;
        pLine1Start = pLine1Start_Other;
        pLine1Start_Other = pTemp;

        pTemp = pLine1Delta;
        pLine1Delta = pLine1Delta_Other;
        pLine1Delta_Other = pTemp;

        pTemp = pLine2Start;
        pLine2Start = pLine2Start_Other;
        pLine2Start_Other = pTemp;

        pTemp = pLine2Delta;
        pLine2Delta = pLine2Delta_Other;
        pLine2Delta_Other = pTemp;

        DeltaVec = -DeltaVec;
    }

    Coors1 = *pLine1Start_Other;
    Coors2 = Coors1 + *pLine1Delta_Other;
    Coors3 = Coors1 + DeltaVec;
    Coors4 = Coors2 + DeltaVec;

    if (CCollision::Test2DLineAgainst2DLine(pLine1Start->x, pLine1Start->y, pLine1Delta->x, pLine1Delta->y, Coors1.x, Coors1.y, Coors2.x - Coors1.x, Coors2.y - Coors1.y) ||
        CCollision::Test2DLineAgainst2DLine(pLine1Start->x, pLine1Start->y, pLine1Delta->x, pLine1Delta->y, Coors2.x, Coors2.y, Coors3.x - Coors2.x, Coors3.y - Coors2.y) ||
        CCollision::Test2DLineAgainst2DLine(pLine1Start->x, pLine1Start->y, pLine1Delta->x, pLine1Delta->y, Coors3.x, Coors3.y, Coors4.x - Coors3.x, Coors4.y - Coors3.y) ||
        CCollision::Test2DLineAgainst2DLine(pLine1Start->x, pLine1Start->y, pLine1Delta->x, pLine1Delta->y, Coors4.x, Coors4.y, Coors1.x - Coors4.x, Coors1.y - Coors4.y) ||
        CCollision::Test2DLineAgainst2DLine(pLine2Start->x, pLine2Start->y, pLine2Delta->x, pLine2Delta->y, Coors1.x, Coors1.y, Coors2.x - Coors1.x, Coors2.y - Coors1.y) ||
        CCollision::Test2DLineAgainst2DLine(pLine2Start->x, pLine2Start->y, pLine2Delta->x, pLine2Delta->y, Coors2.x, Coors2.y, Coors3.x - Coors2.x, Coors3.y - Coors2.y) ||
        CCollision::Test2DLineAgainst2DLine(pLine2Start->x, pLine2Start->y, pLine2Delta->x, pLine2Delta->y, Coors3.x, Coors3.y, Coors4.x - Coors3.x, Coors4.y - Coors3.y) ||
        CCollision::Test2DLineAgainst2DLine(pLine2Start->x, pLine2Start->y, pLine2Delta->x, pLine2Delta->y, Coors4.x, Coors4.y, Coors1.x - Coors4.x, Coors1.y - Coors4.y))
    {
        return true;
    }

    Coors1 = *pLine2Start_Other;
    Coors2 = Coors1 + *pLine2Delta_Other;
    Coors3 = Coors1 + DeltaVec;
    Coors4 = Coors2 + DeltaVec;

    if (CCollision::Test2DLineAgainst2DLine(pLine1Start->x, pLine1Start->y, pLine1Delta->x, pLine1Delta->y, Coors1.x, Coors1.y, Coors2.x - Coors1.x, Coors2.y - Coors1.y) ||
        CCollision::Test2DLineAgainst2DLine(pLine1Start->x, pLine1Start->y, pLine1Delta->x, pLine1Delta->y, Coors2.x, Coors2.y, Coors3.x - Coors2.x, Coors3.y - Coors2.y) ||
        CCollision::Test2DLineAgainst2DLine(pLine1Start->x, pLine1Start->y, pLine1Delta->x, pLine1Delta->y, Coors3.x, Coors3.y, Coors4.x - Coors3.x, Coors4.y - Coors3.y) ||
        CCollision::Test2DLineAgainst2DLine(pLine1Start->x, pLine1Start->y, pLine1Delta->x, pLine1Delta->y, Coors4.x, Coors4.y, Coors1.x - Coors4.x, Coors1.y - Coors4.y) ||
        CCollision::Test2DLineAgainst2DLine(pLine2Start->x, pLine2Start->y, pLine2Delta->x, pLine2Delta->y, Coors1.x, Coors1.y, Coors2.x - Coors1.x, Coors2.y - Coors1.y) ||
        CCollision::Test2DLineAgainst2DLine(pLine2Start->x, pLine2Start->y, pLine2Delta->x, pLine2Delta->y, Coors2.x, Coors2.y, Coors3.x - Coors2.x, Coors3.y - Coors2.y) ||
        CCollision::Test2DLineAgainst2DLine(pLine2Start->x, pLine2Start->y, pLine2Delta->x, pLine2Delta->y, Coors3.x, Coors3.y, Coors4.x - Coors3.x, Coors4.y - Coors3.y) ||
        CCollision::Test2DLineAgainst2DLine(pLine2Start->x, pLine2Start->y, pLine2Delta->x, pLine2Delta->y, Coors4.x, Coors4.y, Coors1.x - Coors4.x, Coors1.y - Coors4.y))
    {
        return true;
    }

    return false;
}

// 0x426350
// ASM Match: 96.4%
void CCarCtrl::WeaveForOtherCar(CEntity* pEntity, CVehicle* pVehicle, float* pLeftAngle, float* pRightAngle)
{
    CVehicle* pOtherCar;

    if (pVehicle->m_autoPilot.Mission == MISSION_RAMPLAYER_CLOSE && pEntity == FindPlayerVehicle())
    {
        return;
    }
    if (pVehicle->m_autoPilot.Mission == MISSION_RAMCAR_CLOSE && pEntity == pVehicle->m_autoPilot.pTargetEntity)
    {
        return;
    }
    if (pVehicle->m_autoPilot.Mission == MISSION_FOLLOWCAR_CLOSE)
    {
        if (pEntity == pVehicle->m_autoPilot.pTargetEntity)
        {
            return;
        }
        if (pEntity->GetIsTypeVehicle() && static_cast<CVehicle*>(pEntity)->vehicleFlags.bPartOfConvoy)
        {
            return;
        }
    }
    if (pVehicle->m_autoPilot.Mission == MISSION_KILLPED_CLOSE && pEntity->GetIsTypePed() && static_cast<CPed*>(pEntity)->bInVehicle &&
        static_cast<CPed*>(pEntity)->m_pMyVehicle == pVehicle->m_autoPilot.pTargetEntity)
    {
        return;
    }

    pOtherCar = static_cast<CVehicle*>(pEntity);
    if ((pOtherCar->m_autoPilot.Mission == MISSION_ESCORT_LEFT || pOtherCar->m_autoPilot.Mission == MISSION_ESCORT_RIGHT ||
            pOtherCar->m_autoPilot.Mission == MISSION_ESCORT_FRONT || pOtherCar->m_autoPilot.Mission == MISSION_ESCORT_REAR ||
            pOtherCar->m_autoPilot.Mission == MISSION_PROTECTION_REAR || pOtherCar->m_autoPilot.Mission == MISSION_PROTECTION_FRONT) &&
        pOtherCar->m_autoPilot.pTargetEntity == pVehicle)
    {
        return;
    }

    float DiffX = pOtherCar->GetPosition().x - pVehicle->GetPosition().x;
    float DiffY = pOtherCar->GetPosition().y - pVehicle->GetPosition().y;

    CVector Line2Start, Line1Delta, Line1Start, Line2Delta;
    float DotProd1;
    float DotProd2;

    DotProd1 = DiffX * pVehicle->GetMatrix().GetForward().x + DiffY * pVehicle->GetMatrix().GetForward().y;
    if (DotProd1 < 0.0f)
    {
        return;
    }
    DotProd2 = DiffX * pVehicle->GetMatrix().GetRight().x + DiffY * pVehicle->GetMatrix().GetRight().y;

    Line1Start = pVehicle->GetMatrix() * CVector(CModelInfo::GetColModel(pVehicle->GetModelIndex()).GetBoundBoxMin().x - 0.2f, DotProd1 > 0.0f ? CModelInfo::GetColModel(pVehicle->GetModelIndex()).GetBoundBoxMax().y + 0.2f : CModelInfo::GetColModel(pVehicle->GetModelIndex()).GetBoundBoxMin().y - 0.2f, 0.0f);
    Line1Delta = pVehicle->GetMatrix().TransformVector(CVector(CModelInfo::GetColModel(pVehicle->GetModelIndex()).GetBoundBoxMax().x + 0.2f - CModelInfo::GetColModel(pVehicle->GetModelIndex()).GetBoundBoxMin().x - 0.2f, 0.0f, 0.0f));
    if (DotProd2 > 0.0f)
    {
        Line2Start = pVehicle->GetMatrix() * CVector(CModelInfo::GetColModel(pVehicle->GetModelIndex()).GetBoundBoxMax().x + 0.2f, CModelInfo::GetColModel(pVehicle->GetModelIndex()).GetBoundBoxMin().y - 0.2f, 0.0f);
    }
    else
    {
        Line2Start = pVehicle->GetMatrix() * CVector(CModelInfo::GetColModel(pVehicle->GetModelIndex()).GetBoundBoxMin().x - 0.2f, CModelInfo::GetColModel(pVehicle->GetModelIndex()).GetBoundBoxMin().y - 0.2f, 0.0f);
    }
    Line2Delta = pVehicle->GetMatrix().TransformVector(CVector(0.0f, CModelInfo::GetColModel(pVehicle->GetModelIndex()).GetBoundBoxMax().y + 0.2f - CModelInfo::GetColModel(pVehicle->GetModelIndex()).GetBoundBoxMin().y - 0.2f, 0.0f));

    CVector Line1Start_Other, Line1Delta_Other, Line2Start_Other, Line2Delta_Other;

    DotProd2 = DiffX * pOtherCar->GetMatrix().GetRight().x + DiffY * pOtherCar->GetMatrix().GetRight().y;

    if (DiffX * pOtherCar->GetMatrix().GetForward().x + DiffY * pOtherCar->GetMatrix().GetForward().y < 0.0f)
    {
        Line1Start_Other = pOtherCar->GetMatrix() * CVector(CModelInfo::GetColModel(pOtherCar->GetModelIndex()).GetBoundBoxMin().x - 0.2f, CModelInfo::GetColModel(pOtherCar->GetModelIndex()).GetBoundBoxMax().y + 0.2f, 0.0f);
    }
    else
    {
        Line1Start_Other = pOtherCar->GetMatrix() * CVector(CModelInfo::GetColModel(pOtherCar->GetModelIndex()).GetBoundBoxMin().x - 0.2f, CModelInfo::GetColModel(pOtherCar->GetModelIndex()).GetBoundBoxMin().y - 0.2f, 0.0f);
    }
    Line1Delta_Other = pOtherCar->GetMatrix().TransformVector(CVector(CModelInfo::GetColModel(pOtherCar->GetModelIndex()).GetBoundBoxMax().x + 0.2f - CModelInfo::GetColModel(pOtherCar->GetModelIndex()).GetBoundBoxMin().x - 0.2f, 0.0f, 0.0f));
    if (DotProd2 < 0.0f)
    {
        Line2Start_Other = pOtherCar->GetMatrix() * CVector(CModelInfo::GetColModel(pOtherCar->GetModelIndex()).GetBoundBoxMax().x + 0.2f, CModelInfo::GetColModel(pOtherCar->GetModelIndex()).GetBoundBoxMin().y - 0.2f, 0.0f);
    }
    else
    {
        Line2Start_Other = pOtherCar->GetMatrix() * CVector(CModelInfo::GetColModel(pOtherCar->GetModelIndex()).GetBoundBoxMin().x - 0.2f, CModelInfo::GetColModel(pOtherCar->GetModelIndex()).GetBoundBoxMin().y - 0.2f, 0.0f);
    }
    Line2Delta_Other = pOtherCar->GetMatrix().TransformVector(CVector(0.0f, CModelInfo::GetColModel(pOtherCar->GetModelIndex()).GetBoundBoxMax().y + 0.2f - CModelInfo::GetColModel(pOtherCar->GetModelIndex()).GetBoundBoxMin().y - 0.2f, 0.0f));

    bool bSwapRound = false;
    if (CModelInfo::GetColModel(pVehicle->GetModelIndex()).GetBoundBoxMax().x < CModelInfo::GetColModel(pOtherCar->GetModelIndex()).GetBoundBoxMax().x)
    {
        bSwapRound = true;
    }

    float OurMoveSpeed = CMaths::Sqrt(pVehicle->m_vecMoveSpeed.x * pVehicle->m_vecMoveSpeed.x + pVehicle->m_vecMoveSpeed.y * pVehicle->m_vecMoveSpeed.y);

    int32 Tests = 0;
    while (Tests < 8 && TestForThisAngle(*pLeftAngle, &Line1Start, &Line1Delta, &Line2Start, &Line2Delta, &Line1Start_Other, &Line1Delta_Other, &Line2Start_Other, &Line2Delta_Other,
               pOtherCar->m_vecMoveSpeed.x, pOtherCar->m_vecMoveSpeed.y, OurMoveSpeed, bSwapRound))
    {
        Tests++;
        *pLeftAngle -= 0.10471976f;
        if (*pLeftAngle < 0.0f)
        {
            *pLeftAngle += TWO_PI;
        }
    }
    Tests = 0;
    while (Tests < 8 && TestForThisAngle(*pRightAngle, &Line1Start, &Line1Delta, &Line2Start, &Line2Delta, &Line1Start_Other, &Line1Delta_Other, &Line2Start_Other, &Line2Delta_Other,
               pOtherCar->m_vecMoveSpeed.x, pOtherCar->m_vecMoveSpeed.y, OurMoveSpeed, bSwapRound))
    {
        Tests++;
        *pRightAngle += 0.10471976f;
        if (*pRightAngle > TWO_PI)
        {
            *pRightAngle -= TWO_PI;
        }
    }
}

// 0x42D680
// ASM Match: 99.8%
template<typename PtrListType> void CCarCtrl::WeaveThroughCarsSectorList(PtrListType& list, CVehicle* pVehicle, CPhysical* pException, float MinX, float MinY, float MaxX, float MaxY, float* pLeftAngle, float* pRightAngle)
{
    auto* pNode = list.GetHeadPtr();
    while (pNode != nullptr)
    {
        CVehicle* pEntity = static_cast<CVehicle*>(pNode->GetPtr());
        pNode = pNode->GetNextPtr();

        if (pEntity->GetScanCode() != CWorld::GetCurrentScanCode() && pEntity->GetUsesCollision() && pEntity != pException)
        {
            pEntity->SetScanCode(CWorld::GetCurrentScanCode());

            CVector centre;
            pEntity->GetBoundCentre(centre);
            if (centre.x > MinX && centre.x < MaxX && centre.y > MinY && centre.y < MaxY)
            {
                if (VABS(pEntity->GetPosition().z - pVehicle->GetPosition().z) < 8.0f && pEntity != pVehicle &&
                    (!pVehicle->vehicleFlags.bPartOfConvoy || !pEntity->vehicleFlags.bPartOfConvoy))
                {
                    WeaveForOtherCar(pEntity, pVehicle, pLeftAngle, pRightAngle);
                }
            }
        }
    }
}

// 0x42D950
// ASM Match: 99.8%
template<typename PtrListType> void CCarCtrl::WeaveThroughObjectsSectorList(PtrListType& list, CVehicle* pVehicle, float MinX, float MinY, float MaxX, float MaxY, float* pLeftAngle, float* pRightAngle)
{
    auto* pNode = list.GetHeadPtr();
    while (pNode != nullptr)
    {
        CEntity* pEntity = static_cast<CEntity*>(pNode->GetPtr());
        pNode = pNode->GetNextPtr();

        if (pEntity->GetScanCode() != CWorld::GetCurrentScanCode() && pEntity->GetUsesCollision())
        {
            pEntity->SetScanCode(CWorld::GetCurrentScanCode());

            if (pEntity->GetPosition().x > MinX && pEntity->GetPosition().x < MaxX && pEntity->GetPosition().y > MinY && pEntity->GetPosition().y < MaxY)
            {
                if (VABS(pEntity->GetPosition().z - pVehicle->GetPosition().z) < 8.0f && pEntity->GetMatrix().GetUp().z > 0.9f)
                {
                    WeaveForObject(pEntity, pVehicle, pLeftAngle, pRightAngle);
                }
            }
        }
    }
}

// 0x426970
// ASM Match: not measured
void CCarCtrl::WeaveForPed(CEntity* pEntity, CVehicle* pVehicle, float* pLeftAngle, float* pRightAngle)
{
    float DiffX, DiffY, Distance, Orientation;
    float BlockedRange;
    float AngleDiff;
    float ProjectedSize;

    if (pVehicle->m_autoPilot.Mission == MISSION_RAMPLAYER_CLOSE && pEntity == FindPlayerPed())
    {
        return;
    }
    if (pVehicle->m_autoPilot.Mission == MISSION_KILLPED_CLOSE && pEntity == pVehicle->m_autoPilot.pTargetEntity)
    {
        return;
    }

    DiffX = pEntity->GetPosition().x - pVehicle->GetPosition().x;
    DiffY = pEntity->GetPosition().y - pVehicle->GetPosition().y;
    Orientation = CGeneral::GetATanOfXY(DiffX, DiffY);
    Distance = CMaths::Sqrt(DiffY * DiffY + DiffX * DiffX);
    if (Distance < 1.0f)
    {
        return;
    }

    BlockedRange = (CModelInfo::GetColModel(pVehicle->GetModelIndex()).GetBoundBoxMax().x * 2.4f + 0.8f) / Distance;

    AngleDiff = Orientation - *pLeftAngle;
    while (AngleDiff < -PI)
    {
        AngleDiff += TWO_PI;
    }
    while (AngleDiff > PI)
    {
        AngleDiff -= TWO_PI;
    }
    AngleDiff = VABS(AngleDiff);
    ProjectedSize = BlockedRange * 0.5f;
    if (AngleDiff < ProjectedSize)
    {
        *pLeftAngle = Orientation - ProjectedSize;
        while (*pLeftAngle < -PI)
        {
            *pLeftAngle += TWO_PI;
        }
    }

    AngleDiff = Orientation - *pRightAngle;
    while (AngleDiff < -PI)
    {
        AngleDiff += TWO_PI;
    }
    while (AngleDiff > PI)
    {
        AngleDiff -= TWO_PI;
    }
    AngleDiff = VABS(AngleDiff);
    if (AngleDiff < ProjectedSize)
    {
        *pRightAngle = ProjectedSize + Orientation;
        while (*pRightAngle > PI)
        {
            *pRightAngle -= TWO_PI;
        }
    }
}

// 0x42D7E0
// ASM Match: 99.8%
template<typename PtrListType> void CCarCtrl::WeaveThroughPedsSectorList(PtrListType& list, CVehicle* pVehicle, CPhysical* pException, float MinX, float MinY, float MaxX, float MaxY, float* pLeftAngle, float* pRightAngle)
{
    auto* pNode = list.GetHeadPtr();
    while (pNode != nullptr)
    {
        CPed* pEntity = static_cast<CPed*>(pNode->GetPtr());
        pNode = pNode->GetNextPtr();

        if (pEntity->GetScanCode() != CWorld::GetCurrentScanCode() && pEntity->GetUsesCollision() && pEntity != pException)
        {
            pEntity->SetScanCode(CWorld::GetCurrentScanCode());

            if (pEntity->GetPosition().x > MinX && pEntity->GetPosition().x < MaxX && pEntity->GetPosition().y > MinY && pEntity->GetPosition().y < MaxY)
            {
                if (VABS(pEntity->GetPosition().z - pVehicle->GetPosition().z) < 4.0f && pEntity->m_pEntityStandingOn != pVehicle && pEntity->m_pAttachedTo != pVehicle)
                {
                    WeaveForPed(pEntity, pVehicle, pLeftAngle, pRightAngle);
                }
            }
        }
    }
}

// 0x427FE0
float CCarCtrl::FindMaxSteerAngle(CVehicle* veh) {
    return std::clamp(0.9f - veh->GetMoveSpeed().Magnitude(), 0.2f, 0.7f);
}

bool CCarCtrl::ThisRoadObjectCouldMove(int16 mi)
{
#ifdef GTA_BRIDGE
	return mi == MI_BRIDGELIFT || mi == MI_BRIDGEROADSEGMENT;
#else
	return false;
#endif
}

bool CCarCtrl::MapCouldMoveInThisArea(float x, float y)
{
#ifdef GTA_BRIDGE	// actually they forgot that in VC...
	// bridge moves up and down
	return x > -342.0f && x < -219.0f &&
		y > -677.0f && y < -580.0f;
#else
	return false;
#endif
}
