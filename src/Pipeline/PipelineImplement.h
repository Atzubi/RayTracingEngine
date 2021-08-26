//
// Created by sebastian on 13.11.19.
//

#ifndef RAYTRACECORE_PIPELINEIMPLEMENT_H
#define RAYTRACECORE_PIPELINEIMPLEMENT_H

#include "API/Pipeline.h"
#include "API/Shader.h"
#include "API/Object.h"
#include "limits"

/**
 * Contains all the information needed that defines a pipeline.
 * PipelineImplement Model:
 *                                      OcclusionShader
 * RayGeneratorShader -> Ray Tracer ->  HitShader       -> ControlShader
 *                                      PierceShader
 *                                      MissShader
 */
class PipelineImplement {
private:
    PipelineInfo pipelineInfo;

    std::vector<RayGeneratorShader *> rayGeneratorShaders;
    std::vector<OcclusionShader *> occlusionShaders;
    std::vector<HitShader *> hitShaders;
    std::vector<PierceShader *> pierceShaders;
    std::vector<MissShader *> missShaders;

    Object *geometry;

    Texture result;

public:
    PipelineImplement(int width, int height, Vector3D *cameraPosition, Vector3D *cameraDirection,
                      Vector3D *cameraUp, std::vector<RayGeneratorShader *> *rayGeneratorShaders,
                      std::vector<OcclusionShader *> *occlusionShaders,
                      std::vector<HitShader *> *hitShaders, std::vector<PierceShader *> *pierceShaders,
                      std::vector<MissShader *> *missShaders, Object *geometry);

    ~PipelineImplement();

    int run();

    Texture getResult();

    void setResolution(int width, int height);

    void setCamera(Vector3D pos, Vector3D dir, Vector3D up);

    Object *getGeometryAsObject();
};

#endif //RAYTRACECORE_PIPELINEIMPLEMENT_H
