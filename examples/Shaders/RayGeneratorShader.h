#include "Common.h"
#include "RayTraceEngine/Shader.h"
#include <cmath>
#include <cstdint>

/**
 * Default implementation of a ray generator shader. It generates a view frustum given a camera position and resolution.
 */
class BasicRayGeneratorShader : public IRayGeneratorShader
{
  public:
    BasicRayGeneratorShader() {}

    BasicRayGeneratorShader(const BasicRayGeneratorShader& copy) {}

    [[nodiscard]] std::unique_ptr<IRayGeneratorShader> Clone() const override
    {
        return std::make_unique<BasicRayGeneratorShader>(*this);
    }

    void Shade(const std::uint64_t                  id,
               const std::vector<IShaderResource*>& shaderResource,
               RayGeneratorOutput&                  rayGeneratorOutput) const override
    {
        const auto* info = dynamic_cast<ViewportInfo*>(shaderResource[0]);
        const auto* cam  = dynamic_cast<CameraInfo*>(shaderResource[1]);
        const auto  x    = (((std::int64_t)id) % info->viewPortHeight) - (info->viewPortWidth) / 2;
        const auto  y    = -(((std::int64_t)id) / info->viewPortWidth) + (info->viewPortHeight) / 2;

        Vector3D camRight = cam->cameraUp.Cross(cam->cameraDirection);
        camRight.Normalize();
        Vector3D rayDirection = cam->cameraDirection + (camRight * (x / (info->viewPortWidth + 0.0)) +
                                                        (cam->cameraUp * (y / (info->viewPortHeight + 0.0))));
        rayDirection.Normalize();

        rayGeneratorOutput.rays.emplace_back(RayType::Closest, cam->cameraPosition, rayDirection);
    }
};
