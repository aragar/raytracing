#include "KDTree.h"

KDTreeNode::~KDTreeNode()
{
    if (axis == Axis::None)
        delete triangles;
    else
        delete[] children;
}

void KDTreeNode::InitLeaf(const std::vector<int>& triangles)
{
    axis = Axis::None;
    this->triangles = new std::vector<int>(triangles);
}

void KDTreeNode::InitTreeNode(Axis axis, double splitPosition)
{
    this->axis = axis;
    this->splitPosition = splitPosition;
    this->children = new KDTreeNode[2];
}
