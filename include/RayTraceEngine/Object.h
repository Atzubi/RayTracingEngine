//
// Created by sebastian on 13.11.19.
//

#ifndef RAYTRACECORE_OBJECT_H
#define RAYTRACECORE_OBJECT_H

#include <vector>
#include <cstdint>
#include "BasicStructures.h"

struct ObjectId {
    int objectId;

    bool operator==(const ObjectId &other) const {
        return objectId == other.objectId;
    }

    bool operator<(const ObjectId &other) const {
        return objectId < other.objectId;
    }
};

template<>
struct std::hash<ObjectId> {
    std::size_t operator()(const ObjectId &k) const {
        return std::hash<int>()(k.objectId);
    }
};

struct InstanceId {
    int instanceId;

    bool operator==(const InstanceId &other) const {
        return instanceId == other.instanceId;
    }

    bool operator<(const InstanceId &other) const {
        return instanceId < other.instanceId;
    }
};

template<>
struct std::hash<InstanceId> {
    std::size_t operator()(const InstanceId &k) const {
        return std::hash<int>()(k.instanceId);
    }
};

/**
 * Container outputted by the ray tracing engine.
 * hit:             Whether the ray intersected geometry.
 * distance:        Distance to the intersected geometry.
 * rayOrigin:       Origin of the ray.
 * rayDirection:    Direction of the ray.
 * normal:          (Interpolated) normal vector of the intersected geometry.
 * position:        Coordinates of the point of intersection.
 * texture:         (Interpolated) texture coordinates.
 * material:        The intersected geometries material.
 */
struct IntersectionInfo {
    bool hit;
    double distance;
    Vector3D rayOrigin;
    Vector3D rayDirection;
    Vector3D normal;
    Vector3D position;
    Vector2D texture;
    Material *material;
};

struct ObjectCapsule {
    ObjectId id;
    BoundingBox boundingBox;
    double cost;
};

/**
 * Base class for all geometry object that the ray tracing engine can work with.
 */
class Object {
public:
    /**
     * Default destructor.
     */
    virtual ~Object() = default;

    /**
     * Creates a clone of this object.
     * @return  Pointer to a new clone.
     */
    virtual Object *clone() = 0;

    /**
     * Computes the axis aligned bounding box of this object.
     * @return An axis aligned bounding box of this object.
     */
    [[nodiscard]] virtual BoundingBox getBoundaries() const = 0;

    /**
     * Computes the first intersection of a ray with this object.
     * @param intersectionInfo  Information container that will be filled with the intersection details on intersection.
     * @param ray               The ray that is used for the intersection calculation.
     * @return                  Returns true if there is an intersection, false otherwise.
     */
    virtual bool intersectFirst(IntersectionInfo *intersectionInfo, Ray *ray) = 0;

    /**
     * Computes the first intersection of a ray with this object.
     * @param intersectionInfo  Information container that will be filled with the intersection details on intersection.
     * @param ray               The ray that is used for the intersection calculation.
     * @return                  Returns true if there is an intersection, false otherwise.
     */
    virtual bool intersectAny(IntersectionInfo *intersectionInfo, Ray *ray) = 0;

    /**
     * Computes all intersections of a ray with this object.
     * @param intersectionInfo  Vector of intersection information containers that will be filled with the intersection
     *                          details for all intersections.
     * @param ray               The ray that is used for the intersection calculation.
     * @return                  Returns true if there is at least one intersection, false otherwise.
     */
    virtual bool intersectAll(std::vector<IntersectionInfo *> *intersectionInfo, Ray *ray) = 0;

    /**
     * Computes the effective surface area of this object.
     * @return The surface area of this object.
     */
    [[nodiscard]] virtual double getSurfaceArea() const = 0;

    [[nodiscard]] virtual ObjectCapsule getCapsule() const = 0;

    /**
     * Tests whether the object in question is identical to this object.
     * @param object    Another object.
     * @return          True if they are equal, false otherwise.
     */
    virtual bool operator==(const Object &object) const = 0;

    virtual bool operator!=(const Object &object) const = 0;
};

#endif //RAYTRACECORE_OBJECT_H
