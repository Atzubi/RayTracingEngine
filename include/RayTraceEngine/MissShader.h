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
class BasicMissShader : public MissShader {
public:
    BasicMissShader() {

    }

    BasicMissShader(const BasicMissShader &copy) {
        // TODO
    }

    [[nodiscard]] std::unique_ptr<MissShader> clone() const override {
        return std::make_unique<BasicMissShader>(*this);
    }

    ShaderOutput shade(uint64_t id, const PipelineInfo &pipelineInfo, const MissShaderInput &shaderInput,
                       const std::vector<ShaderResource *> &shaderResource, RayResource *&rayResource,
                       RayGeneratorOutput &newRays) const override {
        // TODO
        return {};
    }
};

#endif //RAYTRACECORE_MISSSHADER_H
