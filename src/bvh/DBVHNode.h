#pragma once

#include "RayTraceEngine/BasicStructures.h"

struct DBVHNode
{
    std::uint16_t maxDepthLeft;
    std::uint16_t maxDepthRight;
    std::uint32_t leftChild;
    std::uint32_t rightChild;
    BoundingBox   boundingBox;
};
