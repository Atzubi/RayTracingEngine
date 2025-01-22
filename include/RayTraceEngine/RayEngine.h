#pragma once

#include <cstdint>

#include "BasicStructures.h"
#include "Intersectable.h"
#include "Pipeline.h"
#include "Scene.h"
#include "Shader.h"

class RenderTargetHandle
{
};

struct IntersectableObjectDescription
{
    IIntersectable& intersectable;
};

class PipelineHandle
{
  public:
    void Run(RenderTargetHandle& target);
};

class RayEngine
{
  public:
    RayEngine();
    ~RayEngine();

    IntersectableObjectHandle CreateIntersectableObject(const IntersectableObjectDescription& desc);

    SceneHandle CreateScene(const SceneDescription& desc);

    ShaderResourceHandle CreateShaderResource(const ShaderResourceDescription& desc);

    ShaderHandle CreateShader(const ShaderDescription& desc);

    PipelineHandle CreatePipeline(const PipelineDescription& desc);

  private:
    class EngineNode;

    std::unique_ptr<EngineNode> engineNode_;
};
