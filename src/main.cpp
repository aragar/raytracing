#include <cmath>
#include <SDL.h>
#include <vector>

#include "color.h"
#include "sdl.h"
#include "utils.h"
#include "camera.h"
#include "geometry.h"
#include "shading.h"

Color vfb[VFB_MAX_SIZE][VFB_MAX_SIZE];

Camera g_Camera;
std::vector<Node> g_Nodes;
Plane g_Floor, g_Ceiling;
CheckerShader g_FloorShader, g_CeilingShader;
Vector g_LightPos;
float g_LightIntensity;

void SetupScene()
{
    g_Camera.SetPosition({35.f, 60.f, -100.f});
    g_Camera.SetYaw(0.f);
    g_Camera.SetPitch(-15.f);
    g_Camera.SetRoll(0.f);
    g_Camera.SetFOV(90.f);
    g_Camera.SetAspectRatio(GetFrameWidth() / GetFrameHeight());

    g_Floor.y = 1.f;
    g_FloorShader.SetColor1({0.f, 0.f, 0.5f});
    g_FloorShader.SetColor2({1.f, 0.5f, 0.f});

    g_Ceiling.y = 200.f;
    g_CeilingShader.SetColor1({0.5f, 0.5f, 0.5f});
    g_CeilingShader.SetColor2({0.5f, 0.5f, 0.5f});

    g_Nodes.push_back({ &g_Floor, &g_FloorShader });
    g_Nodes.push_back({ &g_Ceiling, &g_CeilingShader });

    g_LightPos = {35.f, 180.f, 100.f};
    g_LightIntensity = 25000.f;

    g_Camera.FrameBegin();
}

Color Raytrace(const Ray& ray)
{
    const Node* closestNode = nullptr;
    float closestDist = INF;
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
        return {0.f, 0.f, 0.f};

    const Color& result = closestNode->shader->Shade(ray, closestInfo);
    return result;
}

void render()
{
    int frameWidth = GetFrameWidth();
    int frameHeight = GetFrameHeight();
    for  ( int y = 0; y < frameHeight; ++y )
        for ( int x = 0; x < frameWidth; ++x )
        {
            Ray ray = g_Camera.GetScreenRay(x, y);
            vfb[y][x] = Raytrace(ray);
        }
}

int main ()
{
    InitGraphics(RESX, RESY);

    SetupScene();
    render();
    DisplayVFB(vfb);
    WaitForUserExit();

    CloseGraphics();

    printf("Exited cleanly\n");
    return 0;
}