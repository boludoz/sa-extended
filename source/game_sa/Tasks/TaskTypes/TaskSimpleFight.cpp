#include "StdInc.h"

#include "TaskSimpleFight.h"
#include "AnimManager.h"
#include "PedStats.h"
#include "Stats.h"

// 0x61C470
CTaskSimpleFight::CTaskSimpleFight(CEntity* entity, int32 nCommand, uint32 nIdlePeriod) : CTaskSimple() {
    m_bIsFinished        = false;
    m_bIsInControl       = true;
    m_bAnimsReferenced   = false;
    m_nRequiredAnimGroup = ANIM_GROUP_MAN;
    m_nIdlePeriod        = (nIdlePeriod > 60000) ? 60000 : static_cast<uint16>(nIdlePeriod);
    m_nIdleCounter       = 0;
    m_nContinueStrike    = -1;
    m_nChainCounter      = -1;
    m_pTargetEntity      = entity;
    m_pAnim              = nullptr;
    m_pIdleAnim          = nullptr;
    m_nComboSet          = 0;
    m_nCurrentMove       = FIGHT_ATTACK_FIGHTIDLE;
    m_nNextCommand       = static_cast<uint8>(nCommand);
    m_nLastCommand       = 0;

    if (entity) {
        entity->RegisterReference(reinterpret_cast<CEntity**>(&m_pTargetEntity));
    }
}

// 0x61C530
CTaskSimpleFight::~CTaskSimpleFight() {
    if (m_pTargetEntity) {
        m_pTargetEntity->CleanUpOldReference(reinterpret_cast<CEntity**>(&m_pTargetEntity));
    }
    if (m_pAnim) {
        m_pAnim->SetDeleteCallback(CDefaultAnimCallback::DefaultAnimCB, nullptr);
    }
    if (m_pIdleAnim) {
        m_pIdleAnim->SetDeleteCallback(CDefaultAnimCallback::DefaultAnimCB, nullptr);
    }
    if (m_bAnimsReferenced && m_nRequiredAnimGroup != ANIM_GROUP_MAN) {
        CAnimManager::RemoveAnimBlockRef(CAnimManager::GetAnimationBlockIndex(m_nRequiredAnimGroup));
        m_bAnimsReferenced = false;
    }
}

// 0x61C5E0
bool CTaskSimpleFight::ControlFight(CEntity* entity, uint8 command) {
    m_bIsInControl = true;
    if (entity != m_pTargetEntity) {
        if (m_pTargetEntity) {
            m_pTargetEntity->CleanUpOldReference(reinterpret_cast<CEntity**>(&m_pTargetEntity));
        }
        m_pTargetEntity = entity;
        if (entity) {
            entity->RegisterReference(reinterpret_cast<CEntity**>(&m_pTargetEntity));
        }
    }
    if (command > m_nNextCommand) {
        m_nNextCommand = command;
    }
    return true;
}

// 0x4ABDC0
bool CTaskSimpleFight::IsComboSet() {
    int32 comboIdx = (m_nComboSet - 4 < 0) ? 0 : (m_nComboSet - 4);
    return ((16 << m_nCurrentMove) & m_aComboData[comboIdx].m_wFlags) != 0;
}

// 0x4ABDF0
bool CTaskSimpleFight::IsHitComboSet() {
    if (m_nCurrentMove > 2) {
        return false;
    }
    int32 comboIdx = (m_nComboSet - 4 < 0) ? 0 : (m_nComboSet - 4);
    return (m_aComboData[comboIdx].m_wFlags & (4096 << m_nCurrentMove)) != 0;
}

// 0x4ABDA0
AssocGroupId CTaskSimpleFight::GetComboAnimGroupID() {
    int32 comboIdx = (m_nComboSet - 4 < 0) ? 0 : (m_nComboSet - 4);
    return m_aComboData[comboIdx].m_nAnimGroup;
}

// 0x5BD360
uint8 CTaskSimpleFight::GetHitLevel(const char* pString) {
    if (*pString == 'H') {
        return 0; // LEVEL_H
    }
    if (*pString == 'L') {
        return 1; // LEVEL_L
    }
    if (*pString == 'G') {
        return 2; // LEVEL_G
    }
    if (*pString == 'B') {
        return 3; // LEVEL_B
    }
    if (!strcmp(pString, "HL")) {
        return 4; // LEVEL_HL
    }
    if (!strcmp(pString, "LL")) {
        return 5; // LEVEL_LL
    }
    if (!strcmp(pString, "GL")) {
        return 6; // LEVEL_GL
    }
    return 7;
}

