#include "StdInc.h"
#include "InteriorManager_c.h"
#include "Interior_c.h"
#include "Tasks/TaskTypes/Interior/TaskInteriorBeInHouse.h"
#include "Tasks/TaskTypes/Interior/TaskInteriorBeInOffice.h"
#include "Tasks/TaskTypes/Interior/TaskInteriorBeInShop.h"
#include "Tasks/TaskTypes/Interior/TaskInteriorShopKeeper.h"
#include "InteriorGroup_c.h"

void InteriorGroup_c::InjectHooks() {
    RH_ScopedClass(InteriorGroup_c);
    RH_ScopedCategory("Interior");

    //RH_ScopedInstall(Constructor, 0x597FE0, { .reversed = false });
    //RH_ScopedInstall(Destructor, 0x597FF0, { .reversed = false });

    RH_ScopedInstall(Init, 0x5947E0);
    RH_ScopedInstall(Update, 0x5968E0);
    RH_ScopedInstall(SetupPeds, 0x596890);
    RH_ScopedInstall(UpdatePeds, 0x596830);
    RH_ScopedInstall(SetupHousePeds, 0x5965E0);
    RH_ScopedInstall(SetupPaths, 0x595590);
    RH_ScopedInstall(ArePathsLoaded, 0x595380);
    RH_ScopedInstall(Setup, 0x595320);
    RH_ScopedInstall(Exit, 0x595290);
    RH_ScopedInstall(ContainsInteriorType, 0x595250);
    RH_ScopedInstall(CalcIsVisible, 0x595200);
    RH_ScopedInstall(DereferenceAnims, 0x595160);
    RH_ScopedInstall(ReferenceAnims, 0x5950D0);
    RH_ScopedInstall(UpdateOfficePeds, 0x594E90);
    RH_ScopedInstall(RemovePed, 0x594E30);
    RH_ScopedInstall(SetupShopPeds, 0x594C10);
    RH_ScopedInstall(SetupOfficePeds, 0x594BF0);
    RH_ScopedInstall(GetEntity, 0x594BD0);
    RH_ScopedInstall(GetPed, 0x594B90);
    RH_ScopedInstall(FindClosestInteriorInfo, 0x594A50);
    RH_ScopedInstall(FindInteriorInfo, 0x594970);
    RH_ScopedInstall(GetNumInteriorInfos, 0x594920);
    RH_ScopedInstall(GetRandomInterior, 0x5948C0);
    RH_ScopedInstall(AddInterior, 0x594840);
}

// 0x5947E0
// ASM Match
void InteriorGroup_c::Init(CEntity* pEntity, int32 groupId)
{
    for (int32 i = 0; i < 8; i++)
    {
        m_pInteriors[i] = nullptr;
    }

    int32 i;
    for (i = 0; i < 16; i++)
    {
        m_pPeds[i] = nullptr;
        m_pPedsToRemove[i] = nullptr;
    }

    m_numInteriors = 0;
    m_pathsSetup = false;
    m_pedsSetup = false;
    m_isVisible = false;
    m_lastIsVisible = false;
    m_animsReferenced = false;
    m_pEntity = pEntity;
    m_groupId = groupId;
}

// 0x5968E0
// ASM Match: 99.8%
void InteriorGroup_c::Update()
{
    CalcIsVisible();

    if (!m_pathsSetup)
    {
        SetupPaths();
    }

    if (m_pathsSetup && !m_pedsSetup)
    {
        SetupPeds();
    }

    if (m_pedsSetup)
    {
        UpdatePeds();
    }

    ReferenceAnims();
}

// 0x594840
// ASM Match
void InteriorGroup_c::AddInterior(Interior_c* pInterior)
{
    for (int32 i = 0; i < 8; i++)
    {
        if (m_pInteriors[i] == nullptr)
        {
            m_pInteriors[i] = pInterior;
            m_numInteriors++;
            return;
        }
    }
}

// 0x596890
// ASM Match: 99.8%
void InteriorGroup_c::SetupPeds()
{
    if (m_pEntryExit && g_interiorMan.m_activePeds)
    {
        if (m_groupType == 0)
        {
            SetupHousePeds();
        }
        else if (m_groupType == 1)
        {
            SetupShopPeds();
        }
        else if (m_groupType == 2)
        {
            SetupOfficePeds();
        }
        m_pedsSetup = true;
    }
}

