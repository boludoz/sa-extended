#pragma once

#include "Base.h"

#include "Accident.h"

const int32 NUM_ACCIDENTS = 16;

class CAccidentManager {
public:
    std::array<CAccident, NUM_ACCIDENTS> m_Accidents;

    static inline auto& gAccidentManager = StaticRef<CAccidentManager*>(0xB9B7D0);

public:
    static void InjectHooks();

    static CAccidentManager* GetInstance();

    void       ReportAccident(CPed* ped);
    int32      GetNumberOfFreeAccidents();
    CAccident* GetNearestFreeAccidentExceptThisOne(const CVector& posn, CAccident* thisOne, bool bIgnoreHeadless);
    CAccident* GetNearestFreeAccident(const CVector& posn, bool bIgnoreHeadless);
};

VALIDATE_SIZE(CAccidentManager, 0x80);

//! calineva API
inline CAccidentManager* GetAccidentManager() { return CAccidentManager::GetInstance(); }