// 0x5BD3B0
int16 CTaskSimpleFight::GetHitSound(int32 nHitSound) {
    switch (nHitSound) {
    case 1:  return AE_PED_HIT_HIGH;            // 61
    case 2:  return AE_PED_HIT_LOW;             // 62
    case 3:  return AE_PED_HIT_GROUND;          // 63
    case 4:  return AE_PED_HIT_GROUND_KICK;     // 64
    case 5:  return AE_PED_HIT_HIGH_UNARMED;    // 65
    case 6:  return AE_PED_HIT_LOW_UNARMED;     // 66
    case 7:  return AE_PED_HIT_MARTIAL_PUNCH;   // 67
    case 8:  return AE_PED_HIT_MARTIAL_KICK;    // 68
    default: return AE_PED_HIT_LOW;             // 62
    }
}

// 0x5BEDC0
void CTaskSimpleFight::LoadMeleeData() {
    ZoneScoped;

    for (auto& combo : m_aComboData) {
        combo.m_nAnimGroup = ANIM_GROUP_MAN;
        combo.m_fRanges    = 1.5f;
        for (int32 j = 0; j < 5; j++) {
            combo.m_fHit[j]      = 100.0f;
            combo.m_fChain[j]    = 100.0f;
            combo.m_fRadius[j]   = 1.0f;
            combo.m_nHitLevel[j] = 7;
            combo.m_nDamage[j]   = 0;
            combo.m_Hit[j]       = 0;
            combo.m_AltHit[j]    = 0;
        }
        combo.m_fGroundLoop = 0.0f;
        combo.ABlockHit     = 100.0f;
        combo.ABlockChain   = 100.0f;
        combo.m_wFlags      = 0;
    }

    for (auto& offset : m_aHitOffset) {
        offset = CVector(0.0f, 0.75f, 0.0f);
    }

    auto* file = CFileMgr::OpenFile("DATA\\MELEE.DAT", "rb");
    if (!file) {
        return;
    }

    char aDataType[32];
    char aTempName[32];
    CVector vecHitLevel;
    float fRange, fHitTime, fChainTime, fRadius, fGroundLoop;
    int32 nDamage, nFlags, nHitSound, nAltHitSound;

    int32 nCurrentCombo = 0;
    int32 nCurrentLine = 0;
    int32 nCurrentAttack = 0;
    bool bReadingCombo = false;
    bool bReadingLevels = false;

    constexpr float ATTACK_ANIM_RATE = 1.0f / 30.0f;

    while (const char* line = CFileLoader::LoadLine(file)) {
        if (*line == '#' || *line == '\0') {
            continue;
        }

        if (!strncmp(line, "END_MELEE_DATA", 14)) {
            break;
        }

        if (bReadingCombo || bReadingLevels) {
            if (!strncmp(line, "END_COMBO", 9)) {
                if (bReadingCombo) {
                    nCurrentCombo++;
                }
                nCurrentLine = 0;
                bReadingCombo = false;
                bReadingLevels = false;
            } else if (bReadingLevels) {
                sscanf(line, "%s %f %f %f", aDataType, &vecHitLevel.x, &vecHitLevel.y, &vecHitLevel.z);
                if (nCurrentLine < (int32)m_aHitOffset.size()) {
                    m_aHitOffset[nCurrentLine] = vecHitLevel;
                }
                nCurrentLine++;
            } else if (nCurrentCombo < (int32)m_aComboData.size()) {
                nCurrentLine++;
                switch (nCurrentLine) {
                case 1: {
                    sscanf(line, "%s %s", aDataType, aTempName);
                    m_aComboData[nCurrentCombo].m_nAnimGroup = CAnimManager::GetAnimationGroupIdByName(aTempName);
                    break;
                }
                case 2:
                    sscanf(line, "%s %f", aDataType, &fRange);
                    m_aComboData[nCurrentCombo].m_fRanges = fRange;
                    break;
                case 3: // MDAT_ATTACK1
                case 4: // MDAT_ATTACK2
                case 5: // MDAT_ATTACK3
                case 6: // MDAT_AGROUND
                case 7: // MDAT_AMOVING
                    fGroundLoop = 0.0f;
                    sscanf(line, "%s %f %f %f %s %d %d %d %f", aDataType, &fHitTime, &fChainTime, &fRadius,
                           aTempName, &nDamage, &nHitSound, &nAltHitSound, &fGroundLoop);
                    switch (nCurrentLine) {
                    case 3: nCurrentAttack = 0; break;
                    case 4: nCurrentAttack = 1; break;
                    case 5: nCurrentAttack = 2; break;
                    case 6: nCurrentAttack = 3; break;
                    case 7: nCurrentAttack = 4; break;
                    }
                    m_aComboData[nCurrentCombo].m_fHit[nCurrentAttack]      = fHitTime * ATTACK_ANIM_RATE;
                    m_aComboData[nCurrentCombo].m_fChain[nCurrentAttack]    = fChainTime * ATTACK_ANIM_RATE;
                    m_aComboData[nCurrentCombo].m_fRadius[nCurrentAttack]   = fRadius;
                    m_aComboData[nCurrentCombo].m_nHitLevel[nCurrentAttack] = GetHitLevel(aTempName);
                    m_aComboData[nCurrentCombo].m_nDamage[nCurrentAttack]   = static_cast<uint8>(nDamage);
                    m_aComboData[nCurrentCombo].m_Hit[nCurrentAttack]       = GetHitSound(nHitSound);
                    m_aComboData[nCurrentCombo].m_AltHit[nCurrentAttack]    = GetHitSound(nAltHitSound);
                    if (fGroundLoop > 0.0f) {
                        m_aComboData[nCurrentCombo].m_fGroundLoop = fGroundLoop * ATTACK_ANIM_RATE;
                    }
                    break;
                case 8:
                    sscanf(line, "%s %f %f", aDataType, &fHitTime, &fChainTime);
                    m_aComboData[nCurrentCombo].ABlockHit   = fHitTime * ATTACK_ANIM_RATE;
                    m_aComboData[nCurrentCombo].ABlockChain = fChainTime * ATTACK_ANIM_RATE;
                    break;
                case 9:
                    sscanf(line, "%s %x", aDataType, &nFlags);
                    m_aComboData[nCurrentCombo].m_wFlags = static_cast<uint16>(nFlags);
                    break;
                }
            }
        } else {
            if (!strncmp(line, "START_COMBO", 11)) {
                bReadingCombo = true;
            } else if (!strncmp(line, "START_LEVELS", 12)) {
                bReadingLevels = true;
            }
        }
    }

    CFileMgr::CloseFile(file);
}

