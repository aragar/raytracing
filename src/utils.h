#ifndef RAYTRACING_UTILS_H
#define RAYTRACING_UTILS_H

#include <cmath>
#include <cstdlib>
#include <string>
#include <vector>

#include "constants.h"
#include "vector.h"

#define  COUNT_OF(arr) int((sizeof(arr)) / sizeof(arr[0]))

inline int SignOf(const double x) { return x > 1e-6 ? +1 : (x < -1e-6 ? -1 : 0); }
inline double Sqr(const double a) { return a*a; }

inline double ToRadians(const double angle) { return angle / 180. * PI; }
inline double ToDegrees(const double angle) { return angle / PI * 180.; }

inline double NearestInt(const double x) { return (int) floor(x + 0.5); }

inline double Min(const double a, const double b) { return a < b ? a : b; }
inline double Max(const double a, const double b) { return a > b ? a : b; }
inline double Max(unsigned a, unsigned b) { return a > b ? a : b; }
inline double Clamp(const double x, const double a, const double b) { return Min(Max(x, a), b); }

inline double Random() { return rand() / (double) RAND_MAX; }
inline double Random(double from, double to) { return (Random()*Max(from, to) + Min(from, to)); }

inline unsigned ConvertTo8Bit(double x) { return NearestInt(Clamp(x, 0.f, 1.f) * 255.f); }
inline unsigned ConvertTo8Bit_sRGB(double x) { return ConvertTo8Bit(x <= 0.0031308 ? x*12.92 : 1.055*pow(x, 1/2.4) - 0.055); }

inline bool IsZero(double a, double eps = 1e-6) { return (-eps <= a && a <= eps); }
inline bool IsZero(const Vector& a, double eps = 1e-6) { return (IsZero(a.x, eps) && IsZero(a.y, eps) && IsZero(a.z, eps)); }

inline bool AreEqual(double lhs, double rhs, double eps = 1e-6) { return IsZero(lhs - rhs, eps); }
inline bool AreEqual(const Vector& lhs, const Vector& rhs, double eps = 1e-6) { return IsZero(lhs - rhs, eps); }

int ToInt(const std::string& s);
double ToDouble(const std::string& s);

std::vector<std::string> Tokenize(const std::string& s);
std::vector<std::string> Split(const std::string& s, char separator);

std::string UpCaseString(std::string s);
std::string ExtensionUpper(const char* filename);
bool FileExists(const char* filename);

class FileRAII {
    FILE* held;
public:
    FileRAII(FILE* init): held(init) {}
    ~FileRAII() { if (held) fclose(held); held = NULL; }

    FileRAII(const FileRAII&) = delete;
    FileRAII& operator = (const FileRAII&) = delete;
    FileRAII(const FileRAII&&) = delete;
    FileRAII& operator = (const FileRAII&&) = delete;
};

/// in must be an unit vector. Returns vectors ray1 and ray2 such that
/// in, ray1 and ray2 form an orthonormal system in 3D (all vectors are
/// unit, and are mutually orthogonal)
void OrthonormalSystem(const Vector& in, Vector& outRay1, Vector& outRay2);

void GenerateDiscPoint(double& outX, double& outY);

#endif //RAYTRACING_UTILS_H
