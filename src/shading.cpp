#include "shading.h"

#include "colors.h"
#include "shadinghelper.h"
#include "texture.h"

extern Color g_AmbientLight;
extern std::vector<Light> g_Lights;

Color Lambert::Shade(const Ray&, const IntersectionInfo& info) const
{
    const Color diffuse = m_Texture ? m_Texture->Sample(info) : m_Color;
    Color result = diffuse*g_AmbientLight;

    const Vector& normalDir = info.normal;
    for (const Light& light : g_Lights)
    {
        const Vector lightDir = Normalize(light.pos - info.ip);
        const double lambertCoeff = Dot(normalDir, lightDir);
        const double lightContribution = ShadingHelper::GetLightContribution(info, light);
        result += diffuse*lambertCoeff*lightContribution;
    }

    return result;
}

Lambert::Lambert(const Color& color, Texture* texture)
: m_Color(color),
  m_Texture(texture)
{
}

double Phong::GetSpecularCoeff(const Ray& ray, const IntersectionInfo& info, const Light& light) const
{
    Vector lightDir = Normalize(info.ip - light.pos);
    Vector reflection = Reflect(lightDir, info.normal);
    double dot = Dot(-ray.dir, reflection);
    double result = pow(Max(dot, 0.), m_SpecularExponent);
    return result;
}

Color Phong::Shade(const Ray& ray, const IntersectionInfo& info) const
{
    Color diffuse = m_Texture ? m_Texture->Sample(info) : m_Color;
    Color result = diffuse*g_AmbientLight;

    const Vector& normalDir = info.normal;
    for (const Light& light : g_Lights)
    {
        Vector lightDir = Normalize(light.pos - info.ip);
        double lambertCoeff = Dot(normalDir, lightDir);
        double lightContribution = ShadingHelper::GetLightContribution(info, light);
        double specularCoeff = GetSpecularCoeff(ray, info, light);
        result += diffuse*lambertCoeff*lightContribution
                  + Color{1.f, 1.f, 1.f}*specularCoeff*m_SpecularMultiplier*lightContribution;
    }

    return result;
}

Phong::Phong(const Color& color, double specularMultiplier, double specularExponent)
: m_Color(color),
  m_SpecularMultiplier(specularMultiplier),
  m_SpecularExponent(specularExponent)
{
}

Phong::Phong(Texture* texture, double specularMultiplier, double specularExponent)
: m_Texture(texture),
  m_SpecularMultiplier(specularMultiplier),
  m_SpecularExponent(specularExponent)
{
}

double BlinnPhong::GetSpecularCoeff(const Ray& ray, const IntersectionInfo& info, const Light& light) const
{
    Vector lightDir = Normalize(light.pos - info.ip);
    Vector halfwayDir = Normalize(lightDir - ray.dir);
    double dot = Dot(info.normal, halfwayDir);
    double result = pow(Max(dot, 0.), m_SpecularExponent);
    return result;
}

Color OrenNayar::Shade(const Ray& ray, const IntersectionInfo& info) const
{
    // http://mimosa-pudica.net/improved-oren-nayar.html
    const Color diffuse = m_Texture ? m_Texture->Sample(info) : m_Color;
    Color result = diffuse*g_AmbientLight;

    const double sigma2 = Sqr(m_Sigma);
    const double VdotN = Dot(-ray.dir, info.normal);
    for (const Light& light : g_Lights)
    {
        const Vector lightDir = Normalize(light.pos - info.ip);
        const double LdotV = Dot(lightDir, -ray.dir);
        const double LdotN = Dot(lightDir, info.normal);
        const double s = LdotV - LdotN*VdotN;
        const double t = (s <= 0. ? 1. : Max(LdotN, VdotN));
        const double a = 1 - 0.5 * sigma2 / (sigma2 + 0.33);
        const double b = 0.45 * sigma2 / (sigma2 + 0.09);

        const double orenNayarCoeff = LdotN*(a + b*s/t);
        const double lightContribution = ShadingHelper::GetLightContribution(info, light);
        result += diffuse*orenNayarCoeff*lightContribution;
    }

    return result;
}

OrenNayar::OrenNayar(const Color& color, double sigma)
: m_Color(color),
  m_Sigma(sigma)
{
}

OrenNayar::OrenNayar(Texture* texture, double sigma)
: m_Texture(texture),
  m_Sigma(sigma)
{
}

Reflection::Reflection(double multiplier)
: m_Multiplier(multiplier)
{
}

extern Color Raytrace(const Ray& ray);
Color Reflection::Shade(const Ray& ray, const IntersectionInfo& info) const
{
    Vector n = Faceforward(ray.dir, info.normal);

    Ray newRay = ray;
    newRay.start = info.ip + n * 0.000001;
    newRay.dir = Reflect(ray.dir, n);
    newRay.depth++;

    const Color& color = Raytrace(newRay) * m_Multiplier;
    return color;
}

Refraction::Refraction(double inOutRatio, double multiplier)
: m_InOutRatio(inOutRatio),
  m_Multiplier(multiplier)
{
}

Color Refraction::Shade(const Ray& ray, const IntersectionInfo& info) const
{
    // ior = eta2 / eta1
    Vector refraction;
    if (Dot(ray.dir, info.normal) < 0)
    {
        // entering the geometry
        refraction = Refract(ray.dir, info.normal, 1 / m_InOutRatio);
    }
    else
    {
        // leaving the geometry
        refraction = Refract(ray.dir, -info.normal, m_InOutRatio);
    }

    if (refraction.LengthSqr() == 0)
        return Colors::RED; // to debug easy

    Ray newRay = ray;
    newRay.start = info.ip - Faceforward(ray.dir, info.normal) * 0.000001;
    newRay.dir = refraction;
    newRay.depth++;

    const Color& color = Raytrace(newRay) * m_Multiplier;
    return color;
}
