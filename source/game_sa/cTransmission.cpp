#include "StdInc.h"

#include "cTransmission.h"
#include "CarCtrl.h"
#include "Enums/eVehicleHandlingFlags.h"

void cTransmission::InjectHooks()
{
    RH_ScopedClass(cTransmission);
    RH_ScopedCategoryGlobal();

    RH_ScopedInstall(DisplayGearRatios, 0x6D0590);
    RH_ScopedInstall(InitGearRatios, 0x6D0460);
    RH_ScopedInstall(CalculateGearForSimpleCar, 0x6D0530);
    RH_ScopedInstall(CalculateDriveAcceleration, 0x6D05E0);
}

const float CHANGE_DOWN_RATIO = 0.42f;
const float CHANGE_UP_RATIO = 0.6667f;

// 0x6D0450
// cTransmission::cTransmission() is defaulted in cTransmission.h

// 0x6D0460
void cTransmission::InitGearRatios()
{
    m_aGears.fill({});
    static auto& pGearRatio1 = StaticRef<tTransmissionGear*>(0xC1CB34);
    static auto& pGearRatio0 = StaticRef<tTransmissionGear*>(0xC1CB30);

    float fIntermediate = 1.0f / (float)m_nNumberOfGears;
    static float FIRST_GEAR_EXTRA_VEL = 0.5f;
    float fVelocityLostToFirstGear = FIRST_GEAR_EXTRA_VEL * m_MaxVelocity * fIntermediate;
    float fMaxVelocityForGears = m_MaxVelocity - fVelocityLostToFirstGear;

    for (uint8 i = 1; i <= m_nNumberOfGears; i++)
    {
        pGearRatio1 = &m_aGears[i];
        pGearRatio0 = &m_aGears[i - 1];

        pGearRatio1->MaxVelocity = (float)i * fMaxVelocityForGears * fIntermediate + fVelocityLostToFirstGear;

        float fGearFraction = pGearRatio1->MaxVelocity - pGearRatio0->MaxVelocity;

        if (i < m_nNumberOfGears)
        {
            m_aGears[i + 1].ChangeDownVelocity = fGearFraction * CHANGE_DOWN_RATIO + pGearRatio0->MaxVelocity;
            pGearRatio1->ChangeUpVelocity = fGearFraction * CHANGE_UP_RATIO + pGearRatio0->MaxVelocity;
        }
        else
        {
            pGearRatio1->ChangeUpVelocity = m_MaxVelocity;
        }
    }

    m_aGears[0].MaxVelocity = m_MaxReverseVelocity;
    m_aGears[0].ChangeUpVelocity = -0.01f;
    m_aGears[0].ChangeDownVelocity = m_MaxReverseVelocity;
    m_aGears[1].ChangeDownVelocity = -0.01f;
}

