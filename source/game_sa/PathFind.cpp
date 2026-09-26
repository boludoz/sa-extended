/*
    Plugin-SDK file
    Authors: GTA Community. See more here
    https://github.com/DK22Pac/plugin-sdk
    Do not delete this comment block. Respect others' work!
*/
#include "StdInc.h"
#include "PathFind.h"

#include <reversiblebugfixes/Bugs.hpp>

// TODO: Move into the class itself
auto& s_pathsNeededPosn = StaticRef<CVector>(0x977B70);
auto& s_bLoadPathsNeeded = StaticRef<bool>(0x96F030);

// TODO: Remove this and use a stack array or smth..
auto& ToBeStreamed = StaticRef<std::array<bool, NUM_PATH_MAP_AREAS>>(0x96EFD0);

auto& XCoorGiven = StaticRef<std::array<float, 64>>(0x96EE80);
auto& YCoorGiven = StaticRef<std::array<float, 64>>(0x96ED80);
auto& ZCoorGiven = StaticRef<std::array<float, 64>>(0x96EC80);
auto& ConnectsToGiven = StaticRef<std::array<std::array<int8, 6>, 64>>(0x96EAC0); // 6 links per node (stride 6 in 0x44DED0/0x44DF60)
auto& DontWanderGiven = StaticRef<std::array<bool, 64>>(0x96EC40);

auto& aInteriorNodeLinkedToExterior = StaticRef<std::array<int32, NUM_PATH_INTERIOR_AREAS>>(0x96EA98);
auto& aExteriorNodeLinkedTo = StaticRef<std::array<CNodeAddress, NUM_PATH_INTERIOR_AREAS>>(0x977B7C);

auto& aNodesToBeCleared = StaticRef<std::array<CNodeAddress, 5000>>(0x972CD0);

void CPathFind::InjectHooks() {
    RH_ScopedClass(CPathFind);
    RH_ScopedCategoryGlobal();

    // Hooks commented out because most of the functions have no definitions
    // thus it's not possible to take their address
    // And I'm lazy to add stubs

    RH_ScopedInstall(AddNodeToNewInterior, 0x450E90);
    RH_ScopedInstall(FindNearestExteriorNodeToInteriorNode, 0x450F30);
    RH_ScopedInstall(ThisNodeHasToBeSwitchedOff, 0x44D3E0);
    RH_ScopedInstall(These2NodesAreAdjacent, 0x44D230);
    RH_ScopedInstall(FindRegionForCoors, 0x44D830);
    RH_ScopedInstall(FindYRegionForCoors, 0x44D8C0);
    RH_ScopedInstall(FindXRegionForCoors, 0x44D890);
    RH_ScopedInstall(AddInteriorLinkToExternalNode, 0x44DF30);
    RH_ScopedInstall(AddInteriorLink, 0x44DED0);
    RH_ScopedInstall(RemoveInteriorLinks, 0x44DF60, { .reversed = true, .enabled = false, .locked = false });
    RH_ScopedInstall(MarkRegionsForCoors, 0x44DB60);
    RH_ScopedOverloadedInstall(FindStartPointOfRegion, "", 0x44D930, void(CPathFind::*)(size_t, size_t, float&, float&));
    RH_ScopedInstall(FindYCoorsForRegion, 0x44D910);
    RH_ScopedInstall(FindXCoorsForRegion, 0x44D8F0);
    RH_ScopedInstall(IsAreaNodesAvailable, 0x420AA0);
    RH_ScopedInstall(HaveRequestedNodesBeenLoaded, 0x450DB0);
    RH_ScopedInstall(MakeRequestForNodesToBeLoaded, 0x450D70);
    RH_ScopedInstall(UpdateStreaming, 0x450A60);
    RH_ScopedInstall(TakeWidthIntoAccountForWandering, 0x4509A0);
    RH_ScopedInstall(Shutdown, 0x450950);
    RH_ScopedOverloadedInstall(FindNodeCoorsForScript, "TwoNodes", 0x450780, CVector(CPathFind::*)(CNodeAddress, CNodeAddress, float&, bool*));
    RH_ScopedOverloadedInstall(FindNodeCoorsForScript, "LinkedNode", 0x4505E0, CVector(CPathFind::*)(CNodeAddress, bool*));
    RH_ScopedInstall(IsWaterNodeNearby, 0x450DE0);
    RH_ScopedInstall(CountNeighboursToBeSwitchedOff, 0x4504F0);
    RH_ScopedInstall(FindNodeOrientationForCarPlacement, 0x450320);
    RH_ScopedInstall(FindNodePairClosestToCoors, 0x44FEE0); // ASM Match
    RH_ScopedInstall(FindNodeClosestToCoorsFavourDirection, 0x44FCE0);
    RH_ScopedInstall(FindNodeClosestToCoors, 0x44F460);
    RH_ScopedInstall(RecordNodesClosestToCoors, 0x44FA30);
    RH_ScopedInstall(MarkRoadNodeAsDontWander, 0x450560);
    RH_ScopedOverloadedInstall(LoadPathFindData, "Area", 0x452F40, void(CPathFind::*)(int32));
    RH_ScopedOverloadedInstall(LoadPathFindData, "FromStream", 0x4529F0, void(CPathFind::*)(RwStream*, int32));
    RH_ScopedInstall(SwitchPedRoadsOffInArea, 0x452F00);
    RH_ScopedInstall(SwitchRoadsOffInArea, 0x452C80);
    RH_ScopedInstall(SwitchRoadsOffInAreaForOneRegion, 0x452820);
    RH_ScopedInstall(ComputeRoute, 0x452760);
    RH_ScopedInstall(CompleteNewInterior, 0x452270); // ASM Match
    RH_ScopedInstall(SwitchOffNodeAndNeighbours, 0x452160);
    RH_ScopedInstall(Find2NodesForCarCreation, 0x452090);
    RH_ScopedInstall(TestCoorsCloseness, 0x452000);
    RH_ScopedInstall(FindNextNodeWandering, 0x451B70); // ASM Match
    RH_ScopedInstall(DoPathSearch, 0x4515D0);
    RH_ScopedInstall(FindParkingNodeInArea, 0x4513F0);
    RH_ScopedInstall(FindLinkBetweenNodes, 0x451350);
    RH_ScopedInstall(ReturnInteriorNodeIndex, 0x451300);
    RH_ScopedInstall(FindNthNodeClosestToCoors, 0x44F8C0); // ASM Match
    RH_ScopedInstall(FindNodeClosestInRegion, 0x44F2C0);
    RH_ScopedInstall(CalcDistToAnyConnectingLinks, 0x44F190); // ASM Match
    RH_ScopedInstall(CalcRoadDensity, 0x44EFC0); // ASM Match
    RH_ScopedInstall(TestForPedTrafficLight, 0x44D480);
    RH_ScopedInstall(UnMarkAllRoadNodesAsDontWander, 0x44D400);
    RH_ScopedInstall(TidyUpNodeSwitchesAfterMission, 0x44D3B0);
    RH_ScopedInstall(ThisNodeWillLeadIntoADeadEnd, 0x44D310);
    RH_ScopedInstall(AddNodeToList, 0x44D1E0);
    RH_ScopedInstall(RemoveNodeFromList, 0x44D1B0);
    RH_ScopedInstall(UnLoadPathFindData, 0x44D0F0);
    RH_ScopedInstall(Init, 0x44D080);
    RH_ScopedInstall(GetPathNode, 0x420AC0);
    RH_ScopedInstall(TestCrossesRoad, 0x44D790);
    RH_ScopedInstall(ReInit, 0x44E4E0);
    RH_ScopedInstall(RemoveInterior, 0x44E1A0);
    RH_ScopedInstall(AddDynamicLinkBetween2Nodes, 0x4512D0);
    RH_ScopedInstall(AddDynamicLinkBetween2Nodes_For1Node, 0x44E000);
    RH_ScopedInstall(StartNewInterior, 0x44DE80);
    RH_ScopedInstall(LoadSceneForPathNodes, 0x44DE00);
    RH_ScopedInstall(AreNodesLoadedForArea, 0x44DD10);
    RH_ScopedInstall(ReleaseRequestedNodes, 0x44DD00);
    RH_ScopedInstall(SetPathsNeededAtPosition, 0x44DCD0);
    RH_ScopedInstall(SetLinksBridgeLights, 0x44D960);
    RH_ScopedInstall(Save, 0x5D34C0);
    RH_ScopedInstall(Load, 0x5D3500);
}

void CPathNode::InjectHooks() {
    RH_ScopedClass(CPathNode);
    RH_ScopedCategoryGlobal();
    RH_ScopedInstall(GetPosition, 0x420A10);
}

// 0x44D080
void CPathFind::Init() {
    ZoneScoped;

    static int32 NumTempExternalNodes = 0; // Unused
    m_nNumNodeSwitches                = 0;
    m_loadAreaRequestPending = false;

    for (auto i = 0u; i < NUM_TOTAL_PATH_NODE_AREAS; ++i) {
        m_pPathNodes[i] = nullptr;
        m_pNaviNodes[i] = nullptr;
        pAdjacentNodes[i] = nullptr;
        m_pLinkLengths[i] = nullptr;
        m_pPathIntersections[i] = nullptr;
        pAdjacentLinks[i] = nullptr; // BUG: Out of array bounds write, same as in original code
        m_aTempNodes[i] = nullptr;    // BUG: Out of array bounds write, same as in original code
    }

    rng::fill(m_interiorIDs, (uint32)-1);
}

// 0x44E4E0
void CPathFind::ReInit() {
    m_nNumNodeSwitches       = 0;
    m_loadAreaRequestPending = false;
}

// 0x44DD00
void CPathFind::MakeRequestForNodesToBeLoaded(float minX, float maxX, float minY, float maxY) {
    m_loadAreaRequestPending = true;
    m_loadAreaRequestMinX = minX;
    m_loadAreaRequestMaxX = maxX;
    m_loadAreaRequestMinY = minY;
    m_loadAreaRequestMaxY = maxY;
    UpdateStreaming(true);
}

// 0x44DD10
bool CPathFind::AreNodesLoadedForArea(float minX, float maxX, float minY, float maxY) {
    return IterAreasTouchingRect({ minX, minY, maxX, maxY }, [this](auto areaId) -> bool { return IsAreaLoaded(areaId); });
}

// 0x450DB0
bool CPathFind::HaveRequestedNodesBeenLoaded() {
    return AreNodesLoadedForArea(
        m_loadAreaRequestMinX,
        m_loadAreaRequestMaxX,
        m_loadAreaRequestMinY,
        m_loadAreaRequestMaxY
    );
}

// 0x450950
void CPathFind::Shutdown() {
    for (auto x = 0u; x < NUM_PATH_MAP_AREA_X; ++x) {
        for (auto y = 0u; y < NUM_PATH_MAP_AREA_Y; ++y) {
            auto relativeId = x + y * NUM_PATH_MAP_AREA_X;
            if (m_pPathNodes[relativeId]) {
                CStreaming::RemoveModel(DATToModelId(relativeId));
            }
        }
    }
}

// 0x44D310
bool CPathFind::ThisNodeWillLeadIntoADeadEnd(CPathNode* startNode, CPathNode* endNode) {
    auto curr = startNode, prev = endNode;
    while (true) {
        CPathNode* next = nullptr;

        for (int32 neighbour = 0; neighbour < (int32)curr->NumberAdjNodes; ++neighbour) {
            const auto linkIndex = curr->IndexAdjacentNodes + neighbour;
            const CNodeAddress newNode = pAdjacentNodes[curr->Region][linkIndex];

            if (IsAreaNodesAvailable(newNode)) {
                CPathNode* pNewNode = GetPathNode(newNode);

                if (pNewNode && pNewNode != prev && (pNewNode->SpecialFunction == 0 || pNewNode->SpecialFunction > 10)) {
                    if (next) {
                        return false;
                    }
                    next = pNewNode;
                }
            }
        }

        if (!next) {
            return true;
        }

        prev = curr;
        curr = next;
    }
}

