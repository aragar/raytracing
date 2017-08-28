#include "geometry.h"

bool Plane::Intersect(const Ray& ray, IntersectionInfo& outInfo) const
{
    if ( ray.start.y > this->y && ray.dir.y >= 0. )
        return false;

    if ( ray.start.y < this->y && ray.dir.y <= 0. )
        return false;

    double scaleFactor = (this->y - ray.start.y) / ray.dir.y;
    outInfo.ip = ray.start + ray.dir*scaleFactor;
    outInfo.distance = scaleFactor;
    outInfo.normal = {0., ray.start.y > this->y ? 1. : -1., 0.};
    outInfo.u = outInfo.ip.x;
    outInfo.v = outInfo.ip.z;

    return true;
}
