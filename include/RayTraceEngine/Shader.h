#pragma once

#include "Intersectable.h"
#include "Vector3D.h"

#include <cstdint>
#include <memory>
#include <span>
#include <vector>

class IShaderResource
{
  public:
    virtual ~IShaderResource() = default;

    virtual std::span<const std::uint8_t>    Serialize() const                                         = 0;
    virtual std::unique_ptr<IShaderResource> Deserialize(std::span<const std::uint8_t> resource) const = 0;
};

/**
 * Container outputted by the ray generator shader.
 * id:              Original id of the ray, this will be passed to potential child rays. This is equivalent to the pixel
 * id. rayOrigin:   Vector of origins of rays. rayDirection:    Vector of directions of rays.
 */
struct RayGeneratorOutput
{
    std::vector<GeneratorRay> rays;
};

/**
 * Container used as input by the occlusion shader.
 * rayOrigin:       The origin of the ray.
 * rayDirection:    The direction of the ray.
 */
struct OcclusionShaderInput
{
    std::uint64_t id;
    Vector3D      rayOrigin;
    Vector3D      rayDirection;
};

/**
 * Container used as input by the hit shader.
 * intersectionInfo:    Contains details about the intersection.
 */
struct HitShaderInput
{
    std::uint64_t     id;
    IntersectionInfo* intersectionInfo;
};

/**
 * Container used as input by the miss shader.
 * rayOrigin:       The origin of the ray.
 * rayDirection:    The direction of the ray.
 */
struct MissShaderInput
{
    std::uint64_t id;
    Vector3D      rayOrigin;
    Vector3D      rayDirection;
};

/**
 * Container used as input by the pierce shader.
 * intersectionInfo:    Vector of intersection information containers, one for each intersection.
 */
struct PierceShaderInput
{
    std::uint64_t                 id;
    std::vector<IntersectionInfo> intersectionInfo;
};

/**
 * Container outputted by shaders. The color is represented as 24 bit rgb.
 */
struct ShaderOutput
{
    Vector3D color;
};

using IRayGeneratorShader = void (*)(std::uint64_t, const std::vector<IShaderResource*>&, RayGeneratorOutput&);

using IOcclusionShader = ShaderOutput (*)(std::uint64_t,
                                          const OcclusionShaderInput&,
                                          const std::vector<IShaderResource*>&,
                                          RayGeneratorOutput&);

using IPierceShader = ShaderOutput (*)(std::uint64_t,
                                       const PierceShaderInput&,
                                       const std::vector<IShaderResource*>&,
                                       RayGeneratorOutput&);

using IHitShader = ShaderOutput (*)(std::uint64_t,
                                    const HitShaderInput&,
                                    const std::vector<IShaderResource*>&,
                                    RayGeneratorOutput&);

using IMissShader = ShaderOutput (*)(std::uint64_t,
                                     const MissShaderInput&,
                                     const std::vector<IShaderResource*>&,
                                     RayGeneratorOutput&);

struct ShaderResourceDescription
{
    const IShaderResource* shaderResouce;
};

class ShaderResourceHandle
{
  public:
    template <typename T> T& Map() { return *reinterpret_cast<T*>(MapImpl()); }

    template <typename T> const T& Map() const { return *reinterpret_cast<const T*>(MapImpl()); }

    virtual ~ShaderResourceHandle() = default;

  private:
    virtual void*       MapImpl()       = 0;
    virtual const void* MapImpl() const = 0;
};

struct GeneratorShaderDescription
{
    IRayGeneratorShader generatorShader;
};

class GeneratorShaderHandle
{
  public:
    virtual ~GeneratorShaderHandle() = default;
};

struct HitShaderDescription
{
    IHitShader hitShader;
};

class HitShaderHandle
{
  public:
    virtual ~HitShaderHandle() = default;
};

struct PierceShaderDescription
{
    IPierceShader pierceShader;
};

class PierceShaderHandle
{
  public:
    virtual ~PierceShaderHandle() = default;
};

struct OcclusionShaderDescription
{
    IOcclusionShader occlusionShader;
};

class OcclusionShaderHandle
{
  public:
    virtual ~OcclusionShaderHandle() = default;
};

struct MissShaderDescription
{
    IMissShader missShader;
};

class MissShaderHandle
{
  public:
    virtual ~MissShaderHandle() = default;
};

template <class Shader>
concept IsShader =
    std::same_as<Shader, IRayGeneratorShader> || std::same_as<Shader, IHitShader> ||
    std::same_as<Shader, IOcclusionShader> || std::same_as<Shader, IPierceShader> || std::same_as<Shader, IMissShader>;
