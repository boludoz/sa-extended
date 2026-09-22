#include "StdInc.h"

#include "TaskUtilityLineUpPedWithCar.h"
#include "PedPlacement.h"
#include "rtslerp.h"

void CTaskUtilityLineUpPedWithCar::InjectHooks() {
    RH_ScopedClass(CTaskUtilityLineUpPedWithCar);
    RH_ScopedCategory("Tasks/TaskTypes");

    RH_ScopedInstall(Constructor, 0x64FBB0);
    RH_ScopedInstall(Destructor, 0x64FC00);

    RH_ScopedInstall(GetLocalPositionToOpenCarDoor, 0x64FC10);
    RH_ScopedInstall(GetPositionToOpenCarDoor, 0x650A80);
    RH_ScopedInstall(ProcessPed, 0x6513A0);
}

// 0x64FBB0
CTaskUtilityLineUpPedWithCar::CTaskUtilityLineUpPedWithCar(const CVector& offset, int32 time, int32 doorOpenPosType, int32 doorIdx) {
    m_Offset = offset;
    m_fDoorOpenPosZ = -999.99f;
    m_fTime = CTimer::GetTimeInMS() + time;
    m_nDoorOpenPosType = doorOpenPosType;
    m_nDoorIdx = doorIdx;
}

// The following 2 functions seem to have copy elision on the returned CVector, that the compiled functions
// took a vector ptr as their first arg. Now, hopefully our code will compile to the same stuff.
// If not, it might crash here, in that case a wrapper function should be used.

