#include "StdInc.h"

#include "ScriptResourceManager.h"

void CScriptResourceManager::InjectHooks() {
    RH_ScopedClass(CScriptResourceManager);
    RH_ScopedCategory("Scripts");

    RH_ScopedInstall(Initialise, 0x470480);
    RH_ScopedInstall(AddToResourceManager, 0x4704B0);
    RH_ScopedInstall(RemoveFromResourceManager, 0x470510);
    RH_ScopedInstall(HasResourceBeenRequested, 0x470620);
    //RH_ScopedInstall(Load, 0x0, { .reversed = false });
    //RH_ScopedInstall(Save, 0x0, { .reversed = false });
}

// 0x470480
void CScriptResourceManager::Initialise() {
    ScriptResources.fill(tScriptResource());
}

// 0x4704B0
// ASM Match
void CScriptResourceManager::AddToResourceManager(int32 Index, eScriptResourceType Type, CRunningScript* pScriptUsingThisResource)
{
    int32 loop = 0;
    int32 free_index = -1;
    for (; loop < 75; ++loop)
    {
        if (ScriptResources[loop].ResourceIndex == Index &&
            ScriptResources[loop].ResourceType == Type &&
            ScriptResources[loop].pUsedBy == pScriptUsingThisResource)
        {
            return;
        }
        if (!ScriptResources[loop].ResourceType)
        {
            free_index = loop;
        }
    }
    if (free_index != -1)
    {
        ScriptResources[free_index].ResourceIndex = Index;
        ScriptResources[free_index].ResourceType = Type;
        ScriptResources[free_index].pUsedBy = pScriptUsingThisResource;
    }
}

// 0x470510
// ASM Match: 91.2%
bool CScriptResourceManager::RemoveFromResourceManager(int32 Index, eScriptResourceType Type, CRunningScript* pScriptFreeingThisResource)
{
    int32 found_index = -1;
    uint32 number_of_other_scripts_using_this_resource = 0;

    for (uint32 loop = 0; loop < 75; ++loop)
    {
        if (ScriptResources[loop].ResourceIndex == Index && ScriptResources[loop].ResourceType == Type)
        {
            if (ScriptResources[loop].pUsedBy == pScriptFreeingThisResource)
            {
                found_index = loop;
            }
            else
            {
                ++number_of_other_scripts_using_this_resource;
            }
        }
    }

    if (found_index != -1)
    {
        ScriptResources[found_index].ResourceIndex = -1;
        ScriptResources[found_index].ResourceType = (eScriptResourceType)0;
        ScriptResources[found_index].pUsedBy = nullptr;
    }

    return !number_of_other_scripts_using_this_resource;
}

// 0x470620
// ASM Match
bool CScriptResourceManager::HasResourceBeenRequested(int32 Index, eScriptResourceType Type)
{
    for (uint32 loop = 0; loop < 75; ++loop)
    {
        if (ScriptResources[loop].ResourceIndex == Index && ScriptResources[loop].ResourceType == Type)
        {
            return true;
        }
    }
    return false;
}

// 0x0
bool CScriptResourceManager::Load() {
    assert(false);
    return true;
}

// 0x0
bool CScriptResourceManager::Save() {
    assert(false);
    return true;
}
