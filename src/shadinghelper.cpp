#include "shadinghelper.h"

#include <vector>
#include "geometry.h"

extern Vector g_LightPos;
extern double g_LightIntensity;

extern std::vector<Node> g_Nodes;

double ShadingHelper::GetLightContribution(const IntersectionInfo& info)
{
    double distanceToLightSqr = (info.ip - g_LightPos).LengthSqr();

    Vector start = info.ip + info.normal*1e-6;
    if ( !ShadingHelper::CheckVisibility(start, g_LightPos) )
        return 0;

    double lightContribution = g_LightIntensity / distanceToLightSqr;
    return lightContribution;
}

bool ShadingHelper::CheckVisibility(const Vector& start, const Vector& end)
{
    Ray ray;
    ray.start = start;
    ray.dir = Normalize(end - start);

    double targetDistSq = (end - start).LengthSqr();

    for ( Node& node : g_Nodes )
    {
        IntersectionInfo info;
        if ( !node.geometry->Intersect(ray, info) )
            continue;

        if ( info.distance*info.distance < targetDistSq )
            return false;
    }

    return true;
}
