#include "StdInc.h"

#include <extensions/utility.hpp>
#include <reversiblebugfixes/Bugs.hpp>

#include "AECollisionAudioEntity.h"
#include "AEAudioHardware.h"
#include "AEAudioUtility.h"

#include "AECollisionAudioEntity.CollisionLookup.h"

void CAECollisionAudioEntity::InjectHooks() {
    RH_ScopedVirtualClass(CAECollisionAudioEntity, 0x862E64, 1);
    RH_ScopedCategory("Audio/Entities");

    RH_ScopedInstall(Initialise, 0x5B9BD0);
    RH_ScopedInstall(InitialisePostLoading, 0x4DA050);
    RH_ScopedInstall(AddCollisionSoundToList, 0x4DAAC0);
    RH_ScopedInstall(Reset, 0x4DA320);
    RH_ScopedInstall(ReportGlassCollisionEvent, 0x4DA070);
    RH_ScopedInstall(UpdateLoopingCollisionSound, 0x4DA540);
    RH_ScopedInstall(GetCollisionSoundStatus, 0x4DA830);
    RH_ScopedInstall(ReportObjectDestruction, 0x4DAB60);
    RH_ScopedInstall(PlayOneShotCollisionSound, 0x4DB150);
    RH_ScopedInstall(PlayLoopingCollisionSound, 0x4DB450);
    RH_ScopedInstall(PlayBulletHitCollisionSound, 0x4DB7C0);
    RH_ScopedInstall(ReportCollision, 0x4DBA10);
    RH_ScopedInstall(ReportBulletHit, 0x4DBDF0);
    RH_ScopedInstall(Service, 0x4DA2C0);
    RH_ScopedInstall(ChooseCollisionSoundID, 0x4DAA50);
    RH_ScopedOverloadedInstall(ReportWaterSplash, "at-position", 0x4DA190, void(CAECollisionAudioEntity::*)(CVector, float));
    RH_ScopedOverloadedInstall(ReportWaterSplash, "for-physical", 0x4DAE40, void(CAECollisionAudioEntity::*)(CPhysical*, float, bool));
}

// 0x5B9BD0
void CAECollisionAudioEntity::Initialise() {
    *this = {};
}

// 0x4DA050
void CAECollisionAudioEntity::InitialisePostLoading() {
    AEAudioHardware.LoadSoundBank(SND_BANK_GENRL_COLLISIONS, SND_BANK_SLOT_COLLISIONS);
    AEAudioHardware.LoadSoundBank(SND_BANK_GENRL_BULLET_HITS, SND_BANK_SLOT_BULLET_HITS);
}

// 0x4DA320
void CAECollisionAudioEntity::Reset() {
    for (auto& entry : m_CollisionSoundList) {
        if (entry.Status != COLLISION_SOUND_LOOPING) {
            continue;
        }

        if (entry.Sound) {
            entry.Sound->StopSoundAndForget();
        }

        entry = {};
    }
}

// 0x4DAAC0
/*
void CAECollisionAudioEntity::AddCollisionSoundToList(
    CEntity* entityA,
    CEntity* entityB,
    eSurfaceType surfaceA,
    eSurfaceType surfaceB,
    CAESound* sound,
    eCollisionSoundStatus status
) {
    // Find an entry with no sound.
    const auto e = rng::find_if_not(m_CollisionSoundList, &tCollisionSound::Sound);
    if (e == m_CollisionSoundList.end()) {
        NOTSA_LOG_WARN("Collision sound list is full");
        return;
    }

    e->EntityA  = entityA;
    e->EntityB  = entityB;

    e->SurfaceA = surfaceA;
    e->SurfaceB = surfaceB;

    e->Sound          = sound;
    e->Status         = status;
    e->LoopStopTimeMs = status == COLLISION_SOUND_LOOPING
        ? CTimer::GetTimeInMS() + 100
        : 0;

    m_NumActiveCollisionSounds++;
}*/
// 0x4DAAC0
void CAECollisionAudioEntity::AddCollisionSoundToList(
    CEntity*              entityA,
    CEntity*              entityB,
    eSurfaceType          surfaceA,
    eSurfaceType          surfaceB,
    CAESound*             sound,
    eCollisionSoundStatus status
) {
    if (!sound) {
        return;
    }

    // Buscar el primer slot disponible (0 a 299)
    int32 freeIndex = -1;
    for (int32 i = 0; i < 300; ++i) {
        if (!m_CollisionSoundList[i].Sound || m_CollisionSoundList[i].Status == COLLISION_SOUND_INACTIVE) {
            freeIndex = i;
            break;
        }
    }

    // Si los 300 slots están ocupados, salir de inmediato sin tocar memoria
    if (freeIndex < 0 || freeIndex >= 300) {
        return;
    }

    tCollisionSound& entry = m_CollisionSoundList[freeIndex];
    entry.EntityA          = entityA;
    entry.EntityB          = entityB;
    entry.SurfaceA         = surfaceA;
    entry.SurfaceB         = surfaceB;
    entry.Sound            = sound;
    entry.Status           = status;

    if (status == COLLISION_SOUND_LOOPING) {
        entry.LoopStopTimeMs = CTimer::GetTimeInMS() + 100;
    } else {
        entry.LoopStopTimeMs = CTimer::GetTimeInMS() + 1'000;
    }

    ++m_NumActiveCollisionSounds;
}

