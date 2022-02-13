//
// Created by Sebastian on 13.02.2022.
//

#ifndef RAYTRACEENGINE_PIPELINEPOOL_H
#define RAYTRACEENGINE_PIPELINEPOOL_H

#include <unordered_map>
#include <memory>
#include "utility/Id.h"
#include "pipeline/PipelineImplement.h"

class PipelinePool{
private:
    std::unordered_map<PipelineId, std::unique_ptr<PipelineImplement>> pipelines;
    std::unordered_map<DBVHNode *, DBVHNode *> pipelineCache;

    std::unordered_map<HitShaderId, std::unique_ptr<HitShader>> hitShaders;
    std::unordered_map<MissShaderId, std::unique_ptr<MissShader>> missShaders;
    std::unordered_map<OcclusionShaderId, std::unique_ptr<OcclusionShader>> occlusionShaders;
    std::unordered_map<PierceShaderId, std::unique_ptr<PierceShader>> pierceShaders;
    std::unordered_map<RayGeneratorShaderId, std::unique_ptr<RayGeneratorShader>> rayGeneratorShaders;

    std::unordered_map<ShaderResourceId, std::unique_ptr<ShaderResource>> shaderResources;

public:
    PipelinePool();

    ~PipelinePool();

    void storePipelineFragments(std::unique_ptr<PipelineImplement> pipeline, PipelineId id);

    bool deletePipelineFragment(PipelineId id);

    PipelineImplement *getPipelineFragment(PipelineId id);

    void addShader(RayGeneratorShaderId id, std::unique_ptr<RayGeneratorShader> shader);

    void addShader(HitShaderId id, std::unique_ptr<HitShader> shader);

    void addShader(OcclusionShaderId id, std::unique_ptr<OcclusionShader> shader);

    void addShader(PierceShaderId id, std::unique_ptr<PierceShader> shader);

    void addShader(MissShaderId id, std::unique_ptr<MissShader> shader);

    RayGeneratorShader *getShader(RayGeneratorShaderId id);

    HitShader *getShader(HitShaderId id);

    OcclusionShader *getShader(OcclusionShaderId id);

    PierceShader *getShader(PierceShaderId id);

    MissShader *getShader(MissShaderId id);

    bool deleteShader(RayGeneratorShaderId id);

    bool deleteShader(HitShaderId id);

    bool deleteShader(OcclusionShaderId id);

    bool deleteShader(PierceShaderId id);

    bool deleteShader(MissShaderId id);

    void storeShaderResource(std::unique_ptr<ShaderResource> shaderResource, ShaderResourceId id);

    bool deleteShaderResource(ShaderResourceId id);

    ShaderResource *getShaderResource(ShaderResourceId id);

    void runPipeline(PipelineId id);

    void runPipelines();
};

#endif //RAYTRACEENGINE_PIPELINEPOOL_H