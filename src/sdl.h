#ifndef RAYTRACING_SDL_H
#define RAYTRACING_SDL_H

#include "color.h"
#include "colors.h"
#include "constants.h"

#include <vector>

extern volatile bool rendering; // used in main/worker thread synchronization

bool InitGraphics(int frameWidth, int frameHeight);
void CloseGraphics();
void DisplayVFB(Color vfb[VFB_MAX_SIZE][VFB_MAX_SIZE], bool useSRGB = false);
void WaitForUserExit();
int GetFrameWidth();
int GetFrameHeight();
void SetWindowCaption(const char* msg, float renderTime = -1.f);

bool RenderScene_Threaded();

struct Rect
{
    int x0, y0, x1, y1, w, h;

    Rect() {}
    Rect(int _x0, int _y0, int _x1, int _y1);

    void Clip(int maxX, int maxY); // clips the rectangle against image size
};

std::vector<Rect> GetBucketList();
bool DrawRect(Rect r, const Color& c, bool useSRGB = false);
bool DisplayVFBRect(Rect r, Color vfb[VFB_MAX_SIZE][VFB_MAX_SIZE], bool useSRGB = false);
bool MarkRegion(Rect r, const Color& bracketColor = Colors::NAVY, bool useSRGB = false);

#endif //RAYTRACING_SDL_H
