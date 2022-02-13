//
// Created by sebastian on 22.07.21.
//

#ifndef RAYTRACECORE_INSTANCE_H
#define RAYTRACECORE_INSTANCE_H

#include "utility/Id.h"
#include "RayTraceEngine/BasicStructures.h"
#include "RayTraceEngine/Intersectable.h"
#include "data_management/DataManagementUnitV2.h"

class DataManagementUnitV2;

class Instance : public Intersectable {
private:
    DataManagementUnitV2 *dmu;

    ObjectId baseObjectId;
    bool objectCached;
    Intersectable *objectCache;

    double cost;
    BoundingBox boundingBox{};
    Matrix4x4 transform{};
    Matrix4x4 inverseTransform{};

    Intersectable *getBaseObject();

public:
    explicit Instance(DataManagementUnitV2 &node, ObjectCapsule &objectCapsule);

    void applyTransform(const Matrix4x4 &newTransform);

    void invalidateCache();

    ~Instance() override;

    [[nodiscard]] std::unique_ptr<Intersectable> clone() const override;

    [[nodiscard]] BoundingBox getBoundaries() const override;

    bool intersectFirst(IntersectionInfo &intersectionInfo, const Ray &ray) override;

    bool intersectAny(IntersectionInfo &intersectionInfo, const Ray &ray) override;

    bool intersectAll(std::vector<IntersectionInfo> &intersectionInfo, const Ray &ray) override;

    [[nodiscard]] double getSurfaceArea() const override;

    [[nodiscard]] ObjectCapsule getCapsule() const override;

    bool operator==(const Intersectable &object) const override;

    bool operator!=(const Intersectable &object) const override;
};

#endif //RAYTRACECORE_INSTANCE_H