// 0x44D3B0
void CPathFind::TidyUpNodeSwitchesAfterMission() {
    m_nNumNodeSwitches = std::min(54u, m_nNumNodeSwitches); // todo: magic number
}

// 0x44D400
void CPathFind::UnMarkAllRoadNodesAsDontWander() {
    for (auto i = 0u; i < NUM_PATH_MAP_AREAS; ++i) {
        if (!m_pPathNodes[i])
            continue;

        for (auto nodeInd = 0u; nodeInd < m_anNumVehicleNodes[i]; ++nodeInd) {
            m_pPathNodes[i][nodeInd].DontWanderHere = false;
        }
    }
}

// 0x44DD00
void CPathFind::ReleaseRequestedNodes() {
    m_loadAreaRequestPending = false;
}

/*!
* @brief Find intersection info between 2 nodes
* @addr notsa 100% inlined
*/
auto CPathFind::FindIntersection(const CNodeAddress& startNodeAddress, const CNodeAddress& targetNodeAddress) -> CPathIntersectionInfo* {
    // Make sure both nodes areas are loaded
    if (!AreNodeAreasLoaded({ targetNodeAddress, startNodeAddress })) {
        return nullptr;
    }

    const auto& startNode = *GetPathNode(startNodeAddress);
    const auto& nodeLinks = pAdjacentNodes[startNodeAddress.Region];
    for (auto i = 0u; i < startNode.NumberAdjNodes; i++) {
        const auto linkedNodeIdx = startNode.IndexAdjacentNodes + i;
        if (nodeLinks[linkedNodeIdx] == targetNodeAddress) {
            return &m_pPathIntersections[startNodeAddress.Region][linkedNodeIdx];
        }
    }

    return nullptr;
}

// 0x44D790
bool CPathFind::TestCrossesRoad(CNodeAddress startNodeAddress, CNodeAddress targetNodeAddress) {
    const auto intersect = FindIntersection(startNodeAddress, targetNodeAddress);
    return intersect && intersect->m_bRoadCross;
}

// 0x44ECA0
bool CPathFind::GeneratePedCreationCoors_Interior(float x, float y, CVector* outCoords, CNodeAddress* unused1, CNodeAddress* unused2, float* outOrientation) {
    return plugin::CallMethodAndReturn<bool, 0x44ECA0>(this, x, y, outCoords, unused1, unused2, outOrientation);
}

// 0x44D480
bool CPathFind::TestForPedTrafficLight(CNodeAddress startNodeAddress, CNodeAddress targetNodeAddress) {
    const auto intersect = FindIntersection(startNodeAddress, targetNodeAddress);
    return intersect && intersect->m_bPedTrafficLight;
}

// 0x4509A0
CVector CPathFind::TakeWidthIntoAccountForWandering(CNodeAddress nodeAddress, int16 randomSeed) {
    // Invalid area, or area not loaded
    if (!nodeAddress.IsValid() || !IsAreaNodesAvailable(nodeAddress)) {
        return {};
    }

    auto node         = GetPathNode(nodeAddress);
    auto basePosition = node->GetPosition();
    auto offsetX      = float(node->Width * ((randomSeed % 16) - 7)); // bottom 8 bits remapped to [-7 : +8]
    auto offsetY      = float(node->Width * (((randomSeed / 16) % 16) - 7)); // top 8 bits remapped to [-7 : +8]
    auto offset       = CVector{ offsetX * 0.00775f, offsetY * 0.00775f, 0.f };
    return basePosition + offset;
}

// 0x44F8C0
CNodeAddress CPathFind::FindNthNodeClosestToCoors(CVector pos, uint8 nodeType, float maxDistance, bool bLowTraffic, bool bUnkn, int32 nthNode, bool bBoatsOnly,
                                                  bool bIgnoreInterior, CNodeAddress* outNode) {
    for (size_t region = 0; region < NUM_TOTAL_PATH_NODE_AREAS; ++region) {
        if (m_pPathNodes[region]) {
            uint32 startNode = 0;
            uint32 endNode = 0;

            switch (nodeType) {
            case 0:
                startNode = 0;
                endNode = m_anNumVehicleNodes[region];
                break;
            case 1:
                startNode = m_anNumVehicleNodes[region];
                endNode = m_anNumNodes[region];
                break;
            default:
                break;
            }

            for (uint32 node = startNode; node < endNode; ++node) {
                m_pPathNodes[region][node].unk1 = 0;
            }
        }
    }

    while (nthNode > 0) {
        CNodeAddress nodeFound = FindNodeClosestToCoors(pos, static_cast<ePathType>(nodeType), maxDistance, bLowTraffic, bUnkn, true, bBoatsOnly, bIgnoreInterior);
        if (outNode) {
            *outNode = nodeFound;
        }

        if (!nodeFound.IsValid()) {
            return nodeFound;
        }

        GetPathNode(nodeFound)->unk1 = 1;
        nthNode--;
    }

    return FindNodeClosestToCoors(pos, static_cast<ePathType>(nodeType), maxDistance, bLowTraffic, bUnkn, true, bBoatsOnly, false);
}

// 0x451B70
void CPathFind::FindNextNodeWandering(uint8 nodeType, CVector vecPos, CNodeAddress* originAddress, CNodeAddress* targetAddress, uint8 dir, uint8* outDir) {
    float DirY, DirX, Length, DiffY, DiffX;
    float HighestDotProd, DotProd;
    CNodeAddress StartNode, AdjNode, OriginalNode;
    CPathNode *pStartNode, *pAdjNode, *pSourceNode;

    OriginalNode = EmptyNodeAddress;

    if (originAddress) {
        OriginalNode = *originAddress;
        if (originAddress->IsValid()) {
            if (!IsAreaNodesAvailable(*originAddress)) {
                *targetAddress = EmptyNodeAddress;
                *outDir = 0;
                return;
            }
        }
    }

    if (!originAddress || !originAddress->IsValid() || (vecPos - GetPathNode(*originAddress)->GetPosition()).Magnitude() > std::max(7.0f, float(GetPathNode(*originAddress)->Width) * 0.125f)) {
        StartNode = FindNodeClosestToCoors(vecPos, static_cast<ePathType>(nodeType), 999999.0f, false, false, false, false, false);
    } else {
        StartNode = *originAddress;
    }

    if (StartNode.IsValid() && IsAreaNodesAvailable(StartNode)) {
        pStartNode = GetPathNode(StartNode);
        *targetAddress = EmptyNodeAddress;

        DirY = std::sin(static_cast<float>(dir) * 0.78539819f);
        DirX = std::cos(static_cast<float>(dir) * 0.78539819f);

        HighestDotProd = -999999.0f;

        for (int16 i = 0; i < (int16)pStartNode->NumberAdjNodes; i++) {
            AdjNode = pAdjacentNodes[pStartNode->Region][pStartNode->IndexAdjacentNodes + i];
            if (!IsAreaNodesAvailable(AdjNode)) {
                continue;
            }

            pAdjNode = GetPathNode(AdjNode);
            if (!pStartNode->SwitchedOff && pAdjNode->SwitchedOff) {
                continue;
            }

            if (!pStartNode->DontWanderHere && pAdjNode->DontWanderHere) {
                continue;
            }

            DiffX = pAdjNode->GetPosition().x - pStartNode->GetPosition().x;
            DiffY = pAdjNode->GetPosition().y - pStartNode->GetPosition().y;
            Length = 1.0f / std::sqrt(DiffX * DiffX + DiffY * DiffY);
            DiffX *= Length;
            DiffY *= Length;

            DotProd = DiffX * DirX + DiffY * DirY;
            if (DotProd >= HighestDotProd) {
                HighestDotProd = DotProd;
                *targetAddress = AdjNode;

                if (DiffY >= 0.0f) {
                    if (std::abs(DiffX) * 2.0f < DiffY) {
                        *outDir = 2;
                    } else if (DiffY * 2.0f < DiffX) {
                        *outDir = 0;
                    } else if (DiffY * -2.0f > DiffX) {
                        *outDir = 4;
                    } else if (DiffX > 0.0f) {
                        *outDir = 1;
                    } else {
                        *outDir = 3;
                    }
                } else {
                    if (-DiffY > std::abs(DiffX) * 2.0f) {
                        *outDir = 6;
                    } else if (DiffY * -2.0f < DiffX) {
                        *outDir = 0;
                    } else if (DiffY * 2.0f > DiffX) {
                        *outDir = 4;
                    } else if (DiffX > 0.0f) {
                        *outDir = 7;
                    } else {
                        *outDir = 5;
                    }
                }
            }
        }

        *originAddress = StartNode;
    } else {
        *targetAddress = EmptyNodeAddress;
        StartNode = EmptyNodeAddress;
    }

    if (!targetAddress->IsValid()) {
        *outDir = 0;
        *targetAddress = StartNode;
    }

    if (*targetAddress == OriginalNode) {
        if (OriginalNode.IsValid() && IsAreaNodesAvailable(OriginalNode)) {
            pSourceNode = GetPathNode(OriginalNode);
            for (int32 N = 0; N < (int32)pSourceNode->NumberAdjNodes; N++) {
                CNodeAddress CandidateNode = pAdjacentNodes[pSourceNode->Region][pSourceNode->IndexAdjacentNodes + N];
                if (IsAreaNodesAvailable(CandidateNode)) {
                    *targetAddress = CandidateNode;
                    break;
                }
            }
        }
    }
}

