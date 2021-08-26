//
// Created by sebastian on 02.07.19.
//

#ifndef RAYTRACECORE_MISSSHADER_H
#define RAYTRACECORE_MISSSHADER_H

#include <iostream>
#include "RayEngine.h"

class BasicMissShader : public MissShader{
public:
    BasicMissShader(){

    }
    BasicMissShader(const BasicMissShader& copy){

    }

    Shader* clone() override{
        return new BasicMissShader(*this);
    }

    ShaderOutput shade(int id, PipelineInfo *pipelineInfo, IntersectionInfo *shaderInput, void* dataInput, RayGeneratorOutput *newRays) override{
        return ShaderOutput();
    }

    void *getAssociatedData() {
        return nullptr;
    }
};

#endif //RAYTRACECORE_MISSSHADER_H
