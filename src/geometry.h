#ifndef RAYTRACING_GEOMETRY_H
#define RAYTRACING_GEOMETRY_H

#include <vector>
#include "vector.h"
#include "ray.h"
#include "utils.h"

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
    virtual bool IsInside(const Vector& point) const { return false; }
    virtual ~Geometry() {}
};

class Plane : public Geometry
{
public:
    void SetHeight(double height) { m_Height = height; }
    virtual bool Intersect(const Ray& ray, IntersectionInfo& outInfo) const override;
    virtual bool IsInside(const Vector& point) const override;

private:
    double m_Height;
};

class RegularPolygon : public Geometry
{
public:
    void SetCenter(const Vector& center) { m_Center = center; }
    void SetRadius(double radius) { m_Radius = radius; }
    void SetSides(unsigned sides) { m_Sides = Max(3, sides); }

    virtual bool Intersect(const Ray& ray, IntersectionInfo& outInfo) const override;
    virtual bool IsInside(const Vector& point) const override;

private:
    Vector m_Center;
    double m_Radius;
    unsigned m_Sides;
};

class Sphere : public Geometry
{
public:
    void SetCenter(const Vector& center) { m_Center = center; }
    void SetRadius(double center) { m_Radius = center; }

    virtual bool Intersect(const Ray& ray, IntersectionInfo& outInfo) const override;
    virtual bool IsInside(const Vector& point) const override;

private:
    Vector m_Center;
    double m_Radius;
};

class Cube : public Geometry
{
public:
    void SetCenter(const Vector& center) { m_Center = center; }
    void SetHalfSide(double halfSide) { m_HalfSide = halfSide; }

    virtual bool Intersect(const Ray& ray, IntersectionInfo& outInfo) const override;
    virtual bool IsInside(const Vector& point) const override;

private:
    Vector m_Center;
    double m_HalfSide;

    bool IntersectSide(double level, double start, double dir, const Ray& ray, const Vector& normal, IntersectionInfo& info) const;
};

class CsgOp : public Geometry
{
public:
    void SetLeft(Geometry* left) { m_Left = left; }
    void SetRight(Geometry* right) {  m_Right = right; }

    virtual bool Operator(bool inA, bool inB) const =0;

private:
    Geometry* m_Left;
    Geometry* m_Right;

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