// 0x4DA830
eCollisionSoundStatus CAECollisionAudioEntity::GetCollisionSoundStatus(CEntity* entityA, CEntity* entityB, eSurfaceType surfaceA, eSurfaceType surfaceB, int32& outIndex) {
    auto status = COLLISION_SOUND_INACTIVE;
    for (auto&& [i, v] : rngv::enumerate(m_CollisionSoundList)) {
        if (v.EntityA == entityA && v.EntityB == entityB || v.EntityA == entityB && v.EntityB == entityA) {
            if ((status = v.Status) == COLLISION_SOUND_LOOPING) {
                outIndex = (int32)(i);
                return v.Status;
            }
        }
    }
    outIndex = -1;
    return status;
}

// 0x4DB150
void CAECollisionAudioEntity::PlayOneShotCollisionSound(CEntity* pEntityA, CEntity* pEntityB, uint8 SurfaceA, uint8 SurfaceB, float fImpulseMagnitude, const CVector& vPosition) {
    const auto ProcessSound = [&](CEntity* eA, CEntity* eB, eSurfaceType sA, eSurfaceType sB) {
        if (sB >= TOTAL_NUM_COLLISION_SURFACE_TYPES) {
            return false;
        }
        if (sB == AE_SURFACE_TYPE_POOL_BALL && sA != AE_SURFACE_TYPE_POOL_BALL) {
            return false;
        }
        const auto isMissionScriptSurface = notsa::contains({ AE_SURFACE_TYPE_POOL_BALL, AE_SURFACE_TYPE_BASKETBALL, AE_SURFACE_TYPE_PUNCHBAG }, sB); // 0x4DB23E
        const auto slot = isMissionScriptSurface
            ? SND_BANK_SLOT_MISSION4
            : SND_BANK_SLOT_COLLISIONS;
        if (isMissionScriptSurface) {
            const auto bank = notsa::find_value(notsa::make_mapping<eSurfaceType, eSoundBank>({
                { AE_SURFACE_TYPE_POOL_BALL, SND_BANK_SCRIPT_POOL_MINIGAME },
                { AE_SURFACE_TYPE_BASKETBALL, SND_BANK_SCRIPT_BASKETBALL },
                { AE_SURFACE_TYPE_PUNCHBAG, SND_BANK_SCRIPT_GYM },
            }), sB);
            if (!AEAudioHardware.IsSoundBankLoaded(bank, slot)) {
                return false;
            }
        }
        const auto soundID = ChooseCollisionSoundID(sB); // 0x4DB280
        if (soundID == -1) {
            return true;
        }
        auto offset = ((float)(gCollisionLookup[sA].ParamD) * fImpulseMagnitude) / 100.f;
        if (sB == AE_SURFACE_TYPE_BMX && sA == SURFACE_PED) { // 0x4DB2A6
            offset *= 10.f;
        }
        offset *= 500.f;
        const auto startAt = 100 - std::min(100, (int32)(std::floor(offset))); // 0x4DB2FB
        if (startAt >= 100) {
            return true;
        }
        const auto PlaySound = [&](float volume, int32 startAt) {
            volume += GetDefaultVolume(AE_GENERAL_COLLISION) - 3.f;
            if (volume <= -100.f) { // 0x4DB37E
                return;
            }
            auto* const sound = AESoundManager.PlaySound({
                .BankSlotID        = slot,
                .SoundID           = soundID,
                .AudioEntity       = this,
                .Pos               = vPosition,
                .Volume            = volume,
                .RollOffFactor     = 2.f,
                .Flags             = SOUND_ROLLED_OFF | SOUND_START_PERCENTAGE | SOUND_REQUEST_UPDATES,
                .FrequencyVariance = 0.02f
            });
            m_CollisionSoundIDHistory[sB] = soundID;
            if (sound) { // 0x4DB3E7
                AddCollisionSoundToList(pEntityA, pEntityB, sA, sB, sound, COLLISION_SOUND_ONE_SHOT);
            }
        };
        const auto maxStartAt = gCollisionLookup[sB].MaxStartAtPercentage;
        if (startAt > maxStartAt) { // 0x4DB32D
            PlaySound(CAEAudioUtility::AudioLog10((float)(100 - startAt) / (float)(100 - maxStartAt)) * 20.f, maxStartAt);
        } else {
            PlaySound(0.f, startAt);
        }
        return true;
    };

    if (!ProcessSound(pEntityA, pEntityB, static_cast<eSurfaceType>(SurfaceA), static_cast<eSurfaceType>(SurfaceB))) {
        return;
    }
    if (!ProcessSound(pEntityB, pEntityA, static_cast<eSurfaceType>(SurfaceB), static_cast<eSurfaceType>(SurfaceA))) {
        return;
    }
}

