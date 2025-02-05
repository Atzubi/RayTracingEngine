#pragma once

#include "RayTraceEngine/BasicStructures.h"
#include "RayTraceEngine/Shader.h"
#include "bvh/DBVHv2.h"
#include <limits>
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
    static void Run(std::vector<unsigned char>&     buffer,
                    const TextureView&              texture,
                    const IIntersectable*           scene,
                    const GeneratorShaderResourceP& generatorShaderPackage,
                    const HitShaderResourceP&       hitShaderPackage,
                    const PierceShaderResourceP&    pierceShaderPackage,
                    const OcclusionShaderResourceP& occlusionShaderPackage,
                    const MissShaderResourceP&      missShaderPackage);

  private:
    struct RayContainer
    {
        int      rayID;
        Vector3D rayOrigin;
        Vector3D rayDirection;
    };

    static void SetPixel(std::vector<unsigned char>& buffer, int id, const ShaderOutput& pixel);

    static Ray InitRay(const std::vector<RayContainer>& rayContainers);

    static IntersectionInfo GetFirstIntersection(std::vector<IntersectionInfo>& infos);

    static void UpdateRayStack(std::vector<RayContainer>& rayContainers, int id, RayGeneratorOutput& newRays);

    static void GeneratePrimaryRays(const GeneratorShaderResourceP& generatorShaderPackage,
                                    std::vector<RayContainer>&      rayContainers,
                                    int                             rayID,
                                    RayGeneratorOutput&             rays);
};