// 0x4515D0
void CPathFind::DoPathSearch(
    ePathType pathType,
    CVector originPos,
    CNodeAddress originAddrAddr,
    CVector targetPos,
    CNodeAddress* outResultNodes,
    int16& outNodesCount,
    int32 maxNodesToFind,
    float* outDistance,
    float maxSearchDistance,
    CNodeAddress* targetNodeAddr,
    float maxUnkLimit,
    bool oneSideOnly,
    CNodeAddress forbiddenNodeAddr,
    bool includeNodesWithoutLinks,
    bool waterPath
) {
    CNodeAddress targetNode;
    if (targetNodeAddr && targetNodeAddr->IsValid() && IsAreaNodesAvailable(*targetNodeAddr)) {
        targetNode = *targetNodeAddr;
    } else {
        targetNode = FindNodeClosestToCoors(targetPos, pathType, maxSearchDistance, 0, 0, 0, waterPath, 0);
    }

    if (!targetNode.IsValid()) {
        outNodesCount = 0;
        if (outDistance) {
            *outDistance = 100000.0f;
        }
        return;
    }

    CNodeAddress startNode = originAddrAddr;
    if (!startNode.IsValid() || !IsAreaNodesAvailable(startNode)) {
        startNode = FindNodeClosestToCoors(originPos, pathType, maxSearchDistance, 0, 0, 0, waterPath, 0);
    }

    if (!startNode.IsValid()) {
        outNodesCount = 0;
        if (outDistance) {
            *outDistance = 100000.0f;
        }
        return;
    }

    if (startNode == targetNode) {
        outNodesCount = 0;
        if (outDistance) {
            *outDistance = 0.0f;
        }
        return;
    }

    CPathNode* pTargetNode = GetPathNode(targetNode);
    CPathNode* pStartNode = GetPathNode(startNode);

    if (pTargetNode->Group != pStartNode->Group) {
        outNodesCount = 0;
        if (outDistance) {
            *outDistance = 100000.0f;
        }
        return;
    }

    for (int32 c = 0; c < 512; c++) {
        m_pathFindHashTable[c] = nullptr;
    }

    AddNodeToList(pTargetNode, 0);
    aNodesToBeCleared[0] = pTargetNode->GetAddress();
    int32 numNodesToBeCleared = 1;
    int32 currentRoughDist = 0;
    int32 currentHashValue = 0;
    bool bStartNodeFound = false;

    while (!bStartNodeFound) {
        for (CPathNode* pCurrNode = m_pathFindHashTable[currentHashValue]; pCurrNode != nullptr; pCurrNode = pCurrNode->m_next) {
            if (pCurrNode == pStartNode) {
                bStartNodeFound = true;
            }

            for (int32 neighbours = 0; neighbours < (int32)pCurrNode->NumberAdjNodes; ++neighbours) {
                const auto linkIndex = neighbours + pCurrNode->IndexAdjacentNodes;
                const CNodeAddress neighbourNode = pAdjacentNodes[pCurrNode->Region][linkIndex];
                if (IsAreaNodesAvailable(neighbourNode)) {
                    CPathNode* pCandidateNode = GetPathNode(neighbourNode);
                    bool bDontConsiderThisNeighbour = false;

                    if (oneSideOnly) {
                        const CCarPathLinkAddress candidateLink = pAdjacentLinks[pCurrNode->Region][linkIndex];
                        if (m_pPathNodes[candidateLink.Region]) {
                            const CCarPathLink& candidateLinkRef = m_pNaviNodes[candidateLink.Region][candidateLink.m_wCarPathLinkId];
                            int32 lanesGoingOurWay;
                            if (candidateLinkRef.Node1 == neighbourNode) {
                                lanesGoingOurWay = candidateLinkRef.LanesTo;
                            } else {
                                lanesGoingOurWay = candidateLinkRef.LanesFro;
                            }

                            if (lanesGoingOurWay == 0) {
                                bDontConsiderThisNeighbour = true;
                            }
                        }
                    }

                    if (neighbourNode == forbiddenNodeAddr) {
                        bDontConsiderThisNeighbour = true;
                    }

                    if (pCurrNode->WaterNode != pCandidateNode->WaterNode) {
                        if (!includeNodesWithoutLinks) {
                            bDontConsiderThisNeighbour = true;
                        }
                    }

                    if (!bDontConsiderThisNeighbour) {
                        const int32 candidateDist = pCurrNode->DistanceToTarget + m_pLinkLengths[pCurrNode->Region][linkIndex];
                        if (candidateDist < pCandidateNode->DistanceToTarget) {
                            if (pCandidateNode->DistanceToTarget != 0x7FFE) {
                                RemoveNodeFromList(pCandidateNode);
                            }
                            if (pCandidateNode->DistanceToTarget == 0x7FFE) {
                                if (numNodesToBeCleared < 5000) {
                                    aNodesToBeCleared[numNodesToBeCleared++] = pCandidateNode->GetAddress();
                                }
                            }

                            AddNodeToList(pCandidateNode, candidateDist);
                        }
                    }
                }
            }

            RemoveNodeFromList(pCurrNode);
        }

        currentRoughDist++;
        currentHashValue = currentRoughDist & 0x1FF;
        if (m_totalNumNodesInPathFindHashTable == 0 || static_cast<float>(currentRoughDist) > maxUnkLimit || numNodesToBeCleared >= 4950) {
            outNodesCount = 0;
            for (int32 node = 0; node < numNodesToBeCleared; ++node) {
                GetPathNode(aNodesToBeCleared[node])->DistanceToTarget = 0x7FFE;
            }
            return;
        }
    }

    outNodesCount = 0;
    if (outDistance) {
        *outDistance = static_cast<float>(pStartNode->DistanceToTarget);
    }

    if (outResultNodes) {
        outResultNodes[outNodesCount++] = pStartNode->GetAddress();
    }

    CPathNode* pBacktrackNode = pStartNode;
    while (outNodesCount < maxNodesToFind && pBacktrackNode != pTargetNode) {
        for (int32 c = 0; c < (int32)pBacktrackNode->NumberAdjNodes; ++c) {
            const auto linkIndex = pBacktrackNode->IndexAdjacentNodes + c;
            const CNodeAddress candidateNode = pAdjacentNodes[pBacktrackNode->Region][linkIndex];
            if (IsAreaNodesAvailable(candidateNode)) {
                CPathNode* pCandidateNode = GetPathNode(candidateNode);
                if (pBacktrackNode->DistanceToTarget - m_pLinkLengths[pBacktrackNode->Region][linkIndex] == pCandidateNode->DistanceToTarget) {
                    pBacktrackNode = pCandidateNode;
                    if (outResultNodes) {
                        outResultNodes[outNodesCount++] = pCandidateNode->GetAddress();
                    } else {
                        outNodesCount++;
                    }
                    c = 29030;
                }
            }
        }
    }

    for (int32 node = 0; node < numNodesToBeCleared; node++) {
        GetPathNode(aNodesToBeCleared[node])->DistanceToTarget = 0x7FFE;
    }
}

// 0x452760
void CPathFind::ComputeRoute(uint8 nodeType, const CVector& vecStart, const CVector& vecEnd, const CNodeAddress& startAddress, CNodeRoute* route) {
    if (!route) {
        return;
    }

    int16 nodesCount = 0;
    std::array<CNodeAddress, 8> pathNodes{};

    DoPathSearch(
        static_cast<ePathType>(nodeType),
        vecStart,
        startAddress,
        vecEnd,
        pathNodes.data(),
        nodesCount,
        static_cast<int32>(pathNodes.size()),
        nullptr,
        999999.88f,
        nullptr,
        999999.88f,
        false,
        CNodeAddress(),
        false,
        false
    );

    route->Clear();
    for (int16 i = 0; i < nodesCount; ++i) {
        if (route->m_NumEntries >= 8) {
            break;
        }
        route->Add(pathNodes[i]);
    }
}

// 0x44D960
void CPathFind::SetLinksBridgeLights(float fXMin, float fXMax, float fYMin, float fYMax, bool value) {
    const auto areaRect = CRect{ {fXMin, fYMin}, {fXMax, fYMax} };
    for (auto areaId = 0u; areaId < NUM_PATH_MAP_AREAS; areaId++) {
        if (!IsAreaLoaded(areaId)) {
            continue;
        }

        for (auto n = 0u; n < m_anNumCarPathLinks[areaId]; n++) {
            auto& node = GetCarPathLink({ areaId, n });
            if (areaRect.IsPointInside(node.GetNodeCoors())) {
                node.BridgeLights = value;
            }
        }
    }
}

namespace detail {
// NOTSA
CVector GetPosnBetweenNodesForScript(CPathNode* nodeA, CVector2D dir) {
    return nodeA->GetPosition() + CVector{dir.GetPerpLeft() * ((float)nodeA->Width / 16.f + 2.7f)};
}
};

// 0x4505E0
CVector CPathFind::FindNodeCoorsForScript(CNodeAddress address, bool* bFound) {
    const auto SetFound = [&](bool found) {
        if (bFound) {
            *bFound = found;
        }
    };
    if (!address.IsValid() || !IsAreaNodesAvailable(address)) {
        SetFound(false);
        return {};
    } else {
        SetFound(true);

        const auto node = GetPathNode(address);
        const auto nodePos = node->GetPosition();
 
        // If this node has a link return some kind of position between this and the first link
        if (node->Width && node->NumberAdjNodes) {
            if (const auto firstLink = pAdjacentNodes[node->IndexAdjacentNodes]) {
                if (const auto firstLinkedNode = GetPathNode(*firstLink)) {
                    auto dir = CVector2D{ firstLinkedNode->GetPosition() - nodePos }.Normalized();

                    // By negating here we invert the direction
                    dir = dir.x >= 0 ? dir : -dir;

                    return detail::GetPosnBetweenNodesForScript(node, dir);
                }
            }
        }

        // Otherwise just return this node's position
        return nodePos;
    }
}

// 0x450780
CVector CPathFind::FindNodeCoorsForScript(CNodeAddress nodeAddrA, CNodeAddress nodeAddrB, float& outHeadingDeg, bool* outFound) {
    const auto SetFound = [&](bool found) {
        if (outFound) {
            *outFound = found;
        }
    };
    if (nodeAddrA.IsValid() && nodeAddrB.IsValid() && AreNodeAreasLoaded({ nodeAddrA, nodeAddrB })) { // Inverted
        SetFound(true);

        const auto nodeA = GetPathNode(nodeAddrA);
        const auto posA  = nodeA->GetPosition();
        const auto dir   = CVector2D{ GetPathNode(nodeAddrB)->GetPosition() - posA }.Normalized();

        outHeadingDeg = RWRAD2DEG(dir.Heading());

        return nodeA->Width
            ? detail::GetPosnBetweenNodesForScript(nodeA, dir)
            : posA;
    } else {
        SetFound(false);
        return {};
    }
}

// 0x450560
void CPathFind::MarkRoadNodeAsDontWander(float x, float y, float z) {
    CVector pos = {x, y, z};
    auto node = FindNodeClosestToCoors(pos, PATH_TYPE_VEH, 999999.88f, 0, 0, 0, 0, 0);
    if (node.IsValid()) {
        m_pPathNodes[node.Region][node.Index].DontWanderHere = true;
    }
}

// 0x452820
void CPathFind::SwitchRoadsOffInAreaForOneRegion(float xMin, float xMax, float yMin, float yMax, float zMin, float zMax, bool bSwitchOff, bool bCars, int areaId, bool bBackToOriginal) {
    assert(areaId >= 0 && areaId < NUM_PATH_MAP_AREAS);

    if (!IsAreaLoaded(areaId)) {
        return;
    }

    auto start = bCars ? 0 : m_anNumVehicleNodes[areaId];
    auto end   = bCars ? m_anNumVehicleNodes[areaId] : m_anNumNodes[areaId];

    for (auto nodeIdx = start; nodeIdx < end; ++nodeIdx) {
        CPathNode& node     = m_pPathNodes[areaId][nodeIdx];
        const auto position = node.GetPosition();

        if (position.x < xMin || position.x > xMax || position.y < yMin || position.y > yMax || position.z < zMin || position.z > zMax) {
            continue;
        }
        if (!ThisNodeHasToBeSwitchedOff(&node) || node.SwitchedOff == (bBackToOriginal ? node.SwitchedOffOriginal : bSwitchOff)) {
            continue;
        }
        CPathNode* next1{};
        CPathNode* next2{};

        SwitchOffNodeAndNeighbours(&node, next1, &next2, bSwitchOff, bBackToOriginal);

        for (auto iter = next1; iter;) {
            SwitchOffNodeAndNeighbours(iter, iter, nullptr, bSwitchOff, bBackToOriginal);
        }
        for (auto iter = next2; iter;) {
            SwitchOffNodeAndNeighbours(iter, iter, nullptr, bSwitchOff, bBackToOriginal);
        }
    }
}

// NOTSA
CPathNode* CPathFind::GetPathNode(CNodeAddress address) {
    assert(address.IsValid());
    assert(IsAreaNodesAvailable(address));
    return &m_pPathNodes[address.Region][address.Index];
}

// notsa
CCarPathLinkAddress CPathFind::GetNaviLink(uint16 area, uint16 linkId) const {
    assert(area < NUM_PATH_MAP_AREAS);
    assert(linkId < m_anNumAddresses[area]);
    return pAdjacentLinks[area][linkId];
}

