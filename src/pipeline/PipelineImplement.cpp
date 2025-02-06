#include "pipeline/PipelineImplement.h"
#include <cstring>

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
    {
        return;
    }
    std::memset(buffer.data(), 0, buffer.size());

    std::vector<RayContainer> rayContainers;
    RayGeneratorOutput        rays;
    RayGeneratorOutput        newRays;
    for (int rayID = 0; rayID < texture.w * texture.h; rayID++)
    {
        GeneratePrimaryRays(generatorShaderPackage, rayContainers, rayID, rays);

        while (!rayContainers.empty())
        {
            Ray ray = InitRay(rayContainers);

            PierceShaderInput pierceInput{};
            IntersectionInfo  info{};
            info.distance = std::numeric_limits<double>::max();
            if (pierceShaderPackage.shader)
            {
                std::vector<IntersectionInfo> infos;
                scene->IntersectAll(infos, ray);

                int id = rayContainers.back().rayID;
                for (auto& info : infos)
                {
                    info.rayOrigin    = rayContainers.back().rayOrigin;
                    info.rayDirection = rayContainers.back().rayDirection;
                }

                pierceInput = {infos};

                info = GetFirstIntersection(infos);
            }
            else if (hitShaderPackage.shader)
            {
                scene->IntersectFirst(info, ray);
            }
            else
            {
                scene->IntersectAny(info, ray);
            }
            info.rayOrigin    = rayContainers.back().rayOrigin;
            info.rayDirection = rayContainers.back().rayDirection;

            if (info.hit)
            {
                if (pierceShaderPackage.shader)
                {
                    const auto pixel =
                        pierceShaderPackage.shader->Shade(rayID, pierceInput, pierceShaderPackage.resources, newRays);
                    SetPixel(buffer, rayID, pixel);
                }
                if (hitShaderPackage.shader)
                {
                    HitShaderInput hitShaderInput = {&info};
                    const auto     pixel =
                        hitShaderPackage.shader->Shade(rayID, hitShaderInput, hitShaderPackage.resources, newRays);
                    SetPixel(buffer, rayID, pixel);
                }
                if (occlusionShaderPackage.shader)
                {
                    OcclusionShaderInput occlusionShaderInput = {ray.origin, ray.direction};
                    const auto           pixel                = occlusionShaderPackage.shader->Shade(
                        rayID, occlusionShaderInput, occlusionShaderPackage.resources, newRays);
                    SetPixel(buffer, rayID, pixel);
                }
            }
            else if (missShaderPackage.shader)
            {
                MissShaderInput missShaderInput = {ray.origin, ray.direction};
                const auto      pixel =
                    missShaderPackage.shader->Shade(rayID, missShaderInput, missShaderPackage.resources, newRays);
                SetPixel(buffer, rayID, pixel);
            }

            UpdateRayStack(rayContainers, rayID, newRays);
        }
    }
}

inline void PipelineImplement::SetPixel(std::vector<unsigned char>& buffer, int id, const ShaderOutput& pixel)
{
    buffer[id * 3] += pixel.color[0];
    buffer[id * 3 + 1] += pixel.color[1];
    buffer[id * 3 + 2] += pixel.color[2];
}

inline Ray PipelineImplement::InitRay(const std::vector<RayContainer>& rayContainers)
{
    const auto r = rayContainers.back();
    return {r.rayOrigin, r.rayDirection, r.rayOrigin.getInverse()};
}

IntersectionInfo PipelineImplement::GetFirstIntersection(std::vector<IntersectionInfo>& infos)
{
    IntersectionInfo closest{false, std::numeric_limits<double>::max()};
    for (auto info : infos)
    {
        if (info.hit && closest.distance > info.distance)
        {
            closest = info;
        }
    }
    return closest;
}

void PipelineImplement::UpdateRayStack(std::vector<RayContainer>& rayContainers, int id, RayGeneratorOutput& newRays)
{
    rayContainers.pop_back();

    for (auto& r : newRays.rays)
    {
        RayContainer rayContainer = {id, r.rayDirection, r.rayOrigin};
        rayContainers.push_back(rayContainer);
    }
    newRays.rays.clear();
}

void PipelineImplement::GeneratePrimaryRays(const GeneratorShaderResourceP& generatorShaderPackage,
                                            std::vector<RayContainer>&      rayContainers,
                                            int                             rayID,
                                            RayGeneratorOutput&             rays)
{
    generatorShaderPackage.shader->Shade(rayID, generatorShaderPackage.resources, rays);
    for (auto& ray : rays.rays)
    {
        RayContainer rayContainer = {rayID, ray.rayOrigin, ray.rayDirection};
        rayContainers.push_back(rayContainer);
    }
    rays.rays.clear();
}
