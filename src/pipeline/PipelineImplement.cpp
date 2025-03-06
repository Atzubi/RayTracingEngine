#include "pipeline/PipelineImplement.h"
#include "RayTraceEngine/BasicStructures.h"
#include "RayTraceEngine/Intersectable.h"
#include "RayTraceEngine/Pipeline.h"

#include <cassert>
#include <cstring>
#include <limits>

namespace
{
    inline Ray InitRay(const std::vector<GeneratorRay>& rayContainers)
    {
        const auto& r = rayContainers.back();
        return {r.rayOrigin, r.rayDirection, r.rayDirection.GetInverse()};
    }

    IntersectionInfo GetFirstIntersection(const std::vector<IntersectionInfo>& infos)
    {
        IntersectionInfo closest{false, std::numeric_limits<float>::max()};
        for (const auto& info : infos)
        {
            if (info.hit && closest.distance > info.distance)
            {
                closest = info;
            }
        }
        return closest;
    }

    void UpdateRayStack(std::vector<GeneratorRay>& rayContainers, const int id, RayGeneratorOutput& newRays)
    {
        rayContainers.pop_back();
        rayContainers.insert(rayContainers.end(), newRays.rays.begin(), newRays.rays.end());
        newRays.rays.clear();
    }

} // namespace

void PipelineImplement::Run(std::vector<Vector3D>&          buffer,
                            const std::uint32_t             width,
                            const std::uint32_t             height,
                            const IIntersectable*           scene,
                            const GeneratorShaderResourceP& generatorShaderPackage,
                            const HitShaderResourceP&       hitShaderPackage,
                            const PierceShaderResourceP&    pierceShaderPackage,
                            const OcclusionShaderResourceP& occlusionShaderPackage,
                            const MissShaderResourceP&      missShaderPackage)
{
    if (!generatorShaderPackage.shader)
        return;
    std::memset(buffer.data(), 0, buffer.size() * sizeof(Vector3D));

    RayGeneratorOutput currentRays;
    RayGeneratorOutput newRays;
    PierceShaderInput  pierceInput{};
    for (std::uint64_t rayID = 0; rayID < width * height; ++rayID)
    {
        generatorShaderPackage.shader(rayID, generatorShaderPackage.resources, currentRays);

        while (!currentRays.rays.empty())
        {
            const auto type = currentRays.rays.back().type;
            const auto ray  = InitRay(currentRays.rays);

            IntersectionInfo info{};
            info.distance = std::numeric_limits<float>::max();

            switch (type)
            {
                case RayType::Pierce:
                    if (pierceShaderPackage.shader)
                    {
                        scene->IntersectAll(pierceInput.intersectionInfo, ray);
                        pierceInput.id = currentRays.rays.back().id;

                        for (auto& info : pierceInput.intersectionInfo)
                        {
                            info.rayOrigin    = ray.origin;
                            info.rayDirection = ray.direction;
                        }

                        if (info.hit = !pierceInput.intersectionInfo.empty())
                        {
                            const auto pixel =
                                pierceShaderPackage.shader(rayID, pierceInput, pierceShaderPackage.resources, newRays);
                            buffer[rayID] += pixel.color;
                        }
                    }
                    break;
                case RayType::Closest:
                    if (hitShaderPackage.shader)
                    {
                        scene->IntersectFirst(info, ray);
                        info.rayOrigin    = ray.origin;
                        info.rayDirection = ray.direction;

                        if (info.hit)
                        {
                            const HitShaderInput hitShaderInput = {currentRays.rays.back().id, &info};
                            const auto           pixel =
                                hitShaderPackage.shader(rayID, hitShaderInput, hitShaderPackage.resources, newRays);
                            buffer[rayID] += pixel.color;
                        }
                    }
                    break;
                case RayType::Any:
                    if (occlusionShaderPackage.shader)
                    {
                        scene->IntersectAny(info, ray);
                        info.rayOrigin    = ray.origin;
                        info.rayDirection = ray.direction;

                        if (info.hit)
                        {
                            const OcclusionShaderInput occlusionShaderInput = {
                                currentRays.rays.back().id, ray.origin, ray.direction};
                            const auto pixel = occlusionShaderPackage.shader(
                                rayID, occlusionShaderInput, occlusionShaderPackage.resources, newRays);
                            buffer[rayID] += pixel.color;
                        }
                    }
                    break;
                default:
                    assert(false);
                    break;
            }
            if (!info.hit && missShaderPackage.shader)
            {
                const MissShaderInput missShaderInput = {currentRays.rays.back().id, ray.origin, ray.direction};
                const auto            pixel =
                    missShaderPackage.shader(rayID, missShaderInput, missShaderPackage.resources, newRays);
                buffer[rayID] += pixel.color;
            }

            UpdateRayStack(currentRays.rays, rayID, newRays);
        }
    }
}

std::vector<std::uint8_t> Texture::Serialize() const
{
    const auto                size = sizeof(std::uint32_t) * 3 + image.size();
    std::vector<std::uint8_t> buffer(size);
    std::memcpy(buffer.data(), &w, sizeof(w));
    auto offset = sizeof(w);
    std::memcpy(buffer.data() + offset, &h, sizeof(h));
    offset += sizeof(h);
    std::memcpy(buffer.data() + offset, &bytesPerTexel, sizeof(bytesPerTexel));
    offset += sizeof(bytesPerTexel);
    const auto imageLength = image.size();
    std::memcpy(buffer.data() + offset, image.data(), imageLength);
    return buffer;
}

std::unique_ptr<IShaderResource> Texture::Deserialize(const std::span<const std::uint8_t> buffer) const
{
    auto texture = std::make_unique<Texture>();
    std::memcpy(&texture->w, buffer.data(), sizeof(texture->w));
    auto offset = sizeof(texture->w);
    std::memcpy(&texture->h, buffer.data() + offset, sizeof(texture->h));
    offset += sizeof(texture->h);
    std::memcpy(&texture->bytesPerTexel, buffer.data() + offset, sizeof(texture->bytesPerTexel));
    offset += sizeof(texture->bytesPerTexel);
    const auto imageSize = texture->w * texture->h * texture->bytesPerTexel;
    texture->image.resize(imageSize);
    std::memcpy(texture->image.data(), buffer.data() + offset, imageSize);
    return texture;
}
