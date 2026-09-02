/*
    Plugin-SDK file
    Authors: GTA Community. See more here
    https://github.com/DK22Pac/plugin-sdk
    Do not delete this comment block. Respect others' work!
*/
#include "StdInc.h"

#include "Bike.h"

#include "Buoyancy.h"
#include "CarCtrl.h"
#include "Explosion.h"
#include "Darkel.h"
#include "VehicleRecording.h"
#include "VisibilityPlugins.h"
#include "Camera.h"
#include "GameLogic.h"
#include "Enums/eControllerType.h"
#include "Enums/eSurfaceType.h"
#include "Enums/eVehicleHandlingFlags.h"
#include "Coronas.h"
#include "PointLights.h"
#include "Shadows.h"
#include "TimeCycle.h"
#include "ModelInfo.h"
#include "Replay.h"
#include "CullZones.h"
#include "FxManager.h"
#include "EventVehicleOnFire.h"
#include "Cheat.h"

void CBike::InjectHooks() {
    RH_ScopedVirtualClass(CBike, 0x871360, 67);
    RH_ScopedCategory("Vehicle");

    RH_ScopedInstall(Constructor, 0x6BF430);
    RH_ScopedInstall(Destructor, 0x6B57A0);
    RH_ScopedInstall(dmgDrawCarCollidingParticles, 0x6B5A00);
    RH_ScopedInstall(DamageKnockOffRider, 0x6B5A10);
    RH_ScopedInstall(KnockOffRider, 0x6B5F40);
    RH_ScopedInstall(SetRemoveAnimFlags, 0x6B5F50);
    RH_ScopedInstall(ReduceHornCounter, 0x6B5F90);
    RH_ScopedInstall(ProcessAI, 0x6BC930);
    RH_ScopedInstall(ProcessBuoyancy, 0x6B5FB0);
    RH_ScopedInstall(ResetSuspension, 0x6B6740);
    RH_ScopedInstall(GetAllWheelsOffGround, 0x6B6790);
    RH_ScopedInstall(DebugCode, 0x6B67A0);
    RH_ScopedInstall(DoSoftGroundResistance, 0x6B6D40);
    RH_ScopedInstall(PlayHornIfNecessary, 0x6B7130);
    RH_ScopedInstall(CalculateLeanMatrix, 0x6B7150);
    RH_ScopedInstall(ProcessRiderAnims, 0x6B7280);
    RH_ScopedInstall(FixHandsToBars, 0x6B7F90, { .reversed = false });
    RH_ScopedInstall(PlaceOnRoadProperly, 0x6BEEB0);
    RH_ScopedInstall(GetCorrectedWorldDoorPosition, 0x6BF230);
    RH_ScopedVMTInstall(Fix, 0x6B7050);
    RH_ScopedVMTInstall(BlowUpCar, 0x6BEA10);
    RH_ScopedVMTInstall(ProcessDrivingAnims, 0x6BF400);
    RH_ScopedVMTInstall(BurstTyre, 0x6BEB20);
    RH_ScopedVMTInstall(ProcessControlInputs, 0x6BE310);
    RH_ScopedVMTInstall(ProcessEntityCollision, 0x6BDEA0);
    RH_ScopedVMTInstall(Render, 0x6BDE20);
    RH_ScopedVMTInstall(PreRender, 0x6BD090);
    RH_ScopedVMTInstall(Teleport, 0x6BCFC0);
    RH_ScopedVMTInstall(ProcessControl, 0x6B9250);
    RH_ScopedVMTInstall(VehicleDamage, 0x6B8EC0);
    RH_ScopedVMTInstall(SetupSuspensionLines, 0x6B89B0);
    RH_ScopedVMTInstall(SetModelIndex, 0x6B8970);
    RH_ScopedVMTInstall(PlayCarHorn, 0x6B7080);
    RH_ScopedVMTInstall(SetupDamageAfterLoad, 0x6B7070);
    RH_ScopedVMTInstall(DoBurstAndSoftGroundRatios, 0x6B6950);
    RH_ScopedVMTInstall(SetUpWheelColModel, 0x6B67E0);
    RH_ScopedVMTInstall(RemoveRefsToVehicle, 0x6B67B0);
    RH_ScopedVMTInstall(ProcessControlCollisionCheck, 0x6B6620);
    RH_ScopedVMTInstall(GetComponentWorldPosition, 0x6B5990);
    RH_ScopedVMTInstall(ProcessOpenDoor, 0x6B58D0);
}

// 0x6BF430
CBike::CBike(int32 modelIndex, eVehicleCreatedBy createdBy) :
    CVehicle(createdBy) {
    auto mi = CModelInfo::GetModelInfo(modelIndex)->AsVehicleModelInfoPtr();
    if (mi->m_nVehicleType == VEHICLE_TYPE_BIKE) {
        const auto& animationStyle = CAnimManager::GetAnimBlocks()[mi->GetAnimFileIndex()].GroupId;
        m_RideAnimData.AnimGroup   = animationStyle;
        if (animationStyle < ANIM_GROUP_BIKES || animationStyle > ANIM_GROUP_WAYFARER) {
            m_RideAnimData.AnimGroup = ANIM_GROUP_BIKES;
        }
    }

    m_nVehicleSubType = VEHICLE_TYPE_BIKE;
    m_nVehicleType    = VEHICLE_TYPE_BIKE;

    m_BlowUpTimer     = 0.0f;
    m_nBrakesOn       = false;
    nBikeFlags        = 0;
    SetModelIndex(modelIndex);

    m_pHandlingData          = gHandlingDataMgr.GetVehiclePointer(mi->m_nHandlingId);
    m_BikeHandling           = gHandlingDataMgr.GetBikeHandlingPointer(mi->m_nHandlingId);
    m_nHandlingFlagsIntValue = m_pHandlingData->m_nHandlingFlags;
    m_pFlyingHandlingData    = gHandlingDataMgr.GetFlyingPointer(static_cast<uint8>(mi->m_nHandlingId));
    m_fBrakeCount            = 20.0f;
    mi->ChooseVehicleColour(m_nPrimaryColor, m_nSecondaryColor, m_nTertiaryColor, m_nQuaternaryColor, 1);
    m_fSwingArmLength       = 0.0f;
    m_fForkYOffset          = 0.0f;
    m_fForkZOffset          = 0.0f;
    m_nFixLeftHand          = false;
    m_nFixRightHand         = false;
    m_fSteerAngleTan        = std::tan(DegreesToRadians(mi->m_fBikeSteerAngle));
    m_fMass                 = m_pHandlingData->m_fMass;
    m_fTurnMass             = m_pHandlingData->m_fTurnMass;
    m_vecCentreOfMass       = m_pHandlingData->m_vecCentreOfMass;
    m_vecCentreOfMass.z     = 0.1f;
    m_fAirResistance        = GetDefaultAirResistance();
    m_fElasticity           = 0.05f;
    m_fBuoyancyConstant     = m_pHandlingData->m_fBuoyancyConstant;
    m_fSteerAngle           = 0.0f;
    m_GasPedal              = 0.0f;
    m_BrakePedal            = 0.0f;
    m_Damager               = nullptr;
    m_pWhoInstalledBombOnMe = nullptr;
    m_GasPedalAudioRevs     = 0.0f;
    m_fTyreTemp             = 1.0f;
    m_fBrakingSlide         = 0.0f;
    m_PrevSpeed             = 0.0f;

    for (auto i = 0; i < 2; ++i) {
        m_nWheelStatus[i]            = 0;
        m_aWheelSkidmarkType[i]      = eSkidmarkType::DEFAULT;
        m_bWheelBloody[i]            = false;
        m_bMoreSkidMarks[i]          = false;
        m_aWheelPitchAngles[i]       = 0.0f;
        m_aWheelAngularVelocity[i]   = 0.0f;
        m_aWheelSuspensionHeights[i] = 0.0f;
        m_aWheelOrigHeights[i]       = 0.0f;
        m_WheelStates[i]             = WHEEL_STATE_NORMAL;
    }

    for (auto i = 0; i < 4; ++i) {
        m_aWheelColPoints[i]     = {};
        m_aWheelRatios[i]        = 1.0f;
        m_aRatioHistory[i]       = 0.0f;
        m_WheelCounts[i]         = 0.0f;
        m_fSuspensionLength[i]   = 0.0f;
        m_fLineLength[i]         = 0.0f;
        m_aGroundPhysicalPtrs[i] = nullptr;
        m_aGroundOffsets[i]      = CVector{};
    }

    m_nNoOfContactWheels              = 0;
    m_NumDriveWheelsOnGround          = 0;
    m_NumDriveWheelsOnGroundLastFrame = 0;
    m_fHeightAboveRoad                = 0.0f;
    m_fExtraTractionMult              = 1.0f;

    if (!mi->m_pColModel->m_pColData->m_pLines) {
        mi->m_pColModel->m_pColData->m_nNumLines              = 4;
        mi->m_pColModel->m_pColData->m_pLines                 = static_cast<CColLine*>(CMemoryMgr::Malloc(4 * sizeof(CColLine)));
        mi->m_pColModel->m_pColData->m_pLines[1].m_vecStart.x = 99'999.99f; // todo: explain this
    }
    mi->m_pColModel->m_pColData->m_pLines[0].m_vecStart.z = 99'999.99f;
    CBike::SetupSuspensionLines();

    m_autoPilot.TempAction = TEMPACT_NONE;
    m_autoPilot.SetCarMission(MISSION_NONE, 0);
    m_autoPilot.AvoidLevelTransitions = false;

    SetStatus(STATUS_SIMPLE);
    m_nNumPassengers         = 0;
    vehicleFlags.bLowVehicle = false;
    vehicleFlags.bIsBig      = false;
    vehicleFlags.bIsVan      = false;

    m_bLeanMatrixCalculated  = false;
    m_mLeanMatrix            = *m_matrix;
    m_vecOldSpeedForPlayback = CVector{};
    m_vehicleAudio.Initialise(this);
}

// 0x6B57A0
CBike::~CBike() {
    m_vehicleAudio.Terminate();
}

// 0x6B5A00
void CBike::dmgDrawCarCollidingParticles(const CVector& position, float power, eWeaponType weaponType) {
    // NOP
}

// 0x6B5A10
bool CBike::DamageKnockOffRider(CVehicle* vehicle, float damageIntensity, uint16 pieceType, CEntity* damager, const CVector& collisionPos, const CVector& collisionImpactVelocity) {
    const auto driver    = vehicle->m_pDriver;
    const auto passenger = vehicle->m_apPassengers[0];

    // Impact force relative to the bike's mass
    auto force = damageIntensity / vehicle->m_fMass * 800.0f;

    // A skilled rider resists being knocked off (unless flagged to always come off)
    if (vehicle->GetStatus() != STATUS_PLAYER) {
        if (driver && driver->CantBeKnockedOffBike != CANT_BE_KNOCKED_OFF_ALWAYS_NORMAL) {
            force *= 1.0f - driver->GetBikeRidingSkill() * 0.6f;
        }
    } else {
        force *= 0.75f;
        if (driver) {
            force *= 1.0f - driver->GetBikeRidingSkill() * 0.5f;
        }
    }

    // Only an actual driver gets knocked off
    if (!driver || !driver->IsStateDriving() || force <= 10.0f) {
        return false;
    }

    // A ped already reacting to a hit isn't also knocked off (cops are exempt)
    if (const auto task = driver->GetIntelligence()->GetTaskManager().GetActiveTask()) {
        if (task->GetTaskType() == TASK_SIMPLE_BE_HIT && !driver->IsCop()) {
            return false;
        }
    }

    const auto impactFwdMag   = vehicle->GetForward().Dot(collisionImpactVelocity);
    const auto impactUpMag    = vehicle->GetUp().Dot(collisionImpactVelocity);
    const auto impactRightMag = vehicle->GetRight().Dot(collisionImpactVelocity);

    // Per-axis weighting of the impact
    auto fwdWeight = 0.6f;
    if (std::abs(impactFwdMag) > 0.85f) {
        const auto vertical = collisionImpactVelocity.z < 0.85f ? 0.0f : collisionImpactVelocity.z;
        fwdWeight           = 7.0f * sq(vertical) + 0.6f;
    }
    if (vehicle->GetUp().z < 0.0f) { // bike lying on its side / upside down
        fwdWeight = 5.0f;
    }

    auto backWeight = 1.5f;
    auto upWeight   = 0.05f;
    if (vehicle->m_nModelIndex == MODEL_SANCHEZ) {
        fwdWeight *= 0.65f;
        upWeight *= 0.75f;
    } else if (vehicle->IsSubQuad()) {
        backWeight = 3.0f;
        fwdWeight *= 0.65f;
        upWeight *= 0.75f;
    }

    if (impactFwdMag > 0.0f) {
        fwdWeight *= 1.0f - driver->GetBikeRidingSkill() * 0.6f;
    }

    force *= std::abs(impactFwdMag) * fwdWeight
        + std::max(impactUpMag, 0.0f) * upWeight
        + std::abs(impactRightMag) * 0.45f
        - std::min(impactUpMag, 0.0f) * backWeight;

    // Don't knock the player off while they're on stairs
    if (driver->IsPlayer() && CCullZones::CamStairsForPlayer() && CCullZones::FindZoneWithStairsAttributeForPlayer()) {
        force = 0.0f;
    }

    // ALWAYS_HARD peds come off at a much lower force threshold
    if (force <= (driver->CantBeKnockedOffBike == CANT_BE_KNOCKED_OFF_ALWAYS_HARD ? 20.0f : 75.0f)) {
        return false;
    }

    // NEVER peds are never knocked off
    if (driver->CantBeKnockedOffBike == CANT_BE_KNOCKED_OFF_NEVER) {
        return false;
    }
    if (passenger && passenger->CantBeKnockedOffBike == CANT_BE_KNOCKED_OFF_NEVER) {
        return false;
    }

    // The driver (guaranteed present here) is thrown off, and so is the passenger, both reacting with the driver's facing
    const auto knockOffDir = (uint8)driver->GetLocalDirection(-CVector2D{ collisionImpactVelocity });

    driver->GetEventGroup().Add(CEventKnockOffBike{ vehicle, vehicle->m_vecMoveSpeed, collisionImpactVelocity, damageIntensity, 0.05f * force, KNOCK_OFF_TYPE_SKIDBACKFRONT, knockOffDir, 0, nullptr, true, false });
    if (passenger) {
        passenger->GetEventGroup().Add(CEventKnockOffBike{ vehicle, vehicle->m_vecMoveSpeed, collisionImpactVelocity, damageIntensity, 0.05f * force, KNOCK_OFF_TYPE_SKIDBACKFRONT, knockOffDir, 0, nullptr, false, false });
    }
    return true;
}

// dummy function
// 0x6B5F40
CPed* CBike::KnockOffRider(eWeaponType arg0, uint8 arg1, CPed* ped, bool arg3) {
    return ped;
}

// 0x6B5F50
void CBike::SetRemoveAnimFlags(CPed* ped) {
    if (ped && ped->GetIsTypePed()) {
        for (auto anim = RpAnimBlendClumpGetFirstAssociation(ped->GetRpClump(), ANIMATION_IS_PARTIAL); anim; anim = RpAnimBlendGetNextAssociation(anim, ANIMATION_IS_PARTIAL)) {
            anim->m_Flags |= ANIMATION_IS_BLEND_AUTO_REMOVE;
        }
    }
}

// 0x6B5F90
void CBike::ReduceHornCounter() {
    if (m_HornCounter) {
        m_HornCounter -= 1;
    }
}

// 0x6B5FB0
void CBike::ProcessBuoyancy() {
    CVector vecBuoyancyTurnPoint;
    CVector vecBuoyancyForce;
    if (!mod_Buoyancy.ProcessBuoyancy(this, m_fBuoyancyConstant, &vecBuoyancyTurnPoint, &vecBuoyancyForce)) {
        vehicleFlags.bIsDrowning        = false;
        physicalFlags.bSubmergedInWater = false;
        physicalFlags.bTouchingWater    = false;
        return;
    }

    physicalFlags.bTouchingWater = true;
    ApplyMoveForce(vecBuoyancyForce);
    ApplyTurnForce(vecBuoyancyForce, vecBuoyancyTurnPoint);

    auto fTimeStep       = std::max(0.01F, CTimer::GetTimeStep());
    auto fUsedMass       = m_fMass / 125.0F;
    auto fBuoyancyForceZ = vecBuoyancyForce.z / (fTimeStep * fUsedMass);

    if (fUsedMass > m_fBuoyancyConstant) {
        fBuoyancyForceZ *= 1.05F * fUsedMass / m_fBuoyancyConstant;
    }

    if (physicalFlags.bMakeMassTwiceAsBig) {
        fBuoyancyForceZ *= 1.5F;
    }

    auto fBuoyancyForceMult = std::max(0.5F, 1.0F - fBuoyancyForceZ / 20.0F);
    auto fSpeedMult         = std::pow(fBuoyancyForceMult, CTimer::GetTimeStep());
    m_vecMoveSpeed *= fSpeedMult;
    m_vecTurnSpeed *= fSpeedMult;

    // 0x6B6443
    if (fBuoyancyForceZ > 0.8F || (fBuoyancyForceZ > 0.4F && IsAnyWheelNotMakingContactWithGround())) {
        vehicleFlags.bIsDrowning        = true;
        physicalFlags.bSubmergedInWater = true;

        m_vecMoveSpeed.z                = std::max(-0.1F, m_vecMoveSpeed.z);

        if (m_pDriver) {
            ProcessPedInVehicleBuoyancy(m_pDriver->AsPed(), true);
        } else {
            vehicleFlags.bEngineOn = false;
        }

        for (const auto passenger : GetPassengers()) {
            ProcessPedInVehicleBuoyancy(passenger, false);
        }
    } else {
        vehicleFlags.bIsDrowning        = false;
        physicalFlags.bSubmergedInWater = false;
    }
}

inline void CBike::ProcessPedInVehicleBuoyancy(CPed* ped, bool bIsDriver) {
    if (!ped) {
        return;
    }

    ped->physicalFlags.bTouchingWater = true;
    if (!ped->IsPlayer() && bikeFlags.bWaterTight) {
        return;
    }

    if (ped->IsPlayer()) {
        ped->AsPlayer()->HandlePlayerBreath(true, 1.0F);
    }

    if (IsAnyWheelMakingContactWithGround()) {
        if (!ped->IsPlayer()) {
            auto pedDamageResponseCalc = CPedDamageResponseCalculator(this, CTimer::GetTimeStep(), eWeaponType::WEAPON_DROWNING, PED_PIECE_TORSO, false);
            auto damageEvent           = CEventDamage(this, CTimer::GetTimeInMS(), eWeaponType::WEAPON_DROWNING, PED_PIECE_TORSO, 0, false, true);
            if (damageEvent.AffectsPed(ped)) {
                pedDamageResponseCalc.ComputeDamageResponse(ped, damageEvent.m_damageResponse, true);
            } else {
                damageEvent.m_damageResponse.m_bDamageCalculated = true;
            }

            ped->GetEventGroup().Add(&damageEvent, false);
        }
    } else {
        auto knockOffBikeEvent = CEventKnockOffBike(this, m_vecMoveSpeed, m_vecLastCollisionImpactVelocity, m_fDamageIntensity, 0.0F, KNOCK_OFF_TYPE_FALL, 0, 0, nullptr, bIsDriver, false);
        ped->GetEventGroup().Add(&knockOffBikeEvent);
        if (bIsDriver) {
            vehicleFlags.bEngineOn = false;
        }
    }
}

// 0x6B6740
void CBike::ResetSuspension() {
    int32 i;
    for (i = 0; i < 2; i++) {
        m_aWheelAngularVelocity[i] = 0.0f;
        m_WheelStates[i]           = WHEEL_STATE_NORMAL;
    }

    for (i = 0; i < 4; i++) {
        m_aWheelRatios[i] = 1.0f;
        m_WheelCounts[i]  = 0.0f;
    }
}

