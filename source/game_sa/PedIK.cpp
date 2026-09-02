/*
    Plugin-SDK file
    Authors: GTA Community. See more here
    https://github.com/DK22Pac/plugin-sdk
    Do not delete this comment block. Respect others' work!
*/

#include "StdInc.h"

static inline auto& bRotateWithNeck = StaticRef<bool>(0x8D2354);

void CPedIK::InjectHooks() {
    RH_ScopedClass(CPedIK);
    RH_ScopedCategoryGlobal();

    RH_ScopedInstall(Constructor, 0x5FD8C0);
    RH_ScopedInstall(RotateTorso, 0x5FDDB0);
    RH_ScopedInstall(RotateTorsoForArm, 0x5FDF90);
    RH_ScopedOverloadedInstall(MoveLimb, "non-normalized", 0x5FDA60, MoveLimbResult(CPedIK::*)(LimbOrientation&, float, float, LimbMovementInfo&));
    RH_ScopedOverloadedInstall(MoveLimb, "normalized", 0x5FDB60, MoveLimbResult(CPedIK::*)(LimbOrientation&, float, float, LimbMovementInfo&, float));
    RH_ScopedInstall(PointGunAtPosition, 0x5FDE20);
    RH_ScopedInstall(PointGunInDirection, 0x5FDC00);
    RH_ScopedInstall(PitchForSlope, 0x5FE0E0);
    RH_ScopedInstall(GetWorldMatrix, 0x5FD8F0);
}

// 0x5FDDB0
void CPedIK::RotateTorso(AnimBlendFrameData* bone, LimbOrientation& orientation, bool changeRoll) {
    RpHAnimBlendInterpFrame* iFrame = bone->KeyFrame;
    if (changeRoll) {
        RtQuatRotate(&iFrame->q, &XaxisIK, RadiansToDegrees(orientation.m_fYaw), rwCOMBINEPRECONCAT);
    } else {
        RtQuatRotate(&iFrame->q, &XaxisIK, RadiansToDegrees(orientation.m_fYaw), rwCOMBINEREPLACE);
    }
    RtQuatRotate(&iFrame->q, &ZaxisIK, RadiansToDegrees(orientation.m_fPitch), rwCOMBINEPRECONCAT);
    m_pPed->bUpdateMatricesRequired = true;
}

// 0x5FDF90
void CPedIK::RotateTorsoForArm(const CVector& direction) {
    float fTargetHeading = std::atan2(-(direction.x - m_pPed->GetPosition().x), direction.y - m_pPed->GetPosition().y);

    if (fTargetHeading > m_pPed->m_fCurrentRotation + PI) {
        fTargetHeading -= TWO_PI;
    } else if (fTargetHeading < m_pPed->m_fCurrentRotation - PI) {
        fTargetHeading += TWO_PI;
    }

    float fDiffAngle = fTargetHeading - m_pPed->m_fCurrentRotation;
    float fResultAngle;

    if (fDiffAngle > DegreesToRadians(45.0f)) {
        fResultAngle = fDiffAngle - DegreesToRadians(45.0f);
        if (fResultAngle > DegreesToRadians(45.0f)) {
            fResultAngle = DegreesToRadians(45.0f);
        }
    } else if (fDiffAngle < -DegreesToRadians(60.0f)) {
        fResultAngle = fDiffAngle - (-DegreesToRadians(60.0f));
        if (fResultAngle < -DegreesToRadians(20.0f)) {
            fResultAngle = -DegreesToRadians(20.0f);
        }
    } else {
        return;
    }

    if (fResultAngle != 0.0f) {
        if (bRotateWithNeck) {
            fResultAngle *= 0.5f;
            RpHAnimBlendInterpFrame* iFrame2 = m_pPed->m_apBones[PED_NODE_NECK]->KeyFrame;
            RtQuatRotate(&iFrame2->q, &XaxisIK, RadiansToDegrees(fResultAngle), rwCOMBINEPOSTCONCAT);
        }
        RpHAnimBlendInterpFrame* iFrame = m_pPed->m_apBones[PED_NODE_UPPER_TORSO]->KeyFrame;
        RtQuatRotate(&iFrame->q, &XaxisIK, RadiansToDegrees(fResultAngle), rwCOMBINEPOSTCONCAT);
    }
}

