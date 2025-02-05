#pragma once

#include "../src/bvh/DBVHv2.h"
#include "RayTraceEngine/Intersectable.h"

/**
 * Contains all the information required to construct a 3d model based on a 3d triangle mesh.
 * Provides necessary methods for using it as object in the ray tracing engine.
 * vertices:        a list of coordinates for position, normal and texture data
 * indices:         a list of indices for the vertices where every 3 define one triangle
 * material:        contains information about an objects surface properties, like texture, reflectiveness, etc.
 * triangles:       object form of every triangle defined by vertices and indices
 * structure:       an intersection acceleration data structure
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
     * Initializes the object given the base information. Creates an acceleration data structure for faster intersection
     * tests.
     * @param vertices  Vector of vertices, each containing a position, a normal and a texture coordinate.
     * @param indices   Vector of indices for the vertices. Every 3 indices define one triangle.
     * @param material  The objects material.
     */
    TriangleMeshObject(const std::vector<Vertex>*   vertices,
                       const std::vector<uint64_t>* indices,
                       const Material*              material);

    /**
     * Destructor, cleans up this object on deletion.
     */
    ~TriangleMeshObject() override;

    /**
     * Computes the axis aligned bounding box of this object.
     * @return An axis aligned bounding box of this object.
     */
    [[nodiscard]] BoundingBox GetBoundaries() const override;

    /**
     * Computes the first intersection of a ray with this object.
     * @param intersectionInfo  Information container that will be filled with the intersection details on intersection.
     * @param ray               The ray that is used for the intersection calculation.
     * @return                  Returns true if there is an intersection, false otherwise.
     */
    bool IntersectFirst(IntersectionInfo& intersectionInfo, const Ray& ray) const override;

    /**
     * Computes the first intersection of a ray with this object.
     * @param intersectionInfo  Information container that will be filled with the intersection details on intersection.
     * @param ray               The ray that is used for the intersection calculation.
     * @return                  Returns true if there is an intersection, false otherwise.
     */
    bool IntersectAny(IntersectionInfo& intersectionInfo, const Ray& ray) const override;

    /**
     * Computes all intersections of a ray with this object.
     * @param intersectionInfo  Vector of intersection information containers that will be filled with the intersection
     *                          details for all intersections.
     * @param ray               The ray that is used for the intersection calculation.
     * @return                  Returns true if there is at least one intersection, false otherwise.
     */
    bool IntersectAll(std::vector<IntersectionInfo>& intersectionInfo, const Ray& ray) const override;

    /**
     * Makes a perfect clone of this object.
     * @return  Pointer to the new clone.
     */
    [[nodiscard]] std::unique_ptr<IIntersectable> Clone() const override;

    /**
     * Computes the effective surface area of this object.
     * @return The surface area of this object.
     */
    [[nodiscard]] double GetSurfaceArea() const override;

    /**
     * Tests whether the object in question is identical to this object.
     * @param object    Another object.
     * @return          True if they are equal, false otherwise.
     */
    bool operator==(const IIntersectable& object) const override;

    bool operator!=(const IIntersectable& object) const override;

  private:
    friend class Triangle;

    std::vector<Vertex>   vertices;
    std::vector<uint64_t> indices;
    Material              material;

    std::vector<std::unique_ptr<IIntersectable>> triangles;
    DBVHv2                                       structure;

    // TODO
    // class TrianglMeshImpl;
    //
    // std::unique_ptr<TrianglMeshImpl> impl_;
};
