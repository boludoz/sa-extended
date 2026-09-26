#include "StdInc.h"

#include "MonsterTruck.h"

float fWheelExtensionRate = 0.1f;                   // 0x8D33AC
bool gbMonsterTruckCapRatios = true;                // 0x8D33B0
float MONSTER_TRUCK_WHEEL_FORCE_MULT = -0.05f;      // 0x8D33B4
float MONSTER_TRUCK_WHEEL_DAMAGE_MULT = 0.05f;      // 0x8D33B8

void CMonsterTruck::InjectHooks() {
    RH_ScopedVirtualClass(CMonsterTruck, 0x8717d8, 71);
    RH_ScopedCategory("Vehicle");

    RH_ScopedInstall(Constructor, 0x6C8D60);

    RH_ScopedInstall(ExtendSuspension, 0x6C7D80);

    RH_ScopedVMTInstall(ProcessEntityCollision, 0x6C8AE0);
    RH_ScopedVMTInstall(ProcessSuspension, 0x6C83A0);
    RH_ScopedVMTInstall(ProcessControlCollisionCheck, 0x6C8330);
    RH_ScopedVMTInstall(ProcessControl, 0x6C8250);
    RH_ScopedVMTInstall(SetupSuspensionLines, 0x6C7FB0);
    RH_ScopedVMTInstall(PreRender, 0x6C7DE0);
    RH_ScopedVMTInstall(ResetSuspension, 0x6C7D40);
    RH_ScopedVMTInstall(BurstTyre, 0x6C7D30);
    RH_ScopedVMTInstall(SetUpWheelColModel, 0x6C7D20);
}

// 0x6C8D60
CMonsterTruck::CMonsterTruck(int32 modelIndex, eVehicleCreatedBy createdBy) : CAutomobile(modelIndex, createdBy, false) {
    std::ranges::fill(m_aBigTyreCompression, 1.0f);
    CMonsterTruck::SetupSuspensionLines();
    autoFlags.bIsMonsterTruck = true;
    m_vehicleType             = VEHICLE_TYPE_MTRUCK;
}

// 0x6C8AE0
int32 CMonsterTruck::ProcessEntityCollision(CEntity* entity, CColPoint* colPoint) {
    if (GetStatus() != STATUS_SIMPLE) {
        vehicleFlags.bVehicleColProcessed = true; // OK
    }

    const auto tcm = GetColModel();

    if (m_nPhysicalFlags.bSkipLineCol || m_nPhysicalFlags.bHalfSpeedCollision || entity->GetIsTypePed()) {
        tcm->GetData()->m_nNumLines = 0; // hmm..... (Later reset back to 4)
    }

    auto wheelColPtsTouchDists{ m_aWheelSuspensionHeights };
    const auto numColPts = CCollision::ProcessColModels(
        GetMatrix(), *tcm,
        entity->GetMatrix(), *entity->GetColModel(),
        *(std::array<CColPoint, 32>*)(colPoint), // trust me bro
        m_aWheelColPoints.data(),
        wheelColPtsTouchDists.data(),
        false
    );

    size_t numProcessedLines{};
    if (tcm->GetData()->m_nNumLines) {
        for (auto i = 0; i < MAX_CARWHEELS; i++) {
            const auto  thisWheelTouchDistNow = wheelColPtsTouchDists[i];
            const auto& thisWheelColPtNow = m_aWheelColPoints[i];

            if (thisWheelTouchDistNow <= m_aWheelSuspensionHeights[i]) {
                continue;
            }

            if (!(GetUsesCollision() || !numColPts)) { // TODO: Why is this in the loop body?
                continue;
            }

            numProcessedLines++;

            m_aWheelRatios[i] = 0.f;
            m_aWheelSuspensionHeights[i] = thisWheelTouchDistNow;

            m_anCollisionLighting[i] = thisWheelColPtNow.m_nLightingB;
            m_nContactSurface = thisWheelColPtNow.m_nSurfaceTypeB;

            // Same as in CAutomobile::ProcessEntityCollision
            switch (entity->GetType()) {
            case ENTITY_TYPE_VEHICLE:
            case ENTITY_TYPE_OBJECT: {
                CEntity::ChangeEntityReference(m_aGroundPhysicalPtrs[i], entity->AsPhysical());

                m_aGroundOffsets[i] = thisWheelColPtNow.m_vecPoint - entity->GetPosition();
                if (entity->GetIsTypeVehicle()) {
                    m_anCollisionLighting[i] = entity->AsVehicle()->m_anCollisionLighting[i];
                }
                break;
            }
            case ENTITY_TYPE_BUILDING: {
                m_pEntityWeAreOn = entity;
                m_bTunnel = entity->m_bTunnel;
                m_bTunnelTransition = entity->m_bTunnelTransition;
                break;
            }
            }
        }
    } else {
        tcm->GetData()->m_nNumLines = MAX_CARWHEELS; // TODO: Magic (Each wheel has 1 suspension line right now, but hardcoding like this isnt good)
    }

    if (numColPts > 0 || numProcessedLines > 0) {
        AddCollisionRecord(entity);
        if (!entity->GetIsTypeBuilding()) {
            entity->AsPhysical()->AddCollisionRecord(this);
        }
        if (numColPts > 0) {
            if (   entity->GetIsTypeBuilding()
                || (entity->GetIsTypeObject() && entity->AsPhysical()->m_nPhysicalFlags.bInfiniteMass)
            ) {
                SetHasHitWall(true);
            }
        }
    }

    return numColPts;
}