// 0x596830
// ASM Match: 94.7%
void InteriorGroup_c::UpdatePeds()
{
    if (m_pEntryExit && g_interiorMan.m_activePeds)
    {
        for (int32 i = 0; i < 16; i++)
        {
            if (m_pPedsToRemove[i])
            {
                if (m_pPedsToRemove[i]->IsPointerValid())
                {
                    RemovePed(m_pPedsToRemove[i]);
                }
                m_pPedsToRemove[i] = nullptr;
            }
        }

        if (m_groupType == 0)
        {
            UpdateHousePeds();
        }
        else if (m_groupType == 1)
        {
            UpdateShopPeds();
        }
        else if (m_groupType == 2)
        {
            UpdateOfficePeds();
        }
    }
}

// 0x5965E0
// ASM Match: 87.6%
void InteriorGroup_c::SetupHousePeds()
{
    CStreaming::StreamPedsForInterior(0);
    CStreaming::LoadAllRequestedModels(false);
    m_numPeds = 0;

    CVector pedPos(0.0f, 0.0f, 0.0f);
    pedPos = m_pEntity->GetMatrix() * pedPos;

    int32 numPedsToCreate;
    int32 randNum = CGeneral::GetRandomNumberInRange(0, 100);
    ePedType gangPedType;
    bool useGangGuys = false;

    numPedsToCreate = (randNum <= 50) ? 2 : 1;

    if (CPopCycle::m_pCurrZoneInfo)
    {
        gangPedType = CPopCycle::PickGangToCreateMembersOf();
        if (gangPedType != PED_TYPE_PLAYER1 && CPopulation::ChooseGangOccupation((eGangID)(gangPedType - PED_TYPE_GANG1)) > -1)
        {
            useGangGuys = true;
            if (CGeneral::GetRandomNumberInRange(0, 100) <= 15)
            {
                numPedsToCreate = CGeneral::GetRandomTrueFalse() + 3;
            }
            else
            {
                useGangGuys = false;
            }
        }
    }

    for (int32 i = 0; i < numPedsToCreate; i++)
    {
        ePedType pedType;
        uint32 pedOccupation;

        if (useGangGuys)
        {
            pedOccupation = CPopulation::ChooseGangOccupation((eGangID)(gangPedType - PED_TYPE_GANG1));
        }
        else if (numPedsToCreate == 1)
        {
            if (CGeneral::GetRandomTrueFalse())
            {
                pedOccupation = CStreaming::FindMIPedSlotForInterior(1);
            }
            else
            {
                pedOccupation = CStreaming::FindMIPedSlotForInterior(0);
            }
        }
        else
        {
            pedOccupation = CStreaming::FindMIPedSlotForInterior(i);
        }

        pedType = ((CPedModelInfo*)CModelInfo::GetModelInfo(pedOccupation))->GetDefaultPedType();

        {
            int32 pedId = m_numPeds;
            m_pPeds[pedId] = CPopulation::AddPed((ePedType)pedType, (eModelID)pedOccupation, pedPos, false);
            if (m_pPeds[pedId])
            {
                m_numPeds++;
                CTheScripts::ScriptsForBrains.StartOrRequestNewStreamedScriptBrainWithThisName("house", m_pPeds[pedId], 3);
                m_pPeds[pedId]->SetCharCreatedBy(PED_MISSION);
                m_pPeds[pedId]->GetPedIntelligence()->SetPedDecisionMakerType(7);
                m_pPeds[pedId]->GetPedIntelligence()->AddTaskDefault(new CTaskInteriorBeInHouse(this));
            }

            if (!g_interiorMan.m_pedsAlive[i])
            {
                RemovePed(m_pPeds[pedId]);
            }
        }
    }
}

