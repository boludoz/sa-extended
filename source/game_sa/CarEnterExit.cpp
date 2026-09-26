#include "StdInc.h"

#include "PedStats.h"
#include "CarEnterExit.h"
#include "TaskSimpleCarSetPedInAsDriver.h"
#include "TaskComplexDriveWander.h"
#include "TaskSimpleCarSetPedInAsPassenger.h"
#include "Events/EventPedEnteredMyVehicle.h"

void CCarEnterExit::InjectHooks() {
    RH_ScopedClass(CCarEnterExit);
    RH_ScopedCategoryGlobal();

    RH_ScopedInstall(AddInCarAnim, 0x64F720);
    RH_ScopedInstall(CarHasDoorToClose, 0x64EE10);
    RH_ScopedInstall(CarHasDoorToOpen, 0x64EDD0);
    RH_ScopedInstall(CarHasOpenableDoor, 0x64EE50);
    RH_ScopedInstall(CarHasPartiallyOpenDoor, 0x64EE70);
    RH_ScopedInstall(ComputeDoorFlag, 0x64E550);
    RH_ScopedInstall(ComputeOppositeDoorFlag, 0x64E610);
    RH_ScopedInstall(IsDriverDoorFlag, 0x64E6D0);
    RH_ScopedInstall(ComputePassengerIndexFromCarDoor, 0x64F1E0);
    RH_ScopedInstall(ComputeQuickJackedPed, 0x64F040);
    RH_ScopedInstall(ComputeSlowJackedPed, 0x64F070);
    RH_ScopedOverloadedInstall(ComputeTargetDoorToEnterAsPassenger, "", 0x64F190, int32(*)(const CVehicle*, int32));
    RH_ScopedInstall(ComputeTargetDoorToExit, 0x64F110);
    RH_ScopedInstall(GetNearestCarDoor, 0x6528F0);
    RH_ScopedInstall(GetNearestCarPassengerDoor, 0x650BB0);
    RH_ScopedInstall(GetPositionToOpenCarDoor, 0x64E740);
    RH_ScopedInstall(IsCarDoorInUse, 0x64EC90);
    RH_ScopedInstall(IsCarDoorReady, 0x64ED90);
    RH_ScopedInstall(IsCarQuickJackPossible, 0x64EF00);
    RH_ScopedInstall(IsCarSlowJackRequired, 0x64EF70);
    RH_ScopedInstall(IsClearToDriveAway, 0x6509B0);
    RH_ScopedInstall(IsPathToDoorBlockedByVehicleCollisionModel, 0x651210);
    RH_ScopedInstall(IsPedHealthy, 0x64EEE0);
    RH_ScopedInstall(IsPlayerToQuitCarEnter, 0x64F240);
    RH_ScopedInstall(IsRoomForPedToLeaveCar, 0x6504C0);
    RH_ScopedInstall(IsVehicleHealthy, 0x64EEC0);
    RH_ScopedInstall(IsVehicleStealable, 0x6510D0);
    RH_ScopedInstall(MakeUndraggedDriverPedLeaveCar, 0x64F600);
    RH_ScopedInstall(MakeUndraggedPassengerPedsLeaveCar, 0x64F540);
    RH_ScopedInstall(QuitEnteringCar, 0x650130);
    RH_ScopedInstall(RemoveCarSitAnim, 0x64F680);
    RH_ScopedInstall(RemoveGetInAnims, 0x64F6E0);
    RH_ScopedInstall(SetAnimOffsetForEnterOrExitVehicle, 0x64F860);
    RH_ScopedInstall(SetPedInCarDirect, 0x650280);
}

// 0x64F720
// ASM Match
void CCarEnterExit::AddInCarAnim(const CVehicle* vehicle, CPed* ped, bool bAsDriver) {
    const auto [grpId, animId] = [&]() -> std::pair<AssocGroupId, AnimationId> {
        if (bAsDriver) { // Inverted
            if (const auto data = const_cast<CVehicle*>(vehicle)->GetRideAnimData()) {
                return { data->AnimGroup, ANIM_ID_BIKE_RIDE };
            } else if (vehicle->IsBoat()) {
                if (!vehicle->m_pHandlingData->m_bSitInBoat) {
                    return { ANIM_GROUP_DEFAULT, ANIM_ID_DRIVE_BOAT };
                }
            } else if (vehicle->vehicleFlags.bLowVehicle) {
                return { ANIM_GROUP_DEFAULT, ANIM_ID_CAR_LSIT };
            }

            return { ANIM_GROUP_DEFAULT, ANIM_ID_CAR_SIT };
        } else {
            if (const auto data = const_cast<CVehicle*>(vehicle)->GetRideAnimData()) {
                return { data->AnimGroup, ANIM_ID_BIKE_RIDE };
            } else if (vehicle->vehicleFlags.bLowVehicle) {
                return { ANIM_GROUP_DEFAULT, ANIM_ID_CAR_SITPLO };
            }

            return { ANIM_GROUP_DEFAULT, ANIM_ID_CAR_SITP };
        }
    }();
    CAnimManager::BlendAnimation(ped->GetRpClump(), grpId, animId, 1000.f);
    ped->StopNonPartialAnims();
}

// 0x64EE10
// ASM Match
bool CCarEnterExit::CarHasDoorToClose(const CVehicle* vehicle, int32 doorId) {
    auto& veh = const_cast<CVehicle&>(*vehicle);
    return !veh.IsDoorMissingU32(doorId) && !veh.IsDoorClosedU32(doorId);
}

// 0x64EDD0
// ASM Match
bool CCarEnterExit::CarHasDoorToOpen(const CVehicle* vehicle, int32 doorId) {
    auto& veh = const_cast<CVehicle&>(*vehicle);
    return !veh.IsDoorMissingU32((uint32)doorId) && !veh.IsDoorFullyOpenU32((uint32)doorId);
}

// 0x64EE50
// ASM Match
bool CCarEnterExit::CarHasOpenableDoor(const CVehicle* vehicle, int32 doorId_UnusedArg, const CPed* ped) {
    return vehicle->CanPedOpenLocks(ped);
}

// 0x64EE70
bool CCarEnterExit::CarHasPartiallyOpenDoor(const CVehicle* vehicle, int32 doorId) {
    auto& veh = const_cast<CVehicle&>(*vehicle); // TODO: Fix
    return !veh.IsDoorMissingU32((uint32)doorId)
        && !veh.IsDoorFullyOpenU32((uint32)doorId)
        && !veh.IsDoorClosedU32((uint32)doorId);
}

