/*
    Plugin-SDK file
    Authors: GTA Community. See more here
    https://github.com/DK22Pac/plugin-sdk
    Do not delete this comment block. Respect others' work!
*/
#pragma once

#include "Object.h"

class NOTSA_EXPORT_VTABLE CProjectile : public CObject {
public:
    CProjectile(int32 modelId);
    ~CProjectile() override = default;

private:
    friend void InjectHooksMain();
    static void InjectHooks();

    CProjectile* Constructor(int32 modelId);
    CProjectile* Destructor();
};

VALIDATE_SIZE(CProjectile, 0x17C);
