#include "StdInc.h"
#include "Curves.h"

void CCurves::InjectHooks() {
    RH_ScopedClass(CCurves);
    RH_ScopedCategoryGlobal();

    RH_ScopedGlobalInstall(TestCurves, 0x43C600, {.locked=true});
    RH_ScopedGlobalInstall(DistForLineToCrossOtherLine, 0x43C610);
    RH_ScopedGlobalInstall(CalcSpeedVariationInBend, 0x43C660);
    RH_ScopedGlobalInstall(CalcSpeedScaleFactor, 0x43C710);
    RH_ScopedGlobalInstall(CalcCorrectedDist, 0x43C880);
    RH_ScopedGlobalInstall(CalcCurvePoint, 0x43C900);
}

// 0x43C610
float CCurves::DistForLineToCrossOtherLine(float LineBaseX, float LineBaseY, float LineDirX, float LineDirY, float OtherLineBaseX, float OtherLineBaseY, float OtherLineDirX, float OtherLineDirY) {
    float Dir = LineDirX * OtherLineDirY - LineDirY * OtherLineDirX;

    if (Dir == 0.0f) {
        return -1.0f; // Lines are parallel, no intersection
    }

    float Dist           = (LineBaseX - OtherLineBaseX) * OtherLineDirY - (LineBaseY - OtherLineBaseY) * OtherLineDirX;
    float DistOfCrossing = -Dist / Dir;

    return DistOfCrossing;
}

// 0x43C660
float CCurves::CalcSpeedVariationInBend(
    const CVector& startCoors,
    const CVector& endCoors,
    float            StartDirX,
    float          StartDirY,
    float          EndDirX,
    float          EndDirY
) {
    float ReturnVal  = 0.0f;
    float DotProduct = StartDirX * EndDirX + StartDirY * EndDirY;

    if (DotProduct <= 0.0f) {
        // If the dot product is <= 0, return a constant value (1/3)
        ReturnVal = 1.0f / 3.0f;
        return ReturnVal;
    }

    if (DotProduct > 0.7f) {
        // Calculate the distance from the start point to the mathematical line defined by the end point and direction
        float DistToLine =
            CCollision::DistToMathematicalLine2D(endCoors.x, endCoors.y, EndDirX, EndDirY, startCoors.x, startCoors.y);

        // Calculate the straight-line distance between the start and end points
        float StraightDist = (startCoors - endCoors).Magnitude2D();

        // Normalize the distance to the line by the straight-line distance
        ReturnVal = (DistToLine / StraightDist) * (1.0f / 3.0f);
        return ReturnVal;
    }

    // If the dot product is <= 0.7, interpolate the return value
    ReturnVal = (1.0f - (DotProduct / 0.7f)) * (1.0f / 3.0f);

    return ReturnVal;
}

// 0x43C710
float CCurves::CalcSpeedScaleFactor(
    const CVector& startCoors,
    const CVector& endCoors,
    float            StartDirX,
    float            StartDirY,
    float            EndDirX,
    float            EndDirY
) {
    float SpeedVariation = CalcSpeedVariationInBend(startCoors, endCoors, StartDirX, StartDirY, EndDirX, EndDirY);

    float DistToPoint1   = DistForLineToCrossOtherLine(
        startCoors.x, startCoors.y, StartDirX, StartDirY, endCoors.x, endCoors.y, EndDirX, EndDirY
    );

    float DistToPoint2 = DistForLineToCrossOtherLine(
        endCoors.x, endCoors.y, -EndDirX, -EndDirY, startCoors.x, startCoors.y, StartDirX, StartDirY
    );

    // NOLINTBEGIN(cppcoreguidelines-init-variables)
    float StraightDist1;
    float StraightDist2;
    float BendDist;
    float BendDist_Time;
    float BendDistOneSegment;
    float TotalDist_Time;
    // NOLINTEND(cppcoreguidelines-init-variables)

    if (DistToPoint1 > 0.0f && DistToPoint2 > 0.0f) {
        BendDistOneSegment = std::min(DistToPoint1, DistToPoint2);
        BendDistOneSegment = std::min(5.0f, BendDistOneSegment);

        StraightDist1      = DistToPoint1 - BendDistOneSegment;
        StraightDist2      = DistToPoint2 - BendDistOneSegment;

        BendDist           = 2.0f * BendDistOneSegment;

        TotalDist_Time     = BendDist;
    } else {
        BendDist       = (startCoors - endCoors).Magnitude2D();
        BendDist_Time  = 1.0f - SpeedVariation;

        StraightDist1  = 0.0f;
        StraightDist2  = 0.0f;

        TotalDist_Time = BendDist / BendDist_Time;
    }

    return TotalDist_Time + StraightDist1 + StraightDist2;
}

