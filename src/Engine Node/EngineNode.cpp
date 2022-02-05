//
// Created by Sebastian on 28.10.2021.
//

#include "EngineNode.h"

#include <utility>
#include "Data Management/DataManagementUnitV2.h"
#include "RayTraceEngine/Object.h"
#include "Pipeline/PipelineImplement.h"
#include "Object/Instance.h"
#include "Acceleration Structures/DBVHv2.h"

EngineNode::MemoryBlock::MemoryBlock() = default;

EngineNode::MemoryBlock::~MemoryBlock() = default;

void EngineNode::MemoryBlock::storeBaseDataFragments( std::unique_ptr<Object> object, ObjectId id) {
    objects[id] = std::move(object);
}

void EngineNode::MemoryBlock::storeInstanceDataFragments(std::unique_ptr<Instance> instance, InstanceId id) {
    objectInstances[id] = std::move(instance);
}

void EngineNode::MemoryBlock::cacheBaseData(std::unique_ptr<Object> object, ObjectId id) {
    // TODO: implement cache eviction
    objectCache[id] = std::move(object);
}

void EngineNode::MemoryBlock::cacheInstanceData(std::unique_ptr<Instance> instance, InstanceId id) {
    // TODO: implement cache eviction
    objectInstanceCache[id] = std::move(instance);
}

void EngineNode::MemoryBlock::storeShaderResource(std::unique_ptr<ShaderResource> shaderResource, ShaderResourceId id) {
    shaderResources[id] = std::move(shaderResource);
}

bool EngineNode::MemoryBlock::deleteShaderResource(ShaderResourceId id) {
    const bool objectRemoved = shaderResources.erase(id) != 0;
    return objectRemoved;
}

bool EngineNode::MemoryBlock::deleteBaseDataFragment(ObjectId id) {
    const bool objectRemoved = objects.erase(id) != 0;
    return objectRemoved;
}

bool EngineNode::MemoryBlock::deleteInstanceDataFragment(InstanceId id) {
    const bool objectRemoved = objectInstances.erase(id) != 0;
    return objectRemoved;
}

Object *EngineNode::MemoryBlock::getBaseDataFragment(ObjectId id) {
    if (objects.count(id) == 0) {
        // object was not originally stored on this node, check cache
        if (objectCache.count(id) == 0) {
            // object is not currently in the cache
            return nullptr;
        } else {
            // object was found in cache
            return objectCache[id].get();
        }
    } else {
        // object was found in node
        return objects[id].get();
    }
}

Instance *EngineNode::MemoryBlock::getInstanceDataFragment(InstanceId id) {
    if (objectInstances.count(id) == 0) {
        // object was not originally stored on this node, check cache
        if (objectInstanceCache.count(id) == 0) {
            // object is not currently in the cache
            return nullptr;
        } else {
            // object was found in cache
            return objectInstanceCache[id].get();
        }
    } else {
        // object was found in node
        return objectInstances[id].get();
    }
}

ShaderResource *EngineNode::MemoryBlock::getShaderResource(ShaderResourceId id) {
    return shaderResources.at(id).get();
}

EngineNode::PipelineBlock::PipelineBlock() = default;

EngineNode::PipelineBlock::~PipelineBlock() = default;

void EngineNode::PipelineBlock::storePipelineFragments(std::unique_ptr<PipelineImplement> pipeline, PipelineId id) {
    pipelines[id] = std::move(pipeline);
}

bool EngineNode::PipelineBlock::deletePipelineFragment(PipelineId id) {
    if (pipelines.count(id) == 0) return false;
    pipelines.erase(id);
    return true;
}

void EngineNode::PipelineBlock::addShader(RayGeneratorShaderId id, std::unique_ptr<RayGeneratorShader> shader) {
    rayGeneratorShaders[id] = std::move(shader);
}

void EngineNode::PipelineBlock::addShader(HitShaderId id, std::unique_ptr<HitShader> shader) {
    hitShaders[id] = std::move(shader);
}

void EngineNode::PipelineBlock::addShader(OcclusionShaderId id, std::unique_ptr<OcclusionShader> shader) {
    occlusionShaders[id] = std::move(shader);
}

