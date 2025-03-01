#include "EngineNode.h"
#include "bvh/DBVH.h"
#include "intersectable/Instance.h"
#include "pipeline/PipelineImplement.h"

#include <functional>

class HandleBase
{
  public:
    HandleBase(std::function<void(std::uint64_t)> deleterCallback, const std::uint64_t id)
        : deleterCallback_(std::move(deleterCallback)), id_(id)
    {
    }

    ~HandleBase() { deleterCallback_(id_); }

  protected:
    std::function<void(std::uint64_t)> deleterCallback_;
    std::uint64_t                      id_;
};

class IntersectableObjectH : public IntersectableObjectHandle, public HandleBase
{
  public:
    IntersectableObjectH(std::function<void(std::uint64_t)> deleterCallback,
                         const std::uint64_t                id,
                         IIntersectable*                    handle)
        : HandleBase(std::move(deleterCallback), id), handle_(handle)
    {
    }

    const IIntersectable* Get() const { return handle_; }

  private:
    IIntersectable* handle_;
};

std::unique_ptr<IntersectableObjectHandle>
RayEngine::EngineNode::CreateIntersectableObject(const IntersectableObjectDescription& desc)
{
    const auto [it, success] =
        intersectables_.emplace(GetNextId(), desc.intersectable.Deserialize(desc.intersectable.Serialize()));
    if (!success)
        throw std::runtime_error("Tried to create duplicate intersectable.");
    return std::make_unique<IntersectableObjectH>(
        [this](const std::uint64_t id) { DeleteResource(id); }, it->first, it->second.get());
}

class InstanceH : public InstanceHandle, public HandleBase
{
  public:
    InstanceH(std::function<void(std::uint64_t)> deleterCallback,
              const std::uint64_t                id,
              IIntersectable*                    handle,
              DBVH*                              reference)
        : HandleBase(std::move(deleterCallback), id), handle_(handle), reference_(reference)
    {
    }

    std::uint64_t GetId() const override { return id_; }

    void Transform(const Matrix4x4& transform) override
    {
        reference_->RemoveObjects({handle_});
        dynamic_cast<Instance*>(handle_)->ApplyTransform(transform);
        reference_->AddObjects({handle_});
    }

    Matrix4x4 GetTransform() const override { return dynamic_cast<Instance*>(handle_)->GetTransform(); }

  private:
    IIntersectable* handle_;
    DBVH*           reference_;
};

class SceneH : public SceneHandle, public HandleBase
{
  public:
    SceneH(std::function<void(std::uint64_t)>           deleterCallback,
           const std::uint64_t                          id,
           std::vector<std::unique_ptr<InstanceHandle>> instances)
        : HandleBase(std::move(deleterCallback), id), instances_(std::move(instances))
    {
    }

    std::span<const std::unique_ptr<InstanceHandle>> GetInstanceHandles() const override { return instances_; }

    const std::uint64_t GetId() const { return id_; }

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
        const auto instanceId    = GetNextId();
        const auto intersectable = dynamic_cast<const IntersectableObjectH&>(intersectablePack.intersectable).Get();
        auto       instance = std::make_unique<Instance>(intersectable, [this]() { FetchIntersectable(); }, instanceId);
        instance->ApplyTransform(intersectablePack.transform);

        const auto [it, success] = intersectables_.emplace(instanceId, std::move(instance));
        if (!success)
            throw std::runtime_error("Tried to create duplicate intersectable.");

        instances.push_back(it->second.get());

        instanceHandles.push_back(std::make_unique<InstanceH>(
            [this](const std::uint64_t id) { DeleteResource(id); }, it->first, it->second.get(), sceneBvh.get()));
    }

    sceneBvh->AddObjects(instances);
    const auto [it, success] = intersectables_.emplace(GetNextId(), std::move(sceneBvh));
    if (!success)
        throw std::runtime_error("Tried to create duplicate intersectable.");

    return std::make_unique<SceneH>(
        [this](const std::uint64_t id) { DeleteResource(id); }, it->first, std::move(instanceHandles));
}

class ShaderResourceH : public ShaderResourceHandle, public HandleBase
{
  public:
    ShaderResourceH(std::function<void(std::uint64_t)> deleterCallback, const std::uint64_t id, IShaderResource* handle)
        : HandleBase(std::move(deleterCallback), id), handle_(handle)
    {
    }

    std::uint64_t GetId() const { return id_; }

  private:
    void*       MapImpl() override { return handle_; }
    const void* MapImpl() const override { return handle_; }

    IShaderResource* handle_;
};

