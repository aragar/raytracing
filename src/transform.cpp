#include "transform.h"

#include "utils.h"

Transform::Transform()
{
    Reset();
}

void Transform::Reset()
{
    m_Transform = Matrix(1);
    m_InverseTransform = Inverse(m_Transform);
    m_TransposedInverse = Matrix(1);
    m_Offset.MakeZero();
}

void Transform::Scale(double x, double y, double z)
{
    Matrix scaling(x);
    scaling.m[1][1] = y;
    scaling.m[2][2] = z;

    m_Transform = m_Transform * scaling;
    m_InverseTransform = Inverse(m_Transform);
    m_TransposedInverse = Transpose(m_InverseTransform);
}

void Transform::Rotate(double yaw, double pitch, double roll)
{
    m_Transform = m_Transform *
                RotationAroundX(ToRadians(pitch)) *
                RotationAroundY(ToRadians(yaw)) *
                RotationAroundZ(ToRadians(roll));
    m_InverseTransform = Inverse(m_Transform);
    m_TransposedInverse = Transpose(m_InverseTransform);
}

void Transform::Translate(const Vector& v)
{
    m_Offset = v;
}

Vector Transform::Point(Vector P) const
{
    P = P * m_Transform;
    P = P + m_Offset;

    return P;
}

Vector Transform::UndoPoint(Vector P) const
{
    P = P - m_Offset;
    P = P * m_InverseTransform;

    return P;
}

Vector Transform::Direction(const Vector& dir) const
{
    return dir * m_Transform;
}

Vector Transform::UndoDirection(const Vector& dir) const
{
    return dir * m_InverseTransform;
}

Vector Transform::Normal(const Vector& dir) const
{
    return dir * m_TransposedInverse;
}

Ray Transform::DoRay(const Ray& ray) const
{
    Ray result = ray;
    result.start = Point(ray.start);
    result.dir   = Direction(ray.dir);
    return result;
}

Ray Transform::UndoRay(const Ray& ray) const
{
    Ray result = ray;
    result.start = UndoPoint(ray.start);
    result.dir   = UndoDirection(ray.dir);
    return result;
}