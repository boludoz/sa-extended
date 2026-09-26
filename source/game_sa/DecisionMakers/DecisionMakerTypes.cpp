#include "StdInc.h"
#include "PedStats.h"

#include "DecisionMakerTypes.h"

void CDecisionMakerTypes::InjectHooks() {
    RH_ScopedClass(CDecisionMakerTypes);
    RH_ScopedCategory("DecisionMakers");
    RH_ScopedInstall(AddDecisionMaker, 0x607050);
    RH_ScopedInstall(RemoveDecisionMaker, 0x6043A0);
}

// 0x607050
// ASM Match: 99.7%
int32 CDecisionMakerTypes::AddDecisionMaker(CDecisionMaker* decisionMaker, eDecisionTypes iType, bool bDecisionMakerForMission)
{
    int32 iDecisionMakerID = -1;

    uint16 StartIndex;
    uint16 EndIndex;

    if (bDecisionMakerForMission)
    {
        StartIndex = 15;
        EndIndex = 20;
    }
    else
    {
        StartIndex = 0;
        EndIndex = 15;
    }

    for (int32 i = StartIndex; i < EndIndex; ++i)
    {
        if (!m_bIsActive[i])
        {
            m_bIsActive[i] = true;
            m_types[i] = iType;

            m_decisionMakers[i] = *decisionMaker;

            ++m_iNoOfDecisionMakers;

            iDecisionMakerID = i;
            break;
        }
    }

    return iDecisionMakerID;
}

// 0x4684F0
CDecisionMakerTypes* CDecisionMakerTypes::GetInstance() {
    return plugin::CallAndReturn<CDecisionMakerTypes*, 0x4684F0>();
}

// 0x606E70
void CDecisionMakerTypes::MakeDecision(CPed* ped, eEventType eventType, int32 eventSourceType, bool bIsPedInVehicle, eTaskType taskTypeToAvoid1, eTaskType taskTypeToAvoid2, eTaskType taskTypeToAvoid3, eTaskType taskTypeToSeek, bool bUseInGroupDecisionMaker, int16& taskType, int16& facialTaskType) {
    plugin::CallMethod<0x606E70>(this, ped, eventType, eventSourceType, bIsPedInVehicle, taskTypeToAvoid1, taskTypeToAvoid2, taskTypeToAvoid3, taskTypeToSeek, bUseInGroupDecisionMaker, &taskType, &facialTaskType);
}

// 0x6043A0
void CDecisionMakerTypes::RemoveDecisionMaker(eDecisionTypes iDecisionMakerIndex)
{
    if (!m_bIsActive[iDecisionMakerIndex])
    {
        return;
    }

    if (m_types[iDecisionMakerIndex] == 0)
    {
        CPedPool& pool = *GetPedPool();

        int32 i = pool.GetSize();

        while (i-- != 0)
        {
            CPed* pPed = pool.GetSlot(i);

            const int32 iGroupDecisionMakerIndex = pPed ? pPed->GetPedIntelligence()->GetPedDecisionMakerType() : -1;

            if (pPed != nullptr && iGroupDecisionMakerIndex == iDecisionMakerIndex)
            {
                pPed->GetPedIntelligence()->SetPedDecisionMakerType(pPed->m_pPedStats->m_iDefaultDecisionMaker);
            }
        }

        m_bIsActive[iDecisionMakerIndex] = false;
        m_types[iDecisionMakerIndex] = 0;
        --m_iNoOfDecisionMakers;
    }
    else
    {
        for (int32 i = 0; i < CPedGroups::MAX_NUM_GROUPS; ++i)
        {
            CPedGroup& group = CPedGroups::ms_groups[i];

            const int32 iGroupDecisionMakerIndex = group.GetGroupIntelligence()->GetGroupDecisionMakerType();

            if (CPedGroups::ms_activeGroups[i] && iDecisionMakerIndex == iGroupDecisionMakerIndex)
            {
                group.GetGroupIntelligence()->SetGroupDecisionMakerType((eDecisionMakerType)-1);
            }
        }

        m_bIsActive[iDecisionMakerIndex] = false;
        m_types[iDecisionMakerIndex] = 0;
        --m_iNoOfDecisionMakers;
    }
}

// 0x606F80
eTaskType CDecisionMakerTypes::MakeDecision(CPedGroup* pedGroup, eEventType eventType, int32 eventSourceType, bool bIsPedInVehicle, eTaskType taskId1, eTaskType taskId2, eTaskType taskId3, eTaskType taskId4) {
    return plugin::CallMethodAndReturn<eTaskType, 0x606F80, CDecisionMakerTypes*, CPedGroup*, int32, int32, bool, int32, int32, int32, int32>(
        this, pedGroup, eventType, eventSourceType, bIsPedInVehicle, taskId1, taskId2, taskId3, taskId4);
}

// 0x6044C0
void CDecisionMakerTypes::AddEventResponse(int32 decisionMakerIndex, eEventType eventType, eTaskType taskId, float* responseChances, int32* flags) {
    plugin::CallMethod<0x6044C0, CDecisionMakerTypes*, int32, int32, int32, float*, int32*>(this, decisionMakerIndex, eventType, taskId, responseChances, flags);
}

// 0x604490
void CDecisionMakerTypes::FlushDecisionMakerEventResponse(int32 decisionMakerIndex, eEventType eventId) {
    plugin::CallMethod<0x604490>(this, decisionMakerIndex, eventId);
}

void CDecisionMakerTypes::LoadEventIndices() {
    plugin::CallMethod<0x600840>(this);
}
