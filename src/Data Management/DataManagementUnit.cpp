//
// Created by sebastian on 28.02.20.
//

#include <API/Pipeline.h>
#include <vector>
#include <API/Object.h>
#include "DataManagementUnit.h"

DataManagementUnit::DataManagementUnit() {
    objectIds.insert(0);
    shaderIds.insert(0);
    pipelineIds.insert(0);
}

DataManagementUnit::~DataManagementUnit() = default;

int DataManagementUnit::addPipeline(Pipeline *pipeline) {
    pipelines.insert(std::pair<int, Pipeline>(*pipelineIds.begin(), *pipeline));

    int buffer = pipelineIds.extract(pipelineIds.begin()).value();

    if (pipelineIds.empty()) {
        pipelineIds.insert(buffer + 1);
    }

    return buffer;
}

bool DataManagementUnit::removePipeline(int id) {
    if (pipelines.count(id) == 0)
        return false;

    pipelines.erase(id);
    pipelineIds.insert(id);

    auto iterator = pipelineIds.rbegin();
    int end = *iterator - 1;

    int buffer = *iterator;
    while (end-- == *++iterator) {
        pipelineIds.erase(buffer);
        buffer = *iterator;
    }

    return true;
}

bool
DataManagementUnit::updatePipelineObject(int pipelineId, int objectInstanceId, Vector3D position, Vector3D orientation,
                                         double newScaleFactor, ObjectParameter objectParameter) {
    return false;
}

bool
DataManagementUnit::updatePipelineShader(int pipelineId, int shaderInstanceId, std::vector<int> *shaderResourceIds) {
    return false;
}

bool DataManagementUnit::bindObjectToPipeline(int pipelineId, int *objectId, Vector3D position, Vector3D orientation,
                                           double newScaleFactor, ObjectParameter objectParameter) {
    return false;
}

bool DataManagementUnit::removePipelineObject(int pipelineId, int objectInstanceId) {
    return false;
}

bool DataManagementUnit::removePipelineShader(int pipelineId, int shaderInstanceId) {
    return false;
}

bool
DataManagementUnit::bindGeometryToPipeline(int pipelineId, std::vector<int> *objectIds, std::vector<Vector3D> *position,
                                           std::vector<Vector3D> *orientation, std::vector<double> *newScaleFactor,
                                           std::vector<ObjectParameter> *objectParameter) {
    //TODO build acceleration data structure
    return false;
}

bool DataManagementUnit::bindShaderToPipeline(int pipelineId, int *shaderId, std::vector<int> *shaderResourceIds) {
    return false;
}

int DataManagementUnit::addObject(Object *object) {
    objects.insert(std::pair<int, Object *>(*objectIds.begin(), object->clone()));

    int buffer = objectIds.extract(objectIds.begin()).value();

    if (objectIds.empty()) {
        objectIds.insert(buffer + 1);
    }

    return buffer;
}

bool DataManagementUnit::removeObject(int id) {
    if (objects.count(id) == 0)
        return false;

    objects.erase(id);
    objectIds.insert(id);

    auto iterator = objectIds.rbegin();
    int end = *iterator - 1;

    int buffer = *iterator;
    while (end-- == *++iterator) {
        objectIds.erase(buffer);
        buffer = *iterator;
    }

    return true;
}

bool DataManagementUnit::updateObject(int id, Object *object) {
    return false;
}

int DataManagementUnit::addShader(ControlShader *shader) {
    return 0;
}

int DataManagementUnit::addShader(HitShader *shader) {
    return 0;
}

int DataManagementUnit::addShader(MissShader *shader) {
    return 0;
}

int DataManagementUnit::addShader(OcclusionShader *shader) {
    return 0;
}

int DataManagementUnit::addShader(PierceShader *shader) {
    return 0;
}

int DataManagementUnit::addShader(RayGeneratorShader *shader) {
    return 0;
}

bool DataManagementUnit::removeShader(int id) {
    return false;
}

int DataManagementUnit::addShaderResource(Any resource) {
    return 0;
}

bool DataManagementUnit::removeShaderResource(int id) {
    return false;
}

