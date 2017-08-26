#ifndef RAYTRACING_GEOMETRY_H
#define RAYTRACING_GEOMETRY_H

#include "vector.h"
#include "ray.h"

struct IntersectionInfo
{
    Vector ip;
    Vector normal;
    float distance;
    float u, v;
};

class Geometry
{
public:
    virtual bool Intersect(const Ray& ray, IntersectionInfo& outInfo) const =0;
    virtual ~Geometry() {}
};

class Plane : public Geometry
{
public:
    virtual bool Intersect(const Ray& ray, IntersectionInfo& outInfo) const override;

    float y;
};

struct Shader;
struct Node
{
    Geometry* geometry;
    Shader* shader;
};

#endif //RAYTRACING_GEOMETRY_H
