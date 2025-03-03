#pragma once

#include "Scene.h"
#include "Shader.h"

#include <cstdint>
#include <vector>

/**
 * Groups shader with the resources that will be bound to it in the pipeline.
 */
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
 * Texture format specifies the byte stride per texel.
 * RGB -> 3 bytes per texel
 * RGBA -> 4 bytes per texel
 */
enum class TextureFormat
{
    RGB,
    RGBA
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

    std::vector<std::uint8_t>        Serialize() const override;
    std::unique_ptr<IShaderResource> Deserialize(const std::span<const std::uint8_t> buffer) const override;
};

/**
 * Contains all necesarry information required to create a render target in the engine.
 * width:    Resolution in horizontal direction.
 * height:   Resolution in vertical direction.
 */
struct RenderTargetDescription
{
    std::uint32_t width;
    std::uint32_t height;
};

/**
 * Resource handle. When this handle goes out of scope the resource is freed in the engine.
 */
class RenderTargetHandle
{
  public:
    /**
     * Utility method for extracting a texture from the internal representation of the render target.
     * @param format Texture format of the extracted texture, e.g RGB.
     * @return       Extracted texture.
     */
    virtual Texture GetAsTexture(TextureFormat format) const = 0;

    /**
     * Utility method for extracting a texture from the internal representation of the render target.
     * @param format    Texture format of the extracted texture, e.g RGB.
     * @param texture   Extracted texture, resized only if necessary, otherwise overwritten.
     */
    virtual void GetAsTexture(const TextureFormat format, Texture& texture) const = 0;

    virtual ~RenderTargetHandle() = default;
};

/**
 * Contains all necessary information for creating a pipeline in the engine.
 * scene:            Geometry of the scene for intersections.
 * generatorShader:  Shader with resources for generating rays that are intersected with the scene.
 * hitShader:        Shader with resources, called on closest intersection.
 * pierceShader:     Shader with resources, called with all intersection of a single ray.
 * occlusionShader:  Shader with resources, called when any intersection occurs.
 * missShader:       Shader with resources, called if there is no intersection.
 */
struct PipelineDescription
{
    SceneHandle*                   scene;
    GeneratorShaderResourcePackage generatorShader;
    HitShaderResourcePackage       hitShader;
    PierceShaderResourcePackage    pierceShader;
    OcclusionShaderResourcePackage occlusionShader;
    MissShaderResourcePackage      missShader;
};

/**
 * Resource handle. When this handle goes out of scope the resource is freed in the engine.
 */
class PipelineHandle
{
  public:
    /**
     * Executes the pipeline and writes the shading results into the render target. Note: All shader results per pixel
     * are accumulated.
     * @param: target    The render target.
     */
    virtual void Run(RenderTargetHandle& target) const = 0;

    virtual ~PipelineHandle() = default;
};
