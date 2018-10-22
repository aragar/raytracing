#ifndef RAYTRACING_SHADING_H
#define RAYTRACING_SHADING_H

#include <array>

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
    virtual ~Shader() =default;
};

class Lambert : public Shader
{
public:
    Lambert(const Color& color);
    Lambert(Texture* texture);

    virtual Color Shade(const Ray& ray, const IntersectionInfo& info) const override;

private:
    Color m_Color;
    Texture* m_Texture = nullptr;
};

class Phong : public Shader
{
public:
    Phong(const Color& color, double specularMultiplier = 1., double specularExponent = 1.);
    Phong(Texture* texture, double specularMultiplier = 1., double specularExponent = 1.);

    virtual Color Shade(const Ray& ray, const IntersectionInfo& info) const override;

protected:
    virtual double GetSpecularCoeff(const Ray& ray, const IntersectionInfo& info, const Light& light) const;

    Color m_Color;
    Texture* m_Texture = nullptr;
    double m_SpecularMultiplier = 10.;
    double m_SpecularExponent = 0.4;
};

class BlinnPhong : public Phong
{
public:
    using Phong::Phong;

protected:
    virtual double GetSpecularCoeff(const Ray& ray, const IntersectionInfo& info, const Light& light) const override;
};

class OrenNayar : public Shader
{
public:
    OrenNayar(const Color& color, double sigma = 0.);
    OrenNayar(Texture* texture, double sigma = 0.);

    virtual Color Shade(const Ray& ray, const IntersectionInfo& info) const override;

private:
    Color m_Color;
    Texture* m_Texture = nullptr;
    double m_Sigma = 0;
};

class Reflection : public Shader
{
public:
    Reflection(double multiplier = 0.99, double glossiness = 1., int samples = 32);
    virtual Color Shade(const Ray& ray, const IntersectionInfo& info) const override;

private:
    double m_Multiplier = 0.99;
    double m_Glossiness = 1.;
    int m_Samples = 32;
};

class Refraction : public Shader
{
public:
    Refraction(double inOutRatio, double multiplier = 0.99);
    virtual Color Shade(const Ray& ray, const IntersectionInfo& info) const override;

private:
    double m_InOutRatio = 1.;
    double m_Multiplier = 0.99;
};

class Layered : public Shader
{
public:
    Layered();
    void AddLayer(Shader* shader, Color blend, Texture* texture = nullptr);

    virtual Color Shade(const Ray& ray, const IntersectionInfo& info) const override;

private:
    struct Layer
    {
        Shader* m_Shader;
        Color m_Blend;
        Texture* m_Texture;
    };

    std::array<Layer, 32> m_Layers;
    unsigned m_NumLayers = 0;
};

#endif //RAYTRACING_SHADING_H
