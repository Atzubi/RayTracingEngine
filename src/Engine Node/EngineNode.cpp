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

void EngineNode::MemoryBlock::storeBaseDataFragments( std::unique_ptr<Object> &object, ObjectId id) {
    objects[id] = std::move(object);
}

void EngineNode::MemoryBlock::storeInstanceDataFragments(std::unique_ptr<Instance> &instance, InstanceId id) {
    objectInstances[id] = std::move(instance);
}

void EngineNode::MemoryBlock::cacheBaseData(std::unique_ptr<Object> &object, ObjectId id) {
    // TODO: implement cache eviction
    objectCache[id] = std::move(object);
}

void EngineNode::MemoryBlock::cacheInstanceData(std::unique_ptr<Instance> &instance, InstanceId id) {
    // TODO: implement cache eviction
    objectInstanceCache[id] = std::move(instance);
}

void EngineNode::MemoryBlock::storeShaderResource(ShaderResource *shaderResource, ShaderResourceId id) {
    shaderResources[id] = shaderResource;
}

bool EngineNode::MemoryBlock::deleteShaderResource(ShaderResourceId id) {
    if (shaderResources.count(id) == 0) return false;

    shaderResources.erase(id);

    return true;
}

bool EngineNode::MemoryBlock::deleteBaseDataFragment(ObjectId id) {
    if (objects.count(id) == 0) return false;
    objects.erase(id);
    return true;
}

bool EngineNode::MemoryBlock::deleteInstanceDataFragment(InstanceId id) {
    if (objectInstances.count(id) == 0) return false;
    objectInstances.erase(id);
    return true;
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
    return shaderResources.at(id);
}

EngineNode::PipelineBlock::PipelineBlock() = default;

EngineNode::PipelineBlock::~PipelineBlock() {
    for (auto p: pipelines) {
        delete p.second;
    }
}

void EngineNode::PipelineBlock::storePipelineFragments(PipelineImplement *pipeline, PipelineId id) {
    pipelines[id] = pipeline;
}

bool EngineNode::PipelineBlock::deletePipelineFragment(PipelineId id) {
    if (pipelines.count(id) == 0) return false;
    delete pipelines[id];
    pipelines.erase(id);
    return true;
}

void EngineNode::PipelineBlock::addShader(RayGeneratorShaderId id, RayGeneratorShader *shader) {
    rayGeneratorShaders[id] = shader;
}

void EngineNode::PipelineBlock::addShader(HitShaderId id, HitShader *shader) {
    hitShaders[id] = shader;
}

void EngineNode::PipelineBlock::addShader(OcclusionShaderId id, OcclusionShader *shader) {
    occlusionShaders[id] = shader;
}

void EngineNode::PipelineBlock::addShader(PierceShaderId id, PierceShader *shader) {
    pierceShaders[id] = shader;
}

void EngineNode::PipelineBlock::addShader(MissShaderId id, MissShader *shader) {
    missShaders[id] = shader;
}

RayGeneratorShader *EngineNode::PipelineBlock::getShader(RayGeneratorShaderId id) {
    if (rayGeneratorShaders.count(id) != 0) {
        return rayGeneratorShaders.at(id);
    }
    return nullptr;
}

HitShader *EngineNode::PipelineBlock::getShader(HitShaderId id) {
    if (hitShaders.count(id) != 0) {
        return hitShaders.at(id);
    }
    return nullptr;
}

OcclusionShader *EngineNode::PipelineBlock::getShader(OcclusionShaderId id) {
    if (occlusionShaders.count(id) != 0) {
        return occlusionShaders.at(id);
    }
    return nullptr;
}

PierceShader *EngineNode::PipelineBlock::getShader(PierceShaderId id) {
    if (pierceShaders.count(id) != 0) {
        return pierceShaders.at(id);
    }
    return nullptr;
}

MissShader *EngineNode::PipelineBlock::getShader(MissShaderId id) {
    if (missShaders.count(id) != 0) {
        return missShaders.at(id);
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
    for (auto p: pipelines) {
        p.second->run();
    }
}

PipelineImplement *EngineNode::PipelineBlock::getPipelineFragment(PipelineId id) {
    if (pipelines.count(id) == 0) return nullptr;
    return pipelines[id];
}

EngineNode::EngineNode(DataManagementUnitV2 *DMU) {
    dataManagementUnit = DMU;
    memoryBlock = new MemoryBlock();
    pipelineBlock = new PipelineBlock();
}

EngineNode::~EngineNode() {
    delete memoryBlock;
    delete pipelineBlock;
}

void EngineNode::storeBaseDataFragments(std::unique_ptr<Object> &object, ObjectId id) {
    memoryBlock->storeBaseDataFragments(object, id);
}

void EngineNode::storeInstanceDataFragments(std::unique_ptr<Instance> &instance, InstanceId id) {
    memoryBlock->storeInstanceDataFragments(instance, id);
}

void EngineNode::cacheBaseData(std::unique_ptr<Object> &object, ObjectId id) {
    memoryBlock->cacheBaseData(object, id);
}

void EngineNode::cacheInstanceData(std::unique_ptr<Instance> &instance, InstanceId id) {
    memoryBlock->cacheInstanceData(instance, id);
}

void EngineNode::storeShaderResource(ShaderResource *shaderResource, ShaderResourceId id) {
    memoryBlock->storeShaderResource(shaderResource, id);
}

bool EngineNode::deleteShaderResource(ShaderResourceId id) {
    return memoryBlock->deleteShaderResource(id);
}

void EngineNode::storePipelineFragments(PipelineImplement *pipeline, PipelineId id) {
    pipeline->setEngine(this);
    pipelineBlock->storePipelineFragments(pipeline, id);
}

bool EngineNode::deletePipelineFragment(PipelineId id) {
    return pipelineBlock->deletePipelineFragment(id);
}

Object *EngineNode::requestBaseData(ObjectId id) {
    auto fragment = memoryBlock->getBaseDataFragment(id);
    if (fragment == nullptr) {
        auto sharedFragment = dataManagementUnit->getBaseDataFragment(id);
        memoryBlock->cacheBaseData(sharedFragment, id);
    }
    return fragment;
}

Instance *EngineNode::requestInstanceData(InstanceId id) {
    auto fragment = memoryBlock->getInstanceDataFragment(id);
    if (fragment == nullptr) {
        auto sharedFragment = dataManagementUnit->getInstanceDataFragment(id);
        memoryBlock->cacheInstanceData(sharedFragment, id);
    }
    return fragment;
}

ShaderResource *EngineNode::getShaderResource(ShaderResourceId id) {
    return memoryBlock->getShaderResource(id);
}

PipelineImplement *EngineNode::requestPipelineFragment(PipelineId id) {
    return pipelineBlock->getPipelineFragment(id);
}

void EngineNode::addShader(RayGeneratorShaderId id, RayGeneratorShader *shader) {
    pipelineBlock->addShader(id, shader);
}

void EngineNode::addShader(HitShaderId id, HitShader *shader) {
    pipelineBlock->addShader(id, shader);
}

void EngineNode::addShader(OcclusionShaderId id, OcclusionShader *shader) {
    pipelineBlock->addShader(id, shader);
}

void EngineNode::addShader(PierceShaderId id, PierceShader *shader) {
    pipelineBlock->addShader(id, shader);
}

void EngineNode::addShader(MissShaderId id, MissShader *shader) {
    pipelineBlock->addShader(id, shader);
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
