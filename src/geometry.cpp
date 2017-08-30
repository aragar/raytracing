#include <algorithm>
#include "geometry.h"
#include "constants.h"
#include "utils.h"

bool Plane::Intersect(const Ray& ray, IntersectionInfo& outInfo) const
{
    if ( ray.start.y > m_Height && ray.dir.y >= 0. )
        return false;

    if ( ray.start.y < m_Height && ray.dir.y <= 0. )
        return false;

    double scaleFactor = (m_Height - ray.start.y) / ray.dir.y;
    outInfo.ip = ray.start + ray.dir*scaleFactor;
    outInfo.distance = scaleFactor;
    outInfo.normal = {0., ray.start.y > m_Height ? 1. : -1., 0.};
    outInfo.u = outInfo.ip.x;
    outInfo.v = outInfo.ip.z;

    return true;
}

bool Sphere::Intersect(const Ray& ray, IntersectionInfo& outInfo) const
{
    Vector H = ray.start - m_Center;
    double A = 1;
    double B = 2*Dot(H, ray.dir);
    double C = H.LengthSqr() - Sqr(m_Radius);

    double discr = Sqr(B) - 4*A*C;
    if ( discr < 0 )
        return false;

    double p1, p2;
    p1 = (-B - sqrt(discr)) / (2*A);
    p2 = (-B + sqrt(discr)) / (2*A);

    double p;
    bool backNormal = false;
    if ( p1 > 0 )
        p = p1;
    else if ( p2 > 0 )
    {
        p = p2;
        backNormal = true;
    }
    else
        return false;

    outInfo.distance = p;
    outInfo.ip = ray.start + ray.dir*p;

    Vector posRelative = outInfo.ip - m_Center;
    outInfo.normal = Normalize(posRelative);
    if ( backNormal )
        outInfo.normal = -outInfo.normal;

    outInfo.u = atan2(posRelative.z, posRelative.x);
    outInfo.u = (outInfo.u + PI) / (2*PI);

    outInfo.v = asin(posRelative.y / m_Radius);
    outInfo.v = (outInfo.v + PI/2) / PI;
    outInfo.u = outInfo.v = 0;

    outInfo.geometry = this;

    return true;
}

bool Cube::IntersectSide(double level, double start, double dir, const Ray& ray, const Vector& normal, IntersectionInfo& outInfo) const
{
    if ( start > level && dir >= 0 )
        return false;
    if ( start < level && dir <= 0 )
        return false;

    double scaleFactor = (level - start) / dir;
    Vector ip = ray.start + ray.dir*scaleFactor;

    if ( ip.y > m_Center.y + m_HalfSide + 1e-6 ) return false;
    if ( ip.y < m_Center.y - m_HalfSide - 1e-6 ) return false;

    if ( ip.x > m_Center.x + m_HalfSide + 1e-6 ) return false;
    if ( ip.x < m_Center.x - m_HalfSide - 1e-6 ) return false;

    if ( ip.z > m_Center.z + m_HalfSide + 1e-6 ) return false;
    if ( ip.z < m_Center.z - m_HalfSide - 1e-6 ) return false;

    double distance = scaleFactor;
    if ( outInfo.distance <= distance )
        return false;

    outInfo.ip = ip;
    outInfo.distance = distance;
    outInfo.normal = normal;
    outInfo.u = outInfo.ip.x + outInfo.ip.z;
    outInfo.v = outInfo.ip.y;
    outInfo.geometry = this;

    return true;
}

bool Cube::Intersect(const Ray& ray, IntersectionInfo& outInfo) const
{
    outInfo.distance = INF;
    IntersectSide(m_Center.x - m_HalfSide, ray.start.x, ray.dir.x, ray, {-1, 0, 0}, outInfo);
    IntersectSide(m_Center.x + m_HalfSide, ray.start.x, ray.dir.x, ray, {+1, 0, 0}, outInfo);
    IntersectSide(m_Center.y - m_HalfSide, ray.start.y, ray.dir.y, ray, { 0,-1, 0}, outInfo);
    IntersectSide(m_Center.y + m_HalfSide, ray.start.y, ray.dir.y, ray, { 0,+1, 0}, outInfo);
    IntersectSide(m_Center.z - m_HalfSide, ray.start.z, ray.dir.z, ray, { 0, 0,-1}, outInfo);
    IntersectSide(m_Center.z + m_HalfSide, ray.start.z, ray.dir.z, ray, { 0, 0,+1}, outInfo);

    bool result = (outInfo.distance < INF);
    return result;
}

void CsgOp::FindAllIntersections(Ray ray, const Geometry& geometry, std::vector<IntersectionInfo>& ips) const
{
    IntersectionInfo outInfo;
    int counter = 30;
    while ( geometry.Intersect(ray, outInfo)  && counter-- > 0 )
    {
        ips.push_back(outInfo);
        ray.start = outInfo.ip + ray.dir*1e-6;
    }

    for ( unsigned i = 1; i < ips.size(); ++i )
        ips[i].distance += ips[i - 1].distance + 1e-6;
}

std::vector<IntersectionInfo> MergeInfos(const std::vector<IntersectionInfo>& lhs, const std::vector<IntersectionInfo>& rhs)
{
    std::vector<IntersectionInfo> allInfos;
    allInfos.reserve(lhs.size() + rhs.size());
    for ( unsigned i = 0, j = 0; i < lhs.size() || j < rhs.size(); )
    {
        if ( i == lhs.size() )
        {
            allInfos.push_back(rhs[j]);
            ++j;
        }
        else if ( j == rhs.size() )
        {
            allInfos.push_back(lhs[i]);
            ++i;
        }
        else if ( lhs[i].distance <= rhs[j].distance )
        {
            allInfos.push_back(lhs[i]);
            ++i;
        }
        else
        {
            allInfos.push_back(rhs[j]);
            ++j;
        }
    }

    return allInfos;
}

std::vector<IntersectionInfo> SortInfos(const std::vector<IntersectionInfo>& lhs, const std::vector<IntersectionInfo>& rhs)
{
    std::vector<IntersectionInfo> allInfos = lhs;
    allInfos.insert(allInfos.end(), rhs.begin(), rhs.end());

    std::sort(allInfos.begin(), allInfos.end(),
                [](const IntersectionInfo& left, const IntersectionInfo& right)
                {
                    return left.distance <= right.distance;
                });

    return allInfos;
}

bool CsgOp::Intersect(const Ray& ray, IntersectionInfo& outInfo) const
{
    std::vector<IntersectionInfo> leftInfos, rightInfos;
    FindAllIntersections(ray, *m_Left, leftInfos);
    FindAllIntersections(ray, *m_Right, rightInfos);

    bool inA = leftInfos.size() % 2 == 0 ? false : true;
    bool inB = rightInfos.size() % 2 == 0 ? false : true;

    std::vector<IntersectionInfo> allInfos = MergeInfos(leftInfos, rightInfos);
//    std::vector<IntersectionInfo> allInfos = SortInfos(leftInfos, rightInfos);

    bool currentPredicate = Operator(inA, inB);
    for ( const IntersectionInfo& info : allInfos )
    {
        if ( info.geometry == m_Left )
            inA = !inA;
        else if ( info.geometry == m_Right )
            inB = !inB;

        bool nextPredicate = Operator(inA, inB);
        if ( nextPredicate == currentPredicate )
            continue;

        outInfo = info;
        return true;
    }

    return false;
}
