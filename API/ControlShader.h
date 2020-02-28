//
// Created by sebastian on 03.07.19.
//

#ifndef RAYTRACECORE_CONTROLSHADER_H
#define RAYTRACECORE_CONTROLSHADER_H

#include "RayEngine.h"


class BasicControlShader : public ControlShader{
public:
    BasicControlShader(){

    }
    BasicControlShader(const BasicControlShader& copy){

    }

    Shader* clone(){
        return new BasicControlShader(*this);
    }

    int shade(int id, ShaderOutput shaderInput, void *dataInput) {

    }

    void *getAssociatedData() {
        return nullptr;
    }
};

#endif //RAYTRACECORE_CONTROLSHADER_H
