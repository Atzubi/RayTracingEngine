//
// Created by sebastian on 02.07.19.
//

#ifndef RAYTRACECORE_HITSHADER_H
#define RAYTRACECORE_HITSHADER_H

//#include "RayEngine.h"

/**
 * Default implementation of a hit shader. Shades light based on the Phong shading model. Considers textures.
 */
class BasicHitShader : public HitShader {
public:
    BasicHitShader() {

    }

    BasicHitShader(const BasicHitShader &copy) {
        // TODO
    }

    [[nodiscard]] std::unique_ptr<HitShader> clone() const override {
        return std::make_unique<BasicHitShader>(*this);
    }

    ShaderOutput shade(uint64_t id, const PipelineInfo &pipelineInfo, const HitShaderInput &shaderInput,
                       const std::vector<ShaderResource *> &shaderResource,
                       RayResource *&rayResource, RayGeneratorOutput &newRays) const override {
        Vector3D Ka = {1, 1, 1};
        Vector3D Kd = {1, 1, 1};
        Vector3D Ks = {1, 1, 1};
        Vector3D pix{};

        auto shaderInputInfo = shaderInput.intersectionInfo;

        auto image = shaderInputInfo->material->map_Kd.image;

        double_t diffuse = 1, specular = 0.5, exponent = 8, ambient = 0.1;

        if (image.empty()) {
            if (shaderInputInfo->material->illum == 2) {
                exponent = shaderInputInfo->material->Ns;
                Ka = shaderInputInfo->material->Ka;
                Kd = shaderInputInfo->material->Kd;
                Ks = shaderInputInfo->material->Ks;
            }
            pix = {255, 255, 255};
        } else {
            int w = shaderInputInfo->material->map_Kd.w;
            int h = shaderInputInfo->material->map_Kd.h;
            double x = fmod(shaderInputInfo->texture.x, 1.0);
            double y = -fmod(shaderInputInfo->texture.y, 1.0);
            if (x < 0) x = 1 + x;
            if (y < 0) y = 1 + y;

            uint64_t pixelCoordinate = ((uint64_t) ((w - 1) * x)) + w * ((uint64_t) ((h - 1) * y));

            pix[0] = image[pixelCoordinate * 3];
            pix[1] = image[pixelCoordinate * 3 + 1];
            pix[2] = image[pixelCoordinate * 3 + 2];

            Ka = shaderInputInfo->material->Ka;
            Kd = shaderInputInfo->material->Kd;
            Ks = shaderInputInfo->material->Ks;
        }

        ShaderOutput shaderOutput{};

        if (shaderInputInfo->distance == std::numeric_limits<double_t>::max()) return shaderOutput;

        Vector3D n{}, v{}, r{}, l{};
        double_t nl;

        l.x = -0.707107;
        l.y = 0.707107;
        l.z = 0;

        n = shaderInputInfo->normal;

        v = (shaderInputInfo->position - pipelineInfo.cameraPosition) * -1;
        l.normalize();
        n.normalize();
        v.normalize();

        nl = fmax(n.x * l.x + n.y * l.y + n.z * l.z, 0);

        r = (n * 2 * nl) - l;

        r.normalize();

        double_t dot = fmax(v.x * r.x + v.y * r.y + v.z * r.z, 0);

        Vector3D color = (Ka * ambient + Kd * diffuse * nl + Ks * specular * pow(dot, exponent)) * pix;

        shaderOutput.color[0] = (uint8_t) fmin(color[0], 255);
        shaderOutput.color[1] = (uint8_t) fmin(color[1], 255);
        shaderOutput.color[2] = (uint8_t) fmin(color[2], 255);

        return shaderOutput;
    }
};

#endif //RAYTRACECORE_HITSHADER_H