// based on `PlayLoopingCollisionSound` and `UpdateLoopingCollisionSound`
std::pair<float, float> CAECollisionAudioEntity::GetLoopingCollisionSoundVolumeAndSpeed(CEntity* entityA, CEntity* entityB, eSurfaceType surfaceA, eSurfaceType surfaceB, bool isForceLooping) {
    const auto CalculateVolumeAtSpeed = [&](float speed) {
        return GetDefaultVolume(AE_GENERAL_COLLISION) + CAEAudioUtility::AudioLog10(std::min(speed / 0.75f, 1.f) * 20.f);
    };
    const auto GetVolumeAndSpeedForPhysical = [&](CVector velocity, CVector turn) -> std::pair<float, float> {
        const auto mag = std::sqrt(std::max(turn.SquaredMagnitude(), velocity.SquaredMagnitude()));
        if (mag == 0.f) {
            return { -100.f, 0.f };
        } else {
            auto speed = std::min(std::sqrt(mag * ((gCollisionLookup[surfaceA].ParamD * gCollisionLookup[surfaceB].ParamD) / 10000.f)) * 3.f, 0.3f);
            if (isForceLooping) {
                speed /= 6.f;
            }
            return { CalculateVolumeAtSpeed(speed), speed };
        }
    };
    
    if (surfaceA == SURFACE_CAR && surfaceB == SURFACE_CAR) {
        const auto vehA = entityA->AsVehicle(),
                   vehB = entityB->AsVehicle();
        return GetVolumeAndSpeedForPhysical(vehA->GetMoveSpeed() - vehB->GetMoveSpeed(), vehA->GetTurnSpeed() - vehB->GetTurnSpeed());
    }

    if ((surfaceB != SURFACE_PED && surfaceA == SURFACE_CAR) || (surfaceA != SURFACE_PED && surfaceB == SURFACE_CAR)) {
        assert(entityA->GetIsTypePhysical());

        const auto* const physicalA = entityA->AsPhysical();
        return GetVolumeAndSpeedForPhysical(physicalA->GetMoveSpeed(), physicalA->GetTurnSpeed());
    }

    return { -100.f, 0.f };
}