// 0x61DB30
int32 CTaskSimpleFight::GetComboType(const char* name) {
    if (!strcmp(name, "UNARMED")) return 4;
    if (!strcmp(name, "BBALLBAT")) return 8;
    if (!strcmp(name, "KNIFE")) return 9;
    if (!strcmp(name, "GOLFCLUB")) return 10;
    if (!strcmp(name, "SWORD")) return 11;
    if (!strcmp(name, "CHAINSAW")) return 12;
    if (!strcmp(name, "DILDO")) return 13;
    return (strcmp(name, "FLOWERS") != 0) ? 4 : 14;
}

float CTaskSimpleFight::GetRange() const {
    const auto comboIdx = m_nComboSet >= 4 ? (m_nComboSet - 4) : 0;
    return m_aComboData[comboIdx].m_fRanges;
}

// 0x61C740
float CTaskSimpleFight::GetStrikeDamage(CPed* attacker) {
    const auto comboIdx = m_nComboSet >= 4 ? (m_nComboSet - 4) : 0;
    const float damage = static_cast<float>(m_aComboData[comboIdx].m_nDamage[m_nCurrentMove]);

    if (attacker->IsPlayer()) {
        if (attacker->GetPlayerData() && attacker->GetPlayerData()->m_bAdrenaline) {
            return 50.0f;
        }
        return CStats::GetFatAndMuscleModifier(STAT_MOD_4) * damage;
    }

    const auto weaponType = attacker->GetActiveWeapon().m_Type;
    if (weaponType == WEAPON_BRASSKNUCKLE) {
        return damage * 1.5f;
    }
    if (weaponType != WEAPON_UNARMED) {
        return damage;
    }
    return damage * (attacker->m_pStats ? attacker->m_pStats->m_fAttackStrength : 1.0f);
}