// 0x6C83A0
// ASM Match: 99.66%
void CMonsterTruck::ProcessSuspension()
{
    int32 i;
    float aWheelSpringForces[4];
    CVector aWheelSpeeds[4];
    CVector aWheelDirections[4];
    CVector aWheelOffsets[4];

    for (i = 0; i < 4; i++)
    {
        aWheelDirections[i] = -1.0f * GetMatrix().GetUp();
        if (m_aWheelRatios[i] < 1.0f)
        {
            aWheelOffsets[i] = m_aWheelColPoints[i].m_vecPoint - GetPosition();
        }
        else
        {
            aWheelOffsets[i] = CVector(0.0f, 0.0f, 0.0f);
        }
    }

    for (i = 0; i < 4; i++)
    {
        if (m_aWheelRatios[i] < 1.0f)
        {
            float fSusBias = m_pHandlingData->m_fSuspensionBiasBetweenFrontAndRear;
            if (i == 1 || i == 3)
            {
                fSusBias = 1.0f - fSusBias;
            }
            ApplySpringCollisionAlt(m_pHandlingData->m_fSuspensionForceLevel, aWheelDirections[i], aWheelOffsets[i], m_aWheelRatios[i], fSusBias, m_aWheelColPoints[i].m_vecNormal, aWheelSpringForces[i]);
        }
    }

    for (i = 0; i < 4; i++)
    {
        aWheelSpeeds[i] = GetSpeed(aWheelOffsets[i]);
        if (m_aGroundPhysicalPtrs[i] != nullptr)
        {
            aWheelSpeeds[i] -= m_aGroundPhysicalPtrs[i]->GetSpeed(m_aGroundOffsets[i]);
        }

        if (m_aWheelRatios[i] < 1.0f && m_aWheelColPoints[i].m_vecNormal.z > 0.35f)
        {
            aWheelDirections[i] = -m_aWheelColPoints[i].m_vecNormal;
        }
    }

    for (i = 0; i < 4; i++)
    {
        if (m_aWheelRatios[i] < 1.0f)
        {
            ApplySpringDampening(m_pHandlingData->m_fSuspensionDampingLevel, aWheelSpringForces[i], aWheelDirections[i], aWheelOffsets[i], aWheelSpeeds[i]);
        }
    }

    for (i = 0; i < 4; i++)
    {
        if (m_aWheelRatios[i] < 1.0f && m_aGroundPhysicalPtrs[i] != nullptr && m_aGroundPhysicalPtrs[i]->GetIsTypeVehicle())
        {
            if (m_aWheelRatios[i] < 0.5f)
            {
                float fDamage = (1.0f - m_aWheelRatios[i]) * m_fMass * MONSTER_TRUCK_WHEEL_DAMAGE_MULT;
                static_cast<CVehicle*>(m_aGroundPhysicalPtrs[i])->VehicleDamage(fDamage, static_cast<eVehicleCollisionComponent>(m_aWheelColPoints[i].m_nPieceTypeB), this, &m_aWheelColPoints[i].m_vecPoint, &m_aWheelColPoints[i].m_vecNormal, WEAPON_RAMMEDBYCAR);
            }

            if (m_aWheelColPoints[i].m_vecNormal.z > 0.5f)
            {
                CVector vecForce = m_aWheelColPoints[i].m_vecNormal;
                vecForce.x *= 0.25f;
                vecForce.y *= 0.25f;
                m_aGroundPhysicalPtrs[i]->ApplyForce(vecForce * MONSTER_TRUCK_WHEEL_FORCE_MULT * (1.0f - m_aWheelRatios[i]) * m_aGroundPhysicalPtrs[i]->m_fMass, m_aWheelColPoints[i].m_vecPoint - m_aGroundPhysicalPtrs[i]->GetPosition(), true);
            }
        }
        m_aGroundPhysicalPtrs[i] = nullptr;
    }
}

