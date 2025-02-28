#pragma once

#include "Common.h"
#include "RayTraceEngine/Shader.h"

ShaderOutput PathTraceShader(const std::uint64_t                  id,
                             const HitShaderInput&                shaderInput,
                             const std::vector<IShaderResource*>& shaderResource,
                             RayGeneratorOutput&                  newRays)
{
    const auto  absorption       = Vector3D{1.f, 1.f, 1.f} - LoadKdTexel(*shaderInput.intersectionInfo);
    const auto& opticalDensity   = shaderInput.intersectionInfo->material->Ni;
    const auto& dissolve         = shaderInput.intersectionInfo->material->d;
    const auto& volumeAbsorption = shaderInput.intersectionInfo->material->Ns;
    const auto& sampleCount      = dynamic_cast<SampleCountInfo*>(shaderResource[0])->samplesPerPixel;
    auto&       accumulatedAbsorption =
        dynamic_cast<PathData*>(shaderResource[1])->absorption[id * sampleCount + shaderInput.id];
    auto& depth = dynamic_cast<PathData*>(shaderResource[1])->depth[id * sampleCount + shaderInput.id];

    if (depth > 1024)
        return {1, 0, 0};

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

    if (accumulatedAbsorption.Sum() == 3.f)
        return {};

    const auto cosTheta           = fmax(-incident.Dot(normal), 0.0f);
    const auto n1                 = backface ? opticalDensity : 1.f;
    const auto n2                 = backface ? 1.f : opticalDensity;
    const auto fresnelReflectance = FresnelSchlick(cosTheta, n1, n2);

    Vector3D newDirection;
    if (static_cast<float>(std::rand()) / RAND_MAX < fresnelReflectance)
    {
        newDirection = Reflect(incident, normal);
    }
    else
    {
        newDirection = Refract(incident, normal, n1, n2);
    }
    newDirection.Normalize();
    // Blend between reflection and Lambertian scatter
    const auto lambertianVector = LambertReflection(normal);
    newDirection                = newDirection * (1.f - dissolve) + (lambertianVector * dissolve);
    newDirection.Normalize();
    newRays.rays.push_back({RayType::Closest,
                            shaderInput.id,
                            shaderInput.intersectionInfo->position + newDirection * 0.0001f,
                            std::move(newDirection)});

    return {};
}