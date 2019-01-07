#ifndef RAYTRACING_GEOMETRY_H
#define RAYTRACING_GEOMETRY_H

#include <vector>

#include "ray.h"
#include "vector.h"
#include "scene.h"
#include "transform.h"
#include "utils.h"

class Geometry;
struct IntersectionInfo
{
    Vector ip;
    Vector normal;
    double distance;
    double u, v;
    const Geometry* geometry;
    Vector rayDir;
    Vector dNdx, dNdy;
};

/**
 * @class Intersectable
 * @brief implements the interface to an intersectable primitive (geometry or node)
 */
class Intersectable
{
public:
    virtual bool Intersect(const Ray& ray, IntersectionInfo& outInfo) const =0;
};

class Geometry : public Intersectable, public SceneElement
{
public:
    virtual ~Geometry() {}

    virtual bool IsInside(const Vector& point) const =0;
    virtual ElementType GetElementType() const override { return ElementType::GEOMETRY; }
};

class Plane : public Geometry
{
public:
    Plane(double height = 0., double limit = 1e99);

    virtual bool Intersect(const Ray& ray, IntersectionInfo& outInfo) const override;
    virtual bool IsInside(const Vector& point) const override;

    virtual void FillProperties(ParsedBlock& pb) override;

private:
    double m_Height = 0.;
    double m_Limit = 1e99;
};

class RegularPolygon : public Geometry
{
public:
    RegularPolygon(const Vector& center = Vector(0, 0, 0), double radius = 1., unsigned sides = 3);

    virtual bool Intersect(const Ray& ray, IntersectionInfo& outInfo) const override;
    virtual bool IsInside(const Vector& point) const override;

    virtual void FillProperties(ParsedBlock& pb) override;

private:
    Vector m_Center;
    double m_Radius = 1.;
    unsigned m_Sides = 3;
};

class Sphere : public Geometry
{
public:
    Sphere(const Vector& center = Vector(0, 0, 0), double radius = 1.);

    virtual bool Intersect(const Ray& ray, IntersectionInfo& outInfo) const override;
    virtual bool IsInside(const Vector& point) const override;

    virtual void FillProperties(ParsedBlock& pb) override;

private:
    Vector m_Center;
    double m_Radius = 1.;
};

class Cube : public Geometry
{
public:
    Cube(const Vector& center = Vector(0, 0, 0), double halfSide = .5);

    virtual bool Intersect(const Ray& ray, IntersectionInfo& outInfo) const override;
    virtual bool IsInside(const Vector& point) const override;

    virtual void FillProperties(ParsedBlock& pb) override;

private:
    Vector m_Center;
    double m_HalfSide = .5;

    bool IntersectSide(double level, double start, double dir, const Ray& ray, const Vector& normal, IntersectionInfo& info) const;
};

class CsgOp : public Geometry
{
public:
    CsgOp() = default;
    CsgOp(Geometry* left, Geometry* right);

    virtual bool Operator(bool inA, bool inB) const =0;
    virtual bool IsInside(const Vector& point) const override;

    virtual void FillProperties(ParsedBlock& pb) override;

private:
    Geometry* m_Left = nullptr;
    Geometry* m_Right = nullptr;

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

struct Node : public Intersectable, public SceneElement
{
    Geometry* geometry = nullptr;
    Shader* shader = nullptr;
    Transform transform;
    Texture* bump = nullptr;

    Node() = default;
    Node(Geometry* geometry, Shader* shader);

    virtual bool Intersect(const Ray& ray, IntersectionInfo& outInfo) const override;

    virtual ElementType GetElementType() const override { return ElementType::NODE; }
    virtual void FillProperties(ParsedBlock& pb) override;
};

#endif //RAYTRACING_GEOMETRY_H