// 0x64E550
int32 CCarEnterExit::ComputeDoorFlag(const CVehicle* vehicle, int32 doorId, bool bSettingFlags) {
    if (bSettingFlags && (vehicle->IsBike() || vehicle->m_pHandlingData->m_bTandemSeats)) {
        switch (doorId) {
        case 8:
        case 10:
        case 18:
            return 5;
        case 9:
        case 11:
            return 10;
        default:
            return 0;
        }
    } else {
        switch (doorId) {
        case 8:
            return 4;
        case 9:
            return 8;
        case 10:
        case 18:
            return 1;
        case 11:
            return 2;
        default:
            return 0;
        }
    }
}

// 0x64E610
int32 CCarEnterExit::ComputeOppositeDoorFlag(const CVehicle* vehicle, int32 doorId, bool bCheckVehicleType) {
    if (bCheckVehicleType && (vehicle->IsBike() || vehicle->m_pHandlingData->m_bTandemSeats)) {
        switch (doorId) {
        case 8:
        case 10:
        case 18:
            return 5;
        case 9:
        case 11:
            return 10;
        default:
            return 0;
        }
    } else {
        switch (doorId) {
        case 8:
            return 1;
        case 9:
            return 2;
        case 10:
        case 18:
            return 4;
        case 11:
            return 8;
        default:
            return 0;
        }
    }
}

// 0x64E6D0
bool CCarEnterExit::IsDriverDoorFlag(const CVehicle* vehicle, uint8 flag, bool bSettingFlags) {
    if (bSettingFlags && (vehicle->IsBike() || vehicle->m_pHandlingData->m_bTandemSeats)) {
        switch (flag) {
        case 5:
            return true;
        case 10:
        default:
            return false;
        }
    } else {
        switch (flag) {
        case 1:
            return true;
        case 2:
        case 4:
        case 8:
        default:
            return false;
        }
    }
}

// 0x64F1E0
// ASM Match
int32 CCarEnterExit::ComputePassengerIndexFromCarDoor(const CVehicle* vehicle, int32 doorId) {
    if (vehicle->IsBike() || vehicle->m_pHandlingData->m_bTandemSeats) {
        switch (doorId) {
        case 9:
        case 11:
            return 0;
        default:
            return -1;
        }
    }

    switch (doorId) {
    case 8:
        return 0;
    case 9:
        return 2;
    case 11:
        return 1;
    default:
        return -1;
    }
}

/*!
* @notsa
* @brief Ped sitting behind `doorId`.
*
* `ComputePassengerIndexFromCarDoor` returns `-1` for doors with no passenger seat (`TARGET_DOOR_UNK`, or
* the right hand side of a bike/tandem). The original code indexed `m_apPassengers` with it anyway, which
* lands on `m_pDriver` - it sits right before the array at 0x460. Kept, since callers rely on that value.
*/
CPed* CCarEnterExit::ComputePedInPassengerSeatFromCarDoor(const CVehicle* vehicle, int32 doorId) {
    const auto psgrIdx = ComputePassengerIndexFromCarDoor(vehicle, doorId);
    return psgrIdx < 0 ? vehicle->m_pDriver : vehicle->m_apPassengers[psgrIdx];
}

// 0x64F040
CPed* CCarEnterExit::ComputeQuickJackedPed(const CVehicle* vehicle, int32 doorId) {
    if (doorId == 10) {
        return vehicle->m_pDriver;
    }
    if (doorId == 11) {
        return vehicle->m_apPassengers[1];
    }
    return nullptr;
}

// 0x64F070
CPed* CCarEnterExit::ComputeSlowJackedPed(const CVehicle* vehicle, int32 doorId) {
    if (vehicle->IsBike() || vehicle->m_pHandlingData->m_bTandemSeats) {
        switch (doorId) {
        case 8:
        case 10:
        case 18:
            return vehicle->m_pDriver;
        case 9:
        case 11:
            return vehicle->m_apPassengers[0];
        default:
            return nullptr;
        }
    } else {
        switch (doorId) {
        case 8:
            return vehicle->m_apPassengers[0];
        case 9:
            return vehicle->m_apPassengers[2];
        case 10:
            return vehicle->m_pDriver;
        case 11:
            return vehicle->m_apPassengers[1];
        default:
            return nullptr;
        }
    }
}

// 0x64F190
// ASM Match
int32 CCarEnterExit::ComputeTargetDoorToEnterAsPassenger(const CVehicle* vehicle, int32 psgrIdx) {
    if (vehicle->vehicleFlags.bIsBus) {
        return 8;
    }

    switch (psgrIdx) {
    case 0:
        if (vehicle->IsBike() || vehicle->m_pHandlingData->m_bTandemSeats) {
            return 11;
        }
        return 8;
    case 1:
        return 11;
    case 2:
        return 9;
    default:
        return -1;
    }
}

// 0x64F110
// ASM Match
int32 CCarEnterExit::ComputeTargetDoorToExit(const CVehicle* vehicle, const CPed* ped) {
    if (vehicle->m_pDriver == ped) {
        return 10;
    }

    if (vehicle->GetAnimGroupId() == ANIM_GROUP_BUSCARANIMS || vehicle->GetAnimGroupId() == ANIM_GROUP_COACHCARANIMS) {
        return 8;
    }

    if (vehicle->m_apPassengers[0] == ped) {
        if (vehicle->IsBike() || vehicle->m_pHandlingData->m_bTandemSeats) {
            return 11;
        }
        return 8;
    }

    if (vehicle->m_apPassengers[1] == ped) {
        return 11;
    }

    if (vehicle->m_apPassengers[2] == ped) {
        return 9;
    }

    return -1;
}

