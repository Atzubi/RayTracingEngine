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

bool RayEngine::removeObject(int id) {
    return false;
}

bool RayEngine::moveObject(int id, Vector3D newPosition) {
    return false;
}

bool RayEngine::turnObject(int id, Vector3D newOrientation) {
    return false;
}

int RayEngine::addObject(Object const &object, Vector3D position, Vector3D orientation, double newScaleFactor,
                         RayEngine::ObjectParameter objectParameter) {
    return 0;
}

bool RayEngine::updateObject(int id, Object const &object) {
    return false;
}

bool RayEngine::scaleObject(int id, double newScaleFactor) {
    return false;
}

bool RayEngine::manipulateObject(int id, Vector3D newPosition, Vector3D newOrientation, double newScaleFactor) {
    return false;
}

Object::Object(std::vector<double>* const vertices, std::vector<double>* const normals, std::vector<double>* const map,
               std::vector<uint64_t>* const ids) : vertices(vertices), normals(normals), map(map), ids(ids) {
    if(ids->size() %3 != 0){
        std::__throw_invalid_argument("Invalid ID Count");
    }
    if(vertices->size() != normals->size()){
        if(!normals->empty()){
            std::__throw_invalid_argument("Invalid Normal Count");
        }
    }else if(vertices->size() != map->size()){
        std::__throw_invalid_argument("Invalid Texture Coordinate Count");
    }
}

Object::~Object() = default;
