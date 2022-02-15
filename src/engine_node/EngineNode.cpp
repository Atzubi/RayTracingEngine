//
// Created by Sebastian on 28.10.2021.
//

#include "EngineNode.h"
#include <functional>

namespace {
    template<class ID>
    requires isShaderId<ID>
    bool helpRemoveShader(PipelinePool &pipelinePool, ID id, IdContainer<ID> &ids) {
        if (!pipelinePool.deleteShader(id))
            return false;
        ids.remove(id);
        return true;
    }

    template<class ID, class Shader>
    requires isShaderId<ID> && isShader<Shader>
    ID helpAddShader(const Shader &shader, PipelinePool &pipelinePool, IdContainer<ID> &ids) {
        auto shaderId = ids.next();

        auto clone = shader.clone();
        pipelinePool.addShader(shaderId, std::move(clone));

        return shaderId;
    }

    template<class ID, class Shader>
    requires isShaderId<ID> && isShader<Shader> && correspondsTo<ID, Shader>
    auto getShaderPackages(PipelinePool &pipelinePool, const std::vector<ShaderResourcePackage<ID>> &shaders) {
        std::vector<ShaderPackage<ID, Shader>> shaderPackages;
        for (const auto &shader: shaders) {
            std::vector<ShaderResource *> shaderResources;
            for (auto resourceId: shader.shaderResourceIds) {
                shaderResources.push_back(pipelinePool.getShaderResource(resourceId));
            }
            ShaderContainer<Shader> shaderContainer{pipelinePool.getShader(shader.shaderId), shaderResources};
            shaderPackages.push_back({shaderContainer, shader.shaderId});
        }
        return shaderPackages;
    }
}

void EngineNode::removeInstanceInPipeline(PipelineId pipelineId, InstanceId objectInstanceId) {
    auto pipeline = pipelinePool->getPipelineFragment(pipelineId);
    auto geometry = pipeline->getGeometry();
    auto instance = dmu->getInstanceDataFragment(objectInstanceId);
    std::vector<Intersectable *> remove{instance};
    DBVHv2::removeObjects(*geometry, remove);
}

bool EngineNode::removeInstanceInEngine(PipelineId pipelineId, InstanceId objectInstanceId) {
    objectInstanceIds.remove(objectInstanceId);
    pipelineToInstanceMap.at(pipelineId).erase(objectInstanceId);
    objectInstanceIdDeviceMap.erase(objectInstanceId);
    return dmu->deleteInstanceDataFragment(objectInstanceId);
}

bool EngineNode::updateInstance(InstanceId instanceId, const Matrix4x4 &transform) {
    if (objectInstanceIdDeviceMap.count(instanceId) == 0)
        return false;
    if (objectInstanceIdDeviceMap[instanceId].id == deviceId.id) {
        auto instance = dmu->getInstanceDataFragment(instanceId);
        if (instance) {
            instance->applyTransform(transform);
        }
    } else {
        // TODO: update instances on other nodes
    }
    return true;
}

