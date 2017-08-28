#ifndef RAYTRACING_UTILS_H
#define RAYTRACING_UTILS_H

#include <cstdlib>
#include <cmath>
#include "constants.h"

inline double SignOf(const double x) { return x > 0. ? +1. : -1.; }
inline double Sqr(const double a) { return a*a; }

inline double ToRadians(const double angle) { return angle / 180. * PI; }
inline double ToDegrees(const double angle) { return angle / PI * 180.; }

inline double NearestInt(const double x) { return (int) floor(x + 0.5); }

inline double Random() { return rand() / (double) RAND_MAX; }

inline double Min(const double a, const double b) { return a < b ? a : b; }
inline double Max(const double a, const double b) { return a > b ? a : b; }
inline double Clamp(const double x, const double a, const double b) { return Min(Max(x, a), b); }

inline unsigned ConvertTo8Bit(double x) { return NearestInt(Clamp(x, 0.f, 1.f) * 255.f); }

#endif //RAYTRACING_UTILS_H
