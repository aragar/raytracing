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

bool wantAA = false;

Color vfb[VFB_MAX_SIZE][VFB_MAX_SIZE];

Camera g_Camera;
std::vector<Node> g_Nodes;

Plane g_Floor, g_Ceiling;
CheckerTexture g_FloorTexture, g_CeilingTexture;
MandelbrotTexture g_MandelbrotTexture;
Lambert g_CeilingShader, g_FloorShader;

Vector g_LightPos;
double g_LightIntensity;
Color g_AmbientLight;

Cube g_Cube;
Sphere g_Ball;
CheckerTexture g_BlueTexture;
Phong g_GeometryShader;

RegularPolygon g_Triangle;
CheckerTexture g_TriangleTexture;
Lambert g_TriangleShader;

RegularPolygon g_Square;
CheckerTexture g_SquareTexture;
Lambert g_SquareeShader;

RegularPolygon g_Pentagon;
CheckerTexture g_PentagonTexture;
Lambert g_PentagonShader;

void SetupScene()
{
    // camera
    g_Camera.SetPosition({0, 60, -100});
    g_Camera.SetYaw(0);
    g_Camera.SetPitch(-30);
    g_Camera.SetRoll(0);
    g_Camera.SetFOV(90);
    g_Camera.SetAspectRatio((double)GetFrameWidth() / GetFrameHeight());

    // floor
    g_Floor.SetHeight(1);

    g_FloorTexture.SetColor1({0, 0, 0});
    g_FloorTexture.SetColor2({0, 0, 1});

    g_MandelbrotTexture.SetColor1(Color{2037680});
    g_MandelbrotTexture.SetColor2(Color{2171945});
    g_MandelbrotTexture.SetScaling(80);

    g_FloorShader.SetTexture(&g_FloorTexture);
    // g_FloorShader.SetTexture(&g_MandelbrotTexture);
    // g_FloorShader.SetSpecularExponent(20);

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

    // Regular polygons
    g_Triangle.SetCenter({-60, 1.1, -50});
    g_Triangle.SetRadius(25);
    g_Triangle.SetSides(3);
    g_TriangleTexture.SetColor1({0, 0, 0});
    g_TriangleTexture.SetColor1({1, 0, 0});
    g_TriangleTexture.SetScaling(5);
    g_TriangleShader.SetTexture(&g_TriangleTexture);

    g_Square.SetCenter({0, 1.1, -50});
    g_Square.SetRadius(25);
    g_Square.SetSides(4);
    g_SquareTexture.SetColor1({0, 0, 0});
    g_SquareTexture.SetColor2({0, 1, 0});
    g_SquareTexture.SetScaling(5);
    g_SquareeShader.SetTexture(&g_SquareTexture);

    g_Pentagon.SetCenter({50, 1.1, -50});
    g_Pentagon.SetRadius(25);
    g_Pentagon.SetSides(5);
    g_PentagonTexture.SetColor1({0, 0, 0});
    g_PentagonTexture.SetColor2({1, 1, 0});
    g_PentagonTexture.SetScaling(5);
    g_PentagonShader.SetTexture(&g_PentagonTexture);

    // world geometry
    g_Nodes.push_back({ &g_Floor, &g_FloorShader });
    g_Nodes.push_back({ &g_Ceiling, &g_CeilingShader });
    // g_Nodes.push_back({ &g_Cube, &g_GeometryShader });
    // g_Nodes.push_back({ &g_Ball, &g_GeometryShader });
    // g_Nodes.push_back({ csg, &g_GeometryShader });
    g_Nodes.push_back({ &g_Triangle, &g_TriangleShader });
    g_Nodes.push_back({ &g_Square, &g_SquareeShader });
    g_Nodes.push_back({ &g_Pentagon, &g_PentagonShader });

    // light
    g_LightPos = {0, 50, 0};
    g_LightIntensity = 30000;
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