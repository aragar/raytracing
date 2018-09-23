#include "shading.h"

#include "texture.h"
#include "shadinghelper.h"

extern Vector g_LightPos;
extern Color g_AmbientLight;
extern std::vector<Light> g_Lights;

Color Lambert::Shade(const Ray& ray, const IntersectionInfo& info) const
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

Lambert::Lambert()
: m_Texture(nullptr)
{
    m_Color.MakeZero();
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

double BlinnPhong::GetSpecularCoeff(const Ray& ray, const IntersectionInfo& info, const Light& light) const
{
    Vector lightDir = Normalize(light.pos - info.ip);
    Vector halfwayDir = Normalize(lightDir - ray.dir);
    double dot = Dot(info.normal, halfwayDir);
    double result = pow(Max(dot, 0.), m_SpecularExponent);
    return result;
}
