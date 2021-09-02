//
// Created by sebastian on 02.07.19.
//

#ifndef RAYTRACECORE_PIERCESHADER_H
#define RAYTRACECORE_PIERCESHADER_H

//#include "RayEngine.h"

class BasicPierceShader : public PierceShader{
public:
    BasicPierceShader(){

    }
    BasicPierceShader(const BasicPierceShader& copy){

    }

    Shader* clone() override{
        return new BasicPierceShader(*this);
    }

    ShaderOutput shade(uint64_t id, PipelineInfo *pipelineInfo, PierceShaderInput *shaderInput, void* dataInput, RayGeneratorOutput *newRays) override{
        return ShaderOutput();
    }

    void *getAssociatedData() {
        return nullptr;
    }
};

#endif //RAYTRACECORE_PIERCESHADER_H
