#pragma once

#include "BasicStructures.h"
#include "Vector3D.h"

#include <cstdint>
#include <limits>
#include <memory>
#include <span>
#include <vector>

/**
 * Container for an axis aligned bounding box.
 * minCorner:   The corner with minimum values.
 * maxCorner:   The corner with maximum values.
 */
struct BoundingBox
{
    Vector3D minCorner = {std::numeric_limits<float>::max(),
                          std::numeric_limits<float>::max(),
                          std::numeric_limits<float>::max()};
    Vector3D maxCorner = {-std::numeric_limits<float>::max(),
                          -std::numeric_limits<float>::max(),
                          -std::numeric_limits<float>::max()};

    /**
     * Computes the surface area of the axis aligned bounding box.
     * @return  The surface area divided by two.
     */
    float GetSA() const
    {
        return (maxCorner.x - minCorner.x) * (maxCorner.y - minCorner.y) +
               (maxCorner.x - minCorner.x) * (maxCorner.z - minCorner.z) +
               (maxCorner.y - minCorner.y) * (maxCorner.z - minCorner.z);
    }
};

/**
 * Container of a ray.
 * origin:      Origin of the ray.
 * direction:   Direction of the ray.
 * dirfrac:     1/direction of the ray. (Performance optimization)
 */
struct Ray
{
    Vector3D origin, direction, dirfrac;
};

/**
 * Container outputted by the ray tracing engine.
 * hit:             Whether the ray intersected geometry.
 * distance:        Distance to the intersected geometry.
 * rayOrigin:       Origin of the ray.
 * rayDirection:    Direction of the ray.
 * position:        Coordinates of the point of intersection.
 * normal:          (Interpolated) Normal vector of the intersected geometry.
 * texture:         (Interpolated) Texture coordinates.
 * instanceId:      Id of the instance that was intersected.
 */
struct IntersectionInfo
{
    bool          hit;
    float         distance;
    Vector3D      rayOrigin;
    Vector3D      rayDirection;
    Vector3D      position;
    Vector3D      normal;
    Vector2D      texture;
    std::uint64_t instanceId;
};

/**
 * Base class for all geometry object that the ray tracing engine can work with.
 */
class IIntersectable
{
  public:
    /**
     * Default destructor.
     */
    virtual ~IIntersectable() = default;

    /**
     * Serializes the intersectable into one continuous buffer.
     * @return   Raw byte representation of the intersectable.
     */
    virtual std::vector<std::uint8_t> Serialize() const = 0;

    /**
     * Creates an intersectable from a buffer.
     * @param buffer     Raw byte data.
     * @return           A new IIntersectable constructed from the buffer.
     */
    virtual std::unique_ptr<IIntersectable> Deserialize(std::span<const std::uint8_t> buffer) const = 0;

    /**
     * Computes the axis aligned bounding box of this object.
     * @return An axis aligned bounding box of this object.
     */
    virtual BoundingBox GetBoundaries() const = 0;

    /**
     * Computes the first intersection of a ray with this object.
     * @param intersectionInfo  Information container that will be filled with the intersection details on intersection.
     * @param ray               The ray that is used for the intersection calculation.
     * @return                  Returns true if there is an intersection, false otherwise.
     */
    virtual bool IntersectFirst(IntersectionInfo& intersectionInfo, const Ray& ray) const = 0;

    /**
     * Computes the first intersection of a ray with this object.
     * @param intersectionInfo  Information container that will be filled with the intersection details on intersection.
     * @param ray               The ray that is used for the intersection calculation.
     * @return                  Returns true if there is an intersection, false otherwise.
     */
    virtual bool IntersectAny(IntersectionInfo& intersectionInfo, const Ray& ray) const = 0;

    /**
     * Computes all intersections of a ray with this object.
     * @param intersectionInfo  Vector of intersection information containers that will be filled with the intersection
     *                          details for all intersections.
     * @param ray               The ray that is used for the intersection calculation.
     * @return                  Returns true if there is at least one intersection, false otherwise.
     */
    virtual bool IntersectAll(std::vector<IntersectionInfo>& intersectionInfo, const Ray& ray) const = 0;

    /**
     * Computes the effective surface area of this object.
     * @return The surface area of this object.
     */
    virtual float GetSurfaceArea() const = 0;

    /**
     * Tests whether the object in question is identical to this object.
     * @param object    Another object.
     * @return          True if they are equal, false otherwise.
     */
    virtual bool operator==(const IIntersectable& object) const = 0;

    virtual bool operator!=(const IIntersectable& object) const = 0;
};

/**
 * Contains all necessary information required to create an intersectable object in the engine.
 * intersectable:    Pointer to an intersectable object.
 */
struct IntersectableObjectDescription
{
    IIntersectable* intersectable;
};

/**
 * Resource handle. When this handle goes out of scope the resource is freed in the engine.
 */
class IntersectableObjectHandle
{
  public:
    virtual ~IntersectableObjectHandle() = default;
};