// 0x6C8330
// ASM Match: 99.84%
void CMonsterTruck::ProcessControlCollisionCheck(bool applySpeed)
{
    ExtendSuspension();
    CAutomobile::ProcessControlCollisionCheck(applySpeed);

    for (int i = 0; i < 4; i++)
    {
        if (m_aWheelRatios[i] < 1.0f)
        {
            m_aWheelRatios[i] = (m_fSuspensionLength[i] - m_aWheelSuspensionHeights[i]) / (m_fSuspensionLength[i] - m_fLineLength[i]);
        }
        else
        {
            m_aWheelRatios[i] = 1.0f;
        }
    }
}

// 0x6C8250
// ASM Match: 99.38%
void CMonsterTruck::ProcessControl()
{
    int32 i;
    bool MONSTER_TRUCK_CAP_RATIOS = gbMonsterTruckCapRatios;

    for (i = 0; i < 4; i++)
    {
        if (m_aWheelRatios[i] < 1.0f)
        {
            m_aWheelRatios[i] = (m_fSuspensionLength[i] - m_aWheelSuspensionHeights[i]) / (m_fSuspensionLength[i] - m_fLineLength[i]);
            if (m_aWheelRatios[i] < 0.0f && MONSTER_TRUCK_CAP_RATIOS)
            {
                m_aWheelRatios[i] = 0.0f;
            }
        }
        else
        {
            m_aWheelRatios[i] = 1.0f;
        }
    }

    CAutomobile::ProcessControl();

    if (!GetWasPostponed() && (m_vecMoveSpeed.x != 0.0f || m_vecMoveSpeed.y != 0.0f || m_vecMoveSpeed.z != 0.0f || m_vecTurnSpeed.x != 0.0f || m_vecTurnSpeed.y != 0.0f || m_vecTurnSpeed.z != 0.0f))
    {
        ExtendSuspension();
    }
}

// 0x6C7FB0
// ASM Match: 99.76%
void CMonsterTruck::SetupSuspensionLines()
{
    CVehicleModelInfo* pModelInfo = GetVehicleModelInfo();
    CColModel& colModel = *pModelInfo->GetColModel();
    CCollisionData* pColData = colModel.m_pColData;
    int16 i;

    m_fWheelRadius = pModelInfo->m_fWheelSizeFront * 0.5f;

    if (pColData->m_pDisks == nullptr)
    {
        pColData->bUsesDisks = true;
        pColData->m_nNumLines = 4;
        pColData->m_pDisks = static_cast<CColDisk*>(CMemoryMgr::Malloc(4 * sizeof(CColDisk)));
    }
    else if (!pColData->bUsesDisks)
    {
        CMemoryMgr::Free(pColData->m_pLines);
        pColData->bUsesDisks = true;
        pColData->m_nNumLines = 4;
        pColData->m_pDisks = static_cast<CColDisk*>(CMemoryMgr::Malloc(4 * sizeof(CColDisk)));
    }

    CVector vecDiskDepth(0.0f, 0.0f, 0.0f);
    for (i = 0; i < 4; i++)
    {
        CVector posn;
        pModelInfo->GetWheelPosn(i, posn, false);

        vecDiskDepth.x = -1.0f;
        if (i >= 2)
        {
            vecDiskDepth.x = 1.0f;
        }

        pColData->m_pDisks[i].Set(m_fWheelRadius, posn, vecDiskDepth, 0.6f * m_fWheelRadius, SURFACE_WHEELBASE, CAR_PIECE_WHEEL_LF, tColLighting(0xFF));
        switch (i)
        {
        case 1:
            pColData->m_pDisks[i].m_Surface.m_nPiece = CAR_PIECE_WHEEL_RL;
            break;
        case 2:
            pColData->m_pDisks[i].m_Surface.m_nPiece = CAR_PIECE_WHEEL_RF;
            break;
        case 3:
            pColData->m_pDisks[i].m_Surface.m_nPiece = CAR_PIECE_WHEEL_RR;
            break;
        }

        m_fSuspensionLength[i] = posn.z + m_pHandlingData->m_fSuspensionUpperLimit;
        m_fLineLength[i] = posn.z + m_pHandlingData->m_fSuspensionLowerLimit;
    }

    float fStart = m_fSuspensionLength[0];
    float fLength = m_fSuspensionLength[0] - m_fLineLength[0];
    fLength *= 1.0f - 1.0f / (m_pHandlingData->m_fSuspensionForceLevel * 4.0f);
    m_fHeightAboveRoad = fLength + (m_fWheelRadius - fStart);
    m_fRearHeightAboveRoad = m_fHeightAboveRoad;

    for (i = 0; i < 4; i++)
    {
        m_aWheelSuspensionHeights[i] = pModelInfo->m_fWheelSizeFront * 0.5f - m_fHeightAboveRoad;
        m_aWheelRatios[i] = 1.0f;
    }

    if (m_fHeightAboveRoad - m_fWheelRadius < colModel.m_boundBox.m_vecMin.z)
    {
        colModel.m_boundBox.m_vecMin.z = m_fHeightAboveRoad - m_fWheelRadius;
    }

    fLength = VMAX(colModel.m_boundBox.m_vecMin.Magnitude(), colModel.m_boundBox.m_vecMax.Magnitude());
    if (colModel.m_boundSphere.m_fRadius < fLength)
    {
        colModel.m_boundSphere.m_fRadius = fLength;
    }
}

