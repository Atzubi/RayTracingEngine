//
// Created by Sebastian on 02.12.2021.
//

#ifndef RAYTRACEENGINE_DBVHV2_H
#define RAYTRACEENGINE_DBVHV2_H

#include "RayTraceEngine/Object.h"

struct DBVHNode {
    uint8_t maxDepthLeft = 0;
    union {
        DBVHNode *leftChild{};
        Object *leftLeaf;
    };

    uint8_t maxDepthRight = 0;
    union {
        DBVHNode *rightChild{};
        Object *rightLeaf;
    };

    BoundingBox boundingBox{std::numeric_limits<double>::max(), std::numeric_limits<double>::max(),
                            std::numeric_limits<double>::max(), -std::numeric_limits<double>::max(),
                            -std::numeric_limits<double>::max(), -std::numeric_limits<double>::max()};
    double surfaceArea{};
};

class DBVHv2 {
public:
    static void addObjects(DBVHNode *root, std::vector<Object *> *objects);

    static void removeObjects(DBVHNode *root, std::vector<Object *> *objects);

    static bool intersectFirst(DBVHNode *root, IntersectionInfo *intersectionInfo, Ray *ray);

    static bool intersectAny(DBVHNode *root, IntersectionInfo *intersectionInfo, Ray *ray);

    static bool intersectAll(DBVHNode *root, std::vector<IntersectionInfo *> *intersectionInfo, Ray *ray);

    static void deleteTree(DBVHNode *root);
};

#endif //RAYTRACEENGINE_DBVHV2_H
