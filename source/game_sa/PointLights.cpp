/*
    Plugin-SDK file
    Authors: GTA Community. See more here
    https://github.com/DK22Pac/plugin-sdk
    Do not delete this comment block. Respect others' work!
*/
#include "StdInc.h"

#include "PointLights.h"

void CPointLights::InjectHooks() {
    RH_ScopedClass(CPointLights);
    RH_ScopedCategoryGlobal();

    RH_ScopedInstall(Init, 0x6FFB40);
    RH_ScopedInstall(GenerateLightsAffectingObject, 0x6FFBB0);
    RH_ScopedInstall(RemoveLightsAffectingObject, 0x6FFFE0);
    RH_ScopedInstall(ProcessVerticalLineUsingCache, 0x6FFFF0);
    RH_ScopedInstall(AddLight, 0x7000E0);
}

// 0x6FFB40
// ASM Match: not measured
void CPointLights::Init()
{
    for (int32 C = 0; C < MAX_CACHED_MAP_READS; ++C)
    {
        aCachedMapReads[C] = CVector(0.0f, 0.0f, 0.0f);
        aCachedMapReadResults[C] = 0.0f;
        NextCachedValue = 0;
    }
}

// 0x6FFBB0
// ASM Match: not measured
float CPointLights::GenerateLightsAffectingObject(const CVector* pCoors, float* pPointLightsTotal, CEntity* pEntity)
{
    float DeltaX = 0.0f;
    float DeltaY = 0.0f;
    float DeltaZ = 0.0f;
    float Range = 0.0f;
    float Distance = 0.0f;
    float Multiplier = 0.0f;
    float DirX = 0.0f;
    float DirY = 0.0f;
    float DirZ = 0.0f;
    float RDistance = 0.0f;
    float DotPr = 0.0f;
    float LightMult = 1.0f;
    float DistScaled = 0.0f;

    // Go through the lights
    for (int32 Light = 0; Light < (int32)NumLights; Light++)
    {
        if (aLights[Light].m_nType != PLTYPE_ONLYFOGEFFECT_ALWAYS && aLights[Light].m_nType != PLTYPE_ONLYFOGEFFECT)
        {
            Range = aLights[Light].m_fRadius;
            DeltaX = aLights[Light].m_vecPosn.x - pCoors->x;
            if (DeltaX > -Range && DeltaX < Range)
            {
                DeltaY = aLights[Light].m_vecPosn.y - pCoors->y;
                if (DeltaY > -Range && DeltaY < Range)
                {
                    DeltaZ = aLights[Light].m_vecPosn.z - pCoors->z;
                    if (DeltaZ > -Range && DeltaZ < Range)
                    {
                        // Within range. Calculate distance
                        Distance = std::sqrt(DeltaX * DeltaX + DeltaY * DeltaY + DeltaZ * DeltaZ);
                        if (Distance < Range)
                        {
                            if (aLights[Light].m_nType == PLTYPE_ANTILIGHT)
                            {
                                LightMult *= (Distance / Range);
                            }
                            else
                            {
                                DistScaled = Distance / Range;
                                // save these point light values for ai use
                                if (pPointLightsTotal != nullptr)
                                {
                                    Multiplier = 1.0f - DistScaled;

                                    const float inv3 = 1.0f / 3.0f;
                                    *pPointLightsTotal += aLights[Light].m_fColorRed * Multiplier * inv3;
                                    *pPointLightsTotal += aLights[Light].m_fColorGreen * Multiplier * inv3;
                                    *pPointLightsTotal += aLights[Light].m_fColorBlue * Multiplier * inv3;
                                }

                                if (DistScaled < 0.5f)
                                {
                                    Multiplier = 1.0f;
                                }
                                else
                                {
                                    Multiplier = 1.0f - 2.0f * (DistScaled - 0.5f);
                                }
                                if (Distance != 0.0f)
                                {
                                    RDistance = 1.0f / Distance;
                                    DirX = DeltaX * RDistance;
                                    DirY = DeltaY * RDistance;
                                    DirZ = DeltaZ * RDistance;

                                    // Directional lights get their intensity decreased depending
                                    // on how far away from the direction light the object is.
                                    if (aLights[Light].m_nType == PLTYPE_DIRECTIONAL && aLights[Light].m_pEntityToLight != pEntity)
                                    {
                                        DotPr = -(DirX * aLights[Light].m_vecDirection.x +
                                                  DirY * aLights[Light].m_vecDirection.y +
                                                  DirZ * aLights[Light].m_vecDirection.z);
                                        Multiplier *= std::max(0.0f, (DotPr - 0.5f) * 2.0f);
                                    }

                                    // Construct this light and add it to the world for this guy
                                    if (Multiplier > 0.0f)
                                    {
                                        AddAnExtraDirectionalLight(Scene.m_pRpWorld, DirX, DirY, DirZ,
                                            aLights[Light].m_fColorRed * Multiplier,
                                            aLights[Light].m_fColorGreen * Multiplier,
                                            aLights[Light].m_fColorBlue * Multiplier);
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    return LightMult;
}

// 0x6FFE70
float CPointLights::GetLightMultiplier(const CVector* point) {
    return plugin::CallAndReturn<float, 0x6FFE70, const CVector*>(point);
}

// 0x6FFFE0
// ASM Match: not measured
void CPointLights::RemoveLightsAffectingObject()
{
    RemoveExtraDirectionalLights(Scene.m_pRpWorld);
}

// 0x6FFFF0
// ASM Match: not measured
bool CPointLights::ProcessVerticalLineUsingCache(CVector Coors, float* pResult)
{
    for (int32 C = 0; C < MAX_CACHED_MAP_READS; ++C)
    {
        if (aCachedMapReads[C] == Coors)
        {
            *pResult = aCachedMapReadResults[C];
            return true;
        }
    }

    CColPoint TestColPoint;
    CEntity* TestEntity = nullptr;

    // Couldn't find cached value. Read again.
    if (CWorld::ProcessVerticalLine(Coors, Coors.z - 20.0f, TestColPoint, TestEntity, true, false, false, false))
    {
        aCachedMapReads[NextCachedValue] = Coors;
        aCachedMapReadResults[NextCachedValue] = TestColPoint.GetPosition().z;
        NextCachedValue = (NextCachedValue + 1) % MAX_CACHED_MAP_READS;
        *pResult = TestColPoint.GetPosition().z;

        return true;
    }

    return false;
}

// 0x7000E0
// ASM Match: not measured
void CPointLights::AddLight(uint8 Type, CVector Coors, CVector Dir, float Range, float Red, float Green, float Blue, uint8 FogEffect, bool bCastsShadowFromPlayerCarAndPed, CEntity* pArgCastingEntity)
{
    constexpr float MAX_DIST_POINT_LIGHT = 15.0f;
    float maxDist = MAX_DIST_POINT_LIGHT + Range;

    // Test the distance to the camera
    float DiffX = Coors.x - TheCamera.GetPosition().x;
    if (DiffX < maxDist && DiffX > -maxDist)
    {
        float DiffY = Coors.y - TheCamera.GetPosition().y;
        if (DiffY < maxDist && DiffY > -maxDist)
        {
            if (NumLights < MAX_POINT_LIGHTS)
            {
                float DiffZ = Coors.z - TheCamera.GetPosition().z;
                float Distance = std::sqrt(DiffX * DiffX + DiffY * DiffY + DiffZ * DiffZ);
                if (Distance < maxDist)
                {
                    auto& light = aLights[NumLights];
                    light.m_nType = static_cast<ePointLightType>(Type);
                    light.m_nFogType = FogEffect;
                    light.m_vecPosn = Coors;
                    light.m_vecDirection = Dir;
                    light.m_fRadius = Range;
                    light.m_bGenerateShadows = bCastsShadowFromPlayerCarAndPed;
                    light.m_pEntityToLight = pArgCastingEntity;

                    if (Distance < 0.75f * maxDist)
                    {
                        light.m_fColorRed = Red;
                        light.m_fColorGreen = Green;
                        light.m_fColorBlue = Blue;
                    }
                    else
                    {
                        float Mult = 1.0f - ((Distance / maxDist) - 0.75f) * 4.0f;
                        light.m_fColorRed = Red * Mult;
                        light.m_fColorGreen = Green * Mult;
                        light.m_fColorBlue = Blue * Mult;
                    }
                    NumLights++;
                }
            }
        }
    }
}

// 0x7002D0
void CPointLights::RenderFogEffect() {
    ZoneScoped;

    plugin::Call<0x7002D0>();
}
