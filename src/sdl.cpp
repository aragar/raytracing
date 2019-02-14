#include "sdl.h"
#include "camera.h"
#include "bitmap.h"

#include <cstdio>
#include <SDL.h>

SDL_Surface* screen = nullptr;
SDL_Thread* renderThread;
SDL_mutex* renderLock;
volatile bool rendering = false;
bool renderAsync;
bool wantToQuit = false;

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

static void HandleEvent(SDL_Event& event)
{
    switch (event.type)
    {
        case SDL_QUIT:
            wantToQuit = true;
            return;
        case SDL_KEYDOWN:
        {
            switch (event.key.keysym.sym)
            {
                case SDLK_ESCAPE:
                    wantToQuit = true;
                    return;
                case SDLK_F12:
                    // Shift+F12: screenshot in EXR; else, do it in (gamma-compressed) BMP.
                    if (event.key.keysym.mod & (KMOD_LSHIFT | KMOD_RSHIFT)) TakeScreenshotAuto(Bitmap::OutputFormat::EXR);
                    else TakeScreenshotAuto(Bitmap::OutputFormat::BMP);
                    break;
                default:
                    break;
            }
        }
        case SDL_MOUSEBUTTONDOWN:
        {
            // raytrace a single ray at the given pixel
            extern void DebugRaytrace(int x, int y);
            DebugRaytrace(event.button.x, event.button.y);
            break;
        }
        default:
            break;
    }
}

