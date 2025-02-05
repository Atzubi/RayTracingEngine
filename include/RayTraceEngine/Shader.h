#pragma once

#include "Intersectable.h"

#include <cstdint>
#include <memory>
#include <vector>

class IShaderResource
{
  public:
    [[nodiscard]] virtual std::unique_ptr<IShaderResource> Clone() const = 0;

    virtual ~IShaderResource() = default;
};

/**
 * Container outputted by the ray generator shader.
 * id:              Original id of the ray, this will be passed to potential child rays. This is equivalent to the pixel
 * id. rayOrigin:       Vector of origins of rays. rayDirection:    Vector of directions of rays.
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
    Vector3D rayOrigin;
    Vector3D rayDirection;
};

/**
 * Container used as input by the hit shader.
 * intersectionInfo:    Contains details about the intersection.
 */
struct HitShaderInput
{
    IntersectionInfo* intersectionInfo;
};

/**
 * Container used as input by the miss shader.
 * rayOrigin:       The origin of the ray.
 * rayDirection:    The direction of the ray.
 */
struct MissShaderInput
{
    Vector3D rayOrigin;
    Vector3D rayDirection;
};

/**
 * Container used as input by the pierce shader.
 * intersectionInfo:    Vector of intersection information containers, one for each intersection.
 */
struct PierceShaderInput
{
    std::vector<IntersectionInfo> intersectionInfo;
};

/**
 * Container outputted by shaders. The color is represented as 24 bit rgb.
 */
struct ShaderOutput
{
    std::uint8_t color[3];
};

/**
 * Template for the Ray Generator Shader to be implemented. On pipeline execution it is called to generate the rays used
 * for ray tracing.
 */
class IRayGeneratorShader
{
  public:
    /**
     * Shading method. This will be called on pipeline execution. Its result is then passed to the ray tracing engine.
     * @param id            Id of the ray (family) being generated.
     * @param pipelineInfo  Contains details about the pipeline this shader is executed in.
     * @param dataInput     Currently unused.
     * @return
     */
    virtual void Shade(uint64_t                             id,
                       const std::vector<IShaderResource*>& shaderResource,
                       RayGeneratorOutput&                  rayGeneratorOutput) const = 0;

    [[nodiscard]] virtual std::unique_ptr<IRayGeneratorShader> Clone() const = 0;

    /**
     * Destructor.
     */
    virtual ~IRayGeneratorShader() = default;
};

/**
 * Template for the Occlusion Shader to be implemented. It is called on pipeline execution whenever a ray hits anything.
 */
class IOcclusionShader
{
  public:
    /**
     * Shading method. This will be called for every ray that intersects with any geometry.
     * @param id            Id of the current ray.
     * @param pipelineInfo  Contains details about the pipeline this shader is executed in.
     * @param shaderInput   Contains information about the intersection.
     * @param dataInput     Currently unused.
     * @param newRays       Optional shader output similar to the ray generator shader. Can be used to create child rays
     * of the current ray.
     * @return              Returns colour information that will be added to the rays corresponding pixel.
     */
    virtual ShaderOutput Shade(uint64_t                             id,
                               const OcclusionShaderInput&          shaderInput,
                               const std::vector<IShaderResource*>& shaderResource,
                               RayGeneratorOutput&                  newRays) const = 0;

    [[nodiscard]] virtual std::unique_ptr<IOcclusionShader> Clone() const = 0;

    /**
     * Destructor.
     */
    virtual ~IOcclusionShader() = default;
};

/**
 * Template for the Pierce Shader to be implemented. It is called on pipeline execution for every object that is hit by
 * a ray.
 */
class IPierceShader
{
  public:
    /**
     * Shading method. This will be called for every ray and every intersection with the geometry.
     * @param id            Id of the current ray.
     * @param pipelineInfo  Contains details about the pipeline this shader is executed in.
     * @param shaderInput   Contains information about the intersections.
     * @param dataInput     Currently unused.
     * @param newRays       Optional shader output similar to the ray generator shader. Can be used to create child rays
     * of the current ray.
     * @return              Returns colour information that will be added to the rays corresponding pixel.
     */
    virtual ShaderOutput Shade(uint64_t                             id,
                               const PierceShaderInput&             shaderInput,
                               const std::vector<IShaderResource*>& shaderResource,
                               RayGeneratorOutput&                  newRays) const = 0;

