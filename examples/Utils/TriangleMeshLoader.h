#pragma once

#include "Intersectables/TriangleMeshObject.h"
#include "OBJ_Loader.h"
#include "RayTraceEngine/BasicStructures.h"
#include "RayTraceEngine/RayEngine.h"

#define STB_IMAGE_IMPLEMENTATION
#include "Utils/Stb/stb_image.h"

#include <cmath>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <vector>

struct MeshHandles
{
    std::vector<std::unique_ptr<IntersectableObjectHandle>> intersectables;
    std::vector<std::unique_ptr<ShaderResourceHandle>>      renderTargets;
};

struct TextureWrapper : public IShaderResource
{
  public:
    Texture texture;

    std::unique_ptr<IShaderResource> Clone() const override { return std::make_unique<TextureWrapper>(*this); };
};

std::unique_ptr<ShaderResourceHandle>
LoadTexture(const std::filesystem::path& path, RayEngine& rayEngine, Material& material)
{
    constexpr std::uint32_t channelCount = STBI_rgb; // 3
    int                     comp;
    int                     width;
    int                     height;
    if (const auto texture = stbi_load(path.string().c_str(), &width, &height, &comp, STBI_rgb))
    {
        TextureWrapper textureWrapper;
        textureWrapper.texture.name          = path.filename().string();
        textureWrapper.texture.bytesPerTexel = channelCount;
        textureWrapper.texture.w             = width;
        textureWrapper.texture.h             = height;
        textureWrapper.texture.image.resize(channelCount * width * height);
        auto tex        = rayEngine.CreateShaderResource({&textureWrapper});
        material.map_Kd = &tex->Map<TextureWrapper>().texture;
        std::memcpy(material.map_Kd->image.data(), texture, channelCount * material.map_Kd->w * material.map_Kd->h);
        return tex;
    }
    return {};
}

MeshHandles LoadTriangleMeshFromObj(const std::filesystem::path& path, RayEngine& rayEngine)
{
    objl::Loader loader;
    if (!loader.LoadFile(path.string()))
        return {};

    MeshHandles handles;

    for (auto& m : loader.LoadedMeshes)
    {
        Material material{};

        // Fill the material description
        material.name  = m.MeshMaterial.name;
        material.Ka    = {m.MeshMaterial.Ka.X, m.MeshMaterial.Ka.Y, m.MeshMaterial.Ka.Z};
        material.Kd    = {m.MeshMaterial.Kd.X, m.MeshMaterial.Kd.Y, m.MeshMaterial.Kd.Z};
        material.Ks    = {m.MeshMaterial.Ks.X, m.MeshMaterial.Ks.Y, m.MeshMaterial.Ks.Z};
        material.Ns    = m.MeshMaterial.Ns;
        material.Ni    = m.MeshMaterial.Ni;
        material.d     = m.MeshMaterial.d;
        material.illum = m.MeshMaterial.illum;
        handles.renderTargets.push_back(
            LoadTexture(path.parent_path().append(m.MeshMaterial.map_Ka), rayEngine, material));
        handles.renderTargets.push_back(
            LoadTexture(path.parent_path().append(m.MeshMaterial.map_Kd), rayEngine, material));
        handles.renderTargets.push_back(
            LoadTexture(path.parent_path().append(m.MeshMaterial.map_Ks), rayEngine, material));
        handles.renderTargets.push_back(
            LoadTexture(path.parent_path().append(m.MeshMaterial.map_Ns), rayEngine, material));
        handles.renderTargets.push_back(
            LoadTexture(path.parent_path().append(m.MeshMaterial.map_d), rayEngine, material));
        handles.renderTargets.push_back(
            LoadTexture(path.parent_path().append(m.MeshMaterial.map_bump), rayEngine, material));

        std::vector<TriangleMeshObject::Vertex> vertices(m.Vertices.size());
        std::memcpy(vertices.data(), m.Vertices.data(), sizeof(TriangleMeshObject::Vertex) * vertices.size());

        // Create a triangle mesh object
        TriangleMeshObject triangleMeshObject(std::move(vertices), std::move(m.Indices), std::move(material));
        handles.intersectables.push_back(rayEngine.CreateIntersectableObject({triangleMeshObject}));
    }
    return handles;
}