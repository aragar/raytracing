#ifndef RAYTRACING_CONSTANTS_H
#define RAYTRACING_CONSTANTS_H

const float LARGE_FLOAT = 1e17f;
const double LARGE_DOUBLE = 1e120;

const unsigned VFB_MAX_SIZE = 1920;
const unsigned RESX = 640;
const unsigned RESY = 480;
const double PI = 3.141592653589793238;
const double INF = 1e99;

const unsigned MAX_TREE_DEPTH = 64;
const unsigned TRIANGLES_PER_LEAF = 20;
const double COST_TRAVERSAL = 0.3;
const double COST_INTERSECT = 1.;

#endif //RAYTRACING_CONSTANTS_H
