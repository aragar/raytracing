#include "sdl.h"
#include "camera.h"

#include <cstdio>
#include <SDL.h>

extern Camera g_Camera;
extern Vector g_LightPos;

SDL_Surface* screen = nullptr;

bool InitGraphics(int frameWidth, int frameHeight)
{
    if ( SDL_Init(SDL_INIT_VIDEO) < 0 )
    {
        printf("Cannot initialize SDL: %s\n", SDL_GetError());
        return false;
    }

    screen = SDL_SetVideoMode(frameWidth, frameHeight, 32, 0);
    if ( !screen )
    {
        printf("Cannot set video mode %dx%d - %s\n", frameWidth, frameHeight, SDL_GetError());
        return false;
    }

    return true;
}

void CloseGraphics()
{
    SDL_Quit();
}

void DisplayVFB(Color vfb[VFB_MAX_SIZE][VFB_MAX_SIZE])
{
    int redShift = screen->format->Rshift;
    int greenShift = screen->format->Gshift;
    int blueShift = screen->format->Bshift;
    for ( int y = 0; y < screen->h; ++y )
    {
        Uint32* row = (Uint32*)((Uint8*) screen->pixels + y*screen->pitch);
        for ( int x = 0; x < screen->w; ++x )
            row[x] = vfb[y][x].toRGB32(redShift, greenShift, blueShift);
    }

    SDL_Flip(screen);
}

void WaitForUserExit()
{
    SDL_Event event;
    while ( true )
    {
        while ( SDL_WaitEvent(&event) )
        {
            switch ( event.type )
            {
                case SDL_QUIT:
                    return;
                case SDL_KEYDOWN:
                {
                    switch ( event.key.keysym.sym )
                    {
                        case SDLK_ESCAPE:
                            return;
                        default:
                            break;
                    }
                }
                default:
                    break;
            }
        }
    }
}

int GetFrameWidth()
{
    if ( screen )
        return screen->w;

    return 0;
}

int GetFrameHeight()
{
    if ( screen )
        return screen->h;

    return 0;
}
