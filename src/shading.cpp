#include "shading.h"

extern Vector g_LightPos;
extern float g_LightIntensity;

Color CheckerShader::Shade(const Ray& ray, const IntersectionInfo& info) const
{
    int x = (int) floorf(info.u / 5.f);
    int y = (int) floorf(info.v / 5.f);

    const Color& color = ((x + y) % 2 == 0) ? m_Color1 : m_Color2;

    const Vector& v1 = info.normal;
    Vector v2 = g_LightPos - info.ip;
    float distanceToLightSqr = v2.LengthSqr();
    v2.Normalize();
    float lambertCoeff = v1*v2;
    float attenuationCoeff = 1.f / distanceToLightSqr;

    const Color result = color * lambertCoeff * attenuationCoeff * g_LightIntensity;
    return result;
}
