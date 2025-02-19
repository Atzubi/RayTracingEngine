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

struct GeneratorRay
{
    Vector3D rayOrigin;
    Vector3D rayDirection;
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

/**
 * Container for storing an image/texture.
 * name:    The name of the texture.
 * w:       The horizontal resolution of the texture.
 * h:       The vertical resolution of the texture.
 * image:   Byte sized rgb values. Every 3 chars define the color of pixel.
 */
struct TextureView
{
    std::string                 name;
    std::uint32_t               w;
    std::uint32_t               h;
    std::vector<unsigned char>* image;
};

/**
 * Material of an object.
 */
struct Material
{
    // Material Name
    std::string name;
    // Ambient Color
    Vector3D Ka;
    // Diffuse Color
    Vector3D Kd;
    // Specular Color
    Vector3D Ks;
    // Specular Exponent
    float Ns;
    // Optical Density
    float Ni;
    // Dissolve
    float d;
    // Illumination
    int illum;
    // Ambient Texture Map
    TextureView map_Ka;
    // Diffuse Texture Map
    TextureView map_Kd;
    // Specular Texture Map
    TextureView map_Ks;
    // Specular Hightlight Map
    TextureView map_Ns;
    // Alpha Texture Map
    TextureView map_d;
    // Bump Map
    TextureView map_bump;
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
