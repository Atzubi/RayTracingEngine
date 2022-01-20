//
// Created by Sebastian on 04.12.2021.
//

#include "DataManagementUnitV2.h"
#include "Engine Node/EngineNode.h"
#include "Pipeline/PipelineImplement.h"
#include "Object/Instance.h"
#include "RayTraceEngine/Pipeline.h"
#include "Acceleration Structures/DBVHv2.h"

DataManagementUnitV2::DataManagementUnitV2() {
    deviceId = getDeviceId();

    objectIds.insert(0);
    shaderIds.insert(0);
    pipelineIds.insert(0);
    objectInstanceIds.insert(0);
    shaderResourceIds.insert(0);

    engineNode = new EngineNode(this);
}

DataManagementUnitV2::~DataManagementUnitV2() {
    delete engineNode;
}

int DataManagementUnitV2::addPipeline(PipelineDescription *pipelineDescription) {
    std::vector<Object *> instances;
    std::vector<int> instanceIds;

    // pull all objects required to create the pipeline
    // only requires id, box and cost
    int c = 0;
    for (auto i: pipelineDescription->objectIDs) {
        if (objectIdDeviceMap.count(i) == 1) {
            if (objectIdDeviceMap[i] == deviceId) {
                auto capsule = engineNode->requestBaseData(i)->getCapsule();
                capsule.id = i;

                // create instances of objects
                auto *instance = new Instance(engineNode, &capsule);
                instance->applyTransform(pipelineDescription->objectTransformations[c]);
                instances.push_back(instance);

                // manage instance ids
                int instanceId = objectInstanceIds.extract(objectInstanceIds.begin()).value();
                if (objectInstanceIds.empty()) {
                    objectInstanceIds.insert(instanceId + 1);
                }
                pipelineDescription->objectInstanceIDs->push_back(instanceId);
                objectToInstanceMap[i].insert(instanceId);
                instanceIds.push_back(instanceId);

                // add instances to engine node
                // TODO spread over nodes
                engineNode->storeInstanceDataFragments(instance, instanceId);

                // add instance location to map
                objectInstanceIdDeviceMap[instanceId] = deviceId;
            } else {
                // TODO request object from other engine nodes
            }
        } else {
            // TODO error handling, object not found
        }
        c++;
    }

    // build bvh on instances
    auto *root = new DBVHNode();
    DBVHv2::addObjects(root, &instances);

    // get shader implementation from id
    std::vector<RayGeneratorShaderPackage> pipelineRayGeneratorShaders;
    for (auto id: pipelineDescription->rayGeneratorShaderIDs) {
        pipelineRayGeneratorShaders.push_back({(RayGeneratorShader *) engineNode->getShader(id), id});
    }

    std::vector<OcclusionShaderPackage> pipelineOcclusionShaders;
    for (auto id: pipelineDescription->occlusionShaderIDs) {
        pipelineOcclusionShaders.push_back({(OcclusionShader *) engineNode->getShader(id), id});
    }

    std::vector<HitShaderPackage> pipelineHitShaders;
    for (auto id: pipelineDescription->hitShaderIDs) {
        pipelineHitShaders.push_back({(HitShader *) engineNode->getShader(id), id});
    }

    std::vector<PierceShaderPackage> pipelinePierceShaders;
    for (auto id: pipelineDescription->pierceShaderIDs) {
        pipelinePierceShaders.push_back({(PierceShader *) engineNode->getShader(id), id});
    }

    std::vector<MissShaderPackage> pipelineMissShaders;
    for (auto id: pipelineDescription->missShaderIDs) {
        pipelineMissShaders.push_back({(MissShader *) engineNode->getShader(id), id});
    }

    // get shader resources from id
    std::vector<ShaderResourceContainer> shaderResources;
    for(auto &shaderResourcePackage : pipelineDescription->shaderResources){
        std::vector<ShaderResource*> shaderRes;
        for(auto id : shaderResourcePackage.shaderResourceIds){
            shaderRes.push_back(engineNode->getShaderResource(id));
        }
        shaderResources.push_back({shaderResourcePackage.shaderId, shaderRes});
    }

    // create new pipeline and add bvh, shaders and description
    auto *pipeline = new PipelineImplement(engineNode, pipelineDescription->resolutionX,
                                           pipelineDescription->resolutionY,
                                           &pipelineDescription->cameraPosition,
                                           &pipelineDescription->cameraDirection,
                                           &pipelineDescription->cameraUp, &pipelineRayGeneratorShaders,
                                           &pipelineOcclusionShaders, &pipelineHitShaders,
                                           &pipelinePierceShaders, &pipelineMissShaders, &shaderResources, root);

    int buffer = pipelineIds.extract(pipelineIds.begin()).value();

    // map instances to pipeline
    pipelineToInstanceMap[buffer].insert(instanceIds.begin(), instanceIds.end());

    engineNode->storePipelineFragments(pipeline, buffer);

    if (pipelineIds.empty()) {
        pipelineIds.insert(buffer + 1);
    }

    // broadcast pipeline to all engine nodes
    // TODO

    return buffer;
}

