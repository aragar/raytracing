#include "KDTree.h"

KDTreeNode::~KDTreeNode()
{
    if (axis == Axis::None)
        delete triangles;
    else
        delete[] children;
}

void KDTreeNode::InitLeaf(const std::vector<unsigned>& triangles)
{
    id = GetID();
    axis = Axis::None;
    this->triangles = new std::vector<unsigned >(triangles);
}

void KDTreeNode::InitTreeNode(Axis axis, double splitPosition)
{
    id = GetID();
    this->axis = axis;
    this->splitPosition = splitPosition;
    this->children = new KDTreeNode[2];
}
