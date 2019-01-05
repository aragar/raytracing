#include "mesh.h"

#include <numeric>

#include "meshdata.h"

Mesh::Mesh(bool isFaceted, bool backCulling)
: m_Faceted(isFaceted)
, m_BackCulling(backCulling)
{
}

Mesh::~Mesh()
{
    if (m_BoundingGeometry)
    {
        delete m_BoundingGeometry;
        m_BoundingGeometry = nullptr;
    }
}

bool Mesh::Intersect(const Ray& ray, IntersectionInfo& outInfo) const
{
    if (m_BoundingGeometry && m_BoundingGeometry->Intersect(ray, outInfo) == false)
        return false;

    bool found = false;
    IntersectionInfo info;
    outInfo.distance = INF;
    for (const MeshTriangle& triangle : m_Triangles)
    {
        if (Intersect(ray, triangle, info) && info.distance < outInfo.distance)
        {
            found = true;
            outInfo = info;
        }
    }

    return found;
}

bool Mesh::IsInside(const Vector& point) const
{
    return false;
}

void Mesh::ComputeBoundingGeometry()
{
    Vector origin = std::accumulate(m_Vertices.begin(), m_Vertices.end(), Vector{0, 0, 0});
    origin /= m_Vertices.size();

    double maxRadius = 0.;
    for (const Vector& vertex : m_Vertices)
    {
        const double distSqr = (origin - vertex).LengthSqr();
        maxRadius = Max(maxRadius, distSqr);
    }

    m_BoundingGeometry = new Sphere(origin, sqrt(maxRadius));
}

double Det(const Vector& a, const Vector& b, const Vector& c)
{
    return (a^b)*c;
}

bool Mesh::Intersect(const Ray& ray, const MeshTriangle& triangle, IntersectionInfo& outInfo) const
{
    if (m_BackCulling && ray.dir * triangle.geometryNormal > 0)
        return false;

    const Vector& A = m_Vertices[triangle.vertices[0]];
    const Vector& B = m_Vertices[triangle.vertices[1]];
    const Vector& C = m_Vertices[triangle.vertices[2]];

    const Vector H = ray.start - A;
    const Vector& D = ray.dir;

    const double dcr = Det(B - A, C - A, -D);
    if (fabs(dcr) < 1e-12)
        return false;

    const double lambda2 = Det(H, C - A, -D) / dcr;
    const double lambda3 = Det(B - A, H, -D) / dcr;
    if (lambda2 < 0 || lambda3 < 0 || lambda2 + lambda3 > 1)
        return false;

    const double gamma = Det(B - A, C - A, H) / dcr;
    if (gamma < 0)
        return false;

    outInfo.distance = gamma;
    outInfo.ip = ray.start + ray.dir*gamma;

    if (!m_Faceted)
    {
        const Vector& nA = m_Normals[triangle.normals[0]];
        const Vector& nB = m_Normals[triangle.normals[1]];
        const Vector& nC = m_Normals[triangle.normals[2]];

        outInfo.normal = nA + lambda2*(nB - nA) + lambda3*(nC - nA);
        outInfo.normal.Normalize();
    }
    else
    {
        outInfo.normal = triangle.geometryNormal;
    }

    const Vector& uvA = m_UVs[triangle.uvs[0]];
    const Vector& uvB = m_UVs[triangle.uvs[1]];
    const Vector& uvC = m_UVs[triangle.uvs[2]];
    const Vector uv = uvA + lambda2*(uvB - uvA) + lambda3*(uvC - uvA);
    outInfo.u = uv[0];
    outInfo.v = uv[1];

    outInfo.geometry = this;

    return true;
}

void Mesh::FillProperties(ParsedBlock& pb)
{
    pb.GetBoolProp("faceted", &m_Faceted);
    pb.GetBoolProp("isTetraeder", &m_IsTetraeder);
}

void Mesh::BeginRender()
{
    if (m_IsTetraeder) GenerateTetraeder();
    else GenerateTruncatedIcosahedron();

    printf("Mesh loaded, %u triangles\n", m_Triangles.size());
    ComputeBoundingGeometry();
}
