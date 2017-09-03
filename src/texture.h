#ifndef RAYTRACING_TEXTURE_H
#define RAYTRACING_TEXTURE_H

#include "color.h"
#include "geometry.h"

class Texture
{
public:
    virtual Color Sample(const IntersectionInfo& info) const =0;
    virtual ~Texture() {}
};

class ConstantColorTexted : public Texture
{
public:
    void SetColor(const Color& color) { m_Color = color; }

    virtual Color Sample(const IntersectionInfo& info) const override { return m_Color; }

private:
    Color m_Color;
};

class CheckerTexture : public Texture
{
public:
    CheckerTexture();

    void SetScaling(double Scaling) { m_Scaling = Scaling; }
    void SetColor1(const Color& color) { m_Color1 = color; }
    void SetColor2(const Color& color) { m_Color2 = color; }

    virtual Color Sample(const IntersectionInfo& info) const override;

private:
    double m_Scaling;
    Color m_Color1;
    Color m_Color2;
};

class MandelbrotTexture : public Texture
{
public:
    void SetScaling(double Scaling) { m_Scaling = Scaling; }
    void SetColor1(const Color& Color1) { m_Color1 = Color1; }
    void SetColor2(const Color& Color2) { m_Color2 = Color2; }

    virtual Color Sample(const IntersectionInfo& info) const override;

private:
    double m_Scaling;
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

#endif //RAYTRACING_TEXTURE_H
