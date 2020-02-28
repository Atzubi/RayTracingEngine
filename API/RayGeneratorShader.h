//
// Created by sebastian on 02.07.19.
//

#ifndef RAYTRACECORE_RAYGENERATORSHADER_H
#define RAYTRACECORE_RAYGENERATORSHADER_H

#include "RayEngine.h"

class BasicRayGeneratorShader : public RayGeneratorShader{
public:
    RayGeneratorOutput shade(int id, void *dataInput) {
        return RayGeneratorOutput();
    }

    void *getAssociatedData() {
        return nullptr;
    }
};


#endif //RAYTRACECORE_RAYGENERATORSHADER_H
