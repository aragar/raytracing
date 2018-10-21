#ifndef RAYTRACING_VECTOR_H
#define RAYTRACING_VECTOR_H

#include <cmath>
#include <cstdio>

struct Vector
{
    union
    {
        struct
        {
            double x, y, z;
        };
        double v[3];
    };

    Vector() { MakeZero(); };
    Vector(double x, double y, double z);

    void Set(double x, double y, double z);
    void MakeZero() { Set(0., 0., 0.); }

    void SetLength(double newLength) { Scale(newLength/Length()); }
    double Length() const { return sqrt(x*x + y*y + z*z); }
    double LengthSqr() const { return (x*x + y*y + z*z); }

    void Scale(const double multiplier);
    void Normalize() { Scale(1.f / Length()); }

    void operator+=(const Vector& other);
    void operator-=(const Vector& other);
    void operator*=(double multiplier) { Scale(multiplier); }
    void operator/=(double divider) { Scale(1.f/divider); }

    int MaxDimension() const;

    inline double& operator[](int index) { return v[index]; }
    inline const double& operator[](int index) const { return v[index]; }

    void print() const { printf("(%.9lf, %.9lf, %.9lf)", x, y, z); }

    static Vector GetZero() { return {0, 0, 0}; }
};

inline double Dot(const Vector& lhs, const Vector& rhs) { return (lhs.x*rhs.x + lhs.y*rhs.y + lhs.z*rhs.z); }
inline double Cross(const Vector& lhs, const Vector& rhs) { return(lhs.x*rhs.y + lhs.y*rhs.z + lhs.z*rhs.x - lhs.x*rhs.z - lhs.y*rhs.x - lhs.z*rhs.y); }
inline Vector Normalize(Vector vector) { vector.Normalize(); return vector; }

inline Vector operator+(const Vector& lhs, const Vector& rhs)       { return { lhs.x + rhs.x,     lhs.y + rhs.y,        lhs.z + rhs.z       }; }
inline Vector operator-(const Vector& lhs, const Vector& rhs)       { return { lhs.x - rhs.x,     lhs.y - rhs.y,        lhs.z - rhs.z       }; }
inline Vector operator-(const Vector& lhs)                          { return {-lhs.x,            -lhs.y,               -lhs.z               }; }
inline double operator*(const Vector& lhs, const Vector& rhs)       { return Dot(lhs, rhs);                                                    }
inline Vector operator*(const Vector& lhs, const double multiplier) { return { lhs.x*multiplier,  lhs.y*multiplier,     lhs.z*multiplier    }; }
inline Vector operator*(const double multiplier, const Vector& rhs) { return { rhs.x*multiplier,  rhs.y*multiplier,     rhs.z*multiplier    }; }
inline Vector operator/(const Vector& lhs, const double divider)    { return { lhs.x/divider,     lhs.y/divider,        lhs.z/divider       }; }
inline Vector operator^(const Vector& lhs, const Vector& rhs)       { return { lhs.y*rhs.z - lhs.z*rhs.y, lhs.z*rhs.x - lhs.x*rhs.z, lhs.x*rhs.y - lhs.y*rhs.x}; }

Vector Reflect(Vector in, const Vector& norm);
Vector Refract(const Vector& in, const Vector& norm, double inOutRatio);
Vector Faceforward(const Vector& ray, const Vector& norm);
double FresnelRatio(const Vector& in, const Vector& norm, double inOutRatio);

#endif //RAYTRACING_VECTOR_H