// NOTSA
bool CPathFind::FindNodeCoorsForScript(CVector& outPos, CNodeAddress addr) {
    bool valid{};
    outPos = FindNodeCoorsForScript(addr, &valid);
    return valid;
}

// 0x452F40
void CPathFind::LoadPathFindData(int32 areaId) {
    CTimer::Suspend();
    sprintf_s(gString, "data\\paths\\nodes%d.dat", areaId);
    auto* stream = RwStreamOpen(RwStreamType::rwSTREAMFILENAME, RwStreamAccessType::rwSTREAMREAD, gString);
    LoadPathFindData(stream, areaId);
    CTimer::Resume();
}

// 0x4529F0
void CPathFind::LoadPathFindData(RwStream* stream, int32 areaId) {
    RwStreamRead(stream, &m_anNumNodes[areaId],        sizeof(m_anNumNodes[areaId]));
    RwStreamRead(stream, &m_anNumVehicleNodes[areaId], sizeof(m_anNumVehicleNodes[areaId]));
    RwStreamRead(stream, &m_anNumPedNodes[areaId],     sizeof(m_anNumPedNodes[areaId]));
    RwStreamRead(stream, &m_anNumCarPathLinks[areaId], sizeof(m_anNumCarPathLinks[areaId]));
    RwStreamRead(stream, &m_anNumAddresses[areaId],    sizeof(m_anNumAddresses[areaId]));

    assert(m_anNumNodes[areaId] == m_anNumVehicleNodes[areaId] + m_anNumPedNodes[areaId]);

    auto numNodes = m_anNumNodes[areaId];
    if (numNodes) {
        m_pPathNodes[areaId] = new CPathNode[numNodes];
        RwStreamRead(stream, m_pPathNodes[areaId], sizeof(CPathNode) * numNodes);
    } else {
        m_pPathNodes[areaId] = new CPathNode[1];
    }

    auto numCarPathLinks = m_anNumCarPathLinks[areaId];
    if (numCarPathLinks) {
        m_pNaviNodes[areaId] = new CCarPathLink[numCarPathLinks];
        RwStreamRead(stream, m_pNaviNodes[areaId], sizeof(CCarPathLink) * numCarPathLinks);
    } else {
        m_pNaviNodes[areaId] = nullptr;
    }

    auto numAddresses = m_anNumAddresses[areaId];
    if (numAddresses) {
        auto numToAdd = numAddresses + NUM_DYNAMIC_LINKS_PER_AREA * 12;
        pAdjacentNodes[areaId] = new CNodeAddress[numToAdd];
        pAdjacentLinks[areaId] = new CCarPathLinkAddress[numAddresses];
        m_pLinkLengths[areaId] = new uint8[numToAdd];
        m_pPathIntersections[areaId] = new CPathIntersectionInfo[numToAdd];
        RwStreamRead(stream, pAdjacentNodes[areaId], sizeof(CNodeAddress) * numToAdd);
        RwStreamRead(stream, pAdjacentLinks[areaId], sizeof(CCarPathLinkAddress) * numAddresses);
        RwStreamRead(stream, m_pLinkLengths[areaId], sizeof(uint8) * numToAdd);
        RwStreamRead(stream, m_pPathIntersections[areaId], sizeof(CPathIntersectionInfo) * numToAdd);
    } else {
        pAdjacentNodes[areaId] = nullptr;
        pAdjacentLinks[areaId] = nullptr;
        m_pLinkLengths[areaId] = nullptr;
        m_pPathIntersections[areaId] = nullptr;
    }

    for (auto i = 0u; i < numNodes; ++i) {
        auto& node = m_pPathNodes[areaId][i];
        node.SwitchedOffOriginal = node.SwitchedOff;
    }

    for (auto i = 0u; i < m_nNumNodeSwitches; ++i) {
        auto& area = m_aNodeSwitches[i];
        SwitchRoadsOffInAreaForOneRegion(area.xMin, area.xMax, area.yMin, area.yMax, area.zMin, area.zMax, area.isOff, area.isCars, areaId, false);
    }
    for (auto i = 0u; i < NUM_DYNAMIC_LINKS_PER_AREA; ++i) {
        rng::fill(m_aDynamicLinksBaseIds[i], -1);
        rng::fill(m_aDynamicLinksIds[i], -1);
    }
}

// 0x44D0F0
void CPathFind::UnLoadPathFindData(int32 index) {
    delete[] m_pPathNodes[index];
    delete[] m_pNaviNodes[index];
    delete[] pAdjacentNodes[index];
    delete[] pAdjacentLinks[index];
    delete[] m_pLinkLengths[index];
    delete[] m_pPathIntersections[index];

    m_pPathNodes[index] = nullptr;
    m_pNaviNodes[index] = nullptr;
    pAdjacentNodes[index] = nullptr;
    pAdjacentLinks[index] = nullptr;
    m_pLinkLengths[index] = nullptr;
    m_pPathIntersections[index] = nullptr;
}

// 0x44DE00
void CPathFind::LoadSceneForPathNodes(CVector point) {
    rng::fill(ToBeStreamed, false);
    MarkRegionsForCoors(point, 350.f);
    for (const auto [areaId, load] : rngv::enumerate(ToBeStreamed)) {
        if (load) {
            CStreaming::RequestModel(DATToModelId((int32)areaId), STREAMING_DEFAULT);
        }
    }
}

// 0x450DE0
bool CPathFind::IsWaterNodeNearby(CVector position, float radius) {
    for (auto areaId = 0u; areaId < NUM_PATH_MAP_AREAS; areaId++) {
        for (const auto& node : GetPathNodesInArea(areaId, PATH_TYPE_VEH)) {
            if (node.WaterNode) {
                if ((node.GetPosition() - position).SquaredMagnitude() <= sq(radius)) {
                    return true;
                }
            }
        }
    }
    return false;
}

// 0x44EFC0
float CPathFind::CalcRoadDensity(float x, float y) {
    float DensityCount = 0.0f;

    for (size_t Region = 0; Region < NUM_PATH_MAP_AREAS; ++Region) {
        if (m_pPathNodes[Region] != nullptr) {
            for (uint32 Node = 0; Node < m_anNumVehicleNodes[Region]; ++Node) {
                CPathNode* pNode = &m_pPathNodes[Region][Node];

                if (std::abs(pNode->GetPosition().x - x) < 80.0f && std::abs(pNode->GetPosition().y - y) < 80.0f && pNode->NumberAdjNodes > 0) {
                    for (int32 Link = 0; Link < (int32)pNode->NumberAdjNodes; ++Link) {
                        CNodeAddress OtherNode = pAdjacentNodes[Region][pNode->IndexAdjacentNodes + Link];
                        if (IsAreaNodesAvailable(OtherNode)) {
                            CPathNode* pOtherNode = GetPathNode(OtherNode);
                            float DeltaX = pNode->GetPosition().x - pOtherNode->GetPosition().x;
                            float DeltaY = pNode->GetPosition().y - pOtherNode->GetPosition().y;
                            float Length = std::sqrt(DeltaX * DeltaX + DeltaY * DeltaY);

                            CCarPathLinkAddress pLinkAddress = pAdjacentLinks[Region][pNode->IndexAdjacentNodes + Link];
                            if (m_pPathNodes[pLinkAddress.Region] != nullptr) {
                                DensityCount += Length * GetCarPathLink(pLinkAddress).LanesFro;
                                DensityCount += Length * GetCarPathLink(pLinkAddress).LanesTo;
                            }
                        }
                    }
                }
            }
        }
    }

    return DensityCount / 2500.0f;
}

// 0x44F190
float CPathFind::CalcDistToAnyConnectingLinks(CPathNode* node, CVector pos) {
    float SmallestDist = 999999.9f;

    for (int32 AdjNode = 0; AdjNode < (int32)node->NumberAdjNodes; ++AdjNode) {
        CNodeAddress OtherNode = pAdjacentNodes[node->Region][node->IndexAdjacentNodes + AdjNode];
        if (IsAreaNodesAvailable(OtherNode)) {
            CPathNode* pOtherNode = GetPathNode(OtherNode);
            CVector Crs1 = node->GetPosition();
            CVector Crs2 = pOtherNode->GetPosition();
            SmallestDist = std::min(SmallestDist, CCollision::DistToLineSqr(Crs1, Crs2, pos));
        }
    }

    return std::sqrt(SmallestDist);
}

// 0x44F2C0
void CPathFind::FindNodeClosestInRegion(CNodeAddress* outAddress, uint16 areaId, CVector pos, uint8 nodeType, float* outDist, bool bLowTraffic, bool bUnkn, bool bBoats, bool bUnused) {
    if (!m_pPathNodes[areaId]) {
        return;
    }

    uint32 startNode = 0;
    uint32 endNode = 0;

    switch (nodeType) {
    case 0:
        startNode = 0;
        endNode = m_anNumVehicleNodes[areaId];
        break;
    case 1:
        startNode = m_anNumVehicleNodes[areaId];
        endNode = m_anNumNodes[areaId];
        break;
    default:
        return;
    }

    for (uint32 nodeIdx = startNode; nodeIdx < endNode; ++nodeIdx) {
        CPathNode* node = &m_pPathNodes[areaId][nodeIdx];

        if (bLowTraffic && node->SwitchedOff) {
            continue;
        }

        if (bUnkn && node->unk1) {
            continue;
        }

        if (node->WaterNode != bBoats) {
            continue;
        }

        const CVector nodePos = node->GetPosition();
        float dist = std::abs(nodePos.x - pos.x) + std::abs(nodePos.y - pos.y) + std::abs(nodePos.z - pos.z) * 3.0f;
        float totalDist = dist * 0.75f;

        if (totalDist < *outDist) {
            *outDist = totalDist;
            outAddress->Region = areaId;
            outAddress->Index = static_cast<uint16>(nodeIdx);
        }
    }
}

