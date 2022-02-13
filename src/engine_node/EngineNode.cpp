//
// Created by Sebastian on 28.10.2021.
//

#include "EngineNode.h"
#include <functional>

namespace {
    template <class ID>
    void removeId(std::set<ID> &set, ID id){
        static_assert(std::is_base_of<GenericId, ID>::value, "ID must inherit from GenericId");
        set.insert(id);

        auto iterator = set.rbegin();
        unsigned long end = iterator->id - 1;

        unsigned long buffer = iterator->id;
        while (end-- == (++iterator)->id) {
            set.erase({buffer});
            buffer = iterator->id;
        }
    }
}

EngineNode::EngineNode() : deviceId(getDeviceId()) {
    dmu = std::make_unique<DataManagementUnitV2>();
    pipelinePool = std::make_unique<PipelinePool>();

    objectIds.insert(ObjectId{0});
    rayGeneratorShaderIds.insert(RayGeneratorShaderId{0});
    hitShaderIds.insert(HitShaderId{0});
    occlusionShaderIds.insert(OcclusionShaderId{0});
    pierceShaderIds.insert(PierceShaderId{0});
    missShaderIds.insert(MissShaderId{0});
    pipelineIds.insert(PipelineId{0});
    objectInstanceIds.insert(InstanceId{0});
    shaderResourceIds.insert(ShaderResourceId{0});
}

EngineNode::~EngineNode() = default;