// 0x6C7DE0
void CMonsterTruck::PreRender() {
    for (auto i = 0; i < 4; i++) {
        m_aWheelSuspensionHeights[i] = std::min(m_aWheelSuspensionHeights[i], m_fSuspensionLength[i]);
    }

    CAutomobile::PreRender();

    const auto mi = GetVehicleModelInfo();
    CMatrix mat;
    CVector pos;

    mi->GetWheelPosn(CAR_WHEEL_FRONT_LEFT, pos, false);
    SetTransmissionRotation(m_aCarNodes[MONSTER_TRANSMISSION_F], m_aWheelSuspensionHeights[CAR_WHEEL_FRONT_LEFT], m_aWheelSuspensionHeights[CAR_WHEEL_FRONT_RIGHT], pos, true);

    mi->GetWheelPosn(CAR_WHEEL_REAR_LEFT, pos, false);
    SetTransmissionRotation(m_aCarNodes[MONSTER_TRANSMISSION_R], m_aWheelSuspensionHeights[CAR_WHEEL_REAR_LEFT], m_aWheelSuspensionHeights[CAR_WHEEL_REAR_RIGHT], pos, false);

    if (m_nModelIndex == MODEL_DUMPER && m_aCarNodes[MONSTER_MISC_A]) {
        SetComponentRotation(m_aCarNodes[MONSTER_MISC_A], AXIS_X, (float)m_wMiscComponentAngle * DUMPER_COL_ANGLEMULT, true);
    }
}

// 0x6C7D80
// ASM Match: 99.69%
void CMonsterTruck::ExtendSuspension()
{
    for (int i = 0; i < 4; i++)
    {
        m_aWheelSuspensionHeights[i] -= CTimer::GetTimeStep() * m_fWheelRadius * fWheelExtensionRate;
        if (m_aWheelSuspensionHeights[i] < m_fLineLength[i])
        {
            m_aWheelSuspensionHeights[i] = m_fLineLength[i];
        }
        else if (m_aWheelSuspensionHeights[i] > m_fSuspensionLength[i])
        {
            m_aWheelSuspensionHeights[i] = m_fSuspensionLength[i];
        }
        m_aWheelRatios[i] = 1.0f;
    }
}

// 0x6C7D40
// ASM Match
void CMonsterTruck::ResetSuspension()
{
    CAutomobile::ResetSuspension();

    for (int32 i = 0; i < 4; i++)
    {
        m_aWheelSuspensionHeights[i] = m_fLineLength[i];
        m_aBigTyreCompression[i] = 1.0f;
    }
}

// 0x6C7D30
bool CMonsterTruck::BurstTyre(uint8 tyreComponentId, bool bPhysicalEffect) {
    return false;
}

// 0x6C7D20
bool CMonsterTruck::SetUpWheelColModel(CColModel* colModel) {
    return false;
}
