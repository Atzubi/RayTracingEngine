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

int RayEngine::addPipeline(Pipeline const &pipeline) {
    while(true){
        int id = random();
        if(pipelines.emplace(std::make_pair(id, pipeline)).second)
            return id;
    }
}

bool RayEngine::removePipeline(int id) {
    return pipelines.erase(id);
}

int RayEngine::runPipeline(int id) {
    return 0;
}

int RayEngine::runAll() {
    return 0;
}

void RayEngine::addGeometry(Geometry geometry) {

}

Object::Object(std::vector<double> const &vertices, std::vector<double> const &normals, std::vector<double> const &map,
               std::vector<uint64_t> const &ids) : vertices(vertices), normals(normals), map(map), ids(ids) {
    if(ids.size() %3 != 0){
        std::__throw_invalid_argument("Invalid ID Count");
    }
    if(vertices.size() != normals.size()){
        if(!normals.empty()){
            std::__throw_invalid_argument("Invalid Normal Count");
        }
    }else if(vertices.size() != map.size()){
        std::__throw_invalid_argument("Invalid Texture Coordinate Count");
    }
}

Object::~Object() = default;

Geometry::Geometry() {

}

Geometry::~Geometry() {

}

int Geometry::addStaticObject(Object object, Vector3D position, Vector3D orientation) {
    return 0;
}

bool Geometry::removeObject(int id) {
    return 0;
}

int Geometry::addAnimatedObject(Object object, Vector3D position, Vector3D orientation) {
    return 0;
}

bool Geometry::moveObject(int id, Vector3D newPosition) {
    return false;
}

bool Geometry::turnObject(int id, Vector3D newOrientation) {
    return false;
}

bool Geometry::moveAndTurnObject(int id, Vector3D newPosition, Vector3D newOrientation) {
    return false;
}

bool Geometry::updateAnimatedObject(int id, Object object) {
    return false;
}
