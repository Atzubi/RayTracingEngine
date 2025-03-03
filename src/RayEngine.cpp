#include "RayTraceEngine/RayEngine.h"
#include "engine_node/EngineNode.h"

RayEngine::RayEngine() { engineNode_ = std::make_unique<EngineNode>(); }

RayEngine::~RayEngine() = default;

std::unique_ptr<IntersectableObjectHandle>
RayEngine::CreateIntersectableObject(const IntersectableObjectDescription& desc)
{
    return engineNode_->CreateIntersectableObject(desc);
}

std::unique_ptr<InstanceHandle> RayEngine::CreateInstance(const InstanceDescription& desc)
{
    return engineNode_->CreateInstance(desc);
}

std::unique_ptr<SceneHandle> RayEngine::CreateScene(const SceneDescription& desc)
{
    return engineNode_->CreateScene(desc);
}

std::unique_ptr<ShaderResourceHandle> RayEngine::CreateShaderResource(const ShaderResourceDescription& desc)
{
    return engineNode_->CreateShaderResource(desc);
}

std::unique_ptr<GeneratorShaderHandle> RayEngine::CreateShader(const GeneratorShaderDescription& desc)
{
    return engineNode_->CreateShader(desc);
}

std::unique_ptr<HitShaderHandle> RayEngine::CreateShader(const HitShaderDescription& desc)
{
    return engineNode_->CreateShader(desc);
}

std::unique_ptr<PierceShaderHandle> RayEngine::CreateShader(const PierceShaderDescription& desc)
{
    return engineNode_->CreateShader(desc);
}

std::unique_ptr<OcclusionShaderHandle> RayEngine::CreateShader(const OcclusionShaderDescription& desc)
{
    return engineNode_->CreateShader(desc);
}

std::unique_ptr<MissShaderHandle> RayEngine::CreateShader(const MissShaderDescription& desc)
{
    return engineNode_->CreateShader(desc);
}

std::unique_ptr<RenderTargetHandle> RayEngine::CreateRenderTarget(const RenderTargetDescription& desc)
{
    return engineNode_->CreateRenderTarget(desc);
}

std::unique_ptr<PipelineHandle> RayEngine::CreatePipeline(const PipelineDescription& desc)
{
    return engineNode_->CreatePipeline(desc);
}