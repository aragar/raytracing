#ifndef RAYTRACING_HEIGHTFIELD_H
#define RAYTRACING_HEIGHTFIELD_H

#include "geometry.h"
#include "bbox.h"

class Heightfield : public Geometry
{
public:
    Heightfield() = default;
    virtual ~Heightfield() override;

    virtual bool Intersect(const Ray& ray, IntersectionInfo& outInfo) const override;
    virtual bool IsInside(const Vector& point) const override { return false; }

    virtual void BeginRender() override;
    virtual void FillProperties(ParsedBlock& pb) override;

private:
    float* m_Heights = nullptr;
    float* m_MaxHeights = nullptr;
    Vector* m_Normals = nullptr;

    BBox m_BBox;

    unsigned int m_Width;
    unsigned int m_Height;

    float GetHeight(int x, int y) const;
    float GetHeighest(int x, int y, int k) const;
    Vector GetNormal(float x, float y) const;

    void BlurImage(Bitmap& bmp, double blur, float& outMinY, float& outMaxY) const;

    void PopulateMaxHeights();
    void PopulateNormals();

    bool m_UseOptimization = false;
    struct OptimizationHighMap { float height[16]; };
    OptimizationHighMap* m_HighMap = nullptr;
    int m_MaxK;

    void BuildHighMap();

    void ComputeNextCoordinates(Vector& p, const Ray& ray, int& outX, int& outZ) const;
};

#endif //RAYTRACING_HEIGHTFIELD_H