constexpr float BIKE_BALANCE_MOVESPEED_CAP = 0.1f;

// 0x6BC930
bool CBike::ProcessAI(uint32& extraHandlingFlags) {
    auto pModelInfo               = CModelInfo::GetModelInfo(m_nModelIndex)->AsVehicleModelInfoPtr();

    m_autoPilot.SlowingDownForCar = false;
    m_autoPilot.SlowingDownForPed = false;

    if (m_autoPilot.RecordingNumber >= 0 && !CVehicleRecording::bUseCarAI[m_autoPilot.RecordingNumber]) {
        extraHandlingFlags += 2;
        return false;
    }

    switch (GetStatus()) {
    case STATUS_PLAYER: {
        extraHandlingFlags += 2;
        bikeFlags.bGettingPickedUp = false;

        if (FindPlayerPed(-1)->GetPedState() != PEDSTATE_EXIT_CAR && FindPlayerPed(-1)->GetPedState() != PEDSTATE_DRAGGED_FROM_CAR) {
            if (m_pDriver) {
                if (CWorld::Players[0].m_pPed == m_pDriver) {
                    ProcessControlInputs(0);
                } else if (CWorld::Players[1].m_pPed == m_pDriver) {
                    ProcessControlInputs(1);
                }
            }

            float fLeanForce;
            CVector com = GetMatrix().TransformVector(m_vecCentreOfMass);
            if (m_RideAnimData.LeanFwd >= 0.0f) {
                m_vecCentreOfMass.y = m_pHandlingData->m_vecCentreOfMass.y + m_RideAnimData.LeanFwd * m_BikeHandling->m_fLeanBakCOM;
                com = GetMatrix().TransformVector(m_vecCentreOfMass);

                if ((GetBrakePedal() == 0.0f && !vehicleFlags.bIsHandbrakeOn) || !m_nNoOfContactWheels) {
                    float fMoveSpeedMult = std::min<float>(BIKE_BALANCE_MOVESPEED_CAP, m_vecMoveSpeed.Magnitude());
                    fLeanForce           = (std::max<float>(fMoveSpeedMult / (float)BIKE_BALANCE_MOVESPEED_CAP, GetGasPedal()) + GetGasPedal()) * (m_BikeHandling->m_fLeanBakForce * m_fTurnMass * m_RideAnimData.LeanFwd * fMoveSpeedMult) * 0.5f;
                    fLeanForce *= CStats::GetFatAndMuscleModifier(STAT_MOD_11);

                    ApplyTurnForce(GetUp() * -(CTimer::GetTimeStep() * fLeanForce), com + GetForward());
                }
            } else {
                m_vecCentreOfMass.y = m_pHandlingData->m_vecCentreOfMass.y + m_RideAnimData.LeanFwd * m_BikeHandling->m_fLeanFwdCOM;
                com = GetMatrix().TransformVector(m_vecCentreOfMass);

                if (GetBrakePedal() <= 0.0f || !m_nNoOfContactWheels) {
                    float fMoveSpeedMult = std::min<float>(BIKE_BALANCE_MOVESPEED_CAP, m_vecMoveSpeed.Magnitude());
                    fLeanForce           = (std::max<float>(fMoveSpeedMult / (float)BIKE_BALANCE_MOVESPEED_CAP, GetBrakePedal()) + GetBrakePedal()) * (m_BikeHandling->m_fLeanFwdForce * m_fTurnMass * m_RideAnimData.LeanFwd * fMoveSpeedMult) * 0.5f;
                    fLeanForce *= CStats::GetFatAndMuscleModifier(STAT_MOD_11);

                    ApplyTurnForce(GetUp() * -(CTimer::GetTimeStep() * fLeanForce), com + GetForward());
                }
            }

            PruneReferences();
            if (GetStatus() == STATUS_PLAYER) {
                DoDriveByShootings();
            }
            DoSoftGroundResistance(extraHandlingFlags);
        }

        if (CPad::GetPad(0)->CarGunJustDown()) {
            ActivateBomb();
        }
        break;
    }
    case STATUS_PLAYER_PLAYBACK_FROM_BUFFER:
        extraHandlingFlags += 2;
        break;
    case STATUS_SIMPLE: {
        CCarAI::UpdateCarAI(this);
        CPhysical::ProcessControl();
        CCarCtrl::UpdateCarOnRails(this);
        m_NumDriveWheelsOnGroundLastFrame = m_NumDriveWheelsOnGround;
        m_nNoOfContactWheels              = 2;
        m_NumDriveWheelsOnGround          = 2;

        m_pHandlingData->m_transmissionData.CalculateGearForSimpleCar(m_autoPilot.ActualSpeed * (1.0f / 50.0f), m_nCurrentGear);

        m_aWheelPitchAngles[0] += CTimer::GetTimeStep() * CVehicle::ProcessWheelRotation(WHEEL_STATE_NORMAL, GetForward(), m_vecMoveSpeed, pModelInfo->GetWheelSize(true) * 0.5f);
        m_aWheelPitchAngles[1] += CTimer::GetTimeStep() * CVehicle::ProcessWheelRotation(WHEEL_STATE_NORMAL, GetForward(), m_vecMoveSpeed, pModelInfo->GetWheelSize(false) * 0.5f);

        PlayHornIfNecessary();
        ReduceHornCounter();

        vehicleFlags.bVehicleColProcessed = false;
        vehicleFlags.bAudioChangingGear   = false;
        bikeFlags.bWheelieForCamera       = false;
        break;
    }
    case STATUS_PHYSICS:
    case STATUS_GHOST:
        CCarAI::UpdateCarAI(this);
        CCarCtrl::SteerAICarWithPhysics(this);

        PlayHornIfNecessary();

        extraHandlingFlags += 2;
        bikeFlags.bWheelieForCamera = false;

        if (vehicleFlags.bIsBeingCarJacked) {
            vehicleFlags.bIsHandbrakeOn = true;
            m_GasPedal                  = 0.0f;
            m_BrakePedal                = 1.0f;
        } else {
            bikeFlags.bGettingPickedUp = false;
        }
        break;
    case STATUS_ABANDONED:
        m_BrakePedal = 0.0f;

        if (m_vecMoveSpeed.SquaredMagnitude() < 0.01f || bikeFlags.bOnSideStand) {
            vehicleFlags.bIsHandbrakeOn = true;
        } else {
            vehicleFlags.bIsHandbrakeOn = false;
        }

        m_GasPedal    = 0.0f;
        m_HornCounter = 0;

        if ((m_pDriver || m_apPassengers[0] || vehicleFlags.bIsBeingCarJacked) && !bikeFlags.bOnSideStand) {
            extraHandlingFlags += 2;
        }

        m_RideAnimData.AnimLeanLeft = 0.0f;
        m_RideAnimData.AnimLeanFwd  = 0.0f;
        bikeFlags.bWheelieForCamera = false;

        if (vehicleFlags.bIsBeingCarJacked) {
            vehicleFlags.bIsHandbrakeOn = true;
            m_GasPedal                  = 0.0f;
            m_BrakePedal                = 1.0f;
        }
        break;
    case STATUS_FORCED_STOP:
        if (m_vecMoveSpeed.SquaredMagnitude() < 0.01f) {
            vehicleFlags.bIsHandbrakeOn = true;
            m_BrakePedal                = 1.0f;
        } else {
            m_BrakePedal                = 0.0f;
            vehicleFlags.bIsHandbrakeOn = false;
        }

        m_fSteerAngle = 0.0f;
        m_GasPedal    = 0.0f;
        m_HornCounter = 0;
        extraHandlingFlags += 2;
        bikeFlags.bWheelieForCamera = false;
        return true;
    case STATUS_WRECKED:
        m_BrakePedal                = 0.05f;
        vehicleFlags.bIsHandbrakeOn = true;
        m_fSteerAngle               = 0.0f;
        m_GasPedal                  = 0.0f;
        m_HornCounter               = 0;
        bikeFlags.bWheelieForCamera = false;
        m_RideAnimData.AnimLeanLeft = 0.0f;
        m_RideAnimData.AnimLeanFwd  = 0.0f;
        break;
    default:
        break;
    }
    return true;
}

// 0x6BF400
void CBike::ProcessDrivingAnims(CPed* driver, bool blend) {
    if (m_bOffscreen && GetStatus() == STATUS_PLAYER) {
        return;
    }

    ProcessRiderAnims(driver, this, &m_RideAnimData, m_BikeHandling, 0);
}

constexpr float PED_BIKE_FOOTDOWN_SPEED   = 0.02f;
constexpr float PED_BIKE_HEAD_WOBBLE      = 6.0f;
constexpr float PED_BIKE_BRAKELEANFWD     = 0.1f;
constexpr float PED_BIKE_GASLEANBACK      = -0.3f;
constexpr float PED_BIKE_WHEELIELEANFWD   = 0.25f;
constexpr float PED_BIKE_SPINFALL_SPEED   = 0.3f;
constexpr float PED_BIKE_LEANFWD_RATE     = 0.89f;
constexpr float PED_BIKE_LEANLEFT_RATE    = 0.86f;
constexpr float PED_BIKE_LEANBLENDLIMIT   = 0.56f;
constexpr float PED_BIKE_FWDBLENDOUTSTART = 0.3f;
constexpr float PED_BIKE_MINBLENDSCALE    = 0.01f;

// 0x6B7280
void CBike::ProcessRiderAnims(CPed* rider, CVehicle* vehicle, CRideAnimData* rideData, tBikeHandlingData* handling, int16 a5) {
    bool bIsPlayer                  = rider->IsPlayer();

    CBike*       bike               = nullptr;
    CAutomobile* car                = nullptr;
    int16        nNumWheelsOnGround = 0;

    if (vehicle->IsBike()) {
        bike               = vehicle->AsBike();
        nNumWheelsOnGround = bike->m_nNoOfContactWheels;
    } else if (vehicle->IsAutomobile()) {
        car                = vehicle->AsAutomobile();
        nNumWheelsOnGround = car->m_nNumContactWheels;
    }

    float fBlendMult    = 1.0f;
    float fForwardSpeed = 0.0f;
    float fReqLeanFwd   = 0.0f;
    float fReqLeanLeft;

    CAnimBlendAssociation* pAnimLeft  = RpAnimBlendClumpGetAssociation(rider->GetRpClump(), ANIM_ID_BIKE_LEFT);
    CAnimBlendAssociation* pAnimRight = RpAnimBlendClumpGetAssociation(rider->GetRpClump(), ANIM_ID_BIKE_RIGHT);
    CAnimBlendAssociation* pAnimStop  = RpAnimBlendClumpGetAssociation(rider->GetRpClump(), ANIM_ID_BIKE_STILL);
    CAnimBlendAssociation* pAnimFwd   = nullptr;
    CAnimBlendAssociation* pAnimBack  = nullptr;
    if (bIsPlayer) {
        pAnimFwd  = RpAnimBlendClumpGetAssociation(rider->GetRpClump(), ANIM_ID_BIKE_FWD);
        pAnimBack = RpAnimBlendClumpGetAssociation(rider->GetRpClump(), ANIM_ID_BIKE_BACK);
    }
    CAnimBlendAssociation* pAnimWalkBack = RpAnimBlendClumpGetAssociation(rider->GetRpClump(), ANIM_ID_BIKE_PUSHES);
    CAnimBlendAssociation* pAnimDriveBy  = RpAnimBlendClumpGetAssociation(rider->GetRpClump(), ANIM_ID_BIKE_DRIVEBYLHS);
    if (!pAnimDriveBy) {
        pAnimDriveBy = RpAnimBlendClumpGetAssociation(rider->GetRpClump(), ANIM_ID_BIKE_DRIVEBYRHS);
        if (!pAnimDriveBy) {
            pAnimDriveBy = RpAnimBlendClumpGetAssociation(rider->GetRpClump(), ANIM_ID_BIKE_DRIVEBYFT);
        }
    }

    fForwardSpeed = DotProduct(vehicle->m_vecMoveSpeed, vehicle->GetForward());
    if (vehicle->IsBike() && (pAnimWalkBack || pAnimDriveBy)) {
        bike->bikeFlags.bWheelieForCamera = false;
    }

    float fMaxSpinSpeed = (rider->GetBikeRidingSkill() + 1.0f) * (float)PED_BIKE_SPINFALL_SPEED;
    if (vehicle->m_vecTurnSpeed.SquaredMagnitude() > fMaxSpinSpeed * fMaxSpinSpeed) {
        CEventKnockOffBike event(vehicle, vehicle->m_vecMoveSpeed, CVector(0.0f, 0.0f, 1.0f), 0.0f, 0.0f, KNOCK_OFF_TYPE_FALL, 2, 0, nullptr, true, false);
        rider->GetEventGroup().Add(&event, false);
        if (vehicle->m_apPassengers[0] && vehicle->m_apPassengers[0] != rider) {
            CEventKnockOffBike passEvent(vehicle, vehicle->m_vecMoveSpeed, CVector(0.0f, 0.0f, 1.0f), 0.0f, 0.0f, KNOCK_OFF_TYPE_FALL, 2, 0, nullptr, false, false);
            vehicle->m_apPassengers[0]->GetEventGroup().Add(&passEvent, false);
        }
    }

    if (std::abs(fForwardSpeed) < (float)PED_BIKE_FOOTDOWN_SPEED && !pAnimDriveBy) {
        if (!pAnimStop || (pAnimStop->GetBlendAmount() < 1.0f && pAnimStop->GetBlendDelta() <= 0.0f)) {
            pAnimStop = CAnimManager::BlendAnimation(rider->GetRpClump(), rideData->AnimGroup, ANIM_ID_BIKE_STILL, 2.0f);
        }
    } else {
        if (fForwardSpeed < 0.0f) {
            float fMaxReverseSpeedMult = (rider->GetBikeRidingSkill() + 1.0f) * 3.5f;
            if (vehicle->IsQuad()) {
                fMaxReverseSpeedMult += fMaxReverseSpeedMult;
            }
            if (fForwardSpeed < vehicle->m_pHandlingData->m_transmissionData.m_MaxReverseVelocity * fMaxReverseSpeedMult && (nNumWheelsOnGround == 0 || (vehicle->GetUp().z < -0.5f && CTimer::GetTimeInMS() - vehicle->m_nLastCollisionTime < 100))) {
                CEventKnockOffBike event(vehicle, vehicle->m_vecMoveSpeed, CVector(0.0f, 0.0f, 1.0f), 0.0f, 0.0f, KNOCK_OFF_TYPE_FALL, 2, 0, nullptr, true, false);
                rider->GetEventGroup().Add(&event, false);
                if (vehicle->m_apPassengers[0] && vehicle->m_apPassengers[0] != rider) {
                    CEventKnockOffBike passEvent(vehicle, vehicle->m_vecMoveSpeed, CVector(0.0f, 0.0f, 1.0f), 0.0f, 0.0f, KNOCK_OFF_TYPE_FALL, 2, 0, nullptr, false, false);
                    vehicle->m_apPassengers[0]->GetEventGroup().Add(&passEvent, false);
                }
            } else if (vehicle->GetGasPedal() < 0.0f && fForwardSpeed > vehicle->m_pHandlingData->m_transmissionData.m_MaxReverseVelocity * 1.5f) {
                if (!pAnimWalkBack || (pAnimWalkBack->GetBlendAmount() < 1.0f && pAnimWalkBack->GetBlendDelta() <= 0.0f)) {
                    pAnimWalkBack = CAnimManager::BlendAnimation(rider->GetRpClump(), rideData->AnimGroup, ANIM_ID_BIKE_PUSHES, 4.0f);
                }
            } else {
                if (bIsPlayer && fForwardSpeed < vehicle->m_pHandlingData->m_transmissionData.m_MaxReverseVelocity * 1.5f) {
                    fReqLeanFwd = -1.0f;
                }
                if (pAnimStop && pAnimStop->GetBlendDelta() >= 0.0f) {
                    pAnimStop->SetBlendDelta(-4.0f);
                }
                if (pAnimWalkBack && pAnimWalkBack->GetBlendDelta() >= 0.0f) {
                    pAnimWalkBack->SetBlendDelta(-4.0f);
                }
            }
        } else {
            if (pAnimStop && pAnimStop->GetBlendDelta() >= 0.0f) {
                pAnimStop->SetBlendDelta(-4.0f);
            }
            if (pAnimWalkBack && pAnimWalkBack->GetBlendDelta() >= 0.0f) {
                pAnimWalkBack->SetBlendDelta(-4.0f);
            }
        }
    }

    if (pAnimStop) {
        fBlendMult = 1.0f - std::min<float>(pAnimStop->GetBlendAmount() + (pAnimStop->GetBlendDelta() * CTimer::GetTimeStepInSeconds()), 1.0f);
    }
    if (pAnimDriveBy) {
        fBlendMult -= std::min<float>(fBlendMult, pAnimDriveBy->GetBlendAmount() + (pAnimDriveBy->GetBlendDelta() * CTimer::GetTimeStepInSeconds()));
    }
    if (pAnimWalkBack) {
        fBlendMult -= std::min<float>(fBlendMult, pAnimWalkBack->GetBlendAmount() + (pAnimWalkBack->GetBlendDelta() * CTimer::GetTimeStepInSeconds()));
    }

    fReqLeanLeft = rideData->LeanAngle / handling->m_fFullAnimLean;
    if (fReqLeanFwd == -1.0f) {
        fReqLeanLeft = 0.0f;
    } else if (fReqLeanLeft > 1.0f) {
        fReqLeanLeft = 1.0f;
    } else if (fReqLeanLeft < -1.0f) {
        fReqLeanLeft = -1.0f;
    }

    float fLeanLeftRate_Timed = std::pow((float)PED_BIKE_LEANLEFT_RATE, CTimer::GetTimeStep());
    rideData->AnimLeanLeft    = (fLeanLeftRate_Timed * rideData->AnimLeanLeft) + (fReqLeanLeft * (1.0f - fLeanLeftRate_Timed));

    if (bIsPlayer && !vehicle->m_apPassengers[0]) {
        if (fReqLeanFwd > -1.0f) {
            fReqLeanFwd = rideData->LeanFwd;
            if (bike) {
                bike->bikeFlags.bWheelieForCamera = false;
                if (bike->m_aWheelRatios[0] > 0.0f || bike->m_aWheelRatios[1] > 0.0f || bike->GetUp().z <= 0.0f || (bike->m_aWheelColPoints[0].m_nSurfaceTypeB <= 0 && bike->m_aWheelColPoints[1].m_nSurfaceTypeB <= 0)) {
                    if (bike->m_aWheelColPoints[0].m_nSurfaceTypeB <= 0 && bike->m_aWheelColPoints[1].m_nSurfaceTypeB <= 0) {
                        float fStoppieAngle = bike->GetUp().z;
                        if (fStoppieAngle < 0.0f && (bike->m_aWheelRatios[0] > 0.0f || bike->m_aWheelRatios[1] > 0.0f) && (handling->m_fStoppieAng - fStoppieAngle) > (handling->m_fStoppieAng * 0.6f)) {
                            bike->bikeFlags.bWheelieForCamera = true;
                        }
                    }
                } else {
                    float fWheelieAngle = handling->m_fWheelieAng - bike->GetUp().z;
                    if (fWheelieAngle < (handling->m_fWheelieAng * 0.5f)) {
                        bike->bikeFlags.bWheelieForCamera = true;
                    }
                }
            } else if (car) {
                if (car->m_fWheelsSuspensionCompression[0] <= 0.0f && car->m_fWheelsSuspensionCompression[1] <= 0.0f && car->GetUp().z > 0.0f && (car->m_wheelColPoint[0].m_nSurfaceTypeB > 0 || car->m_wheelColPoint[1].m_nSurfaceTypeB > 0)) {
                    float fWheelieAngle = handling->m_fWheelieAng - car->GetUp().z;
                    if (fWheelieAngle < 0.15f) {
                        fReqLeanFwd = std::max<float>(fReqLeanFwd, (float)PED_BIKE_WHEELIELEANFWD);
                    }
                }
            }

            if (vehicle->GetBrakePedal() > 0.5f && fForwardSpeed > 0.01f) {
                fReqLeanFwd = std::max<float>(fReqLeanFwd, (float)PED_BIKE_BRAKELEANFWD);
            } else if (vehicle->GetGasPedal() > 0.5f && fReqLeanFwd <= 0.0f && fForwardSpeed < vehicle->m_pHandlingData->m_transmissionData.m_MaxFlatVelocity * 0.3f) {
                fReqLeanFwd = std::min<float>(fReqLeanFwd, (float)PED_BIKE_GASLEANBACK);
            }

            if (std::abs(fReqLeanLeft) > (float)PED_BIKE_FWDBLENDOUTSTART) {
                float fFactor = 1.0f - (std::abs(fReqLeanLeft) - (float)PED_BIKE_FWDBLENDOUTSTART) / ((float)PED_BIKE_LEANBLENDLIMIT - (float)PED_BIKE_FWDBLENDOUTSTART);
                if (fFactor < 0.0f) {
                    fFactor = 0.0f;
                }
                fReqLeanFwd *= fFactor;
            }
        }
    } else {
        if (bike) {
            bike->bikeFlags.bWheelieForCamera = false;
        }
        fReqLeanFwd = 0.0f;
    }

    if (bIsPlayer) {
        float fLeanFwdRate_Timed = std::pow((float)PED_BIKE_LEANFWD_RATE, CTimer::GetTimeStep());
        rideData->AnimLeanFwd    = (fLeanFwdRate_Timed * rideData->AnimLeanFwd) + (fReqLeanFwd * (1.0f - fLeanFwdRate_Timed));
    } else {
        rideData->AnimLeanFwd = 0.0f;
    }

    float fLeanLeftScale = 1.0f;
    float fLeanFwdScale  = 0.0f;

    if (std::abs(rideData->AnimLeanLeft) <= (float)PED_BIKE_LEANBLENDLIMIT && bIsPlayer) {
        if (std::abs(rideData->AnimLeanFwd) > (float)PED_BIKE_LEANBLENDLIMIT) {
            fLeanLeftScale = 0.0f;
            fLeanFwdScale  = 1.0f;
        } else {
            float fTempTotal = std::sqrt((rideData->AnimLeanLeft * rideData->AnimLeanLeft) + (rideData->AnimLeanFwd * rideData->AnimLeanFwd));
            if (fTempTotal > (float)PED_BIKE_MINBLENDSCALE) {
                fLeanFwdScale  = std::abs(rideData->AnimLeanFwd / fTempTotal);
                fLeanLeftScale = std::abs(rideData->AnimLeanLeft / fTempTotal);
            } else {
                fLeanFwdScale  = std::abs(rideData->AnimLeanFwd);
                fLeanLeftScale = std::abs(rideData->AnimLeanLeft);
            }
        }
    }

    float fFwdAmount  = fLeanFwdScale * fBlendMult;
    float fLeftAmount = fLeanLeftScale * fBlendMult;

    if (bIsPlayer) {
        if (!pAnimFwd) {
            pAnimFwd = CAnimManager::AddAnimation(rider->GetRpClump(), rideData->AnimGroup, ANIM_ID_BIKE_FWD);
        }
        if (!pAnimBack) {
            pAnimBack = CAnimManager::AddAnimation(rider->GetRpClump(), rideData->AnimGroup, ANIM_ID_BIKE_BACK);
        }

        if (rideData->AnimLeanFwd >= 0.0f) {
            pAnimFwd->SetBlendAmount(fFwdAmount);
            pAnimFwd->SetCurrentTime(pAnimFwd->m_BlendHier->GetTotalTime() * rideData->AnimLeanFwd);
            pAnimFwd->m_Flags &= ~ANIMATION_IS_PLAYING;
            pAnimBack->SetBlendAmount(0.0f);
        } else {
            pAnimBack->SetBlendAmount(fFwdAmount);
            pAnimBack->SetCurrentTime(-(pAnimBack->m_BlendHier->GetTotalTime() * rideData->AnimLeanFwd));
            pAnimBack->m_Flags &= ~ANIMATION_IS_PLAYING;
            pAnimFwd->SetBlendAmount(0.0f);
        }
    }

    if (!pAnimLeft) {
        pAnimLeft = CAnimManager::AddAnimation(rider->GetRpClump(), rideData->AnimGroup, ANIM_ID_BIKE_LEFT);
    }
    if (!pAnimRight) {
        pAnimRight = CAnimManager::AddAnimation(rider->GetRpClump(), rideData->AnimGroup, ANIM_ID_BIKE_RIGHT);
    }

    if (rideData->AnimLeanLeft >= 0.0f) {
        pAnimRight->SetBlendAmount(fLeftAmount);
        pAnimRight->SetCurrentTime(pAnimRight->m_BlendHier->GetTotalTime() * rideData->AnimLeanLeft);
        pAnimRight->m_Flags &= ~ANIMATION_IS_PLAYING;
        pAnimLeft->SetBlendAmount(0.0f);
    } else {
        pAnimLeft->SetBlendAmount(fLeftAmount);
        pAnimLeft->SetCurrentTime(-(pAnimLeft->m_BlendHier->GetTotalTime() * rideData->AnimLeanLeft));
        pAnimLeft->m_Flags &= ~ANIMATION_IS_PLAYING;
        pAnimRight->SetBlendAmount(0.0f);
    }

    if (fForwardSpeed > (float)PED_BIKE_SPINFALL_SPEED) {
        if (rider->m_apBones[PED_NODE_HEAD] && rider->m_apBones[PED_NODE_HEAD]->KeyFrame) {
            RtQuat* q = &rider->m_apBones[PED_NODE_HEAD]->KeyFrame->q;
            RtQuatRotate(q, &CPedIK::XaxisIK, CGeneral::GetRandomNumberInRange(fForwardSpeed * -(float)PED_BIKE_HEAD_WOBBLE, fForwardSpeed * (float)PED_BIKE_HEAD_WOBBLE), rwCOMBINEPRECONCAT);
            RtQuatRotate(q, &CPedIK::YaxisIK, CGeneral::GetRandomNumberInRange(fForwardSpeed * -(float)PED_BIKE_HEAD_WOBBLE, fForwardSpeed * (float)PED_BIKE_HEAD_WOBBLE), rwCOMBINEPRECONCAT);
            rider->bUpdateMatricesRequired = true;
        }
    }
}

