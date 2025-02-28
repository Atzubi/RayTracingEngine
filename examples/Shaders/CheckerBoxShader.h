#pragma once

#include "Common.h"
#include "RayTraceEngine/Shader.h"

/**
 * Checker pattern sky box shader.
 */
ShaderOutput CheckerBoxShader(const std::uint64_t                  id,
                              const MissShaderInput&               shaderInput,
                              const std::vector<IShaderResource*>& shaderResource,
                              RayGeneratorOutput&                  newRays)
{
    const auto& v = shaderInput.rayDirection;

    // Convert to cube-map coordinates (based on largest component)
    const auto absX = std::fabs(v.x);
    const auto absY = std::fabs(v.y);
    const auto absZ = std::fabs(v.z);

    float uTex, vTex;
    if (absX >= absY && absX >= absZ)
    {
        uTex = v.y / absX;
        vTex = v.z / absX;
    }
    else if (absY >= absX && absY >= absZ)
    {
        uTex = v.x / absY;
        vTex = v.z / absY;
    }
    else
    {
        uTex = v.x / absZ;
        vTex = v.y / absZ;
    }
    const auto gridX = static_cast<std::uint8_t>((uTex + 1.0) * 20);
    const auto gridY = static_cast<std::uint8_t>((vTex + 1.0) * 20);

    const std::uint8_t color = ((gridX / 2) + (gridY / 2)) % 2;

    if (shaderResource.size() == 0)
        return {static_cast<const float>(color), static_cast<const float>(color), static_cast<const float>(color)};

    const auto& sampleCount = dynamic_cast<SampleCountInfo*>(shaderResource[0])->samplesPerPixel;
    const auto& accumulatedAbsorption =
        dynamic_cast<PathData*>(shaderResource[1])->absorption[id * sampleCount + shaderInput.id];

    return {std::max((color - accumulatedAbsorption.x), 0.f) / sampleCount,
            std::max((color - accumulatedAbsorption.y), 0.f) / sampleCount,
            std::max((color - accumulatedAbsorption.z), 0.f) / sampleCount};
}
