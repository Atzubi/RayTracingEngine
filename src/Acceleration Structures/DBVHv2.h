//
// Created by Sebastian on 02.12.2021.
//

#ifndef RAYTRACEENGINE_DBVHV2_H
#define RAYTRACEENGINE_DBVHV2_H

#include "RayTraceEngine/Object.h"
#include <variant>

struct DBVHNode {
    DBVHNode();

    ~DBVHNode();

    uint8_t maxDepthLeft;
    std::variant<std::unique_ptr<DBVHNode>, Object*> leftChild;

    uint8_t maxDepthRight;
    std::variant<std::unique_ptr<DBVHNode>, Object*> rightChild;

    BoundingBox boundingBox;
    double surfaceArea{};
};

class DBVHv2 {
public:
    static void addObjects(DBVHNode &root, const std::vector<Object *> &objects);

    static void removeObjects(DBVHNode &root, const std::vector<Object *> &objects);

    static bool intersectFirst(DBVHNode &root, IntersectionInfo *intersectionInfo, Ray *ray);

    static bool intersectAny(DBVHNode &root, IntersectionInfo *intersectionInfo, Ray *ray);

    static bool intersectAll(DBVHNode &root, std::vector<IntersectionInfo *> *intersectionInfo, Ray *ray);
};

#endif //RAYTRACEENGINE_DBVHV2_H
