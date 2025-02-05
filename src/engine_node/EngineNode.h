#pragma once

#include "RayTraceEngine/RayEngine.h"
#include "cache/Cache.h"
#include <memory>
#include <unordered_set>

template <typename T> struct PtrHasher
{
    std::hash<const T*> hash;
    using is_transparent = void;
    std::size_t operator()(const std::unique_ptr<T>& ptr) const { return hash(ptr.get()); }
    std::size_t operator()(const T* ptr) const { return hash(ptr); }
};

template <typename T> struct PtrEqual
{
    using is_transparent = void;
    bool operator()(const std::unique_ptr<T>& lhs, const T* rhs) const { return lhs.get() == rhs; }
    bool operator()(const T* lhs, const std::unique_ptr<T>& rhs) const { return lhs == rhs.get(); }
    bool operator()(const std::unique_ptr<T>& lhs, const std::unique_ptr<T>& rhs) const
    {
        return lhs.get() == rhs.get();
    }
};

template <typename T> struct PtrLess
{
    std::less<T*> equal;
    using is_transparent = void;
    bool operator()(const std::unique_ptr<T>& lhs, const T* rhs) const { return equal(lhs.get(), rhs); }
    bool operator()(const T* lhs, const std::unique_ptr<T>& rhs) const { return equal(lhs, rhs.get()); }
    bool operator()(const std::unique_ptr<T>& lhs, const std::unique_ptr<T>& rhs) const
    {
        return equal(lhs.get(), rhs.get());
    }
};

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
    std::unordered_set<std::unique_ptr<IIntersectable>, PtrHasher<IIntersectable>, PtrEqual<IIntersectable>>
        intersectables_;
    std::unordered_set<std::unique_ptr<IShaderResource>, PtrHasher<IShaderResource>, PtrEqual<IShaderResource>>
        shaderResources_;
    std::unordered_set<std::unique_ptr<IRayGeneratorShader>,
                       PtrHasher<IRayGeneratorShader>,
                       PtrEqual<IRayGeneratorShader>>
                                                                                                 generatorShaders_;
    std::unordered_set<std::unique_ptr<IHitShader>, PtrHasher<IHitShader>, PtrEqual<IHitShader>> hitShaders_;
    std::unordered_set<std::unique_ptr<IPierceShader>, PtrHasher<IPierceShader>, PtrEqual<IPierceShader>>
        pierceShaders_;
    std::unordered_set<std::unique_ptr<IOcclusionShader>, PtrHasher<IOcclusionShader>, PtrEqual<IOcclusionShader>>
                                                                                                    occlusionShaders_;
    std::unordered_set<std::unique_ptr<IMissShader>, PtrHasher<IMissShader>, PtrEqual<IMissShader>> missShaders_;
    std::unordered_set<std::unique_ptr<std::vector<unsigned char>>,
                       PtrHasher<std::vector<unsigned char>>,
                       PtrEqual<std::vector<unsigned char>>>
        renderTargets_;
};
