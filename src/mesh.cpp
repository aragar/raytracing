#include "mesh.h"

#include <numeric>
#include <SDL.h>

Mesh::Mesh(bool isFaceted, bool backCulling)
: m_Faceted(isFaceted)
, m_BackCulling(backCulling)
{
}

Mesh::~Mesh()
{
    if (m_KDRoot)
        delete m_KDRoot;
}

bool Mesh::Intersect(const Ray& ray, IntersectionInfo& outInfo) const
{
    if (!m_BBox.TestIntersect(ray))
        return false;

    bool found = false;
    if (m_KDRoot)
    {
        outInfo.distance = INF;
        found = Intersect(m_KDRoot, m_BBox, ray, outInfo);
    }
    else
    {
        outInfo.distance = INF;
        for (const MeshTriangle& triangle : m_Triangles)
        {
            if (Intersect(ray, triangle, outInfo))
            {
                found = true;
            }
        }
    }

    return found;
}

bool Mesh::Intersect(KDTreeNode* node, BBox bbox, const Ray& ray, IntersectionInfo& outInfo) const
{
    bool result = false;
    if (node->axis == Axis::None)
    {
        bool found = false;
        for (int triangleIdx : *node->triangles)
            if (Intersect(ray, m_Triangles[triangleIdx], outInfo))
                found = true;

        result = (found && bbox.IsInside(outInfo.ip));
    }
    else
    {
        BBox childBBoxes[2];
        bbox.Split(node->axis, node->splitPosition, childBBoxes[0], childBBoxes[1]);

        int childOrder[2] = {0, 1};
        if (ray.start[static_cast<unsigned>(node->axis)] > node->splitPosition)
            std::swap(childOrder[0], childOrder[1]);

        for (unsigned i = 0; i < COUNT_OF(childBBoxes); ++i)
        {
            const BBox& childBBox = childBBoxes[childOrder[i]];
            if (childBBox.TestIntersect(ray) && Intersect(&node->children[childOrder[i]], childBBox, ray, outInfo))
            {
                result = true;
                break;
            }
        }
    }

    return result;
}

bool Mesh::IsInside(const Vector& point) const
{
    return false;
}

void Mesh::ComputeBoundingGeometry()
{
    m_BBox.MakeEmpty();
    for (const Vector& v : m_Vertices)
        m_BBox.Add(v);
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

    const double gamma = Det(B - A, C - A, H) / dcr;
    if (gamma < 0 || gamma > outInfo.distance)
        return false;

    const double lambda2 = Det(H, C - A, -D) / dcr;
    const double lambda3 = Det(B - A, H, -D) / dcr;
    if (lambda2 < 0 || lambda3 < 0 || lambda2 + lambda3 > 1)
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

    outInfo.dNdx = triangle.dNdx;
    outInfo.dNdy = triangle.dNdy;

    outInfo.geometry = this;

    return true;
}

void Mesh::FillProperties(ParsedBlock& pb)
{
    pb.GetBoolProp("faceted", &m_Faceted);
    pb.GetBoolProp("backCulling", &m_BackCulling);
    pb.GetBoolProp("useKDTree", &m_UseKDTree);

    pb.RequiredProp("file");

    char fn[256];
    pb.GetFilenameProp("file", fn);
    if (!LoadFromOBJ(fn))
        pb.SignalError("Could not parse OBJ file");
}

void Mesh::BeginRender()
{
    printf("Mesh loaded, %d triangles\n", int(m_Triangles.size()));
    ComputeBoundingGeometry();
    ComputeKDRoot();
}

void Mesh::ComputeKDRoot()
{
    if (!m_UseKDTree || m_Triangles.size() < 50)
        return;

    const Uint32 start = SDL_GetTicks();

    m_KDRoot = new KDTreeNode;
    std::vector<int> triangleList(m_Triangles.size());
    std::iota(triangleList.begin(), triangleList.end(), 0);
    BuildKD(m_KDRoot, m_BBox, triangleList, 0);

    const Uint32 end = SDL_GetTicks();
    printf(" -> KDTree built in %.2lfs\n", (end - start) / 1000.0);
}

void Mesh::BuildKD(KDTreeNode* node, BBox bbox, const std::vector<int>& triangleList, unsigned int depth) const
{
    if (depth > MAX_TREE_DEPTH || triangleList.size() < TRIANGLES_PER_LEAF)
    {
        node->InitLeaf(triangleList);
    }
    else
    {
        BuildKDNodeChildren(node, bbox, triangleList, depth);
    }
}

void Mesh::BuildKDNodeChildren(KDTreeNode* node, const BBox& bbox, const std::vector<int>& triangleList, unsigned int depth) const
{
    const Axis axis = static_cast<Axis>(depth % 3);
    const double leftLimit = bbox.GetMin(axis);
    const double rightLimit = bbox.GetMax(axis);

    const double optimalSplitPosition = (leftLimit + rightLimit) * 0.5; // TODO: could be better

    BBox bboxLeft, bboxRight;
    bbox.Split(axis, optimalSplitPosition, bboxLeft, bboxRight);

    std::vector<int> trianglesLeft, trianglesRight;
    for (int triangleIdx : triangleList)
    {
        const MeshTriangle& triangle = m_Triangles[triangleIdx];
        const Vector& A = m_Vertices[triangle.vertices[0]];
        const Vector& B = m_Vertices[triangle.vertices[1]];
        const Vector& C = m_Vertices[triangle.vertices[2]];

        if (bboxLeft.IntersectTriangle(A, B, C))
            trianglesLeft.push_back(triangleIdx);

        if (bboxRight.IntersectTriangle(A, B, C))
            trianglesRight.push_back(triangleIdx);
    }

    node->InitTreeNode(axis, optimalSplitPosition);

    BuildKD(&node->children[0], bboxLeft, trianglesLeft, depth + 1);
    BuildKD(&node->children[1], bboxRight, trianglesRight, depth + 1);
}

