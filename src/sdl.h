#ifndef RAYTRACING_SDL_H
#define RAYTRACING_SDL_H

#include "color.h"
#include "constants.h"

bool InitGraphics(int frameWidth, int frameHeight);
void CloseGraphics();
void DisplayVFB(Color vfb[VFB_MAX_SIZE][VFB_MAX_SIZE]);
void WaitForUserExit();
int FrameWidth();
int FrameHeight();

#endif //RAYTRACING_SDL_H
