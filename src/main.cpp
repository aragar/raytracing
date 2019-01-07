#include <cmath>
#include <SDL.h>
#include <vector>

#include "camera.h"
#include "color.h"
#include "environment.h"
#include "geometry.h"
#include "random_generator.h"
#include "sdl.h"
#include "shading.h"
#include "texture.h"
#include "utils.h"

Color vfb[VFB_MAX_SIZE][VFB_MAX_SIZE];
bool needChange[VFB_MAX_SIZE][VFB_MAX_SIZE] = {{false}};

Color Raytrace(const Ray& ray)
{
    if (ray.depth > scene.settings.maxTraceDepth)
        return Color{0, 0, 0};

    const Node* closestNode = nullptr;
    double closestDist = INF;
    IntersectionInfo closestInfo = IntersectionInfo();
    for (const Node* node : scene.nodes)
    {
        IntersectionInfo info;
        if (!node->Intersect(ray, info))
            continue;

        if (closestDist <= info.distance)
            continue;

        closestDist = info.distance;
        closestNode = node;
        closestInfo = info;
    }

    // check if we hit the sky
    Color result{0, 0, 0};
    if (closestNode)
    {
        closestInfo.rayDir = ray.dir;
        if (closestNode->bump)
            closestNode->bump->ModifyNormal(closestInfo);

        result = closestNode->shader->Shade(ray, closestInfo);
    }
    else if (scene.environment)
    {
        result = scene.environment->GetEnvironment(ray.dir);
    }

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

        if (diff > scene.settings.aaThreshold)
            return true;
    }

    return false;
}

void simpleRender()
{
    SetWindowCaption("Quad Damage: Simple Pass");

    Uint32 lastTicks = SDL_GetTicks();

    const int frameWidth = GetFrameWidth();
    const int frameHeight = GetFrameHeight();
    for (int y = 0; y < frameHeight; ++y)
    {
        for (int x = 0; x < frameWidth; ++x)
        {
            const Ray ray = scene.camera->GetScreenRay(x, y);
            vfb[y][x] = Raytrace(ray);
        }

        if (scene.settings.wantProgressiveDisplay)
        {
            const Uint32 ticks = SDL_GetTicks();
            if (ticks - lastTicks > scene.settings.progressiveDisplayDelay)
            {
                DisplayVFB(vfb, scene.settings.useSRGB);
                lastTicks = ticks;
            }
        }
    }

    DisplayVFB(vfb, scene.settings.useSRGB);
}

void aaRender()
{
    SetWindowCaption("Quad Damage: AA Pass");

    if (!scene.settings.wantAA)
        return;

    const double kernel[5][2] = {
            {0.0, 0.0},
            {0.6, 0.0},
            {0.0, 0.6},
            {0.3, 0.3},
            {0.6, 0.6}
    };
    const int kernelSize = COUNT_OF(kernel);

    const int frameWidth = GetFrameWidth();
    const int frameHeight = GetFrameHeight();
    for (int y = 0; y < frameHeight; ++y)
        for (int x = 0; x < frameWidth; ++x)
            needChange[x][y] = !scene.settings.wantAdaptiveAA || isTooDifferent(x, y);

    Uint32 lastTicks = SDL_GetTicks();
    for (int y = 0; y < frameHeight; ++y)
    {
        for (int x = 0; x < frameWidth; ++x)
        {
            if (!needChange[x][y])
                continue;

            if (scene.settings.showAA)
            {
                vfb[y][x] = scene.settings.aaDebugColor;
            }
            else
            {
                Color result = vfb[y][x];
                for (int i = 1; i < kernelSize; ++i)
                {
                    Ray ray = scene.camera->GetScreenRay(x + kernel[i][0], y + kernel[i][1]);
                    result += Raytrace(ray);
                }

                vfb[y][x] = result / double(kernelSize);
            }
        }

        if (scene.settings.wantProgressiveDisplay)
        {
            const Uint32 ticks = SDL_GetTicks();
            if (ticks - lastTicks > scene.settings.progressiveDisplayDelay)
            {
                DisplayVFB(vfb, scene.settings.useSRGB);
                lastTicks = ticks;
            }
        }
    }

    DisplayVFB(vfb, scene.settings.useSRGB);
}

void render()
{
    scene.BeginFrame();

    simpleRender();
    aaRender();
}

void DebugRaytrace(int x, int y)
{
    Ray ray = scene.camera->GetScreenRay(x, y);
    ray.debug = true;
    Raytrace(ray);
}

// don't remove main arguments, it's required by SDL
int main (int argc, char* argv[])
{
//    test_random();
    InitRandom(42);
    if (!scene.ParseScene("../data/bumpmap.qdmg"))
    {
        printf("Could not parse the scene!\n");
        return -1;
    }

    InitGraphics(scene.settings.frameWidth, scene.settings.frameHeight);
    scene.BeginRender();

//    const int rotations = 10;
//    for (int i = 0; i < rotations; ++i)
//    {
//        g_Camera.SetYaw(1. * i * 2*PI/rotations);
//        g_Camera.FrameBegin();

        const Uint32 startTicks = SDL_GetTicks();
        render();
        const Uint32 elapsedMs = SDL_GetTicks() - startTicks;
        printf("Render took %.2lfs\n", elapsedMs / 1000.);
        SetWindowCaption("Quad Damage: rendered in %.2fs\n", elapsedMs / 1000.f);

//    }

    WaitForUserExit();
    CloseGraphics();

    printf("Exited cleanly\n");
    return 0;
}