// 0x64FC10
CVector CTaskUtilityLineUpPedWithCar::GetLocalPositionToOpenCarDoor(CVehicle* vehicle, float animProgress, CAnimBlendAssociation* assoc) {
    const auto mi = vehicle->GetVehicleModelInfo();

    const auto wideVehicleXBlendOffset = vehicle->vehicleFlags.bIsVan && (m_nDoorIdx == TARGET_DOOR_REAR_LEFT || m_nDoorIdx == TARGET_DOOR_REAR_RIGHT)
        ? 0.f
        : animProgress * vehicle->m_pHandlingData->m_fSeatOffsetDistance;

    // Which door offset the current anim is lined up against
    const auto offsetType = [&] {
        enum { ENTER, EXIT, JACKED, NONE } kind = ENTER;
        if (assoc) {
            switch (assoc->GetAnimId()) {
            case ANIM_ID_CAR_ALIGN_LHS: // 351 - 372
            case ANIM_ID_CAR_ALIGN_RHS:
            case ANIM_ID_CAR_ALIGNHI_LHS:
            case ANIM_ID_CAR_ALIGNHI_RHS:
            case ANIM_ID_CAR_OPEN_LHS:
            case ANIM_ID_CAR_OPEN_RHS:
            case ANIM_ID_CAR_OPEN_LHS_1:
            case ANIM_ID_CAR_OPEN_RHS_1:
            case ANIM_ID_CAR_GETIN_LHS_0:
            case ANIM_ID_CAR_GETIN_RHS_0:
            case ANIM_ID_CAR_GETIN_LHS_1:
            case ANIM_ID_CAR_GETIN_RHS_1:
            case ANIM_ID_CAR_GETIN_BIKE_FRONT:
            case ANIM_ID_CAR_PULLOUT_LHS:
            case ANIM_ID_CAR_PULLOUT_RHS:
            case ANIM_ID_UNKNOWN_15:
            case ANIM_ID_CAR_CLOSEDOOR_LHS_0:
            case ANIM_ID_CAR_CLOSEDOOR_RHS_0:
            case ANIM_ID_CAR_CLOSEDOOR_LHS_1:
            case ANIM_ID_CAR_CLOSEDOOR_RHS_1:
            case ANIM_ID_CAR_SHUFFLE_RHS_0:
            case ANIM_ID_CAR_SHUFFLE_RHS_1:
            case ANIM_ID_CAR_DOORLOCKED_LHS:
            case ANIM_ID_CAR_DOORLOCKED_RHS:
                kind = ENTER;
                break;
            case ANIM_ID_DEFAULT_CAR_CRAWLOUTRHS_0:
            case ANIM_ID_DEFAULT_CAR_CRAWLOUTRHS_1:
            case ANIM_ID_CAR_GETOUT_LHS_0:
            case ANIM_ID_CAR_GETOUT_RHS_0:
            case ANIM_ID_CAR_GETOUT_LHS_1:
            case ANIM_ID_CAR_GETOUT_RHS_1:
            case ANIM_ID_CAR_CLOSE_LHS_0:
            case ANIM_ID_CAR_CLOSE_RHS_0:
            case ANIM_ID_CAR_CLOSE_LHS_1:
            case ANIM_ID_CAR_CLOSE_RHS_1:
            case ANIM_ID_CAR_FALLOUT_LHS:
            case ANIM_ID_CAR_FALLOUT_RHS:
                kind = EXIT;
                break;
            case ANIM_ID_CAR_JACKEDLHS:
            case ANIM_ID_CAR_JACKEDRHS:
                kind = JACKED;
                break;
            default:
                kind = NONE; // NOTSA: Originally an uninitialised variable was used here
                break;
            }
        }

        // Both `ENTER` and `EXIT` use the entry offsets. `JACKED` ends up at 0 too.
        switch (m_nDoorIdx) {
        case TARGET_DOOR_REAR_LEFT:
        case TARGET_DOOR_REAR_RIGHT:
            return kind == ENTER || kind == EXIT ? ENTER_REAR : ENTER_FRONT;
        case 18:
            return kind == ENTER || kind == EXIT ? ENTER_BIKE_FRONT : ENTER_FRONT;
        default:
            return ENTER_FRONT;
        }
    }();

    const auto animGroupIdx = vehicle->m_pHandlingData->m_nAnimGroup;
    const auto animOffset = [&] {
        const auto groupOffset = CVehicleAnimGroupData::GetVehicleAnimGroup(animGroupIdx).ComputeAnimDoorOffsets(offsetType);
        if (assoc
            && (assoc->GetAnimId() == ANIM_ID_CAR_DOORLOCKED_LHS || assoc->GetAnimId() == ANIM_ID_CAR_DOORLOCKED_RHS)
            && CVehicleAnimGroupData::GetVehicleAnimGroup(animGroupIdx).m_specialFlags.bRunSpecialLockedDoor
        ) {
            // Blend towards the default group's offsets
            const auto defaultOffset = CVehicleAnimGroupData::GetVehicleAnimGroup(0).ComputeAnimDoorOffsets(offsetType);
            return groupOffset * (1.f - assoc->m_BlendAmount) + defaultOffset * assoc->m_BlendAmount;
        }
        return groupOffset;
    }();

    CVector localSeatPos, doorOffset = animOffset, posToRunAnim;
    switch (m_nDoorIdx) {
    case TARGET_DOOR_FRONT_RIGHT:
    case TARGET_DOOR_REAR_RIGHT:
        localSeatPos = m_nDoorIdx == TARGET_DOOR_FRONT_RIGHT ? mi->GetFrontSeatPosn() : mi->GetBackSeatPosn();
        localSeatPos.x += wideVehicleXBlendOffset;
        doorOffset.x = -doorOffset.x;
        posToRunAnim = localSeatPos - doorOffset;
        break;
    case TARGET_DOOR_DRIVER:
    case TARGET_DOOR_REAR_LEFT:
        localSeatPos = m_nDoorIdx == TARGET_DOOR_DRIVER ? mi->GetFrontSeatPosn() : mi->GetBackSeatPosn();
        localSeatPos.x = -(localSeatPos.x + wideVehicleXBlendOffset);
        posToRunAnim = localSeatPos - doorOffset;
        break;
    case 18:
        localSeatPos = mi->GetFrontSeatPosn();
        posToRunAnim = doorOffset + localSeatPos;
        break;
    default:
        localSeatPos = mi->GetFrontSeatPosn();
        doorOffset = CVector{};
        posToRunAnim = localSeatPos;
        break;
    }

    if (vehicle->IsBike() && m_nDoorIdx != 18) {
        static_cast<CBike*>(vehicle)->GetCorrectedWorldDoorPosition(posToRunAnim, doorOffset, localSeatPos);
        posToRunAnim = vehicle->GetMatrix().InverseTransformVector(posToRunAnim - vehicle->GetPosition());
    }

    return posToRunAnim;
}

// 0x650A80
// ASM Match
CVector CTaskUtilityLineUpPedWithCar::GetPositionToOpenCarDoor(CVehicle* vehicle, float animProgress, CAnimBlendAssociation* assoc) {
    const CMatrix vehMat = vehicle->GetMatrix();
    return vehMat.TransformVector(GetLocalPositionToOpenCarDoor(vehicle, animProgress, assoc)) + vehicle->GetPosition();
}