// 0x6528F0
bool CCarEnterExit::GetNearestCarDoor(const CPed* ped, const CVehicle* vehicle, CVector& outPos, int32& doorId) {
    auto driverDraggedOutOffset = vehicle->m_pDriver ? &ms_vecPedQuickDraggedOutCarAnimOffset : nullptr;
    auto psgrDraggedOutOffset   = vehicle->HasPassengerAtSeat(0) ? &ms_vecPedQuickDraggedOutCarAnimOffset : nullptr;

    if ((vehicle->IsBike() && !vehicle->IsSubBMX()) || vehicle->IsSubQuad()) {
        driverDraggedOutOffset = nullptr;
        psgrDraggedOutOffset = nullptr;

        if (ped->GetTaskManager().GetActiveTask()->GetTaskType() != TASK_COMPLEX_ENTER_CAR_AS_PASSENGER) {
            if (std::abs(vehicle->GetRight().z) < 0.1f) { // Isn't on it's side
                // Angle of the ped around the vehicle, relative to the vehicle's heading
                const auto pedToVeh = ped->GetPosition() - vehicle->GetPosition();
                auto angle = std::atan2(-pedToVeh.x, pedToVeh.y) - vehicle->GetHeading();
                if (angle > PI) {
                    angle -= TWO_PI;
                } else if (angle < -PI) {
                    angle += TWO_PI;
                }

                // Within 30 degrees of the vehicle's front
                if (std::abs(angle) < PI / 6.f
                    && (
                        (ped->IsPlayer() && ped->GetPlayerData()->m_fMoveBlendRatio > 1.5f && doorId == 0)
                        || (!ped->IsPlayer() && ped->m_nPedType != PED_TYPE_COP && ped->m_nMoveState == PEDMOVE_RUN && ped->m_pPedStats->m_nTemper > 65 && doorId == 0)
                        || doorId == 18
                    )
                    // 18 here is probably either from eBikeNodes or eQuadNodes, not sure?
                    && IsRoomForPedToLeaveCar(vehicle, 18)
                ) {
                    doorId = 18;
                    outPos = GetPositionToOpenCarDoor(vehicle, 18);
                    return true;
                }
            }
        }
    } else if (vehicle->vehicleFlags.bIsBus || vehicle->vehicleFlags.bLowVehicle) {
        driverDraggedOutOffset = nullptr;
        psgrDraggedOutOffset = nullptr;
    }


    const auto posDoorFLeft = GetPositionToOpenCarDoor(vehicle, CAR_DOOR_LF);
    const auto posDoorFRight = GetPositionToOpenCarDoor(vehicle, CAR_DOOR_RF);

    CVector2D pedPos2D = ped->GetPosition();
    CVector2D dir2DToDoorFLeft = posDoorFLeft - pedPos2D, dir2DToDoorFRight = posDoorFRight - pedPos2D;

    if (vehicle->m_pVehicleBeingTowed) {
        if (dir2DToDoorFLeft.SquaredMagnitude() < dir2DToDoorFRight.SquaredMagnitude()) {
            if (IsPathToDoorBlockedByVehicleCollisionModel(ped, vehicle->m_pVehicleBeingTowed, posDoorFLeft)) {
                dir2DToDoorFLeft = { 999.90002f, 999.90002f };
            }
        } else {
            if (IsPathToDoorBlockedByVehicleCollisionModel(ped, vehicle->m_pVehicleBeingTowed, posDoorFRight)) {
                dir2DToDoorFRight = { 999.90002f, 999.90002f };
            }
        }
    }

    if (vehicle->m_pHandlingData->m_bForceDoorCheck && vehicle->IsAutomobile()) {
        const auto aut = static_cast<const CAutomobile*>(vehicle);

        // Only vehicles missing one of the front doors take this path, otherwise the generic code below runs
        if (!aut->m_aCarNodes[CAR_DOOR_LF] || !aut->m_aCarNodes[CAR_DOOR_RF]) {
            if (aut->m_aCarNodes[CAR_DOOR_LF] && IsRoomForPedToLeaveCar(vehicle, CAR_DOOR_LF, driverDraggedOutOffset)) {
                doorId = CAR_DOOR_LF;
                outPos = posDoorFLeft;
                return true;
            }

            if (aut->m_aCarNodes[CAR_DOOR_RF] && IsRoomForPedToLeaveCar(vehicle, CAR_DOOR_RF, driverDraggedOutOffset)) {
                doorId = CAR_DOOR_RF;
                outPos = posDoorFLeft; // Original code returns the left door's position here too
                return true;
            }

            return false;
        }
    }

    if (doorId != CAR_NODE_NONE && IsRoomForPedToLeaveCar(vehicle, doorId, driverDraggedOutOffset)) {
        switch (doorId) {
        case CAR_DOOR_LF: {
            doorId = CAR_DOOR_LF;
            outPos = posDoorFLeft;
            return true;
        }
        case CAR_DOOR_RF: {
            doorId = CAR_DOOR_RF;
            outPos = posDoorFRight;
            return true;
        }
        default:
            return false;
        }
    }

    if (!vehicle->m_pDriver
    || (!CPedGroups::AreInSameGroup(ped, vehicle->m_pDriver) && !vehicle->m_pDriver->bDontDragMeOutCar)
    ) {
        if (vehicle->vehicleFlags.bIsBus
         || dir2DToDoorFRight.SquaredMagnitude() > dir2DToDoorFLeft.SquaredMagnitude()
        ) {
            if (IsRoomForPedToLeaveCar(vehicle, CAR_DOOR_LF, driverDraggedOutOffset)) {
                doorId = CAR_DOOR_LF;
                outPos = posDoorFLeft;
                return true;
            }
            if (IsRoomForPedToLeaveCar(vehicle, CAR_DOOR_RF, psgrDraggedOutOffset)) {
                doorId = CAR_DOOR_RF;
                outPos = posDoorFRight;
                return true;
            }
        } else {
            if (IsRoomForPedToLeaveCar(vehicle, CAR_DOOR_RF, psgrDraggedOutOffset)) {
                if ((
                        vehicle->HasPassengerAtSeat(0)
                        && !vehicle->IsBike()
                        && !vehicle->m_pHandlingData->m_bTandemSeats
                        && (CPedGroups::AreInSameGroup(vehicle->m_apPassengers[0], ped) || vehicle->m_apPassengers[0]->bDontDragMeOutCar || vehicle->IsMissionVehicle())
                        && IsRoomForPedToLeaveCar(vehicle, CAR_DOOR_LF, driverDraggedOutOffset)
                    ) || (
                        vehicle->m_nGettingInFlags & 4
                        && IsRoomForPedToLeaveCar(vehicle, CAR_DOOR_LF, driverDraggedOutOffset)
                    )
                ) {
                    doorId = CAR_DOOR_LF;
                    outPos = posDoorFLeft;
                    return true;
                } else {
                    doorId = CAR_DOOR_RF;
                    outPos = posDoorFRight;
                    return true;
                }
            }

            if (IsRoomForPedToLeaveCar(vehicle, CAR_DOOR_LF, driverDraggedOutOffset)) {
                doorId = CAR_DOOR_LF;
                outPos = posDoorFLeft;
                return true;
            }
        }
    }
    return false;
}

