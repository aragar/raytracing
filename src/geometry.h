#ifndef RAYTRACING_GEOMETRY_H
#define RAYTRACING_GEOMETRY_H

#include <vector>
#include "vector.h"
#include "ray.h"

class Geometry;
struct IntersectionInfo
{
    Vector ip;
    Vector normal;
    double distance;
    double u, v;
    const Geometry* geometry;
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
    void SetHeight(double height) { m_Height = height; }
    virtual bool Intersect(const Ray& ray, IntersectionInfo& outInfo) const override;

private:
    double m_Height;
};

class Sphere : public Geometry
{
public:
    void SetCenter(const Vector& Center) { m_Center = Center; }
    void SetRadius(double Radius) { m_Radius = Radius; }

    virtual bool Intersect(const Ray& ray, IntersectionInfo& outInfo) const override;

private:
    Vector m_Center;
    double m_Radius;
};

class Cube : public Geometry
{
public:
    void SetCenter(const Vector& center) { m_Center = center; }
    void SetHalfSide(double HalfSide) { m_HalfSide = HalfSide; }

    virtual bool Intersect(const Ray& ray, IntersectionInfo& outInfo) const override;

private:
    Vector m_Center;
    double m_HalfSide;

    bool IntersectSide(double level, double start, double dir, const Ray& ray, const Vector& normal, IntersectionInfo& info) const;
};

class CsgOp : public Geometry
{
public:
    void SetLeft(Geometry* Left) { m_Left = Left; }
    void SetRight(Geometry* Right) {  m_Right = Right; }

    virtual bool Operator(bool inA, bool inB) const =0;

private:
    Geometry* m_Left;
    Geometry* m_Right;

    void FindAllIntersections(Ray ray, const Geometry& geometry, std::vector<IntersectionInfo>& ips) const;
    bool Intersect(const Ray& ray, IntersectionInfo& outInfo) const override;
};

class CsgAnd : public CsgOp
{
public:
    virtual bool Operator(bool inA, bool inB) const override { return (inA && inB); }
};

class CsgPlus : public CsgOp
{
public:
    virtual bool Operator(bool inA, bool inB) const override { return (inA || inB); }
};

class CsgMinus : public CsgOp
{
public:
    virtual bool Operator(bool inA, bool inB) const override { return (inA && !inB); }
};

struct Shader;
struct Node
{
    Geometry* geometry;
    Shader* shader;
};

#endif //RAYTRACING_GEOMETRY_H
