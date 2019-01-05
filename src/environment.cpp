#include <cstring>

#include "bitmap.h"
#include "environment.h"
#include "utils.h"

bool CubemapEnvironment::LoadMaps(const char* folder)
{
    // the maps are stored in order - negx, negy, negz, posx, posy, posz
    const char* prefixes[2] = {"neg", "pos"};
    const char* axes[3] = {"x", "y", "z"};
    const char* suffixes[2] = {".bmp", ".exr"};

    int n = 0;
    for (const char* prefix : prefixes)
        for (const char* axis : axes)
        {
            Bitmap* map = new Bitmap;
            char filename[256];
            for (const char* suffix : suffixes)
            {
                sprintf(filename, "%s/%s%s%s", folder, prefix, axis, suffix);
                if (FileExists(filename) && map->LoadImage(filename))
                    break;
            }

            if (!map->IsOK())
            {
                UnloadMaps();
                return false;
            }

            m_Maps[n++] = map;
        }

    return true;
}

CubemapEnvironment::~CubemapEnvironment()
{
    UnloadMaps();
}

void CubemapEnvironment::UnloadMaps()
{
    for (Bitmap* map : m_Maps)
    {
        if (!map)
            continue;

        delete map;
        map = nullptr;
    }
}

// a helper function (see getEnvironment()) that accepts two coordinates within the square
// (-1, -1) .. (+1, +1), and transforms them to (0, 0)..(W, H) where W, H are the bitmap width and height.
Color CubemapEnvironment::GetSide(const Bitmap& bmp, double x, double y) const
{
    const unsigned width = bmp.GetWidth();
    const double nx = (x + 1) * 0.5 * (width - 1);

    const unsigned height = bmp.GetHeight();
    const double ny = (y + 1) * 0.5 * (height - 1);

    const Color color = m_UseBilinearFiltering ? bmp.GetBilinearFilteredPixel(nx, ny)
                                               : bmp.GetPixel(static_cast<unsigned>(nx), static_cast<unsigned>(ny));
    return color;
}
Color CubemapEnvironment::GetEnvironment(const Vector& indir) const
{
    Vector vec = indir;
    // Get a color from a cube-map
    // First, we get at which dimension, the absolute value of the direction is largest
    // (it is 0, 1 or 2, which is, respectively, X, Y or Z)
    const int maxDim = vec.MaxDimension();

    // Normalize the vector, so that now its largest dimension is either +1, or -1
    const Vector t = vec / std::abs(vec[maxDim]);

    // Create a state of (maximalDimension * 2) + (1 if it is negative)
    // so that:
    // if state is 0, the max dimension is 0 (X) and it is positive -> we hit the +X side
    // if state is 1, the max dimension is 0 (X) and it is negative -> we hit the -X side
    // if state is 2, the max dimension is 1 (Y) and it is positive -> we hit the +Y side
    // state is 3 -> -Y
    // state is 4 -> +Z
    // state is 5 -> -Z
    int state = ((t[maxDim] < 0) ? 0 : 3) + maxDim;
    switch (state)
    {
        // for each case, we have to use the other two dimensions as coordinates within the bitmap for
        // that side. The ordering of plusses and minuses is specific for the arrangement of
        // bitmaps we use (the orientations are specific for vertical-cross type format, where each
        // cube side is taken verbatim from a 3:4 image of V-cross environment texture.

        // In every case, the other two coordinates are real numbers in the square (-1, -1)..(+1, +1)
        // We use the getSide() helper function, to convert these coordinates to texture coordinates and fetch
        // the color value from the bitmap.
        case 0: return GetSide(*m_Maps[static_cast<int>(CubeOrder::NEGX)],  t.z, -t.y);
        case 1: return GetSide(*m_Maps[static_cast<int>(CubeOrder::NEGY)],  t.x, -t.z);
        case 2: return GetSide(*m_Maps[static_cast<int>(CubeOrder::NEGZ)],  t.x,  t.y);
        case 3: return GetSide(*m_Maps[static_cast<int>(CubeOrder::POSX)], -t.z, -t.y);
        case 4: return GetSide(*m_Maps[static_cast<int>(CubeOrder::POSY)],  t.x,  t.z);
        case 5: return GetSide(*m_Maps[static_cast<int>(CubeOrder::POSZ)],  t.x, -t.y);
        default: return Color(0.0f, 0.0f, 0.0f);
    }
}

void CubemapEnvironment::FillProperties(ParsedBlock& pb)
{
    Environment::FillProperties(pb);

    char folder[256];

    pb.RequiredProp("folder");
    pb.GetFilenameProp("folder", folder);

    if (!LoadMaps(folder))
        fprintf(stderr, "CubemapEnvironment: Could not load maps from `%s'\n", folder);
}
