#pragma once

#include "RayTraceEngine/Vector3D.h"

#include <cstdint>
#include <limits>
#include <vector>

/**
 * Contains x and y coordinates representing a vector in 2 dimensions.
 */
struct Vector2D
{
    float x;
    float y;
};

enum class RayType
{
    Closest,
    Pierce,
    Any
};

struct GeneratorRay
{
    RayType       type;
    std::uint64_t id;
    Vector3D      rayOrigin;
    Vector3D      rayDirection;
};

/**
 * Contains additional parameters of an object that are used when constructing the data structure for rendering.
 * bounding:        a parameter used for describing the looseness of an objects bounding, higher values create
 *                  bigger boxes that cripple general rendering performance but speed up reconstructing the data
 *                  structure on an object update (animations)
 */
struct ObjectParameter
{
    float bounding;
};

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
    [[nodiscard]] float GetSA() const
    {
        return (maxCorner.x - minCorner.x) * (maxCorner.y - minCorner.y) +
               (maxCorner.x - minCorner.x) * (maxCorner.z - minCorner.z) +
               (maxCorner.y - minCorner.y) * (maxCorner.z - minCorner.z);
    }
};

enum class TextureFormat
{
    RGB,
    RGBA
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