// 0x595590
// ASM Match: 74.0%
void InteriorGroup_c::SetupPaths()
{
    if (!ArePathsLoaded())
    {
        return;
    }

    int32 nodeOffset = 0;
    int32 nodeOffsets[8];
    int32 currNode = 0;

    ThePaths.StartNewInterior(m_id);

    for (int32 i = 0; i < 8; i++)
    {
        Interior_c* pInterior = m_pInteriors[i];
        if (pInterior)
        {
            nodeOffsets[i] = nodeOffset;

            if (pInterior->m_numGotoPts > 2)
            {
                ThePaths.AddNodeToNewInterior(pInterior->m_gotoPts[0].pos.x, pInterior->m_gotoPts[0].pos.y, pInterior->m_gotoPts[0].pos.z, false, -1, -1, -1, -1, -1, -1);
                currNode++;
                ThePaths.AddNodeToNewInterior(pInterior->m_gotoPts[1].pos.x, pInterior->m_gotoPts[1].pos.y, pInterior->m_gotoPts[1].pos.z, false, -1, -1, -1, -1, -1, -1);
                ThePaths.AddInteriorLink(currNode, nodeOffset);
                currNode++;

                for (int32 j = 2; j < pInterior->m_numGotoPts; j++)
                {
                    ThePaths.AddNodeToNewInterior(pInterior->m_gotoPts[j].pos.x, pInterior->m_gotoPts[j].pos.y, pInterior->m_gotoPts[j].pos.z, false, -1, -1, -1, -1, -1, -1);
                    ThePaths.AddInteriorLink(currNode, nodeOffset + j - 2);
                    if (j % 2)
                    {
                        ThePaths.AddInteriorLink(currNode, nodeOffset + j - 1);
                    }
                    currNode++;
                }
            }

            nodeOffset += pInterior->m_numGotoPts;
        }
    }

    int32 numNodes = 0;
    int32 nodeIds[16];
    RwV3d nodePositions[16];
    bool nodeJoined[16];

    for (int32 i = 0; i < 8; i++)
    {
        int32 leftNode, mainNode, topNode, rightNode;
        Interior_c* pInterior = m_pInteriors[i];
        if (pInterior == nullptr)
        {
            continue;
        }

        bool setupDoorNode = false;

        if (pInterior->m_box->m_door > -1)
        {
            CNodeAddress doorAddr = ThePaths.AddNodeToNewInterior(pInterior->m_exitPts[0].pos.x, pInterior->m_exitPts[0].pos.y, pInterior->m_exitPts[0].pos.z, false, -1, -1, -1, -1, -1, -1);
            if (pInterior->m_exitPts[0].link1 > -1)
            {
                ThePaths.AddInteriorLink(currNode, nodeOffsets[i] + pInterior->m_exitPts[0].link1);
            }
            if (pInterior->m_exitPts[0].link2 > -1)
            {
                ThePaths.AddInteriorLink(currNode, nodeOffsets[i] + pInterior->m_exitPts[0].link2);
            }
            mainNode = currNode;
            pInterior->m_doorPos = pInterior->m_exitPts[0].pos;
            pInterior->m_doorAddr = doorAddr;
            setupDoorNode = true;

            ThePaths.AddNodeToNewInterior(pInterior->m_exitPts[1].pos.x, pInterior->m_exitPts[1].pos.y, pInterior->m_exitPts[1].pos.z, m_groupType != 0, -1, -1, -1, -1, -1, -1);
            ThePaths.AddInteriorLink(currNode + 1, currNode);

            CNodeAddress exitNode = ThePaths.FindNearestExteriorNodeToInteriorNode(currNode + 1);
            if (exitNode.IsEmpty())
            {
                nodeIds[numNodes] = currNode + 1;
                nodePositions[numNodes] = pInterior->m_exitPts[1].pos;
                nodeJoined[numNodes] = false;
                numNodes++;
            }
            else
            {
                pInterior->m_exitAddr = exitNode;
                CVector exitPos;
                ThePaths.FindNodePointer(exitNode)->GetCoors(exitPos);
                pInterior->m_exitPos = exitPos;
                if (m_groupType == 0)
                {
                    CVector pos1;
                    ThePaths.FindNodePointer(exitNode)->GetCoors(pos1);
                    CVector pos2 = pInterior->m_exitPts[1].pos;
                    CVector pos12 = pos1 - pos2;
                    if (pos12.MagnitudeSqr() <= 9.0f)
                    {
                        ThePaths.AddInteriorLinkToExternalNode(currNode + 1, exitNode);
                    }
                }
            }
            currNode += 2;
        }

        if (pInterior->m_box->m_lDoorStart > -1)
        {
            CNodeAddress doorAddr = ThePaths.AddNodeToNewInterior(pInterior->m_exitPts[2].pos.x, pInterior->m_exitPts[2].pos.y, pInterior->m_exitPts[2].pos.z, false, -1, -1, -1, -1, -1, -1);
            if (pInterior->m_exitPts[2].link1 != -1)
            {
                ThePaths.AddInteriorLink(currNode, nodeOffsets[i] + pInterior->m_exitPts[2].link1);
            }
            if (pInterior->m_exitPts[2].link2 != -1)
            {
                ThePaths.AddInteriorLink(currNode, nodeOffsets[i] + pInterior->m_exitPts[2].link2);
            }
            leftNode = currNode;
            if (!setupDoorNode)
            {
                pInterior->m_doorPos = pInterior->m_exitPts[2].pos;
                pInterior->m_doorAddr = doorAddr;
                setupDoorNode = true;
            }

            ThePaths.AddNodeToNewInterior(pInterior->m_exitPts[3].pos.x, pInterior->m_exitPts[3].pos.y, pInterior->m_exitPts[3].pos.z, false, -1, -1, -1, -1, -1, -1);
            ThePaths.AddInteriorLink(currNode + 1, currNode);

            CNodeAddress exitNode = ThePaths.FindNearestExteriorNodeToInteriorNode(currNode + 1);
            if (exitNode.IsEmpty())
            {
                nodeIds[numNodes] = currNode + 1;
                nodePositions[numNodes] = pInterior->m_exitPts[3].pos;
                nodeJoined[numNodes] = false;
                numNodes++;
            }
            else
            {
                pInterior->m_exitAddr = exitNode;
                CVector exitPos;
                ThePaths.FindNodePointer(exitNode)->GetCoors(exitPos);
                pInterior->m_exitPos = exitPos;
                if (m_groupType == 0)
                {
                    CVector pos1;
                    ThePaths.FindNodePointer(exitNode)->GetCoors(pos1);
                    CVector pos2 = pInterior->m_exitPts[3].pos;
                    CVector pos12 = pos1 - pos2;
                    if (pos12.MagnitudeSqr() <= 9.0f)
                    {
                        ThePaths.AddInteriorLinkToExternalNode(currNode + 1, exitNode);
                    }
                }
            }
            currNode += 2;
        }

        if (pInterior->m_box->m_tDoorStart > -1)
        {
            CNodeAddress doorAddr = ThePaths.AddNodeToNewInterior(pInterior->m_exitPts[4].pos.x, pInterior->m_exitPts[4].pos.y, pInterior->m_exitPts[4].pos.z, false, -1, -1, -1, -1, -1, -1);
            if (pInterior->m_exitPts[4].link1 != -1)
            {
                ThePaths.AddInteriorLink(currNode, nodeOffsets[i] + pInterior->m_exitPts[4].link1);
            }
            if (pInterior->m_exitPts[4].link2 != -1)
            {
                ThePaths.AddInteriorLink(currNode, nodeOffsets[i] + pInterior->m_exitPts[4].link2);
            }
            topNode = currNode;
            if (!setupDoorNode)
            {
                pInterior->m_doorPos = pInterior->m_exitPts[4].pos;
                pInterior->m_doorAddr = doorAddr;
                setupDoorNode = true;
            }

            ThePaths.AddNodeToNewInterior(pInterior->m_exitPts[5].pos.x, pInterior->m_exitPts[5].pos.y, pInterior->m_exitPts[5].pos.z, false, -1, -1, -1, -1, -1, -1);
            ThePaths.AddInteriorLink(currNode + 1, currNode);

            CNodeAddress exitNode = ThePaths.FindNearestExteriorNodeToInteriorNode(currNode + 1);
            if (exitNode.IsEmpty())
            {
                nodeIds[numNodes] = currNode + 1;
                nodePositions[numNodes] = pInterior->m_exitPts[5].pos;
                nodeJoined[numNodes] = false;
                numNodes++;
            }
            else
            {
                pInterior->m_exitAddr = exitNode;
                CVector exitPos;
                ThePaths.FindNodePointer(exitNode)->GetCoors(exitPos);
                pInterior->m_exitPos = exitPos;
                if (m_groupType == 0)
                {
                    CVector pos1;
                    ThePaths.FindNodePointer(exitNode)->GetCoors(pos1);
                    CVector pos2 = pInterior->m_exitPts[5].pos;
                    CVector pos12 = pos1 - pos2;
                    if (pos12.MagnitudeSqr() <= 9.0f)
                    {
                        ThePaths.AddInteriorLinkToExternalNode(currNode + 1, exitNode);
                    }
                }
            }
            currNode += 2;
        }

        if (pInterior->m_box->m_rDoorStart > -1)
        {
            CNodeAddress doorAddr = ThePaths.AddNodeToNewInterior(pInterior->m_exitPts[6].pos.x, pInterior->m_exitPts[6].pos.y, pInterior->m_exitPts[6].pos.z, false, -1, -1, -1, -1, -1, -1);
            if (pInterior->m_exitPts[6].link1 != -1)
            {
                ThePaths.AddInteriorLink(currNode, nodeOffsets[i] + pInterior->m_exitPts[6].link1);
            }
            if (pInterior->m_exitPts[6].link2 != -1)
            {
                ThePaths.AddInteriorLink(currNode, nodeOffsets[i] + pInterior->m_exitPts[6].link2);
            }
            rightNode = currNode;
            if (!setupDoorNode)
            {
                pInterior->m_doorPos = pInterior->m_exitPts[6].pos;
                pInterior->m_doorAddr = doorAddr;
                setupDoorNode = true;
            }

            ThePaths.AddNodeToNewInterior(pInterior->m_exitPts[7].pos.x, pInterior->m_exitPts[7].pos.y, pInterior->m_exitPts[7].pos.z, false, -1, -1, -1, -1, -1, -1);
            ThePaths.AddInteriorLink(currNode + 1, currNode);

            CNodeAddress exitNode = ThePaths.FindNearestExteriorNodeToInteriorNode(currNode + 1);
            if (exitNode.IsEmpty())
            {
                nodeIds[numNodes] = currNode + 1;
                nodePositions[numNodes] = pInterior->m_exitPts[7].pos;
                nodeJoined[numNodes] = false;
                numNodes++;
            }
            else
            {
                pInterior->m_exitAddr = exitNode;
                CVector exitPos;
                ThePaths.FindNodePointer(exitNode)->GetCoors(exitPos);
                pInterior->m_exitPos = exitPos;
                if (m_groupType == 0)
                {
                    CVector pos1;
                    ThePaths.FindNodePointer(exitNode)->GetCoors(pos1);
                    CVector pos2 = pInterior->m_exitPts[7].pos;
                    CVector pos12 = pos1 - pos2;
                    if (pos12.MagnitudeSqr() <= 9.0f)
                    {
                        ThePaths.AddInteriorLinkToExternalNode(currNode + 1, exitNode);
                    }
                }
            }
            currNode += 2;
        }

        if (pInterior->m_numGotoPts == 0)
        {
            if (pInterior->m_box->m_door > -1)
            {
                if (pInterior->m_box->m_lDoorStart > -1)
                {
                    ThePaths.AddInteriorLink(mainNode, leftNode);
                }
                if (pInterior->m_box->m_tDoorStart > -1)
                {
                    ThePaths.AddInteriorLink(mainNode, topNode);
                }
                if (pInterior->m_box->m_rDoorStart > -1)
                {
                    ThePaths.AddInteriorLink(mainNode, rightNode);
                }
            }
            if (pInterior->m_box->m_tDoorStart > -1)
            {
                if (pInterior->m_box->m_lDoorStart > -1)
                {
                    ThePaths.AddInteriorLink(topNode, leftNode);
                }
                if (pInterior->m_box->m_rDoorStart > -1)
                {
                    ThePaths.AddInteriorLink(topNode, rightNode);
                }
            }
        }
    }

    for (int32 i = 0; i < numNodes; i++)
    {
        if (nodeJoined[i])
        {
            continue;
        }

        float closestDistSq = 1000000000.0f;
        int32 nodeToJoinTo = -1;
        for (int32 j = i + 1; j < numNodes; j++)
        {
            if (!nodeJoined[j])
            {
                RwV3d vec;
                vec.x = nodePositions[i].x - nodePositions[j].x;
                vec.y = nodePositions[i].y - nodePositions[j].y;
                vec.z = nodePositions[i].z - nodePositions[j].z;
                float distSq = vec.x * vec.x + vec.y * vec.y + vec.z * vec.z;
                if (distSq < closestDistSq)
                {
                    closestDistSq = distSq;
                    nodeToJoinTo = j;
                }
            }
        }

        if (closestDistSq < 3.0f)
        {
            nodeJoined[i] = true;
            nodeJoined[nodeToJoinTo] = true;
            ThePaths.AddInteriorLink(nodeIds[i], nodeIds[nodeToJoinTo]);
        }
        else
        {
            ThePaths.RemoveInteriorLinks(nodeIds[i]);
        }
    }

    ThePaths.CompleteNewInterior(nullptr);
    m_pathsSetup = true;
}

