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
    int width, height;

public:
    Pipeline();
    ~Pipeline();

    void setResolution(int width, int height);
};

#endif //RAYTRACECORE_PIPELINE_H
