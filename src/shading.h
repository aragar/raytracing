#ifndef RAYTRACING_SHADING_H
#define RAYTRACING_SHADING_H

#include "color.h"
#include "geometry.h"

class Shader
{
public:
    virtual Color Shade(const Ray& ray, const IntersectionInfo& info) const =0;
    virtual ~Shader() {}
};

class CheckerShader : public  Shader
{
public:
    const Color& GetColor1() const { return m_Color1; }
    void SetColor1(const Color& color) { m_Color1 = color; }
    const Color& GetColor2() const { return  m_Color2; }
    void SetColor2(const Color& color) { m_Color2 = color; }

    virtual Color Shade(const Ray& ray, const IntersectionInfo& info) const override;

private:
    Color m_Color1;
    Color m_Color2;
};

#endif //RAYTRACING_SHADING_H