void EngineNode::PipelineBlock::addShader(PierceShaderId id, std::unique_ptr<PierceShader> shader) {
    pierceShaders[id] = std::move(shader);
}

void EngineNode::PipelineBlock::addShader(MissShaderId id, std::unique_ptr<MissShader> shader) {
    missShaders[id] = std::move(shader);
}

RayGeneratorShader *EngineNode::PipelineBlock::getShader(RayGeneratorShaderId id) {
    if (rayGeneratorShaders.count(id) != 0) {
        return rayGeneratorShaders.at(id).get();
    }
    return nullptr;
}

HitShader *EngineNode::PipelineBlock::getShader(HitShaderId id) {
    if (hitShaders.count(id) != 0) {
        return hitShaders.at(id).get();
    }
    return nullptr;
}

OcclusionShader *EngineNode::PipelineBlock::getShader(OcclusionShaderId id) {
    if (occlusionShaders.count(id) != 0) {
        return occlusionShaders.at(id).get();
    }
    return nullptr;
}

PierceShader *EngineNode::PipelineBlock::getShader(PierceShaderId id) {
    if (pierceShaders.count(id) != 0) {
        return pierceShaders.at(id).get();
    }
    return nullptr;
}

MissShader *EngineNode::PipelineBlock::getShader(MissShaderId id) {
    if (missShaders.count(id) != 0) {
        return missShaders.at(id).get();
    }
    return nullptr;
}

bool EngineNode::PipelineBlock::deleteShader(RayGeneratorShaderId id) {
    if (rayGeneratorShaders.count(id) != 0) {
        rayGeneratorShaders.erase(id);
        return true;
    }
    return false;
}

bool EngineNode::PipelineBlock::deleteShader(HitShaderId id) {
    if (hitShaders.count(id) != 0) {
        hitShaders.erase(id);
        return true;
    }
    return false;
}

bool EngineNode::PipelineBlock::deleteShader(OcclusionShaderId id) {
    if (occlusionShaders.count(id) != 0) {
        occlusionShaders.erase(id);
        return true;
    }
    return false;
}

bool EngineNode::PipelineBlock::deleteShader(PierceShaderId id) {
    if (pierceShaders.count(id) != 0) {
        pierceShaders.erase(id);
        return true;
    }
    return false;
}

bool EngineNode::PipelineBlock::deleteShader(MissShaderId id) {
    if (missShaders.count(id) != 0) {
        missShaders.erase(id);
        return true;
    }
    return false;
}

void EngineNode::PipelineBlock::runPipeline(PipelineId id) {
    if (pipelines.count(id) == 1) {
        pipelines[id]->run();
    }
}

void EngineNode::PipelineBlock::runPipelines() {
    for (auto &p : pipelines) {
        p.second->run();
    }
}

PipelineImplement *EngineNode::PipelineBlock::getPipelineFragment(PipelineId id) {
    if (pipelines.count(id) == 0) return nullptr;
    return pipelines[id].get();
}

EngineNode::EngineNode(DataManagementUnitV2 *DMU) {
    dataManagementUnit = DMU;
    memoryBlock = std::make_unique<MemoryBlock>();
    pipelineBlock = std::make_unique<PipelineBlock>();
}

EngineNode::~EngineNode() = default;

void EngineNode::storeBaseDataFragments(std::unique_ptr<Object> object, ObjectId id) {
    memoryBlock->storeBaseDataFragments(std::move(object), id);
}

void EngineNode::storeInstanceDataFragments(std::unique_ptr<Instance> instance, InstanceId id) {
    memoryBlock->storeInstanceDataFragments(std::move(instance), id);
}

void EngineNode::cacheBaseData(std::unique_ptr<Object> object, ObjectId id) {
    memoryBlock->cacheBaseData(std::move(object), id);
}

void EngineNode::cacheInstanceData(std::unique_ptr<Instance> instance, InstanceId id) {
    memoryBlock->cacheInstanceData(std::move(instance), id);
}

void EngineNode::storeShaderResource(std::unique_ptr<ShaderResource> shaderResource, ShaderResourceId id) {
    memoryBlock->storeShaderResource(std::move(shaderResource), id);
}

