//
// Created by Sebastian on 04.12.2021.
//

#include "DataManagementUnitV2.h"
#include "Engine Node/EngineNode.h"
#include "Pipeline/PipelineImplement.h"
#include "Object/Instance.h"
#include "RayTraceEngine/Pipeline.h"
#include "Acceleration Structures/DBVHv2.h"
#include "RayTraceEngine/Shader.h"

DataManagementUnitV2::DataManagementUnitV2() : deviceId(getDeviceId()) {
    objectIds.insert(ObjectId{0});
    rayGeneratorShaderIds.insert(RayGeneratorShaderId{0});
    hitShaderIds.insert(HitShaderId{0});
    occlusionShaderIds.insert(OcclusionShaderId{0});
    pierceShaderIds.insert(PierceShaderId{0});
    missShaderIds.insert(MissShaderId{0});
    pipelineIds.insert(PipelineId{0});
    objectInstanceIds.insert(InstanceId{0});
    shaderResourceIds.insert(ShaderResourceId{0});

    engineNode = std::make_unique<EngineNode>(this);
}

DataManagementUnitV2::~DataManagementUnitV2() = default;

PipelineId DataManagementUnitV2::createPipeline(PipelineDescription &pipelineDescription) {
    std::vector<Object *> instances;
    std::vector<InstanceId> instanceIds;

    // pull all objects required to create the pipeline
    // only requires id, box and cost
    int c = 0;
    for (auto i: pipelineDescription.objectIDs) {
        if (objectIdDeviceMap.count(i) == 1) {
            if (objectIdDeviceMap[i].id == deviceId.id) {
                auto buffer = engineNode->requestBaseData(i)->getCapsule();
                auto capsule = ObjectCapsule{ObjectId{i.id}, buffer.boundingBox, buffer.cost};

                // create instances of objects
                auto instance = std::make_unique<Instance>(*engineNode, capsule);
                instance->applyTransform(pipelineDescription.objectTransformations[c]);
                instances.push_back(instance.get());

                // manage instance ids
                auto instanceId = objectInstanceIds.extract(objectInstanceIds.begin()).value();
                if (objectInstanceIds.empty()) {
                    objectInstanceIds.insert(InstanceId{instanceId.id + 1});
                }
                pipelineDescription.objectInstanceIDs->push_back(instanceId);
                objectToInstanceMap[i].insert(instanceId);
                instanceIds.push_back(instanceId);

                // add instances to engine node
                // TODO spread over nodes
                engineNode->storeInstanceDataFragments(std::move(instance), instanceId);

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
    auto root = std::make_unique<DBVHNode>();
    DBVHv2::addObjects(*root, instances);

    // get shader implementation from id
    std::vector<RayGeneratorShaderPackage> pipelineRayGeneratorShaders;
    for (const auto &shader: pipelineDescription.rayGeneratorShaders) {
        RayGeneratorShaderContainer rayGeneratorShaderContainer;
        std::vector<ShaderResource *> shaderResources;

        for (auto resourceId: shader.shaderResourceIds) {
            shaderResources.push_back(engineNode->getShaderResource(resourceId));
        }

        rayGeneratorShaderContainer.shaderResources = shaderResources;
        rayGeneratorShaderContainer.rayGeneratorShader = engineNode->getShader(shader.shaderId);
        pipelineRayGeneratorShaders.push_back({rayGeneratorShaderContainer, shader.shaderId});
    }

    std::vector<OcclusionShaderPackage> pipelineOcclusionShaders;
    for (const auto &shader: pipelineDescription.occlusionShaders) {
        OcclusionShaderContainer occlusionShaderContainer;
        std::vector<ShaderResource *> shaderResources;

        for (auto resourceId: shader.shaderResourceIds) {
            shaderResources.push_back(engineNode->getShaderResource(resourceId));
        }

        occlusionShaderContainer.shaderResources = shaderResources;
        occlusionShaderContainer.occlusionShader = engineNode->getShader(shader.shaderId);
        pipelineOcclusionShaders.push_back({occlusionShaderContainer, shader.shaderId});
    }

    std::vector<HitShaderPackage> pipelineHitShaders;
    for (const auto &shader: pipelineDescription.hitShaders) {
        HitShaderContainer hitShaderContainer;
        std::vector<ShaderResource *> shaderResources;

        for (auto resourceId: shader.shaderResourceIds) {
            shaderResources.push_back(engineNode->getShaderResource(resourceId));
        }

        hitShaderContainer.shaderResources = shaderResources;
        hitShaderContainer.hitShader = engineNode->getShader(shader.shaderId);
        pipelineHitShaders.push_back({hitShaderContainer, shader.shaderId});
    }

    std::vector<PierceShaderPackage> pipelinePierceShaders;
    for (const auto &shader: pipelineDescription.pierceShaders) {
        PierceShaderContainer pierceShaderContainer;
        std::vector<ShaderResource *> shaderResources;

        for (auto resourceId: shader.shaderResourceIds) {
            shaderResources.push_back(engineNode->getShaderResource(resourceId));
        }

        pierceShaderContainer.shaderResources = shaderResources;
        pierceShaderContainer.pierceShader = engineNode->getShader(shader.shaderId);
        pipelinePierceShaders.push_back({pierceShaderContainer, shader.shaderId});
    }

    std::vector<MissShaderPackage> pipelineMissShaders;
    for (const auto &shader: pipelineDescription.missShaders) {
        MissShaderContainer missShaderContainer;
        std::vector<ShaderResource *> shaderResources;

        for (auto resourceId: shader.shaderResourceIds) {
            shaderResources.push_back(engineNode->getShaderResource(resourceId));
        }

        missShaderContainer.shaderResources = shaderResources;
        missShaderContainer.missShader = engineNode->getShader(shader.shaderId);
        pipelineMissShaders.push_back({missShaderContainer, shader.shaderId});
    }

    // create new pipeline and add bvh, shaders and description
    auto pipeline = std::make_unique<PipelineImplement>(engineNode.get(),
                                                        pipelineDescription.resolutionX,
                                                        pipelineDescription.resolutionY,
                                                        pipelineDescription.cameraPosition,
                                                        pipelineDescription.cameraDirection,
                                                        pipelineDescription.cameraUp,
                                                        pipelineRayGeneratorShaders,
                                                        pipelineOcclusionShaders,
                                                        pipelineHitShaders,
                                                        pipelinePierceShaders,
                                                        pipelineMissShaders, std::move(root));

    auto pipelineId = pipelineIds.extract(pipelineIds.begin()).value();

    // map instances to pipeline
    pipelineToInstanceMap[pipelineId].insert(instanceIds.begin(), instanceIds.end());

    engineNode->storePipelineFragments(std::move(pipeline), pipelineId);

    if (pipelineIds.empty()) {
        pipelineIds.insert(PipelineId{pipelineId.id + 1});
    }

    // broadcast pipeline to all engine nodes
    // TODO

    return PipelineId{pipelineId};
}

bool DataManagementUnitV2::removePipeline(PipelineId id) {
    // TODO: broadcast remove to all nodes;
    auto removed = engineNode->deletePipelineFragment(id);
    if (removed) {
        for (auto instance: pipelineToInstanceMap.at(id)) {
            removePipelineObject(id, instance);
        }

        pipelineIds.insert(id);

        auto iterator = pipelineIds.rbegin();
        unsigned long end = iterator->id - 1;

        unsigned long buffer = iterator->id;
        while (end-- == (++iterator)->id) {
            pipelineIds.erase(PipelineId{buffer});
            buffer = iterator->id;
        }

        return true;
    }
    return false;
}

bool
DataManagementUnitV2::updatePipelineObjects(PipelineId pipelineId, const std::vector<InstanceId> &objectInstanceIDs,
                                            const std::vector<Matrix4x4> &transforms,
                                            const std::vector<ObjectParameter> &objectParameters) {
    if (objectInstanceIDs.size() != transforms.size()) return false;

    for (unsigned long i = 0; i < objectInstanceIDs.size(); i++) {
        if (objectInstanceIds.count(objectInstanceIDs.at(i)) == 1) {
            if (objectInstanceIdDeviceMap[objectInstanceIDs.at(i)].id == deviceId.id) {
                auto instance = engineNode->requestInstanceData(objectInstanceIDs.at(i));
                if (instance == nullptr) continue;
                instance->applyTransform(transforms.at(i));
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
DataManagementUnitV2::updatePipelineShader(PipelineId pipelineId, RayGeneratorShaderId shaderId,
                                           const std::vector<ShaderResourceId> &resourceIds) {
    auto pipeline = engineNode->requestPipelineFragment(pipelineId);

    if (pipeline == nullptr) return false;

    std::vector<ShaderResource *> shaderResources;

    shaderResources.reserve(resourceIds.size());
    for (auto shaderResource: resourceIds) {
        shaderResources.push_back(engineNode->getShaderResource(shaderResource));
    }

    return pipeline->updateShader(shaderId, shaderResources);
}

bool DataManagementUnitV2::updatePipelineShader(PipelineId pipelineId, HitShaderId shaderId,
                                                const std::vector<ShaderResourceId> &resourceIds) {
    auto pipeline = engineNode->requestPipelineFragment(pipelineId);

    if (pipeline == nullptr) return false;

    std::vector<ShaderResource *> shaderResources;

    shaderResources.reserve(resourceIds.size());
    for (auto shaderResource: resourceIds) {
        shaderResources.push_back(engineNode->getShaderResource(shaderResource));
    }

    return pipeline->updateShader(shaderId, shaderResources);
}

bool DataManagementUnitV2::updatePipelineShader(PipelineId pipelineId, OcclusionShaderId shaderId,
                                                const std::vector<ShaderResourceId> &resourceIds) {
    auto pipeline = engineNode->requestPipelineFragment(pipelineId);

    if (pipeline == nullptr) return false;

    std::vector<ShaderResource *> shaderResources;

    shaderResources.reserve(resourceIds.size());
    for (auto shaderResource: resourceIds) {
        shaderResources.push_back(engineNode->getShaderResource(shaderResource));
    }

    return pipeline->updateShader(shaderId, shaderResources);
}

bool DataManagementUnitV2::updatePipelineShader(PipelineId pipelineId, PierceShaderId shaderId,
                                                const std::vector<ShaderResourceId> &resourceIds) {
    auto pipeline = engineNode->requestPipelineFragment(pipelineId);

    if (pipeline == nullptr) return false;

    std::vector<ShaderResource *> shaderResources;

    shaderResources.reserve(resourceIds.size());
    for (auto shaderResource: resourceIds) {
        shaderResources.push_back(engineNode->getShaderResource(shaderResource));
    }

    return pipeline->updateShader(shaderId, shaderResources);
}

bool DataManagementUnitV2::updatePipelineShader(PipelineId pipelineId, MissShaderId shaderId,
                                                const std::vector<ShaderResourceId> &resourceIds) {
    auto pipeline = engineNode->requestPipelineFragment(pipelineId);

    if (pipeline == nullptr) return false;

    std::vector<ShaderResource *> shaderResources;

    shaderResources.reserve(resourceIds.size());
    for (auto shaderResource: resourceIds) {
        shaderResources.push_back(engineNode->getShaderResource(shaderResource));
    }

    return pipeline->updateShader(shaderId, shaderResources);
}

bool DataManagementUnitV2::removePipelineObject(PipelineId pipelineId, InstanceId objectInstanceId) {
    if (objectInstanceIdDeviceMap.count(objectInstanceId) == 1) {
        if (objectInstanceIdDeviceMap[objectInstanceId].id == deviceId.id) {
            auto pipeline = engineNode->requestPipelineFragment(pipelineId);
            auto geometry = pipeline->getGeometry();
            auto instance = engineNode->requestInstanceData(objectInstanceId);
            std::vector<Object *> remove{instance};
            DBVHv2::removeObjects(*geometry, remove);
            return engineNode->deleteInstanceDataFragment(objectInstanceId);
        } else {
            // TODO: delete instance on other nodes
        }
    }
    return false;
}

bool DataManagementUnitV2::removePipelineShader(PipelineId pipelineId, RayGeneratorShaderId shaderInstanceId) {
    auto pipeline = engineNode->requestPipelineFragment(pipelineId);

    if (pipeline == nullptr) return false;

    return pipeline->removeShader(shaderInstanceId);
}

bool DataManagementUnitV2::removePipelineShader(PipelineId pipelineId, HitShaderId shaderInstanceId) {
    auto pipeline = engineNode->requestPipelineFragment(pipelineId);

    if (pipeline == nullptr) return false;

    return pipeline->removeShader(shaderInstanceId);
}

bool DataManagementUnitV2::removePipelineShader(PipelineId pipelineId, OcclusionShaderId shaderInstanceId) {
    auto pipeline = engineNode->requestPipelineFragment(pipelineId);

    if (pipeline == nullptr) return false;

    return pipeline->removeShader(shaderInstanceId);
}

bool DataManagementUnitV2::removePipelineShader(PipelineId pipelineId, PierceShaderId shaderInstanceId) {
    auto pipeline = engineNode->requestPipelineFragment(pipelineId);

    if (pipeline == nullptr) return false;

    return pipeline->removeShader(shaderInstanceId);
}

bool DataManagementUnitV2::removePipelineShader(PipelineId pipelineId, MissShaderId shaderInstanceId) {
    auto pipeline = engineNode->requestPipelineFragment(pipelineId);

    if (pipeline == nullptr) return false;

    return pipeline->removeShader(shaderInstanceId);
}

bool
DataManagementUnitV2::bindGeometryToPipeline(PipelineId pipelineId, const std::vector<ObjectId> &objectIDs,
                                             const std::vector<Matrix4x4> &transforms,
                                             const std::vector<ObjectParameter> &objectParameters,
                                             std::vector<InstanceId> &instanceIDs) {
    auto pipeline = engineNode->requestPipelineFragment(pipelineId);
    if (objectIDs.size() != transforms.size() || pipeline == nullptr) return false;

    auto geometry = pipeline->getGeometry();

    std::vector<Object *> instances;
    std::vector<InstanceId> instanceIds;

    for (unsigned long i = 0; i < objectIDs.size(); i++) {
        if (objectIdDeviceMap.count(objectIDs.at(i)) == 1) {
            if (objectIdDeviceMap[objectIDs.at(i)].id == deviceId.id) {
                auto buffer = engineNode->requestBaseData(objectIDs.at(i))->getCapsule();
                auto capsule = ObjectCapsule{ObjectId{i}, buffer.boundingBox, buffer.cost};

                // create instances of objects
                auto instance = std::make_unique<Instance>(*engineNode, capsule);
                instance->applyTransform(transforms.at(i));
                instances.push_back(instance.get());

                // manage instance ids
                auto instanceId = objectInstanceIds.extract(objectInstanceIds.begin()).value();
                if (objectInstanceIds.empty()) {
                    objectInstanceIds.insert(InstanceId{instanceId.id + 1});
                }
                instanceIDs.push_back(instanceId);
                objectToInstanceMap[objectIDs.at((i))].insert(instanceId);
                instanceIds.push_back(instanceId);

                // add instances to engine node
                // TODO spread over nodes
                engineNode->storeInstanceDataFragments(std::move(instance), instanceId);

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

    DBVHv2::addObjects(*geometry, instances);

    return true;
}

bool DataManagementUnitV2::bindShaderToPipeline(PipelineId pipelineId, RayGeneratorShaderId shaderId,
                                                const std::vector<ShaderResourceId> &resourceIds) {
    auto pipeline = engineNode->requestPipelineFragment(pipelineId);
    auto shader = engineNode->getShader(shaderId);

    if (pipeline == nullptr || shader == nullptr) return false;

    std::vector<ShaderResource *> shaderResources;

    shaderResources.reserve(resourceIds.size());
    for (auto shaderResource: resourceIds) {
        shaderResources.push_back(engineNode->getShaderResource(shaderResource));
    }

    RayGeneratorShaderContainer rayGeneratorShaderContainer = {shader, shaderResources};

    pipeline->addShader(shaderId, rayGeneratorShaderContainer);

    return true;
}

bool DataManagementUnitV2::bindShaderToPipeline(PipelineId pipelineId, HitShaderId shaderId,
                                                const std::vector<ShaderResourceId> &resourceIds) {
    auto pipeline = engineNode->requestPipelineFragment(pipelineId);
    auto shader = engineNode->getShader(shaderId);

    if (pipeline == nullptr || shader == nullptr) return false;

    std::vector<ShaderResource *> shaderResources;

    shaderResources.reserve(resourceIds.size());
    for (auto shaderResource: resourceIds) {
        shaderResources.push_back(engineNode->getShaderResource(shaderResource));
    }

    HitShaderContainer hitShaderContainer = {shader, shaderResources};

    pipeline->addShader(shaderId, hitShaderContainer);

    return true;
}

bool DataManagementUnitV2::bindShaderToPipeline(PipelineId pipelineId, OcclusionShaderId shaderId,
                                                const std::vector<ShaderResourceId> &resourceIds) {
    auto pipeline = engineNode->requestPipelineFragment(pipelineId);
    auto shader = engineNode->getShader(shaderId);

    if (pipeline == nullptr || shader == nullptr) return false;

    std::vector<ShaderResource *> shaderResources;

    shaderResources.reserve(resourceIds.size());
    for (auto shaderResource: resourceIds) {
        shaderResources.push_back(engineNode->getShaderResource(shaderResource));
    }

    OcclusionShaderContainer occlusionShaderContainer = {shader, shaderResources};

    pipeline->addShader(shaderId, occlusionShaderContainer);

    return true;
}

bool DataManagementUnitV2::bindShaderToPipeline(PipelineId pipelineId, PierceShaderId shaderId,
                                                const std::vector<ShaderResourceId> &resourceIds) {
    auto pipeline = engineNode->requestPipelineFragment(pipelineId);
    auto shader = engineNode->getShader(shaderId);

    if (pipeline == nullptr || shader == nullptr) return false;

    std::vector<ShaderResource *> shaderResources;

    shaderResources.reserve(resourceIds.size());
    for (auto shaderResource: resourceIds) {
        shaderResources.push_back(engineNode->getShaderResource(shaderResource));
    }

    PierceShaderContainer pierceShaderContainer = {shader, shaderResources};

    pipeline->addShader(shaderId, pierceShaderContainer);

    return true;
}

bool DataManagementUnitV2::bindShaderToPipeline(PipelineId pipelineId, MissShaderId shaderId,
                                                const std::vector<ShaderResourceId> &resourceIds) {
    auto pipeline = engineNode->requestPipelineFragment(pipelineId);
    auto shader = engineNode->getShader(shaderId);

    if (pipeline == nullptr || shader == nullptr) return false;

    std::vector<ShaderResource *> shaderResources;

    shaderResources.reserve(resourceIds.size());
    for (auto shaderResource: resourceIds) {
        shaderResources.push_back(engineNode->getShaderResource(shaderResource));
    }

    MissShaderContainer missShaderContainer = {shader, shaderResources};

    pipeline->addShader(shaderId, missShaderContainer);

    return true;
}

ObjectId DataManagementUnitV2::addObject(const Object &object) {
    auto buffer = objectIds.extract(objectIds.begin()).value();

    objectIdDeviceMap[buffer] = deviceId;

    // TODO: spread over nodes
    auto clone = object.clone();
    engineNode->storeBaseDataFragments(std::move(clone), buffer);

    if (objectIds.empty()) {
        objectIds.insert(ObjectId{buffer.id + 1});
    }

    return buffer;
}

bool DataManagementUnitV2::removeObject(ObjectId id) {
    if (!engineNode->deleteBaseDataFragment(id)) return false;

    // remove instances
    for (auto instanceId: objectToInstanceMap.at(id)) {
        for (auto &pipelineInstances: pipelineToInstanceMap) {
            if (pipelineInstances.second.count(instanceId) != 0) {
                removePipelineObject(pipelineInstances.first, instanceId);
                pipelineInstances.second.erase(instanceId);
                break;
            }
        }
    }

    objectIdDeviceMap.erase(id);

    objectIds.insert(id);

    auto iterator = objectIds.rbegin();
    unsigned long end = iterator->id - 1;

    unsigned long buffer = iterator->id;
    while (end-- == (++iterator)->id) {
        objectIds.erase(ObjectId{buffer});
        buffer = iterator->id;
    }

    return true;
}

bool DataManagementUnitV2::updateObject(ObjectId id, const Object &object) {
    if (engineNode->deleteBaseDataFragment(id)) return false;
    auto clone = object.clone();
    engineNode->storeBaseDataFragments(std::move(clone), id);
    return true;
}

HitShaderId DataManagementUnitV2::addShader(const HitShader &shader) {
    auto shaderId = hitShaderIds.extract(hitShaderIds.begin()).value();

    auto clone = shader.clone();
    engineNode->addShader(shaderId, std::move(clone));

    if (hitShaderIds.empty()) {
        hitShaderIds.insert(HitShaderId{shaderId.id + 1});
    }

    return shaderId;
}

MissShaderId DataManagementUnitV2::addShader(const MissShader &shader) {
    auto shaderId = missShaderIds.extract(missShaderIds.begin()).value();

    auto clone = shader.clone();
    engineNode->addShader(shaderId, std::move(clone));

    if (missShaderIds.empty()) {
        missShaderIds.insert(MissShaderId{shaderId.id + 1});
    }

    return shaderId;
}

OcclusionShaderId DataManagementUnitV2::addShader(const OcclusionShader &shader) {
    auto shaderId = occlusionShaderIds.extract(occlusionShaderIds.begin()).value();

    auto clone = shader.clone();
    engineNode->addShader(shaderId, std::move(clone));

    if (occlusionShaderIds.empty()) {
        occlusionShaderIds.insert(OcclusionShaderId{shaderId.id + 1});
    }

    return shaderId;
}

PierceShaderId DataManagementUnitV2::addShader(const PierceShader &shader) {
    auto shaderId = pierceShaderIds.extract(pierceShaderIds.begin()).value();

    auto clone = shader.clone();
    engineNode->addShader(shaderId, std::move(clone));

    if (pierceShaderIds.empty()) {
        pierceShaderIds.insert(PierceShaderId{shaderId.id + 1});
    }

    return shaderId;
}

RayGeneratorShaderId DataManagementUnitV2::addShader(const RayGeneratorShader &shader) {
    auto shaderId = rayGeneratorShaderIds.extract(rayGeneratorShaderIds.begin()).value();

    auto clone = shader.clone();
    engineNode->addShader(shaderId, std::move(clone));

    if (rayGeneratorShaderIds.empty()) {
        rayGeneratorShaderIds.insert(RayGeneratorShaderId{shaderId.id + 1});
    }

    return shaderId;
}

bool DataManagementUnitV2::removeShader(RayGeneratorShaderId id) {
    if (!engineNode->deleteShader(id)) return false;
    // TODO: remove shader from pipelines

    rayGeneratorShaderIds.insert(id);

    auto iterator = rayGeneratorShaderIds.rbegin();
    unsigned long end = iterator->id - 1;

    unsigned long buffer = iterator->id;
    while (end-- == (++iterator)->id) {
        rayGeneratorShaderIds.erase(RayGeneratorShaderId{buffer});
        buffer = iterator->id;
    }

    return true;
}

bool DataManagementUnitV2::removeShader(HitShaderId id) {
    if (!engineNode->deleteShader(id)) return false;
    // TODO: remove shader from pipelines

    hitShaderIds.insert(id);

    auto iterator = hitShaderIds.rbegin();
    unsigned long end = iterator->id - 1;

    unsigned long buffer = iterator->id;
    while (end-- == (++iterator)->id) {
        hitShaderIds.erase(HitShaderId{buffer});
        buffer = iterator->id;
    }

    return true;
}

bool DataManagementUnitV2::removeShader(OcclusionShaderId id) {
    if (!engineNode->deleteShader(id)) return false;
    // TODO: remove shader from pipelines

    occlusionShaderIds.insert(id);

    auto iterator = occlusionShaderIds.rbegin();
    unsigned long end = iterator->id - 1;

    unsigned long buffer = iterator->id;
    while (end-- == (++iterator)->id) {
        occlusionShaderIds.erase(OcclusionShaderId{buffer});
        buffer = iterator->id;
    }

    return true;
}

bool DataManagementUnitV2::removeShader(PierceShaderId id) {
    if (!engineNode->deleteShader(id)) return false;
    // TODO: remove shader from pipelines

    pierceShaderIds.insert(id);

    auto iterator = pierceShaderIds.rbegin();
    unsigned long end = iterator->id - 1;

    unsigned long buffer = iterator->id;
    while (end-- == (++iterator)->id) {
        pierceShaderIds.erase(PierceShaderId{buffer});
        buffer = iterator->id;
    }

    return true;
}

bool DataManagementUnitV2::removeShader(MissShaderId id) {
    if (!engineNode->deleteShader(id)) return false;
    // TODO: remove shader from pipelines

    missShaderIds.insert(id);

    auto iterator = missShaderIds.rbegin();
    unsigned long end = iterator->id - 1;

    unsigned long buffer = iterator->id;
    while (end-- == (++iterator)->id) {
        missShaderIds.erase(MissShaderId{buffer});
        buffer = iterator->id;
    }

    return true;
}

ShaderResourceId DataManagementUnitV2::addShaderResource(const ShaderResource &resource) {
    auto shaderId = shaderResourceIds.extract(shaderResourceIds.begin()).value();

    auto clone = resource.clone();
    engineNode->storeShaderResource(std::move(clone), shaderId);

    if (shaderResourceIds.empty()) {
        shaderResourceIds.insert(ShaderResourceId{shaderId.id + 1});
    }

    return shaderId;
}

bool DataManagementUnitV2::removeShaderResource(ShaderResourceId id) {
    if (!engineNode->deleteShaderResource(id)) return false;

    shaderResourceIds.insert(id);

    auto iterator = shaderResourceIds.rbegin();
    unsigned long end = iterator->id - 1;

    unsigned long buffer = iterator->id;
    while (end-- == (++iterator)->id) {
        shaderResourceIds.erase(ShaderResourceId{buffer});
        buffer = iterator->id;
    }

    return true;
}

int DataManagementUnitV2::runPipeline(PipelineId id) {
    engineNode->runPipeline(id);
    return 0;
}

int DataManagementUnitV2::runAllPipelines() {
    engineNode->runPipelines();
    return 0;
}

void
DataManagementUnitV2::updatePipelineCamera(PipelineId id, int resolutionX, int resolutionY,
                                           const Vector3D &cameraPosition, const Vector3D &cameraDirection,
                                           const Vector3D &cameraUp) {
    auto pipeline = engineNode->requestPipelineFragment(id);
    pipeline->setResolution(resolutionX, resolutionY);
    pipeline->setCamera(cameraPosition, cameraDirection, cameraUp);
}

Texture *DataManagementUnitV2::getPipelineResult(PipelineId id) {
    auto pipeline = engineNode->requestPipelineFragment(id);
    return pipeline->getResult();
}

DeviceId DataManagementUnitV2::getDeviceId() {
    // TODO
    return DeviceId{0};
}

std::unique_ptr<Object> DataManagementUnitV2::getBaseDataFragment(ObjectId id) {
    if (objectIdDeviceMap.count(id) == 0) return nullptr;
    // TODO: request object with id from other device
    return nullptr;
}

std::unique_ptr<Instance> DataManagementUnitV2::getInstanceDataFragment(InstanceId id) {
    if (objectInstanceIdDeviceMap.count(id) == 0) return nullptr;
    // TODO: request object with id from other device
    return nullptr;
}