void EngineNode::createInstances(const std::vector<ObjectId> &objectIDs, const std::vector<Matrix4x4> &transforms,
                                 std::vector<Intersectable *> &instances, std::vector<InstanceId> &instanceIds) {
    for (unsigned long i = 0; i < objectIDs.size(); i++) {
        auto objectId = objectIDs.at(i);
        if (objectIdDeviceMap.count(objectId) == 1) {
            if (objectIdDeviceMap[objectId].id == deviceId.id) {
                auto buffer = dmu->getBaseDataFragment(objectId)->getCapsule();
                auto capsule = ObjectCapsule{ObjectId{i}, buffer.boundingBox, buffer.cost};

                // create instances of objects
                auto instance = std::make_unique<Instance>(dmu.get(), capsule);
                instance->applyTransform(transforms.at(i));
                instances.push_back(instance.get());

                // manage instance ids
                auto instanceId = objectInstanceIds.next();
                objectToInstanceMap[objectId].insert(instanceId);
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
}

std::unique_ptr<PipelineImplement> EngineNode::createPipeline(const PipelineDescription &pipelineDescription,
                                const std::vector<Intersectable *> &instances) {
    // build bvh on instances
    auto root = std::make_unique<DBVHNode>();
    DBVHv2::addObjects(*root, instances);

    // get shader implementation from id
    auto pipelineRayGeneratorShaders = getShaderPackages<RayGeneratorShaderId, RayGeneratorShader>(*pipelinePool,
                                                                                                   pipelineDescription.rayGeneratorShaders);
    auto pipelineOcclusionShaders = getShaderPackages<OcclusionShaderId, OcclusionShader>(*pipelinePool,
                                                                                          pipelineDescription.occlusionShaders);
    auto pipelineHitShaders = getShaderPackages<HitShaderId, HitShader>(*pipelinePool, pipelineDescription.hitShaders);
    auto pipelinePierceShaders = getShaderPackages<PierceShaderId, PierceShader>(*pipelinePool,
                                                                                 pipelineDescription.pierceShaders);
    auto pipelineMissShaders = getShaderPackages<MissShaderId, MissShader>(*pipelinePool,
                                                                           pipelineDescription.missShaders);

    // create new pipeline and add bvh, shaders and description
    return std::make_unique<PipelineImplement>(dmu.get(),
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
}

PipelineId EngineNode::registerPipeline(std::unique_ptr<PipelineImplement> pipeline, std::vector<InstanceId> instanceIds) {
    auto pipelineId = pipelineIds.next();
    // map instances to pipeline
    pipelineToInstanceMap[pipelineId].insert(instanceIds.begin(), instanceIds.end());
    pipelinePool->storePipelineFragments(std::move(pipeline), pipelineId);
    return pipelineId;
}

EngineNode::EngineNode() : deviceId(getDeviceId()) {
    dmu = std::make_unique<DataManagementUnitV2>();
    pipelinePool = std::make_unique<PipelinePool>();
}

EngineNode::~EngineNode() = default;

PipelineId EngineNode::createPipeline(PipelineDescription &pipelineDescription) {
    std::vector<Intersectable *> instances;
    std::vector<InstanceId> instanceIds;

    createInstances(pipelineDescription.objectIDs, pipelineDescription.objectTransformations, instances, instanceIds);
    *pipelineDescription.objectInstanceIDs = instanceIds;

    auto pipeline = createPipeline(pipelineDescription, instances);
    auto pipelineId = registerPipeline(std::move(pipeline), instanceIds);

    // TODO broadcast pipeline to all engine nodes

    return pipelineId;
}

bool EngineNode::removePipeline(PipelineId id) {
    // TODO: broadcast remove to all nodes;
    auto removed = pipelinePool->deletePipelineFragment(id);
    if (removed) {
        for (auto instance: pipelineToInstanceMap.at(id)) {
            removePipelineObject(id, instance);
        }
        pipelineToInstanceMap.erase(id);
        pipelineIds.remove(id);
        return true;
    }
    return false;
}

bool
EngineNode::updatePipelineObjects(PipelineId pipelineId, const std::vector<InstanceId> &instanceIds,
                                  const std::vector<Matrix4x4> &transforms,
                                  const std::vector<ObjectParameter> &objectParameters) {
    if (instanceIds.size() != transforms.size())
        return false;

    for (unsigned long i = 0; i < instanceIds.size(); i++) {
        if (!updateInstance(instanceIds.at(i), transforms.at(i)))
            return false;
    }

    return true;
}

bool EngineNode::removePipelineObject(PipelineId pipelineId, InstanceId objectInstanceId) {
    if (objectInstanceIdDeviceMap.count(objectInstanceId) == 0)
        return false;
    if (objectInstanceIdDeviceMap[objectInstanceId] == deviceId) {
        removeInstanceInPipeline(pipelineId, objectInstanceId);
        return removeInstanceInEngine(pipelineId, objectInstanceId);
    } else {
        // TODO: delete instance on other nodes
        return false;
    }
}

bool
EngineNode::bindGeometryToPipeline(PipelineId pipelineId, const std::vector<ObjectId> &objectIDs,
                                   const std::vector<Matrix4x4> &transforms,
                                   const std::vector<ObjectParameter> &objectParameters,
                                   std::vector<InstanceId> &instanceIDs) {
    auto pipeline = pipelinePool->getPipelineFragment(pipelineId);
    if (objectIDs.size() != transforms.size() || pipeline == nullptr)
        return false;

    auto geometry = pipeline->getGeometry();

    std::vector<Intersectable *> instances;
    std::vector<InstanceId> instanceIds;

    createInstances(objectIDs, transforms, instances, instanceIds);

    pipelineToInstanceMap[pipelineId].insert(instanceIds.begin(), instanceIds.end());

    DBVHv2::addObjects(*geometry, instances);

    return true;
}

ObjectId EngineNode::addObject(const Intersectable &object) {
    auto buffer = objectIds.next();

    objectIdDeviceMap[buffer] = deviceId;

    // TODO: spread over nodes
    auto clone = object.clone();
    dmu->storeBaseDataFragments(std::move(clone), buffer);

    return buffer;
}

bool EngineNode::removeObject(ObjectId id) {
    if (!dmu->deleteBaseDataFragment(id))
        return false;

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
    objectIds.remove(id);
    return true;
}

bool EngineNode::updateObject(ObjectId id, const Intersectable &object) {
    if (dmu->deleteBaseDataFragment(id))
        return false;
    auto clone = object.clone();
    dmu->storeBaseDataFragments(std::move(clone), id);
    return true;
}

HitShaderId EngineNode::addShader(const HitShader &shader) {
    return helpAddShader(shader, *pipelinePool, hitShaderIds);
}

MissShaderId EngineNode::addShader(const MissShader &shader) {
    return helpAddShader(shader, *pipelinePool, missShaderIds);
}

OcclusionShaderId EngineNode::addShader(const OcclusionShader &shader) {
    return helpAddShader(shader, *pipelinePool, occlusionShaderIds);
}

PierceShaderId EngineNode::addShader(const PierceShader &shader) {
    return helpAddShader(shader, *pipelinePool, pierceShaderIds);
}

RayGeneratorShaderId EngineNode::addShader(const RayGeneratorShader &shader) {
    return helpAddShader(shader, *pipelinePool, rayGeneratorShaderIds);
}

bool EngineNode::removeShader(RayGeneratorShaderId id) {
    return helpRemoveShader(*pipelinePool, id, rayGeneratorShaderIds);
}

bool EngineNode::removeShader(HitShaderId id) {
    return helpRemoveShader(*pipelinePool, id, hitShaderIds);
}

bool EngineNode::removeShader(OcclusionShaderId id) {
    return helpRemoveShader(*pipelinePool, id, occlusionShaderIds);
}

bool EngineNode::removeShader(PierceShaderId id) {
    return helpRemoveShader(*pipelinePool, id, pierceShaderIds);
}

bool EngineNode::removeShader(MissShaderId id) {
    return helpRemoveShader(*pipelinePool, id, missShaderIds);
}

ShaderResourceId EngineNode::addShaderResource(const ShaderResource &resource) {
    auto shaderId = shaderResourceIds.next();

    auto clone = resource.clone();
    pipelinePool->storeShaderResource(std::move(clone), shaderId);

    return shaderId;
}

bool EngineNode::removeShaderResource(ShaderResourceId id) {
    if (!pipelinePool->deleteShaderResource(id))
        return false;
    shaderResourceIds.remove(id);
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