// 0x44F460
CNodeAddress CPathFind::FindNodeClosestToCoors(
    CVector SearchCoors,
    ePathType GraphType,
    float CutoffDist,
    uint16 bIgnoreSwitchedOff,
    int32 bIgnoreBetweenLevels,
    uint16 bIgnoreAlreadyFound,
    uint16 bBoatNodes,
    int32 bIgnoreInteriors
) {
    int32 CenterRegionY, CenterRegionX, RegionX, RegionY;
    int32 Area;
    CNodeAddress ClosestNode;
    float ClosestDist, DistToBorderOfRegion;
    uint16 Region;

    ClosestDist = CutoffDist;

    CenterRegionX = (int32)FindXRegionForCoors(SearchCoors.x);
    CenterRegionY = (int32)FindYRegionForCoors(SearchCoors.y);
    Region = static_cast<uint16>(CenterRegionX + 8 * CenterRegionY);

    DistToBorderOfRegion = SearchCoors.x - FindXCoorsForRegion(CenterRegionX);
    DistToBorderOfRegion = std::min(DistToBorderOfRegion, FindXCoorsForRegion(CenterRegionX + 1) - SearchCoors.x);
    DistToBorderOfRegion = std::min(DistToBorderOfRegion, SearchCoors.y - FindYCoorsForRegion(CenterRegionY));
    DistToBorderOfRegion = std::min(DistToBorderOfRegion, FindYCoorsForRegion(CenterRegionY + 1) - SearchCoors.y);

    FindNodeClosestInRegion(&ClosestNode, Region, SearchCoors, (uint8)GraphType, &ClosestDist, bIgnoreSwitchedOff != 0, bIgnoreAlreadyFound != 0, bBoatNodes != 0, bIgnoreInteriors != 0);

    if (ClosestDist > DistToBorderOfRegion) {
        for (Area = 1; Area < 5; Area++) {
            RegionX = CenterRegionX - Area;
            if (RegionX >= 0 && RegionX < NUM_PATH_MAP_AREA_X) {
                for (RegionY = CenterRegionY - Area; RegionY <= CenterRegionY + Area; RegionY++) {
                    if (RegionY >= 0 && RegionY < NUM_PATH_MAP_AREA_Y) {
                        FindNodeClosestInRegion(&ClosestNode, static_cast<uint16>(RegionX + 8 * RegionY), SearchCoors, (uint8)GraphType, &ClosestDist, bIgnoreSwitchedOff != 0, bIgnoreAlreadyFound != 0, bBoatNodes != 0, bIgnoreInteriors != 0);
                    }
                }
            }

            RegionX = CenterRegionX + Area;
            if (RegionX >= 0 && RegionX < NUM_PATH_MAP_AREA_X) {
                for (RegionY = CenterRegionY - Area; RegionY <= CenterRegionY + Area; RegionY++) {
                    if (RegionY >= 0 && RegionY < NUM_PATH_MAP_AREA_Y) {
                        FindNodeClosestInRegion(&ClosestNode, static_cast<uint16>(RegionX + 8 * RegionY), SearchCoors, (uint8)GraphType, &ClosestDist, bIgnoreSwitchedOff != 0, bIgnoreAlreadyFound != 0, bBoatNodes != 0, bIgnoreInteriors != 0);
                    }
                }
            }

            RegionY = CenterRegionY - Area;
            if (RegionY >= 0 && RegionY < NUM_PATH_MAP_AREA_Y) {
                for (RegionX = CenterRegionX - Area + 1; RegionX < CenterRegionX + Area; RegionX++) {
                    if (RegionX >= 0 && RegionX < NUM_PATH_MAP_AREA_X) {
                        FindNodeClosestInRegion(&ClosestNode, static_cast<uint16>(RegionX + 8 * RegionY), SearchCoors, (uint8)GraphType, &ClosestDist, bIgnoreSwitchedOff != 0, bIgnoreAlreadyFound != 0, bBoatNodes != 0, bIgnoreInteriors != 0);
                    }
                }
            }

            RegionY = CenterRegionY + Area;
            if (RegionY >= 0 && RegionY < NUM_PATH_MAP_AREA_Y) {
                for (RegionX = CenterRegionX - Area + 1; RegionX < CenterRegionX + Area; RegionX++) {
                    if (RegionX >= 0 && RegionX < NUM_PATH_MAP_AREA_X) {
                        FindNodeClosestInRegion(&ClosestNode, static_cast<uint16>(RegionX + 8 * RegionY), SearchCoors, (uint8)GraphType, &ClosestDist, bIgnoreSwitchedOff != 0, bIgnoreAlreadyFound != 0, bBoatNodes != 0, bIgnoreInteriors != 0);
                    }
                }
            }

            DistToBorderOfRegion += 750.0f;
            if (ClosestDist < DistToBorderOfRegion) {
                break;
            }
        }
    }

    if (!bIgnoreInteriors) {
        for (uint16 InteriorRegion = NUM_PATH_MAP_AREAS; InteriorRegion < NUM_TOTAL_PATH_NODE_AREAS; InteriorRegion++) {
            FindNodeClosestInRegion(&ClosestNode, InteriorRegion, SearchCoors, (uint8)GraphType, &ClosestDist, bIgnoreSwitchedOff != 0, bIgnoreAlreadyFound != 0, bBoatNodes != 0, false);
        }
    }

    return ClosestNode;
}

// 0x44FA30
void CPathFind::RecordNodesClosestToCoors(CVector pos, uint8 nodeType, int count, CNodeAddress* outAddresses, float maxDist, bool bIgnoreSwitchedOff, bool bIgnoreBetweenLevels, bool bWaterNode, bool bIgnoreInteriors) {
    for (size_t region = 0; region < NUM_TOTAL_PATH_NODE_AREAS; ++region) {
        if (m_pPathNodes[region]) {
            uint32 startNode = 0;
            uint32 endNode = 0;

            switch (nodeType) {
            case 0:
                startNode = 0;
                endNode = m_anNumVehicleNodes[region];
                break;
            case 1:
                startNode = m_anNumVehicleNodes[region];
                endNode = m_anNumNodes[region];
                break;
            default:
                break;
            }

            for (uint32 node = startNode; node < endNode; ++node) {
                m_pPathNodes[region][node].unk1 = 0;
            }
        }
    }

    while (count > 0) {
        CNodeAddress nodeFound = FindNodeClosestToCoors(pos, static_cast<ePathType>(nodeType), maxDist, bIgnoreSwitchedOff, bIgnoreBetweenLevels, true, bWaterNode, bIgnoreInteriors);
        if (!nodeFound.IsValid()) {
            return;
        }

        GetPathNode(nodeFound)->unk1 = 1;
        *outAddresses = nodeFound;
        count--;
        outAddresses++;
    }
}

// 0x452090
void CPathFind::Find2NodesForCarCreation(CVector pos, CNodeAddress* outAddress1, CNodeAddress* outAddress2, bool bLowTraffic) {
    CNodeAddress aNodes[4];

    RecordNodesClosestToCoors(pos, 0, 4, aNodes, 999999.9f, bLowTraffic, false, false, true);

    if (!aNodes[0].IsValid()) {
        outAddress1->ResetAreaId();
        outAddress1->ResetNodeId();
        outAddress2->ResetAreaId();
        outAddress2->ResetNodeId();
        return;
    }

    *outAddress1 = aNodes[0];

    for (int32 c = 1; c < 4; ++c) {
        if (aNodes[c].IsValid() && !These2NodesAreAdjacent(aNodes[0], aNodes[c])) {
            *outAddress2 = aNodes[c];
            return;
        }
    }
}

// 0x452030
int16 DummyResult;
int16 DummyResult2;
CNodeAddress NodeList[32];

bool CPathFind::TestCoorsCloseness(CVector Coors, uint8 GraphType, CVector PlayerCoors) // Not used
{
    float Distance;
    CNodeAddress Empty;

#if FIX_BUGS
    float MinDist = 0.50f * TheCamera.m_fGenerationDistMultiplier;
    float MaxDist = 0.100f * TheCamera.m_fGenerationDistMultiplier;
    DoPathSearch(static_cast<ePathType>(GraphType), PlayerCoors, Empty, Coors, nullptr, DummyResult2, 0, &Distance, MinDist, nullptr, MaxDist, false, EmptyNodeAddress, false, false);
#else
    
    DoPathSearch(static_cast<ePathType>(GraphType), PlayerCoors, Empty, Coors, nullptr, DummyResult2, 0, &Distance, 50.0f, nullptr, 100.0f, false, EmptyNodeAddress, false, false);
#endif

if (Distance < 100.0f)
    {
        return true;
    }
    else
    {
        return false;
    }
}

// 0x450A60
void CPathFind::UpdateStreaming(bool bForceStreaming) {
    ZoneScoped;

    // The time thingy I think is some kind of `% 512`, not sure yet, will have to figure it out.
    if (!s_bLoadPathsNeeded && !bForceStreaming && (CTimer::m_snTimeInMilliseconds ^ CTimer::m_snPreviousTimeInMilliseconds) < 512) {
        return;
    }

    rng::fill(ToBeStreamed, false);
    std::array<bool, NUM_PATH_MAP_AREAS> ToBeStreamedForScript{};

    // Mark areas around the player
    if (FindPlayerPed()) {
        MarkRegionsForCoors(FindPlayerCoors(), 350.f);
    }

    // Mark areas requested by `SetPathsNeededAtPosition`
    if (s_bLoadPathsNeeded) {
        MarkRegionsForCoors(s_pathsNeededPosn, 300.f);
        s_bLoadPathsNeeded = false;
    }

    // Mark paths around some specific mission vehicles
    for (const auto& veh : GetVehiclePool()->GetAllValid()) {
        if (!veh.IsMissionVehicle()) {
            continue;
        }

        switch (veh.GetVehicleType()) {
        case VEHICLE_TYPE_HELI:
        case VEHICLE_TYPE_PLANE:
        case VEHICLE_TYPE_BOAT:
        case VEHICLE_TYPE_TRAIN:
        case VEHICLE_TYPE_FPLANE:
            break;
        default:
            MarkRegionsForCoors(veh.GetPosition(), 300.f);
        }
    }

    // Mark areas inside load request rect
    if (m_loadAreaRequestPending) {
        IterAreasTouchingRect(
            { m_loadAreaRequestMinX, m_loadAreaRequestMinY, m_loadAreaRequestMaxX, m_loadAreaRequestMaxY },
            [&, this](auto areaId) {
                ToBeStreamed[areaId] = ToBeStreamedForScript[areaId] = true;
                return true;
            }
        );
    }

    // Load/unload areas as per `ToBeStreamed`
    for (const auto [areaId, shouldBeLoaded] : rngv::enumerate(ToBeStreamed)) {
        if (shouldBeLoaded) {
            if (!IsAreaLoaded(areaId)) {
                CStreaming::RequestModel(
                    DATToModelId(areaId),
                    ToBeStreamedForScript[areaId]
                        ? STREAMING_MISSION_REQUIRED
                        : STREAMING_KEEP_IN_MEMORY
                );
                NOTSA_LOG_DEBUG("Requested area: {}", (int)areaId);
            }
        } else if (IsAreaLoaded(areaId)) {
            CStreaming::RemoveModel(DATToModelId(areaId));
            NOTSA_LOG_DEBUG("Removed area: {}", (int)areaId);
        }
    }
}

// 0x44DE80
void CPathFind::StartNewInterior(int32 interiorNum) {
    InteriorIDBeingBuilt = interiorNum;
    bInteriorBeingBuilt = true;
    NumNodesGiven = 0;
    NumLinksToExteriorNodes = 0;

    // BUG: Possible endless loop if 8 interiors are loaded i think
    NewInteriorSlot = 0;
    while (m_interiorIDs[NewInteriorSlot] != (uint32)-1) {
        NewInteriorSlot++;
        assert(NewInteriorSlot < 8);
    }
}

// 0x450E90
CNodeAddress CPathFind::AddNodeToNewInterior(
    float x,
    float y,
    float z,
    bool bDontWander,
    int8 con0,
    int8 con1,
    int8 con2,
    int8 con3,
    int8 con4,
    int8 con5
) {
    const auto idx = NumNodesGiven++;
    XCoorGiven[idx] = x;
    YCoorGiven[idx] = y;
    ZCoorGiven[idx] = z;
    DontWanderGiven[idx] = bDontWander;
    rng::copy(std::array{ con0, con1, con2, con3, con4, con5 }, ConnectsToGiven[idx].begin());
    return { (uint16)(NUM_PATH_MAP_AREAS + NewInteriorSlot), (uint16)idx };
}

// 0x451300 unused
CNodeAddress CPathFind::ReturnInteriorNodeIndex(int32 unkn, uint32 intId, int16 nodeId) {
    for (auto i = 0; i < NUM_PATH_INTERIOR_AREAS; ++i) {
        if (m_interiorIDs[i] == intId) {
            return CNodeAddress(NUM_PATH_MAP_AREAS + i, nodeId);
        }
    }
    return {};
}

// 0x451350
CCarPathLinkAddress CPathFind::FindLinkBetweenNodes(CNodeAddress nodeAddrA, CNodeAddress nodeAddrB) {
    if (AreNodeAreasLoaded({ nodeAddrA, nodeAddrB })) {
        const auto nodeA = GetPathNode(nodeAddrA);
        for (auto i = 0u; i < nodeA->NumberAdjNodes; i++) {
            const auto linkIdx = nodeA->IndexAdjacentNodes + i;
            if (pAdjacentNodes[nodeA->Region][linkIdx] == nodeAddrB) {
                return pAdjacentLinks[nodeA->Region][linkIdx];
            }
        }
    }
    return {};
}