void WaitForUserExit()
{
    SDL_Event event;
    while (!wantToQuit)
        while (!wantToQuit && SDL_WaitEvent(&event))
            HandleEvent(event);
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

class MutexRAII
{
public:
    MutexRAII(SDL_mutex* mutex)
    : m_Mutex(mutex)
    {
        SDL_mutexP(m_Mutex);
    }

    ~MutexRAII()
    {
        SDL_mutexV(m_Mutex);
    }

private:
    SDL_mutex* m_Mutex;
};

bool RenderScene_Threaded()
{
    renderAsync = true;
    rendering = true;

    extern int RenderSceneThreaded(void*);
    renderThread = SDL_CreateThread(RenderSceneThreaded, nullptr);
    if (renderThread == nullptr)
    {
        rendering = false;
        renderAsync = false;
        return false;
    }

    while (!wantToQuit)
    {
        {
            MutexRAII raii(renderLock);
            if (!rendering)
                break;

            SDL_Event event;
            while (SDL_PollEvent(&event))
            {
                HandleEvent(event);
                if (wantToQuit)
                    break;
            }
        }

        SDL_Delay(100);
    }

    rendering = false;
    SDL_WaitThread(renderThread, nullptr);
    renderThread = nullptr;

    renderAsync = false;
    return true;
}

Rect::Rect(int _x0, int _y0, int _x1, int _y1)
: x0{_x0}
, y0{_y0}
, x1{_x1}
, y1{_y1}
{
    w = x1 - x0;
    h = y1 - y0;
}

void Rect::Clip(int maxX, int maxY)
{
    x1 = std::min(x1, maxX);
    y1 = std::min(y1, maxY);
    w = std::max(0, x1 - x0);
    h = std::max(0, y1 - y0);
}

std::vector<Rect> GetBucketList()
{
    std::vector<Rect> result;

    const int bucket_size = 48;
    int w = GetFrameWidth();
    int h = GetFrameHeight();
    int bw = (w - 1) / bucket_size + 1;
    int bh = (h - 1) / bucket_size + 1;
    for (int y = 0; y < bh; ++y)
    {
        if (y % 2 == 0)
            for (int x = 0; x < bw; ++x)
                result.push_back(Rect(x*bucket_size, y*bucket_size, (x + 1)*bucket_size, (y + 1)*bucket_size));
        else
            for (int x = bw - 1; x >= 0; --x)
                result.push_back(Rect(x*bucket_size, y*bucket_size, (x + 1)*bucket_size, (y + 1)*bucket_size));
    }

    for (unsigned i = 0; i < result.size(); ++i)
        result[i].Clip(w, h);

    return result;
}

bool DrawRect(Rect r, const Color& c, bool useSRGB/* = false*/)
{
    MutexRAII raii(renderLock);

    if (renderAsync && !rendering)
        return false;

    r.Clip(GetFrameWidth(), GetFrameHeight());

    int rs = screen->format->Rshift;
    int gs = screen->format->Gshift;
    int bs = screen->format->Bshift;
    Uint32 color = useSRGB ? c.toSRGB32(rs, gs, bs) : c.toRGB32(rs, gs, bs);
    for (int y = r.y0; y < r.y1; ++y)
    {
        Uint32* row = (Uint32*)((Uint8*)screen->pixels + y*screen->pitch);
        for (int x = r.x0; x < r.x1; ++x)
            row[x] = color;
    }

    SDL_UpdateRect(screen, r.x0, r.y0, r.w, r.h);
    return true;
}

bool DisplayVFBRect(Rect r, Color vfb[VFB_MAX_SIZE][VFB_MAX_SIZE], bool useSRGB/* = false*/)
{
    MutexRAII raii(renderLock);

    if (renderAsync && !rendering)
        return false;

    r.Clip(GetFrameWidth(), GetFrameHeight());

    int rs = screen->format->Rshift;
    int gs = screen->format->Gshift;
    int bs = screen->format->Bshift;
    for (int y = r.y0; y < r.y1; ++y)
    {
        Uint32* row = (Uint32*)((Uint8*)screen->pixels + y*screen->pitch);
        for (int x = r.x0; x < r.x1; ++x)
            row[x] = useSRGB ? vfb[y][x].toSRGB32(rs, gs, bs)
                             : vfb[y][x].toRGB32(rs, gs, bs);
    }

    SDL_UpdateRect(screen, r.x0, r.y0, r.w, r.h);
    return true;
}

bool MarkRegion(Rect r, const Color& bracketColor/* = Colors::NAVY*/, bool useSRGB/* = false*/)
{
    MutexRAII raii(renderLock);

    if (renderAsync && !rendering)
        return false;

    r.Clip(GetFrameWidth(), GetFrameHeight());

    const int L = 8;
    if (r.w < L + 3 || r.h < L + 3)
        return true; // region is too small to be marked

    const Uint32 bracket_color = useSRGB ? bracketColor.toSRGB32() : bracketColor.toRGB32();
    const Uint32 outline_color = useSRGB ? Colors::SILVER.toSRGB32() : Colors::SILVER.toRGB32();

#define DrawOne(x, y, color) \
    ((Uint32*)(((Uint8*)screen->pixels) + ((r.y0 + (y)) * screen->pitch)))[r.x0 + (x)] = (color);

#define Draw(x, y, color) \
    DrawOne((x), (y), (color)); \
    DrawOne((y), (x), (color)); \
    DrawOne(r.w - 1 - (x), (y), (color)); \
    DrawOne(r.w - 1 - (y), (x), (color)); \
    DrawOne((x), r.h - 1 - (y), (color)); \
    DrawOne((y), r.h - 1 - (x), (color)); \
    DrawOne(r.w - 1 - (x), r.h - 1 - (y), (color)); \
    DrawOne(r.w - 1 - (y), r.h - 1 - (x), (color));

    for (int i = 1; i <= L; i++) {
        Draw(i, 0, outline_color);
    }
    Draw(1, 1, outline_color);
    Draw(L + 1, 1, outline_color);
    for (int i = 0; i <= L; i++) {
        Draw(i, 2, outline_color);
    }
    for  (int i = 2; i <= L; i++) {
        Draw(i, 1, bracket_color);
    }

    SDL_UpdateRect(screen, r.x0, r.y0, r.w, r.h);
    return true;
}