// 0x6513A0
bool CTaskUtilityLineUpPedWithCar::ProcessPed(CPed* ped, CVehicle* vehicle, CAnimBlendAssociation* assoc) {
    if (m_nDoorOpenPosType == 0) {
        ped->m_vecMoveSpeed = CVector{};
    }

    // Heading the ped should end up with
    bool bIsUpsideDown = false;
    if (vehicle->GetUp().z <= -0.8f) {
        bIsUpsideDown = true;
        ped->m_fAimingRotation = m_nDoorIdx == TARGET_DOOR_FRONT_RIGHT || m_nDoorIdx == TARGET_DOOR_REAR_RIGHT
            ? vehicle->GetHeading() - PI
            : vehicle->GetHeading();
    } else if (m_nDoorIdx == 18) {
        ped->m_fAimingRotation = vehicle->GetHeading() + PI;
    } else if (m_nDoorOpenPosType != 2) {
        ped->m_fAimingRotation = vehicle->GetHeading();
    }

    // How far along the door (X) and up from the ground (Z) the ped should be
    float xBlend = 0.f, zBlend = 0.f;
    if (assoc) {
        const auto animId = assoc->GetAnimId();
        const auto t = assoc->m_CurrentTime / assoc->m_BlendHier->m_fTotalTime;
        const auto CriticalBlendTime = [&] {
            return CVehicleAnimGroupData::ComputeCriticalBlendTime((AssocGroupId)vehicle->m_pHandlingData->m_nAnimGroup, animId);
        };

        switch (animId) {
        case ANIM_ID_DEFAULT_CAR_CRAWLOUTRHS_0:
        case ANIM_ID_DEFAULT_CAR_CRAWLOUTRHS_1:
            zBlend = 0.f;
            xBlend = t;
            break;
        case ANIM_ID_CAR_ALIGN_LHS:
        case ANIM_ID_CAR_ALIGN_RHS:
        case ANIM_ID_CAR_ALIGNHI_LHS:
        case ANIM_ID_CAR_ALIGNHI_RHS: {
            xBlend = 1.f;
            const auto f = CriticalBlendTime();
            auto fMod = std::abs(f);
            if (fMod < 10.f) {
                zBlend = 0.f;
            } else {
                fMod -= 11.f;
                if (f < 0.f) {
                    zBlend = t < fMod ? 0.f : (t - fMod) / (1.f - fMod);
                } else {
                    zBlend = t >= fMod ? 1.f : t / fMod;
                }
            }
            if (vehicle->GetModelId() == MODEL_AT400) {
                zBlend = 1.f;
            }
            break;
        }
        case ANIM_ID_CAR_OPEN_LHS:
        case ANIM_ID_CAR_OPEN_RHS:
        case ANIM_ID_CAR_OPEN_LHS_1:
        case ANIM_ID_CAR_OPEN_RHS_1:
            xBlend = 1.f;
            zBlend = std::abs(CriticalBlendTime()) >= 10.f ? 1.f : 0.f;
            if (vehicle->GetModelId() == MODEL_AT400) {
                zBlend = 1.f;
            }
            break;
        case ANIM_ID_CAR_GETIN_LHS_0:
        case ANIM_ID_CAR_GETIN_RHS_0:
        case ANIM_ID_CAR_GETIN_LHS_1:
        case ANIM_ID_CAR_GETIN_RHS_1:
        case ANIM_ID_CAR_GETIN_BIKE_FRONT: {
            xBlend = 1.f - t;
            const auto f = CriticalBlendTime();
            const auto fMod = std::abs(f);
            if (fMod >= 10.f) {
                zBlend = 1.f;
            } else if (f < 0.f) {
                zBlend = t >= fMod ? (t - fMod) / (1.f - fMod) : 0.f;
            } else {
                zBlend = t >= fMod ? 1.f : t / fMod;
            }
            if (vehicle->GetModelId() == MODEL_AT400) {
                zBlend = 1.f;
            }
            break;
        }
        case ANIM_ID_CAR_PULLOUT_LHS:
        case ANIM_ID_CAR_PULLOUT_RHS:
        case ANIM_ID_UNKNOWN_15:
        case ANIM_ID_CAR_CLOSE_LHS_0:
        case ANIM_ID_CAR_CLOSE_RHS_0:
        case ANIM_ID_CAR_CLOSE_LHS_1:
        case ANIM_ID_CAR_CLOSE_RHS_1:
            xBlend = 1.f;
            zBlend = 0.f;
            break;
        case ANIM_ID_CAR_CLOSEDOOR_LHS_0:
        case ANIM_ID_CAR_CLOSEDOOR_RHS_0:
        case ANIM_ID_CAR_CLOSEDOOR_LHS_1:
        case ANIM_ID_CAR_CLOSEDOOR_RHS_1:
        case ANIM_ID_CAR_SHUFFLE_RHS_0:
        case ANIM_ID_CAR_SHUFFLE_RHS_1:
        case ANIM_ID_CAR_ROLLDOOR:
            xBlend = 0.f;
            zBlend = 1.f;
            break;
        case ANIM_ID_CAR_DOORLOCKED_LHS:
        case ANIM_ID_CAR_DOORLOCKED_RHS:
            xBlend = 1.f;
            zBlend = CVehicleAnimGroupData::GetVehicleAnimGroup(vehicle->m_pHandlingData->m_nAnimGroup).m_specialFlags.bRunSpecialLockedDoor ? 1.f : 0.f;
            break;
        case ANIM_ID_CAR_GETOUT_LHS_0:
        case ANIM_ID_CAR_GETOUT_RHS_0:
        case ANIM_ID_CAR_GETOUT_LHS_1:
        case ANIM_ID_CAR_GETOUT_RHS_1:
        case ANIM_ID_CAR_JACKEDLHS:
        case ANIM_ID_CAR_JACKEDRHS:
        case ANIM_ID_CAR_ROLLOUT_LHS:
        case ANIM_ID_CAR_ROLLOUT_RHS:
        case ANIM_ID_CAR_FALLOUT_LHS:
        case ANIM_ID_CAR_FALLOUT_RHS: {
            xBlend = t;
            const auto f = CriticalBlendTime();
            const auto fMod = std::abs(f);
            if (f < 0.f) {
                zBlend = t < fMod ? 1.f : 1.f - (t - fMod) / (1.f - fMod);
            } else {
                zBlend = t >= fMod ? 0.f : 1.f - t / fMod;
            }
            break;
        }
        default:
            break;
        }
    }

    auto targetPos = m_nDoorOpenPosType == 2
        ? ped->GetPosition()
        : GetPositionToOpenCarDoor(vehicle, xBlend, assoc);

    // Ground height at the (fully open) door
    auto roadPos = targetPos;
    if (!vehicle->IsBike() && m_nDoorOpenPosType != 2) {
        roadPos = GetPositionToOpenCarDoor(vehicle, 1.f, assoc);
    }

    if (vehicle->physicalFlags.bSubmergedInWater) {
        if (vehicle->IsBoat() && vehicle->IsUpsideDown()) {
            roadPos.z += 1.f;
        }
    } else {
        const auto sidePos   = DotProduct(roadPos - vehicle->GetPosition(), vehicle->GetRight());
        const auto minHeight = vehicle->GetPosition().z - vehicle->GetHeightAboveRoad() + vehicle->GetRight().z * sidePos + 1.f;
        const auto originalHeight = roadPos.z;
        roadPos = CPedPlacement::FindZCoorForPed(roadPos).first;
        if (minHeight - 0.5f > roadPos.z) {
            roadPos.z = originalHeight;
        }
    }
    m_fDoorOpenPosZ = roadPos.z;

    // Fall under gravity until reaching the ground
    if (m_nDoorOpenPosType == 1 || m_nDoorOpenPosType == 2) {
        const auto gravityZStep = ped->m_vecMoveSpeed.z - CTimer::GetTimeStep() * 0.008f;
        if (gravityZStep + ped->GetPosition().z >= roadPos.z) {
            ped->m_vecMoveSpeed.z = gravityZStep;
            targetPos.z = ped->GetPosition().z;
        } else {
            targetPos.z = roadPos.z;
            ped->m_vecMoveSpeed = CVector{};
        }
    }

    if (m_fDoorOpenPosZ > targetPos.z && vehicle->IsBike() && assoc && assoc->GetAnimId() != ANIM_ID_CAR_GETIN_BIKE_FRONT) {
        const auto animId = assoc->GetAnimId();
        const auto blend = [&] {
            switch (animId) {
            case ANIM_ID_CAR_GETOUT_LHS_0:
            case ANIM_ID_CAR_GETOUT_RHS_0:
            case ANIM_ID_CAR_GETOUT_LHS_1:
            case ANIM_ID_CAR_GETOUT_RHS_1:
                return 1.f - xBlend;
            case ANIM_ID_CAR_GETIN_LHS_0:
            case ANIM_ID_CAR_GETIN_RHS_0:
            case ANIM_ID_CAR_GETIN_LHS_1:
            case ANIM_ID_CAR_GETIN_RHS_1: {
                const auto t = assoc->m_CurrentTime / assoc->m_BlendHier->m_fTotalTime;
                return std::min(t + t, 1.f);
            }
            default:
                return 0.f;
            }
        }();
        targetPos.z = (vehicle->GetPosition().z - vehicle->GetHeightAboveRoad() + 1.f - m_fDoorOpenPosZ) * blend + m_fDoorOpenPosZ;
    } else if (m_fDoorOpenPosZ > targetPos.z || m_nDoorOpenPosType == 0) {
        targetPos.z = (targetPos.z - m_fDoorOpenPosZ) * zBlend + m_fDoorOpenPosZ;
    }

    // Blend the heading and the initial offset out over time
    if (CTimer::GetTimeInMS() >= (uint32)m_fTime) {
        ped->m_fCurrentRotation = ped->m_fAimingRotation;
    } else {
        auto aimingRotation = CGeneral::LimitRadianAngle(ped->m_fAimingRotation);
        const auto currentRotation = ped->m_fCurrentRotation;
        const auto timeLeft = (float)(m_fTime - (int32)CTimer::GetTimeInMS()) / 600.f;
        if (timeLeft <= 0.f) {
            m_Offset.x = 0.f;
            m_Offset.y = 0.f;
        }
        m_Offset.z = 0.f;
        targetPos -= m_Offset * timeLeft;

        if (currentRotation + PI < aimingRotation) {
            aimingRotation -= TWO_PI;
        } else if (currentRotation - PI > aimingRotation) {
            aimingRotation += TWO_PI;
        }
        ped->m_fCurrentRotation = currentRotation - (currentRotation - aimingRotation) * (1.f - timeLeft);
    }

    // Slerp the ped's orientation between `from` and `to`, placing it at `targetPos`
    const auto SetPedMatrixSlerped = [&](const CMatrix& from, const CMatrix& to) {
        RwMatrix rwFrom, rwTo;
        from.CopyToRwMatrix(&rwFrom);
        to.CopyToRwMatrix(&rwTo);

        RtQuat quatFrom, quatTo;
        RtQuatConvertFromMatrix(&quatFrom, &rwFrom);
        RtQuatConvertFromMatrix(&quatTo, &rwTo);

        RtQuatSlerpCache slerpCache;
        RtQuatSetupSlerpCache(&quatFrom, &quatTo, &slerpCache);

        RtQuat quatResult;
        const auto t = assoc->m_CurrentTime / assoc->m_BlendHier->m_fTotalTime;
        RtQuatSlerp(&quatResult, &quatFrom, &quatTo, t, &slerpCache);

        RwMatrix rwResult;
        RtQuatConvertToMatrix(&quatResult, &rwResult);

        CMatrix result;
        result.UpdateMatrix(&rwResult);
        result.GetPosition() = targetPos;
        ped->SetMatrix(result);
    };

    if (assoc) {
        switch (assoc->GetAnimId()) {
        case ANIM_ID_CAR_GETIN_LHS_0:
        case ANIM_ID_CAR_GETIN_RHS_0:
        case ANIM_ID_CAR_GETIN_LHS_1:
        case ANIM_ID_CAR_GETIN_RHS_1:
        case ANIM_ID_CAR_GETIN_BIKE_FRONT: {
            // Turn from the ped's orientation to the vehicle's
            CMatrix vehMat{ vehicle->GetMatrix() };
            if (assoc->GetAnimId() == ANIM_ID_CAR_GETIN_BIKE_FRONT) {
                CMatrix rotation;
                rotation.SetRotateZ(PI);
                vehMat *= rotation;
            }
            SetPedMatrixSlerped(CMatrix{ ped->GetMatrix() }, vehMat);
            return false;
        }
        case ANIM_ID_CAR_GETOUT_LHS_0:
        case ANIM_ID_CAR_GETOUT_RHS_0:
        case ANIM_ID_CAR_GETOUT_LHS_1:
        case ANIM_ID_CAR_GETOUT_RHS_1:
        case ANIM_ID_UNKNOWN_26: {
            // Turn from the vehicle's orientation to the ped's heading
            CMatrix pedMat{ ped->GetMatrix() };
            pedMat.SetRotateZOnly(ped->m_fCurrentRotation);
            SetPedMatrixSlerped(CMatrix{ vehicle->GetMatrix() }, pedMat);
            return false;
        }
        default:
            break;
        }
    }

    if (xBlend > 0.2f || bIsUpsideDown || vehicle->IsBike() || vehicle->IsSubQuad()) {
        ped->SetPosn(targetPos);
        ped->SetOrientation(0.f, 0.f, ped->m_fCurrentRotation);
    } else {
        CMatrix vehMat{ vehicle->GetMatrix() };
        vehMat.GetPosition() += vehMat.TransformVector(GetLocalPositionToOpenCarDoor(vehicle, 0.f, assoc));
        ped->SetMatrix(vehMat);
    }

    return false;
}
