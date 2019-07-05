//
// Created by sebastian on 02.07.19.
//

#include <iostream>
#include "../API/RayEngine.h"
#include "./Data Management Unit/DataManagementUnit.cpp"

RayEngine::RayEngine() {
    dataManagementUnit = new DataManagementUnit();
}

RayEngine::~RayEngine() {
    delete dataManagementUnit;
}

bool RayEngine::addPipeline(Pipeline const &pipeline) {
    for(auto p : pipelines){
        if(&p == &pipeline)
            return false;
    }
    pipelines.push_back(pipeline);
    return true;
}

bool RayEngine::removePipeline(Pipeline const &pipeline) {
    std::vector<Pipeline> newPipelines;
    for(auto p : pipelines) {
        if (&p != &pipeline)
            newPipelines.push_back(p);
    }
    bool deleted = newPipelines.size() != pipelines.size();
    pipelines = newPipelines;
    return deleted;
}

int RayEngine::runPipeline(Pipeline const &pipeline) {
    return 0;
}

int RayEngine::runAll() {
    return 0;
}

void RayEngine::addGeometry() {

}

void *MissShader::getAssociatedData() {
    return nullptr;
}

ShaderOutput MissShader::shade(int id, RayTracerOutput shaderInput, void *dataInput) {
    return ShaderOutput();
}

Object::Object(std::vector<double> &vertices, std::vector<double> &normals, std::vector<double> &map,
               std::vector<uint64_t> &ids) : vertices(vertices), normals(normals), map(map), ids(ids) {
    if(ids.size() %3 != 0){
        std::__throw_invalid_argument("Invalid ID Count");
    }
    if(vertices.size() != normals.size()){
        if(normals.size() != 0){
            std::__throw_invalid_argument("Invalid Normal Count");
        }
    }else if(vertices.size() != map.size()){
        std::__throw_invalid_argument("Invalid Texture Coordinate Count");
    }
}

Object::~Object() = default;
