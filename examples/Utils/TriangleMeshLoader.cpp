#include "TriangleMeshLoader.h"
#include "Intersectables/TriangleMeshObject.h"
#include "OBJ_Loader.h"
#include "Shaders/Common.h"

#define STB_IMAGE_IMPLEMENTATION
#include "Utils/Stb/stb_image.h"

#include <cstdint>
#include <cstring>

Texture LoadTexture(const std::filesystem::path& path)
{
    constexpr std::uint32_t channelCount = STBI_rgb; // 3
    int                     comp;
    int                     width;
    int                     height;
    if (const auto texture = stbi_load(path.string().c_str(), &width, &height, &comp, STBI_rgb))
    {
        Texture tex{};
        tex.bytesPerTexel = channelCount;
        tex.w             = width;
        tex.h             = height;
        tex.image.resize(channelCount * width * height);
        std::memcpy(tex.image.data(), texture, tex.image.size());
        return tex;
    }
    return {};
}

std::vector<MeshHandles> LoadTriangleMeshFromObj(const std::filesystem::path& path, RayEngine& rayEngine)
{
    objl::Loader loader;
    if (!loader.LoadFile(path.string()))
        return {};

    std::vector<MeshHandles> handles;

    for (auto& m : loader.LoadedMeshes)
    {
        Material material{};

        // Fill the material description
        material.Ka       = {m.MeshMaterial.Ka.X, m.MeshMaterial.Ka.Y, m.MeshMaterial.Ka.Z};
        material.Kd       = {m.MeshMaterial.Kd.X, m.MeshMaterial.Kd.Y, m.MeshMaterial.Kd.Z};
        material.Ks       = {m.MeshMaterial.Ks.X, m.MeshMaterial.Ks.Y, m.MeshMaterial.Ks.Z};
        material.Ns       = m.MeshMaterial.Ns;
        material.Ni       = m.MeshMaterial.Ni;
        material.d        = m.MeshMaterial.d;
        material.illum    = m.MeshMaterial.illum;
        material.map_Ka   = LoadTexture(path.parent_path().append(m.MeshMaterial.map_Ka));
        material.map_Kd   = LoadTexture(path.parent_path().append(m.MeshMaterial.map_Kd));
        material.map_Ks   = LoadTexture(path.parent_path().append(m.MeshMaterial.map_Ks));
        material.map_Ns   = LoadTexture(path.parent_path().append(m.MeshMaterial.map_Ns));
        material.map_d    = LoadTexture(path.parent_path().append(m.MeshMaterial.map_d));
        material.map_bump = LoadTexture(path.parent_path().append(m.MeshMaterial.map_bump));

        MeshHandles meshHandles{};
        meshHandles.material = rayEngine.CreateShaderResource({&material});

        std::vector<TriangleMeshObject::Vertex> vertices(m.Vertices.size());
        std::memcpy(vertices.data(), m.Vertices.data(), sizeof(TriangleMeshObject::Vertex) * vertices.size());

        // Create a triangle mesh object
        TriangleMeshObject triangleMeshObject(std::move(vertices), std::move(m.Indices));
        meshHandles.intersectable = rayEngine.CreateIntersectableObject({triangleMeshObject});

        handles.push_back(std::move(meshHandles));
    }
    return handles;
}