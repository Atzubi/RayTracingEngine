#include "EngineNode.h"
#include "bvh/DBVH.h"
#include "intersectable/Instance.h"
#include "pipeline/PipelineImplement.h"

template <typename T> class HandleBase
{
  public:
    HandleBase(std::function<void(const T)> deleterCallback, T handle)
        : deleterCallback_(std::move(deleterCallback)), handle_(handle)
    {
    }

    ~HandleBase() { deleterCallback_(handle_); }

  protected:
    std::function<void(const T)> deleterCallback_;
    T                            handle_;
};

class IntersectableObjectH : public IntersectableObjectHandle, public HandleBase<IIntersectable*>
{
  public:
    IntersectableObjectH(std::function<void(const IIntersectable*)> deleterCallback, IIntersectable* handle)
        : HandleBase(std::move(deleterCallback), handle)
    {
    }

    const IIntersectable* Get() const { return handle_; }
};

std::unique_ptr<IntersectableObjectHandle>
RayEngine::EngineNode::CreateIntersectableObject(const IntersectableObjectDescription& desc)
{
    const auto [it, success] = intersectables_.insert(desc.intersectable.Clone());
    if (!success)
        throw std::runtime_error("Tried to create duplicate intersectable.");
    return std::make_unique<IntersectableObjectH>(
        [this](const IIntersectable* intersectable) { DeleteIntersectable(intersectable); }, it->get());
}

class InstanceH : public InstanceHandle, public HandleBase<IIntersectable*>
{
  public:
    InstanceH(std::function<void(const IIntersectable*)> deleterCallback, IIntersectable* handle, DBVH* reference)
        : HandleBase(std::move(deleterCallback), handle), reference_(reference)
    {
    }

    void Transform(const Matrix4x4& transform) override
    {
        reference_->RemoveObjects({handle_});
        dynamic_cast<Instance*>(handle_)->ApplyTransform(transform);
        reference_->AddObjects({handle_});
    }

    Matrix4x4 GetTransform() const override { return dynamic_cast<Instance*>(handle_)->GetTransform(); }

  private:
    DBVH* reference_;
};

class SceneH : public SceneHandle, public HandleBase<IIntersectable*>
{
  public:
    SceneH(std::function<void(const IIntersectable*)>   deleterCallback,
           IIntersectable*                              handle,
           std::vector<std::unique_ptr<InstanceHandle>> instances)
        : HandleBase(std::move(deleterCallback), handle), instances_(std::move(instances))
    {
    }

    std::span<const std::unique_ptr<InstanceHandle>> GetInstanceHandles() const override { return instances_; }

    const IIntersectable* Get() const { return handle_; }

  private:
    std::vector<std::unique_ptr<InstanceHandle>> instances_;
};

std::unique_ptr<SceneHandle> RayEngine::EngineNode::CreateScene(const SceneDescription& desc)
{
    auto                                         sceneBvh = std::make_unique<DBVH>();
    std::vector<const IIntersectable*>           instances;
    std::vector<std::unique_ptr<InstanceHandle>> instanceHandles;
    for (const auto& intersectablePack : desc.intersectables)
    {
        const auto intersectable = dynamic_cast<const IntersectableObjectH&>(intersectablePack.intersectable).Get();
        auto       instance      = std::make_unique<Instance>(intersectable, [this]() { FetchIntersectable(); });
        instance->ApplyTransform(intersectablePack.transform);

        const auto [it, success] = intersectables_.insert(std::move(instance));
        if (!success)
            throw std::runtime_error("Tried to create duplicate intersectable.");

        instances.push_back(it->get());

        instanceHandles.push_back(std::make_unique<InstanceH>(
            [this](const IIntersectable* inters) { DeleteIntersectable(inters); }, it->get(), sceneBvh.get()));
    }

    sceneBvh->AddObjects(instances);
    const auto [it, success] = intersectables_.insert(std::move(sceneBvh));
    if (!success)
        throw std::runtime_error("Tried to create duplicate intersectable.");

    return std::make_unique<SceneH>([this](const IIntersectable* intersectable) { DeleteIntersectable(intersectable); },
                                    it->get(),
                                    std::move(instanceHandles));
}

class ShaderResourceH : public ShaderResourceHandle, public HandleBase<IShaderResource*>
{
  public:
    ShaderResourceH(std::function<void(const IShaderResource*)> deleterCallback, IShaderResource* handle)
        : HandleBase(std::move(deleterCallback), handle)
    {
    }

