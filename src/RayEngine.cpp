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

int RayEngine::runPipeline(int id) {
    return dataManagementUnit->runPipeline(id);
}

int RayEngine::runAll() {
    return dataManagementUnit->runAllPipelines();
}

int RayEngine::createPipeline(PipelineDescription *pipelineDescription) {
    return dataManagementUnit->addPipeline(pipelineDescription);
}

bool RayEngine::deletePipeline(int id) {
    return dataManagementUnit->removePipeline(id);
}

bool RayEngine::bindGeometryToPipeline(int pipelineId, std::vector<int> *objectIds, std::vector<Matrix4x4> *transforms,
                                       std::vector<ObjectParameter> *objectParameters, std::vector<int>*instanceIDs) {
    return dataManagementUnit->bindGeometryToPipeline(pipelineId, objectIds, transforms, objectParameters, instanceIDs);
}

int RayEngine::addObject(Object *object) {
    return dataManagementUnit->addObject(object);
}

bool RayEngine::removeObject(int id) {
    return dataManagementUnit->removeObject(id);
}

bool RayEngine::updateObject(int id, Object *object) {
    return dataManagementUnit->updateObject(id, object);
}

bool RayEngine::bindShaderToPipeline(int pipelineId, int *shaderId, std::vector<int> *shaderResourceIds) {
    return dataManagementUnit->bindShaderToPipeline(pipelineId, shaderId, shaderResourceIds);
}

int RayEngine::addShader(HitShader *shader) {
    return dataManagementUnit->addShader(shader);
}

int RayEngine::addShader(MissShader *shader) {
    return dataManagementUnit->addShader(shader);
}

int RayEngine::addShader(OcclusionShader *shader) {
    return dataManagementUnit->addShader(shader);
}

int RayEngine::addShader(PierceShader *shader) {
    return dataManagementUnit->addShader(shader);
}

int RayEngine::addShader(RayGeneratorShader *shader) {
    return dataManagementUnit->addShader(shader);
}

bool RayEngine::removeShader(int id) {
    return dataManagementUnit->removeShader(id);
}

int RayEngine::addShaderResource(ShaderResource *resource) {
    return dataManagementUnit->addShaderResource(resource);
}

bool RayEngine::removeShaderResource(int id) {
    return dataManagementUnit->removeShaderResource(id);
}

bool RayEngine::updatePipelineObjects(int pipelineId, std::vector<int> *objectInstanceIDs,
                                      std::vector<Matrix4x4 *> *transforms,
                                      std::vector<ObjectParameter *> *objectParameters) {
    return dataManagementUnit->updatePipelineObjects(pipelineId, objectInstanceIDs, transforms, objectParameters);
}

bool RayEngine::updatePipelineShader(int pipelineId, int shaderInstanceId, std::vector<int> *shaderResourceIds) {
    return dataManagementUnit->updatePipelineShader(pipelineId, shaderInstanceId, shaderResourceIds);
}

bool RayEngine::removePipelineObject(int pipelineId, int objectInstanceId) {
    return dataManagementUnit->removePipelineObject(pipelineId, objectInstanceId);
}

bool RayEngine::removePipelineShader(int pipelineId, int shaderInstanceId) {
    return dataManagementUnit->removePipelineShader(pipelineId, shaderInstanceId);
}

void
RayEngine::updatePipelineCamera(int id, int resolutionX, int resolutionY, Vector3D cameraPosition, Vector3D cameraDirection,
                                Vector3D cameraUp) {
    dataManagementUnit->updatePipelineCamera(id, resolutionX, resolutionY, cameraPosition, cameraDirection, cameraUp);
}

Texture *RayEngine::getPipelineResult(int id) {
    return dataManagementUnit->getPipelineResult(id);
}