static void ParseOBJTrio(const std::string& s, int& vertex, int& uv, int& normal)
{
    std::vector<std::string> items = Split(s, '/');

    // "3", "3/4", "3//5", "3/4/5"  (v/uv/normal)
    vertex = ToInt(items[0]);
    uv = items.size() >= 2 ? ToInt(items[1]) : 0;
    normal = items.size() >= 3 ? ToInt(items[2]) : 0;
}

static MeshTriangle ParseOBJTriangle(const std::string& s0, const std::string& s1, const std::string& s2)
{
    MeshTriangle t;
    ParseOBJTrio(s0, t.vertices[0], t.uvs[0], t.normals[0]);
    ParseOBJTrio(s1, t.vertices[1], t.uvs[1], t.normals[1]);
    ParseOBJTrio(s2, t.vertices[2], t.uvs[2], t.normals[2]);

    return t;
}

static void Solve2D(const Vector& a, const Vector& b, const Vector& c, double& x, double& y)
{
    // solve x*a + y*b = c
    double m[2][2] = {{a.x, b.x}, {a.y, b.y}};
    double h[2] ={c.x, c.y};

    double Dcr = m[0][0]*m[1][1] - m[1][0]*m[0][1];
    x = (    h[0] * m[1][1] -    h[1] * m[0][1]) / Dcr;
    y = ( m[0][0] *    h[1] - m[1][0] *    h[0]) / Dcr;
}

void Mesh::GenerateTrianglesData()
{
    for (MeshTriangle& t : m_Triangles)
    {
        // geometry normal
        const Vector& a = m_Vertices[t.vertices[0]];
        const Vector& b = m_Vertices[t.vertices[1]];
        const Vector& c = m_Vertices[t.vertices[2]];

        const Vector ab = b - a;
        const Vector ac = c - a;

        t.geometryNormal = ab ^ ac;
        t.geometryNormal.Normalize();

        // partial derivatives
        // (1, 0) = px * texAB + qx * texAC; (1)
        // (0, 1) = py * texAB + qy * texAC; (2)

        const Vector& ta = m_UVs[t.uvs[0]];
        const Vector& tb = m_UVs[t.uvs[1]];
        const Vector& tc = m_UVs[t.uvs[2]];

        const Vector tab = tb - ta;
        const Vector tac = tc - ta;

        double px, py, qx, qy;
        Solve2D(tab, tac, Vector(1, 0, 0), px, qx); // (1)
        Solve2D(tab, tac, Vector(0, 1, 0), py, qy); // (2)

        t.dNdx = px*ab + qx*ac;
        t.dNdy = py*ab + qy*ac;

        t.dNdx.Normalize();
        t.dNdy.Normalize();
    }
}

bool Mesh::LoadFromOBJ(const char* filename)
{
    FILE* f = fopen(filename, "rt");
    FileRAII fraii(f);

    if (!f)
        return false;

    m_Vertices.clear();
    m_Normals.clear();
    m_UVs.clear();
    m_Triangles.clear();

    m_Vertices.push_back(Vector(0, 0, 0));
    m_UVs.push_back(Vector(0, 0, 0));
    m_Normals.push_back(Vector(0, 0, 0));

    char line[10000];
    while (fgets(line, sizeof(line), f))
    {
        std::vector<std::string> tokens = Tokenize(line);
        if (tokens.empty())
            continue;

              if (tokens[0] == "v" )  m_Vertices.push_back(Vector(ToDouble(tokens[1]), ToDouble(tokens[2]), ToDouble(tokens[3])));
         else if (tokens[0] == "vn")   m_Normals.push_back(Vector(ToDouble(tokens[1]), ToDouble(tokens[2]), ToDouble(tokens[3])));
         else if (tokens[0] == "vt")       m_UVs.push_back(Vector(ToDouble(tokens[1]), ToDouble(tokens[2]),                   0));
         else if (tokens[0] == "f" )
         {
             for (unsigned i = 0; i < tokens.size() - 3; ++i)
                 m_Triangles.push_back(ParseOBJTriangle(tokens[1], tokens[2 + i], tokens[3 + i]));
         }
    }

    GenerateTrianglesData();

    return true;
}

bool IntersectTriangleFast(const Ray& ray, const Vector& A, const Vector& B, const Vector& C, double& dist)
{
    Vector AB = B - A;
    Vector AC = C - A;
    Vector D = -ray.dir;
    Vector H = ray.start - A;

    /* 2. Solve the equation:
     *
     * A + lambda2 * AB + lambda3 * AC = ray.start + gamma * ray.dir
     *
     * which can be rearranged as:
     * lambda2 * AB + lambda3 * AC + gamma * D = ray.start - A
     *
     * Which is a linear system of three rows and three unknowns, which we solve using Carmer's rule
     */

    // find the determinant of the left part of the equation
    Vector ABcrossAC = AB^AC;
    double Dcr = ABcrossAC * D;

    // are the ray and the triangle parallel?
    if (fabs(Dcr) < 1e-12)
        return false;

    double gamma   = ( ABcrossAC * H) / Dcr;
    if (gamma < 0 || gamma > dist) // is intersection behind us, or too far?
        return false;

    double lambda2 = ( ( H ^ AC) * D) / Dcr;
    double lambda3 = ( (AB ^  H) * D) / Dcr;

    // is the intersection outside the triangle?
    if (lambda2 < 0 || lambda3 < 0 || lambda2 + lambda3 > 1)
        return false;

    dist = gamma;
    return true;
}