// 0x43C880
float CCurves::CalcCorrectedDist(float Current, float Total, float SpeedVariation, float* pInterPol) {
    if (Total < 0.00001f) // Epsilon to avoid division by zero
    {
        *pInterPol = 0.5f;
        return 0.0f;
    }

    *pInterPol        = 0.5f - (std::cosf(PI * (Current / Total)) * 0.5f);

    float AverageSpeed  = std::sinf((Current / Total) * TWO_PI);
    float CorrectedDist = AverageSpeed * (Total * (1.0f / TWO_PI)) * SpeedVariation + ((1.0f - (SpeedVariation + SpeedVariation) + 1.0f) * 0.5f) * Current;

    return CorrectedDist;
}

// 0x43C900
void CCurves::CalcCurvePoint(const CVector& startCoors, const CVector& endCoors, const CVector& startDir, const CVector& endDir, float Time, int TraverselTimeInMillis, CVector& resultCoor, CVector& resultSpeed) {
    if (!(Time > 0.0f)) {
        Time = 0.0f;
    }
    if (!(Time < 1.0f)) {
        Time = 1.0f;
    }
    float     BendDist;
    float     BendDist_Time;
    float     CurrentDist_Time;
    float     Interpol;
    float     StraightDist2;
    float     StraightDist1;
    float     TotalDist_Time;
    float     OurTime;
    float     BendDistOneSegment;
    CVector CoorsOnLine1;
    CVector CoorsOnLine2;

    float SpeedVariation =
        CCurves::CalcSpeedVariationInBend(startCoors, endCoors, startDir.x, startDir.y, endDir.x, endDir.y);

    float DistToPoint1 = DistForLineToCrossOtherLine(
        startCoors.x, startCoors.y, startDir.x, startDir.y, endCoors.x, endCoors.y, endDir.x, endDir.y
    );

    float DistToPoint2 = DistForLineToCrossOtherLine(
        endCoors.x, endCoors.y, -endDir.x, -endDir.y, startCoors.x, startCoors.y, startDir.x, startDir.y
    );

    if (DistToPoint1 > 0.0f && DistToPoint2 > 0.0f) {
        BendDistOneSegment = std::min(DistToPoint1, DistToPoint2);
        BendDistOneSegment = std::min(5.0f, BendDistOneSegment);

        StraightDist1      = DistToPoint1 - BendDistOneSegment;
        StraightDist2      = DistToPoint2 - BendDistOneSegment;

        BendDist           = 2.0f * BendDistOneSegment;

        TotalDist_Time     = BendDist + StraightDist2 + StraightDist1;
        BendDist_Time      = TotalDist_Time * Time;

        if (BendDist_Time < StraightDist1) {
            resultCoor = startCoors + startDir * BendDist_Time;
        } else if (BendDist_Time < StraightDist1 + BendDist) {
            float BendInter          = (BendDist_Time - StraightDist1) / BendDist;

            CVector BendStartCoors = startCoors + startDir * StraightDist1;
            CVector BendEndCoors   = endCoors - endDir * StraightDist2;

            BendStartCoors         = BendStartCoors + startDir * BendInter * BendDistOneSegment;
            BendEndCoors           = BendEndCoors - endDir * (1.0f - BendInter) * BendDistOneSegment;

            resultCoor             = BendStartCoors * (1.0f - BendInter) + BendEndCoors * BendInter;
        } else {
            BendDist_Time = BendDist_Time - TotalDist_Time;
            resultCoor    = endCoors + endDir * BendDist_Time;
        }
    } else {
        BendDist         = (startCoors - endCoors).Magnitude2D();

        Interpol         = BendDist / (1.0f - SpeedVariation);

        CurrentDist_Time = CalcCorrectedDist(Time * Interpol, Interpol, SpeedVariation, &OurTime);

        CoorsOnLine1     = startCoors + startDir * CurrentDist_Time;
        CoorsOnLine2     = endCoors + endDir * (CurrentDist_Time - BendDist);

        resultCoor       = CoorsOnLine1 * (1.0f - OurTime) + CoorsOnLine2 * OurTime;

        StraightDist1    = 0.0f;
        StraightDist2    = 0.0f;
        TotalDist_Time   = BendDist + StraightDist2 + StraightDist1;
    }

    resultSpeed.x = (TotalDist_Time * (startDir.x * (1.0f - Time) + endDir.x * Time)) / (static_cast<float>(TraverselTimeInMillis) * 0.001f);
    resultSpeed.y = (TotalDist_Time * (startDir.y * (1.0f - Time) + endDir.y * Time)) / (static_cast<float>(TraverselTimeInMillis) * 0.001f);
    resultSpeed.z = 0.0f;
}

// unused
// 0x43C600
void CCurves::TestCurves() {
}
