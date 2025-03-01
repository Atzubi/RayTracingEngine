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

/**
 * Container for storing an image/texture.
 * w:               The horizontal resolution of the texture.
 * h:               The vertical resolution of the texture.
 * bytesPerTexel:   Amount of bytes per texel.
 * image:           Raw color values.
 */
struct Texture : public IShaderResource
{
    std::uint32_t             w;
    std::uint32_t             h;
    std::uint32_t             bytesPerTexel;
    std::vector<std::uint8_t> image;

    std::vector<std::uint8_t> Serialize() const override
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

    std::unique_ptr<IShaderResource> Deserialize(const std::span<const std::uint8_t> buffer) const override
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
