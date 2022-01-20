//
// Created by sebastian on 22.07.21.
//

#ifndef RAYTRACECORE_INSTANCE_H
#define RAYTRACECORE_INSTANCE_H

#include "RayTraceEngine/Object.h"

class EngineNode;

class Instance : public Object {
private:
    EngineNode *engineNode;

    ObjectId baseObjectId;
    bool objectCached;
    Object *objectCache;

    double cost;
    BoundingBox boundingBox{};
    Matrix4x4 transform{};
    Matrix4x4 inverseTransform{};

public:
    explicit Instance(EngineNode *node, ObjectCapsule *objectCapsule);

    void applyTransform(Matrix4x4 *newTransform);

    void invalidateCache();

    ~Instance() override;

    Object *clone() override;

    BoundingBox getBoundaries() override;

    bool intersectFirst(IntersectionInfo *intersectionInfo, Ray *ray) override;

    bool intersectAny(IntersectionInfo *intersectionInfo, Ray *ray) override;

    bool intersectAll(std::vector<IntersectionInfo *> *intersectionInfo, Ray *ray) override;

    double getSurfaceArea() override;

    ObjectCapsule getCapsule() override;

    bool operator==(Object *object) override;
};

#endif //RAYTRACECORE_INSTANCE_H