// 0x4DB450
/*
void CAECollisionAudioEntity::PlayLoopingCollisionSound(CEntity* entityA, CEntity* entityB, eSurfaceType surfaceA, eSurfaceType surfaceB, float force, const CVector& pos, bool isForceLooping) {
    const auto [volume, speed] = GetLoopingCollisionSoundVolumeAndSpeed(entityA, entityB, surfaceA, surfaceB, isForceLooping);

    const auto PlaySound = [&](eSoundID soundID) {
        if (auto* const sound = AESoundManager.PlaySound({
            .BankSlotID    = SND_BANK_SLOT_COLLISIONS,
            .SoundID       = soundID,
            .AudioEntity   = this,
            .Pos           = pos,
            .Volume        = volume,
            .RollOffFactor = 2.f,
            .Speed         = std::max(speed, 0.75f) * 0.8f,
            .Flags         = SOUND_REQUEST_UPDATES,
            })
        ) {
            AddCollisionSoundToList(entityA, entityB, surfaceA, surfaceB, sound, COLLISION_SOUND_LOOPING);
        }
    };

    // FIX(#1406):
    // This fixes an OOB access in the original code,
    // Namely, originally for BMX `AE_SURFACE_TYPE_BMX` was passed in,
    // but it's not an actual surface type, but a special case for sound selection.

    // If both surfaces are valid we preserve the original logic...
    if (!notsa::bugfixes::CAECollisionAudioEntity_PlayLoopingCollisionSound_InvalidSurfaceType || (surfaceA < TOTAL_NUM_SURFACE_TYPES && surfaceB < TOTAL_NUM_SURFACE_TYPES)) {
        if (g_surfaceInfos.IsAudioGrass(surfaceA) || g_surfaceInfos.IsAudioGrass(surfaceB)) {
            PlaySound(SND_GENRL_COLLISIONS_GRASS_SKID);
        } else if (g_surfaceInfos.IsAudioWater(surfaceA) || g_surfaceInfos.IsAudioWater(surfaceB)) {
            PlaySound(SND_GENRL_COLLISIONS_WATER_LOOP);
        } else if (g_surfaceInfos.IsAudioMetal(surfaceA) || g_surfaceInfos.IsAudioMetal(surfaceB)) {
            PlaySound(SND_GENRL_COLLISIONS_METAL_SCRAPE);
        } else {
            PlaySound(SND_GENRL_COLLISIONS_GRAVEL_SKID);
        }
    } else {
        const auto GetSoundIDForSurface = [&](eSurfaceType surface) -> std::optional<eSoundID> { // 0x4DB6AF
            assert(surface < TOTAL_NUM_COLLISION_SURFACE_TYPES && "Expected `AE_SURFACE_TYPE_*` - Any other values are a bug");
            if (surface < TOTAL_NUM_SURFACE_TYPES) {
                if (g_surfaceInfos.IsAudioGrass(surface)) {
                    return SND_GENRL_COLLISIONS_GRASS_SKID;
                } else if (g_surfaceInfos.IsAudioWater(surface)) {
                    return SND_GENRL_COLLISIONS_WATER_LOOP;
                } else if (g_surfaceInfos.IsAudioMetal(surface)) {
                    return SND_GENRL_COLLISIONS_METAL_SCRAPE;
                }
            }
            return std::nullopt;
        };
        if (const auto soundOfSurfaceA = GetSoundIDForSurface(surfaceA)) {
            PlaySound(*soundOfSurfaceA);
        } else if (const auto soundOfSurfaceB = GetSoundIDForSurface(surfaceB)) {
            PlaySound(*soundOfSurfaceB);
        } else {
            PlaySound(SND_GENRL_COLLISIONS_GRAVEL_SKID);
        }
    }
}*/
// 0x4DB450
void CAECollisionAudioEntity::PlayLoopingCollisionSound(
    CEntity*       pEntityA,
    CEntity*       pEntityB,
    uint8          SurfaceA,
    uint8          SurfaceB,
    float          fImpulseMagnitude,
    const CVector& vPosition,
    bool           bForceLooping
) {
    float fVolume = -100.0f;
    float fSpeed  = 0.0f;

    CVector vecDiff(0.0f, 0.0f, 0.0f);

    CPhysical* physA = (pEntityA && pEntityA->GetIsTypePhysical()) ? pEntityA->AsPhysical() : nullptr;
    CPhysical* physB = (pEntityB && pEntityB->GetIsTypePhysical()) ? pEntityB->AsPhysical() : nullptr;

    if (SurfaceA == SURFACE_CAR) {
        if (SurfaceB == SURFACE_CAR && physA && physB) {
            CVector vecMoveDiff = physA->m_vecMoveSpeed - physB->m_vecMoveSpeed;
            CVector vecTurnDiff = physA->m_vecTurnSpeed - physB->m_vecTurnSpeed;

            if (vecMoveDiff.SquaredMagnitude() > vecTurnDiff.SquaredMagnitude()) {
                vecDiff = vecMoveDiff;
            } else {
                vecDiff = vecTurnDiff;
            }
        } else if (SurfaceB != SURFACE_PED && physA) {
            if (physA->m_vecMoveSpeed.SquaredMagnitude() > physA->m_vecTurnSpeed.SquaredMagnitude()) {
                vecDiff = physA->m_vecMoveSpeed;
            } else {
                vecDiff = physA->m_vecTurnSpeed;
            }
        }
    } else if (SurfaceB == SURFACE_CAR && SurfaceA != SURFACE_PED && physA) {
        if (physA->m_vecMoveSpeed.SquaredMagnitude() > physA->m_vecTurnSpeed.SquaredMagnitude()) {
            vecDiff = physA->m_vecMoveSpeed;
        } else {
            vecDiff = physA->m_vecTurnSpeed;
        }
    }

    float fVelocity = vecDiff.Magnitude();
    if (fVelocity != 0.0f) {
        float fCalculatedSpeed = std::sqrt(fVelocity * (gCollisionLookup[SurfaceA].ParamD * gCollisionLookup[SurfaceB].ParamD) * 0.0001f) * 3.0f;
        if (fCalculatedSpeed >= 0.3f) {
            fCalculatedSpeed = 0.3f;
        }

        if (bForceLooping) {
            fSpeed = fCalculatedSpeed * (1.0f / 6.0f);
        } else {
            fSpeed = fCalculatedSpeed;
        }

        float fLogRatio = 1.3333334f * fSpeed;
        if (fLogRatio >= 1.0f) {
            fLogRatio = 1.0f;
        }

        fVolume = std::log10(fLogRatio) * 20.0f + GetDefaultVolume(AE_GENERAL_COLLISION);
    }

    float fSoundSpeed = fSpeed;
    if (fSoundSpeed <= 0.75f) {
        fSoundSpeed = 0.75f;
    }
    fSoundSpeed *= 0.8f;

    eSoundID nSoundId;
    if (g_surfaceInfos.IsAudioGrass(SurfaceA) || g_surfaceInfos.IsAudioGrass(SurfaceB)) {
        nSoundId = SND_GENRL_COLLISIONS_GRASS_SKID;
    } else if (g_surfaceInfos.IsAudioWater(SurfaceA) || g_surfaceInfos.IsAudioWater(SurfaceB)) {
        nSoundId = SND_GENRL_COLLISIONS_WATER_LOOP;
    } else if (g_surfaceInfos.IsAudioMetal(SurfaceA) || g_surfaceInfos.IsAudioMetal(SurfaceB)) {
        nSoundId = SND_GENRL_COLLISIONS_METAL_SCRAPE;
    } else {
        nSoundId = SND_GENRL_COLLISIONS_GRAVEL_SKID;
    }

    CAESound sound;
    sound.Initialise(
        SND_BANK_SLOT_COLLISIONS,
        nSoundId,
        this,
        vPosition,
        fVolume,
        2.0f,
        fSoundSpeed,
        1.0f,
        0,
        SOUND_REQUEST_UPDATES,
        0.0f,
        0
    );

    if (physA) {
        sound.RegisterWithPhysicalEntity(physA);
    } else if (physB) {
        sound.RegisterWithPhysicalEntity(physB);
    }

    CAESound* pSound = AESoundManager.RequestNewSound(&sound);
    if (pSound) {
        AddCollisionSoundToList(pEntityA, pEntityB, static_cast<eSurfaceType>(SurfaceA), static_cast<eSurfaceType>(SurfaceB), pSound, COLLISION_SOUND_LOOPING);
    }
}

