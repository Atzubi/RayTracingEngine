//
// Created by Sebastian on 02.12.2021.
//

#ifndef RAYTRACEENGINE_DBVHV2_H
#define RAYTRACEENGINE_DBVHV2_H

#include "RayTraceEngine/Object.h"

struct DBVHNode {
    uint8_t maxDepthLeft = 0;
    std::unique_ptr<DBVHNode> leftChild;
    Object *leftLeaf = nullptr;

    uint8_t maxDepthRight = 0;
    std::unique_ptr<DBVHNode> rightChild;
    Object *rightLeaf = nullptr;

    BoundingBox boundingBox;
    double surfaceArea = 0;
};

class DBVHv2 {
public:
    static void addObjects(DBVHNode &root, const std::vector<Object *> &objects);

    static void removeObjects(DBVHNode &root, const std::vector<Object *> &objects);

    static bool intersectFirst(const DBVHNode &root, IntersectionInfo &intersectionInfo, const Ray &ray);

    static bool intersectAny(const DBVHNode &root, IntersectionInfo &intersectionInfo, const Ray &ray);

    static bool intersectAll(const DBVHNode &root, std::vector<IntersectionInfo> &intersectionInfo, const Ray &ray);
};

#endif //RAYTRACEENGINE_DBVHV2_H