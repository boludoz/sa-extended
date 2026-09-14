#include "StdInc.h"

#include "TaskSimpleRunNamedAnim.h"
#include "Plugins/RpAnimBlendPlugin/RpAnimBlend.h"

void CTaskSimpleRunNamedAnim::InjectHooks() {
    RH_ScopedVirtualClass(CTaskSimpleRunNamedAnim, 0x86D54C, 9);
    RH_ScopedCategory("Tasks/TaskTypes");

    RH_ScopedOverloadedInstall(Constructor, "Default", 0x6674B0, CTaskSimpleRunNamedAnim*(CTaskSimpleRunNamedAnim::*)());
    RH_ScopedOverloadedInstall(Constructor, "Anim", 0x61A990, CTaskSimpleRunNamedAnim*(CTaskSimpleRunNamedAnim::*)(char const*, char const*, int32, float, int32, bool, bool, bool, bool));
    RH_ScopedInstall(Destructor, 0x61BF10);

    RH_ScopedInstall(FinishRunAnimMovePedCB, 0x61AAA0);
    RH_ScopedInstall(StartAnim, 0x61BB10);
    RH_ScopedVMTInstall(Clone, 0x61B770);
    RH_ScopedVMTInstall(GetTaskType, 0x61AA90);
    RH_ScopedVMTInstall(ProcessPed, 0x61BF20);
    RH_ScopedInstall(OffsetPedPosition, 0x61AB00);
}

// 0x6674B0
CTaskSimpleRunNamedAnim::CTaskSimpleRunNamedAnim() :
    CTaskSimpleAnim{ false }
{
    m_Timer.m_nStartTime = 0;
    m_Timer.m_nInterval = 0;
    m_Timer.m_bStarted = false;
    m_Timer.m_bStopped = false;
}

// 0x61A990
CTaskSimpleRunNamedAnim::CTaskSimpleRunNamedAnim(
    const char* animName,
    const char* animGroupName,
    uint32 animFlags,
    float blendDelta,
    uint32 endTime,
    bool bDontInterrupt,
    bool bRunInSequence,
    bool bOffsetPed,
    bool bHoldLastFrame
) :
    CTaskSimpleAnim(bHoldLastFrame),
    m_fBlendDelta{ blendDelta },
    m_Time{ endTime },
    m_vecOffsetAtEnd{ 0.0f, 0.0f, 0.0f },
    m_animFlags{ animFlags }
{
    m_Timer.m_nStartTime = 0;
    m_Timer.m_nInterval = 0;
    m_Timer.m_bStarted = false;
    m_Timer.m_bStopped = false;

    m_bDontInterrupt = bDontInterrupt;
    m_bRunInSequence = bRunInSequence;
    m_bOffsetAtEnd = bOffsetPed;

    strcpy_s(m_animName, animName);
    strcpy_s(m_animGroupName, animGroupName);

    if (const auto block = CAnimManager::GetAnimationBlock(m_animGroupName)) {
        m_pAnimHierarchy = CAnimManager::GetAnimation(m_animName, block);
    } else {
        m_pAnimHierarchy = nullptr;
    }
}

// 0x61B770
CTask* CTaskSimpleRunNamedAnim::Clone() const {
    return new CTaskSimpleRunNamedAnim(
        m_animName,
        m_animGroupName,
        m_animFlags,
        m_fBlendDelta,
        m_Time,
        m_bDontInterrupt,
        m_bRunInSequence,
        m_bOffsetAtEnd,
        m_bHoldLastFrame
    );
}

// 0x61AAA0
void CTaskSimpleRunNamedAnim::FinishRunAnimMovePedCB(CAnimBlendAssociation* pAnim, void* pData) {
    auto* pTask = static_cast<CTaskSimpleRunNamedAnim*>(pData);
    CAnimBlendNode* pNode = pAnim->GetNode(0);
    pNode->GetCurrentTranslation(pTask->m_vecOffsetAtEnd, 0.0f);
    pTask->m_bOffsetAvailable = true;
    pAnim->SetFlag(ANIMATION_IGNORE_ROOT_TRANSLATION);

    if (!pAnim->HasFlag(ANIMATION_IS_FINISH_AUTO_REMOVE) && pAnim->m_nCallbackType != ANIM_BLEND_CALLBACK_DELETE) {
        pAnim->SetDeleteCallback(CTaskSimpleAnim::FinishRunAnimCB, pTask);
    } else {
        pTask->m_bIsFinished = true;
        pTask->m_pAnim = nullptr;
    }
}

