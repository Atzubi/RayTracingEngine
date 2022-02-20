//
// Created by Sebastian on 19.02.2022.
//

#ifndef RAYTRACEENGINE_DBVHNODE_H
#define RAYTRACEENGINE_DBVHNODE_H

#include "RayTraceEngine/Intersectable.h"
#include <cstdint>

struct DBVHNode {
    uint8_t maxDepthLeft = 0;
    union {
        uint64_t leftPosition;
        Intersectable *leftLeaf = nullptr;
    };

    uint8_t maxDepthRight = 0;
    union {
        uint64_t rightPosition;
        Intersectable *rightLeaf = nullptr;
    };

    BoundingBox boundingBox;
    double surfaceArea = 0;
};

#endif //RAYTRACEENGINE_DBVHNODE_H