std::unique_ptr<ShaderResourceHandle> RayEngine::EngineNode::CreateShaderResource(const ShaderResourceDescription& desc)
{
    const auto [it, success] =
        shaderResources_.emplace(GetNextId(), desc.shaderResouce->Deserialize(desc.shaderResouce->Serialize()));
    if (!success)
        throw std::runtime_error("Tried to create duplicate intersectable.");
    return std::make_unique<ShaderResourceH>(
        [this](const std::uint64_t id) { DeleteResource(id); }, it->first, it->second.get());
}

class GeneratorShaderH : public GeneratorShaderHandle, public HandleBase
{
  public:
    GeneratorShaderH(std::function<void(std::uint64_t)> deleterCallback, const std::uint64_t id)
        : HandleBase(std::move(deleterCallback), id)
    {
    }

    std::uint64_t GetId() const { return id_; }
};

std::unique_ptr<GeneratorShaderHandle> RayEngine::EngineNode::CreateShader(const GeneratorShaderDescription& desc)
{
    const auto [it, success] = generatorShaders_.emplace(GetNextId(), desc.generatorShader);
    if (!success)
        throw std::runtime_error("Tried to create duplicate intersectable.");
    return std::make_unique<GeneratorShaderH>([this](const std::uint64_t id) { DeleteResource(id); }, it->first);
}

class HitShaderH : public HitShaderHandle, public HandleBase
{
  public:
    HitShaderH(std::function<void(std::uint64_t)> deleterCallback, const std::uint64_t id)
        : HandleBase(std::move(deleterCallback), id)
    {
    }

    std::uint64_t GetId() const { return id_; }
};

std::unique_ptr<HitShaderHandle> RayEngine::EngineNode::CreateShader(const HitShaderDescription& desc)
{
    const auto [it, success] = hitShaders_.emplace(GetNextId(), desc.hitShader);
    if (!success)
        throw std::runtime_error("Tried to create duplicate shader.");
    return std::make_unique<HitShaderH>([this](const std::uint64_t id) { DeleteResource(id); }, it->first);
}

class PierceShaderH : public PierceShaderHandle, public HandleBase
{
  public:
    PierceShaderH(std::function<void(std::uint64_t)> deleterCallback, const std::uint64_t id)
        : HandleBase(std::move(deleterCallback), id)
    {
    }

    std::uint64_t GetId() const { return id_; }
};

std::unique_ptr<PierceShaderHandle> RayEngine::EngineNode::CreateShader(const PierceShaderDescription& desc)
{
    const auto [it, success] = pierceShaders_.emplace(GetNextId(), desc.pierceShader);
    if (!success)
        throw std::runtime_error("Tried to create duplicate shader.");
    return std::make_unique<PierceShaderH>([this](const std::uint64_t id) { DeleteResource(id); }, it->first);
}

class OcclusionShaderH : public OcclusionShaderHandle, public HandleBase
{
  public:
    OcclusionShaderH(std::function<void(std::uint64_t)> deleterCallback, const std::uint64_t id)
        : HandleBase(std::move(deleterCallback), id)
    {
    }

    std::uint64_t GetId() const { return id_; }
};

std::unique_ptr<OcclusionShaderHandle> RayEngine::EngineNode::CreateShader(const OcclusionShaderDescription& desc)
{
    const auto [it, success] = occlusionShaders_.emplace(GetNextId(), desc.occlusionShader);
    if (!success)
        throw std::runtime_error("Tried to create duplicate shader.");
    return std::make_unique<OcclusionShaderH>([this](const std::uint64_t id) { DeleteResource(id); }, it->first);
}

class MissShaderH : public MissShaderHandle, public HandleBase
{
  public:
    MissShaderH(std::function<void(std::uint64_t)> deleterCallback, const std::uint64_t id)
        : HandleBase(std::move(deleterCallback), id)
    {
    }

    std::uint64_t GetId() const { return id_; }
};

std::unique_ptr<MissShaderHandle> RayEngine::EngineNode::CreateShader(const MissShaderDescription& desc)
{
    const auto [it, success] = missShaders_.emplace(GetNextId(), desc.missShader);
    if (!success)
        throw std::runtime_error("Tried to create duplicate shader.");
    return std::make_unique<MissShaderH>([this](const std::uint64_t id) { DeleteResource(id); }, it->first);
}

class RenderTargetH : public RenderTargetHandle, public HandleBase
{
  public:
    RenderTargetH(std::function<void(std::uint64_t)> deleterCallback,
                  const std::uint64_t                id,
                  std::vector<Vector3D>*             handle,
                  const std::uint32_t                width,
                  const std::uint32_t                height)
        : HandleBase(std::move(deleterCallback), id), handle_(handle), width_(width), height_(height)
    {
    }

