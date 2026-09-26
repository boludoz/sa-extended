/*
    Plugin-SDK file
    Authors: GTA Community. See more here
    https://github.com/DK22Pac/plugin-sdk
    Do not delete this comment block. Respect others' work!
*/
#pragma once

#include "Vector.h"

class CEntity;
class CObject;

struct tScriptForBrains {
    int16 StreamedScriptIndex{-1}; /// SCM ID for `CStreaming` (Translated using SCMToModelId)
    int8  TypeOfBrain{ -1 };
    int8  ObjectGroupingID{ -1 };
    bool  bBrainActive{ true };
    float ObjectBrainActivationRange{ 5.f };
    union {
        struct {
            int16  PedModelOrPedGeneratorIndex;
            uint16 PercentageChance;
            uint32 Pad;
        };
        char ScriptName[8]{};
    };
};

using script_brain_struct = tScriptForBrains; //!< calineva API

class CScriptsForBrains {
public:
    //! calineva API
    enum {
        PED_STREAMED,
        OBJECT_STREAMED,
        PED_GENERATOR_STREAMED,
        CODE_PED,
        CODE_OBJECT,
        CODE_ATTRACTOR_PED
    };
    enum { MAX_SCRIPT_BRAINS = 70 };

    std::array<tScriptForBrains, 70> ScriptBrainArray;

public:
    static void InjectHooks();

    void Init();

    void AddNewScriptBrain(int16 ImgIndex, int16 Model, uint16 priority, int8 attachType, int8 Type, float Radius);
    void AddNewStreamedScriptBrainForCodeUse(int16 a2, char* a3, int8 attachtype);

    void CheckIfNewEntityNeedsScript(CEntity* entity, int8 attachType, void* unused);

    int16 GetIndexOfScriptBrainWithThisName(const char* name, int8 type);

    bool HasAttractorScriptBrainWithThisNameLoaded(const char* name);
    bool IsObjectWithinBrainActivationRange(CObject* entity, const CVector& point);

    void MarkAttractorScriptBrainWithThisNameAsNoLongerNeeded(const char* name);
    void RequestAttractorScriptBrainWithThisName(const char* name);

    void StartAttractorScriptBrainWithThisName(const char* name, CPed* ped, bool bHasAScriptBrain);
    void StartNewStreamedScriptBrain(uint8 index, CEntity* entity, bool bHasAScriptBrain);
    void StartOrRequestNewStreamedScriptBrain(uint8 index, CEntity* entity, int8 attachType, bool bAddToWaitingArray);
    void StartOrRequestNewStreamedScriptBrainWithThisName(const char* name, CEntity* entity, int8 attachType);

    void SwitchAllObjectBrainsWithThisID(int8 ID, bool bStatus);
};
