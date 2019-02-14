#include "shading.h"

#include "colors.h"
#include "light.h"
#include "shadinghelper.h"
#include "texture.h"

#include <cstring>

Color Lambert::Shade(const Ray& ray, const IntersectionInfo& info) const
{
    const Color diffuse = m_Texture ? m_Texture->Sample(info) : m_Color;
    Color result = diffuse*scene.settings.ambientLight;

    for (const Light* light : scene.lights)
    {
        const Vector lightDir = Normalize(info.ip - light->pos); // from light towards the intersection point
        const Vector normal = Faceforward(lightDir, info.normal); // orient so that surface points to the light
        const double lambertCoeff = Dot(normal, -lightDir);
        const double lightContribution = ShadingHelper::GetLightContribution(info, *light);
        result += diffuse*lambertCoeff*lightContribution;
    }

    return result;
}

Lambert::Lambert(const Color& color)
: m_Color(color)
{
}

Lambert::Lambert(Texture* texture)
: m_Texture(texture)
{}

void Lambert::FillProperties(ParsedBlock& pb)
{
    pb.GetColorProp("color", &m_Color);
    pb.GetTextureProp("texture", &m_Texture);
}

double Phong::GetSpecularCoeff(const Ray& ray, const IntersectionInfo& info, const Light& light) const
{
    const Vector lightDir = Normalize(info.ip - light.pos); // from light towards the intersection point
    const Vector normal = Faceforward(lightDir, info.normal); // orient so that the surface points to the light
    const Vector reflection = Reflect(lightDir, normal);
    const double cosGamma = Dot(-ray.dir, reflection);
    const double result = cosGamma > 0. ? pow(cosGamma, m_SpecularExponent) : 0.;
    return result;
}

Color Phong::Shade(const Ray& ray, const IntersectionInfo& info) const
{
    const Color diffuse = m_Texture ? m_Texture->Sample(info) : m_Color;
    Color result = diffuse*scene.settings.ambientLight;

    for (const Light* light : scene.lights)
    {
        const Vector lightDir = Normalize(info.ip - light->pos); // from light towards the intersection point
        const Vector normal = Faceforward(lightDir, info.normal); // orient so that the surface points to the light
        const double lambertCoeff = Dot(normal, -lightDir);
        const double lightContribution = ShadingHelper::GetLightContribution(info, *light);
        const double specularCoeff = GetSpecularCoeff(ray, info, *light);
        result += diffuse*lambertCoeff*lightContribution
                  + Color{1.f, 1.f, 1.f}*specularCoeff*m_SpecularMultiplier*lightContribution;
    }

    return result;
}

Phong::Phong(const Color& color, double specularMultiplier, double specularExponent)
: m_Color(color)
, m_SpecularMultiplier(specularMultiplier)
, m_SpecularExponent(specularExponent)
{
}

Phong::Phong(Texture* texture, double specularMultiplier, double specularExponent)
: m_Texture(texture)
, m_SpecularMultiplier(specularMultiplier)
, m_SpecularExponent(specularExponent)
{
}

void Phong::FillProperties(ParsedBlock& pb)
{
    pb.GetColorProp("color", &m_Color);
    pb.GetTextureProp("texture", &m_Texture);
    pb.GetDoubleProp("specularMultiplier", &m_SpecularMultiplier);
    pb.GetDoubleProp("specularExponent", &m_SpecularExponent);
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
    Color result = diffuse*scene.settings.ambientLight;

    const double sigma2 = Sqr(m_Sigma);
    const double VdotN = Dot(-ray.dir, info.normal);
    for (const Light* light : scene.lights)
    {
        const Vector lightDir = Normalize(light->pos - info.ip);
        const double LdotV = Dot(lightDir, -ray.dir);
        const double LdotN = Dot(lightDir, info.normal);
        const double s = LdotV - LdotN*VdotN;
        const double t = (s <= 0. ? 1. : Max(LdotN, VdotN));
        const double a = 1 - 0.5 * sigma2 / (sigma2 + 0.33);
        const double b = 0.45 * sigma2 / (sigma2 + 0.09);

        const double orenNayarCoeff = LdotN*(a + b*s/t);
        const double lightContribution = ShadingHelper::GetLightContribution(info, *light);
        result += diffuse*orenNayarCoeff*lightContribution;
    }

    return result;
}

OrenNayar::OrenNayar(const Color& color, double sigma)
: m_Color(color)
, m_Sigma(sigma)
{
}

OrenNayar::OrenNayar(Texture* texture, double sigma)
: m_Texture(texture)
, m_Sigma(sigma)
{
}

void OrenNayar::FillProperties(ParsedBlock& pb)
{
    pb.GetColorProp("color", &m_Color);
    pb.GetTextureProp("texture", &m_Texture);
    pb.GetDoubleProp("sigma", &m_Sigma);
}

