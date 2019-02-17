#ifndef RAYTRACING_MESH_H
#define RAYTRACING_MESH_H

#include "bbox.h"
#include "geometry.h"
#include "vector.h"
#include "KDTree.h"

#include <array>
#include <vector>

struct MeshTriangle
{
    std::array<int, 3> vertices;
    std::array<int, 3> normals;
    std::array<int, 3> uvs;
    Vector geometryNormal;
    Vector dNdx, dNdy;
};

class Mesh : public Geometry
{
public:
    Mesh(bool isFaceted = true, bool backCulling = true);
    virtual ~Mesh() override;

    void SetFaceted(bool faceted) { m_Faceted = faceted; }
    void SetBackCulling(bool backCulling) { m_BackCulling = backCulling; }

    virtual bool Intersect(const Ray& ray, IntersectionInfo& outInfo) const override;
    virtual bool IsInside(const Vector& point) const override;

    virtual void FillProperties(ParsedBlock& pb) override;
    virtual void BeginRender() override;

private:
    std::vector<Vector> m_Vertices;
    std::vector<Vector> m_Normals;
    std::vector<Vector> m_UVs;
    std::vector<MeshTriangle> m_Triangles;
    BBox m_BBox;

    bool m_UseKDTree = true;
    KDTreeNode* m_KDRoot = nullptr;

    bool m_Faceted = true;
    bool m_BackCulling = true;

    bool Intersect(const Ray& ray, const MeshTriangle& triangle, IntersectionInfo& outInfo) const;
    bool Intersect(KDTreeNode* node, BBox bbox, const Ray& ray, IntersectionInfo& outInfo) const;

    bool LoadFromOBJ(const char* filename);
    void GenerateTrianglesData();

    void ComputeBoundingGeometry();

    void ComputeKDRoot();
    void BuildKD(KDTreeNode* node, BBox bbox, const std::vector<int>& triangleList, unsigned int depth) const;
    void BuildKDNodeChildren(KDTreeNode* node, const BBox& bbox, const std::vector<int>& triangleList, unsigned int depth) const;
};


#endif //RAYTRACING_MESH_H
