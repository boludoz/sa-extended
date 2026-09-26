#include "StdInc.h"

#include "ProjectileInfo.h"

#include "Entity/Object/Projectile.h"
#include "Radar.h"
#include "World.h"
#include "Pools/Pools.h"
#include "Collision/Box.h"
#include "Explosion.h"
#include "AudioEngine.h"
#include "Streaming.h"
#include "Replay.h"
#include "Camera.h"
#include "Weapon.h"
#include "WeaponInfo.h"
#include "ObjectData.h"
#include "FxManager.h"
#include "FxSystem.h"
#include "FxPrtMult.h"
#include "Fx.h"

void CProjectileInfo::InjectHooks() {
    RH_ScopedClass(CProjectileInfo);
    RH_ScopedCategoryGlobal();

    RH_ScopedInstall(Initialise, 0x737B40);
    RH_ScopedInstall(Shutdown, 0x737BC0);
    RH_ScopedInstall(GetProjectileInfo, 0x737BF0);
    RH_ScopedInstall(RemoveNotAdd, 0x737C00);
    RH_ScopedInstall(AddProjectile, 0x737C80);
    RH_ScopedInstall(RemoveDetonatorProjectiles, 0x738860);
    RH_ScopedInstall(RemoveProjectile, 0x7388F0);
    RH_ScopedInstall(Update, 0x738B20);
    RH_ScopedInstall(IsProjectileInRange, 0x739860);
    RH_ScopedInstall(RemoveAllProjectiles, 0x7399B0);
    RH_ScopedInstall(RemoveIfThisIsAProjectile, 0x739A40);
    RH_ScopedInstall(RemoveFXSystem, 0x737B80);
}

// 0x737B40
void CProjectileInfo::Initialise() {
    ms_apProjectile.fill(nullptr);
    for (auto& info : gaProjectileInfo) {
        info.m_eProjectileType     = WEAPON_GRENADE;
        info.m_pEntProjectileOwner = nullptr;
        info.m_fTimeExpires        = 0;
        info.m_bProjectileActive   = false;
        info.m_fxSystem            = nullptr;
    }
}

// 0x737BC0
void CProjectileInfo::Shutdown() {
    for (auto& info : gaProjectileInfo) {
        if (info.m_fxSystem) {
            g_fxMan.DestroyFxSystem(info.m_fxSystem);
            info.m_fxSystem = nullptr;
        }
    }
}

// 0x737BF0
CProjectileInfo* CProjectileInfo::GetProjectileInfo(int32 infoId) {
    return &gaProjectileInfo[infoId];
}

// 0x737C00
void CProjectileInfo::RemoveNotAdd(CEntity* creator, eWeaponType weaponType, CVector pos) {
    eExplosionType expType;
    switch (weaponType) {
    case WEAPON_GRENADE:
    case WEAPON_REMOTE_SATCHEL_CHARGE:
        expType = eExplosionType::EXPLOSION_GRENADE;
        break;
    case WEAPON_MOLOTOV:
        expType = eExplosionType::EXPLOSION_MOLOTOV;
        break;
    case WEAPON_ROCKET:
    case WEAPON_ROCKET_HS:
        expType = eExplosionType::EXPLOSION_ROCKET;
        break;
    default:
        return;
    }
    CExplosion::AddExplosion(nullptr, creator, expType, pos, 0, true, -1.0f, false);
}

