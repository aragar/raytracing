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

#endif //RAYTRACING_TEXTURE_H
