#include "bbox.h"

#include "constants.h"
#include "utils.h"
#include "ray.h"

void BBox::MakeEmpty()
{
    m_Min.Set(+INF, +INF, +INF);
    m_Max.Set(-INF, -INF, -INF);
}

void BBox::Add(const Vector& point)
{
    m_Min.x = Min(m_Min.x, point.x);
    m_Min.y = Min(m_Min.y, point.y);
    m_Min.z = Min(m_Min.z, point.z);

    m_Max.x = Max(m_Max.x, point.x);
    m_Max.y = Max(m_Max.y, point.y);
    m_Max.z = Max(m_Max.z, point.z);
}

bool BBox::IsInside(const Vector& point) const
{
    bool result = IsBetween(point.x, m_Min.x, m_Max.x)
               && IsBetween(point.y, m_Min.y, m_Max.y)
               && IsBetween(point.z, m_Min.z, m_Max.z);

    return result;
}

bool BBox::TestIntersect(const Ray& ray) const
{
    if (IsInside(ray.start))
        return  true;

    for (unsigned dim = 0; dim < 3; ++dim)
    {
        if (ray.dir[dim] < 0 && ray.start[dim] < m_Min[dim])
            continue;

        if (ray.dir[dim] > 0 && ray.start[dim] > m_Max[dim])
            continue;

        if (IsZero(ray.dir[dim], 1e-9))
            continue;

        const double mul = 1. / ray.dir[dim];
        const int u = (dim == 0) ? 1 : 0;
        const int v = (dim == 2) ? 1 : 2;

        double dist, x, y;
        /* (*) this is a good optimization I found out by chance. Consider the following scenario
			 *
			 *   ---+  ^  (ray)
			 *      |   \
			 * bbox |    \
			 *      |     \
			 *      |      * (camera)
			 * -----+
			 *
			 * if we're considering the walls up and down of the bbox (which belong to the same axis),
			 * the optimization in (*) says that we can skip testing with the "up" wall, if the "down"
			 * wall is behind us. The rationale for that is, that we can never intersect the "down" wall,
			 * and even if we have the chance to intersect the "up" wall, we'd be intersection the "right"
			 * wall first. So we can just skip any further intersection tests for this axis.
			 * This may seem bogus at first, as it doesn't work if the camera is inside the BBox, but then we would
			 * have quitted the function because of the inside(ray.start) condition in the first line of the function.
             */
        dist = (m_Min[dim] - ray.start[dim])*mul;
        if (dist < 0)
            continue;

        x = ray.start[u] + ray.dir[u]*dist;
        if (IsBetween(x, m_Min[u], m_Max[u], 0))
        {
            y = ray.start[v] + ray.dir[v]*dist;
            if (IsBetween(y, m_Min[v], m_Max[v], 0))
                return true;
        }

        dist = (m_Max[dim] - ray.start[dim])*mul;
        if (dist < 0)
            continue;

        x = ray.start[u] + ray.dir[u]*dist;
        if (IsBetween(x, m_Min[u], m_Max[u], 0))
        {
            y = ray.start[v] + ray.dir[v]*dist;
            if (IsBetween(y, m_Min[v], m_Max[v], 0))
                return true;
        }
    }

    return false;
}

double BBox::ClosestIntersection(const Ray& ray) const
{
    if (IsInside(ray.start))
        return 0.;

    double minDist = INF;
    for (unsigned dim = 0; dim < 3; ++dim)
    {
        if (ray.dir[dim] < 0 && ray.start[dim] < m_Min[dim])
            continue;

        if (ray.dir[dim] > 0 && ray.start[dim] > m_Max[dim])
            continue;

        if (IsZero(ray.dir[dim], 1e-9))
            continue;

        const double mul = 1. / ray.dir[dim];
        const double xs[2] = { m_Min[dim], m_Max[dim] };
        const int u = (dim == 0) ? 1 : 0;
        const int v = (dim == 2) ? 1 : 2;
        for (unsigned j = 0; j < 2; ++j)
        {
            const double dist = (xs[j] - ray.start[dim])*mul;
            if (dist < 0)
                continue;

            const double x = ray.start[u] + ray.dir[u]*dist;
            if (IsBetween(x, m_Min[u], m_Max[u], 0))
            {
                const double y = ray.start[v] + ray.dir[v]*dist;
                if (IsBetween(y, m_Min[v], m_Max[v], 0))
                    minDist = Min(minDist, dist);
            }
        }
    }

    return minDist;
}


bool BBox::IntersectTriangle(const Vector& a, const Vector& b, const Vector& c) const
{
    if (IsInside(a) || IsInside(b) || IsInside(c))
        return true;

    Ray ray;
    Vector t[3] = { a, b, c };
    for (int i = 0; i < 3; ++i)
        for (int j = i + 1; j < 3; ++j)
        {
            ray.start = t[i];
            ray.dir = t[j] - t[i];
            if (TestIntersect(ray))
            {
                ray.start = t[j];
                ray.dir = t[i] - t[j];
                if (TestIntersect(ray))
                    return true;
            }
        }

    const Vector ab = b - a;
    const Vector ac = c - a;
    const Vector ABCrossAC = ab ^ ac;
    const double d = a * ABCrossAC;
    for (int mask = 0; mask < 7; ++mask)
    {
        for (int j = 0; j < 3; ++j)
        {
            if (mask & (1 << j))
                continue;

            ray.start.Set((mask & 1) ? m_Max.x : m_Min.x
                        , (mask & 2) ? m_Max.y : m_Min.y
                        , (mask & 4) ? m_Max.z : m_Min.z);

            Vector rayEnd = ray.start;
            rayEnd[j] = m_Max[j];
            if (SignOf(ray.start * ABCrossAC - d) != SignOf(rayEnd * ABCrossAC - d))
            {
                ray.dir = rayEnd - ray.start;
                double gamma = 1.0000001;
                if (IntersectTriangleFast(ray, a, b, c, gamma))
                    return true;
            }
        }
    }

    return false;
}

void BBox::Split(Axis axis, double where, BBox& left, BBox& right) const
{
    const unsigned index = static_cast<unsigned>(axis);

    left = *this;
    left.m_Max[index] = where;

    right = *this;
    right.m_Min[index] = where;
}
