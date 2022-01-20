//
// Created by sebastian on 02.07.19.
//

#include <iostream>

#include "RayTraceEngine/RayEngine.h"
#include "Data Management/DataManagementUnitV2.h"


RayEngine::RayEngine() {
    dataManagementUnit = new DataManagementUnitV2();
}

RayEngine::~RayEngine() {
    delete dataManagementUnit;
}

int RayEngine::runPipeline(PipelineId id) {
    return dataManagementUnit->runPipeline(id);
}

int RayEngine::runAll() {
    return dataManagementUnit->runAllPipelines();
}

PipelineId RayEngine::createPipeline(PipelineDescription *pipelineDescription) {
    return dataManagementUnit->createPipeline(pipelineDescription);
}

bool RayEngine::deletePipeline(PipelineId id) {
    return dataManagementUnit->removePipeline(id);
}

bool RayEngine::bindGeometryToPipeline(PipelineId pipelineId, std::vector<ObjectId> *objectIds, std::vector<Matrix4x4> *transforms,
                                       std::vector<ObjectParameter> *objectParameters, std::vector<InstanceId>*instanceIDs) {
    return dataManagementUnit->bindGeometryToPipeline(pipelineId, objectIds, transforms, objectParameters, instanceIDs);
}

ObjectId RayEngine::addObject(Object *object) {
    return dataManagementUnit->addObject(object);
}

bool RayEngine::removeObject(ObjectId id) {
    return dataManagementUnit->removeObject(id);
}

bool RayEngine::updateObject(ObjectId id, Object *object) {
    return dataManagementUnit->updateObject(id, object);
}

bool RayEngine::bindShaderToPipeline(PipelineId pipelineId, RayGeneratorShaderId shaderId, std::vector<ShaderResourceId> *shaderResourceIds) {
    return dataManagementUnit->bindShaderToPipeline(pipelineId, shaderId, shaderResourceIds);
}

bool RayEngine::bindShaderToPipeline(PipelineId pipelineId, HitShaderId shaderId, std::vector<ShaderResourceId> *shaderResourceIds) {
    return dataManagementUnit->bindShaderToPipeline(pipelineId, shaderId, shaderResourceIds);
}

bool RayEngine::bindShaderToPipeline(PipelineId pipelineId, OcclusionShaderId shaderId, std::vector<ShaderResourceId> *shaderResourceIds) {
    return dataManagementUnit->bindShaderToPipeline(pipelineId, shaderId, shaderResourceIds);
}

bool RayEngine::bindShaderToPipeline(PipelineId pipelineId, PierceShaderId shaderId, std::vector<ShaderResourceId> *shaderResourceIds) {
    return dataManagementUnit->bindShaderToPipeline(pipelineId, shaderId, shaderResourceIds);
}

bool RayEngine::bindShaderToPipeline(PipelineId pipelineId, MissShaderId shaderId, std::vector<ShaderResourceId> *shaderResourceIds) {
    return dataManagementUnit->bindShaderToPipeline(pipelineId, shaderId, shaderResourceIds);
}

HitShaderId RayEngine::addShader(HitShader *shader) {
    return dataManagementUnit->addShader(shader);
}

MissShaderId RayEngine::addShader(MissShader *shader) {
    return dataManagementUnit->addShader(shader);
}

OcclusionShaderId RayEngine::addShader(OcclusionShader *shader) {
    return dataManagementUnit->addShader(shader);
}

PierceShaderId RayEngine::addShader(PierceShader *shader) {
    return dataManagementUnit->addShader(shader);
}

RayGeneratorShaderId RayEngine::addShader(RayGeneratorShader *shader) {
    return dataManagementUnit->addShader(shader);
}

bool RayEngine::removeShader(RayGeneratorShaderId id) {
    return dataManagementUnit->removeShader(id);
}

bool RayEngine::removeShader(HitShaderId id) {
    return dataManagementUnit->removeShader(id);
}

