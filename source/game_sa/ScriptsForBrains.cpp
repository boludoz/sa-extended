#include "StdInc.h"

#include "ScriptsForBrains.h"

static const float fDefaultRadius = 5.0f;

void CScriptsForBrains::InjectHooks() {
    RH_ScopedClass(CScriptsForBrains);
    RH_ScopedCategory("Tasks/TaskTypes");

    RH_ScopedInstall(Init, 0x46A8C0, {.reversed = false});
    //RH_ScopedInstall(SwitchAllObjectBrainsWithThisID, 0x46A900);
    //RH_ScopedInstall(AddNewScriptBrain, 0x46A930);
    //RH_ScopedInstall(AddNewStreamedScriptBrainForCodeUse, 0x46A9C0);
    RH_ScopedInstall(GetIndexOfScriptBrainWithThisName, 0x46AA30);
    RH_ScopedInstall(HasAttractorScriptBrainWithThisNameLoaded, 0x46AB20);
    //RH_ScopedInstall(StartNewStreamedScriptBrain, 0x46B270);
    RH_ScopedInstall(StartAttractorScriptBrainWithThisName, 0x46B390);
    RH_ScopedInstall(StartOrRequestNewStreamedScriptBrain, 0x46CD80, {.reversed = false});
    //RH_ScopedInstall(StartOrRequestNewStreamedScriptBrainWithThisName, 0x46CED0);
    RH_ScopedInstall(IsObjectWithinBrainActivationRange, 0x46B3D0, {.reversed=false});
    RH_ScopedInstall(MarkAttractorScriptBrainWithThisNameAsNoLongerNeeded, 0x46AAE0);
    RH_ScopedInstall(RequestAttractorScriptBrainWithThisName, 0x46AA80);
}


// 0x46A8C0
void CScriptsForBrains::Init() {
    for (auto& script : ScriptBrainArray) {
        script = tScriptForBrains();
    }
}

// 0x46A900
// ASM Match
void CScriptsForBrains::SwitchAllObjectBrainsWithThisID(int8 object_grouping_id, bool bBrainOn)
{
    if (object_grouping_id >= 0)
    {
        for (int32 i = 0; i < MAX_SCRIPT_BRAINS; ++i)
        {
            if (ScriptBrainArray[i].ObjectGroupingID == object_grouping_id)
            {
                ScriptBrainArray[i].bBrainActive = bBrainOn;
            }
        }
    }
}

// 0x46A930
// ASM Match: 99.7%
void CScriptsForBrains::AddNewScriptBrain(int16 StreamedScriptIndex, int16 PedModelOrPedGeneratorIndex, uint16 Percentage, int8 BrainType, int8 object_grouping_id, float ObjectActivationRange)
{
    bool bFound = false;
    uint8 loop = 0;

    do
    {
        if (bFound)
        {
            break;
        }
        if (ScriptBrainArray[loop].StreamedScriptIndex == -1)
        {
            bFound = true;
        }
        else
        {
            loop++;
        }
    } while (loop < 70);

    if (!bFound)
    {
        return;
    }

    ScriptBrainArray[loop].StreamedScriptIndex = StreamedScriptIndex;
    ScriptBrainArray[loop].PedModelOrPedGeneratorIndex = PedModelOrPedGeneratorIndex;
    ScriptBrainArray[loop].PercentageChance = Percentage;
    ScriptBrainArray[loop].ObjectGroupingID = object_grouping_id;
    ScriptBrainArray[loop].TypeOfBrain = BrainType;
    ScriptBrainArray[loop].bBrainActive = true;
    if (ObjectActivationRange <= 0.0f)
    {
        ScriptBrainArray[loop].ObjectBrainActivationRange = fDefaultRadius;
    }
    else
    {
        ScriptBrainArray[loop].ObjectBrainActivationRange = ObjectActivationRange;
    }
}

// 0x46A9C0
// ASM Match: 99.4%
void CScriptsForBrains::AddNewStreamedScriptBrainForCodeUse(int16 StreamedScriptIndex, char* pScriptName, int8 BrainType)
{
    bool bFound = false;
    uint8 loop = 0;

    do
    {
        if (bFound)
        {
            break;
        }
        if (ScriptBrainArray[loop].StreamedScriptIndex == -1)
        {
            bFound = true;
        }
        else
        {
            loop++;
        }
    } while (loop < 70);

    if (!bFound)
    {
        return;
    }

    ScriptBrainArray[loop].StreamedScriptIndex = StreamedScriptIndex;
    strcpy(ScriptBrainArray[loop].ScriptName, pScriptName);
    ScriptBrainArray[loop].TypeOfBrain = BrainType;
    ScriptBrainArray[loop].ObjectGroupingID = -1;
    ScriptBrainArray[loop].bBrainActive = true;
    ScriptBrainArray[loop].ObjectBrainActivationRange = fDefaultRadius;
}

void CScriptsForBrains::CheckIfNewEntityNeedsScript(CEntity* entity, int8 attachType, void* unused) {
    plugin::CallMethod<0x46FF20, CScriptsForBrains*, CEntity*, int8, void*>(this, entity, attachType, unused);
}

