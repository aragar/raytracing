#ifndef RAYTRACING_TEXTURE_H
#define RAYTRACING_TEXTURE_H

#include "color.h"
#include "scene.h"
#include "geometry.h"

struct IntersectionInfo;
class Texture : public SceneElement
{
public:
    virtual Color Sample(const IntersectionInfo& info) const =0;
    virtual ~Texture() override {}

    virtual ElementType GetElementType() const override { return ElementType::TEXTURE; }
};

class ConstantColorTexture : public Texture
{
public:
    ConstantColorTexture(const Color& color = Colors::BLACK);

    virtual Color Sample(const IntersectionInfo&) const override { return m_Color; }
    virtual void FillProperties(ParsedBlock& pb) override;

private:
    Color m_Color;
};

class CheckerTexture : public Texture
{
public:
    CheckerTexture() = default;
    CheckerTexture(const Color& color1, const Color& color2, double scaling);

    virtual Color Sample(const IntersectionInfo& info) const override;
    virtual void FillProperties(ParsedBlock& pb) override;

private:
    double m_Scaling = 1.;
    Color m_Color1;
    Color m_Color2;
};

class MandelbrotTexture : public Texture
{
public:
    MandelbrotTexture() = default;
    MandelbrotTexture(double scaling, const Color& color1, const Color& color2);

    virtual Color Sample(const IntersectionInfo& info) const override;
    virtual void FillProperties(ParsedBlock& pb) override;

private:
    double m_Scaling = 1.;
    Color m_Color1;
    Color m_Color2;
};

class ProceduralTexture : public Texture
{
public:
    virtual Color Sample(const IntersectionInfo& info) const override;
    virtual void FillProperties(ParsedBlock& pb) override;

private:
    struct ProceduralLine
    {
        Color colorU;
        double freqU;

        Color colorV;
        double freqV;
    };

    std::vector<ProceduralLine> m_Lines;
};

class BitmapHelper
{
public:
    BitmapHelper(const char* filename = nullptr, double scaling = 1., bool useBilinearFiltering = false);
    ~BitmapHelper();

    Color GetColor(double u, double v) const;

    void FillProperties(ParsedBlock& pb);

private:
    Bitmap* m_Bitmap = nullptr;
    double m_Scaling = 1.;
    bool m_UseBilinearFiltering = false;

    void GetCoords(double u, double v, int& x, int& y) const;
    void GetCoords(double u, double v, double& x, double& y) const;
};

class Bitmap;
class BitmapTexture : public Texture
{
public:
    BitmapTexture(const char* filename = nullptr, double scaling = 1.0, bool useBilinearFiltering = false);

    virtual Color Sample(const IntersectionInfo& info) const override;
    virtual void FillProperties(ParsedBlock& pb) override;

private:
    BitmapHelper m_BitmapHelper;
};

class BumpTexture : public Texture
{

};

class Fresnel : public Texture
{
public:
    Fresnel(double inOutRatio = 1.33);

    virtual Color Sample(const IntersectionInfo& info) const override;
    virtual void FillProperties(ParsedBlock& pb) override;

private:
    double m_InOutRatio = 1.;
};

#endif //RAYTRACING_TEXTURE_H