Reflection::Reflection(double multiplier, double glossiness, int samples)
: m_Multiplier(multiplier)
, m_Glossiness(glossiness)
, m_Samples(samples)
{
}

extern Color Raytrace(const Ray& ray);
Color Reflection::Shade(const Ray& ray, const IntersectionInfo& info) const
{
    Vector n = Faceforward(ray.dir, info.normal);

    Color result(0, 0, 0);
    if (m_Glossiness == 1)
    {
        Ray newRay = ray;
        newRay.start = info.ip + n * 0.000001;
        newRay.dir = Reflect(ray.dir, n);
        newRay.depth++;

        result = Raytrace(newRay) * m_Multiplier;
    }
    else
    {
        int count = ray.depth > 0 ? 2 : m_Samples;
        for (int i = 0; i < count; ++i)
        {
            Vector a, b;
            OrthonormalSystem(n, a, b);

            double x, y;
            GenerateDiscPoint(x, y);

            const double k = tan((1 - m_Glossiness) * PI / 2);
            x *= k;
            y *= k;

            Vector modifiedNormal = n + a*x + b*y;

            Ray newRay = ray;
            newRay.start = info.ip + n * 0.000001;
            newRay.dir = Reflect(ray.dir, modifiedNormal);
            newRay.depth++;

            result += Raytrace(newRay) * m_Multiplier;
        }

        result /= count;
    }

    return result;
}

void Reflection::FillProperties(ParsedBlock& pb)
{
    pb.GetDoubleProp("multiplier", &m_Multiplier);
    pb.GetDoubleProp("glossiness", &m_Glossiness, 0., 1.);
    pb.GetIntProp("numSamples", &m_Samples, 1);
}

Refraction::Refraction(double inOutRatio, double multiplier)
: m_InOutRatio(inOutRatio)
, m_Multiplier(multiplier)
{
}

Color Refraction::Shade(const Ray& ray, const IntersectionInfo& info) const
{
    // ior = eta2 / eta1
    Vector refraction;
    if (Dot(ray.dir, info.normal) < 0.)
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

void Refraction::FillProperties(ParsedBlock& pb)
{
    pb.GetDoubleProp("multiplier", &m_Multiplier);
    pb.GetDoubleProp("ior", &m_InOutRatio, 1e-6, 10.);
}

Layered::Layered()
: m_NumLayers(0)
{
}

void Layered::AddLayer(Shader* shader, Color blend, Texture* texture)
{
    if (m_NumLayers == m_Layers.size())
        return;

    m_Layers[m_NumLayers] = {shader, blend, texture};
    ++m_NumLayers;
}

Color Layered::Shade(const Ray& ray, const IntersectionInfo& info) const
{
    Color result(0, 0, 0);
    for (unsigned i = 0; i < m_NumLayers; ++i)
    {
        const Layer& layer = m_Layers[i];
        Color fromLayer = layer.m_Shader->Shade(ray, info);
        Color blendAmount = layer.m_Blend;
        if (layer.m_Texture)
            blendAmount *= layer.m_Texture->Sample(info);

        result = blendAmount*fromLayer + (Colors::WHITE - blendAmount)*result;
    }

    return result;
}

void Layered::FillProperties(ParsedBlock& pb)
{
    char name[128];
    char value[256];
    int srcLine;
    for (int i = 0; i < pb.GetBlockLines(); i++)
    {
        // fetch and parse all lines like "layer <shader>, <color>[, <texture>]"
        pb.GetBlockLine(i, srcLine, name, value);
        if (!strcmp(name, "layer"))
        {
            char shaderName[200];
            char textureName[200] = "";
            bool err = false;
            if (!GetFrontToken(value, shaderName))
            {
                err = true;
            }
            else
            {
                StripPunctuation(shaderName);
            }

            if (!strlen(value)) err = true;

            if (!err && value[strlen(value) - 1] != ')')
            {
                if (!GetLastToken(value, textureName))
                {
                    err = true;
                }
                else
                {
                    StripPunctuation(textureName);
                }
            }

            if (!err && !strcmp(textureName, "nullptr"))
                strcpy(textureName, "");

            Shader* shader = nullptr;
            Texture* texture = nullptr;
            if (!err)
            {
                shader = pb.GetParser().FindShaderByName(shaderName);
                err = (shader == nullptr);
            }

            if (!err && strlen(textureName))
            {
                texture = pb.GetParser().FindTextureByName(textureName);
                err = (texture == nullptr);
            }

            if (err) throw SyntaxError(srcLine, "Expected a line like `layer <shader>, <color>[, <texture>]'");

            double x, y, z;
            Get3Doubles(srcLine, value, x, y, z);
            AddLayer(shader, Color((float) x, (float) y, (float) z), texture);
        }
    }
}


ConstColorShader::ConstColorShader(Color color)
: m_Color(color)
{
}

void ConstColorShader::FillProperties(ParsedBlock& pb)
{
    pb.GetColorProp("color", &m_Color);
}
