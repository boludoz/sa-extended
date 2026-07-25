#include "StdInc.h"

#include "RoadBlocks.h"

#include "PathFind.h"
#include "Gangs.h"
#include "PopCycle.h"
#include "PedPlacement.h"
#include "VisibilityPlugins.h"
#include "Entity/Ped/CopPed.h"
#include "Entity/Ped/CivilianPed.h"
#include "Events/EventScriptCommand.h"
#include "Tasks/TaskTypes/TaskComplexWanderCop.h"
#include "Tasks/TaskTypes/TaskSimpleStandStill.h"
#include "Tasks/TaskTypes/TaskComplexKillPedOnFoot.h"
#include "Population.h"
#include "Wanted.h"
#include "CarCtrl.h"
#include "DamageManager.h"
#include "FireManager.h"
#include "ModelIndices.h"
#include "Entity/Object/Object.h"
#include "Entity/Vehicle/Automobile.h"

void CRoadBlocks::InjectHooks() {
    RH_ScopedClass(CRoadBlocks);
    RH_ScopedCategoryGlobal();

    RH_ScopedInstall(Init, 0x461100);
    RH_ScopedInstall(ClearScriptRoadBlocks, 0x460EC0);
    RH_ScopedInstall(ClearSpaceForRoadBlockObject, 0x461020);
    RH_ScopedInstall(CreateRoadBlockBetween2Points, 0x4619C0);
    RH_ScopedInstall(GenerateRoadBlockCopsForCar, 0x461170);
    RH_ScopedInstall(GenerateRoadBlocks, 0x4629E0);
    RH_ScopedInstall(GetRoadBlockNodeInfo, 0x460EE0);
    RH_ScopedInstall(RegisterScriptRoadBlock, 0x460DF0);
}

// 0x461100
void CRoadBlocks::Init() {
    rng::fill(InOrOut, true);

    bGenerateDynamicRoadBlocks = false;

    auto* const file = CFileMgr::OpenFile("data\\paths\\roadblox.dat", "rb");
    CFileMgr::Read(file, &NumRoadBlocks, sizeof(NumRoadBlocks));
    CFileMgr::Read(file, RoadBlockNodes.data(), sizeof(RoadBlockNodes));
    CFileMgr::CloseFile(file);

    ClearScriptRoadBlocks();
}

// 0x460EC0
void CRoadBlocks::ClearScriptRoadBlocks() {
    for (auto& rb : aScriptRoadBlocks) {
        rb.bActive = false;
    }
}

// 0x461020
bool CRoadBlocks::ClearSpaceForRoadBlockObject(CVector cornerA, CVector cornerB) {
    int16    numFound{};
    CEntity* found[2]{};
    CWorld::FindObjectsIntersectingCube(cornerA, cornerB, &numFound, (int16)std::size(found), found, false, true, true, true, false);

    if (numFound > (int16)std::size(found)) { // More in the way than we were willing to look at
        return false;
    }

    for (auto* const entity : std::span{ found, (size_t)std::max<int16>(numFound, 0) }) {
        switch (entity->m_nType) {
        case ENTITY_TYPE_VEHICLE: {
            const auto veh = entity->AsVehicle();
            if (!veh->CanBeDeleted()) {
                return false;
            }
            if (veh->vehicleFlags.bCreateRoadBlockPeds) { // It's part of the roadblock itself, leave it be
                continue;
            }
            break;
        }
        case ENTITY_TYPE_PED:
            if (!entity->AsPed()->CanBeDeleted()) {
                return false;
            }
            break;
        case ENTITY_TYPE_OBJECT: {
            const auto obj = entity->AsObject();
            if (obj->m_nObjectType == OBJECT_GAME || !obj->CanBeDeleted()) {
                return false;
            }
            break;
        }
        default:
            return false;
        }

        CWorld::Remove(entity);
        delete entity;
    }

    return true;
}

