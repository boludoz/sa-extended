#include "StdInc.h"

#include "Bmx.h"

#include "Shadows.h"
#include "Stats.h"
#include "Cheat.h"
#include "CarAI.h"
#include "CarCtrl.h"

void CBmx::InjectHooks() {
    RH_ScopedVirtualClass(CBmx, 0x871528, 67);
    RH_ScopedCategory("Vehicle");

    RH_ScopedInstall(Constructor, 0x6BF820);
    RH_ScopedVMTInstall(SetUpWheelColModel, 0x6BF9B0);
    RH_ScopedVMTInstall(BurstTyre, 0x6BF9C0);
    RH_ScopedVMTInstall(FindWheelWidth, 0x6C0550);
    RH_ScopedVMTInstall(ProcessControl, 0x6BFA30);
    RH_ScopedVMTInstall(ProcessDrivingAnims, 0x6BFB50);
    RH_ScopedVMTInstall(PreRender, 0x6C0810);
    RH_ScopedVMTInstall(ProcessAI, 0x6C1470);
    RH_ScopedInstall(ProcessBunnyHop, 0x6C0590);
    RH_ScopedInstall(LaunchBunnyHopCB, 0x6C0390);
}

float BMX_LEAN_TO_PITCH_SCALE = -0.05f;

// 0x6BF820
// ASM Match
CBmx::CBmx(int32 nModelIndex, eVehicleCreatedBy CreatedBy) : CBike(nModelIndex, CreatedBy)
{
    CVehicleModelInfo* pModelInfo = static_cast<CVehicleModelInfo*>(CModelInfo::GetModelInfo(nModelIndex));

    m_vehicleType = VEHICLE_TYPE_BMX;

    AssocGroupId firstAssocGroup = CAnimManager::GetAnimBlocks()[pModelInfo->GetAnimFileIndex()].GroupId;
    RideAnimData.AnimGroup = firstAssocGroup;

    if (firstAssocGroup < ANIM_GROUP_BMX || firstAssocGroup > ANIM_GROUP_CHOPPA)
    {
        RideAnimData.AnimGroup = ANIM_GROUP_BMX;
    }

    RwFrame* pFrame = m_aBikeNodes[BMX_WHEEL_F];  // Front wheel

    m_fControlJump = 0.0f;                        // time step?
    m_fControlPedaling = 0.0f;                    // base anim lean
    m_fSprintLeanAngle = 0.0f;                    // rot pedal chainset
    m_fCrankAngle = 0.0f;                         // rot pedal R
    m_fPedalAngleL = 0.0f;                        // rot pedal L
    m_fPedalAngleR = 0.0f;                        // distance between wheels

    m_nFixLeftHand = false;                       // ped left hand fixed
    m_nFixRightHand = false;                      // ped right hand fixed

    m_bIsFreewheeling = false;

    RwMatrix tempNodeMatrx = *RwFrameGetMatrix(pFrame);

    for (RwFrame* pParentFrame = RwFrameGetParent(pFrame); pParentFrame;)
    {
        RwMatrixTransform(&tempNodeMatrx, RwFrameGetMatrix(pParentFrame), rwCOMBINEPOSTCONCAT);
        pParentFrame = RwFrameGetParent(pParentFrame);
        if (pParentFrame == pFrame)
        {
            break;
        }
    }

    RwV3d posnF = tempNodeMatrx.pos;

    pFrame = m_aBikeNodes[BMX_WHEEL_R];  // Rear wheel

    tempNodeMatrx = *RwFrameGetMatrix(pFrame);

    for (RwFrame* pParentFrame = RwFrameGetParent(pFrame); pParentFrame;)
    {
        RwMatrixTransform(&tempNodeMatrx, RwFrameGetMatrix(pParentFrame), rwCOMBINEPOSTCONCAT);
        pParentFrame = RwFrameGetParent(pParentFrame);
        if (pParentFrame == pFrame)
        {
            break;
        }
    }

    // Calculate mid-wheel stuff
    m_fMidWheelDistY = posnF.y - tempNodeMatrx.pos.y;
    m_fMidWheelFracY = posnF.y / m_fMidWheelDistY;
}

// 0x6BF9B0
// ASM Match
bool CBmx::SetUpWheelColModel(CColModel* wheelCol)
{
    return false;
}

// 0x6BF9C0
// ASM Match
bool CBmx::BurstTyre(uint8 tyreComponentId, bool bPhysicalEffect)
{
    return false;
}

// 0x6BF9D0
// ASM Match
CBmx::~CBmx()
{
    m_vehicleAudio.Terminate();
}

float BMX_SPRINT_LEANSTART = HALF_PI;
float BMX_SPRINT_LEANMULT = 0.3f;
float BMX_PEDAL_LEANSTART = 0.0f;
float BMX_PEDAL_LEANMULT = 0.07f;

float MTB_SPRINT_LEANMULT = 0.087f;
float MTB_PEDAL_LEANMULT = 0.02f;

// 0x6BFA30
// ASM Match
void CBmx::ProcessControl()
{
    CBike::ProcessControl();

    if (GetWasPostponed() || GetStatus() != STATUS_PLAYER)
    {
        return;
    }

    if (m_pDriver != nullptr)
    {
        CAnimBlendAssociation* pAnimSprint = RpAnimBlendClumpGetAssociation(m_pDriver->GetRpClump(), ANIM_ID_BIKE_SPRINT);
        if (pAnimSprint != nullptr && pAnimSprint->GetBlendAmount() > 0.01f)
        {
            float fSprintMult = BMX_SPRINT_LEANMULT;
            if (GetModelIndex() == MODEL_MTBIKE)
            {
                fSprintMult = MTB_SPRINT_LEANMULT;
            }
            m_fSprintLeanAngle = CMaths::Sin(pAnimSprint->GetCurrentTime() / pAnimSprint->GetHier()->GetTotalTime() * TWO_PI + BMX_SPRINT_LEANSTART) * pAnimSprint->GetBlendAmount() * fSprintMult;
        }
        else
        {
            float fPedalLeanMult = BMX_PEDAL_LEANMULT;
            if (GetModelIndex() == MODEL_MTBIKE)
            {
                fPedalLeanMult = MTB_PEDAL_LEANMULT;
            }

            pAnimSprint = RpAnimBlendClumpGetAssociation(m_pDriver->GetRpClump(), ANIM_ID_BIKE_PEDAL);
            if (pAnimSprint != nullptr)
            {
                if (pAnimSprint->GetBlendAmount() > 0.01f)
                {
                    RideAnimData.LeanAngle += CMaths::Sin(pAnimSprint->GetCurrentTime() / pAnimSprint->GetHier()->GetTotalTime() * HALF_PI + BMX_PEDAL_LEANSTART) * pAnimSprint->GetBlendAmount() * fPedalLeanMult;
                }
            }
            m_fSprintLeanAngle *= 0.95f;
        }
    }
}