bool DataManagementUnitV2::removePipeline(int id) {
    // TODO: broadcast remove to all nodes;
    // TODO: remove instances used by this pipeline
    auto removed = engineNode->deletePipelineFragment(id);
    if (removed) {
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
    return false;
}

bool
DataManagementUnitV2::updatePipelineObjects(int pipelineId, std::vector<int> *objectInstanceIDs,
                                            std::vector<Matrix4x4 *> *transforms,
                                            std::vector<ObjectParameter *> *objectParameters) {
    if (objectInstanceIDs->size() != transforms->size()) return false;

    for (int i = 0; i < objectInstanceIDs->size(); i++) {
        if (objectInstanceIds.count(objectInstanceIDs->at(i)) == 1) {
            if (objectInstanceIdDeviceMap[objectInstanceIDs->at(i)] == deviceId) {
                auto instance = engineNode->requestInstanceData(objectInstanceIDs->at(i));
                if (instance == nullptr) continue;
                instance->applyTransform(transforms->at(i));
            } else {
                // TODO: update instances on other nodes
            }
        } else {
            // TODO error handling, object not found
        }
    }

    return true;
}

bool
DataManagementUnitV2::updatePipelineShader(int pipelineId, int shaderInstanceId, std::vector<int> *resourceIds) {
    return false;
}

bool DataManagementUnitV2::removePipelineObject(int pipelineId, int objectInstanceId) {
    if (objectIdDeviceMap.count(objectInstanceId) == 1) {
        if (objectIdDeviceMap[objectInstanceId] == deviceId) {
            auto pipeline = engineNode->requestPipelineFragment(pipelineId);
            auto geometry = pipeline->getGeometry();
            auto instance = engineNode->requestInstanceData(objectInstanceId);
            std::vector<Object*> remove = {instance};
            DBVHv2::removeObjects(geometry, &remove);
            return engineNode->deleteInstanceDataFragment(objectInstanceId);
        } else {
            // TODO: delete instance on other nodes
        }
    }
    return false;
}

bool DataManagementUnitV2::removePipelineShader(int pipelineId, int shaderInstanceId) {
    return false;
}

bool
DataManagementUnitV2::bindGeometryToPipeline(int pipelineId, std::vector<int> *objectIDs,
                                             std::vector<Matrix4x4> *transforms,
                                             std::vector<ObjectParameter> *objectParameters,
                                             std::vector<int> *instanceIDs) {
    auto pipeline = engineNode->requestPipelineFragment(pipelineId);
    if (objectIDs->size() != transforms->size() || pipeline == nullptr) return false;

    auto geometry = pipeline->getGeometry();

    std::vector<Object *> instances;
    std::vector<int> instanceIds;

    for (int i = 0; i < objectIDs->size(); i++) {
        if (objectIdDeviceMap.count(objectIDs->at(i)) == 1) {
            if (objectIdDeviceMap[objectIDs->at(i)] == deviceId) {
                auto capsule = engineNode->requestBaseData(objectIDs->at(i))->getCapsule();
                capsule.id = objectIDs->at(i);

                // create instances of objects
                auto *instance = new Instance(engineNode, &capsule);
                instance->applyTransform(&transforms->at(i));
                instances.push_back(instance);

                // manage instance ids
                int instanceId = objectInstanceIds.extract(objectInstanceIds.begin()).value();
                if (objectInstanceIds.empty()) {
                    objectInstanceIds.insert(instanceId + 1);
                }
                instanceIDs->push_back(instanceId);
                objectToInstanceMap[objectIDs->at((i))].insert(instanceId);
                instanceIds.push_back(instanceId);

                // add instances to engine node
                // TODO spread over nodes
                engineNode->storeInstanceDataFragments(instance, instanceId);

                // add instance location to map
                objectInstanceIdDeviceMap[instanceId] = deviceId;
            } else {
                // TODO request object from other engine nodes
            }
        } else {
            // TODO error handling, object not found
        }
    }

    pipelineToInstanceMap[pipelineId].insert(instanceIds.begin(), instanceIds.end());

    DBVHv2::addObjects(geometry, &instances);

    return true;
}

bool DataManagementUnitV2::bindShaderToPipeline(int pipelineId, int *shaderId, std::vector<int> *shaderResourceIds) {
    return false;
}

int DataManagementUnitV2::addObject(Object *object) {
    int buffer = objectIds.extract(objectIds.begin()).value();

    objectIdDeviceMap[buffer] = deviceId;

    // TODO: spread over nodes
    engineNode->storeBaseDataFragments(object->clone(), buffer);

    if (objectIds.empty()) {
        objectIds.insert(buffer + 1);
    }

    return buffer;
}

bool DataManagementUnitV2::removeObject(int id) {
    if (!engineNode->deleteBaseDataFragment(id)) return false;

    // remove instances
    for(auto instanceId : objectToInstanceMap.at(id)){
        for(auto &pipelineInstances : pipelineToInstanceMap){
            if(pipelineInstances.second.count(instanceId) != 0){
                removePipelineObject(pipelineInstances.first, instanceId);
                pipelineInstances.second.erase(instanceId);
                break;
            }
        }
    }

    objectIdDeviceMap.erase(id);

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

bool DataManagementUnitV2::updateObject(int id, Object *object) {
    if (engineNode->deleteBaseDataFragment(id)) return false;
    engineNode->storeBaseDataFragments(object->clone(), id);
    return true;
}

int DataManagementUnitV2::addShader(HitShader *shader) {
    int buffer = shaderIds.extract(shaderIds.begin()).value();

    engineNode->addShader(buffer, shader);

    if (shaderIds.empty()) {
        shaderIds.insert(buffer + 1);
    }

    return buffer;
}

int DataManagementUnitV2::addShader(MissShader *shader) {
    int buffer = shaderIds.extract(shaderIds.begin()).value();

    engineNode->addShader(buffer, shader);

    if (shaderIds.empty()) {
        shaderIds.insert(buffer + 1);
    }

    return buffer;
}

int DataManagementUnitV2::addShader(OcclusionShader *shader) {
    int buffer = shaderIds.extract(shaderIds.begin()).value();

    engineNode->addShader(buffer, shader);

    if (shaderIds.empty()) {
        shaderIds.insert(buffer + 1);
    }

    return buffer;
}

int DataManagementUnitV2::addShader(PierceShader *shader) {
    int buffer = shaderIds.extract(shaderIds.begin()).value();

    engineNode->addShader(buffer, shader);

    if (shaderIds.empty()) {
        shaderIds.insert(buffer + 1);
    }

    return buffer;
}

int DataManagementUnitV2::addShader(RayGeneratorShader *shader) {
    int buffer = shaderIds.extract(shaderIds.begin()).value();

    engineNode->addShader(buffer, shader);

    if (shaderIds.empty()) {
        shaderIds.insert(buffer + 1);
    }

    return buffer;
}

bool DataManagementUnitV2::removeShader(int id) {
    if (!engineNode->deleteShader(id)) return false;
    // TODO: remove shader from pipelines

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

int DataManagementUnitV2::addShaderResource(ShaderResource *resource) {
    int buffer = shaderResourceIds.extract(shaderResourceIds.begin()).value();

    engineNode->storeShaderResource(resource, buffer);

    if (shaderResourceIds.empty()) {
        shaderResourceIds.insert(buffer + 1);
    }

    return buffer;
}

bool DataManagementUnitV2::removeShaderResource(int id) {
    if (!engineNode->deleteShaderResource(id)) return false;

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

int DataManagementUnitV2::runPipeline(int id) {
    engineNode->runPipeline(id);
    return 0;
}

int DataManagementUnitV2::runAllPipelines() {
    engineNode->runPipelines();
    return 0;
}

void DataManagementUnitV2::updatePipelineCamera(int id, int resolutionX, int resolutionY, Vector3D cameraPosition,
                                                Vector3D cameraDirection, Vector3D cameraUp) {
    auto pipeline = engineNode->requestPipelineFragment(id);
    pipeline->setResolution(resolutionX, resolutionY);
    pipeline->setCamera(cameraPosition, cameraDirection, cameraUp);
}

Texture *DataManagementUnitV2::getPipelineResult(int id) {
    auto pipeline = engineNode->requestPipelineFragment(id);
    return pipeline->getResult();
}

int DataManagementUnitV2::getDeviceId() {
    // TODO
    return 0;
}

Object *DataManagementUnitV2::getBaseDataFragment(int id) {
    if (objectIdDeviceMap.count(id) == 0) return nullptr;
    // TODO: request object with id from other device
    return nullptr;
}

Instance *DataManagementUnitV2::getInstanceDataFragment(int id) {
    if (objectInstanceIdDeviceMap.count(id) == 0) return nullptr;
    // TODO: request object with id from other device
    return nullptr;
}