// 0x737C80
bool CProjectileInfo::AddProjectile(CEntity* creator, eWeaponType projectileType, CVector origin, float force, const CVector* dir, CEntity* target) {
    CMatrix mat;
    float fHeading = 0.0f;
    float fForce = 0.0f;
    CVector vecMoveSpeed;
    uint32 nDestroyTime = 0;
    float fElasticity = 0.75f;
    bool bDoGravity = true;
    uint8 nCollisionSpecial = 0;

    switch (projectileType) {
    case WEAPON_GRENADE:
    case WEAPON_REMOTE_SATCHEL_CHARGE: {
        nDestroyTime = CTimer::GetTimeInMS() + 2000;
        fForce = (force == 0.0f) ? 0.0f : (force * 0.22f + 0.15f);
        if (projectileType == WEAPON_REMOTE_SATCHEL_CHARGE) {
            fForce *= 0.5f;
        }

        if (creator->m_matrix) {
            fHeading = std::atan2(-creator->m_matrix->GetForward().x, creator->m_matrix->GetForward().y);
        } else {
            fHeading = creator->GetHeading();
        }

        if (creator->GetIsTypeVehicle()) {
            fHeading = CGeneral::LimitRadianAngle(fHeading + PI);
        }

        mat.SetTranslate(CVector(0.0f, 0.0f, 0.0f));
        mat.RotateZ(fHeading);
        mat.GetPosition() += origin;

        vecMoveSpeed.x = -std::sin(fHeading) * fForce;
        vecMoveSpeed.y = std::cos(fHeading) * fForce;
        vecMoveSpeed.z = (force + 1.0f) * 0.4f * fForce;

        if (creator->m_nModelIndex == MODEL_ENFORCER) {
            vecMoveSpeed += static_cast<CPhysical*>(creator)->m_vecMoveSpeed;
        }

        nCollisionSpecial = 5;
        fElasticity = (projectileType == WEAPON_REMOTE_SATCHEL_CHARGE) ? 0.03f : 0.5f;
        break;
    }
    case WEAPON_TEARGAS: {
        nDestroyTime = CTimer::GetTimeInMS() + 20000;
        fForce = (force == 0.0f) ? 0.0f : (force * 0.22f + 0.15f);

        if (creator->m_matrix) {
            fHeading = std::atan2(-creator->m_matrix->GetForward().x, creator->m_matrix->GetForward().y);
        } else {
            fHeading = creator->GetHeading();
        }

        mat.SetTranslate(CVector(0.0f, 0.0f, 0.0f));
        mat.RotateZ(fHeading);
        mat.GetPosition() += origin;

        vecMoveSpeed.x = -std::sin(fHeading) * fForce;
        vecMoveSpeed.y = std::cos(fHeading) * fForce;
        vecMoveSpeed.z = (force + 1.0f) * 0.4f * fForce;

        nCollisionSpecial = 5;
        fElasticity = 0.5f;
        break;
    }
    case WEAPON_MOLOTOV: {
        nDestroyTime = CTimer::GetTimeInMS() + 2000;
        fForce = std::max(force * 0.22f + 0.15f, 0.2f);

        if (creator->m_matrix) {
            fHeading = std::atan2(-creator->m_matrix->GetForward().x, creator->m_matrix->GetForward().y);
        } else {
            fHeading = creator->GetHeading();
        }

        mat.SetTranslate(CVector(0.0f, 0.0f, 0.0f));
        mat.RotateZ(fHeading);
        mat.GetPosition() += origin;

        vecMoveSpeed.x = -std::sin(fHeading) * fForce;
        vecMoveSpeed.y = std::cos(fHeading) * fForce;
        vecMoveSpeed.z = (force * 0.2f + 0.4f) * fForce;

        fElasticity = 0.5f;
        break;
    }
    case WEAPON_ROCKET:
    case WEAPON_ROCKET_HS: {
        float fSpeed = 0.0f;
        if (projectileType == WEAPON_ROCKET) {
            nDestroyTime = CTimer::GetTimeInMS() + 3000;
            fSpeed = 0.4f;
        } else {
            nDestroyTime = CTimer::GetTimeInMS() + 10000;
            fSpeed = 0.2f;
        }

        if (creator->GetIsTypeVehicle()) {
            mat = creator->GetMatrix();
            mat.GetPosition() = origin;
            fSpeed += static_cast<CPhysical*>(creator)->m_vecMoveSpeed.Magnitude();
        } else if (creator->GetIsTypePed() && creator->AsPed()->IsPlayer()) {
            const auto& activeCam = TheCamera.m_aCams[TheCamera.m_nActiveCam];
            mat.GetForward()  = activeCam.m_vecFront;
            mat.GetUp()       = activeCam.m_vecUp;
            mat.GetRight()    = CrossProduct(activeCam.m_vecUp, activeCam.m_vecFront);
            mat.GetPosition() = origin;
        } else if (dir) {
            mat.GetForward()  = *dir;
            mat.GetRight()    = creator->GetMatrix().GetRight();
            mat.GetUp()       = CrossProduct(mat.GetRight(), mat.GetForward());
            mat.GetPosition() = origin;
        } else {
            mat = creator->GetMatrix();
            mat.GetPosition() = origin;
        }

        vecMoveSpeed = mat.TransformVector(CVector(0.0f, fSpeed, 0.0f));
        bDoGravity = false;
        break;
    }
    case WEAPON_FREEFALL_BOMB:
    case WEAPON_FLARE: {
        if (projectileType == WEAPON_FREEFALL_BOMB) {
            nDestroyTime = CTimer::GetTimeInMS() + 2000000;
        } else {
            CStreaming::RequestModel(MODEL_FLARE, STREAMING_DEFAULT);
            nDestroyTime = CTimer::GetTimeInMS() + 10000;
        }

        if (creator->GetIsTypePhysical()) {
            vecMoveSpeed = static_cast<CPhysical*>(creator)->m_vecMoveSpeed;
        } else {
            vecMoveSpeed.Set(0.0f, 0.0f, 0.0f);
        }
        mat = creator->GetMatrix();
        mat.GetPosition() = origin;
        break;
    }
    default:
        break;
    }

    uint32 slot = 0;
    for (; slot < MAX_PROJECTILES; ++slot) {
        if (!gaProjectileInfo[slot].m_bProjectileActive) {
            break;
        }
    }
    if (slot == MAX_PROJECTILES) {
        return false;
    }

    CProjectile* proj = nullptr;
    switch (projectileType) {
    case WEAPON_GRENADE:
    case WEAPON_TEARGAS:
    case WEAPON_MOLOTOV:
    case WEAPON_REMOTE_SATCHEL_CHARGE: {
        auto* weaponInfo = CWeaponInfo::GetWeaponInfo(projectileType, eWeaponSkill::STD);
        proj = new CProjectile(weaponInfo->m_nModelId1);
        ms_apProjectile[slot] = proj;
        if (proj) {
            auto* colModel = CModelInfo::GetModelInfo(proj->m_nModelIndex)->GetColModel();
            auto* colData = colModel->m_pColData;
            float radius = colModel->m_boundSphere.m_fRadius * 0.75f;
            if (colData) {
                if (!colData->m_nNumSpheres && !colData->m_pSpheres) {
                    colData->m_nNumSpheres = 1;
                    colData->m_pSpheres = static_cast<CColSphere*>(CMemoryMgr::Malloc(sizeof(CColSphere)));
                    colData->m_pSpheres->Set(radius, colModel->m_boundSphere.m_vecCenter, SURFACE_GIRDER, 0, tColLighting{ 0xFF });
                }
            } else {
                colModel->AllocateData(1, 0, 0, 0, 0, 0);
                colModel->m_pColData->m_pSpheres->Set(radius, colModel->m_boundSphere.m_vecCenter, SURFACE_GIRDER, 0, tColLighting{ 0xFF });
            }
        }
        break;
    }
    case WEAPON_ROCKET:
    case WEAPON_ROCKET_HS:
    case WEAPON_FREEFALL_BOMB: {
        auto* weaponInfo = CWeaponInfo::GetWeaponInfo(projectileType, eWeaponSkill::STD);
        proj = new CProjectile(weaponInfo->m_nModelId1);
        ms_apProjectile[slot] = proj;
        break;
    }
    case WEAPON_FLARE: {
        proj = new CProjectile(MODEL_FLARE);
        ms_apProjectile[slot] = proj;
        if (proj) {
            proj->m_fAirResistance = 0.9f;
        }
        break;
    }
    default:
        break;
    }

    if (!ms_apProjectile[slot]) {
        return false;
    }

    auto& info = gaProjectileInfo[slot];
    info.m_eProjectileType     = projectileType;
    info.m_pEntProjectileOwner = creator;
    if (creator) {
        creator->RegisterReference(reinterpret_cast<CEntity**>(&info.m_pEntProjectileOwner));
    }

    proj->SetMatrix(mat);
    proj->m_vecMoveSpeed = vecMoveSpeed;
    proj->m_nPhysicalFlags.bDoGravity = bDoGravity;
    info.m_fTimeExpires = nDestroyTime;
    proj->m_fElasticity = fElasticity;
    if (nCollisionSpecial == 5) {
        proj->m_pObjectInfo = &CObjectData::GetAtIndex(4);
    }

    info.m_pTargetEntity = target;
    if (target) {
        target->RegisterReference(reinterpret_cast<CEntity**>(&info.m_pTargetEntity));
    }

    info.m_bProjectileActive = true;
    CWorld::Add(proj);
    proj->RegisterReference(reinterpret_cast<CEntity**>(&ms_apProjectile[slot]));
    info.OldCoors = proj->GetPosition();

    if (projectileType == WEAPON_TEARGAS) {
        RwMatrix* modellingMatrix = proj->GetModellingMatrix();
        if (modellingMatrix) {
            CVector zeroPos{ 0.0f, 0.0f, 0.0f };
            auto* fx = g_fxMan.CreateFxSystem("teargasAD", zeroPos, modellingMatrix, false);
            info.m_fxSystem = fx;
            if (fx) {
                fx->Play();
            }
        }
    }

    proj->m_nPhysicalFlagsRaw |= PHYSICAL_CANBECOLLIDEDWITH;
    proj->m_pEntityIgnoredCollision = creator;

    if (creator && creator->GetIsTypePhysical()) {
        auto* phys = creator->AsPhysical();
        if (!phys->m_pEntityIgnoredCollision) {
            phys->m_pEntityIgnoredCollision = creator;
        }
    }

    if (projectileType == WEAPON_ROCKET_HS) {
        auto blip = CRadar::SetEntityBlip(
            eBlipType::BLIP_OBJECT,
            GetObjectPool()->GetRef(proj),
            0xFF0000FF,
            eBlipDisplay::BLIP_DISPLAY_BLIPONLY
        );
        CRadar::ChangeBlipScale(blip, 1);
        if (creator == FindPlayerPed(-1) || creator == FindPlayerVehicle(-1, false)) {
            CRadar::ChangeBlipColour(blip, eBlipColour::BLIP_COLOUR_WHITE);
        } else {
            CRadar::ChangeBlipColour(blip, eBlipColour::BLIP_COLOUR_BLUE);
        }
    }

    AudioEngine.ReportWeaponEvent(AE_PROJECTILE_FIRE, projectileType, proj);
    return true;
}