constexpr float fBikeBurstForceMult       = 0.02f;
constexpr float fBikeBurstFallSpeed       = 0.3f;
constexpr float fBikeBurstFallSpeedPlayer = 0.55f;

// 0x6BEB20
bool CBike::BurstTyre(uint8 tyreComponentId, bool bPhysicalEffect) {
    if (vehicleFlags.bTyresDontBurst || physicalFlags.bRenderScorched) {
        return false;
    }

    switch (tyreComponentId) {
    case CAR_PIECE_WHEEL_LF:
        tyreComponentId = 0;
        break;
    case CAR_PIECE_WHEEL_RL:
        tyreComponentId = 1;
        break;
    }

    bool bBurstTyre = false;
    if (m_nWheelStatus[tyreComponentId] == eCarWheelStatus::WHEEL_STATUS_OK) {
        m_nWheelStatus[tyreComponentId] = eCarWheelStatus::WHEEL_STATUS_BURST;

#ifdef FIX_BUGS
        CStats::IncrementStat(STAT_TIRES_POPPED_WITH_GUNFIRE, 1);
#endif
        m_vehicleAudio.AddAudioEvent(AE_TYRE_BURST);

        if (GetStatus() == STATUS_SIMPLE) {
            CCarCtrl::SwitchVehicleToRealPhysics(this);
        }

        if (bPhysicalEffect) {
            const auto fForce = CGeneral::GetRandomNumberInRange(-fBikeBurstForceMult, fBikeBurstForceMult);
            ApplyMoveForce(fForce * m_fMass * GetRight());
            ApplyTurnForce(fForce * m_fTurnMass * GetRight(), GetForward());
        }
        bBurstTyre = true;
    }

    if (m_pDriver) {
#ifdef FIX_BUGS
        if ((tyreComponentId == 0 && (m_aRatioHistory[0] < 1.0f || m_aRatioHistory[1] < 1.0f)) ||
            (tyreComponentId == 1 && (m_aRatioHistory[2] < 1.0f || m_aRatioHistory[3] < 1.0f)))
#else
        if ((tyreComponentId == CAR_PIECE_WHEEL_LF && (m_aRatioHistory[0] < 1.0f || m_aRatioHistory[1] < 1.0f)) ||
            (tyreComponentId == CAR_PIECE_WHEEL_RL && (m_aRatioHistory[2] < 1.0f || m_aRatioHistory[3] < 1.0f)))
#endif
        {
            const auto fSpeed = m_vecMoveSpeed.Magnitude();
            if (fSpeed > fBikeBurstFallSpeed && (GetStatus() != STATUS_PLAYER || fSpeed > fBikeBurstFallSpeedPlayer)) {
#ifdef FIX_BUGS
                if (tyreComponentId == 0)
#else
                if (tyreComponentId == CAR_PIECE_WHEEL_LF)
#endif
                {
                    CEventKnockOffBike event(this, m_vecMoveSpeed, m_vecLastCollisionImpactVelocity, 0.0f, 0.0f, KNOCK_OFF_TYPE_SKIDBACKFRONT, 0, 0, nullptr, true, false);
                    m_pDriver->GetEventGroup().Add(&event);
                    if (m_apPassengers[0]) {
                        CEventKnockOffBike passEvent(this, m_vecMoveSpeed, m_vecLastCollisionImpactVelocity, 0.0f, 0.0f, KNOCK_OFF_TYPE_SKIDBACKFRONT, 0, 0, nullptr, false, false);
                        m_apPassengers[0]->GetEventGroup().Add(&passEvent);
                    }
                } else {
                    ApplyTurnForce(2.0f * fBikeBurstForceMult * m_fTurnMass * GetRight(), GetForward());
                }
            }
        }
    }

    return bBurstTyre;
}

#define BIKE_STEER_SMOOTH_RATE (0.2f)
constexpr float BIKE_MOUSE_STEER_SENS   = -0.0035f;
constexpr float BIKE_MOUSE_CENTRE_RANGE = 0.35f;
constexpr float BIKE_MOUSE_CENTRE_MULT  = 0.98f;

// 0x6BE310
void CBike::ProcessControlInputs(uint8 playerNum) {
    const float forwardness = DotProduct(m_vecMoveSpeed, GetForward());
    CPad*       pad         = CPad::GetPad(playerNum);

    if (pad->GetExitVehicle()) {
        vehicleFlags.bIsHandbrakeOn = true;
    } else {
        vehicleFlags.bIsHandbrakeOn = pad->GetHandBrake();
    }

    if (TheCamera.m_bUseMouse3rdPerson && m_bEnableMouseSteering) {
        if (CPad::NewMouseControllerState.m_AmountMoved.IsZero()
            && (std::abs(m_fRawSteerAngle) != 0.0f || m_nLastControlInput != eControllerType::MOUSE || pad->GetSteeringLeftRight() != 0 || pad->GetSteeringUpDown() != 0)) {
            if (pad->GetSteeringLeftRight() != 0 || pad->GetSteeringUpDown() != 0 || m_nLastControlInput != eControllerType::MOUSE) {
                m_nLastControlInput = eControllerType::JOY_STICK;
                m_fRawSteerAngle += (((float)-pad->GetSteeringLeftRight() * (1.0f / 128.0f)) - m_fRawSteerAngle) * CTimer::GetTimeStep() * BIKE_STEER_SMOOTH_RATE;
                m_RideAnimData.LeanFwd += (((float)-pad->GetSteeringUpDown() * (1.0f / 128.0f)) - m_RideAnimData.LeanFwd) * CTimer::GetTimeStep() * 0.2f;
            }
        } else {
            m_nLastControlInput = eControllerType::MOUSE;
            if (!pad->NewState.m_bVehicleMouseLook) {
                m_fRawSteerAngle += CPad::NewMouseControllerState.m_AmountMoved.x * BIKE_MOUSE_STEER_SENS;
                m_RideAnimData.LeanFwd += CPad::NewMouseControllerState.m_AmountMoved.y * BIKE_MOUSE_STEER_SENS;
            }
            if (std::abs(m_fRawSteerAngle) < BIKE_MOUSE_CENTRE_RANGE || pad->NewState.m_bVehicleMouseLook) {
                m_fRawSteerAngle *= std::pow(BIKE_MOUSE_CENTRE_MULT, CTimer::GetTimeStep());
            }
            if (std::abs(m_RideAnimData.LeanFwd) < BIKE_MOUSE_CENTRE_RANGE || pad->NewState.m_bVehicleMouseLook) {
                m_RideAnimData.LeanFwd *= std::pow(BIKE_MOUSE_CENTRE_MULT, CTimer::GetTimeStep());
            }
        }
    } else {
        m_fRawSteerAngle += (((float)-pad->GetSteeringLeftRight() * (1.0f / 128.0f)) - m_fRawSteerAngle) * CTimer::GetTimeStep() * BIKE_STEER_SMOOTH_RATE;
        m_RideAnimData.LeanFwd += (((float)-pad->GetSteeringUpDown() * (1.0f / 128.0f)) - m_RideAnimData.LeanFwd) * CTimer::GetTimeStep() * 0.2f;
    }

    m_fRawSteerAngle       = std::clamp(m_fRawSteerAngle, -1.0f, 1.0f);
    m_RideAnimData.LeanFwd = std::clamp(m_RideAnimData.LeanFwd, -1.0f, 1.0f);

    const float fPedal     = (float(pad->GetAccelerate()) - float(pad->GetBrake())) * (1.0f / 255.0f);

    if (std::abs(forwardness) < 0.01f) {
        if (pad->GetAccelerate() > 150 && pad->GetBrake() > 150 && m_nVehicleSubType != VEHICLE_TYPE_BMX) {
            m_GasPedal   = float(pad->GetAccelerate()) * (1.0f / 255.0f);
            m_BrakePedal = float(pad->GetBrake()) * (1.0f / 255.0f);
            m_nBrakesOn  = 1;
        } else {
            m_GasPedal   = fPedal;
            m_BrakePedal = 0.0f;
        }
    } else {
        if (forwardness >= 0.0f) {
            if (fPedal >= 0.0f) {
                m_GasPedal   = fPedal;
                m_BrakePedal = 0.0f;
            } else {
                m_GasPedal   = 0.0f;
                m_BrakePedal = -fPedal;
            }
        } else {
            if (fPedal >= 0.0f) {
                m_BrakePedal = fPedal;
                m_GasPedal   = 0.0f;
            } else {
                m_GasPedal   = fPedal;
                m_BrakePedal = 0.0f;
            }
        }
    }

    static float fValue;
    if (m_fRawSteerAngle >= 0.0f) {
        fValue = m_fRawSteerAngle * m_fRawSteerAngle;
    } else {
        fValue = -m_fRawSteerAngle * m_fRawSteerAngle;
    }

    if (m_autoPilot.RecordingNumber < 0 || CVehicleRecording::bUseCarAI[m_autoPilot.RecordingNumber]) {
        m_fSteerAngle = fValue * (m_pHandlingData->m_fSteeringLock * (3.14159265f / 180.0f));
    }

    if (vehicleFlags.bComedyControls) {
        if ((CTimer::GetTimeInMS() & 0x3C00) < 0x3000) {
            m_GasPedal = 1.0f;
        }
        if ((((CTimer::GetTimeInMS() >> 10) + 6) & 0xF) < 0xC) {
            m_BrakePedal = 0.0f;
        }
        vehicleFlags.bIsHandbrakeOn = false;
        if (CTimer::GetTimeInMS() & 0x800) {
            m_fSteerAngle += 0.08f;
        } else {
            m_fSteerAngle -= 0.03f;
        }
    }

    if (CPad::GetPad(0)->DisablePlayerControls && CGameLogic::SkipState != 2) {
        m_BrakePedal                = 1.0f;
        vehicleFlags.bIsHandbrakeOn = true;
        m_GasPedal                  = 0.0f;
        FindPlayerPed()->KeepAreaAroundPlayerClear();
        const float speed = m_vecMoveSpeed.Magnitude();
        if (speed > 0.28f) {
            m_vecMoveSpeed *= (0.28f / speed);
        }
    }
}

// 0x6BDEA0
int32 CBike::ProcessEntityCollision(CEntity* entity, CColPoint* outColPoints) {
    if (GetStatus() != STATUS_SIMPLE) {
        vehicleFlags.bVehicleColProcessed = true;
    }

    const auto tcd = GetColData(),
               ocd = entity->GetColData();

#ifdef FIX_BUGS // Text search for `FIX_BUGS@CAutomobile::ProcessEntityCollision:1`
    if (!tcd || !ocd) {
        return 0;
    }
#endif

    if (physicalFlags.bSkipLineCol || physicalFlags.bProcessingShift || entity->GetIsTypePed()) {
        tcd->m_nNumLines = 0; // Later reset back to original value
    }

    const auto ogWheelRatios = m_aWheelRatios;

    auto numColPts           = CCollision::ProcessColModels(
        GetMatrix(), *GetColModel(), entity->GetMatrix(), *entity->GetColModel(), *(std::array<CColPoint, 32>*)(outColPoints), m_aWheelColPoints.data(), m_aWheelRatios.data(), false
    );

    // Possibly add driver & entity collisions to `outColPoints`
    if (m_pDriver && m_nTestPedCollision) {
        const auto pcd = m_pDriver->GetColData();
        if (!pcd->m_nNumLines) {
            std::array<CColPoint, 32> pedCPs{};

            CMatrix driverMat = GetMatrix();
            driverMat.GetPosition() += GetDriverSeatDummyPositionWS();

            std::array<CColPoint, 32> pedEntityColPts{};
            const auto                numPedEntityColPts = CCollision::ProcessColModels(
                driverMat, *m_pDriver->GetColModel(), entity->GetMatrix(), *entity->GetColModel(), pedEntityColPts, nullptr, nullptr, false
            );

            if (numPedEntityColPts) {
                if (m_nTestPedCollision == 1) {
                    m_nTestPedCollision = 0;
                } else {
                    for (auto i = 0; i < numPedEntityColPts && numColPts < 32; i++) {
                        const auto& pedEntityCP = pedCPs[i];
                        if (pedEntityCP.m_nPieceTypeA == PED_COL_SPHERE_LEG) {
                            continue;
                        }
                        outColPoints[numColPts++] = pedEntityCP;
                    }
                }
            }
        }
    }

    size_t numProcessedLines{};
    if (tcd->m_nNumLines) {
        // Process the real wheels
        for (auto i = 0; i < NUM_SUSP_LINES; i++) {
            const auto& cp                  = m_aWheelColPoints[i];

            const auto wheelColPtsTouchDist = m_aWheelRatios[i];
            if (wheelColPtsTouchDist >= 1.f || wheelColPtsTouchDist >= ogWheelRatios[i]) {
                continue;
            }

            numProcessedLines++;

            m_anCollisionLighting[i] = cp.m_nLightingB;
            m_nContactSurface        = cp.m_nSurfaceTypeB;

            switch (entity->GetType()) {
            case ENTITY_TYPE_VEHICLE:
            case ENTITY_TYPE_OBJECT:  {
                CEntity::ChangeEntityReference(m_aGroundPhysicalPtrs[i], entity->AsPhysical());

                m_aGroundOffsets[i] = cp.m_vecPoint - entity->GetPosition();
                if (entity->GetIsTypeVehicle()) {
                    m_anCollisionLighting[i] = entity->AsVehicle()->m_anCollisionLighting[i];
                }
                break;
            }
            case ENTITY_TYPE_BUILDING: {
                m_pEntityWeAreOn    = entity;
                m_bTunnel           = entity->m_bTunnel;
                m_bTunnelTransition = entity->m_bTunnelTransition;
                break;
            }
            }
        }
    } else {
        tcd->m_nNumLines = NUM_SUSP_LINES;
    }

    if (numColPts > 0 || numProcessedLines > 0) {
        AddCollisionRecord(entity);
        if (!entity->GetIsTypeBuilding()) {
            entity->AsPhysical()->AddCollisionRecord(this);
        }
        if (numColPts > 0) {
            if (entity->GetIsTypeBuilding()
                || (entity->GetIsTypeObject() && entity->AsPhysical()->physicalFlags.bDisableCollisionForce)) {
                SetHasHitWall(true);
            }
        }
    }

    return numColPts;
}

