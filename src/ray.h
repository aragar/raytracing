#ifndef RAYTRACING_RAY_H_H
#define RAYTRACING_RAY_H_H

#include "vector.h"

struct Ray
{
    Vector start;
    Vector dir; // normed
    int depth = 0;
};

#endif //RAYTRACING_RAY_H_H
