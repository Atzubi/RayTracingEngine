#pragma once

#include "RayTraceEngine/RayEngine.h"

#include <memory>
#include <unordered_map>
#include <unordered_set>

class RayEngine::EngineNode
{
  public:
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

  private:
    void FetchIntersectable();
    void FetchShaderResource();

    void DeleteResource(std::uint64_t id);

    std::uint64_t GetNextId();

    std::uint64_t                     usedIds_;
    std::unordered_set<std::uint64_t> freeIds_;

    // TODO replace intersectables and shader resources with caches
    // Cache<Key, T> cache_;

    // Resources
    std::unordered_map<std::uint64_t, std::unique_ptr<IIntersectable>>        intersectables_;
    std::unordered_map<std::uint64_t, std::unique_ptr<IShaderResource>>       shaderResources_;
    std::unordered_map<std::uint64_t, IRayGeneratorShader>                    generatorShaders_;
    std::unordered_map<std::uint64_t, IHitShader>                             hitShaders_;
    std::unordered_map<std::uint64_t, IPierceShader>                          pierceShaders_;
    std::unordered_map<std::uint64_t, IOcclusionShader>                       occlusionShaders_;
    std::unordered_map<std::uint64_t, IMissShader>                            missShaders_;
    std::unordered_map<std::uint64_t, std::unique_ptr<std::vector<Vector3D>>> renderTargets_;
};