// 0x5FDC00
bool CPedIK::PointGunInDirection(float zAngle, float distance, bool flag, float normalize) {
    bool rt = true;
    zAngle = CGeneral::LimitRadianAngle(zAngle - m_pPed->m_fCurrentRotation);

    bGunReachedTarget = false;
    bTorsoUsed = true;

    RpHAnimHierarchy* pHierarchy = GetAnimHierarchyFromSkinClump(m_pPed->GetRpClump());
    int32 bone = RpHAnimIDGetIndex(pHierarchy, m_pPed->m_apBones[PED_NODE_NECK]->BoneTag);
    RwMatrix* pNeckLTM = RwMatrixCreate();
    *pNeckLTM = RpHAnimHierarchyGetMatrixArray(pHierarchy)[bone];
    RwMatrixDestroy(pNeckLTM);

    MoveLimbResult bodyRt;
    if (normalize >= 1.0f) {
        bodyRt = MoveLimb(m_TorsoOrient, zAngle, distance, ms_torsoInfo, normalize);
    } else {
        bodyRt = MoveLimb(m_TorsoOrient, zAngle, distance, ms_torsoInfo);
    }

    if (bodyRt == CANT_REACH_TARGET) {
        rt = false;
    } else if (bodyRt == REACHED_TARGET) {
        bGunReachedTarget = true;
    }

    RpHAnimHierarchy* pHierarchy2;
    int32 nTestBone = 2;
    RwMatrix* pMatrix;
    float fHipYaw;

    pHierarchy2 = GetAnimHierarchyFromSkinClump(m_pPed->GetRpClump());
    pMatrix = &RpHAnimHierarchyGetMatrixArray(pHierarchy2)[nTestBone];

    fHipYaw = std::atan2(-pMatrix->at.y, -pMatrix->at.x) - m_pPed->m_fCurrentRotation;
    fHipYaw = CGeneral::LimitRadianAngle(fHipYaw);
    fHipYaw = -fHipYaw;

    RwV3d pitchAxis;
    pitchAxis.x = 0.0f;
    if (flag) {
        pitchAxis.y = std::cos(fHipYaw);
        pitchAxis.z = std::sin(fHipYaw);
    } else {
        pitchAxis.y = -std::sin(fHipYaw);
        pitchAxis.z = std::cos(fHipYaw);
    }

    RpHAnimBlendInterpFrame* iFrame = m_pPed->m_apBones[PED_NODE_UPPER_TORSO]->KeyFrame;
    RtQuatRotate(&iFrame->q, &pitchAxis, RadiansToDegrees(m_TorsoOrient.m_fPitch), rwCOMBINEPOSTCONCAT);
    RtQuatRotate(&iFrame->q, &XaxisIK, RadiansToDegrees(m_TorsoOrient.m_fYaw), rwCOMBINEPOSTCONCAT);

    m_pPed->bUpdateMatricesRequired = true;
    return rt;
}

// 0x5FDE20
void CPedIK::PointGunAtPosition(const CVector& aimAt, float normalize) {
    float desiredYaw, desiredPitch;
    float xD, yD;
    CWeaponInfo* pWeaponInfo;
    CVector vecAimFrom;

    pWeaponInfo = CWeaponInfo::GetWeaponInfo(m_pPed->GetActiveWeapon().m_Type, m_pPed->GetWeaponSkill());
    vecAimFrom = m_pPed->GetPosition();
    CMatrix* pMat = m_pPed->m_matrix;
    bool bDucking = m_pPed->bIsDucking;
    const auto& offset = pWeaponInfo->GetAimingOffset();
    vecAimFrom += (bDucking ? offset.DuckX : offset.AimX) * pMat->GetRight();
    vecAimFrom += (bDucking ? offset.DuckZ : offset.AimZ) * pMat->GetUp();
    desiredYaw = CGeneral::GetRadianAngleBetweenPoints(aimAt.x, aimAt.y, vecAimFrom.x, vecAimFrom.y);
    xD = vecAimFrom.x - aimAt.x;
    yD = vecAimFrom.y - aimAt.y;
    desiredPitch = CGeneral::GetRadianAngleBetweenPoints(
        aimAt.z,
        std::sqrt(xD * xD + yD * yD),
        vecAimFrom.z,
        0.0f
    );
    PointGunInDirection(desiredYaw, desiredPitch, false, normalize);
}

