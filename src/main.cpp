#include <cmath>
#include <SDL.h>
#include <vector>

#include "camera.h"
#include "color.h"
#include "colors.h"
#include "environment.h"
#include "geometry.h"
#include "mesh.h"
#include "meshdata.h"
#include "sdl.h"
#include "shading.h"
#include "texture.h"
#include "utils.h"

const bool g_WantAA = true;
const bool g_WantAdaptiveAA = true;
const bool g_ShowAA = false;
const double g_AAThreshold = .1;

const bool g_WantProgressiveDisplay = true;
const int g_ProgressiveDisplayDelay = 100;

const bool g_UseStandartRGB = false;

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
    g_Camera.SetPosition({45, 120, -300});
    g_Camera.SetYaw(5);
    g_Camera.SetPitch(-5);
    g_Camera.SetRoll(0);
    g_Camera.SetFOV(60);
    g_Camera.SetAspectRatio((double)GetFrameWidth() / GetFrameHeight());

    // start
    g_Camera.FrameBegin();

    // light
    g_Lights.push_back(Light{ {-90, 1200, -750}, 1200000 });
    g_AmbientLight = Color{1, 1, 1}*0.5;

    // plane
    Plane* plane = new Plane(1., 200.);
    Texture* texture = new BitmapTexture("../data/texture/wood.bmp", 100, true /*use bilinear filtering*/);
    Layered* planeShader = new Layered;
    planeShader->AddLayer(new Lambert(texture), Colors::WHITE);
    planeShader->AddLayer(new Reflection, Colors::WHITE * 0.05, new Fresnel(1.33));
    g_Nodes.push_back({ plane, planeShader });

    // football
    Mesh* mesh = GenerateTruncatedIcosahedron();
    mesh->SetBackCulling(true);
    mesh->SetFaceted(false);
    mesh->Translate({-100, 50, 0});
    mesh->ComputeBoundingGeometry();

    CheckerTexture* checker = new CheckerTexture(Colors::WHITE * 0.7, Colors::WHITE * 0.15, 1);
    Lambert* meshShader = new Lambert(checker);
    g_Nodes.push_back({ mesh, meshShader });

    // sphere
    Sphere* sphere = new Sphere({100, 50, 50}, 50);
    Shader* glossy = new Reflection(0.9, 0.95, 25);
    g_Nodes.push_back({ sphere, glossy });

    Color colors[3] = { Colors::RED, Colors::YELLOW, Colors::GREEN};
    for (int i = 0; i < COUNT_OF(colors); ++i)
    {
        Color& color = colors[i];
        color.AdjustSaturation(0.9f);
        Sphere* ball = new Sphere({10. + 32*i, 15, 0}, 15.);
        Shader* ballShader = new Phong(color*.75, 30, 100);
        g_Nodes.push_back({ ball, ballShader });
    }

    // environment
    g_Environment = new CubemapEnvironment("../data/env/forest", true /*use bilinear filtering*/);
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

void simpleRender()
{
    SetWindowCaption("Quad Damage: Simple Pass");

    Uint32 lastTicks = SDL_GetTicks();
    int frameWidth = GetFrameWidth();
    int frameHeight = GetFrameHeight();
    for ( int y = 0; y < frameHeight; ++y )
    {
        for ( int x = 0; x < frameWidth; ++x )
        {
            Ray ray = g_Camera.GetScreenRay(x, y);
            vfb[y][x] = Raytrace(ray);
        }

        if (g_WantProgressiveDisplay)
        {
            Uint32 ticks = SDL_GetTicks();
            if ( ticks - lastTicks > g_ProgressiveDisplayDelay )
            {
                DisplayVFB(vfb, g_UseStandartRGB);
                lastTicks = ticks;
            }
        }
    }

    DisplayVFB(vfb, g_UseStandartRGB);
}

void aaRender()
{
    SetWindowCaption("Quad Damage: AA Pass");

    if (!g_WantAA)
        return;

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
    for (int y = 0; y < frameHeight; ++y)
        for (int x = 0; x < frameWidth; ++x)
            needChange[x][y] = !g_WantAdaptiveAA || isTooDifferent(x, y);

    Uint32 lastTicks = SDL_GetTicks();
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

        if (g_WantProgressiveDisplay)
        {
            Uint32 ticks = SDL_GetTicks();
            if ( ticks - lastTicks > g_ProgressiveDisplayDelay )
            {
                DisplayVFB(vfb, g_UseStandartRGB);
                lastTicks = ticks;
            }
        }
    }

    DisplayVFB(vfb, g_UseStandartRGB);
}

void render()
{
    simpleRender();
    aaRender();
}

void DebugRaytrace(int x, int y)
{
    Ray ray = g_Camera.GetScreenRay(x, y);
    ray.debug = true;
    Raytrace(ray);
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

//    }

    WaitForUserExit();

    CloseGraphics();

    printf("Exited cleanly\n");
    return 0;
}