// 0x738860
void CProjectileInfo::RemoveDetonatorProjectiles() {
    for (auto i = 0u; i < MAX_PROJECTILES; ++i) {
        auto& info = gaProjectileInfo[i];
        if (info.m_bProjectileActive && info.m_eProjectileType == WEAPON_REMOTE_SATCHEL_CHARGE) {
            auto* proj = ms_apProjectile[i];
            CExplosion::AddExplosion(nullptr, info.m_pEntProjectileOwner, eExplosionType::EXPLOSION_GRENADE, proj->GetPosition(), 0, true, -1.0f, false);
            info.m_bProjectileActive = false;
            if (info.m_fxSystem) {
                info.m_fxSystem->Kill();
                info.m_fxSystem = nullptr;
            }
            proj->m_bRemoveFromWorld = true;
        }
    }
}

// 0x7388F0
void CProjectileInfo::RemoveProjectile(CProjectileInfo* info, CProjectile* object) {
    switch (info->m_eProjectileType) {
    case WEAPON_GRENADE:
    case WEAPON_FREEFALL_BOMB:
        CExplosion::AddExplosion(nullptr, info->m_pEntProjectileOwner, eExplosionType::EXPLOSION_GRENADE, object->GetPosition(), 0, true, -1.0f, false);
        break;
    case WEAPON_MOLOTOV:
        CExplosion::AddExplosion(nullptr, info->m_pEntProjectileOwner, eExplosionType::EXPLOSION_MOLOTOV, object->GetPosition(), 0, true, -1.0f, false);
        if (object) {
            AudioEngine.ReportObjectDestruction(object);
        }
        break;
    case WEAPON_ROCKET: {
        CEntity* creator = info->m_pEntProjectileOwner;
        if (creator && creator->GetIsTypeVehicle()) {
            creator = creator->AsVehicle()->m_pDriver;
        }
        CExplosion::AddExplosion(nullptr, creator, eExplosionType::EXPLOSION_ROCKET, object->GetPosition(), 0, true, -1.0f, false);
        break;
    }
    case WEAPON_ROCKET_HS: {
        CEntity* creator = info->m_pEntProjectileOwner;
        eExplosionType expType = (creator == FindPlayerPed(-1)) ? eExplosionType::EXPLOSION_ROCKET : eExplosionType::EXPLOSION_WEAK_ROCKET;
        CExplosion::AddExplosion(nullptr, creator, expType, object->GetPosition(), 0, true, -1.0f, false);
        break;
    }
    default:
        break;
    }

    info->m_bProjectileActive = false;
    if (info->m_fxSystem) {
        info->m_fxSystem->Kill();
        info->m_fxSystem = nullptr;
    }
    CRadar::ClearBlipForEntity(eBlipType::BLIP_OBJECT, GetObjectPool()->GetRef(object));
    CWorld::Remove(object);
    delete object;
}

