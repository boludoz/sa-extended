#include "StdInc.h"

#include "ProjectileInfo.h"

#include "Entity/Object/Projectile.h"
#include "Radar.h"
#include "World.h"
#include "Pools/Pools.h"
#include "Collision/Box.h"

#include "AudioEngine.h"
#include "Fx/FxManager.h"
#include "Streaming.h"
#include "ObjectData.h"
#include "General.h"
#include "Camera.h"
#include "WeaponInfo.h"
#include "ModelInfo.h"

void CProjectileInfo::InjectHooks() {
    RH_ScopedClass(CProjectileInfo);
    RH_ScopedCategoryGlobal();

#if MODERN_CAM
    RH_ScopedInstall(Initialise, 0x737B40);
    RH_ScopedInstall(Shutdown, 0x737BC0);
    RH_ScopedInstall(GetProjectileInfo, 0x737BF0);
    RH_ScopedInstall(RemoveNotAdd, 0x737C00, { .reversed = false });
    RH_ScopedInstall(AddProjectile, 0x737C80);
    RH_ScopedInstall(RemoveDetonatorProjectiles, 0x738860);
    RH_ScopedInstall(RemoveProjectile, 0x7388F0);
    RH_ScopedInstall(Update, 0x738B20);
    RH_ScopedInstall(IsProjectileInRange, 0x739860);
    RH_ScopedInstall(RemoveAllProjectiles, 0x7399B0);
    RH_ScopedInstall(RemoveIfThisIsAProjectile, 0x739A40);
    RH_ScopedInstall(RemoveFXSystem, 0x737B80);
#else
    RH_ScopedInstall(Initialise, 0x737B40, { .reversed = false });
    RH_ScopedInstall(Shutdown, 0x737BC0, { .reversed = false });
    RH_ScopedInstall(GetProjectileInfo, 0x737BF0, { .reversed = false });
    RH_ScopedInstall(RemoveNotAdd, 0x737C00, { .reversed = false });
    RH_ScopedInstall(AddProjectile, 0x737C80, { .reversed = false });
    RH_ScopedInstall(RemoveDetonatorProjectiles, 0x738860, { .reversed = false });
    RH_ScopedInstall(RemoveProjectile, 0x7388F0, { .reversed = false });
    RH_ScopedInstall(Update, 0x738B20, { .reversed = false });
    RH_ScopedInstall(IsProjectileInRange, 0x739860);
    RH_ScopedInstall(RemoveAllProjectiles, 0x7399B0, { .reversed = false });
    RH_ScopedInstall(RemoveIfThisIsAProjectile, 0x739A40, { .reversed = false });
    RH_ScopedInstall(RemoveFXSystem, 0x737B80, { .reversed = false });
#endif
}

// 0x737B40
void CProjectileInfo::Initialise() {
#if MODERN_CAM
    for (auto& info : gaProjectileInfo) {
        info.m_nWeaponType = WEAPON_UNARMED;
        info.m_pCreator = nullptr;
        info.m_pVictim = nullptr;
        info.m_nDestroyTime = 0;
        info.m_bActive = false;
        info.m_vecLastPosn.Set(0.0f, 0.0f, 0.0f);
        info.m_pFxSystem = nullptr;
    }
    for (auto& proj : ms_apProjectile) {
        proj = nullptr;
    }
#else
    plugin::Call<0x737B40>();
#endif
}

// 0x737BC0
void CProjectileInfo::Shutdown() {
#if MODERN_CAM
    RemoveAllProjectiles();
#else
    plugin::Call<0x737BC0>();
#endif
}

// 0x737BF0
CProjectileInfo* CProjectileInfo::GetProjectileInfo(int32 infoId) {
#if MODERN_CAM
    return &gaProjectileInfo[infoId];
#else
    return plugin::CallAndReturn<CProjectileInfo*, 0x737BF0, int32>(infoId);
#endif
}

// 0x737C00
void CProjectileInfo::RemoveNotAdd(CEntity* creator, eWeaponType weaponType, CVector pos) {
    plugin::Call<0x737C00, CEntity*, eWeaponType, CVector>(creator, weaponType, pos);
}

