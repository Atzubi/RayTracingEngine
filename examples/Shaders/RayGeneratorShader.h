//
// Created by sebastian on 02.07.19.
//

#ifndef RAYTRACECORE_RAYGENERATORSHADER_H
#define RAYTRACECORE_RAYGENERATORSHADER_H

#include <cmath>
#include "RayEngine.h"

/**
 * Default implementation of a ray generator shader. It generates a view frustum given a camera position and resolution.
 */
class BasicRayGeneratorShader : public RayGeneratorShader {
public:
    BasicRayGeneratorShader() {

    }

    BasicRayGeneratorShader(const BasicRayGeneratorShader &copy) {

    }

    [[nodiscard]] std::unique_ptr<RayGeneratorShader> clone() const override {
        return std::make_unique<BasicRayGeneratorShader>(*this);
    }

    void shade(uint64_t id, const PipelineInfo &pipelineInfo, const std::vector<ShaderResource *> &shaderResource,
               RayGeneratorOutput &rayGeneratorOutput) const override {
        int64_t x = (((int64_t) id) % pipelineInfo.width) - (pipelineInfo.width) / 2;
        int64_t y = -(((int64_t) id) / pipelineInfo.height) + (pipelineInfo.height) / 2;

        Vector3D camRight = pipelineInfo.cameraUp.cross(pipelineInfo.cameraDirection);
        camRight.normalize();
        Vector3D rayDirection = pipelineInfo.cameraDirection + (camRight * (x / (pipelineInfo.width + 0.0)) +
                                                       (pipelineInfo.cameraUp * (y / (pipelineInfo.height + 0.0))));
        rayDirection.normalize();

        GeneratorRay generatorRay = {pipelineInfo.cameraPosition, rayDirection};
        rayGeneratorOutput.rays.push_back(generatorRay);
    }

    void *getAssociatedData() {
        return nullptr;
    }
};


#endif //RAYTRACECORE_RAYGENERATORSHADER_H
