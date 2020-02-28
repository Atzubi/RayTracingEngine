//
// Created by sebastian on 02.07.19.
//

#ifndef RAYTRACECORE_OCCLUSIONSHADER_H
#define RAYTRACECORE_OCCLUSIONSHADER_H

#include "RayEngine.h"

class BasicOcclusionShader : public OcclusionShader{
public:
    BasicOcclusionShader(){

    }
    BasicOcclusionShader(const BasicOcclusionShader& copy){

    }

    Shader* clone(){
        return new BasicOcclusionShader(*this);
    }

    ShaderOutput shade(int id, RayTracerOutput shaderInput, void *dataInput) {
        return ShaderOutput();
    }

    void *getAssociatedData() {
        return nullptr;
    }
};

#endif //RAYTRACECORE_OCCLUSIONSHADER_H
