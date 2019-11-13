//
// Created by sebastian on 13.11.19.
//

#ifndef RAYTRACECORE_PIPELINE_H
#define RAYTRACECORE_PIPELINE_H

#include "Shader.h"

/**
 * Contains all the information needed that defines a pipeline.
 * Pipeline Model:
 *                                      OcclusionShader
 * RayGeneratorShader -> Ray Tracer ->  HitShader       -> ControlShader
 *                                      PierceShader
 *                                      MissShader
 */
class Pipeline{
private:
    RayGeneratorShader* rayGeneratorShader;
    OcclusionShader* occlusionShader;
    PierceShader* pierceShader;
    HitShader* hitShader;
    MissShader* missShader;
    ControlShader* controlShader;
    int width, height;

public:
    Pipeline();
    ~Pipeline();

    void setResolution(int width, int height);

    bool addRayGeneratorShader(RayGeneratorShader *rayGeneratorShader);
    bool addOcclusionShader(OcclusionShader *occlusionShader);
    bool addPierceShader(PierceShader *pierceShader);
    bool addHitShader(HitShader *hitShader);
    bool addMissShader(MissShader *missShader);
    bool addControlShader(ControlShader *controlShader);
};

#endif //RAYTRACECORE_PIPELINE_H
