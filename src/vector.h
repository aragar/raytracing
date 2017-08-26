#ifndef RAYTRACING_VECTOR_H
#define RAYTRACING_VECTOR_H

#include <cmath>

struct Vector
{
    float x, y, z;

    Vector() = default;
    Vector(float x, float y, float z);

    void Set(float x, float y, float z);

    void MakeZero() { Set(0.f, 0.f, 0.f); }
    void SetLength(float newLength) { Scale(newLength/Length()); }

    float Length() const { return sqrtf(x*x + y*y + z*z); }
    float LengthSqr() const { return (x*x + y*y + z*z); }

    void Scale(const float multiplier);
    void Normalize() { Scale(1.f / Length()); }

    void operator*=(float multiplier) { Scale(multiplier); }
    void operator/=(float divider) { Scale(1.f/divider); }
};

inline Vector operator+(const Vector& lhs, const Vector& rhs) { return {lhs.x + rhs.x, lhs.y + rhs.y, lhs.z + rhs.z}; }
inline Vector operator-(const Vector& lhs, const Vector& rhs) { return {lhs.x - rhs.x, lhs.y - rhs.y, lhs.z - rhs.z}; }
inline Vector operator-(const Vector& lhs) { return {-lhs.x, -lhs.y, -lhs.z}; }
inline double operator*(const Vector& lhs, const Vector& rhs) { return (lhs.x*rhs.x + lhs.y*rhs.y + lhs.z*rhs.z); }
inline Vector operator*(const Vector& lhs, const float multiplier) { return {lhs.x*multiplier, lhs.y*multiplier, lhs.z*multiplier}; }
inline Vector operator*(const float multiplier, const Vector& rhs) { return {rhs.x*multiplier, rhs.y*multiplier, rhs.z*multiplier}; }
inline Vector operator/(const Vector& lhs, const float divider) { return {lhs.x/divider, lhs.y/divider, lhs.z/divider}; }
inline Vector operator^(const Vector& lhs, const Vector& rhs) { return {lhs.y*rhs.z - lhs.z*rhs.y, lhs.z*rhs.x - lhs.x*rhs.z, lhs.x*rhs.y - lhs.y*rhs.x}; }

inline double Dot(const Vector& lhs, const Vector& rhs) { return (lhs.x*rhs.x + lhs.y*rhs.y + lhs.z*rhs.z); }

#endif //RAYTRACING_VECTOR_H
