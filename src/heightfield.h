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

    virtual void FillProperties(ParsedBlock& pb) override;
private:
    float* m_Heights = nullptr;
    float* m_MaxHeights = nullptr;
    Vector* m_Normals = nullptr;

    BBox m_BBox;

    unsigned int m_Width;
    unsigned int m_Height;

    float GetHeight(unsigned int x, unsigned int y) const;
    Vector GetNormal(float x, float y) const;

    void BlurImage(Bitmap& bmp, double blur, float& outMinY, float& outMaxY) const;

    void PopulateMaxHeights();
    void PopulateNormals();
};

#endif //RAYTRACING_HEIGHTFIELD_H