// 0x4DA540
void CAECollisionAudioEntity::UpdateLoopingCollisionSound(
    CAESound*      sound,
    CEntity*       entityA,
    CEntity*       entityB,
    eSurfaceType   surfaceA,
    eSurfaceType   surfaceB,
    float          impulseForce,
    const CVector& pos,
    bool           isForceLooping
) {
    const auto [volume, speed] = GetLoopingCollisionSoundVolumeAndSpeed(entityA, entityB, surfaceA, surfaceB, isForceLooping);
    sound->SetSpeed(notsa::step_to(std::max(0.75f, speed), sound->GetSpeed(), 0.1f));
    sound->SetVolume(notsa::step_to(volume, sound->GetVolume(), 1.0f));
    sound->SetPosition(pos);
}

// 0x4DB7C0
void CAECollisionAudioEntity::PlayBulletHitCollisionSound(uint8 HitSurface, const CVector& vPosition, float fAngleOfIncidence) {
    if (HitSurface >= 195) {
        return;
    }

    float volume      = GetDefaultVolume(AE_BULLET_HIT);
    float maxDistance = 1.5f;
    int16 soundId     = -1;

    if (HitSurface == SURFACE_PED) {
        do {
            soundId = static_cast<int16>(CAEAudioUtility::GetRandomNumberInRange(7, 9));
        } while (soundId == m_LastBulletHitSoundID);
    } else if (g_surfaceInfos.IsAudioWater(HitSurface)) {
        do {
            soundId = static_cast<int16>(CAEAudioUtility::GetRandomNumberInRange(16, 18));
        } while (soundId == m_LastBulletHitSoundID);

        maxDistance = 2.0f;
        volume += 6.0f;
    } else if (g_surfaceInfos.IsAudioWood(HitSurface)) {
        do {
            soundId = static_cast<int16>(CAEAudioUtility::GetRandomNumberInRange(19, 21));
        } while (soundId == m_LastBulletHitSoundID);
    } else if (g_surfaceInfos.IsAudioMetal(HitSurface)) {
        const float ricochetChance = (90.0f - fAngleOfIncidence) * 0.0055555557f;
        if (CAEAudioUtility::ResolveProbability(ricochetChance)) {
            do {
                soundId = static_cast<int16>(CAEAudioUtility::GetRandomNumberInRange(10, 12));
            } while (soundId == m_LastBulletHitSoundID);
        } else {
            do {
                soundId = static_cast<int16>(CAEAudioUtility::GetRandomNumberInRange(4, 6));
            } while (soundId == m_LastBulletHitSoundID);
        }
    } else if (g_surfaceInfos.IsAudioConcrete(HitSurface)
               || g_surfaceInfos.IsAudioGravel(HitSurface)
               || g_surfaceInfos.IsAudioTile(HitSurface)) {
        do {
            soundId = static_cast<int16>(CAEAudioUtility::GetRandomNumberInRange(13, 15));
        } while (soundId == m_LastBulletHitSoundID);
    } else {
        do {
            soundId = static_cast<int16>(CAEAudioUtility::GetRandomNumberInRange(1, 3));
        } while (soundId == m_LastBulletHitSoundID);
    }

    if (soundId >= 0) {
        CAESound sound;
        sound.Initialise(
            SND_BANK_SLOT_BULLET_HITS,
            static_cast<eSoundID>(soundId),
            this,
            vPosition,
            volume,
            maxDistance,
            1.0f,
            1.0f,
            0,
            0,
            0.02f,
            0
        );

        AESoundManager.RequestNewSound(&sound);
        m_LastBulletHitSoundID = soundId;
    }
}

// 0x4DA070
void CAECollisionAudioEntity::ReportGlassCollisionEvent(eAudioEvents event, const CVector& posn, uint32 time) {
    const auto PlayCollisionSound = [&](eSoundID soundID, float speed) {
        if (time) {
            AESoundManager.PlaySound({
                .BankSlotID     = SND_BANK_SLOT_COLLISIONS,
                .SoundID        = soundID,
                .AudioEntity    = this,
                .Pos            = posn,
                .Volume         = GetDefaultVolume(event),
                .RollOffFactor  = 1.5f,
                .Speed          = 0.f,
                .Flags          = SOUND_REQUEST_UPDATES,
                .EventID        = event,
                .ClientVariable = (float)(time + CTimer::GetTimeInMS()),
            });
        } else {
            AESoundManager.PlaySound({
                .BankSlotID     = SND_BANK_SLOT_COLLISIONS,
                .SoundID        = soundID,
                .AudioEntity    = this,
                .Pos            = posn,
                .Volume         = GetDefaultVolume(event),
                .RollOffFactor  = 1.5f,
                .Speed          = speed,
            });
        }
    };
    switch (event) {
    case AE_GLASS_HIT:             PlayCollisionSound(51, 1.f);                                                break;
    case AE_GLASS_CRACK:           PlayCollisionSound(68, 1.f);                                                break;
    case AE_GLASS_BREAK_SLOW:      PlayCollisionSound(56, 0.75f);                                              break;
    case AE_GLASS_BREAK_FAST:      PlayCollisionSound(56, 1.f);                                                break;
    case AE_GLASS_HIT_GROUND:      PlayCollisionSound(CAEAudioUtility::GetRandomNumberInRange(15, 18), 1.f);   break;
    case AE_GLASS_HIT_GROUND_SLOW: PlayCollisionSound(CAEAudioUtility::GetRandomNumberInRange(15, 18), 0.56f); break;
    default:                       NOTSA_UNREACHABLE();
    }
}