bool RayEngine::removeShader(OcclusionShaderId id) {
    return dataManagementUnit->removeShader(id);
}

bool RayEngine::removeShader(PierceShaderId id) {
    return dataManagementUnit->removeShader(id);
}

bool RayEngine::removeShader(MissShaderId id) {
    return dataManagementUnit->removeShader(id);
}

ShaderResourceId RayEngine::addShaderResource(ShaderResource *resource) {
    return dataManagementUnit->addShaderResource(resource);
}

bool RayEngine::removeShaderResource(ShaderResourceId id) {
    return dataManagementUnit->removeShaderResource(id);
}

bool RayEngine::updatePipelineObjects(PipelineId pipelineId, std::vector<InstanceId> *objectInstanceIDs,
                                      std::vector<Matrix4x4 *> *transforms,
                                      std::vector<ObjectParameter *> *objectParameters) {
    return dataManagementUnit->updatePipelineObjects(pipelineId, objectInstanceIDs, transforms, objectParameters);
}

bool RayEngine::updatePipelineShader(PipelineId pipelineId, RayGeneratorShaderId shaderId, std::vector<ShaderResourceId> *shaderResourceIds) {
    return dataManagementUnit->updatePipelineShader(pipelineId, shaderId, shaderResourceIds);
}

bool RayEngine::updatePipelineShader(PipelineId pipelineId, HitShaderId shaderId, std::vector<ShaderResourceId> *shaderResourceIds) {
    return dataManagementUnit->updatePipelineShader(pipelineId, shaderId, shaderResourceIds);
}

bool RayEngine::updatePipelineShader(PipelineId pipelineId, OcclusionShaderId shaderId, std::vector<ShaderResourceId> *shaderResourceIds) {
    return dataManagementUnit->updatePipelineShader(pipelineId, shaderId, shaderResourceIds);
}

bool RayEngine::updatePipelineShader(PipelineId pipelineId, PierceShaderId shaderId, std::vector<ShaderResourceId> *shaderResourceIds) {
    return dataManagementUnit->updatePipelineShader(pipelineId, shaderId, shaderResourceIds);
}

bool RayEngine::updatePipelineShader(PipelineId pipelineId, MissShaderId shaderId, std::vector<ShaderResourceId> *shaderResourceIds) {
    return dataManagementUnit->updatePipelineShader(pipelineId, shaderId, shaderResourceIds);
}

bool RayEngine::removePipelineObject(PipelineId pipelineId, InstanceId objectInstanceId) {
    return dataManagementUnit->removePipelineObject(pipelineId, objectInstanceId);
}

bool RayEngine::removePipelineShader(PipelineId pipelineId, RayGeneratorShaderId shaderId) {
    return dataManagementUnit->removePipelineShader(pipelineId, shaderId);
}

bool RayEngine::removePipelineShader(PipelineId pipelineId, HitShaderId shaderId) {
    return dataManagementUnit->removePipelineShader(pipelineId, shaderId);
}

bool RayEngine::removePipelineShader(PipelineId pipelineId, OcclusionShaderId shaderId) {
    return dataManagementUnit->removePipelineShader(pipelineId, shaderId);
}

bool RayEngine::removePipelineShader(PipelineId pipelineId, PierceShaderId shaderId) {
    return dataManagementUnit->removePipelineShader(pipelineId, shaderId);
}

bool RayEngine::removePipelineShader(PipelineId pipelineId, MissShaderId shaderId) {
    return dataManagementUnit->removePipelineShader(pipelineId, shaderId);
}

void
RayEngine::updatePipelineCamera(PipelineId id, int resolutionX, int resolutionY, Vector3D cameraPosition, Vector3D cameraDirection,
                                Vector3D cameraUp) {
    dataManagementUnit->updatePipelineCamera(id, resolutionX, resolutionY, cameraPosition, cameraDirection, cameraUp);
}

Texture *RayEngine::getPipelineResult(PipelineId id) {
    return dataManagementUnit->getPipelineResult(id);
}
