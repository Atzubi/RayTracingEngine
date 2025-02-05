#pragma once

#include "BasicStructures.h"
#include "Intersectable.h"
#include "Pipeline.h"
#include "Scene.h"
#include "Shader.h"

#include <cstdint>
#include <memory>

class RayEngine
{
  public:
    RayEngine();

    std::unique_ptr<IntersectableObjectHandle> CreateIntersectableObject(const IntersectableObjectDescription& desc);

    std::unique_ptr<SceneHandle> CreateScene(const SceneDescription& desc);

    std::unique_ptr<ShaderResourceHandle> CreateShaderResource(const ShaderResourceDescription& desc);

    std::unique_ptr<GeneratorShaderHandle> CreateShader(const GeneratorShaderDescription& desc);
    std::unique_ptr<HitShaderHandle>       CreateShader(const HitShaderDescription& desc);
    std::unique_ptr<PierceShaderHandle>    CreateShader(const PierceShaderDescription& desc);
    std::unique_ptr<OcclusionShaderHandle> CreateShader(const OcclusionShaderDescription& desc);
    std::unique_ptr<MissShaderHandle>      CreateShader(const MissShaderDescription& desc);

    std::unique_ptr<RenderTargetHandle> CreateRenderTarget(const RenderTargetDescription& desc);

    std::unique_ptr<PipelineHandle> CreatePipeline(const PipelineDescription& desc);

    ~RayEngine();

  private:
    class EngineNode;

    std::unique_ptr<EngineNode> engineNode_;
};
