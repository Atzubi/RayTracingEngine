#include "RayTraceEngine/Shader.h"
#include <cmath>

struct ViewportInfo : public IShaderResource
{
    std::uint32_t viewPortWidth;
    std::uint32_t viewPortHeight;

    Vector3D cameraPosition;
    Vector3D cameraUp;
    Vector3D cameraDirection;

    std::unique_ptr<IShaderResource> Clone() const override
    {
        auto clone             = std::make_unique<ViewportInfo>();
        clone->viewPortWidth   = viewPortWidth;
        clone->viewPortHeight  = viewPortHeight;
        clone->cameraPosition  = cameraPosition;
        clone->cameraUp        = cameraUp;
        clone->cameraDirection = cameraDirection;
        return clone;
    }
};

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

    void Shade(uint64_t                             id,
               const std::vector<IShaderResource*>& shaderResource,
               RayGeneratorOutput&                  rayGeneratorOutput) const override
    {
        ViewportInfo* info = dynamic_cast<ViewportInfo*>(shaderResource[0]);
        int64_t       x    = (((int64_t)id) % info->viewPortHeight) - (info->viewPortWidth) / 2;
        int64_t       y    = -(((int64_t)id) / info->viewPortWidth) + (info->viewPortHeight) / 2;

        Vector3D camRight = info->cameraUp.Cross(info->cameraDirection);
        camRight.Normalize();
        Vector3D rayDirection = info->cameraDirection + (camRight * (x / (info->viewPortWidth + 0.0)) +
                                                         (info->cameraUp * (y / (info->viewPortHeight + 0.0))));
        rayDirection.Normalize();

        GeneratorRay generatorRay = {info->cameraPosition, rayDirection};
        rayGeneratorOutput.rays.push_back(generatorRay);
    }
};
