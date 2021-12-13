//
// Created by sebastian on 13.11.19.
//

#ifndef RAYTRACECORE_PIPELINEIMPLEMENT_H
#define RAYTRACECORE_PIPELINEIMPLEMENT_H

#include <limits>

#include "RayTraceEngine/Object.h"
#include "RayTraceEngine/Pipeline.h"
#include "RayTraceEngine/Shader.h"
#include "Acceleration Structures/DBVHv2.h"


/**
 * Contains all the information needed that defines a pipeline.
 * PipelineImplement Model:
 *                                      OcclusionShader
 * RayGeneratorShader -> Ray Tracer ->  HitShader
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

    DBVHNode *geometry;

    Texture result;

public:
    PipelineImplement(int width, int height, Vector3D *cameraPosition, Vector3D *cameraDirection,
                      Vector3D *cameraUp, std::vector<RayGeneratorShader *> *rayGeneratorShaders,
                      std::vector<OcclusionShader *> *occlusionShaders,
                      std::vector<HitShader *> *hitShaders, std::vector<PierceShader *> *pierceShaders,
                      std::vector<MissShader *> *missShaders, DBVHNode *geometry);

    ~PipelineImplement();

    int run();

    Texture getResult();

    void setResolution(int width, int height);

    void setCamera(Vector3D pos, Vector3D dir, Vector3D up);

    DBVHNode *getGeometry();

    Object *getGeometryAsObject();
};

#endif //RAYTRACECORE_PIPELINEIMPLEMENT_H