// 0x738B20
void CProjectileInfo::Update() {
    if (CReplay::Mode == 1) {
            return;
    }

    for (auto i = 0u; i < MAX_PROJECTILES; ++i) {
        auto& info = gaProjectileInfo[i];
        if (!info.m_bProjectileActive) {
            continue;
        }

        auto* proj = ms_apProjectile[i];
        if (!proj) {
            info.m_bProjectileActive = false;
            continue;
        }

        // Si está sumergido en agua, apagar el sistema de partículas
        if (proj->m_nPhysicalFlags.bIsInWater) {
            if (info.m_fxSystem) {
                info.m_fxSystem->Kill();
                info.m_fxSystem = nullptr;
            }
        }

        // Validar el puntero del creador/dueño
        if (info.m_pEntProjectileOwner) {
            if (info.m_pEntProjectileOwner->GetIsTypePed() && !GetPedPool()->IsObjectValid(info.m_pEntProjectileOwner->AsPed())) {
                info.m_pEntProjectileOwner = nullptr;
            }
        }

        // Granadas, satchel y gas lacrimógeno: reducir rebote si la velocidad es muy baja
        if (info.m_eProjectileType == WEAPON_REMOTE_SATCHEL_CHARGE || info.m_eProjectileType == WEAPON_GRENADE || info.m_eProjectileType == WEAPON_TEARGAS) {
            if (proj->m_fElasticity > 0.1f
                && std::fabs(proj->m_vecMoveSpeed.x) < 0.05f
                && std::fabs(proj->m_vecMoveSpeed.y) < 0.05f
                && std::fabs(proj->m_vecMoveSpeed.z) < 0.05f) {
                proj->m_fElasticity = 0.03f;
            }

            // Efecto de asfixia del gas lacrimógeno
            if (info.m_eProjectileType == WEAPON_TEARGAS
                && CTimer::GetTimeInMS() > info.m_fTimeExpires - 17500
                && CGeneral::GetRandomNumberInRange(0, 100) < 10) {
                const CVector& pos = proj->GetPosition();
                CWorld::SetPedsChoking(pos.x, pos.y, pos.z, 6.0f, info.m_pEntProjectileOwner);
            }
        }

        // Partículas de humo para cohetes
        if (info.m_eProjectileType == WEAPON_ROCKET || info.m_eProjectileType == WEAPON_ROCKET_HS) {
            FxPrtMult_c particleData(0.3f, 0.3f, 0.3f, 0.3f, 0.5f, 1.0f, 0.08f);
            CVector moveVec = proj->m_vecMoveSpeed * CTimer::GetTimeStep();
            
#if FIX_BUGS // FPS Fix
            float distBase = (proj->m_vecMoveSpeed * CTimer::GetDefaultTimeStep()).Magnitude();
            int32 numParticles = std::max(1, static_cast<int32>(distBase * CTimer::GetTimeStepFix()));
            float fNumParticles = static_cast<float>(numParticles);
#else
            float stepDist = moveVec.Magnitude();
            int32 numParticles = std::max(1, static_cast<int32>(stepDist));
            float fNumParticles = static_cast<float>(numParticles);
#endif
            for (int32 p = 0; p < numParticles; ++p) {
                float randCol = CGeneral::GetRandomNumberInRange(0.25f, 0.50f);
                particleData.m_Color = { randCol, randCol, randCol, 0.3f };
                particleData.m_fLife = CGeneral::GetRandomNumberInRange(0.08f, 0.12f);

                float frac = 1.0f - static_cast<float>(p) / fNumParticles;
                CVector pos = proj->GetPosition() - moveVec * frac;

                CVector randDir(
                    CGeneral::GetRandomNumberInRange(-1.0f, 1.0f),
                    CGeneral::GetRandomNumberInRange(-1.0f, 1.0f),
                    CGeneral::GetRandomNumberInRange(-1.0f, 1.0f)
                );
                randDir.Normalise();

                CVector moveDir = proj->m_vecMoveSpeed;
                moveDir.Normalise();

                CVector vel = CrossProduct(moveDir, randDir) * 1.5f;
                g_fx.m_SmokeHuge->AddParticle(pos, vel, 0.0f, particleData, -1.0f, 1.2f, 0.6f, false);
            }
        }

        // Expiración por tiempo
        if (info.m_fTimeExpires && CTimer::GetTimeInMS() > info.m_fTimeExpires) {
            if (info.m_eProjectileType == WEAPON_REMOTE_SATCHEL_CHARGE) {
                if (info.m_pEntProjectileOwner && info.m_pEntProjectileOwner->GetIsTypePed() && info.m_pEntProjectileOwner->AsPed()->IsPlayer()) {
                    auto* player = info.m_pEntProjectileOwner->AsPed();
                    auto detonatorSlot = player->GetWeaponSlot(WEAPON_DETONATOR);
                    if (player->GetWeapon(eWeaponType(detonatorSlot)).m_Type != WEAPON_DETONATOR || player->GetWeapon(eWeaponType(detonatorSlot)).m_TotalAmmo == 0) {
                        info.m_fTimeExpires = 0;
                    }
                }
                info.OldCoors = proj->GetPosition();
                continue;
            }

            RemoveProjectile(&info, proj);
            continue;
        }

        // Comportamiento por tipo de proyectil
        if (info.m_eProjectileType == WEAPON_ROCKET) {
            CVector accel = proj->GetMatrix().GetForward() * (CTimer::GetTimeStep() * 0.008f);
            proj->m_vecMoveSpeed += accel;
            float speed = proj->m_vecMoveSpeed.Magnitude();
            if (speed > 9.9f) {
                proj->m_vecMoveSpeed *= (9.9f / speed);
            }
        } else if (info.m_eProjectileType == WEAPON_FLARE) {
            proj->m_bUsesCollision = false;
            CWorld::pIgnoreEntity = info.m_pEntProjectileOwner;
            bool isClear = CWorld::GetIsLineOfSightClear(info.OldCoors, proj->GetPosition(), true, true, true, true, false, false, false);
            proj->m_bUsesCollision = true;
            CWorld::pIgnoreEntity = nullptr;

            if (!isClear) {
                proj->m_vecMoveSpeed.Set(0.0f, 0.0f, 0.0f);
                proj->SetPosition(info.OldCoors);
            }
            info.OldCoors = proj->GetPosition();
            continue;
        } else if (info.m_eProjectileType == WEAPON_MOLOTOV || info.m_eProjectileType == WEAPON_FREEFALL_BOMB) {
            CWorld::pIgnoreEntity = info.m_pEntProjectileOwner;
            proj->m_bUsesCollision = false;

            bool bOwnerClose = false;
            if (info.m_pEntProjectileOwner) {
                float distSqr = (info.OldCoors - info.m_pEntProjectileOwner->GetPosition()).SquaredMagnitude();
                if (distSqr < 2.0f) {
                    bOwnerClose = true;
                }
            }

            bool bExplode = false;
            if (!info.m_pEntProjectileOwner || !bOwnerClose) {
                if (proj->m_nPhysicalFlags.bCollidedThisFrame || !CWorld::GetIsLineOfSightClear(info.OldCoors, proj->GetPosition(), true, true, true, true, false, false, false)) {
                    bExplode = true;
                }
            }
            CWorld::pIgnoreEntity = nullptr;
            proj->m_bUsesCollision = true;

            if (bExplode) {
                RemoveProjectile(&info, proj);
                continue;
            }
            info.OldCoors = proj->GetPosition();
            continue;
        } else if (info.m_eProjectileType == WEAPON_REMOTE_SATCHEL_CHARGE) {
            if (proj->m_fDamageIntensity > 0.0f) {
                if (proj->m_pDamageEntity && !proj->m_pAttachedTo) {
                    proj->AttachEntityToEntity(proj->m_pDamageEntity->AsPhysical(), nullptr, nullptr);
                    proj->m_bUsesCollision = false;
                }
            }
            info.OldCoors = proj->GetPosition();
            continue;
        } else if (info.m_eProjectileType == WEAPON_ROCKET_HS) {
            if (info.m_pTargetEntity) {
                if (info.m_pTargetEntity == FindPlayerVehicle(-1, false)) {
                    AudioEngine.ReportFrontendAudioEvent(AE_MISSILE_LOCK, 0.0f, 1.0f);
                }

                CVector forward = proj->GetMatrix().GetForward();
                CVector pos = proj->GetPosition() + forward;
                float bestScore = CWeapon::EvaluateTargetForHeatSeekingMissile(info.m_pTargetEntity, pos, forward, 1.2f, true, nullptr);

                CEntity* bestTarget = nullptr;
                float highestScore = 0.0f;
                for (auto f = 0u; f < MAX_PROJECTILES; ++f) {
                    if (gaProjectileInfo[f].m_eProjectileType == WEAPON_FLARE && gaProjectileInfo[f].m_bProjectileActive) {
                        auto* flare = ms_apProjectile[f];
                        float score = CWeapon::EvaluateTargetForHeatSeekingMissile(flare, pos, forward, 1.2f, true, nullptr);
                        if (score >= highestScore) {
                            highestScore = score;
                            bestTarget = flare;
                        }
                    }
                }

                if (!bestTarget || highestScore <= bestScore) {
                    bestTarget = info.m_pTargetEntity;
                }

                bool bSuperMissile = false;
                if (bestTarget->GetIsTypeVehicle()) {
                    auto* bestVeh = bestTarget->AsVehicle();
                    auto* owner = info.m_pEntProjectileOwner;
                    if ((owner == FindPlayerPed(-1) || owner == FindPlayerVehicle(-1, false))
                        && bestVeh->GetBaseVehicleType() == VEHICLE_TYPE_PLANE) {
                        bSuperMissile = true;
                    }
                }

                CVector futureProjPos = bSuperMissile ? proj->GetPosition() : proj->GetPosition() + proj->m_vecMoveSpeed * 100.0f;
                float distToTarget = (proj->GetPosition() - bestTarget->GetPosition()).Magnitude();
                float predictTime = std::min(distToTarget, bSuperMissile ? 1.5f : 50.0f);

                CVector targetFuturePos = bestTarget->GetPosition() + bestTarget->AsPhysical()->m_vecMoveSpeed * predictTime;
                CVector toTarget = targetFuturePos - futureProjPos;

                CVector currentMoveDir = proj->m_vecMoveSpeed;
                currentMoveDir.Normalise();

                float dot = DotProduct(toTarget, currentMoveDir);
                if (dot < 0.0f) {
                    toTarget -= currentMoveDir * dot;
                }
                toTarget.Normalise();

                float turnRate = 0.009f;
                if (info.m_pEntProjectileOwner == FindPlayerPed(-1) || info.m_pEntProjectileOwner == FindPlayerVehicle(-1, false)) {
                    turnRate = 0.0117f;
                }
                if (bestTarget->AsPhysical()->m_vecMoveSpeed.Magnitude() > 0.8f) {
                    turnRate *= 1.2f;
                }

                float friction = 1.0f;
                if (bSuperMissile) {
                    friction = std::pow(0.95f, CTimer::GetTimeStepFix());
                    turnRate = 0.15f;
                }

                proj->m_vecMoveSpeed *= friction;
                proj->m_vecMoveSpeed += toTarget * (turnRate * CTimer::GetTimeStep());

                float speed = proj->m_vecMoveSpeed.Magnitude();
                if (speed > 9.9f) {
                    proj->m_vecMoveSpeed *= (9.9f / speed);
                }
                proj->GetMatrix().GetForward() = currentMoveDir;
            }
        } else {
            info.OldCoors = proj->GetPosition();
            continue;
        }

        // Comprobación de colisiones / trayectorias para proyectiles tipo ROCKET y ROCKET_HS
        bool bCollided = false;
        if (!proj->m_nPhysicalFlags.bCollidedThisFrame) {
            CWorld::pIgnoreEntity = info.m_pEntProjectileOwner;
            proj->m_bUsesCollision = false;
            bool isClear = CWorld::GetIsLineOfSightClear(info.OldCoors, proj->GetPosition(), true, true, true, true, false, false, false);
            CWorld::pIgnoreEntity = nullptr;
            proj->m_bUsesCollision = true;
            proj->m_pEntityIgnoredCollision = info.m_pEntProjectileOwner;

            if (!isClear) {
                bCollided = true;
            }
        }

        if (!bCollided && proj->m_nNumEntitiesCollided > 0) {
            auto* colEntity = proj->m_apCollidedEntities[0];
            if (colEntity && colEntity != info.m_pEntProjectileOwner && colEntity->m_nModelIndex != MODEL_FLARE) {
                bCollided = true;
            }
        }

        if (bCollided) {
            RemoveProjectile(&info, proj);
            continue;
        }

        info.OldCoors = proj->GetPosition();
    }
}