// 0x4DA190
void CAECollisionAudioEntity::ReportWaterSplash(CVector posn, float volume) {
    if (!AEAudioHardware.EnsureSoundBankIsLoaded(SND_BANK_GENRL_COLLISIONS, SND_BANK_SLOT_COLLISIONS, true)) {
        return;
    }
    AESoundManager.PlaySound({                        
        .BankSlotID    = SND_BANK_SLOT_COLLISIONS,
        .SoundID       = 67,
        .AudioEntity   = this,
        .Pos           = posn,
        .Volume        = GetDefaultVolume(AE_WATER_SPLASH) + volume,
        .RollOffFactor = 2.5f,
        .Speed         = 1.26f,
        .Flags         = SOUND_REQUEST_UPDATES,
        .EventID       = 1                               
    });
    AESoundManager.PlaySound({
        .BankSlotID     = SND_BANK_SLOT_COLLISIONS,
        .SoundID        = 66,
        .AudioEntity    = this,
        .Pos            = posn,
        .Volume         = GetDefaultVolume(AE_WATER_SPLASH) + volume,
        .RollOffFactor  = 2.5f,
        .Speed          = 0.f,
        .Flags          = SOUND_REQUEST_UPDATES,
        .EventID        = 2,
        .ClientVariable = (float)(CTimer::GetTimeInMS() + 166)
    });
}

// 0x4DAE40
void CAECollisionAudioEntity::ReportWaterSplash(CPhysical* physical, float volume, bool isForceSplash) {
    assert(physical->GetIsTypePhysical());

    if (!isForceSplash) {
        if (physical->GetMoveSpeed().z > -0.1f && volume == -100.f) {
            return;
        }
        if (rng::any_of(std::to_array({ 1, 2, 3 }), [&](int32 event) {
            return AESoundManager.AreSoundsOfThisEventPlayingForThisEntityAndPhysical(event, this, physical) != 0;
        })) {
            return;
        }
    }
    if (!AEAudioHardware.EnsureSoundBankIsLoaded(SND_BANK_GENRL_COLLISIONS, SND_BANK_SLOT_COLLISIONS, true)) { // 0x4DAED2
        return;
    }
    const auto PlaySound = [&](int32 eventID, float speed, float volumeOffset, uint32 interval) {
        if (volume <= -100.f) { // 0x4DAF90
            volume = std::max(CAEAudioUtility::AudioLog10(std::min(0.6f, physical->GetMoveSpeed().Magnitude())) * 20.f, -18.f);
        }
        volume += GetDefaultVolume(AE_WATER_SPLASH) + volumeOffset;

        AESoundManager.PlaySound({
            .BankSlotID         = SND_BANK_SLOT_COLLISIONS,
            .SoundID            = 67,
            .AudioEntity        = this,
            .Pos                = physical->GetPosition(),
            .Volume             = volume,
            .RollOffFactor      = 2.5f,
            .Speed              = speed,
            .Flags              = SOUND_REQUEST_UPDATES,
            .EventID            = 1,
        });
        AESoundManager.PlaySound({
            .BankSlotID         = SND_BANK_SLOT_COLLISIONS,
            .SoundID            = 66,
            .AudioEntity        = this,
            .Pos                = physical->GetPosition(),
            .Volume             = volume,
            .RollOffFactor      = 2.5f,
            .Speed              = speed,
            .Flags              = SOUND_REQUEST_UPDATES | SOUND_LIFESPAN_TIED_TO_PHYSICAL_ENTITY,
            .RegisterWithEntity = physical,
            .EventID            = eventID,
            .ClientVariable     = (float)(CTimer::GetTimeInMS() + interval),
        });
    };
    if (physical->GetIsTypePed()) { // 0x4DAF2B - Inverted
        if (isForceSplash && !physical->AsPed()->GetIntelligence()->GetTaskSwim()) {
            PlaySound(2, 1.26f, -6.f, 166);
        }
    } else if (physical->GetIsTypeVehicle()) { // 0x4DAF52
        PlaySound(3, 0.94f, 0.f, 248);
    } else { // 0x4DAF70
        PlaySound(2, 1.26f, -12.f, 166);
    }
}

