//
// Created by sebastian on 02.07.19.
//

#include <iostream>
#include "../API/RayEngine.h"
#include "./Data Management/DataManagementUnit.h"

RayEngine::RayEngine() {
    dataManagementUnit = new DataManagementUnit();
}

RayEngine::~RayEngine() {
    delete dataManagementUnit;
}

int RayEngine::runPipeline(int id) {
    return 0;
}

int RayEngine::runAll() {
    return 0;
}

int RayEngine::addPipeline(Pipeline const &pipeline) {
    return 0;
}

bool RayEngine::removePipeline(int id) {
    return false;
}

bool RayEngine::bindGeometryToPipeline(int pipelineId, std::vector<int>* objectIds) {
    return false;
}

int RayEngine::addObject(Object const &object, Vector3D position, Vector3D orientation, double newScaleFactor,
                      RayEngine::ObjectParameter objectParameter) {
    return 0;
}

bool RayEngine::removeObject(int id) {
    return false;
}

bool RayEngine::updateObject(int id, Object const &object) {
    return false;
}

bool RayEngine::bindShaderToPipeline(int pipelineId, int shaderId, std::vector<int> shaderResourceIds) {
    return false;
}

int RayEngine::addShader(Shader shader) {
    return 0;
}

bool RayEngine::removeShader(int id) {
    return false;
}

int RayEngine::addShaderResource(Any resource) {
    return 0;
}

bool RayEngine::removeShaderResource(int id) {
    return false;
}
