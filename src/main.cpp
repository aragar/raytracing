#include <cmath>
#include <SDL.h>

#include "color.h"
#include "sdl.h"
#include "utils.h"

Color vfb[VFB_MAX_SIZE][VFB_MAX_SIZE];

void render()
{
    int frameWidth = FrameWidth();
    int frameHeight = FrameHeight();
    for  ( int y = 0; y < frameHeight; ++y )
        for ( int x = 0; x < frameWidth; ++x )
                vfb[y][x] = Color(float(x) / frameWidth, float(y) / frameHeight, 0.f);
}

int main (int argc, char** argv )
{
    InitGraphics(RESX, RESY);

    render();
    DisplayVFB(vfb);
    WaitForUserExit();

    CloseGraphics();

    printf("Exited cleanly\n");
    return 0;
}