// 0x4513F0
CVector CPathFind::FindParkingNodeInArea(float minX, float maxX, float minY, float maxY, float minZ, float maxZ) {
    static int32 dwParkingNodeIndex = 0;

    int32 index = 0;
    int32 lookingForIndex = dwParkingNodeIndex;
    CVector found0;
    CVector foundIndex;
    bool bFound0 = false;
    bool bFoundIndex = false;

    for (size_t region = 0; region < NUM_PATH_MAP_AREAS; ++region) {
        if (m_pPathNodes[region]) {
            uint32 startNode = 0;
            uint32 endNode = m_anNumVehicleNodes[region];
            for (uint32 nodeIdx = startNode; nodeIdx < endNode; ++nodeIdx) {
                CPathNode& node = m_pPathNodes[region][nodeIdx];
                CVector pos = node.GetPosition();
                if (pos.x > minX && pos.x < maxX && pos.y > minY && pos.y < maxY && pos.z > minZ && pos.z < maxZ) {
                    if (node.SpecialFunction == 2) {
                        if (index == 0) {
                            found0 = pos;
                            bFound0 = true;
                        }
                        if (index == lookingForIndex) {
                            foundIndex = pos;
                            bFoundIndex = true;
                        }
                        index++;
                    }
                }
            }
        }
    }

    dwParkingNodeIndex++;
    if (dwParkingNodeIndex >= index) {
        dwParkingNodeIndex = 0;
    }

    if (!bFound0) {
        return { 0.0f, 0.0f, 0.0f };
    }
    if (bFoundIndex) {
        return foundIndex;
    }
    return found0;
}

// 0x450F30
CNodeAddress CPathFind::FindNearestExteriorNodeToInteriorNode(int32 interiorId) {
    return FindNodeClosestToCoors(
        { XCoorGiven[interiorId], YCoorGiven[interiorId], ZCoorGiven[interiorId] },
        ePathType::PATH_TYPE_PED,
        3.f,
        0,
        0,
        0,
        0,
        true
    );
}

// 0x4512D0
void CPathFind::AddDynamicLinkBetween2Nodes(CNodeAddress NodeAddress1, CNodeAddress NodeAddress2)
{
    AddDynamicLinkBetween2Nodes_For1Node(NodeAddress1, NodeAddress2);
    AddDynamicLinkBetween2Nodes_For1Node(NodeAddress2, NodeAddress1);
}

// 0x44E000
void CPathFind::AddDynamicLinkBetween2Nodes_For1Node(CNodeAddress first, CNodeAddress second) {
    assert(IsAreaNodesAvailable(first));

    auto& firstPathInfo = m_pPathNodes[first.Region][first.Index];
    auto numAddresses = m_anNumAddresses[first.Region];

    uint32 firstLinkId;
    if (static_cast<uint32>(firstPathInfo.IndexAdjacentNodes) >= numAddresses)
        firstLinkId = firstPathInfo.IndexAdjacentNodes;
    else {
        auto* nodeLink = &pAdjacentNodes[first.Region][numAddresses];
        auto linkCounter = 0u;
        while (!nodeLink->IsValid()) {
            nodeLink += 12; // No clue why we jump 12 objects each time (Search: MAGIC_NUM_12)
            ++linkCounter;
        }

        firstLinkId = numAddresses + 12 * linkCounter;
        for (auto i = 0u; i < firstPathInfo.NumberAdjNodes; ++i) {
            pAdjacentNodes[first.Region][firstLinkId + i] = pAdjacentNodes[first.Region][firstPathInfo.IndexAdjacentNodes + i];
            m_pLinkLengths[first.Region][firstLinkId + i] = m_pLinkLengths[first.Region][firstPathInfo.IndexAdjacentNodes + i];
            m_pPathIntersections[first.Region][firstLinkId + i] = m_pPathIntersections[first.Region][firstPathInfo.IndexAdjacentNodes + i];
        }

        if (first.Region < NUM_PATH_MAP_AREAS) {
            auto& linkInfo = m_aDynamicLinksBaseIds[first.Region];
            for (auto i = 0u; i < NUM_DYNAMIC_LINKS_PER_AREA; ++i) {
                if (linkInfo[i] == -1) {
                    m_aDynamicLinksBaseIds[first.Region][i] = firstPathInfo.IndexAdjacentNodes;
                    m_aDynamicLinksIds[first.Region][i] = firstLinkId;
                    break;
                }
            }
        }
    }

    pAdjacentNodes[first.Region][firstLinkId + firstPathInfo.NumberAdjNodes] = second;
    m_pLinkLengths[first.Region][firstLinkId + firstPathInfo.NumberAdjNodes] = 5;
    m_pPathIntersections[first.Region][firstLinkId + firstPathInfo.NumberAdjNodes].Clear();
    firstPathInfo.NumberAdjNodes++;
    firstPathInfo.IndexAdjacentNodes = firstLinkId;
}

// 0x44D230
bool CPathFind::These2NodesAreAdjacent(CNodeAddress nodeAddress1, CNodeAddress nodeAddress2) {
    const auto node1 = GetPathNode(nodeAddress1);
    for (auto i = 0u; i < node1->NumberAdjNodes; i++) {
        if (pAdjacentNodes[node1->Region][node1->IndexAdjacentNodes + i] == nodeAddress2) {
            return true;
        }
    }
    return false;
}

// 0x44FCE0
CNodeAddress CPathFind::FindNodeClosestToCoorsFavourDirection(CVector SearchCoors, ePathType GraphType, CVector2D Dir) {
    int32 Node, Region;
    float ClosestDist, ThisDist;
    int32 StartNode, EndNode;
    float Length, DiffY, DiffX;
    CNodeAddress ClosestNode;
    float DirX = Dir.x;
    float DirY = Dir.y;

    Length = std::sqrt(DirX * DirX + DirY * DirY);
    if (Length != 0.0f) {
        DirX /= Length;
        DirY /= Length;
    } else {
        DirX = 1.0f;
    }

    ClosestDist = 10000.0f;

    for (Region = 0; Region < NUM_TOTAL_PATH_NODE_AREAS; Region++) {
        if (m_pPathNodes[Region] == nullptr) {
            continue;
        }
        switch ((uint8)GraphType) {
        case 0:
            StartNode = 0;
            EndNode = m_anNumVehicleNodes[Region];
            break;
        case 1:
            StartNode = m_anNumVehicleNodes[Region];
            EndNode = m_anNumNodes[Region];
            break;
        default:
            continue;
        }

        for (Node = StartNode; Node < EndNode; Node++) {
            CPathNode* pNode = &m_pPathNodes[Region][Node];

            CVector nodePos = pNode->GetPosition();

            if ((ThisDist = (std::abs(nodePos.x - SearchCoors.x) + std::abs(nodePos.y - SearchCoors.y) + 3.0f * std::abs(nodePos.z - SearchCoors.z))) < ClosestDist) {
                DiffX = nodePos.x - SearchCoors.x;
                DiffY = nodePos.y - SearchCoors.y;
                Length = std::sqrt(DiffX * DiffX + DiffY * DiffY);
                if (Length != 0.0f) {
                    DiffX /= Length;
                    DiffY /= Length;
                } else {
                    DiffX = 1.0f;
                }

                ThisDist += -20.0f * ((DiffX * DirX + DiffY * DirY) - 1.0f);

                if (ThisDist < ClosestDist) {
                    ClosestDist = ThisDist;
                    ClosestNode = { static_cast<uint16>(Region), static_cast<uint16>(Node) };
                }
            }
        }
    }
    return ClosestNode;
}

// 0x5D34C0
bool CPathFind::Save() {
    CGenericGameStorage::SaveDataToWorkBuffer(m_nNumNodeSwitches);
    for (auto& area : std::span{ m_aNodeSwitches, m_nNumNodeSwitches }) {
        CGenericGameStorage::SaveDataToWorkBuffer(area);
    }
    return true;
}

// 0x5D3500
bool CPathFind::Load() {
    CGenericGameStorage::LoadDataFromWorkBuffer(m_nNumNodeSwitches);
    for (auto& area : std::span{ m_aNodeSwitches, m_nNumNodeSwitches }) {
        CGenericGameStorage::LoadDataFromWorkBuffer(area);
    }
    return true;
}

bool CPathFind::AreNodeAreasLoaded(const std::initializer_list<CNodeAddress>& addrs) const {
    return rng::all_of(addrs, [this](auto&& addr) { return IsAreaNodesAvailable(addr); });
}

// 0x44DCD0
void CPathFind::SetPathsNeededAtPosition(const CVector& posn) {
    s_pathsNeededPosn = posn;
    s_bLoadPathsNeeded = true;
}

namespace detail {
constexpr size_t RegionValueOf(float p, size_t nareas) {
    return std::clamp((uint32)((p + 3000.f) / (6000.f / (float)nareas)), 0u, (uint32)nareas - 1);
}
}; // namespace detail

size_t CPathFind::FindXRegionForCoors(float x) const {
    return detail::RegionValueOf(x, NUM_PATH_MAP_AREA_X);
}

size_t CPathFind::FindYRegionForCoors(float y) const {
    return detail::RegionValueOf(y, NUM_PATH_MAP_AREA_Y);
}

// 0x44DB60
void CPathFind::MarkRegionsForCoors(CVector pos, float radius) {
    // HACK: Since the below function isnt `static` (TODO...) we gotta use the class instance here...
    ThePaths.IterAreasTouchingRect(
        { pos, radius },
        [](auto areaId) {
            ToBeStreamed[areaId] = true;
            return true;
        }
    );
}

// 0x44D3E0
bool CPathFind::ThisNodeHasToBeSwitchedOff(CPathNode* node) {
    if (node->SpecialFunction != 1 && node->SpecialFunction != 2) {
        return true;
    }
    return false;
}

// 0x4504F0
int32 CPathFind::CountNeighboursToBeSwitchedOff(CPathNode* node) {
    int32 result = 0;

    for (int32 neighbour = 0; neighbour < (int32)node->NumberAdjNodes; ++neighbour) {
        CNodeAddress neighbourNode = pAdjacentNodes[node->Region][node->IndexAdjacentNodes + neighbour];
        if (IsAreaLoaded(neighbourNode.Region)) {
            CPathNode* pNeighbourNode = &m_pPathNodes[neighbourNode.Region][neighbourNode.Index];
            if (ThisNodeHasToBeSwitchedOff(pNeighbourNode)) {
                result++;
            }
        }
    }

    return result;
}

