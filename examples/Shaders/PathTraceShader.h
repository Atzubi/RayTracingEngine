#pragma once

#include "Common.h"
#include "RayTraceEngine/Shader.h"

#include <cstdlib>

ShaderOutput PathTraceShader(const std::uint64_t                     id,
                             const HitShaderInput&                   shaderInput,
                             const std::span<IShaderResource* const> shaderResource,
                             RayGeneratorOutput&                     newRays)
{
    const auto* materialMap = dynamic_cast<MaterialMap*>(shaderResource[2]);
    const auto* material    = dynamic_cast<Material*>(
        shaderResource[3 + materialMap->instanceToMaterial.at(shaderInput.intersectionInfo->instanceId)]);
    const auto  absorption       = Vector3D{1.f, 1.f, 1.f} - LoadKdTexel(*shaderInput.intersectionInfo, *material);
    const auto& opticalDensity   = material->Ni;
    const auto& dissolve         = material->d;
    const auto& volumeAbsorption = material->Ns;
    const auto& sampleCount      = dynamic_cast<SampleCountInfo*>(shaderResource[0])->samplesPerPixel;
    auto&       accumulatedAbsorption =
        dynamic_cast<PathData*>(shaderResource[1])->absorption[id * sampleCount + shaderInput.id];
    auto& depth = dynamic_cast<PathData*>(shaderResource[1])->depth[id * sampleCount + shaderInput.id];

    if (depth > 64)
        return {0, 0, 0};

    ++depth;

    const auto& incident = shaderInput.intersectionInfo->rayDirection;
    auto&       normal   = shaderInput.intersectionInfo->normal;
    const auto  backface = normal.Dot(incident) > 0;
    if (backface)
    {
        normal *= -1;

        // Absorption through volume
        accumulatedAbsorption += (Vector3D{1.f, 1.f, 1.f} - accumulatedAbsorption) * absorption * volumeAbsorption *
                                 shaderInput.intersectionInfo->distance;
        accumulatedAbsorption = {std::min(1.f, accumulatedAbsorption.x),
                                 std::min(1.f, accumulatedAbsorption.y),
                                 std::min(1.f, accumulatedAbsorption.z)};
    }
    // Surface absorption
    accumulatedAbsorption += (Vector3D{1.f, 1.f, 1.f} - accumulatedAbsorption) * absorption;
    accumulatedAbsorption = {std::min(1.f, accumulatedAbsorption.x),
                             std::min(1.f, accumulatedAbsorption.y),
                             std::min(1.f, accumulatedAbsorption.z)};

    if ((accumulatedAbsorption.x > 0.997f) && (accumulatedAbsorption.y > 0.997f) && (accumulatedAbsorption.z > 0.997f))
        return {0, 0, 0};

    Vector3D newDirection;
    while (true)
    {
        auto microfacetNormal = SampleMicrofacet(dissolve, normal);
        if (microfacetNormal.Dot(incident) > 0) // flipped face
            microfacetNormal = Reflect(microfacetNormal * -1.f, normal);

        const auto cosTheta           = fmax(-incident.Dot(microfacetNormal), 0.0f);
        const auto n1                 = backface ? opticalDensity : 1.f;
        const auto n2                 = backface ? 1.f : opticalDensity;
        const auto fresnelReflectance = FresnelSchlick(cosTheta, n1, n2);

        if (static_cast<float>(std::rand()) / RAND_MAX < fresnelReflectance)
        {
            newDirection = Reflect(incident, microfacetNormal);
            if (newDirection.Dot(normal) < 0)
                continue; // reflected into material -> resample
        }
        else
        {
            newDirection = Refract(incident, microfacetNormal, n1, n2);
            if (newDirection.Dot(normal) > 0)
                continue; // refracted out of material -> resample
        }
        newDirection.Normalize();
        break;
    }

    newRays.rays.push_back({RayType::Closest,
                            shaderInput.id,
                            shaderInput.intersectionInfo->position + newDirection * 0.0001f,
                            std::move(newDirection)});

    return {};
}