// 0x650BB0
bool CCarEnterExit::GetNearestCarPassengerDoor(const CPed* ped, const CVehicle* vehicle, CVector* outVec, int32* doorId, bool CheckIfOccupiedTandemSeat, bool CheckIfDoorIsEnterable, bool CheckIfRoomToGetIn) {
    const auto CanUseDoor = [&](int32 door, uint8 doorFlag, CPed* seatOccupant) {
        return (!CheckIfOccupiedTandemSeat || !seatOccupant)
            && (!CheckIfDoorIsEnterable || (vehicle->m_nGettingInFlags & doorFlag) == 0)
            && (!CheckIfRoomToGetIn || IsRoomForPedToLeaveCar(vehicle, door));
    };

    // Buses and coaches only have the front right door
    if (vehicle->GetAnimGroupId() == ANIM_GROUP_COACHCARANIMS || vehicle->GetAnimGroupId() == ANIM_GROUP_BUSCARANIMS) {
        if (CheckIfDoorIsEnterable && (vehicle->m_nGettingInFlags & 4) != 0) {
            return false;
        }
        if (CheckIfRoomToGetIn && !IsRoomForPedToLeaveCar(vehicle, TARGET_DOOR_FRONT_RIGHT)) {
            return false;
        }
        *outVec = GetPositionToOpenCarDoor(vehicle, TARGET_DOOR_FRONT_RIGHT);
        *doorId = TARGET_DOOR_FRONT_RIGHT;
        return true;
    }

    const CVector2D pedPos = ped->GetPosition();

    CVector posFrontRight{}, posRearLeft{}, posRearRight{};
    CVector2D toFrontRight{ 999.f, 999.f }, toRearLeft{ 999.f, 999.f }, toRearRight{ 999.f, 999.f };
    bool bFoundDoor = false;

    if (vehicle->IsBike() || vehicle->m_pHandlingData->m_bTandemSeats) {
        if (CanUseDoor(TARGET_DOOR_REAR_LEFT, 2, vehicle->m_apPassengers[0])) {
            posRearLeft = GetPositionToOpenCarDoor(vehicle, TARGET_DOOR_REAR_LEFT);
            toRearLeft = CVector2D{ posRearLeft } - pedPos;
            bFoundDoor = true;
        }
        if (CanUseDoor(TARGET_DOOR_REAR_RIGHT, 8, vehicle->m_apPassengers[0])) {
            posRearRight = GetPositionToOpenCarDoor(vehicle, TARGET_DOOR_REAR_RIGHT);
            toRearRight = CVector2D{ posRearRight } - pedPos;
            bFoundDoor = true;
        }
    } else if (CanUseDoor(TARGET_DOOR_FRONT_RIGHT, 4, vehicle->m_apPassengers[0])) {
        posFrontRight = GetPositionToOpenCarDoor(vehicle, TARGET_DOOR_FRONT_RIGHT);
        toFrontRight = CVector2D{ posFrontRight } - pedPos;
        bFoundDoor = true;
    }

    if (vehicle->GetVehicleModelInfo()->GetNumDoors() > 2) {
        // Vehicles with `m_bForceDoorCheck` may be missing their rear doors
        const auto HasRearDoor = [vehicle](eCarNodes node) {
            return !vehicle->m_pHandlingData->m_bForceDoorCheck
                || (vehicle->IsAutomobile() && static_cast<const CAutomobile*>(vehicle)->m_aCarNodes[node]);
        };

        if (HasRearDoor(CAR_DOOR_LR) && CanUseDoor(TARGET_DOOR_REAR_LEFT, 2, vehicle->m_apPassengers[1])) {
            posRearLeft = GetPositionToOpenCarDoor(vehicle, TARGET_DOOR_REAR_LEFT);
            toRearLeft = CVector2D{ posRearLeft } - pedPos;
            bFoundDoor = true;
        }
        if (HasRearDoor(CAR_DOOR_RR) && CanUseDoor(TARGET_DOOR_REAR_RIGHT, 8, vehicle->m_apPassengers[2])) {
            posRearRight = GetPositionToOpenCarDoor(vehicle, TARGET_DOOR_REAR_RIGHT);
            toRearRight = CVector2D{ posRearRight } - pedPos;
            bFoundDoor = true;
        }
    }

    // Pick the closest one. Unused doors stay at 999.0 and never win.
    auto toNearest = toFrontRight;
    *outVec = posFrontRight;
    *doorId = TARGET_DOOR_FRONT_RIGHT;

    if (toRearLeft.SquaredMagnitude() < toFrontRight.SquaredMagnitude()) {
        *doorId = TARGET_DOOR_REAR_LEFT;
        toNearest = toRearLeft;
        *outVec = posRearLeft;
    }

    if (toRearRight.SquaredMagnitude() < toNearest.SquaredMagnitude()) {
        *doorId = TARGET_DOOR_REAR_RIGHT;
        *outVec = posRearRight;
    }

    return bFoundDoor;
}