// 0x4DAB60
void CAECollisionAudioEntity::ReportObjectDestruction(CEntity* entity) {
    if (!entity) {
        return;
    }

    if (entity->GetModelId() == ModelIndices::MI_GRASSHOUSE) { // 0x4DAB94 - Moved up here
        AESoundManager.PlaySound({ // 0x4DABEF
            .BankSlotID    = SND_BANK_SLOT_COLLISIONS,
            .SoundID       = (eSoundID)(56),
            .AudioEntity   = this,
            .Pos           = entity->GetPosition(),
            .Volume        = GetDefaultVolume(AE_GLASS_BREAK_SLOW),
            .RollOffFactor = 1.5f,
            .Speed         = 0.75f,
        });
        AESoundManager.PlaySound({ // 0x4DAC65
            .BankSlotID    = SND_BANK_SLOT_COLLISIONS,
            .SoundID       = (eSoundID)(CAEAudioUtility::GetRandomNumberInRange(15, 18)),
            .AudioEntity   = this,
            .Pos           = entity->GetPosition(),
            .Volume        = GetDefaultVolume(AE_GLASS_HIT_GROUND_SLOW),
            .RollOffFactor = 1.5f,
            .Speed         = 0.0f,
            .Flags         = SOUND_REQUEST_UPDATES,
            .EventID        = AE_GLASS_HIT_GROUND_SLOW,
            .ClientVariable = (float)(CTimer::GetTimeInMS() + 600),
        });
        return;
    }

    const auto PlaySoundForSurface = [&](eSurfaceType surface) {
        const auto PlaySound = [&](eSoundID soundID) {
            AESoundManager.PlaySound({
                .BankSlotID        = SND_BANK_SLOT_COLLISIONS,
                .SoundID           = soundID,
                .AudioEntity       = this,
                .Pos               = entity->GetPosition(),
                .Volume            = GetDefaultVolume(AE_GENERAL_COLLISION),
                .RollOffFactor     = 2.f,
                .FrequencyVariance = 0.0588f,
            });
        };
        switch (surface) {
        case SURFACE_WOOD_CRATES:
        case SURFACE_WOOD_BENCH:
        case SURFACE_WOOD_PICKET_FENCE:
        case SURFACE_WOOD_SLATTED_FENCE:
        case SURFACE_WOOD_RANCH_FENCE:   PlaySound(CAEAudioUtility::GetRandomNumberInRange(41, 44)); break; // 0x4DAD14
        case SURFACE_GLASS:              PlaySound(11); break;                                              // 0x4DAD43
        default:                         break;
        }
    };
    if (entity->GetModelId() == MODEL_MOLOTOV) { // 0x4DAB7B
        PlaySoundForSurface(SURFACE_GLASS);
    } else if (const auto cd = entity->GetColData()) { // 0x4DAB89
        if (const auto spheres = cd->GetSpheres(); !spheres.empty()) {
            PlaySoundForSurface(spheres.front().GetSurfaceType());
        } else if (const auto boxes = cd->GetBoxes(); !boxes.empty()) {
            PlaySoundForSurface(boxes.front().GetSurfaceType());
        } else if (const auto tris = cd->GetTris(); !tris.empty()) {
            PlaySoundForSurface(tris.front().GetSurfaceType());
        }
    }
}