// 0x595380
// ASM Match: 88.0%
int8 InteriorGroup_c::ArePathsLoaded()
{
    CVector bbMinLcl = CModelInfo::GetBoundingBox(m_pEntity->GetModelIndex()).GetBoundBoxMin();
    CVector bbMaxLcl = CModelInfo::GetBoundingBox(m_pEntity->GetModelIndex()).GetBoundBoxMax();
    CVector corners[8];
    CVector bbMinWld;
    CVector bbMaxWld;

    corners[0].x = bbMinLcl.x;
    corners[0].y = bbMinLcl.y;
    corners[0].z = bbMinLcl.z;
    corners[1].x = bbMinLcl.x;
    corners[1].y = bbMaxLcl.y;
    corners[1].z = bbMinLcl.z;
    corners[2].x = bbMaxLcl.x;
    corners[2].y = bbMaxLcl.y;
    corners[2].z = bbMinLcl.z;
    corners[3].x = bbMaxLcl.x;
    corners[3].y = bbMinLcl.y;
    corners[3].z = bbMinLcl.z;
    corners[4].x = bbMinLcl.x;
    corners[4].y = bbMinLcl.y;
    corners[4].z = bbMaxLcl.z;
    corners[5].x = bbMinLcl.x;
    corners[5].y = bbMaxLcl.y;
    corners[5].z = bbMaxLcl.z;
    corners[6].x = bbMaxLcl.x;
    corners[6].y = bbMaxLcl.y;
    corners[6].z = bbMaxLcl.z;
    corners[7].x = bbMaxLcl.x;
    corners[7].y = bbMinLcl.y;
    corners[7].z = bbMaxLcl.z;

    bbMinWld.x = 999999.0f;
    bbMinWld.y = 999999.0f;
    bbMinWld.z = 999999.0f;
    bbMaxWld.x = -999999.0f;
    bbMaxWld.y = -999999.0f;
    bbMaxWld.z = -999999.0f;

    for (int32 i = 0; i < 8; i++)
    {
        corners[i] = m_pEntity->GetMatrix() * corners[i];

        if (corners[i].x < bbMinWld.x)
        {
            bbMinWld.x = corners[i].x;
        }
        if (corners[i].y < bbMinWld.y)
        {
            bbMinWld.y = corners[i].y;
        }
        if (corners[i].z < bbMinWld.z)
        {
            bbMinWld.z = corners[i].z;
        }
        if (corners[i].x > bbMaxWld.x)
        {
            bbMaxWld.x = corners[i].x;
        }
        if (corners[i].y > bbMaxWld.y)
        {
            bbMaxWld.y = corners[i].y;
        }
        if (corners[i].z > bbMaxWld.z)
        {
            bbMaxWld.z = corners[i].z;
        }
    }

    return ThePaths.AreNodesLoadedForArea(bbMinWld.x, bbMaxWld.x, bbMinWld.y, bbMaxWld.y);
}

