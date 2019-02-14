#ifndef RAYTRACING_BBOX_H
#define RAYTRACING_BBOX_H

#include "ray.h"
#include "vector.h"

enum class Axis
{
    X,
    Y,
    Z,

    None
};

// from mesh.cpp
bool IntersectTriangleFast(const Ray& ray, const Vector& A, const Vector& B, const Vector& C, double& dist);

struct Ray;
class BBox
{
public:
    void SetMin(const Vector& min) { m_Min = min; }
    void SetMax(const Vector& max) { m_Max = max; }

    void MakeEmpty();
    void Add(const Vector& point);

    bool IsInside(const Vector& point) const;
    bool TestIntersect(const Ray& ray) const;
    double ClosestIntersection(const Ray& ray) const;
    bool IntersectTriangle(const Vector& a, const Vector& b, const Vector& c) const;
    void Split(Axis axis, double where, BBox& left, BBox& right) const;

private:
    Vector m_Min;
    Vector m_Max;
};

#endif //RAYTRACING_BBOX_H
