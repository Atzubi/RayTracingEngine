//
// Created by sebastian on 13.11.19.
//

#ifndef RAYTRACECORE_PIPELINE_H
#define RAYTRACECORE_PIPELINE_H

#include "API/Shader.h"
#include "API/Object.h"

/**
 * Contains all the information needed that defines a pipeline.
 * Pipeline Model:
 *                                      OcclusionShader
 * RayGeneratorShader -> Ray Tracer ->  HitShader       -> ControlShader
 *                                      PierceShader
 *                                      MissShader
 */
class Pipeline {
private:
    int width, height;
    Vector3D cameraPosition;
    Vector3D cameraDirection;
    Vector3D cameraUp;

    std::vector<RayGeneratorShader *> rayGeneratorShaders;
    std::vector<OcclusionShader *> occlusionShaders;
    std::vector<HitShader *> hitShaders;
    std::vector<PierceShader *> pierceShaders;
    std::vector<MissShader *> missShaders;

    Object *geometry;

public:
    Pipeline(int width, int height, Vector3D cameraPosition, Vector3D cameraDirection, Vector3D cameraUp);

    ~Pipeline();

    void setResolution(int width, int height);

    Object *getGeometryAsObject();
};

#endif //RAYTRACECORE_PIPELINE_H