// 0x6B6790
bool CBike::GetAllWheelsOffGround() const {
    return m_nNoOfContactWheels == 0;
}

// 0x6B67A0
void CBike::DebugCode() {
    // NOP
}

constexpr float BIKE_RATIO_BURST_MULT                = 0.2f;
constexpr float BIKE_RATIO_SINK_IN_SAND              = 0.25f;
constexpr float BIKE_RATIO_SINK_IN_SAND_OA1          = 0.15f;
constexpr float BIKE_RATIO_SINK_IN_SAND_OA2          = 0.1f;
constexpr float BIKE_RATIO_SAND_BOGDOWN_LIMIT        = 0.3f;
constexpr float BIKE_RATIO_RAILTRACK_BUMP_MULT       = 0.3f;
constexpr float BIKE_RATIO_RAILTRACK_BUMP_DIST       = 1.5f;
constexpr float BIKE_RATIO_RAILTRACK_BUMP_DIST_SPEED = 0.3f;
constexpr float BIKE_SAND_MOVERES_MULT               = 0.02f;

// 0x6B6D40
void CBike::DoSoftGroundResistance(uint32& extraHandlingFlags) {
    if ((m_aWheelRatios[0] < 1.0f && g_surfaceInfos.GetAdhesionGroup(m_aWheelColPoints[0].m_nSurfaceTypeB) == ADHESION_GROUP_SAND)
        || (m_aWheelRatios[1] < 1.0f && g_surfaceInfos.GetAdhesionGroup(m_aWheelColPoints[1].m_nSurfaceTypeB) == ADHESION_GROUP_SAND)
        || (m_aWheelRatios[2] < 1.0f && g_surfaceInfos.GetAdhesionGroup(m_aWheelColPoints[2].m_nSurfaceTypeB) == ADHESION_GROUP_SAND)
        || (m_aWheelRatios[3] < 1.0f && g_surfaceInfos.GetAdhesionGroup(m_aWheelColPoints[3].m_nSurfaceTypeB) == ADHESION_GROUP_SAND)) {
        CVector vecThisSpeed = m_vecMoveSpeed - DotProduct(m_vecMoveSpeed, GetUp()) * GetUp();
        if (m_GasPedal > 0.3f) {
            if (vecThisSpeed.SquaredMagnitude() < BIKE_RATIO_SAND_BOGDOWN_LIMIT * BIKE_RATIO_SAND_BOGDOWN_LIMIT) {
                extraHandlingFlags += 4;
            }
            vecThisSpeed -= DotProduct(vecThisSpeed, GetForward()) * GetForward();
        }
        ApplyMoveForce(-BIKE_SAND_MOVERES_MULT * m_fMass * CTimer::GetTimeStep() * vecThisSpeed);
    } else if ((m_aWheelRatios[0] < 1.0f && m_aWheelColPoints[0].m_nSurfaceTypeB == SURFACE_RAILTRACK)
               || (m_aWheelRatios[1] < 1.0f && m_aWheelColPoints[1].m_nSurfaceTypeB == SURFACE_RAILTRACK)
               || (m_aWheelRatios[2] < 1.0f && m_aWheelColPoints[2].m_nSurfaceTypeB == SURFACE_RAILTRACK)
               || (m_aWheelRatios[3] < 1.0f && m_aWheelColPoints[3].m_nSurfaceTypeB == SURFACE_RAILTRACK)) {
        CVector vecThisSpeed = m_vecMoveSpeed - DotProduct(m_vecMoveSpeed, GetUp()) * GetUp();
        ApplyMoveForce(-CVehicle::ms_fRailTrackResistance * m_fMass * CTimer::GetTimeStep() * vecThisSpeed);
    }
}

// 0x6B7130
void CBike::PlayHornIfNecessary() {
    if (m_autoPilot.SlowingDownForCar || m_autoPilot.SlowingDownForPed) {
        PlayCarHorn();
    }
}

// 0x6B7150
void CBike::CalculateLeanMatrix() {
    if (m_bLeanMatrixCalculated) {
        return;
    }

    CMatrix mat;
    mat.SetRotateX(fabs(m_RideAnimData.LeanAngle) * -0.05f);
    mat.RotateY(m_RideAnimData.LeanAngle);
    m_mLeanMatrix = GetMatrix();
    m_mLeanMatrix = m_mLeanMatrix * mat;
    // place wheel back on ground
    m_mLeanMatrix.GetPosition() += GetUp() * (1.0f - cos(m_RideAnimData.LeanAngle)) * GetColModel()->GetBoundingBox().m_vecMin.z;
    m_bLeanMatrixCalculated = true;
}

// 0x6B7F90
void CBike::FixHandsToBars(CPed* rider) {
    ((void(__thiscall*)(CBike*, CPed*))0x6B7F90)(this, rider);
}

// 0x6BEEB0
void CBike::PlaceOnRoadProperly() {
    const float carLengthFront   = GetColModel()->GetBoundingBox().m_vecMax.y;
    const float carLengthRear    = -GetColModel()->GetBoundingBox().m_vecMin.y;

    const CVector vecAlongLength = GetForward();

    CVector rearPoint            = GetPosition() - vecAlongLength * carLengthRear;
    CVector frontPoint           = GetPosition() + vecAlongLength * carLengthFront;
    frontPoint.z                 = GetPosition().z - 5.0f;

    CVector highPoint            = frontPoint;
    highPoint.z                  = GetPosition().z + 5.0f;
    CColPoint testColPoint;
    CEntity*  testEntity;
    if (CWorld::ProcessVerticalLine(highPoint, frontPoint.z, testColPoint, testEntity, true, false, false, false, false, false, nullptr)) {
        frontPoint.z             = testColPoint.m_vecPoint.z;
        m_anCollisionLighting[0] = testColPoint.m_nLightingB;
        m_pEntityWeAreOn         = testEntity;
        m_bTunnel                = testEntity->m_bTunnel;
        m_bTunnelTransition      = testEntity->m_bTunnelTransition;
    } else {
        frontPoint.z = GetPosition().z;
    }

    rearPoint.z = GetPosition().z - 5.0f;
    highPoint   = rearPoint;
    highPoint.z = GetPosition().z + 5.0f;
    if (CWorld::ProcessVerticalLine(highPoint, rearPoint.z, testColPoint, testEntity, true, false, false, false, false, false, nullptr)) {
        rearPoint.z              = testColPoint.m_vecPoint.z;
        m_anCollisionLighting[1] = testColPoint.m_nLightingB;
        m_pEntityWeAreOn         = testEntity;
        m_bTunnel                = testEntity->m_bTunnel;
        m_bTunnelTransition      = testEntity->m_bTunnelTransition;
    } else {
        rearPoint.z = GetPosition().z;
    }

    const float totalLength    = carLengthFront + carLengthRear;
    const float elevation      = std::atan((frontPoint.z - rearPoint.z) / totalLength);
    const float cosElevation   = std::cos(elevation);
    const float sinElevation   = std::sin(elevation);

    GetMatrix().GetRight().x   = (frontPoint.y - rearPoint.y) / totalLength;
    GetMatrix().GetRight().y   = -(frontPoint.x - rearPoint.x) / totalLength;
    GetMatrix().GetRight().z   = 0.0f;

    GetMatrix().GetForward().x = -cosElevation * GetMatrix().GetRight().y;
    GetMatrix().GetForward().y = cosElevation * GetMatrix().GetRight().x;
    GetMatrix().GetForward().z = sinElevation;

    const CVector up           = CrossProduct(GetRight(), GetForward());
    GetMatrix().GetUp()        = up;

    CVector resultCoors        = (frontPoint + rearPoint) * 0.5f;
    resultCoors.z += m_fHeightAboveRoad;

    SetPosn(resultCoors);
}

// 0x6BF230
void CBike::GetCorrectedWorldDoorPosition(CVector& outPos, CVector vecAnimOffset, CVector vecSeatPos) {
    const CVector vecRight   = CrossProduct(GetForward(), CVector(0.0f, 0.0f, 1.0f));
    const CVector vecUp      = CrossProduct(vecRight, GetForward());

    const float fUpRight     = DotProduct(GetUp(), vecRight);
    auto&       colModel     = *GetColModel();
    float       fRightAdjust = 0.0f;
    if (colModel.GetBoundingBox().m_vecMax.z > colModel.GetBoundingBox().m_vecMax.x) {
        fRightAdjust = colModel.GetBoundingBox().m_vecMax.z - colModel.GetBoundingBox().m_vecMax.x;
    }

    outPos = CVector(0.0f, 0.0f, 0.0f);
    outPos += GetForward() * (vecSeatPos.y - vecAnimOffset.y);
    outPos += vecRight * ((vecSeatPos.x - vecAnimOffset.x) + (fUpRight * fRightAdjust));
    outPos += vecUp * (vecSeatPos.z - vecAnimOffset.z);
    outPos += GetPosition();
}

// 0x6BEA10
void CBike::BlowUpCar(CEntity* damager, bool bHideExplosion) {
    if (vehicleFlags.bCanBeDamaged) {
        m_vecMoveSpeed.z += 0.13f;
        SetStatus(STATUS_WRECKED);
        physicalFlags.bRenderScorched = true;
        CVisibilityPlugins::SetClumpForAllAtomicsFlag(GetRpClump(), (int32)eAtomicComponentFlag::ATOMIC_PIPE_NO_EXTRA_PASSES);
        m_fHealth          = 0.0f;
        m_DelayedExplosion = 0;
        TheCamera.CamShake(0.4f, GetPosition());
        KillPedsInVehicle();
        m_nOverrideLights      = eVehicleOverrideLightsState::NO_CAR_LIGHT_OVERRIDE;
        vehicleFlags.bEngineOn = false;
        vehicleFlags.bLightsOn = false;
        ChangeLawEnforcerState(false);
        CExplosion::AddExplosion(this, damager, eExplosionType::EXPLOSION_CAR, GetPosition(), 0, true, -1.0f, bHideExplosion);
        CDarkel::RegisterCarBlownUpByPlayer(*this, 0);
    }
}

// 0x6B7050
void CBike::Fix() {
    vehicleFlags.bIsDamaged = false;
    bikeFlags.bEngineOnFire = false;
    m_nWheelStatus[0]       = 0;
    m_nWheelStatus[1]       = 0;
}

// 0x6BD090
void CBike::PreRender() {
    CVehicle::PreRender();

    CColModel*      colModel = GetColModel();
    CCollisionData* pColData = colModel ? colModel->m_pColData : nullptr;

    if (vehicleFlags.bVehicleColProcessed && pColData) {
        DoBurstAndSoftGroundRatios();

        if (m_fLineLength[0] > 0.0f) {
            float baseRatio = 1.0f - m_fSuspensionLength[0] / m_fLineLength[0];
            if (baseRatio < 1.0f) {
                float ratio   = (std::min(m_aWheelRatios[1], m_aWheelRatios[0]) - baseRatio) / (1.0f - baseRatio);
                float fHeight = pColData->m_pLines[0].m_vecStart.z;
                if (ratio > 0.0f) {
                    fHeight -= ratio * m_fSuspensionLength[0];
                }
                m_aWheelSuspensionHeights[0] += (fHeight - m_aWheelSuspensionHeights[0]) * 0.75f;
            }
        }

        if (m_fLineLength[2] > 0.0f) {
            float baseRatio = 1.0f - m_fSuspensionLength[2] / m_fLineLength[2];
            if (baseRatio < 1.0f) {
                float ratio   = (std::min(m_aWheelRatios[3], m_aWheelRatios[2]) - baseRatio) / (1.0f - baseRatio);
                float fHeight = pColData->m_pLines[2].m_vecStart.z;
                if (ratio > 0.0f) {
                    fHeight -= ratio * m_fSuspensionLength[2];
                }
                m_aWheelSuspensionHeights[1] += (fHeight - m_aWheelSuspensionHeights[1]) * 0.75f;
            }
        }
    }

    if (GetStatus() == STATUS_PHYSICS || GetStatus() == STATUS_PLAYER || GetStatus() == STATUS_PLAYER_PLAYBACK_FROM_BUFFER || GetStatus() == STATUS_SIMPLE) {
        bool bDoFrontSkidSmoke = false;
        if (m_WheelStates[1] == WHEEL_STATE_SKIDDING) {
            bDoFrontSkidSmoke = true;
        }

        float fTempSpeed = m_vecMoveSpeed.Magnitude();

        for (int32 i = 0; i < 2; ++i) {
            int32 nUseWheelLine;
            if (i == 0) {
                nUseWheelLine = 0;
                if (m_aRatioHistory[0] >= 1.0f && m_aRatioHistory[1] < 1.0f) {
                    nUseWheelLine = 1;
                }
            } else {
                nUseWheelLine = 3;
                if (m_aRatioHistory[3] >= 1.0f && m_aRatioHistory[2] < 1.0f) {
                    nUseWheelLine = 2;
                }
            }

            uint32 nWheelParticleFlags = 0;
            if (i == 0 && !bDoFrontSkidSmoke) {
                nWheelParticleFlags = 4;
            }

            float   fTyreTouchSide       = GetColModel()->m_boundBox.m_vecMin.z * (std::sin(m_RideAnimData.LeanAngle) * 0.8f);
            CVector vecRearWheelColPoint = m_aWheelColPoints[nUseWheelLine].m_vecPoint;
            vecRearWheelColPoint += GetRight() * fTyreTouchSide;

            if (m_bWheelBloody[i]) {
                nWheelParticleFlags |= 1;
            }
            if (m_bMoreSkidMarks[i]) {
                nWheelParticleFlags |= 2;
            }

            float fOutsideVector = (m_RideAnimData.LeanAngle <= 0.0f) ? 1.0f : -1.0f;

            AddSingleWheelParticles(
                m_WheelStates[i],
                m_nWheelStatus[i],
                m_aRatioHistory[nUseWheelLine],
                fTempSpeed,
                &m_aWheelColPoints[nUseWheelLine],
                &vecRearWheelColPoint,
                fOutsideVector,
                i,
                static_cast<uint32>(m_aWheelSkidmarkType[i]),
                &m_bWheelBloody[i],
                nWheelParticleFlags
            );
        }
    }

    m_bLeanMatrixCalculated = false;
    CalculateLeanMatrix();

    const float SPEED_LIMIT_NO_EXHAUSTS = 130.0f;
    const float fAutomobileSpeed        = DotProduct(GetForward(), m_vecMoveSpeed) / 0.0055556f;

    if (vehicleFlags.bEngineOn && !(m_pHandlingData->m_nModelFlags & VEHICLE_HANDLING_MODEL_NO_EXHAUST) && fAutomobileSpeed < SPEED_LIMIT_NO_EXHAUSTS && !vehicleFlags.bIsDrowning && !vehicleFlags.bDisableParticles) {
        AddExhaustParticles();
    }
    AddDamagedVehicleParticles();

    if (m_nModelIndex == MODEL_COPBIKE && vehicleFlags.bSirenOrAlarm && vehicleFlags.bEngineOn && !m_bOffscreen) {
        CVector SirenCoorsL(0.28f, 0.6f, 0.3f);
        CVector SirenCoorsR(-0.28f, 0.6f, 0.3f);

        uint32 TimeVal = CTimer::GetTimeInMS() & 511;
        uint8  Red     = 255;
        uint8  Blue    = 255;

        if (TimeVal < 256) {
            Red = static_cast<uint8>(TimeVal);
            Blue -= static_cast<uint8>(TimeVal);
            CCoronas::RegisterCorona(
                reinterpret_cast<uint32>(this) + 21,
                this,
                static_cast<uint8>(25.5f * CTimeCycle::GetSpriteBrightness()),
                0,
                0,
                255,
                SirenCoorsR,
                0.4f,
                40.0f,
                CORONATYPE_SHINYSTAR,
                FLARETYPE_NONE,
                CORREFL_NONE,
                LOSCHECK_OFF,
                TRAIL_OFF,
                0.0f,
                false,
                1.5f,
                false,
                30.0f,
                false,
                true
            );
        } else {
            Red = static_cast<uint8>(-static_cast<int32>(TimeVal));
            Blue -= static_cast<uint8>(-static_cast<int32>(TimeVal));
            CCoronas::RegisterCorona(
                reinterpret_cast<uint32>(this) + 22,
                this,
                0,
                0,
                static_cast<uint8>(25.5f * CTimeCycle::GetSpriteBrightness()),
                255,
                SirenCoorsL,
                0.4f,
                40.0f,
                CORONATYPE_SHINYSTAR,
                FLARETYPE_NONE,
                CORREFL_NONE,
                LOSCHECK_OFF,
                TRAIL_OFF,
                0.0f,
                false,
                1.5f,
                false,
                30.0f,
                false,
                true
            );
        }

        CPointLights::AddLight(
            0,
            GetPosition() + GetForward() + 0.5f * GetUp(),
            CVector(0.0f, 0.0f, 0.0f),
            10.0f,
            static_cast<float>(Red) * (1.0f / 1024.0f),
            0.0f,
            static_cast<float>(Blue) * (1.0f / 1024.0f),
            0,
            true,
            nullptr
        );
    }

    DoVehicleLights(m_mLeanMatrix, (eVehicleLightsFlags)4);
    CShadows::StoreShadowForVehicle(this, VEH_SHD_BIKE);

    auto* pModelInfo        = CModelInfo::GetModelInfo(m_nModelIndex)->AsVehicleModelInfoPtr();

    float   fSin            = std::sin(m_fSteerAngle);
    float   fCos            = std::cos(m_fSteerAngle);
    CVector vecFrontForward = GetMatrix().TransformVector(CVector(-fSin, fCos, 0.0f));
    CVector vecRearForward  = GetForward();

    if (m_WheelCounts[0] > 0.0f || m_WheelCounts[1] > 0.0f) {
        CVector vecWheelOffset(
            0.0f,
            (pColData->m_pLines[0].m_vecStart.y + pColData->m_pLines[1].m_vecStart.y) * 0.5f,
            pColData->m_pLines[0].m_vecStart.z - (m_fSuspensionLength[0] * std::min(m_aRatioHistory[0], m_aRatioHistory[1])) - (pModelInfo->m_fWheelSizeFront * 0.5f)
        );
        CVector vecWheelSpeed      = GetSpeed(vecWheelOffset);
        float   fWheelSpeed        = ProcessWheelRotation(WHEEL_STATE_NORMAL, vecFrontForward, vecWheelSpeed, pModelInfo->m_fWheelSizeFront * 0.5f);
        m_aWheelAngularVelocity[0] = fWheelSpeed;
        m_aWheelPitchAngles[0] += fWheelSpeed * CTimer::GetTimeStep();
    }

    if (m_WheelCounts[2] > 0.0f || m_WheelCounts[3] > 0.0f) {
        CVector vecWheelOffset(
            0.0f,
            (pColData->m_pLines[2].m_vecStart.y + pColData->m_pLines[3].m_vecStart.y) * 0.5f,
            pColData->m_pLines[2].m_vecStart.z - (m_fSuspensionLength[2] * std::min(m_aRatioHistory[2], m_aRatioHistory[3])) - (pModelInfo->m_fWheelSizeFront * 0.5f)
        );
        CVector vecWheelSpeed      = GetSpeed(vecWheelOffset);
        float   fWheelSpeed        = ProcessWheelRotation(m_WheelStates[1], vecRearForward, vecWheelSpeed, pModelInfo->m_fWheelSizeRear * 0.5f);
        m_aWheelAngularVelocity[1] = fWheelSpeed;
        m_aWheelPitchAngles[1] += fWheelSpeed * CTimer::GetTimeStep();
    }

    CMatrix matrix;
    CVector posn;

    if (m_aBikeNodes[BIKE_FORKS_FRONT]) {
        matrix.Attach(RwFrameGetMatrix(m_aBikeNodes[BIKE_FORKS_FRONT]), false);
        posn = matrix.GetPosition();

        CMatrix  tempMat;
        RwMatrix tempRwMat;
        tempMat.SetUnity();
        tempMat.UpdateRW();

        float   steerRad = pModelInfo->m_fBikeSteerAngle * (PI / 180.0f);
        CVector vecRotAxis(0.0f, std::sin(steerRad), -std::cos(steerRad));
        vecRotAxis.Normalise();

        RwV3d       rotAxis = { vecRotAxis.x, vecRotAxis.y, vecRotAxis.z };
        CQuaternion tempQuat;
        tempQuat.Set(&rotAxis, -m_RideAnimData.BarSteerAngle);
        tempQuat.Get(&tempRwMat);
        tempMat.UpdateMatrix(&tempRwMat);

        matrix.SetUnity();
        matrix *= tempMat;
        matrix.SetTranslateOnly(posn);
        matrix.UpdateRW();

        if (m_aBikeNodes[BIKE_HANDLEBARS]) {
            matrix.Attach(RwFrameGetMatrix(m_aBikeNodes[BIKE_HANDLEBARS]), false);
            posn = matrix.GetPosition();

            if (GetStatus() == STATUS_ABANDONED || GetStatus() == STATUS_WRECKED) {
                matrix.SetUnity();
                matrix *= tempMat;
                matrix.SetTranslateOnly(posn);
            } else {
                matrix.SetTranslate(posn);
            }
            matrix.UpdateRW();
        }
    }

    if (m_aBikeNodes[BIKE_FORKS_REAR]) {
        matrix.Attach(RwFrameGetMatrix(m_aBikeNodes[BIKE_FORKS_REAR]), false);
        posn = matrix.GetPosition();
        if (m_fSwingArmLength > 0.0f) {
            float fRotateAng = -asin((m_aWheelSuspensionHeights[1] - m_aWheelOrigHeights[1]) / m_fSwingArmLength);
            matrix.SetRotate(fRotateAng, 0.0f, 0.0f);
        } else {
            matrix.SetRotate(0.0f, 0.0f, 0.0f);
        }
        matrix.SetTranslateOnly(posn);
        matrix.UpdateRW();
    }

    if (m_aBikeNodes[BIKE_WHEEL_FRONT]) {
        matrix.Attach(RwFrameGetMatrix(m_aBikeNodes[BIKE_WHEEL_FRONT]), false);
        float fFrontLineAvgY = (pColData->m_pLines[0].m_vecStart.y + pColData->m_pLines[1].m_vecStart.y) * 0.5f;
        posn.x               = matrix.GetPosition().x;
        posn.z               = m_aWheelSuspensionHeights[0] - m_fForkZOffset;
        posn.y               = fFrontLineAvgY - m_fForkYOffset - (m_aWheelSuspensionHeights[0] - m_aWheelOrigHeights[0]) * m_fSteerAngleTan;

        if (m_nWheelStatus[0] == 1) {
            matrix.SetRotate(m_aWheelPitchAngles[0], 0.0f, std::sin(m_aWheelPitchAngles[0]) * 0.05f);
        } else {
            matrix.SetRotateX(m_aWheelPitchAngles[0]);
        }
        matrix.SetTranslateOnly(posn);
        matrix.UpdateRW();

        if (m_aBikeNodes[BIKE_MUDGUARD]) {
            matrix.Attach(RwFrameGetMatrix(m_aBikeNodes[BIKE_MUDGUARD]), false);
            matrix.SetTranslateOnly(posn);
            matrix.UpdateRW();
        }
    }

    if (m_aBikeNodes[BIKE_WHEEL_REAR]) {
        matrix.Attach(RwFrameGetMatrix(m_aBikeNodes[BIKE_WHEEL_REAR]), false);
        posn = matrix.GetPosition();
        if (m_nWheelStatus[1] == 1) {
            matrix.SetRotate(m_aWheelPitchAngles[1], 0.0f, std::sin(m_aWheelPitchAngles[1]) * 0.07f);
        } else {
            matrix.SetRotateX(m_aWheelPitchAngles[1]);
        }
        matrix.SetTranslateOnly(posn);
        matrix.UpdateRW();
    }

    if (m_aBikeNodes[BIKE_CHASSIS]) {
        matrix.Attach(RwFrameGetMatrix(m_aBikeNodes[BIKE_CHASSIS]), false);
        posn = CVector(0.0f, 0.0f, (1.0f - std::cos(m_RideAnimData.LeanAngle)) * colModel->m_boundBox.m_vecMin.z * 0.9f);
        matrix.SetRotateX(std::abs(m_RideAnimData.LeanAngle) * -0.05f);
        matrix.RotateY(m_RideAnimData.LeanAngle);
        matrix.SetTranslateOnly(posn);
        matrix.UpdateRW();
    }
}