PipelineId EngineNode::createPipeline(PipelineDescription &pipelineDescription) {
    std::vector<Intersectable *> instances;
    std::vector<InstanceId> instanceIds;

    // pull all objects required to create the pipeline
    // only requires id, box and cost
    int c = 0;
    for (auto i: pipelineDescription.objectIDs) {
        if (objectIdDeviceMap.count(i) == 1) {
            if (objectIdDeviceMap[i].id == deviceId.id) {
                auto buffer = dmu->getBaseDataFragment(i)->getCapsule();
                auto capsule = ObjectCapsule{ObjectId{i.id}, buffer.boundingBox, buffer.cost};

                // create instances of objects
                auto instance = std::make_unique<Instance>(dmu.get(), capsule);
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
                dmu->storeInstanceDataFragments(std::move(instance), instanceId);

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
            shaderResources.push_back(pipelinePool->getShaderResource(resourceId));
        }

        rayGeneratorShaderContainer.shaderResources = shaderResources;
        rayGeneratorShaderContainer.rayGeneratorShader = pipelinePool->getShader(shader.shaderId);
        pipelineRayGeneratorShaders.push_back({rayGeneratorShaderContainer, shader.shaderId});
    }

    std::vector<OcclusionShaderPackage> pipelineOcclusionShaders;
    for (const auto &shader: pipelineDescription.occlusionShaders) {
        OcclusionShaderContainer occlusionShaderContainer;
        std::vector<ShaderResource *> shaderResources;

        for (auto resourceId: shader.shaderResourceIds) {
            shaderResources.push_back(pipelinePool->getShaderResource(resourceId));
        }

        occlusionShaderContainer.shaderResources = shaderResources;
        occlusionShaderContainer.occlusionShader = pipelinePool->getShader(shader.shaderId);
        pipelineOcclusionShaders.push_back({occlusionShaderContainer, shader.shaderId});
    }

    std::vector<HitShaderPackage> pipelineHitShaders;
    for (const auto &shader: pipelineDescription.hitShaders) {
        HitShaderContainer hitShaderContainer;
        std::vector<ShaderResource *> shaderResources;

        for (auto resourceId: shader.shaderResourceIds) {
            shaderResources.push_back(pipelinePool->getShaderResource(resourceId));
        }

        hitShaderContainer.shaderResources = shaderResources;
        hitShaderContainer.hitShader = pipelinePool->getShader(shader.shaderId);
        pipelineHitShaders.push_back({hitShaderContainer, shader.shaderId});
    }

    std::vector<PierceShaderPackage> pipelinePierceShaders;
    for (const auto &shader: pipelineDescription.pierceShaders) {
        PierceShaderContainer pierceShaderContainer;
        std::vector<ShaderResource *> shaderResources;

        for (auto resourceId: shader.shaderResourceIds) {
            shaderResources.push_back(pipelinePool->getShaderResource(resourceId));
        }

        pierceShaderContainer.shaderResources = shaderResources;
        pierceShaderContainer.pierceShader = pipelinePool->getShader(shader.shaderId);
        pipelinePierceShaders.push_back({pierceShaderContainer, shader.shaderId});
    }

    std::vector<MissShaderPackage> pipelineMissShaders;
    for (const auto &shader: pipelineDescription.missShaders) {
        MissShaderContainer missShaderContainer;
        std::vector<ShaderResource *> shaderResources;

        for (auto resourceId: shader.shaderResourceIds) {
            shaderResources.push_back(pipelinePool->getShaderResource(resourceId));
        }

        missShaderContainer.shaderResources = shaderResources;
        missShaderContainer.missShader = pipelinePool->getShader(shader.shaderId);
        pipelineMissShaders.push_back({missShaderContainer, shader.shaderId});
    }

    // create new pipeline and add bvh, shaders and description
    auto pipeline = std::make_unique<PipelineImplement>(dmu.get(),
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

    pipelinePool->storePipelineFragments(std::move(pipeline), pipelineId);

    if (pipelineIds.empty()) {
        pipelineIds.insert(PipelineId{pipelineId.id + 1});
    }

    // broadcast pipeline to all engine nodes
    // TODO

    return PipelineId{pipelineId};
}

bool EngineNode::removePipeline(PipelineId id) {
    // TODO: broadcast remove to all nodes;
    auto removed = pipelinePool->deletePipelineFragment(id);
    if (removed) {
        for (auto instance: pipelineToInstanceMap.at(id)) {
            removePipelineObject(id, instance);
        }
        pipelineToInstanceMap.erase(id);
        removeId(pipelineIds, id);
        return true;
    }
    return false;
}

bool
EngineNode::updatePipelineObjects(PipelineId pipelineId, const std::vector<InstanceId> &objectInstanceIDs,
                                  const std::vector<Matrix4x4> &transforms,
                                  const std::vector<ObjectParameter> &objectParameters) {
    if (objectInstanceIDs.size() != transforms.size()) return false;

    for (unsigned long i = 0; i < objectInstanceIDs.size(); i++) {
        if (objectInstanceIds.count(objectInstanceIDs.at(i)) == 1) {
            if (objectInstanceIdDeviceMap[objectInstanceIDs.at(i)].id == deviceId.id) {
                auto instance = dmu->getInstanceDataFragment(objectInstanceIDs.at(i));
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
EngineNode::updatePipelineShader(PipelineId pipelineId, RayGeneratorShaderId shaderId,
                                 const std::vector<ShaderResourceId> &resourceIds) {
    auto pipeline = pipelinePool->getPipelineFragment(pipelineId);

    if (pipeline == nullptr) return false;

    std::vector<ShaderResource *> shaderResources;

    shaderResources.reserve(resourceIds.size());
    for (auto shaderResource: resourceIds) {
        shaderResources.push_back(pipelinePool->getShaderResource(shaderResource));
    }

    return pipeline->updateShader(shaderId, shaderResources);
}

bool EngineNode::updatePipelineShader(PipelineId pipelineId, HitShaderId shaderId,
                                      const std::vector<ShaderResourceId> &resourceIds) {
    auto pipeline = pipelinePool->getPipelineFragment(pipelineId);

    if (pipeline == nullptr) return false;

    std::vector<ShaderResource *> shaderResources;

    shaderResources.reserve(resourceIds.size());
    for (auto shaderResource: resourceIds) {
        shaderResources.push_back(pipelinePool->getShaderResource(shaderResource));
    }

    return pipeline->updateShader(shaderId, shaderResources);
}

bool EngineNode::updatePipelineShader(PipelineId pipelineId, OcclusionShaderId shaderId,
                                      const std::vector<ShaderResourceId> &resourceIds) {
    auto pipeline = pipelinePool->getPipelineFragment(pipelineId);

    if (pipeline == nullptr) return false;

    std::vector<ShaderResource *> shaderResources;

    shaderResources.reserve(resourceIds.size());
    for (auto shaderResource: resourceIds) {
        shaderResources.push_back(pipelinePool->getShaderResource(shaderResource));
    }

    return pipeline->updateShader(shaderId, shaderResources);
}

bool EngineNode::updatePipelineShader(PipelineId pipelineId, PierceShaderId shaderId,
                                      const std::vector<ShaderResourceId> &resourceIds) {
    auto pipeline = pipelinePool->getPipelineFragment(pipelineId);

    if (pipeline == nullptr) return false;

    std::vector<ShaderResource *> shaderResources;

    shaderResources.reserve(resourceIds.size());
    for (auto shaderResource: resourceIds) {
        shaderResources.push_back(pipelinePool->getShaderResource(shaderResource));
    }

    return pipeline->updateShader(shaderId, shaderResources);
}

bool EngineNode::updatePipelineShader(PipelineId pipelineId, MissShaderId shaderId,
                                      const std::vector<ShaderResourceId> &resourceIds) {
    auto pipeline = pipelinePool->getPipelineFragment(pipelineId);

    if (pipeline == nullptr) return false;

    std::vector<ShaderResource *> shaderResources;

    shaderResources.reserve(resourceIds.size());
    for (auto shaderResource: resourceIds) {
        shaderResources.push_back(pipelinePool->getShaderResource(shaderResource));
    }

    return pipeline->updateShader(shaderId, shaderResources);
}

bool EngineNode::removePipelineObject(PipelineId pipelineId, InstanceId objectInstanceId) {
    if (objectInstanceIdDeviceMap.count(objectInstanceId) == 1) {
        if (objectInstanceIdDeviceMap[objectInstanceId] == deviceId) {
            auto pipeline = pipelinePool->getPipelineFragment(pipelineId);
            auto geometry = pipeline->getGeometry();
            auto instance = dmu->getInstanceDataFragment(objectInstanceId);
            std::vector<Intersectable *> remove{instance};
            DBVHv2::removeObjects(*geometry, remove);
            removeId(objectInstanceIds, objectInstanceId);
            pipelineToInstanceMap.at(pipelineId).erase(objectInstanceId);
            objectInstanceIdDeviceMap.erase(objectInstanceId);
            return dmu->deleteInstanceDataFragment(objectInstanceId);
        } else {
            // TODO: delete instance on other nodes
        }
    }
    return false;
}

bool EngineNode::removePipelineShader(PipelineId pipelineId, RayGeneratorShaderId shaderInstanceId) {
    auto pipeline = pipelinePool->getPipelineFragment(pipelineId);

    if (pipeline == nullptr) return false;

    return pipeline->removeShader(shaderInstanceId);
}

bool EngineNode::removePipelineShader(PipelineId pipelineId, HitShaderId shaderInstanceId) {
    auto pipeline = pipelinePool->getPipelineFragment(pipelineId);

    if (pipeline == nullptr) return false;

    return pipeline->removeShader(shaderInstanceId);
}

bool EngineNode::removePipelineShader(PipelineId pipelineId, OcclusionShaderId shaderInstanceId) {
    auto pipeline = pipelinePool->getPipelineFragment(pipelineId);

    if (pipeline == nullptr) return false;

    return pipeline->removeShader(shaderInstanceId);
}

bool EngineNode::removePipelineShader(PipelineId pipelineId, PierceShaderId shaderInstanceId) {
    auto pipeline = pipelinePool->getPipelineFragment(pipelineId);

    if (pipeline == nullptr) return false;

    return pipeline->removeShader(shaderInstanceId);
}

bool EngineNode::removePipelineShader(PipelineId pipelineId, MissShaderId shaderInstanceId) {
    auto pipeline = pipelinePool->getPipelineFragment(pipelineId);

    if (pipeline == nullptr) return false;

    return pipeline->removeShader(shaderInstanceId);
}

bool
EngineNode::bindGeometryToPipeline(PipelineId pipelineId, const std::vector<ObjectId> &objectIDs,
                                   const std::vector<Matrix4x4> &transforms,
                                   const std::vector<ObjectParameter> &objectParameters,
                                   std::vector<InstanceId> &instanceIDs) {
    auto pipeline = pipelinePool->getPipelineFragment(pipelineId);
    if (objectIDs.size() != transforms.size() || pipeline == nullptr) return false;

    auto geometry = pipeline->getGeometry();

    std::vector<Intersectable *> instances;
    std::vector<InstanceId> instanceIds;

    for (unsigned long i = 0; i < objectIDs.size(); i++) {
        if (objectIdDeviceMap.count(objectIDs.at(i)) == 1) {
            if (objectIdDeviceMap[objectIDs.at(i)].id == deviceId.id) {
                auto buffer = dmu->getBaseDataFragment(objectIDs.at(i))->getCapsule();
                auto capsule = ObjectCapsule{ObjectId{i}, buffer.boundingBox, buffer.cost};

                // create instances of objects
                auto instance = std::make_unique<Instance>(dmu.get(), capsule);
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
                dmu->storeInstanceDataFragments(std::move(instance), instanceId);

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

bool EngineNode::bindShaderToPipeline(PipelineId pipelineId, RayGeneratorShaderId shaderId,
                                      const std::vector<ShaderResourceId> &resourceIds) {
    auto pipeline = pipelinePool->getPipelineFragment(pipelineId);
    auto shader = pipelinePool->getShader(shaderId);

    if (pipeline == nullptr || shader == nullptr) return false;

    std::vector<ShaderResource *> shaderResources;

    shaderResources.reserve(resourceIds.size());
    for (auto shaderResource: resourceIds) {
        shaderResources.push_back(pipelinePool->getShaderResource(shaderResource));
    }

    RayGeneratorShaderContainer rayGeneratorShaderContainer = {shader, shaderResources};

    pipeline->addShader(shaderId, rayGeneratorShaderContainer);

    return true;
}

bool EngineNode::bindShaderToPipeline(PipelineId pipelineId, HitShaderId shaderId,
                                      const std::vector<ShaderResourceId> &resourceIds) {
    auto pipeline = pipelinePool->getPipelineFragment(pipelineId);
    auto shader = pipelinePool->getShader(shaderId);

    if (pipeline == nullptr || shader == nullptr) return false;

    std::vector<ShaderResource *> shaderResources;

    shaderResources.reserve(resourceIds.size());
    for (auto shaderResource: resourceIds) {
        shaderResources.push_back(pipelinePool->getShaderResource(shaderResource));
    }

    HitShaderContainer hitShaderContainer = {shader, shaderResources};

    pipeline->addShader(shaderId, hitShaderContainer);

    return true;
}

bool EngineNode::bindShaderToPipeline(PipelineId pipelineId, OcclusionShaderId shaderId,
                                      const std::vector<ShaderResourceId> &resourceIds) {
    auto pipeline = pipelinePool->getPipelineFragment(pipelineId);
    auto shader = pipelinePool->getShader(shaderId);

    if (pipeline == nullptr || shader == nullptr) return false;

    std::vector<ShaderResource *> shaderResources;

    shaderResources.reserve(resourceIds.size());
    for (auto shaderResource: resourceIds) {
        shaderResources.push_back(pipelinePool->getShaderResource(shaderResource));
    }

    OcclusionShaderContainer occlusionShaderContainer = {shader, shaderResources};

    pipeline->addShader(shaderId, occlusionShaderContainer);

    return true;
}

bool EngineNode::bindShaderToPipeline(PipelineId pipelineId, PierceShaderId shaderId,
                                      const std::vector<ShaderResourceId> &resourceIds) {
    auto pipeline = pipelinePool->getPipelineFragment(pipelineId);
    auto shader = pipelinePool->getShader(shaderId);

    if (pipeline == nullptr || shader == nullptr) return false;

    std::vector<ShaderResource *> shaderResources;

    shaderResources.reserve(resourceIds.size());
    for (auto shaderResource: resourceIds) {
        shaderResources.push_back(pipelinePool->getShaderResource(shaderResource));
    }

    PierceShaderContainer pierceShaderContainer = {shader, shaderResources};

    pipeline->addShader(shaderId, pierceShaderContainer);

    return true;
}

bool EngineNode::bindShaderToPipeline(PipelineId pipelineId, MissShaderId shaderId,
                                      const std::vector<ShaderResourceId> &resourceIds) {
    auto pipeline = pipelinePool->getPipelineFragment(pipelineId);
    auto shader = pipelinePool->getShader(shaderId);

    if (pipeline == nullptr || shader == nullptr) return false;

    std::vector<ShaderResource *> shaderResources;

    shaderResources.reserve(resourceIds.size());
    for (auto shaderResource: resourceIds) {
        shaderResources.push_back(pipelinePool->getShaderResource(shaderResource));
    }

    MissShaderContainer missShaderContainer = {shader, shaderResources};

    pipeline->addShader(shaderId, missShaderContainer);

    return true;
}

ObjectId EngineNode::addObject(const Intersectable &object) {
    auto buffer = objectIds.extract(objectIds.begin()).value();

    objectIdDeviceMap[buffer] = deviceId;

    // TODO: spread over nodes
    auto clone = object.clone();
    dmu->storeBaseDataFragments(std::move(clone), buffer);

    if (objectIds.empty()) {
        objectIds.insert(ObjectId{buffer.id + 1});
    }

    return buffer;
}

bool EngineNode::removeObject(ObjectId id) {
    if (!dmu->deleteBaseDataFragment(id)) return false;

    // remove instances
    for (auto instanceId: objectToInstanceMap.at(id)) {
        for (auto &pipelineInstances: pipelineToInstanceMap) {
            if (pipelineInstances.second.count(instanceId) != 0) {
                removePipelineObject(pipelineInstances.first, instanceId);
                break;
            }
        }
    }

    objectIdDeviceMap.erase(id);
    removeId(objectIds, id);
    return true;
}

bool EngineNode::updateObject(ObjectId id, const Intersectable &object) {
    if (dmu->deleteBaseDataFragment(id)) return false;
    auto clone = object.clone();
    dmu->storeBaseDataFragments(std::move(clone), id);
    return true;
}

HitShaderId EngineNode::addShader(const HitShader &shader) {
    auto shaderId = hitShaderIds.extract(hitShaderIds.begin()).value();

    auto clone = shader.clone();
    pipelinePool->addShader(shaderId, std::move(clone));

    if (hitShaderIds.empty()) {
        hitShaderIds.insert(HitShaderId{shaderId.id + 1});
    }

    return shaderId;
}

MissShaderId EngineNode::addShader(const MissShader &shader) {
    auto shaderId = missShaderIds.extract(missShaderIds.begin()).value();

    auto clone = shader.clone();
    pipelinePool->addShader(shaderId, std::move(clone));

    if (missShaderIds.empty()) {
        missShaderIds.insert(MissShaderId{shaderId.id + 1});
    }

    return shaderId;
}

OcclusionShaderId EngineNode::addShader(const OcclusionShader &shader) {
    auto shaderId = occlusionShaderIds.extract(occlusionShaderIds.begin()).value();

    auto clone = shader.clone();
    pipelinePool->addShader(shaderId, std::move(clone));

    if (occlusionShaderIds.empty()) {
        occlusionShaderIds.insert(OcclusionShaderId{shaderId.id + 1});
    }

    return shaderId;
}

PierceShaderId EngineNode::addShader(const PierceShader &shader) {
    auto shaderId = pierceShaderIds.extract(pierceShaderIds.begin()).value();

    auto clone = shader.clone();
    pipelinePool->addShader(shaderId, std::move(clone));

    if (pierceShaderIds.empty()) {
        pierceShaderIds.insert(PierceShaderId{shaderId.id + 1});
    }

    return shaderId;
}

RayGeneratorShaderId EngineNode::addShader(const RayGeneratorShader &shader) {
    auto shaderId = rayGeneratorShaderIds.extract(rayGeneratorShaderIds.begin()).value();

    auto clone = shader.clone();
    pipelinePool->addShader(shaderId, std::move(clone));

    if (rayGeneratorShaderIds.empty()) {
        rayGeneratorShaderIds.insert(RayGeneratorShaderId{shaderId.id + 1});
    }

    return shaderId;
}

bool EngineNode::removeShader(RayGeneratorShaderId id) {
    if (!pipelinePool->deleteShader(id)) return false;
    removeId(rayGeneratorShaderIds, id);
    return true;
}

bool EngineNode::removeShader(HitShaderId id) {
    if (!pipelinePool->deleteShader(id)) return false;
    removeId(hitShaderIds, id);
    return true;
}

bool EngineNode::removeShader(OcclusionShaderId id) {
    if (!pipelinePool->deleteShader(id)) return false;
    removeId(occlusionShaderIds, id);
    return true;
}

bool EngineNode::removeShader(PierceShaderId id) {
    if (!pipelinePool->deleteShader(id)) return false;
    removeId(pierceShaderIds, id);
    return true;
}

bool EngineNode::removeShader(MissShaderId id) {
    if (!pipelinePool->deleteShader(id)) return false;
    removeId(missShaderIds, id);
    return true;
}

ShaderResourceId EngineNode::addShaderResource(const ShaderResource &resource) {
    auto shaderId = shaderResourceIds.extract(shaderResourceIds.begin()).value();

    auto clone = resource.clone();
    pipelinePool->storeShaderResource(std::move(clone), shaderId);

    if (shaderResourceIds.empty()) {
        shaderResourceIds.insert(ShaderResourceId{shaderId.id + 1});
    }

    return shaderId;
}

bool EngineNode::removeShaderResource(ShaderResourceId id) {
    if (!pipelinePool->deleteShaderResource(id)) return false;
    removeId(shaderResourceIds,  id);
    return true;
}

int EngineNode::runPipeline(PipelineId id) {
    pipelinePool->runPipeline(id);
    return 0;
}

int EngineNode::runAllPipelines() {
    pipelinePool->runPipelines();
    return 0;
}

void
EngineNode::updatePipelineCamera(PipelineId id, int resolutionX, int resolutionY,
                                 const Vector3D &cameraPosition, const Vector3D &cameraDirection,
                                 const Vector3D &cameraUp) {
    auto pipeline = pipelinePool->getPipelineFragment(id);
    pipeline->setResolution(resolutionX, resolutionY);
    pipeline->setCamera(cameraPosition, cameraDirection, cameraUp);
}

Texture *EngineNode::getPipelineResult(PipelineId id) {
    auto pipeline = pipelinePool->getPipelineFragment(id);
    return pipeline->getResult();
}

DeviceId EngineNode::getDeviceId() {
    // TODO
    return DeviceId{0};
}