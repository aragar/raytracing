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

Plane g_Floor, g_Ceiling;
CheckerTexture g_FloorTexture, g_CeilingTexture;
MandelbrotTexture g_MandelbrotTexture;
Lambert g_CeilingShader;
Phong g_FloorShader;

Vector g_LightPos;
double g_LightIntensity;
Color g_AmbientLight;

Cube g_Cube;
Sphere g_Ball;
CheckerTexture g_BlueTexture;
Phong g_GeometryShader;

void SetupScene()
{
    // camera
    g_Camera.SetPosition({0, 60, -100});
    g_Camera.SetYaw(0);
    g_Camera.SetPitch(-15);
    g_Camera.SetRoll(0);
    g_Camera.SetFOV(90);
    g_Camera.SetAspectRatio((double)GetFrameWidth() / GetFrameHeight());

    // floor
    g_Floor.SetHeight(1);

    g_FloorTexture.SetColor1({0, 0, 0.5});
    g_FloorTexture.SetColor2({1, 0.5, 0});

    g_MandelbrotTexture.SetColor1(Color{2037680});
    g_MandelbrotTexture.SetColor2(Color{2171945});
    g_MandelbrotTexture.SetScaling(80);

    // g_FloorShader.SetTexture(&g_FloorTexture);
    g_FloorShader.SetTexture(&g_MandelbrotTexture);
    g_FloorShader.SetSpecularExponent(20);
    g_FloorShader.SetSpecularMultiplier(5.3);

    // ceiling
    g_Ceiling.SetHeight(200);
    g_CeilingTexture.SetColor1({0.5, 0.5, 0.5});
    g_CeilingTexture.SetColor2({0.5, 0.5, 0.5});
    g_CeilingShader.SetTexture(&g_CeilingTexture);

    // Cube
    g_Cube.SetCenter({0, 30, -30});
    g_Cube.SetHalfSide(15);
    g_BlueTexture.SetColor1({0.2f, 0.4f, 1.f});
    g_BlueTexture.SetColor2({0.4f, 0.4f, 0.4f});
    g_BlueTexture.SetScaling(2);
    g_GeometryShader.SetTexture(&g_BlueTexture);
    g_GeometryShader.SetSpecularExponent(200);
    g_GeometryShader.SetSpecularMultiplier(0.5);

    // Ball
    g_Ball.SetCenter({0, 30, -30});
    g_Ball.SetRadius(20.5);

    // CSG
    CsgOp* csg = new CsgAnd;
    csg->SetLeft(&g_Cube);
    csg->SetRight(&g_Ball);

    // world geometry
    g_Nodes.push_back({ &g_Floor, &g_FloorShader });
    g_Nodes.push_back({ &g_Ceiling, &g_CeilingShader });
    // g_Nodes.push_back({ &g_Cube, &g_GeometryShader });
    // g_Nodes.push_back({ &g_Ball, &g_GeometryShader });
    // g_Nodes.push_back({ csg, &g_GeometryShader });

    // light
    g_LightPos = {15, 180, -60};
    g_LightIntensity = 23000;
    g_AmbientLight = Color{1, 1, 1}*0.1;

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
            { 0.0, 0.0 },
            { 0.6, 0.0 },
            { 0.0, 0.6 },
            { 0.3, 0.3 },
            { 0.6, 0.6 }
    };
    const int kernelSize = COUNT_OF(kernel);

    int frameWidth = GetFrameWidth();
    int frameHeight = GetFrameHeight();
    for  ( int y = 0; y < frameHeight; ++y )
        for ( int x = 0; x < frameWidth; ++x )
            if ( wantAA )
            {
                Color result(0, 0, 0);
                for ( int i = 0; i < kernelSize; ++i )
                {
                    Ray ray = g_Camera.GetScreenRay(x + kernel[i][0], y + kernel[i][1]);
                    result += Raytrace(ray);
                }

                vfb[y][x] = result/double(kernelSize);
            }
            else
            {
                Ray ray = g_Camera.GetScreenRay(x, y);
                vfb[y][x] = Raytrace(ray);
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