// 0x61DAE0
void CTaskSimpleFight::FinishMeleeAnimCB(CAnimBlendAssociation* anim, void* data) {
    auto* task = static_cast<CTaskSimpleFight*>(data);
    if (task->m_pAnim == anim) {
        task->m_pAnim = nullptr;
    } else if (task->m_pIdleAnim == anim) {
        task->m_pIdleAnim = nullptr;
    }
    if (anim->m_AnimId == ANIM_ID_FIGHT_IDLE) {
        task->m_bIsFinished = true;
    }
    if (!task->m_pIdleAnim) {
        auto lastCmd = task->m_nLastCommand;
        if (lastCmd == 16 || lastCmd == 17 || lastCmd == 15 || lastCmd == 1) {
            task->m_bIsFinished = true;
        }
    }
}

// 0x61D5F0
void CTaskSimpleFight::FightSetUpCol(float radius) {
    if (!m_sStrikeColModel.m_pColData) {
        m_sStrikeColModel.m_pColData = &m_sStrikeColData;
        m_sStrikeColData.m_pSpheres = m_sStrikeSpheres.data();
        m_sStrikeColData.m_nNumSpheres = 1;
    }
    m_sStrikeSpheres[0].Set(radius, CVector(0.0f, 0.0f, 0.0f), SURFACE_DEFAULT);
    m_sStrikeColModel.m_boundSphere.m_fRadius = radius;
    m_sStrikeColModel.m_boundSphere.m_vecCenter = CVector(0.0f, 0.0f, 0.0f);
    m_sStrikeColModel.m_boundBox.m_vecMin = CVector(-radius, -radius, -radius);
    m_sStrikeColModel.m_boundBox.m_vecMax = CVector(radius, radius, radius);
}

// 0x61C770
bool CTaskSimpleFight::BeHitWhileBlocking(CPed* ped1, CPed* ped2, int8 attackCombo, int8 attackMove) {
    return plugin::CallMethodAndReturn<bool, 0x61C770, CTaskSimpleFight*, CPed*, CPed*, int8, int8>(this, ped1, ped2, attackCombo, attackMove);
}

// 0x61C7F0
void CTaskSimpleFight::GetAvailableComboSet(CPed* ped, int8 nextCommand) {
    plugin::CallMethod<0x61C7F0, CTaskSimpleFight*, CPed*, int8>(this, ped, nextCommand);
}

// 0x61C9B0
void CTaskSimpleFight::SetPlayerMoveAnim(CPlayerPed* player) {
    plugin::CallMethod<0x61C9B0, CTaskSimpleFight*, CPlayerPed*>(this, player);
}

// 0x61CBA0
CPed* CTaskSimpleFight::FightHitPed(CPed* creator, CPed* victim, const CVector& posn1, const CVector& posn2, int16 pieceType) {
    return plugin::CallMethodAndReturn<CPed*, 0x61CBA0, CTaskSimpleFight*, CPed*, CPed*, const CVector&, const CVector&, int16>(this, creator, victim, posn1, posn2, pieceType);
}

// 0x61D0B0
void CTaskSimpleFight::FightHitCar(CPed* ped, CVehicle* vehicle, const CVector& posn1, const CVector& posn2, int16 pieceType, uint8 surfaceType) {
    plugin::CallMethod<0x61D0B0, CTaskSimpleFight*, CPed*, CVehicle*, const CVector&, const CVector&, int16, uint8>(this, ped, vehicle, posn1, posn2, pieceType, surfaceType);
}

// 0x61D400
void CTaskSimpleFight::FightHitObj(CPed* ped, CObject* object, const CVector& posn1, const CVector& posn2, int16 pieceType, uint8 surfaceType) {
    plugin::CallMethod<0x61D400, CTaskSimpleFight*, CPed*, CObject*, const CVector&, const CVector&, int16, uint8>(this, ped, object, posn1, posn2, pieceType, surfaceType);
}

// 0x61D6F0
bool CTaskSimpleFight::FindTargetOnGround(CPed* ped) {
    return plugin::CallMethodAndReturn<bool, 0x61D6F0, CTaskSimpleFight*, CPed*>(this, ped);
}

// 0x624710
int16 CTaskSimpleFight::ChooseAttackPlayer(CPed* ped) {
    return plugin::CallMethodAndReturn<int16, 0x624710, CTaskSimpleFight*, CPed*>(this, ped);
}

// 0x624A40
int16 CTaskSimpleFight::ChooseAttackAI(CPed* ped) {
    return plugin::CallMethodAndReturn<int16, 0x624A40, CTaskSimpleFight*, CPed*>(this, ped);
}

// 0x623B10
void CTaskSimpleFight::StartAnim(CPed* ped, int32 newMove) {
    plugin::CallMethod<0x623B10, CTaskSimpleFight*, CPed*, int32>(this, ped, newMove);
}

