#pragma once
#include <common.h>

class CMaths {
public:
    static void InjectHooks();
    static void InitMathsTables();

    // Originally named `SinTabel` with argument named `Arg` which sucks, so replaced with custom name.
    // NOTE: For SinTable[x + 64] use GetCosFast
    static float GetSinFast(float rad);

    // Originally named `CosTabel` with argument named `Arg`, same story as above
    static float GetCosFast(float rad);

    // Same semantics as the original CMaths (calineva math.hpp): plain CRT float calls and
    // ternary min/max, so NaN/inf/zero cases behave exactly as in the game.
    static float Sqrt(float v) { return sqrtf(v); }
    static float RecipSqrt(float n, float d) { return n / sqrtf(d); }
    static float RecipSqrt(float d) { return RecipSqrt(1.0f, d); }
    static float Sin(float f) { return sinf(f); }
    static float Cos(float f) { return cosf(f); }
    static float ASin(float f) { return asinf(f); }
    static float ACos(float f) { return acosf(f); }
    static float ATan(float x) { return atanf(x); }
    static float ATan2(float y, float x) { return atan2f(y, x); }
    static float Pow(float x, float y) { return powf(x, y); }
    static float Floor(float f) { return floorf(f); }
    static float Ceil(float f) { return ceilf(f); }
    static float Abs(float f) { return fabsf(f); }
    static float Min(float v1, float v2) { return v1 < v2 ? v1 : v2; }
    static float Max(float v1, float v2) { return v1 > v2 ? v1 : v2; }
    static float Clamp(float val, float minval, float maxval) { return Max(Min(val, maxval), minval); }
    static bool  Eq(float val1, float val2, float epsilon) { return Abs(val1 - val2) <= epsilon; }
};

// calineva math.hpp macros (V prefix avoids clashing with windows.h)
#ifndef VMAX
#define VMAX(a, b) (((a) > (b)) ? (a) : (b))
#define VMIN(a, b) (((a) < (b)) ? (a) : (b))
#define VCLAMP(lo, hi, v) VMAX(VMIN(v, hi), lo)
#define VABS(x) ((x) < 0 ? -(x) : (x))
#endif