// 0x595320
// ASM Match
void InteriorGroup_c::Setup()
{
    if (ContainsInteriorType(2))
    {
        m_groupType = 0;
    }
    else if (ContainsInteriorType(0) || ContainsInteriorType(6))
    {
        m_groupType = 1;
    }
    else if (ContainsInteriorType(1))
    {
        m_groupType = 2;
    }
    else
    {
        m_groupType = -1;
    }

    ReferenceAnims();
}

// 0x595290
// ASM Match: 99.6%
void InteriorGroup_c::Exit()
{
    for (int32 i = 0; i < 8; i++)
    {
        if (m_pInteriors[i])
        {
            m_pInteriors[i]->Exit();
            g_interiorMan.ReturnInteriorToPool(m_pInteriors[i]);
        }
    }

    for (int32 i = 0; i < 16; i++)
    {
        if (m_pPeds[i])
        {
            if (m_pPeds[i]->IsPointerValid())
            {
                if (!m_pPeds[i]->IsAlive())
                {
                    g_interiorMan.m_pedsAlive[i] = false;
                }
                CPopulation::RemovePed(m_pPeds[i]);
            }
            else
            {
                g_interiorMan.m_pedsAlive[i] = false;
            }
            m_pPeds[i] = nullptr;
        }
    }

    m_numPeds = 0;
    ThePaths.RemoveInterior(m_id);
    DereferenceAnims();
}

