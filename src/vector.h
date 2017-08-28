#ifndef RAYTRACING_VECTOR_H
#define RAYTRACING_VECTOR_H

#include <cmath>

struct Vector
{
    double x, y, z;

    Vector() = default;
    Vector(double x, double y, double z);

    void Set(double x, double y, double z);

    void MakeZero() { Set(0., 0., 0.); }
    void SetLength(double newLength) { Scale(newLength/Length()); }

    double Length() const { return sqrtf(x*x + y*y + z*z); }
    double LengthSqr() const { return (x*x + y*y + z*z); }

    void Scale(const double multiplier);
    void Normalize() { Scale(1.f / Length()); }

    void operator+=(const Vector& other);
    void operator*=(double multiplier) { Scale(multiplier); }
    void operator/=(double divider) { Scale(1.f/divider); }
};

inline Vector operator+(const Vector& lhs, const Vector& rhs)       { return { lhs.x + rhs.x,     lhs.y + rhs.y,        lhs.z + rhs.z       }; }
inline Vector operator-(const Vector& lhs, const Vector& rhs)       { return { lhs.x - rhs.x,     lhs.y - rhs.y,        lhs.z - rhs.z       }; }
inline Vector operator-(const Vector& lhs)                          { return {-lhs.x,            -lhs.y,               -lhs.z               }; }
inline double operator*(const Vector& lhs, const Vector& rhs)       { return ( lhs.x*rhs.x + lhs.y*rhs.y + lhs.z*rhs.z                      ); }
inline Vector operator*(const Vector& lhs, const double multiplier) { return { lhs.x*multiplier,  lhs.y*multiplier,     lhs.z*multiplier    }; }
inline Vector operator*(const double multiplier, const Vector& rhs) { return { rhs.x*multiplier,  rhs.y*multiplier,     rhs.z*multiplier    }; }
inline Vector operator/(const Vector& lhs, const double divider)    { return { lhs.x/divider,     lhs.y/divider,        lhs.z/divider       }; }
inline Vector operator^(const Vector& lhs, const Vector& rhs)       { return { lhs.y*rhs.z - lhs.z*rhs.y, lhs.z*rhs.x - lhs.x*rhs.z, lhs.x*rhs.y - lhs.y*rhs.x}; }

inline double Dot(const Vector& lhs, const Vector& rhs) { return (lhs.x*rhs.x + lhs.y*rhs.y + lhs.z*rhs.z); }
inline Vector Normalize(Vector vector) { vector.Normalize(); return vector; }

Vector Reflect(Vector in, const Vector& norm);

#endif //RAYTRACING_VECTOR_H
