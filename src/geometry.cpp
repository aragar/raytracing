#include <algorithm>
#include <cstdio>
#include "geometry.h"

bool Plane::Intersect(const Ray& ray, IntersectionInfo& outInfo) const
{
    if ( ray.start.y > m_Height && ray.dir.y >= 0. )
        return false;

    if ( ray.start.y < m_Height && ray.dir.y <= 0. )
        return false;

    double scaleFactor = (m_Height - ray.start.y) / ray.dir.y;
    outInfo.ip = ray.start + ray.dir*scaleFactor;
    if (std::abs(outInfo.ip.x) > m_Limit || std::abs(outInfo.ip.z) > m_Limit)
        return false;

    outInfo.distance = scaleFactor;
    outInfo.normal = {0., ray.start.y > m_Height ? 1. : -1., 0.};
    outInfo.u = outInfo.ip.x;
    outInfo.v = outInfo.ip.z;
    outInfo.geometry = this;

    return true;
}

bool Plane::IsInside(const Vector& point) const
{
    // parallel to the XZ plane
    bool result = (point.y == m_Height);
    return result;
}

Plane::Plane(double height, double limit)
: m_Height(height)
, m_Limit(limit)
{
}

void Plane::FillProperties(ParsedBlock& pb)
{
    pb.GetDoubleProp("height", &m_Height);
    pb.GetDoubleProp("limit", &m_Limit);
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
    if ( p1 > 0 )
        p = p1;
    else if ( p2 > 0 )
        p = p2;
    else
        return false;

    outInfo.distance = p;
    outInfo.ip = ray.start + ray.dir*p;

    Vector posRelative = outInfo.ip - m_Center;
    outInfo.normal = Normalize(posRelative);

    outInfo.u = atan2(posRelative.z, posRelative.x);
    outInfo.u = (outInfo.u + PI) / (2*PI);

    outInfo.v = asin(posRelative.y / m_Radius);
    outInfo.v = -(outInfo.v + PI/2) / PI;

    outInfo.geometry = this;

    return true;
}

bool Sphere::IsInside(const Vector& point) const
{
    bool result = ((point - m_Center).LengthSqr() <= Sqr(m_Radius));
    return result;
}

Sphere::Sphere(const Vector& center, double radius)
: m_Center(center)
, m_Radius(radius)
{
}

