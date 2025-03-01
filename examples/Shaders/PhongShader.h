#include "Common.h"
#include "RayTraceEngine/Shader.h"

/**
 * Shades light based on the Phong shading model.
 */
ShaderOutput BasicPhongHitShader(const std::uint64_t                     id,
                                 const HitShaderInput&                   shaderInput,
                                 const std::span<IShaderResource* const> shaderResource,
                                 RayGeneratorOutput&                     newRays)
{
    const auto* shaderInputInfo = shaderInput.intersectionInfo;
    const auto* material        = dynamic_cast<Material*>(shaderResource[1]);

    const auto exponent = material->Ns;
    const auto Kd       = LoadKdTexel(*shaderInputInfo, *material);
    const auto Ka       = material->Ka * Kd;
    const auto Ks       = material->Ks;

    Vector3D n{}, v{}, r{}, l{};
    float    nl;

    l.x = 1;
    l.y = -2;
    l.z = -1;
    l *= -1;

    n = shaderInputInfo->normal;

    v = (shaderInputInfo->position - dynamic_cast<CameraInfo*>(shaderResource[0])->cameraPosition) * -1;
    l.Normalize();
    n.Normalize();
    v.Normalize();

    nl = fmax(n.x * l.x + n.y * l.y + n.z * l.z, 0);

    r = (n * 2 * nl) - l;

    r.Normalize();

    const auto dot = std::max(v.x * r.x + v.y * r.y + v.z * r.z, 0.f);

    const auto color = (Ka + Kd * nl + Ks * pow(dot, exponent));

    ShaderOutput shaderOutput{};
    shaderOutput.color[0] = std::min(color[0], 1.f);
    shaderOutput.color[1] = std::min(color[1], 1.f);
    shaderOutput.color[2] = std::min(color[2], 1.f);

    return shaderOutput;
}
