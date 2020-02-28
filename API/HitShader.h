//
// Created by sebastian on 02.07.19.
//

#ifndef RAYTRACECORE_HITSHADER_H
#define RAYTRACECORE_HITSHADER_H

//#include "RayEngine.h"

class BasicHitShader : public HitShader{
public:
    ShaderOutput shade(int id, RayTracerOutput shaderInput, void *dataInput) {
        return ShaderOutput();
    }

    void *getAssociatedData() {
        return nullptr;
    }
};

#endif //RAYTRACECORE_HITSHADER_H
