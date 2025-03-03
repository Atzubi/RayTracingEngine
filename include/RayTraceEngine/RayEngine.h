#pragma once

#include "Intersectable.h"
#include "Pipeline.h"
#include "Scene.h"
#include "Shader.h"

#include <memory>

/**
 * Ray tracing engine. Performs ray intersections on provided geometry and calls user provided shaders on
 * intersections/misses.
 * Note: The engine will copy all resources provided.
 */
class RayEngine
{
  public:
    /**
     * Constructs and initializes the engine.
     */
    RayEngine();

    /**
     * Creates an intersectable object from a user provided intersectable.
     * @param desc:  Description of the object to be created.
     * @return:      Handle, all further interactions with the object are handled by it.
     */
    std::unique_ptr<IntersectableObjectHandle> CreateIntersectableObject(const IntersectableObjectDescription& desc);

    /**
     * Creates an instance of an intersectable object.
     * @param desc:  Description of the instance to be created.
     * @return:      Handle, all further interactions with the object are handled by it.
     */
    std::unique_ptr<InstanceHandle> CreateInstance(const InstanceDescription& desc);

    /**
     * Creates a scene object from intersectable objects.
     * @param desc:  Description of the scene to be created.
     * @return:      Handle, all further interactions with the scene are handled by it.
     */
    std::unique_ptr<SceneHandle> CreateScene(const SceneDescription& desc);

    /**
     * Creates a user defined shader resource.
     * @param desc:  Description of the resource to be created.
     * @return:      Handle, all further interactions with the resource are handled by it.
     */
    std::unique_ptr<ShaderResourceHandle> CreateShaderResource(const ShaderResourceDescription& desc);

    /**
     * Creates a shader from a user provided shading function.
     * @param desc:  Description of the shader to be created.
     * @return:      Handle, all further interactions with the shader are handled by it.
     */
    std::unique_ptr<GeneratorShaderHandle> CreateShader(const GeneratorShaderDescription& desc);
    std::unique_ptr<HitShaderHandle>       CreateShader(const HitShaderDescription& desc);
    std::unique_ptr<PierceShaderHandle>    CreateShader(const PierceShaderDescription& desc);
    std::unique_ptr<OcclusionShaderHandle> CreateShader(const OcclusionShaderDescription& desc);
    std::unique_ptr<MissShaderHandle>      CreateShader(const MissShaderDescription& desc);

    /**
     * Creates a render target with a given resolution.
     * @param desc:  Description of the render target to be created.
     * @return:      Handle, all further interactions with the render target are handled by it.
     */
    std::unique_ptr<RenderTargetHandle> CreateRenderTarget(const RenderTargetDescription& desc);

    /**
     * Creates a runable pipeline from geometry, shaders and their resources.
     * @param desc:  Description of the pipeline to be created.
     * @return:      Handle, all further interactions with the pipeline are handled by it.
     */
    std::unique_ptr<PipelineHandle> CreatePipeline(const PipelineDescription& desc);

    ~RayEngine();

  private:
    class EngineNode;

    std::unique_ptr<EngineNode> engineNode_;
};
