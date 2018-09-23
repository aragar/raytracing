#include "shading.h"

#include "texture.h"
#include "shadinghelper.h"

extern Vector g_LightPos;
extern Color g_AmbientLight;

Color Lambert::Shade(const Ray& ray, const IntersectionInfo& info) const
{
    Color diffuse = m_Texture ? m_Texture->Sample(info) : m_Color;

    const Vector& normalDir = info.normal;
    Vector lightDir = Normalize(g_LightPos - info.ip);
    double lambertCoeff = Dot(normalDir, lightDir);

    const double lightContribution = ShadingHelper::GetLightContribution(info);

    Color result = diffuse*g_AmbientLight + diffuse*lambertCoeff*lightContribution;
    return result;
}

Lambert::Lambert()
: m_Texture(nullptr)
{
    m_Color.MakeZero();
}

double Phong::GetCoeff(const Ray& ray, const IntersectionInfo& info) const
{
    Vector lightDir = Normalize(info.ip - g_LightPos);
    Vector reflection = Reflect(lightDir, info.normal);
    double dot = Dot(-ray.dir, reflection);
    double result = pow(Max(dot, 0.), m_SpecularExponent);
    return result;
}

Color Phong::Shade(const Ray& ray, const IntersectionInfo& info) const
{
    Color diffuse = m_Texture ? m_Texture->Sample(info) : m_Color;

    const Vector& normalDir = info.normal;
    Vector lightDir = Normalize(g_LightPos - info.ip);
    double lambertCoeff = Dot(normalDir, lightDir);

    double lightContribution = ShadingHelper::GetLightContribution(info);
    double coeff = GetCoeff(ray, info);

    Color result = g_AmbientLight*diffuse
                   + diffuse*lambertCoeff*lightContribution
                   + Color{1.f, 1.f, 1.f}*coeff*m_SpecularMultiplier*lightContribution;
    return result;
}

double BlinnPhong::GetCoeff(const Ray& ray, const IntersectionInfo& info) const
{
    Vector lightDir = Normalize(g_LightPos - info.ip);
    Vector halfwayDir = Normalize(lightDir - ray.dir);
    double dot = Dot(info.normal, halfwayDir);
    double result = pow(Max(dot, 0.), m_SpecularExponent);
    return result;
}