// 0x46AAE0
// ASM Match: not measured
void CScriptsForBrains::MarkAttractorScriptBrainWithThisNameAsNoLongerNeeded(const char* pScriptName)
{
    int16 ScriptBrainIndex = GetIndexOfScriptBrainWithThisName(pScriptName, CODE_ATTRACTOR_PED);
    if (ScriptBrainIndex >= 0)
    {
        uint8 CurrentNumberOfUsers = CTheScripts::StreamedScripts.GetNumUsersOfStreamedScript(ScriptBrainArray[ScriptBrainIndex].StreamedScriptIndex);
        if (CurrentNumberOfUsers > 0)
        {
            CTheScripts::StreamedScripts.SetNumUsersOfStreamedScript(ScriptBrainArray[ScriptBrainIndex].StreamedScriptIndex, CurrentNumberOfUsers - 1);
        }
    }
}

// 0x46AA80
// ASM Match: not measured
void CScriptsForBrains::RequestAttractorScriptBrainWithThisName(const char* pScriptName)
{
    int16 ScriptBrainIndex = GetIndexOfScriptBrainWithThisName(pScriptName, CODE_ATTRACTOR_PED);
    if (ScriptBrainIndex >= 0)
    {
        int16 StreamedScriptIndex = ScriptBrainArray[ScriptBrainIndex].StreamedScriptIndex;
        if (!CStreaming::HasScriptLoaded(StreamedScriptIndex))
        {
            CStreaming::RequestScript(StreamedScriptIndex, STRFLAG_PRIORITY_LOAD);
        }
        CTheScripts::StreamedScripts.SetNumUsersOfStreamedScript(StreamedScriptIndex, CTheScripts::StreamedScripts.GetNumUsersOfStreamedScript(StreamedScriptIndex) + 1);
    }
}

// 0x46B270
// ASM Match: 99.8%
void CScriptsForBrains::StartNewStreamedScriptBrain(uint8 ScriptBrainIndex, CEntity* pEntity, bool bPedCreatedAtAttractor)
{
    script_brain_struct* pBrain = &ScriptBrainArray[ScriptBrainIndex];
    CRunningScript* pNewOne = CTheScripts::StreamedScripts.StartNewStreamedScript(pBrain->StreamedScriptIndex);
    pNewOne->m_ExternalType = pBrain->TypeOfBrain;

    switch (pBrain->TypeOfBrain)
    {
    case PED_STREAMED:
    case CODE_PED:
    case CODE_ATTRACTOR_PED:
    {
        CPed* pPed = static_cast<CPed*>(pEntity);
        pNewOne->m_LocalVars[0].iParam = (*GetPedPool()).GetIndex(pPed);
        pPed->bHasAScriptBrain = true;
        if (pBrain->TypeOfBrain == CODE_ATTRACTOR_PED)
        {
            pNewOne->m_LocalVars[1].iParam = bPedCreatedAtAttractor;
        }
        pPed->bWaitingForScriptBrainToLoad = false;
        CTheScripts::RemoveFromWaitingForScriptBrainArray(pPed, pPed->StreamedScriptBrainToLoad);
        pPed->StreamedScriptBrainToLoad = -1;
        break;
    }
    case OBJECT_STREAMED:
    case CODE_OBJECT:
    {
        CObject* pObject = static_cast<CObject*>(pEntity);
        pNewOne->m_LocalVars[0].iParam = (*GetObjectPool()).GetIndex(pObject);
        pObject->m_nObjectFlags.ScriptBrainStatus = CObject::OBJECT_RUNNING_SCRIPT_BRAIN;
        break;
    }
    default:
        break;
    }
}

void CScriptsForBrains::StartOrRequestNewStreamedScriptBrain(uint8 index, CEntity* entity, int8 attachType, bool bAddToWaitingArray) {
    NOTSA_UNREACHABLE();
}

bool CScriptsForBrains::HasAttractorScriptBrainWithThisNameLoaded(const char* name) {
    if (const auto idx = GetIndexOfScriptBrainWithThisName(name, 5); idx >= 0) {
        return CStreaming::IsModelLoaded(SCMToModelId(ScriptBrainArray[idx].StreamedScriptIndex));
    }
    return false;
}

// 0x46B3D0
bool CScriptsForBrains::IsObjectWithinBrainActivationRange(CObject* entity, const CVector& point) {
    return plugin::CallMethodAndReturn<bool, 0x46B3D0, CScriptsForBrains*, CObject*, const CVector&>(this, entity, point);
}

int16 CScriptsForBrains::GetIndexOfScriptBrainWithThisName(const char* name, int8 type) {
    const auto it = rng::find_if(ScriptBrainArray, [=](tScriptForBrains& script) {
        return script.TypeOfBrain == type && !_stricmp(script.ScriptName, name);
    });
    return it != ScriptBrainArray.end()
        ? rng::distance(ScriptBrainArray.begin(), it)
        : -1;
}

void CScriptsForBrains::StartAttractorScriptBrainWithThisName(const char* name, CPed* ped, bool bHasAScriptBrain) {
    if (!ped->bWaitingForScriptBrainToLoad && !ped->bHasAScriptBrain) {
        if (const auto idx = GetIndexOfScriptBrainWithThisName(name, 5); idx >= 0) {
            StartNewStreamedScriptBrain(static_cast<uint8>(idx), ped, bHasAScriptBrain);
        }
    }
}

// 0x46CED0
// ASM Match
void CScriptsForBrains::StartOrRequestNewStreamedScriptBrainWithThisName(
    const char* pScriptName, CEntity* pEntity, int8 BrainType)
{
    int16 ScriptBrainIndex = GetIndexOfScriptBrainWithThisName(pScriptName, BrainType);
    if (ScriptBrainIndex >= 0)
    {
        StartOrRequestNewStreamedScriptBrain(ScriptBrainIndex, pEntity, BrainType, true);
    }
}
