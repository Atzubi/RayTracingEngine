#include "Common.h"
#include "RayTraceEngine/Shader.h"

/**
 * Default implementation of a hit shader. Shades light based on the Phong shading model. Considers textures.
 */
class BasicPhongHitShader : public IHitShader
{
  public:
    BasicPhongHitShader() {}

    BasicPhongHitShader(const BasicPhongHitShader& copy)
    {
        // TODO
    }

    [[nodiscard]] std::unique_ptr<IHitShader> Clone() const override
    {
        return std::make_unique<BasicPhongHitShader>(*this);
    }

    ShaderOutput Shade(const std::uint64_t                  id,
                       const HitShaderInput&                shaderInput,
                       const std::vector<IShaderResource*>& shaderResource,
                       RayGeneratorOutput&                  newRays) const override
    {
        Vector3D Ka = {1, 1, 1};
        Vector3D Kd = {1, 1, 1};
        Vector3D Ks = {1, 1, 1};
        Vector3D pix{};

        const auto* shaderInputInfo = shaderInput.intersectionInfo;

        const auto image = shaderInputInfo->material->map_Kd.image;

        double diffuse = 1, specular = 0.5, exponent = 8, ambient = 0.1;

        if (!image || image->empty())
        {
            if (shaderInputInfo->material->illum == 2)
            {
                exponent = shaderInputInfo->material->Ns;
                Ka       = shaderInputInfo->material->Ka;
                Kd       = shaderInputInfo->material->Kd;
                Ks       = shaderInputInfo->material->Ks;
            }
            pix = {255, 255, 255};
        }
        else
        {
            const auto w = shaderInputInfo->material->map_Kd.w;
            const auto h = shaderInputInfo->material->map_Kd.h;
            auto       x = fmod(shaderInputInfo->texture.x, 1.0);
            auto       y = -fmod(shaderInputInfo->texture.y, 1.0);
            if (x < 0)
                x = 1 + x;
            if (y < 0)
                y = 1 + y;

            const auto pixelCoordinate = ((std::uint64_t)((w - 1) * x)) + w * ((std::uint64_t)((h - 1) * y));

            pix[0] = (*image)[pixelCoordinate * 3];
            pix[1] = (*image)[pixelCoordinate * 3 + 1];
            pix[2] = (*image)[pixelCoordinate * 3 + 2];

            Ka = shaderInputInfo->material->Ka;
            Kd = shaderInputInfo->material->Kd;
            Ks = shaderInputInfo->material->Ks;
        }

        ShaderOutput shaderOutput{};

        if (shaderInputInfo->distance == std::numeric_limits<double_t>::max())
            return shaderOutput;

        Vector3D n{}, v{}, r{}, l{};
        double_t nl;

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

        const auto dot = fmax(v.x * r.x + v.y * r.y + v.z * r.z, 0);

        const auto color = (Ka * ambient + Kd * diffuse * nl + Ks * specular * pow(dot, exponent)) * pix;

        shaderOutput.color[0] = (uint8_t)fmin(color[0], 255);
        shaderOutput.color[1] = (uint8_t)fmin(color[1], 255);
        shaderOutput.color[2] = (uint8_t)fmin(color[2], 255);

        return shaderOutput;
    }
};
