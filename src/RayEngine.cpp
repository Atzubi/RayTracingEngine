//
// Created by sebastian on 02.07.19.
//

#include <iostream>

#include "RayTraceEngine/RayEngine.h"
#include "engine_node/EngineNode.h"


RayEngine::RayEngine() {
    engineNode = std::make_unique<EngineNode>();
}

RayEngine::~RayEngine() = default;

int RayEngine::runPipeline(PipelineId id) {
    return engineNode->runPipeline(id);
}

int RayEngine::runAll() {
    return engineNode->runAllPipelines();
}

PipelineId RayEngine::createPipeline(PipelineDescription &pipelineDescription) {
    return engineNode->createPipeline(pipelineDescription);
}

bool RayEngine::deletePipeline(PipelineId id) {
    return engineNode->removePipeline(id);
}

bool RayEngine::bindGeometryToPipeline(PipelineId pipelineId, const std::vector<ObjectId> &objectIds,
                                       const std::vector<Matrix4x4> &transforms,
                                       const std::vector<ObjectParameter> &objectParameters,
                                       std::vector<InstanceId> &instanceIDs) {
    return engineNode->bindGeometryToPipeline(pipelineId, objectIds, transforms, objectParameters, instanceIDs);
}

ObjectId RayEngine::addObject(const Intersectable &object) {
    return engineNode->addObject(object);
}

bool RayEngine::removeObject(ObjectId id) {
    return engineNode->removeObject(id);
}

bool RayEngine::updateObject(ObjectId id, const Intersectable &object) {
    return engineNode->updateObject(id, object);
}

bool RayEngine::bindShaderToPipeline(PipelineId pipelineId, RayGeneratorShaderId shaderId,
                                     std::vector<ShaderResourceId> &shaderResourceIds) {
    return engineNode->bindShaderToPipeline(pipelineId, shaderId, shaderResourceIds);
}

bool RayEngine::bindShaderToPipeline(PipelineId pipelineId, HitShaderId shaderId,
                                     std::vector<ShaderResourceId> &shaderResourceIds) {
    return engineNode->bindShaderToPipeline(pipelineId, shaderId, shaderResourceIds);
}

bool RayEngine::bindShaderToPipeline(PipelineId pipelineId, OcclusionShaderId shaderId,
                                     std::vector<ShaderResourceId> &shaderResourceIds) {
    return engineNode->bindShaderToPipeline(pipelineId, shaderId, shaderResourceIds);
}

bool RayEngine::bindShaderToPipeline(PipelineId pipelineId, PierceShaderId shaderId,
                                     std::vector<ShaderResourceId> &shaderResourceIds) {
    return engineNode->bindShaderToPipeline(pipelineId, shaderId, shaderResourceIds);
}

bool RayEngine::bindShaderToPipeline(PipelineId pipelineId, MissShaderId shaderId,
                                     std::vector<ShaderResourceId> &shaderResourceIds) {
    return engineNode->bindShaderToPipeline(pipelineId, shaderId, shaderResourceIds);
}

HitShaderId RayEngine::addShader(const HitShader &shader) {
    return engineNode->addShader(shader);
}

MissShaderId RayEngine::addShader(const MissShader &shader) {
    return engineNode->addShader(shader);
}

OcclusionShaderId RayEngine::addShader(const OcclusionShader &shader) {
    return engineNode->addShader(shader);
}

PierceShaderId RayEngine::addShader(const PierceShader &shader) {
    return engineNode->addShader(shader);
}

RayGeneratorShaderId RayEngine::addShader(const RayGeneratorShader &shader) {
    return engineNode->addShader(shader);
}

bool RayEngine::removeShader(RayGeneratorShaderId id) {
    return engineNode->removeShader(id);
}

bool RayEngine::removeShader(HitShaderId id) {
    return engineNode->removeShader(id);
}

bool RayEngine::removeShader(OcclusionShaderId id) {
    return engineNode->removeShader(id);
}

bool RayEngine::removeShader(PierceShaderId id) {
    return engineNode->removeShader(id);
}

bool RayEngine::removeShader(MissShaderId id) {
    return engineNode->removeShader(id);
}

ShaderResourceId RayEngine::addShaderResource(const ShaderResource &resource) {
    return engineNode->addShaderResource(resource);
}

bool RayEngine::removeShaderResource(ShaderResourceId id) {
    return engineNode->removeShaderResource(id);
}

bool RayEngine::updatePipelineObjects(PipelineId pipelineId, const std::vector<InstanceId> &objectInstanceIDs,
                                      const std::vector<Matrix4x4> &transforms,
                                      const std::vector<ObjectParameter> &objectParameters) {
    return engineNode->updatePipelineObjects(pipelineId, objectInstanceIDs, transforms, objectParameters);
}

bool RayEngine::updatePipelineShader(PipelineId pipelineId, RayGeneratorShaderId shaderId,
                                     const std::vector<ShaderResourceId> &shaderResourceIds) {
    return engineNode->updatePipelineShader(pipelineId, shaderId, shaderResourceIds);
}

bool RayEngine::updatePipelineShader(PipelineId pipelineId, HitShaderId shaderId,
                                     const std::vector<ShaderResourceId> &shaderResourceIds) {
    return engineNode->updatePipelineShader(pipelineId, shaderId, shaderResourceIds);
}

bool RayEngine::updatePipelineShader(PipelineId pipelineId, OcclusionShaderId shaderId,
                                     const std::vector<ShaderResourceId> &shaderResourceIds) {
    return engineNode->updatePipelineShader(pipelineId, shaderId, shaderResourceIds);
}

bool RayEngine::updatePipelineShader(PipelineId pipelineId, PierceShaderId shaderId,
                                     const std::vector<ShaderResourceId> &shaderResourceIds) {
    return engineNode->updatePipelineShader(pipelineId, shaderId, shaderResourceIds);
}

bool RayEngine::updatePipelineShader(PipelineId pipelineId, MissShaderId shaderId,
                                     const std::vector<ShaderResourceId> &shaderResourceIds) {
    return engineNode->updatePipelineShader(pipelineId, shaderId, shaderResourceIds);
}

bool RayEngine::removePipelineObject(PipelineId pipelineId, InstanceId objectInstanceId) {
    return engineNode->removePipelineObject(pipelineId, objectInstanceId);
}

bool RayEngine::removePipelineShader(PipelineId pipelineId, RayGeneratorShaderId shaderId) {
    return engineNode->removePipelineShader(pipelineId, shaderId);
}

bool RayEngine::removePipelineShader(PipelineId pipelineId, HitShaderId shaderId) {
    return engineNode->removePipelineShader(pipelineId, shaderId);
}

bool RayEngine::removePipelineShader(PipelineId pipelineId, OcclusionShaderId shaderId) {
    return engineNode->removePipelineShader(pipelineId, shaderId);
}

bool RayEngine::removePipelineShader(PipelineId pipelineId, PierceShaderId shaderId) {
    return engineNode->removePipelineShader(pipelineId, shaderId);
}

bool RayEngine::removePipelineShader(PipelineId pipelineId, MissShaderId shaderId) {
    return engineNode->removePipelineShader(pipelineId, shaderId);
}

void
RayEngine::updatePipelineCamera(PipelineId id, int resolutionX, int resolutionY, const Vector3D &cameraPosition,
                                const Vector3D &cameraDirection, const Vector3D &cameraUp) {
    engineNode->updatePipelineCamera(id, resolutionX, resolutionY, cameraPosition, cameraDirection, cameraUp);
}

Texture *RayEngine::getPipelineResult(PipelineId id) {
    return engineNode->getPipelineResult(id);
}
