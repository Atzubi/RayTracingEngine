//
// Created by Sebastian on 13.02.2022.
//

#include "pipeline/PipelinePool.h"

PipelinePool::PipelinePool() = default;

PipelinePool::~PipelinePool() = default;

void PipelinePool::storePipelineFragments(std::unique_ptr<PipelineImplement> pipeline, PipelineId id) {
    pipelines[id] = std::move(pipeline);
}

bool PipelinePool::deletePipelineFragment(PipelineId id) {
    bool pipelineRemoved = pipelines.erase(id);
    return pipelineRemoved;
}

void PipelinePool::addShader(RayGeneratorShaderId id, std::unique_ptr<RayGeneratorShader> shader) {
    rayGeneratorShaders[id] = std::move(shader);
}

void PipelinePool::addShader(HitShaderId id, std::unique_ptr<HitShader> shader) {
    hitShaders[id] = std::move(shader);
}

void PipelinePool::addShader(OcclusionShaderId id, std::unique_ptr<OcclusionShader> shader) {
    occlusionShaders[id] = std::move(shader);
}

void PipelinePool::addShader(PierceShaderId id, std::unique_ptr<PierceShader> shader) {
    pierceShaders[id] = std::move(shader);
}

void PipelinePool::addShader(MissShaderId id, std::unique_ptr<MissShader> shader) {
    missShaders[id] = std::move(shader);
}

RayGeneratorShader *PipelinePool::getShader(RayGeneratorShaderId id) {
    if (rayGeneratorShaders.count(id) == 0)
        return nullptr;
    return rayGeneratorShaders.at(id).get();
}

HitShader *PipelinePool::getShader(HitShaderId id) {
    if (hitShaders.count(id) == 0)
        return nullptr;
    return hitShaders.at(id).get();
}

OcclusionShader *PipelinePool::getShader(OcclusionShaderId id) {
    if (occlusionShaders.count(id) == 0)
        return nullptr;
    return occlusionShaders.at(id).get();
}

PierceShader *PipelinePool::getShader(PierceShaderId id) {
    if (pierceShaders.count(id) == 0)
        return nullptr;
    return pierceShaders.at(id).get();
}

MissShader *PipelinePool::getShader(MissShaderId id) {
    if (missShaders.count(id) == 0)
        return nullptr;
    return missShaders.at(id).get();
}

bool PipelinePool::deleteShader(RayGeneratorShaderId id) {
    bool shaderRemoved = rayGeneratorShaders.erase(id);
    return shaderRemoved;
}

bool PipelinePool::deleteShader(HitShaderId id) {
    bool shaderRemoved = hitShaders.erase(id);
    return shaderRemoved;
}

bool PipelinePool::deleteShader(OcclusionShaderId id) {
    bool shaderRemoved = occlusionShaders.erase(id);
    return shaderRemoved;
}

bool PipelinePool::deleteShader(PierceShaderId id) {
    bool shaderRemoved = pierceShaders.erase(id);
    return shaderRemoved;
}

bool PipelinePool::deleteShader(MissShaderId id) {
    bool shaderRemoved = missShaders.erase(id);
    return shaderRemoved;
}

void PipelinePool::storeShaderResource(std::unique_ptr<ShaderResource> shaderResource, ShaderResourceId id) {
    shaderResources[id] = std::move(shaderResource);
}

bool PipelinePool::deleteShaderResource(ShaderResourceId id) {
    const bool objectRemoved = shaderResources.erase(id) != 0;
    return objectRemoved;
}

ShaderResource *PipelinePool::getShaderResource(ShaderResourceId id) {
    return shaderResources.at(id).get();
}

void PipelinePool::runPipeline(PipelineId id) {
    if (pipelines.count(id) == 1) {
        pipelines[id]->run();
    }
}

void PipelinePool::runPipelines() {
    for (auto &p: pipelines) {
        p.second->run();
    }
}

PipelineImplement *PipelinePool::getPipelineFragment(PipelineId id) {
    if (pipelines.count(id) == 0)
        return nullptr;
    return pipelines[id].get();
}