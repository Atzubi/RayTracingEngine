//
// Created by sebastian on 13.11.19.
//

#ifndef RAYTRACECORE_SHADER_H
#define RAYTRACECORE_SHADER_H

#include <cstdint>
#include <vector>
#include "Object.h"
#include "Utility/Id.h"

struct RayGeneratorShaderResourcePackage{
    RayGeneratorShaderId shaderId;
    std::vector<ShaderResourceId> shaderResourceIds;
};

struct HitShaderResourcePackage{
    HitShaderId shaderId;
    std::vector<ShaderResourceId> shaderResourceIds;
};

struct OcclusionShaderResourcePackage{
    OcclusionShaderId shaderId;
    std::vector<ShaderResourceId> shaderResourceIds;
};

struct PierceShaderResourcePackage{
    PierceShaderId shaderId;
    std::vector<ShaderResourceId> shaderResourceIds;
};

struct MissShaderResourcePackage{
    MissShaderId shaderId;
    std::vector<ShaderResourceId> shaderResourceIds;
};

class ShaderResource {
public:
    virtual ShaderResource *clone() = 0;
};

class RayResource {
public:
    virtual RayResource *clone() = 0;
};

/**
 * Container passed to shaders, containing the basic information about the Pipeline.
 * width:           Horizontal resolution.
 * height:          Vertical resolution.
 * cameraPosition:  Position of the virtual camera.
 * cameraDirection: Direction of the virtual camera facing forwards.
 * cameraUp:        Direction of the virtual camera facing upwards.
 */
struct PipelineInfo {
    int width{}, height{};
    Vector3D cameraPosition{};
    Vector3D cameraDirection{};
    Vector3D cameraUp{};
};

/**
 * Container outputted by the ray generator shader.
 * id:              Original id of the ray, this will be passed to potential child rays. This is equivalent to the pixel id.
 * rayOrigin:       Vector of origins of rays.
 * rayDirection:    Vector of directions of rays.
 */
struct RayGeneratorOutput {
    std::vector<GeneratorRay> rays;
};

/**
 * Container used as input by the occlusion shader.
 * rayOrigin:       The origin of the ray.
 * rayDirection:    The direction of the ray.
 */
struct OcclusionShaderInput {
    Vector3D rayOrigin;
    Vector3D rayDirection;
};

/**
 * Container used as input by the hit shader.
 * intersectionInfo:    Contains details about the intersection.
 */
struct HitShaderInput {
    IntersectionInfo *intersectionInfo;
};

/**
 * Container used as input by the miss shader.
 * rayOrigin:       The origin of the ray.
 * rayDirection:    The direction of the ray.
 */
struct MissShaderInput {
    Vector3D rayOrigin;
    Vector3D rayDirection;
};

/**
 * Container used as input by the pierce shader.
 * intersectionInfo:    Vector of intersection information containers, one for each intersection.
 */
struct PierceShaderInput {
    std::vector<IntersectionInfo> intersectionInfo;
};

/**
 * Container outputted by shaders. The color is represented as 24 bit rgb.
 */
struct ShaderOutput {
    uint8_t color[3];
};

/**
 * Base class for all shaders. Shaders can be added to a pipeline.
 */
class Shader {
public:
    /**
     * Clones a shader.
     * @return  New pointer to the cloned Shader object.
     */
    virtual Shader *clone() = 0;
};

/**
 * Template for the Ray Generator Shader to be implemented. On pipeline execution it is called to generate the rays used
 * for ray tracing.
 */
class RayGeneratorShader : public Shader {
public:
    /**
     * Shading method. This will be called on pipeline execution. Its result is then passed to the ray tracing engine.
     * @param id            Id of the ray (family) being generated.
     * @param pipelineInfo  Contains details about the pipeline this shader is executed in.
     * @param dataInput     Currently unused.
     * @return
     */
    virtual void
    shade(uint64_t id, PipelineInfo *pipelineInfo, std::vector<ShaderResource *> *shaderResource,
          RayGeneratorOutput *rayGeneratorOutput) = 0;

    /**
     * Destructor.
     */
    virtual ~RayGeneratorShader() = default;
};

/**
 * Template for the Occlusion Shader to be implemented. It is called on pipeline execution whenever a ray hits anything.
 */
class OcclusionShader : public Shader {
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
    virtual ShaderOutput
    shade(uint64_t id, PipelineInfo *pipelineInfo, OcclusionShaderInput *shaderInput,
          std::vector<ShaderResource *> *shaderResource,
          RayResource **rayResource, RayGeneratorOutput *newRays) = 0;

    /**
     * Destructor.
     */
    virtual ~OcclusionShader() = default;
};

/**
 * Template for the Pierce Shader to be implemented. It is called on pipeline execution for every object that is hit by a ray.
 */
class PierceShader : public Shader {
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
    virtual ShaderOutput
    shade(uint64_t id, PipelineInfo *pipelineInfo, PierceShaderInput *shaderInput,
          std::vector<ShaderResource *> *shaderResource,
          RayResource **rayResource, RayGeneratorOutput *newRays) = 0;

    /**
     * Destructor.
     */
    virtual ~PierceShader() = default;
};

/**
 * Template for the Hit Shader to be implemented. It is called on pipeline execution for the closest object hit by a ray.
 */
class HitShader : public Shader {
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
    virtual ShaderOutput
    shade(uint64_t id, PipelineInfo *pipelineInfo, HitShaderInput *shaderInput,
          std::vector<ShaderResource *> *shaderResource,
          RayResource **rayResource, RayGeneratorOutput *newRays) = 0;

    /**
     * Destructor.
     */
    virtual ~HitShader() = default;
};

/**
 * Template for the Miss Shader to be implemented. It is called on pipeline execution whenever a ray hits no geometry.
 */
class MissShader : public Shader {
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
    virtual ShaderOutput
    shade(uint64_t id, PipelineInfo *pipelineInfo, MissShaderInput *shaderInput,
          std::vector<ShaderResource *> *shaderResource,
          RayResource **rayResource, RayGeneratorOutput *newRays) = 0;

    /**
     * Destructor.
     */
    virtual ~MissShader() = default;
};

#endif //RAYTRACECORE_SHADER_H
