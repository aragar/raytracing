#include "heightfield.h"

#include "bitmap.h"
#include "utils.h"

#include <algorithm>
#include <cmath>

Heightfield::~Heightfield()
{
    if (m_Heights)
        delete[] m_Heights;

    if (m_MaxHeights)
        delete[] m_MaxHeights;

    if (m_Normals)
        delete[] m_Normals;
}

bool Heightfield::Intersect(const Ray& ray, IntersectionInfo& outInfo) const
{
    const Vector step = ray.dir / (sqrt(Sqr(ray.dir.x) + Sqr(ray.dir.z)));
    const double dist = m_BBox.ClosestIntersection(ray);

    const double mx = 1.0 / ray.dir.x; // mx = how much to go along ray.dir until the unit distance along X is traversed
    const double mz = 1.0 / ray.dir.z; // same as mx, for Z

    Vector p = ray.start + ray.dir * (dist + 1e-6); // step firmly inside the bbox
    while (m_BBox.IsInside(p))
    {
        const unsigned x0 = (unsigned) floor(p.x);
        const unsigned z0 = (unsigned) floor(p.z);
        if (x0 < 0 || x0 >= m_Width || z0 < 0 || z0 >= m_Height)
            break; // if outside the [0..W)x[0..H) rect, get out

        // calculate how much we need to go along ray.dir until we hit the next X voxel boundary:
        const double lx = ray.dir.x > 0 ? (ceil(p.x) - p.x) * mx : (floor(p.x) - p.x) * mx;
        // same as lx, for the Z direction:
        const double lz = ray.dir.z > 0 ? (ceil(p.z) - p.z) * mz : (floor(p.z) - p.z) * mz;
        // advance p along ray.dir until we hit the next X or Z gridline
        // also, go a little more than that, to assure we're firmly inside the next voxel:
        const Vector p_next = p + step * (std::min(lx, lz) + 1e-6);
        // "p" is position before advancement; p_next is after we take a single step.
        // if any of those are below the height of the nearest four voxels of the heightfield,
        // we need to test the current voxel for intersection:
        if (std::min(p.y, p_next.y) < m_MaxHeights[z0*m_Width + x0])
        {
            double closestDist = INF;
            // form ABCD - the four corners of the current voxel, whose heights are taken from the heightmap
            // then form triangles ABD and BCD and try to intersect the ray with each of them:
            Vector A = Vector(x0    , GetHeight(x0    , z0    ), z0    );
            Vector B = Vector(x0 + 1, GetHeight(x0 + 1, z0    ), z0    );
            Vector C = Vector(x0 + 1, GetHeight(x0 + 1, z0 + 1), z0 + 1);
            Vector D = Vector(x0    , GetHeight(x0    , z0 + 1), z0 + 1);
            if (IntersectTriangleFast(ray, A, B, D, closestDist) ||
                IntersectTriangleFast(ray, B, C, D, closestDist))
            {
                // intersection found: ray hits either triangle ABD or BCD. Which one exactly isn't
                // important, because we calculate the normals by bilinear interpolation of the
                // precalculated normals at the four corners:
                outInfo.distance = closestDist;
                outInfo.ip = ray.start + ray.dir * closestDist;
                outInfo.normal = GetNormal(static_cast<float>(outInfo.ip.x), static_cast<float>(outInfo.ip.z));
                outInfo.u = outInfo.ip.x / m_Width;
                outInfo.v = outInfo.ip.z / m_Height;
                outInfo.dNdx = Vector(1, 0, 0);
                outInfo.dNdy = Vector(0, 0, 1);
                outInfo.geometry = this;
                return true;
            }
        }

        p = p_next;
    }

    return false;
}

void Heightfield::FillProperties(ParsedBlock& pb)
{
    Bitmap bmp;
    pb.RequiredProp("file");
    pb.GetBitmapFileProp("file", bmp);
    m_Width = bmp.GetWidth();
    m_Height = bmp.GetHeight();

    double blur = 0.;
    pb.GetDoubleProp("blur", &blur, 0, 1000);

    m_Heights = new float[m_Width*m_Height];
    float minY = LARGE_FLOAT;
    float maxY = -LARGE_FLOAT;
    BlurImage(bmp, blur, minY, maxY);

    m_BBox.SetMin({0, minY, 0});
    m_BBox.SetMax({double(m_Width), maxY, double(m_Height)});

    PopulateMaxHeights();
    PopulateNormals();
}

