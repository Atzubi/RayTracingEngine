#pragma once

#include "Intersectables/TriangleMeshObject.h"
#include "OBJ_Loader.h"
#include "RayTraceEngine/BasicStructures.h"
#include "RayTraceEngine/RayEngine.h"

#define STB_IMAGE_IMPLEMENTATION
#include "Utils/Stb/stb_image.h"

#include <cstdint>
#include <filesystem>
#include <vector>

struct MeshHandles
{
    std::vector<std::unique_ptr<IntersectableObjectHandle>> intersectables;
    std::vector<std::unique_ptr<RenderTargetHandle>>        renderTargets;
};

std::unique_ptr<RenderTargetHandle>
LoadTexture(const std::filesystem::path& path, RayEngine& rayEngine, Material& material)
{
    constexpr std::uint32_t channelCount = STBI_rgb; // 3
    int                     comp;
    if (const auto texture = stbi_load(path.string().c_str(),
                                       reinterpret_cast<int*>(&material.map_Kd.w),
                                       reinterpret_cast<int*>(&material.map_Kd.h),
                                       &comp,
                                       STBI_rgb))
    {
        auto tex        = rayEngine.CreateRenderTarget({material.map_Kd.w, material.map_Kd.h, channelCount});
        material.map_Kd = tex->GetAsTexture();
        std::memcpy(material.map_Kd.image->data(), texture, channelCount * material.map_Kd.w * material.map_Kd.h);
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
        material.name        = m.MeshMaterial.name;
        material.Ka          = {m.MeshMaterial.Ka.X, m.MeshMaterial.Ka.Y, m.MeshMaterial.Ka.Z};
        material.Kd          = {m.MeshMaterial.Kd.X, m.MeshMaterial.Kd.Y, m.MeshMaterial.Kd.Z};
        material.Ks          = {m.MeshMaterial.Ks.X, m.MeshMaterial.Ks.Y, m.MeshMaterial.Ks.Z};
        material.Ns          = m.MeshMaterial.Ns;
        material.Ni          = m.MeshMaterial.Ni;
        material.d           = m.MeshMaterial.d;
        material.illum       = m.MeshMaterial.illum;
        material.map_Ka.name = m.MeshMaterial.map_Ka;
        handles.renderTargets.push_back(
            LoadTexture(path.parent_path().append(material.map_Ka.name), rayEngine, material));
        material.map_Kd.name = m.MeshMaterial.map_Kd;
        handles.renderTargets.push_back(
            LoadTexture(path.parent_path().append(material.map_Kd.name), rayEngine, material));
        material.map_Ks.name = m.MeshMaterial.map_Ks;
        handles.renderTargets.push_back(
            LoadTexture(path.parent_path().append(material.map_Ks.name), rayEngine, material));
        material.map_Ns.name = m.MeshMaterial.map_Ns;
        handles.renderTargets.push_back(
            LoadTexture(path.parent_path().append(material.map_Ns.name), rayEngine, material));
        material.map_d.name = m.MeshMaterial.map_d;
        handles.renderTargets.push_back(
            LoadTexture(path.parent_path().append(material.map_d.name), rayEngine, material));
        material.map_bump.name = m.MeshMaterial.map_bump;
        handles.renderTargets.push_back(
            LoadTexture(path.parent_path().append(material.map_bump.name), rayEngine, material));

        std::vector<TriangleMeshObject::Vertex> vertices(m.Vertices.size());
        std::memcpy(vertices.data(), m.Vertices.data(), sizeof(TriangleMeshObject::Vertex) * vertices.size());

        // Create a triangle mesh object
        TriangleMeshObject triangleMeshObject(std::move(vertices), std::move(m.Indices), std::move(material));
        handles.intersectables.push_back(rayEngine.CreateIntersectableObject({triangleMeshObject}));
    }
    return handles;
}