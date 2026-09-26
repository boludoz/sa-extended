#pragma once

#include "ePedStats.h"

class CPedStat {
public:
    uint32 m_ePedStatType;
    char   m_sPedStatName[24];
    float  m_fFleeDistance;
    float  m_fMaxHeadingChange;
    uint8  m_nFear;
    uint8  m_nTemper;
    uint8  m_nLawfulness;
    uint8  m_nSexiness;
    float  m_fAttackMult;
    float  m_fDefendMult;
    uint16 m_nStatFlags;
    char   m_iDefaultDecisionMaker;
};

VALIDATE_SIZE(CPedStat, 0x34);

class CPedStats {
public:
    static inline auto& ms_apPedStats = StaticRef<CPedStat*>(0xC0BBEC);

public:
    static void InjectHooks();

    static void Initialise();
    static void Shutdown();
    static void LoadPedStats();
    static ePedStats GetPedStatType(const char* statName);
    static CPedStat* GetPedStatInfo(const char* statName);
    static CPedStat* GetPedStatByArrayIndex(uint32 statIndex);
    static void FindIndexWithPedStat(void* a1);
};