void Heightfield::PopulateNormals()
{
    m_Normals = new Vector[m_Width * m_Height];
    for (unsigned y = 0; y < m_Height - 1; ++y)
        for (unsigned x = 0; x < m_Width - 1; ++x)
        {
            float h0 = m_Heights[y*m_Width + x];
            float hdx = m_Heights[y*m_Width + x + 1];
            float hdy = m_Heights[(y + 1)*m_Width + x];
            Vector vdx{1, hdx - h0, 0};
            Vector vdy{0, hdy - h0, 1};
            Vector norm = vdy^vdx;
            norm.Normalize();

            m_Normals[y*m_Width + x] = norm;
        }

    for (unsigned y = 0; y < m_Height; ++y)
        m_Normals[y*m_Width + m_Width - 1] = m_Normals[y*m_Width + m_Width - 2];

    for (unsigned x = 0; x < m_Width; ++x)
        m_Normals[(m_Height - 1)*m_Width + x] = m_Normals[(m_Height - 2)*m_Width + x];
}

void Heightfield::PopulateMaxHeights()
{
    m_MaxHeights = new float[m_Width * m_Height];
    for (unsigned y = 0; y < m_Height; ++y)
        for (unsigned x = 0; x < m_Width; ++x)
        {
            float& maxHeight = m_MaxHeights[y * m_Width + x];
            maxHeight = m_Heights[y * m_Width + x];
            if (x < m_Width - 1)
                maxHeight = std::max(maxHeight, m_Heights[y * m_Width + x + 1]);
            if (y < m_Height - 1)
            {
                maxHeight = std::max(maxHeight, m_Heights[(y + 1) * m_Width + x]);
                if (x < m_Width - 1)
                    maxHeight = std::max(maxHeight, m_Heights[(y + 1) * m_Width + x + 1]);
            }
        }
}

void Heightfield::BlurImage(Bitmap& bmp, double blur, float& outMinY, float& outMaxY) const
{
    if (blur <= 0.) // just fetch the source image
    {
        for (unsigned y = 0; y < m_Height; ++y)
            for (unsigned x = 0; x < m_Width; ++x)
            {
                float h = bmp.GetPixel(x, y).Intensity();
                m_Heights[y * m_Width + x] = h;
                outMinY = std::min(outMinY, h);
                outMaxY = std::max(outMaxY, h);
            }
    }
    else
    {
        // 1. convert the image to grayscale
        for (unsigned y = 0; y < m_Height; ++y)
            for (unsigned x = 0; x < m_Width; ++x)
            {
                float f = bmp.GetPixel(x, y).Intensity();
                bmp.SetPixel(x, y, Color(f, f, f));
            }

        // 2. calculate the gaussian coefficent - see http://en.wikipedia.org/wiki/Gaussian_blur
        static float gauss[128][128];
        int r = std::min(128, NearestInt(3 * blur));
        for (int y = 0; y < r; ++y)
            for (int x = 0; x < r; ++x)
                gauss[y][x] = static_cast<float>(exp(-(Sqr(x) + Sqr(y))/(2*Sqr(blur))) / (2*PI*Sqr(blur)));

        // 3. apply gaussian blur with the specified number of blur units (slow for big r)
        for (unsigned y = 0; y < m_Height; ++y)
            for (unsigned x = 0; x < m_Width; ++x)
            {
                float sum = 0.f;
                for (int dy = -r + 1; dy < r; ++dy)
                    for (int dx = -r + 1; dx < r; ++dx)
                        sum += gauss[abs(dy)][abs(dx)] * bmp.GetPixel(x + dx, y + dy).r;

                m_Heights[y * m_Width + x] = sum;
                outMinY = std::min(outMinY, sum);
                outMaxY = std::max(outMaxY, sum);
            }
    }
}

float Heightfield::GetHeight(unsigned int x, unsigned int y) const
{
    x = std::min(m_Width - 1, x);
    y = std::min(m_Height - 1, y);
    return m_Heights[y*m_Width + x];
}

Vector Heightfield::GetNormal(float x, float y) const
{
    const unsigned int x0 = static_cast<unsigned int>(floor(x));
    const unsigned int y0 = static_cast<unsigned int>(floor(y));
    const float p = x - x0;
    const float q = y - y0;
    const unsigned int x1 = std::min(m_Width - 1, x0 + 1);
    const unsigned int y1 = std::min(m_Height - 1, y0 + 1);

    Vector v = m_Normals[y0*m_Width + x0]*((1 - p)*(1 - q))
             + m_Normals[y0*m_Width + x1]*((    p)*(1 - q))
             + m_Normals[y1*m_Width + x0]*((1 - p)*(    q))
             + m_Normals[y1*m_Width + x1]*((    p)*(    q));

    return v;
}