// 0x6D05E0
float cTransmission::CalculateDriveAcceleration(const float& fThrottleSetting, uint8& nCurrentGear, float& fGearChangeCount,
    const float& v, float* pEngineRevs, float* pEngineForce, uint8 nDriveWheelsOnGround, uint8 nFasterCheat)
{
    static float& fAcceleration = StaticRef<float>(0xC1CB38);
    static float& fCheat        = StaticRef<float>(0xC1CB3C);
    static float& fVelocity     = StaticRef<float>(0xC1CB40);

    fVelocity = v;
    if (fVelocity < m_MaxReverseVelocity)
    {
        return 0.0f;
    }

    if (fVelocity > m_MaxVelocity)
    {
        return 0.0f;
    }

    m_Velocity = fVelocity;

    if (fVelocity > m_aGears[nCurrentGear].ChangeUpVelocity)
    {
        if (nCurrentGear != 0 || fThrottleSetting > 0.0f)
        {
            nCurrentGear++;
            return CalculateDriveAcceleration(fThrottleSetting, nCurrentGear, fGearChangeCount, fVelocity, 0, 0, 0, 0);
        }
    }
    else if (fVelocity < m_aGears[nCurrentGear].ChangeDownVelocity)
    {
        if (nCurrentGear > 0 && (nCurrentGear != 1 || fThrottleSetting < 0.0f))
        {
            nCurrentGear--;
            return CalculateDriveAcceleration(fThrottleSetting, nCurrentGear, fGearChangeCount, fVelocity, 0, 0, 0, 0);
        }
    }

    float fMult;
    float fScale;

    if (m_nNumberOfGears == 1)
    {
        fScale = 1.0f;
        fMult = 1.0f;
    }
    else if (nCurrentGear < 1)
    {
        fScale = 1.0f;
        fMult = 4.5f;
    }
    else
    {
        fMult = 1.0f - (((float)nCurrentGear - 1.0f) / ((float)m_nNumberOfGears - 1.0f));
        if (m_handlingFlags & VEHICLE_HANDLING_1G_BOOST)
        {
            fMult = 1.0f + fMult * fMult * 5.0f;
        }
        else if (m_handlingFlags & VEHICLE_HANDLING_2G_BOOST)
        {
            fMult = 1.0f + fMult * fMult * 4.0f;
        }
        else
        {
            fMult = 1.0f + fMult * fMult * 3.0f;
        }

        fScale = 1.0f;
    }

    fCheat = 1.0f;
    if (nFasterCheat == 1)
    {
        fCheat = TRANSMISSION_AI_CHEAT_MULT;
    }
    else if (nFasterCheat == 2)
    {
        fScale = TRANSMISSION_NITROS_MULT;
    }
    fAcceleration = CTimer::GetTimeStep() * (fThrottleSetting * (0.4f * ((fMult * (fCheat * m_EngineAcceleration)) * fScale)));

    float fVal = 0.0f;

    if (pEngineRevs && pEngineForce)
    {
        if (!nDriveWheelsOnGround)
        {
            float fNewRevs = (std::abs(fThrottleSetting) / m_EngineInertia) * CTimer::GetTimeStep() * TRANSMISSION_FREE_ACCELERATION + *pEngineRevs;
            if (1.0f < fNewRevs)
            {
                fNewRevs = 1.0f;
            }
            *pEngineRevs = fNewRevs;
            *pEngineForce = 0.1f;
        }
        else
        {
            if (nCurrentGear == 0)
            {
                fVal = ((m_MaxVelocity / (float)m_nNumberOfGears) * (1.0f - CHANGE_UP_RATIO) - fVelocity) /
                       ((m_MaxVelocity / (float)m_nNumberOfGears) * (1.0f - CHANGE_UP_RATIO) - m_aGears[0].ChangeDownVelocity);
            }
            else if (nCurrentGear == 1)
            {
                fVal = (fVelocity + (m_MaxVelocity / (float)m_nNumberOfGears) * (1.0f - CHANGE_UP_RATIO)) /
                       (m_aGears[1].ChangeUpVelocity + (m_MaxVelocity / (float)m_nNumberOfGears) * (1.0f - CHANGE_UP_RATIO));
            }
            else
            {
                fVal = (fVelocity - m_aGears[nCurrentGear].ChangeDownVelocity) /
                       (m_aGears[nCurrentGear].ChangeUpVelocity - m_aGears[nCurrentGear].ChangeDownVelocity);
            }

            float fTempCalc = fVal - *pEngineRevs;
            if (nFasterCheat == 1)
            {
                fTempCalc *= TRANSMISSION_AI_CHEAT_INERTIA_MULT;
            }
            else if (nFasterCheat == 2)
            {
                fTempCalc *= TRANSMISSION_NITROS_INERTIA_MULT;
            }
            fTempCalc = 1.0f - (fTempCalc * m_EngineInertia);
            if (!(1.0f < fTempCalc) && (0.1f > fTempCalc))
            {
                fTempCalc = 0.1f;
            }
            else
            {
                fTempCalc = std::min(1.0f, fTempCalc);
            }
            fTempCalc = (1.0f - TRANSMISSION_SMOOTHER_FRAC) * fTempCalc + TRANSMISSION_SMOOTHER_FRAC * *pEngineForce;
            fAcceleration *= fTempCalc;
            *pEngineForce = fTempCalc;
            *pEngineRevs = fVal;
        }
    }

    if (m_aGears[nCurrentGear].MaxVelocity < 0.0f && fVelocity < m_aGears[nCurrentGear].MaxVelocity * fCheat)
    {
        fVal = m_aGears[nCurrentGear].MaxVelocity * fCheat - fVelocity;
    }
    else if (m_aGears[nCurrentGear].MaxVelocity > 0.0f && fVelocity > m_aGears[nCurrentGear].MaxVelocity * fCheat)
    {
        fVal = fVelocity - m_aGears[nCurrentGear].MaxVelocity * fCheat;
    }
    else
    {
        return fAcceleration;
    }

    static float MAX_SPEED_LIMIT_RANGE = 0.05f;
    fVal /= MAX_SPEED_LIMIT_RANGE;
    if (1.0f < fVal)
    {
        fVal = 1.0f;
    }
    fAcceleration *= (1.0f - fVal);
    return fAcceleration;
}

// 0x6D0530
void cTransmission::CalculateGearForSimpleCar(float CurrentVel, uint8& nCurrentGear)
{
    m_Velocity = CurrentVel;
    if (CurrentVel > m_aGears[nCurrentGear].ChangeUpVelocity)
    {
        nCurrentGear = (m_nNumberOfGears >= nCurrentGear + 1) ? (nCurrentGear + 1) : m_nNumberOfGears;
    }
    else if (CurrentVel < m_aGears[nCurrentGear].ChangeDownVelocity)
    {
        nCurrentGear = std::max<uint8>(0, nCurrentGear - 1);
    }
}

float unknown = 0.277778f / 50.0f; // flt_853CE0

// 0x6D0590
void cTransmission::DisplayGearRatios()
{
    for (int i = 0; i < 6; i++)
    {
        printf("%d, max v = %3.2f, up at = %3.2f, down at = %3.2f\n",
            i,
            1.0f / unknown * m_aGears[i].MaxVelocity,
            1.0f / unknown * m_aGears[i].ChangeUpVelocity,
            1.0f / unknown * m_aGears[i].ChangeDownVelocity);
    }
}
