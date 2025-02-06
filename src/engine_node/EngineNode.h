#pragma once

#include "RayTraceEngine/RayEngine.h"
#include "cache/Cache.h"
#include <memory>
#include <type_traits>
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

    void DeleteIntersectable(const IIntersectable* intersectable);
    void DeleteShaderResource(const IShaderResource* shaderResource);
    void DeleteShader(const IRayGeneratorShader* shader);
    void DeleteShader(const IHitShader* shader);
    void DeleteShader(const IPierceShader* shader);
    void DeleteShader(const IOcclusionShader* shader);
    void DeleteShader(const IMissShader* shader);
    void DeleteRenderTarget(const std::vector<unsigned char>* renderTarget);

    // TODO replace intersectables and shader resources with caches
    // Cache<Key, T> cache_;

    // Resources
    std::unordered_set<std::unique_ptr<IIntersectable>>             intersectables_;
    std::unordered_set<std::unique_ptr<IShaderResource>>            shaderResources_;
    std::unordered_set<std::unique_ptr<IRayGeneratorShader>>        generatorShaders_;
    std::unordered_set<std::unique_ptr<IHitShader>>                 hitShaders_;
    std::unordered_set<std::unique_ptr<IPierceShader>>              pierceShaders_;
    std::unordered_set<std::unique_ptr<IOcclusionShader>>           occlusionShaders_;
    std::unordered_set<std::unique_ptr<IMissShader>>                missShaders_;
    std::unordered_set<std::unique_ptr<std::vector<unsigned char>>> renderTargets_;
};
