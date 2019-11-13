//
// Created by sebastian on 13.11.19.
//

#include "../../API/Object.h"

bool Object::moveObject(Vector3D newPosition) {
    return false;
}

bool Object::turnObject(Vector3D newOrientation) {
    return false;
}

bool Object::scaleObject(double newScaleFactor) {
    return false;
}

bool Object::manipulateObject(Vector3D newPosition, Vector3D newOrientation, double newScaleFactor) {
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