void Sphere::FillProperties(ParsedBlock& pb)
{
    pb.GetVectorProp("center", &m_Center);
    pb.GetDoubleProp("radius", &m_Radius, 0.f);
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

bool Cube::IsInside(const Vector& point) const
{
    bool result = AreEqual(point.x, m_Center.x, m_HalfSide);
    result &= AreEqual(point.y, m_Center.y, m_HalfSide);
    result &= AreEqual(point.z, m_Center.z, m_HalfSide);
    return result;
}

Cube::Cube(const Vector& center, double halfSide)
: m_Center(center)
, m_HalfSide(halfSide)
{
}

void Cube::FillProperties(ParsedBlock& pb)
{
    pb.GetVectorProp("center", &m_Center);
    pb.GetDoubleProp("halfSide", &m_HalfSide);
}

bool CsgOp::Intersect(const Ray& ray, IntersectionInfo& outInfo) const
{
    IntersectionInfo leftInfo;
    IntersectionInfo rightInfo;

    bool inA = m_Left && m_Left->IsInside(ray.start);
    bool inB = m_Right && m_Right->IsInside(ray.start);
    bool currentPredicate = Operator(inA, inB);

    bool leftIntersection = m_Left && m_Left->Intersect(ray, leftInfo);
    bool rightIntersection = m_Right && m_Right->Intersect(ray, rightInfo);

    bool result = false;
    while ( leftIntersection || rightIntersection )
    {
        if ( leftIntersection && (!rightIntersection || leftInfo.distance <= rightInfo.distance) )
        {
            inA = !inA;
            bool nextPredicate = Operator(inA, inB);
            if ( nextPredicate != currentPredicate )
            {
                outInfo = leftInfo;
                result = true;
                break;
            }

            double prevDist = leftInfo.distance;
            leftIntersection = m_Left->Intersect({leftInfo.ip + ray.dir*1e-6, ray.dir}, leftInfo);
            if ( leftIntersection )
                leftInfo.distance += prevDist + 1e-6;
        }
        else if ( rightIntersection && (!leftIntersection || rightInfo.distance <= leftInfo.distance) )
        {
            inB = !inB;
            bool nextPredicate = Operator(inA, inB);
            if ( nextPredicate != currentPredicate )
            {
                outInfo = rightInfo;
                result = true;
                break;
            }

            double prevDist = rightInfo.distance;
            rightIntersection = m_Right->Intersect({rightInfo.ip + ray.dir*1e-6, ray.dir}, rightInfo);
            if ( rightIntersection )
                rightInfo.distance += prevDist + 1e-6;
        }
    }

    return result;
}

bool CsgOp::IsInside(const Vector& point) const
{
    bool inA = (m_Left && m_Left->IsInside(point));
    bool inB = (m_Right && m_Right->IsInside(point));
    bool result = Operator(inA, inB);
    return result;
}

CsgOp::CsgOp(Geometry* left, Geometry* right)
: m_Left(left)
, m_Right(right)
{
}

void CsgOp::FillProperties(ParsedBlock& pb)
{
    pb.RequiredProp("left");
    pb.RequiredProp("right");

    pb.GetGeometryProp("left", &m_Left);
    pb.GetGeometryProp("right", &m_Right);
}

bool RegularPolygon::Intersect(const Ray& ray, IntersectionInfo& outInfo) const
{
    if ( ray.start.y > m_Center.y && ray.dir.y >= 0. )
        return false;

    if ( ray.start.y < m_Center.y && ray.dir.y <= 0. )
        return false;

    double scaleFactor = (m_Center.y - ray.start.y) / ray.dir.y;
    Vector ip = ray.start + ray.dir*scaleFactor;

    Vector dir = ip - m_Center;
    double angle = atan2(dir.z, dir.x);
    if ( angle < 0 )
        angle += 2*PI;

    double sideAngle = 2*PI/m_Sides;
    unsigned k1 = angle/sideAngle; // the line between the center and the ip is intersecting the side from k to k+1
    unsigned k2 = (k1 + 1 == m_Sides ? 0 : k1 + 1);

    Vector vk1{m_Center.x + cos(k1*sideAngle)*m_Radius, m_Center.y, m_Center.z + sin(k1*sideAngle)*m_Radius};
    Vector vk2{m_Center.x + cos(k2*sideAngle)*m_Radius, m_Center.y, m_Center.z + sin(k2*sideAngle)*m_Radius};

    int s1 = SignOf(Cross(vk1 - m_Center, ip - vk1));
    int s2 = SignOf(Cross(vk2 - vk1, ip - vk2));
    int s3 = SignOf(Cross(m_Center - vk2, ip - m_Center));
    if ( !((s1 >= 0 && s2 >= 0 && s3 >= 0) || (s1 <= 0 && s2 <= 0 && s3 <= 0)) )
        return false;

    outInfo.ip = ip;
    outInfo.distance = scaleFactor;
    outInfo.normal = {0., ray.start.y > m_Center.y ? 1. : -1., 0.};
    outInfo.u = ip.x - m_Center.x;
    outInfo.v = ip.z - m_Center.z;
    outInfo.geometry = this;

    return true;
}

bool RegularPolygon::IsInside(const Vector& point) const
{
    // parallel to the XZ plane
    bool result = (point.y == m_Center.y);
    return result;
}

RegularPolygon::RegularPolygon(const Vector& center, double radius, unsigned int sides)
: m_Center(center)
, m_Radius(radius)
, m_Sides(Max(sides, 3))
{
}

void RegularPolygon::FillProperties(ParsedBlock& pb)
{
    pb.GetVectorProp("center", &m_Center);
    pb.GetDoubleProp("radius", &m_Radius, 0.);
    pb.GetUnsignedProp("sides", &m_Sides);
}

bool Node::Intersect(const Ray& ray, IntersectionInfo& outInfo) const
{
    // world space -> object's canonic space
    Ray rayCanonic = ray;
    rayCanonic.start = transform.UndoPoint(ray.start);
    rayCanonic.dir = transform.UndoDirection(ray.dir);

    double rayDirLength = rayCanonic.dir.Length();
    rayCanonic.dir.Normalize();
    if (!geometry->Intersect(rayCanonic, outInfo))
        return false;

    // The intersection found is in object space, convert to world space:
    outInfo.normal = transform.Normal(outInfo.normal);
    outInfo.normal.Normalize();
//	outInfo.dNdx = normalize(transform.direction(outInfo.dNdx));
//	outInfo.dNdy = normalize(transform.direction(outInfo.dNdy));
    outInfo.ip = transform.Point(outInfo.ip);
    outInfo.distance /= rayDirLength;  // (5)
    return true;
}

void Node::FillProperties(ParsedBlock& pb)
{
    pb.GetGeometryProp("geometry", &geometry);
    pb.GetShaderProp("shader", &shader);
    pb.GetTransformProp(transform);
    pb.GetTextureProp("bump", &bump);
}