// 0x4DBA10
void CAECollisionAudioEntity::ReportCollision(
    CEntity*       entityA,
    CEntity*       entityB,
    eSurfaceType   surfaceA,
    eSurfaceType   surfaceB,
    const CVector& pos,
    const CVector* normal,
    float          impulseForce,
    float          relVelSq,
    bool           isForceOneShot,
    bool           isForceLooping
) {
    if (!AEAudioHardware.IsSoundBankLoaded(SND_BANK_GENRL_COLLISIONS, SND_BANK_SLOT_COLLISIONS)) {
        return;
    }
    if (!CanAddNewSound()) {
        return;
    }
    if (relVelSq <= 0.f) {
        return;
    }

    const auto GetSurfaceToUse = [&](
        CEntity* eA, eSurfaceType& sA,
        CEntity* eB, eSurfaceType& sB
    ) {
        if (!eA->GetIsTypeVehicle()) {      
            switch (eA->GetModelId()) {
            case MODEL_MOLOTOV:
                return SURFACE_GLASS;
            case MODEL_SATCHEL:
                return AE_SURFACE_TYPE_SATCHEL_CHARGE;
            case MODEL_GRENADE:
            case MODEL_BOMB:
            case MODEL_TEARGAS:
                return AE_SURFACE_TYPE_GRENADE;
            }
            const auto eAModelId = eA->GetModelId();
            if (eAModelId == ModelIndices::MI_BASKETBALL) {
                return AE_SURFACE_TYPE_BASKETBALL;
            }
            if (eAModelId == ModelIndices::MI_PUNCHBAG) {
                return AE_SURFACE_TYPE_PUNCHBAG;
            }
            if (eAModelId == ModelIndices::MI_GRASSHOUSE) {
                return SURFACE_UNBREAKABLE_GLASS;
            }
            if (eAModelId == ModelIndices::MI_IMY_GRAY_CRATE) {
                return SURFACE_WOOD_SOLID;
            }
            if (eA->GetIsTypePhysical() && eA->AsPhysical()->m_nPhysicalFlags.bExtraHeavy) {
                return AE_SURFACE_TYPE_POOL_BALL;
            }
            return sA;        
        }
        if (eB && eB->GetIsTypeBuilding() && normal && eA->GetUp().Dot(*normal) > 0.6f) {
            if (eA->AsVehicle()->IsSubBMX()) {
                return AE_SURFACE_TYPE_BMX;
            }
            if (g_surfaceInfos.GetFrictionEffect(sB) != FRICTION_FX_SPARKS) {
                return SURFACE_RUBBER;
            }
        }
        if (eA->AsVehicle()->IsSubBMX()) {
            return AE_SURFACE_TYPE_BMX;
        }
        return SURFACE_CAR;
    };

    surfaceA = GetSurfaceToUse(entityA, surfaceA, entityB, surfaceB);
    surfaceB = GetSurfaceToUse(entityB, surfaceB, entityA, surfaceA);

    if (isForceOneShot) { // 0x4DBC68
        PlayOneShotCollisionSound(entityA, entityB, surfaceA, surfaceB, impulseForce, pos);
    } else  { // 0x4DBC83
        int32 entryID;
        switch (const auto soundStatus = CAECollisionAudioEntity::GetCollisionSoundStatus(entityA, entityB, surfaceA, surfaceB, entryID)) {
        case COLLISION_SOUND_INACTIVE: { // 0x4DBD1C, 0x4DBD5A
            if (isForceLooping) {
                return PlayLoopingCollisionSound(entityA, entityB, surfaceA, surfaceB, impulseForce, pos, isForceLooping);
            }
            return PlayOneShotCollisionSound(entityA, entityB, surfaceA, surfaceB, impulseForce, pos);
        }
        case COLLISION_SOUND_ONE_SHOT: // 0x4DBD1C, 0x4DBDE0
            return PlayLoopingCollisionSound(entityA, entityB, surfaceA, surfaceB, impulseForce, pos, isForceLooping);
        case COLLISION_SOUND_LOOPING: { // 0x4DBCA3, 0x4DBD44
            const auto e = &m_CollisionSoundList[entryID];
            e->LoopStopTimeMs = CTimer::GetTimeInMS() + 100;
            if (e->Sound) {
                UpdateLoopingCollisionSound(e->Sound, e->EntityA, e->EntityB, e->SurfaceA, e->SurfaceB, impulseForce, pos, isForceLooping);
            }
            break;
        }
        default:
            NOTSA_UNREACHABLE("Invalid soundStatus: {}", (int32)(soundStatus));
        }
    }
}

// 0x4DBDF0
void CAECollisionAudioEntity::ReportBulletHit(CEntity* entity, eSurfaceType surface, const CVector& posn, float angleWithColPointNorm) {
    if (!AEAudioHardware.IsSoundBankLoaded(SND_BANK_GENRL_BULLET_HITS, SND_BANK_SLOT_BULLET_HITS)) {
        return;
    }
    if (entity && entity->GetIsTypeVehicle()) {
        PlayBulletHitCollisionSound(
            entity->AsVehicle()->IsSubBMX()
                ? AE_SURFACE_TYPE_BMX
                : SURFACE_CAR,
            posn,
            angleWithColPointNorm
        );
    } else {
        PlayBulletHitCollisionSound(surface, posn, angleWithColPointNorm);
    }
}

// 0x4DA2C0
void CAECollisionAudioEntity::Service() {
    const uint32 currentTime = CTimer::GetTimeInMS();

    for (int32 i = 0; i < 300; ++i) {
        tCollisionSound& entry = m_CollisionSoundList[i];

        if (entry.Status == COLLISION_SOUND_INACTIVE) {
            continue;
        }

        bool bShouldClear = false;

        if (entry.Status == COLLISION_SOUND_LOOPING) {
            if (currentTime >= entry.LoopStopTimeMs) {
                bShouldClear = true;
            }
        } else if (entry.Status == COLLISION_SOUND_ONE_SHOT) {
            // Si el puntero es nulo o ya expiró el tiempo asignado en AddCollisionSoundToList
            if (!entry.Sound || (entry.LoopStopTimeMs != 0 && currentTime >= entry.LoopStopTimeMs)) {
                bShouldClear = true;
            }
        }

        if (bShouldClear) {
            if (entry.Sound) {
                entry.Sound->StopSoundAndForget();
            }

            entry = {};
            if (m_NumActiveCollisionSounds > 0) {
                --m_NumActiveCollisionSounds;
            }
        }
    }
}

// 0x4DAA50
eSoundID CAECollisionAudioEntity::ChooseCollisionSoundID(eSurfaceType surface) {
    const auto* const l = &gCollisionLookup[surface];
    if (l->MinSoundID == l->MaxSoundID) {
        return l->MinSoundID;
    }
    for (auto retry = 0; retry < 100; retry++) { // NOTSA: Handle very unlikely infinite loop
        const auto soundID = CAEAudioUtility::GetRandomNumberInRange(l->MinSoundID, l->MaxSoundID);
        if (soundID != m_CollisionSoundIDHistory[surface]) {
            return soundID;
        }
    }
    NOTSA_LOG_WARN("Failed to generate collision sound ID not in history!"); // NOTSA
    return l->MinSoundID; // NOTSA
}
