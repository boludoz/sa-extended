#include "StdInc.h"

#include "RoadBlocks.h"
#include "PedPlacement.h"
#include "TaskComplexWanderCop.h"
#include "TaskSimpleStandStill.h"
#include "VisibilityPlugins.h"
#include "Population.h"
#include "CarCtrl.h"
#include "DamageManager.h"
#include "FireManager.h"
#include "ModelIndices.h"
#include <extensions/File.hpp>

void CRoadBlocks::InjectHooks() {
    RH_ScopedClass(CRoadBlocks);
    RH_ScopedCategoryGlobal();

    RH_ScopedInstall(Init, 0x461100);
    RH_ScopedInstall(ClearScriptRoadBlocks, 0x460EC0);
    RH_ScopedInstall(ClearSpaceForRoadBlockObject, 0x461020);
    RH_ScopedInstall(CreateRoadBlockBetween2Points, 0x4619C0);
    RH_ScopedInstall(GenerateRoadBlockPedsForCar, 0x461170);
    RH_ScopedInstall(GenerateRoadBlocks, 0x4629E0);
    RH_ScopedInstall(GetRoadBlockNodeInfo, 0x460EE0);
    RH_ScopedInstall(RegisterScriptRoadBlock, 0x460DF0);
}

// 0x461100
void CRoadBlocks::Init() {
    rng::fill(InOrOut, true);
    GenerateDynamicRoadBlocks = false;

    if (notsa::File rbx("data\\paths\\roadblox.dat", "rb"); rbx) {
        rbx.Read(&NumRoadBlocks, sizeof(int32));
        assert(NumRoadBlocks <= MAX_ROADBLOCKS);
        rbx.Read(RoadBlockNodes.data(), RoadBlockNodes.size() * sizeof(CNodeAddress));
    } else {
        NOTSA_UNREACHABLE("roadblox.dat couldn't be opened!");
    }
    ClearScriptRoadBlocks();
}

// 0x460EC0
void CRoadBlocks::ClearScriptRoadBlocks() {
    for (auto& srb : aScriptRoadBlocks) {
        srb.IsActive = false;
    }
}

// 0x461020
// Returns true if cleared successfully.
bool CRoadBlocks::ClearSpaceForRoadBlockObject(CVector cornerA, CVector cornerB){
    int16 numEntities{};
    CEntity* entities[2]{};
    CWorld::FindObjectsIntersectingCube(
        cornerA,
        cornerB,
        &numEntities,
        std::size(entities),
        entities,
        false,
        true,
        true,
        true,
        false
    );

    if (numEntities > std::size(entities) || numEntities <= 0) {
        return numEntities <= 0;
    }

    const auto Remove = [](CEntity* e) {
        CWorld::Remove(e);
        delete e;
    };

    for (auto* entity : entities | rngv::take(numEntities)) {
        switch (entity->GetType()) {
        case ENTITY_TYPE_VEHICLE:
            if (auto* v = entity->AsVehicle(); !v->CanBeDeleted()) {
                return false;
            } else if (!v->vehicleFlags.bCreateRoadBlockPeds) {
                Remove(v);
            }
            break;
        case ENTITY_TYPE_PED:
            if (auto* p = entity->AsPed(); p->CanBeDeleted()) {
                Remove(p);
            } else {
                return false;
            }
            break;
        case ENTITY_TYPE_OBJECT:
            if (auto* o = entity->AsObject(); o->CanBeDeleted() && o->m_nObjectType != OBJECT_GAME) {
                Remove(o);
            } else {
                return false;
            }
            break;
        default:
            NOTSA_UNREACHABLE();
        }
    }

    return true;
}

