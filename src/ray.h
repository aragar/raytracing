#ifndef RAYTRACING_RAY_H_H
#define RAYTRACING_RAY_H_H

#include "vector.h"

struct Ray
{
    Vector start;
    Vector dir; // normalized
    int depth = 0;
    bool debug = false;
};

#endif //RAYTRACING_RAY_H_H