// 0x6BDE20
void CBike::Render() {
    auto savedRef = 0;
    RwRenderStateGet(rwRENDERSTATEALPHATESTFUNCTIONREF, &savedRef);
    RwRenderStateSet(rwRENDERSTATEALPHATESTFUNCTIONREF, RWRSTATE(1));

    m_nTimeTillWeNeedThisCar = CTimer::GetTimeInMS() + 3'000;
    CVehicle::Render();

    if (m_renderLights.m_bRightFront) {
        CalculateLeanMatrix();
        CVehicle::DoHeadLightBeam(DUMMY_LIGHT_FRONT_MAIN, m_mLeanMatrix, true);
    }

    RwRenderStateSet(rwRENDERSTATEALPHATESTFUNCTIONREF, RWRSTATE(savedRef));
}

// 0x6BCFC0
void CBike::Teleport(CVector destination, bool resetRotation) {
    CWorld::Remove(this);

    GetPosition() = destination;
    if (resetRotation) {
        SetOrientation(0.0f, 0.0f, 0.0f);
    }

    ResetMoveSpeed();
    ResetTurnSpeed();
    ResetSuspension();

    CWorld::Add(this);
}

// 0x6B8EC0
void CBike::VehicleDamage(float damageIntensity, eVehicleCollisionComponent component, CEntity* damager, CVector* vecCollisionCoors, CVector* vecCollisionDirection, eWeaponType weapon) {
    if (damageIntensity > 0.0f || m_fDamageIntensity < 1.0f || !vehicleFlags.bCanBeDamaged) {
        return;
    }

    damageIntensity = m_fDamageIntensity;
    if (GetCreatedBy() == RANDOM_VEHICLE && CStats::GetPercentageProgress() >= 100.0f) {
        damageIntensity *= 0.5f;
    }

    if (bikeFlags.bOnSideStand && damageIntensity > 20.0f) {
        bikeFlags.bOnSideStand = false;
    }

    DamageKnockOffRider(this, m_fDamageIntensity, (uint16)component, damager, m_vecLastCollisionPosn, m_vecLastCollisionImpactVelocity);

    if (m_pDamageEntity && m_pDamageEntity->GetIsTypeVehicle()) {
        m_nLastWeaponDamageType = WEAPON_RAMMEDBYCAR;
        m_pLastDamageEntity     = m_pDamageEntity;
        CEntity::RegisterReference(m_pLastDamageEntity);
    }

    if (!physicalFlags.bCollisionProof) {
        if (!m_pDamageEntity || !m_pDamageEntity->GetIsTypeBuilding() || DotProduct(m_vecLastCollisionImpactVelocity, GetUp()) <= 0.6f) {
            if (damageIntensity > 25.0f && GetStatus() != STATUS_WRECKED) {
                if (vehicleFlags.bIsLawEnforcer) {
                    if (FindPlayerVehicle()) {
                        if (m_pDamageEntity == FindPlayerVehicle() && GetStatus() != STATUS_ABANDONED) {
                            if (FindPlayerVehicle()->m_vecMoveSpeed.Magnitude() >= m_vecMoveSpeed.Magnitude()) {
                                if (FindPlayerVehicle()->m_vecMoveSpeed.Magnitude() > 0.1f) {
                                    FindPlayerPed()->SetWantedLevelNoDrop(eWantedLevel::WANTED_LEVEL_1);
                                }
                            }
                        }
                    }
                }

                damageIntensity = (damageIntensity - 25.0f) * m_pHandlingData->m_fCollisionDamageMultiplier;
                if (damageIntensity > 0.0f) {
                    if (damageIntensity > 5.0f) {
                        if (m_pDriver && m_pDamageEntity && m_pDamageEntity->GetIsTypeVehicle()) {
                            CVehicle* damVeh = m_pDamageEntity->AsVehicle();
                            if (FindPlayerVehicle() != this || damVeh->GetCreatedBy() != MISSION_VEHICLE) {
                                if (damVeh->m_pDriver) {
                                    m_pDriver->Say(CTX_GLOBAL_CAR_CRASH);
                                }
                            }
                        }
                    }

                    float fOldHealth = m_fHealth;
                    if (this == FindPlayerVehicle(-1, false)) {
                        if (!vehicleFlags.bTakeLessDamage) {
                            m_fHealth -= (damageIntensity) / 2.0f;
                        } else {
                            m_fHealth -= (damageIntensity) / 6.0f;
                        }
                    } else if (!vehicleFlags.bTakeLessDamage) {
                        if (m_pDamageEntity && m_pDamageEntity == FindPlayerVehicle()) {
                            m_fHealth -= (damageIntensity) / 1.5f;
                        } else {
                            m_fHealth -= (damageIntensity) / 4.0f;
                        }
                    } else {
                        m_fHealth -= (damageIntensity) / 12.0f;
                    }

                    if (m_fHealth <= 1.0f && fOldHealth > 1.0f) {
                        m_fHealth = 1.0f;
                    }
                }
            }

            if (m_fHealth < 250.0f) {
                if (!bikeFlags.bEngineOnFire) {
                    bikeFlags.bEngineOnFire = true;
                    m_BlowUpTimer           = 0.0f;
                    m_Damager               = m_pDamageEntity;
                    if (m_pDamageEntity) {
                        CEntity::RegisterReference(m_Damager);
                    }
                }
            }
        }
    }
}

// 0x6B89B0
void CBike::SetupSuspensionLines() {
    auto pModelInfo = CModelInfo::GetModelInfo(m_nModelIndex)->AsVehicleModelInfoPtr();
    auto pColModel = pModelInfo->GetColModel();
    auto pColData = pColModel->GetData();

    bool bFirstTime = (pColData->m_pLines[1].m_vecStart.x == 99999.99f);
    bool bResetSuspension = (pColData->m_pLines[0].m_vecStart.z != 99999.99f);

    for (int16 i = 0; i < 4; i++) {
        CVector posn;
        float fYOffset = 0.0f;

        if (bResetSuspension) {
            posn = pColData->m_pLines[i].m_vecStart;
            if (i < 2) {
                posn.z = m_aWheelOrigHeights[0];
            } else {
                posn.z = m_aWheelOrigHeights[1];
            }
        } else {
            RwFrame* pFrame = nullptr;
            switch (i) {
            case 0:
                fYOffset = pModelInfo->m_fWheelSizeFront * -0.5f;
                pFrame = m_aBikeNodes[BIKE_WHEEL_FRONT];
                break;
            case 1:
                pFrame = m_aBikeNodes[BIKE_WHEEL_FRONT];
                fYOffset = pModelInfo->m_fWheelSizeFront * 0.5f;
                break;
            case 2:
                fYOffset = pModelInfo->m_fWheelSizeRear * -0.5f;
                pFrame = m_aBikeNodes[BIKE_WHEEL_REAR];
                break;
            case 3:
                pFrame = m_aBikeNodes[BIKE_WHEEL_REAR];
                fYOffset = pModelInfo->m_fWheelSizeRear * 0.5f;
                break;
            }

            RwMatrix tempNodeMatrx;
            tempNodeMatrx = pFrame->modelling;
            RwFrame* pRootFrame = RpClumpGetFrame(GetRpClump());
            RwFrame* pParent = RwFrameGetParent(pFrame);
            if (pParent) {
                do {
                    RwMatrixTransform(&tempNodeMatrx, &pParent->modelling, rwCOMBINEPOSTCONCAT);
                    pParent = RwFrameGetParent(pParent);
                } while (pParent != pRootFrame && pParent);
            }

            posn = *reinterpret_cast<CVector*>(&tempNodeMatrx.pos);

            if (i == 0) {
                m_aWheelOrigHeights[0] = posn.z;
            } else if (i == 2) {
                m_aWheelOrigHeights[1] = posn.z;
                if (m_aBikeNodes[BIKE_FORKS_REAR]) {
                    tempNodeMatrx = m_aBikeNodes[BIKE_FORKS_REAR]->modelling;
                    pParent = RwFrameGetParent(m_aBikeNodes[BIKE_FORKS_REAR]);
                    if (pParent) {
                        do {
                            RwMatrixTransform(&tempNodeMatrx, &pParent->modelling, rwCOMBINEPOSTCONCAT);
                            pParent = RwFrameGetParent(pParent);
                        } while (pParent != pRootFrame && pParent);
                    }
                    m_fSwingArmLength = std::sqrt(sq(posn.y - tempNodeMatrx.pos.y) + sq(posn.z - tempNodeMatrx.pos.z));
                } else {
                    m_fSwingArmLength = 0.0f;
                }
            }

            posn.y += fYOffset;
        }

        posn.z += m_pHandlingData->m_fSuspensionUpperLimit;
        pColData->m_pLines[i].m_vecStart = posn;

        float fWheelRadius = (i == 0 || i == 1) ? pModelInfo->m_fWheelSizeFront * 0.5f : pModelInfo->m_fWheelSizeRear * 0.5f;
        posn.z += -m_pHandlingData->m_fSuspensionUpperLimit + m_pHandlingData->m_fSuspensionLowerLimit - fWheelRadius;
        pColData->m_pLines[i].m_vecEnd = posn;

        m_fSuspensionLength[i] = m_pHandlingData->m_fSuspensionUpperLimit - m_pHandlingData->m_fSuspensionLowerLimit;
        m_fLineLength[i] = pColData->m_pLines[i].m_vecStart.z - pColData->m_pLines[i].m_vecEnd.z;
    }

    if (!bResetSuspension) {
        RwMatrix tempNodeMatrx;
        tempNodeMatrx = m_aBikeNodes[BIKE_FORKS_FRONT]->modelling;
        RwFrame* pRootFrame = RpClumpGetFrame(GetRpClump());
        RwFrame* pParent = RwFrameGetParent(m_aBikeNodes[BIKE_FORKS_FRONT]);
        if (pParent) {
            do {
                RwMatrixTransform(&tempNodeMatrx, &pParent->modelling, rwCOMBINEPOSTCONCAT);
                pParent = RwFrameGetParent(pParent);
            } while (pParent != pRootFrame && pParent);
        }

        m_fForkYOffset = tempNodeMatrx.pos.y;
        m_fForkZOffset = tempNodeMatrx.pos.z;
    }

    float fStart = pColData->m_pLines[0].m_vecStart.z;
    float fLength = m_fSuspensionLength[0] * (1.0f - 1.0f / (4.0f * m_pHandlingData->m_fSuspensionForceLevel));
    m_fHeightAboveRoad = -fStart + fLength + pModelInfo->m_fWheelSizeFront * 0.5f;

    for (int16 i = 0; i < 2; i++) {
        m_aWheelSuspensionHeights[i] = (i == 0 ? pModelInfo->m_fWheelSizeFront : pModelInfo->m_fWheelSizeRear) * 0.5f - m_fHeightAboveRoad;
    }

    if (pColData->m_pLines[0].m_vecEnd.z < pColModel->GetBoundingBox().m_vecMin.z) {
        pColModel->GetBoundingBox().m_vecMin.z = pColData->m_pLines[0].m_vecEnd.z;
    }

    fLength = pColModel->GetBoundingBox().m_vecMin.Magnitude();
    if (pColModel->GetBoundingBox().m_vecMax.Magnitude() > fLength) {
        fLength = pColModel->GetBoundingBox().m_vecMax.Magnitude();
    }
    if (pColModel->GetBoundingSphere().m_fRadius < fLength) {
        pColModel->GetBoundingSphere().m_fRadius = fLength;
    }

    if ((m_pHandlingData->m_nHandlingFlags & 0x40000000) && bFirstTime) {
        float MIN_CLEARANCE_ON_MODEL = 0.1f;
        float fMinPos = 0.3f - m_fHeightAboveRoad;
        for (int16 i = 0; i < pColData->m_nNumBoxes; i++) {
            auto& box = pColData->m_pBoxes[i];
            if (box.m_vecMax.z - box.m_vecMin.z < fMinPos) {
                if (box.m_vecMin.z < MIN_CLEARANCE_ON_MODEL) {
                    if (box.m_vecMax.z - fMinPos < MIN_CLEARANCE_ON_MODEL) {
                        box.m_vecMin.z = MIN_CLEARANCE_ON_MODEL;
                    } else {
                        box.m_vecMin.z = box.m_vecMax.z - fMinPos;
                    }
                }
                box.m_vecMax.z = box.m_vecMin.z + fMinPos;
            }
        }
    }
}

// 0x6B8970
void CBike::SetModelIndex(uint32 index) {
    CVehicle::SetModelIndex(index);
    SetupModelNodes();
}

// 0x6B5960
void CBike::SetupModelNodes() {
    std::ranges::fill(m_aBikeNodes, nullptr);
    CClumpModelInfo::FillFrameArray(GetRpClump(), m_aBikeNodes.data());
}

// 0x6B7080
void CBike::PlayCarHorn() {
    if ((m_nAlarmState && m_nAlarmState != (uint16)-1 && GetStatus() != STATUS_WRECKED) || m_HornCounter) {
        return;
    }

    if (m_nCarHornTimer) {
        m_nCarHornTimer--;
        return;
    }

    m_nCarHornTimer = static_cast<uint8>(150 + (CGeneral::GetRandomNumber() & 0x7F));
    const auto r = m_nCarHornTimer & 0x7;
    if (r < 2) {
        m_HornCounter = 45;
    } else if (r < 4) {
        if (m_pDriver && m_autoPilot.SlowingDownForCar) {
            m_pDriver->Say(CTX_GLOBAL_BLOCKED);
        }
        m_HornCounter = 45;
    } else {
        if (m_pDriver) {
            m_pDriver->Say(CTX_GLOBAL_BLOCKED);
        }
    }
}

// 0x6B7070
void CBike::SetupDamageAfterLoad() {
    // NOP
}

constexpr CVector vecTestResistance(0.9995f, 0.9f, 0.95f);
constexpr float fDAxisX                 = 1.0f;
constexpr float fDAxisXExtra            = 100.0f;
constexpr float fInAirXRes              = 0.98f;
constexpr float fDAxisY                 = 1000.0f;
constexpr float fTestSkidMult1          = 0.7f;
constexpr float fTestSkidMult2          = 0.7f;
constexpr float fRiderStabiliseForce    = -0.07f;
constexpr float fPickUpStabiliseForce   = -0.1f;
constexpr float BIKE_BURNOUT_STEER_MULT = -0.0007f;