// 0x64E740
// Originally RVO'd
CVector CCarEnterExit::GetPositionToOpenCarDoor(const CVehicle* vehicle, int32 doorId) {
    const auto mi = vehicle->GetVehicleModelInfo();
    auto& animGroup = CVehicleAnimGroupData::GetVehicleAnimGroup(vehicle->m_pHandlingData->m_nAnimGroup);

    if (!vehicle->IsBike() && !vehicle->m_pHandlingData->m_bTandemSeats) {
        // Original code compares the raw anim group index against an `AssocGroupId`, so this is never true
        const auto wideVehicleXBlendOffset = vehicle->m_pHandlingData->m_nAnimGroup == ANIM_GROUP_VANCARANIMS && (doorId == TARGET_DOOR_REAR_LEFT || doorId == TARGET_DOOR_REAR_RIGHT)
            ? 0.f
            : vehicle->m_pHandlingData->m_fSeatOffsetDistance;

        const auto offsetType = [doorId] {
            switch (doorId) {
            case TARGET_DOOR_DRIVER:
            case TARGET_DOOR_FRONT_RIGHT:
                return ENTER_FRONT;
            case TARGET_DOOR_REAR_LEFT:
            case TARGET_DOOR_REAR_RIGHT:
                return ENTER_REAR;
            case 18:
                return ENTER_BIKE_FRONT;
            default:
                return ENTER_FRONT; // NOTSA: Originally an uninitialised variable, the offset is discarded for these doors anyway
            }
        }();
        auto animOffset = animGroup.ComputeAnimDoorOffsets(offsetType);

        CVector localSeatPos;
        switch (doorId) {
        case TARGET_DOOR_DRIVER:
            localSeatPos = mi->GetFrontSeatPosn();
            localSeatPos.x = -(localSeatPos.x + wideVehicleXBlendOffset);
            break;
        case TARGET_DOOR_FRONT_RIGHT:
            localSeatPos = mi->GetFrontSeatPosn();
            localSeatPos.x += wideVehicleXBlendOffset;
            animOffset.x = -animOffset.x;
            break;
        case TARGET_DOOR_REAR_LEFT:
            localSeatPos = mi->GetBackSeatPosn();
            localSeatPos.x = -(localSeatPos.x + wideVehicleXBlendOffset);
            break;
        case TARGET_DOOR_REAR_RIGHT:
            localSeatPos = mi->GetBackSeatPosn();
            localSeatPos.x += wideVehicleXBlendOffset;
            animOffset.x = -animOffset.x;
            break;
        default:
            localSeatPos = mi->GetFrontSeatPosn();
            animOffset = CVector{};
            break;
        }

        auto posToRunAnim = localSeatPos - animOffset;
        if (vehicle->IsSubMonsterTruck() || (vehicle->m_pHandlingData->m_bIsBig && !vehicle->IsSubPlane())) {
            posToRunAnim.z = 0.95f - const_cast<CVehicle*>(vehicle)->GetHeightAboveRoad();
        }
        return vehicle->m_matrix->TransformVector(posToRunAnim) + vehicle->GetPosition();
    }

    if (doorId == 18) {
        auto animOffset = animGroup.ComputeAnimDoorOffsets(ENTER_BIKE_FRONT);
        animOffset.x = -animOffset.x;
        animOffset.z = -animOffset.z;
        return vehicle->m_matrix->TransformPoint(mi->GetFrontSeatPosn() + animOffset);
    }

    const auto animOffset = animGroup.ComputeAnimDoorOffsets(ENTER_FRONT);
    auto doorOffset = animOffset;
    auto localSeatPos = doorId == TARGET_DOOR_REAR_LEFT || doorId == TARGET_DOOR_REAR_RIGHT
        ? mi->GetBackSeatPosn()
        : mi->GetFrontSeatPosn();
    const auto isRightSide = doorId == TARGET_DOOR_REAR_RIGHT || doorId == TARGET_DOOR_FRONT_RIGHT;

    if (vehicle->IsBike()) {
        if (isRightSide) {
            doorOffset.x = animOffset.x * -1.f;
        }
        CVector out;
        static_cast<CBike*>(const_cast<CVehicle*>(vehicle))->GetCorrectedWorldDoorPosition(out, doorOffset, localSeatPos);
        return out;
    }

    const auto seatOffsetDist = vehicle->m_pHandlingData->m_fSeatOffsetDistance;
    if (isRightSide) {
        doorOffset.x = animOffset.x * -1.f;
        localSeatPos.x += seatOffsetDist;
    } else {
        localSeatPos.x -= seatOffsetDist;
    }
    return vehicle->m_matrix->TransformPoint(localSeatPos - doorOffset);
}

// 0x64EC90
bool CCarEnterExit::IsCarDoorInUse(const CVehicle* vehicle, int32 firstDoorId, int32 secondDoorId) {
    const auto CheckIsDoorInUse = [vehicle](int32 doorId) {
        uint8 flag = 0;
        switch (doorId) {
        case 10:
        case 18:
            flag = 1;
            break;
        case 11:
            flag = 2;
            break;
        case 8:
            flag = 4;
            break;
        case 9:
            flag = 8;
            break;
        default:
            return false;
        }
        return ((vehicle->m_nGettingInFlags & flag) != 0) || ((vehicle->m_nGettingOutFlags & flag) != 0);
    };

    if (firstDoorId != 0 && CheckIsDoorInUse(firstDoorId)) {
        return true;
    }
    if (secondDoorId != 0 && CheckIsDoorInUse(secondDoorId)) {
        return true;
    }
    return false;
}

// 0x64ED90
bool CCarEnterExit::IsCarDoorReady(const CVehicle* vehicle, int32 doorId) {
    // TODO: Make IsDoorReadyU32 a const member function to avoid const_cast
    auto& veh = const_cast<CVehicle&>(*vehicle); // TODO: Fix
    return veh.IsDoorReadyU32((uint32)doorId)
        || veh.IsDoorFullyOpenU32((uint32)doorId);
}

// 0x64EF00
// ASM Match
bool CCarEnterExit::IsCarQuickJackPossible(CVehicle* vehicle, int32 doorId, const CPed* ped) {
    // I think doorId 10 is the driver's door
    //if (doorId == 10 && vehicle->IsAutomobile() && !vehicle->IsDoorMissingU32(doorId) && vehicle->IsDoorClosedU32(doorId)) {
    //    // This does *nothing* - I tried `return vehicle->CanPedOpenLocks(ped);` but that just breaks everything.
    //    // Basically, returning anything but `false` from here breaks the code (in `CTaskComplexEnterCar`)
    //    vehicle->CanPedOpenLocks(ped); 
    //}
    return false;
}

// 0x64EF70
bool CCarEnterExit::IsCarSlowJackRequired(const CVehicle* vehicle, int32 doorId) {
    CPed* pPed = nullptr;
    if (vehicle->IsBike() || vehicle->m_pHandlingData->m_bTandemSeats) {
        switch (doorId) {
        case 8:
        case 10:
        case 18:
            pPed = vehicle->m_pDriver;
            break;
        case 9:
        case 11:
            pPed = vehicle->m_apPassengers[0];
            break;
        default:
            return false;
        }
    } else if (vehicle->GetAnimGroupId() == ANIM_GROUP_BUSCARANIMS || vehicle->GetAnimGroupId() == ANIM_GROUP_COACHCARANIMS) {
        switch (doorId) {
        case 8:
            return false;
        case 10:
            pPed = vehicle->m_pDriver;
            break;
        default:
            return false;
        }
    } else {
        switch (doorId) {
        case 8:
            pPed = vehicle->m_apPassengers[0];
            break;
        case 9:
            pPed = vehicle->m_apPassengers[2];
            break;
        case 10:
            pPed = vehicle->m_pDriver;
            break;
        case 11:
            pPed = vehicle->m_apPassengers[1];
            break;
        default:
            return false;
        }
    }
    return pPed != nullptr;
}

// 0x6509B0
bool CCarEnterExit::IsClearToDriveAway(const CVehicle* vehicle) {
    const auto& vehPos = vehicle->GetPosition();
    const auto  bbSizeY = vehicle->GetColModel()->GetBoundingBox().GetSize().y;
    CEntity* hitEntity{};
    CColPoint hitCP{};
    return !CWorld::ProcessLineOfSight(vehPos + vehicle->GetForward() * (bbSizeY * 0.5f + 1.5f), vehPos,hitCP, hitEntity, true, true, false, false, false, true, true, false) || hitEntity == vehicle;
}

