/*
    Plugin-SDK file
    Authors: GTA Community. See more here
    https://github.com/DK22Pac/plugin-sdk
    Do not delete this comment block. Respect others' work!
*/
#pragma once

#include "TaskSimple.h"
class CAnimBlendAssociation;
class CEntity;
class CPed;
class CPlayerPed;

enum eFightAttackType : int8 {
    FIGHT_ATTACK_HIT_1 = 0,
    FIGHT_ATTACK_HIT_2 = 1,
    FIGHT_ATTACK_HIT_3 = 2,
    FIGHT_ATTACK_FIGHT_BLOCK = 3,
    FIGHT_ATTACK_FIGHTIDLE = 4,
};

class NOTSA_EXPORT_VTABLE CMeleeInfo {
public:
    AssocGroupId         m_nAnimGroup;
    float                m_fRanges;
    std::array<float, 5> m_fHit;       // FireTime
    std::array<float, 5> m_fChain;     // ComboTime
    std::array<float, 5> m_fRadius;    // Radius
    float                m_fGroundLoop;
    float                ABlockHit;
    float                ABlockChain;
    std::array<uint8, 5> m_nHitLevel;  // HitLevel
    std::array<uint8, 5> m_nDamage;    // Damage
    uint8                __pad0[2];
    std::array<int32, 5> m_Hit;        // HitSound
    std::array<int32, 5> m_AltHit;     // AltHitSound
    uint16               m_wFlags;
    uint8                __pad1[2];
};
VALIDATE_SIZE(CMeleeInfo, 0x88);

class NOTSA_EXPORT_VTABLE CTaskSimpleFight : public CTaskSimple {
public:
    bool                   m_bIsFinished;        // +8
    bool                   m_bIsInControl;       // +9
    bool                   m_bAnimsReferenced;   // +10
    AssocGroupId           m_nRequiredAnimGroup; // +12
    uint16                 m_nIdlePeriod;        // +16
    uint16                 m_nIdleCounter;       // +18
    int8                   m_nContinueStrike;    // +20
    int8                   m_nChainCounter;      // +21
    CEntity*               m_pTargetEntity;      // +24
    CAnimBlendAssociation* m_pAnim;              // +28
    CAnimBlendAssociation* m_pIdleAnim;          // +32
    int8                   m_nComboSet;          // +36
    eFightAttackType       m_nCurrentMove;       // +37
    uint8                  m_nNextCommand;       // +38
    uint8                  m_nLastCommand;       // +39

    static inline auto& m_aComboData = StaticRef<std::array<CMeleeInfo, 13>>(0xC170D0);
    static inline auto& m_aHitOffset = StaticRef<std::array<CVector, 7>>(0xC177D0);
    static inline auto& m_sStrikeColModel = StaticRef<CColModel>(0xC17824);
    static inline auto& m_sStrikeColData  = StaticRef<CCollisionData>(0xC17854);
    static inline auto& m_sStrikeSpheres  = StaticRef<std::array<CColSphere, 1>>(0xC17884);

public:
    static constexpr auto Type = eTaskType::TASK_SIMPLE_FIGHT;

    CTaskSimpleFight(CEntity* entity, int32 nCommand, uint32 nIdlePeriod = 10000);
    ~CTaskSimpleFight() override;

    eTaskType GetTaskType() const override { return Type; }
    CTask* Clone() const override { return new CTaskSimpleFight(m_pTargetEntity, m_nLastCommand, m_nIdlePeriod); }
    bool MakeAbortable(CPed* ped, eAbortPriority priority = ABORT_PRIORITY_URGENT, const CEvent* event = nullptr) override;
    bool ProcessPed(CPed* ped) override;

    static void LoadMeleeData();

    bool BeHitWhileBlocking(CPed* ped1, CPed* ped2, int8 attackCombo, int8 attackMove);
    int16 ChooseAttackAI(CPed* ped);
    int16 ChooseAttackPlayer(CPed* ped);
    bool ControlFight(CEntity* entity, uint8 command);

    void FightHitCar(CPed* ped, CVehicle* vehicle, const CVector& posn1, const CVector& posn2, int16 pieceType, uint8 surfaceType);
    void FightHitObj(CPed* ped, CObject* object, const CVector& posn1, const CVector& posn2, int16 pieceType, uint8 surfaceType);
    CPed* FightHitPed(CPed* creator, CPed* victim, const CVector& posn1, const CVector& posn2, int16 pieceType);
    void FightSetUpCol(float radius);
    void FightStrike(CPed* ped, CVector& posn);

    bool FindTargetOnGround(CPed* ped);
    static void FinishMeleeAnimCB(CAnimBlendAssociation* anim, void* data);

    bool IsComboSet();
    bool IsHitComboSet();

    void GetAvailableComboSet(CPed* ped, int8 nextCommand);
    static int32 GetComboType(const char* name);
    AssocGroupId GetComboAnimGroupID();
    static uint8 GetHitLevel(const char* levelStr);
    static int16 GetHitSound(int32 nHitSound);
    float GetRange() const;
    float GetStrikeDamage(CPed* ped);

    void SetPlayerMoveAnim(CPlayerPed* player);
    void StartAnim(CPed* ped, int32 newMove);

private:
    friend void InjectHooksMain();
    static void InjectHooks();

    CTaskSimpleFight* Constructor(CEntity* entity, int32 nCommand, uint32 nIdlePeriod);
    CTaskSimpleFight* Destructor();

};
VALIDATE_SIZE(CTaskSimpleFight, 0x28);