float BMX_INAIR_ROTF = 0.002f;
float BMX_INAIR_ROTLIM = 0.04f;
float BMX_SPRINT_BOOST = 0.25f;
float BMX_SPRINT_TIRED_RELEASESUB = 0.4f;
float BMX_SPRINT_START_THRESHHOLD = 1.2f;
float BMX_SPRINT_AI_SPRINTINGSUB = 0.02f;
float BMX_SPRINT_AI_RECOVERINGSUB = 0.01f;

// 0x6C1470
// ASM Match: 98.18%
bool CBmx::ProcessAI(uint32& extraHandlingFlags)
{
    CVehicleModelInfo* pModelInfo = static_cast<CVehicleModelInfo*>(CModelInfo::GetModelInfo(m_nModelIndex));

    m_autoPilot.SlowingDownForCar = false;
    m_autoPilot.SlowingDownForPed = false;
    m_bIsFreewheeling = false;

    switch (GetStatus())
    {
        case STATUS_PLAYER:
            extraHandlingFlags += 2;
            m_nBikeFlags.bGettingPickedUp = false;

            if (m_pDriver && m_pDriver->IsPlayer())
            {
                ProcessControlInputs(m_pDriver->m_nPedType - PED_TYPE_PLAYER1);

                CPad* pPad = m_pDriver->AsPlayer()->GetPadFromPlayer();
                CPlayerPed* pPlayerPed = m_pDriver->AsPlayer();
                float fLeanForce;

                if (RideAnimData.LeanFwd < 0.0f)
                {
                    m_vecCentreOfMass.y = pBikeHandling->m_fLeanBakCOM * RideAnimData.LeanFwd + m_pHandlingData->m_vecCentreOfMass.y;

                    if ((GetBrakePedal() == 0.0f && !GetIsHandbrakeOn()) || !nNoOfContactWheels)
                    {
                        if (GetModelIndex() == MODEL_SANCHEZ)
                        {
                            fLeanForce = VMIN(0.1f, m_vecMoveSpeed.Magnitude()) * (GetGasPedal() * 0.7f + 0.3f);
                        }
                        else
                        {
                            fLeanForce = VMIN(0.1f, m_vecMoveSpeed.Magnitude()) * ((GetGasPedal() + 1.0f) * 0.5f);
                        }
                        fLeanForce *= pBikeHandling->m_fLeanBakForce * m_fTurnMass * RideAnimData.LeanFwd;

                        ApplyTurnForce(-(CTimer::GetTimeStep() * fLeanForce) * GetMatrix().GetUp(), m_vecCentreOfMass + GetMatrix().GetForward());
                    }
                }
                else
                {
                    m_vecCentreOfMass.y = pBikeHandling->m_fLeanFwdCOM * RideAnimData.LeanFwd + m_pHandlingData->m_vecCentreOfMass.y;

                    if (GetBrakePedal() < 0.0f || !nNoOfContactWheels)
                    {
                        fLeanForce = pBikeHandling->m_fLeanFwdForce * m_fTurnMass * VMIN(0.1f, m_vecMoveSpeed.Magnitude()) * RideAnimData.LeanFwd;

                        ApplyTurnForce(-(CTimer::GetTimeStep() * fLeanForce) * GetMatrix().GetUp(), m_vecCentreOfMass + GetMatrix().GetForward());
                    }
                }

                PruneReferences();
                if (GetStatus() == STATUS_PLAYER)
                {
                    DoDriveByShootings();
                }
                DoSoftGroundResistance(extraHandlingFlags);

                if (m_aWheelRatios[0] == 1.0f && m_aWheelRatios[1] == 1.0f && m_aWheelRatios[2] == 1.0f && m_aWheelRatios[3] == 1.0f)
                {
                    float fZRotSpeed = DotProduct(m_vecTurnSpeed, GetMatrix().GetUp());
                    if ((fZRotSpeed < BMX_INAIR_ROTLIM && pPad->GetSteeringLeftRight() < 0.0f) || (-BMX_INAIR_ROTLIM < fZRotSpeed && pPad->GetSteeringLeftRight() > 0.0f))
                    {
                        ApplyTurnForce((pPad->GetSteeringLeftRight() / 128.0f) * m_fTurnMass * CTimer::GetTimeStep() * BMX_INAIR_ROTF * GetMatrix().GetRight(), GetMatrix().GetForward());
                    }
                }

                ProcessBunnyHop();

                CAnimBlendAssociation* pAnimSprint = RpAnimBlendClumpGetAssociation(m_pDriver->GetRpClump(), ANIM_ID_BIKE_SPRINT);
                if (pPlayerPed->ControlButtonSprint(SPRINT_BMX) > BMX_SPRINT_START_THRESHHOLD || (pPlayerPed->GetButtonSprintResults(SPRINT_BMX) > 1.0f && pAnimSprint && pAnimSprint->GetBlendAmount() > 0.5f))
                {
                    m_nBikeFlags.bPlayerBoost = true;
                    m_fControlPedaling = pPlayerPed->GetPlayerData()->m_fMoveSpeed;
                }
                else
                {
                    pPlayerPed->HandleSprintEnergy(false, CMaths::Max(0.5f, 1.0f - pPad->GetAccelerate() / 255.0f * 0.5f));
                    if (pPlayerPed->GetButtonSprintResults(SPRINT_BMX) > 0.0f)
                    {
                        m_fControlPedaling = 4.9f;
                        if (GetGasPedal() == 0.0f && GetBrakePedal() == 0.0f)
                        {
                            SetGasPedal(1.0f);
                        }
                    }
                    else if (pPlayerPed->GetPlayerData()->m_fTimeCanRun < 0.0f)
                    {
                        if (pPad->GetAccelerateJustDown())
                        {
                            m_fControlPedaling = 4.9f;
                        }
                        else
                        {
                            m_fControlPedaling = CMaths::Max(0.0f, m_fControlPedaling - BMX_SPRINT_TIRED_RELEASESUB);
                        }
                    }
                    else
                    {
                        m_fControlPedaling = 0.0f;
                    }
                }

                CStats::UpdateStatsWhenCycling(m_nBikeFlags.bPlayerBoost, this);

                if (pPad->CarGunJustDown())
                {
                    ActivateBomb();
                }
            }
            break;
        case STATUS_PLAYER_PLAYBACK_FROM_BUFFER:
            extraHandlingFlags += 2;
            break;
        case STATUS_SIMPLE:
            CCarAI::UpdateCarAI(this);
            CPhysical::ProcessControl();
            CCarCtrl::UpdateCarOnRails(this);
            nNoOfContactWheels = 2;
            m_nDriveWheelsOnGroundLastFrame = m_nDriveWheelsOnGround;
            m_nDriveWheelsOnGround = 2;

            m_pHandlingData->m_transmissionData.CalculateGearForSimpleCar(m_autoPilot.ActualSpeed * 0.02f, m_nCurrentGear);

            m_aWheelPitchAngles[0] += CVehicle::ProcessWheelRotation(WHEEL_STATE_NORMAL, GetMatrix().GetForward(), m_vecMoveSpeed, pModelInfo->m_fWheelSizeFront * 0.5f);
            m_aWheelPitchAngles[1] += CVehicle::ProcessWheelRotation(WHEEL_STATE_NORMAL, GetMatrix().GetForward(), m_vecMoveSpeed, pModelInfo->m_fWheelSizeRear * 0.5f);

            PlayHornIfNecessary();
            ReduceHornCounter();

            vehicleFlags.bVehicleColProcessed = false;
            vehicleFlags.bAudioChangingGear = false;
            m_nBikeFlags.bWheelieForCamera = false;
            m_fControlPedaling = 0.0f;
            break;
        case STATUS_PHYSICS:
            CCarAI::UpdateCarAI(this);
            CCarCtrl::SteerAICarWithPhysics(this);
            PlayHornIfNecessary();

            extraHandlingFlags += 2;
            m_nBikeFlags.bWheelieForCamera = false;

            if (vehicleFlags.bIsBeingCarJacked)
            {
                SetGasPedal(0.0f);
                SetBrakePedal(1.0f);
                SetIsHandbrakeOn(true);
            }
            else
            {
                m_nBikeFlags.bGettingPickedUp = false;
            }

            if (m_fControlPedaling > 0.0f)
            {
                if (m_fControlPedaling > 5.0f)
                {
                    m_fControlPedaling -= CTimer::GetTimeStep() * BMX_SPRINT_AI_SPRINTINGSUB;
                }
                else if (vehicleFlags.bUseCarCheats)
                {
                    m_fControlPedaling -= CTimer::GetTimeStep() * BMX_SPRINT_AI_RECOVERINGSUB * 2.0f;
                }
                else
                {
                    m_fControlPedaling -= CTimer::GetTimeStep() * BMX_SPRINT_AI_RECOVERINGSUB;
                }

                if (m_fControlPedaling < 0.0f)
                {
                    m_fControlPedaling = 0.0f;
                }
            }
            break;
        case STATUS_ABANDONED:
            SetBrakePedal(0.0f);

            if (m_vecMoveSpeed.SquaredMagnitude() < 0.01f || m_nBikeFlags.bOnSideStand)
            {
                SetIsHandbrakeOn(true);
            }
            else
            {
                SetIsHandbrakeOn(false);
            }

            SetGasPedal(0.0f);
            m_cHorn = 0;

            if ((m_pDriver || m_apPassengers[0] || vehicleFlags.bIsBeingCarJacked) && !m_nBikeFlags.bOnSideStand)
            {
                extraHandlingFlags += 2;
            }

            RideAnimData.AnimLeanLeft = 0.0f;
            RideAnimData.AnimLeanFwd = 0.0f;
            m_nBikeFlags.bWheelieForCamera = false;
            m_fControlPedaling = 0.0f;

            if (vehicleFlags.bIsBeingCarJacked)
            {
                SetGasPedal(0.0f);
                SetBrakePedal(1.0f);
                SetIsHandbrakeOn(true);
            }
            break;
        case STATUS_FORCED_STOP:
            if (m_vecMoveSpeed.SquaredMagnitude() < 0.01f)
            {
                SetBrakePedal(1.0f);
                SetIsHandbrakeOn(true);
            }
            else
            {
                SetBrakePedal(0.0f);
                SetIsHandbrakeOn(false);
            }

            SetSteerAngle(0.0f);
            SetGasPedal(0.0f);
            m_cHorn = 0;
            extraHandlingFlags += 2;
            m_nBikeFlags.bWheelieForCamera = false;
            m_fControlPedaling = 0.0f;
            break;
        case STATUS_WRECKED:
            SetBrakePedal(0.05f);
            SetIsHandbrakeOn(true);
            SetSteerAngle(0.0f);
            SetGasPedal(0.0f);
            m_cHorn = 0;
            m_nBikeFlags.bWheelieForCamera = false;
            m_fControlPedaling = 0.0f;
            RideAnimData.AnimLeanLeft = 0.0f;
            RideAnimData.AnimLeanFwd = 0.0f;
            break;
    }

    if (m_pDriver)
    {
        CAnimBlendAssociation* pAnimLean;
        CAnimBlendAssociation* pAnimDriveBy;

        pAnimLean = RpAnimBlendClumpGetAssociation(m_pDriver->GetRpClump(), ANIM_ID_BIKE_FWD);
        if (pAnimLean == nullptr || pAnimLean->GetBlendAmount() < 0.5f)
        {
            pAnimLean = RpAnimBlendClumpGetAssociation(m_pDriver->GetRpClump(), ANIM_ID_BIKE_BACK);
        }

        pAnimDriveBy = RpAnimBlendClumpGetAssociation(m_pDriver->GetRpClump(), ANIM_ID_BIKE_DRIVEBYLHS);
        if (pAnimDriveBy == nullptr || pAnimDriveBy->GetBlendAmount() < 0.5f)
        {
            pAnimDriveBy = RpAnimBlendClumpGetAssociation(m_pDriver->GetRpClump(), ANIM_ID_BIKE_DRIVEBYRHS);
        }
        if (pAnimDriveBy == nullptr || pAnimDriveBy->GetBlendAmount() < 0.5f)
        {
            pAnimDriveBy = RpAnimBlendClumpGetAssociation(m_pDriver->GetRpClump(), ANIM_ID_BIKE_DRIVEBYFT);
        }

        if ((pAnimLean && pAnimLean->GetBlendAmount() > 0.5f) || (pAnimDriveBy && pAnimDriveBy->GetBlendAmount() > 0.5f))
        {
            SetGasPedal(0.0f);
            if (!GetIsHandbrakeOn() && (m_aWheelRatios[0] < 1.0f || m_aWheelRatios[1] < 1.0f || m_aWheelRatios[2] < 1.0f || m_aWheelRatios[3] < 1.0f))
            {
                m_bIsFreewheeling = true;
            }
            return true;
        }
    }

    if (m_fControlPedaling > 5.0f && (m_aWheelRatios[2] < 1.0f || m_aWheelRatios[3] < 1.0f))
    {
        float fBoostPercentage = 2.4f - DotProduct(m_vecMoveSpeed, GetMatrix().GetForward()) / m_pHandlingData->m_transmissionData.m_MaxFlatVelocity * 1.5f;
        if (fBoostPercentage > 2.0f)
        {
            fBoostPercentage = 2.0f;
        }
        else if (fBoostPercentage < 0.0f)
        {
            fBoostPercentage = 0.0f;
        }

        if (GetStatus() == STATUS_PLAYER)
        {
            fBoostPercentage *= CStats::GetFatAndMuscleModifier(STAT_MOD_5);
        }
        else if (vehicleFlags.bUseCarCheats)
        {
            fBoostPercentage *= 1.25f;
        }

        ApplyMoveForce(fBoostPercentage * (CTimer::GetTimeStep() * m_fMass * 0.3f) * 0.008f * GetMatrix().GetForward());
    }

    return true;
}