// 0x737C80
bool CProjectileInfo::AddProjectile(CEntity* creator, eWeaponType projectileType, CVector origin, float force, const CVector* dir, CEntity* target) {
    CMatrix a;
    a.SetUnity();

    float elasticity = 0.75f;
    bool applyGravity = true;
    uint8 objectInfoIdx = 0;
    CVector moveSpeed(0.0f, 0.0f, 0.0f);
    uint32 timeToDestroy = 0;

    switch (projectileType) {
    case WEAPON_GRENADE:
    case WEAPON_REMOTE_SATCHEL_CHARGE: {
        timeToDestroy = CTimer::GetTimeInMS() + 2000;
        float speed = (force == 0.0f) ? 0.0f : (force * 0.22f + 0.15f);
        if (projectileType == WEAPON_REMOTE_SATCHEL_CHARGE) {
            speed *= 0.5f;
        }

        float angle;
        if (creator && creator->m_matrix) {
            angle = std::atan2(-creator->m_matrix->GetForward().x, creator->m_matrix->GetForward().y);
        } else if (creator) {
            angle = creator->m_placement.m_fHeading;
        } else {
            angle = 0.0f;
        }

        if (creator && creator->GetIsTypeVehicle()) {
            angle = CGeneral::LimitRadianAngle(angle + PI);
        }

        a.SetUnity();
        a.RotateZ(angle);
        a.GetPosition() += origin;

#if MODERN_CAM
        if (dir) {
            moveSpeed = *dir;
        } else
#endif
        {
            moveSpeed.x = -std::sin(angle) * speed;
            moveSpeed.y = std::cos(angle) * speed;
            moveSpeed.z = (force + 1.0f) * 0.4f * speed;

            if (creator && creator->m_nModelIndex == MODEL_SENTINEL && creator->GetIsTypePhysical()) {
                moveSpeed += creator->AsPhysical()->m_vecMoveSpeed;
            }
        }

        objectInfoIdx = 5;
        elasticity = (projectileType == WEAPON_REMOTE_SATCHEL_CHARGE) ? 0.03f : 0.5f;
        break;
    }
    case WEAPON_TEARGAS: {
        timeToDestroy = CTimer::GetTimeInMS() + 20000;
        float speed = (force == 0.0f) ? 0.0f : (force * 0.22f + 0.15f);

        float angle;
        if (creator && creator->m_matrix) {
            angle = std::atan2(-creator->m_matrix->GetForward().x, creator->m_matrix->GetForward().y);
        } else if (creator) {
            angle = creator->m_placement.m_fHeading;
        } else {
            angle = 0.0f;
        }

        a.SetUnity();
        a.RotateZ(angle);
        objectInfoIdx = 5;
        elasticity = 0.5f;
        a.GetPosition() += origin;

#if MODERN_CAM
        if (dir) {
            moveSpeed = *dir;
        } else
#endif
        {
            moveSpeed.x = -std::sin(angle) * speed;
            moveSpeed.y = std::cos(angle) * speed;
            moveSpeed.z = (force * 0.2f + 0.4f) * speed;
        }
        break;
    }
    case WEAPON_MOLOTOV: {
        timeToDestroy = CTimer::GetTimeInMS() + 2000;
        float speed = std::max(0.2f, force * 0.22f + 0.15f);

        float angle;
        if (creator && creator->m_matrix) {
            angle = std::atan2(-creator->m_matrix->GetForward().x, creator->m_matrix->GetForward().y);
        } else if (creator) {
            angle = creator->m_placement.m_fHeading;
        } else {
            angle = 0.0f;
        }

        a.SetUnity();
        a.RotateZ(angle);
        a.GetPosition() += origin;

#if MODERN_CAM
        if (dir) {
            moveSpeed = *dir;
        } else
#endif
        {
            moveSpeed.x = -std::sin(angle) * speed;
            moveSpeed.y = std::cos(angle) * speed;
            moveSpeed.z = (force * 0.2f + 0.4f) * speed;
        }
        break;
    }
    case WEAPON_ROCKET:
    case WEAPON_ROCKET_HS: {
        float speed;
        if (projectileType == WEAPON_ROCKET) {
            timeToDestroy = CTimer::GetTimeInMS() + 3000;
            speed = 0.4f;
        } else {
            timeToDestroy = CTimer::GetTimeInMS() + 10000;
            speed = 0.2f;
        }

        if (creator && creator->GetIsTypeVehicle()) {
            if (creator->m_matrix) {
                a = *creator->m_matrix;
            } else {
                a.SetUnity();
                a.RotateZ(creator->m_placement.m_fHeading);
            }
            a.GetPosition() = origin;
            if (creator->GetIsTypePhysical()) {
                speed += creator->AsPhysical()->m_vecMoveSpeed.Magnitude();
            }
        } else if (creator && creator->GetIsTypePed() && creator->AsPed()->IsPlayer()) {
            auto& activeCam = TheCamera.GetActiveCam();
            a.GetForward() = activeCam.m_vecFront;
            a.GetUp() = activeCam.m_vecUp;
            a.GetRight() = CrossProduct(activeCam.m_vecUp, activeCam.m_vecFront);
            a.GetPosition() = origin;
        } else if (dir) {
            a.GetForward() = *dir;
            if (creator) {
                a.GetRight() = creator->GetMatrix().GetRight();
            } else {
                a.GetRight() = CVector{1.0f, 0.0f, 0.0f};
            }
            a.GetUp() = CrossProduct(a.GetRight(), a.GetForward());
            a.GetPosition() = origin;
        } else if (creator) {
            if (creator->m_matrix) {
                a = *creator->m_matrix;
            } else {
                a.SetUnity();
                a.RotateZ(creator->m_placement.m_fHeading);
            }
            a.GetPosition() = origin;
        }

        CVector inVec(0.0f, speed, 0.0f);
        moveSpeed = a.TransformVector(inVec);
        applyGravity = false;
        break;
    }
    case WEAPON_FREEFALL_BOMB: {
        timeToDestroy = CTimer::GetTimeInMS() + 2000000;
        if (creator && creator->GetIsTypePhysical()) {
            moveSpeed = creator->AsPhysical()->m_vecMoveSpeed;
        } else {
            moveSpeed.Set(0.0f, 0.0f, 0.0f);
        }
        if (creator) {
            if (creator->m_matrix) {
                a = *creator->m_matrix;
            } else {
                a.SetUnity();
                a.RotateZ(creator->m_placement.m_fHeading);
            }
        }
        a.GetPosition() = origin;
        break;
    }
    case WEAPON_FLARE: {
        CStreaming::RequestModel(ModelIndices::MI_FLARE, STREAMING_DEFAULT);
        timeToDestroy = CTimer::GetTimeInMS() + 10000;
        if (creator && creator->GetIsTypePhysical()) {
            moveSpeed = creator->AsPhysical()->m_vecMoveSpeed;
        } else {
            moveSpeed.Set(0.0f, 0.0f, 0.0f);
        }
        if (creator) {
            if (creator->m_matrix) {
                a = *creator->m_matrix;
            } else {
                a.SetUnity();
                a.RotateZ(creator->m_placement.m_fHeading);
            }
        }
        a.GetPosition() = origin;
        break;
    }
    default:
        break;
    }

    int32 slot = 0;
    for (; slot < MAX_PROJECTILES; slot++) {
        if (!gaProjectileInfo[slot].m_bActive)
            break;
    }
    if (slot == MAX_PROJECTILES) {
        return false;
    }

    CProjectile* projectile = nullptr;
    switch (projectileType) {
    case WEAPON_GRENADE:
    case WEAPON_TEARGAS:
    case WEAPON_MOLOTOV:
    case WEAPON_REMOTE_SATCHEL_CHARGE: {
        CWeaponInfo* info = CWeaponInfo::GetWeaponInfo(projectileType, eWeaponSkill::STD);
        projectile = new CProjectile(info->m_nModelId1);
        ms_apProjectile[slot] = projectile;
        if (projectile) {
            CColModel* colModel = CModelInfo::GetModelInfo(projectile->m_nModelIndex)->GetColModel();
            CCollisionData* colData = colModel->m_pColData;
            if (colData) {
                if (!colData->m_nNumSpheres && !colData->m_pSpheres) {
                    colData->m_nNumSpheres = 1;
                    colData->m_pSpheres = static_cast<CColSphere*>(CMemoryMgr::Malloc(sizeof(CColSphere)));
                    float radius = colModel->m_boundSphere.m_fRadius * 0.75f;
                    colData->m_pSpheres[0].Set(radius, colModel->m_boundSphere.m_vecCenter, static_cast<eSurfaceType>(0x38), 0, tColLighting{0xFF});
                }
            } else {
                colModel->AllocateData(1, 0, 0, 0, 0, false);
                float radius = colModel->m_boundSphere.m_fRadius * 0.75f;
                colModel->m_pColData->m_pSpheres[0].Set(radius, colModel->m_boundSphere.m_vecCenter, static_cast<eSurfaceType>(0x38), 0, tColLighting{0xFF});
            }
            colModel->m_bHasCollisionVolumes = true;
        }
        break;
    }
    case WEAPON_ROCKET:
    case WEAPON_ROCKET_HS:
    case WEAPON_FREEFALL_BOMB: {
        CWeaponInfo* info = CWeaponInfo::GetWeaponInfo(projectileType, eWeaponSkill::STD);
        projectile = new CProjectile(info->m_nModelId1);
        ms_apProjectile[slot] = projectile;
        break;
    }
    case WEAPON_FLARE: {
        projectile = new CProjectile(ModelIndices::MI_FLARE);
        ms_apProjectile[slot] = projectile;
        if (projectile) {
            projectile->m_fAirResistance = 0.9f;
        }
        break;
    }
    default:
        break;
    }

    if (!ms_apProjectile[slot]) {
        return false;
    }

    CProjectileInfo& projInfo = gaProjectileInfo[slot];
    projInfo.m_nWeaponType = projectileType;
    projInfo.m_pCreator = creator;
    if (creator) {
        creator->RegisterReference(reinterpret_cast<CEntity**>(&projInfo.m_pCreator));
    }

    projectile->SetMatrix(a);
    projectile->m_vecMoveSpeed = moveSpeed;
    projectile->physicalFlags.bApplyGravity = applyGravity;
    projInfo.m_nDestroyTime = timeToDestroy;
    projectile->m_fElasticity = elasticity;

    if (objectInfoIdx == 5) {
        projectile->m_pObjectInfo = &CObjectData::GetAtIndex(4);
    }

    projInfo.m_pVictim = target;
    if (target) {
        target->RegisterReference(reinterpret_cast<CEntity**>(&projInfo.m_pVictim));
    }

    projInfo.m_bActive = true;
    CWorld::Add(projectile);
    projectile->RegisterReference(reinterpret_cast<CEntity**>(&ms_apProjectile[slot]));

    projInfo.m_vecLastPosn = projectile->GetPosition();

    if (projectileType == WEAPON_TEARGAS) {
        CVector zero{0.0f, 0.0f, 0.0f};
        RwMatrix* modMat = projectile->GetModellingMatrix();
        if (modMat) {
            projInfo.m_pFxSystem = g_fxMan.CreateFxSystem("teargasAD", zero, modMat, false);
            if (projInfo.m_pFxSystem) {
                projInfo.m_pFxSystem->Play();
            }
        }
    }

    projectile->m_nPhysicalFlags |= 0x10000000;
    projectile->physicalFlags.bDisableCollisionForce = false;
    projectile->m_bUsesCollision = true;
    projectile->physicalFlags.bCollidable = true;
    projectile->m_pEntityIgnoredCollision = creator;

    if (creator && (creator->GetIsTypeVehicle() || creator->GetIsTypePed())) {
        if (!creator->AsPhysical()->m_pEntityIgnoredCollision) {
            creator->AsPhysical()->m_pEntityIgnoredCollision = projectile;
        }
    }

    if (projectileType == WEAPON_ROCKET_HS) {
        int32 poolIdx = GetObjectPool()->GetIndex(projectile);
        int32 blipHandle = CRadar::SetEntityBlip(BLIP_OBJECT, poolIdx, 0xFF0000FF, BLIP_DISPLAY_BOTH);
        CRadar::ChangeBlipScale(blipHandle, 1);
        if (creator == FindPlayerPed() || creator == FindPlayerVehicle()) {
            CRadar::ChangeBlipColour(blipHandle, static_cast<eBlipColour>(0xFFFFFFFF));
        } else {
            CRadar::ChangeBlipColour(blipHandle, static_cast<eBlipColour>(0xFF0000FF));
        }
    }

    AudioEngine.ReportWeaponEvent(AE_PROJECTILE_FIRE, projectileType, projectile);
    return true;
}