    const IShaderResource* Get() const { return handle_; }
    IShaderResource*       Get() { return handle_; }

  private:
    void*       MapImpl() override { return handle_; }
    const void* MapImpl() const override { return handle_; }
};

std::unique_ptr<ShaderResourceHandle> RayEngine::EngineNode::CreateShaderResource(const ShaderResourceDescription& desc)
{
    const auto [it, success] = shaderResources_.insert(desc.shaderResouce->Clone());
    if (!success)
        throw std::runtime_error("Tried to create duplicate intersectable.");
    return std::make_unique<ShaderResourceH>(
        [this](const IShaderResource* resource) { DeleteShaderResource(resource); }, it->get());
}

class GeneratorShaderH : public GeneratorShaderHandle, public HandleBase<IRayGeneratorShader>
{
  public:
    GeneratorShaderH(std::function<void(const IRayGeneratorShader)> deleterCallback, IRayGeneratorShader handle)
        : HandleBase(std::move(deleterCallback), handle)
    {
    }

    const IRayGeneratorShader Get() const { return handle_; }
};

std::unique_ptr<GeneratorShaderHandle> RayEngine::EngineNode::CreateShader(const GeneratorShaderDescription& desc)
{
    const auto [it, success] = generatorShaders_.insert(desc.generatorShader);
    if (!success)
        throw std::runtime_error("Tried to create duplicate intersectable.");
    return std::make_unique<GeneratorShaderH>([this](const IRayGeneratorShader shader) { DeleteShader(shader); }, *it);
}

class HitShaderH : public HitShaderHandle, public HandleBase<IHitShader>
{
  public:
    HitShaderH(std::function<void(const IHitShader)> deleterCallback, IHitShader handle)
        : HandleBase(std::move(deleterCallback), handle)
    {
    }

    const IHitShader Get() const { return handle_; }
};

std::unique_ptr<HitShaderHandle> RayEngine::EngineNode::CreateShader(const HitShaderDescription& desc)
{
    const auto [it, success] = hitShaders_.insert(desc.hitShader);
    if (!success)
        throw std::runtime_error("Tried to create duplicate shader.");
    return std::make_unique<HitShaderH>([this](const IHitShader shader) { DeleteShader(shader); }, *it);
}

class PierceShaderH : public PierceShaderHandle, public HandleBase<IPierceShader>
{
  public:
    PierceShaderH(std::function<void(const IPierceShader)> deleterCallback, IPierceShader handle)
        : HandleBase(std::move(deleterCallback), handle)
    {
    }

    const IPierceShader Get() const { return handle_; }
};

std::unique_ptr<PierceShaderHandle> RayEngine::EngineNode::CreateShader(const PierceShaderDescription& desc)
{
    const auto [it, success] = pierceShaders_.insert(desc.pierceShader);
    if (!success)
        throw std::runtime_error("Tried to create duplicate shader.");
    return std::make_unique<PierceShaderH>([this](const IPierceShader shader) { DeleteShader(shader); }, *it);
}

class OcclusionShaderH : public OcclusionShaderHandle, public HandleBase<IOcclusionShader>
{
  public:
    OcclusionShaderH(std::function<void(const IOcclusionShader)> deleterCallback, IOcclusionShader handle)
        : HandleBase(std::move(deleterCallback), handle)
    {
    }

    const IOcclusionShader Get() const { return handle_; }
};

std::unique_ptr<OcclusionShaderHandle> RayEngine::EngineNode::CreateShader(const OcclusionShaderDescription& desc)
{
    const auto [it, success] = occlusionShaders_.insert(desc.occlusionShader);
    if (!success)
        throw std::runtime_error("Tried to create duplicate shader.");
    return std::make_unique<OcclusionShaderH>([this](const IOcclusionShader shader) { DeleteShader(shader); }, *it);
}

class MissShaderH : public MissShaderHandle, public HandleBase<IMissShader>
{
  public:
    MissShaderH(std::function<void(const IMissShader)> deleterCallback, IMissShader handle)
        : HandleBase(std::move(deleterCallback), handle)
    {
    }

    const IMissShader Get() const { return handle_; }
};

std::unique_ptr<MissShaderHandle> RayEngine::EngineNode::CreateShader(const MissShaderDescription& desc)
{
    const auto [it, success] = missShaders_.insert(desc.missShader);
    if (!success)
        throw std::runtime_error("Tried to create duplicate shader.");
    return std::make_unique<MissShaderH>([this](const IMissShader shader) { DeleteShader(shader); }, *it);
}