float BMX_PEDAL_BLENDOUTLIMIT = 0.4f;
float BMX_PEDAL_BLENDOUTLIMIT_SPRINT = 0.7f;
float BMX_PEDAL_BLENDOUTLEAN = 0.95f;
float BMX_PEDAL_SPEEDLOWLIM = 0.01f;

float BMX_PEDAL_ANIMSPEEDMULT = 3.0f;
float BMX_PEDAL_SPRINTSPEEDLIMIT = 2.5f;
float MTB_PEDAL_ANIMSPEEDMULT = 5.0f;
float MTB_PEDAL_SPRINTSPEEDLIMIT = 2.0f;

float BMX_CRANK_PEDALSTARTPOS = 0.0f;
float BMX_CRANK_TURNLEFTPOS = PI;
float BMX_CRANK_TURNRIGHTPOS = 0.0f;
float BMX_CRANK_BUNNYHOPPOS = HALF_PI;
float BMX_CRANK_RETURNRATE = 0.97f;

// 0x6BFB50
// ASM Match
void CBmx::ProcessDrivingAnims(CPed* pPed, bool playRadioAnim)
{
    if (!m_bOffscreen || (pPed && pPed->IsPlayer()))
    {
        m_nFixLeftHand = true;
        m_nFixRightHand = true;

        CAnimBlendAssociation* pAnimHop = RpAnimBlendClumpGetAssociation(pPed->GetRpClump(), ANIM_ID_BIKE_BUNNYHOP);
        if (pAnimHop)
        {
            m_fCrankAngle = (1.0f - pAnimHop->GetBlendAmount()) * m_fCrankAngle + BMX_CRANK_BUNNYHOPPOS * pAnimHop->GetBlendAmount();
            m_fPedalAngleL = -m_fCrankAngle;
            m_fPedalAngleR = -m_fCrankAngle;
            return;
        }
        else
        {
            if (pPed->GetPlayerData())
            {
                pPed->SetMoveState(PEDMOVE_NONE);
            }

            float fForwardSpeed = DotProduct(m_vecMoveSpeed, GetMatrix().GetForward());

            CAnimBlendAssociation* pAnimPedal = RpAnimBlendClumpGetAssociation(pPed->GetRpClump(), ANIM_ID_BIKE_PEDAL);
            CAnimBlendAssociation* pAnimSprint = RpAnimBlendClumpGetAssociation(pPed->GetRpClump(), ANIM_ID_BIKE_SPRINT);
            CAnimBlendAssociation* pAnimLeanLeft = RpAnimBlendClumpGetAssociation(pPed->GetRpClump(), ANIM_ID_BIKE_LEFT);
            CAnimBlendAssociation* pAnimLeanRight = RpAnimBlendClumpGetAssociation(pPed->GetRpClump(), ANIM_ID_BIKE_RIGHT);
            CAnimBlendAssociation* pAnimLeanFwd = RpAnimBlendClumpGetAssociation(pPed->GetRpClump(), ANIM_ID_BIKE_FWD);
            CAnimBlendAssociation* pAnimDriveBy = RpAnimBlendClumpGetAssociation(pPed->GetRpClump(), ANIM_ID_BIKE_DRIVEBYLHS);
            if (pAnimDriveBy == nullptr)
            {
                pAnimDriveBy = RpAnimBlendClumpGetAssociation(pPed->GetRpClump(), ANIM_ID_BIKE_DRIVEBYRHS);
                if (pAnimDriveBy == nullptr)
                {
                    pAnimDriveBy = RpAnimBlendClumpGetAssociation(pPed->GetRpClump(), ANIM_ID_BIKE_DRIVEBYFT);
                }
            }

            float fLeanBlendLimit = m_fControlPedaling > 5.0f ? BMX_PEDAL_BLENDOUTLIMIT_SPRINT : BMX_PEDAL_BLENDOUTLIMIT;

            if (CMaths::Abs(RideAnimData.LeanAngle) < fLeanBlendLimit && RideAnimData.LeanFwd < fLeanBlendLimit && fForwardSpeed > BMX_PEDAL_SPEEDLOWLIM && pAnimDriveBy == nullptr)
            {
                float fPedalSpeed;
                float fSprintSpeedMax;
                if (GetModelIndex() == MODEL_MTBIKE)
                {
                    if (m_nCurrentGear < 1)
                    {
                        fPedalSpeed = 0.0f;
                    }
                    else
                    {
                        fPedalSpeed = MTB_PEDAL_ANIMSPEEDMULT * fForwardSpeed / (m_nCurrentGear * m_pHandlingData->m_transmissionData.m_MaxFlatVelocity - 0.25f);
                    }
                    fSprintSpeedMax = MTB_PEDAL_SPRINTSPEEDLIMIT;
                }
                else
                {
                    fPedalSpeed = BMX_PEDAL_ANIMSPEEDMULT * fForwardSpeed;
                    fSprintSpeedMax = BMX_PEDAL_SPRINTSPEEDLIMIT;
                }

                bool bJustBlendedAnim = false;
                if (m_fControlPedaling > 5.0f && fPedalSpeed < fSprintSpeedMax)
                {
                    if (pAnimSprint == nullptr || (pAnimSprint->GetBlendAmount() < 1.0f && pAnimSprint->GetBlendDelta() <= 0.0f))
                    {
                        pAnimSprint = CAnimManager::BlendAnimation(pPed->GetRpClump(), RideAnimData.AnimGroup, ANIM_ID_BIKE_SPRINT, 4.0f);
                        bJustBlendedAnim = true;
                    }
                    pAnimSprint->SetFlag(ANIMATION_IS_PLAYING, true);
                    if (pAnimPedal)
                    {
                        pAnimPedal->SetFlag(ANIMATION_IS_PLAYING, true);
                        pAnimPedal->SetSpeed(fPedalSpeed);
                    }
                    else
                    {
                        pAnimSprint->SetSpeed(fPedalSpeed);
                        pAnimPedal = pAnimSprint;
                    }
                }
                else if (GetGasPedal() == 0.0f && !(m_fControlPedaling > 0.0f) && GetStatus() != STATUS_SIMPLE)
                {
                    if (pAnimPedal == nullptr || (pAnimPedal->GetBlendAmount() < 1.0f && pAnimPedal->GetBlendDelta() <= 0.0f))
                    {
                        pAnimPedal = CAnimManager::BlendAnimation(pPed->GetRpClump(), RideAnimData.AnimGroup, ANIM_ID_BIKE_PEDAL, 4.0f);
                        bJustBlendedAnim = true;
                    }
                    pAnimPedal->SetFlag(ANIMATION_IS_PLAYING, false);

                    if (!GetIsHandbrakeOn() && (m_aRatioHistory[0] < 1.0f || m_aRatioHistory[1] < 1.0f || m_aRatioHistory[2] < 1.0f || m_aRatioHistory[3] < 1.0f))
                    {
                        m_bIsFreewheeling = true;
                    }
                }
                else
                {
                    if (pAnimPedal == nullptr || (pAnimPedal->GetBlendAmount() < 1.0f && pAnimPedal->GetBlendDelta() <= 0.0f))
                    {
                        pAnimPedal = CAnimManager::BlendAnimation(pPed->GetRpClump(), RideAnimData.AnimGroup, ANIM_ID_BIKE_PEDAL, 4.0f);
                        bJustBlendedAnim = true;
                    }
                    pAnimPedal->SetFlag(ANIMATION_IS_PLAYING, true);
                    pAnimPedal->SetSpeed(fPedalSpeed);
                }

                if (pAnimPedal)
                {
                    float fDesiredCrankAngle = -1000.0f;
                    if (bJustBlendedAnim)
                    {
                        if (pAnimLeanLeft && pAnimLeanLeft->GetBlendAmount() > 0.5f)
                        {
                            fDesiredCrankAngle = BMX_CRANK_TURNLEFTPOS;
                        }
                        else if (pAnimLeanRight && pAnimLeanRight->GetBlendAmount() > 0.5f)
                        {
                            fDesiredCrankAngle = BMX_CRANK_TURNRIGHTPOS;
                        }
                        else if (pAnimLeanFwd && pAnimLeanFwd->GetBlendAmount() > 0.5f)
                        {
                            fDesiredCrankAngle = BMX_CRANK_BUNNYHOPPOS;
                        }
                    }

                    if (fDesiredCrankAngle > -1000.0f)
                    {
                        fDesiredCrankAngle = (BMX_CRANK_PEDALSTARTPOS - fDesiredCrankAngle) / TWO_PI;
                        if (fDesiredCrankAngle < 0.0f)
                        {
                            fDesiredCrankAngle += 1.0f;
                        }
                        pAnimPedal->SetCurrentTime(pAnimPedal->GetHier()->GetTotalTime() * fDesiredCrankAngle);
                        m_fCrankAngle = fDesiredCrankAngle;
                    }
                    else
                    {
                        m_fCrankAngle = BMX_CRANK_PEDALSTARTPOS - pAnimPedal->GetCurrentTime() / pAnimPedal->GetHier()->GetTotalTime() * TWO_PI;
                    }
                }
                else
                {
                    m_fCrankAngle = CMaths::Pow(BMX_CRANK_RETURNRATE, CTimer::GetTimeStep()) * m_fCrankAngle;
                }

                if (CMaths::Abs(RideAnimData.AnimLeanLeft) > 0.05f || CMaths::Abs(RideAnimData.AnimLeanFwd) > 0.05f)
                {
                    RideAnimData.AnimLeanLeft = BMX_PEDAL_BLENDOUTLEAN * RideAnimData.AnimLeanLeft;
                    RideAnimData.AnimLeanFwd = BMX_PEDAL_BLENDOUTLEAN * RideAnimData.AnimLeanFwd;
                }
            }
            else
            {
                if ((pAnimPedal && pAnimPedal->GetBlendDelta() >= 0.0f && pAnimPedal->GetBlendAmount() > 0.0f) || (pAnimSprint && pAnimSprint->GetBlendDelta() >= 0.0f && pAnimSprint->GetBlendAmount() > 0.0f))
                {
                    if (pAnimPedal)
                    {
                        pAnimPedal->SetFlag(ANIMATION_IS_PLAYING, false);
                        pAnimPedal->SetBlendDelta(-8.0f);
                    }
                    if (pAnimSprint)
                    {
                        pAnimSprint->SetFlag(ANIMATION_IS_PLAYING, false);
                        pAnimSprint->SetBlendDelta(-8.0f);
                    }
                    RideAnimData.AnimLeanLeft = BMX_PEDAL_BLENDOUTLEAN * RideAnimData.AnimLeanLeft;
                    RideAnimData.AnimLeanFwd = BMX_PEDAL_BLENDOUTLEAN * RideAnimData.AnimLeanFwd;
                }
                else
                {
                    CBike::ProcessRiderAnims(pPed, this, &RideAnimData, pBikeHandling, 0);
                }

                if (pAnimPedal == nullptr && pAnimSprint == nullptr)
                {
                    m_fCrankAngle = 0.0f;
                }
                else
                {
                    if (pAnimPedal == nullptr)
                    {
                        pAnimPedal = pAnimSprint;
                    }
                    m_fCrankAngle = BMX_CRANK_PEDALSTARTPOS - pAnimPedal->GetCurrentTime() / pAnimPedal->GetHier()->GetTotalTime() * TWO_PI;
                }

                if (pAnimLeanLeft && pAnimLeanLeft->GetBlendAmount() > 0.1f)
                {
                    m_fCrankAngle = (1.0f - pAnimLeanLeft->GetBlendAmount()) * m_fCrankAngle + BMX_CRANK_TURNLEFTPOS * pAnimLeanLeft->GetBlendAmount();
                    m_bIsFreewheeling = true;
                }
                else if (pAnimLeanRight && pAnimLeanRight->GetBlendAmount() > 0.1f)
                {
                    m_fCrankAngle = (1.0f - pAnimLeanRight->GetBlendAmount()) * m_fCrankAngle + BMX_CRANK_TURNRIGHTPOS * pAnimLeanRight->GetBlendAmount();
                    m_bIsFreewheeling = true;
                }
                else if (pAnimLeanFwd && pAnimLeanFwd->GetBlendAmount() > 0.1f)
                {
                    m_fCrankAngle = (1.0f - pAnimLeanFwd->GetBlendAmount()) * m_fCrankAngle + BMX_CRANK_BUNNYHOPPOS * pAnimLeanFwd->GetBlendAmount();
                }
                else
                {
                    m_fCrankAngle = CMaths::Pow(BMX_CRANK_RETURNRATE, CTimer::GetTimeStep()) * m_fCrankAngle;
                }

                if (pAnimDriveBy)
                {
                    m_nFixRightHand = 0;
                    m_bIsFreewheeling = true;
                }
            }

            if (pPed->IsPlayer())
            {
                m_nBikeFlags.bWheelieForCamera = false;

                if (m_aWheelCounts[0] <= 0.0f && m_aWheelCounts[1] <= 0.0f && GetMatrix().GetForward().z > 0.0f && (m_aWheelCounts[2] > 0.0f || m_aWheelCounts[3] > 0.0f))
                {
                    float fWheelieAngle = pBikeHandling->m_fWheelieAng - GetMatrix().GetForward().z;
                    if (fWheelieAngle < pBikeHandling->m_fWheelieAng * 0.5f)
                    {
                        m_nBikeFlags.bWheelieForCamera = true;
                    }
                }
                else if (m_aWheelCounts[2] <= 0.0f && m_aWheelCounts[3] <= 0.0f && GetMatrix().GetForward().z < 0.0f && (m_aWheelCounts[0] > 0.0f || m_aWheelCounts[1] > 0.0f))
                {
                    float fStoppieAngle = pBikeHandling->m_fStoppieAng - GetMatrix().GetForward().z;
                    if (fStoppieAngle > pBikeHandling->m_fStoppieAng * 0.6f)
                    {
                        m_nBikeFlags.bWheelieForCamera = true;
                    }
                }
            }
        }

        m_fPedalAngleL = -m_fCrankAngle;
        m_fPedalAngleR = -m_fCrankAngle;
    }
}

