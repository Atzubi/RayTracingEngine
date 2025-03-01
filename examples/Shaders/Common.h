
#pragma once

#include "RayTraceEngine/BasicStructures.h"
#include "RayTraceEngine/Pipeline.h"
#include "RayTraceEngine/Shader.h"
#include "RayTraceEngine/Vector3D.h"

#include <cstdint>
#include <cstring>
#include <unordered_map>
#include <vector>

/**
 * Material of an object.
 */
struct Material : public IShaderResource
{
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
    Texture map_Ka;
    // Diffuse Texture Map
    Texture map_Kd;
    // Specular Texture Map
    Texture map_Ks;
    // Specular Hightlight Map
    Texture map_Ns;
    // Alpha Texture Map
    Texture map_d;
    // Bump Map
    Texture map_bump;

    std::vector<std::uint8_t>        Serialize() const;
    std::unique_ptr<IShaderResource> Deserialize(const std::span<const std::uint8_t> buffer) const;
};

struct CameraInfo : public IShaderResource
{
    Vector3D cameraPosition;
    Vector3D cameraUp;
    Vector3D cameraDirection;

    std::vector<std::uint8_t>        Serialize() const;
    std::unique_ptr<IShaderResource> Deserialize(const std::span<const std::uint8_t> buffer) const;
};

struct ViewportInfo : public IShaderResource
{
    std::uint32_t viewPortWidth;
    std::uint32_t viewPortHeight;

    std::vector<std::uint8_t>        Serialize() const;
    std::unique_ptr<IShaderResource> Deserialize(const std::span<const std::uint8_t> buffer) const;
};

struct SampleCountInfo : public IShaderResource
{
    std::uint32_t samplesPerPixel;

    std::vector<std::uint8_t>        Serialize() const;
    std::unique_ptr<IShaderResource> Deserialize(const std::span<const std::uint8_t> buffer) const;
};

struct PathData : public IShaderResource
{
    std::vector<Vector3D>      absorption;
    std::vector<std::uint32_t> depth;

    std::vector<std::uint8_t>        Serialize() const override;
    std::unique_ptr<IShaderResource> Deserialize(const std::span<const std::uint8_t> buffer) const override;
};

struct MaterialMap : public IShaderResource
{
    std::unordered_map<std::uint64_t, std::uint64_t> instanceToMaterial;

    std::vector<std::uint8_t>        Serialize() const override;
    std::unique_ptr<IShaderResource> Deserialize(const std::span<const std::uint8_t> buffer) const;
};

Vector3D LambertReflection(const Vector3D& normal);

float FresnelSchlick(float cosTheta, float n1, float n2);

Vector3D Reflect(const Vector3D& incident, const Vector3D& normal);

Vector3D Refract(const Vector3D& incident, const Vector3D& normal, float n1, float n2);

Vector3D LoadKdTexel(const IntersectionInfo& info, const Material& material);