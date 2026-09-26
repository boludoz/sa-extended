#pragma once

#include "Base.h"

#include "Vector.h"
#include "InteriorInfo_t.h"

class CEntity;
class CPed;
class Interior_c;
struct InteriorInfo_t;

enum class eInteriorGroupType : int8 {
    HOUSE = 0,
    SHOP = 1,
    OFFICE = 2,
};

class InteriorGroup_c : public ListItem_c<InteriorGroup_c> {
public:
    CEntity*    m_pEntity;             // 0x8
    uint8       m_id;                  // 0xC
    uint8       m_groupId;             // 0xD
    uint8       m_groupType;           // 0xE - TODO: eInteriorGroupType
    uint8       m_numInteriors;        // 0xF
    Interior_c* m_pInteriors[8];        // 0x10
    CEntryExit* m_pEntryExit;                // 0x30
    bool        m_isVisible;           // 0x34
    bool        m_lastIsVisible;       // 0x35
    int8        m_numPeds;             // 0x36
    CPed*       m_pPeds[16];            // 0x38
    CPed*       m_pPedsToRemove[16];    // 0x78
    int8        m_pathsSetup;   // 0xB8
    int8        m_pedsSetup;          // 0xB9
    int8        m_animsReferenced; // 0xBA

public:
    static void InjectHooks();

    InteriorGroup_c() = default;  // 0x597FE0
    ~InteriorGroup_c() = default; // 0x597FF0

    auto GetInteriors() { return m_pInteriors | rng::views::take(m_numInteriors); }
    auto GetPeds() { return m_pPeds | rng::views::take(m_numPeds); }

    void Init(CEntity* entity, int32 id);
    void Update();
    void AddInterior(Interior_c* interior);
    void SetupPeds();
    void UpdatePeds();
    void SetupHousePeds();
    void SetupPaths();
    int8 ArePathsLoaded();
    void Setup();
    void Exit();
    int8 ContainsInteriorType(int32 a2);
    void CalcIsVisible();

    void DereferenceAnims();
    void ReferenceAnims();

    void UpdateOfficePeds();
    void RemovePed(CPed* a2);
    void UpdateHousePeds() {} // empty in the original (inlined away)
    void UpdateShopPeds() {}  // empty in the original (inlined away)
    void SetupShopPeds();
    void SetupOfficePeds();
    CEntity* GetEntity();
    CPed* GetPed(int32);
    bool FindClosestInteriorInfo(int32 a, CVector point, float b, InteriorInfo_t** interiorInfo, Interior_c** interior, float* pSome);
    bool FindInteriorInfo(eInteriorInfoType infoType, InteriorInfo_t** a3, Interior_c** a4);
    int32 GetNumInteriorInfos(int32 a2);
    Interior_c* GetRandomInterior();
    auto GetId() const { return m_id; }

    auto GetInteriors() const { return m_pInteriors | std::views::take(m_numInteriors); }
private:

    //! @notsa
    const char* GetAnimBlockName();
};
VALIDATE_SIZE(InteriorGroup_c, 0xBC);
