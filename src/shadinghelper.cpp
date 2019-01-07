#include "shadinghelper.h"

#include <vector>

#include "geometry.h"
#include "light.h"
#include "shading.h"

extern std::vector<Node> g_Nodes;

double ShadingHelper::GetLightContribution(const IntersectionInfo& info, const Light& light)
{
    const Vector start = info.ip + info.normal*1e-6;
    const float shadowTransparency = ShadingHelper::GetShadowTransparency(start, light.pos);
    const double distanceToLightSqr = (info.ip - light.pos).LengthSqr();
    const double lightContribution = shadowTransparency * light.intensity / distanceToLightSqr;
    return lightContribution;
}

float ShadingHelper::GetShadowTransparency(const Vector& start, const Vector& end)
{
    Ray ray;
    ray.start = start;
    ray.dir = Normalize(end - start);

    const double targetDistSq = (end - start).LengthSqr();

    float result = 1.f;
    for (Node* node : scene.nodes)
    {
        IntersectionInfo info;
        if (!node->Intersect(ray, info))
            continue;

        if (Sqr(info.distance) < targetDistSq)
            result *= node->shadowTransparency;
    }

    return result;
}
