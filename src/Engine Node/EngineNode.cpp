//
// Created by Sebastian on 28.10.2021.
//

#include "EngineNode.h"
#include "Data Management/DataManagementUnitV2.h"
#include "RayTraceEngine/Object.h"
#include "Pipeline/PipelineImplement.h"
#include "Object/Instance.h"

EngineNode::MemoryBlock::MemoryBlock(EngineNode *engine) {
    engineNode = engine;
}

void EngineNode::MemoryBlock::storeBaseDataFragments(Object *object, int id) {
    objects[id] = object;
}

void EngineNode::MemoryBlock::storeInstanceDataFragments(Instance *instance, int id) {
    objectInstances[id] = instance;
}

void EngineNode::MemoryBlock::cacheBaseData(Object *object, int id) {
    // TODO: implement cache eviction
    objectCache[id] = object;
}

void EngineNode::MemoryBlock::cacheInstanceData(Instance *instance, int id) {
    // TODO: implement cache eviction
    objectInstanceCache[id] = instance;
}

bool EngineNode::MemoryBlock::deleteBaseDataFragment(int id) {
    if (objects.count(id) == 0) return false;
    delete objects[id];
    objects.erase(id);
    return true;
}

bool EngineNode::MemoryBlock::deleteInstanceDataFragment(int id) {
    if (objectInstances.count(id) == 0) return false;
    delete objectInstances[id];
    objectInstances.erase(id);
    return true;
}

Object *EngineNode::MemoryBlock::getBaseDataFragment(int id) {
    if (objects.count(id) == 0) {
        // object was not originally stored on this node, check cache
        if (objectCache.count(id) == 0) {
            // object is not currently in the cache, request it from other nodes
            return engineNode->dataManagementUnit->getBaseDataFragment(id);
        } else {
            // object was found in cache
            return objectCache[id];
        }
    } else {
        // object was found in node
        return objects[id];
    }
}

Instance *EngineNode::MemoryBlock::getInstanceDataFragment(int id) {
    if (objects.count(id) == 0) {
        // object was not originally stored on this node, check cache
        if (objectCache.count(id) == 0) {
            // object is not currently in the cache
            return nullptr;
        } else {
            // object was found in cache
            return objectInstanceCache[id];
        }
    } else {
        // object was found in node
        return objectInstances[id];
    }
}

void EngineNode::PipelineBlock::storePipelineFragments(PipelineImplement *pipeline, int id) {
    pipelines[id] = pipeline;
}

bool EngineNode::PipelineBlock::deletePipelineFragment(int id) {
    if (pipelines.count(id) == 0) return false;
    delete pipelines[id];
    pipelines.erase(id);
    return true;
}

void EngineNode::PipelineBlock::runPipeline(int id) {
    if (pipelines.count(id) == 1) {
        pipelines[id]->run();
    }
}

void EngineNode::PipelineBlock::runPipelines() {
    for (auto p: pipelines) {
        p.second->run();
    }
}

PipelineImplement *EngineNode::PipelineBlock::getPipelineFragment(int id) {
    if (pipelines.count(id) == 0) return nullptr;
    return pipelines[id];
}

EngineNode::EngineNode(DataManagementUnitV2 *DMU) {
    dataManagementUnit = DMU;
    memoryBlock = new MemoryBlock(this);
    pipelineBlock = new PipelineBlock();
}

void EngineNode::storeBaseDataFragments(Object *object, int id) {
    memoryBlock->storeBaseDataFragments(object, id);
}

void EngineNode::storeInstanceDataFragments(Instance *instance, int id) {
    memoryBlock->storeInstanceDataFragments(instance, id);
}

void EngineNode::cacheBaseData(Object *object, int id) {
    memoryBlock->cacheBaseData(object, id);
}

void EngineNode::cacheInstanceData(Instance *instance, int id) {
    memoryBlock->cacheInstanceData(instance, id);
}

void EngineNode::storePipelineFragments(PipelineImplement *pipeline, int id) {
    pipeline->setEngine(this);
    pipelineBlock->storePipelineFragments(pipeline, id);
}

bool EngineNode::deletePipelineFragment(int id) {
    return pipelineBlock->deletePipelineFragment(id);
}

Object *EngineNode::requestBaseData(int id) {
    auto fragment = memoryBlock->getBaseDataFragment(id);
    if (fragment == nullptr) {
        fragment = dataManagementUnit->getBaseDataFragment(id);
        memoryBlock->cacheBaseData(fragment, id);
    }
    return fragment;
}

Instance *EngineNode::requestInstanceData(int id) {
    auto fragment = memoryBlock->getInstanceDataFragment(id);
    if (fragment == nullptr) {
        fragment = dataManagementUnit->getInstanceDataFragment(id);
        memoryBlock->cacheInstanceData(fragment, id);
    }
    return fragment;
}

PipelineImplement *EngineNode::requestPipelineFragment(int id) {
    return pipelineBlock->getPipelineFragment(id);
}

void EngineNode::runPipeline(int id) {
    pipelineBlock->runPipeline(id);
}

void EngineNode::runPipelines() {
    pipelineBlock->runPipelines();
}

bool EngineNode::deleteBaseDataFragment(int id) {
    return memoryBlock->deleteBaseDataFragment(id);
}

bool EngineNode::deleteInstanceDataFragment(int id) {
    return memoryBlock->deleteInstanceDataFragment(id);
}