// 0x595250
// ASM Match: 79.3%
int8 InteriorGroup_c::ContainsInteriorType(int32 interiorType)
{
    for (int32 i = 0; i < 8; i++)
    {
        if (m_pInteriors[i] && m_pInteriors[i]->m_box->m_type == interiorType)
        {
            return true;
        }
    }

    return false;
}

// 0x595200
// ASM Match
void InteriorGroup_c::CalcIsVisible()
{
    m_lastIsVisible = m_isVisible;
    m_isVisible = false;

    for (int32 i = 0; i < m_numInteriors; i++)
    {
        if (m_pInteriors[i]->IsVisible())
        {
            m_isVisible = true;
            return;
        }
    }
}

// 0x595160
void InteriorGroup_c::DereferenceAnims() {
    if (!m_animsReferenced) {
        return;
    }
    CAnimManager::AddAnimBlockRef(CAnimManager::GetAnimationBlockIndex(GetAnimBlockName()));
    m_animsReferenced = false;
}

// 0x5950D0
void InteriorGroup_c::ReferenceAnims() {
    if (m_animsReferenced) {
        return;
    }
    const auto animBlkIdx = CAnimManager::GetAnimationBlockIndex(GetAnimBlockName());
    if (CStreaming::IsModelLoaded(IFPToModelId(animBlkIdx))) {
        CAnimManager::AddAnimBlockRef(animBlkIdx);
        m_animsReferenced = true;
    } else {
        CStreaming::RequestModel(IFPToModelId(animBlkIdx), STREAMING_KEEP_IN_MEMORY);
    }
}

