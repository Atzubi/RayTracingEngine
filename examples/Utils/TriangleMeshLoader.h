#pragma once

#include "RayTraceEngine/Intersectable.h"
#include "RayTraceEngine/RayEngine.h"
#include "RayTraceEngine/Shader.h"

#include <filesystem>
#include <vector>

struct MeshHandles
{
    std::unique_ptr<IntersectableObjectHandle> intersectable;
    std::unique_ptr<ShaderResourceHandle>      material;
};

Texture LoadTexture(const std::filesystem::path& path);

std::vector<MeshHandles> LoadTriangleMeshFromObj(const std::filesystem::path& path, RayEngine& rayEngine);