// 0x6239F0
bool CTaskSimpleFight::MakeAbortable(CPed* ped, eAbortPriority priority, const CEvent* event) {
    if (priority == ABORT_PRIORITY_IMMEDIATE || priority == ABORT_PRIORITY_URGENT) {
        if (event && (event->GetEventType() <= 31 || event->GetEventType() == 60)) {
            return false;
        }
        if (m_pAnim) {
            if (priority == ABORT_PRIORITY_URGENT) {
                m_pAnim->SetBlendDelta(-1000.0f);
            }
            m_pAnim->SetFinishCallback(CDefaultAnimCallback::DefaultAnimCB, nullptr);
            m_pAnim = nullptr;
        }
        if (m_pIdleAnim) {
            m_pIdleAnim->SetFinishCallback(CDefaultAnimCallback::DefaultAnimCB, nullptr);
            if (m_pIdleAnim->m_BlendAmount > 0.0f && m_pIdleAnim->m_BlendDelta >= 0.0f) {
                if (priority == ABORT_PRIORITY_URGENT) {
                    CAnimManager::BlendAnimation(ped->GetRpClump(), ped->m_nAnimGroup, ANIM_ID_IDLE, 1000.0f);
                } else {
                    CAnimManager::BlendAnimation(ped->GetRpClump(), ped->m_nAnimGroup, ANIM_ID_IDLE, 16.0f);
                }
            }
            m_pIdleAnim = nullptr;
        }
        if (ped && ped->IsPlayer()) {
            SetPlayerMoveAnim(static_cast<CPlayerPed*>(ped));
        }
        m_bIsInControl = false;
        m_nNextCommand = 0;
        m_bIsFinished = true;
        m_nCurrentMove = FIGHT_ATTACK_FIGHTIDLE;
        return true;
    }
    return false;
}

// 0x629920
bool CTaskSimpleFight::ProcessPed(CPed* ped) {
    return plugin::CallMethodAndReturn<bool, 0x629920, CTaskSimpleFight*, CPed*>(this, ped);
}

// 0x6240B0
void CTaskSimpleFight::FightStrike(CPed* ped, CVector& posn) {
    plugin::CallMethod<0x6240B0, CTaskSimpleFight*, CPed*, CVector&>(this, ped, posn);
}

void CTaskSimpleFight::InjectHooks() {
    RH_ScopedVirtualClass(CTaskSimpleFight, 0x86D684, 9);
    RH_ScopedCategory("Tasks/TaskTypes");

    RH_ScopedInstall(Constructor, 0x61C470);
    RH_ScopedInstall(Destructor, 0x61C530);
    RH_ScopedInstall(ControlFight, 0x61C5E0);
    RH_ScopedInstall(IsComboSet, 0x4ABDC0);
    RH_ScopedInstall(IsHitComboSet, 0x4ABDF0);
    RH_ScopedInstall(GetComboAnimGroupID, 0x4ABDA0);
    RH_ScopedInstall(GetComboType, 0x61DB30);
    RH_ScopedInstall(GetStrikeDamage, 0x61C740);
    RH_ScopedInstall(FinishMeleeAnimCB, 0x61DAE0);
    RH_ScopedInstall(FightSetUpCol, 0x61D5F0);
    RH_ScopedInstall(GetHitLevel, 0x5BD360);
    RH_ScopedInstall(GetHitSound, 0x5BD3B0);
    RH_ScopedInstall(LoadMeleeData, 0x5BEDC0);
    RH_ScopedInstall(FightStrike, 0x6240B0, { .reversed = false });

    RH_ScopedVMTInstall(Clone, 0x622E40, { .reversed = false });
    RH_ScopedVMTInstall(GetTaskType, 0x61C520);
    RH_ScopedVMTInstall(MakeAbortable, 0x6239F0, { .reversed = false });
    RH_ScopedVMTInstall(ProcessPed, 0x629920, { .reversed = false });
}

CTaskSimpleFight* CTaskSimpleFight::Constructor(CEntity* entity, int32 nCommand, uint32 nIdlePeriod) {
    this->CTaskSimpleFight::CTaskSimpleFight(entity, nCommand, nIdlePeriod);
    return this;
}

CTaskSimpleFight* CTaskSimpleFight::Destructor() {
    this->CTaskSimpleFight::~CTaskSimpleFight();
    return this;
}
