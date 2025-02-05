#pragma once

#include "RayTraceEngine/Intersectable.h"

struct DBVHNode
{
    uint8_t maxDepthLeft = 0;
    union
    {
        DBVHNode*       leftChild;
        IIntersectable* leftLeaf = nullptr;
    };

    uint8_t maxDepthRight = 0;
    union
    {
        DBVHNode*       rightChild;
        IIntersectable* rightLeaf = nullptr;
    };

    BoundingBox boundingBox;
    double      surfaceArea = 0;
};
