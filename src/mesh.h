#ifndef RAYTRACING_MESH_H
#define RAYTRACING_MESH_H

#include "geometry.h"
#include "vector.h"
#include "scene.h"

#include <array>
#include <vector>

struct MeshTriangle
{
    std::array<int, 3> vertices;
    std::array<int, 3> normals;
    std::array<int, 3> uvs;
    Vector geometryNormal;
};

class Mesh : public Geometry
{
public:
    Mesh(bool isFaceted = true, bool backCulling = true);
    virtual ~Mesh() override;

    void SetFaceted(bool faceted) { m_Faceted = faceted; }
    void SetBackCulling(bool backCulling) { m_BackCulling = backCulling; }
    void ComputeBoundingGeometry();

    virtual bool Intersect(const Ray& ray, IntersectionInfo& outInfo) const override;
    virtual bool IsInside(const Vector& point) const override;

    virtual void FillProperties(ParsedBlock& pb) override;
    virtual void BeginRender() override;

private:
    std::vector<Vector> m_Vertices;
    std::vector<Vector> m_Normals;
    std::vector<Vector> m_UVs;
    std::vector<MeshTriangle> m_Triangles;

    Geometry* m_BoundingGeometry = nullptr;

    bool m_Faceted = true;
    bool m_BackCulling = true;
    bool m_IsTetraeder = false;

    bool Intersect(const Ray& ray, const MeshTriangle& triangle, IntersectionInfo& outInfo) const;

    void GenerateTetraeder();
    void GenerateTruncatedIcosahedron();
};


#endif //RAYTRACING_MESH_H