// 0x651210
bool CCarEnterExit::IsPathToDoorBlockedByVehicleCollisionModel(const CPed* ped, const CVehicle* vehicle, const CVector& pos) {
    if (vehicle->GetModelIndex() == eModelID::MODEL_AT400) {
        return false;
    }

    // Test in the vehicle's space, flattened to the ped's height
    const auto vehMatInv = Invert(*vehicle->m_matrix);
    const auto pedPos    = vehMatInv.TransformPoint(ped->GetPosition());
    const auto doorPos   = vehMatInv.TransformPoint(pos);
    const CColLine line{ pedPos, CVector{ doorPos.x, doorPos.y, pedPos.z } };

    for (const auto& sp : vehicle->GetColModel()->GetData()->GetSpheres()) {
        const CColSphere flatSphere{ CVector{ sp.m_vecCenter.x, sp.m_vecCenter.y, pedPos.z }, sp.m_fRadius };
        if (CCollision::TestLineSphere(line, flatSphere)) {
            return true;
        }
    }

    return false;
}

// 0x64EEE0
// ASM Match
bool CCarEnterExit::IsPedHealthy(CPed* ped) {
    return ped->m_fHealth > 0.f;
}

// 0x64F240
bool CCarEnterExit::IsPlayerToQuitCarEnter(const CPed* ped, const CVehicle* vehicle, int32 startTime, CTask* task) {
    CPad* pad = static_cast<const CPlayerPed*>(ped)->GetPadFromPlayer();
    float taskHeading = ped->m_fCurrentHeading;
    bool bAtTheCar = false;

    if (task) {
        switch (task->GetTaskType()) {
        case TASK_COMPLEX_LEAVE_CAR:
        case TASK_SIMPLE_CAR_OPEN_DOOR_FROM_OUTSIDE:
        case TASK_SIMPLE_CAR_OPEN_LOCKED_DOOR_FROM_OUTSIDE:
        case TASK_SIMPLE_BIKE_PICK_UP:
        case TASK_SIMPLE_CAR_QUICK_DRAG_PED_OUT:
        case TASK_SIMPLE_CAR_SLOW_DRAG_PED_OUT:
            bAtTheCar = true;
            [[fallthrough]];
        case TASK_SIMPLE_STAND_STILL:
        case TASK_COMPLEX_FALL_AND_GET_UP:
        case TASK_SIMPLE_CAR_ALIGN:
        case TASK_SIMPLE_CAR_CLOSE_DOOR_FROM_INSIDE:
        case TASK_SIMPLE_CAR_GET_IN:
        case TASK_SIMPLE_CAR_SHUFFLE:
        case TASK_SIMPLE_CAR_SET_PED_IN_AS_DRIVER:
        case TASK_SIMPLE_CAR_SET_PED_OUT:
        case TASK_SIMPLE_WAIT_UNTIL_PED_OUT_CAR: {
            // Face away from the side of the vehicle the ped is on
            taskHeading = DotProduct(ped->GetPosition() - vehicle->GetPosition(), vehicle->GetRight()) > 0.f
                ? vehicle->GetHeading() + HALF_PI
                : vehicle->GetHeading() - HALF_PI;
            if (vehicle->GetUp().z < 0.f) {
                taskHeading += PI;
                if (taskHeading > PI) {
                    taskHeading -= TWO_PI;
                }
            }
            if (taskHeading > PI) {
                taskHeading -= TWO_PI;
            } else if (taskHeading < -PI) {
                taskHeading += TWO_PI;
            }
            break;
        }
        default:
            break;
        }
    }

    if (vehicle->m_pFire) {
        return true;
    }

    if (pad->ArePlayerControlsDisabled()) {
        return false;
    }

    if (bAtTheCar) {
        if (pad->MeleeAttackJustDown(false)) {
            return true;
        }
    } else if ((float)(CTimer::GetTimeInMS() - startTime) <= ms_fPlayerMinQuitTime) {
        return false;
    }

    const auto walkUpDown    = (float)pad->GetPedWalkUpDown();
    const auto walkLeftRight = (float)pad->GetPedWalkLeftRight();

    auto inputDirn = CGeneral::GetRadianAngleBetweenPoints(0.f, 0.f, -walkLeftRight, walkUpDown) - TheCamera.m_fOrientation;
    const auto inputMag = std::sqrt(walkUpDown * walkUpDown + walkLeftRight * walkLeftRight) / 128.f;

    if (inputDirn > taskHeading + PI) {
        inputDirn -= TWO_PI;
    } else if (inputDirn < taskHeading - PI) {
        inputDirn += TWO_PI;
    }

    return inputMag > 0.75f && std::abs(inputDirn - taskHeading) > PI / 4.f;
}

// 0x6504C0
bool CCarEnterExit::IsRoomForPedToLeaveCar(const CVehicle* vehicle, int32 doorId, const CVector* pos) {
    const auto mi = vehicle->GetVehicleModelInfo();

    CVector localSeatPos;
    if (!vehicle->IsBike() && !vehicle->m_pHandlingData->m_bTandemSeats) {
        switch (doorId) {
        case TARGET_DOOR_DRIVER:
            localSeatPos = mi->GetFrontSeatPosn();
            localSeatPos.x = -localSeatPos.x;
            break;
        case TARGET_DOOR_FRONT_RIGHT:
            localSeatPos = mi->GetFrontSeatPosn();
            break;
        case TARGET_DOOR_REAR_LEFT:
            localSeatPos = mi->GetBackSeatPosn();
            localSeatPos.x = -localSeatPos.x;
            break;
        case TARGET_DOOR_REAR_RIGHT:
            localSeatPos = mi->GetBackSeatPosn();
            break;
        default:
            return false;
        }
    } else {
        localSeatPos = doorId == TARGET_DOOR_REAR_LEFT || doorId == TARGET_DOOR_REAR_RIGHT
            ? mi->GetBackSeatPosn()
            : mi->GetFrontSeatPosn();
        if (doorId == TARGET_DOOR_DRIVER || doorId == TARGET_DOOR_REAR_LEFT) {
            localSeatPos.x = -localSeatPos.x;
        }
    }

    auto seatPos = vehicle->m_matrix->TransformPoint(localSeatPos);
    auto exitPos = GetPositionToOpenCarDoor(vehicle, doorId);

    if (pos) {
        auto carJackOffset = *pos;
        if (doorId == TARGET_DOOR_FRONT_RIGHT || doorId == TARGET_DOOR_REAR_RIGHT) {
            carJackOffset.x = -carJackOffset.x;
        }
        exitPos += vehicle->m_matrix->TransformVector(carJackOffset);
    }

    if (vehicle->GetUp().z < 0.f) {
        seatPos.z += 0.5f;
        exitPos.z += 0.5f;
    }

    CVector extendedExitPos;
    if (vehicle->IsBike()) {
        exitPos.z += 0.2f;
        extendedExitPos = exitPos;
        exitPos.z += 0.35f;
    } else {
        auto diff = exitPos - seatPos;
        diff.z = 0.f;
        const auto length = diff.Magnitude();
        extendedExitPos = seatPos + diff * ((length + 0.35f) / length);
        extendedExitPos.z = exitPos.z;
    }

    if (!CWorld::GetIsLineOfSightClear(seatPos, extendedExitPos, true, false, false, true, false, false, false)) {
        return false;
    }

    if (const auto hitEntity = CWorld::TestSphereAgainstWorld(exitPos, 0.35f, const_cast<CVehicle*>(vehicle), !vehicle->IsTrain(), true, false, true, false, false)) {
        // Peds leaving the AT400 may touch its boarding stairs
        if ((hitEntity->GetModelId() != MODEL_TUGSTAIR || vehicle->GetModelId() != MODEL_AT400) && hitEntity != vehicle->m_pAttachedTo) {
            return false;
        }
    }

    CColPoint cp{};
    CEntity* hitEntity{};
    const auto hitCeiling = CWorld::ProcessVerticalLine(exitPos, 1000.f, cp, hitEntity, true, false, false, true, false);
    const auto zCeiling   = cp.m_vecPoint.z;
    if (hitCeiling && zCeiling > exitPos.z && zCeiling < exitPos.z + 0.6f) {
        return false;
    }

    float zFloor;
    if (vehicle->IsBoat() || notsa::contains({ MODEL_SKIMMER, MODEL_VORTEX, MODEL_SEASPAR, MODEL_LEVIATHN }, vehicle->GetModelId())) {
        zFloor = zCeiling - 1.f;
    } else {
        if (!CWorld::ProcessVerticalLine(exitPos, -1000.f, cp, hitEntity, true, false, false, true, false)) {
            return false;
        }
        zFloor = cp.m_vecPoint.z;
    }

    return zCeiling == 0.f || zCeiling >= zFloor;
}

