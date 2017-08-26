#include "vector.h"

Vector::Vector(float x, float y, float z)
: x(x),
  y(y),
  z(z)
{}

void Vector::Set(float x, float y, float z)
{
    this->x = x;
    this->y = y;
    this->z = z;
}

void Vector::Scale(const float multiplier)
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