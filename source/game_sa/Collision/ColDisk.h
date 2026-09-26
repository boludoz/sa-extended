/*
    Plugin-SDK file
    Authors: GTA Community. See more here
    https://github.com/DK22Pac/plugin-sdk
    Do not delete this comment block. Respect others' work!
*/
#pragma once

#include "Vector.h"
#include "ColSurface.h"

class CColDisk : public CColSphere {
public:
    CVector     m_vecThickness{};
    float       m_fThickness{};

public:
    // 0x40FD50
    // ASM Match: not measured
    void Set(float fRadius, const CVector& vecCentre, const CVector& vecThickness, float fThickness, eSurfaceType nSurfaceType, uint8 nPieceType, tColLighting nLighting) {
        CColSphere::Set(fRadius, vecCentre, nSurfaceType, nPieceType, nLighting);
        m_vecThickness = vecThickness;
        m_fThickness = fThickness;
    }
};
VALIDATE_SIZE(CColDisk, 0x24);
