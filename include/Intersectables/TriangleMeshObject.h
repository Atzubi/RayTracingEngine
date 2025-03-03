#pragma once

#include "RayTraceEngine/BasicStructures.h"
#include "RayTraceEngine/Intersectable.h"
#include "RayTraceEngine/Vector3D.h"

#include <cstdint>
#include <memory>
#include <vector>

/**
 * Triangle mesh intersectable.
 */
class TriangleMeshObject : public IIntersectable
{
  public:
    struct Vertex
    {
        Vector3D position;
        Vector3D normal;
        Vector2D texture;
    };

    /**
     * Constructs the object from vertices and indices. Creates an acceleration data structure for faster intersection
     * tests.
     * @param vertices: Vector of vertices, each containing a position, a normal and a texture coordinate.
     * @param indices:  Vector of indices for the vertices. Every 3 indices define one triangle.
     */
    TriangleMeshObject(std::vector<Vertex> vertices, std::vector<std::uint32_t> indices);

    /**
     * Computes the axis aligned bounding box of this object.
     * @return: An axis aligned bounding box of this object.
     */
    BoundingBox GetBoundaries() const override;

    /**
     * Computes the first intersection of a ray with this object.
     * @param intersectionInfo: Information container that will be filled with the intersection details on intersection.
     * @param ray:              The ray that is used for the intersection calculation.
     * @return:                 Returns true if there is an intersection, false otherwise.
     */
    bool IntersectFirst(IntersectionInfo& intersectionInfo, const Ray& ray) const override;

    /**
     * Computes the first intersection of a ray with this object.
     * @param intersectionInfo: Information container that will be filled with the intersection details on intersection.
     * @param ray:              The ray that is used for the intersection calculation.
     * @return:                 Returns true if there is an intersection, false otherwise.
     */
    bool IntersectAny(IntersectionInfo& intersectionInfo, const Ray& ray) const override;

    /**
     * Computes all intersections of a ray with this object.
     * @param intersectionInfo: Vector of intersection information containers that will be filled with the intersection
     *                          details for all intersections.
     * @param ray:              The ray that is used for the intersection calculation.
     * @return:                 Returns true if there is at least one intersection, false otherwise.
     */
    bool IntersectAll(std::vector<IntersectionInfo>& intersectionInfo, const Ray& ray) const override;

    /**
     * Serializes the intersectable into one continuous buffer.
     * @return   Raw byte representation of the intersectable.
     */
    std::vector<std::uint8_t> Serialize() const override;

    /**
     * Creates an intersectable from a buffer.
     * @param buffer     Raw byte data.
     * @return           A new IIntersectable constructed from the buffer.
     */
    std::unique_ptr<IIntersectable> Deserialize(std::span<const std::uint8_t> buffer) const override;

    /**
     * Computes the effective surface area of this object.
     * @return The surface area of this object.
     */
    float GetSurfaceArea() const override;

    /**
     * Tests whether the object in question is identical to this object.
     * @param object    Another object.
     * @return          True if they are equal, false otherwise.
     */
    bool operator==(const IIntersectable& object) const override;

    bool operator!=(const IIntersectable& object) const override;

    /**
     * Default destructor.
     */
    ~TriangleMeshObject();

  private:
    class TrianglMeshImpl;

    std::unique_ptr<TrianglMeshImpl> impl_;
};
