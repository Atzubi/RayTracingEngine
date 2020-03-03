//
// Created by sebastian on 02.07.19.
//

#include <iostream>
#include "../API/RayEngine.h"
#include "./Data Management/DataManagementUnit.h"

RayEngine::RayEngine() {
    dataManagementUnit = new DataManagementUnit();
}

RayEngine::~RayEngine() {
    delete dataManagementUnit;
}

int RayEngine::runPipeline(int id) {
    return 0;
}

int RayEngine::runAll() {
    return 0;
}

int RayEngine::addPipeline(Pipeline *pipeline) {
    return dataManagementUnit->addPipeline(pipeline);
}

bool RayEngine::removePipeline(int id) {
    return dataManagementUnit->removePipeline(id);
}

bool RayEngine::bindGeometryToPipeline(int pipelineId, std::vector<int> *objectIds, std::vector<Vector3D> *position,
                                       std::vector<Vector3D> *orientation, std::vector<double> *newScaleFactor,
                                       std::vector<ObjectParameter> *objectParameter) {
    return dataManagementUnit->bindGeometryToPipeline(pipelineId, objectIds, position, orientation, newScaleFactor,
                                                      objectParameter);
}

int RayEngine::addObject(Object *object, Vector3D position, Vector3D orientation, double newScaleFactor,
                         ObjectParameter objectParameter) {
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

int RayEngine::addShader(ControlShader *shader) {
    return dataManagementUnit->addShader(shader);
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

int RayEngine::addShaderResource(Any *resource) {
    return dataManagementUnit->addShaderResource(resource);
}

bool RayEngine::removeShaderResource(int id) {
    return dataManagementUnit->removeShaderResource(id);
}

bool RayEngine::updatePipelineObject(int pipelineId, int objectInstanceId, Vector3D position, Vector3D orientation,
                                     double newScaleFactor, ObjectParameter objectParameter) {
    return dataManagementUnit->updatePipelineObject(pipelineId, objectInstanceId, position, orientation, newScaleFactor,
                                                    objectParameter);
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

bool RayEngine::bindObjectToPipeline(int pipelineId, int *objectId, Vector3D position, Vector3D orientation,
                                     double newScaleFactor, ObjectParameter objectParameter) {
    return dataManagementUnit->bindObjectToPipeline(pipelineId, objectId, position, orientation, newScaleFactor,
                                                    objectParameter);
}
