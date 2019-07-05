//
// Created by sebastian on 02.07.19.
//

#ifndef RAYTRACECORE_PIERCESHADER_H
#define RAYTRACECORE_PIERCESHADER_H

#include "RayEngine.h"

class BasicPierceShader : public PierceShader{
public:
    ShaderOutput shade(int id, RayTracerOutput shaderInput, void *dataInput) override {
        return ShaderOutput();
    }

    void *getAssociatedData() override {
        return nullptr;
    }
};

#endif //RAYTRACECORE_PIERCESHADER_H
