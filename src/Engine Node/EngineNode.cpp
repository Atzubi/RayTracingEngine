//
// Created by Sebastian on 28.10.2021.
//

#include "EngineNode.h"
#include "Data Management/DataManagementUnitV2.h"
#include "RayTraceEngine/Object.h"
#include "Pipeline/PipelineImplement.h"
#include "Object/Instance.h"
#include "Acceleration Structures/DBVHv2.h"

EngineNode::MemoryBlock::MemoryBlock() = default;

EngineNode::MemoryBlock::~MemoryBlock() {
    for (auto o: objects) {
        delete o.second;
    }
    for (auto o: objectInstances) {
        delete o.second;
    }
    for (auto o: objectCache) {
        delete o.second;
    }
    for (auto o: objectInstanceCache) {
        delete o.second;
    }
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

void EngineNode::MemoryBlock::storeShaderResource(ShaderResource *shaderResource, int id) {
    shaderResources[id] = shaderResource;
}

bool EngineNode::MemoryBlock::deleteShaderResource(int id) {
    if (shaderResources.count(id) == 0) return false;

    shaderResources.erase(id);

    return true;
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
            // object is not currently in the cache
            return nullptr;
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

EngineNode::PipelineBlock::PipelineBlock() = default;

EngineNode::PipelineBlock::~PipelineBlock() {
    for (auto p: pipelines) {
        delete p.second;
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

void EngineNode::PipelineBlock::addShader(int id, RayGeneratorShader *shader) {
    rayGeneratorShaders[id] = shader;
}

void EngineNode::PipelineBlock::addShader(int id, HitShader *shader) {
    hitShaders[id] = shader;
}

void EngineNode::PipelineBlock::addShader(int id, OcclusionShader *shader) {
    occlusionShaders[id] = shader;
}

void EngineNode::PipelineBlock::addShader(int id, PierceShader *shader) {
    pierceShaders[id] = shader;
}

void EngineNode::PipelineBlock::addShader(int id, MissShader *shader) {
    missShaders[id] = shader;
}

Shader *EngineNode::PipelineBlock::getShader(int id) {
    if (hitShaders.count(id) != 0) {
        return (Shader *) hitShaders.at(id);
    } else if (missShaders.count(id) != 0) {
        return (Shader *) missShaders.at(id);
    } else if (occlusionShaders.count(id) != 0) {
        return (Shader *) occlusionShaders.at(id);
    } else if (pierceShaders.count(id) != 0) {
        return (Shader *) pierceShaders.at(id);
    } else if (rayGeneratorShaders.count(id) != 0) {
        return (Shader *) rayGeneratorShaders.at(id);
    } else {
        return nullptr;
    }
}

bool EngineNode::PipelineBlock::deleteShader(int id) {
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
    memoryBlock = new MemoryBlock();
    pipelineBlock = new PipelineBlock();
}

EngineNode::~EngineNode() {
    delete memoryBlock;
    delete pipelineBlock;
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

void EngineNode::storeShaderResource(ShaderResource *shaderResource, int id) {
    memoryBlock->storeShaderResource(shaderResource, id);
}

bool EngineNode::deleteShaderResource(int id) {
    return memoryBlock->deleteShaderResource(id);
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

void EngineNode::addShader(int id, RayGeneratorShader *shader) {
    pipelineBlock->addShader(id, shader);
}

void EngineNode::addShader(int id, HitShader *shader) {
    pipelineBlock->addShader(id, shader);
}

void EngineNode::addShader(int id, OcclusionShader *shader) {
    pipelineBlock->addShader(id, shader);
}

void EngineNode::addShader(int id, PierceShader *shader) {
    pipelineBlock->addShader(id, shader);
}

void EngineNode::addShader(int id, MissShader *shader) {
    pipelineBlock->addShader(id, shader);
}

Shader *EngineNode::getShader(int id) {
    return pipelineBlock->getShader(id);
}

bool EngineNode::deleteShader(int id) {
    return pipelineBlock->deleteShader(id);
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
