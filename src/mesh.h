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
    virtual void EndRender() override;

private:
    std::vector<Vector> m_Vertices;
    std::vector<Vector> m_Normals;
    std::vector<Vector> m_UVs;
    std::vector<MeshTriangle> m_Triangles;
    BBox m_BBox;

    bool m_UseKDTree = true;
    bool m_UseSAH = false;
    KDTreeNode* m_KDRoot = nullptr;

    bool m_Faceted = true;
    bool m_BackCulling = true;

    bool Intersect(const Ray& ray, const MeshTriangle& triangle, IntersectionInfo& outInfo) const;
    bool Intersect(KDTreeNode* node, BBox bbox, const Ray& ray, IntersectionInfo& outInfo) const;

    bool LoadFromOBJ(const char* filename);
    void GenerateTrianglesData();

    void ComputeBoundingGeometry();

    void ComputeKDRoot();
    void BuildKD(KDTreeNode* node, const BBox& bbox, const std::vector<unsigned>& triangleList, unsigned depth) const;

    enum class KDPosition
    {
        Before,
        After,
        Intersection
    };
    KDPosition PartitionTriangle(const unsigned tidx , const Axis axis, const double position) const;
    double CalculateCost(const Axis axis, const double position, const std::vector<unsigned>& triangleList, const BBox& leftBox, const BBox& rightBox) const;
    double CalculateSingleVoxelCost(const BBox& bbox, const std::vector<unsigned>& triangleList) const;
    bool FindOptimalSplitPosition(const BBox& bbox, const std::vector<unsigned>& triangleList, const unsigned depth, double& outSplitPosition, Axis& outSplitAxis) const;
};


#endif //RAYTRACING_MESH_H
