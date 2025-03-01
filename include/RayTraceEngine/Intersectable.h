#pragma once

#include "BasicStructures.h"

#include <cstdint>
#include <memory>
#include <span>
#include <vector>

/**
 * Container outputted by the ray tracing engine.
 * hit:             Whether the ray intersected geometry.
 * distance:        Distance to the intersected geometry.
 * rayOrigin:       Origin of the ray.
 * rayDirection:    Direction of the ray.
 * normal:          (Interpolated) Normal vector of the intersected geometry.
 * position:        Coordinates of the point of intersection.
 * texture:         (Interpolated) Texture coordinates.
 * objectId:        Id of the instance that was intersected.
 */
struct IntersectionInfo
{
    bool          hit;
    float         distance;
    Vector3D      rayOrigin;
    Vector3D      rayDirection;
    Vector3D      normal;
    Vector3D      position;
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
    [[nodiscard]] virtual BoundingBox GetBoundaries() const = 0;

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
    [[nodiscard]] virtual float GetSurfaceArea() const = 0;

    /**
     * Tests whether the object in question is identical to this object.
     * @param object    Another object.
     * @return          True if they are equal, false otherwise.
     */
    virtual bool operator==(const IIntersectable& object) const = 0;

    virtual bool operator!=(const IIntersectable& object) const = 0;
};

struct IntersectableObjectDescription
{
    IIntersectable& intersectable;
};

class IntersectableObjectHandle
{
  public:
    virtual ~IntersectableObjectHandle() = default;
};
