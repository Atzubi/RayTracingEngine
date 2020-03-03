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
    if(objects.count(id) == 0) {
        return false;
    }else{
        objects.at(id) = object->clone();
        return true;
    }
}

int DataManagementUnit::addShader(ControlShader *shader) {
    controlShaders.insert(std::pair<int, ControlShader *>(*shaderIds.begin(), shader));

    int buffer = shaderIds.extract(shaderIds.begin()).value();

    if (shaderIds.empty()) {
        shaderIds.insert(buffer + 1);
    }

    return buffer;
}

int DataManagementUnit::addShader(HitShader *shader) {
    hitShaders.insert(std::pair<int, HitShader *>(*shaderIds.begin(), shader));

    int buffer = shaderIds.extract(shaderIds.begin()).value();

    if (shaderIds.empty()) {
        shaderIds.insert(buffer + 1);
    }

    return buffer;
}

int DataManagementUnit::addShader(MissShader *shader) {
    missShaders.insert(std::pair<int, MissShader *>(*shaderIds.begin(), shader));

    int buffer = shaderIds.extract(shaderIds.begin()).value();

    if (shaderIds.empty()) {
        shaderIds.insert(buffer + 1);
    }

    return buffer;
}

int DataManagementUnit::addShader(OcclusionShader *shader) {
    occlusionShaders.insert(std::pair<int, OcclusionShader *>(*shaderIds.begin(), shader));

    int buffer = shaderIds.extract(shaderIds.begin()).value();

    if (shaderIds.empty()) {
        shaderIds.insert(buffer + 1);
    }

    return buffer;
}

int DataManagementUnit::addShader(PierceShader *shader) {
    pierceShaders.insert(std::pair<int, PierceShader *>(*shaderIds.begin(), shader));

    int buffer = shaderIds.extract(shaderIds.begin()).value();

    if (shaderIds.empty()) {
        shaderIds.insert(buffer + 1);
    }

    return buffer;
}

int DataManagementUnit::addShader(RayGeneratorShader *shader) {
    rayGeneratorShaders.insert(std::pair<int, RayGeneratorShader *>(*shaderIds.begin(), shader));

    int buffer = shaderIds.extract(shaderIds.begin()).value();

    if (shaderIds.empty()) {
        shaderIds.insert(buffer + 1);
    }

    return buffer;
}

bool DataManagementUnit::removeShader(int id) {
    if (controlShaders.count(id) != 0) {
        controlShaders.erase(id);
    } else if (hitShaders.count(id) != 0) {
        hitShaders.erase(id);
    } else if (missShaders.count(id) != 0) {
        missShaders.erase(id);
    } else if (occlusionShaders.count(id) != 0) {
        occlusionShaders.erase(id);
    } else if (pierceShaders.count(id) != 0) {
        pierceShaders.erase(id);
    } else if (rayGeneratorShaders.count(id) != 0) {
        rayGeneratorShaders.erase(id);
    } else {
        return false;
    }

    shaderIds.insert(id);

    auto iterator = shaderIds.rbegin();
    int end = *iterator - 1;

    int buffer = *iterator;
    while (end-- == *++iterator) {
        shaderIds.erase(buffer);
        buffer = *iterator;
    }

    return true;
}

int DataManagementUnit::addShaderResource(Any *resource) {
    shadersResources.insert(std::pair<int, Any *>(*shaderResourceIds.begin(), resource));

    int buffer = shaderResourceIds.extract(shaderResourceIds.begin()).value();

    if (shaderResourceIds.empty()) {
        shaderResourceIds.insert(buffer + 1);
    }

    return buffer;
}

bool DataManagementUnit::removeShaderResource(int id) {
    if (shadersResources.count(id) == 0)
        return false;

    shadersResources.erase(id);
    shaderResourceIds.insert(id);

    auto iterator = shaderResourceIds.rbegin();
    int end = *iterator - 1;

    int buffer = *iterator;
    while (end-- == *++iterator) {
        shaderResourceIds.erase(buffer);
        buffer = *iterator;
    }

    return true;
}