// 0x5FE0E0
void CPedIK::PitchForSlope() {
    const auto clumpData = RpAnimBlendClumpGetData(m_pPed->GetRpClump());
    const auto hier = GetAnimHierarchyFromSkinClump(m_pPed->GetRpClump());

    if (std::abs(m_fBodyRoll) > 0.01f) {
        m_fBodyRoll = std::clamp(m_fBodyRoll, DegreesToRadians(-30.0f), DegreesToRadians(30.0f));
    }

    if (!m_pPed->IsStateDying()) {
        if (std::abs(m_fSlopePitch) > 0.01f) {
            const auto offset = (m_pPed->m_nMoveState != PEDMOVE_WALK) ? 0.1f : 0.085f;
            m_fSlopePitchLimitMult = (m_fSlopePitchLimitMult * 0.9f) + offset;

            const auto lowBound  = m_fSlopePitchLimitMult * DegreesToRadians(-35.0f);
            const auto highBound = m_fSlopePitchLimitMult * DegreesToRadians(+30.0f);
            m_fSlopePitch = std::clamp(m_fSlopePitch, lowBound, highBound);
        }

        if (std::abs(m_fSlopeRoll) > 0.01f) {
            m_fSlopeRoll = std::clamp(m_fSlopeRoll, DegreesToRadians(-45.0f), DegreesToRadians(45.0f));
        }

        const auto GetAnimHierarchyMatrix = [&hier](eBoneTag bone) {
            return &RpHAnimHierarchyGetMatrixArray(hier)[RpHAnimIDGetIndex(hier, bone)];
        };

        const auto RotateBone = [clumpData, &hier](eBoneTag bone, float angle, const CVector& axis = ZaxisIK) {
            RtQuatRotate(&clumpData->m_FrameDatas[RpHAnimIDGetIndex(hier, bone)].KeyFrame->q, &axis, angle, rwCOMBINEPRECONCAT);
        };

        if (std::abs(m_fSlopePitch) > 0.01f) {
            if (&m_pPed->GetIntelligence()->GetTaskManager() && m_pPed->GetIntelligence()->GetTaskManager().GetActiveTask()->GetTaskType() == TASK_SIMPLE_STEALTH_KILL) {
                RotateBone(BONE_SPINE, RadiansToDegrees(m_fSlopePitch));
                m_pPed->bUpdateMatricesRequired = true;
            } else {
                RotateBone(BONE_SPINE, RadiansToDegrees(-m_fSlopePitch) * 0.75f);
                m_pPed->bUpdateMatricesRequired = true;

                RotateBone(BONE_R_THIGH, std::asin(GetAnimHierarchyMatrix(BONE_R_THIGH)->up.z) * 30.0f * m_fSlopePitch);
                RotateBone(BONE_R_CALF, std::max(-std::asin(GetAnimHierarchyMatrix(BONE_R_THIGH)->up.z), 0.0f) * -30.0f * m_fSlopePitch);
                RotateBone(BONE_R_FOOT, std::max(std::asin(GetAnimHierarchyMatrix(BONE_R_FOOT)->up.z), 0.0f) * -30.0f * m_fSlopePitch);
                RotateBone(BONE_L_THIGH, std::asin(GetAnimHierarchyMatrix(BONE_L_THIGH)->up.z) * 30.0f * m_fSlopePitch);
                RotateBone(BONE_L_CALF, std::max(-std::asin(GetAnimHierarchyMatrix(BONE_L_THIGH)->up.z), 0.0f) * -30.0f * m_fSlopePitch);
                RotateBone(BONE_L_FOOT, std::max(-std::asin(GetAnimHierarchyMatrix(BONE_L_FOOT)->up.z), 0.0f) * -30.0f * m_fSlopePitch);

                CMatrix result(m_pPed->GetModellingMatrix(), false);
                CMatrix translate;
                translate.SetTranslate(CVector{0.0f, 0.0f, 0.0f});
                translate.RotateX(-m_fSlopePitch / 2.0f);
                result *= translate;

                result.GetPosition() += m_pPed->GetForward() * 0.75f * std::sin(m_fSlopePitch);
                result.GetPosition().z += 0.3f / std::cos(m_fSlopeRoll) - 0.3f;
                result.UpdateRW();
                m_pPed->UpdateRwFrame();
            }
        } else {
            m_fSlopePitch = 0.0f;
        }

        if (std::abs(m_fSlopeRoll) > 0.01f) {
            const auto RotateFoot = [&](eBoneTag bone) {
                const auto hierMatrix = GetAnimHierarchyMatrix(bone);
                const auto angle = CGeneral::LimitRadianAngle(atan2(hierMatrix->at.y, hierMatrix->at.x) - m_pPed->m_fCurrentRotation);

                RotateBone(bone, RadiansToDegrees(m_fSlopeRoll), CVector{0.0f, std::cos(angle), std::sin(angle)});
            };

            RotateFoot(BONE_L_FOOT);
            RotateFoot(BONE_R_FOOT);
            m_pPed->bUpdateMatricesRequired = true;
        } else {
            m_fSlopeRoll = 0.0f;
        }

    } else {
        CMatrix matResult(m_pPed->GetModellingMatrix());

        CMatrix matTransl;
        matTransl.SetTranslate(ZaxisIK);
        matTransl.RotateX(-m_fSlopePitch);
        matTransl.RotateY(m_fSlopeRoll);
        matResult *= matTransl;
        matResult.GetPosition().z -= 1.0f;
        matResult.UpdateRW();
        m_pPed->UpdateRwFrame();
    }

    bSlopePitch = false;
}

