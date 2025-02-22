#include "pipeline/PipelineImplement.h"
#include <algorithm>
#include <cassert>
#include <cstring>
#include <execution>
#include <ranges>

namespace
{
    struct RayContainer
    {
        int      rayID;
        RayType  type;
        Vector3D rayOrigin;
        Vector3D rayDirection;
    };

    inline void SetPixel(std::vector<unsigned char>& buffer, const int id, const ShaderOutput& pixel)
    {
        buffer[id * 3] += pixel.color[0];
        buffer[id * 3 + 1] += pixel.color[1];
        buffer[id * 3 + 2] += pixel.color[2];
    }

    inline Ray InitRay(const std::vector<RayContainer>& rayContainers)
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

    void UpdateRayStack(std::vector<RayContainer>& rayContainers, const int id, RayGeneratorOutput& newRays)
    {
        rayContainers.pop_back();

        for (const auto& r : newRays.rays)
        {
            RayContainer rayContainer = {id, r.type, r.rayDirection, r.rayOrigin};
            rayContainers.push_back(rayContainer);
        }
        newRays.rays.clear();
    }

    void GeneratePrimaryRays(const GeneratorShaderResourceP& generatorShaderPackage,
                             std::vector<RayContainer>&      rayContainers,
                             const int                       rayID,
                             RayGeneratorOutput&             rays)
    {
        generatorShaderPackage.shader->Shade(rayID, generatorShaderPackage.resources, rays);
        for (auto& ray : rays.rays)
        {
            RayContainer rayContainer = {rayID, ray.type, ray.rayOrigin, ray.rayDirection};
            rayContainers.push_back(rayContainer);
        }
        rays.rays.clear();
    }

} // namespace

void PipelineImplement::Run(std::vector<unsigned char>&     buffer,
                            const TextureView&              texture,
                            const IIntersectable*           scene,
                            const GeneratorShaderResourceP& generatorShaderPackage,
                            const HitShaderResourceP&       hitShaderPackage,
                            const PierceShaderResourceP&    pierceShaderPackage,
                            const OcclusionShaderResourceP& occlusionShaderPackage,
                            const MissShaderResourceP&      missShaderPackage)
{
    if (!generatorShaderPackage.shader)
        return;
    std::memset(buffer.data(), 0, buffer.size());

    std::vector<RayContainer> rayContainers;
    RayGeneratorOutput        newRays;
    PierceShaderInput         pierceInput{};
    for (int rayID = 0; rayID < texture.w * texture.h; ++rayID)
    {

        GeneratePrimaryRays(generatorShaderPackage, rayContainers, rayID, newRays);

        while (!rayContainers.empty())
        {
            const auto type = rayContainers.back().type;
            const auto ray  = InitRay(rayContainers);

            IntersectionInfo info{};
            info.distance = std::numeric_limits<float>::max();

            switch (type)
            {
                case RayType::Pierce:
                    if (pierceShaderPackage.shader)
                    {
                        scene->IntersectAll(pierceInput.intersectionInfo, ray);

                        const auto id = rayContainers.back().rayID;
                        for (auto& info : pierceInput.intersectionInfo)
                        {
                            info.rayOrigin    = ray.origin;
                            info.rayDirection = ray.direction;
                        }

                        if (info.hit = !pierceInput.intersectionInfo.empty())
                        {
                            const auto pixel = pierceShaderPackage.shader->Shade(
                                rayID, pierceInput, pierceShaderPackage.resources, newRays);
                            SetPixel(buffer, rayID, pixel);
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
                            const HitShaderInput hitShaderInput = {&info};
                            const auto           pixel          = hitShaderPackage.shader->Shade(
                                rayID, hitShaderInput, hitShaderPackage.resources, newRays);
                            SetPixel(buffer, rayID, pixel);
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
                            const OcclusionShaderInput occlusionShaderInput = {ray.origin, ray.direction};
                            const auto                 pixel                = occlusionShaderPackage.shader->Shade(
                                rayID, occlusionShaderInput, occlusionShaderPackage.resources, newRays);
                            SetPixel(buffer, rayID, pixel);
                        }
                    }
                    break;
                default:
                    assert(false);
                    break;
            }
            if (!info.hit && missShaderPackage.shader)
            {
                const MissShaderInput missShaderInput = {ray.origin, ray.direction};
                const auto            pixel =
                    missShaderPackage.shader->Shade(rayID, missShaderInput, missShaderPackage.resources, newRays);
                SetPixel(buffer, rayID, pixel);
            }

            UpdateRayStack(rayContainers, rayID, newRays);
        }
    }
    //);
}
