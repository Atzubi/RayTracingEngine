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

    [[nodiscard]] std::unique_ptr<PierceShader> clone() const override{
        return std::make_unique<BasicPierceShader>(*this);
    }

    ShaderOutput shade(uint64_t id, PipelineInfo *pipelineInfo, PierceShaderInput *shaderInput, std::vector<ShaderResource *> *shaderResource,
                       RayResource **rayResource, RayGeneratorOutput *newRays) override{
        // TODO
        return {};
    }
};

#endif //RAYTRACECORE_PIERCESHADER_H
