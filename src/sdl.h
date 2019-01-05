#ifndef RAYTRACING_SDL_H
#define RAYTRACING_SDL_H

#include "color.h"
#include "constants.h"

bool InitGraphics(int frameWidth, int frameHeight);
void CloseGraphics();
void DisplayVFB(Color vfb[VFB_MAX_SIZE][VFB_MAX_SIZE], bool useSRGB = false);
void WaitForUserExit();
int GetFrameWidth();
int GetFrameHeight();
void SetWindowCaption(const char* msg, float renderTime = -1.f);

#endif //RAYTRACING_SDL_H
