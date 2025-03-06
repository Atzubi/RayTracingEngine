#pragma once

#include "Intersectable.h"
#include "Vector3D.h"

#include <cstdint>
#include <memory>
#include <span>
#include <vector>

/**
 * Interface for user provided shader resources.
 */
class IShaderResource
{
  public:
    virtual ~IShaderResource() = default;

    /**
     * Serializes the content of the resource into raw bytes.
     * @return   Vector of bytes.
     */
    virtual std::vector<std::uint8_t> Serialize() const = 0;

    /**
     * Deserializes raw bytes back into a shader resource.
     * @param buffer:    Raw byte input.
     * @return           Deserialized shader resource.
     */
    virtual std::unique_ptr<IShaderResource> Deserialize(std::span<const std::uint8_t> buffer) const = 0;
};

/**
 * Specifies the intersection type of the ray.
 */
enum class RayType
{
    Closest, // -> Hit shader
    Pierce,  // -> Pierce Shader
    Any      // -> Occlusion Shader
};

/**
 * Info of a freshly generated ray.
 * type:         Intersection type of the ray.
 * id:           Custom identifier.
 * rayOrigin     Origin of the ray.
 * rayDirection: Direction of the ray.
 */
struct GeneratorRay
{
    RayType       type;
    std::uint64_t id;
    Vector3D      rayOrigin;
    Vector3D      rayDirection;
};

/**
 * Container of new rays, returned by the generator shader and optionally other shaders.
 * rays:    Vector of new rays.
 */
struct RayGeneratorOutput
{
    std::vector<GeneratorRay> rays;
};

/**
 * Container used as input by the occlusion shader.
 * id:              Custom id set in the GeneratorRay.
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
 * id:                  Custom id set in the GeneratorRay.
 * intersectionInfo:    Contains details about the intersection.
 */
struct HitShaderInput
{
    std::uint64_t     id;
    IntersectionInfo* intersectionInfo;
};

/**
 * Container used as input by the miss shader.
 * id:              Custom id set in the GeneratorRay.
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
 * id:                  Custom id set in the GeneratorRay.
 * intersectionInfo:    Vector of intersection information containers, one for each intersection.
 */
struct PierceShaderInput
{
    std::uint64_t                 id;
    std::vector<IntersectionInfo> intersectionInfo;
};

/**
 * Color output of shaders.
 * color:   Float RGB.
 */
struct ShaderOutput
{
    Vector3D color;
};

/**
 * Function signature of the generator shader. Will be called for each pixel.
 * @param id:               Id of the pixel this shader will generate rays for.
 * @param shaderResources:  Custom shader resources.
 * @param generatedRays:    Generated rays.
 */
using IRayGeneratorShader = void (*)(std::uint64_t                     id,
                                     std::span<IShaderResource* const> shaderResources,
                                     RayGeneratorOutput&               generatedRays);

/**
 * Function signature of the hit shader. Will be called for each hit with ray type closest.
 * @param id:               Id of the pixel this shader output to.
 * @param shaderInput:      Intersection info.
 * @param shaderResources:  Custom shader resources.
 * @param generatedRays:    Optionally generated rays.
 * @return:                 Color output.
 */
using IHitShader = ShaderOutput (*)(std::uint64_t                     id,
                                    const HitShaderInput&             shaderInput,
                                    std::span<IShaderResource* const> shaderResources,
                                    RayGeneratorOutput&               generatedRays);

/**
 * Function signature of the pierce shader. Will be called for all hits with ray type pierce.
 * @param id:               Id of the pixel this shader will output to.
 * @param shaderInput:      Intersection infos.
 * @param shaderResources:  Custom shader resources.
 * @param generatedRays:    Optionally generated rays.
 * @return:                 Color output.
 */
using IPierceShader = ShaderOutput (*)(std::uint64_t                     id,
                                       const PierceShaderInput&          shaderInput,
                                       std::span<IShaderResource* const> shaderResources,
                                       RayGeneratorOutput&               generatedRays);

/**
 * Function signature of the occlusion shader. Will be called if there was any intersection with ray type any.
 * @param id:               Id of the pixel this shader will output to.
 * @param shaderInput:      Intersection info.
 * @param shaderResources:  Custom shader resources.
 * @param generatedRays:    Optionally generated rays.
 * @return:                 Color output.
 */
using IOcclusionShader = ShaderOutput (*)(std::uint64_t                     id,
                                          const OcclusionShaderInput&       shaderInput,
                                          std::span<IShaderResource* const> shaderResources,
                                          RayGeneratorOutput&               generatedRays);

/**
 * Function signature of the miss shader. Will be called for any ray that does not hit regardless of type.
 * @param id:               Id of the pixel this shader will output to.
 * @param shaderInput:      Intersection info.
 * @param shaderResources:  Custom shader resources.
 * @param generatedRays:    Optionally generated rays.
 * @return:                 Color output.
 */
using IMissShader = ShaderOutput (*)(std::uint64_t                     id,
                                     const MissShaderInput&            shaderInput,
                                     std::span<IShaderResource* const> shaderResources,
                                     RayGeneratorOutput&               generatedRays);

/**
 * Contains all information required to create a shader resource in the engine.
 * shaderResource:   Reference to a user provided resource.
 */
struct ShaderResourceDescription
{
    const IShaderResource* shaderResouce;
};

/**
 * Resource handle. When this handle goes out of scope the resource is freed in the engine.
 */
class ShaderResourceHandle
{
  public:
    /**
     * Maps the resource to any type.
     * @return:  Reference to the resource with given type.
     */
    template <typename T> T& Map() { return *reinterpret_cast<T*>(MapImpl()); }

    /**
     * Maps the resource to any type.
     * @return:  Reference to the resource with given type.
     */
    template <typename T> const T& Map() const { return *reinterpret_cast<const T*>(MapImpl()); }

    virtual ~ShaderResourceHandle() = default;

  private:
    virtual void*       MapImpl()       = 0;
    virtual const void* MapImpl() const = 0;
};

/**
 * Descriptions and handles for shaders:
 */

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
