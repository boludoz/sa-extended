#pragma once

// librw has no include guard on rw.h, and only declares rw::d3d::d3ddevice
// when d3d9.h was seen first.
#ifdef RW_D3D9
#include <d3d9.h>
#endif
#include <rw.h>