// 0x4619C0
void CRoadBlocks::CreateRoadBlockBetween2Points(CVector a, CVector b, bool isGangRoadBlock) {
    //! How many vehicles a single roadblock may consist of
    constexpr auto MAX_ROADBLOCK_VEHICLES = 5;

    //! How many barriers get lined up alongside it
    constexpr auto MAX_ROADBLOCK_BARRIERS = 8;

    //! Temporary props stay around for 10 minutes
    constexpr auto TEMP_OBJECT_LIFETIME = 600'000u;

    const auto length = (b - a).Magnitude();
    const auto center = (a + b) * 0.5f;

    auto along = b - a;
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

        m.SetTranslateOnly(a + along * dist);
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
        veh->SetStatus(STATUS_ABANDONED);

        m.GetPosition().z += veh->GetHeightAboveRoad() - 0.6f;
        veh->GetMatrix() = m;
        veh->PlaceOnRoadProperly();
        veh->SetIsStatic(false);
        veh->UpdateRwMatrix();
        veh->UpdateRwFrame();

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

        CVisibilityPlugins::SetClumpAlpha(veh->GetRpClump(), 0);
        CWorld::Add(veh);

        veh->vehicleFlags.bCreateRoadBlockPeds = true;
        veh->m_nTimeTillWeNeedThisCar          = CTimer::GetTimeInMS() + 7'000;
        veh->m_nNumPedsForRoadBlock            = numVehicles <= 3 ? 2 : 1;

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
            if (veh->m_nNumPedsForRoadBlock > 1) {
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
        auto pos = a + along * dist + across * 5.0f;
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
void CRoadBlocks::GenerateRoadBlockPedsForCar(CVehicle* vehicle, int32 pedsPositionsType, ePedType pedType) {
    const auto Generate = [&](eModelID pedModel = MODEL_INVALID, eCopType copType = COP_TYPE_CITYCOP, bool isSpecialCop = false) {
        static constexpr auto PLACEMENTS = std::to_array<CVector>({
            { -1.5f, +1.9f, 0.0f },
            { -1.5f, -2.6f, 0.0f },
            { +1.5f, +1.9f, 0.0f },
            { +1.5f, -2.6f, 0.0f },
            { -1.5f,  0.0f, 0.0f },
            { +1.5f,  0.0f, 0.0f },
        });

        static constexpr auto SPECIAL_COP_PLACEMENTS = std::to_array<CVector>({
            {  0.0f, +3.2f, 0.0f },
            { +1.5f, -1.8f, 0.0f },
            {  0.0f, +3.2f, 0.0f },
            { -1.5f, -1.8f, 0.0f },
            { -1.5f,  0.0f, 0.0f },
            { +1.5f,  0.0f, 0.0f },
        });

        const auto placementIdx = 2 * pedsPositionsType;
        const auto radiusRatio  = vehicle->GetColModel()->GetBoundingSphere().m_fRadius
            / CModelInfo::GetModelInfo(CStreaming::GetDefaultCopCarModel(false))->GetColModel()->GetBoundingSphere().m_fRadius;

        for (auto i = 0u; i < vehicle->m_nNumPedsForRoadBlock; i++) {
            const auto offset = (isSpecialCop ? SPECIAL_COP_PLACEMENTS : PLACEMENTS)[placementIdx + i] * radiusRatio;
            const auto pos = vehicle->GetMatrix().TransformPoint(offset);

            auto* ped = [&]() -> CPed* {
                if (pedType != PED_TYPE_COP) { // 0x461560
                    return new CCivilianPed(pedType, pedModel);
                } else {
                    auto* p = new CCopPed(CStreaming::IsModelLoaded(pedModel) ? copType : COP_TYPE_CITYCOP);
                    if (copType == COP_TYPE_CITYCOP) {
                        p->SetCurrentWeapon(WEAPON_PISTOL);
                    }
                    return p;
                }
            }();

            ped->SetPosn(std::get<CVector>(CPedPlacement::FindZCoorForPed(pos)));
            ped->GetMatrix().SetRotateKeepPos({ 0.0f, 0.0f, -HALF_PI });

            if (pedType == PED_TYPE_COP) {
                auto* t = new CTaskComplexWanderCop(PEDMOVE_STILL, CGeneral::GetRandomNumberInRange(8ui8));
                t->m_nSubTaskCreatedTimer = {};
                t->m_nScanForStuffTimer   = {};
                ped->GetTaskManager().SetTask(t, TASK_PRIMARY_PRIMARY);
            }
            ped->GetTaskManager().SetTask(new CTaskSimpleStandStill(0, true), TASK_PRIMARY_DEFAULT);

            ped->bStayInSamePlace         = true;
            ped->bNotAllowedToDuck        = true;
            ped->m_nTimeTillWeNeedThisPed = CTimer::GetTimeInMS() + 10'000;
            ped->bCrouchWhenShooting      = !isSpecialCop || pedsPositionsType != 2;
            ped->bCullExtraFarAway        = true;
            CEntity::RegisterReference(ped->m_pVehicle = vehicle);
            CVisibilityPlugins::SetClumpAlpha(ped->GetRpClump(), 0);

            if (pedType != PED_TYPE_COP) {
                const auto weapon = CGangs::Gang[pedType - PED_TYPE_GANG1].GetRandomWeapon(false);
                if (weapon != WEAPON_UNARMED) {
                    ped->GiveDelayedWeapon(weapon, 25'001);
                    ped->SetCurrentWeapon(weapon);
                }
            }
            CWorld::Add(ped);
            ped->GetEventGroup().Add<CEventScriptCommand>({ TASK_PRIMARY_PRIMARY, new CTaskComplexKillPedOnFoot(FindPlayerPed()) });
        }
    };

    if (pedType == PED_TYPE_COP) {
        switch (vehicle->GetModelId()) {
        case MODEL_ENFORCER: Generate(MODEL_SWAT,    COP_TYPE_SWAT1,   true); break;
        case MODEL_BARRACKS: Generate(MODEL_ARMY,    COP_TYPE_ARMY,    true); break;
        case MODEL_FBIRANCH: Generate(MODEL_FBI,     COP_TYPE_FBI,     true); break;
        case MODEL_COPCARRU: Generate(MODEL_INVALID, COP_TYPE_CITYCOP, true); break;
        default:             Generate(MODEL_INVALID, COP_TYPE_CITYCOP, false); break;
        }
    } else if (IsPedTypeGang(pedType)) {
        for (auto i = 0; i < TOTAL_GANGS; i++) {
            if (!CPopCycle::m_pCurrZoneInfo->GangStrength[i]) {
                continue;
            }
            const auto pedModel = CGangs::ChooseGangPedModel((eGangID)i);
            if (pedModel == MODEL_INVALID) {
                continue;
            }
            Generate(pedModel);
            return;
        }
    } else {
        Generate();
    }
}

// 0x4629E0
void CRoadBlocks::GenerateRoadBlocks() {
    ZoneScoped;

    if (FindPlayerWanted()->m_ChanceOnRoadBlock && FindPlayerVehicle()) {
        if (!GenerateDynamicRoadBlocks) {
            rng::fill(InOrOut, true);
            GenerateDynamicRoadBlocks = true;
        }

        const auto counter1      = MAX_ROADBLOCKS * (CTimer::GetFrameCounter() % 16 + 1);
        const auto rbsToGenerate = std::min((uint32)NumRoadBlocks, ((counter1 % 16) + counter1) / 16);
        auto       counter2      = MAX_ROADBLOCKS * (CTimer::GetFrameCounter() % 16) / 16;

        for (; counter2 < rbsToGenerate; counter2++) {
            const auto& mrbNode = RoadBlockNodes[counter2];
            if (!ThePaths.IsAreaLoaded(mrbNode)) {
                continue;
            }
            const auto& mainNode = ThePaths.GetPathNode(mrbNode);
            const auto  playerPos = FindPlayerCoors();
            if (std::abs(playerPos.x - mainNode->GetPosition().x) >= 90.0f ||
                std::abs(playerPos.y - mainNode->GetPosition().y) >= 90.0f ||
                DistanceBetweenPoints2D(playerPos, mainNode->GetPosition()) >= 90.0f)
            {
                InOrOut[counter2] = false;
                continue;
            }

            if (InOrOut[counter2]) {
                continue;
            }
            InOrOut[counter2] = true;

            if (CGeneral::GetRandomNumberInRange(128u) >= FindPlayerWanted()->m_ChanceOnRoadBlock) {
                continue;
            }

            float mrbWidth{};
            CVector mrbDir{};
            if (!GetRoadBlockNodeInfo(mrbNode, mrbWidth, mrbDir)) {
                continue;
            }

            if (mainNode->m_nPathWidth) {
                const auto width = mainNode->m_nPathWidth / 16.0f;
                CreateRoadBlockBetween2Points(
                    mainNode->GetPosition() + mrbDir * (mrbWidth / 2.f + width),
                    mainNode->GetPosition() + mrbDir * width,
                    false
                );
                CreateRoadBlockBetween2Points(
                    mainNode->GetPosition() - mrbDir * width,
                    mainNode->GetPosition() - mrbDir * (mrbWidth / 2.f + width),
                    false
                );
                continue;
            }

            for (auto&& [i, nodeAddr] : rngv::enumerate(RoadBlockNodes)) {
                if (counter2 == i || InOrOut[i] || !ThePaths.IsAreaLoaded(nodeAddr.m_wAreaId)) {
                    continue;
                }
                const auto& node = ThePaths.GetPathNode(nodeAddr);

                if (std::abs(mainNode->GetPosition().x - node->GetPosition().x) >= 30.0f ||
                    std::abs(mainNode->GetPosition().y - node->GetPosition().y) >= 30.0f)
                {
                    continue;
                }

                float   width{};
                CVector dir{}; 
                if (!GetRoadBlockNodeInfo(nodeAddr, width, dir)) {
                    continue;
                }

                if (mrbWidth != width || dir.Dot(mrbDir) <= 0.7f) {
                    continue;
                }

                [[maybe_unused]] CColPoint col{};
                [[maybe_unused]] CEntity*  colEntity{};
                if (CWorld::ProcessLineOfSight(
                    mainNode->GetPosition() + CVector{0.0f, 0.0f, 1.0f},
                    node->GetPosition() + CVector{0.0f, 0.0f, 1.0f},
                    col,
                    colEntity,
                    true,
                    false,
                    false,
                    false,
                    false,
                    false,
                    false,
                    false
                )) {
                    continue;
                }

                const auto dirFromMain = (node->GetPosition() - mainNode->GetPosition()).Normalized();
                CreateRoadBlockBetween2Points(
                    node->GetPosition()     + dirFromMain * (mrbWidth / 2.0f),
                    mainNode->GetPosition() - dirFromMain * (mrbWidth / 2.0f),
                    false
                );
                InOrOut[i] = true;

                if (i == NumRoadBlocks) {
                    CreateRoadBlockBetween2Points(
                        mainNode->GetPosition() - dirFromMain * (mrbWidth / 2.0f),
                        mainNode->GetPosition() + dirFromMain * (mrbWidth / 2.0f),
                        false
                    );
                    break;
                }
            }
        }
    } else {
        GenerateDynamicRoadBlocks = false;
    }

    if (auto& srb = aScriptRoadBlocks[CTimer::GetFrameCounter() % MAX_SCRIPT_ROADBLOCKS]; srb.IsActive) {
        const auto c = CVector::Centroid({ srb.CornerA, srb.CornerB });

        if (DistanceBetweenPoints(FindPlayerCoors(), c) >= 90.0f) {
            srb.IsSafeToCreate = true;
        } else if (srb.IsSafeToCreate) {
            CreateRoadBlockBetween2Points(srb.CornerA, srb.CornerB, srb.IsGangRoadBlock);
            srb.IsSafeToCreate = false;
        }
    }
}

// 0x460EE0
bool CRoadBlocks::GetRoadBlockNodeInfo(CNodeAddress nodeAddress, float& outWidth, CVector& outDir) {
    auto* const node = ThePaths.GetPathNode(nodeAddress);
    assert(node);

    assert(node->m_nNumLinks >= 2);
    const auto naviLinkAddrA = ThePaths.GetNaviLink(nodeAddress.m_wAreaId, node->m_wBaseLinkId + 0),
               naviLinkAddrB = ThePaths.GetNaviLink(nodeAddress.m_wAreaId, node->m_wBaseLinkId + 1);
    if (!ThePaths.IsAreaLoaded(naviLinkAddrA.m_wAreaId) || !ThePaths.IsAreaLoaded(naviLinkAddrB.m_wAreaId)) {
        return false;
    }

    const auto &naviLinkA = ThePaths.GetCarPathLink(naviLinkAddrA),
               &naviLinkB = ThePaths.GetCarPathLink(naviLinkAddrB);

    const auto maxNumLanes = std::max(
        naviLinkA.m_numOppositeDirLanes + naviLinkA.m_numSameDirLanes,
        naviLinkB.m_numOppositeDirLanes + naviLinkB.m_numSameDirLanes
    );

    outWidth = ((float)maxNumLanes + 1.f) * 5.f;
    outDir   = CVector{ (naviLinkB.GetNodeCoors() - naviLinkA.GetNodeCoors()).GetPerpRight(), 0.f }.Normalized();

    return true;
}

// 0x460DF0
void CRoadBlocks::RegisterScriptRoadBlock(CVector cornerA, CVector cornerB, bool isGangRoadBlock) {
    auto free = rng::find_if(aScriptRoadBlocks, [](const auto& srb) { return !srb.IsActive; });
    if (free == aScriptRoadBlocks.end()) {
        // No free script roadblock found
        return;
    }

    free->CornerA         = cornerA;
    free->CornerB         = cornerB;
    free->IsActive        = true;
    free->IsSafeToCreate  = true;
    free->IsGangRoadBlock = isGangRoadBlock;
}
