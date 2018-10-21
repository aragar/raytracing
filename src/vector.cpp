#include "vector.h"
#include "utils.h"

Vector::Vector(double x, double y, double z)
: x(x),
  y(y),
  z(z)
{}

void Vector::Set(double x, double y, double z)
{
    this->x = x;
    this->y = y;
    this->z = z;
}

void Vector::Scale(const double multiplier)
{
    x *= multiplier;
    y *= multiplier;
    z *= multiplier;
}

void Vector::operator+=(const Vector& other)
{
    x += other.x;
    y += other.y;
    z += other.z;
}

int Vector::MaxDimension() const
{
    double maxVal = std::abs(x);
    int maxDim = 0;
    if ( std::abs(y) > maxVal )
    {
        maxVal = std::abs(y);
        maxDim = 1;
    }
    if ( std::abs(z) > maxVal )
    {
        maxVal = std::abs(z);
        maxDim = 2;
    }

    return maxDim;
}

void Vector::operator-=(const Vector& other)
{
    x -= other.x;
    y -= other.y;
    z -= other.z;
}

Vector Reflect(Vector in, const Vector& norm)
{
    in.Normalize();
    in -= 2 * norm * Dot(norm, in);
    in.Normalize();
    return in;
}

Vector Faceforward(const Vector& ray, const Vector& norm)
{
    if (Dot(ray, norm) < 0)
        return norm;

    return -norm;
}

Vector Refract(const Vector& in, const Vector& norm, double inOutRatio)
{
    double NdotI = Dot(in, norm);
    double k = 1. - Sqr(inOutRatio) * (1. - Sqr(NdotI));

    Vector result = Vector(0, 0, 0);
    if (k >= 0.) // otherwise we have total internal reflection
        result = inOutRatio*in - (inOutRatio*NdotI + sqrt(k))*norm;

    return result;
}
