//
// Created by sebastian on 02.07.19.
//

#ifndef RAYTRACECORE_RAYGENERATORSHADER_H
#define RAYTRACECORE_RAYGENERATORSHADER_H

#include <cmath>
#include "RayEngine.h"

class BasicRayGeneratorShader : public RayGeneratorShader {
public:
    BasicRayGeneratorShader() {

    }

    BasicRayGeneratorShader(const BasicRayGeneratorShader &copy) {

    }

    Shader *clone() override {
        return new BasicRayGeneratorShader(*this);
    }

    RayGeneratorOutput shade(int id, PipelineInfo *pipelineInfo, void *dataInput) override {
        int x = (id % pipelineInfo->width) - (pipelineInfo->width) / 2;
        int y = -(id / pipelineInfo->height) + (pipelineInfo->height) / 2;

        RayGeneratorOutput rayGeneratorOutput;
        Vector3D rayOrigin;
        Vector3D rayDirection;

        Vector3D camRight;

        camRight = {pipelineInfo->cameraUp.y * pipelineInfo->cameraDirection.z -
                    pipelineInfo->cameraUp.z * pipelineInfo->cameraDirection.y,
                    0,
                    pipelineInfo->cameraUp.x * pipelineInfo->cameraDirection.y -
                    pipelineInfo->cameraUp.y * pipelineInfo->cameraDirection.x};

        double camRightLength = std::sqrt(
                camRight.x * camRight.x + camRight.y * camRight.y + camRight.z * camRight.z);

        camRight.x /= camRightLength;
        camRight.y /= camRightLength;
        camRight.z /= camRightLength;

        rayOrigin = pipelineInfo->cameraPosition;
        rayDirection.x =
                pipelineInfo->cameraDirection.x +
                (camRight.x * (x / (pipelineInfo->width + 0.0)) +
                 (pipelineInfo->cameraUp.x * (y / (pipelineInfo->height + 0.0))));
        rayDirection.y =
                pipelineInfo->cameraDirection.y +
                (camRight.y * (x / (pipelineInfo->width + 0.0)) +
                 (pipelineInfo->cameraUp.y * (y / (pipelineInfo->height + 0.0))));
        rayDirection.z =
                pipelineInfo->cameraDirection.z +
                (camRight.z * (x / (pipelineInfo->width + 0.0)) +
                 (pipelineInfo->cameraUp.z * (y / (pipelineInfo->height + 0.0))));

        double length = std::sqrt(rayDirection.x * rayDirection.x +
                                  rayDirection.y * rayDirection.y +
                                  rayDirection.z * rayDirection.z);

        rayDirection.x /= length;
        rayDirection.y /= length;
        rayDirection.z /= length;

        rayGeneratorOutput.rayOrigin.push_back(rayOrigin);
        rayGeneratorOutput.rayDirection.push_back(rayDirection);

        return rayGeneratorOutput;
    }

    void *getAssociatedData() {
        return nullptr;
    }
};


#endif //RAYTRACECORE_RAYGENERATORSHADER_H
