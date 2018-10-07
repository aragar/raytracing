#ifndef RAYTRACING_TEXTURE_H
#define RAYTRACING_TEXTURE_H

#include "color.h"
#include "geometry.h"

struct IntersectionInfo;
class Texture
{
public:
    virtual Color Sample(const IntersectionInfo& info) const =0;
    virtual ~Texture() {}
};

class ConstantColorTexture : public Texture
{
public:
    ConstantColorTexture(const Color& color);

    virtual Color Sample(const IntersectionInfo&) const override { return m_Color; }

private:
    Color m_Color;
};

class CheckerTexture : public Texture
{
public:
    CheckerTexture(const Color& color1, const Color& color2, double scaling);

    virtual Color Sample(const IntersectionInfo& info) const override;

private:
    double m_Scaling = 1.;
    Color m_Color1;
    Color m_Color2;
};

class MandelbrotTexture : public Texture
{
public:
    MandelbrotTexture(double scaling, const Color& color1, const Color& color2);

    virtual Color Sample(const IntersectionInfo& info) const override;

private:
    double m_Scaling = 1.;
    Color m_Color1;
    Color m_Color2;
};

class ProceduralTexture : public Texture
{
public:
    ProceduralTexture();

    virtual Color Sample(const IntersectionInfo& info) const override ;

private:
    static const unsigned NUM = 3;

    Color m_ColorU[NUM];
    double m_FreqU[NUM];

    Color m_ColorV[NUM];
    double m_FreqV[NUM];
};

class Bitmap;
class BitmapTexture : public Texture
{
public:
    BitmapTexture(const char* filename, double scaling = 1.0);
    virtual ~BitmapTexture() override;

    virtual Color Sample(const IntersectionInfo& info) const override;

private:
    Bitmap* m_Bitmap = nullptr;
    double m_Scaling = 1.;
};

#endif //RAYTRACING_TEXTURE_H