// 0x4619C0
void CRoadBlocks::CreateRoadBlockBetween2Points(CVector from, CVector to, bool isGangRoadBlock) {
    //! How many vehicles a single roadblock may consist of
    constexpr auto MAX_ROADBLOCK_VEHICLES = 5;

    //! How many barriers get lined up alongside it
    constexpr auto MAX_ROADBLOCK_BARRIERS = 8;

    //! Temporary props stay around for 10 minutes
    constexpr auto TEMP_OBJECT_LIFETIME = 600'000u;

    const auto length = (to - from).Magnitude();
    const auto center = (from + to) * 0.5f;

    auto along = to - from;
    along.Normalise();

    auto across = CVector{ along.y, -along.x, 0.0f };
    across.Normalise();
    if (DotProduct(FindPlayerCoors() - center, across) < 0.0f) { // Point the block at the player
        across *= -1.0f;
    }

    eModelID model;
    if (isGangRoadBlock) {
        model = CPopulation::PickRiotRoadBlockCar();
        if (model == MODEL_INVALID) {
            return;
        }
    } else {
        const auto* const wanted = FindPlayerWanted();
        model = wanted->AreArmyRequired()  ? MODEL_BARRACKS
              : wanted->AreFbiRequired()   ? MODEL_FBIRANCH
              : wanted->AreSwatRequired()  ? MODEL_ENFORCER
                                           : CStreaming::GetDefaultCopCarModel(false);
        if (!CStreaming::IsModelLoaded(model)) {
            model = CStreaming::GetDefaultCopCarModel(false);
        }
        if (model == MODEL_COPBIKE) { // A bike is no use as a barricade
            return;
        }
    }

    const auto vehBB = CModelInfo::GetModelInfo(model)->GetColModel()->m_boundBox;

    // How much of the line one vehicle eats up, depending on how it is turned
    const auto sidewaysSpan = vehBB.m_vecMax.x - vehBB.m_vecMin.x + 2.0f;
    const auto forwardSpan  = vehBB.m_vecMax.y - vehBB.m_vecMin.y + 0.2f + (isGangRoadBlock ? 0.5f : 0.0f);

    // Decide the orientation of each vehicle up front, then see how many actually fit
    std::array<bool, MAX_ROADBLOCK_VEHICLES> isSideways{};
    auto numVehicles = 0;
    auto lengthLeft  = length;
    for (auto i = 0; i < MAX_ROADBLOCK_VEHICLES; i++) {
        isSideways[i] = model == MODEL_BARRACKS || isGangRoadBlock
                      ? false
                      : (CGeneral::GetRandomNumber() & 1) != 0;

        const auto span = isSideways[i] ? sidewaysSpan : forwardSpan;
        if (lengthLeft < span) {
            break;
        }
        lengthLeft -= span;
        if (++numVehicles == MAX_ROADBLOCK_VEHICLES) {
            break;
        }
    }

    const auto vehGap = lengthLeft / (float)(numVehicles + 1);

    auto cursor = vehGap;
    for (auto i = 0; i < numVehicles; i++) {
        auto onItsSide = false; // Riot roadblocks like to tip cars over

        CMatrix m{};
        float   dist;
        if (isSideways[i]) {
            dist = sidewaysSpan * 0.5f + cursor;

            m.GetRight()   = CVector{ across.y, -across.x, 0.0f };
            m.GetForward() = across;
            m.GetUp()      = CVector{ 0.0f, 0.0f, 1.0f };
        } else {
            dist = forwardSpan * 0.5f + cursor;

            if (isGangRoadBlock) {
                onItsSide = CGeneral::GetRandomNumber() < 64;
            }

            if (onItsSide) {
                m.GetRight()   = CVector{ 0.0f, 0.0f, 1.0f };
                m.GetForward() = along;
                m.GetUp()      = CVector{ -along.y, along.x, 0.0f };
            } else {
                m.GetRight()   = CVector{ along.y, -along.x, 0.0f };
                m.GetForward() = along;
                m.GetUp()      = CVector{ 0.0f, 0.0f, 1.0f };
            }

            if (CGeneral::GetRandomNumber() & 1) { // Face the other way down the road
                if (onItsSide) {
                    m.RotateY(PI);
                } else {
                    m.RotateZ(PI);
                }
            }
        }

        m.RotateZ(((float)CGeneral::GetRandomNumber() - 128.0f) * (isSideways[i]
            ? (isGangRoadBlock ? 0.004f : 0.002f)
            : (isGangRoadBlock ? 0.006f : 0.003f)));

        m.SetTranslateOnly(from + along * dist);
        m.GetPosition().z += 0.3f - (onItsSide ? vehBB.m_vecMin.x : vehBB.m_vecMin.z);

        cursor = forwardSpan * 0.5f + vehGap + dist;

        if (!ClearSpaceForRoadBlockObject(m.TransformPoint(vehBB.m_vecMin), m.TransformPoint(vehBB.m_vecMax))) {
            continue;
        }

        if (isGangRoadBlock && (onItsSide || CGeneral::GetRandomNumber() < 64)) {
            // A burnt-out wreck rather than a working vehicle
            auto* const wreck = new CObject(ModelIndices::MI_ROADBLOCKFUCKEDCAR1, true);
            wreck->GetMatrix() = m;
            wreck->SetPosn(m.GetPosition());
            wreck->SetIsStatic(false);

            CObject::nNoTempObjects++;
            wreck->m_nObjectType  = OBJECT_TEMPORARY;
            wreck->m_nRemovalTime = CTimer::GetTimeInMS() + TEMP_OBJECT_LIFETIME;
            CWorld::Add(wreck);

            gFireManager.StartFire(wreck, nullptr, 2.8f, true, 60'000, 2);
            if (auto* const fire = wreck->m_pFire) {
                fire->SetRemovalDist(92);
            }
            continue;
        }

        auto* const veh = new CAutomobile(model, RANDOM_VEHICLE, true);
        veh->m_nStatus = STATUS_ABANDONED;

        m.GetPosition().z += veh->GetHeightAboveRoad() - 0.6f;
        veh->GetMatrix() = m;
        veh->PlaceOnRoadProperly();
        veh->SetIsStatic(false);
        veh->UpdateRW();

        veh->m_nDoorLock = CARLOCK_UNLOCKED;
        CCarCtrl::JoinCarWithRoadSystem(veh);

        veh->m_autoPilot.m_nCarMission  = MISSION_NONE;
        veh->m_autoPilot.m_nTempAction  = TEMPACT_NONE;
        veh->m_autoPilot.m_nCurrentLane = 0;
        veh->m_autoPilot.m_nNextLane    = 0;
        veh->m_autoPilot.m_speed        = 0.0f;
        veh->m_autoPilot.m_nCruiseSpeed = 0;

        veh->vehicleFlags.bNeverUseSmallerRemovalRange = true;
        veh->vehicleFlags.bIsLocked                    = false;
        veh->vehicleFlags.bEngineOn                    = false;

        auto onFire = false;
        if (isGangRoadBlock) {
            if ((int32)CGeneral::GetRandomNumberInRange(0.0f, 4.0f) != 0) { // 3 in 4 are already burning
                veh->BlowUpCarCutSceneNoExtras(true, true, true, true);
                veh->m_nTimeWhenBlowedUp += 1'000'000;
                onFire = true;
            } else {
                veh->SetTotalDamage(true);
            }
        } else if (veh->UsesSiren() && (CGeneral::GetRandomNumber() & 1)) {
            veh->vehicleFlags.bSirenOrAlarm = true;
        }

        if (veh->GetMatrix().GetUp().z <= 0.94f) { // Ended up on its roof, not worth keeping
            delete veh;
            continue;
        }

        CVisibilityPlugins::SetClumpAlpha(veh->m_pRwClump, 0);
        CWorld::Add(veh);

        veh->vehicleFlags.bCreateRoadBlockPeds = true;
        veh->m_nTimeTillWeNeedThisCar          = CTimer::GetTimeInMS() + 7'000;
        veh->m_nNumCopsForRoadBlock            = numVehicles <= 3 ? 2 : 1;

        if (isSideways[i]) {
            // Sideways cars get their doors flung open, and the peds stand behind them
            veh->m_nPedsPositionForRoadBlock = 2;

            const auto OpenDoorIfPresent = [veh](eDoors door) {
                const auto node = CDamageManager::GetCarNodeIndexFromDoor(door);
                if (veh->m_aCarNodes[node]) {
                    veh->OpenDoor(nullptr, node, door, 1.0f, true);
                }
            };
            OpenDoorIfPresent(DOOR_LEFT_FRONT);
            if (veh->m_nNumCopsForRoadBlock > 1) {
                OpenDoorIfPresent(DOOR_RIGHT_FRONT);
            }
        } else {
            // Peds go on whichever side of the car the player is approaching from
            veh->m_nPedsPositionForRoadBlock = DotProduct(veh->GetPosition() - FindPlayerCoors(), veh->GetMatrix().GetRight()) >= 0.0f;
        }

        if (onFire) {
            gFireManager.StartFire(veh, nullptr, 2.8f, true, 60'000, 2);
            if (auto* const fire = veh->m_pFire) {
                fire->SetRemovalDist(92);
            }
        }
    }

    if (isGangRoadBlock) { // Riots don't get tidy roadwork barriers
        return;
    }

    const auto barrierBB   = CModelInfo::GetModelInfo(ModelIndices::MI_ROADWORKBARRIER1)->GetColModel()->m_boundBox;
    const auto barrierSpan = barrierBB.m_vecMax.x - barrierBB.m_vecMin.x + 0.5f;

    const auto numBarriers = std::min((int32)(length / barrierSpan), MAX_ROADBLOCK_BARRIERS);
    const auto barrierGap  = (length - (float)numBarriers * barrierSpan) / (float)(numBarriers + 1);

    CObject::DeleteAllTempObjectsInArea(center, length * 0.5f);

    for (auto i = 0; i < numBarriers; i++) {
        const auto dist = ((float)i + 0.5f) * barrierSpan + (float)(i + 1) * barrierGap;

        CMatrix m{};
        m.SetUnity();
        m.SetTranslate(CVector{});

        // Barriers stand across the block, i.e. turned 90 degrees from the vehicles
        m.GetRight()   = along;
        m.GetForward() = CVector{ along.y, -along.x, 0.0f };
        m.GetUp()      = CVector{ 0.0f, 0.0f, 1.0f };
        m.RotateZ(((float)CGeneral::GetRandomNumber() - 128.0f) * 0.003f);

        // Set back from the vehicles, on the near side
        auto pos = from + along * dist + across * 5.0f;
        pos.x += (float)(CGeneral::GetRandomNumber() & 0xF) * 0.05f;
        pos.y += (float)(CGeneral::GetRandomNumber() & 0xF) * 0.05f;

        bool foundGround{};
        pos.z = CWorld::FindGroundZFor3DCoord({ pos.x, pos.y, pos.z + 2.0f }, &foundGround, nullptr);
        if (!foundGround) {
            continue;
        }
        pos.z -= barrierBB.m_vecMin.z;
        m.SetTranslateOnly(pos);

        if (!ClearSpaceForRoadBlockObject(m.TransformPoint(barrierBB.m_vecMin), m.TransformPoint(barrierBB.m_vecMax))) {
            continue;
        }

        auto* const barrier = new CObject(ModelIndices::MI_ROADWORKBARRIER1, true);
        barrier->GetMatrix() = m;
        barrier->SetPosn(m.GetPosition());

        CObject::nNoTempObjects++;
        barrier->m_nObjectType  = OBJECT_TEMPORARY;
        barrier->m_nRemovalTime = CTimer::GetTimeInMS() + TEMP_OBJECT_LIFETIME;
        CWorld::Add(barrier);
    }
}

// 0x461170
void CRoadBlocks::GenerateRoadBlockCopsForCar(CVehicle* vehicle, int32 pedsPositionsType, ePedType pedType) {
    // Ped positions relative to the vehicle. The first 6 are used for ordinary
    // cop cars, the last 6 for vehicles that carry a special ped model.
    static constexpr std::array<CVector, 12> s_PedOffsets{ {
        { -1.5f,  1.9f, 0.0f },
        { -1.5f, -2.6f, 0.0f },
        {  1.5f,  1.9f, 0.0f },
        {  1.5f, -2.6f, 0.0f },
        { -1.5f,  0.0f, 0.0f },
        {  1.5f,  0.0f, 0.0f },

        {  0.0f,  3.2f, 0.0f },
        {  1.5f, -1.8f, 0.0f },
        {  0.0f,  3.2f, 0.0f },
        { -1.5f, -1.8f, 0.0f },
        { -1.5f,  0.0f, 0.0f },
        {  1.5f,  0.0f, 0.0f },
    } };

    auto copType          = COP_TYPE_CITYCOP;
    auto pedModel         = (eModelID)0;
    auto isSpecialVehicle = false;

    if (pedType == PED_TYPE_COP) {
        switch (vehicle->m_nModelIndex) {
        case MODEL_ENFORCER:
            copType = COP_TYPE_SWAT1, pedModel = MODEL_SWAT, isSpecialVehicle = true;
            break;
        case MODEL_BARRACKS:
            copType = COP_TYPE_ARMY, pedModel = MODEL_ARMY, isSpecialVehicle = true;
            break;
        case MODEL_FBIRANCH:
            copType = COP_TYPE_FBI, pedModel = MODEL_FBI, isSpecialVehicle = true;
            break;
        case MODEL_COPCARRU:
            isSpecialVehicle = true;
            break;
        default:
            isSpecialVehicle = false;
            break;
        }
    } else if (pedType >= PED_TYPE_GANG1 && pedType <= PED_TYPE_GANG10) {
        for (auto&& [i, strength] : rngv::enumerate(CPopCycle::m_pCurrZoneInfo->GangStrength)) {
            if (!strength) {
                continue;
            }
            pedModel = CGangs::ChooseGangPedModel((eGangID)i);
            if (pedModel != MODEL_INVALID) {
                break;
            }
        }
        if (pedModel == MODEL_INVALID) { // No gang present in this zone could give us a model
            return;
        }
    }

    // Bigger vehicles push their peds further out
    const auto scale = vehicle->GetColModel()->m_boundSphere.m_fRadius
                     / CModelInfo::GetModelInfo(CStreaming::GetDefaultCopCarModel(false))->GetColModel()->m_boundSphere.m_fRadius;

    for (auto i = 0; i < (int32)vehicle->m_nNumCopsForRoadBlock; i++) {
        const auto offsIdx = 2 * pedsPositionsType + i;
        auto       pos     = vehicle->GetMatrix().TransformPoint(s_PedOffsets[isSpecialVehicle ? offsIdx + 6 : offsIdx] * scale);

        CPed* ped;
        if (pedType == PED_TYPE_COP) {
            if (copType != COP_TYPE_CITYCOP && !CStreaming::IsModelLoaded(pedModel)) {
                ped = new CCopPed(COP_TYPE_CITYCOP); // Special ped model isn't streamed in, settle for a beat cop
                ped->SetCurrentWeapon(WEAPON_PISTOL);
            } else {
                ped = new CCopPed(copType);
                if (copType == COP_TYPE_CITYCOP) {
                    ped->SetCurrentWeapon(WEAPON_PISTOL);
                }
            }
        } else {
            ped = new CCivilianPed(pedType, pedModel);
            ped->SetCurrentWeapon(WEAPON_PISTOL);
        }

        pos = CPedPlacement::FindZCoorForPed(pos).first;
        ped->SetPosn(pos);
        ped->SetOrientation(0.0f, 0.0f, -HALF_PI);

        if (pedType == PED_TYPE_COP) {
            ped->AsCop()->m_bDontPursuit = true;
            ped->GetTaskManager().SetTask(
                new CTaskComplexWanderCop(PEDMOVE_STILL, (uint8)CGeneral::GetRandomNumberInRange(0.0f, 8.0f)),
                TASK_PRIMARY_PRIMARY
            );
        }
        ped->GetTaskManager().SetTask(new CTaskSimpleStandStill(0, true, false, 8.0f), TASK_PRIMARY_DEFAULT);

        ped->bStayInSamePlace        = true;
        ped->bNotAllowedToDuck       = true;
        ped->m_nTimeTillWeNeedThisPed = CTimer::GetTimeInMS() + 10'000;

        // The peds standing in front of the special vehicles have nothing to duck behind
        ped->bCrouchWhenShooting = !(isSpecialVehicle && pedsPositionsType == 2);

        ped->m_pVehicle = vehicle;
        CEntity::RegisterReference(ped->m_pVehicle);
        ped->bCullExtraFarAway = true;
        CVisibilityPlugins::SetClumpAlpha(ped->m_pRwClump, 0);

        if (pedType != PED_TYPE_COP) {
            // Weighted pick between the gang's weapons: roughly even across however many it has
            const auto& weapons = CGangs::Gang[pedType - PED_TYPE_GANG1].m_nGangWeapons;
            const auto  roll    = (int32)CGeneral::GetRandomNumberInRange(0.0f, 100.0f);

            auto weapon = weapons[2];
            if (weapon) {
                if (roll < 33) {
                    weapon = weapons[0];
                } else if (roll < 66) {
                    weapon = weapons[1];
                }
            } else if (weapons[1] && roll >= 50) {
                weapon = weapons[1];
            } else {
                weapon = weapons[0];
            }

            if (weapon) {
                ped->GiveDelayedWeapon(weapon, 25'001);
                ped->SetCurrentWeapon(CWeaponInfo::GetWeaponInfo(weapon, eWeaponSkill::STD)->m_nSlot);
            }
        }

        CWorld::Add(ped);

        ped->GetIntelligence()->m_eventGroup.Add(
            CEventScriptCommand{ TASK_PRIMARY_PRIMARY, new CTaskComplexKillPedOnFoot(FindPlayerPed(), -1, 0, 0, 0, 1) }
        );
    }
}

// 0x4629E0
void CRoadBlocks::GenerateRoadBlocks() {
    ZoneScoped;

    //! Only nodes this close to the player are worth blocking off
    constexpr auto MAX_DIST_FROM_PLAYER = 90.0f;

    //! How far apart two nodes may be for one roadblock to span both
    constexpr auto MAX_NODE_PAIR_DIST = 30.0f;

    //! The node list is walked in this many slices, one per frame
    constexpr auto NUM_SLICES = 16;

    if (!FindPlayerWanted()->m_nChanceOnRoadBlock || !FindPlayerVehicle()) {
        bGenerateDynamicRoadBlocks = false;
    } else {
        if (!bGenerateDynamicRoadBlocks) {
            rng::fill(InOrOut, true);
        }
        bGenerateDynamicRoadBlocks = true;

        const auto slice = CTimer::GetFrameCounter() % NUM_SLICES;
        const auto begin = (int32)(NUM_ROADBLOX_NODES * slice / NUM_SLICES);
        const auto end   = (int32)(NUM_ROADBLOX_NODES * (slice + 1) / NUM_SLICES);

        for (auto i = begin; i < std::min(NumRoadBlocks, end); i++) {
            const auto nodeAddr = RoadBlockNodes[i];
            if (!ThePaths.IsAreaLoaded(nodeAddr.m_wAreaId)) {
                continue;
            }
            const auto& node    = ThePaths.m_pPathNodes[nodeAddr.m_wAreaId][nodeAddr.m_wNodeId];
            const auto  nodePos = node.GetPosition();

            const auto toPlayer = FindPlayerCoors() - nodePos;
            if (toPlayer.x <= -MAX_DIST_FROM_PLAYER || toPlayer.x >= MAX_DIST_FROM_PLAYER
             || toPlayer.y <= -MAX_DIST_FROM_PLAYER || toPlayer.y >= MAX_DIST_FROM_PLAYER
             || CVector2D{ toPlayer }.Magnitude() >= MAX_DIST_FROM_PLAYER
            ) {
                InOrOut[i] = false; // Out of range, so it becomes available again once the player returns
                continue;
            }

            if (InOrOut[i]) { // Already dealt with while the player was in range
                continue;
            }
            InOrOut[i] = true;

            if ((CGeneral::GetRandomNumber() & 0x7F) >= FindPlayerWanted()->m_nChanceOnRoadBlock) {
                continue;
            }

            float   width;
            CVector across;
            if (!GetRoadBlockNodeInfo(nodeAddr, width, across)) {
                continue;
            }

            if (node.m_nPathWidth) {
                // The node knows how wide its road is: block both halves, leaving the
                // centre line free so the two halves don't overlap.
                const auto inner = (float)node.m_nPathWidth / 16.0f;
                const auto outer = width * 0.5f + inner;

                CreateRoadBlockBetween2Points(nodePos + across * outer, nodePos + across * inner, false);
                CreateRoadBlockBetween2Points(nodePos - across * inner, nodePos - across * outer, false);
                continue;
            }

            // No width recorded, so look for a nearby node to span the road between
            auto paired = false;
            for (auto j = 0; j < NumRoadBlocks; j++) {
                if (j == i || InOrOut[j]) {
                    continue;
                }

                const auto otherAddr = RoadBlockNodes[j];
                if (!ThePaths.IsAreaLoaded(otherAddr.m_wAreaId)) {
                    continue;
                }
                const auto& other    = ThePaths.m_pPathNodes[otherAddr.m_wAreaId][otherAddr.m_wNodeId];
                const auto  otherPos = other.GetPosition();

                const auto delta = nodePos - otherPos;
                if (delta.x <= -MAX_NODE_PAIR_DIST || delta.x >= MAX_NODE_PAIR_DIST
                 || delta.y <= -MAX_NODE_PAIR_DIST || delta.y >= MAX_NODE_PAIR_DIST
                ) {
                    continue;
                }

                float   otherWidth;
                CVector otherAcross;
                if (!GetRoadBlockNodeInfo(otherAddr, otherWidth, otherAcross)) {
                    continue;
                }
                if (width != otherWidth) { // Both ends have to be the same kind of road
                    continue;
                }
                if (std::fabs(DotProduct(across, otherAcross)) <= 0.7f) { // The two roads must run roughly parallel
                    continue;
                }

                // Nothing may stand between the two nodes
                CColPoint cp{};
                CEntity*  hitEntity{};
                if (CWorld::ProcessLineOfSight(
                        nodePos + CVector{ 0.0f, 0.0f, 1.0f },
                        otherPos + CVector{ 0.0f, 0.0f, 1.0f },
                        cp, hitEntity, true, false, false, false, false, false, false, false
                )) {
                    continue;
                }

                auto along = otherPos - nodePos;
                along.Normalise();

                const auto half = width * 0.5f;
                CreateRoadBlockBetween2Points(otherPos + along * half, nodePos - along * half, false);

                InOrOut[j] = true;
                paired     = true;
                break;
            }

            if (!paired) { // Settle for a block centred on this node alone
                const auto half = width * 0.5f;
                CreateRoadBlockBetween2Points(nodePos + across * half, nodePos - across * half, false);
            }
        }
    }

    // Script-requested roadblocks are serviced one slot per frame, and only ever
    // built while the player is too far away to see the props appear.
    auto& rb = aScriptRoadBlocks[CTimer::GetFrameCounter() % NUM_SCRIPT_ROADBLOCKS];
    if (!rb.bActive) {
        return;
    }

    const auto center = (rb.Coors1 + rb.Coors2) * 0.5f;
    if ((center - FindPlayerCoors()).Magnitude() >= MAX_DIST_FROM_PLAYER) {
        rb.bSafeToCreate = true;
    } else if (rb.bSafeToCreate) {
        CreateRoadBlockBetween2Points(rb.Coors1, rb.Coors2, rb.bGangRoadBlock);
        rb.bSafeToCreate = false;
    }
}

// 0x460EE0
bool CRoadBlocks::GetRoadBlockNodeInfo(CNodeAddress nodeAddr, float& outWidth, CVector& outDir) {
    // The roadblock sits across the segment spanned by the node's first two car path links
    const auto* const links = &ThePaths.m_pNaviLinks[nodeAddr.m_wAreaId][ThePaths.m_pPathNodes[nodeAddr.m_wAreaId][nodeAddr.m_wNodeId].m_wBaseLinkId];

    const auto fromAddr = links[0];
    const auto toAddr   = links[1];
    if (!ThePaths.IsAreaLoaded(fromAddr.m_wAreaId) || !ThePaths.IsAreaLoaded(toAddr.m_wAreaId)) {
        return false;
    }

    const auto& from = ThePaths.GetCarPathLink(fromAddr);
    const auto& to   = ThePaths.GetCarPathLink(toAddr);

    const auto NumLanes = [](const CCarPathLink& link) {
        return link.m_numOppositeDirLanes + link.m_numSameDirLanes;
    };
    outWidth = (float)(std::max(NumLanes(from), NumLanes(to)) + 1) * 5.0f;

    // Perpendicular of the segment, so the block is laid across the road rather than along it
    const auto delta = to.GetNodeCoors() - from.GetNodeCoors();
    outDir = CVector{ delta.y, -delta.x, 0.0f };
    outDir.Normalise();

    return true;
}

// 0x460DF0
void CRoadBlocks::RegisterScriptRoadBlock(CVector cornerA, CVector cornerB, bool isGangRoadBlock) {
    const auto it = rng::find_if(aScriptRoadBlocks, [](const CScriptRoadBlock& rb) { return !rb.bActive; });
    if (it == aScriptRoadBlocks.end()) { // All roadblock slots are taken
        return;
    }

    it->bActive        = true;
    it->bSafeToCreate  = true;
    it->Coors1         = cornerA;
    it->Coors2         = cornerB;
    it->bGangRoadBlock = isGangRoadBlock;
}