// 0x739860
bool CProjectileInfo::IsProjectileInRange(float x1, float x2, float y1, float y2, float z1, float z2, bool bDestroy) {
    const CBox bb{
        CVector{ x1, y1, z1 },
        CVector{ x2, y2, z2 }
    };
    bool found = false;
    for (auto&& [info, proj] : rngv::zip(gaProjectileInfo, ms_apProjectile)) {
        if (!info.m_bProjectileActive) {
            continue;
        }

        if (!IsWeaponTypeProjectile(info.m_eProjectileType)) {
            continue;
        }

        if (!bb.IsPointInside(proj->GetPosition())) {
            continue;
        }

        found = true;
        if (bDestroy) {
            info.m_bProjectileActive = false;
            info.RemoveFXSystem(false);
            CRadar::ClearBlipForEntity(eBlipType::BLIP_OBJECT, GetObjectPool()->GetRef(proj));
            CWorld::Remove(proj);
            delete proj;
        }
    }
    return found;
}

// 0x7399B0
void CProjectileInfo::RemoveAllProjectiles() {
    for (auto i = 0u; i < MAX_PROJECTILES; ++i) {
        auto& info = gaProjectileInfo[i];
        if (info.m_bProjectileActive) {
            info.m_bProjectileActive = false;
            if (info.m_fxSystem) {
                g_fxMan.DestroyFxSystem(info.m_fxSystem);
                info.m_fxSystem = nullptr;
            }
            auto* proj = ms_apProjectile[i];
            CRadar::ClearBlipForEntity(eBlipType::BLIP_OBJECT, GetObjectPool()->GetRef(proj));
            CWorld::Remove(proj);
            delete proj;
        }
    }
}

// 0x739A40
bool CProjectileInfo::RemoveIfThisIsAProjectile(CObject* object) {
    for (auto i = 0u; i < MAX_PROJECTILES; ++i) {
        if (ms_apProjectile[i] == object && gaProjectileInfo[i].m_bProjectileActive) {
            auto& info = gaProjectileInfo[i];
            info.m_bProjectileActive = false;
            if (info.m_fxSystem) {
                info.m_fxSystem->Kill();
                info.m_fxSystem = nullptr;
            }
            CRadar::ClearBlipForEntity(eBlipType::BLIP_OBJECT, GetObjectPool()->GetRef(object));
            CWorld::Remove(object);
            delete object;
            ms_apProjectile[i] = nullptr;
            return true;
        }
    }
    return false;
}

// 0x737B80
void CProjectileInfo::RemoveFXSystem(bool bInstantly) {
    if (m_fxSystem) {
        if (bInstantly) {
            g_fxMan.DestroyFxSystem(m_fxSystem);
        } else {
            m_fxSystem->Kill();
        }
        m_fxSystem = nullptr;
    }
}
