#pragma once

#include "Base.h"
#include "Vector.h"
#include <extensions/WEnum.hpp>

class CEntity;

/// What a ped does when it walks up to an interior info marker.
/// NOTE: `STANDSTILL_INDEFINITELY` fixes an obvious typo in the recovered name.
enum class eInteriorInfoType {
    NONE = 0,
    SITIN_CHAIR,
    STANDSTILL_INDEFINITELY,
    LIEINBED_LEFT,
    LIEINBED_RIGHT,
    RUNTIMED_ANIM,
    SITAT_DESK,
    STANDSTILL_TIMED,
    RUN_ANIM,
    RUNSEQUENCE_ANIM,
    STANDSTILL_LOOPED,
};

struct InteriorInfo_t {
    notsa::WEnumS8<eInteriorInfoType> Type;
    bool                              IsInUse;
    CVector                           Pos;
    CVector                           Dir;
    CEntity*                          EntityIgnoredCollision;
};
VALIDATE_SIZE(InteriorInfo_t, 0x20);
