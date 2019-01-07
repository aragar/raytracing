#include "sdl.h"
#include "camera.h"
#include "bitmap.h"

#include <cstdio>
#include <SDL.h>

extern Camera g_Camera;

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

void DisplayVFB(Color vfb[VFB_MAX_SIZE][VFB_MAX_SIZE], bool useSRGB)
{
    int redShift = screen->format->Rshift;
    int greenShift = screen->format->Gshift;
    int blueShift = screen->format->Bshift;
    for ( int y = 0; y < screen->h; ++y )
    {
        Uint32* row = (Uint32*)((Uint8*) screen->pixels + y*screen->pitch);
        for ( int x = 0; x < screen->w; ++x )
            row[x] = useSRGB ? vfb[y][x].toSRGB32(redShift, greenShift, blueShift)
                             : vfb[y][x].toRGB32(redShift, greenShift, blueShift);
    }

    SDL_Flip(screen);
}

void FindUnusedFilename(char filename[], const char* suffix)
{
    char tmp[256];
    int index = 0;
    while (1)
    {
        sprintf(tmp, "quad_damage_%04d.bmp", index);
        FILE* f = fopen(tmp, "rb");
        if (!f)
        {
            sprintf(tmp, "quad_damage_%04d.exr", index);
            f = fopen(tmp, "rb");
        }

        if (!f)
            break;

        fclose(f);
        ++index;
    }

    sprintf(filename, "quad_damage_%04d.%s", index, suffix);
}

bool TakeScreenshot(const char* filename)
{
    extern Color vfb[VFB_MAX_SIZE][VFB_MAX_SIZE];

    Bitmap bmp;
    const unsigned width = static_cast<unsigned>(GetFrameWidth());
    const unsigned height = static_cast<unsigned>(GetFrameHeight());
    bmp.GenerateEmptyImage(width, height);
    for (unsigned y = 0; y < height; ++y)
        for (unsigned x = 0; x < width; ++x)
            bmp.SetPixel(x, y, vfb[y][x]);

    bool result = bmp.SaveImage(filename);
    if (result) printf("Saved a screenshot as '%s'\n", filename);
    else printf("Failed to take a screenshot\n");
    return result;
}

bool TakeScreenshotAuto(Bitmap::OutputFormat format)
{
    char filename[256];
    FindUnusedFilename(filename, format == Bitmap::OutputFormat::BMP ? "bmp" : "exr");
    return TakeScreenshot(filename);
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
                        case SDLK_F12:
                            if (event.key.keysym.mod & (KMOD_LSHIFT | KMOD_RSHIFT)) TakeScreenshotAuto(Bitmap::OutputFormat::EXR);
                            else TakeScreenshotAuto(Bitmap::OutputFormat::BMP);
                            break;
                        default:
                            break;
                    }
                }
                case SDL_MOUSEBUTTONDOWN:
                {
                    extern void DebugRaytrace(int x, int y);
                    DebugRaytrace(event.button.x, event.button.y);
                    break;
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

void SetWindowCaption(const char* msg, float renderTime)
{
    if (renderTime >= 0.f)
    {
        char message[128];
        sprintf(message, msg, renderTime);
        SDL_WM_SetCaption(message, nullptr);
    }
    else
    {
        SDL_WM_SetCaption(msg, nullptr);
    }
}
