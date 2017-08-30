#include "shading.h"

#include "texture.h"
#include "shadinghelper.h"

extern Vector g_LightPos;
extern Color g_AmbientLight;

Color Lambert::Shade(const Ray& ray, const IntersectionInfo& info) const
{
    Color diffuse = m_Texture ? m_Texture->Sample(info) : m_Color;

    const Vector& v1 = info.normal;
    Vector v2 = Normalize(g_LightPos - info.ip);
    double lambertCoeff = Dot(v1, v2);

    const double lightContribution = ShadingHelper::GetLightContribution(info);
    Color result = diffuse*g_AmbientLight + diffuse*lambertCoeff*lightContribution;
    return result;
}

Lambert::Lambert()
: m_Texture(nullptr)
{
    m_Color.MakeZero();
}

Color Phong::Shade(const Ray& ray, const IntersectionInfo& info) const
{
    Color diffuse = m_Texture ? m_Texture->Sample(info) : m_Color;

    const Vector& v1 = info.normal;
    Vector v2 = Normalize(g_LightPos - info.ip);
    double lamberCoeff = Dot(v1, v2);
    double lightContribution = ShadingHelper::GetLightContribution(info);

    Vector reflection = Reflect(info.ip - g_LightPos, info.normal);
    double cosGamma = Dot(-ray.dir, reflection);
    double phongCoeff = 0;
    if ( cosGamma > 0. )
    {
        phongCoeff = pow(cosGamma, m_SpecularExponent);
    }

    Color result = g_AmbientLight*diffuse
                   + diffuse*lamberCoeff*lightContribution
                   + Color{1.f, 1.f, 1.f}*phongCoeff*m_SpecularMultiplier*lightContribution;
    return result;
}