//
// Created by Sebastian on 28.10.2021.
//

#include "EngineNode.h"

void EngineNode::MemoryBlock::storeBaseDataFragments(Object* object, int id) {

}

void EngineNode::MemoryBlock::storeInstanceDataFragments(Instance* instance, int id) {

}

void EngineNode::MemoryBlock::cacheBaseData(Object* object, int id) {

}

void EngineNode::MemoryBlock::cacheInstanceData(Instance* instance, int id) {

}

void EngineNode::MemoryBlock::requestTraversalData(int id) {

}

void EngineNode::PipelineBlock::storePipelineFragments() {

}

void EngineNode::PipelineBlock::requestPipelineData() {

}

bool EngineNode::MemoryBlock::deleteBaseDataFragment(int id) {
    return false;
}

bool EngineNode::MemoryBlock::deleteInstanceDataFragment(int id) {
    return false;
}

Object *EngineNode::MemoryBlock::getBaseDataFragment(int id) {
    return nullptr;
}

Instance *EngineNode::MemoryBlock::getInstanceDataFragment(int id) {
    return nullptr;
}

EngineNode::EngineNode() {

}

void EngineNode::storeBaseDataFragments(Object *object, int id) {

}

void EngineNode::storeInstanceDataFragments(Instance *instance, int id) {

}

void EngineNode::cacheBaseData(Object *object, int id) {

}

void EngineNode::cacheInstanceData(Instance *instance, int id) {

}

void EngineNode::storePipelineFragments() {

}

Object* EngineNode::requestBaseData(int id) {
    return nullptr;
}

Instance* EngineNode::requestInstanceData(int id) {
    return nullptr;
}

void EngineNode::runPipelines() {

}

bool EngineNode::deleteBaseDataFragment(int id) {
    return false;
}

bool EngineNode::deleteInstanceDataFragment(int id) {
    return false;
}
