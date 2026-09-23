#include "StdInc.h"

#include "TaskSimpleTriggerLookAt.h"

// 0x634440
// Not reversed yet: run the original ctor so the task gets the original vtable (Clone/MakeAbortable/ProcessPed/dtor)
CTaskSimpleTriggerLookAt::CTaskSimpleTriggerLookAt(CEntity* entity, int32 time, int32 offsetBoneTag, RwV3d offsetPos, bool bUseTorso, float speed, int32 blendTime, int32 priority) {
    plugin::CallMethod<0x634440, CTaskSimpleTriggerLookAt*, CEntity*, int32, int32, RwV3d, bool, float, int32, int32>(this, entity, time, offsetBoneTag, offsetPos, bUseTorso, speed, blendTime, priority);
}
