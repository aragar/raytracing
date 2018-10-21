#include <cmath>
#include <SDL.h>
#include <vector>

#include "camera.h"
#include "color.h"
#include "environment.h"
#include "geometry.h"
#include "sdl.h"
#include "shading.h"
#include "texture.h"
#include "utils.h"
#include "colors.h"

const bool g_WantAA = true;
const bool g_WantAdaptiveAA = true;
const bool g_ShowAA = false;
const double g_AAThreshold = .1;

Color vfb[VFB_MAX_SIZE][VFB_MAX_SIZE];
bool needChange[VFB_MAX_SIZE][VFB_MAX_SIZE] = {{false}};

Camera g_Camera;
std::vector<Node> g_Nodes;

std::vector<Light> g_Lights;
Color g_AmbientLight;

Environment* g_Environment = nullptr;
const int maxRaytraceDepth = 10;

void SetupScene()
{
    // camera
    g_Camera.SetPosition({0, 60, -120});
    g_Camera.SetYaw(0);
    g_Camera.SetPitch(-30);
    g_Camera.SetRoll(0);
    g_Camera.SetFOV(90);
    g_Camera.SetAspectRatio((double)GetFrameWidth() / GetFrameHeight());

    // light
    g_Lights.push_back(Light{ {120, 180, 0}, 10000 });
    // g_Lights.push_back(Light{{-600, 700, -350}, 800000});
    g_AmbientLight = Color{1, 1, 1}*0.1;

    // ground
    Plane* plane = new Plane(1., 100.);
    Texture* tiles = new BitmapTexture("../data/texture/wood.bmp", 100);
    Phong* floor = new Phong(tiles, 5.3, 20.);

    Layered* layeredFloor = new Layered;
    layeredFloor->AddLayer(floor, Colors::WHITE);
    layeredFloor->AddLayer(new Reflection(0.9), Colors::WHITE * 0.02);

    g_Nodes.push_back({ plane, layeredFloor });

    Sphere* sphere = new Sphere( {-30, 30, -30}, 27 );
    Reflection* reflection = new Reflection(0.9);

    Cube* cube = new Cube({30, 30, -50}, 15);
    Layered* glass = new Layered;
    const double inOutRatioGlass = 1.3;
    glass->AddLayer(new Refraction(inOutRatioGlass, 0.9), Colors::WHITE);
    glass->AddLayer(new Reflection(0.2), Colors::WHITE, new Fresnel(inOutRatioGlass));

    g_Nodes.push_back({ sphere, reflection });
    g_Nodes.push_back({ cube, glass });

    g_Environment = new CubemapEnvironment("../data/env/forest");

    // start
    g_Camera.FrameBegin();
}

Color Raytrace(const Ray& ray)
{
    if (ray.depth > maxRaytraceDepth)
        return Color{0, 0, 0};

    const Node* closestNode = nullptr;
    double closestDist = INF;
    IntersectionInfo closestInfo = IntersectionInfo();
    for ( const Node& node : g_Nodes )
    {
        IntersectionInfo info;
        if ( !node.geometry->Intersect(ray, info) )
            continue;

        if ( closestDist <= info.distance )
            continue;

        closestDist = info.distance;
        closestNode = &node;
        closestInfo = info;
    }

    Color result{0, 0, 0};
    if (closestNode)
        result = closestNode->shader->Shade(ray, closestInfo);
    else if (g_Environment)
        result = g_Environment->GetEnvironment(ray.dir);

    return result;
}

bool isTooDifferent(int x, int y)
{
    const int dx[] = {0,  0, -1, 1, -1, -1, 1,  1};
    const int dy[] = {1, -1,  0, 0, -1,  1, 1, -1};
    for (unsigned i = 0; i < COUNT_OF(dx); ++i)
    {
        unsigned nx = (unsigned) x + dx[i];
        unsigned ny = (unsigned) y + dy[i];
        if (VFB_MAX_SIZE <= nx || VFB_MAX_SIZE <= ny)
            continue;

        double diff = 0;
        for ( unsigned j = 0; j < 3; ++j )
        {
//            if (vfb[y][x][j] >= 1 && vfb[ny][nx][j] >= 1)
////                continue;

            diff += std::abs(vfb[y][x][j] - vfb[ny][nx][j]);
        }

        if (diff > g_AAThreshold)
            return true;
    }

    return false;
}

void render()
{
    const double kernel[5][2] = {
            {0.0, 0.0},
            {0.6, 0.0},
            {0.0, 0.6},
            {0.3, 0.3},
            {0.6, 0.6}
    };
    const int kernelSize = COUNT_OF(kernel);

    int frameWidth = GetFrameWidth();
    int frameHeight = GetFrameHeight();
    for ( int y = 0; y < frameHeight; ++y )
    {
        for ( int x = 0; x < frameWidth; ++x )
        {
            Ray ray = g_Camera.GetScreenRay(x, y);
            vfb[y][x] = Raytrace(ray);
        }

//        if (y % 10 == 0)
//            DisplayVFB(vfb);
    }

    DisplayVFB(vfb);
    if (!g_WantAA)
        return;

    for (int y = 0; y < frameHeight; ++y)
        for (int x = 0; x < frameWidth; ++x)
            needChange[x][y] = !g_WantAdaptiveAA || isTooDifferent(x, y);

    for (int y = 0; y < frameHeight; ++y)
    {
        for ( int x = 0; x < frameWidth; ++x )
        {
            if ( !needChange[x][y] )
                continue;

            if ( g_ShowAA )
            {
                vfb[y][x] = Color{0x0000FF};
            } else
            {
                Color result = vfb[y][x];
                for ( int i = 1; i < kernelSize; ++i )
                {
                    Ray ray = g_Camera.GetScreenRay(x + kernel[i][0], y + kernel[i][1]);
                    result += Raytrace(ray);
                }

                vfb[y][x] = result / double(kernelSize);
            }
        }

        if (y % 10 == 0)
            DisplayVFB(vfb);
    }

    DisplayVFB(vfb);
}

// don't remove main arguments, it's required by SDL
int main (int argc, char* argv[])
{
    InitGraphics(RESX, RESY);

    SetupScene();

//    const int rotations = 10;
//    for (int i = 0; i < rotations; ++i)
//    {
//        g_Camera.SetYaw(1. * i * 2*PI/rotations);
//        g_Camera.FrameBegin();

        Uint32 startTicks = SDL_GetTicks();
        render();
        Uint32 elapsedMs = SDL_GetTicks() - startTicks;
        printf("Render took %.2lfs\n", elapsedMs / 1000.);
        SetWindowCaption("Quad Damage: rendered in %.2fs\n", elapsedMs / 1000.f);

//        DisplayVFB(vfb);
//    }

    WaitForUserExit();

    CloseGraphics();

    printf("Exited cleanly\n");
    return 0;
}