// 0x44FEE0
void CPathFind::FindNodePairClosestToCoors(CVector pos, uint8 nodeType, CNodeAddress* outFirst, CNodeAddress* outSecond, float* outDist, float minDist, float maxDist, bool bLowTraffic, bool bUnused, bool bBoats) {
    float ClosestDist, ThisDist, Length;
    int32 StartNode, EndNode;
    CNodeAddress ClosestNode, ClosestOtherNode, OtherNode;
    CVector Diff;

    ClosestDist = 10000.0f;
    ClosestNode = EmptyNodeAddress;
    ClosestOtherNode = EmptyNodeAddress;

    for (size_t Region = 0; Region < NUM_TOTAL_PATH_NODE_AREAS; Region++) {
        if (m_pPathNodes[Region]) {
            switch (nodeType) {
            case 0:
                StartNode = 0;
                EndNode = (int32)m_anNumVehicleNodes[Region];
                break;
            case 1:
                StartNode = (int32)m_anNumVehicleNodes[Region];
                EndNode = (int32)m_anNumNodes[Region];
                break;
            default:
                break;
            }

            for (int32 Node = StartNode; Node < EndNode; Node++) {
                CPathNode* pNode = &m_pPathNodes[Region][Node];

                if (bLowTraffic && pNode->SwitchedOff) {
                    continue;
                }

                if (bBoats != (bool)pNode->WaterNode) {
                    continue;
                }

                ThisDist = std::abs(pNode->GetPosition().x - pos.x) + std::abs(pNode->GetPosition().y - pos.y) + 3.0f * std::abs(pNode->GetPosition().z - pos.z);

                if (ThisDist < ClosestDist) {
                    for (int32 Neighbour = 0; Neighbour < (int32)pNode->NumberAdjNodes; Neighbour++) {
                        OtherNode = pAdjacentNodes[Region][pNode->IndexAdjacentNodes + Neighbour];

                        if (IsAreaNodesAvailable(OtherNode)) {
                            CPathNode* pOtherNode = GetPathNode(OtherNode);

                            if (bLowTraffic && pOtherNode->SwitchedOff) {
                                continue;
                            }

                            if (bBoats != (bool)pOtherNode->WaterNode) {
                                continue;
                            }

                            if ((pNode->GetPosition() - pOtherNode->GetPosition()).Magnitude() > minDist) {
                                ClosestDist = ThisDist;
                                ClosestNode = CNodeAddress((uint16)Region, (uint16)Node);
                                ClosestOtherNode = OtherNode;
                            }
                        }
                    }
                }
            }
        }
    }

    if (ClosestDist < maxDist) {
        *outFirst = ClosestNode;
        *outSecond = ClosestOtherNode;

        CPathNode* pNodeClosest = GetPathNode(ClosestNode);
        CPathNode* pNodeClosestOther = GetPathNode(ClosestOtherNode);

        Diff = pNodeClosest->GetPosition() - pNodeClosestOther->GetPosition();
        Diff.z = 0.0f;
        Diff.Normalise();
        Length = RadiansToDegrees(std::atan2(-Diff.x, Diff.y));
        pos = Diff;
        *outDist = Length;
    } else {
        *outFirst = EmptyNodeAddress;
        *outSecond = EmptyNodeAddress;
        *outDist = 0.0f;
    }
}

// 0x450320
float CPathFind::FindNodeOrientationForCarPlacement(CNodeAddress address) {
    if (!address.IsValid() || !IsAreaLoaded(address.Region)) {
        return 0.0f;
    }

    CPathNode* node = &m_pPathNodes[address.Region][address.Index];
    if (node->NumberAdjNodes == 0) {
        return 0.0f;
    }

    int32 nodeToUse = 0;
    for (; nodeToUse < (int32)node->NumberAdjNodes - 1; nodeToUse++) {
        CCarPathLinkAddress linkAddr = pAdjacentLinks[address.Region][node->IndexAdjacentNodes + nodeToUse];
        if (IsAreaLoaded(linkAddr.Region)) {
            const CCarPathLink& link = m_pNaviNodes[linkAddr.Region][linkAddr.m_wCarPathLinkId];
            if (link.Node1 == address) {
                if (link.LanesTo != 0) {
                    break;
                }
            } else {
                if (link.LanesFro != 0) {
                    break;
                }
            }
        }
    }

    CNodeAddress adjAddr = pAdjacentNodes[address.Region][node->IndexAdjacentNodes + nodeToUse];
    if (!adjAddr.IsValid() || !IsAreaLoaded(adjAddr.Region)) {
        return 0.0f;
    }

    CPathNode* adjNode = &m_pPathNodes[adjAddr.Region][adjAddr.Index];

    CVector diff = adjNode->GetPosition() - node->GetPosition();
    diff.z = 0.0f;
    diff.Normalise();

    return std::atan2(-diff.x, diff.y) * 57.295776f;
}

// 0x452160
void CPathFind::SwitchOffNodeAndNeighbours(CPathNode* node, CPathNode*& outNext1, CPathNode** outNext2, bool bWhatToSwitchTo, bool bBackToOriginal) {
    node->SwitchedOff = bBackToOriginal ? node->SwitchedOffOriginal : bWhatToSwitchTo;
   
    outNext1 = nullptr;
    if (outNext2) {
        *outNext2 = nullptr;
    }

    if (CountNeighboursToBeSwitchedOff(node) > 2) {
        return;
    }

    for (auto& linked : GetNodeLinkedNodes(*node)) {
        if (!linked.HasToBeSwitchedOff()) {
            continue;
        }
        if (linked.SwitchedOff == bWhatToSwitchTo) {
            continue;
        }
        if (CountNeighboursToBeSwitchedOff(&linked) > 2) {
            continue;
        }
        if (!outNext1) {
            outNext1 = &linked;
        }
#ifdef FIX_BUGS // Above it was checked whenever it's set so I assume this was a bug
        else if (outNext2) // Don't get confused, `outNext2` is a ptr to a ptr
#else
        else
#endif
        {
            *outNext2 = &linked;
        }
    }
}

// 0x452270 helpers
static void MakeSureLinkExists(int8 Node, int8 LinkedTo) {
    for (int32 C = 0; C < 6; C++) {
        if (ConnectsToGiven[LinkedTo][C] == Node) {
            return;
        }
    }
    for (int32 C = 0; C < 6; C++) {
        if (ConnectsToGiven[LinkedTo][C] < 0) {
            ConnectsToGiven[LinkedTo][C] = Node;
            return;
        }
    }
}

static void SetOneAdjacentNodeForThisNode(int32 Region, int8 Node, int32 AdjRegion, int32 AdjNode, int32 FreeSlot, int32& CurrentAdjacentNode) {
    ThePaths.pAdjacentNodes[FreeSlot + 64][CurrentAdjacentNode] = CNodeAddress((uint16)AdjRegion, (uint16)AdjNode);
    float Length = (ThePaths.m_pPathNodes[Region][Node].GetPosition() - ThePaths.m_pPathNodes[AdjRegion][AdjNode].GetPosition()).Magnitude();
    if (Length > 255.0f) {
        Length = 255.0f;
    }
    if (static_cast<uint8>(Length) < 1) {
        Length = 1.0f;
    }
    ThePaths.m_pLinkLengths[FreeSlot + 64][CurrentAdjacentNode] = static_cast<uint8>(Length);
    ThePaths.m_pPathIntersections[FreeSlot + 64][CurrentAdjacentNode].Clear();
    ThePaths.m_pPathNodes[FreeSlot + 64][Node].NumberAdjNodes++;
    CurrentAdjacentNode++;
}

// 0x452270
void CPathFind::CompleteNewInterior(CNodeAddress* outAddress) {
    if (outAddress) {
        *outAddress = EmptyNodeAddress;
    }

    if (NumNodesGiven == 0) {
        bInteriorBeingBuilt = false;
        return;
    }

    uint8 InteriorGroup = (uint8)(NewInteriorSlot + 100);
    if (NumLinksToExteriorNodes > 0) {
        CPathNode* pNode = GetPathNode(aExteriorNodeLinkedTo[0]);
        InteriorGroup = pNode->Group;
    }

    m_interiorIDs[NewInteriorSlot] = InteriorIDBeingBuilt;
    m_pPathNodes[NewInteriorSlot + 64] = (CPathNode*)CMemoryMgr::Malloc(NumNodesGiven * sizeof(CPathNode));

    for (int32 Node = 0; Node < (int32)NumNodesGiven; Node++) {
        auto& node = m_pPathNodes[NewInteriorSlot + 64][Node];
        node.m_vPos = CompressedLargeVector{ CVector{ XCoorGiven[Node], YCoorGiven[Node], ZCoorGiven[Node] } };
        node.Region = (uint16)(NewInteriorSlot + 64);
        node.Index = (uint16)Node;
        node.Width = 0;
        node.Group = InteriorGroup;
        node.OnDeadEnd = 0;
        node.SwitchedOff = 1;
        node.SwitchedOffOriginal = 1;
        node.RoadBlock = 0;
        node.WaterNode = 0;
        node.unk1 = 0;
        node.DontWanderHere = DontWanderGiven[Node];
        node.unk2 = 1;
        node.Speed = 0;
        node.unk3 = 0;
        node.unk4 = 0;
        node.Density = 15;
        node.SpecialFunction = 0;
        node.DistanceToTarget = 32766;
    }

    for (int32 Node = 0; Node < (int32)NumNodesGiven; Node++) {
        for (int32 C = 0; C < 6; C++) {
            if (ConnectsToGiven[Node][C] >= 0) {
                MakeSureLinkExists((int8)Node, ConnectsToGiven[Node][C]);
            }
        }
    }

    int32 TotalAdjacentNodes = 0;
    for (int32 Node = 0; Node < (int32)NumNodesGiven; Node++) {
        for (int32 C = 0; C < 6; C++) {
            if (ConnectsToGiven[Node][C] >= 0) {
                TotalAdjacentNodes++;
            }
        }
    }

    pAdjacentNodes[NewInteriorSlot + 64] = (CNodeAddress*)CMemoryMgr::Malloc((TotalAdjacentNodes + 192) * sizeof(CNodeAddress));
    m_pLinkLengths[NewInteriorSlot + 64] = (uint8*)CMemoryMgr::Malloc(TotalAdjacentNodes + 192);
    m_pPathIntersections[NewInteriorSlot + 64] = (CPathIntersectionInfo*)CMemoryMgr::Malloc(TotalAdjacentNodes + 192);

    int32 CurrentAdjacentNode = 0;
    for (int32 Node = 0; Node < (int32)NumNodesGiven; Node++) {
        m_pPathNodes[NewInteriorSlot + 64][Node].IndexAdjacentNodes = (int16)CurrentAdjacentNode;
        m_pPathNodes[NewInteriorSlot + 64][Node].NumberAdjNodes = 0;

        for (int32 C = 0; C < 6; C++) {
            if (ConnectsToGiven[Node][C] >= 0) {
                SetOneAdjacentNodeForThisNode(NewInteriorSlot + 64, (int8)Node, NewInteriorSlot + 64, ConnectsToGiven[Node][C], NewInteriorSlot, CurrentAdjacentNode);
            }
        }
    }

    for (int32 C = TotalAdjacentNodes; C < TotalAdjacentNodes + 192; C++) {
        pAdjacentNodes[NewInteriorSlot + 64][C] = EmptyNodeAddress;
    }

    m_anNumNodes[NewInteriorSlot + 64] = NumNodesGiven;
    m_anNumVehicleNodes[NewInteriorSlot + 64] = 0;
    m_anNumPedNodes[NewInteriorSlot + 64] = NumNodesGiven;
    m_anNumCarPathLinks[NewInteriorSlot + 64] = 0;
    m_anNumAddresses[NewInteriorSlot + 64] = TotalAdjacentNodes;

    for (int32 C = 0; C < NumLinksToExteriorNodes; C++) {
        CNodeAddress Temp{ (uint16)(NewInteriorSlot + 64), (uint16)aInteriorNodeLinkedToExterior[C] };
        AddDynamicLinkBetween2Nodes(Temp, aExteriorNodeLinkedTo[C]);
    }

    bInteriorBeingBuilt = false;
}

// 0x44DF30
void CPathFind::AddInteriorLinkToExternalNode(int32 interiorNodeIdx, CNodeAddress externalNodeAddr) {
    const auto idx = NumLinksToExteriorNodes++;
    aInteriorNodeLinkedToExterior[idx] = interiorNodeIdx;
    aExteriorNodeLinkedTo[idx] = externalNodeAddr;
}

