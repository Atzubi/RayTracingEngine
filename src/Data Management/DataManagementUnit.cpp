//
// Created by sebastian on 28.02.20.
//


#include <vector>

#include "RayTraceEngine/Object.h"
#include "Acceleration Structures/DBVH.h"
#include "Data Management/DataManagementUnit.h"
#include "Object/Instance.h"
#include "Pipeline/PipelineImplement.h"


DataManagementUnit::DataManagementUnit() {
    objectIds.insert(0);
    shaderIds.insert(0);
    pipelineIds.insert(0);
}

DataManagementUnit::~DataManagementUnit(){
    for(auto pair : pipelines){
        delete pair.second;
    }
}

int DataManagementUnit::addPipeline(PipelineDescription *pipelineDescription) {
    DBVH *dbvh = new DBVH();
    std::vector<Object *> pipelineObjects;
    for (auto id: pipelineDescription->objectIDs) {
        pipelineObjects.push_back(objects.at(id));
    }
    dbvh->addObjects(&pipelineObjects);

    std::vector<RayGeneratorShader *> pipelineRayGeneratorShaders;
    for (auto id: pipelineDescription->rayGeneratorShaderIDs) {
        pipelineRayGeneratorShaders.push_back(rayGeneratorShaders.at(id));
    }

    std::vector<OcclusionShader *> pipelineOcclusionShaders;
    for (auto id: pipelineDescription->occlusionShaderIDs) {
        pipelineOcclusionShaders.push_back(occlusionShaders.at(id));
    }

    std::vector<HitShader *> pipelineHitShaders;
    for (auto id: pipelineDescription->hitShaderIDs) {
        pipelineHitShaders.push_back(hitShaders.at(id));
    }

    std::vector<PierceShader *> pipelinePierceShaders;
    for (auto id: pipelineDescription->pierceShaderIDs) {
        pipelinePierceShaders.push_back(pierceShaders.at(id));
    }

    std::vector<MissShader *> pipelineMissShaders;
    for (auto id: pipelineDescription->missShaderIDs) {
        pipelineMissShaders.push_back(missShaders.at(id));
    }

    auto *pipeline = new PipelineImplement(pipelineDescription->resolutionX,
                                           pipelineDescription->resolutionY,
                                           &pipelineDescription->cameraPosition,
                                           &pipelineDescription->cameraDirection,
                                           &pipelineDescription->cameraUp, &pipelineRayGeneratorShaders,
                                           &pipelineOcclusionShaders, &pipelineHitShaders,
                                           &pipelinePierceShaders, &pipelineMissShaders, dbvh);

    pipelines.insert(std::pair<int, PipelineImplement *>(*pipelineIds.begin(), pipeline));

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
DataManagementUnit::updatePipelineObjects(int pipelineId, std::vector<int> *objectInstanceIDs,
                                          std::vector<Matrix4x4 *> *transforms,
                                          std::vector<ObjectParameter *> *objectParameters) {
    return false;
}

bool
DataManagementUnit::updatePipelineShader(int pipelineId, int shaderInstanceId, std::vector<int> *shaderResourceIds) {
    return false;
}

bool DataManagementUnit::removePipelineObject(int pipelineId, int objectInstanceId) {
    return false;
}

bool DataManagementUnit::removePipelineShader(int pipelineId, int shaderInstanceId) {
    return false;
}

bool
DataManagementUnit::bindGeometryToPipeline(int pipelineId, std::vector<int> *objectIds,
                                           std::vector<Matrix4x4> *transforms,
                                           std::vector<ObjectParameter> *objectParameters,
                                           std::vector<int> *instanceIDs) {
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
    if (objects.count(id) == 0) {
        return false;
    } else {
        objects.at(id) = object->clone();
        return true;
    }
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
    if (hitShaders.count(id) != 0) {
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

int DataManagementUnit::runPipeline(int id) {
    return pipelines.at(id)->run();
}

int DataManagementUnit::runAllPipelines() {
    for (auto p: pipelines) {
        p.second->run();
    }
    return 0;
}

void DataManagementUnit::updatePipelineCamera(int id, int resolutionX, int resolutionY, Vector3D cameraPosition,
                                              Vector3D cameraDirection, Vector3D cameraUp) {
    auto pipeline = pipelines.at(id);
    pipeline->setResolution(resolutionX, resolutionY);
    pipeline->setCamera(cameraPosition, cameraDirection, cameraUp);
}

Texture DataManagementUnit::getPipelineResult(int id) {
    auto pipeline = pipelines.at(id);
    return pipeline->getResult();
}

