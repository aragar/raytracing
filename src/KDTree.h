#ifndef RAYTRACING_KDTREE_H
#define RAYTRACING_KDTREE_H

#include "bbox.h"

#include <vector>

struct KDTreeNode
{
    int id;
    Axis axis;
    double splitPosition;

    union
    {
        std::vector<unsigned>* triangles;
        KDTreeNode* children;
    };

    ~KDTreeNode();

    void InitLeaf(const std::vector<unsigned>& triangles);
    void InitTreeNode(Axis axis, double splitPosition);

    bool IsLeaf() const { return axis == Axis::None; }

    int GetID()
    {
        static int ids = 0;
        ++ids;
        return ids;
    }
};

#endif //RAYTRACING_KDTREE_H