// 0x44E1A0
void CPathFind::RemoveInterior(uint32 intId) {
    for (auto intSlot = 0u; intSlot < NUM_PATH_INTERIOR_AREAS; intSlot++) {
        const auto intSlotAreaId = NUM_PATH_MAP_AREAS + intSlot;

        if (m_interiorIDs[intSlot] != intId) {
            continue;
        }

        for (auto areaId = 0u; areaId < NUM_TOTAL_PATH_NODE_AREAS; areaId++) {
            for (auto& node : GetPathNodesInArea(areaId, PATH_TYPE_PED)) {
                // I assume this checks if the link is an interior link?
                if ((int32)node.IndexAdjacentNodes < (int32)m_anNumAddresses[areaId]) {
                    continue;
                }

                // Remove all link of this node that point to a node int the current interior
                bool foundNodeFromOtherInt{}, foundNodeFromThisInt{};
                (void)rng::remove_if(GetNodeLinkedNodes(node, false), [&](CPathNode& linkedNode) {
                    if (linkedNode.Region == intSlotAreaId) {
                        node.NumberAdjNodes--;
                        foundNodeFromOtherInt = true;
                        return true;
                    } else if (linkedNode.Region >= NUM_PATH_MAP_AREAS) {
                        foundNodeFromThisInt = true;
                    }
                    return false;
                });

                // If we found a linked node and there was no other node from another interior
                if (foundNodeFromOtherInt || !foundNodeFromThisInt) {
                    continue;
                }

                // TODO: Magic number `12` (Search: MAGIC_NUM_12)
                // Null out all links of this node
                rng::fill(std::span{ &pAdjacentNodes[node.Region][node.IndexAdjacentNodes], 12 }, CNodeAddress{});

                // Delete dynamic link of this area
                // Honestly, this doesn't make much sense... As in, I don't think these are dynamic areas? We'll see.. TODO
                const auto& dynLinks  = m_aDynamicLinksIds[intSlot];
                const auto  dynLinkIt = rng::find(dynLinks, (int32)node.IndexAdjacentNodes);
                if (dynLinkIt != rng::end(dynLinks)) {
                    const auto dynLinkIdx = rng::distance(rng::begin(dynLinks), dynLinkIt);
                    node.IndexAdjacentNodes = m_aDynamicLinksBaseIds[intSlot][dynLinkIdx];
                    m_aDynamicLinksBaseIds[intSlot][dynLinkIdx] = -1;
                    m_aDynamicLinksIds[intSlot][dynLinkIdx] = -1;
                }
            }
        }

        // Finally, unload area and related data
        const auto FreeAndNull = [](auto& ptr) {
            CMemoryMgr::Free(ptr);
            ptr = nullptr;
        };
        FreeAndNull(m_pPathIntersections[intSlotAreaId]);
        FreeAndNull(m_pLinkLengths[intSlotAreaId]);
        FreeAndNull(m_pPathNodes[intSlotAreaId]);
        FreeAndNull(m_pLinkLengths[intSlotAreaId]);

        m_anNumAddresses[intSlotAreaId] = 0;
        m_anNumCarPathLinks[intSlotAreaId] = 0;
        m_anNumPedNodes[intSlotAreaId] = 0;
        m_anNumVehicleNodes[intSlotAreaId] = 0;
        m_anNumNodes[intSlotAreaId] = 0;
    }
}

// 0x44D930
void CPathFind::FindStartPointOfRegion(size_t x, size_t y, float& outX, float& outY) {
    const auto pos = FindStartPointOfRegion(x, y);
    outX = pos.x;
    outY = pos.y;
}

// notsa
CVector2D CPathFind::FindStartPointOfRegion(size_t x, size_t y) {
    return { FindXCoorsForRegion(x), FindYCoorsForRegion(y) };
}

namespace detail {
constexpr auto GetCoorsOfRegion(size_t p, size_t nareas) {
    return (6000.f / (float)nareas) * (float)p - 3000.f;
}
};

// 0x44D8F0
float CPathFind::FindXCoorsForRegion(size_t x) {
    return detail::GetCoorsOfRegion(x, NUM_PATH_MAP_AREA_X);
}

// 0x44D910
float CPathFind::FindYCoorsForRegion(size_t y) {
    return detail::GetCoorsOfRegion(y, NUM_PATH_MAP_AREA_Y);
}

// 0x44DED0
void CPathFind::AddInteriorLink(int32 intNodeA, int32 intNodeB) {
    const auto AddLink = [](int32 intIdx, int32 linkTo) {
        const auto it = rng::find(ConnectsToGiven[intIdx], -1); 
        assert(!(*it >= 0)); // NOTE: Original code did a `while (*it >= 0), if anything goes bad use `>= 0` for `rng::find`
        *it = linkTo;
    };
    AddLink(intNodeA, intNodeB);
    AddLink(intNodeB, intNodeA);
}

// notsa
std::span<CPathNode> CPathFind::GetPathNodesInArea(size_t areaId, ePathType ptype) const {
    if (const auto allNodes = m_pPathNodes[areaId]) {
        const auto numVehNodes = m_anNumVehicleNodes[areaId];
        switch (ptype) {
        case ePathType::PATH_TYPE_VEH: // Vehicles, boats, race tracks
            return std::span{ allNodes, m_anNumVehicleNodes[areaId] };
        case ePathType::PATH_TYPE_PED: // Peds only
            assert(m_anNumPedNodes[areaId] == m_anNumNodes[areaId] - numVehNodes); // Pirulax: I'm assuming this is true, so if this doesnt assert for a long time remove it
            return std::span{ allNodes + numVehNodes, m_anNumPedNodes[areaId] };
        case ePathType::PATH_TYPE_ALL: // All of the above
            return std::span{ allNodes, m_anNumNodes[areaId] };
        default:
            NOTSA_UNREACHABLE("Invalid pathType: {}", (int)ptype);
        }
    }
    return {}; // Area not loaded, return nothing.. Perhaps assert here instead?
}

// 0x44D1B0
void CPathFind::RemoveNodeFromList(CPathNode* node) {
    node->m_prev->m_next = node->m_next;
    if (node->m_next) {
        node->m_next->m_prev = node->m_prev;
    }

    m_totalNumNodesInPathFindHashTable--;
}

void CPathFind::AddNodeToList(CPathNode* node, int32 distFromOrigin) {
    // Insert the node as the head into it's bucket

    auto& head = m_pathFindHashTable[distFromOrigin % std::size(m_pathFindHashTable)];

    node->m_next = head;

    // Make this node's `next` point to the head in the hash table
    // I guess this works as long as you only access the `m_prev` variable
    // as that's at offset 0
    // This is a really bad hack to avoid having to do special handling
    // for the head in `RemoveNodeFromList`...
    node->m_prev = reinterpret_cast<CPathNode*>(&head);

    if (head) {
        head->m_prev = node;
    }

    head = node;

    assert(distFromOrigin <= std::numeric_limits<decltype(node->DistanceToTarget)>::max()); // Prevent bugs from overflow
    node->DistanceToTarget = (int16)distFromOrigin;

    m_totalNumNodesInPathFindHashTable++;
}

// 0x452C80
void CPathFind::SwitchRoadsOffInArea(float xMin, float xMax, float yMin, float yMax, float zMin, float zMax, bool bSwitchOff, bool bCars, bool bBackToOriginal) {
    for (auto areaId = 0u; areaId < NUM_PATH_MAP_AREAS; ++areaId) {
        SwitchRoadsOffInAreaForOneRegion(xMin, xMax, yMin, yMax, zMin, zMax, bSwitchOff, bCars, areaId, bBackToOriginal);
    }

    for (auto i = 0u; i < m_nNumNodeSwitches; ++i) {
        auto* pArea = &m_aNodeSwitches[i];

        if (notsa::bugfixes::CPathFind_SwitchRoadsOffInArea_StrayAreas) {
            // some missions create both types of switches at the same area, so we store them separately
            if (pArea->isCars != bCars) {
                continue;
            }

            // avoid creating stray areas, potentially leaving no space for important areas later in game
            // ideally, the script would use SWITCH_ROADS_BACK_TO_ORIGINAL or SWITCH_PED_ROADS_BACK_TO_ORIGINAL but that's not always the case
            // so we consider toggling the same area as "back to original"
            if (pArea->xMin == xMin && pArea->yMin == yMin && pArea->zMin == zMin && pArea->xMax == xMax && pArea->yMax == yMax && pArea->zMax == zMax && pArea->isOff != bSwitchOff) {
                bBackToOriginal = true;
            }
        }

        // If the existing area is completely inside the area we are switching off, remove it
        if (pArea->xMin < xMin || pArea->yMin < yMin || pArea->zMin < zMin || pArea->xMax > xMax || pArea->yMax > yMax || pArea->zMax > zMax) {
            continue;
        }

        for (auto j = i; j < m_nNumNodeSwitches - 1; ++j) {
            if (notsa::bugfixes::CPathFind_SwitchRoadsOffInArea_StrayAreas) {
                m_aNodeSwitches[j] = m_aNodeSwitches[j + 1];
            } else {
                // R* bug, they messed up with the index
                m_aNodeSwitches[i] = m_aNodeSwitches[i + 1];
            }
        }

        --m_nNumNodeSwitches;
        --i;
    }

    if (!bBackToOriginal && m_nNumNodeSwitches < NUM_PATH_MAP_AREAS) {
        auto& area  = m_aNodeSwitches[m_nNumNodeSwitches];
        area.xMin   = xMin;
        area.xMax   = xMax;
        area.yMin   = yMin;
        area.yMax   = yMax;
        area.zMin   = zMin;
        area.zMax   = zMax;
        area.isOff  = bSwitchOff;
        area.isCars = bCars;
        m_nNumNodeSwitches++;
    }
}

// 0x452F00
void CPathFind::SwitchPedRoadsOffInArea(float xMin, float xMax, float yMin, float yMax, float zMin, float zMax, bool bSwitchOff, bool bBackToOriginal) {
    SwitchRoadsOffInArea(xMin, xMax, yMin, yMax, zMin, zMax, bSwitchOff, false, bBackToOriginal);
}

// Inlined in retail (no address)
// AI Stub - checked against the Android binary (libGTASA)
void CPathFind::RemoveBadStartNode(CVector pos, CNodeAddress* address, int16* numPathFindNodes) {
    if (*numPathFindNodes < 2) {
        return;
    }
    if (!IsRegionLoaded(address[0]) || !IsRegionLoaded(address[1])) {
        return;
    }

    const CPathNode* pNode0 = FindNodePointer(address[0]);
    const CPathNode* pNode1 = FindNodePointer(address[1]);

    const float DeltaX1 = pNode0->GetCoors().x - pos.x;
    const float DeltaY1 = pNode0->GetCoors().y - pos.y;
    const float DeltaX2 = pNode1->GetCoors().x - pos.x;
    const float DeltaY2 = pNode1->GetCoors().y - pos.y;

    // The first node is behind us (relative to the second one) - drop it
    if (DeltaX1 * DeltaX2 + DeltaY1 * DeltaY2 < 0.0f) {
        (*numPathFindNodes)--;
        for (int16 C = 0; C < *numPathFindNodes; C++) {
            address[C] = address[C + 1];
        }
    }
}

// 0x44DF60
// AI Stub
void CPathFind::RemoveInteriorLinks(uint32 intIdx) {
    for (int32 TestNode = 0; TestNode < (int32)NumNodesGiven; TestNode++) {
        for (int32 Link = 0; Link < 6; Link++) {
            if (TestNode == (int32)intIdx || ConnectsToGiven[TestNode][Link] == (int8)intIdx) {
                ConnectsToGiven[TestNode][Link] = -1;
            }
        }
    }
}