// 0x594E90
// ASM Match: 96.0%
void InteriorGroup_c::UpdateOfficePeds()
{
    if (m_isVisible)
    {
        if (!m_lastIsVisible)
        {
            CVector pedPos(0.0f, 0.0f, 0.0f);
            pedPos = m_pEntity->GetMatrix() * pedPos;

            int32 numDeskInfos = GetNumInteriorInfos(6);
            int32 numPedsToCreate;

            if (CClock::GetIsTimeInRange(9, 18))
            {
                numPedsToCreate = CGeneral::GetRandomNumberInRange(numDeskInfos / 2, numDeskInfos);
            }
            else if (CClock::GetIsTimeInRange(18, 22) || CClock::GetIsTimeInRange(6, 9))
            {
                numPedsToCreate = CGeneral::GetRandomNumberInRange(0, numDeskInfos / 2);
            }
            else
            {
                numPedsToCreate = 0;
            }

            if (numPedsToCreate > 16)
            {
                numPedsToCreate = 16;
            }

            for (int32 i = 0; i < numPedsToCreate; i++)
            {
                ePedType pedType;
                uint32 pedOccupation = CStreaming::FindMIPedSlotForInterior(CGeneral::GetRandomNumberInRange(0, 8));
                pedType = ((CPedModelInfo*)CModelInfo::GetModelInfo(pedOccupation))->GetDefaultPedType();

                int32 pedId = m_numPeds;
                m_pPeds[pedId] = CPopulation::AddPed((ePedType)pedType, (eModelID)pedOccupation, pedPos, false);
                if (m_pPeds[pedId])
                {
                    m_numPeds++;
                    m_pPeds[pedId]->SetCharCreatedBy(PED_MISSION);
                    m_pPeds[pedId]->GetPedIntelligence()->SetPedDecisionMakerType(7);
                    m_pPeds[pedId]->GetPedIntelligence()->AddTaskDefault(new CTaskInteriorBeInOffice(this));
                }
            }
        }
    }
    else if (m_lastIsVisible)
    {
        for (int32 i = 0; i < 16; i++)
        {
            if (m_pPeds[i])
            {
                if (m_pPeds[i]->IsPointerValid())
                {
                    CPopulation::RemovePed(m_pPeds[i]);
                }
                m_pPeds[i] = nullptr;
            }
        }
        m_numPeds = 0;
    }
}

// 0x594E30
// ASM Match
void InteriorGroup_c::RemovePed(CPed* pPed)
{
    for (int32 i = 0; i < 16; i++)
    {
        if (m_pPeds[i] && m_pPeds[i] == pPed)
        {
            CPopulation::RemovePed(pPed);
            m_pPeds[i] = nullptr;
            m_numPeds--;
            return;
        }
    }
}

// 0x594C10
// ASM Match: 99.7%
void InteriorGroup_c::SetupShopPeds()
{
    CStreaming::StreamPedsForInterior(1);
    CStreaming::LoadAllRequestedModels(false);
    m_numPeds = 0;

    int32 numPedsToCreate = m_numInteriors * CGeneral::GetRandomNumberInRange(2, 5) + 1;

    for (int32 i = 0; i < numPedsToCreate; i++)
    {
        ePedType pedType;
        uint32 pedOccupation;

        if (i == 0)
        {
            pedOccupation = CStreaming::FindMIPedSlotForInterior(0);
        }
        else
        {
            pedOccupation = CStreaming::FindMIPedSlotForInterior(CGeneral::GetRandomNumberInRange(1, 8));
        }

        pedType = ((CPedModelInfo*)CModelInfo::GetModelInfo(pedOccupation))->GetDefaultPedType();

        int32 y, x;
        CVector pedPos(0.0f, 0.0f, 0.0f);
        Interior_c* pInterior = GetRandomInterior();
        pInterior->GetRandomTile((eTileStatus)3, x, y);
        pInterior->GetTileCentre((float)x, (float)y, pedPos);
        pedPos.z += 1.0f;

        int32 pedId = m_numPeds;
        m_pPeds[pedId] = CPopulation::AddPed((ePedType)pedType, (eModelID)pedOccupation, pedPos, false);
        if (m_pPeds[pedId])
        {
            m_numPeds++;
            m_pPeds[pedId]->SetCharCreatedBy(PED_MISSION);
            m_pPeds[pedId]->GetPedIntelligence()->SetPedDecisionMakerType(7);
            if (i == 0)
            {
                m_pPeds[pedId]->GetPedIntelligence()->AddTaskDefault(new CTaskInteriorShopKeeper(this, false));
            }
            else
            {
                m_pPeds[pedId]->GetPedIntelligence()->AddTaskDefault(new CTaskInteriorBeInShop(this));
            }
        }
    }
}

// 0x594BF0
// ASM Match
void InteriorGroup_c::SetupOfficePeds()
{
    CStreaming::StreamPedsForInterior(2);
    CStreaming::LoadAllRequestedModels(false);
    m_numPeds = 0;
}

