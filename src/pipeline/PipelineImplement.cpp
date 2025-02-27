#include "pipeline/PipelineImplement.h"
#include <algorithm>
#include <cassert>
#include <cstring>
#include <execution>
#include <ranges>

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
        generatorShaderPackage.shader->Shade(rayID, generatorShaderPackage.resources, currentRays);

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
                            const auto pixel = pierceShaderPackage.shader->Shade(
                                rayID, pierceInput, pierceShaderPackage.resources, newRays);
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
                            const auto           pixel          = hitShaderPackage.shader->Shade(
                                rayID, hitShaderInput, hitShaderPackage.resources, newRays);
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
                            const auto pixel = occlusionShaderPackage.shader->Shade(
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
                    missShaderPackage.shader->Shade(rayID, missShaderInput, missShaderPackage.resources, newRays);
                buffer[rayID] += pixel.color;
            }

            UpdateRayStack(currentRays.rays, rayID, newRays);
        }
    }
}
