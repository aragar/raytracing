#include "shadinghelper.h"

#include <vector>
#include "geometry.h"
#include "shading.h"

extern std::vector<Node> g_Nodes;

double ShadingHelper::GetLightContribution(const IntersectionInfo& info, const Light& light)
{
    double distanceToLightSqr = (info.ip - light.pos).LengthSqr();

    Vector start = info.ip + info.normal*1e-6;
    if ( !ShadingHelper::CheckVisibility(start, light.pos) )
        return 0;

    double lightContribution = light.intensity / distanceToLightSqr;
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