// 0x594BD0
// ASM Match
CEntity* InteriorGroup_c::GetEntity()
{
    return m_pEntity;
}

// 0x594B90
// ASM Match
CPed* InteriorGroup_c::GetPed(int32 id)
{
    return m_pPeds[id];
}

// 0x594A50
// ASM Match: 88.9%
bool InteriorGroup_c::FindClosestInteriorInfo(int32 infoType, CVector pos, float range, InteriorInfo_t** ppInteriorInfo, Interior_c** ppInterior, float* distSq)
{
    InteriorInfo_t* pClosestInfo;
    Interior_c* pClosestInterior;
    float minDistSq;
    float rangeSq;

    pClosestInfo = nullptr;
    rangeSq = range * range;
    pClosestInterior = nullptr;
    minDistSq = 999999.0f;

    for (int32 j = 0; j < 8; j++)
    {
        if (m_pInteriors[j] && m_pInteriors[j]->IsPtInside(pos, CVector(0.0f, 0.0f, 0.0f)))
        {
            for (int32 k = 0; k < m_pInteriors[j]->m_numInteriorInfos; k++)
            {
                InteriorInfo_t* pInfo = &m_pInteriors[j]->m_interiorInfos[k];
                if ((infoType == -1 || (int32)(eInteriorInfoType)pInfo->type == infoType) && !pInfo->beingUsed)
                {
                    CVector vec = pos - CVector(pInfo->pos);
                    float thisDistSq = vec.MagnitudeSqr();
                    if (thisDistSq < rangeSq && thisDistSq < minDistSq)
                    {
                        minDistSq = thisDistSq;
                        pClosestInfo = pInfo;
                        pClosestInterior = m_pInteriors[j];
                    }
                }
            }
        }
    }

    if (pClosestInfo)
    {
        *ppInteriorInfo = pClosestInfo;
        *ppInterior = pClosestInterior;
        *distSq = minDistSq;
        return true;
    }

    return false;
}

// 0x594970
// ASM Match: 99.9%
bool InteriorGroup_c::FindInteriorInfo(eInteriorInfoType infoType, InteriorInfo_t** ppInteriorInfo, Interior_c** ppInterior)
{
    int32 numInfos;
    InteriorInfo_t* pInteriorInfos[64];
    Interior_c* pInteriors[64];

    numInfos = 0;
    for (int32 j = 0; j < 8; j++)
    {
        if (m_pInteriors[j])
        {
            for (int32 k = 0; k < m_pInteriors[j]->m_numInteriorInfos; k++)
            {
                if ((eInteriorInfoType)m_pInteriors[j]->m_interiorInfos[k].type == infoType && !m_pInteriors[j]->m_interiorInfos[k].beingUsed)
                {
                    pInteriorInfos[numInfos] = &m_pInteriors[j]->m_interiorInfos[k];
                    pInteriors[numInfos] = m_pInteriors[j];
                    numInfos++;
                }
            }
        }
    }

    if (numInfos > 0)
    {
        int32 randId = CGeneral::GetRandomNumberInRange(0, numInfos);
        *ppInteriorInfo = pInteriorInfos[randId];
        *ppInterior = pInteriors[randId];
        return true;
    }

    *ppInteriorInfo = nullptr;
    *ppInterior = nullptr;
    return false;
}

// 0x594920
// ASM Match
int32 InteriorGroup_c::GetNumInteriorInfos(int32 infoType)
{
    int32 numInfos = 0;

    for (int32 j = 0; j < 8; j++)
    {
        if (m_pInteriors[j])
        {
            for (int32 k = 0; k < m_pInteriors[j]->m_numInteriorInfos; k++)
            {
                if ((int32)(eInteriorInfoType)m_pInteriors[j]->m_interiorInfos[k].type == infoType)
                {
                    numInfos++;
                }
            }
        }
    }

    return numInfos;
}

// 0x5948C0
// ASM Match: 99.9%
Interior_c* InteriorGroup_c::GetRandomInterior()
{
    int32 randNum = CGeneral::GetRandomNumberInRange(0, (int32)m_numInteriors);
    int32 numInteriors = 0;

    for (int32 i = 0; i < 8; i++)
    {
        if (m_pInteriors[i])
        {
            if (numInteriors == randNum)
            {
                return m_pInteriors[i];
            }
            numInteriors++;
        }
    }

    return nullptr;
}

//! @notsa
const char* InteriorGroup_c::GetAnimBlockName() {
    switch ((eInteriorGroupType)m_groupType) {
    case eInteriorGroupType::HOUSE:  return "int_house";
    case eInteriorGroupType::SHOP:   return "int_shop";
    case eInteriorGroupType::OFFICE: return "int_office";
    default:                         NOTSA_UNREACHABLE();
    }
}