float MAX_JUMP_CONTROL = 25.0f;
float MAX_CROUCH_ANIM_POS = 0.2f;
float BMX_BUNNYHOP_FORCE = 0.06f;
float BMX_BUNNYHOP_TURNF = 0.01f;

float BMX_BUNNYHOP_JUMPPLAYSPEED = 1.5f;

// 0x6C0590
// ASM Match
void CBmx::ProcessBunnyHop()
{
    CAnimBlendAssociation* pAnimHop = nullptr;
    if (m_pDriver)
    {
        pAnimHop = RpAnimBlendClumpGetAssociation(m_pDriver->GetRpClump(), ANIM_ID_BIKE_BUNNYHOP);
    }

    if (GetStatus() == STATUS_PLAYER && m_pDriver && m_pDriver->IsPlayer())
    {
        CPad* pPad = m_pDriver->AsPlayer()->GetPadFromPlayer();

        if (pPad->IsLeftShoulder1Pressed() && !pPad->DisablePlayerControls && m_fControlJump == 0.0f)
        {
            m_fControlJump += CTimer::GetTimeStep();
            pAnimHop = CAnimManager::BlendAnimation(m_pDriver->GetRpClump(), RideAnimData.AnimGroup, ANIM_ID_BIKE_BUNNYHOP, 8.0f);
            if (pAnimHop)
            {
                pAnimHop->SetCurrentTime(0.0f);
                pAnimHop->SetFlag(ANIMATION_IS_PLAYING, false);
            }
        }

        if (m_fControlJump > 0.0f)
        {
            if (pPad->DisablePlayerControls)
            {
                m_fControlJump = 0.0f;
                if (!pAnimHop)
                {
                    return;
                }
                pAnimHop->SetFlag(ANIMATION_IS_PLAYING, true);
                pAnimHop->SetFlag(ANIMATION_IS_BLEND_AUTO_REMOVE, true);
                pAnimHop->SetBlendDelta(-8.0f);
            }
            else
            {
                if (!pAnimHop)
                {
                    m_fControlJump = 0.0f;
                    return;
                }

                if (pPad->IsLeftShoulder1())
                {
                    if (!pAnimHop->IsPlaying())
                    {
                        m_fControlJump += CTimer::GetTimeStep();
                        if (m_fControlJump > MAX_JUMP_CONTROL)
                        {
                            m_fControlJump = MAX_JUMP_CONTROL;
                        }
                        pAnimHop->SetCurrentTime(m_fControlJump / MAX_JUMP_CONTROL * MAX_CROUCH_ANIM_POS);
                    }
                }
                else if (!pAnimHop->IsPlaying())
                {
                    if (pAnimHop->GetCurrentTime() < MAX_CROUCH_ANIM_POS)
                    {
                        const float fProgress = (MAX_CROUCH_ANIM_POS - pAnimHop->GetCurrentTime()) / MAX_CROUCH_ANIM_POS;
                        pAnimHop->SetCurrentTime(fProgress * (pAnimHop->GetHier()->GetTotalTime() - MAX_CROUCH_ANIM_POS) + MAX_CROUCH_ANIM_POS);
                    }
                    pAnimHop->SetSpeed(BMX_BUNNYHOP_JUMPPLAYSPEED);
                    pAnimHop->SetFlag(ANIMATION_IS_PLAYING, true);
                    pAnimHop->SetFinishCallback(LaunchBunnyHopCB, this);
                }
            }
        }
        else
        {
            if (!pAnimHop)
            {
                return;
            }
            pAnimHop->SetFlag(ANIMATION_IS_PLAYING, true);
            pAnimHop->SetFlag(ANIMATION_IS_BLEND_AUTO_REMOVE, true);
            pAnimHop->SetBlendDelta(-8.0f);
        }
        if (pAnimHop && pAnimHop->GetBlendAmount() > 0.5f)
        {
            SetGasPedal(0.0f);
            FindPlayerPed(-1)->GetPlayerData()->m_fMoveSpeed = 0.0f;
            if (!GetIsHandbrakeOn() && (m_aWheelRatios[0] < 1.0f || m_aWheelRatios[1] < 1.0f || m_aWheelRatios[2] < 1.0f || m_aWheelRatios[3] < 1.0f))
            {
                m_bIsFreewheeling = true;
            }
        }
    }
    else if (pAnimHop)
    {
        pAnimHop->SetFlag(ANIMATION_IS_PLAYING, true);
        pAnimHop->SetFlag(ANIMATION_IS_BLEND_AUTO_REMOVE, true);
        pAnimHop->SetBlendDelta(-8.0f);
    }
}

