#ifndef RAYTRACING_UTILS_H
#define RAYTRACING_UTILS_H

#include <cstdlib>
#include <cmath>
#include "constants.h"

inline float SignOf(const float x) { return x > 0.f ? +1.f : -1.f; }
inline float Sqr(const float a) { return a*a; }

inline float ToRadians(const float angle) { return angle / 180.f * PI; }
inline float ToDegrees(const float angle) { return angle / PI * 180.f; }

inline float NearestInt(const float x) { return (int) floor(x + 0.5f); }

inline float Random() { return rand() / (float) RAND_MAX; }

inline float Min(const float a, const float b) { return a < b ? a : b; }
inline float Max(const float a, const float b) { return a > b ? a : b; }
inline float Clamp(const float x, const float a, const float b) { return Min(Max(x, a), b); }

inline unsigned ConvertTo8Bit(float x) { return NearestInt(Clamp(x, 0.f, 1.f) * 255.f); }

#endif //RAYTRACING_UTILS_H