// 0x5FD8F0
RwMatrixTag* CPedIK::GetWorldMatrix(RwFrame* frame, RwMatrixTag* transformMat) {
    RwMatrixCopy(transformMat, RwFrameGetMatrix(frame));
    RwFrame* parent = RwFrameGetParent(frame);
    while (parent) {
        RwMatrixTransform(transformMat, RwFrameGetMatrix(parent), rwCOMBINEPOSTCONCAT);
        parent = RwFrameGetParent(parent);
    }
    return transformMat;
}

// 0x5FDA60
MoveLimbResult CPedIK::MoveLimb(LimbOrientation& limb, float targetYaw, float targetPitch, LimbMovementInfo& moveInfo) {
    MoveLimbResult rt = HAVENT_REACHED_TARGET;

    if (std::abs(limb.m_fYaw - targetYaw) <= moveInfo.yawD) {
        limb.m_fYaw = targetYaw;
        rt = REACHED_TARGET;
    } else if (limb.m_fYaw < targetYaw) {
        limb.m_fYaw += moveInfo.yawD;
    } else {
        limb.m_fYaw -= moveInfo.yawD;
    }

    if (std::abs(limb.m_fPitch - targetPitch) <= moveInfo.pitchD) {
        limb.m_fPitch = targetPitch;
    } else if (limb.m_fPitch < targetPitch) {
        limb.m_fPitch += moveInfo.pitchD;
        rt = HAVENT_REACHED_TARGET;
    } else {
        limb.m_fPitch -= moveInfo.pitchD;
        rt = HAVENT_REACHED_TARGET;
    }

    if (limb.m_fYaw > moveInfo.maxYaw || limb.m_fYaw < moveInfo.minYaw || limb.m_fPitch > moveInfo.maxPitch || limb.m_fPitch < moveInfo.minPitch) {
        return CANT_REACH_TARGET;
    }

    return rt;
}

// 0x5FDB60
MoveLimbResult CPedIK::MoveLimb(LimbOrientation& limb, float targetYaw, float targetPitch, LimbMovementInfo& moveInfo, float speedMult) {
    MoveLimbResult rt = HAVENT_REACHED_TARGET;

    float yawDelta = moveInfo.yawD * speedMult;
    float pitchDelta = moveInfo.pitchD * speedMult;

    if (std::abs(limb.m_fYaw - targetYaw) <= yawDelta) {
        limb.m_fYaw = targetYaw;
        rt = REACHED_TARGET;
    } else if (limb.m_fYaw < targetYaw) {
        limb.m_fYaw += yawDelta;
    } else {
        limb.m_fYaw -= yawDelta;
    }

    if (std::abs(limb.m_fPitch - targetPitch) <= pitchDelta) {
        limb.m_fPitch = targetPitch;
    } else if (limb.m_fPitch < targetPitch) {
        limb.m_fPitch += pitchDelta;
        rt = HAVENT_REACHED_TARGET;
    } else {
        limb.m_fPitch -= pitchDelta;
        rt = HAVENT_REACHED_TARGET;
    }

    if (limb.m_fYaw > moveInfo.maxYaw || limb.m_fYaw < moveInfo.minYaw || limb.m_fPitch > moveInfo.maxPitch || limb.m_fPitch < moveInfo.minPitch) {
        return CANT_REACH_TARGET;
    }

    return rt;
}
