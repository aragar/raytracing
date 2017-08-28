#include "vector.h"

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

Vector Reflect(Vector in, const Vector& norm)
{
    in.Normalize();
    in += 2 * norm * Dot(norm, -in);
    in.Normalize();
    return in;
}