class RenderTargetH : public RenderTargetHandle, public HandleBase<std::vector<Vector3D>*>
{
  public:
    RenderTargetH(std::function<void(const std::vector<Vector3D>*)> deleterCallback,
                  std::vector<Vector3D>*                            handle,
                  const std::uint32_t                               width,
                  const std::uint32_t                               height)
        : HandleBase(std::move(deleterCallback), handle), width_(width), height_(height)
    {
    }

    Texture GetAsTexture(const TextureFormat format) const override
    {
        Texture texture{"Render Target", width_, height_, format == TextureFormat::RGB ? 3u : 4u};
        for (const auto& color : *handle_)
        {
            texture.image.push_back(std::min(std::max(color.x, 0.f), 1.f) * 255);
            texture.image.push_back(std::min(std::max(color.y, 0.f), 1.f) * 255);
            texture.image.push_back(std::min(std::max(color.z, 0.f), 1.f) * 255);
            if (format == TextureFormat::RGBA)
                texture.image.push_back(255);
        }
        return texture;
    }

    void GetAsTexture(const TextureFormat format, Texture& texture) const override
    {
        texture.name          = "Render Target";
        texture.w             = width_;
        texture.h             = height_;
        texture.bytesPerTexel = format == TextureFormat::RGB ? 3u : 4u;
        texture.image.resize(texture.w * texture.h * texture.bytesPerTexel);

        for (std::size_t i = 0; i < handle_->size(); ++i)
        {
            texture.image[i * texture.bytesPerTexel]     = std::min(std::max(handle_->at(i).x, 0.f), 1.f) * 255;
            texture.image[i * texture.bytesPerTexel + 1] = std::min(std::max(handle_->at(i).y, 0.f), 1.f) * 255;
            texture.image[i * texture.bytesPerTexel + 2] = std::min(std::max(handle_->at(i).z, 0.f), 1.f) * 255;
            if (format == TextureFormat::RGBA)
                texture.image[i * texture.bytesPerTexel + 3] = 255;
        }
    }

    std::uint32_t GetWidth() { return width_; }
    std::uint32_t GetHeight() { return height_; }

    std::vector<Vector3D>* Get() { return handle_; }

  private:
    std::uint32_t width_;
    std::uint32_t height_;
};

std::unique_ptr<RenderTargetHandle> RayEngine::EngineNode::CreateRenderTarget(const RenderTargetDescription& desc)
{
    const auto [it, success] = renderTargets_.insert(std::make_unique<std::vector<Vector3D>>(desc.width * desc.height));
    if (!success)
        throw std::runtime_error("Tried to create duplicate render target.");
    return std::make_unique<RenderTargetH>([this](const std::vector<Vector3D>* renderTarget)
                                           { DeleteRenderTarget(renderTarget); },
                                           it->get(),
                                           desc.width,
                                           desc.height);
}

class PipelineH : public PipelineHandle
{
  public:
    PipelineH(const IIntersectable*    scene,
              GeneratorShaderResourceP generatorShaderPackage,
              HitShaderResourceP       hitShaderPackage,
              PierceShaderResourceP    pierceShaderPackage,
              OcclusionShaderResourceP occlusionShaderPackage,
              MissShaderResourceP      missShaderPackage)
        : scene_(scene),
          generatorShaderPackage_(std::move(generatorShaderPackage)),
          hitShaderPackage_(std::move(hitShaderPackage)),
          pierceShaderPackage_(std::move(pierceShaderPackage)),
          occlusionShaderPackage_(std::move(occlusionShaderPackage)),
          missShaderPackage_(std::move(missShaderPackage))
    {
    }

    void Run(RenderTargetHandle& target) const override
    {
        PipelineImplement::Run(*dynamic_cast<RenderTargetH*>(&target)->Get(),
                               dynamic_cast<RenderTargetH*>(&target)->GetWidth(),
                               dynamic_cast<RenderTargetH*>(&target)->GetHeight(),
                               scene_,
                               generatorShaderPackage_,
                               hitShaderPackage_,
                               pierceShaderPackage_,
                               occlusionShaderPackage_,
                               missShaderPackage_);
    }

  private:
    const IIntersectable*    scene_;
    GeneratorShaderResourceP generatorShaderPackage_;
    HitShaderResourceP       hitShaderPackage_;
    PierceShaderResourceP    pierceShaderPackage_;
    OcclusionShaderResourceP occlusionShaderPackage_;
    MissShaderResourceP      missShaderPackage_;
};

