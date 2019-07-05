//
// Created by sebastian on 03.07.19.
//

#ifndef RAYTRACECORE_CONTROLSHADER_H
#define RAYTRACECORE_CONTROLSHADER_H

#include "RayEngine.h"


class BasicControlShader : public ControlShader{
public:
    int shade(int id, ShaderOutput shaderInput, void *dataInput) override {

    }

    void *getAssociatedData() override {
        return nullptr;
    }
};

#endif //RAYTRACECORE_CONTROLSHADER_H