    Texture GetAsTexture(const TextureFormat format) const override
    {
        Texture texture{};
        texture.w             = width_;
        texture.h             = height_;
        texture.bytesPerTexel = format == TextureFormat::RGB ? 3u : 4u;
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

    std::uint64_t          GetId() { return id_; }
    std::vector<Vector3D>* Get() { return handle_; }

  private:
    std::vector<Vector3D>* handle_;
    std::uint32_t          width_;
    std::uint32_t          height_;
};

std::unique_ptr<RenderTargetHandle> RayEngine::EngineNode::CreateRenderTarget(const RenderTargetDescription& desc)
{
    const auto [it, success] =
        renderTargets_.emplace(GetNextId(), std::make_unique<std::vector<Vector3D>>(desc.width * desc.height));
    if (!success)
        throw std::runtime_error("Tried to create duplicate render target.");
    return std::make_unique<RenderTargetH>(
        [this](const std::uint64_t id) { DeleteResource(id); }, it->first, it->second.get(), desc.width, desc.height);
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
    const auto* scene = intersectables_[dynamic_cast<SceneH*>(desc.scene)->GetId()].get();

    GeneratorShaderResourceP generatorShaderResourcePackage{};
    if (desc.generatorShader.shader)
    {
        generatorShaderResourcePackage.shader =
            generatorShaders_[dynamic_cast<GeneratorShaderH*>(desc.generatorShader.shader)->GetId()];
        for (auto* resource : desc.generatorShader.resources)
        {
            generatorShaderResourcePackage.resources.push_back(
                shaderResources_[dynamic_cast<ShaderResourceH*>(resource)->GetId()].get());
        }
    }

    HitShaderResourceP hitShaderResourcePackage{};
    if (desc.hitShader.shader)
    {
        hitShaderResourcePackage.shader = hitShaders_[dynamic_cast<HitShaderH*>(desc.hitShader.shader)->GetId()];
        for (auto* resource : desc.hitShader.resources)
        {
            hitShaderResourcePackage.resources.push_back(
                shaderResources_[dynamic_cast<ShaderResourceH*>(resource)->GetId()].get());
        }
    }

    PierceShaderResourceP pierceShaderResourcePackage{};
    if (desc.pierceShader.shader)
    {
        pierceShaderResourcePackage.shader =
            pierceShaders_[dynamic_cast<PierceShaderH*>(desc.pierceShader.shader)->GetId()];
        for (auto* resource : desc.pierceShader.resources)
        {
            pierceShaderResourcePackage.resources.push_back(
                shaderResources_[dynamic_cast<ShaderResourceH*>(resource)->GetId()].get());
        }
    }

    OcclusionShaderResourceP occlusionShaderResourcePackage{};
    if (desc.occlusionShader.shader)
    {
        occlusionShaderResourcePackage.shader =
            occlusionShaders_[dynamic_cast<OcclusionShaderH*>(desc.occlusionShader.shader)->GetId()];
        for (auto* resource : desc.occlusionShader.resources)
        {
            occlusionShaderResourcePackage.resources.push_back(
                shaderResources_[dynamic_cast<ShaderResourceH*>(resource)->GetId()].get());
        }
    }

    MissShaderResourceP missShaderResourcePackage{};
    if (desc.missShader.shader)
    {
        missShaderResourcePackage.shader = missShaders_[dynamic_cast<MissShaderH*>(desc.missShader.shader)->GetId()];
        for (auto* resource : desc.missShader.resources)
        {
            missShaderResourcePackage.resources.push_back(
                shaderResources_[dynamic_cast<ShaderResourceH*>(resource)->GetId()].get());
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

void RayEngine::EngineNode::DeleteResource(const std::uint64_t id)
{
    const auto erased = intersectables_.erase(id) || shaderResources_.erase(id) || generatorShaders_.erase(id) ||
                        hitShaders_.erase(id) || pierceShaders_.erase(id) || occlusionShaders_.erase(id) ||
                        missShaders_.erase(id) || renderTargets_.erase(id);
    if (!erased)
        throw std::runtime_error("Failed to delete resource");
}

std::uint64_t RayEngine::EngineNode::GetNextId()
{
    auto id = usedIds_;
    if (!freeIds_.empty())
    {
        id = *freeIds_.begin();
        freeIds_.erase(id);
    }
    else
    {
        ++usedIds_;
    }
    return id;
}