#include "Fx/Fx.h"
#include "Replay.h"
#include "Explosion.h"
#include "Vehicle.h"

// 0x738860
void CProjectileInfo::RemoveDetonatorProjectiles() {
#if MODERN_CAM
    for (int32 i = 0; i < MAX_PROJECTILES; i++) {
        if (gaProjectileInfo[i].m_bActive && gaProjectileInfo[i].m_nWeaponType == WEAPON_REMOTE_SATCHEL_CHARGE) {
            if (ms_apProjectile[i]) {
                RemoveProjectile(&gaProjectileInfo[i], ms_apProjectile[i]);
            }
        }
    }
#else
    plugin::Call<0x738860>();
#endif
}

// 0x7388F0
void CProjectileInfo::RemoveProjectile(CProjectileInfo* info, CProjectile* object) {
#if MODERN_CAM
    if (!info || !object)
        return;

    const CVector pos = object->GetPosition();

    switch (static_cast<eWeaponType>(info->m_nWeaponType)) {
    case WEAPON_GRENADE:
    case WEAPON_REMOTE_SATCHEL_CHARGE:
    case WEAPON_FREEFALL_BOMB: {
        CExplosion::AddExplosion(nullptr, info->m_pCreator, eExplosionType::EXPLOSION_GRENADE, pos, 0, true, -1.0f, false);
        break;
    }
    case WEAPON_MOLOTOV: {
        CExplosion::AddExplosion(nullptr, info->m_pCreator, eExplosionType::EXPLOSION_MOLOTOV, pos, 0, true, -1.0f, false);
        AudioEngine.ReportObjectDestruction(object);
        break;
    }
    case WEAPON_ROCKET: {
        CEntity* launcher = info->m_pCreator;
        if (launcher && launcher->GetIsTypeVehicle()) {
            if (auto* veh = launcher->AsVehicle()) {
                launcher = veh->m_pDriver;
            }
        }
        CExplosion::AddExplosion(nullptr, launcher, eExplosionType::EXPLOSION_ROCKET, pos, 0, true, -1.0f, false);
        break;
    }
    case WEAPON_ROCKET_HS: {
        bool isPlayer = (info->m_pCreator == FindPlayerPed());
        eExplosionType expType = isPlayer ? eExplosionType::EXPLOSION_ROCKET : eExplosionType::EXPLOSION_WEAK_ROCKET;
        CExplosion::AddExplosion(nullptr, info->m_pCreator, expType, pos, 0, true, -1.0f, false);
        break;
    }
    default:
        break;
    }

    info->m_bActive = false;
    if (info->m_pFxSystem) {
        info->m_pFxSystem->Kill();
        info->m_pFxSystem = nullptr;
    }

    CRadar::ClearBlipForEntity(eBlipType::BLIP_OBJECT, GetObjectPool()->GetRef(object));
    CWorld::Remove(object);
    delete object;
#else
    plugin::Call<0x7388F0, CProjectileInfo*, CProjectile*>(info, object);
#endif
}