// 0x6C0390
// ASM Match
void CBmx::LaunchBunnyHopCB(CAnimBlendAssociation* pAnim, void* pData)
{
    CBmx* pBmx = static_cast<CBmx*>(pData);
    if ((pBmx->m_aWheelCounts[0] > 0.0f || pBmx->m_aWheelCounts[1] > 0.0f) && (pBmx->m_aWheelCounts[2] > 0.0f || pBmx->m_aWheelCounts[3] > 0.0f))
    {
        float fJumpForce = VMIN(1.0f, pBmx->m_fControlJump / MAX_JUMP_CONTROL) + 1.0f;

        if (pBmx->GetStatus() == STATUS_PLAYER)
        {
            fJumpForce *= CStats::GetFatAndMuscleModifier(STAT_MOD_6);
        }

        if (CCheat::IsActive(CJPHONEHOME_CHEAT))
        {
            fJumpForce *= 5.0f;
        }

        pBmx->ApplyMoveForce(pBmx->m_fMass * BMX_BUNNYHOP_FORCE * fJumpForce * pBmx->GetMatrix().GetUp());
        pBmx->ApplyTurnForce(pBmx->m_fTurnMass * BMX_BUNNYHOP_TURNF * fJumpForce * pBmx->GetMatrix().GetUp(), pBmx->GetMatrix().GetForward());
    }
}

