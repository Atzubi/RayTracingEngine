//
// Created by sebastian on 22.07.21.
//

#ifndef RAYTRACECORE_INSTANCE_H
#define RAYTRACECORE_INSTANCE_H

#include "API/Object.h"

class Instance : public Object{
private:
    Object *baseObject;
    BoundingBox boundingBox{};
    Matrix4x4 transform{};
    Matrix4x4 inverseTransform{};

public:
    explicit Instance(Object*);

    void applyTransform(Matrix4x4 *newTransform);

    ~Instance() override;

    Object *clone() override;

    BoundingBox getBoundaries() override;

    bool intersectFirst(IntersectionInfo *intersectionInfo, Ray *ray) override;
    bool intersectAny(IntersectionInfo *intersectionInfo, Ray *ray) override;
    bool intersectAll(std::vector<IntersectionInfo *> *intersectionInfo, Ray *ray) override;

    double getSurfaceArea() override;

    bool operator==(Object *object) override;
};

#endif //RAYTRACECORE_INSTANCE_H