    [[nodiscard]] virtual std::unique_ptr<IPierceShader> Clone() const = 0;

    /**
     * Destructor.
     */
    virtual ~IPierceShader() = default;
};

/**
 * Template for the Hit Shader to be implemented. It is called on pipeline execution for the closest object hit by a
 * ray.
 */
class IHitShader
{
  public:
    /**
     * Shading Method. This will be called for the closest intersection for all rays that intersect anything.
     * @param id            Id of the current ray.
     * @param pipelineInfo  Contains details about the pipeline this shader is executed in.
     * @param shaderInput   Contains information about the intersections.
     * @param dataInput     Currently unused.
     * @param newRays       Optional shader output similar to the ray generator shader. Can be used to create child rays
     * of the current ray.
     * @return              Returns colour information that will be added to the rays corresponding pixel.
     */
    virtual ShaderOutput Shade(uint64_t                             id,
                               const HitShaderInput&                shaderInput,
                               const std::vector<IShaderResource*>& shaderResource,
                               RayGeneratorOutput&                  newRays) const = 0;

    [[nodiscard]] virtual std::unique_ptr<IHitShader> Clone() const = 0;

    /**
     * Destructor.
     */
    virtual ~IHitShader() = default;
};

/**
 * Template for the Miss Shader to be implemented. It is called on pipeline execution whenever a ray hits no geometry.
 */
class IMissShader
{
  public:
    /**
     * Shading method. It is called for every ray that does not intersect with any geometry.
     * @param id            Id of the current ray.
     * @param pipelineInfo  Contains details about the pipeline this shader is executed in.
     * @param shaderInput   Contains information about the intersections.
     * @param dataInput     Currently unused.
     * @param newRays       Optional shader output similar to the ray generator shader. Can be used to create child rays
     * of the current ray.
     * @return              Returns colour information that will be added to the rays corresponding pixel.
     */
    virtual ShaderOutput Shade(uint64_t                             id,
                               const MissShaderInput&               shaderInput,
                               const std::vector<IShaderResource*>& shaderResource,
                               RayGeneratorOutput&                  newRays) const = 0;

    [[nodiscard]] virtual std::unique_ptr<IMissShader> Clone() const = 0;

    /**
     * Destructor.
     */
    virtual ~IMissShader() = default;
};

struct ShaderResourceDescription
{
    IShaderResource* shaderResouce;
};

class ShaderResourceHandle
{
  public:
    virtual ~ShaderResourceHandle() = default;
};

struct GeneratorShaderDescription
{
    IRayGeneratorShader* generatorShader;
};

class GeneratorShaderHandle
{
  public:
    virtual ~GeneratorShaderHandle() = default;
};

struct HitShaderDescription
{
    IHitShader* hitShader;
};

class HitShaderHandle
{
  public:
    virtual ~HitShaderHandle() = default;
};

struct PierceShaderDescription
{
    IPierceShader* pierceShader;
};

class PierceShaderHandle
{
  public:
    virtual ~PierceShaderHandle() = default;
};

struct OcclusionShaderDescription
{
    IOcclusionShader* occlusionShader;
};

class OcclusionShaderHandle
{
  public:
    virtual ~OcclusionShaderHandle() = default;
};

struct MissShaderDescription
{
    IMissShader* missShader;
};

class MissShaderHandle
{
  public:
    virtual ~MissShaderHandle() = default;
};

template <class Shader>
concept isShader =
    std::same_as<Shader, IRayGeneratorShader> || std::same_as<Shader, IHitShader> ||
    std::same_as<Shader, IOcclusionShader> || std::same_as<Shader, IPierceShader> || std::same_as<Shader, IMissShader>;
