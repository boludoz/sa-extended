/*
    Plugin-SDK file
    Authors: GTA Community. See more here
    https://github.com/DK22Pac/plugin-sdk
    Do not delete this comment block. Respect others' work!
*/
#pragma once

#include "TaskTimer.h"

class C2dEffect;
class CEntity;
class CVector;

class CAttractorScanner {
public:
    char       field_0;
    char       _pad[3];
    CTaskTimer field_4;
    C2dEffect* m_pEffectInUse;
    int32      field_14;
    int32      field_18[10];
    int32      field_40[10];
    int32      field_68[10];

    void Clear();

    // 0x6002F0
    static bool GetClosestAttractorOfType(const CVector& pos, float radius, int32 modelIndexToFind, int32 queueType, const char* triggeredScriptName, bool bMustBeFree, const C2dEffect* effectToNeglect, C2dEffect*& closestEffect, CEntity*& closestEntity) {
        return plugin::CallAndReturn<bool, 0x6002F0, const CVector&, float, int32, int32, const char*, bool, const C2dEffect*, C2dEffect*&, CEntity*&>(pos, radius, modelIndexToFind, queueType, triggeredScriptName, bMustBeFree, effectToNeglect, closestEffect, closestEntity);
    }
};

VALIDATE_SIZE(CAttractorScanner, 0x90);
