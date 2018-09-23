#ifndef RAYTRACING_SHADING_H
#define RAYTRACING_SHADING_H

#include "color.h"
#include "vector.h"

class IntersectionInfo;
class Ray;
class Texture;

struct Light
{
    Vector pos;
    double intensity;
};

class Shader
{
public:
    virtual Color Shade(const Ray& ray, const IntersectionInfo& info) const =0;
    virtual ~Shader() {}
};

class Lambert : public Shader
{
public:
    void SetColor(const Color& color) { m_Color = color; }
    void SetTexture(Texture* texture) { m_Texture = texture; }

    Lambert();
    virtual Color Shade(const Ray& ray, const IntersectionInfo& info) const override;

private:
    Color m_Color;
    Texture* m_Texture;
};

class Phong : public Shader
{
public:
    void SetColor(const Color& color) { m_Color = color; }
    void SetTexture(Texture* texture) { m_Texture = texture; }
    void SetSpecularMultiplier(double multiplier) { m_SpecularMultiplier = multiplier; }
    void SetSpecularExponent(double exponent) { m_SpecularExponent = exponent; }

    virtual Color Shade(const Ray& ray, const IntersectionInfo& info) const override;

protected:
    virtual double GetSpecularCoeff(const Ray& ray, const IntersectionInfo& info, const Light& light) const;

    Color m_Color;
    Texture* m_Texture;
    double m_SpecularMultiplier;
    double m_SpecularExponent;
};

class BlinnPhong : public Phong
{
protected:
    virtual double GetSpecularCoeff(const Ray& ray, const IntersectionInfo& info, const Light& light) const override;
};

#endif //RAYTRACING_SHADING_H