// 0x64EEC0
// ASM Match
bool CCarEnterExit::IsVehicleHealthy(const CVehicle* vehicle) {
    return vehicle->GetStatus() != STATUS_WRECKED;
}

// 0x6510D0
// ASM Match
bool CCarEnterExit::IsVehicleStealable(const CVehicle* vehicle, const CPed* ped) {
    if (vehicle->GetVehicleType() == VEHICLE_TYPE_PLANE || vehicle->GetVehicleType() == VEHICLE_TYPE_HELI) {
        return false;
    }

    if (vehicle->GetBaseVehicleType() != VEHICLE_TYPE_AUTOMOBILE && vehicle->GetBaseVehicleType() != VEHICLE_TYPE_BIKE) {
        return false;
    }

    if (vehicle->GetCreatedBy() != RANDOM_VEHICLE && vehicle->GetCreatedBy() != PARKED_VEHICLE) {
        if (ped->m_pMyVehicle != vehicle) {
            return false;
        }
    }

    if (CUpsideDownCarCheck{}.IsCarUpsideDown(vehicle)) {
        return false;
    }

    if (!vehicle->CanBeDriven()) {
        return false;
    }

    if (vehicle->IsLawEnforcementVehicle()) {
        return false;
    }

    if (const auto drvr = vehicle->m_pDriver) {
        if (   drvr->IsCreatedByMission()
            || drvr->IsPlayer()
            || ped->GetIntelligence()->IsFriendlyWith(*drvr)
            || CPedGroups::AreInSameGroup(ped, drvr)
        ) {
            return false;
        }
    }

    if (const auto grp = ped->GetGroup()) {
        if (grp->IsAnyoneUsingCar(vehicle)) {
            return false;
        }
    }

    if (vehicle->m_pFire) {
        return false;
    }

    if (vehicle->m_fHealth <= 600.f) {
        return false;
    }

    if (vehicle->IsUpsideDown() || vehicle->IsOnItsSide()) {
        return false;
    }

    if (!IsClearToDriveAway(vehicle)) {
        return false;
    }

    return true;
}

// 0x64F600
void CCarEnterExit::MakeUndraggedDriverPedLeaveCar(const CVehicle* vehicle, const CPed* pedGettingIn) {
    auto& veh = const_cast<CVehicle&>(*vehicle); // TODO: Fix
    auto& ped = const_cast<CPed&>(*pedGettingIn); // TODO: Fix
    veh.m_pDriver->GetEventGroup().Add(CEventDraggedOutCar{ &veh, &ped, true });
}

// 0x64F540
// ASM Match
void CCarEnterExit::MakeUndraggedPassengerPedsLeaveCar(const CVehicle* targetVehicle, const CPed* draggedPed, const CPed* ped) {
    for (int32 i = 0; i < targetVehicle->m_nMaxPassengers; i++) {
        CPed* passenger = targetVehicle->m_apPassengers[i];
        if (passenger && passenger != draggedPed && !passenger->bStayInCarOnJack) {
            int32 door = ComputeTargetDoorToExit(targetVehicle, passenger);
            CEventPedEnteredMyVehicle event(const_cast<CPed*>(ped), const_cast<CVehicle*>(targetVehicle), static_cast<eTargetDoor>(door));
            passenger->GetEventGroup().Add(&event, false);
        }
    }
}

// 0x650130
void CCarEnterExit::QuitEnteringCar(CPed* ped, CVehicle* vehicle, int32 doorId, bool bCarWasBeingJacked) {
    RemoveGetInAnims(ped);
    ped->RestartNonPartialAnims();
    if (!RpAnimBlendClumpGetAssociation(ped->GetRpClump(), ANIM_ID_IDLE)) {
        CAnimManager::BlendAnimation(ped->GetRpClump(), ped->m_nAnimGroup, ANIM_ID_IDLE, 1000.0f);
    }

    if (bCarWasBeingJacked) {
        vehicle->vehicleFlags.bIsBeingCarJacked = false;
    }
    if (vehicle->m_nNumGettingIn > 0) {
        vehicle->m_nNumGettingIn--;
    }

    if (vehicle->IsBike() || vehicle->m_pHandlingData->m_bTandemSeats) {
        if (doorId == 8 || doorId == 10) {
            vehicle->ClearGettingInFlags(5);
        } else if (doorId == 9 || doorId == 11) {
            vehicle->ClearGettingInFlags(10);
        }
        if (vehicle->IsBike()) {
            static_cast<CBike*>(vehicle)->m_nBikeFlags.bGettingPickedUp = false;
        }
    } else {
        switch (doorId) {
        case 8:
            vehicle->ClearGettingInFlags(4);
            break;
        case 9:
            vehicle->ClearGettingInFlags(8);
            break;
        case 10:
            vehicle->ClearGettingInFlags(vehicle->m_nMaxPassengers != 0 ? 1 : 3);
            break;
        case 11:
            vehicle->ClearGettingInFlags(vehicle->m_nMaxPassengers != 0 ? 2 : 3);
            break;
        }
    }
    ped->SetUsesCollision(true);
}

