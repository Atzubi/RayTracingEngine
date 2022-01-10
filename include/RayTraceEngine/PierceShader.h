//
// Created by sebastian on 02.07.19.
//

#ifndef RAYTRACECORE_PIERCESHADER_H
#define RAYTRACECORE_PIERCESHADER_H

//#include "RayEngine.h"

/**
 * Default implementation of a pierce shader.
 */
class BasicPierceShader : public PierceShader{
public:
    BasicPierceShader(){

    }
    BasicPierceShader(const BasicPierceShader& copy){
        // TODO
    }

    Shader* clone() override{
        return new BasicPierceShader(*this);
    }

    ShaderOutput shade(uint64_t id, PipelineInfo *pipelineInfo, PierceShaderInput *shaderInput, ShaderResource *shaderResource,
                       RayResource **rayResource, RayGeneratorOutput *newRays) override{
        // TODO
        return ShaderOutput();
    }
};

#endif //RAYTRACECORE_PIERCESHADER_H
