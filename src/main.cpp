#include <cmath>
#include <SDL.h>
#include <vector>

#include "color.h"
#include "sdl.h"
#include "utils.h"
#include "camera.h"
#include "geometry.h"
#include "shading.h"
#include "texture.h"

#define  COUNT_OF(arr) int((sizeof(arr)) / sizeof(arr[0]))

bool wantAA = true;

Color vfb[VFB_MAX_SIZE][VFB_MAX_SIZE];

Camera g_Camera;
std::vector<Node> g_Nodes;

std::vector<Light> g_Lights;
Color g_AmbientLight;

void SetupScene()
{
    // camera
    g_Camera.SetPosition({0, 200, 50});
    g_Camera.SetYaw(0);
    g_Camera.SetPitch(-15);
    g_Camera.SetRoll(0);
    g_Camera.SetFOV(90);
    g_Camera.SetAspectRatio((double)GetFrameWidth() / GetFrameHeight());

    // light
    g_Lights.push_back(Light{{600, 700, -350}, 800000});
    // g_Lights.push_back(Light{{-600, 700, -350}, 800000});
    g_AmbientLight = Color{1, 1, 1}*0.2;

    // ground
    Plane* plane = new Plane;
    CheckerTexture* checkerTexture = new CheckerTexture;
    checkerTexture->SetColor1({1, 1, 1});
    checkerTexture->SetColor2({0, 0, 0});
    checkerTexture->SetScaling(0.1);
    Lambert* lambert = new Lambert;
    lambert->SetColor({1, 1, 1});
    lambert->SetTexture(checkerTexture);
    g_Nodes.push_back({ plane, lambert });

    // phong sphere
    Sphere* phongSphere = new Sphere;
    phongSphere->SetCenter({-160, 125, 200});
    phongSphere->SetRadius(35);
    BlinnPhong* blinnPhong = new BlinnPhong;
    blinnPhong->SetColor({0.5, 0.5, 0.5});
    blinnPhong->SetSpecularExponent(60);
    blinnPhong->SetSpecularMultiplier(1);
    g_Nodes.push_back({ phongSphere, blinnPhong });

    //oren nayar spheres
    for (int i = 0; i < 4; ++i)
    {
        Sphere* onSphere = new Sphere;
        onSphere->SetCenter({-80. + i*80, 125, 200});
        onSphere->SetRadius(35);
        OrenNayar* orenNayar = new OrenNayar;
        orenNayar->SetColor({0.5, 0.5, 0.5});
        orenNayar->SetSigma(i*0.3);
        g_Nodes.push_back({ onSphere, orenNayar });
    }
    // start
    g_Camera.FrameBegin();
}

Color Raytrace(const Ray& ray)
{
    const Node* closestNode = nullptr;
    double closestDist = INF;
    IntersectionInfo closestInfo;
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

    if ( !closestNode )
        return {0, 0, 0};

    const Color& result = closestNode->shader->Shade(ray, closestInfo);
    return result;
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
        for ( int x = 0; x < frameWidth; ++x )
        {
            if ( wantAA )
            {
                Color result(0, 0, 0);
                for ( int i = 0; i < kernelSize; ++i )
                {
                    Ray ray = g_Camera.GetScreenRay(x + kernel[i][0], y + kernel[i][1]);
                    result += Raytrace(ray);
                }

                vfb[y][x] = result / double(kernelSize);
            }
            else
            {
                Ray ray = g_Camera.GetScreenRay(x, y);
                vfb[y][x] = Raytrace(ray);
            }
        }
}

// don't remove main arguments, it's required by STL
int main (int argc, char* argv[])
{
    InitGraphics(RESX, RESY);

    SetupScene();

    Uint32 startTicks = SDL_GetTicks();
    render();
    Uint32 elapsedMs = SDL_GetTicks() - startTicks;
    printf("Render took %.2lfs\n", elapsedMs / 1000.);

    DisplayVFB(vfb);
    WaitForUserExit();

    CloseGraphics();

    printf("Exited cleanly\n");
    return 0;
}