// 0x6C0500
// ASM Match
void CBmx::GetFrameOffset(float& fZOffset, float& fAngleOffset)
{
    float fMoveFront = m_aWheelSuspensionHeights[0] - m_aWheelOrigHeights[0];
    float fMoveRear = m_aWheelSuspensionHeights[1] - m_aWheelOrigHeights[1];

    fZOffset = (1.0f - m_fMidWheelFracY) * fMoveFront + fMoveRear * m_fMidWheelFracY;
    fAngleOffset = CMaths::ATan2(fMoveFront - fMoveRear, m_fMidWheelDistY);
}

// 0x6C0550
// ASM Match
float CBmx::FindWheelWidth(bool bRear)
{
    return 0.07f;
}

// 0x6C0560
// ASM Match
void CBmx::BlowUpCar(CEntity* damager, bool bHideExplosion)
{
}

// 0x6C0810
// ASM Match
void CBmx::PreRender()
{
    CVehicle::PreRender();

    CColModel* colModel = GetColModel();
    CCollisionData* pColData = colModel ? colModel->m_pColData : nullptr;

    if (vehicleFlags.bVehicleColProcessed && pColData)
    {
        DoBurstAndSoftGroundRatios();

        float ratio = 1.0f - m_fSuspensionLength[0] / m_fLineLength[0];
        ratio = (VMIN(m_aWheelRatios[1], m_aWheelRatios[0]) - ratio) / (1.0f - ratio);

        float fHeight = pColData->m_pLines[0].m_vecStart.z;
        if (ratio > 0.0f)
        {
            fHeight -= ratio * m_fSuspensionLength[0];
        }
        m_aWheelSuspensionHeights[0] += (fHeight - m_aWheelSuspensionHeights[0]) * 0.75f;

        ratio = 1.0f - m_fSuspensionLength[2] / m_fLineLength[2];
        ratio = (VMIN(m_aWheelRatios[3], m_aWheelRatios[2]) - ratio) / (1.0f - ratio);

        fHeight = pColData->m_pLines[2].m_vecStart.z;
        if (ratio > 0.0f)
        {
            fHeight -= ratio * m_fSuspensionLength[2];
        }
        m_aWheelSuspensionHeights[1] += (fHeight - m_aWheelSuspensionHeights[1]) * 0.75f;
    }

    if (GetStatus() == STATUS_PHYSICS || GetStatus() == STATUS_PLAYER || GetStatus() == STATUS_PLAYER_PLAYBACK_FROM_BUFFER || GetStatus() == STATUS_SIMPLE)
    {
        float fTempSpeed = m_vecMoveSpeed.Magnitude();
        float fOutsideVector;
        uint32 nWheelParticleFlags;
        int32 nUseWheelLine;

        for (int32 i = 0; i < 2; i++)
        {
            if (i == 0)
            {
                nUseWheelLine = 0;
                if (m_aRatioHistory[0] >= 1.0f && m_aRatioHistory[1] < 1.0f)
                {
                    nUseWheelLine = 1;
                }
            }
            else
            {
                nUseWheelLine = 3;
                if (m_aRatioHistory[3] >= 1.0f && m_aRatioHistory[2] < 1.0f)
                {
                    nUseWheelLine = 2;
                }
            }

            nWheelParticleFlags = 0;
            if (i == 0 || m_aWheelState[1] == WHEEL_STATE_FIXED)
            {
                nWheelParticleFlags = 4;
            }

            float fTyreTouchSide = colModel->m_boundBox.m_vecMin.z * CMaths::Sin(RideAnimData.LeanAngle) * 0.8f;
            CVector vecRearWheelColPoint = m_aWheelColPoints[nUseWheelLine].m_vecPoint;
            vecRearWheelColPoint += fTyreTouchSide * GetMatrix().GetRight();

            if (bWheelBloody[i])
            {
                nWheelParticleFlags += 1;
            }
            if (bMoreSkidMarks[i])
            {
                nWheelParticleFlags += 2;
            }

            fOutsideVector = RideAnimData.LeanAngle > 0.0f ? -1.0f : 1.0f;

            AddSingleWheelParticles(m_aWheelState[i], m_nWheelStatus[i], m_aRatioHistory[nUseWheelLine], fTempSpeed, &m_aWheelColPoints[nUseWheelLine], &vecRearWheelColPoint, fOutsideVector, i, static_cast<uint32>(m_aWheelSkidmarkType[i]), &bWheelBloody[i], nWheelParticleFlags);
        }
    }

    m_bLeanMatrix = false;
    CalculateLeanMatrix();
    CShadows::StoreShadowForVehicle(this, VEH_SHD_BIKE);

    CMatrix matrix;
    CVector posn;

    CVehicleModelInfo* pModelInfo = static_cast<CVehicleModelInfo*>(CModelInfo::GetModelInfo(m_nModelIndex));
    CVector vecWheelSpeed;
    CVector vecWheelOffset;

    float fSin = CMaths::Sin(m_fSteerAngle);
    float fCos = CMaths::Cos(m_fSteerAngle);
    CVector vecFrontForward = GetMatrix().TransformVector(CVector(-fSin, fCos, 0.0f));
    CVector vecRearForward = GetMatrix().GetForward();

    if (m_aWheelCounts[0] > 0.0f || m_aWheelCounts[1] > 0.0f)
    {
        vecWheelOffset.x = 0.0f;
        vecWheelOffset.y = (pColData->m_pLines[1].m_vecStart.y + pColData->m_pLines[0].m_vecStart.y) * 0.5f;
        vecWheelOffset.z = pColData->m_pLines[0].m_vecStart.z - VMIN(m_aRatioHistory[0], m_aRatioHistory[1]) * m_fSuspensionLength[0] - pModelInfo->m_fWheelSizeFront * 0.5f;
        vecWheelSpeed = GetSpeed(vecWheelOffset);
        m_aWheelAngularVelocity[0] = ProcessWheelRotation(WHEEL_STATE_NORMAL, vecFrontForward, vecWheelSpeed, pModelInfo->m_fWheelSizeFront * 0.5f);
        m_aWheelPitchAngles[0] += m_aWheelAngularVelocity[0] * CTimer::GetTimeStep();
    }

    if (m_aWheelCounts[2] > 0.0f || m_aWheelCounts[3] > 0.0f)
    {
        vecWheelOffset.x = 0.0f;
        vecWheelOffset.y = (pColData->m_pLines[3].m_vecStart.y + pColData->m_pLines[2].m_vecStart.y) * 0.5f;
        vecWheelOffset.z = pColData->m_pLines[2].m_vecStart.z - VMIN(m_aRatioHistory[2], m_aRatioHistory[3]) * m_fSuspensionLength[2] - pModelInfo->m_fWheelSizeRear * 0.5f;
        vecWheelSpeed = GetSpeed(vecWheelOffset);
        m_aWheelAngularVelocity[1] = ProcessWheelRotation(m_aWheelState[1], vecRearForward, vecWheelSpeed, pModelInfo->m_fWheelSizeRear * 0.5f);
        m_aWheelPitchAngles[1] += m_aWheelAngularVelocity[1] * CTimer::GetTimeStep();
    }

    if (m_aBikeNodes[BMX_FORKS])
    {
        matrix.Attach(RwFrameGetMatrix(m_aBikeNodes[BMX_FORKS]), false);
        posn = matrix.GetPosition();

        CMatrix tempMat;
        RwMatrix tempRwMat;
        tempMat.Attach(&tempRwMat, false);
        tempMat.SetUnity();
        tempMat.UpdateRW();

        CVector vecRotAxis(0.0f, CMaths::Sin(pModelInfo->m_fBikeSteerAngle * 0.017453292f), -CMaths::Cos(pModelInfo->m_fBikeSteerAngle * 0.017453292f));
        vecRotAxis.Normalise();

        RwV3d rotAxis = { vecRotAxis.x, vecRotAxis.y, vecRotAxis.z };
        CQuaternion tempQuat;
        tempQuat.Set(&rotAxis, -RideAnimData.BarSteerAngle);
        tempQuat.Get(&tempRwMat);
        tempMat.Update();

        matrix.SetUnity();
        matrix = matrix * tempMat;
        matrix.Translate(posn);
        matrix.UpdateRW();

        if (m_aBikeNodes[BMX_HANDLEBARS])
        {
            matrix.Attach(RwFrameGetMatrix(m_aBikeNodes[BMX_HANDLEBARS]), false);
            posn = matrix.GetPosition();

            if (GetStatus() == STATUS_ABANDONED || GetStatus() == STATUS_WRECKED)
            {
                matrix.SetUnity();
                matrix *= tempMat;
                matrix.Translate(posn);
            }
            else
            {
                matrix.SetTranslate(posn);
            }
            matrix.UpdateRW();
        }
    }

    if (m_aBikeNodes[BMX_SWINGARM])
    {
        float fRotateAng = CMaths::ASin((m_aWheelSuspensionHeights[1] - m_aWheelOrigHeights[1]) / m_fSwingArmLength) * -1.0f;
        matrix.Attach(RwFrameGetMatrix(m_aBikeNodes[BMX_SWINGARM]), false);
        posn = matrix.GetPosition();
        matrix.SetRotate(fRotateAng, 0.0f, 0.0f);
        matrix.Translate(posn);
        matrix.UpdateRW();
    }

    static bool bMoveWheels;

    matrix.Attach(RwFrameGetMatrix(m_aBikeNodes[BMX_WHEEL_F]), false);
    posn = matrix.GetPosition();
    if (bMoveWheels)
    {
        posn.z = m_aWheelSuspensionHeights[0] - m_fForkZOffset;
        posn.y = (pColData->m_pLines[0].m_vecStart.y + pColData->m_pLines[1].m_vecStart.y) * 0.5f - m_fForkYOffset - (m_aWheelSuspensionHeights[0] - m_aWheelOrigHeights[0]) * m_fSteerAngleTan;
    }
    if (m_nWheelStatus[0] == 1)
    {
        matrix.SetRotate(m_aWheelPitchAngles[0], 0.0f, CMaths::Sin(m_aWheelPitchAngles[0]) * 0.02f);
    }
    else
    {
        matrix.SetRotateX(m_aWheelPitchAngles[0]);
    }
    matrix.Translate(posn);
    matrix.UpdateRW();

    matrix.Attach(RwFrameGetMatrix(m_aBikeNodes[BMX_WHEEL_R]), false);
    posn = matrix.GetPosition();
    if (bMoveWheels && m_aBikeNodes[BMX_SWINGARM] == nullptr)
    {
        posn.z = m_aWheelSuspensionHeights[1];
    }
    if (m_nWheelStatus[1] == 1)
    {
        matrix.SetRotate(m_aWheelPitchAngles[1], 0.0f, CMaths::Sin(m_aWheelPitchAngles[1]) * 0.04f);
    }
    else
    {
        matrix.SetRotateX(m_aWheelPitchAngles[1]);
    }
    matrix.Translate(posn);
    matrix.UpdateRW();

    if (m_aBikeNodes[BMX_CHASSIS])
    {
        float fMoveChassisZ = 0.0f;
        float fRotateChassisX = 0.0f;
        if (!bMoveWheels)
        {
            GetFrameOffset(fMoveChassisZ, fRotateChassisX);
        }

        matrix.Attach(RwFrameGetMatrix(m_aBikeNodes[BMX_CHASSIS]), false);
        posn = matrix.GetPosition();
        posn.z = fMoveChassisZ + colModel->m_boundBox.m_vecMin.z * 0.9f * (1.0f - CMaths::Cos(RideAnimData.LeanAngle));
        matrix.SetRotateX(CMaths::Abs(RideAnimData.LeanAngle) * BMX_LEAN_TO_PITCH_SCALE + fRotateChassisX);
        matrix.RotateY(m_fSprintLeanAngle + RideAnimData.LeanAngle);
        matrix.Translate(posn);
        matrix.UpdateRW();
    }

    if (m_aBikeNodes[BMX_CHAINSET])
    {
        matrix.Attach(RwFrameGetMatrix(m_aBikeNodes[BMX_CHAINSET]), false);
        posn = matrix.GetPosition();
        matrix.SetRotate(m_fCrankAngle, 0.0f, 0.0f);
        matrix.Translate(posn);
        matrix.UpdateRW();
    }

    if (m_aBikeNodes[BMX_PEDAL_L])
    {
        matrix.Attach(RwFrameGetMatrix(m_aBikeNodes[BMX_PEDAL_L]), false);
        posn = matrix.GetPosition();
        matrix.SetRotate(m_fPedalAngleL, 0.0f, 0.0f);
        matrix.Translate(posn);
        matrix.UpdateRW();
    }

    if (m_aBikeNodes[BMX_PEDAL_R])
    {
        matrix.Attach(RwFrameGetMatrix(m_aBikeNodes[BMX_PEDAL_R]), false);
        posn = matrix.GetPosition();
        matrix.SetRotate(m_fPedalAngleR, 0.0f, 0.0f);
        matrix.Translate(posn);
        matrix.UpdateRW();
    }
}