// 0x738B20
void CProjectileInfo::Update() {
#if MODERN_CAM
    if (CReplay::Mode == MODE_PLAYBACK) {
        return;
    }

    for (int32 i = 0; i < MAX_PROJECTILES; i++) {
        CProjectileInfo& info = gaProjectileInfo[i];
        if (!info.m_bActive) {
            continue;
        }

        CProjectile* proj = ms_apProjectile[i];
        if (!proj) {
            info.m_bActive = false;
            continue;
        }

        if (proj->physicalFlags.bTouchingWater) {
            if (info.m_pFxSystem) {
                info.m_pFxSystem->Kill();
                info.m_pFxSystem = nullptr;
            }
        }

        if (info.m_pCreator && info.m_pCreator->GetIsTypePed()) {
            if (!info.m_pCreator->AsPed()->IsPointerValid()) {
                info.m_pCreator = nullptr;
            }
        }

        const auto weaponType = static_cast<eWeaponType>(info.m_nWeaponType);

        if (weaponType == WEAPON_REMOTE_SATCHEL_CHARGE || weaponType == WEAPON_GRENADE || weaponType == WEAPON_TEARGAS) {
            if (proj->m_fElasticity > 0.1f &&
                std::abs(proj->m_vecMoveSpeed.x) < 0.05f &&
                std::abs(proj->m_vecMoveSpeed.y) < 0.05f &&
                std::abs(proj->m_vecMoveSpeed.z) < 0.05f)
            {
                proj->m_fElasticity = 0.03f;
            }

            if (weaponType == WEAPON_TEARGAS &&
                CTimer::GetTimeInMS() > static_cast<uint32>(info.m_nDestroyTime - 17500) &&
                CGeneral::GetRandomNumberInRange(0, 100) < 10)
            {
                CVector pos = proj->GetPosition();
                CWorld::SetPedsChoking(pos.x, pos.y, pos.z, 6.0f, info.m_pCreator);
            }
        }

        if (weaponType == WEAPON_ROCKET || weaponType == WEAPON_ROCKET_HS) {
            FxPrtMult_c prtMult{0.3f, 0.3f, 0.3f, 0.3f, 0.5f, 1.0f, 0.08f};
            CVector moveStep = proj->m_vecMoveSpeed * CTimer::GetTimeStep();
            float dist = moveStep.Magnitude();
            int32 numParticles = std::max(1, static_cast<int32>(dist));

            for (int32 p = 0; p < numParticles; p++) {
                float randCol = (float)rand() * (1.0f / 32767.0f) * 0.25f + 0.25f;
                prtMult.SetColor(randCol, randCol, randCol, 0.3f);
                prtMult.m_fLife = (float)rand() * (1.0f / 32767.0f) * 0.04f + 0.08f;

                float factor = 1.0f - (float)p / (float)numParticles;
                CVector pos = proj->GetPosition() - moveStep * factor;

                CVector randDir(
                    (float)rand() * (2.0f / 32767.0f) - 1.0f,
                    (float)rand() * (2.0f / 32767.0f) - 1.0f,
                    (float)rand() * (2.0f / 32767.0f) - 1.0f
                );
                randDir.Normalise();

                CVector moveDir = proj->m_vecMoveSpeed;
                moveDir.Normalise();

                CVector vel = CrossProduct(moveDir, randDir) * 1.5f;
                if (g_fx.m_SmokeHuge) {
                    g_fx.m_SmokeHuge->AddParticle(pos, vel, 0.0f, prtMult, -1.0f, 1.2f, 0.6f, false);
                }
            }
        }

        if (info.m_nDestroyTime && CTimer::GetTimeInMS() > static_cast<uint32>(info.m_nDestroyTime)) {
            if (weaponType == WEAPON_REMOTE_SATCHEL_CHARGE) {
                if (info.m_pCreator && info.m_pCreator->GetIsTypePed() && info.m_pCreator->AsPed()->IsPlayer()) {
                    auto* player = info.m_pCreator->AsPed();
                    auto slot = player->GetWeaponSlot(eWeaponType::WEAPON_DETONATOR);
                    if (player->m_aWeapons[slot].m_Type != WEAPON_DETONATOR || player->m_aWeapons[slot].m_TotalAmmo == 0) {
                        info.m_nDestroyTime = 0;
                    }
                }
                info.m_vecLastPosn = proj->GetPosition();
                continue;
            }

            RemoveProjectile(&info, proj);
            continue;
        }

        if (weaponType == WEAPON_ROCKET) {
            float accel = CTimer::GetTimeStep() * 0.008f;
            proj->m_vecMoveSpeed += proj->GetForward() * accel;
            float curSpeed = proj->m_vecMoveSpeed.Magnitude();
            if (curSpeed > 9.9f) {
                proj->m_vecMoveSpeed *= (9.9f / curSpeed);
            }
        }
        else if (weaponType == WEAPON_FLARE) {
            proj->m_bUsesCollision = false;
            CWorld::pIgnoreEntity = info.m_pCreator;
            CVector curPos = proj->GetPosition();
            bool losClear = CWorld::GetIsLineOfSightClear(info.m_vecLastPosn, curPos, true, true, true, true, false, false, false);
            proj->m_bUsesCollision = true;
            CWorld::pIgnoreEntity = nullptr;

            if (!losClear) {
                proj->m_vecMoveSpeed.Set(0.0f, 0.0f, 0.0f);
                proj->GetPosition() = info.m_vecLastPosn;
            }
            info.m_vecLastPosn = proj->GetPosition();
            continue;
        }
        else if (weaponType == WEAPON_MOLOTOV || weaponType == WEAPON_FREEFALL_BOMB) {
            CVector curPos = proj->GetPosition();
            CWorld::pIgnoreEntity = info.m_pCreator;
            proj->m_bUsesCollision = false;

            bool explode = false;
            if (!info.m_pCreator || (info.m_vecLastPosn - info.m_pCreator->GetPosition()).Magnitude() > 0.0f) {
                if (proj->physicalFlags.bCollisionProof || !CWorld::GetIsLineOfSightClear(info.m_vecLastPosn, curPos, true, true, true, true, false, false, false)) {
                    explode = true;
                }
            }
            CWorld::pIgnoreEntity = nullptr;
            proj->m_bUsesCollision = true;

            if (explode) {
                RemoveProjectile(&info, proj);
                continue;
            }
            info.m_vecLastPosn = proj->GetPosition();
            continue;
        }
        else if (weaponType == WEAPON_ROCKET_HS) {
            if (info.m_pVictim) {
                if (info.m_pVictim == FindPlayerVehicle()) {
                    AudioEngine.ReportFrontendAudioEvent(AE_MISSILE_LOCK, 0.0f, 1.0f);
                }

                CVector targetFront = proj->GetForward();
                CVector targetPos = proj->GetPosition() + targetFront;
                float bestScore = CWeapon::EvaluateTargetForHeatSeekingMissile(info.m_pVictim, targetPos, targetFront, 1.2f, true, nullptr);

                float bestFlareScore = 0.0f;
                CEntity* flareTarget = nullptr;
                for (int32 f = 0; f < MAX_PROJECTILES; f++) {
                    if (gaProjectileInfo[f].m_nWeaponType == WEAPON_FLARE && gaProjectileInfo[f].m_bActive) {
                        float flareScore = CWeapon::EvaluateTargetForHeatSeekingMissile(ms_apProjectile[f], targetPos, targetFront, 1.2f, true, nullptr);
                        if (flareScore >= bestFlareScore) {
                            bestFlareScore = flareScore;
                            flareTarget = ms_apProjectile[f];
                        }
                    }
                }

                CEntity* targetEntity = (flareTarget && bestFlareScore > bestScore) ? flareTarget : info.m_pVictim;

                bool isPlane = false;
                if (targetEntity->GetIsTypeVehicle()) {
                    auto* player = FindPlayerPed();
                    if ((info.m_pCreator == player || info.m_pCreator == FindPlayerVehicle()) && targetEntity->AsVehicle()->IsPlane()) {
                        isPlane = true;
                    }
                }

                CVector projPos = proj->GetPosition();
                CVector projAhead = projPos + proj->m_vecMoveSpeed * 100.0f;
                if (isPlane) {
                    projAhead = projPos;
                }

                CVector targetEntityPos = targetEntity->GetPosition();
                float distToTarget = (projPos - targetEntityPos).Magnitude();
                float leadDist = std::min(50.0f, distToTarget);
                if (isPlane) {
                    leadDist = std::min(1.5f, distToTarget);
                }

                CVector targetAimPos = targetEntityPos + (targetEntity->GetIsTypePhysical() ? targetEntity->AsPhysical()->m_vecMoveSpeed : CVector{}) * leadDist;
                CVector toTarget = targetAimPos - projAhead;

                CVector moveDir = proj->m_vecMoveSpeed;
                moveDir.Normalise();

                toTarget.Normalise();
                float dot = DotProduct(toTarget, moveDir);
                if (dot < 0.0f) {
                    toTarget -= moveDir * dot;
                }
                toTarget.Normalise();

                float turnRate = 0.009f;
                float damp = 1.0f;
                if (info.m_pCreator == FindPlayerPed() || info.m_pCreator == FindPlayerVehicle()) {
                    turnRate = 0.0117f;
                }
                if (targetEntity->GetIsTypePhysical() && targetEntity->AsPhysical()->m_vecMoveSpeed.Magnitude() > 0.8f) {
                    turnRate *= 1.2f;
                }

                proj->m_vecMoveSpeed *= damp;
                proj->m_vecMoveSpeed += toTarget * (turnRate * CTimer::GetTimeStep());

                float speed = proj->m_vecMoveSpeed.Magnitude();
                if (speed > 9.9f) {
                    proj->m_vecMoveSpeed *= (9.9f / speed);
                }

                proj->GetMatrix().GetForward() = moveDir;
            }
        }
        else if (weaponType == WEAPON_REMOTE_SATCHEL_CHARGE) {
            if (proj->m_fDamageIntensity > 0.0f) {
                if (proj->m_pDamageEntity && !proj->m_pAttachedTo && proj->m_pDamageEntity->GetIsTypePhysical()) {
                    proj->AttachEntityToEntity(proj->m_pDamageEntity->AsPhysical(), nullptr, nullptr);
                    proj->m_bUsesCollision = false;
                }
            }
            info.m_vecLastPosn = proj->GetPosition();
            continue;
        }

        if (weaponType == WEAPON_ROCKET || weaponType == WEAPON_ROCKET_HS) {
            if (!proj->physicalFlags.bCollisionProof) {
                CWorld::pIgnoreEntity = info.m_pCreator;
                proj->m_bUsesCollision = false;
                CVector curPos = proj->GetPosition();
                bool losClear = CWorld::GetIsLineOfSightClear(info.m_vecLastPosn, curPos, true, true, true, true, false, false, false);
                CWorld::pIgnoreEntity = nullptr;
                proj->m_bUsesCollision = true;
                proj->m_pEntityIgnoredCollision = info.m_pCreator;

                if (!losClear) {
                    RemoveProjectile(&info, proj);
                    continue;
                }
            }

            if (proj->m_nNumEntitiesCollided > 0) {
                CEntity* col = proj->m_apCollidedEntities[0];
                if (col && col != info.m_pCreator && col->m_nModelIndex != ModelIndices::MI_FLARE) {
                    RemoveProjectile(&info, proj);
                    continue;
                }
            }
        }

        info.m_vecLastPosn = proj->GetPosition();
    }
#else
    return plugin::Call<0x738B20>();
#endif
}