bool EngineNode::deleteShaderResource(ShaderResourceId id) {
    return memoryBlock->deleteShaderResource(id);
}

void EngineNode::storePipelineFragments(std::unique_ptr<PipelineImplement> pipeline, PipelineId id) {
    pipeline->setEngine(this);
    pipelineBlock->storePipelineFragments(std::move(pipeline), id);
}

bool EngineNode::deletePipelineFragment(PipelineId id) {
    return pipelineBlock->deletePipelineFragment(id);
}

Object *EngineNode::requestBaseData(ObjectId id) {
    auto fragment = memoryBlock->getBaseDataFragment(id);
    if (fragment == nullptr) {
        auto sharedFragment = dataManagementUnit->getBaseDataFragment(id);
        memoryBlock->cacheBaseData(std::move(sharedFragment), id);
    }
    return memoryBlock->getBaseDataFragment(id);
}

Instance *EngineNode::requestInstanceData(InstanceId id) {
    auto fragment = memoryBlock->getInstanceDataFragment(id);
    if (fragment == nullptr) {
        auto sharedFragment = dataManagementUnit->getInstanceDataFragment(id);
        memoryBlock->cacheInstanceData(std::move(sharedFragment), id);
    }
    return memoryBlock->getInstanceDataFragment(id);
}

ShaderResource *EngineNode::getShaderResource(ShaderResourceId id) {
    return memoryBlock->getShaderResource(id);
}

PipelineImplement *EngineNode::requestPipelineFragment(PipelineId id) {
    return pipelineBlock->getPipelineFragment(id);
}

void EngineNode::addShader(RayGeneratorShaderId id, std::unique_ptr<RayGeneratorShader> shader) {
    pipelineBlock->addShader(id, std::move(shader));
}

void EngineNode::addShader(HitShaderId id, std::unique_ptr<HitShader>shader) {
    pipelineBlock->addShader(id, std::move(shader));
}

void EngineNode::addShader(OcclusionShaderId id, std::unique_ptr<OcclusionShader> shader) {
    pipelineBlock->addShader(id, std::move(shader));
}

void EngineNode::addShader(PierceShaderId id, std::unique_ptr<PierceShader> shader) {
    pipelineBlock->addShader(id, std::move(shader));
}

void EngineNode::addShader(MissShaderId id, std::unique_ptr<MissShader> shader) {
    pipelineBlock->addShader(id, std::move(shader));
}

RayGeneratorShader *EngineNode::getShader(RayGeneratorShaderId id) {
    return pipelineBlock->getShader(id);
}

HitShader *EngineNode::getShader(HitShaderId id) {
    return pipelineBlock->getShader(id);
}

OcclusionShader *EngineNode::getShader(OcclusionShaderId id) {
    return pipelineBlock->getShader(id);
}

PierceShader *EngineNode::getShader(PierceShaderId id) {
    return pipelineBlock->getShader(id);
}

MissShader *EngineNode::getShader(MissShaderId id) {
    return pipelineBlock->getShader(id);
}

bool EngineNode::deleteShader(RayGeneratorShaderId id) {
    return pipelineBlock->deleteShader(id);
}

bool EngineNode::deleteShader(HitShaderId id) {
    return pipelineBlock->deleteShader(id);
}

bool EngineNode::deleteShader(OcclusionShaderId id) {
    return pipelineBlock->deleteShader(id);
}

bool EngineNode::deleteShader(PierceShaderId id) {
    return pipelineBlock->deleteShader(id);
}

bool EngineNode::deleteShader(MissShaderId id) {
    return pipelineBlock->deleteShader(id);
}

void EngineNode::runPipeline(PipelineId id) {
    pipelineBlock->runPipeline(id);
}

void EngineNode::runPipelines() {
    pipelineBlock->runPipelines();
}

bool EngineNode::deleteBaseDataFragment(ObjectId id) {
    return memoryBlock->deleteBaseDataFragment(id);
}

bool EngineNode::deleteInstanceDataFragment(InstanceId id) {
    return memoryBlock->deleteInstanceDataFragment(id);
}
