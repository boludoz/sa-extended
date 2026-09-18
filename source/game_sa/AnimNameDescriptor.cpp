#include "StdInc.h"
#include "AnimNameDescriptor.h"

void CAnimNameDescriptor::InjectHooks() {
    RH_ScopedClass(CAnimNameDescriptor);
    RH_ScopedCategory("Animation");

    RH_ScopedInstall(Constructor, 0x46A340);
}