// 0x6B9250
void CBike::ProcessControl() {
    bool bSkipPhysics = false;
    uint32 nBikePCFlags = 0;

    m_vehicleAudio.Service();

    m_bLeanMatrixCalculated = false;
    m_nBrakesOn = 0;
    bikeFlags.bPlayerBoost = false;
    vehicleFlags.bAudioChangingGear = false;
    vehicleFlags.bVehicleColProcessed = false;
    vehicleFlags.bRestingOnPhysical   = false;

    if (CReplay::Mode == 1) {
        return;
    }

    ProcessCarAlarm();
    ActivateBombWhenEntered();
    UpdateClumpAlpha();

    if (m_pDriver && (m_pDriver->IsPlayer() || (m_apPassengers[0] && m_apPassengers[0]->IsPlayer()))) {
        if (m_nTestPedCollision == 1) {
            m_nTestPedCollision = 2;
        } else if (m_nTestPedCollision < 1) {
            m_nTestPedCollision = 1;
        }
    } else {
        m_nTestPedCollision = 0;
    }

    ProcessAI(nBikePCFlags);

    if (GetStatus() == STATUS_SIMPLE) {
        return;
    }

    if (!(nBikePCFlags & 2) && !(bikeFlags.bOnSideStand || bikeFlags.bGettingPickedUp)) {
        m_vecCentreOfMass.x = m_pHandlingData->m_vecCentreOfMass.x;
        m_vecCentreOfMass.y = m_pHandlingData->m_vecCentreOfMass.y;
        m_vecCentreOfMass.z = m_BikeHandling->m_fNoPlayerCOMz;
    } else {
        float fDx = fDAxisX;
        CVector TurnResistance = vecTestResistance;
        CVector m_vecTurnSpeedTemp = GetMatrix().InverseTransformVector(m_vecTurnSpeed);

        if (GetStatus() == STATUS_PLAYER) {
            if (m_aWheelRatios[0] >= 1.0f && m_aWheelRatios[1] >= 1.0f) {
                float fMinWheelieDampingMod = CStats::GetFatAndMuscleModifier(STAT_MOD_13) * 0.2f;
                if (m_aWheelRatios[2] >= 1.0f && m_aWheelRatios[3] >= 1.0f) {
                    fDx = fDAxisXExtra;
                    TurnResistance.x = fInAirXRes;
                } else {
                    fDx = fDAxisXExtra;
                    if (GetForward().z <= 0.0f) {
                        TurnResistance.x = fInAirXRes;
                    } else {
                        TurnResistance.x = TurnResistance.x - std::min(fMinWheelieDampingMod * std::abs(m_BikeHandling->m_fWheelieAng - GetForward().z), 0.05f);
                    }
                }
            } else if (m_WheelCounts[2] <= 0.0f && m_WheelCounts[3] <= 0.0f) {
                float fMinWheelieDampingMod = CStats::GetFatAndMuscleModifier(STAT_MOD_13) * 0.075f;
                float fWheelieDampingMult = CStats::GetFatAndMuscleModifier(STAT_MOD_13) * 0.25f;
                fDx = fDAxisXExtra;
                if (GetForward().z < 0.0f) {
                    TurnResistance.x = TurnResistance.x * (std::min(fMinWheelieDampingMod, fWheelieDampingMult * std::abs(m_BikeHandling->m_fStoppieAng - GetForward().z)) + 0.9f);
                }
            }
        }

        float fBalanceSpeedMult = CTimer::GetTimeStep();
        float fBaseX = TurnResistance.x * (1.0f / ((m_vecTurnSpeedTemp.x * (m_vecTurnSpeedTemp.x * fDx)) + 1.0f));
        float fBaseY = TurnResistance.y * (1.0f / ((m_vecTurnSpeedTemp.y * (m_vecTurnSpeedTemp.y * fDAxisY)) + 1.0f));
        float fTempTurnForceX = std::pow(std::max(fBaseX, 0.0f), CTimer::GetTimeStep());
        float fTempTurnForceY = std::pow(std::max(fBaseY, 0.0f), fBalanceSpeedMult);

        float v26 = -(m_vecTurnSpeedTemp.y - (m_vecTurnSpeedTemp.y * fTempTurnForceY));
        float v27 = -(m_vecTurnSpeedTemp.x - (m_vecTurnSpeedTemp.x * fTempTurnForceX));

        ApplyTurnForce(GetUp() * (v26 * m_fTurnMass * -1.0f), GetRight() + GetMatrix().TransformVector(m_vecCentreOfMass));
        ApplyTurnForce(GetUp() * (v27 * m_fTurnMass), GetForward() + GetMatrix().TransformVector(m_vecCentreOfMass));

        if (GetStatus() != STATUS_PLAYER) {
            m_vecCentreOfMass = m_pHandlingData->m_vecCentreOfMass;
        }
    }

    if (!physicalFlags.bProcessCollisionEvenIfStationary && (GetStatus() == STATUS_ABANDONED || GetStatus() == STATUS_WRECKED)) {
        if (!bikeFlags.bOnSideStand) {
            bool isVehicleIdle;
            if (vehicleFlags.bVehicleColProcessed) {
                isVehicleIdle = false;
            } else {
                isVehicleIdle = (m_vecMoveSpeed == CVector(0.0f, 0.0f, 0.0f) && m_aRatioHistory[3] != 1.0f);
            }

            float forceLimitRadius = 0.005f;
            float torqueLimitRadius = 0.0015f;
            float movingSpeedLimit = 0.005f;
            if (GetStatus() == STATUS_WRECKED) {
                forceLimitRadius = 0.01f;
                torqueLimitRadius = 0.0025f;
                movingSpeedLimit = 0.015f;
            }

            m_vecForce = (m_vecForce + m_vecMoveSpeed) * 0.5f;
            m_vecTorque = (m_vecTorque + m_vecTurnSpeed) * 0.5f;

            bool resetSpeed = true;
            forceLimitRadius *= CTimer::GetTimeStep();
            torqueLimitRadius *= CTimer::GetTimeStep();
            if (forceLimitRadius * forceLimitRadius < m_vecForce.SquaredMagnitude()
                || torqueLimitRadius * torqueLimitRadius < m_vecTorque.SquaredMagnitude()
                || movingSpeedLimit <= m_fMovingSpeed) {
                resetSpeed = false;
            }

            if (!resetSpeed && !isVehicleIdle) {
                m_nFakePhysics = 0;
            } else {
                CVector currentPos = GetPosition();
                // if (!CCarCtrl::MapCouldMoveInThisArea(currentPos.x, currentPos.y)) {
                if (!false) {
                    m_nFakePhysics++;
                    if (m_nFakePhysics > 10 || isVehicleIdle) {
                        if (!isVehicleIdle || m_nFakePhysics > 10)
                            m_nFakePhysics = 10;
                        m_vecMoveSpeed = CVector(0.0f, 0.0f, 0.0f);
                        m_vecTurnSpeed = CVector(0.0f, 0.0f, 0.0f);
                        bSkipPhysics = true;
                    }
                } else {
                    m_nFakePhysics = 0;
                }
            }
        }
    }

    for (int32 i = 0; i < 4; i++) {
        if (m_aGroundPhysicalPtrs[i]) {
            vehicleFlags.bRestingOnPhysical = true;
            if (CWorld::bForceProcessControl) {
                if (m_aGroundPhysicalPtrs[i]->GetIsInSafePosition()) {
                    SetWasPostponed(true);
                    return;
                }
            }
        }
    }

    if (vehicleFlags.bRestingOnPhysical) {
        bSkipPhysics = false;
        m_nFakePhysics = 0;
    }

    VehicleDamage(0.0f, eVehicleCollisionComponent::DEFAULT, nullptr, nullptr, nullptr, WEAPON_UNARMED);

    bool bStabiliseToVertical = bikeFlags.bOnSideStand;
    if (m_fDamageIntensity > 0.0f) {
        if (std::abs(DotProduct(m_vecLastCollisionImpactVelocity, GetRight())) > 0.5f && m_vecMoveSpeed.SquaredMagnitude() < 0.1f) {
            bStabiliseToVertical = true;
        }
    }

    auto mi = CModelInfo::GetModelInfo(m_nModelIndex)->AsVehicleModelInfoPtr();
    auto colData = GetColModel()->m_pColData;

    if (bSkipPhysics) {
        CPhysical::SkipPhysics();
        vehicleFlags.bVehicleColProcessed = false;
        vehicleFlags.bAudioChangingGear = false;
        if (bikeFlags.bGettingPickedUp) {
            if (m_RideAnimData.BarSteerAngle < 0.34907f)
                m_RideAnimData.BarSteerAngle += CTimer::GetTimeStep() * 0.017453f;
            float fTimeStepCorrectedLeanReturn = std::pow(0.97f, CTimer::GetTimeStep());
            float fRightUp = std::asin(std::clamp(GetRight().z, -1.0f, 1.0f));
            float fDesiredLeanAngle = -(((fRightUp + 0.2618f) - (fTimeStepCorrectedLeanReturn * (fRightUp + 0.2618f))) - (fTimeStepCorrectedLeanReturn * m_RideAnimData.DesiredLeanAngle));
            m_RideAnimData.DesiredLeanAngle = fDesiredLeanAngle;
            m_RideAnimData.LeanAngle = fDesiredLeanAngle;
        }
    } else {
        if (!vehicleFlags.bVehicleColProcessed) {
            ProcessControlCollisionCheck(true);
        }

        if ((nBikePCFlags & 2) == 0 && !(bikeFlags.bOnSideStand || bikeFlags.bGettingPickedUp)) {
            if (GetRight().z >= 0.0f) {
                if (m_fSteerAngle < 0.43633f)
                    m_fSteerAngle += CTimer::GetTimeStep() * 0.0087266f;
            } else if (m_fSteerAngle > -0.43633f) {
                m_fSteerAngle -= CTimer::GetTimeStep() * 0.0087266f;
            }
        }

        float fAirResistanceOld = m_fAirResistance;
        if (GetStatus() == STATUS_PLAYER && m_pDriver) {
            CAnimBlendAssociation* pAnimFwd = RpAnimBlendClumpGetAssociation(m_pDriver->GetRpClump(), ANIM_ID_BIKE_FWD);
            if (pAnimFwd && pAnimFwd->GetBlendAmount() > 0.5f) {
                float fCurrentTime = pAnimFwd->GetCurrentTime();
                if (fCurrentTime > 0.06f && fCurrentTime < 0.14f) {
                    m_fAirResistance *= CCullZones::DoExtraAirResistanceForPlayer() ? 0.85f : 0.6f;
                    if (m_GasPedal > 0.5f) {
                        CVector fwd = GetForward();
                        if (DotProduct(m_vecMoveSpeed, fwd) > 0.25f) {
                            float v85 = CTimer::GetTimeStep() * ((m_fMass * 0.2f) * 0.008f);
                            ApplyMoveForce(fwd * v85);
                            bikeFlags.bPlayerBoost = true;
                        }
                    }
                }
            }
        }

        bool bPreviouslyInWater = physicalFlags.bTouchingWater;
        CPhysical::ProcessControl();
        m_fAirResistance = fAirResistanceOld;
        ProcessBuoyancy();

        if (!bPreviouslyInWater && physicalFlags.bTouchingWater) {
            if (m_pDriver && m_pDriver->IsPlayer()) {
                m_pDriver->AsPlayer()->ResetPlayerBreath();
            } else if (m_nMaxPassengers) {
                for (int32 p = 0; p < m_nMaxPassengers; p++) {
                    if (m_apPassengers[p] && m_apPassengers[p]->IsPlayer())
                        m_apPassengers[p]->AsPlayer()->ResetPlayerBreath();
                }
            }
        }

        for (int32 i = 0; i < 4; i++) {
            if (m_fLineLength[i] > 0.0001f && m_fSuspensionLength[i] > 0.0f) {
                float ratio = 1.0f - (m_fSuspensionLength[i] / m_fLineLength[i]);
                if (1.0f - ratio > 0.0001f) {
                    m_aWheelRatios[i] = (m_aWheelRatios[i] - ratio) / (1.0f - ratio);
                } else {
                    m_aWheelRatios[i] = 1.0f;
                }
            } else {
                m_aWheelRatios[i] = 1.0f;
            }
        }

        float aWheelSpringForces[4];
        CVector aWheelDirections[4];
        CVector aWheelSpeeds[4];
        CVector aWheelOffsets[4];
        for (int32 i = 0; i < 4; i++) {
            if (m_aWheelRatios[i] < 1.0f) {
                aWheelOffsets[i] = m_aWheelColPoints[i].m_vecPoint - GetPosition();
                aWheelDirections[i] = GetMatrix().TransformVector(colData->m_pLines[i].m_vecEnd - colData->m_pLines[i].m_vecStart);
                aWheelDirections[i].Normalise();
            }
        }

        m_aWheelSkidmarkType[0] = eSkidmarkType::DEFAULT;
        m_aWheelSkidmarkType[1] = eSkidmarkType::DEFAULT;
        m_bMoreSkidMarks[0] = false;
        m_bMoreSkidMarks[1] = false;

        for (int32 i = 0; i < 4; i++) {
            float v117 = m_aWheelRatios[i];
            if (v117 >= 1.0f) {
                aWheelOffsets[i] = GetMatrix().TransformVector(colData->m_pLines[i].m_vecStart);
            } else {
                float fSuspensionBias = m_pHandlingData->m_fSuspensionBiasBetweenFrontAndRear;
                float fSuspensionForce = m_pHandlingData->m_fSuspensionForceLevel;
                if ((i | 1) == 3)
                    fSuspensionBias = 1.0f - fSuspensionBias;

                if (m_aWheelColPoints[i].m_vecNormal.z <= 0.35f)
                    ApplySpringCollision(fSuspensionForce, aWheelDirections[i], aWheelOffsets[i], v117, fSuspensionBias, aWheelSpringForces[i]);
                else
                    ApplySpringCollisionAlt(fSuspensionForce, aWheelDirections[i], aWheelOffsets[i], v117, fSuspensionBias, m_aWheelColPoints[i].m_vecNormal, aWheelSpringForces[i]);

                eSkidmarkType SkidmarkType = static_cast<eSkidmarkType>(g_surfaceInfos.GetSkidmarkType(m_aWheelColPoints[i].m_nSurfaceTypeB));
                m_aWheelSkidmarkType[i > 1] = SkidmarkType;
                if (SkidmarkType == eSkidmarkType::SANDY)
                    m_bMoreSkidMarks[i > 1] = true;
            }
        }

        for (int32 i = 0; i < 4; i++) {
            aWheelSpeeds[i] = GetSpeed(aWheelOffsets[i]);
            if (m_aGroundPhysicalPtrs[i]) {
                aWheelSpeeds[i] -= m_aGroundPhysicalPtrs[i]->GetSpeed(m_aGroundOffsets[i]);
            }
        }

        if (m_aWheelRatios[0] < 1.0f || m_aWheelRatios[1] < 1.0f) {
            int32 idx = (m_aWheelRatios[0] >= 1.0f) ? 1 : 0;
            if (m_aWheelColPoints[idx].m_vecNormal.z > 0.35f) {
                aWheelDirections[0] = -m_aWheelColPoints[idx].m_vecNormal;
            }
            int32 idx1 = (m_aWheelRatios[1] < 1.0f) ? 1 : 0;
            if (m_aWheelColPoints[idx1].m_vecNormal.z > 0.35f) {
                aWheelDirections[1] = -m_aWheelColPoints[idx1].m_vecNormal;
            }
        }

        if (m_aWheelRatios[2] < 1.0f || m_aWheelRatios[3] < 1.0f) {
            int32 idx2 = (m_aWheelRatios[2] < 1.0f) ? 2 : 3;
            if (m_aWheelColPoints[idx2].m_vecNormal.z > 0.35f) {
                aWheelDirections[2] = -m_aWheelColPoints[idx2].m_vecNormal;
            }
            int32 idx3 = (m_aWheelRatios[3] < 1.0f) ? 3 : 2;
            if (m_aWheelColPoints[idx3].m_vecNormal.z > 0.35f) {
                aWheelDirections[3] = -m_aWheelColPoints[idx3].m_vecNormal;
            }
        }

        if (m_aWheelRatios[0] < 1.0f)
            ApplySpringDampening(m_pHandlingData->m_fSuspensionDampingLevel, aWheelSpringForces[0], aWheelDirections[0], aWheelOffsets[0], aWheelSpeeds[0]);
        if (m_aWheelRatios[1] < 1.0f)
            ApplySpringDampening(m_pHandlingData->m_fSuspensionDampingLevel, aWheelSpringForces[1], aWheelDirections[1], aWheelOffsets[1], aWheelSpeeds[1]);
        if (m_aWheelRatios[2] < 1.0f)
            ApplySpringDampening(m_pHandlingData->m_fSuspensionDampingLevel, aWheelSpringForces[2], aWheelDirections[2], aWheelOffsets[2], aWheelSpeeds[2]);
        if (m_aWheelRatios[3] < 1.0f)
            ApplySpringDampening(m_pHandlingData->m_fSuspensionDampingLevel, aWheelSpringForces[3], aWheelDirections[3], aWheelOffsets[3], aWheelSpeeds[3]);

        for (int32 i = 0; i < 4; i++) {
            aWheelSpeeds[i] = GetSpeed(aWheelOffsets[i]);
            if (m_aGroundPhysicalPtrs[i]) {
                aWheelSpeeds[i] -= m_aGroundPhysicalPtrs[i]->GetSpeed(m_aGroundOffsets[i]);
            }
        }

        float fSpeed = DotProduct(m_vecMoveSpeed, GetForward());
        float fDriveAcceleration = m_pHandlingData->m_transmissionData.CalculateDriveAcceleration(
            m_GasPedal, m_nCurrentGear, m_fGearChangeCount, fSpeed, nullptr, nullptr, m_NumDriveWheelsOnGround, CCheat::IsActive(CHEAT_PERFECT_HANDLING)
        );
        fDriveAcceleration /= m_fVelocityFrequency;

        float fBrakeDecel = m_BrakePedal * m_pHandlingData->m_fBrakeDeceleration;
        float fFrontBrakeBias, fRearBrakeBias, fRearTractionBias, fFrontTractionBias;
        if (GetStatus() != STATUS_PLAYER && m_pHandlingData->m_bProcRearwheelFirst) {
            fFrontBrakeBias = 1.0f;
            fRearBrakeBias = 1.0f;
            fRearTractionBias = 1.0f;
            fFrontTractionBias = 1.0f;
        } else {
            fFrontBrakeBias = m_pHandlingData->m_fBrakeBias * 2.0f;
            fRearBrakeBias = (1.0f - m_pHandlingData->m_fBrakeBias) * 2.0f;
            fRearTractionBias = m_pHandlingData->m_fTractionBias * 2.0f;
            fFrontTractionBias = 2.0f - fRearTractionBias;
        }

        float fDriveForce = fDriveAcceleration;
        float fBrakeForce = fBrakeDecel * CTimer::GetTimeStep();

        m_NumDriveWheelsOnGroundLastFrame = m_NumDriveWheelsOnGround;
        m_nNoOfContactWheels = 0;
        m_NumDriveWheelsOnGround = 0;

        for (int32 i = 0; i < 4; i++) {
            if (m_aWheelRatios[i] >= 1.0f) {
                m_WheelCounts[i] = m_WheelCounts[i] - CTimer::GetTimeStep();
                if (m_WheelCounts[i] <= 0.0f) {
                    m_WheelCounts[i] = 0.0f;
                }
            } else {
                m_WheelCounts[i] = 4.0f;
                if (m_nNoOfContactWheels == 0) {
                    m_vecAveGroundNormal = m_aWheelColPoints[i].m_vecNormal;
                } else {
                    m_vecAveGroundNormal += m_aWheelColPoints[i].m_vecNormal;
                }
                m_nNoOfContactWheels++;
                if (i >= 2) {
                    m_NumDriveWheelsOnGround = 1;
                }
            }
        }

        if (m_nNoOfContactWheels) {
            m_vecAveGroundNormal /= static_cast<float>(m_nNoOfContactWheels);
            if (DotProduct(m_vecAveGroundNormal, GetUp()) < -0.5f) {
                m_vecAveGroundNormal = -m_vecAveGroundNormal;
            }
        } else {
            m_vecAveGroundNormal = CVector(0.0f, 0.0f, 1.0f);
        }

        bool bFrontWheelPair = (m_aWheelRatios[0] >= m_aWheelRatios[1]);
        int32 nFrontWheelLine = bFrontWheelPair ? 1 : 0;
        int32 nRearWheelLine = (m_aWheelRatios[2] >= m_aWheelRatios[3]) ? 3 : 2;

        CVector vecFrontOffset(0.0f, colData->m_pLines[0].m_vecStart.y, (colData->m_pLines[0].m_vecStart.z - (m_aWheelRatios[nFrontWheelLine] * m_fSuspensionLength[0])) - (mi->m_fWheelSizeFront * 0.5f));
        CVector vecFrontResolvedOffset = GetMatrix().TransformVector(vecFrontOffset);

        CVector vecRearOffset(0.0f, colData->m_pLines[2].m_vecStart.y, (colData->m_pLines[2].m_vecStart.z - (m_aWheelRatios[nRearWheelLine] * m_fSuspensionLength[2])) - (mi->m_fWheelSizeRear * 0.5f));
        CVector vecRearResolvedOffset = GetMatrix().TransformVector(vecRearOffset);

        float fAdhesiveScalar = ((m_fExtraTractionMult * 0.004f) * m_pHandlingData->m_fTractionMultiplier) * 0.25f;

        float fSteerAngleForWheels;
        if (GetStatus() != STATUS_PLAYER && bikeFlags.bGettingPickedUp) {
            if (m_RideAnimData.BarSteerAngle < 0.34907f)
                m_RideAnimData.BarSteerAngle += CTimer::GetTimeStep() * 0.02618f;
            fSteerAngleForWheels = m_RideAnimData.BarSteerAngle;
        } else if (std::abs(m_vecMoveSpeed.x) < 0.01f && std::abs(m_vecMoveSpeed.y) < 0.01f && m_fSteerAngle == 0.0f) {
            m_RideAnimData.BarSteerAngle = std::pow(0.96f, CTimer::GetTimeStep()) * m_RideAnimData.BarSteerAngle;
            fSteerAngleForWheels = m_RideAnimData.BarSteerAngle;
        } else {
            float fSteerScale = 1.0f;
            if (fSpeed > 0.01f && (m_WheelCounts[0] > 0.0f || m_WheelCounts[1] > 0.0f)) {
                if (GetStatus() == STATUS_PLAYER) {
                    CColPoint testColPoint;
                    testColPoint.m_nSurfaceTypeA = SURFACE_WHEELBASE;
                    testColPoint.m_nSurfaceTypeB = SURFACE_TARMAC;
                    float fSpeedSteerLimit = fAdhesiveScalar * (m_BikeHandling->m_fSpeedSteer * 4.0f) * g_surfaceInfos.GetAdhesiveLimit(&testColPoint);
                    uint8 nRearSurface = m_aWheelColPoints[nRearWheelLine].m_nSurfaceTypeB;
                    if (g_surfaceInfos.GetAdhesionGroup(nRearSurface) == ADHESION_GROUP_LOOSE || g_surfaceInfos.GetAdhesionGroup(nRearSurface) == ADHESION_GROUP_SAND) {
                        fSpeedSteerLimit *= m_BikeHandling->m_fSlipSteer;
                    }
                    float fSteerDenom = fSpeed * fSpeed;
                    float fSteerLock = m_pHandlingData->m_fSteeringLock * 3.14159265f / 180.0f;
                    if (fSteerDenom > 0.0001f && fSteerLock > 0.0001f) {
                        float fVal = std::clamp(fSpeedSteerLimit / fSteerDenom, -1.0f, 1.0f);
                        float fMaxSteerAngle = std::asin(fVal);
                        float fSteerRatio = fMaxSteerAngle / fSteerLock;
                        if ((m_fSteerAngle < 0.0f && m_RideAnimData.LeanAngle < 0.0f) || (m_fSteerAngle > 0.0f && m_RideAnimData.LeanAngle > 0.0f)) {
                            fSteerRatio += fSteerRatio;
                        }
                        fSteerScale = std::min(fSteerRatio, 1.0f);
                    }
                }
            }
            if (GetStatus() != STATUS_PLAYER) {
                fSteerScale = 1.0f;
            }
            fSteerAngleForWheels = m_fSteerAngle * fSteerScale;
            m_RideAnimData.BarSteerAngle = fSteerAngleForWheels;
        }

        CVector vecMoveSpeedBefore = m_vecMoveSpeed;
        tWheelState wheelStates[2] = { m_WheelStates[0], m_WheelStates[1] };

        if (!m_pHandlingData->m_bProcRearwheelFirst) {
            if (m_WheelCounts[0] <= 0.0f && m_WheelCounts[1] <= 0.0f) {
                m_aWheelAngularVelocity[0] *= 0.95f;
                m_aWheelPitchAngles[0] += m_aWheelAngularVelocity[0];
            } else {
                float fSin = std::sin(fSteerAngleForWheels);
                float fCos = std::cos(fSteerAngleForWheels);
                CVector vecForward(-fSin, fCos, 0.0f);
                vecForward = GetMatrix().TransformVector(vecForward);

                float fAlongNormal = DotProduct(vecForward, m_aWheelColPoints[nFrontWheelLine].m_vecNormal);
                vecForward -= m_aWheelColPoints[nFrontWheelLine].m_vecNormal * fAlongNormal;
                vecForward.Normalise();

                CVector vecRight = CrossProduct(vecForward, m_aWheelColPoints[nFrontWheelLine].m_vecNormal);
                vecRight.Normalise();

                if (bStabiliseToVertical)
                    vecRight.z = 0.0f;

                float fThrust = 0.0f;
                m_aWheelColPoints[nFrontWheelLine].m_nSurfaceTypeA = SURFACE_WHEELBASE;
                float fSideAdhesiveMult = 1.0f;
                if (m_fBrakingSlide > 0.0f) {
                    if ((g_surfaceInfos.GetAdhesionGroup(m_aWheelColPoints[nFrontWheelLine].m_nSurfaceTypeB) - 1) >= 3)
                        fSideAdhesiveMult = 1.0f;
                    else
                        fSideAdhesiveMult = 0.7f;
                }

                float fAdhesiveLimit = fAdhesiveScalar * g_surfaceInfos.GetAdhesiveLimit(&m_aWheelColPoints[nFrontWheelLine]);
                if (GetStatus() == STATUS_PLAYER)
                    fAdhesiveLimit *= g_surfaceInfos.GetWetMultiplier(m_aWheelColPoints[nFrontWheelLine].m_nSurfaceTypeB);

                if (m_nWheelStatus[0] == eCarWheelStatus::WHEEL_STATUS_BURST)
                    fAdhesiveLimit *= 0.4f;

                wheelStates[0] = m_WheelStates[0];
                CVector vecWheelSpeed = GetSpeed(vecFrontResolvedOffset);
                if (m_aGroundPhysicalPtrs[nFrontWheelLine])
                    vecWheelSpeed -= m_aGroundPhysicalPtrs[nFrontWheelLine]->GetSpeed(m_aGroundOffsets[nFrontWheelLine]);

                ProcessBikeWheel(
                    vecForward, vecRight, vecWheelSpeed, vecFrontResolvedOffset,
                    2, fThrust, fBrakeForce * fFrontBrakeBias, fFrontTractionBias * fAdhesiveLimit, fSideAdhesiveMult,
                    0, m_aWheelAngularVelocity.data(), wheelStates, BIKE_WHEEL_F_STD, m_nWheelStatus[0]
                );

                if ((nBikePCFlags & 4) != 0 && (wheelStates[0] == WHEEL_STATE_SPINNING || wheelStates[0] == WHEEL_STATE_SKIDDING))
                    wheelStates[0] = WHEEL_STATE_NORMAL;
            }
        }

        if (m_WheelCounts[2] <= 0.0f && m_WheelCounts[3] <= 0.0f) {
            if (vehicleFlags.bIsHandbrakeOn) {
                m_aWheelAngularVelocity[1] = 0.0f;
            } else if (fDriveForce == 0.0f) {
            } else if (fDriveForce <= 0.0f) {
                if (m_aWheelAngularVelocity[1] > -1.0f)
                    m_aWheelAngularVelocity[1] += 0.05f;
            } else if (m_aWheelAngularVelocity[1] < 1.0f) {
                m_aWheelAngularVelocity[1] -= 0.1f;
            }
            m_aWheelPitchAngles[1] += m_aWheelAngularVelocity[1] * CTimer::GetTimeStep();
        } else {
            CVector vecForward = GetForward();
            float fAlongNormal = DotProduct(vecForward, m_aWheelColPoints[nRearWheelLine].m_vecNormal);
            vecForward -= m_aWheelColPoints[nRearWheelLine].m_vecNormal * fAlongNormal;
            vecForward.Normalise();

            CVector vecRight = CrossProduct(vecForward, m_aWheelColPoints[nRearWheelLine].m_vecNormal);
            vecRight.Normalise();

            if (bStabiliseToVertical)
                vecRight.z = 0.0f;

            float fRearBrakeFriction;
            float fRearAdhesiveScalar = fAdhesiveScalar;
            if (vehicleFlags.bIsHandbrakeOn) {
#ifdef FIX_BUGS
                fRearBrakeFriction = 20000.0f * CTimer::GetTimeStepFix();
#else
                fRearBrakeFriction = 20000.0f;
#endif
                m_fTyreTemp = 1.0f;
            } else if (m_nBrakesOn) {
                ApplyTurnForce(GetRight() * (m_fSteerAngle * m_fTurnMass * BIKE_BURNOUT_STEER_MULT * CTimer::GetTimeStep()), vecRearResolvedOffset);
                fRearBrakeFriction = 0.0f;
                fRearAdhesiveScalar = 0.0f;
            } else {
                fRearBrakeFriction = fBrakeForce;
                if (m_fTyreTemp < 1.0f && m_GasPedal > 0.75f) {
                    fRearAdhesiveScalar = fAdhesiveScalar * m_fTyreTemp;
                    ApplyTurnForce(GetRight() * ((m_fSteerAngle - m_fTyreTemp * m_fSteerAngle) * m_fTurnMass * BIKE_BURNOUT_STEER_MULT * CTimer::GetTimeStep()), vecRearResolvedOffset);
                }
            }

            float fThrust = fDriveForce;
            m_aWheelColPoints[nRearWheelLine].m_nSurfaceTypeA = SURFACE_WHEELBASE;
            if (fThrust > 0.0f && fBrakeForce > 0.0f)
                fBrakeForce = 0.0f;

            float fSideAdhesiveMult = 1.0f;
            if (m_fBrakingSlide > 0.0f) {
                if ((g_surfaceInfos.GetAdhesionGroup(m_aWheelColPoints[nRearWheelLine].m_nSurfaceTypeB) - 1) >= 3)
                    fSideAdhesiveMult = 1.0f;
                else
                    fSideAdhesiveMult = 0.7f;
            }

            float fAdhesiveLimit = fRearAdhesiveScalar * g_surfaceInfos.GetAdhesiveLimit(&m_aWheelColPoints[nRearWheelLine]);
            if (GetStatus() == STATUS_PLAYER)
                fAdhesiveLimit *= g_surfaceInfos.GetWetMultiplier(m_aWheelColPoints[nRearWheelLine].m_nSurfaceTypeB);

            if (m_nWheelStatus[1] == eCarWheelStatus::WHEEL_STATUS_BURST)
                fAdhesiveLimit *= 0.4f;

            wheelStates[1] = m_WheelStates[1];
            CVector vecWheelSpeed = GetSpeed(vecRearResolvedOffset);
            if (m_aGroundPhysicalPtrs[nRearWheelLine])
                vecWheelSpeed -= m_aGroundPhysicalPtrs[nRearWheelLine]->GetSpeed(m_aGroundOffsets[nRearWheelLine]);

            ProcessBikeWheel(
                vecForward, vecRight, vecWheelSpeed, vecRearResolvedOffset,
                2, fThrust, fRearBrakeBias * fRearBrakeFriction, fRearTractionBias * fAdhesiveLimit, fSideAdhesiveMult,
                1, &m_aWheelAngularVelocity[1], &wheelStates[1], BIKE_WHEEL_R_STD, m_nWheelStatus[1]
            );

            if ((nBikePCFlags & 4) != 0 && (wheelStates[1] == WHEEL_STATE_SPINNING || wheelStates[1] == WHEEL_STATE_SKIDDING))
                wheelStates[1] = WHEEL_STATE_NORMAL;

            if (m_nBrakesOn && m_WheelStates[1] == WHEEL_STATE_SPINNING) {
                m_fTyreTemp -= CTimer::GetTimeStep() * 0.002f;
                if (m_fTyreTemp < 0.0f)
                    m_fTyreTemp = 0.0f;
            } else if (m_fTyreTemp < 1.0f) {
                m_fTyreTemp += CTimer::GetTimeStep() * 0.005f;
            }
        }

        if (m_pHandlingData->m_bProcRearwheelFirst) {
            if (m_WheelCounts[0] <= 0.0f && m_WheelCounts[1] <= 0.0f) {
                m_aWheelAngularVelocity[0] *= 0.95f;
                m_aWheelPitchAngles[0] += m_aWheelAngularVelocity[0] * CTimer::GetTimeStep();
            } else {
                float fSin = std::sin(m_RideAnimData.BarSteerAngle);
                float fCos = std::cos(m_RideAnimData.BarSteerAngle);
                CVector vecForward(-fSin, fCos, 0.0f);
                vecForward = GetMatrix().TransformVector(vecForward);

                float fAlongNormal = DotProduct(vecForward, m_aWheelColPoints[nFrontWheelLine].m_vecNormal);
                vecForward -= m_aWheelColPoints[nFrontWheelLine].m_vecNormal * fAlongNormal;
                vecForward.Normalise();

                CVector vecRight = CrossProduct(vecForward, m_aWheelColPoints[nFrontWheelLine].m_vecNormal);
                vecRight.Normalise();

                float fThrust = 0.0f;
                m_aWheelColPoints[nFrontWheelLine].m_nSurfaceTypeA = SURFACE_WHEELBASE;
                float fSideAdhesiveMult = 1.0f;
                if (m_fBrakingSlide > 0.0f) {
                    if ((g_surfaceInfos.GetAdhesionGroup(m_aWheelColPoints[nFrontWheelLine].m_nSurfaceTypeB) - 1) >= 3)
                        fSideAdhesiveMult = 1.0f;
                    else
                        fSideAdhesiveMult = 0.7f;
                }

                float fAdhesiveLimit = fAdhesiveScalar * g_surfaceInfos.GetAdhesiveLimit(&m_aWheelColPoints[nFrontWheelLine]);
                if (GetStatus() == STATUS_PLAYER)
                    fAdhesiveLimit *= g_surfaceInfos.GetWetMultiplier(m_aWheelColPoints[nFrontWheelLine].m_nSurfaceTypeB);

                if (m_nWheelStatus[0] == eCarWheelStatus::WHEEL_STATUS_BURST)
                    fAdhesiveLimit *= 0.4f;

                wheelStates[0] = m_WheelStates[0];
                CVector vecWheelSpeed = GetSpeed(vecFrontResolvedOffset);
                if (m_aGroundPhysicalPtrs[nFrontWheelLine])
                    vecWheelSpeed -= m_aGroundPhysicalPtrs[nFrontWheelLine]->GetSpeed(m_aGroundOffsets[nFrontWheelLine]);

                ProcessBikeWheel(
                    vecForward, vecRight, vecWheelSpeed, vecFrontResolvedOffset,
                    2, fThrust, fFrontBrakeBias * fBrakeForce, fFrontTractionBias * fAdhesiveLimit, fSideAdhesiveMult,
                    0, m_aWheelAngularVelocity.data(), wheelStates, BIKE_WHEEL_F_STD, m_nWheelStatus[0]
                );

                if ((nBikePCFlags & 4) != 0 && (wheelStates[0] == WHEEL_STATE_SPINNING || wheelStates[0] == WHEEL_STATE_SKIDDING))
                    wheelStates[0] = WHEEL_STATE_NORMAL;
            }
        }

        m_aGroundPhysicalPtrs[0] = nullptr;
        m_aGroundPhysicalPtrs[1] = nullptr;
        m_aGroundPhysicalPtrs[2] = nullptr;
        m_aGroundPhysicalPtrs[3] = nullptr;

        float fRiderStopLean = 0.0f;
        if (m_pDriver) {
            CAnimBlendAssociation* pAnimStop = RpAnimBlendClumpGetAssociation(m_pDriver->GetRpClump(), ANIM_ID_BIKE_STILL);
            if (pAnimStop)
                fRiderStopLean = pAnimStop->GetBlendAmount() * 0.17453f;
        }

        if (bStabiliseToVertical) {
            m_vecAveGroundNormal = CVector(0.0f, 0.0f, 1.0f);
            CVector vecTempRight = CrossProduct(GetForward(), m_vecAveGroundNormal);
            vecTempRight.Normalise();
            m_vecAveGroundNormal = CrossProduct(vecTempRight, GetForward());
            m_vecAveGroundNormal.Normalise();
        }

        if ((nBikePCFlags & 2) == 0 && !bikeFlags.bOnSideStand) {
            if (bikeFlags.bGettingPickedUp) {
                float fLeanReturn = std::pow(0.97f, CTimer::GetTimeStep());
                float fRightUp = std::asin(std::clamp(GetRight().z, -1.0f, 1.0f));
                m_RideAnimData.DesiredLeanAngle = -(((fRiderStopLean + (fRightUp + 0.2618f)) - (fLeanReturn * (fRiderStopLean + (fRightUp + 0.2618f)))) - (fLeanReturn * m_RideAnimData.DesiredLeanAngle));
            } else {
                m_RideAnimData.DesiredLeanAngle = std::pow(0.95f, CTimer::GetTimeStep()) * m_RideAnimData.DesiredLeanAngle;
            }
        } else {
            m_vecGroundRight = CrossProduct(GetForward(), m_vecAveGroundNormal);
            m_vecGroundRight.Normalise();

            float fSideForce = 0.0f;
            if (!m_pAttachedTo) {
                if (m_nNoOfContactWheels == 0) {
                    float fSteerLock = (m_pHandlingData->m_fSteeringLock * 3.14159265f / 180.0f);
                    float fSteerFrac = (fSteerLock > 0.0001f) ? (m_fSteerAngle / fSteerLock) : 0.0f;
                    fSideForce = CTimer::GetTimeStep() * ((fSteerFrac * -0.5f) * 0.008f);
                } else {
                    CVector vecSpeedDiff;
                    if (physicalFlags.bInfiniteMass) {
                        vecSpeedDiff = vecMoveSpeedBefore - m_vecOldSpeedForPlayback;
                        m_vecOldSpeedForPlayback = vecMoveSpeedBefore;
                    } else {
                        vecSpeedDiff = m_vecMoveSpeed - vecMoveSpeedBefore;
                    }
                    fSideForce = DotProduct(vecSpeedDiff, m_vecGroundRight);
                }
            }

            float fMaxBankAngle = m_BikeHandling->m_fMaxLean;
            float fTimeStepFactor = std::max(CTimer::GetTimeStep(), 0.01f) * 0.008f;
            float fBankTarget = (fTimeStepFactor > 0.00001f) ? (fSideForce / fTimeStepFactor) : 0.0f;
            if (m_nWheelStatus[0] == eCarWheelStatus::WHEEL_STATUS_BURST) {
                if (fBankTarget > fMaxBankAngle * 0.4f)
                    fBankTarget = fMaxBankAngle * 0.4f;
                else if (fBankTarget < -fMaxBankAngle * 0.4f)
                    fBankTarget = -fMaxBankAngle * 0.4f;
            } else {
                if (fBankTarget > fMaxBankAngle)
                    fBankTarget = fMaxBankAngle;
                else if (fBankTarget < -fMaxBankAngle)
                    fBankTarget = -fMaxBankAngle;
            }

            float fTimeStepCorrectedLeanReturn = std::pow(m_BikeHandling->m_fDesLean, CTimer::GetTimeStep());
            float fTargetLean = std::asin(std::clamp(fBankTarget, -1.0f, 1.0f));
            m_RideAnimData.DesiredLeanAngle = ((fTargetLean - fRiderStopLean) - (fTimeStepCorrectedLeanReturn * (fTargetLean - fRiderStopLean))) + (fTimeStepCorrectedLeanReturn * m_RideAnimData.DesiredLeanAngle);
        }
        m_RideAnimData.LeanAngle = m_RideAnimData.DesiredLeanAngle;

        m_WheelStates[0] = wheelStates[0];
        if (wheelStates[1] == WHEEL_STATE_SPINNING && m_GasPedal < 0.0f)
            m_WheelStates[1] = WHEEL_STATE_NORMAL;
        else
            m_WheelStates[1] = wheelStates[1];

        if (GetStatus() != STATUS_PLAYER) {
            ReduceHornCounter();
        } else {
            ProcessSirenAndHorn(1);
        }
    }

    if (m_fHealth >= 250.0f || GetStatus() == STATUS_WRECKED) {
        m_BlowUpTimer = 0.0f;
        if (m_pFireParticle) {
            m_pFireParticle->Kill();
            m_pFireParticle = nullptr;
        }
    } else {
        if (m_nVehicleSubType != VEHICLE_TYPE_BMX) {
            if (GetRwObject() && !m_pFireParticle) {
                RwMatrix* pParentMat = GetModellingMatrix();
                CVector engineOffset = GetDummyPosition(DUMMY_ENGINE);
                m_pFireParticle = g_fxMan.CreateFxSystem("fire_bike", engineOffset, pParentMat, false);
                if (m_pFireParticle) {
                    m_pFireParticle->Play();
                    GetEventGlobalGroup()->Add(CEventVehicleOnFire{ this });
                }
            }
        }
        m_BlowUpTimer += (CTimer::GetTimeStep() / 50.0f) * 1000.0f;
        if (m_BlowUpTimer > 5000.0f) {
            BlowUpCar(m_Damager, false);
        }
    }
    ProcessDelayedExplosion();

    {
        float fBumpShakePad = 0.0f;
        float fSurfaceShakePad = 0.0f;
        float fVelSqr = m_vecMoveSpeed.SquaredMagnitude();

        for (int32 i = 0; i < 4; i++) {
            float fWheelRatioDiff = m_aRatioHistory[i] - m_aWheelRatios[i];
            if (fVelSqr > 0.04f && !(nBikePCFlags & 8)) {
                if (fWheelRatioDiff > 0.3f) {
                    if (GetStatus() == STATUS_PLAYER) {
                        if (fWheelRatioDiff > fBumpShakePad)
                            fBumpShakePad = fWheelRatioDiff;
                    } else if (GetStatus() == STATUS_PHYSICS && fWheelRatioDiff > fBumpShakePad) {
                        fBumpShakePad = fWheelRatioDiff;
                    }
                }
            }

            if (m_aWheelRatios[i] < 1.0f && GetStatus() == STATUS_PLAYER) {
                int32 Roughness = g_surfaceInfos.GetRoughness(m_aWheelColPoints[i].m_nSurfaceTypeB);
                fSurfaceShakePad = std::max(fSurfaceShakePad, Roughness * 0.1f);
            }

            m_aRatioHistory[i] = m_aWheelRatios[i];
            m_aWheelRatios[i] = 1.0f;
        }

        if ((CTimer::GetTimeInMS() & 0x7FF) > 800) {
            if (fSurfaceShakePad >= 0.29f)
                fBumpShakePad = 0.0f;
            fSurfaceShakePad = 0.0f;
        }

        if ((fBumpShakePad > 0.0f || fSurfaceShakePad > 0.0f) && GetStatus() == STATUS_PLAYER) {
            float fSpeedSq = m_vecMoveSpeed.SquaredMagnitude();
            if (fSpeedSq > 0.01f) {
                float fSpeedVal = std::sqrt(fSpeedSq);
                uint8 nFreq;
                uint16 nLength;
                if (fBumpShakePad <= 0.0f) {
                    float fMassVal = std::max(m_fMass, 1.0f);
                    nFreq = static_cast<uint8>(std::clamp(((((fSurfaceShakePad * 200.0f) * fSpeedVal) * 2000.0f) / fMassVal) + 40.0f, 1.0f, 150.0f));
                    nLength = static_cast<uint16>(CTimer::GetTimeStep() * 5000.0f / (float)nFreq);
                } else {
                    float fMassVal = std::max(m_fMass, 1.0f);
                    nFreq = static_cast<uint8>(std::clamp(((((fBumpShakePad * 200.0f) * fSpeedVal) * 2000.0f) / fMassVal) + 100.0f, 1.0f, 250.0f));
                    nLength = static_cast<uint16>(CTimer::GetTimeStep() * 20000.0f / (float)nFreq);
                }
                CPad::GetPad(0)->StartShake(nLength, nFreq, 0);
            }
        }

        vehicleFlags.bVehicleColProcessed = false;
        vehicleFlags.bAudioChangingGear = false;

        if (!vehicleFlags.bWarnedPeds)
            CCarCtrl::ScanForPedDanger(this);

        if (!physicalFlags.bInfiniteMass) {
            if (!bSkipPhysics
                && (m_GasPedal == 0.0f || GetStatus() == STATUS_WRECKED)
                && std::abs(m_vecMoveSpeed.x) < 0.005f
                && std::abs(m_vecMoveSpeed.y) < 0.005f
                && std::abs(m_vecMoveSpeed.z) < 0.005f) {
                if (m_fDamageIntensity <= 0.0f || m_pDamageEntity != FindPlayerPed(-1)) {
                    m_vecMoveSpeed = CVector(0.0f, 0.0f, 0.0f);
                    m_vecTurnSpeed.z = 0.0f;
                }
            }
        } else {
            m_vecTurnSpeed.y = 0.0f;
            m_vecFrictionMoveSpeed.z = 0.0f;
            m_vecMoveSpeed = CVector(0.0f, 0.0f, 0.0f);
        }
    }

    if ((nBikePCFlags & 2) != 0 || (bikeFlags.bOnSideStand || bikeFlags.bGettingPickedUp)) {
        float fDot = DotProduct(GetRight(), m_vecAveGroundNormal);
        fDot = std::clamp(fDot, -1.0f, 1.0f);
        CVector vecCOM = GetMatrix().TransformVector(m_vecCentreOfMass);

        if ((nBikePCFlags & 2) != 0) {
            float fForce = (fDot * fRiderStabiliseForce) * m_fTurnMass;
            ApplyTurnForce(GetUp() * (fForce * CTimer::GetTimeStep()), vecCOM + GetRight());
            bikeFlags.bGettingPickedUp = false;
        } else {
            float fForce = (fDot * fPickUpStabiliseForce) * m_fTurnMass;
            ApplyTurnForce(GetUp() * (fForce * CTimer::GetTimeStep()), vecCOM + GetRight());
        }

        if (GetStatus() == STATUS_PLAYER) {
            if (m_WheelCounts[0] > 0.0f
                || m_WheelCounts[1] > 0.0f
                || GetForward().z <= 0.0f
                || (m_WheelCounts[2] <= 0.0f && m_WheelCounts[3] <= 0.0f)) {
                if (m_WheelCounts[2] <= 0.0f && m_WheelCounts[3] <= 0.0f) {
                    float zy = GetForward().z;
                    if (zy < 0.0f && (m_WheelCounts[0] > 0.0f || m_WheelCounts[1] > 0.0f)) {
                        float fDiff = m_BikeHandling->m_fStoppieAng - zy;
                        if (fDiff <= 0.15f) {
                            if (fDiff < -0.15f)
                                fDiff = std::min(-0.3f - fDiff, 0.0f);
                        } else {
                            fDiff = std::max(0.3f - fDiff, 0.0f);
                        }

                        float fBalanceMod = fDiff * (m_BikeHandling->m_fStoppieStabMult * std::min(m_vecMoveSpeed.Magnitude(), 0.1f));
                        float fFatMuscleMod = CStats::GetFatAndMuscleModifier(STAT_MOD_12);
                        float fTurnForce = m_fTurnMass * ((CTimer::GetTimeStep() * 0.5f) * (fFatMuscleMod * fBalanceMod));

                        ApplyTurnForce(GetUp() * fTurnForce, vecCOM + GetForward());

                        float fForwardSpeed = DotProduct(m_vecMoveSpeed, GetForward());
                        float fRudderForce = (fForwardSpeed + m_vecMoveSpeed.z * GetForward().z) * (m_fTurnMass * -0.05f * CTimer::GetTimeStep());

                        CVector vecFlatRight = CrossProduct(CVector(0.0f, 0.0f, 1.0f), GetForward());
                        vecFlatRight.Normalise();

                        ApplyTurnForce(GetRight() * fRudderForce, vecCOM - vecFlatRight);
                    }
                }
            } else {
                float zy = GetForward().z;
                float fDiff = m_BikeHandling->m_fWheelieAng - zy;
                if (fDiff <= 0.15f) {
                    if (fDiff < -0.08f)
                        fDiff = std::min(-0.14f - fDiff, 0.0f);
                } else {
                    fDiff = std::max(0.3f - fDiff, 0.0f);
                }

                float fBalanceMod = fDiff * (m_BikeHandling->m_fWheelieStabMult * std::min(m_vecMoveSpeed.Magnitude(), 0.1f));
                float fFatMuscleMod = CStats::GetFatAndMuscleModifier(STAT_MOD_12);
                float fTurnForce = m_fTurnMass * ((CTimer::GetTimeStep() * 0.5f) * (fFatMuscleMod * fBalanceMod));

                ApplyTurnForce(GetUp() * fTurnForce, vecCOM + GetForward());

                float fSteerForce = (((CTimer::GetTimeStep() * 0.5f) * m_RideAnimData.BarSteerAngle) * m_BikeHandling->m_fWheelieSteer) * m_fTurnMass;
                ApplyTurnForce(GetRight() * fSteerForce, vecCOM + GetForward());

                float fSpeedMag = m_vecMoveSpeed.Magnitude();
                float fMoveForce = m_fMass * (m_BikeHandling->m_fWheelieSteer * (m_RideAnimData.BarSteerAngle * (CTimer::GetTimeStep() * (fSpeedMag * 0.01f))));
                ApplyMoveForce(GetRight() * (fSpeedMag * fMoveForce));

                m_RideAnimData.LeanAngle += (CTimer::GetTimeStep() * -0.1f) * m_RideAnimData.BarSteerAngle;
            }
        }
    }
}

