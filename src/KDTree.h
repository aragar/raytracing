#ifndef RAYTRACING_KDTREE_H
#define RAYTRACING_KDTREE_H

#include "bbox.h"

#include <vector>

struct KDTreeNode
{
    Axis axis;
    double splitPosition;

    union
    {
        std::vector<int>* triangles;
        KDTreeNode* children;
    };

    ~KDTreeNode();

    void InitLeaf(const std::vector<int>& triangles);
    void InitTreeNode(Axis axis, double splitPosition);

    bool IsLeaf() const { return axis == Axis::None; }
};


#endif //RAYTRACING_KDTREE_H
