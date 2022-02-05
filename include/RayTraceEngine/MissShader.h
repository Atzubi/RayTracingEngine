//
// Created by sebastian on 02.07.19.
//

#ifndef RAYTRACECORE_MISSSHADER_H
#define RAYTRACECORE_MISSSHADER_H

#include <iostream>
#include "RayEngine.h"

/**
 * Default implementation of a miss shader.
 */
class BasicMissShader : public MissShader{
public:
    BasicMissShader(){

    }
    BasicMissShader(const BasicMissShader& copy){
        // TODO
    }

    std::unique_ptr<MissShader> clone() override{
        return std::make_unique<BasicMissShader>(*this);
    }

    ShaderOutput shade(uint64_t id, PipelineInfo *pipelineInfo, MissShaderInput *shaderInput, std::vector<ShaderResource *> *shaderResource,
                       RayResource **rayResource, RayGeneratorOutput *newRays) override{
        // TODO
        return ShaderOutput();
    }
};

#endif //RAYTRACECORE_MISSSHADER_H