// 0x6B6950
void CBike::DoBurstAndSoftGroundRatios() {
    int32 nWheelLine1         = 0;
    int32 nWheelLine2         = 1;

    bool aWheelOk[4]          = { true, true, true, true };

    auto        pModelInfo    = CModelInfo::GetModelInfo(m_nModelIndex)->AsVehicleModelInfoPtr();
    const float fForwardSpeed = std::abs(DotProduct(m_vecMoveSpeed, GetForward()));

    for (int32 i = 0; i < 2; i++) {
        if (i == 1) {
            nWheelLine1 = 2;
            nWheelLine2 = 3;
        }

        if (m_nWheelStatus[i] == eCarWheelStatus::WHEEL_STATUS_MISSING) {
            aWheelOk[nWheelLine1]       = false;
            aWheelOk[nWheelLine2]       = false;
            m_aWheelRatios[nWheelLine1] = 1.0f;
            m_aWheelRatios[nWheelLine2] = 1.0f;
        } else if (m_nWheelStatus[i] == eCarWheelStatus::WHEEL_STATUS_BURST) {
            if (CGeneral::GetRandomNumberInRange(0, 98 + static_cast<int32>(fForwardSpeed * 40.0f)) < 100) {
                float fWheelRatioAdd        = BIKE_RATIO_BURST_MULT * (m_fLineLength[nWheelLine1] - m_fSuspensionLength[nWheelLine1]) / m_fLineLength[nWheelLine1];
                m_aWheelRatios[nWheelLine1] = std::min(m_aWheelRatios[nWheelLine1] + fWheelRatioAdd, 1.0f);
                m_aWheelRatios[nWheelLine2] = std::min(m_aWheelRatios[nWheelLine2] + fWheelRatioAdd, 1.0f);
            }
            aWheelOk[nWheelLine1] = false;
            aWheelOk[nWheelLine2] = false;
        } else if ((m_aWheelRatios[nWheelLine1] < 1.0f && m_aWheelColPoints[nWheelLine1].m_nSurfaceTypeB == SURFACE_RAILTRACK)
                   || (m_aWheelRatios[nWheelLine2] < 1.0f && m_aWheelColPoints[nWheelLine2].m_nSurfaceTypeB == SURFACE_RAILTRACK)) {
            float fBumpAngle = BIKE_RATIO_RAILTRACK_BUMP_DIST / (pModelInfo->GetWheelSize(i == 0) * 0.5f);
            if (fForwardSpeed > BIKE_RATIO_RAILTRACK_BUMP_DIST_SPEED) {
                fBumpAngle *= (fForwardSpeed / BIKE_RATIO_RAILTRACK_BUMP_DIST_SPEED);
            }

            float fWheelAngleFrac = m_aWheelPitchAngles[i] / fBumpAngle;
            fWheelAngleFrac -= std::floor(fWheelAngleFrac);

            float fNewWheelAngleFrac = (m_aWheelPitchAngles[i] + m_aWheelAngularVelocity[i] * CTimer::GetTimeStep()) / fBumpAngle;
            fNewWheelAngleFrac -= std::floor(fNewWheelAngleFrac);

            if ((m_aWheelAngularVelocity[i] > 0.0f && fNewWheelAngleFrac < fWheelAngleFrac)
                || (m_aWheelAngularVelocity[i] < 0.0f && fNewWheelAngleFrac > fWheelAngleFrac)) {
                aWheelOk[nWheelLine1]       = false;
                aWheelOk[nWheelLine2]       = false;
                float fWheelRatioAdd        = BIKE_RATIO_RAILTRACK_BUMP_MULT * (m_fLineLength[nWheelLine1] - m_fSuspensionLength[nWheelLine1]) / m_fLineLength[nWheelLine1];
                m_aWheelRatios[nWheelLine1] = std::max(m_aWheelRatios[nWheelLine1] - fWheelRatioAdd, 0.2f);
                m_aWheelRatios[nWheelLine2] = std::max(m_aWheelRatios[nWheelLine2] - fWheelRatioAdd, 0.2f);
            }
        }
    }

    for (int32 i = 0; i < 4; i++) {
        if (aWheelOk[i]) {
            if (m_aWheelRatios[i] < 1.0f) {
                if (g_surfaceInfos.GetAdhesionGroup(m_aWheelColPoints[i].m_nSurfaceTypeB) == ADHESION_GROUP_SAND
                    && m_nModelIndex != MODEL_RHINO) {
                    float fSinkRatio = BIKE_RATIO_SINK_IN_SAND;
                    if (m_pHandlingData->m_bOffroadAbility2) {
                        fSinkRatio = BIKE_RATIO_SINK_IN_SAND_OA2;
                    } else if (m_pHandlingData->m_bOffroadAbility) {
                        fSinkRatio = BIKE_RATIO_SINK_IN_SAND_OA1;
                    }

                    m_aWheelRatios[i] += fSinkRatio * std::max(0.4f, 1.0f - 0.7f * fForwardSpeed / BIKE_RATIO_SAND_BOGDOWN_LIMIT - 0.7f * CWeather::WetRoads) * (m_fLineLength[i] - m_fSuspensionLength[i]) / m_fLineLength[i];
                    if (m_aWheelRatios[i] > 1.0f) {
                        m_aWheelRatios[i] = 1.0f;
                    }
                }
            }
        }
    }
}

