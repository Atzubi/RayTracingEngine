//
// Created by sebastian on 02.07.19.
//

#ifndef RAYTRACECORE_OCCLUSIONSHADER_H
#define RAYTRACECORE_OCCLUSIONSHADER_H

#include "RayEngine.h"

/**
 * Default implementation of an occlusion shader.
 */
class BasicOcclusionShader : public OcclusionShader {
public:
    BasicOcclusionShader() {

    }

    BasicOcclusionShader(const BasicOcclusionShader &copy) {
        // TODO
    }

    [[nodiscard]] std::unique_ptr<OcclusionShader> clone() const override {
        return std::make_unique<BasicOcclusionShader>(*this);
    }

    ShaderOutput shade(uint64_t id, const PipelineInfo &pipelineInfo, const OcclusionShaderInput &shaderInput,
                       const std::vector<ShaderResource *> &shaderResource, RayResource *&rayResource,
                       RayGeneratorOutput &newRays) const override {
        // TODO
        return {};
    }
};

#endif //RAYTRACECORE_OCCLUSIONSHADER_H
