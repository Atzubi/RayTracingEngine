//
// Created by sebastian on 02.07.19.
//

#ifndef RAYTRACECORE_OCCLUSIONSHADER_H
#define RAYTRACECORE_OCCLUSIONSHADER_H

#include "RayEngine.h"

class BasicOcclusionShader : public OcclusionShader{
public:
    ShaderOutput shade(int id, RayTracerOutput shaderInput, void *dataInput) override {
        return ShaderOutput();
    }

    void *getAssociatedData() override {
        return nullptr;
    }
};

#endif //RAYTRACECORE_OCCLUSIONSHADER_H