// 0x6B67E0
bool CBike::SetUpWheelColModel(CColModel* wheelCol) {
    auto pModelInfo = CModelInfo::GetModelInfo(m_nModelIndex)->AsVehicleModelInfoPtr();
    RwFrame* pRootFrame = m_aBikeNodes[BIKE_CHASSIS];
    CCollisionData* pColData = wheelCol->GetData();
    CColModel* colModel = GetColModel();

    wheelCol->GetBoundingSphere() = colModel->GetBoundingSphere();
    wheelCol->GetBoundingBox() = colModel->GetBoundingBox();

    RwMatrix matWheel;
    matWheel = m_aBikeNodes[BIKE_WHEEL_FRONT]->modelling;
    RwFrame* pParent = RwFrameGetParent(m_aBikeNodes[BIKE_WHEEL_FRONT]);
    if (pParent) {
        do {
            RwMatrixTransform(&matWheel, &pParent->modelling, rwCOMBINEPOSTCONCAT);
            pParent = RwFrameGetParent(pParent);
        } while (pParent != pRootFrame && pParent);
    }

    pColData->m_pSpheres[0].Set(pModelInfo->m_fWheelSizeFront * 0.5f, CVector(matWheel.pos.x, matWheel.pos.y, matWheel.pos.z), SURFACE_WHEELBASE, 13);

    matWheel = m_aBikeNodes[BIKE_WHEEL_REAR]->modelling;
    pParent = RwFrameGetParent(m_aBikeNodes[BIKE_WHEEL_REAR]);
    if (pParent) {
        do {
            RwMatrixTransform(&matWheel, &pParent->modelling, rwCOMBINEPOSTCONCAT);
            pParent = RwFrameGetParent(pParent);
        } while (pParent != pRootFrame && pParent);
    }

    pColData->m_pSpheres[1].Set(pModelInfo->m_fWheelSizeRear * 0.5f, CVector(matWheel.pos.x, matWheel.pos.y, matWheel.pos.z), SURFACE_WHEELBASE, 15);

    pColData->m_nNumSpheres = 2;
    return true;
}

// 0x6B67B0
void CBike::RemoveRefsToVehicle(CEntity* entityToRemove) {
    for (auto& entity : m_aGroundPhysicalPtrs) {
        if (entity == entityToRemove) {
            entity = nullptr;
        }
    }
}

// 0x6B6620
void CBike::ProcessControlCollisionCheck(bool applySpeed) {
    const CMatrix oldMat = GetMatrix();
    SetIsStuck(false);
    SkipPhysics();
    physicalFlags.bSkipLineCol     = false;
    physicalFlags.bProcessingShift = false;
    m_fMovingSpeed                 = 0.0f;
    rng::fill(m_aWheelRatios, 1.0f);

    if (applySpeed) {
        ApplyMoveSpeed();
        ApplyTurnSpeed();

        for (auto i = 0; CheckCollision() && i < 5; i++) {
            GetMatrix() = oldMat;
            ApplyMoveSpeed();
            ApplyTurnSpeed();
        }
    } else {
        const auto usesCollision = GetUsesCollision();
        SetUsesCollision(false);
        CheckCollision();
        SetUsesCollision(usesCollision);
    }

    SetIsStuck(false);
    SetIsInSafePosition(true);
}

// 0x6B5990
void CBike::GetComponentWorldPosition(int32 componentId, CVector& outPos) {
    if (IsComponentPresent(componentId)) {
        outPos = RwFrameGetLTM(m_aBikeNodes[componentId])->pos;
    } else {
        NOTSA_LOG_DEBUG("BikeNode missing: model={}, nodeIdx={}", m_nModelIndex, componentId);
    }
}

// 0x6B58D0
void CBike::ProcessOpenDoor(CPed* ped, uint32 doorComponentId, uint32 animGroup, uint32 animId, float fTime) {
    // NOP
}
