/*
    Plugin-SDK file
    Authors: GTA Community. See more here
    https://github.com/DK22Pac/plugin-sdk
    Do not delete this comment block. Respect others' work!
*/
#pragma once

class CRunningScript;

enum eScriptResourceType {
    RESOURCE_TYPE_DEFAULT               = 0,
    RESOURCE_TYPE_ANIMATION             = 1,
    RESOURCE_TYPE_MODEL_OR_SPECIAL_CHAR = 2,
    RESOURCE_TYPE_DECISION_MAKER        = 3
};

struct tScriptResource {
    int32               ResourceIndex;
    void*               pUsedBy;
    eScriptResourceType ResourceType;

    tScriptResource() {
        ResourceIndex = -1;
        ResourceType    = RESOURCE_TYPE_DEFAULT;
        pUsedBy  = nullptr;
    }
};

class CScriptResourceManager {
public:
    std::array<tScriptResource, 75> ScriptResources;

public:
    static void InjectHooks();

    CScriptResourceManager() = delete;

    void Initialise();
    void AddToResourceManager(int32 modelId, eScriptResourceType type, CRunningScript* script);
    bool RemoveFromResourceManager(int32 modelId, eScriptResourceType type, CRunningScript* script);
    bool HasResourceBeenRequested(int32 modelId, eScriptResourceType type);
    bool Load();
    bool Save();
};

VALIDATE_SIZE(CScriptResourceManager, 0x384);
VALIDATE_SIZE(tScriptResource, 0xC);
