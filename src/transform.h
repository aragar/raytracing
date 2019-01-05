#ifndef RAYTRACING_TRANSFORM_H
#define RAYTRACING_TRANSFORM_H

#include "matrix.h"
#include "ray.h"
#include "vector.h"

/// A transformation class, which implements model-view transform. Objects can be
/// arbitrarily scaled, rotated and translated.
class Transform
{
public:
    Transform();

    void Reset();
    void Scale(double x, double y, double z);
    void Rotate(double yaw, double pitch, double roll);
    void Translate(const Vector& v);

    Vector Point(Vector p) const;
    Vector UndoPoint(Vector p) const;
    Vector Direction(const Vector& dir) const;
    Vector UndoDirection(const Vector& dir) const;
    Vector Normal(const Vector& dir) const;

    Ray DoRay(const Ray& ray) const;
    Ray UndoRay(const Ray& ray) const;

private:
    Matrix m_Transform;
    Matrix m_InverseTransform;
    Matrix m_TransposedInverse;
    Vector m_Offset;
};

#endif // RAYTRACING_TRANSFORM_H