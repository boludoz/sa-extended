#pragma once

#include "rtquat.h"

// librw has no slerp cache; keep the endpoints and let rw::slerp do the work.
struct RtQuatSlerpCache
{
    RtQuat from;
    RtQuat to;
};

inline void RtQuatSetupSlerpCache(const RtQuat* from, const RtQuat* to, RtQuatSlerpCache* cache) {
    cache->from = *from;
    cache->to   = *to;
}

inline void RtQuatSlerp(RtQuat* out, const RtQuat* from, const RtQuat* to, RwReal t, const RtQuatSlerpCache* cache) {
    const auto q = rw::slerp(*reinterpret_cast<const rw::Quat*>(&cache->from), *reinterpret_cast<const rw::Quat*>(&cache->to), t);
    *reinterpret_cast<rw::Quat*>(out) = q;
}