std::unique_ptr<PipelineHandle> RayEngine::EngineNode::CreatePipeline(const PipelineDescription& desc)
{
    const auto* scene = dynamic_cast<SceneH*>(desc.scene)->Get();

    GeneratorShaderResourceP generatorShaderResourcePackage{};
    if (desc.generatorShader.shader)
    {
        generatorShaderResourcePackage.shader = dynamic_cast<GeneratorShaderH*>(desc.generatorShader.shader)->Get();
        for (auto* resource : desc.generatorShader.resources)
        {
            generatorShaderResourcePackage.resources.push_back(dynamic_cast<ShaderResourceH*>(resource)->Get());
        }
    }

    HitShaderResourceP hitShaderResourcePackage{};
    if (desc.hitShader.shader)
    {
        hitShaderResourcePackage.shader = dynamic_cast<HitShaderH*>(desc.hitShader.shader)->Get();
        for (auto* resource : desc.hitShader.resources)
        {
            hitShaderResourcePackage.resources.push_back(dynamic_cast<ShaderResourceH*>(resource)->Get());
        }
    }

    PierceShaderResourceP pierceShaderResourcePackage{};
    if (desc.pierceShader.shader)
    {
        pierceShaderResourcePackage.shader = dynamic_cast<PierceShaderH*>(desc.pierceShader.shader)->Get();
        for (auto* resource : desc.pierceShader.resources)
        {
            pierceShaderResourcePackage.resources.push_back(dynamic_cast<ShaderResourceH*>(resource)->Get());
        }
    }

    OcclusionShaderResourceP occlusionShaderResourcePackage{};
    if (desc.occlusionShader.shader)
    {
        occlusionShaderResourcePackage.shader = dynamic_cast<OcclusionShaderH*>(desc.occlusionShader.shader)->Get();
        for (auto* resource : desc.occlusionShader.resources)
        {
            occlusionShaderResourcePackage.resources.push_back(dynamic_cast<ShaderResourceH*>(resource)->Get());
        }
    }

    MissShaderResourceP missShaderResourcePackage{};
    if (desc.missShader.shader)
    {
        missShaderResourcePackage.shader = dynamic_cast<MissShaderH*>(desc.missShader.shader)->Get();
        for (auto* resource : desc.missShader.resources)
        {
            missShaderResourcePackage.resources.push_back(dynamic_cast<ShaderResourceH*>(resource)->Get());
        }
    }

    return std::make_unique<PipelineH>(scene,
                                       generatorShaderResourcePackage,
                                       hitShaderResourcePackage,
                                       pierceShaderResourcePackage,
                                       occlusionShaderResourcePackage,
                                       missShaderResourcePackage);
}

void RayEngine::EngineNode::FetchIntersectable()
{
    // TODO
}

void RayEngine::EngineNode::FetchShaderResource()
{
    // TODO
}

void RayEngine::EngineNode::DeleteIntersectable(const IIntersectable* intersectable)
{
    intersectables_.erase(std::ranges::find_if(intersectables_,
                                               [intersectable](const std::unique_ptr<IIntersectable>& ptr)
                                               { return ptr.get() == intersectable; }));
}

void RayEngine::EngineNode::DeleteShaderResource(const IShaderResource* shaderResource)
{
    shaderResources_.erase(std::ranges::find_if(shaderResources_,
                                                [shaderResource](const std::unique_ptr<IShaderResource>& ptr)
                                                { return ptr.get() == shaderResource; }));
}

void RayEngine::EngineNode::DeleteShader(const IRayGeneratorShader shader) { generatorShaders_.erase(shader); }

void RayEngine::EngineNode::DeleteShader(const IHitShader shader) { hitShaders_.erase(shader); }

void RayEngine::EngineNode::DeleteShader(const IPierceShader shader) { pierceShaders_.erase(shader); }

void RayEngine::EngineNode::DeleteShader(const IOcclusionShader shader) { occlusionShaders_.erase(shader); }

void RayEngine::EngineNode::DeleteShader(const IMissShader shader) { missShaders_.erase(shader); }

void RayEngine::EngineNode::DeleteRenderTarget(const std::vector<Vector3D>* renderTarget)
{
    renderTargets_.erase(std::ranges::find_if(renderTargets_,
                                              [renderTarget](const std::unique_ptr<std::vector<Vector3D>>& ptr)
                                              { return ptr.get() == renderTarget; }));
}