// 0x64F680
// ASM Match
void CCarEnterExit::RemoveCarSitAnim(const CPed* ped) {
    for (auto anim = RpAnimBlendClumpGetFirstAssociation(ped->GetRpClump(), ANIMATION_SECONDARY_TASK_ANIM); anim; anim = RpAnimBlendGetNextAssociation(anim, ANIMATION_SECONDARY_TASK_ANIM)) {
        anim->SetFlag(ANIMATION_IS_BLEND_AUTO_REMOVE);
        anim->m_BlendDelta = -1000.f;
    }
    CAnimManager::BlendAnimation(ped->GetRpClump(), ped->m_nAnimGroup, ANIM_ID_IDLE, 1000.0);
}

// 0x64F6E0
// ASM Match
void CCarEnterExit::RemoveGetInAnims(const CPed* ped) {
    for (auto anim = RpAnimBlendClumpGetFirstAssociation(ped->GetRpClump(), ANIMATION_IS_PARTIAL); anim; anim = RpAnimBlendGetNextAssociation(anim, ANIMATION_IS_PARTIAL)) {
        anim->SetFlag(ANIMATION_IS_BLEND_AUTO_REMOVE);
        anim->m_BlendDelta = -1000.f;
    }
}

// 0x64F860
void CCarEnterExit::SetAnimOffsetForEnterOrExitVehicle() {
    if (ms_bPedOffsetsCalculated) {
        return;
    }

    const auto animBlockIdxs = {
        CAnimManager::GetAnimationBlockIndex("int_house"),
        CAnimManager::GetAnimationBlockIndex("int_office")
    };

    for (const auto idx : animBlockIdxs) {
        CStreaming::RequestModel(IFPToModelId(idx), STREAMING_KEEP_IN_MEMORY);
    }
    CStreaming::LoadAllRequestedModels(false);

    for (const auto idx : animBlockIdxs) {
        CAnimManager::AddAnimBlockRef(idx);
    }

    {
        const auto anim = CAnimManager::GetAnimAssociation(ANIM_GROUP_DEFAULT, ANIM_ID_GETUP_0);
        CAnimManager::UncompressAnimation(anim->m_BlendHier);
        const auto& seq = anim->m_BlendHier->GetSequences()[0];
        ms_vecPedGetUpAnimOffset = seq.m_FramesNum ? seq.GetUKeyFrame(0)->Trans : CVector{};
    }

    ms_vecPedQuickDraggedOutCarAnimOffset = CVector{ -1.841797f, -0.3261719f, -0.01269531f };

    const std::tuple<AssocGroupId, AnimationId, CVector*> toProcess[]{
        {ANIM_GROUP_INT_HOUSE,  ANIM_ID_BED_IN_L,   &ms_vecPedBedLAnimOffset  },
        {ANIM_GROUP_INT_HOUSE,  ANIM_ID_BED_IN_R,   &ms_vecPedBedRAnimOffset  },
        {ANIM_GROUP_INT_OFFICE, ANIM_ID_OFF_SIT_IN, &ms_vecPedDeskAnimOffset  },
        {ANIM_GROUP_INT_HOUSE,  ANIM_ID_LOU_IN,     &ms_vecPedChairAnimOffset },
    };
    for (const auto [grpId, animId, out] : toProcess) {
        // Calculate translation delta between first and last sequence frames
        *out = [grpId, animId] {
            const auto anim = CAnimManager::GetAnimAssociation(grpId, animId);
            CAnimManager::UncompressAnimation(anim->m_BlendHier);
            const auto& seq = anim->m_BlendHier->GetSequences()[0];
            if (seq.m_FramesNum > 0) {
                return seq.GetUKeyFrame(seq.m_FramesNum - 1)->Trans - seq.GetUKeyFrame(0)->Trans;
            }
            return CVector{};
        }();
    }

    for (const auto idx : animBlockIdxs) {
        CAnimManager::RemoveAnimBlockRef(idx);
    }

    ms_bPedOffsetsCalculated = true;
}

// 0x650280
bool CCarEnterExit::SetPedInCarDirect(CPed* ped, CVehicle* vehicle, int32 doorId, bool bAsDriver) {
    if (bAsDriver) {
        // Warp ped into vehicle
        CTaskSimpleCarSetPedInAsDriver task{ vehicle };
        task.m_bWarpingInToCar = true;
        task.ProcessPed(ped);

        // And make them drive
        ped->GetTaskManager().SetTask(new CTaskComplexCarDriveWander{ vehicle, vehicle->m_autoPilot.DrivingMode, (float)vehicle->m_autoPilot.CruiseSpeed }, TASK_PRIMARY_PRIMARY);

        return true;
    }

    // Warp ped into vehicle
    {
        CTaskSimpleCarSetPedInAsPassenger task{ vehicle, (eTargetDoor)doorId };
        task.m_bWarpingInToCar = true;
        task.ProcessPed(ped);
    }

    if (vehicle->IsBike()) {
        ped->GetTaskManager().SetTask(new CTaskComplexCarDrive{ vehicle, false }, TASK_PRIMARY_PRIMARY);
    }

    // Set mutal acquaintance respect between the ped and all other occupants up to the ped's seat
    // I assume the function is only ever called with `bAsDriver` if there are no passengers
    // So that's why this code-path is only reachable if `bAsDriver` is false

    const auto SetMutalAcquaintanceWith = [ped](CPed* other) {
        if (other) {
            const auto SetWith = [](CPed* of, CPed* with) {
                if (!of->IsCreatedByMission()) {
                    of->GetAcquaintance().SetAsAcquaintance(ACQUAINTANCE_RESPECT, CPedType::GetPedFlag(with->m_nPedType));
                }
            };
            SetWith(ped, other);
            SetWith(other, ped);
        }
    };

    SetMutalAcquaintanceWith(vehicle->m_pDriver);

    // Set with all other passengers up to the ped's seat (none if the door has no passenger seat)
    const auto psgrIdx = ComputePassengerIndexFromCarDoor(vehicle, doorId);
    rng::for_each(vehicle->GetPassengers() | rng::views::take((size_t)std::max(psgrIdx, 0)), SetMutalAcquaintanceWith);

    return true;
}
