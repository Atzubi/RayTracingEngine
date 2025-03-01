#include "Common.h"
#include "RayTraceEngine/Shader.h"
#include <cmath>
#include <cstdint>

/**
 * Perspective ray generator shader. It generates a view frustum given a camera position and resolution. Multiple
 * samples per pixel are supported.
 */
void PerspectiveGeneratorShader(const std::uint64_t                     id,
                                const std::span<IShaderResource* const> shaderResource,
                                RayGeneratorOutput&                     rayGeneratorOutput)
{
    const auto* info    = dynamic_cast<ViewportInfo*>(shaderResource[0]);
    const auto* cam     = dynamic_cast<CameraInfo*>(shaderResource[1]);
    const auto* samples = dynamic_cast<SampleCountInfo*>(shaderResource[2]);
    const auto  x       = (((std::int64_t)id) % info->viewPortHeight) - (info->viewPortWidth) / 2;
    const auto  y       = -(((std::int64_t)id) / info->viewPortWidth) + (info->viewPortHeight) / 2;

    Vector3D camRight = cam->cameraUp.Cross(cam->cameraDirection);
    camRight.Normalize();
    Vector3D rayDirection = cam->cameraDirection + (camRight * (x / (info->viewPortWidth + 0.0)) +
                                                    (cam->cameraUp * (y / (info->viewPortHeight + 0.0))));
    rayDirection.Normalize();

    for (int i = 0; i < samples->samplesPerPixel; ++i)
    {
        rayGeneratorOutput.rays.emplace_back(RayType::Closest, i, cam->cameraPosition, rayDirection);
    }
}
