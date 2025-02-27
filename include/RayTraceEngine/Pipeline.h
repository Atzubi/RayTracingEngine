#pragma once

#include "BasicStructures.h"
#include "Scene.h"
#include "Shader.h"

#include <cstdint>
#include <vector>

struct GeneratorShaderResourcePackage
{
    GeneratorShaderHandle*             shader;
    std::vector<ShaderResourceHandle*> resources;
};

struct HitShaderResourcePackage
{
    HitShaderHandle*                   shader;
    std::vector<ShaderResourceHandle*> resources;
};

struct PierceShaderResourcePackage
{
    PierceShaderHandle*                shader;
    std::vector<ShaderResourceHandle*> resources;
};

struct OcclusionShaderResourcePackage
{
    OcclusionShaderHandle*             shader;
    std::vector<ShaderResourceHandle*> resources;
};

struct MissShaderResourcePackage
{
    MissShaderHandle*                  shader;
    std::vector<ShaderResourceHandle*> resources;
};

struct RenderTargetDescription
{
    std::uint32_t width;
    std::uint32_t height;
};

class RenderTargetHandle
{
  public:
    virtual Texture GetAsTexture(TextureFormat format) const                         = 0;
    virtual void    GetAsTexture(const TextureFormat format, Texture& texture) const = 0;

    virtual ~RenderTargetHandle() = default;
};

struct PipelineDescription
{
    SceneHandle*                   scene;
    GeneratorShaderResourcePackage generatorShader;
    HitShaderResourcePackage       hitShader;
    PierceShaderResourcePackage    pierceShader;
    OcclusionShaderResourcePackage occlusionShader;
    MissShaderResourcePackage      missShader;
};

class PipelineHandle
{
  public:
    virtual void Run(RenderTargetHandle& target) const = 0;

    virtual ~PipelineHandle() = default;
};