// 0x61BB10
void CTaskSimpleRunNamedAnim::StartAnim(CPed* ped) {
    if ((int32)m_Time >= 0) {
        m_Timer.m_nStartTime = CTimer::m_snTimeInMilliseconds;
        m_Timer.m_nInterval = m_Time;
        m_Timer.m_bStarted = true;
    }

    if (!(m_animFlags & 8)) {
        m_animFlags |= 4;
    }

    m_pAnim = CAnimManager::BlendAnimation(ped->GetRpClump(), m_pAnimHierarchy, m_animFlags, m_fBlendDelta);

    if (m_bOffsetAtEnd) {
        m_pAnim->SetFinishCallback(FinishRunAnimMovePedCB, this);
    } else if (!(m_animFlags & 8) && !m_bRunInSequence) {
        m_pAnim->SetDeleteCallback(CTaskSimpleAnim::FinishRunAnimCB, this);
    } else {
        m_pAnim->SetFinishCallback(CTaskSimpleAnim::FinishRunAnimCB, this);
    }

    m_nAnimId = m_pAnim->GetAnimId();
}

// 0x61AB00
void CTaskSimpleRunNamedAnim::OffsetPedPosition(CPed* ped) {
    ped->UpdateRpHAnim();
    ped->m_bDontUpdateHierarchy = true;
    ped->GetPosition() += ped->GetMatrix().TransformVector(m_vecOffsetAtEnd);
    m_bOffsetAvailable = false;
}

// 0x61BF20
bool CTaskSimpleRunNamedAnim::ProcessPed(CPed* ped) {
    if (m_bOffsetAvailable) {
        OffsetPedPosition(ped);
    }

    if (m_bIsFinished) {
        if (!m_bOffsetAtEnd) {
            CAnimBlendAssociation* pAssoc = RpAnimBlendClumpGetAssociation(ped->GetRpClump(), true, m_pAnimHierarchy);
            const CVector bonePos = ped->GetBonePosition(BONE_SPINE1, false);

            if (m_fBlendDelta > 100.0f && pAssoc) {
                if ((ped->GetPosition() - bonePos).Magnitude() > 1.0f) {
                    pAssoc->SetFlag(ANIMATION_IS_BLEND_AUTO_REMOVE);
                    pAssoc->SetBlendDelta(-m_fBlendDelta);
                }
            }
        }
        return true;
    }

    if (!m_pAnimHierarchy || !m_pAnimHierarchy->m_pSequences) {
        return true;
    }

    if (m_bDontInterrupt && (m_animFlags & 10) != 8) {
        ped->GetIntelligence()->GetEventGroup().RemoveInvalidEvents(true);
        ped->GetIntelligence()->GetEventGroup().Reorganise();
    }

    if (m_Timer.m_bStarted && m_Timer.IsOutOfTime()) {
        if (m_animFlags & 8) {
            MakeAbortable(ped, ABORT_PRIORITY_URGENT, nullptr);
        } else {
            if (m_pAnim) {
                m_pAnim->SetFinishCallback(CDefaultAnimCallback::DefaultAnimCB, nullptr);
            }
            m_bIsFinished = true;
            m_pAnim = nullptr;
            return true;
        }
    }

    if (!m_pAnim) {
        CAnimBlendAssociation* pAssoc = RpAnimBlendClumpGetAssociation(ped->GetRpClump(), true, m_pAnimHierarchy);
        if (pAssoc && pAssoc->m_pCallbackData) {
            return true;
        }
        StartAnim(ped);
    }

    return m_bIsFinished;
}
