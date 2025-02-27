#pragma once

#include "RayTraceEngine/Shader.h"
#include "RayTraceEngine/Vector3D.h"

#include <vector>

struct GeneratorShaderResourceP
{
    const IRayGeneratorShader*    shader;
    std::vector<IShaderResource*> resources;
};

struct HitShaderResourceP
{
    const IHitShader*             shader;
    std::vector<IShaderResource*> resources;
};

struct PierceShaderResourceP
{
    const IPierceShader*          shader;
    std::vector<IShaderResource*> resources;
};

struct OcclusionShaderResourceP
{
    const IOcclusionShader*       shader;
    std::vector<IShaderResource*> resources;
};

struct MissShaderResourceP
{
    const IMissShader*            shader;
    std::vector<IShaderResource*> resources;
};

/**
 * Contains all the information needed that defines a pipeline.
 * PipelineImplement Model:
 *                                      OcclusionShader
 * RayGeneratorShader -> Ray Tracer ->  HitShader
 *                                      PierceShader
 *                                      MissShader
 */
class PipelineImplement
{
  public:
    static void Run(std::vector<Vector3D>&          buffer,
                    const std::uint32_t             width,
                    const std::uint32_t             height,
                    const IIntersectable*           scene,
                    const GeneratorShaderResourceP& generatorShaderPackage,
                    const HitShaderResourceP&       hitShaderPackage,
                    const PierceShaderResourceP&    pierceShaderPackage,
                    const OcclusionShaderResourceP& occlusionShaderPackage,
                    const MissShaderResourceP&      missShaderPackage);
};