// 0x739860
bool CProjectileInfo::IsProjectileInRange(float x1, float x2, float y1, float y2, float z1, float z2, bool bDestroy) {
    const CBox bb{
        CVector{ x1, y1, z1 },
        CVector{ x2, y2, z2 }
    };
    bool found = false;
    for (auto&& [info, proj] : rngv::zip(gaProjectileInfo, ms_apProjectile)) {
        if (!info.m_bActive) {
            continue;
        }

        if (!IsWeaponTypeProjectile(static_cast<eWeaponType>(info.m_nWeaponType))) {
            continue;
        }

        if (!bb.IsPointInside(proj->GetPosition())) {
            continue;
        }

        found = true;
        if (bDestroy) {
            RemoveProjectile(&info, proj);
        }
    }
    return found;
}

// 0x7399B0
void CProjectileInfo::RemoveAllProjectiles() {
#if MODERN_CAM
    for (int32 i = 0; i < MAX_PROJECTILES; i++) {
        if (gaProjectileInfo[i].m_bActive && ms_apProjectile[i]) {
            RemoveProjectile(&gaProjectileInfo[i], ms_apProjectile[i]);
        }
    }
#else
    plugin::Call<0x7399B0>();
#endif
}

// 0x739A40
bool CProjectileInfo::RemoveIfThisIsAProjectile(CObject* object) {
#if MODERN_CAM
    for (int32 i = 0; i < MAX_PROJECTILES; i++) {
        if (gaProjectileInfo[i].m_bActive && ms_apProjectile[i] == object) {
            RemoveProjectile(&gaProjectileInfo[i], ms_apProjectile[i]);
            return true;
        }
    }
    return false;
#else
    return plugin::CallAndReturn<bool, 0x739A40, CObject*>(object);
#endif
}

// 0x737B80
void CProjectileInfo::RemoveFXSystem(bool bInstantly) {
#if MODERN_CAM
    if (m_pFxSystem) {
        if (bInstantly) {
            m_pFxSystem->Kill();
        } else {
            m_pFxSystem->Stop();
        }
        m_pFxSystem = nullptr;
    }
#else
    plugin::CallMethod<0x737B80, CProjectileInfo*, bool>(this, bInstantly);
#endif
}
